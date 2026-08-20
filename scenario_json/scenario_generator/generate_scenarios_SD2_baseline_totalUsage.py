#!/usr/bin/env python3

import os
import sys
import json
import math
import shutil
import hashlib
import subprocess
import itertools
from datetime import datetime
from concurrent.futures import ProcessPoolExecutor, as_completed
import random

# ==========================================
# GLOBAL CONFIGURATION
# ==========================================
#NAME="SD2_baseline_20runsx20reqx01cons_4servx10nodes_0P4_200PF_totalUsage"
#NAME="SD2_baseline_20runsx20reqx05cons_4servx10nodes_0P4_200PF_totalUsage_Pool5x"
NAME="SD2_baseline_20runsx01reqx20cons_4servx10nodes_0P4_200PF_totalUsage"
TIMESTAMP = datetime.now().strftime("%Y%m%d-%H%M%S")
WORKDIR = os.path.join(os.getcwd(), "generated_scenarios", NAME)
OUTDIR = os.path.join(os.getcwd(), "..", NAME)

# This removes the folder and everything inside it permanently
if os.path.exists(WORKDIR):
    shutil.rmtree(WORKDIR)
if os.path.exists(OUTDIR):
    shutil.rmtree(OUTDIR)

# Total number of runs (each will get it's own JSON and thus its own row in the CSV file - MATLAB will average them all)
NUM_RUNS = 20

NUM_SERVICES_LIST = [4]

# Workflows draw their service names from a pool this many times larger than num_services. 1.0 keeps
# the historical naming (/service0../service{n-1}). Raising it makes two independently generated
# workflows much less likely to name the same services, so consumers running their own workflow share
# less. E.g. 3.0 with 5 services samples 5 distinct names out of /service0../service14.
SERVICE_POOL_MULTIPLIER = 1.0
NUM_NODES_LIST = [10]
EDGERATIO_LIST = [0.5]
#HOSTRATIO_LIST = [0.0, 0.2, 0.4, 0.6, 0.8, 1.0]
#HOSTRATIO_LIST = [0.0, 0.1, 0.2, 0.3]
HOSTRATIO_LIST = [0.1, 0.2, 0.3, 0.4]

LINK_DELAY_AVG_MS = 1
LINK_DELAY_VARIATION_PCT = 0.80  # percent variation.
#CCR_LIST = [0.1, 0.5, 1, 2, 10]  # CCR is communication to computation ratio
CCR_LIST = [0.01, 0.1, 1]  # CCR is communication to computation ratio
#CCR_LIST = [0.01]  # CCR is communication to computation ratio
MAKESPAN_VARIATION_PCT = 0.80  # percent variation.

# Consumer options
#POISSON_FREQ = 2 # wait on average 500ms between receiving results from prev WF, and generating SD2 request for next WF.
#POISSON_FREQ = 100 # wait on average 10ms between receiving results from prev WF, and generating SD2 request for next WF.
POISSON_FREQ = 200 # wait on average 5ms between receiving results from prev WF, and generating SD2 request for next WF.
#POISSON_NUM_INTERESTS = 100
#POISSON_NUM_CONSUMERS = 100
POISSON_NUM_INTERESTS = 1
POISSON_NUM_CONSUMERS = 20

PRODUCER_FRESHNESS_UNIFORM_DIST = 0 # 0: not random, use hardcoded value. 1: randomly chosen once. 2: randomly chosen each time an interest arrives.
PRODUCER_FRESHNESS_MS_MIN = 0
PRODUCER_FRESHNESS_MS_MAX = 0

START_TIME_OFFSET_SD = 1
START_TIME_OFFSET_WF = 2

VISUALIZE = False

# Define specific pairs as "workflow:topology"
WF_TOPO_PAIRS = [
    "linear:multi_tiered",
    "map_reduce:star_of_stars",
    "map_reduce:mesh",
    "wavefront:mesh"
]

PREFIX_LIST = ["nesco"]




