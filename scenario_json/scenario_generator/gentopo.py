#!/usr/bin/env python3

import graph

import random
import argparse
import pathlib
import os
import json
from itertools import combinations

def bottleneck(args):
    # TODO
    raise "not implemented :("


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
            if user in args.router_list:
                args.router_list.remove(user)
    else:
        args.router_list = [f"rtr{i}" for i in range(args.num_routers)]

    topo = gen_spanning_tree(args.router_list, args.num_edges, args.sensor_list, args.user_list)

    for node in topo.get_nodes():
        topo.update_metadata(node, **{
            'comment': args.comment,
            'y': args.ypos,
            'x': args.xpos,
        })
        if args.mpi:
            self.update_metadata(node, **{'mpi-partition': args.mpi})

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
    st_parser.add_argument('--metric', type=int, default=1, help='default link metric')
    st_parser.add_argument('--delay', type=str, default="10ms", help='default link delay')
    st_parser.add_argument('--queue', type=int, default=100, help='default link queue')
    # TODO: more options to generate metadata, such as mean and stddev

    bn_parser = subparsers.add_parser('bottleneck', help="combine two topologies via a bottleneck, topologies MUST NOT share routers")
    bn_parser.set_defaults(algorithm=bottleneck)
    bn_parser.add_argument('-1', '--topo1', type=pathlib.Path, help="json file of first topology to use")
    bn_parser.add_argument('-2', '--topo2', type=pathlib.Path, help="json file of second topology to use")
    bn_parser.add_argument('--rtr1', type=str, help="specify router for topology 1, otherwise choose randomly")
    bn_parser.add_argument('--rtr2', type=str, help="specify router for topology 2, otherwise choose randomly")
    bn_parser.add_argument('--bandwidth', type=str, default="1Mbps", help="bandwidth for bottleneck")

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
        f.write(json.dumps(topo.get_dict()))

    with open(args.output_txt, "w") as f:
        f.write("# generated by gentopo.py\n\n")
        topo.write_txt(f)


if __name__ == '__main__':
    main()
