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


LOGS=CustomAppConsumer="error|warn"
LOGS=$LOGS:CustomAppConsumerServiceDiscovery="error|warn|info"
LOGS=$LOGS:CustomAppConsumerPoisson="error|warn|info"
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


declare -a scenarios=(
	# 20 Reuse (Abilene topology)
	"ndn-cabeee-intervals-20reuse-orchestratorA orchA 20-reuse.json 20-reuse-inAbilene.hosting topo-cabeee-Abilene.txt"
####"ndn-cabeee-intervals-20reuse-orchestratorB orchB 20-reuse.json 20-reuse-inAbilene.hosting topo-cabeee-Abilene.txt"
	"ndn-cabeee-intervals-20reuse-nesco nesco 20-reuse.json 20-reuse-inAbilene.hosting topo-cabeee-Abilene.txt"
	"ndn-cabeee-intervals-20reuse-nescoSCOPT nescoSCOPT 20-reuse.json 20-reuse-inAbilene.hosting topo-cabeee-Abilene.txt"
	)
	
scenario_log="$SCENARIO_DIR/scenario.log"
csv_out="$SCENARIO_DIR/perf-results-simulation-intervals2-reuse.csv"




header="Scenario/Scheme, Scenario, Min Service Latency(us), Low Quartile Service Latency(us), Mid Quartile Service Latency(us), High Quartile Service Latency(us), Max Service Latency(us), Total Service Latency(us), Avg Service Latency(us), Requests Fulfilled, Final Result, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"

if [ ! -f "$csv_out" ]; then
	echo "Creating csv..."
	echo "$header" > "$csv_out"
elif ! grep -q -F "$header" "$csv_out"; then
	echo "Overwriting csv..."
	mv "$csv_out" "$csv_out.bak"
	echo "$header" > "$csv_out"
else
	#echo "Updating csv..."
	#cp "$csv_out" "$csv_out.bak"
	echo "Overwriting csv..."
	mv "$csv_out" "$csv_out.bak"
	echo "$header" > "$csv_out"
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
	#"$SCENARIO_DIR/waf" --run="$scenario" |& tee "$scenario_log"
	"$SCENARIO_DIR/waf" --run="$scenario" &> "$scenario_log"

	echo "Parsing logs..."

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


	#latencies=$( \
		#python process_nfd_logs_intervals.py "$scenario_log" | sed -n \
		#-e 's/^\s*\/consumerL min latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerL low latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerL mid latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerL high latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerL max latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerL total latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerL avg latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerL requests fulfilled: \([0-9\.]*\) total requests$/\1,/p' \
		#-e 's/^\s*\/consumerL Final answer: \([0-9\.]*\) numerical$/\1,/p' \
		#-e 's/^\s*\/consumerP min latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerP low latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerP mid latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerP high latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerP max latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerP total latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerP avg latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerP requests fulfilled: \([0-9\.]*\) total requests$/\1,/p' \
		#-e 's/^\s*\/consumerP Final answer: \([0-9\.]*\) numerical$/\1,/p' \
		#-e 's/^\s*\/consumerR min latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerR low latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerR mid latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerR high latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerR max latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerR total latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerR avg latency: \([0-9\.]*\) microseconds$/\1,/p' \
		#-e 's/^\s*\/consumerR requests fulfilled: \([0-9\.]*\) total requests$/\1,/p' \
		#-e 's/^\s*\/consumerR Final answer: \([0-9\.]*\) numerical$/\1,/p' \
		#| tr -d '\n' \
	#)
	latencies=$(python process_nfd_logs_intervals.py "$scenario_log")

	sensor_min_latency="$(echo "$latencies" | cut -d',' -f1)"
	sensor_low_latency="$(echo "$latencies" | cut -d',' -f2)"
	sensor_mid_latency="$(echo "$latencies" | cut -d',' -f3)"
	sensor_high_latency="$(echo "$latencies" | cut -d',' -f4)"
	sensor_max_latency="$(echo "$latencies" | cut -d',' -f5)"
	sensor_total_latency="$(echo "$latencies" | cut -d',' -f6)"
	sensor_avg_latency="$(echo "$latencies" | cut -d',' -f7)"
	sensor_requests_fulfilled="$(echo "$latencies" | cut -d',' -f8)"
	sensor_final_answer="$(echo "$latencies" | cut -d',' -f9)"

	linear_min_latency="$(echo "$latencies" | cut -d',' -f10)"
	linear_low_latency="$(echo "$latencies" | cut -d',' -f11)"
	linear_mid_latency="$(echo "$latencies" | cut -d',' -f12)"
	linear_high_latency="$(echo "$latencies" | cut -d',' -f13)"
	linear_max_latency="$(echo "$latencies" | cut -d',' -f14)"
	linear_total_latency="$(echo "$latencies" | cut -d',' -f15)"
	linear_avg_latency="$(echo "$latencies" | cut -d',' -f16)"
	linear_requests_fulfilled="$(echo "$latencies" | cut -d',' -f17)"
	linear_final_answer="$(echo "$latencies" | cut -d',' -f18)"

	reuse_min_latency="$(echo "$latencies" | cut -d',' -f19)"
	reuse_low_latency="$(echo "$latencies" | cut -d',' -f20)"
	reuse_mid_latency="$(echo "$latencies" | cut -d',' -f21)"
	reuse_high_latency="$(echo "$latencies" | cut -d',' -f22)"
	reuse_max_latency="$(echo "$latencies" | cut -d',' -f23)"
	reuse_total_latency="$(echo "$latencies" | cut -d',' -f24)"
	reuse_avg_latency="$(echo "$latencies" | cut -d',' -f25)"
	reuse_requests_fulfilled="$(echo "$latencies" | cut -d',' -f26)"
	reuse_final_answer="$(echo "$latencies" | cut -d',' -f27)"

	row1="$scenario, 20-sensor, $sensor_min_latency, $sensor_low_latency, $sensor_mid_latency, $sensor_high_latency, $sensor_max_latency, $sensor_total_latency, $sensor_avg_latency, $sensor_requests_fulfilled, $sensor_final_answer, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"
	row2="$scenario, 20-linear, $linear_min_latency, $linear_low_latency, $linear_mid_latency, $linear_high_latency, $linear_max_latency, $linear_total_latency, $linear_avg_latency, $linear_requests_fulfilled, $linear_final_answer, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"
	row3="$scenario, 20-reuse, $reuse_min_latency, $reuse_low_latency, $reuse_mid_latency, $reuse_high_latency, $reuse_max_latency, $reuse_total_latency, $reuse_avg_latency, $reuse_requests_fulfilled, $reuse_final_answer, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"

	echo "Dumping to csv..."
	#line_num="$(grep -n -F "$scenario," "$csv_out" | cut -d: -f1 | head -1)"
	#if [ -n "$line_num" ]; then
		#sed --in-place -e "${line_num}c\\$row" "$csv_out"
	#else
		echo "$row1"
		echo "$row2"
		echo "$row3"
		echo "$row1" >> "$csv_out"
		echo "$row2" >> "$csv_out"
		echo "$row3" >> "$csv_out"
	#fi

	echo ""
done

mv cs-trace*.txt trace_results
mv cs-usage*.txt trace_results

echo "All scenarios ran"
