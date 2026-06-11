#!/bin/bash

set -e

# Generate a single timestamp to be used for all files in this run
export name="fwdOptSD2Sweep20_5x8"
export TIMESTAMP=$(date +%Y%m%d-%H%M%S)
export workdir="$PWD/generated_scenarios/$name"
mkdir -p "$workdir"
mkdir -p "../${name}/"

generate_wf_linear() {
    # generate linear workflow
    length="$1"
    ser="$2"
    # Create a zero-padded version of the length (e.g., 20 -> 020)
    padded_length=$(printf "%03d" "$length")
    output="${TIMESTAMP}-${ser}--wf_linear-${padded_length}srv.json"
    ./genworkflow.py layered --num-services "$length" --num-layers "$((length+2))" --num-producers 1 --num-consumers 1 --output "$workdir/$output"
    #./genworkflow.py layered -n "$length" -l "$((length+2))" -p 1 -c 1 -o "$output"
    echo "$output"
}

generate_wf_parallel_1() {
    # generate parallel workflow with multiple sensors, intermediate services, into aggregator, into consumer
    width="$1"
    ser="$2"
    # Create a zero-padded version
    padded_width=$(printf "%03d" "$width")
    output="${TIMESTAMP}-${ser}--wf_parallel-${padded_width}prod-${padded_width}srv.json"
    ./genworkflow.py layered --num-services 0 --num-layers 2 --aggregate --split --num-producers "$width" --num-consumers 1 --output "$workdir/$output"
    #./genworkflow.py layered -n 0 -l 2 -a -t -p "$width" -c 1 -o "$output"
    echo "$output"
}

generate_wf_parallel_2() {
    # generate parallel workflow with 1 sensor, intermediate services, into aggregator, into consumer
    width="$1"
    ser="$2"
    # Create a zero-padded version
    padded_width=$(printf "%03d" "$width")
    output="${TIMESTAMP}-${ser}--wf_parallel-1prod-${padded_width}srv.json"
    ./genworkflow.py layered --num-services "$width" --num-layers 3 --aggregate --num-producers 1 --num-consumers 1 --output "$workdir/$output"
    echo "$output"
}

generate_wf_messy() {
    servs="$1"
    prods="$2"
    cons="$3"
    layers="$4"
    skips="$5"
    ser="$6"
    # Create zero-padded versions
    padded_servs=$(printf "%03d" "$servs")
    padded_prods=$(printf "%03d" "$prods")
    padded_cons=$(printf "%03d" "$cons")
    padded_layers=$(printf "%03d" "$layers")
    padded_skips=$(printf "%03d" "$skips")
    output="${TIMESTAMP}-${ser}--wf_messy-${padded_servs}srv-${padded_prods}prod-${padded_cons}con-${padded_skips}skip-agg-${padded_layers}layer.json"
    ./genworkflow.py layered --num-services "$servs" --num-layers "$layers" --aggregate --num-producers "$prods" --num-consumers "$cons" --num-skips "$skips" --output "$workdir/$output"
    #output1="${output%.json}-c1.json"
    #output2="${output%.json}-c2.json"
    #output3="${output%.json}-c3.json"
    #output4="${output%.json}-c4.json"
    #./genworkflow.py separate --workflow "$output" --output "$output1" "$output2" "$output3" "$output4"
    echo "$output"
}