# ==========================================
# HELPER FUNCTIONS
# ==========================================
def run_cmd(cmd):
    """Executes a command safely and throws an exception if it fails."""
    cmd_str = [str(arg) for arg in cmd]
    try:
        subprocess.run(cmd_str, check=True, stdout=subprocess.DEVNULL)
    except subprocess.CalledProcessError as e:
        print(f"\n[CRASH] Command failed: {' '.join(cmd_str)}", file=sys.stderr)
        raise

def get_cat_id(pair):
    """Mimics the bash: echo "$wf_topo_pair" | cksum | cut -c1-3"""
    # Using md5 for a consistent, deterministic 3-digit hash
    return str(int(hashlib.md5(pair.encode()).hexdigest(), 16))[:3]




# ==========================================
# GENERATOR COMMAND WRAPPERS
# ==========================================
def generate_wf_messy(servs, prods, cons, layers, skips, ser, shuffle=False):
    output_name = f"{TIMESTAMP}-{ser}--wf_messy-{servs:03d}srv-{prods:03d}prod-{cons:03d}con-{skips:03d}skip-agg-{layers:03d}layer.json"
    output_path = os.path.join(WORKDIR, output_name)
    
    cmd = [
        "./genworkflow.py", "layered",
        "--num-services", servs,
        "--num-layers", layers,
        "--aggregate",
        "--num-producers", prods,
        "--num-consumers", cons,
        "--num-skips", skips,
        "--service-pool-multiplier", SERVICE_POOL_MULTIPLIER,
        "--output", output_path
    ]
    if shuffle:
        cmd.append("--shuffle-services")
    run_cmd(cmd)
    return output_name

def generate_tp(tp_type, nodes, snsrs, usrs, cs, delay_avg, delay_var, wf_cat, ser, extra_args):
    output_name = f"{TIMESTAMP}-{ser}--tp_{tp_type}4{wf_cat}-{nodes:03d}rtr-{snsrs:03d}snsr-{usrs:03d}usr-{cs:06d}cs.json"
    output_path = os.path.join(WORKDIR, output_name)
    
    cmd = [
        "./gentopo.py", "-o", output_path, tp_type,
        "-n", nodes, "-s", snsrs, "-u", usrs,
        "--cs-size", cs, "--delay-avg", delay_avg, "--delay-var", delay_var
    ] + extra_args
    run_cmd(cmd)
    return output_name

def generate_hs(wf_filenames, tp_filename, snsrs, usrs, makespanMinNS, makespanMaxNS, ser, hostRatio, prev_hs):
    tp_path = os.path.join(WORKDIR, tp_filename)
    
    with open(tp_path, 'r') as f:
        tp_data = json.load(f)
        count = len(tp_data.get('router', []))
        
    minHosts = max(1, int(count * hostRatio))
    maxHosts = max(1, int(count * hostRatio))

    tp_clean = tp_filename.split("--")[-1].replace(".json", "")
    output_name = f"{TIMESTAMP}-{ser}-hR_{hostRatio:03.1f}--hs-merged_wf-{tp_clean}.json"
    output_path = os.path.join(WORKDIR, output_name)

    wf_full_paths = [os.path.join(WORKDIR, w) for w in wf_filenames]

    cmd = [
        "./genhosting.py", "--output", output_path, "uniform",
        "--workflows"
    ] + wf_full_paths + [
        "--topology", tp_path,
        "-s", snsrs, "-u", usrs,
        "--makespan-min", makespanMinNS,
        "--makespan-max", makespanMaxNS,
        "--min-hosts", minHosts,
        "--max-hosts", maxHosts
    ]
    
    if prev_hs:
        cmd.extend(["--base-hosting", os.path.join(WORKDIR, prev_hs)])

    run_cmd(cmd)
    return output_name

