#!/bin/sh

set -e

# Generate a single timestamp to be used for all files in this run
TIMESTAMP=$(date +%Y%m%d-%H%M%S)

workdir="$PWD/generated_scenarios"
mkdir -p "$workdir"

generate_wf_linear() {
    # generate linear workflow
    length="$1"
    # Create a zero-padded version of the length (e.g., 20 -> 020)
    padded_length=$(printf "%03d" "$length")
    output="${TIMESTAMP}-wf_linear-${padded_length}srv.json"
    ./genworkflow.py layered --num-services "$length" --num-layers "$((length+2))" --num-producers 1 --num-consumers 1 --output "$workdir/$output"
    #./genworkflow.py layered -n "$length" -l "$((length+2))" -p 1 -c 1 -o "$output"
    echo "$output"
}

generate_wf_parallel_1() {
    # generate parallel workflow with multiple sensors, intermediate services, into aggregator, into consumer
    width="$1"
    # Create a zero-padded version
    padded_width=$(printf "%03d" "$width")
    output="${TIMESTAMP}-wf_parallel-${padded_width}prod-${padded_width}srv.json"
    ./genworkflow.py layered --num-services 0 --num-layers 2 --aggregate --split --num-producers "$width" --num-consumers 1 --output "$workdir/$output"
    #./genworkflow.py layered -n 0 -l 2 -a -t -p "$width" -c 1 -o "$output"
    echo "$output"
}

generate_wf_parallel_2() {
    # generate parallel workflow with 1 sensor, intermediate services, into aggregator, into consumer
    width="$1"
    # Create a zero-padded version
    padded_width=$(printf "%03d" "$width")
    output="${TIMESTAMP}-wf_parallel-1prod-${padded_width}srv.json"
    ./genworkflow.py layered --num-services "$width" --num-layers 3 --aggregate --num-producers 1 --num-consumers 1 --output "$workdir/$output"
    echo "$output"
}

generate_wf_messy() {
    servs="$1"
    prods="$2"
    cons="$3"
    layers="$4"
    skips="$5"
    # Create zero-padded versions
    padded_servs=$(printf "%03d" "$servs")
    padded_prods=$(printf "%03d" "$prods")
    padded_cons=$(printf "%03d" "$cons")
    padded_layers=$(printf "%03d" "$layers")
    padded_skips=$(printf "%03d" "$skips")
    output="${TIMESTAMP}-wf_messy-${padded_servs}srv-${padded_prods}prod-${padded_cons}con-${padded_skips}skip-agg-${padded_layers}layer.json"
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
    # Create zero-padded versions
    padded_nodes=$(printf "%03d" "$nodes")
    padded_edges=$(printf "%03d" "$edges")
    padded_snsrs=$(printf "%03d" "$snsrs")
    padded_usrs=$(printf "%03d" "$usrs")
    padded_cs=$(printf "%06d" "$cs")
    output="${TIMESTAMP}-tp_st-${padded_nodes}rtr-${padded_snsrs}snsr-${padded_usrs}usr-${padded_edges}edge-${padded_cs}cs.json"
    ./gentopo.py -o "$workdir/$output" "$type" -n "$nodes" -e "$edges" -s "$snsrs" -u "$usrs" --cs-size "$cs"
    echo "$output"
}
generate_tp_mesh() {
    type="mesh"
    nodes="$1"
    snsrs="$2"
    usrs="$3"
    prob="$4"
    cs="$5"
    # Create zero-padded versions
    padded_nodes=$(printf "%03d" "$nodes")
    padded_snsrs=$(printf "%03d" "$snsrs")
    padded_usrs=$(printf "%03d" "$usrs")
    padded_cs=$(printf "%06d" "$cs")
    output="${TIMESTAMP}-tp_mesh-${padded_nodes}rtr-${padded_snsrs}snsr-${padded_usrs}usr-${padded_cs}cs.json"
    ./gentopo.py -o "$workdir/$output" "$type" -n "$nodes" -s "$snsrs" -u "$usrs" -p "$prob" --cs-size "$cs"
    echo "$output"
}
generate_tp_sos() {
    type="star_of_stars"
    nodes="$1"
    snsrs="$2"
    usrs="$3"
    branches="$4"
    cs="$5"
    # Create zero-padded versions
    padded_nodes=$(printf "%03d" "$nodes")
    padded_snsrs=$(printf "%03d" "$snsrs")
    padded_usrs=$(printf "%03d" "$usrs")
    padded_cs=$(printf "%06d" "$cs")
    output="${TIMESTAMP}-tp_sos-${padded_nodes}rtr-${padded_snsrs}snsr-${padded_usrs}usr-${padded_cs}cs.json"
    ./gentopo.py -o "$workdir/$output" "$type" -n "$nodes" -s "$snsrs" -u "$usrs" -b "$branches" --cs-size "$cs"
    echo "$output"
}
generate_tp_mt() {
    type="multi_tiered"
    nodes="$1"
    snsrs="$2"
    usrs="$3"
    tier_counts="$4"
    cs="$5"
    # Create zero-padded versions
    padded_nodes=$(printf "%03d" "$nodes")
    padded_snsrs=$(printf "%03d" "$snsrs")
    padded_usrs=$(printf "%03d" "$usrs")
    padded_cs=$(printf "%06d" "$cs")
    output="${TIMESTAMP}-tp_mt-${padded_nodes}rtr-${padded_snsrs}snsr-${padded_usrs}usr-${padded_cs}cs.json"
    ./gentopo.py -o "$workdir/$output" "$type" -n "$nodes" -s "$snsrs" -u "$usrs" --tiers "$tier_counts" --cs-size "$cs"
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
        --makespan-min "$makespanMin" \
        --makespan-max "$makespanMax"
    echo "$output"
}

