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

#declare -a scenarios=(
#	# 4 DAG
#	#"ndn-cabeee-4dag-orchestratorA orchA 4dag.json 4dag.hosting topo-cabeee-3node"
#	#"ndn-cabeee-4dag-orchestratorB orchB 4dag.json 4dag.hosting topo-cabeee-3node"
#	"ndn-cabeee-4dag-nesco nesco 4dag.json 4dag.hosting topo-cabeee-3node"
#	"ndn-cabeee-4dag-nescoSCOPT nescoSCOPT 4dag.json 4dag.hosting topo-cabeee-3node"
#	# 8 DAG
#	#"ndn-cabeee-8dag-orchestratorA orchA 8dag.json 8dag.hosting topo-cabeee-3node"
#	#"ndn-cabeee-8dag-orchestratorB orchB 8dag.json 8dag.hosting topo-cabeee-3node"
#	"ndn-cabeee-8dag-nesco nesco 8dag.json 8dag.hosting topo-cabeee-3node"
#	"ndn-cabeee-8dag-nescoSCOPT nescoSCOPT 8dag.json 8dag.hosting topo-cabeee-3node"
#	# 8 DAG w/ caching
#	#"ndn-cabeee-8dag-caching-orchestratorA orchA 8dag.json 8dag.hosting topo-cabeee-3node"
#	#"ndn-cabeee-8dag-caching-orchestratorB orchB 8dag.json 8dag.hosting topo-cabeee-3node"
#	"ndn-cabeee-8dag-caching-nesco nesco 8dag.json 8dag.hosting topo-cabeee-3node"
#	"ndn-cabeee-8dag-caching-nescoSCOPT nescoSCOPT 8dag.json 8dag.hosting topo-cabeee-3node"
#	# 20 Sensor (new hosting using 3node topology)
#	#"ndn-cabeee-20sensor-orchestratorA orchA 20-sensor.json 20-sensor-in3node.hosting topo-cabeee-3node"
#	#"ndn-cabeee-20sensor-orchestratorB orchB 20-sensor.json 20-sensor-in3node.hosting topo-cabeee-3node"
#	"ndn-cabeee-20sensor-nesco nesco 20-sensor.json 20-sensor-in3node.hosting topo-cabeee-3node"
#	"ndn-cabeee-20sensor-nescoSCOPT nescoSCOPT 20-sensor.json 20-sensor-in3node.hosting topo-cabeee-3node"
#	# 20 Linear (new hosting using 3node topology)
#	#"ndn-cabeee-20linear-orchestratorA orchA 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node"
#	#"ndn-cabeee-20linear-orchestratorB orchB 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node"
#	"ndn-cabeee-20linear-nesco nesco 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node"
#	"ndn-cabeee-20linear-nescoSCOPT nescoSCOPT 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node"
#	# 20 Reuse (new hosting using 3node topology)
#	#"ndn-cabeee-20reuse-orchestratorA orchA 20-reuse.json 20-reuse-in3node.hosting topo-cabeee-3node"
#	#"ndn-cabeee-20reuse-orchestratorB orchB 20-reuse.json 20-reuse-in3node.hosting topo-cabeee-3node"
#	"ndn-cabeee-20reuse-nesco nesco 20-reuse.json 20-reuse-in3node.hosting topo-cabeee-3node"
#	"ndn-cabeee-20reuse-nescoSCOPT nescoSCOPT 20-reuse.json 20-reuse-in3node.hosting topo-cabeee-3node"
#
#	# 20 Scramble (new hosting using 3node topology)
#	#"ndn-cabeee-20scrambled-orchestratorA orchA 20-linear.json 20-scrambled-in3node.hosting topo-cabeee-3node"
#	#"ndn-cabeee-20scrambled-orchestratorB orchB 20-linear.json 20-scrambled-in3node.hosting topo-cabeee-3node"
#	#"ndn-cabeee-20scrambled-nesco nesco 20-linear.json 20-scrambled-in3node.hosting topo-cabeee-3node"
#	#"ndn-cabeee-20scrambled-nescoSCOPT nescoSCOPT 20-linear.json 20-scrambled-in3node.hosting topo-cabeee-3node"
#	# 20 Parallel (new hosting using 3node topology)
#	#"ndn-cabeee-20parallel-orchestratorA orchA 20-parallel.json 20-parallel-in3node.hosting topo-cabeee-3node"
#	#"ndn-cabeee-20parallel-orchestratorB orchB 20-parallel.json 20-parallel-in3node.hosting topo-cabeee-3node"
#	#"ndn-cabeee-20parallel-nesco nesco 20-parallel.json 20-parallel-in3node.hosting topo-cabeee-3node"
#	#"ndn-cabeee-20parallel-nescoSCOPT nescoSCOPT 20-parallel.json 20-parallel-in3node.hosting topo-cabeee-3node"
#	# 20-Node Linear
#	#"ndn-cabeee-20node-linear-orchestratorA orchA 20-linear.json 20-linear.hosting topo-cabeee-20node-linear"
#	#"ndn-cabeee-20node-linear-orchestratorB orchB 20-linear.json 20-linear.hosting topo-cabeee-20node-linear"
#	#"ndn-cabeee-20node-linear-nesco nescoSCOPT 20-linear.json 20-linear.hosting topo-cabeee-20node-linear"
#	#"ndn-cabeee-20node-linear-nescoSCOPT nescoSCOPT 20-linear.json 20-linear.hosting topo-cabeee-20node-linear"
#	# 20-Node Parallel
#	#"ndn-cabeee-20node-parallel-orchestratorA orchA 20-parallel.json 20-parallel.hosting topo-cabeee-20node-parallel"
#	#"ndn-cabeee-20node-parallel-orchestratorB orchB 20-parallel.json 20-parallel.hosting topo-cabeee-20node-parallel"
#	#"ndn-cabeee-20node-parallel-nesco nescoSCOPT 20-parallel.json 20-parallel.hosting topo-cabeee-20node-parallel"
#	#"ndn-cabeee-20node-parallel-nescoSCOPT nescoSCOPT 20-parallel.json 20-parallel.hosting topo-cabeee-20node-parallel"
#	# 20-Sensor (Parallel with 20 nodes and 20 sensors)
#	#"ndn-cabeee-20sensor-parallel-orchestratorA orchA 20-sensor.json 20-sensor.hosting topo-cabeee-20sensor"
#	#"ndn-cabeee-20sensor-parallel-orchestratorB orchB 20-sensor.json 20-sensor.hosting topo-cabeee-20sensor"
#	#"ndn-cabeee-20sensor-parallel-nesco nescoSCOPT 20-sensor.json 20-sensor.hosting topo-cabeee-20sensor"
#	#"ndn-cabeee-20sensor-parallel-nescoSCOPT nescoSCOPT 20-sensor.json 20-sensor.hosting topo-cabeee-20sensor"
#	# 20-Node Scrambled
#	# Misc
#	#"ndn-cabeee-8dag-reuse"
#	#"ndn-cabeee-20sensor-3node"
#)

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


