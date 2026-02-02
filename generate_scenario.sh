#!/bin/sh

# examples generators
# TODO

generate_linear_wf() {
    # generate linear workflow
    length="$1"
    output="wf-linear-${length}srv.json"
    ./genworkflow.py layered --num-services "$length" --num-layers "$((length+2))" --num-producers 1 --num-consumers 1 --output "$output"
    #./genworkflow.py layered -n "$length" -l "$((length+2))" -p 1 -c 1 -o "$output"
}

generate_parallel_wf_1() {
    # generate parallel workflow with 12 sensors, intermediate services, into aggregator, into consumer
    width="$1"
    output="wf-parallel-${width}prod-${width}srv.json"
    ./genworkflow.py layered --num-services 0 --num-layers 2 --aggregate --split --num-producers "$width" --num-consumers 1 --output "$output"
    #./genworkflow.py layered -n 0 -l 2 -a -t -p "$width" -c 1 -o "$output"
}

generate_parallel_wf_2() {
    # generate parallel workflow with 1 sensors, intermediate services, into aggregator, into consumer
    width="$1"
    output="wf-parallel-1prod-${width}srv.json"
    ./genworkflow.py layered --num-services "$width" --num-layers 3 --aggregate --num-producers 1 --num-consumers 1 --output "$output"
}

generate_messy_wf() {
    servs="$1"
    prods="$1"
    cons="$1"
    layers="$1"
    output="wf-messy-${servs}srv-${prods}prod-${cons}con-agg-${layers}layer.json"
    ./genworkflow.py layered --num-services 14 --num-layers 6 --aggregate --num-producers 3 --num-consumers 4 --output "$output"
    output1="${output%.json}-c1.json"
    output2="${output%.json}-c2.json"
    output3="${output%.json}-c3.json"
    output4="${output%.json}-c4.json"
    ./genworkflow.py separate --workflow "$output" --output "$output1" "$output2" "$output3" "$output4"
}

generate_tp() {
    n="$1"
    e="$2"
    output="tp-${n}node-${e}edge.json"
    ./gentopo.py -o "$output" spanning_tree -n "$1" -e "$2"
}

generate_hs() {
    wf="$1"
    tp="$2"
    output="hs-${1%.json}-${2%.json}.json"
    ./genhosting.py --workflow "$wf" --topology "$tp" --output "$output"
}
