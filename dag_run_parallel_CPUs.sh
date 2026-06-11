#!/bin/bash
# vim: sw=8 noet

#----- THIS ONLY NEEDS TO BE RUN ONCE --------------------
#cd ~/ndnSIM/scenario
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
#export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
#./waf clean
#CXXFLAGS="-std=c++17" ./waf configure --debug
#---------------------------------------------------------

START_TIME=$(date +%s)

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


#---------------------------------  SETTINGS -----------------------------------
#TYPE="cascon_main"
#TYPE="cascon_main_cat"
#TYPE="cascon_cpm"
#TYPE="cascon_cpm_random"
#TYPE="cascon_cpm_random_nesco"
#TYPE="cascon_lat-bw-cpm"
#TYPE="cascon_intervals"
#TYPE="fwdOptSD"
#TYPE="fwdOptSDSweep"
#TYPE="fwdOptSDSweep2"
#TYPE="fwdOptSDSweep3"
#TYPE="fwdOptSDSweep4"
#TYPE="fwdOptSDSweep5"
#TYPE="fwdOptSDSweep20"
#TYPE="fwdOptSDSweep20_5x8"
#TYPE="cascon_random_test"
#TYPE="linearWFs_SD20"
#TYPE="fwdOptSD2Sweep20_test"
TYPE="fwdOptSD2Sweep20_5x8"
#TYPE="linearSD2Sweep20_5x8"


#export GEN_ALLOCATION_GRAPHS="true"
export GEN_ALLOCATION_GRAPHS="false"
#export FORCE_RERUN_ALL="true"   # Set to "true" to run everything. Set to "false" to only run missing/failed scenarios.
export FORCE_RERUN_ALL="false"   # Set to "true" to run everything. Set to "false" to only run missing/failed scenarios.
export FORCE_TRACE=0    # Set to override trace settings in JSON file. This value is the trace interval in seconds.
#export FORCE_TRACE=0.1    # Set to override trace settings in JSON file. This value is the trace interval in seconds.
#export FORCE_MAKESPAN=20000000    # Set to override service makespanNS settings in JSON file.
export FORCE_MAKESPAN=0    # Set to override service makespanNS settings in JSON file.


#------------------------------ END OF SETTINGS -----------------------------------



export NDNSIM_HOME="$HOME/ndnSIM"
export SCENARIO_DIR="$HOME/ndnSIM/scenario"
export SCENARIO_LOGS_DIR="$SCENARIO_DIR/scenario_logs"
export SCENARIO_TRACE_DIR="$SCENARIO_DIR/trace_results/$TYPE"
export WORKFLOW_DIR="$HOME/ndnSIM/scenario/workflows"
export TOPOLOGY_DIR="$HOME/ndnSIM/scenario/topologies"
export CPM_DIR="$HOME/CPM"
export USAGE_ALLOCATION_GRAPHS_DIR="$HOME/ndnSIM/scenario/usage_allocation_graphs"



export SCENARIO_JSON_DIR="$SCENARIO_DIR/scenario_json/$TYPE"
export csv_out="$SCENARIO_DIR/perf-results-simulation-generic_${TYPE}.csv"

mkdir -p "$SCENARIO_LOGS_DIR"
mkdir -p "$SCENARIO_TRACE_DIR"

