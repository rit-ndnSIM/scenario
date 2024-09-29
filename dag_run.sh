#!/bin/bash


#----- THIS ONLY NEEDS TO BE RUN ONCE --------------------
#cd ~/ndnSIM/scenario
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
#export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
#./waf clean
#CXXFLAGS="-std=c++17" ./waf configure --debug
#---------------------------------------------------------


clear

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

declare -a scenarios=(
# 4 DAG
"ndn-cabeee-4dag-orchestratorA orchA 4dag.json 4dag.hosting topo-cabeee-3node.txt"
"ndn-cabeee-4dag-orchestratorB orchB 4dag.json 4dag.hosting topo-cabeee-3node.txt"
"ndn-cabeee-4dag-nesco nescoSCOPT 4dag.json 4dag.hosting topo-cabeee-3node.txt"
"ndn-cabeee-4dag-nescoSCOPT nescoSCOPT 4dag.json 4dag.hosting topo-cabeee-3node.txt"
# 8 DAG
"ndn-cabeee-8dag-orchestratorA orchA 8dag.json 8dag.hosting topo-cabeee-3node.txt"
"ndn-cabeee-8dag-orchestratorB orchB 8dag.json 8dag.hosting topo-cabeee-3node.txt"
"ndn-cabeee-8dag-nesco nescoSCOPT 8dag.json 8dag.hosting topo-cabeee-3node.txt"
"ndn-cabeee-8dag-nescoSCOPT nescoSCOPT 8dag.json 8dag.hosting topo-cabeee-3node.txt"
# 8 DAG w/ caching
"ndn-cabeee-8dag-caching-orchestratorA orchA 8dag.json 8dag.hosting topo-cabeee-3node.txt"
"ndn-cabeee-8dag-caching-orchestratorB orchB 8dag.json 8dag.hosting topo-cabeee-3node.txt"
"ndn-cabeee-8dag-caching-nesco nescoSCOPT 8dag.json 8dag.hosting topo-cabeee-3node.txt"
"ndn-cabeee-8dag-caching-nescoSCOPT nescoSCOPT 8dag.json 8dag.hosting topo-cabeee-3node.txt"
# 20 Parallel (new hosting using 3node topology
"ndn-cabeee-20node-parallel-orchestratorA orchA 20-parallel.json 20-parallel.hosting topo-cabeee-20node-parallel.txt"
"ndn-cabeee-20node-parallel-orchestratorB orchB 20-parallel.json 20-parallel.hosting topo-cabeee-20node-parallel.txt"
"ndn-cabeee-20node-parallel-nesco nescoSCOPT 20-parallel.json 20-parallel.hosting topo-cabeee-20node-parallel.txt"
"ndn-cabeee-20node-parallel-nescoSCOPT nescoSCOPT 20-parallel.json 20-parallel.hosting topo-cabeee-20node-parallel.txt"
# 20 Sensor (new hosting using 3node topology)
"ndn-cabeee-20sensor-orchestratorA orchA 20-sensor.json 20-sensor.hosting topo-cabeee-20sensor.txt"
"ndn-cabeee-20sensor-orchestratorB orchB 20-sensor.json 20-sensor.hosting topo-cabeee-20sensor.txt"
"ndn-cabeee-20sensor-nesco nescoSCOPT 20-sensor.json 20-sensor.hosting topo-cabeee-20sensor.txt"
"ndn-cabeee-20sensor-nescoSCOPT nescoSCOPT 20-sensor.json 20-sensor.hosting topo-cabeee-20sensor.txt"
# 20 Linear (new hosting using 3node topology)
"ndn-cabeee-20linear-orchestratorA orchA 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node.txt"
"ndn-cabeee-20linear-orchestratorB orchB 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node.txt"
"ndn-cabeee-20linear-nesco nescoSCOPT 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node.txt"
"ndn-cabeee-20linear-nescoSCOPT nescoSCOPT 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node.txt"
# 20-Node Linear
"ndn-cabeee-20node-linear-orchestratorA orchA 20-linear.json 20-linear.hosting topo-cabeee-20node-linear.txt"
"ndn-cabeee-20node-linear-orchestratorB orchB 20-linear.json 20-linear.hosting topo-cabeee-20node-linear.txt"
"ndn-cabeee-20node-linear-nesco nescoSCOPT 20-linear.json 20-linear.hosting topo-cabeee-20node-linear.txt"
"ndn-cabeee-20node-linear-nescoSCOPT nescoSCOPT 20-linear.json 20-linear.hosting topo-cabeee-20node-linear.txt"
# Misc
#"ndn-cabeee-8dag-reuse"
#"ndn-cabeee-20sensor-3node"

# CriticalPath-Metric (CPM) Scenarios
#"ndn-cabeee-cpm-orchA-10_2 orchA cpm-10_x.json cpm-10_2.hosting topo-cabeee-cpm-x_2.txt"
#"ndn-cabeee-cpm-orchA-10_5 orchA cpm-10_x.json cpm-10_5.hosting topo-cabeee-cpm-x_5.txt"
#"ndn-cabeee-cpm-orchA-10_10 orchA cpm-10_x.json cpm-10_10.hosting topo-cabeee-cpm-x_10.txt"
#"ndn-cabeee-cpm-orchA-10_20 orchA cpm-10_x.json cpm-10_20.hosting topo-cabeee-cpm-x_20.txt"
#"ndn-cabeee-cpm-orchA-10_50 orchA cpm-10_x.json cpm-10_50.hosting topo-cabeee-cpm-x_50.txt"
#"ndn-cabeee-cpm-orchA-20_5 orchA cpm-20_x.json cpm-20_5.hosting topo-cabeee-cpm-x_5.txt"
#"ndn-cabeee-cpm-orchA-20_10 orchA cpm-20_x.json cpm-20_10.hosting topo-cabeee-cpm-x_10.txt"
#"ndn-cabeee-cpm-orchA-20_20 orchA cpm-20_x.json cpm-20_20.hosting topo-cabeee-cpm-x_20.txt"
#"ndn-cabeee-cpm-orchA-20_40 orchA cpm-20_x.json cpm-20_40.hosting topo-cabeee-cpm-x_40.txt"
#"ndn-cabeee-cpm-orchA-20_100 orchA cpm-20_x.json cpm-20_100.hosting topo-cabeee-cpm-x_100.txt"
#"ndn-cabeee-cpm-orchA-50_12 orchA cpm-50_x.json cpm-50_12.hosting topo-cabeee-cpm-x_12.txt"
#"ndn-cabeee-cpm-orchA-50_25 orchA cpm-50_x.json cpm-50_25.hosting topo-cabeee-cpm-x_25.txt"
#"ndn-cabeee-cpm-orchA-50_50 orchA cpm-50_x.json cpm-50_50.hosting topo-cabeee-cpm-x_50.txt"
#"ndn-cabeee-cpm-orchA-50_100 orchA cpm-50_x.json cpm-50_100.hosting topo-cabeee-cpm-x_100.txt"
#"ndn-cabeee-cpm-orchA-50_250 orchA cpm-50_x.json cpm-50_250.hosting topo-cabeee-cpm-x_250.txt"
#"ndn-cabeee-cpm-orchA-100_25 orchA cpm-100_x.json cpm-100_25.hosting topo-cabeee-cpm-x_25.txt"
#"ndn-cabeee-cpm-orchA-100_50 orchA cpm-100_x.json cpm-100_50.hosting topo-cabeee-cpm-x_50.txt"
#"ndn-cabeee-cpm-orchA-100_100 orchA cpm-100_x.json cpm-100_100.hosting topo-cabeee-cpm-x_100.txt"
#"ndn-cabeee-cpm-orchA-100_200 orchA cpm-100_x.json cpm-100_200.hosting topo-cabeee-cpm-x_200.txt"
#"ndn-cabeee-cpm-orchA-100_500 orchA cpm-100_x.json cpm-100_500.hosting topo-cabeee-cpm-x_500.txt"

#"ndn-cabeee-cpm-orchB-10_2 orchB cpm-10_x.json cpm-10_2.hosting topo-cabeee-cpm-x_2.txt"
#"ndn-cabeee-cpm-orchB-10_5 orchB cpm-10_x.json cpm-10_5.hosting topo-cabeee-cpm-x_5.txt"
#"ndn-cabeee-cpm-orchB-10_10 orchB cpm-10_x.json cpm-10_10.hosting topo-cabeee-cpm-x_10.txt"
#"ndn-cabeee-cpm-orchB-10_20 orchB cpm-10_x.json cpm-10_20.hosting topo-cabeee-cpm-x_20.txt"
#"ndn-cabeee-cpm-orchB-10_50 orchB cpm-10_x.json cpm-10_50.hosting topo-cabeee-cpm-x_50.txt"
#"ndn-cabeee-cpm-orchB-20_5 orchB cpm-20_x.json cpm-20_5.hosting topo-cabeee-cpm-x_5.txt"
#"ndn-cabeee-cpm-orchB-20_10 orchB cpm-20_x.json cpm-20_10.hosting topo-cabeee-cpm-x_10.txt"
#"ndn-cabeee-cpm-orchB-20_20 orchB cpm-20_x.json cpm-20_20.hosting topo-cabeee-cpm-x_20.txt"
#"ndn-cabeee-cpm-orchB-20_40 orchB cpm-20_x.json cpm-20_40.hosting topo-cabeee-cpm-x_40.txt"
#"ndn-cabeee-cpm-orchB-20_100 orchB cpm-20_x.json cpm-20_100.hosting topo-cabeee-cpm-x_100.txt"
#"ndn-cabeee-cpm-orchB-50_12 orchB cpm-50_x.json cpm-50_12.hosting topo-cabeee-cpm-x_12.txt"
#"ndn-cabeee-cpm-orchB-50_25 orchB cpm-50_x.json cpm-50_25.hosting topo-cabeee-cpm-x_25.txt"
#"ndn-cabeee-cpm-orchB-50_50 orchB cpm-50_x.json cpm-50_50.hosting topo-cabeee-cpm-x_50.txt"
#"ndn-cabeee-cpm-orchB-50_100 orchB cpm-50_x.json cpm-50_100.hosting topo-cabeee-cpm-x_100.txt"
#"ndn-cabeee-cpm-orchB-50_250 orchB cpm-50_x.json cpm-50_250.hosting topo-cabeee-cpm-x_250.txt"
#"ndn-cabeee-cpm-orchB-100_25 orchB cpm-100_x.json cpm-100_25.hosting topo-cabeee-cpm-x_25.txt"
#"ndn-cabeee-cpm-orchB-100_50 orchB cpm-100_x.json cpm-100_50.hosting topo-cabeee-cpm-x_50.txt"
#"ndn-cabeee-cpm-orchB-100_100 orchB cpm-100_x.json cpm-100_100.hosting topo-cabeee-cpm-x_100.txt"
#"ndn-cabeee-cpm-orchB-100_200 orchB cpm-100_x.json cpm-100_200.hosting topo-cabeee-cpm-x_200.txt"
#"ndn-cabeee-cpm-orchB-100_500 orchB cpm-100_x.json cpm-100_500.hosting topo-cabeee-cpm-x_500.txt"

#"ndn-cabeee-cpm-nescoSCOPT-10_2 nescoSCOPT cpm-10_x.json cpm-10_2.hosting topo-cabeee-cpm-x_2.txt"
#"ndn-cabeee-cpm-nescoSCOPT-10_5 nescoSCOPT cpm-10_x.json cpm-10_5.hosting topo-cabeee-cpm-x_5.txt"
#"ndn-cabeee-cpm-nescoSCOPT-10_10 nescoSCOPT cpm-10_x.json cpm-10_10.hosting topo-cabeee-cpm-x_10.txt"
#"ndn-cabeee-cpm-nescoSCOPT-10_20 nescoSCOPT cpm-10_x.json cpm-10_20.hosting topo-cabeee-cpm-x_20.txt"
#"ndn-cabeee-cpm-nescoSCOPT-10_50 nescoSCOPT cpm-10_x.json cpm-10_50.hosting topo-cabeee-cpm-x_50.txt"
#"ndn-cabeee-cpm-nescoSCOPT-20_5 nescoSCOPT cpm-20_x.json cpm-20_5.hosting topo-cabeee-cpm-x_5.txt"
#"ndn-cabeee-cpm-nescoSCOPT-20_10 nescoSCOPT cpm-20_x.json cpm-20_10.hosting topo-cabeee-cpm-x_10.txt"
#"ndn-cabeee-cpm-nescoSCOPT-20_20 nescoSCOPT cpm-20_x.json cpm-20_20.hosting topo-cabeee-cpm-x_20.txt"
#"ndn-cabeee-cpm-nescoSCOPT-20_40 nescoSCOPT cpm-20_x.json cpm-20_40.hosting topo-cabeee-cpm-x_40.txt"
#"ndn-cabeee-cpm-nescoSCOPT-20_100 nescoSCOPT cpm-20_x.json cpm-20_100.hosting topo-cabeee-cpm-x_100.txt"
#"ndn-cabeee-cpm-nescoSCOPT-50_12 nescoSCOPT cpm-50_x.json cpm-50_12.hosting topo-cabeee-cpm-x_12.txt"
#"ndn-cabeee-cpm-nescoSCOPT-50_25 nescoSCOPT cpm-50_x.json cpm-50_25.hosting topo-cabeee-cpm-x_25.txt"
#"ndn-cabeee-cpm-nescoSCOPT-50_50 nescoSCOPT cpm-50_x.json cpm-50_50.hosting topo-cabeee-cpm-x_50.txt"
#"ndn-cabeee-cpm-nescoSCOPT-50_100 nescoSCOPT cpm-50_x.json cpm-50_100.hosting topo-cabeee-cpm-x_100.txt"
#"ndn-cabeee-cpm-nescoSCOPT-50_250 nescoSCOPT cpm-50_x.json cpm-50_250.hosting topo-cabeee-cpm-x_250.txt"
#"ndn-cabeee-cpm-nescoSCOPT-100_25 nescoSCOPT cpm-100_x.json cpm-100_25.hosting topo-cabeee-cpm-x_25.txt"
#"ndn-cabeee-cpm-nescoSCOPT-100_50 nescoSCOPT cpm-100_x.json cpm-100_50.hosting topo-cabeee-cpm-x_50.txt"
#"ndn-cabeee-cpm-nescoSCOPT-100_100 nescoSCOPT cpm-100_x.json cpm-100_100.hosting topo-cabeee-cpm-x_100.txt"
#"ndn-cabeee-cpm-nescoSCOPT-100_200 nescoSCOPT cpm-100_x.json cpm-100_200.hosting topo-cabeee-cpm-x_200.txt"
#"ndn-cabeee-cpm-nescoSCOPT-100_500 nescoSCOPT cpm-100_x.json cpm-100_500.hosting topo-cabeee-cpm-x_500.txt"

)

