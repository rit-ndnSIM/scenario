#!/usr/bin/env python3

import graph
import topo2json
import json2topo

from collections import defaultdict
from itertools import combinations
from pathlib import Path
import argparse
import json
import os
import sys
import random


def main():
    parser = argparse.ArgumentParser("build_scenario")
    parser.add_argument('-p', '--prefix', type=str, default='/nescoSCOPT', help="NDN prefix string")
    parser.add_argument('-s', '--strategy', type=str, default='multicast', help="routing strategy")
    parser.add_argument('-c', '--cs-size', type=int, default=0, help="content store size to enable caching")
    parser.add_argument('-x', '--topo-txt', type=Path, required=True, help="Topology txt file to use")
    parser.add_argument('-t', '--topo-json', type=Path, help="Topology json file to use")
    parser.add_argument('-w', '--workflow', type=Path, required=True, help="DAG workflow json")
    parser.add_argument('-g', '--hosting', type=Path, required=True, help="Hosting json")
    parser.add_argument('-o', '--output', type=Path, default='/dev/stdout', help="Scenario json output")
    parser.add_argument('-f', '--force', action='store_true', default=False, help="overwrite files")
    parser.add_argument('-sd', '--serviceDiscovery', type=int, default=0, help="perform service discovery")
    parser.add_argument('-ru', '--resourceUtilization', type=int, default=0, help="consider resource utilization")
    parser.add_argument('-ra', '--resourceAllocation', type=int, default=0, help="perform resource allocation")
    parser.add_argument('-ar', '--allocationReuse', type=int, default=0, help="perform allocation reuse")
    parser.add_argument('-sc', '--scheduleCompaction', type=int, default=0, help="perform schedule compaction")
    parser.add_argument('-stosd', '--startTimeOffsetSD', type=int, default=0, help="start time offset for service discovery")
    parser.add_argument('-stowf', '--startTimeOffsetWF', type=int, default=2, help="start time offset for workflow")
    parser.add_argument('-set', '--simulationEndTime', type=int, default=200, help="set simulation end time")

    args = parser.parse_args()

    with open(args.workflow) as f:
        workflow = json.load(f)

    with open(args.hosting) as f:
        hosting = json.load(f)

    if args.topo_json is not None:
        with open(args.topo_json) as f:
            topology = json.load(f)

        if not os.path.exists(args.topo_txt) or args.force:
            with open(args.topo_txt, 'w') as f:
                graph.Topology.from_dict(topology).write_txt(f)
        else:
            raise FileExistsError(args.topo_txt)
    else:
        with open(args.topo_txt) as f:
            topology = graph.Topology.from_txt(f).get_dict()

    prefix = args.prefix
    strategy = args.strategy
    cs_size = args.cs_size
    services = workflow['services']
    dag = workflow['dag']


    # Add the serviceDiscovery entry if the flag is set to 1
    if args.serviceDiscovery == 1:
        services.append({
            "name": "/serviceDiscovery",
            "type": "service",
            "strategy": "/localhost/nfd/strategy/best-route"
        })

    # update the strategy with the command line input
    strategy_path = f"/localhost/nfd/strategy/{strategy}"
    for s in services:
        s['strategy'] = strategy_path

    # update the cs_size with the command line input
    routers = topology['router']
    for r in routers:
        if not r['node'].startswith("user"):
            r['cs-size'] = cs_size


    # TODO: should support multiple workflows for multiple consumers? the bones are there but little extra work
    # is single workflow with multiple consumers acceptable?
    scenario = {
        'prefix': prefix,
        'topofile': str(args.topo_txt),
        'serviceDiscovery': (args.serviceDiscovery),
        'resourceUtilization': (args.resourceUtilization),
        'resourceAllocation': (args.resourceAllocation),
        'allocationReuse': (args.allocationReuse),
        'scheduleCompaction': (args.scheduleCompaction),
        'startTimeOffsetSD': (args.startTimeOffsetSD),
        'startTimeOffsetWF': (args.startTimeOffsetWF),
        'simulationEndTime': (args.simulationEndTime),
        #'workflowFile': str(args.workflow),
        **topology,
        'services': services,
        **hosting,
        'dag': {
            'dag1': dag,
        }
    }

    if not os.path.exists(args.output) or args.force:
        with open(args.output, "w") as f:
            json.dump(scenario, f, indent=2)
    else:
        raise FileExistsError(args.output)


if __name__ == '__main__':
    main()