#for iterator in "${scenarios[@]}"
#do
	#read -a itrArray <<< "$iterator" #default whitespace IFS
	#scenario=${itrArray[0]}
	#type=${itrArray[1]}
	#wf=${WORKFLOW_DIR}/${itrArray[2]}
	#hosting=${WORKFLOW_DIR}/${itrArray[3]}
	#topo=${TOPOLOGY_DIR}/${itrArray[4]}
	#echo "Scenario: $scenario"
	#echo "type: $type"
	#echo "Workflow: $wf"
	#echo "Hosting: $hosting"
	#echo "Topology: $topo"

# Run a scenario for every file found in the directory
for filepath in "$SCENARIO_JSON_DIR"/*
do
	filename=$(basename "$filepath ") 	# remove path
	scenario="${filename%.*}"			# remove file extension
	echo "Scenario: $scenario"

	scenario_log="$SCENARIO_DIR/scenario.log"

	scenario_json="$SCENARIO_JSON_DIR/$scenario.json"

	# TODO: keep this commented out for now, since I don't want the json files overwritten.
	#"$SCENARIO_DIR/build_scenario.py" \
		#--topo "$topo.txt" --topo-out "$topo.json" \
		#--dag "$wf" --hosting "$hosting" --prefix "$type" | jq > "$scenario_json"

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

    # cpm now determines type, wf, hosting, and topo information from the full json, and somehow pass it into the cpm program cleanly
    cpm_output=$(../CPM/cpm --scenarioJSON "${scenario_json}")

    cpm=$(echo "$cpm_output" | sed -n 's/^metric: \([0-9]*\)/\1/p' | tr -d '\n')
    cpm_t=$(echo "$cpm_output" | sed -n 's/^time: \([0-9]*\) ns/\1/p' | tr -d '\n')

	row="$scenario, $interest_gen, $data_gen, $interest_trans, $data_trans, $cpm, $cpm_t, $latency, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"

	echo "Dumping to csv..."
	line_num="$(grep -n -F "$scenario," "$csv_out" | cut -d: -f1 | head -1)"
	if [ -n "$line_num" ]; then
		sed --in-place -e "${line_num}c\\$row" "$csv_out"
	else
		echo "$row" >> "$csv_out"
	fi

	echo ""
done

echo "All scenarios ran"
