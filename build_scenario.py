#!/usr/bin/env python3

import critical_path_metric as cpm
import topo2json
import json2topo

from collections import defaultdict
import argparse
import json 
import pathlib
import os
import random


# returns str(topofile), dict(topo)
def get_topo(topo_in, topo_type=None, topo_out=None):
    if not topo_type:
        _, ext = os.path.splitext(topo_in)
        if ext == '.txt':
            topo_type = 'txt'
        elif ext == '.json':
            topo_type = 'json'
        else:
            raise ValueError(f"Couldn't infer topo type from {topo_in}, extension should be json or txt")

    # txt needs to be written if input is json, otherwise json doesn't need written to a file
    if not topo_out and topo_type == 'json':
        path, basename = os.path.split(topo_in)
        name, _ = os.path.splitext(basename)
        topo_out = os.path.join(path, name + '.txt')

    topofile = ""

    if topo_type == 'txt':
        topofile = topo_in
        with open(topofile, "r") as f:
            topo = topo2json.parse_topo(f)

        # write only if requested
        if topo_out:
            with open(topo_out, "w") as f:
                f.write(json.dumps(topo))
    elif topo_type == 'json':
        topofile = topo_out
        with open(topo_in, "r") as f:
            topo = json.load(f)

        with open(topofile, "w") as f:
            json2topo.write_topo(f, topo)
    else:
        raise ValueError(f"Invalid topo_type {topo_type}, should be json or txt")

    return str(topofile), topo


def get_services(dag_file, hosting=None):
    dag_cxns = cpm.workflow_connections_from_file(dag_file)
    dag = cpm.Workflow(dag_cxns)

    services = dict()
    services["services"] = []

    dag_producers = dag.get_producers()
    dag_consumers = dag.get_consumers()
    dag_services = dag.get_services()
    dag_nodes = dag.get_nodes()

    if (hosting):
        hosting_services = [h["service"] for h in hosting["routerHosting"]]

        for srv in hosting_services:
            if srv not in dag_nodes:
                dag_nodes.add(srv)
                dag_services.add(srv)

    for node in dag_nodes:
        if node in dag_producers:
            typ = "producer"
        elif node in dag_consumers:
            typ = "consumer"
        elif node in dag_services:
            typ = "service"
        else:
            raise ValueError("Node {node} is neither a producer, consumer, nor service (should be impossible, bug in Workflow(Graph))")

        service = { "name": node, "type": typ }
        services["services"].append(service)

    return services


# TODO
#def generate_topo(num_routers, user_node="user"):
#    routers = [{ "node": user_node, "comment": "NA", "x": 0, "y": 0 }]
#
#    x = 20
#    for i in range(1, num_routers+1):
#        routers.append({ "node": f"rtr-{i}", "comment": "NA", "x": x, "y": 0 })
#        x += 20
#
#    return {"router": routers}


def generate_hosting(
    services, routers, 
    base_hosting=None, user_node="user",
    max_routers=3, num_hosts_range=(1,1), 
    start_time_range=(0,0),
    stop_time_range=None
):
    min_hosts = num_hosts_range[0]
    max_hosts = num_hosts_range[1]

    if (min_hosts < 1):
        raise ValueError("Min routers per service must be at least 1")
    if (max_hosts < 1):
        raise ValueError("Max routers per service must be at least 1")
    if (max_hosts < min_hosts):
        raise ValueError("Max routers per service cannot be less than min routers per service")
    # TODO: more error checking

    # TODO: there is probably a better way to do this (binning based on filter)
    router_list = [rtr["node"] for rtr in routers if rtr["node"] != user_node]
    user_list = [rtr["node"] for rtr in routers if rtr["node"] == user_node]
    if len(user_list) != 1:
        print(user_list)
        raise ValueError("Routers should contain exactly one user")

    # TODO: there is probably a better way to do this (binning based on filter)
    service_list = [srv["name"] for srv in services if srv["type"] != "consumer"]
    consumers = [srv["name"] for srv in services if srv["type"] == "consumer"]
    if len(consumers) != 1:
        raise ValueError("Services should contain exactly one consumer")
    consumer = consumers[0]

    num_hosts_map = {srv: random.randint(min_hosts, max_hosts) for srv in service_list}
    host_population = [r for r in router_list for _ in range(max_routers)]

    if base_hosting:
        for host in base_hosting["routerHosting"]:
            num_hosts_map[host["service"]] -= 1
            host_population.remove(host["router"])

    num_host_samples = sum(n for n in num_hosts_map.values())
    host_samples = random.sample(host_population, k=num_host_samples)
    srv_samples = [srv for srv in service_list for _ in range(num_hosts_map[srv])]

    hosting = [{"router": host, "service": srv} for host, srv in zip(host_samples, srv_samples)]

    if start_time_range:
        for host in hosting:
            host["start"] = random.randint(*start_time_range)

    if stop_time_range:
        for host in hosting:
            host["stop"] = random.randint(*stop_time_range)

    if not base_hosting or not any(user_node == rtr["node"] for rtr in base_hosting["routerHosting"]):
        hosting.append({"router": user_node, "service": consumer, "start": 0})

    if base_hosting:
        hosting.extend(base_hosting["routerHosting"])

    return {"routerHosting": hosting}


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


# TODO: generate random topology
# TODO: generate random dag
def main():
    parser = argparse.ArgumentParser("build_scenario")
    parser.add_argument('-p', '--prefix', type=str, default='/nesco', help="NDN prefix string")
    parser.add_argument('-t', '--topo', type=pathlib.Path, required=True, help="Topology file to use, can be json or txt")
    parser.add_argument('--topo-type', type=str, choices=('json', 'txt'), help="json or txt, inferred from '--topo'")
    parser.add_argument('--topo-out', type=pathlib.Path, help="Topo file output, creates file for generated json or txt")
    parser.add_argument('-d', '--dag', type=pathlib.Path, required=True, help="DAG workflow json")
    parser.add_argument('-r', '--hosting', type=pathlib.Path, help="Hosting json")
    parser.add_argument('--generate-hosting', action='store_true', help="Generate hosting")
    parser.add_argument('-o', '--output', type=argparse.FileType('w'), default='-', help="Scenario json output")
    # TODO: option to pretty print (just pipe into jq for now)

    args = parser.parse_args()

    prefix = args.prefix
    topofile, topo = get_topo(args.topo, args.topo_type, args.topo_out)
    hosting = None

    if args.hosting:
        hosting = get_hosting(args.hosting, args.dag)
    else:
        hosting = None

    services = get_services(args.dag, hosting)

    if args.generate_hosting:
        hosting = generate_hosting(services["services"], topo["router"], base_hosting=hosting)

    if not hosting:
        print("No argument supplied to generate hosting", file=sys.stderr)

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
    print(json.dumps(scenario), file=args.output)


if __name__ == '__main__':
    main()
