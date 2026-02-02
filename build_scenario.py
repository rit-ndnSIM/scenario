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
import random


def get_hosting(hosting_file, workflow_file):
    hosting = dict()
    hosting["routerHosting"] = []

    with open(hosting_file, "r") as f:
        dag_hosting = json.load(f)

    for router, services in dag_hosting["routerHosting"].items():
        for service in services:
            if service == '/consumer':
                hosting["routerHosting"].append({ "router": router, "service": service, "workflowFile": str(workflow_file), "dag": "dag1", "start": 0, "end": -1})
            else:
                hosting["routerHosting"].append({ "router": router, "service": service })

    return hosting


def main():
    parser = argparse.ArgumentParser("build_scenario")
    parser.add_argument('-p', '--prefix', type=str, default='/nesco', help="NDN prefix string")
    parser.add_argument('-t', '--topo-txt', type=Path, required=True, help="Topology txt file to use")
    parser.add_argument('-j', '--topo-json', type=Path, help="Topology json file to use")
    parser.add_argument('-w', '--workflow', type=Path, required=True, help="DAG workflow json")
    parser.add_argument('-g', '--hosting', type=Path, required=True, help="Hosting json")
    parser.add_argument('-o', '--output', type=Path, default='/dev/stdout', help="Scenario json output")
    parser.add_argument('-f', '--force', type=bool, action='store_true', default=False, help="overwrite files")

    args = parser.parse_args()

    with open(args.workflow) as f:
        workflow = graph.Workflow.from_dict(json.load(f))

    # TODO: need "workflowFile": file, "dag": "dag1", "start": 0, "end": -1
    with open(args.hosting) as f:
        hosting = json.load(f)

    if args.topo_json is not None:
        with open(args.topo_json) as f:
            topology = graph.Topology.from_dict(json.load(f))

        if not os.path.exists(args.topo_txt) or args.force:
            with open(args.topo_txt, 'w') as f:
                topology.write_txt(f)
        else:
            raise FileExistsError(args.topo_txt)
    else:
        with open(args.topo_txt) as f:
            topology = graph.Topology.from_txt(f)

    prefix = args.prefix

    with open(args.dag, "r") as f:
        dag = json.load(f)
        dag["dag"] = {"dag1": dag["dag"]} # insert the new level "dag1". This is targeting scenarios that only have one consumer (thus only dag1).

    scenario = {
        'prefix': prefix,
        'topofile': topofile,
        #'workflowFile': str(args.dag),
        **topo,
        **services,
        **hosting,
        **dag,
    }

    # TODO: don't overwrite file if it exists (in case it has been manually altered for some reason)
    with open(args.output, "r") as f:
        print(json.dumps(scenario))


if __name__ == '__main__':
    main()
