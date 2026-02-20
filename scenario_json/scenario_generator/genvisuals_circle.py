#!/usr/bin/env python3

import json
import sys
import os
import networkx as nx
import matplotlib.pyplot as plt

def visualize_graphs(json_path):
    # --- File Handling ---
    if not os.path.exists(json_path):
        print(f"Error: File '{json_path}' not found.")
        return

    try:
        with open(json_path, 'r') as f:
            data = json.load(f)
    except Exception as e:
        print(f"Error parsing JSON: {e}")
        return

    # Generate output filename: "input_name_network_viz.png"
    base_name = os.path.splitext(json_path)[0]
    output_filename = f"{base_name}_network_viz.png"

    # --- 1. Create Topology Graph ---
    topo_g = nx.Graph()
    topo_node_colors = []
    
    for rtr in data.get('router', []):
        node_name = rtr['node']
        topo_g.add_node(node_name)
        
        # Color logic: user -> blue, sensor -> red
        lower_name = node_name.lower()
        if lower_name.startswith("user"):
            topo_node_colors.append('skyblue')
        elif lower_name.startswith("sensor"):
            topo_node_colors.append('salmon')
        else:
            topo_node_colors.append('lightgrey')

    for link in data.get('link', []):
        topo_g.add_edge(link['from'], link['to'])

    # --- 2. Create DAG Graph ---
    dag_g = nx.DiGraph()
    dags = data.get('dag', {})
    if not dags:
        print("No DAG structure found in JSON.")
        return
        
    # Get the first DAG available (e.g., 'dag1')
    first_dag_id = list(dags.keys())[0]
    dag_dict = dags[first_dag_id]
    
    for source, targets in dag_dict.items():
        for target, input_idx in targets.items():
            dag_g.add_edge(source, target, label=input_idx)

    # Color Logic: Root (Red), Sink (Blue), Intermediate (Green)
    dag_node_colors = []
    for node in dag_g.nodes():
        in_deg = dag_g.in_degree(node)
        out_deg = dag_g.out_degree(node)
        
        if in_deg == 0:
            dag_node_colors.append('salmon')    # Root
        elif out_deg == 0:
            dag_node_colors.append('skyblue')   # Sink
        else:
            dag_node_colors.append('lightgreen') # Intermediate

    # --- 3. Plotting ---
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(18, 9))

    # Topology Plot
    pos_topo = nx.kamada_kawai_layout(topo_g)
    nx.draw(topo_g, pos_topo, ax=ax1, with_labels=True, node_color=topo_node_colors, 
            node_size=2500, font_size=10, font_weight='bold', edge_color='gray')
    ax1.set_title(f"Topology: {os.path.basename(json_path)}\n(Blue: User | Red: Sensor)", pad=20)

    # DAG Plot
    # Using shell layout to help visualize the flow from roots to sinks
    pos_dag = nx.shell_layout(dag_g)
    nx.draw(dag_g, pos_dag, ax=ax2, with_labels=True, node_color=dag_node_colors, 
            node_size=2000, font_size=8, arrowsize=25, edge_color='#cccccc')
    
    # Edge labels for input sequence numbers
    edge_labels = nx.get_edge_attributes(dag_g, 'label')
    nx.draw_networkx_edge_labels(dag_g, pos_dag, edge_labels=edge_labels, font_size=8)
    ax2.set_title(f"DAG: {first_dag_id}\n(Red: Root | Blue: Sink | Green: Intermediate)", pad=20)

    # Final Save
    plt.tight_layout()
    plt.savefig(output_filename)
    print(f"Success! Output saved to: {output_filename}")
    
    # Optional: Close plot to free up memory if running in a loop
    plt.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 script.py <scenario.json>")
    else:
        visualize_graphs(sys.argv[1])

