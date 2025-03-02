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
#LOGS=$LOGS:CustomAppConsumerPoisson
#LOGS=$LOGS:CustomAppConsumer2
#LOGS=$LOGS:CustomAppProducer
#LOGS=$LOGS:DagForwarderApp
#LOGS=$LOGS:DagOrchestratorA_App
#LOGS=$LOGS:DagServiceA_App
#LOGS=$LOGS:DagOrchestratorB_App
#LOGS=$LOGS:DagServiceB_App
#LOGS=$LOGS:ndn.App
#LOGS=$LOGS:ndn-cxx.nfd.Forwarder

export NS_LOG="$LOGS"

NDNSIM_HOME="$HOME/ndnSIM"
SCENARIO_DIR="$HOME/ndnSIM/scenario"
WORKFLOW_DIR="$HOME/ndnSIM/scenario/workflows"
TOPOLOGY_DIR="$HOME/ndnSIM/scenario/topologies"


declare -a scenarios=(
	# 20 Sensor (using 3node topology)
	"ndn-cabeee-intervals-20sensor-orchestratorA orchA 20-sensor.json 20-sensor-in3node.hosting topo-cabeee-3node.txt"
####"ndn-cabeee-intervals-20sensor-orchestratorB orchB 20-sensor.json 20-sensor-in3node.hosting topo-cabeee-3node.txt"
	"ndn-cabeee-intervals-20sensor-nesco nesco 20-sensor.json 20-sensor-in3node.hosting topo-cabeee-3node.txt"
	"ndn-cabeee-intervals-20sensor-nescoSCOPT nescoSCOPT 20-sensor.json 20-sensor-in3node.hosting topo-cabeee-3node.txt"
	# 20 Linear (using 3node topology)
	"ndn-cabeee-intervals-20linear-orchestratorA orchA 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node.txt"
####"ndn-cabeee-intervals-20linear-orchestratorB orchB 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node.txt"
	"ndn-cabeee-intervals-20linear-nesco nesco 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node.txt"
	"ndn-cabeee-intervals-20linear-nescoSCOPT nescoSCOPT 20-linear.json 20-linear-in3node.hosting topo-cabeee-3node.txt"
	# 20 Scramble (using 3node topology)
	"ndn-cabeee-intervals-20scrambled-orchestratorA orchA 20-linear.json 20-scrambled-in3node.hosting topo-cabeee-3node.txt"
####"ndn-cabeee-intervals-20scrambled-orchestratorB orchB 20-linear.json 20-scrambled-in3node.hosting topo-cabeee-3node.txt"
	"ndn-cabeee-intervals-20scrambled-nesco nesco 20-linear.json 20-scrambled-in3node.hosting topo-cabeee-3node.txt"
	"ndn-cabeee-intervals-20scrambled-nescoSCOPT nescoSCOPT 20-linear.json 20-scrambled-in3node.hosting topo-cabeee-3node.txt"
	)
	
scenario_log="$SCENARIO_DIR/scenario.log"
csv_out="$SCENARIO_DIR/perf-results-simulation-intervals.csv"




header="Example, Min Service Latency(us), Low Quartile Service Latency(us), Mid Quartile Service Latency(us), High Quartile Service Latency(us), Max Service Latency(us), Total Service Latency(us), Avg Service Latency(us), Requests Fulfilled, Final Result, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"

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
	"$SCENARIO_DIR/waf" --run="$scenario" |& tee "$scenario_log"

	echo "Parsing logs..."
	latencies=$( \
		python process_nfd_logs_intervals.py "$scenario_log" | sed -n \
		-e 's/^\s*consumer min latency: \([0-9\.]*\) microseconds$/\1,/p' \
		-e 's/^\s*consumer low latency: \([0-9\.]*\) microseconds$/\1,/p' \
		-e 's/^\s*consumer mid latency: \([0-9\.]*\) microseconds$/\1,/p' \
		-e 's/^\s*consumer high latency: \([0-9\.]*\) microseconds$/\1,/p' \
		-e 's/^\s*consumer max latency: \([0-9\.]*\) microseconds$/\1,/p' \
		-e 's/^\s*consumer total latency: \([0-9\.]*\) microseconds$/\1,/p' \
		-e 's/^\s*consumer avg latency: \([0-9\.]*\) microseconds$/\1,/p' \
		-e 's/^\s*consumer requests fulfilled: \([0-9\.]*\) total requests$/\1,/p' \
		-e 's/^\s*consumer Final answer: \([0-9\.]*\) numerical$/\1,/p' \
		| tr -d '\n' \
	)
	min_latency="$(echo "$latencies" | cut -d',' -f1)"
	low_latency="$(echo "$latencies" | cut -d',' -f2)"
	mid_latency="$(echo "$latencies" | cut -d',' -f3)"
	high_latency="$(echo "$latencies" | cut -d',' -f4)"
	max_latency="$(echo "$latencies" | cut -d',' -f5)"
	total_latency="$(echo "$latencies" | cut -d',' -f6)"
	avg_latency="$(echo "$latencies" | cut -d',' -f7)"
	requests_fulfilled="$(echo "$latencies" | cut -d',' -f8)"
	final_answer="$(echo "$latencies" | cut -d',' -f9)"


	row="$scenario, $min_latency, $low_latency, $mid_latency, $high_latency, $max_latency, $total_latency, $avg_latency, $requests_fulfilled, $final_answer, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"

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
