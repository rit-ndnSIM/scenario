#!/usr/bin/env python3

import json
import sys
import os
import re
import networkx as nx
import matplotlib
matplotlib.use('Agg') 
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import numpy as np

def get_router_num(name):
    match = re.search(r'\d+', name)
    return int(match.group()) if match else 0

def get_ordered_fabric_layers(G):
    """
    Determines tiers based on naming and connectivity:
    1. Tier 0: Routers with the smallest numbers.
    2. Endpoints: Users/Sensors at the bottom.
    3. Intermediate: Grouped by their parent-connectivity.
    """
    layers = {}
    endpoints = [n for n in G.nodes() if n.lower().startswith(('user', 'sensor'))]
    routers = sorted([n for n in G.nodes() if n not in endpoints], key=get_router_num)
    
    if not routers: return {}

    # Tier 0 are routers that don't have a 'parent' with a smaller number
    # Or more simply, routers connected to Tier 1 routers.
    # Given your rule: Tier 0 is the 'top'
    
    # We'll use a BFS approach starting from Tier 0 (rtr0, etc.)
    # but respecting your naming rule.
    current_tier_nodes = [routers[0]]
    # Find all nodes connected to routers[0] that aren't endpoints
    # In your rules, rtr0 connects to all Tier 1.
    
    # Let's use shortest path distance from rtr0 to establish tiers
    for node in G.nodes():
        if node in endpoints:
            layers[node] = 999 # Placeholder for bottom
        else:
            try:
                # Distance from rtr0 defines the Tier index
                layers[node] = nx.shortest_path_length(G, routers[0], node)
            except:
                layers[node] = 0

    max_rtr_layer = max([v for k, v in layers.items() if v != 999] or [0])
    for n in layers:
        if layers[n] == 999:
            layers[n] = max_rtr_layer + 1
            
    return layers

def create_balanced_layout(G, layer_dict):
    layer_groups = {}
    for n, l in layer_dict.items():
        layer_groups.setdefault(l, []).append(n)
    
    pos = {}
    for l, nodes in layer_groups.items():
        # Sort nodes by name so rtr2 is left of rtr3, etc.
        nodes = sorted(nodes, key=get_router_num)
        n_nodes = len(nodes)
        x_coords = np.linspace(-2, 2, n_nodes) if n_nodes > 1 else [0]
        for i, node in enumerate(nodes):
            pos[node] = np.array([x_coords[i], -l])
    return pos

def visualize_graphs(json_path):
    if not os.path.exists(json_path): return
    with open(json_path, 'r') as f:
        data = json.load(f)

    base_name = os.path.splitext(json_path)[0]
    output_filename = f"{base_name}_network_viz.png"

    # --- 1. Topology ---
    topo_g = nx.Graph()
    for rtr in data.get('router', []):
        topo_g.add_node(rtr['node'])
    for link in data.get('link', []):
        topo_g.add_edge(link['from'], link['to'])

    topo_layers = get_ordered_fabric_layers(topo_g)
    color_map_func = plt.get_cmap('tab20')
    router_to_color = {n: mcolors.to_hex(color_map_func(i % 20)) for i, n in enumerate(topo_g.nodes())}

    # --- 2. DAG ---
    service_hosts = {}
    for entry in data.get('routerHosting', []):
        srv, rtr = entry.get('service'), entry.get('router')
        if srv and rtr in router_to_color:
            service_hosts.setdefault(srv, []).append(router_to_color[rtr])

    dag_g = nx.DiGraph()
    dags = data.get('dag', {})
    if dags:
        dag_id = list(dags.keys())[0]
        for s, targets in dags[dag_id].items():
            for t, idx in targets.items():
                dag_g.add_edge(s, t, label=idx)

    dag_layers = {}
    for node in dag_g.nodes():
        roots = [n for n in dag_g.nodes() if dag_g.in_degree(n) == 0]
        paths = [nx.shortest_path_length(dag_g, r, node) for r in roots if nx.has_path(dag_g, r, node)]
        dag_layers[node] = max(paths) if paths else 0

    # --- 3. Plotting ---
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(28, 14))

    # Topology
    pos_topo = create_balanced_layout(topo_g, topo_layers)
    nx.draw(topo_g, pos_topo, ax=ax1, with_labels=True, 
            node_color=[router_to_color[n] for n in topo_g.nodes()], 
            node_size=2400, font_size=9, font_weight='bold', edge_color='silver')
    ax1.set_title("Tiered Topology (Strict Name & Link Rules)", fontsize=16)

    # DAG
    pos_dag = create_balanced_layout(dag_g, dag_layers)
    nx.draw_networkx_edges(dag_g, pos_dag, ax=ax2, arrowsize=20, edge_color='gray', connectionstyle="arc3,rad=0.1")
    nx.draw_networkx_labels(dag_g, pos_dag, ax=ax2, font_size=8)
    
    for node, (x, y) in pos_dag.items():
        colors = service_hosts.get(node, ['#FFFFFF'])
        if len(colors) > 1:
            ax_p = ax2.inset_axes([x-0.035, y-0.035, 0.07, 0.07], transform=ax2.transData)
            ax_p.pie([1]*len(colors), colors=colors, wedgeprops={'edgecolor': 'white', 'linewidth': 0.5})
            ax_p.axis('off')
        else:
            nx.draw_networkx_nodes(dag_g, pos_dag, nodelist=[node], ax=ax2, 
                                   node_color=colors[0], node_size=1700, edgecolors='black')

    edge_labels = nx.get_edge_attributes(dag_g, 'label')
    nx.draw_networkx_edge_labels(dag_g, pos_dag, edge_labels=edge_labels, font_size=7)
    ax2.set_title("Service Workflow (Hosting Colors)", fontsize=16)

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300)
    plt.close(fig)
    print(f"            Final visualization generated: {output_filename}")

if __name__ == "__main__":
    if len(sys.argv) > 1: visualize_graphs(sys.argv[1])