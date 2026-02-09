#!/usr/bin/env python3

import graph

import random
import argparse
from pathlib import Path
import os
import json
from itertools import combinations


def uniform(args):
    if args.num_sensors:
        args.sensor_list = [f"sensor{i}" for i in range(args.num_sensors)]

    if args.num_users:
        args.user_list = [f"user{i}" for i in range(args.num_users)]

    with open(args.workflow) as f:
        workflow = graph.Workflow.from_dict(json.load(f))

    with open(args.topology) as f:
        topology = graph.Topology.from_dict(json.load(f))

    if len(args.start_times) != len(args.stop_times):
        raise Exception("length start times and stop times must match")

    hosting = gen_uniform_hosting(workflow, topology, args.sensor_list, args.user_list, args.min_hosts, args.max_hosts)
    services = workflow.get_services()
    consumers = workflow.get_consumers()
    producers = workflow.get_producers()

    for item in hosting:
        if item['service'] in services:
            start, stop = random.choice(tuple(zip(args.start_times, args.stop_times)))
            item.update({'start': start, 'end': stop})
        elif item['service'] in consumers:
            # TODO: this may need to be configurable
            item.update({"workflowFile": str(args.workflow), "dag": "dag1", "start": 0, "end": -1 })
        elif item['service'] in producers:
            item.update({"start": 0, "end": -1 })

    return { "routerHosting": hosting }


def gen_uniform_hosting(workflow, topology, sensors=["sensor"], users=["user"], min_hosts=1, max_hosts=None):
    routers = topology.get_nodes()

    if not all(node in routers for node in (*sensors, *users)):
        raise ValueError("all sensors and users must be in the topology")

    if min_hosts < 1:
        raise ValueError("min_hosts must be at least 1")
    if max_hosts is not None and min_hosts > max_hosts:
        raise ValueError("min_hosts cannot be greater than max_hosts")

    consumers = list(workflow.get_consumers())
    producers = list(workflow.get_producers())

    if len(consumers) < len(users):
        raise ValueError("cannot have more users than consumers")

    if len(producers) < len(sensors):
        raise ValueError("cannot have more sensors than producers")

    hosting = []

    for user in users:
        hosting.append({ "router": user, "service": consumers.pop()})

    for consumer in consumers:
        hosting.append({ "router": random.choice(users), "service": consumer})

    for sensor in sensors:
        hosting.append({ "router": sensor, "service": producers.pop()})

    for producer in producers:
        hosting.append({ "router": random.choice(sensors), "service": producer})

    if max_hosts is None or max_hosts > len(routers):
        max_hosts = len(routers) # cap at total available routers

    for service in workflow.get_services():
        num_hosts = random.randint(min_hosts, max_hosts)
        chosen_routers = random.sample(sorted(routers), num_hosts)

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
    parser.add_argument('-c', '--compact-output', action='store_true', default=False, help="print compact json")
    parser.set_defaults(algorithm=None)
    subparsers = parser.add_subparsers(title='algorithm', description='algorithm to use for hosting generation')

    uni_parser = subparsers.add_parser('uniform', help="distribute services uniformly")
    uni_parser.set_defaults(algorithm=uniform)
    uni_parser.add_argument('-t', '--topology', type=Path, required=True, help="topology json file input")
    uni_parser.add_argument('-w', '--workflow', type=Path, required=True, help="workflow json file input")

    sensors_group = uni_parser.add_mutually_exclusive_group()
    sensors_group.add_argument('--sensor-list', nargs='+', type=str, default=['sensor'], help="list of sensor routers")
    sensors_group.add_argument('-s', '--num-sensors', type=int, help="number of sensor routers with standard names")

    users_group = uni_parser.add_mutually_exclusive_group()
    users_group.add_argument('--user-list', nargs='+', type=str, default=['user'], help="list of user routers")
    users_group.add_argument('-u', '--num-users', type=int, help="number of user routers with standard names")

    uni_parser.add_argument('-n', '--min-hosts', type=int, default=1, help='min number of hosts per service')
    uni_parser.add_argument('-m', '--max-hosts', type=int, default=1, help='max number of hosts per service')
    uni_parser.add_argument('--start-times', nargs='+', type=int, default=[0], help="list of start time choices, paired with --stop-times")
    uni_parser.add_argument('--stop-times', nargs='+', type=int, default=[-1], help="list of stop time choices, paired with --start-times")

    comb_parser = subparsers.add_parser('combine', help="combine two hosting files")
    comb_parser.set_defaults(algorithm=combine)
    comb_parser.add_argument('-g', '--hosting', action='append', required=True, type=Path, help="topology json file input")

    args = parser.parse_args()

    if not args.algorithm:
        print("must supply algorithm")
        parser.print_help()
        exit(1)

    hosting = args.algorithm(args)

    if args.compact_output:
        indent = None
    else:
        indent = 4

    with open(args.output, "w") as f:
        json.dump(hosting, f, indent=indent)


if __name__ == '__main__':
    main()