generate_tp_st() {
    type="spanning_tree"
    nodes="$1"
    edges="$2"
    snsrs="$3"
    usrs="$4"
    cs="$5"
    delay="$6"
    wf="$7"
    ser="$8"
    # Create zero-padded versions
    padded_nodes=$(printf "%03d" "$nodes")
    padded_edges=$(printf "%03d" "$edges")
    padded_snsrs=$(printf "%03d" "$snsrs")
    padded_usrs=$(printf "%03d" "$usrs")
    padded_cs=$(printf "%06d" "$cs")
    output="${TIMESTAMP}-${ser}--tp_st4${wf}-${padded_nodes}rtr-${padded_snsrs}snsr-${padded_usrs}usr-${padded_edges}edge-${padded_cs}cs.json"
    ./gentopo.py -o "$workdir/$output" "$type" -n "$nodes" -e "$edges" -s "$snsrs" -u "$usrs" --cs-size "$cs" --delay "$delay"
    echo "$output"
}
generate_tp_mesh() {
    type="mesh"
    nodes="$1"
    snsrs="$2"
    usrs="$3"
    prob="$4"
    cs="$5"
    delay="$6"
    wf="$7"
    ser="$8"
    # Create zero-padded versions
    padded_nodes=$(printf "%03d" "$nodes")
    padded_snsrs=$(printf "%03d" "$snsrs")
    padded_usrs=$(printf "%03d" "$usrs")
    padded_cs=$(printf "%06d" "$cs")
    output="${TIMESTAMP}-${ser}--tp_mesh4${wf}-${padded_nodes}rtr-${padded_snsrs}snsr-${padded_usrs}usr-${padded_cs}cs.json"
    ./gentopo.py -o "$workdir/$output" "$type" -n "$nodes" -s "$snsrs" -u "$usrs" -p "$prob" --cs-size "$cs" --delay "$delay"
    echo "$output"
}
generate_tp_sos() {
    type="star_of_stars"
    nodes="$1"
    snsrs="$2"
    usrs="$3"
    branches="$4"
    cs="$5"
    delay="$6"
    wf="$7"
    ser="$8"
    # Create zero-padded versions
    padded_nodes=$(printf "%03d" "$nodes")
    padded_snsrs=$(printf "%03d" "$snsrs")
    padded_usrs=$(printf "%03d" "$usrs")
    padded_cs=$(printf "%06d" "$cs")
    output="${TIMESTAMP}-${ser}--tp_sos4${wf}-${padded_nodes}rtr-${padded_snsrs}snsr-${padded_usrs}usr-${padded_cs}cs.json"
    ./gentopo.py -o "$workdir/$output" "$type" -n "$nodes" -s "$snsrs" -u "$usrs" -b "$branches" --cs-size "$cs" --delay "$delay"
    echo "$output"
}
generate_tp_mt() {
    type="multi_tiered"
    nodes="$1"
    snsrs="$2"
    usrs="$3"
    tier_counts="$4"
    cs="$5"
    delay="$6"
    wf="$7"
    ser="$8"
    # Create zero-padded versions
    padded_nodes=$(printf "%03d" "$nodes")
    padded_snsrs=$(printf "%03d" "$snsrs")
    padded_usrs=$(printf "%03d" "$usrs")
    padded_cs=$(printf "%06d" "$cs")
    output="${TIMESTAMP}-${ser}--tp_mt4${wf}-${padded_nodes}rtr-${padded_snsrs}snsr-${padded_usrs}usr-${padded_cs}cs.json"
    ./gentopo.py -o "$workdir/$output" "$type" -n "$nodes" -s "$snsrs" -u "$usrs" --tiers "$tier_counts" --cs-size "$cs" --delay "$delay"
    echo "$output"
}

