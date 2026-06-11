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

    base_assignments = []
    if args.base_hosting and args.base_hosting.exists():
        with open(args.base_hosting) as f:
            base_assignments = json.load(f).get("routerHosting", [])

    # Load all workflows and track their respective execution paths
    workflows = []
    workflow_paths = []
    for wf_path in args.workflows:
        with open(wf_path) as f:
            workflows.append(graph.Workflow.from_dict(json.load(f)))
            workflow_paths.append(wf_path)

    #with open(args.workflow) as f:
    #    workflow = graph.Workflow.from_dict(json.load(f))

    with open(args.topology) as f:
        topology = graph.Topology.from_dict(json.load(f))

    if len(args.start_times) != len(args.stop_times):
        raise Exception("length start times and stop times must match")

    # Pass the base_assignments to the generator
    #hosting = gen_uniform_hosting(workflow, topology, args.sensor_list, args.user_list, args.min_hosts, args.max_hosts)
    hosting = gen_uniform_hosting(workflows, workflow_paths, topology, args.sensor_list, args.user_list, args.min_hosts, args.max_hosts, base_assignments)

    # Compile a unique structural inventory of components across all parallel DAG contexts
    all_services = set()
    all_consumers = set()
    all_producers = set()
    for idx, wf in enumerate(workflows, start=1):
        wf_consumers = set(wf.get_consumers())
        
        # Filter out consumers from the general service pool
        all_services.update(set(wf.get_services()) - wf_consumers)
        
        # Append index to make the consumer name globally unique
        for c in wf_consumers:
            all_consumers.add(f"{c}{idx}")
            
        all_producers.update(wf.get_producers())

    '''
    for item in hosting:
        if item['service'] in services:
            start, stop = random.choice(tuple(zip(args.start_times, args.stop_times)))
            makespan = random.randint(args.makespan_min, args.makespan_max)
            item.update({'start': start, 'end': stop, 'makespanNS': makespan})
        elif item['service'] in consumers:
            # TODO: this may need to be configurable
            item.update({"workflowFile": str(args.workflow), "dag": "dag1", "start": 0, "end": -1 })
        elif item['service'] in producers:
            #item.update({"start": 0, "end": -1 })
            item.update({"start": 0, "end": -1, 'makespanNS': 1000000})
    '''
    for item in hosting:
        srv_name = item['service']
        if srv_name in all_services:
            start, stop = random.choice(tuple(zip(args.start_times, args.stop_times)))
            makespan = random.randint(args.makespan_min, args.makespan_max)
            item.update({'start': start, 'end': stop, 'makespanNS': makespan})
        elif srv_name in all_consumers:
            # Metadata maps directly during initial provisioning inside gen_uniform_hosting
            pass
        elif srv_name in all_producers:
            item.update({"start": 0, "end": -1, 'makespanNS': 1000000})

    return { "routerHosting": hosting }

