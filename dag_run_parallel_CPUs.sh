#!/bin/bash
# vim: sw=8 noet

#----- THIS ONLY NEEDS TO BE RUN ONCE --------------------
#cd ~/ndnSIM/scenario
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
#export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
#./waf clean
#CXXFLAGS="-std=c++17" ./waf configure --debug
#---------------------------------------------------------

set -e

# log levels: error, warn, info, debug
# inclusive log levels don't seem to work (using level_debug won't show more severe errors, only debug)

LOGS=CustomAppConsumer="error|warn"
LOGS=$LOGS:CustomAppConsumerServiceDiscovery="error|warn|info"
LOGS=$LOGS:CustomAppConsumer2="error|warn"
LOGS=$LOGS:CustomAppProducer="error|warn"
LOGS=$LOGS:DagForwarderApp="error|warn"
LOGS=$LOGS:DagServiceDiscoveryApp="error|warn|info|time|node|func"
#LOGS=$LOGS:DagServiceDiscoveryApp="error|warn|info|debug|time|node|func"
LOGS=$LOGS:ndn.App="error|warn"
LOGS=$LOGS:DagOrchestratorA_App="error|warn"
LOGS=$LOGS:DagServiceA_App="error|warn"
LOGS=$LOGS:DagOrchestratorB_App="error|warn"
LOGS=$LOGS:DagServiceB_App="error|warn"
LOGS=$LOGS:ndn-cxx.nfd.Forwarder="error|warn|info|time|node|func"
#LOGS=$LOGS:ndn-cxx.nfd.Forwarder="error|warn|info|debug|time|node|func"

export NS_LOG="$LOGS"

export NDNSIM_HOME="$HOME/ndnSIM"
export SCENARIO_DIR="$HOME/ndnSIM/scenario"
export SCENARIO_LOGS_DIR="$SCENARIO_DIR/scenario_logs"
export WORKFLOW_DIR="$HOME/ndnSIM/scenario/workflows"
export TOPOLOGY_DIR="$HOME/ndnSIM/scenario/topologies"
export CPM_DIR="$HOME/ndnSIM/CPM"
export USAGE_ALLOCATION_GRAPHS_DIR="$HOME/ndnSIM/scenario/usage_allocation_graphs"
#export GEN_ALLOCATION_GRAPHS="true"
export GEN_ALLOCATION_GRAPHS="false"

#TYPE="cascon_main"
#TYPE="cascon_cpm"
TYPE="cascon_cpm_random"
#TYPE="cascon_intervals"
#TYPE="fwdOptSD"
#TYPE="cascon_random_test"

export SCENARIO_JSON_DIR="$SCENARIO_DIR/scenario_json/$TYPE"
export csv_out="$SCENARIO_DIR/perf-results-simulation-generic_${TYPE}.csv"

# --- 1. Setup CSV Header ---
header="Example, SD Interest Packets Generated, SD Data Packets Generated, SD Interest Packets Transmitted, SD Data Packets Transmitted, WF Interest Packets Generated, WF Data Packets Generated, WF Interest Packets Transmitted, WF Data Packets Transmitted, Critical-Path-Metric, CPM-t_exec(ns), SD Latency (us), SD Estimated WF Service Latency (us), WF Service Latency (us), Total Node Usage Time (us), Average Node Utilization (%), Coefficient of Variation (load distribution), Final Result, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"

if [ ! -f "$csv_out" ]; then
    echo "Creating csv..."
    echo "$header" > "$csv_out"
elif ! grep -q -F "$header" "$csv_out"; then
    echo "Overwriting csv..."
    mv "$csv_out" "$csv_out.bak"
    echo "$header" > "$csv_out"
else
    echo "Updating csv..."
    cp "$csv_out" "$csv_out.bak"
fi

# --- 2. Cache Git Hashes (Run Once) ---
export ns_3_hash="$(git -C "$NDNSIM_HOME/ns-3" rev-parse HEAD)"
export pybindgen_hash="$(git -C "$NDNSIM_HOME/pybindgen" rev-parse HEAD)"
export scenario_hash="$(git -C "$NDNSIM_HOME/scenario" rev-parse HEAD)"
export ndnsim_hash="$(git -C "$NDNSIM_HOME/ns-3/src/ndnSIM" rev-parse HEAD)"

# --- 3. Define CSV Update Helper ---
# This function handles the actual file write, designed to be locking
update_csv() {
    local scenario="$1"
    local row="$2"
    local csv_file="$3"
    local line_num
    
    line_num="$(grep -n -F "$scenario," "$csv_file" | cut -d: -f1 | head -1)"
    if [ -n "$line_num" ]; then
        sed --in-place -e "${line_num}c\\$row" "$csv_file"
    else
        echo "$row" >> "$csv_file"
    fi
}
export -f update_csv

