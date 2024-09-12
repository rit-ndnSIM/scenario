#!/usr/bin/env python

import json
import re
import copy

from collections import defaultdict, deque, namedtuple

# TODO: add caching
# generally make more optimized??

class Scenario(object):
    """ All scenario information, including workflow and topology """

    def __init__(self, workflow, topology, hosting):
        self.workflow = Workflow(workflow)
        self.topology = Topology(topology)
        self.hosting = Hosting(hosting)

    def build_interest_tree(self, router, consumer):
        """ Builds an interest tree for the scenario """

        workflow = self.workflow.clone()
        tree = Graph([], True)
        Branch = namedtuple('Branch', ['router', 'service', 'time'])
        branches = deque()
        branches.appendleft(Branch(router, consumer, 0))

        while len(branches) > 0:
            current_branch = branches.pop()
            router, interest_service, time = current_branch

            for hosted_service in self.hosting.get_connections(router):
                for upstream_service in workflow.get_nodes(hosted_service):
                    next_router = self.next_hop(router, upstream_service)
                    if next_router == router:
                        next_branch = Branch(router, upstream_service, time)
                        branches.append(next_branch)
                    else:
                        next_branch = Branch(next_router, upstream_service, time+1)
                        branches.appendleft(next_branch)
                    workflow.remove_connection(upstream_service, hosted_service)
                    tree.add(current_branch, next_branch)

            if not self.hosting.is_connected(router, interest_service):
                next_router = self.next_hop(router, interest_service)
                next_branch = Branch(next_router, interest_service, time+1)
                branches.appendleft(next_branch)
                tree.add(current_branch, next_branch)

        return tree

    def get_critical_path_metric(self, router, consumer):
        tree = self.build_interest_tree(router, consumer)

        terminations = tree.get_connections() - tree.get_nodes()

        metric = max(branch.time for branch in terminations)

        return metric

    def next_hop(self, router, service):
        """ Returns the next closest router to service """

        if self.hosting.is_connected(router, service):
            return router

        paths = []

        for dest_router in self.hosting.get_nodes(service):
            paths.append(self.topology.shortest_path(router, dest_router))

        shortest_path = min(paths, key=len)

        return shortest_path[0]


    def __str__(self):
        return '{}({}, {}, {})'.format(self.__class__.__name__, self.workflow, self.topology, self.hosting)
        

# https://stackoverflow.com/questions/19472530/representing-graphs-data-structure-in-python  
class Graph(object):
    """ Graph data structure, undirected by default. """

    def __init__(self, connections=[], directed=False):
        self._graph = defaultdict(set)
        self._directed = directed
        self.add_connections(connections)

    def clone(self):
        """ Create a clone of the object """

        return __class__([(node, service) for node, services in self._graph.items() for service in services], self._directed)

    def add_connections(self, connections):
        """ Add connections (list of tuple pairs) to graph """

        for node1, node2 in connections:
            self.add(node1, node2)

    def add(self, node1, node2):
        """ Add connection between node1 and node2 """

        self._graph[node1].add(node2)
        if not self._directed:
            self._graph[node2].add(node1)

    def remove(self, node):
        """ Remove all references to node """

        delete_nodes = set()
        for n, cxns in self._graph.items():
            if node in cxns:
                cxns.remove(node)

            if not cxns:
                delete_nodes.add(n)

        for n in delete_nodes:
            del self._graph[n]

        if node in self._graph:
            del self._graph[node]

    def remove_connection(self, node, connection):
        """ Remove a connection between nodes """

        cxns = self._graph[node]

        if connection in cxns:
            cxns.remove(connection)

        if not cxns:
            del self._graph[node]

    def is_connected(self, node=None, connection=None):
        """ Is node1 directly connected to node2 """

        if node is None and connection is None:
            return bool(self._graph)

        if connection is None:
            return bool(self.get_connections(node))

        if node is None:
            return bool(self.get_nodes(connection))

        return node in self._graph and connection in self._graph[node]

    def contains(self, node):
        """ Does graph contain node """

        if node in self._graph:
            return True

        if any(node in nodes for nodes in self._graph.values()):
            return True

        return False

    def get_connections(self, node=None):
        """ Get all connections of node """

        if node is None:
            return set(node for nodes in self._graph.values() for node in nodes)

        if node not in self._graph:
            return set()

        return self._graph[node]

    def get_nodes(self, connection=None):
        """ Get all nodes with a connection """

        if connection is None:
            return set(self._graph.keys())

        return set(node for node, cxns in self._graph.items() if connection in cxns)

    def find_path(self, node1, node2, path=[]):
        """ Find any path between node1 and node2 (may not be shortest) """

        path = path + [node1]
        if node1 == node2:
            return path
        if not self.contains():
            return None
        for node in self._graph[node1]:
            if node not in path:
                new_path = self.find_path(node, node2, path)
                if new_path:
                    return new_path
        return None

    # TODO: some sort of state to not re-do work each time we call this
    def shortest_path(self, source, target):
        """ Find the shortest path using Djikstra's algorithm """

        unvisited_nodes = self.get_nodes() & self.get_connections()
        dist = {node: float('inf') for node in unvisited_nodes}
        dist[source] = 0
        prev = dict()

        while len(unvisited_nodes) > 0:
            node = min(unvisited_nodes, key=dist.get)
            unvisited_nodes.remove(node)

            if node == target:
                break

            for neighbor in self.get_connections(node):
                alt = dist[node] + 1
                if alt < dist[neighbor]:
                    dist[neighbor] = alt
                    prev[neighbor] = node

        # return a list of nodes to take

        if target not in prev:
            return None

        path = []
        node = target
        while node != source:
            path.append(node)
            node = prev[node]

        path.reverse()

        return path

    def __str__(self):
        return '{}({})'.format(self.__class__.__name__, dict(self._graph))


