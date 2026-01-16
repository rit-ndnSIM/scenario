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

LOGS=CustomAppConsumer
LOGS=$LOGS:CustomAppConsumer2
LOGS=$LOGS:CustomAppProducer
LOGS=$LOGS:DagForwarderApp
LOGS=$LOGS:ndn.App
LOGS=$LOGS:DagOrchestratorA_App
LOGS=$LOGS:DagServiceA_App
LOGS=$LOGS:DagOrchestratorB_App
LOGS=$LOGS:DagServiceB_App
LOGS=$LOGS:ndn-cxx.nfd.Forwarder

export NS_LOG="$LOGS"

NDNSIM_HOME="$HOME/ndnSIM"
SCENARIO_DIR="$HOME/ndnSIM/scenario"
WORKFLOW_DIR="$HOME/ndnSIM/scenario/workflows"
TOPOLOGY_DIR="$HOME/ndnSIM/scenario/topologies"
SCENARIO_JSON_DIR="$SCENARIO_DIR/scenario_json"

mkdir -p "$SCENARIO_JSON_DIR"



csv_out="$SCENARIO_DIR/perf-results-generic.csv"

header="Example, Interest Packets Generated, Data Packets Generated, Interest Packets Transmitted, Data Packets Transmitted, Critical-Path-Metric, CPM-t_exec(ns), Service Latency(us), Final Result, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"

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
    	parse_out=$( \
    		"$SCENARIO_DIR/waf" --run="ndn-cabeee-generic --scenario=$scenario_json" |& tee "$scenario_log" | sed -n \
    		-e 's/^\s*The final answer is: \([0-9]*\)$/\1,/p' \
    		-e 's/^\s*Service Latency: \([0-9\.]*\) microseconds.$/\1,/p' \
    		| tr -d '\n' \
    	)

    	echo "Parsing logs..."
    	result="$(echo "$parse_out" | cut -d',' -f1)"
    	latency="$(echo "$parse_out" | cut -d',' -f2)"

    	packets=$( \
    		python process_nfd_logs.py | sed -n \
    		-e 's/^Interest Packets Generated: \([0-9]*\) interests$/\1,/p' \
    		-e 's/^Data Packets Generated: \([0-9]*\) data$/\1,/p' \
    		-e 's/^Interest Packets Transmitted: \([0-9]*\) interests$/\1,/p' \
    		-e 's/^Data Packets Transmitted: \([0-9]*\) data/\1,/p' \
    		| tr -d '\n' \
    	)

    	interest_gen="$(echo "$packets" | cut -d',' -f1)"
    	data_gen="$(echo "$packets" | cut -d',' -f2)"
    	interest_trans="$(echo "$packets" | cut -d',' -f3)"
    	data_trans="$(echo "$packets" | cut -d',' -f4)"

        set +e
        # cpm now determines type, wf, hosting, and topo information from the full json, and somehow pass it into the cpm program cleanly
        cpm_output=$(../CPM/cpm --scenarioJSON "${scenario_json}" 2>&1)
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

    	row="$scenario, $interest_gen, $data_gen, $interest_trans, $data_trans, $cpm, $cpm_t, $latency, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"

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
