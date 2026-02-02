#!/usr/bin/env python3

import graph

import random
import argparse
from pathlib import Path
import os
import json
from itertools import combinations


def uniform(args):
    with open(args.workflow) as f:
        workflow = graph.Workflow.from_dict(json.load(f))

    with open(args.topology) as f:
        topology = graph.Topology.from_dict(json.load(f))

    hosting = gen_uniform_hosting(workflow, topology, args.sensors, args.users, args.min_hosts, args.max_hosts)

    return { "routerHosting": hosting }


def gen_uniform_hosting(workflow, topology, sensors=["sensor"], users=["user"], min_hosts=1, max_hosts=None):
    routers = topology.get_nodes()

    if min_hosts < 1:
        raise ValueError("min_hosts must be at least 1")
    if max_hosts is not None and min_hosts > max_hosts:
        raise ValueError("min_hosts cannot be greater than max_hosts")

    consumers = list(workflow.get_consumers())
    producers = list(workflow.get_producers())

    if len(consumers) != len(users):
        raise ValueError("length of users and consumers must match")

    if len(producers) != len(sensors):
        raise ValueError("length of users and consumers must match")

    hosting = []

    for user, consumer in zip(users, consumers):
        hosting.append({ "router": user, "service": consumer})
        routers.remove(user)

    for sensor, producer in zip(sensors, producers):
        hosting.append({ "router": sensor, "service": producer})
        routers.remove(sensor)

    if max_hosts is None or max_hosts > len(routers):
        max_hosts = len(routers) # cap at total available routers

    for service in workflow.get_services():
        num_hosts = random.randint(min_hosts, max_hosts)
        chosen_routers = random.sample(routers, num_hosts)

        for router in chosen_routers:
            hosting.append({ "router": router, "service": service })

    return hosting


def combine(args):
    hosting = []
    for filename in args.hosting:
        with open(filename) as f:
            hosting.extend(json.load(f)["routerHosting"])

    return { "routerHosting": hosting }


def main():
    parser = argparse.ArgumentParser("genhosting")
    parser.add_argument('-o', '--output', type=Path, default='/dev/stdout', help="hosting json file output")
    parser.set_defaults(algorithm=None)
    subparsers = parser.add_subparsers(title='algorithm', description='algorithm to use for hosting generation')

    uni_parser = subparsers.add_parser('uniform', help="distribute services uniformly")
    uni_parser.set_defaults(algorithm=uniform)
    uni_parser.add_argument('-t', '--topology', type=Path, help="topology json file input")
    uni_parser.add_argument('-w', '--workflow', type=Path, help="workflow json file input")
    uni_parser.add_argument('-s', '--sensors', nargs='+', type=str, default=['sensor'], help="list of sensor routers")
    uni_parser.add_argument('-u', '--users', nargs='+', type=str, default=['user'], help="list of user routers")
    uni_parser.add_argument('-n', '--min-hosts', type=int, default=1, help='min number of hosts per service')
    uni_parser.add_argument('-m', '--max-hosts', type=int, default=1, help='max number of hosts per service')

    comb_parser = subparsers.add_parser('combine', help="combine two hosting files")
    comb_parser.set_defaults(algorithm=combine)
    comb_parser.add_argument('-g', '--hosting', action='append', required=True, type=Path, help="topology json file input")

    args = parser.parse_args()

    if not args.algorithm:
        print("must supply algorithm")
        parser.print_help()
        exit(1)

    hosting = args.algorithm(args)

    with open(args.output, "w") as f:
        f.write(json.dumps(hosting))


if __name__ == '__main__':
    main()
