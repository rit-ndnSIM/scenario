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

declare -a scenarios=(
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


    # CriticalPath-Metric (CPM) Scenarios
    "ndn-cabeee-cpm-orchA-10_2 orchA cpm-10_x.json cpm-10_2.hosting topo-cabeee-cpm-x_2"
    "ndn-cabeee-cpm-orchA-10_5 orchA cpm-10_x.json cpm-10_5.hosting topo-cabeee-cpm-x_5"
    "ndn-cabeee-cpm-orchA-10_10 orchA cpm-10_x.json cpm-10_10.hosting topo-cabeee-cpm-x_10"
    "ndn-cabeee-cpm-orchA-10_20 orchA cpm-10_x.json cpm-10_20.hosting topo-cabeee-cpm-x_20"
    "ndn-cabeee-cpm-orchA-10_50 orchA cpm-10_x.json cpm-10_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-orchA-20_5 orchA cpm-20_x.json cpm-20_5.hosting topo-cabeee-cpm-x_5"
    "ndn-cabeee-cpm-orchA-20_10 orchA cpm-20_x.json cpm-20_10.hosting topo-cabeee-cpm-x_10"
    "ndn-cabeee-cpm-orchA-20_20 orchA cpm-20_x.json cpm-20_20.hosting topo-cabeee-cpm-x_20"
    "ndn-cabeee-cpm-orchA-20_40 orchA cpm-20_x.json cpm-20_40.hosting topo-cabeee-cpm-x_40"
    "ndn-cabeee-cpm-orchA-20_100 orchA cpm-20_x.json cpm-20_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-orchA-50_12 orchA cpm-50_x.json cpm-50_12.hosting topo-cabeee-cpm-x_12"
    "ndn-cabeee-cpm-orchA-50_25 orchA cpm-50_x.json cpm-50_25.hosting topo-cabeee-cpm-x_25"
    "ndn-cabeee-cpm-orchA-50_50 orchA cpm-50_x.json cpm-50_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-orchA-50_100 orchA cpm-50_x.json cpm-50_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-orchA-50_250 orchA cpm-50_x.json cpm-50_250.hosting topo-cabeee-cpm-x_250"
    "ndn-cabeee-cpm-orchA-100_25 orchA cpm-100_x.json cpm-100_25.hosting topo-cabeee-cpm-x_25"
    "ndn-cabeee-cpm-orchA-100_50 orchA cpm-100_x.json cpm-100_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-orchA-100_100 orchA cpm-100_x.json cpm-100_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-orchA-100_200 orchA cpm-100_x.json cpm-100_200.hosting topo-cabeee-cpm-x_200"
    "ndn-cabeee-cpm-orchA-100_500 orchA cpm-100_x.json cpm-100_500.hosting topo-cabeee-cpm-x_500"
    
    "ndn-cabeee-cpm-orchB-10_2 orchB cpm-10_x.json cpm-10_2.hosting topo-cabeee-cpm-x_2"
    "ndn-cabeee-cpm-orchB-10_5 orchB cpm-10_x.json cpm-10_5.hosting topo-cabeee-cpm-x_5"
    "ndn-cabeee-cpm-orchB-10_10 orchB cpm-10_x.json cpm-10_10.hosting topo-cabeee-cpm-x_10"
    "ndn-cabeee-cpm-orchB-10_20 orchB cpm-10_x.json cpm-10_20.hosting topo-cabeee-cpm-x_20"
    "ndn-cabeee-cpm-orchB-10_50 orchB cpm-10_x.json cpm-10_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-orchB-20_5 orchB cpm-20_x.json cpm-20_5.hosting topo-cabeee-cpm-x_5"
    "ndn-cabeee-cpm-orchB-20_10 orchB cpm-20_x.json cpm-20_10.hosting topo-cabeee-cpm-x_10"
    "ndn-cabeee-cpm-orchB-20_20 orchB cpm-20_x.json cpm-20_20.hosting topo-cabeee-cpm-x_20"
    "ndn-cabeee-cpm-orchB-20_40 orchB cpm-20_x.json cpm-20_40.hosting topo-cabeee-cpm-x_40"
    "ndn-cabeee-cpm-orchB-20_100 orchB cpm-20_x.json cpm-20_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-orchB-50_12 orchB cpm-50_x.json cpm-50_12.hosting topo-cabeee-cpm-x_12"
    "ndn-cabeee-cpm-orchB-50_25 orchB cpm-50_x.json cpm-50_25.hosting topo-cabeee-cpm-x_25"
    "ndn-cabeee-cpm-orchB-50_50 orchB cpm-50_x.json cpm-50_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-orchB-50_100 orchB cpm-50_x.json cpm-50_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-orchB-50_250 orchB cpm-50_x.json cpm-50_250.hosting topo-cabeee-cpm-x_250"
    "ndn-cabeee-cpm-orchB-100_25 orchB cpm-100_x.json cpm-100_25.hosting topo-cabeee-cpm-x_25"
    "ndn-cabeee-cpm-orchB-100_50 orchB cpm-100_x.json cpm-100_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-orchB-100_100 orchB cpm-100_x.json cpm-100_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-orchB-100_200 orchB cpm-100_x.json cpm-100_200.hosting topo-cabeee-cpm-x_200"
    "ndn-cabeee-cpm-orchB-100_500 orchB cpm-100_x.json cpm-100_500.hosting topo-cabeee-cpm-x_500"
    
    "ndn-cabeee-cpm-nesco-10_2 nesco cpm-10_x.json cpm-10_2.hosting topo-cabeee-cpm-x_2"
    "ndn-cabeee-cpm-nesco-10_5 nesco cpm-10_x.json cpm-10_5.hosting topo-cabeee-cpm-x_5"
    "ndn-cabeee-cpm-nesco-10_10 nesco cpm-10_x.json cpm-10_10.hosting topo-cabeee-cpm-x_10"
    "ndn-cabeee-cpm-nesco-10_20 nesco cpm-10_x.json cpm-10_20.hosting topo-cabeee-cpm-x_20"
    "ndn-cabeee-cpm-nesco-10_50 nesco cpm-10_x.json cpm-10_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-nesco-20_5 nesco cpm-20_x.json cpm-20_5.hosting topo-cabeee-cpm-x_5"
    "ndn-cabeee-cpm-nesco-20_10 nesco cpm-20_x.json cpm-20_10.hosting topo-cabeee-cpm-x_10"
    "ndn-cabeee-cpm-nesco-20_20 nesco cpm-20_x.json cpm-20_20.hosting topo-cabeee-cpm-x_20"
    "ndn-cabeee-cpm-nesco-20_40 nesco cpm-20_x.json cpm-20_40.hosting topo-cabeee-cpm-x_40"
    "ndn-cabeee-cpm-nesco-20_100 nesco cpm-20_x.json cpm-20_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-nesco-50_12 nesco cpm-50_x.json cpm-50_12.hosting topo-cabeee-cpm-x_12"
    "ndn-cabeee-cpm-nesco-50_25 nesco cpm-50_x.json cpm-50_25.hosting topo-cabeee-cpm-x_25"
    "ndn-cabeee-cpm-nesco-50_50 nesco cpm-50_x.json cpm-50_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-nesco-50_100 nesco cpm-50_x.json cpm-50_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-nesco-50_250 nesco cpm-50_x.json cpm-50_250.hosting topo-cabeee-cpm-x_250"
    "ndn-cabeee-cpm-nesco-100_25 nesco cpm-100_x.json cpm-100_25.hosting topo-cabeee-cpm-x_25"
    "ndn-cabeee-cpm-nesco-100_50 nesco cpm-100_x.json cpm-100_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-nesco-100_100 nesco cpm-100_x.json cpm-100_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-nesco-100_200 nesco cpm-100_x.json cpm-100_200.hosting topo-cabeee-cpm-x_200"
    "ndn-cabeee-cpm-nesco-100_500 nesco cpm-100_x.json cpm-100_500.hosting topo-cabeee-cpm-x_500"
    
    "ndn-cabeee-cpm-nescoSCOPT-10_2 nescoSCOPT cpm-10_x.json cpm-10_2.hosting topo-cabeee-cpm-x_2"
    "ndn-cabeee-cpm-nescoSCOPT-10_5 nescoSCOPT cpm-10_x.json cpm-10_5.hosting topo-cabeee-cpm-x_5"
    "ndn-cabeee-cpm-nescoSCOPT-10_10 nescoSCOPT cpm-10_x.json cpm-10_10.hosting topo-cabeee-cpm-x_10"
    "ndn-cabeee-cpm-nescoSCOPT-10_20 nescoSCOPT cpm-10_x.json cpm-10_20.hosting topo-cabeee-cpm-x_20"
    "ndn-cabeee-cpm-nescoSCOPT-10_50 nescoSCOPT cpm-10_x.json cpm-10_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-nescoSCOPT-20_5 nescoSCOPT cpm-20_x.json cpm-20_5.hosting topo-cabeee-cpm-x_5"
    "ndn-cabeee-cpm-nescoSCOPT-20_10 nescoSCOPT cpm-20_x.json cpm-20_10.hosting topo-cabeee-cpm-x_10"
    "ndn-cabeee-cpm-nescoSCOPT-20_20 nescoSCOPT cpm-20_x.json cpm-20_20.hosting topo-cabeee-cpm-x_20"
    "ndn-cabeee-cpm-nescoSCOPT-20_40 nescoSCOPT cpm-20_x.json cpm-20_40.hosting topo-cabeee-cpm-x_40"
    "ndn-cabeee-cpm-nescoSCOPT-20_100 nescoSCOPT cpm-20_x.json cpm-20_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-nescoSCOPT-50_12 nescoSCOPT cpm-50_x.json cpm-50_12.hosting topo-cabeee-cpm-x_12"
    "ndn-cabeee-cpm-nescoSCOPT-50_25 nescoSCOPT cpm-50_x.json cpm-50_25.hosting topo-cabeee-cpm-x_25"
    "ndn-cabeee-cpm-nescoSCOPT-50_50 nescoSCOPT cpm-50_x.json cpm-50_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-nescoSCOPT-50_100 nescoSCOPT cpm-50_x.json cpm-50_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-nescoSCOPT-50_250 nescoSCOPT cpm-50_x.json cpm-50_250.hosting topo-cabeee-cpm-x_250"
    "ndn-cabeee-cpm-nescoSCOPT-100_25 nescoSCOPT cpm-100_x.json cpm-100_25.hosting topo-cabeee-cpm-x_25"
    "ndn-cabeee-cpm-nescoSCOPT-100_50 nescoSCOPT cpm-100_x.json cpm-100_50.hosting topo-cabeee-cpm-x_50"
    "ndn-cabeee-cpm-nescoSCOPT-100_100 nescoSCOPT cpm-100_x.json cpm-100_100.hosting topo-cabeee-cpm-x_100"
    "ndn-cabeee-cpm-nescoSCOPT-100_200 nescoSCOPT cpm-100_x.json cpm-100_200.hosting topo-cabeee-cpm-x_200"
    "ndn-cabeee-cpm-nescoSCOPT-100_500 nescoSCOPT cpm-100_x.json cpm-100_500.hosting topo-cabeee-cpm-x_500"
)


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
	echo "Topology: $topo".txt

    scenario_json="$SCENARIO_JSON_DIR/$scenario.json"

    # keep this commented out if you don't want the json files overwritten.
    "$SCENARIO_DIR/build_scenario.py" \
        --topo-txt "$topo.txt" --topo-json "$topo.json" \
        --workflow "$wf" --hosting "$hosting" --prefix "$type" | jq > "$scenario_json"
done





echo "All scenarios generated"
