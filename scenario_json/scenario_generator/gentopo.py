#!/usr/bin/env python3

import graph

import random
import argparse
import pathlib
import os
import re
import random
import sys
import json
from itertools import combinations

def bottleneck(args):
    # Combines two existing topology JSON files by adding a single link between them.
    # Load the two topologies
    with open(args.topo1, 'r') as f:
        data1 = json.load(f)
    with open(args.topo2, 'r') as f:
        data2 = json.load(f)

    topo1 = graph.Topology.from_dict(data1)
    topo2 = graph.Topology.from_dict(data2)

    # Merge topo2 into topo1
    # Note: Ensure your graph library supports a merge or handles node name collisions
    topo1.merge(topo2) 

    # Select routers to connect
    r1 = args.rtr1 if args.rtr1 else random.choice(topo1.get_nodes())
    r2 = args.rtr2 if args.rtr2 else random.choice(topo2.get_nodes())

    # Create the bottleneck link
    topo1.add(r1, r2)
    topo1.update_edge_metadata(r1, r2, **{
        'capacity': args.bandwidth,
        'metric': args.metric,
        'delay': args.delay,
        'queue': args.queue,
    })

    return topo1

def star_of_stars(args):
    """
    Creates a hierarchical star topology.
    - 1 Core Router
    - N Aggregation Routers (branches from core)
    - Remaining routers distributed as Edge routers (branches from aggregation)
    """
    if args.num_sensors:
        args.sensor_list = [f"sensor{i}" for i in range(args.num_sensors)]

    if args.num_users:
        args.user_list = [f"user{i}" for i in range(args.num_users)]


    if args.num_routers < args.branches + 1:
        raise ValueError("Not enough routers to support the requested number of branches.")

    topo = graph.Topology()
    
    # 1. Define the hierarchy
    routers = [f"rtr{i}" for i in range(args.num_routers)]
    core = routers[0]
    agg_routers = routers[1 : args.branches + 1]
    edge_routers = routers[args.branches + 1 :]

    # 2. Connect Aggregation to Core
    for agg in agg_routers:
        topo.add(core, agg)

    # 3. Connect Edge routers to Aggregation (round-robin)
    for i, edge in enumerate(edge_routers):
        parent = agg_routers[i % len(agg_routers)]
        topo.add(parent, edge)

    # 4. Attach Users and Sensors to the Edge routers
    # (If no edge routers exist, attach to aggregation)
    leaf_parents = edge_routers if edge_routers else agg_routers
    
    for leaf_node in (args.user_list + args.sensor_list):
        parent = random.choice(leaf_parents)
        topo.add(parent, leaf_node)

    # Apply Metadata
    for node in topo.get_nodes():
        meta = {
            'comment': args.comment,
            'y': args.ypos,
            'x': args.xpos,
            'cs-size': args.cs_size,
        }
        if args.mpi:
            meta['mpi-partition'] = args.mpi
        topo.update_metadata(node, **meta)

    for n, m in topo.get_connections():
        topo.update_edge_metadata(n, m, **{
            'capacity': args.bandwidth,
            'metric': args.metric,
            'delay': args.delay,
            'queue': args.queue,
        })

    return topo

def mesh(args):
    """
    Generates a full or partial mesh.
    - Full: Every router connected to every other router.
    - Partial: Links generated based on a probability (args.prob).
    """
    if args.num_sensors:
        args.sensor_list = [f"sensor{i}" for i in range(args.num_sensors)]

    if args.num_users:
        args.user_list = [f"user{i}" for i in range(args.num_users)]


    topo = graph.Topology()
    routers = [f"rtr{i}" for i in range(args.num_routers)]
    
    # Ensure connectivity by starting with a spanning tree if it's a partial mesh
    if args.prob < 1.0:
        # Re-use your existing logic to ensure the graph isn't fragmented
        topo = gen_spanning_tree(routers, len(routers)-1, [], [])
    
    # Add remaining edges based on probability
    for u, v in combinations(routers, 2):
        if not topo.is_connected(u, v): # Skip if spanning tree already added it
            if random.random() < args.prob:
                topo.add(u, v)
    
    # Attach users/sensors randomly to any router
    for leaf in (args.user_list + args.sensor_list):
        topo.add(leaf, random.choice(routers))

    # Apply Metadata
    for node in topo.get_nodes():
        meta = {
            'comment': args.comment,
            'y': args.ypos,
            'x': args.xpos,
            'cs-size': args.cs_size,
        }
        if args.mpi:
            meta['mpi-partition'] = args.mpi
        topo.update_metadata(node, **meta)
        
    for n, m in topo.get_connections():
        topo.update_edge_metadata(n, m, **{
            'capacity': args.bandwidth,
            'metric': args.metric,
            'delay': args.delay,
            'queue': args.queue,
        })
        
    return topo