'''
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

def gen_uniform_hosting(workflow, topology, sensors=["sensor"], users=["user"], min_hosts=1, max_hosts=None, base_assignments=[]):
    routers = topology.get_nodes()
    # ... existing validation checks ...

    # If we have base assignments, start with them instead of an empty list
    if base_assignments:
        hosting = list(base_assignments)
    else:
        hosting = []
        # Only perform initial sensor/user/producer/consumer mapping if hosting is empty
        consumers = list(workflow.get_consumers())
        producers = list(workflow.get_producers())

        for user in users:
            hosting.append({ "router": user, "service": consumers.pop()})
        for consumer in consumers:
            hosting.append({ "router": random.choice(users), "service": consumer})
        for sensor in sensors:
            hosting.append({ "router": sensor, "service": producers.pop()})
        for producer in producers:
            hosting.append({ "router": random.choice(sensors), "service": producer})

    if max_hosts is None or max_hosts > len(routers):
        max_hosts = len(routers)

    for service in workflow.get_services():
        # Determine current hosting for this service from the base file
        current_routers = {h['router'] for h in hosting if h['service'] == service}
        current_count = len(current_routers)

        # Target number of hosts for this iteration
        target_hosts = random.randint(min_hosts, max_hosts)
        
        # Only add if we haven't reached the target
        if target_hosts > current_count:
            needed = target_hosts - current_count
            # Available routers are those NOT already hosting this specific service
            available_routers = sorted(list(set(routers) - current_routers))
            
            if available_routers:
                # Sample from the remaining available routers
                new_routers = random.sample(available_routers, min(needed, len(available_routers)))
                for router in new_routers:
                    hosting.append({ "router": router, "service": service })

    return hosting
'''
def gen_uniform_hosting(workflows, workflow_paths, topology, sensors=["sensor"], users=["user"], min_hosts=1, max_hosts=None, base_assignments=[]):
    routers = topology.get_nodes()

    if not all(node in routers for node in (*sensors, *users)):
        raise ValueError("all sensors and users must be in the topology")

    if min_hosts < 1:
        raise ValueError("min_hosts must be at least 1")
    if max_hosts is not None and min_hosts > max_hosts:
        raise ValueError("min_hosts cannot be greater than max_hosts")

    # Build maps tracking consumer profiles and shared infrastructure services
    consumers = []
    consumer_meta_map = {}
    global_services = set()
    global_producers = []

    for idx, (wf, path) in enumerate(zip(workflows, workflow_paths), start=1):
        wf_consumers = list(wf.get_consumers())
        
        # Append the idx so the dictionary doesn't overwrite 100 times!
        for c in wf_consumers:
            unique_c = f"{c}{idx}"
            consumers.append(unique_c)
            consumer_meta_map[unique_c] = {
                "workflowFile": str(path),
                "dag": f"dag{idx}",
                "start": 0,
                "end": -1
            }
            
        # Add remaining services to global pools
        global_services.update(set(wf.get_services()) - set(wf_consumers))
        global_producers.extend(list(wf.get_producers()))


    # Clean deduplication pass on core infrastructure asset labels
    global_producers = list(set(global_producers))

    if len(consumers) < len(users):
        raise ValueError("cannot have more users than consumers")

    if len(global_producers) < len(sensors):
        raise ValueError("cannot have more sensors than producers")

    if base_assignments:
        hosting = list(base_assignments)
    else:
        hosting = []
        
        # Reverse stack layout for sequential array pops
        consumers.reverse()
        global_producers.reverse()

        for user in users:
            c_service = consumers.pop()
            entry = { "router": user, "service": c_service }
            entry.update(consumer_meta_map[c_service])
            hosting.append(entry)

        #for consumer in reversed(consumers):
        for consumer in consumers:
            entry = { "router": random.choice(users), "service": consumer }
            entry.update(consumer_meta_map[consumer])
            hosting.append(entry)

        for sensor in sensors:
            hosting.append({ "router": sensor, "service": global_producers.pop() })

        for producer in global_producers:
            hosting.append({ "router": random.choice(sensors), "service": producer })

    if max_hosts is None or max_hosts > len(routers):
        max_hosts = len(routers)

    for service in global_services:
        current_routers = {h['router'] for h in hosting if h['service'] == service}
        current_count = len(current_routers)
        target_hosts = random.randint(min_hosts, max_hosts)
        
        if target_hosts > current_count:
            needed = target_hosts - current_count
            available_routers = sorted(list(set(routers) - current_routers))
            
            if available_routers:
                new_routers = random.sample(available_routers, min(needed, len(available_routers)))
                for router in new_routers:
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
    uni_parser.add_argument('-w', '--workflows', nargs='+', required=True, help="workflow json file input(s)")

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
    uni_parser.add_argument('--makespan-min', type=int, default=0, help='minimum service makespan in NS')
    uni_parser.add_argument('--makespan-max', type=int, default=0, help='maximum service makespan in NS')
    uni_parser.add_argument('-b', '--base-hosting', type=Path, help="base hosting json file to extend")

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