scenario_log="$SCENARIO_DIR/scenario.log"
csv_out="$SCENARIO_DIR/perf-results-simulation.csv"
header="Example, Interest Packets Generated, Data Packets Generated, Interest Packets Transmitted, Data Packets Transmitted, Critical-Path-Metric, Service Latency, Final Result, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"

if [ ! -f "$csv_out" ]; then
	echo "$header" > "$csv_out"
elif ! grep -q -F "$header" "$csv_out"; then
	mv "$csv_out" "$csv_out.bak"
	echo "Overwriting csv..."
	echo "$header" > "$csv_out"
else
	cp "$csv_out" "$csv_out.bak"
fi


for iterator in "${scenarios[@]}"
do
	read -a itrArray <<< "$iterator" #default whitespace IFS
	scenario=${itrArray[0]}
	type=${itrArray[1]}
	wf=${WORKFLOW_DIR}/${itrArray[2]}
	hosting=${WORKFLOW_DIR}/${itrArray[3]}
	topo=${TOPOLOGY_DIR}/${itrArray[4]}
	echo "Scenario: $scenario"
	echo "type: $type"
	echo "Workflow: $wf"
	echo "Hosting: $hosting"
	echo "Topology: $topo"

	now="$(date -Iseconds)"

	ns_3_hash="$(git -C "$NDNSIM_HOME/ns-3" rev-parse HEAD)"
	pybindgen_hash="$(git -C "$NDNSIM_HOME/pybindgen" rev-parse HEAD)"
	scenario_hash="$(git -C "$NDNSIM_HOME/scenario" rev-parse HEAD)"
	ndnsim_hash="$(git -C "$NDNSIM_HOME/ns-3/src/ndnSIM" rev-parse HEAD)"

	echo "Running..."
	parse_out=$( \
		"$SCENARIO_DIR/waf" --run="$scenario" |& tee "$scenario_log" | sed -n \
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

	cpm=$( \
		python critical-path-metric.py -type ${type} -workflow ${wf} -hosting ${hosting} -topology ${topo} | sed -n \
		-e 's/^metric is \([0-9]*\)/\1/p' \
		| tr -d '\n' \
	)

	row="$scenario, $interest_gen, $data_gen, $interest_trans, $data_trans, $cpm, $latency, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"

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