generate_hs() {
    wf_set="$1"
    tp="$2"
    snsrs="$3"
    usrs="$4"
    makespanMinNS="$5"
    makespanMaxNS="$6"
    ser="$7"
    hostRatio="$8"
    prev_hs="$9"

    hostRatio_string=$(printf "%03.1f" "$hostRatio")
    count=$(jq '.router | length' "generated_scenarios/${name}/${tp}")

    minHosts=$(echo "($count * $hostRatio) / 1" | bc)
    maxHosts=$(echo "($count * $hostRatio) / 1" | bc)
    if [ "$minHosts" -lt 1 ]; then minHosts=1; fi
    if [ "$maxHosts" -lt 1 ]; then maxHosts=1; fi

    first_wf=$(echo "$wf_set" | awk '{print $1}')
    first_wf_basename=$(basename "$first_wf")

    # Clean the topology file name safely
    tp_temp=${tp#*-}
    tp_temp=${tp_temp#*-}
    tp_clean=${tp_temp#*--}

    local output="${TIMESTAMP}-${ser}-hR_${hostRatio_string}--hs-merged_wf-${tp_clean%.json}.json"
    
    ./genhosting.py --output "$workdir/$output" uniform \
        --workflows $wf_set \
        --topology "$workdir/$tp" \
        -s "$snsrs" \
        -u "$usrs" \
        --makespan-min "$makespanMinNS" \
        --makespan-max "$makespanMaxNS" \
        --min-hosts "$minHosts" \
        --max-hosts "$maxHosts" \
        ${prev_hs:+--base-hosting "generated_scenarios/${name}/${prev_hs}"}

    echo "$output"
}


# Export functions so GNU Parallel can see them
export -f generate_wf_messy generate_tp_st generate_tp_mesh generate_tp_sos generate_tp_mt generate_hs


# Define the logic for ONE run of ONE category pair
run_category_task() {
# Define the sweep arrays
    run=$1
    wf_topo_pair=$2


    workflowCategory=${wf_topo_pair%%:*}
    topoCategory=${wf_topo_pair#*:}
    
    # Use a unique ID for this specific parallel task
    # We use a hash of the pair to keep category codes consistent but separate
    CAT_ID=$(echo "$wf_topo_pair" | cksum | cut -c1-3)
    padded_catCode=$(printf "%03d-%s" "$run" "$CAT_ID")


    '''
    case $workflowCategory in
        "linear")
            wf_set="$export_wfs_linear"
            ;;
        "map_reduce")
            wf_set="$export_wfs_map_reduce"
            ;;
        "wavefront")
            wf_set="$export_wfs_wavefront"
            ;;
        "multi_sink")
            wf_set="$export_wfs_multi_sink"
            ;;
    esac
    '''

    # Match raw file names
    case $workflowCategory in
        "linear")       wf_filenames="$export_wfs_linear" ;;
        "map_reduce")   wf_filenames="$export_wfs_map_reduce" ;;
        "wavefront")    wf_filenames="$export_wfs_wavefront" ;;
        "multi_sink")   wf_filenames="$export_wfs_multi_sink" ;;
    esac
    # DYNAMICALLY RECONSTRUCT ABSOLUTE PATHS INSIDE THREAD
    wf_set_paths=""
    for f in $wf_filenames; do
        wf_set_paths="$wf_set_paths $workdir/$f"
    done
    wf_set_paths=$(echo "$wf_set_paths" | xargs) # trim whitespace

    generated_topos=""

    # Generate Topologies
    for num_nodes in $NUM_NODES_LIST; do
        for edgeratio in $EDGERATIO_LIST; do
            case $topoCategory in
                "multi_tiered")
                    # TODO: check and change these values so that we actually generate multi-tiered topo that makes sense
                    # Use 'bc' for floating point division and piping to 'read' to handle the result
                    #nodes=$(echo "scale=0; $services / $service2node" | bc -l)
                    # Ensure nodes is at least 1 to avoid math errors
                    #if [ "$nodes" -lt 1 ]; then nodes=1; fi
                    # we use the same number of sensor topology nodes as we have producer services in the workflow. Producer services are randomly distributed onto sensors nodes.
                    #sensors=${producers}
                    sensors=1
                    if [ "$sensors" -lt 1 ]; then sensors=1; fi
                    users=1
                    tiers=$(echo "scale=0; $num_nodes / 6" | bc -l)
                    if [ "$tiers" -lt 2 ]; then tiers=2; fi
                    cs_size=0
                    delay="1ms"
                    echo "---   Topology: Multi-Tiered, nodes=$num_nodes, sensors=$sensors, users=$users, tiers=$tiers ---"
                    tp="$(generate_tp_mt ${num_nodes} ${sensors} ${users} ${tiers} ${cs_size} ${delay} ${workflowCategory} ${padded_catCode})"
                    ;;
                "mesh")
                    # Use 'bc' for floating point division and piping to 'read' to handle the result
                    #nodes=$(echo "scale=0; $services / $service2node" | bc -l)
                    # Ensure nodes is at least 1 to avoid math errors
                    #if [ "$nodes" -lt 1 ]; then nodes=1; fi
                    # we use the same number of sensor topology nodes as we have producer services in the workflow. Producer services are randomly distributed onto sensors nodes.
                    #sensors=${producers}
                    sensors=1
                    if [ "$sensors" -lt 1 ]; then sensors=1; fi
                    users=1
                    prob=0.1
                    cs_size=0
                    delay="1ms"
                    echo "---   Topology: Mesh, nodes=$num_nodes, sensors=$sensors, users=$users ---"
                    tp="$(generate_tp_mesh ${num_nodes} ${sensors} ${users} ${prob} ${cs_size} ${delay} ${workflowCategory} ${padded_catCode})"
                    ;;
                "star_of_stars")
                    # Use 'bc' for floating point division and piping to 'read' to handle the result
                    #nodes=$(echo "scale=0; $services / $service2node" | bc -l)
                    # Ensure nodes is at least 1 to avoid math errors
                    #if [ "$nodes" -lt 1 ]; then nodes=1; fi
                    # we use the same number of sensor topology nodes as we have producer services in the workflow. Producer services are randomly distributed onto sensors nodes.
                    #sensors=${producers}
                    sensors=1
                    if [ "$sensors" -lt 1 ]; then sensors=1; fi
                    users=1
                    #branches=3
                    branches=$(echo "scale=0; $num_nodes / 6" | bc -l)
                    if [ "$branches" -lt 1 ]; then branches=1; fi
                    cs_size=0
                    delay="1ms"
                    echo "---   Topology: Star-of-Stars, nodes=$num_nodes, sensors=$sensors, users=$users ---"
                    tp="$(generate_tp_sos ${num_nodes} ${sensors} ${users} ${branches} ${cs_size} ${delay} ${workflowCategory} ${padded_catCode})"
                    ;;
                "spanning_tree")
                    # Use 'bc' for floating point division and piping to 'read' to handle the result
                    #nodes=$(echo "scale=0; $services / $service2node" | bc -l)
                    # Ensure nodes is at least 1 to avoid math errors
                    #if [ "$nodes" -lt 1 ]; then nodes=1; fi
                    # Calculate edges using floating point math for the ratio
                    # Formula: edges = ((nodes-1)*(edgeratio*(nodes-2)+2)/2)
                    edges=$(echo "scale=0; ($num_nodes-1)*($edgeratio*($num_nodes-2)+2)/2" | bc -l)
                    # we use the same number of sensor topology nodes as we have producer services in the workflow. Producer services are randomly distributed onto sensors nodes.
                    #sensors=${producers}
                    sensors=1
                    if [ "$sensors" -lt 1 ]; then sensors=1; fi
                    users=1
                    cs_size=0
                    delay="1ms"
                    echo "---   Topology: Spanning Tree, nodes=$num_nodes, edges=$edges, sensors=$sensors, users=$users ---"
                    tp="$(generate_tp_st ${num_nodes} ${edges} ${sensors} ${users} ${cs_size} ${delay} ${workflowCategory} ${padded_catCode})"
                    ;;
            esac
            generated_topos="$generated_topos $tp"
        done
    done




    # Combinations
    for tp in $generated_topos; do
        prev_hs=""
        # Generate hostings (sweep through hosting ratios, adding hosts to services as we increase the ratio - retain hosting from previous iteration)
        for hostRatio in $HOSTRATIO_LIST; do
            # Hosting Selection
            sensors=1
            users=$POISSON_NUM_CONSUMERS
            makespanMinNS=8000000
            makespanMaxNS=8000000
            #hs="$(generate_hs "$wf_set_paths" "$tp" ${sensors} ${users} ${makespanMinNS} ${makespanMaxNS} ${padded_catCode} ${hostRatio} ${prev_hs})"
            hs=$(generate_hs "$wf_set_paths" "$tp" "$sensors" "$users" "$makespanMinNS" "$makespanMaxNS" "$padded_catCode" "$hostRatio" "$prev_hs")
            hostRatio_string=$(printf "%03.1f" "$hostRatio")
            prev_hs=${hs}

            # Clean the hs name
            #hs_clean=${hs#*-}
            #hs_clean=${hs_clean#*-}
            # Strip the Timestamp (Date and Time)
            # ${hs#*-} removes '20260309-', ${inner#*-} removes '012651-'
            hs_temp=${hs#*-}
            hs_temp=${hs_temp#*-}
            # Strip the Serial Number (e.g., '0001--')
            # This removes everything from the start up to the double hyphen
            hs_clean=${hs_temp#*--}


            #prefixes="nescoSCOPT orchA orchB"
            prefixes="nesco"
            for prefix in $prefixes; do


                output_filename="$workdir/${padded_catCode}-hR_${hostRatio_string}--sn-${topoCategory}-${workflowCategory}-${prefix}--1-noSD2-multicast.json"
                strategy="multicast"
                cs_size=0

            
                ./build_scenario.py -f \
                    --topo-json "$workdir/$tp" \
                    --topo-txt "$workdir/${tp%.json}.txt" \
                    --hosting "$workdir/$hs" \
                    --output "${output_filename}" \
                    --prefix ${prefix} \
                    --strategy ${strategy} \
                    --cs-size ${cs_size} \
                    --serviceDiscovery 0 \
                    --resourceUtilization 0 \
                    --resourceAllocation 0 \
                    --allocationReuse 0 \
                    --scheduleCompaction 0 \
                    --startTimeOffsetSD 1 \
                    --startTimeOffsetWF 2 \
                    --simulationEndTime 1000 \
                    --poissonConsumerFrequency ${POISSON_FREQ} \
                    --poissonConsumerNumInterests ${POISSON_NUM_INTERESTS} \
                    --workflow $wf_set_paths

                cp "${output_filename}" ../$name/

                output_filename="$workdir/${padded_catCode}-hR_${hostRatio_string}--sn-${topoCategory}-${workflowCategory}-${prefix}--2-noSD2-bestRoute.json"
                strategy="best-route"
                cs_size=0

                ./build_scenario.py -f \
                    --topo-json "$workdir/$tp" \
                    --topo-txt "$workdir/${tp%.json}.txt" \
                    --hosting "$workdir/$hs" \
                    --output "${output_filename}" \
                    --prefix ${prefix} \
                    --strategy ${strategy} \
                    --cs-size ${cs_size} \
                    --serviceDiscovery 0 \
                    --resourceUtilization 0 \
                    --resourceAllocation 0 \
                    --allocationReuse 0 \
                    --scheduleCompaction 0 \
                    --startTimeOffsetSD 1 \
                    --startTimeOffsetWF 2 \
                    --simulationEndTime 1000 \
                    --poissonConsumerFrequency ${POISSON_FREQ} \
                    --poissonConsumerNumInterests ${POISSON_NUM_INTERESTS} \
                    --workflow $wf_set_paths

                cp "${output_filename}" ../$name/


                output_filename="$workdir/${padded_catCode}-hR_${hostRatio_string}--sn-${topoCategory}-${workflowCategory}-${prefix}--3-SD2-noUtilization.json"
                strategy="best-route"
                cs_size=0

                ./build_scenario.py -f \
                    --topo-json "$workdir/$tp" \
                    --topo-txt "$workdir/${tp%.json}.txt" \
                    --hosting "$workdir/$hs" \
                    --output "${output_filename}" \
                    --prefix ${prefix} \
                    --strategy ${strategy} \
                    --cs-size ${cs_size} \
                    --serviceDiscovery 2 \
                    --resourceUtilization 0 \
                    --resourceAllocation 0 \
                    --allocationReuse 0 \
                    --scheduleCompaction 0 \
                    --startTimeOffsetSD 1 \
                    --startTimeOffsetWF 2 \
                    --simulationEndTime 1000 \
                    --poissonConsumerFrequency ${POISSON_FREQ} \
                    --poissonConsumerNumInterests ${POISSON_NUM_INTERESTS} \
                    --workflow $wf_set_paths

                cp "${output_filename}" ../$name/

                output_filename="$workdir/${padded_catCode}-hR_${hostRatio_string}--sn-${topoCategory}-${workflowCategory}-${prefix}--4-SD2-utilization-noCaching.json"
                strategy="best-route"
                cs_size=0

                ./build_scenario.py -f \
                    --topo-json "$workdir/$tp" \
                    --topo-txt "$workdir/${tp%.json}.txt" \
                    --hosting "$workdir/$hs" \
                    --output "${output_filename}" \
                    --prefix ${prefix} \
                    --strategy ${strategy} \
                    --cs-size ${cs_size} \
                    --serviceDiscovery 2 \
                    --resourceUtilization 1 \
                    --resourceAllocation 0 \
                    --allocationReuse 0 \
                    --scheduleCompaction 0 \
                    --startTimeOffsetSD 1 \
                    --startTimeOffsetWF 2 \
                    --simulationEndTime 1000 \
                    --poissonConsumerFrequency ${POISSON_FREQ} \
                    --poissonConsumerNumInterests ${POISSON_NUM_INTERESTS} \
                    --workflow $wf_set_paths

                cp "${output_filename}" ../$name/

                output_filename="$workdir/${padded_catCode}-hR_${hostRatio_string}--sn-${topoCategory}-${workflowCategory}-${prefix}--5-SD2-utilization-caching.json"
                strategy="best-route"
                cs_size=1000

                ./build_scenario.py -f \
                    --topo-json "$workdir/$tp" \
                    --topo-txt "$workdir/${tp%.json}.txt" \
                    --hosting "$workdir/$hs" \
                    --output "${output_filename}" \
                    --prefix ${prefix} \
                    --strategy ${strategy} \
                    --cs-size ${cs_size} \
                    --serviceDiscovery 2 \
                    --resourceUtilization 1 \
                    --resourceAllocation 0 \
                    --allocationReuse 0 \
                    --scheduleCompaction 0 \
                    --startTimeOffsetSD 1 \
                    --startTimeOffsetWF 2 \
                    --simulationEndTime 1000 \
                    --poissonConsumerFrequency ${POISSON_FREQ} \
                    --poissonConsumerNumInterests ${POISSON_NUM_INTERESTS} \
                    --workflow $wf_set_paths

                cp "${output_filename}" ../$name/





            done

            if [ "$VISUALIZE" = true ]; then
                # 1. Extracting Node count (looking for 'NNNrtr')
                # This takes the string like '...-004rtr-...', strips 'rtr', and removes leading zeros
                current_nodes=$(echo "$tp" | grep -oE '[0-9]{3}rtr' | sed 's/rtr//' | sed 's/^0*//')

                # 2. Extracting Service count (looking for 'SSSsrv')
                # This takes the string like '...-020srv-...', strips 'srv', and removes leading zeros
                current_servs=$(echo "$wf_set" | grep -oE '[0-9]{3}srv' | head -n 1 | sed 's/srv//' | sed 's/^0*//')

                # Handle cases where grep might fail (set to 100 if empty so we skip visualization)
                current_nodes=${current_nodes:-100}
                current_servs=${current_servs:-100}

                echo "Detected: $current_nodes nodes, $current_servs services"

                if [ "$current_nodes" -lt 9 ] && [ "$current_servs" -lt 21 ]; then
                    echo "---         Visualizing: nodes=$current_nodes,  services=$current_servs. ---"
                    ./genvisuals_top_down_hosting_colors.py "${output_filename}" > /dev/null 2>&1 || true
                    ./genvisuals_top_down_hosting_colors_hierarchical-topo.py "${output_filename}" > /dev/null 2>&1 || true
                else
                    echo "---         Skipping visualization: too many nodes ($current_nodes) or services ($current_servs). We don't visualize large graphs ---"
                fi
            fi
        done
    done
}



export -f run_category_task

# --- EXECUTION ---

# Total number of runs (each will get it's own JSON and thus its own row in the CSV file - MATLAB will average them all)
NUM_RUNS=20

NUM_SERVICES_LIST="5"
NUM_NODES_LIST="8"
EDGERATIO_LIST="0.5"
HOSTRATIO_LIST="0 0.2 0.4 0.6 0.8 1"

# Consumer options
POISSON_FREQ=100
POISSON_NUM_INTERESTS=100
POISSON_NUM_CONSUMERS=100

VISUALIZE=false

# Define specific pairs as "workflow:topology"
wf_topo_pairs="linear:multi_tiered map_reduce:star_of_stars map_reduce:mesh wavefront:mesh"

# Generate Workflows
echo "Pre-generating $POISSON_NUM_CONSUMERS unique workflows per workflow category..."
for num_services in $NUM_SERVICES_LIST; do

    for pair in $wf_topo_pairs; do
        workflowCategory=${pair%%:*}

        # Avoid running duplicates (e.g. map_reduce occurs twice in pairs)
        if [[ "$workflowCategory" == "map_reduce" && -n "$generated_wfs_map_reduce" ]]; then
            continue
        fi

        for wf_num in $(seq 1 $POISSON_NUM_CONSUMERS); do

            case $workflowCategory in
                "linear")
                    wf_code=$(printf "ln-%03d" "$wf_num")
                    producers=1
                    consumers=1
                    layers=$num_services # Each service is a layer
                    skips=0
                    echo "---         Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                    wf_linear="$(generate_wf_messy ${num_services} ${producers} ${consumers} ${layers} ${skips} ${wf_code})"
                    generated_wfs_linear="$generated_wfs_linear $wf_linear"
                    #length=$num_services
                    #wf="$(generate_wf_linear ${length})"
                    ;;
                "map_reduce")
                    wf_code=$(printf "mr-%03d" "$wf_num")
                    # 1/6 of the services will be producers
                    #producers=$(echo "scale=0; $num_services / 6" | bc -l)
                    #if [ "$producers" -lt 1 ]; then producers=1; fi
                    producers=1
                    consumers=1
                    layers=3         # Standard Map-Shuffle-Reduce
                    skips=0
                    echo "---         Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                    wf_map_reduce="$(generate_wf_messy ${num_services} ${producers} ${consumers} ${layers} ${skips} ${wf_code})"
                    generated_wfs_map_reduce="$generated_wfs_map_reduce $wf_map_reduce"
                    ;;
                "wavefront")
                    wf_code=$(printf "wf-%03d" "$wf_num")
                    # square root of the services will be producers
                    #producers=$(echo "scale=0; sqrt($num_services)" | bc -l)
                    #if [ "$producers" -lt 1 ]; then producers=1; fi
                    producers=1
                    consumers=1
                    #layers=$producers
                    layers=$(echo "scale=0; sqrt($num_services)" | bc -l)
                    if [ "$layers" -lt 3 ]; then layers=3; fi
                    skips=$(echo "scale=0; $layers / 3" | bc -l)
                    if [ "$skips" -lt 1 ]; then skips=1; fi
                    echo "---         Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                    wf_wavefront="$(generate_wf_messy ${num_services} ${producers} ${consumers} ${layers} ${skips} ${wf_code})"
                    generated_wfs_wavefront="$generated_wfs_wavefront $wf_wavefront"
                    ;;
                "multi_sink")
                    wf_code=$(printf "ms-%03d" "$wf_num")
                    #producers=2
                    producers=1
                    consumers=1
                    layers=4
                    layers=$(echo "scale=0; $num_services / 3" | bc -l)
                    if [ "$layers" -lt 1 ]; then layers=1; fi
                    skips=2
                    echo "---         Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                    wf_multi_sink="$(generate_wf_messy ${num_services} ${producers} ${consumers} ${layers} ${skips} ${wf_code})"
                    generated_wfs_multi_sink="$generated_wfs_multi_sink $wf_multi_sink"
                    ;;
            esac

            # Convert space-separated filenames into absolute paths recognizable by your python scripts
            wf_set_paths=""
            for f in $wf_set; do
                wf_set_paths="$wf_set_paths $workdir/$f"
            done
            # Trim leading space
            wf_set_paths=$(echo "$wf_set_paths" | xargs)

        done
    done
