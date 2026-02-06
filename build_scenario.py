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
    parser.add_argument('-p', '--prefix', type=str, default='/nesco', help="NDN prefix string")
    parser.add_argument('-x', '--topo-txt', type=Path, required=True, help="Topology txt file to use")
    parser.add_argument('-t', '--topo-json', type=Path, help="Topology json file to use")
    parser.add_argument('-w', '--workflow', type=Path, required=True, help="DAG workflow json")
    parser.add_argument('-g', '--hosting', type=Path, required=True, help="Hosting json")
    parser.add_argument('-o', '--output', type=Path, default='/dev/stdout', help="Scenario json output")
    parser.add_argument('-f', '--force', action='store_true', default=False, help="overwrite files")

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
    services = workflow['services']
    dag = workflow['dag']

    # TODO: should support multiple workflows for multiple consumers? the bones are there but little extra work
    # is single workflow with multiple consumers acceptable?
    scenario = {
        'prefix': prefix,
        'topofile': str(args.topo_txt),
        #'workflowFile': str(args.workflow),
        **topology,
        'services': workflow['services'],
        **hosting,
        'dag': {
            'dag1': workflow['dag'],
        }
    }

    if not os.path.exists(args.output) or args.force:
        with open(args.output, "w") as f:
            json.dump(scenario, f, indent=2)
    else:
        raise FileExistsError(args.output)


if __name__ == '__main__':
    main()
