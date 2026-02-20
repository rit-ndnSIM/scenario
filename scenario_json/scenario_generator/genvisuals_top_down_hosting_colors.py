#!/usr/bin/env python3

import json
import sys
import os
import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

def draw_pie_node(ax, pos, colors, size=2000):
    """Helper to draw a single node as a pie chart if it has multiple colors."""
    if len(colors) <= 1:
        return colors[0] if colors else 'white'
    
    # Calculate the wedges
    n = len(colors)
    slices = [1/n] * n
    
    # We use a sub-axes or a pie plot at the coordinate
    # For simplicity in NetworkX, we can draw actual pie charts on top
    # of the existing axes.
    ax_pie = ax.inset_axes([pos[0]-0.02, pos[1]-0.02, 0.04, 0.04], transform=ax.transData)
    ax_pie.pie(slices, colors=colors, startangle=90)
    ax_pie.set_aspect('equal')
    ax_pie.axis('off')
    return 'none' # Tell nx.draw to not fill the node background

def visualize_graphs(json_path):
    if not os.path.exists(json_path): return
    with open(json_path, 'r') as f:
        data = json.load(f)

    base_name = os.path.splitext(json_path)[0]
    output_filename = f"{base_name}_network_viz.png"

    # --- 1. Topology & Color Map ---
    topo_g = nx.Graph()
    color_map_func = plt.get_cmap('tab20') 
    router_to_color = {}
    
    for i, rtr in enumerate(data.get('router', [])):
        node_name = rtr['node']
        topo_g.add_node(node_name)
        router_to_color[node_name] = mcolors.to_hex(color_map_func(i % 20))

    for link in data.get('link', []):
        topo_g.add_edge(link['from'], link['to'])

    # --- 2. Hosting Lookup (Multi-host support) ---
    service_hosts = {} # service -> list of colors
    for entry in data.get('routerHosting', []):
        srv = entry.get('service')
        rtr = entry.get('router')
        if srv and rtr in router_to_color:
            if srv not in service_hosts: service_hosts[srv] = []
            service_hosts[srv].append(router_to_color[rtr])

    # --- 3. DAG Setup ---
    dag_g = nx.DiGraph()
    dags = data.get('dag', {})
    dag_id = list(dags.keys())[0]
    for source, targets in dags[dag_id].items():
        for target, input_idx in targets.items():
            dag_g.add_edge(source, target, label=input_idx)

    # Hierarchical Layout Logic
    for node in dag_g.nodes():
        preds = nx.ancestors(dag_g, node)
        dag_g.nodes[node]['layer'] = max([nx.shortest_path_length(dag_g, r, node) 
                                         for r in [n for n in dag_g.nodes() if dag_g.in_degree(n)==0] 
                                         if nx.has_path(dag_g, r, node)] + [0])

    # --- 4. Plotting ---
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(24, 12))

    # Plot Topology
    pos_topo = nx.kamada_kawai_layout(topo_g)
    nx.draw(topo_g, pos_topo, ax=ax1, with_labels=True, 
            node_color=[router_to_color[n] for n in topo_g.nodes()], 
            node_size=2500, font_weight='bold', edge_color='gray')

    # Plot DAG
    pos_dag = nx.multipartite_layout(dag_g, subset_key="layer", align='horizontal')
    for node in pos_dag: pos_dag[node][1] = -pos_dag[node][1] # Flip for top-to-bottom

    # Draw edges first
    nx.draw_networkx_edges(dag_g, pos_dag, ax=ax2, arrowsize=20, edge_color='darkgray', connectionstyle="arc3,rad=0.1")
    nx.draw_networkx_labels(dag_g, pos_dag, ax=ax2, font_size=8)
    
    # Draw Nodes: If multi-hosted, draw a pie chart
    for node, (x, y) in pos_dag.items():
        colors = service_hosts.get(node, ['#FFFFFF'])
        if len(colors) > 1:
            # Draw pie for multi-host
            ax_pie = ax2.inset_axes([x-0.03, y-0.03, 0.06, 0.06], transform=ax2.transData)
            ax_pie.pie([1]*len(colors), colors=colors, wedgeprops={'linewidth': 0.5, 'edgecolor': 'white'})
            ax_pie.axis('off')
        else:
            # Draw standard circle for single host
            nx.draw_networkx_nodes(dag_g, pos_dag, nodelist=[node], ax=ax2, 
                                   node_color=colors[0], node_size=1500, 
                                   edgecolors='black', linewidths=0.5)

    edge_labels = nx.get_edge_attributes(dag_g, 'label')
    nx.draw_networkx_edge_labels(dag_g, pos_dag, edge_labels=edge_labels, font_size=8)

    plt.savefig(output_filename)
    print(f"Success! Output saved to: {output_filename}")

if __name__ == "__main__":
    if len(sys.argv) > 1: visualize_graphs(sys.argv[1])