# --- 1. Setup CSV Header ---
#header="Example, SD Interest Packets Generated, SD Data Packets Generated, SD Interest Packets Transmitted, SD Data Packets Transmitted, WF Interest Packets Generated, WF Data Packets Generated, WF Interest Packets Transmitted, WF Data Packets Transmitted, Critical-Path-Metric, CPM-t_exec(ns), SD Latency (us), SD Estimated WF Service Latency (us), WF Service Latency (us), Total Node Usage Time (us), Average Node Utilization (%), Coefficient of Variation (load distribution), Final Result, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"
#header="Example, SD Interest Packets Generated, SD Data Packets Generated, SD Interest Packets Transmitted, SD Data Packets Transmitted, WF Interest Packets Generated, WF Data Packets Generated, WF Interest Packets Transmitted, WF Data Packets Transmitted, Critical-Path-Metric, CPM-t_exec(ns), SD Latency (us), SD Estimated WF Service Latency (us), WF Service Latency (us), Total Node Usage Time (us), Average Node Utilization (%), Coefficient of Variation (load distribution), Total Cache Hits, Total Cache Misses, Avg Cache Usage, Total KB Transferred, Final Result, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"
header="Example, SD Interest Packets Generated, SD Data Packets Generated, SD Interest Packets Transmitted, SD Data Packets Transmitted, WF Interest Packets Generated, WF Data Packets Generated, WF Interest Packets Transmitted, WF Data Packets Transmitted, Critical-Path-Metric, CPM-t_exec(ns), SD Latency (us), SD Estimated WF Service Latency (us), WF Service Latency (us), Total Node Usage Time (us), Average Node Utilization (%), Coefficient of Variation (load distribution), Total Cache Hits, Total Cache Misses, Avg Cache Usage, Total KB Transferred, Avg SD2 Latency (us), Min Service Latency(us), Low Quartile Service Latency(us), Mid Quartile Service Latency(us), High Quartile Service Latency(us), Max Service Latency(us), Total Service Latency(us), Avg Service Latency(us), Requests Fulfilled, Result Sum, Final Result, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"

# Function to write sorted placeholder rows into the CSV file
populate_blank_scenarios() {
    # Count how many total columns exist by counting commas in the header
    local total_commas=$(echo "$header" | tr -cd ',' | wc -c)
    local trailing_commas=""
    
    # Generate the exact number of trailing commas needed for the blank rows
    for i in $(seq 1 $total_commas); do
        trailing_commas="${trailing_commas},"
    done

    # 1. Find all JSON files
    # 2. Extract just the filename without the path
    # 3. Strip the .json extension
    # 4. Sort them alphabetically
    # 5. Append the commas and write to the CSV
    find "$SCENARIO_JSON_DIR" -maxdepth 1 -name "*.json" -exec basename {} .json \; | sort | while read -r scenario; do
        echo "${scenario}${trailing_commas}" >> "$csv_out"
    done
}

if [ ! -f "$csv_out" ]; then
    echo "Creating csv..."
    echo "$header" > "$csv_out"
    populate_blank_scenarios
