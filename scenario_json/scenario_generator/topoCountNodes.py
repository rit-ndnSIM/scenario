#!/usr/bin/env python3

import argparse
import pathlib
import json
import sys

def main():
    parser = argparse.ArgumentParser(description="topoCountNodes")
    parser.add_argument('-t', '--topo', type=pathlib.Path, required=True, help="topology json file to analyze")

    args = parser.parse_args()

    # Check if file exists
    if not args.topo.exists():
        print(f"Error: File {args.topo} not found.", file=sys.stderr)
        sys.exit(1)

    try:
        with open(args.topo, "r") as f:
            data = json.load(f)
            
        # Based on your Topology.get_dict() return: {"router": routers, "link": links}
        if "router" in data:
            node_count = len(data["router"])
            print(node_count)
        else:
            print("Error: JSON does not contain 'router' key.", file=sys.stderr)
            sys.exit(1)
            
    except json.JSONDecodeError:
        print(f"Error: Failed to decode JSON from {args.topo}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()