#length=5
#wf="$(generate_wf_linear ${length})"

#width=18
#wf="$(generate_wf_parallel_1 ${width})"
#wf="$(generate_wf_parallel_2 ${width})"

# #run for services = [10,20,50,100]
# services=10
# #producers=$(((services/5)+1))
# producers=3
# consumers=1
# #layers=$(echo "scale=0; sqrt($number)" | bc -l)
# layers=6
# skips=5
# wf="$(generate_wf_messy ${services} ${producers} ${consumers} ${layers} ${skips})"

# #run for service2node = [5,2,1,0.5,0.2]
# service2node=5
# nodes=$((services/service2node))
# #pick edges to determine how connected the topology is. edgeratio = 0 means linear connections only, =1 means fully connected
# #run for edgeratio = [0,0.2,0.4,0.6,1]
# edgeratio=0
# edges=$(( (nodes-1)*(edgeratio*(nodes-2)+2)/2 ))
# #edges=$((nodes-1))
# #edges=$(( ((nodes*( nodes-1 ))/2 + (nodes-1))  / 2  ))
# #edges=$(( (nodes*( nodes-1 ))/2  ))
# sensors=1
# users=1
# cs_size=0
# tp="$(generate_tp_st ${nodes} ${edges} ${sensors} ${users} ${cs_size})"

# makespanMin=0
# makespanMax=0
# hs="$(generate_hs "$wf" "$tp" ${sensors} ${users} ${makespanMin} ${makespanMax})"

# # Clean the hs name for the final scenario folder
# hs_clean=${hs#*-}
# hs_clean=${hs_clean#*-}

# prefixes="nescoSCOPT orchA orchB"

# for prefix in $prefixes; do
#     echo "Generating scenario for prefix: $prefix"

#     output_filename="$workdir/${TIMESTAMP}-sn-${prefix}-${hs_clean#hs-}"
#     ./build_scenario.py -f \
#         --workflow "$workdir/$wf" \
#         --topo-json "$workdir/$tp" \
#         --topo-txt "$workdir/${tp%.json}.txt" \
#         --hosting "$workdir/$hs" \
#         --output  ${output_filename} \
#         --prefix ${prefix} \
#         --serviceDiscovery 0 \
#         --resourceAllocation 0 \
#         --allocationReuse 0 \
#         --scheduleCompaction 0 \
#         --startTimeOffsetSD 0 \
#         --startTimeOffsetWF 0 \
#         --simulationEndTime 200


#     cp $workdir/${TIMESTAMP}-sn-${prefix}-${hs_clean#hs-} ../cascon_cpm_random/
# done

# #./genvisuals_circle.py ${output_filename}
# #./genvisuals_top_down.py ${output_filename}
# ./genvisuals_top_down_hosting_colors.py ${output_filename}


# Define specific pairs as "workflow:topology"
wf_topo_pairs="linear:multi_tiered map_reduce:mesh wavefront:mesh map_reduce:star_of_stars"
#wf_topo_pairs="map_reduce:mesh"

