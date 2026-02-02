#!/usr/bin/env python3

import graph

import random
import argparse
from pathlib import Path
import os
import json
from itertools import combinations
from collections import defaultdict

def strlist(prefix, length):
    if length < 1:
        raise ValueError("length must be positive")

    if length == 1:
        return [prefix]

    return [prefix + str(i) for i in range(length)]


def connect_layers(workflow, above, below):
    # ensure all nodes have outgoing connections
    for u in above:
        v = random.choice(below)
        workflow.add(u, v)

    # ensure all nodes have incoming connections
    for v in below:
        u = random.choice(above)
        workflow.add(u, v)


def layered(args):
    workflow = gen_layered_dag(args.num_services, args.num_producers, args.num_consumers, args.num_layers, args.num_skips, args.aggregate, args.split)

    with open(args.output, "w") as f:
        f.write(json.dumps(workflow.get_dict()))


def separate(args):
    with open(args.workflow) as f:
        parent_workflow = graph.Workflow.from_dict(json.load(f))

    consumers = parent_workflow.get_consumers()
    if len(consumers) != len(args.outputs):
        raise ValueError("number of consumer must match number of output files")

    for consumer, output in zip(consumers, args.outputs):
        workflow  = parent_workflow.clone().prune_downstream(consumer)

        with open(output, 'w') as f:
            f.write(json.dumps(workflow.get_dict()))


def gen_layered_dag(num_services, num_producers=1, num_consumers=1, num_layers=5, num_skips=0, aggregate=False, split=False):
    if num_services < 0:
        raise ValueError("number of services must be nonnegative")

    if num_producers < 1:
        raise ValueError("number of producers must be positive")

    if num_consumers < 1:
        raise ValueError("number of consumers must be positive")

    if num_skips < 0:
        raise ValueError("number of skips must be nonnegative")

    # generate "skip nodes", which will be removed and its inputs will be connected to the outputs
    total_nodes = num_producers + num_services + num_consumers + num_skips

    if num_layers - 2 > num_services:
        raise ValueError("cannot have more layers than nodes")

    if num_layers < 3 and (num_layers < 2 or num_services != 0 or num_skips != 0):
        # producer layer, consumer layer, and at least one service layer
        raise ValueError("must have at least three layers")

    workflow = graph.Workflow()

    layers = [[] for _ in range(num_layers)]
    services = [f"/service{i}" for i in range(num_services)]
    skips = [f"skip{i}" for i in range(num_skips)]

    producers = strlist("/sensor", num_producers)
    consumers = strlist("/consumer", num_consumers)
    if split:
        splitters = strlist("/splitter", num_producers)
    if aggregate:
        aggregators = strlist("/aggregator", num_consumers)

    layers[0].extend(splitters if split else producers)
    layers[-1].extend(aggregators if aggregate else consumers)

    for layer in layers[1:-1]:
        layer.append(services.pop(0))

    services.extend(skips)

    while services:
        layer_index = random.randrange(1, num_layers - 1)
        layers[layer_index].append(services.pop())

    # add splitters
    if split:
        for producer, splitter in zip(producers, splitters):
            workflow.add(producer, splitter)

    # add connections between layers
    for current_layer, next_layer in zip(layers[0:num_layers - 1], layers[1:num_layers]):
        connect_layers(workflow, current_layer, next_layer)

    # bypass and remove "skip nodes"
    for node in skips:
        above = tuple(workflow.get_incoming(node))
        below = tuple(workflow.get_outgoing(node))

        connect_layers(workflow, above, below)

        workflow.remove(node)

    # add aggregators
    if aggregate:
        for aggregator, consumer in zip(aggregators, consumers):
            workflow.add(aggregator, consumer)

    return workflow


def metadata():
    workflow = graph.Workflow.from_dict(json.load(f))

    for service in workflow['services']:
        typ = service['type']
        if typ == 'service':
            metadata = random.choice(args.service_data)
            service.update(metadata)
        elif typ == 'consumer':
            service.update(args.consumer_data)
        elif typ == 'producer':
            service.update(args.producer_data)
        else:
            raise Exception("malformed workflow json, bad service type")

    with open(args.output, "w") as f:
        f.write(json.dumps(workflow.get_dict()))

    return


def kv_parse(kv_str):
    default_args = {
        'stop': -1,
        'start': 0,
    }

    arg_dict = {k:int(v) for k,v in (item.split(':') for item in kv_str.split(','))}

    for key, value in default_args.items():
        if key not in arg_dict:
            arg_dict[key] = value

    for key, value in arg_dict.items():
        if key not in default_args:
            raise ValueError(f"unknown argument {key} passed with value {value}")

    return arg_dict


def main():
    parser = argparse.ArgumentParser("genworkflow")
    parser.set_defaults(algorithm=None)
    subparsers = parser.add_subparsers(title='algorithm', description='algorithm to use for hosting generation')

    lay_parser = subparsers.add_parser('layered', help="use layered algorithm")
    lay_parser.set_defaults(algorithm=layered)
    lay_parser.add_argument('-o', '--output', type=Path, default='/dev/stdout', help="workflow json file output")
    lay_parser.add_argument('-n', '--num-services', type=int, required=True, help='number of services')
    lay_parser.add_argument('-p', '--num-producers', type=int, default=1, help='number of producers')
    lay_parser.add_argument('-c', '--num-consumers', type=int, default=1, help='number of consumers')
    lay_parser.add_argument('-l', '--num-layers', type=int, required=True, help="number of layers")
    lay_parser.add_argument('-s', '--num-skips', type=int, default=0, help="number of skip nodes")
    lay_parser.add_argument('-a', '--aggregate', action='store_true', default=False, help='add aggregators before consumer')
    lay_parser.add_argument('-t', '--split', action='store_true', default=False, help='add splitters after producers')

    # i really wannna call this "split" but that shadows --split and i can't think of a better name
    sep_parser = subparsers.add_parser('separate', help="separate workflow into one for each consumer")
    sep_parser.set_defaults(algorithm=separate)
    sep_parser.add_argument('-w', '--workflow', type=Path, required=True, help='workflow to separate')
    sep_parser.add_argument('-o', '--outputs', nargs='+', type=Path, help="workflow json file outputs, one per consumer")

    met_parser = subparsers.add_parser('metadata', help="add metadata to existing workflow")
    met_parser.set_defaults(algorithm=metadata)
    met_parser.add_argument('-w', '--workflow', type=Path, required=True, help='workflow to add metadata to')
    met_parser.add_argument('-o', '--output', type=Path, default='/dev/stdout', help="workflow json file output")
    met_parser.add_argument('-c', '--consumer-data', type=kv_parse, required=True, help='consumer metadata')
    met_parser.add_argument('-p', '--producer-data', type=kv_parse, required=True, help='producer metadata')
    met_parser.add_argument('-s', '--service-data', nargs='+', type=kv_parse, required=True, help='services metadata, can supply multiple to randomly assign')

    args = parser.parse_args()

    if not args.algorithm:
        print("must supply algorithm")
        parser.print_help()
        exit(1)

    workflow = args.algorithm(args)


if __name__ == '__main__':
    main()