def build_scenario(out_name, wf_filenames, tp_filename, hs_filename, prefix, strategy, cs_size, sim_end, freshness_ms, sd, ru, icnfcM, ndnfcpTMS):
    tp_path = os.path.join(WORKDIR, tp_filename)
    tp_txt_path = tp_path.replace(".json", ".txt")
    hs_path = os.path.join(WORKDIR, hs_filename)
    out_path = os.path.join(WORKDIR, out_name)
    wf_full_paths = [os.path.join(WORKDIR, w) for w in wf_filenames]


    cmd = [
        "./build_scenario.py", "-f",
        "--topo-json", tp_path,
        "--topo-txt", tp_txt_path,
        "--hosting", hs_path,
        "--output", out_path,
        "--prefix", prefix,
        "--strategy", strategy,
        "--cs-size", cs_size,
        "--serviceDiscovery", sd,
        "--resourceUtilization", ru,
        "--resourceAllocation", 0,
        "--allocationReuse", 0,
        "--scheduleCompaction", 0,
        "--sdTimeoutComputationMultiplier", 2,
        "--startTimeOffsetSD", START_TIME_OFFSET_SD,
        "--startTimeOffsetWF", START_TIME_OFFSET_WF,
        "--simulationEndTime", sim_end,
        "--poissonConsumerFrequency", POISSON_FREQ,
        "--poissonConsumerNumInterests", POISSON_NUM_INTERESTS,
        "--producerFreshnessUniformDist", PRODUCER_FRESHNESS_UNIFORM_DIST,
        "--producerFreshnessMSmin", PRODUCER_FRESHNESS_MS_MIN,
        "--producerFreshnessMSmax", PRODUCER_FRESHNESS_MS_MAX,
        "--producerFreshnessMS", freshness_ms,
        "--icnfcM", icnfcM,
        "--ndnfcpTMS", ndnfcpTMS,
        "--workflow"
    ] + wf_full_paths

    run_cmd(cmd)
    shutil.copy(out_path, OUTDIR)
    return out_path