# Define the sweep arrays
#workflow_categories="linear map_reduce wavefront multi_sink"
#topology_categories="multi_tiered mesh star_of_stars spanning_tree"
services_list="10 20 50 100"
service2node_list="5 2 1 0.5 0.2"
#edgeratio_list="0 0.2 0.4 0.6 1"
edgeratio_list="0.5" # only used in spanning_tree

##workflow_categories="linear"
##topology_categories="star_of_stars"
#services_list="10"
#service2node_list="5"
#edgeratio_list="0.5"


#for workflowCategory in $workflow_categories; do

#for topoCategory in $topology_categories; do


for wf_topo_pair in $wf_topo_pairs; do

    # split the pair into individual variables
    workflowCategory=${wf_topo_pair%%:*}
    topoCategory=${wf_topo_pair#*:}

    for services in $services_list; do

        for service2node in $service2node_list; do

            for edgeratio in $edgeratio_list; do
                
                echo "--- Running Generation: TopoCat=$topoCategory, WorkflowCat=$workflowCategory, Services=$services, S2N=$service2node, EdgeRatio=$edgeratio ---"


                # 1. Workflow Generation

                case $workflowCategory in
                    "linear")
                        producers=1
                        consumers=1
                        layers=$services # Each service is a layer
                        skips=0
                        echo "---           Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                        wf="$(generate_wf_messy ${services} ${producers} ${consumers} ${layers} ${skips})"
                        #length=$services
                        #wf="$(generate_wf_linear ${length})"
                        ;;
                    "map_reduce")
                        # 1/6 of the services will be producers
                        producers=$(echo "scale=0; $services / 6" | bc -l)
                        if [ "$producers" -lt 1 ]; then producers=1; fi
                        consumers=1
                        layers=3         # Standard Map-Shuffle-Reduce
                        skips=0
                        echo "---           Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                        wf="$(generate_wf_messy ${services} ${producers} ${consumers} ${layers} ${skips})"
                        ;;
                    "wavefront")
                        # square root of the services will be producers
                        producers=$(echo "scale=0; sqrt($services)" | bc -l)
                        if [ "$producers" -lt 1 ]; then producers=1; fi
                        consumers=1
                        layers=$producers
                        skips=$(echo "scale=0; $layers / 3" | bc -l)
                        if [ "$skips" -lt 1 ]; then skips=1; fi
                        echo "---           Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                        wf="$(generate_wf_messy ${services} ${producers} ${consumers} ${layers} ${skips})"
                        ;;
                    "multi_sink")
                        producers=2
                        consumers=4
                        layers=4
                        layers=$(echo "scale=0; $services / 3" | bc -l)
                        if [ "$layers" -lt 1 ]; then layers=1; fi
                        skips=2
                        echo "---           Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                        wf="$(generate_wf_messy ${services} ${producers} ${consumers} ${layers} ${skips})"
                        ;;
                    *) # Default fallback
                        echo "--- ERROR, workflowCategory $workflowCategory not found. Skipping... ---"
                        continue
                        ;;
                esac



                # 2. Topology Generation

                case $topoCategory in
                    "multi_tiered")
                        # TODO: change these values so that we actually generate multi-tiered topo that makes sense
                        # Use 'bc' for floating point division and piping to 'read' to handle the result
                        nodes=$(echo "scale=0; $services / $service2node" | bc -l)
                        # Ensure nodes is at least 1 to avoid math errors
                        if [ "$nodes" -lt 1 ]; then nodes=1; fi
                        # we use the same number of sensor topology nodes as we have producer services in the workflow. Producer services are randomly distributed onto sensors nodes.
                        sensors=${producers}
                        users=1
                        tiers=$(echo "scale=0; $nodes / 6" | bc -l)
                        cs_size=0
                        if [ "$tiers" -lt 2 ]; then tiers=2; fi
                        echo "---           Topology: Multi-Tiered, nodes=$nodes, sensors=$sensors, users=$users, tiers=$tiers ---"
                        tp="$(generate_tp_mt ${nodes} ${sensors} ${users} ${tiers} ${cs_size})"
                        ;;
                    "mesh")
                        # Use 'bc' for floating point division and piping to 'read' to handle the result
                        nodes=$(echo "scale=0; $services / $service2node" | bc -l)
                        # Ensure nodes is at least 1 to avoid math errors
                        if [ "$nodes" -lt 1 ]; then nodes=1; fi
                        # we use the same number of sensor topology nodes as we have producer services in the workflow. Producer services are randomly distributed onto sensors nodes.
                        sensors=${producers}
                        users=1
                        prob=0.1
                        cs_size=0
                        echo "---           Topology: Mesh, nodes=$nodes, sensors=$sensors, users=$users ---"
                        tp="$(generate_tp_mesh ${nodes} ${sensors} ${users} ${prob} ${cs_size})"
                        ;;
                    "star_of_stars")
                        # Use 'bc' for floating point division and piping to 'read' to handle the result
                        nodes=$(echo "scale=0; $services / $service2node" | bc -l)
                        # Ensure nodes is at least 1 to avoid math errors
                        if [ "$nodes" -lt 1 ]; then nodes=1; fi
                        # we use the same number of sensor topology nodes as we have producer services in the workflow. Producer services are randomly distributed onto sensors nodes.
                        sensors=${producers}
                        users=1
                        #branches=3
                        branches=$(echo "scale=0; $nodes / 6" | bc -l)
                        if [ "$branches" -lt 1 ]; then branches=1; fi
                        cs_size=0
                        echo "---           Topology: Star-of-Stars, nodes=$nodes, sensors=$sensors, users=$users ---"
                        tp="$(generate_tp_sos ${nodes} ${sensors} ${users} ${branches} ${cs_size})"
                        ;;
                    "spanning_tree")
                        # Use 'bc' for floating point division and piping to 'read' to handle the result
                        nodes=$(echo "scale=0; $services / $service2node" | bc -l)
                        # Ensure nodes is at least 1 to avoid math errors
                        if [ "$nodes" -lt 1 ]; then nodes=1; fi
                        # Calculate edges using floating point math for the ratio
                        # Formula: edges = ((nodes-1)*(edgeratio*(nodes-2)+2)/2)
                        edges=$(echo "scale=0; ($nodes-1)*($edgeratio*($nodes-2)+2)/2" | bc -l)
                        # we use the same number of sensor topology nodes as we have producer services in the workflow. Producer services are randomly distributed onto sensors nodes.
                        sensors=${producers}
                        users=1
                        cs_size=0
                        echo "---           Topology: Spanning Tree, nodes=$nodes, edges=$edges, sensors=$sensors, users=$users ---"
                        tp="$(generate_tp_st ${nodes} ${edges} ${sensors} ${users} ${cs_size})"
                        ;;
                    *) # Default fallback
                        echo "--- ERROR, workflowCategory $workflowCategory not found. Skipping... ---"
                        continue
                        ;;
                esac


                # 3. Hosting Selection

                makespanMin=0
                makespanMax=0
                hs="$(generate_hs "$wf" "$tp" ${sensors} ${users} ${makespanMin} ${makespanMax})"

                # Clean the hs name
                hs_clean=${hs#*-}
                hs_clean=${hs_clean#*-}

                # 4. Scenario Building Loop

                prefixes="nescoSCOPT orchA orchB"
                for prefix in $prefixes; do
                    output_filename="$workdir/${TIMESTAMP}-sn-${topoCategory}-${workflowCategory}-${prefix}-${hs_clean#hs-}"
                    
                    ./build_scenario.py -f \
                        --workflow "$workdir/$wf" \
                        --topo-json "$workdir/$tp" \
                        --topo-txt "$workdir/${tp%.json}.txt" \
                        --hosting "$workdir/$hs" \
                        --output "${output_filename}" \
                        --prefix ${prefix} \
                        --serviceDiscovery 0 \
                        --resourceAllocation 0 \
                        --allocationReuse 0 \
                        --scheduleCompaction 0 \
                        --startTimeOffsetSD 0 \
                        --startTimeOffsetWF 0 \
                        --simulationEndTime 200

                    cp "${output_filename}" ../cascon_cpm_random/
                done

                # 5. Visualization
                if [ "$nodes" -lt 20 ] && [ "$services" -lt 20 ]; then
                    ./genvisuals_top_down_hosting_colors.py "${output_filename}"
                    ./genvisuals_top_down_hosting_colors_hierarchical-topo.py "${output_filename}"
                else
                    echo "--- Skipping visualization: too many nodes ($nodes) or services ($services). We don't visualize if more than 20 ---"
                fi

                # 6. Delay to ensure file system stability and separation
                echo "Iteration complete."
                #echo "Sleeping for 1 second..."
                #sleep 1

            done
        done
    done
done