def verify_and_repair_connectivity(topo, all_nodes):
    """
    Runs a BFS to find isolated components and links them together.
    Guarantees the network is never disjoint.
    """
    # Build adjacency list
    adj = {n: set() for n in all_nodes}
    for u, v in topo.get_connections():
        if u in adj and v in adj:
            adj[u].add(v)
            adj[v].add(u)
    
    components = []
    visited = set()
    
    # Identify all disconnected islands
    for node in all_nodes:
        if node not in visited:
            comp = set()
            queue = [node]
            while queue:
                curr = queue.pop(0)
                if curr not in comp:
                    comp.add(curr)
                    visited.add(curr)
                    queue.extend(list(adj[curr] - comp))
            components.append(comp)
            
    # If there are multiple islands, bridge them together
    if len(components) > 1:
        # ---- FIX: Print to stderr instead of stdout ----
        print(f"  [!] Detected {len(components)} disjoint components. Repairing...", file=sys.stderr)
        
        for i in range(1, len(components)):
            node_a = random.choice(list(components[i-1]))
            node_b = random.choice(list(components[i]))
            topo.add(node_a, node_b)


def multi_tiered(args):
    """
    Generates an N-tiered hierarchy.
    args.tier_counts: list of integers representing nodes per tier [1, 3, 9]
    """

    '''
    if args.num_sensors:
        args.sensor_list = [f"sensor{i}" for i in range(args.num_sensors)]

    if args.num_users:
        args.user_list = [f"user{i}" for i in range(args.num_users)]
    '''


    '''
    # ignore sensors and users passed on the router list
    if args.router_list:
        for user in args.user_list:
            if user in args.router_list:
                args.router_list.remove(user)

        for sensor in args.sensor_list:
            if sensor in args.router_list:
                args.router_list.remove(sensor)
    else:
        args.router_list = [f"rtr{i}" for i in range(args.num_routers)]


    topo = graph.Topology()
    tier_nodes = []
    node_count = 0
    
    # 1. Create nodes for each tier
    for count in args.tier_counts:
        current_tier = [f"rtr_{node_count + i}" for i in range(count)]
        tier_nodes.append(current_tier)
        node_count += count

    # 2. Connect tiers (each node connects to one parent in the tier above)
    for i in range(1, len(tier_nodes)):
        prev_tier = tier_nodes[i-1]
        curr_tier = tier_nodes[i]
        for node in curr_tier:
            parent = random.choice(prev_tier)
            topo.add(parent, node)
            
    # 3. Attach leaf nodes to the bottom tier
    bottom_tier = tier_nodes[-1]
    for leaf in (args.user_list + args.sensor_list):
        topo.add(leaf, random.choice(bottom_tier))
    '''





    """
    Generates an N-tiered hierarchy.
    Scales the ratios provided in args.tier_counts to match args.num_routers.
    This generator allows connections between tier0 routers, and places users and sensors all on the bottom-most tier.
    """

    '''
    topo = graph.Topology()

    # 1. Distribute args.num_routers based on the ratios in args.tier_counts
    total_ratio = sum(args.tier_counts)
    tier_sizes = []
    routers_left = args.num_routers

    for i, ratio in enumerate(args.tier_counts):
        if i == len(args.tier_counts) - 1:
            # Last tier gets all remaining routers to avoid rounding loss
            tier_sizes.append(routers_left)
        else:
            # Calculate proportion, guarantee at least 1 node per tier
            size = max(1, int((ratio / total_ratio) * args.num_routers))
            tier_sizes.append(size)
            routers_left -= size

    # 2. Create nodes using standard naming (rtr0, rtr1...)
    routers = [f"rtr{i}" for i in range(args.num_routers)]
    tier_nodes = []
    current_idx = 0
    for size in tier_sizes:
        tier_nodes.append(routers[current_idx : current_idx + size])
        current_idx += size

    # 3. Connect the Core (Tier 0) fully so the network isn't partitioned
    core_nodes = tier_nodes[0]
    for i in range(len(core_nodes)):
        for j in range(i + 1, len(core_nodes)):
            topo.add(core_nodes[i], core_nodes[j])

    # 4. Connect each tier to the tier above it
    for i in range(1, len(tier_nodes)):
        prev_tier = tier_nodes[i-1]
        curr_tier = tier_nodes[i]
        for node in curr_tier:
            # Connect to at least one parent in the tier above
            parent = random.choice(prev_tier)
            topo.add(parent, node)
            
            # Optional: 50% chance to add a secondary uplink for realism/redundancy
            if len(prev_tier) > 1 and random.random() > 0.5:
                second_parent = random.choice([p for p in prev_tier if p != parent])
                topo.add(second_parent, node)

    # 5. Fulfill the remaining edges requested by args.num_edges
    existing_edges = set()
    for u, v in topo.get_connections():
        existing_edges.add(tuple(sorted((u, v))))

    all_possible = [tuple(sorted(c)) for c in combinations(routers, 2)]
    candidates = [e for e in all_possible if e not in existing_edges]

    extra_edges_needed = args.num_edges - len(existing_edges)
    if extra_edges_needed > 0:
        actual_extra = min(extra_edges_needed, len(candidates))
        for u, v in random.sample(candidates, actual_extra):
            topo.add(u, v)

    # 6. Attach leaf nodes (Sensors and Users) to the bottom tier
    bottom_tier = tier_nodes[-1]
    for leaf in (args.user_list + args.sensor_list):
        topo.add(leaf, random.choice(bottom_tier))

    # 7. Apply standard metadata
    for node in topo.get_nodes():
        meta = {
            'comment': args.comment,
            'y': random.randint(0, 1000),
            'x': random.randint(0, 1000),
        }
        if args.mpi:
            meta['mpi-partition'] = args.mpi
        topo.update_metadata(node, **meta)

    for n, m in topo.get_connections():
        topo.update_edge_metadata(n, m, **{
            'capacity': args.bandwidth,
            'metric': args.metric,
            'delay': args.delay,
            'queue': args.queue,
        })
    '''

    # 0. Initialize lists if counts were passed
    if args.num_sensors and not args.sensor_list:
        args.sensor_list = [f"sensor{i}" for i in range(args.num_sensors)]
    if args.num_users and not args.user_list:
        args.user_list = [f"user{i}" for i in range(args.num_users)]
    if not args.sensor_list: args.sensor_list = []
    if not args.user_list: args.user_list = []

    if args.num_routers < args.tiers:
        raise ValueError("Total number of routers must be at least the number of tiers.")

    topo = graph.Topology()

    # 1. Naturally Sort the Routers (rtr0, rtr1, ..., rtr10)
    if hasattr(args, 'router_list') and args.router_list:
        routers = [r for r in args.router_list if r not in args.user_list and r not in args.sensor_list]
    else:
        routers = [f"rtr{i}" for i in range(args.num_routers)]

    def natural_sort_key(s):
        return [int(text) if text.isdigit() else text.lower() for text in re.split(r'(\d+)', s)]
    
    routers.sort(key=natural_sort_key)
    args.num_routers = len(routers)

    # 2. Distribute routers using powers of 2 (1 : 2 : 4 : 8)
    weights = [2**i for i in range(args.tiers)]
    total_weight = sum(weights)
    
    tier_sizes = []
    routers_left = args.num_routers
    
    for i, w in enumerate(weights):
        if i == args.tiers - 1:
            tier_sizes.append(routers_left) # Lowest tier gets all remaining nodes
        else:
            size = max(1, int(round((w / total_weight) * args.num_routers)))
            tier_sizes.append(size)
            routers_left -= size

    # 3. Bucket nodes into tiers
    tiers = []
    current_idx = 0
    for size in tier_sizes:
        tiers.append(routers[current_idx : current_idx + size])
        current_idx += size

    # 4. STRICT CONNECTION RULES

    # Rule: Tier 1 connected to ALL nodes in Tier 0 (Highest Tier)
    if len(tiers) > 1:
        for node1 in tiers[1]:
            for node0 in tiers[0]:
                topo.add(node1, node0)

    # Rule: Tier 2 and lower connected to exactly 2 nodes in the tier above it
    if len(tiers) > 2:
        for i in range(2, len(tiers)):
            prev_tier = tiers[i-1]
            curr_tier = tiers[i]
            
            for node in curr_tier:
                # Fallback to 1 if the tier above somehow only has 1 node, otherwise strictly 2
                num_parents = min(2, len(prev_tier))
                parents = random.sample(prev_tier, num_parents)
                for p in parents:
                    topo.add(node, p)

    # Rule: Users and Sensors connected to exactly 1 node in the lowest tier
    bottom_tier = tiers[-1]
    
    for user in args.user_list:
        parent = random.choice(bottom_tier)
        topo.add(user, parent)
        
    for sensor in args.sensor_list:
        parent = random.choice(bottom_tier)
        topo.add(sensor, parent)

    # 5. Apply Visual Metadata
    for t_idx, tier in enumerate(tiers):
        for node in tier:
            meta = {
                'comment': args.comment,
                'y': (t_idx + 1) * 200,          # Visually layer the tiers perfectly
                'x': random.randint(100, 900)    
            }
            if args.mpi: meta['mpi-partition'] = args.mpi
            topo.update_metadata(node, **meta)

    for leaf in (args.user_list + args.sensor_list):
        meta = {
            'comment': args.comment,
            'y': (len(tiers) + 1) * 200,         # Visually drop sensors/users to the very bottom
            'x': random.randint(100, 900)
        }
        topo.update_metadata(leaf, **meta)

    # Apply Metadata
    for node in topo.get_nodes():
        meta = {
            'comment': args.comment,
            'y': args.ypos,
            'x': args.xpos,
            'cs-size': args.cs_size,
        }
        if args.mpi:
            meta['mpi-partition'] = args.mpi
        topo.update_metadata(node, **meta)
        
    for n, m in topo.get_connections():
        topo.update_edge_metadata(n, m, **{
            'capacity': args.bandwidth,
            'metric': args.metric,
            'delay': args.delay,
            'queue': args.queue,
        })

        
    return topo