elif ! grep -q -F "$header" "$csv_out"; then
    echo "Overwriting csv..."
    mv "$csv_out" "$csv_out.bak"
    echo "$header" > "$csv_out"
    populate_blank_scenarios
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
    local force_run="$2"
    local csv_file="$3"
    local force_trace="$4"
    local force_makespan="$5"
    local filename=$(basename "$filepath")
    local scenario="${filename%.*}"
    local scenario_json="$filepath"
    local scenario_log="$SCENARIO_LOGS_DIR/scenario_${scenario}.log"
    local scenario_csTrace="$SCENARIO_TRACE_DIR/cs-trace_${scenario}.txt"
    local scenario_csUsage="$SCENARIO_TRACE_DIR/cs-usage_${scenario}.txt"
    local scenario_rateTrace="$SCENARIO_TRACE_DIR/rate-trace_${scenario}.txt"
    local now="$(date -Iseconds)"

    # --- Smart Skip Logic ---
    if [ "$force_run" = "false" ]; then
        # Locate the exact line for this scenario
        local csv_line=$(grep -F "$scenario," "$csv_file" | head -1)
        if [ -n "$csv_line" ]; then
            # Parse the 14th and 29th columns using comma separation, trimming spaces
            local col14=$(echo "$csv_line" | cut -d',' -f14 | tr -d '[:space:]')
            local col29=$(echo "$csv_line" | cut -d',' -f29 | tr -d '[:space:]')

            local col14_valid=0
            local col29_valid=0
            
            # Check if either is a valid positive float or integer (> 0)
            if [[ "$col14" =~ ^[0-9]+(\.[0-9]+)?$ ]] && (( $(echo "$col14 > 0" | bc -l) )); then
                col14_valid=1
            fi
            if [[ "$col29" =~ ^[0-9]+(\.[0-9]+)?$ ]] && (( $(echo "$col29 > 0" | bc -l) )); then
                col29_valid=1
            fi
            if [ $col14_valid -eq 1 ] || [ $col29_valid -eq 1 ]; then
                echo "[SKIP] Scenario '$scenario' already has a valid result."
                return 0
            fi
        fi
    fi

    echo "[$(date '+%Y-%m-%d %H:%M:%S')] Starting Scenario: $scenario"

    # Run simulation, logging output to a unique file
    #"$SCENARIO_DIR/waf" --run="ndn-cabeee-generic --scenario=$scenario_json --verbose=true" > "$scenario_log" 2>&1
    #"$SCENARIO_DIR/waf" --run="ndn-cabeee-generic --scenario=$scenario_json --verbose=false --overrideTrace=$force_trace --traceDir=$SCENARIO_TRACE_DIR --overrideMakespan=$force_makespan" > "$scenario_log" 2>&1

    # Parse logs
    local estimatedWFLatency=$(grep "Service Latency estimated by SD:" "$scenario_log" | tail -n 1 | sed -n 's/^\s*Service Latency estimated by SD: \([0-9\.]*\) microseconds.$/\1/p')
    estimatedWFLatency="${estimatedWFLatency:-N.A.}"

    #local SDlatency=$(grep "Service Discovery Latency:" "$scenario_log" | tail -n 1 | sed -n 's/^\s*Service Discovery Latency: \([0-9\.]*\) microseconds.$/\1/p')
    #SDlatency="${SDlatency:-N.A.}"
    local SDlatency=$(grep -o "Service Discovery Latency: [0-9]* microseconds" "$scenario_log" | tail -n 1 | cut -d' ' -f4)
    SDlatency="${SDlatency:-N.A.}"


    local WFlatency=$(grep "Service Latency:" "$scenario_log" | sed -n 's/^\s*Service Latency: \([0-9\.]*\) microseconds.$/\1/p')
    WFlatency="${WFlatency:-Error_WFLatency}"

    local result=$(grep "The final answer is:" "$scenario_log" | tail -n 1 | sed -n 's/^\s*The final answer is: \([0-9]*\)$/\1/p')
    result="${result:-N.A.}"

    local packets=$( \
        python3 "$SCENARIO_DIR/process_nfd_logs_SD2.py" --logfile "$scenario_log" --output "$USAGE_ALLOCATION_GRAPHS_DIR/${scenario}.png" --graph "$GEN_ALLOCATION_GRAPHS" | sed -n \
        -e 's/^SD Interest Packets Generated: \([0-9]*\) interests$/\1,/p' \
        -e 's/^SD Data Packets Generated: \([0-9]*\) data$/\1,/p' \
        -e 's/^SD Interest Packets Transmitted: \([0-9]*\) interests$/\1,/p' \
        -e 's/^SD Data Packets Transmitted: \([0-9]*\) data/\1,/p' \
        -e 's/^WF Interest Packets Generated: \([0-9]*\) interests$/\1,/p' \
        -e 's/^WF Data Packets Generated: \([0-9]*\) data$/\1,/p' \
        -e 's/^WF Interest Packets Transmitted: \([0-9]*\) interests$/\1,/p' \
        -e 's/^WF Data Packets Transmitted: \([0-9]*\) data/\1,/p' \
        -e 's/^Global workflow min latency: \([0-9.]*\) microseconds$/\1,/p' \
        -e 's/^Global workflow low latency: \([0-9.]*\) microseconds$/\1,/p' \
        -e 's/^Global workflow mid latency: \([0-9.]*\) microseconds$/\1,/p' \
        -e 's/^Global workflow high latency: \([0-9.]*\) microseconds$/\1,/p' \
        -e 's/^Global workflow max latency: \([0-9.]*\) microseconds$/\1,/p' \
        -e 's/^Global workflow total latency: \([0-9.]*\) microseconds$/\1,/p' \
        -e 's/^Global workflow avg latency: \([0-9.]*\) microseconds$/\1,/p' \
        -e 's/^Global workflow total requests fulfilled: \([0-9.]*\) total requests$/\1,/p' \
        -e 's/^Global workflow final answers sum: \([0-9.]*\) numerical$/\1,/p' \
        -e 's/^Global service discovery average latency: \([0-9.]*\) microseconds$/\1,/p' \
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

    local global_wf_min_latency="${packetArray[8]:-N.A.}"
    local global_wf_low_latency="${packetArray[9]:-N.A.}"
    local global_wf_mid_latency="${packetArray[10]:-N.A.}"
    local global_wf_high_latency="${packetArray[11]:-N.A.}"
    local global_wf_max_latency="${packetArray[12]:-N.A.}"
    local global_wf_total_latency="${packetArray[13]:-N.A.}"
    local global_wf_avg_latency="${packetArray[14]:-N.A.}"
    local global_wf_requests_fulfilled="${packetArray[15]:-N.A.}"
    local global_wf_final_answers_sum="${packetArray[16]:-N.A.}"

    local global_sd_avg_latency="${packetArray[17]:-N.A.}"

    local totalNodeUsageTime="${packetArray[18]:-N.A.}"
    local avgNodeUsage="${packetArray[19]:-N.A.}"
    local coeffVariation="${packetArray[20]:-N.A.}"


    # --- Trace Analysis (with existence checks) ---

    # 1. Total Cache Hits and Misses
    if [ -f "$scenario_csTrace" ]; then
        local csTraceOutput=$(python3 "$SCENARIO_DIR/process_trace_logs.py" "$scenario_csTrace" "cs-trace")
        local totalHits=$(echo "$csTraceOutput" | sed -n 's/^Total Cache Hits:\s*\([0-9]*\)$/\1/p')
        local totalMisses=$(echo "$csTraceOutput" | sed -n 's/^Total Cache Misses:\s*\([0-9]*\)$/\1/p')
    else
        local totalHits=""
        local totalMisses=""
    fi

    # 2. Average Cache Usage
    if [ -f "$scenario_csUsage" ]; then
        local csUsageOutput=$(python3 "$SCENARIO_DIR/process_trace_logs.py" "$scenario_csUsage" "cs-usage")
        local avgCacheUsage=$(echo "$csUsageOutput" | sed -n 's/^Average Total Usage (All Nodes) across.*: \([0-9.]*\)$/\1/p')
    else
        local avgCacheUsage=""
    fi

    # 3. Total Kilobytes Transferred
    if [ -f "$scenario_rateTrace" ]; then
        local rateTraceOutput=$(python3 "$SCENARIO_DIR/process_trace_logs.py" "$scenario_rateTrace" "rate-trace")
        local totalKB=$(echo "$rateTraceOutput" | sed -n 's/^Total Kilobytes: \([0-9.]*\) KB$/\1/p')
    else
        local totalKB=""
    fi

    # Final cleanup: ensure variables are empty strings if parsing failed but file existed
    totalHits="${totalHits:-}"
    totalMisses="${totalMisses:-}"
    avgCacheUsage="${avgCacheUsage:-}"
    totalKB="${totalKB:-}"



    set +e
    local cpm_output=$(${CPM_DIR}/cpm --scenarioJSON "${scenario_json}" 2>&1)
    local cpm_status=$?
    set -e

    local cpm cpm_t
    if  [ $cpm_status -ne 0 ]; then
        echo "Warning: cpm failed with exit code $cpm_status on scenario $scenario"
        cpm=""
        cpm_t=""
    else
        cpm=$(echo "$cpm_output" | sed -n 's/^metric: \([0-9]*\)/\1/p' | tr -d '\n')
        cpm_t=$(echo "$cpm_output" | sed -n 's/^time: \([0-9]*\) ns/\1/p' | tr -d '\n')
    fi

    #local row="$scenario, $SDinterest_gen, $SDdata_gen, $SDinterest_trans, $SDdata_trans, $WFinterest_gen, $WFdata_gen, $WFinterest_trans, $WFdata_trans, $cpm, $cpm_t, $SDlatency, $estimatedWFLatency, $WFlatency, $totalNodeUsageTime, $avgNodeUsage, $coeffVariation, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"
    #local row="$scenario, $SDinterest_gen, $SDdata_gen, $SDinterest_trans, $SDdata_trans, $WFinterest_gen, $WFdata_gen, $WFinterest_trans, $WFdata_trans, $cpm, $cpm_t, $SDlatency, $estimatedWFLatency, $WFlatency, $totalNodeUsageTime, $avgNodeUsage, $coeffVariation, $totalHits, $totalMisses, $avgCacheUsage, $totalKB, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"
    local row="$scenario, $SDinterest_gen, $SDdata_gen, $SDinterest_trans, $SDdata_trans, $WFinterest_gen, $WFdata_gen, $WFinterest_trans, $WFdata_trans, $cpm, $cpm_t, $SDlatency, $estimatedWFLatency, $WFlatency, $totalNodeUsageTime, $avgNodeUsage, $coeffVariation, $totalHits, $totalMisses, $avgCacheUsage, $totalKB, $global_sd_avg_latency, $global_wf_min_latency, $global_wf_low_latency, $global_wf_mid_latency, $global_wf_high_latency, $global_wf_max_latency, $global_wf_total_latency, $global_wf_avg_latency, $global_wf_requests_fulfilled, $global_wf_final_answers_sum, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"


    # Lock the CSV file writing process to prevent data corruption
    #sem --id csv_lock update_csv "$scenario" "$row" "$csv_out"
    (
        flock -x 200
        update_csv "$scenario" "$row" "$csv_out"
    ) 200> "${csv_out}.lock"

    echo "[$(date '+%Y-%m-%d %H:%M:%S')] Finished Scenario: $scenario"
}
export -f run_simulation

