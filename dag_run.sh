#!/bin/bash


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

scenarios=(
# 4 DAG
"ndn-cabeee-4dag-orchestratorA"
"ndn-cabeee-4dag-orchestratorB"
"ndn-cabeee-4dag-nesco"
"ndn-cabeee-4dag-nescoSCOPT"
# 8 DAG
"ndn-cabeee-8dag-orchestratorA"
"ndn-cabeee-8dag-orchestratorB"
"ndn-cabeee-8dag-nesco"
"ndn-cabeee-8dag-nescoSCOPT"
# 8 DAG w/ caching
"ndn-cabeee-8dag-caching-orchestratorA"
"ndn-cabeee-8dag-caching-orchestratorB"
"ndn-cabeee-8dag-caching-nesco"
"ndn-cabeee-8dag-caching-nescoSCOPT"
# 20 Parallel
"ndn-cabeee-20node-parallel-orchestratorA"
"ndn-cabeee-20node-parallel-orchestratorB"
"ndn-cabeee-20node-parallel-nesco"
"ndn-cabeee-20node-parallel-nescoSCOPT"
# 20 Sensor (Parallel)
"ndn-cabeee-20sensor-orchestratorA"
"ndn-cabeee-20sensor-orchestratorB"
"ndn-cabeee-20sensor-nesco"
"ndn-cabeee-20sensor-nescoSCOPT"
# 20-Node Linear
"ndn-cabeee-20node-linear-orchestratorA"
"ndn-cabeee-20node-linear-orchestratorB"
"ndn-cabeee-20node-linear-nesco"
"ndn-cabeee-20node-linear-nescoSCOPT"
# 20 Linear (new hosting using 3node topology)
"ndn-cabeee-20linear-orchestratorA"
"ndn-cabeee-20linear-orchestratorB"
"ndn-cabeee-20linear-nesco"
"ndn-cabeee-20linear-nescoSCOPT"
# Misc
#"ndn-cabeee-8dag-reuse"
#"ndn-cabeee-20sensor-3node"
)

scenario_log="$SCENARIO_DIR/scenario.log"
csv_out="$SCENARIO_DIR/perf-results-simulation.csv"
header="Example, Interest Packets Generated, Data Packets Generated, Interest Packets Transmitted, Data Packets Transmitted, Service Latency, Time, ns-3 commit, pybindgen commit, scenario commit, ndnSIM commit"

if [ ! -f "$csv_out" ]; then
	echo "$header" > "$csv_out"
elif ! grep -q -F "$header" "$csv_out"; then
	mv "$csv_out" "$csv_out.bak"
	echo "Overwriting csv..."
	echo "$header" > "$csv_out"
else
	cp "$csv_out" "$csv_out.bak"
fi

for scenario in "${scenarios[@]}"
do
	echo "Scenaraio: $scenario"

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

	row="$scenario, $interest_gen, $data_gen, $interest_trans, $data_trans, $latency, $result, $now, $ns_3_hash, $pybindgen_hash, $scenario_hash, $ndnsim_hash"

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