def spanning_tree(args):
    if args.num_sensors:
        args.sensor_list = [f"sensor{i}" for i in range(args.num_sensors)]

    if args.num_users:
        args.user_list = [f"user{i}" for i in range(args.num_users)]

    # ignore sensors and users passed on the router list
    if args.router_list:
        for user in args.user_list:
            if user in args.router_list:
                args.router_list.remove(user)

        for sensor in args.sensor_list:
            if sensor in args.router_list:
                args.router_list.remove(sensor)
    else:
        args.router_list = [f"rtr{i}" for i in range(args.num_routers)]

    topo = gen_spanning_tree(args.router_list, args.num_edges, args.sensor_list, args.user_list)

    '''
    for node in topo.get_nodes():
        topo.update_metadata(node, **{
            'comment': args.comment,
            'y': args.ypos,
            'x': args.xpos,
        })
        if args.mpi:
            self.update_metadata(node, **{'mpi-partition': args.mpi})
    '''
    # Apply Metadata
    for node in topo.get_nodes():
        meta = {
            'comment': args.comment,
            'y': args.ypos,
            'x': args.xpos,
            'cs-size': args.cs_size,
        }
        if args.mpi:
            meta['mpi-partition'] = args.mpi
        topo.update_metadata(node, **meta)

        # TODO: override with random generation

    for n, m in topo.get_connections():
        topo.update_edge_metadata(n, m, **{
            'capacity': args.bandwidth,
            'metric': args.metric,
            'delay': args.delay,
            'queue': args.queue,
        })

    #    # TODO: override with random generation

    return topo