class Workflow(Graph):
    """ Workflow data structure, directed graph """

    def __init__(self, connections):
        super().__init__(connections, True)

    def clone(self):
        """ Create a clone of the object """

        return __class__([(node, service) for node, services in self._graph.items() for service in services])

    def get_roots(self):
        return self.get_nodes() - self.get_connections()

    def get_sinks(self):
        return self.get_connections() - self.get_nodes()

    def prune_downstream(self, service):
        if not self.contains(service):
            return None

        sinks = self.get_sinks()
        while (sinks):
            sink = sinks.pop()
            if sink != service:
                self.remove(sink)
                sinks.update(self.get_sinks())

        return self

    def prune_upstream(self, service):
        if not self.contains(service):
            return None

        roots = self.get_roots()
        while (roots):
            root = roots.pop()
            if root != service:
                self.remove(root)
                roots.update(self.get_roots())

        return self

    def prune_split(self, service):
        return (self.clone().prune_downstream(service), self.clone().prune_upstream(service))


class Topology(Graph):
    """ Topology data structure, undirected graph """

    def clone(self):
        """ Create a clone of the object """

        return __class__([(node, service) for node, services in self._graph.items() for service in services])

    def __init__(self, connections):
        super().__init__(connections, False)


class Hosting(Graph):
    """ Hosting data structure """

    def clone(self):
        """ Create a clone of the object """

        return __class__([(node, service) for node, services in self._graph.items() for service in services])

    def __init__(self, connections):
        super().__init__(connections, True)


def workflow_connections_from_file(workflow_path):
    with open(workflow_path) as f:
        workflow_json = json.load(f)
        workflow_connections = [(parent, child) for parent, child_dict in workflow_json['dag'].items() for child in child_dict.keys()]

    return workflow_connections


def topology_connections_from_file(topology_path):
    with open(topology_path) as f:
        lines = iter(f.readlines())

        while (not next(lines).startswith("link")):
            pass
        
        # TODO: this logic could be a lot more robust, but it should work for now
        pattern = re.compile("^([a-zA-Z0-9_-]+)\s+([a-zA-Z0-9_-]+).*$")
        topology_connections = list()

        for line in lines:
            if line.startswith(('#', '\n')):
                continue

            matches = pattern.match(line)
            topology_connections.append((matches.group(1), matches.group(2)))

    return topology_connections
    

def hosting_connections_from_file(hosting_path):
    with open(hosting_path) as f:
        hosting_json = json.load(f)
        hosting_connections = [(key, service) for key, services in hosting_json['routerHosting'].items() for service in services]

    return hosting_connections
 

def scenario_from_files(workflow_path, topology_path, hosting_path):
    workflow_connections = workflow_connections_from_file(workflow_path)
    topology_connections = topology_connections_from_file(topology_path)
    hosting_connections = hosting_connections_from_file(hosting_path)

    return Scenario(workflow_connections, topology_connections, hosting_connections)


def main():
    workflow_file = "workflows/4dag.json"
    topology_file = "topologies/topo-cabeee-3node.txt"
    hosting_file = "workflows/4dag.hosting"

    scenario = scenario_from_files(workflow_file, topology_file, hosting_file)

    tree = scenario.build_interest_tree("user", "/consumer")

    print(tree)

    metric = scenario.get_critical_path_metric("user", "/consumer")

    print(f"metric is {metric}")

if __name__ == '__main__':
    main()
