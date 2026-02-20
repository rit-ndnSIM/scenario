#!/usr/bin/env python3

import json
import sys
import os
import networkx as nx
import matplotlib.pyplot as plt

def visualize_graphs(json_path):
    if not os.path.exists(json_path):
        print(f"Error: File '{json_path}' not found.")
        return

    try:
        with open(json_path, 'r') as f:
            data = json.load(f)
    except Exception as e:
        print(f"Error parsing JSON: {e}")
        return

    # Dynamic filename handling
    base_name = os.path.splitext(json_path)[0]
    output_filename = f"{base_name}_network_viz.png"

    # --- 1. Topology Graph ---
    topo_g = nx.Graph()
    topo_node_colors = []
    for rtr in data.get('router', []):
        node_name = rtr['node']
        topo_g.add_node(node_name)
        lower_name = node_name.lower()
        if lower_name.startswith("user"):
            topo_node_colors.append('skyblue')
        elif lower_name.startswith("sensor"):
            topo_node_colors.append('salmon')
        else:
            topo_node_colors.append('lightgray')

    for link in data.get('link', []):
        topo_g.add_edge(link['from'], link['to'])

    # --- 2. DAG Graph ---
    dag_g = nx.DiGraph()
    dags = data.get('dag', {})
    if not dags:
        print("No DAG found.")
        return
        
    dag_id = list(dags.keys())[0]
    dag_dict = dags[dag_id]
    
    for source, targets in dag_dict.items():
        for target, input_idx in targets.items():
            dag_g.add_edge(source, target, label=input_idx)

    # Assign layers for Top-to-Bottom layout
    # We find the longest path to each node to determine its vertical level
    for node in dag_g.nodes():
        # ancestors() returns all nodes that can reach this node
        predecessors = nx.ancestors(dag_g, node)
        if not predecessors:
            dag_g.nodes[node]['layer'] = 0
        else:
            # Longest path from any root to this node
            paths = [nx.shortest_path_length(dag_g, source=root, target=node) 
                     for root in [n for n in dag_g.nodes() if dag_g.in_degree(n) == 0]
                     if nx.has_path(dag_g, root, node)]
            dag_g.nodes[node]['layer'] = max(paths) if paths else 0

    dag_node_colors = []
    for node in dag_g.nodes():
        in_deg = dag_g.in_degree(node)
        out_deg = dag_g.out_degree(node)
        if in_deg == 0:
            dag_node_colors.append('salmon')    # Root: Red
        elif out_deg == 0:
            dag_node_colors.append('skyblue')   # Sink: Blue
        else:
            dag_node_colors.append('lightgreen') # Intermediate: Green

    # --- 3. Plotting ---
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(20, 10))

    # Plot Topology
    pos_topo = nx.kamada_kawai_layout(topo_g)
    nx.draw(topo_g, pos_topo, ax=ax1, with_labels=True, node_color=topo_node_colors, 
            node_size=2000, font_size=10, font_weight='bold', edge_color='gray')
    ax1.set_title(f"Network Topology\n(Blue: User | Red: Sensor)", pad=20)

    # Plot Hierarchical DAG
    # multipartite_layout uses the 'layer' attribute
    pos_dag = nx.multipartite_layout(dag_g, subset_key="layer", align='horizontal')
    
    # Flip the Y-axis so Layer 0 is at the top
    for node in pos_dag:
        pos_dag[node][1] = -pos_dag[node][1]

    # Use named colors ('gray', 'darkgray') to avoid the ValueError
    nx.draw(dag_g, pos_dag, ax=ax2, with_labels=True, node_color=dag_node_colors, 
            node_size=2000, font_size=8, arrowsize=25, edge_color='darkgray',
            connectionstyle="arc3,rad=0.1") 
    
    edge_labels = nx.get_edge_attributes(dag_g, 'label')
    nx.draw_networkx_edge_labels(dag_g, pos_dag, edge_labels=edge_labels, font_size=8)
    ax2.set_title(f"Top-to-Bottom Workflow\n(Red: Roots | Blue: Sinks)", pad=20)

    plt.tight_layout()
    plt.savefig(output_filename)
    print(f"Success! Graphs saved to {output_filename}")
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 script.py <scenario.json>")
    else:
        visualize_graphs(sys.argv[1])