def gen_spanning_tree(routers, num_edges, sensors=["sensor"], users=["user"]):
    num_nodes = len(routers)

    if num_edges < num_nodes - 1:
        raise ValueError("A connected graph requires at least num_nodes - 1 edges.")
    if num_edges > num_nodes * (num_nodes - 1) // 2:
        raise ValueError("Too many edges for a simple undirected graph.")

    topo = graph.Topology()

    # create a random spanning tree to ensure connectivity
    random.shuffle(routers)
    connected = {routers[0]}
    remaining = set(routers[1:])

    while remaining:
        a = random.choice(list(connected))
        b = random.choice(list(remaining))
        topo.add(a, b)
        connected.add(b)
        remaining.remove(b)

    # add additional random edges until we reach num_edges
    # TODO: The all_possible_edges logic below is checking is_connected(u, v), which can be computationally expensive for large graphs. Swap this to a set-based lookup of existing edges.
    all_possible_edges = [(u, v) for u, v in combinations(routers, 2)
                          if not topo.is_connected(u, v)]

    extra_edges_needed = num_edges - (num_nodes - 1)
    if extra_edges_needed > 0:
        extra_edges = random.sample(all_possible_edges, extra_edges_needed)
        for u, v in extra_edges:
            topo.add(u, v)

    # have a single network uplink for users and sensors
    for user in users:
        user_link = random.choice(routers)
        topo.add(user_link, user)

    for sensor in sensors:
        sensor_link = random.choice(routers)
        topo.add(sensor, sensor_link)

    return topo


