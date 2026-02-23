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
    type="$1"
    nodes="$2"
    edges="$3"
    snsrs="$4"
    usrs="$5"
    output="${TIMESTAMP}-tp-${nodes}rtr-${snsrs}snsr-${usrs}usr-${edges}edge.json"
    ./gentopo.py -o "$workdir/$output" "$type" -n "$nodes" -e "$edges" -s "$snsrs" -u "$usrs"
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

#length=5
#wf="$(generate_linear_wf ${length})"

#width=18
#wf="$(generate_parallel_wf_1 ${width})"
#wf="$(generate_parallel_wf_2 ${width})"

# #run for services = [10,20,50,100]
# services=10
# #producers=$(((services/5)+1))
# producers=3
# consumers=1
# #layers=$(echo "scale=0; sqrt($number)" | bc -l)
# layers=6
# skips=5
# wf="$(generate_messy_wf ${services} ${producers} ${consumers} ${layers} ${skips})"

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
# tp="$(generate_tp ${nodes} ${edges} ${sensors} ${users})"

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


# Define the sweep arrays
#topology_categories="multi_tiered mesh star_of_stars"
#workflow_categories="linear map_reduce wavefront multi_sink"
#services_list="10 20 50 100"
#service2node_list="5 2 1 0.5 0.2"
#edgeratio_list="0 0.2 0.4 0.6 1"

topology_categories="spanning_tree"
workflow_categories="wavefront"
services_list="20"
service2node_list="5"
edgeratio_list="0.4"




for topoCategory in $topology_categories; do

    for workflowCategory in $workflow_categories; do

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
                            wf="$(generate_messy_wf ${services} ${producers} ${consumers} ${layers} ${skips})"
                            #length=$services
                            #wf="$(generate_linear_wf ${length})"
                            ;;
                        "map_reduce")
                            # 1/6 of the services will be producers
                            producers=$(echo "scale=0; $services / 6" | bc -l)
                            if [ "$producers" -lt 1 ]; then producers=1; fi
                            consumers=1
                            layers=3         # Standard Map-Shuffle-Reduce
                            skips=0
                            echo "---           Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                            wf="$(generate_messy_wf ${services} ${producers} ${consumers} ${layers} ${skips})"
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
                            wf="$(generate_messy_wf ${services} ${producers} ${consumers} ${layers} ${skips})"
                            ;;
                        "multi_sink")
                            producers=2
                            consumers=4
                            layers=4
                            layers=$(echo "scale=0; $services / 3" | bc -l)
                            if [ "$layers" -lt 1 ]; then layers=1; fi
                            skips=2
                            echo "---           Workflow: producers=$producers, consumers=$consumers, layers=$layers, skips=$skips ---"
                            wf="$(generate_messy_wf ${services} ${producers} ${consumers} ${layers} ${skips})"
                            ;;
                        *) # Default fallback
                            echo "--- ERROR, workflowCategory $workflowCategory not found. Skipping... ---"
                            continue
                            ;;
                    esac



                    # 2. Topology Generation

                    case $topoCategory in
                        "multi_tiered")
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
                            echo "---           Topology: nodes=$nodes, edges=$edges, sensors=$sensors, users=$users ---"
                            tp="$(generate_tp spanning_tree ${nodes} ${edges} ${sensors} ${users})"
                            ;;
                        "mesh")
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
                            echo "---           Topology: nodes=$nodes, edges=$edges, sensors=$sensors, users=$users ---"
                            tp="$(generate_tp spanning_tree ${nodes} ${edges} ${sensors} ${users})"
                            ;;
                        "star_of_stars")
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
                            echo "---           Topology: nodes=$nodes, edges=$edges, sensors=$sensors, users=$users ---"
                            tp="$(generate_tp spanning_tree ${nodes} ${edges} ${sensors} ${users})"
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
                            echo "---           Topology: nodes=$nodes, edges=$edges, sensors=$sensors, users=$users ---"
                            tp="$(generate_tp spanning_tree ${nodes} ${edges} ${sensors} ${users})"
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

                        #cp "${output_filename}" ../cascon_cpm_random/
                    done

                    # 5. Visualization
                    ./genvisuals_top_down_hosting_colors.py "${output_filename}"

                    # 6. Delay to ensure file system stability and separation
                    echo "Iteration complete. Sleeping for 1 second..."
                    sleep 1

                done
            done
        done
    done
done