# ==========================================
# THREAD EXECUTION TASK (Replaces GNU Parallel Task)
# ==========================================
def run_category_task(run_id, pair, generated_workflows):
    workflowCategory, topoCategory = pair.split(":")
    cat_id = get_cat_id(pair)
    padded_catCode = f"{run_id:03d}-{cat_id}"

    # Sweep the service count alongside the node count. Each pass picks only the workflows generated
    # for that size AND for this run, so every scenario has ONE service count (the Srv_ field in its
    # filename is accurate) and each run gets its own DAGs.
    for num_services in NUM_SERVICES_LIST:
        wf_filenames = generated_workflows[(run_id, workflowCategory, num_services)]

        for num_nodes in NUM_NODES_LIST:
            for edgeratio in EDGERATIO_LIST:
                # Generate Topologies
                sensors = 1
                users = 1
                cs_size = 0
                delay_avg_str = f"{LINK_DELAY_AVG_MS}ms"
                delay_var = LINK_DELAY_VARIATION_PCT
            
                if topoCategory == "multi_tiered":
                    tiers = max(2, num_nodes // 6)
                    tp = generate_tp("multi_tiered", num_nodes, sensors, users, cs_size, delay_avg_str, delay_var, workflowCategory, padded_catCode, ["--tiers", tiers])
                elif topoCategory == "mesh":
                    tp = generate_tp("mesh", num_nodes, sensors, users, cs_size, delay_avg_str, delay_var, workflowCategory, padded_catCode, ["-p", 0.1])
                elif topoCategory == "star_of_stars":
                    branches = max(1, num_nodes // 6)
                    tp = generate_tp("star_of_stars", num_nodes, sensors, users, cs_size, delay_avg_str, delay_var, workflowCategory, padded_catCode, ["-b", branches])
                elif topoCategory == "spanning_tree":
                    edges = int((num_nodes - 1) * (edgeratio * (num_nodes - 2) + 2) / 2)
                    tp = generate_tp("spanning_tree", num_nodes, sensors, users, cs_size, delay_avg_str, delay_var, workflowCategory, padded_catCode, ["-e", edges])

                # Sweep CCR and Generate Hostings & Scenarios
                for ccr in CCR_LIST:
                    # Reset previous hosting structure across different CCR runs
                    prev_hs = None
                    #ccr_str = f"CCR_{ccr}".replace('.', 'p')  # e.g., CCR_0p5 avoids dots in filenames
                    formatted_float = f"{ccr:05.2f}"
                    ccr_str = f"CCR_{formatted_float.replace('.', 'p')}"

                    link_delay_ns = LINK_DELAY_AVG_MS * 1_000_000
                    avg_makespan_ns = link_delay_ns / ccr
                    makespanMinNS = int(avg_makespan_ns * (1.0 - MAKESPAN_VARIATION_PCT))
                    makespanMaxNS = int(avg_makespan_ns * (1.0 + MAKESPAN_VARIATION_PCT))

                    freshness_ms = 1/POISSON_FREQ * 1_000 # assumes interest gets new freshness when created, thus aggregator data will be very fresh no matter what pDAG looks like.
                    #freshness_ms = max(NUM_SERVICES_LIST) * avg_makespan_ns/1_000_000

                    # Sweep host ratio incrementally 
                    for hostRatio in HOSTRATIO_LIST:
                        hs_users = 1

                        hs_code = f"{padded_catCode}-{ccr_str}-Srv_{num_services}"
                        hs = generate_hs(wf_filenames, tp, sensors, hs_users, makespanMinNS, makespanMaxNS, hs_code, hostRatio, prev_hs)
                        prev_hs = hs
                        hr_str = f"{hostRatio:03.1f}"

                        sim_end_time = POISSON_NUM_CONSUMERS * POISSON_NUM_INTERESTS * num_services * makespanMaxNS/1_000_000_000
                        sim_end_time = math.ceil(sim_end_time * 10) + START_TIME_OFFSET_WF # extra buffer room to finish simulation

                        for prefix in PREFIX_LIST:
                            base_name = f"{padded_catCode}-hR_{hr_str}-{ccr_str}-Srv_{num_services:02d}-Nodes_{num_nodes:02d}--sn-{topoCategory}-{workflowCategory}-{prefix}"

                            # Scenario 1: noSD2, multicast, cs=0
                            build_scenario(f"{base_name}--1-noSD2-multicast.json", wf_filenames, tp, hs, prefix, "multicast", 0, sim_end_time, freshness_ms, sd=0, ru=0, icnfcM=0, ndnfcpTMS=0)
                    
                            # Scenario 2: noSD2, bestRoute, cs=0
                            build_scenario(f"{base_name}--2-noSD2-bestRoute.json", wf_filenames, tp, hs, prefix, "best-route", 0, sim_end_time, freshness_ms, sd=0, ru=0, icnfcM=0, ndnfcpTMS=0)
                    
                            # Scenario 3: SD2, bestRoute, noUtil, cs=0
                            build_scenario(f"{base_name}--3-SD2-noUtilization.json", wf_filenames, tp, hs, prefix, "best-route", 0, sim_end_time, freshness_ms, sd=2, ru=0, icnfcM=0, ndnfcpTMS=0)
                    
                            # Scenario 4: SD2, bestRoute, Util, noCaching, cs=0
                            build_scenario(f"{base_name}--4-SD2-utilization-noCaching.json", wf_filenames, tp, hs, prefix, "best-route", 0, sim_end_time, freshness_ms, sd=2, ru=1, icnfcM=0, ndnfcpTMS=0)
                    
                            # Scenario 5: SD2, bestRoute, Util, Caching, cs=1000
                            out_path = build_scenario(f"{base_name}--5-SD2-utilization-caching.json", wf_filenames, tp, hs, prefix, "best-route", 1000, sim_end_time, freshness_ms, sd=2, ru=1, icnfcM=0, ndnfcpTMS=0)

                            if VISUALIZE and num_nodes < 9 and len(wf_filenames) < 21:
                                # Best effort visualization
                                subprocess.run(["./genvisuals_top_down_hosting_colors.py", out_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                                subprocess.run(["./genvisuals_top_down_hosting_colors_hierarchical-topo.py", out_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    return f"Completed Run {run_id} | Pair: {pair}"

# ==========================================
# MAIN PIPELINE
# ==========================================
if __name__ == "__main__":
    os.makedirs(WORKDIR, exist_ok=True)
    os.makedirs(OUTDIR, exist_ok=True)

    # Workflows are generated per run, so that each of the NUM_RUNS runs is an independent sample of
    # workflow structure as well as of topology and hosting. Previously this was generated once and the
    # same DAGs were handed to every run, which meant any quirk of that single DAG was baked into all
    # NUM_RUNS samples and the run-to-run spread understated the true variance.
    # Keyed by (run_id, wf_cat, num_services). num_services is in the key because run_category_task
    # now sweeps it: keying by category alone made every service count land in the same list, so one
    # scenario would get one consumer per service count all at once, and there would be no single
    # "number of services" to name the scenario after.
    print(f"Pre-generating {POISSON_NUM_CONSUMERS} unique workflows per category, per run, for {NUM_RUNS} runs...")
    generated_wfs = {}

    # Extract unique workflow categories from pairs to avoid duplicate generation.
    # Categories appearing in more than one pair (e.g. map_reduce in both map_reduce:mesh and
    # map_reduce:star_of_stars) still share one DAG within a run, so those pairs stay a controlled
    # topology-only comparison.
    unique_wf_cats = set([pair.split(":")[0] for pair in WF_TOPO_PAIRS])

    for run_id in range(1, NUM_RUNS + 1):
        for num_services in NUM_SERVICES_LIST:
            for wf_cat in unique_wf_cats:
                generated_wfs.setdefault((run_id, wf_cat, num_services), [])
                for wf_num in range(1, POISSON_NUM_CONSUMERS + 1):
                    wf_code = ""
                    prods = 1
                    cons = 1

                    if wf_cat == "linear":
                        wf_code = f"ln-{run_id:03d}-{wf_num:03d}"
                        layers = num_services
                        skips = 0
                    elif wf_cat == "map_reduce":
                        wf_code = f"mr-{run_id:03d}-{wf_num:03d}"
                        layers = 3
                        skips = 0
                    elif wf_cat == "wavefront":
                        wf_code = f"wf-{run_id:03d}-{wf_num:03d}"
                        layers = max(3, int(math.sqrt(num_services)))
                        skips = max(1, int(layers / 3))
                    elif wf_cat == "multi_sink":
                        wf_code = f"ms-{run_id:03d}-{wf_num:03d}"
                        layers = max(1, int(num_services / 3))
                        skips = 2

                    layers = random.randint(layers-1, layers+1)
                    layers = max(3, layers)
                    skips = random.randint(skips-1, skips+1)
                    skips = max(0, skips)

                    wf = generate_wf_messy(num_services, prods, cons, layers, skips, wf_code, shuffle=True)
                    generated_wfs[(run_id, wf_cat, num_services)].append(wf)

    # Cross Product: 20 runs x 4 categories
    tasks = list(itertools.product(range(1, NUM_RUNS + 1), WF_TOPO_PAIRS))
    total_tasks = len(tasks)
    
    print(f"Distributing {total_tasks} simulation tasks across all CPU cores...")
    
    # ProcessPoolExecutor perfectly replicates GNU parallel, utilizing all available CPU cores
    with ProcessPoolExecutor() as executor:
        futures = []
        for run_id, pair in tasks:
            futures.append(executor.submit(run_category_task, run_id, pair, generated_wfs))

        completed = 0
        for future in as_completed(futures):
            completed += 1
            try:
                result = future.result()
                print(f"[{completed}/{total_tasks}] {result}")
            except Exception as e:
                print(f"[{completed}/{total_tasks}] Task failed with error: {e}")

    print("\nAll scenarios generated successfully.")