# --- 4. Define the Simulation Worker ---
run_simulation() {
    local filepath="$1"
    local filename=$(basename "$filepath")
    local scenario="${filename%.*}"
    local scenario_json="$filepath"
    local scenario_log="$SCENARIO_LOGS_DIR/scenario_${scenario}.log"
    local now="$(date -Iseconds)"

    echo "Starting Scenario: $scenario"

    # Run simulation, logging output to a unique file
    "$SCENARIO_DIR/waf" --run="ndn-cabeee-generic --scenario=$scenario_json --verbose=false" > "$scenario_log" 2>&1

    # Parse logs
    local estimatedWFLatency=$(grep "Service Latency estimated by SD:" "$scenario_log" | tail -n 1 | sed -n 's/^\s*Service Latency estimated by SD: \([0-9\.]*\) microseconds.$/\1/p')
    estimatedWFLatency="${estimatedWFLatency:-N.A.}"

    local SDlatency=$(grep "Service Discovery Latency:" "$scenario_log" | tail -n 1 | sed -n 's/^\s*Service Discovery Latency: \([0-9\.]*\) microseconds.$/\1/p')
    SDlatency="${SDlatency:-N.A.}"

    local WFlatency=$(grep "Service Latency:" "$scenario_log" | sed -n 's/^\s*Service Latency: \([0-9\.]*\) microseconds.$/\1/p')
    WFlatency="${WFlatency:-Error_WFLatency}"

    local result=$(grep "The final answer is:" "$scenario_log" | tail -n 1 | sed -n 's/^\s*The final answer is: \([0-9]*\)$/\1/p')
    result="${result:-N.A.}"

    local packets=$( \
        python "$SCENARIO_DIR/process_nfd_logs_SD.py" --graph "$GEN_ALLOCATION_GRAPHS" --output "$USAGE_ALLOCATION_GRAPHS_DIR/${scenario}.png" | sed -n \
        -e 's/^SD Interest Packets Generated: \([0-9]*\) interests$/\1,/p' \
        -e 's/^SD Data Packets Generated: \([0-9]*\) data$/\1,/p' \
        -e 's/^SD Interest Packets Transmitted: \([0-9]*\) interests$/\1,/p' \
        -e 's/^SD Data Packets Transmitted: \([0-9]*\) data/\1,/p' \
        -e 's/^WF Interest Packets Generated: \([0-9]*\) interests$/\1,/p' \
        -e 's/^WF Data Packets Generated: \([0-9]*\) data$/\1,/p' \
        -e 's/^WF Interest Packets Transmitted: \([0-9]*\) interests$/\1,/p' \
        -e 's/^WF Data Packets Transmitted: \([0-9]*\) data/\1,/p' \
        -e 's/^Overall Total Busy Time (All Nodes): \([0-9.]*\) microseconds/\1,/p' \
        -e 's/^Average Utilization (All Nodes): \([0-9.]*\)%$/\1,/p' \
        -e 's/^Coefficient of Variation (load distribution): \([0-9.]*\)/\1,/p' \
        | tr -d '\n' \
    )

    IFS=',' read -r -a packetArray <<< "$packets"
    local SDinterest_gen="${packetArray[0]:-N.A.}"
    local SDdata_gen="${packetArray[1]:-N.A.}"
    local SDinterest_trans="${packetArray[2]:-N.A.}"
    local SDdata_trans="${packetArray[3]:-N.A.}"
    local WFinterest_gen="${packetArray[4]:-N.A.}"
    local WFdata_gen="${packetArray[5]:-N.A.}"
    local WFinterest_trans="${packetArray[6]:-N.A.}"
    local WFdata_trans="${packetArray[7]:-N.A.}"
    local totalNodeUsageTime="${packetArray[8]:-N.A.}"
    local avgNodeUsage="${packetArray[9]:-N.A.}"
    local coeffVariation="${packetArray[10]:-N.A.}"

    set +e
    local cpm_output=$(${CPM_DIR}/cpm --scenarioJSON "${scenario_json}" 2>&1)
    local cpm_status=$?
    set -e

    local cpm cpm_t
    if  [ $cpm_status -ne 0 ]; then
        echo "Warning: cpm failed with exit code $cpm_status on scenario $scenario"
        cpm=-1
        cpm_t=-1
    else
        cpm=$(echo "$cpm_output" | sed -n 's/^metric: \([0-9]*\)/\1/p' | tr -d '\n')
        cpm_t=$(echo "$cpm_output" | sed -n 's/^time: \([0-9]*\) ns/\1/p' | tr -d '\n')
    fi

    local row="$scenario, $SDinterest_gen, $SDdata_gen, $SDinterest_trans, $SDdata_trans, $WFinterest_gen, $WFdata_gen, $WFinterest_trans, $WFdata_trans, $cpm, $cpm_t, $SDlatency, $estimatedWFLatency, $WFlatency, $totalNodeUsageTime, $avgNodeUsage, $coeffVariation, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"

    # Lock the CSV file writing process to prevent data corruption
    #sem --id csv_lock update_csv "$scenario" "$row" "$csv_out"
    (
        flock -x 200
        update_csv "$scenario" "$row" "$csv_out"
    ) 200> "${csv_out}.lock"

    echo "Finished Scenario: $scenario"
}
export -f run_simulation

# --- 5. Dispatch Jobs ---
echo "Dispatching jobs to all available CPU cores..."

# Find all JSON files and pipe them into parallel
find "$SCENARIO_JSON_DIR" -maxdepth 1 -name "*.json" | parallel --jobs 0 run_simulation {}

# Wait for all semaphores to clear just to be safe
#sem --wait --id csv_lock

echo "All scenarios completed."