# --- 5. Dispatch Jobs ---
#echo "Dispatching jobs to all available CPU cores..."
echo "Dispatching jobs in prioritized hR order (0.0 -> 0.2 -> 0.4 -> 0.6 -> 0.8 -> 1.0)..."


# Find all JSON files and pipe them into parallel using all system resources (may overload system if too many jobs are started at once)
#find "$SCENARIO_JSON_DIR" -maxdepth 1 -name "*.json" | parallel --ungroup --jobs 0 run_simulation {}
# Find all JSON files and pipe them into parallel using all cores
#find "$SCENARIO_JSON_DIR" -maxdepth 1 -name "*.json" | parallel --ungroup --jobs 90% run_simulation {}
# Find all JSON files and pipe them into parallel using 7 cores
#find "$SCENARIO_JSON_DIR" -maxdepth 1 -name "*.json" | parallel --ungroup --jobs 7 run_simulation {}
# Find all JSON files and pipe them into parallel using half of the cores
#find "$SCENARIO_JSON_DIR" -maxdepth 1 -name "*.json" | parallel --ungroup --jobs 50% run_simulation {}


# 1. Find all json files
# 2. Extract 'hR_X.X' from the middle of the filename to use as a sorting prefix
# 3. Sort them alphanumerically (0.0 first, 1.0 last)
# 4. Strip the prefix so GNU Parallel receives the original clean file path
find "$SCENARIO_JSON_DIR" -maxdepth 1 -name "*.json" | awk '
    {
        # Match "hR_" followed by a number, a dot, and another number
        if (match($0, /hR_[0-9]+\.[0-9]+/)) {
            print substr($0, RSTART, RLENGTH) "\t" $0
        } else {
            # Catch-all fallback just in case a file misses the pattern
            print "z_no_hR" "\t" $0
        }
    }
' | sort -V | cut -f2- | parallel --ungroup --jobs 90% run_simulation {} "$FORCE_RERUN_ALL" "$csv_out" "$FORCE_TRACE" "$FORCE_MAKESPAN"

# Wait for all semaphores to clear just to be safe
#sem --wait --id csv_lock

echo "All scenarios completed."
END_TIME=$(date +%s)
TOTAL_RUNTIME=$((END_TIME - START_TIME))
# Format the time into HH:MM:SS
H=$((TOTAL_RUNTIME / 3600))
M=$(((TOTAL_RUNTIME % 3600) / 60))
S=$((TOTAL_RUNTIME % 60))
printf "\nTotal Execution Time: %02d:%02d:%02d (%d seconds)\n" $H $M $S $TOTAL_RUNTIME

echo -e "\a"
sleep 0.5
echo -e "\a"
sleep 0.5
echo -e "\a"
sleep 0.5
echo -e "\a"
sleep 0.5
echo -e "\a"
sleep 0.5
echo -e "\a"
sleep 0.5
echo -e "\a"
sleep 0.5
echo -e "\a"
sleep 0.5
echo -e "\a"
sleep 0.5
echo -e "\a"
