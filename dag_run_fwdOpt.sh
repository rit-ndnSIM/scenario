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
LOGS=$LOGS:CustomAppConsumerServiceDiscovery
LOGS=$LOGS:CustomAppConsumer2
LOGS=$LOGS:CustomAppProducer
LOGS=$LOGS:DagForwarderApp
LOGS=$LOGS:DagServiceDiscoveryApp
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
	#"ndn-cabeee-fwdOpt-4dag-nesco-noSD nesco 4dag.json 4dag-fwdOpt.hosting topo-cabeee-3node-fwdOpt.json"
	#"ndn-cabeee-fwdOpt-4dag-nesco-SD-noAllocation nesco 4dag.json 4dag-fwdOpt.hosting topo-cabeee-3node-fwdOpt.json"
	#"ndn-cabeee-fwdOpt-4dag-nesco-SD-allocation nesco 4dag.json 4dag-fwdOpt.hosting topo-cabeee-3node-fwdOpt.json"
	"ndn-cabeee-fwdOpt-8dag-nesco-noSD nesco 8dag.json 8dag-fwdOpt.hosting topo-cabeee-3node-fwdOpt.json"
	"ndn-cabeee-fwdOpt-8dag-nesco-SD-noAllocation nesco 8dag.json 8dag-fwdOpt.hosting topo-cabeee-3node-fwdOpt.json"
	"ndn-cabeee-fwdOpt-8dag-nesco-SD-allocation nesco 8dag.json 8dag-fwdOpt.hosting topo-cabeee-3node-fwdOpt.json"
	)
	
scenario_log="$SCENARIO_DIR/scenario.log"
csv_out="$SCENARIO_DIR/perf-results-simulation-fwdOpt.csv"




header="Example, Interest Packets Generated, Data Packets Generated, Interest Packets Transmitted, Data Packets Transmitted, Critical-Path-Metric, CPM-t_exec(ns), SD Estimated Service Latency(us), Service Latency(us), Final Result, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"

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
	#parse_out=$( \
	#	"$SCENARIO_DIR/waf" --run="$scenario" |& tee "$scenario_log" | sed -n \
	#	-e 's/^\s*Service Latency estimated by SD: \([0-9\.]*\) microseconds.$/\1,/p' \
	#	-e 's/^\s*Service Latency: \([0-9\.]*\) microseconds.$/\1,/p' \
	#	-e 's/^\s*The final answer is: \([0-9]*\)$/\1,/p' \
	#	| tr -d '\n' \
	#)

	# Run the simulation, piping ALL output (stderr and stdout) to tee, 
    # which duplicates it to the screen and the log file ($scenario_log).
	"$SCENARIO_DIR/waf" --run="$scenario" |& tee "$scenario_log"

	echo "Parsing logs..."
	#estimatedLatency="$(echo "$parse_out" | cut -d',' -f1)"
	#latency="$(echo "$parse_out" | cut -d',' -f2)"
	#result="$(echo "$parse_out" | cut -d',' -f3)"

	# Temporarily set the Internal Field Separator to a comma
    #IFS=',' read -r -a parseArray <<< "$parse_out"
    #estimatedLatency="${parseArray[0]:-N.A.}"	# 1. SD Estimated Service Latency (Expected in parseArray[0])
    #latency="${parseArray[1]:-N.A.}"    		# 2. Service Latency (Expected in parseArray[1])
    #result="${parseArray[2]:-N.A.}"    			# 3. Final Result (Expected in parseArray[2])

	# 1. SD Estimated Service Latency (Col 8)
    estimatedLatency=$(grep "Service Latency estimated by SD:" "$scenario_log" | sed -n 's/^\s*Service Latency estimated by SD: \([0-9\.]*\) microseconds.$/\1/p')
    estimatedLatency="${estimatedLatency:-N.A.}"
    
    # 2. Service Latency (Col 9)
    latency=$(grep "Service Latency:" "$scenario_log" | sed -n 's/^\s*Service Latency: \([0-9\.]*\) microseconds.$/\1/p')
    latency="${latency:-N.A.}"
    
    # 3. Final Result (Col 10)
    result=$(grep "The final answer is:" "$scenario_log" | sed -n 's/^\s*The final answer is: \([0-9]*\)$/\1/p')
    result="${result:-N.A.}"

	packets=$( \
		python process_nfd_logs.py | sed -n \
		-e 's/^Interest Packets Generated: \([0-9]*\) interests$/\1,/p' \
		-e 's/^Data Packets Generated: \([0-9]*\) data$/\1,/p' \
		-e 's/^Interest Packets Transmitted: \([0-9]*\) interests$/\1,/p' \
		-e 's/^Data Packets Transmitted: \([0-9]*\) data/\1,/p' \
		| tr -d '\n' \
	)

	#interest_gen="$(echo "$packets" | cut -d',' -f1)"
	#data_gen="$(echo "$packets" | cut -d',' -f2)"
	#interest_trans="$(echo "$packets" | cut -d',' -f3)"
	#data_trans="$(echo "$packets" | cut -d',' -f4)"

	IFS=',' read -r -a packetArray <<< "$packets"
	interest_gen="${packetArray[0]:-N.A.}"
	data_gen="${packetArray[1]:-N.A.}"
	interest_trans="${packetArray[2]:-N.A.}"
	data_trans="${packetArray[3]:-N.A.}"

	cpm=$( \
		../CPM/cpm --scheme ${type} --workflow ${wf} --hosting ${hosting} --topology ${topo} | sed -n \
		-e 's/^metric: \([0-9]*\)/\1/p' \
		| tr -d '\n' \
	)
	cpm_t=$( \
		../CPM/cpm --scheme ${type} --workflow ${wf} --hosting ${hosting} --topology ${topo} | sed -n \
		-e 's/^time: \([0-9]*\) ns/\1/p' \
		| tr -d '\n' \
	)

	row="$scenario, $interest_gen, $data_gen, $interest_trans, $data_trans, $cpm, $cpm_t, $estimatedLatency, $latency, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"

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
