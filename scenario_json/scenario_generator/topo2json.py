#!/usr/bin/env python3

import graph

from collections import defaultdict
import os.path
import string
import argparse
import json

def main():
    parser = argparse.ArgumentParser("topo2json")
    parser.add_argument("file", nargs='+', help="Files to convert")
    args = parser.parse_args()

    for filename in args.file:
        json_name = os.path.splitext(filename)[0] + ".json"
        with open(filename, "r") as topo_file:
            with open(json_name, "w") as json_file:
                topo = graph.Topology.from_txt(topo_file)
                json.dump(topo.get_dict(), f)


if __name__ == '__main__':
    main()
