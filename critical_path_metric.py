#!/usr/bin/env python

import json
import re
import copy

import sys, argparse
import time

from collections import defaultdict, deque, namedtuple

from graph import *

def main():
    #workflow_file = "workflows/4dag.json"
    #topology_file = "topologies/topo-cabeee-3node.txt"
    #hosting_file = "workflows/4dag.hosting"

    #workflow_file = "workflows/8dag.json"
    #topology_file = "topologies/topo-cabeee-3node.txt"
    #hosting_file = "workflows/8dag.hosting"
    
    #workflow_file = "workflows/20-parallel.json"
    #topology_file = "topologies/topo-cabeee-20node-parallel.txt"
    #hosting_file = "workflows/20-parallel.hosting"
    
    #workflow_file = "workflows/20-sensor.json"
    #topology_file = "topologies/topo-cabeee-20sensor.txt"
    #hosting_file = "workflows/20-sensor.hosting"

    #workflow_file = "workflows/20-linear.json"
    #topology_file = "topologies/topo-cabeee-3node.txt"
    #hosting_file = "workflows/20-linear-in3node.hosting"

    #workflow_file = "workflows/20-linear.json"
    #topology_file = "topologies/topo-cabeee-20node-linear.txt"
    #hosting_file = "workflows/20-linear.hosting"

    # Parse command line options
    parser = argparse.ArgumentParser()
    parser.add_argument('-type', type=str, default=None, help='Command line option to enter forwarding scheme (nescoSCOPT, orchA or orchB)')
    parser.add_argument('-workflow', type=str, default=None, help='Command line option to enter workflow file name')
    parser.add_argument('-topology', type=str, default=None, help='Command line option to enter topology file name')
    parser.add_argument('-hosting', type=str, default=None, help='Command line option to enter hosting file name')
    args = parser.parse_args()


    if args.type is None:
        print("Please enter a fowarding type (nesco, nescoSCOPT, orchA or orchB")
        sys.exit(0)
    if args.workflow is None:
        print("Please enter a workflow, topology, and hosting file using the command line options")
        sys.exit(0)
    if args.topology is None:
        print("Please enter a workflow, topology, and hosting file using the command line options")
        sys.exit(0)
    if args.hosting is None:
        print("Please enter a workflow, topology, and hosting file using the command line options")
        sys.exit(0)

    workflow_file = args.workflow
    topology_file = args.topology
    hosting_file = args.hosting

    start_time = time.time()

    scenario = scenario_from_files(workflow_file, topology_file, hosting_file)

    #tree = scenario.build_interest_tree("user", "/consumer")

    #print(tree)

    #metric = scenario.critical_path_metric("user", "/consumer")

    if (args.type == 'nesco'):
        metric = scenario.critical_path_metric("user", "/consumer", False)
    if (args.type == 'nescoSCOPT'):
        metric = scenario.critical_path_metric("user", "/consumer", True)
    if (args.type == 'orchA'):
        metric = scenario.orch_a_critical_path_metric("user", "/consumer")
    if (args.type == 'orchB'):
        metric = scenario.orch_b_critical_path_metric("user", "/consumer")


    #tree = scenario.build_interest_tree("user", "/consumer")
    end_time = time.time()

    print(f"metric is {metric}")
    print(f"time is {(end_time - start_time)}")

if __name__ == '__main__':
    main()

# vim: tw=0 shiftwidth=4 expandtab tabstop=8 softtabstop=8 smarttab autoindent
