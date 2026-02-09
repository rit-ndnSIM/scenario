#!/bin/sh

set -e

workdir="$PWD/generated_scenarios"
mkdir -p "$workdir"

generate_linear_wf() {
    # generate linear workflow
    length="$1"
    output="wf-linear-${length}srv.json"
    ./genworkflow.py layered --num-services "$length" --num-layers "$((length+2))" --num-producers 1 --num-consumers 1 --output "$workdir/$output"
    #./genworkflow.py layered -n "$length" -l "$((length+2))" -p 1 -c 1 -o "$output"
    echo "$output"
}

generate_parallel_wf_1() {
    # generate parallel workflow with 12 sensors, intermediate services, into aggregator, into consumer
    width="$1"
    output="wf-parallel-${width}prod-${width}srv.json"
    ./genworkflow.py layered --num-services 0 --num-layers 2 --aggregate --split --num-producers "$width" --num-consumers 1 --output "$workdir/$output"
    #./genworkflow.py layered -n 0 -l 2 -a -t -p "$width" -c 1 -o "$output"
    echo "$output"
}

generate_parallel_wf_2() {
    # generate parallel workflow with 1 sensors, intermediate services, into aggregator, into consumer
    width="$1"
    output="wf-parallel-1prod-${width}srv.json"
    ./genworkflow.py layered --num-services "$width" --num-layers 3 --aggregate --num-producers 1 --num-consumers 1 --output "$workdir/$output"
    echo "$output"
}

generate_messy_wf() {
    servs="$1"
    prods="$2"
    cons="$3"
    layers="$4"
    skips="$5"
    output="wf-messy-${servs}srv-${prods}prod-${cons}con-${skips}skip-agg-${layers}layer.json"
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
    output="tp-${nodes}rtr-${snsrs}snsr-${usrs}usr-${edges}edge.json"
    ./gentopo.py -o "$workdir/$output" spanning_tree -n "$1" -e "$2" -s "$snsrs" -u "$usrs"
    echo "$output"
}

generate_hs() {
    wf="$1"
    tp="$2"
    snsrs="$3"
    usrs="$4"
    output="hs-${1%.json}-${2%.json}.json"
    ./genhosting.py --output "$workdir/$output" uniform --workflow "$workdir/$wf" --topology "$workdir/$tp" -s "$snsrs" -u "$usrs"
    echo "$output"
}

wf="$(generate_parallel_wf_1 18)"
tp="$(generate_tp 3 2 1 1)"
hs="$(generate_hs "$wf" "$tp" 1 1)"

./build_scenario.py -f --workflow "$workdir/$wf" --topo-json "$workdir/$tp" --topo-txt "$workdir/${tp%.json}.txt" --hosting "$workdir/$hs" --output "$workdir/sn-${hs#hs-}"