done

# Export variables globally so GNU Parallel execution threads read them successfully
export export_wfs_linear="$generated_wfs_linear"
export export_wfs_map_reduce="$generated_wfs_map_reduce"
export export_wfs_wavefront="$generated_wfs_wavefront"
export export_wfs_multi_sink="$generated_wfs_multi_sink"

export POISSON_FREQ
export POISSON_NUM_INTERESTS
export POISSON_NUM_CONSUMERS

export VISUALIZE
export NUM_NODES_LIST
export EDGERATIO_LIST
export HOSTRATIO_LIST

echo "Distributing $NUM_RUNS runs x 4 categories across all cores..."

# ::: is the GNU Parallel separator for arguments. When we provide multiple ::: input groups, GNU parallel creates a Cartesian product (cross-product) of all items.
# Group 1 ($(seq 1 20)) contains 20 items, 1-20
# Group 2 ($wf_topo_pairs) contains 4 comma-separated string items.
# Because of the two ::: operators, GNU parallel schedules 20x4 total tasks (one for each combination of the items)

#parallel --bar run_category_task ::: $(seq 1 $NUM_RUNS) ::: $wf_topo_pairs
parallel \
    --env TIMESTAMP \
    --env workdir \
    --env export_wfs_linear \
    --env export_wfs_map_reduce \
    --env export_wfs_wavefront \
    --env export_wfs_multi_sink \
    --env NUM_NODES_LIST \
    --env EDGERATIO_LIST \
    --env HOSTRATIO_LIST \
    --env POISSON_FREQ \
    --env POISSON_NUM_INTERESTS \
    --env POISSON_NUM_CONSUMERS \
    --env VISUALIZE \
    --bar run_category_task ::: $(seq 1 $NUM_RUNS) ::: $wf_topo_pairs

echo "All tasks complete."