def main():
    '''
    parser = argparse.ArgumentParser("gentopo")
    parser.add_argument('-o', '--output-json', type=pathlib.Path, help="topology json file output")
    parser.add_argument('-t', '--output-txt', type=pathlib.Path, help="topology txt file output")
    parser.add_argument('-c', '--compact-output', action='store_true', default=False, help="print compact json")
    parser.set_defaults(algorithm=None)
    subparsers = parser.add_subparsers(title='algorithm', description='algorithm to use for graph generation')

    st_parser = subparsers.add_parser('spanning_tree', help="use spanning tree algorithm")
    st_parser.set_defaults(algorithm=spanning_tree)
    st_parser.add_argument('-e', '--num-edges', type=int, required=True, help='number of edges to generate')

    routers_group = st_parser.add_mutually_exclusive_group(required=True)
    routers_group.add_argument('-r', '--router-list', nargs='+', type=str, help='routers to use')
    routers_group.add_argument('-n', '--num-routers', type=int, help='number of routers to generate')

    sensors_group = st_parser.add_mutually_exclusive_group()
    sensors_group.add_argument('--sensor-list', nargs='+', type=str, default=['sensor'], help="list of sensor routers")
    sensors_group.add_argument('-s', '--num-sensors', type=int, help="number of sensor routers with standard names")

    users_group = st_parser.add_mutually_exclusive_group()
    users_group.add_argument('--user-list', nargs='+', type=str, default=['user'], help="list of user routers")
    users_group.add_argument('-u', '--num-users', type=int, help="number of user routers with standard names")

    st_parser.add_argument('--comment', type=str, default="NA", help='default router comment')
    st_parser.add_argument('--ypos', type=int, default=0, help='default router y-position')
    st_parser.add_argument('--xpos', type=int, default=0, help='default router x-position')
    st_parser.add_argument('--mpi', type=int, help='default router mpi partition')
    st_parser.add_argument('--bandwidth', type=str, default="10Mbps", help='default link bandwidth')
    st_parser.add_argument('--metric', type=str, default="1", help='default link metric')
    st_parser.add_argument('--delay', type=str, default="10ms", help='default link delay')
    st_parser.add_argument('--queue', type=str, default="100000", help='default link queue')
    # TODO: more options to generate metadata, such as mean and stddev

    bn_parser = subparsers.add_parser('bottleneck', help="combine two topologies via a bottleneck, topologies MUST NOT share routers")
    bn_parser.set_defaults(algorithm=bottleneck)
    bn_parser.add_argument('-1', '--topo1', type=pathlib.Path, help="json file of first topology to use")
    bn_parser.add_argument('-2', '--topo2', type=pathlib.Path, help="json file of second topology to use")
    bn_parser.add_argument('--rtr1', type=str, help="specify router for topology 1, otherwise choose randomly")
    bn_parser.add_argument('--rtr2', type=str, help="specify router for topology 2, otherwise choose randomly")
    bn_parser.add_argument('--bandwidth', type=str, default="1Mbps", help="bandwidth for bottleneck")
    bn_parser.add_argument('--metric', type=str, default="1", help='default link metric')
    bn_parser.add_argument('--delay', type=str, default="10ms", help='default link delay')
    bn_parser.add_argument('--queue', type=str, default="100000", help='default link queue')

    star_parser = subparsers.add_parser('star_of_stars', help="generate a hierarchical star topology")
    star_parser.set_defaults(algorithm=star_of_stars)
    #star_parser.add_argument('-n', '--num-routers', type=int, required=True, help='total number of routers')
    star_parser.add_argument('-b', '--branches', type=int, default=3, help='number of aggregation routers connected to the core')
    # Re-use our standard metadata/list arguments for consistency
    star_parser.add_argument('--sensor-list', nargs='+', default=[], help="list of sensors")
    star_parser.add_argument('--user-list', nargs='+', default=[], help="list of users")
    star_parser.add_argument('--bandwidth', type=str, default="1Gbps")

    mesh_parser = subparsers.add_parser('mesh', help="Full or Partial Mesh")
    mesh_parser.set_defaults(algorithm=mesh)
    #mesh_parser.add_argument('-n', '--num-routers', type=int, required=True)
    mesh_parser.add_argument('-p', '--prob', type=float, default=1.0, help="Probability of edge creation (1.0 = Full Mesh)")

    tier_parser = subparsers.add_parser('multi_tiered', help="Multi-tier Hierarchical")
    tier_parser.set_defaults(algorithm=multi_tiered)
    # Example: --tier-counts 1 2 4  (1 Core, 2 Distro, 4 Access)
    tier_parser.add_argument('--tier-counts', nargs='+', type=int, required=True)
    '''

    parser = argparse.ArgumentParser("gentopo")
    parser.add_argument('-o', '--output-json', type=pathlib.Path, help="topology json file output")
    parser.add_argument('-t', '--output-txt', type=pathlib.Path, help="topology txt file output")
    parser.add_argument('-j', '--compact-jsonoutput', action='store_true', default=False, help="print compact json")
    parser.set_defaults(algorithm=None)

    # --- Create a parent parser for shared arguments ---
    shared_parser = argparse.ArgumentParser(add_help=False)
    shared_parser.add_argument('-n', '--num-routers', type=int, help='number of routers')
    #shared_parser.add_argument('-e', '--num-edges', type=int, help='number of edges')
    shared_parser.add_argument('-s', '--num-sensors', type=int, help="number of sensors")
    shared_parser.add_argument('-u', '--num-users', type=int, help="number of users")
    shared_parser.add_argument('--sensor-list', nargs='+', default=[])
    shared_parser.add_argument('--user-list', nargs='+', default=[])
    shared_parser.add_argument('--router-list', nargs='+', help='explicit router names')
    
    # Common Metadata
    shared_parser.add_argument('--cs-size', type=int, default=0, help='content store size for all routers')
    shared_parser.add_argument('--comment', type=str, default="NA")
    shared_parser.add_argument('--ypos', type=int, default=0)
    shared_parser.add_argument('--xpos', type=int, default=0)
    shared_parser.add_argument('--mpi', type=int)
    shared_parser.add_argument('--bandwidth', type=str, default="10Mbps")
    shared_parser.add_argument('--metric', type=str, default="1")
    shared_parser.add_argument('--delay', type=str, default="10ms")
    shared_parser.add_argument('--queue', type=str, default="100000")

    subparsers = parser.add_subparsers(title='algorithm')

    # Spanning Tree
    st_parser = subparsers.add_parser('spanning_tree', parents=[shared_parser])
    st_parser.set_defaults(algorithm=spanning_tree)
    st_parser.add_argument('-e', '--num-edges', type=int, required=True, help='number of edges to generate')

    # Bottleneck
    bn_parser = subparsers.add_parser('bottleneck', parents=[shared_parser])
    bn_parser.set_defaults(algorithm=bottleneck)
    bn_parser.add_argument('-1', '--topo1', type=pathlib.Path, required=True)
    bn_parser.add_argument('-2', '--topo2', type=pathlib.Path, required=True)
    bn_parser.add_argument('--rtr1', type=str)
    bn_parser.add_argument('--rtr2', type=str)

    # Star of Stars
    star_parser = subparsers.add_parser('star_of_stars', parents=[shared_parser])
    star_parser.set_defaults(algorithm=star_of_stars)
    star_parser.add_argument('-b', '--branches', type=int, default=3)

    # Mesh
    mesh_parser = subparsers.add_parser('mesh', parents=[shared_parser])
    mesh_parser.set_defaults(algorithm=mesh)
    mesh_parser.add_argument('-p', '--prob', type=float, default=1.0)

    # Multi Tiered
    tier_parser = subparsers.add_parser('multi_tiered', parents=[shared_parser])
    tier_parser.set_defaults(algorithm=multi_tiered)
    tier_parser.add_argument('--tiers', type=int, required=True, help="Number of tiers (e.g., 3)")




    args = parser.parse_args()

    if not args.algorithm:
        print("must supply algorithm")
        parser.print_help()
        exit(1)

    if not (args.output_txt or args.output_json):
        print("supply either json or txt output")
        parser.print_help()
        exit(1)
    elif not args.output_txt:
        name, ext = os.path.splitext(args.output_json)
        args.output_txt = f"{name}.txt"
    elif not args.output_json:
        name, ext = os.path.splitext(args.output_txt)
        args.output_json = f"{name}.json"

    topo = args.algorithm(args)

    with open(args.output_json, "w") as f:
        f.write(json.dumps(topo.get_dict(), indent=2))

    with open(args.output_txt, "w") as f:
        f.write("# generated by gentopo.py\n\n")
        topo.write_txt(f)


if __name__ == '__main__':
    main()
