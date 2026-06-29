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
    parser.add_argument('-w', '--workflow', nargs='+', required=True, help="DAG workflow json file(s)")
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
    parser.add_argument('-pcf', '--poissonConsumerFrequency', type=int, default=1, help="set consumer Poisson rate")
    parser.add_argument('-pni', '--poissonConsumerNumInterests', type=int, default=1, help="set consumer Poisson number of interests to generate (total)")
    parser.add_argument('-fud', '--producerFreshnessUniformDist', type=int, default=0, help="set producer freshness uniform distribution")
    parser.add_argument('-fmin', '--producerFreshnessMSmin', type=int, default=1, help="set producer freshness min value in milliseconds")
    parser.add_argument('-fmax', '--producerFreshnessMSmax', type=int, default=1, help="set producer freshness max value in milliseconds")
    parser.add_argument('-fresh', '--producerFreshnessMS', type=float, default=1, help="set producer freshness in milliseconds")

    args = parser.parse_args()

    # Dynamic containers to assemble data from all 100 workflow files
    services = []
    dags_dict = {}
    seen_services = set()

    # Loop through every workflow file path passed from Bash
    for idx, workflow_path in enumerate(args.workflow, start=1):
        with open(workflow_path) as f:
            workflow_data = json.load(f)
            
            # Extract and deduplicate services across files
            if 'services' in workflow_data:
                for service in workflow_data['services']:
                    service_name = service.get('name')
                    
                    # FIX 1: Make consumer names unique so they aren't deleted as duplicates
                    if service.get('type') == 'consumer':
                        service_name = f"{service_name}{idx}"
                        service['name'] = service_name

                    if service_name not in seen_services:
                        seen_services.add(service_name)
                        services.append(service)
            
            # Map each workflow's DAG to a unique key (dag1, dag2, dag3...)
            if 'dag' in workflow_data:
                dags_dict[f'dag{idx}'] = workflow_data['dag']

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

    # Add the serviceDiscovery entry if the flag is set to 1
    if args.serviceDiscovery == 1:
        services.append({
            "name": "/serviceDiscovery",
            "type": "service",
            "strategy": "/localhost/nfd/strategy/best-route"
        })
    # Add the serviceDiscovery entry if the flag is set to 2
    if args.serviceDiscovery == 2:
        services.append({
            "name": "/serviceDiscovery2",
            "type": "service",
            "strategy": "/localhost/nfd/strategy/best-route"
        })

    # Update the strategy with the command line input
    strategy_path = f"/localhost/nfd/strategy/{strategy}"
    for s in services:
        s['strategy'] = strategy_path

    # Update the cs_size with the command line input
    routers = topology['router']
    for r in routers:
        if not r['node'].startswith("user"):
            r['cs-size'] = cs_size

    # Construct the final merged scenario dictionary
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
        'poissonConsumerFrequency': (args.poissonConsumerFrequency),
        'poissonConsumerNumInterests': (args.poissonConsumerNumInterests),
        'producerFreshnessUniformDist': (args.producerFreshnessUniformDist),
        'producerFreshnessMSmin': (args.producerFreshnessMSmin),
        'producerFreshnessMSmax': (args.producerFreshnessMSmax),
        'producerFreshnessMS': (args.producerFreshnessMS),
        **topology,
        'services': services,
        **hosting,
        'dag': dags_dict  # Contains dag1, dag2 ... dag100 nested cleanly
    }

    if not os.path.exists(args.output) or args.force:
        with open(args.output, "w") as f:
            json.dump(scenario, f, indent=2)
    else:
        raise FileExistsError(args.output)


if __name__ == '__main__':
    main()