#!/bin/sh

set -e

# Generate a single timestamp to be used for all files in this run
TIMESTAMP=$(date +%Y%m%d-%H%M%S)

workdir="$PWD/generated_scenarios"
mkdir -p "$workdir"

generate_linear_wf() {
    # generate linear workflow
    length="$1"
    output="${TIMESTAMP}-wf-linear-${length}srv.json"
    ./genworkflow.py layered --num-services "$length" --num-layers "$((length+2))" --num-producers 1 --num-consumers 1 --output "$workdir/$output"
    #./genworkflow.py layered -n "$length" -l "$((length+2))" -p 1 -c 1 -o "$output"
    echo "$output"
}

generate_parallel_wf_1() {
    # generate parallel workflow with multiple sensors, intermediate services, into aggregator, into consumer
    width="$1"
    output="${TIMESTAMP}-wf-parallel-${width}prod-${width}srv.json"
    ./genworkflow.py layered --num-services 0 --num-layers 2 --aggregate --split --num-producers "$width" --num-consumers 1 --output "$workdir/$output"
    #./genworkflow.py layered -n 0 -l 2 -a -t -p "$width" -c 1 -o "$output"
    echo "$output"
}

generate_parallel_wf_2() {
    # generate parallel workflow with 1 sensor, intermediate services, into aggregator, into consumer
    width="$1"
    output="${TIMESTAMP}-wf-parallel-1prod-${width}srv.json"
    ./genworkflow.py layered --num-services "$width" --num-layers 3 --aggregate --num-producers 1 --num-consumers 1 --output "$workdir/$output"
    echo "$output"
}

generate_messy_wf() {
    servs="$1"
    prods="$2"
    cons="$3"
    layers="$4"
    skips="$5"
    output="${TIMESTAMP}-wf-messy-${servs}srv-${prods}prod-${cons}con-${skips}skip-agg-${layers}layer.json"
    ./genworkflow.py layered --num-services "$servs" --num-layers "$layers" --aggregate --num-producers "$prods" --num-consumers "$cons" --num-skips "$skips" --output "$workdir/$output"
    #output1="${output%.json}-c1.json"
    #output2="${output%.json}-c2.json"
    #output3="${output%.json}-c3.json"
    #output4="${output%.json}-c4.json"
    #./genworkflow.py separate --workflow "$output" --output "$output1" "$output2" "$output3" "$output4"
    echo "$output"
}

generate_tp() {
    nodes="$1"
    edges="$2"
    snsrs="$3"
    usrs="$4"
    output="${TIMESTAMP}-tp-${nodes}rtr-${snsrs}snsr-${usrs}usr-${edges}edge.json"
    ./gentopo.py -o "$workdir/$output" spanning_tree -n "$1" -e "$2" -s "$snsrs" -u "$usrs"
    echo "$output"
}

generate_hs() {
    wf="$1"
    tp="$2"
    snsrs="$3"
    usrs="$4"
    makespanMin="$5"
    makespanMax="$6"

    # Strip the timestamp from the input filenames before building the new name
    # ${wf#*-} removes the date, then ${inner#*-} removes the time.
    wf_clean=${wf#*-}
    wf_clean=${wf_clean#*-}
    
    tp_clean=${tp#*-}
    tp_clean=${tp_clean#*-}

    #output="${TIMESTAMP}-hs-${1%.json}-${2%.json}.json"
    output="${TIMESTAMP}-hs-${wf_clean%.json}-${tp_clean%.json}.json"
    ./genhosting.py --output "$workdir/$output" uniform \
        --workflow "$workdir/$wf" \
        --topology "$workdir/$tp" \
        -s "$snsrs" \
        -u "$usrs" \
        -msmin "$makespanMin" \
        -msmax "$makespanMax"
    echo "$output"
}

length=5
#wf="$(generate_linear_wf ${length})"
width=18
wf="$(generate_parallel_wf_1 ${width})"
#wf="$(generate_parallel_wf_2 ${width})"
#wf="$(generate_messy_wf ${width})"

nodes=3
edges=2
sensors=1
users=1
tp="$(generate_tp ${nodes} ${edges} ${sensors} ${users})"

makespanMin=0
makespanMax=0
hs="$(generate_hs "$wf" "$tp" ${sensors} ${users} ${makespanMin} ${makespanMax})"


# Clean the hs name for the final scenario folder
hs_clean=${hs#*-}
hs_clean=${hs_clean#*-}
    #--output "$workdir/${TIMESTAMP}-sn-${hs#hs-}" \

./build_scenario.py -f \
    --workflow "$workdir/$wf" \
    --topo-json "$workdir/$tp" \
    --topo-txt "$workdir/${tp%.json}.txt" \
    --hosting "$workdir/$hs" \
    --output "$workdir/${TIMESTAMP}-sn-${hs_clean#hs-}" \
    --prefix "nescoSCOPT" \
    --serviceDiscovery 0 \
    --resourceAllocation 0 \
    --allocationReuse 0 \
    --scheduleCompaction 0 \
    --startTimeOffsetSD 0 \
    --startTimeOffsetWF 0 \
    --simulationEndTime 200

cp $workdir/${TIMESTAMP}-sn-${hs_clean#hs-} ../cascon_random_test/