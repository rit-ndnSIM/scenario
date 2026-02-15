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

NDNSIM_HOME="$HOME/ndnSIM"
SCENARIO_DIR="$HOME/ndnSIM/scenario"
WORKFLOW_DIR="$HOME/ndnSIM/scenario/workflows"
TOPOLOGY_DIR="$HOME/ndnSIM/scenario/topologies"
CPM_DIR="$HOME/ndnSIM/CPM"
USAGE_ALLOCATION_GRAPHS_DIR="$HOME/ndnSIM/scenario/usage_allocation_graphs"

#mkdir -p "$SCENARIO_JSON_DIR"

TYPE="cascon_main"
#TYPE="cascon_cpm"
#TYPE="fwdOptSD"
#TYPE="cascon_random_test"

SCENARIO_JSON_DIR="$SCENARIO_DIR/scenario_json/$TYPE"
csv_out="$SCENARIO_DIR/perf-results-simulation-${TYPE}_generic.csv"

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



# Run a scenario for every file found in the directory
for filepath in "$SCENARIO_JSON_DIR"/*
do
    if [ -f "$filepath" ]; then
    	filename=$(basename "$filepath ") 	# remove path
    	scenario="${filename%.*}"			# remove file extension
    	echo "Scenario: $scenario"

    	scenario_log="$SCENARIO_DIR/scenario.log"

    	scenario_json="$SCENARIO_JSON_DIR/$scenario.json"

    	now="$(date -Iseconds)"

    	ns_3_hash="$(git -C "$NDNSIM_HOME/ns-3" rev-parse HEAD)"
    	pybindgen_hash="$(git -C "$NDNSIM_HOME/pybindgen" rev-parse HEAD)"
    	scenario_hash="$(git -C "$NDNSIM_HOME/scenario" rev-parse HEAD)"
    	ndnsim_hash="$(git -C "$NDNSIM_HOME/ns-3/src/ndnSIM" rev-parse HEAD)"

    	echo "Running..."

        # Run the simulation, piping ALL output (stderr and stdout) to tee, 
        # which duplicates it to the screen and the log file ($scenario_log).
        "$SCENARIO_DIR/waf" --run="ndn-cabeee-generic --scenario=$scenario_json --verbose=false" |& tee "$scenario_log"
        #"$SCENARIO_DIR/waf" --run="ndn-cabeee-generic --scenario=$scenario_json --verbose=true" |& tee "$scenario_log"

    	echo "Parsing logs..."
    
        # 1. SD Estimated Service Latency
        #estimatedWFLatency=$(grep "Service Latency estimated by SD:" "$scenario_log" | sed -n 's/^\s*Service Latency estimated by SD: \([0-9\.]*\) microseconds.$/\1/p')
        estimatedWFLatency=$(grep "Service Latency estimated by SD:" "$scenario_log" | tail -n 1 | sed -n 's/^\s*Service Latency estimated by SD: \([0-9\.]*\) microseconds.$/\1/p')
        estimatedWFLatency="${estimatedWFLatency:-N.A.}"
    
        # 2. Service Discovery Latency
        SDlatency=$(grep "Service Discovery Latency:" "$scenario_log" | tail -n 1 | sed -n 's/^\s*Service Discovery Latency: \([0-9\.]*\) microseconds.$/\1/p')
        SDlatency="${SDlatency:-N.A.}"
    
        # 3. Service Latency
        WFlatency=$(grep "Service Latency:" "$scenario_log" | sed -n 's/^\s*Service Latency: \([0-9\.]*\) microseconds.$/\1/p')
        WFlatency="${WFlatency:-Error_WFLatency}"
    
        # 4. Final Result
        result=$(grep "The final answer is:" "$scenario_log" | tail -n 1 | sed -n 's/^\s*The final answer is: \([0-9]*\)$/\1/p')
        result="${result:-N.A.}"

        packets=$( \
            python process_nfd_logs_SD.py $USAGE_ALLOCATION_GRAPHS_DIR/${scenario}.png | sed -n \
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
        SDinterest_gen="${packetArray[0]:-N.A.}"
        SDdata_gen="${packetArray[1]:-N.A.}"
        SDinterest_trans="${packetArray[2]:-N.A.}"
        SDdata_trans="${packetArray[3]:-N.A.}"
        WFinterest_gen="${packetArray[4]:-N.A.}"
        WFdata_gen="${packetArray[5]:-N.A.}"
        WFinterest_trans="${packetArray[6]:-N.A.}"
        WFdata_trans="${packetArray[7]:-N.A.}"
        totalNodeUsageTime="${packetArray[8]:-N.A.}"
        avgNodeUsage="${packetArray[9]:-N.A.}"
        coeffVariation="${packetArray[10]:-N.A.}"



        set +e
        # cpm now determines type, wf, hosting, and topo information from the full json, and somehow pass it into the cpm program cleanly
        cpm_output=$(${CPM_DIR}/cpm --scenarioJSON "${scenario_json}" 2>&1)
        cpm_status=$?
        set -e

        if  [ $cpm_status -ne 0 ]; then
            echo "Warning: cpm failed with exit code $cpm_status"
            cpm=-1
            cpm_t=-1
        else
            cpm=$(echo "$cpm_output" | sed -n 's/^metric: \([0-9]*\)/\1/p' | tr -d '\n')
            cpm_t=$(echo "$cpm_output" | sed -n 's/^time: \([0-9]*\) ns/\1/p' | tr -d '\n')
        fi

        row="$scenario, $SDinterest_gen, $SDdata_gen, $SDinterest_trans, $SDdata_trans, $WFinterest_gen, $WFdata_gen, $WFinterest_trans, $WFdata_trans, $cpm, $cpm_t, $SDlatency, $estimatedWFLatency, $WFlatency, $totalNodeUsageTime, $avgNodeUsage, $coeffVariation, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"

    	echo "Dumping to csv..."
    	line_num="$(grep -n -F "$scenario," "$csv_out" | cut -d: -f1 | head -1)"
    	if [ -n "$line_num" ]; then
    		sed --in-place -e "${line_num}c\\$row" "$csv_out"
    	else
    		echo "$row" >> "$csv_out"
    	fi

    	echo ""
    fi
done

echo "All scenarios ran"
