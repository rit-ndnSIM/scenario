#!/usr/bin/env python

import json
import re
import copy

import sys, argparse
import time

from collections import defaultdict, deque, namedtuple

class Scenario(object):
    """ All scenario information, including workflow and topology """

    def __init__(self, workflow_cxns, topology_cxns, hosting_cxns, wf_meta=None, topo_node_meta=None, topo_edge_meta=None):
        self.workflow = Workflow(workflow_cxns, metadata=wf_meta)
        self.topology = Topology(topology_cxns, metadata=topo_node_meta, edge_metadata=topo_edge_meta)
        self.hosting = Hosting(hosting_cxns)

    def build_interest_tree(self, user, consumer, shortcutOPT=False):
        """ Builds an interest tree for the scenario """

        workflow = self.workflow.clone()
        tree = Graph([], True)
        Branch = namedtuple('Branch', ['router', 'service', 'time'])
        branches = deque()
        branches.appendleft(Branch(user, consumer, 0))

        while len(branches) > 0:
            current_branch = branches.pop()
            router, interest_service, time = current_branch

            if not self.hosting.is_hosting(router, interest_service): 
                # if not hosting, take a hop
                next_router = self.next_hop(router, interest_service)
                next_branch = Branch(next_router, interest_service, time+1)
                branches.appendleft(next_branch)
                tree.add(current_branch, next_branch)
            else:
                # if hosted, generate interests for get upstream service
                for upstream_service in workflow.get_incoming(interest_service):
                    next_branch = Branch(router, upstream_service, time)
                    # .append to handle immediately, keeps things synced (i think)
                    branches.append(next_branch)

                    # mark as accounted for by removing connection
                    workflow.remove_connection(upstream_service, interest_service)
                    tree.add(current_branch, next_branch)

            # repeat code here but it's fine
            # shortcutOPT
            if shortcutOPT:
                for hosted_service in self.hosting.get_hosting(router):
                    for upstream_service in workflow.get_incoming(hosted_service):
                        next_branch = Branch(router, upstream_service, time)
                        branches.append(next_branch)
                        workflow.remove_connection(upstream_service, hosted_service)
                        tree.add(current_branch, next_branch)

        return tree

    # TODO: add caching
    # generally make more optimized??
    def critical_path_metric(self, user, consumer, shortcutOPT=False):
        """ Get critical path for distributed orchestration """

        self.check_scenario(user, consumer)

        tree = self.build_interest_tree(user, consumer, shortcutOPT)

        terminations = tree.get_sinks()

        metric = max(branch.time for branch in terminations)

        return metric

    def orch_a_critical_path_metric(self, user, consumer):
        """ Get critical path metric for orchestrator A """

        self.check_scenario(user, consumer)

        branches = deque()
        dist = {}

        roots = self.workflow.get_roots()
        for root in roots:
            dist[root] = len(self.shortest_service_path(user, root))
            branches.extendleft(self.workflow.get_outgoing(root))

        while len(branches) > 0:
            service = branches.pop()

            path = self.shortest_service_path(user, service)
            if path:
                router = path[-1]
            else:
                router = user
            # reflexive interest, interest for inputs, interest for service
            hops = len(path)

            upstream_services = self.workflow.get_incoming(service)
            # account for previous hops
            hops += max(dist[upstream] for upstream in upstream_services)
            # account for inputs to service (sent from this node)
            # assuming it does this in parallel? i see no reason why it would not
            # also, assume consumer is hosted on orchestrator and these don't need grabbed
            # probably a better way to handle this
            if service != consumer:
                hops += max(len(self.shortest_service_path(router, upstream)) for upstream in upstream_services)

            dist[service] = hops

            branches.extendleft(self.workflow.get_outgoing(service))

        return dist[consumer]

    def orch_b_critical_path_metric(self, user, consumer):
        """ Get critical path metric for orchestrator B """

        self.check_scenario(user, consumer)

        branches = deque()
        dist = {}

        roots = self.workflow.get_roots()
        for root in roots:
            dist[root] = len(self.shortest_service_path(user, root))
            branches.extendleft(self.workflow.get_outgoing(root))

        while len(branches) > 0:
            service = branches.pop()

            path = self.shortest_service_path(user, service)
            # reflexive interest, interest for inputs, interest for service
            hops = 3 * len(path)

            upstream_services = self.workflow.get_incoming(service)
            # account for previous hops
            hops += max(dist[upstream] for upstream in upstream_services)

            dist[service] = hops

            branches.extendleft(self.workflow.get_outgoing(service))

        return dist[consumer]

    def check_scenario(self, user, consumer):
        if not self.hosting.is_hosting(user, consumer):
            raise ValueError(f"Specified user host '{user}' not hosting consumer '{consumer}'")

        if consumer not in self.workflow.get_nodes():
            print(self.workflow)
            raise ValueError(f"Specified consumer '{consumer}' not in workflow")

        if user not in self.topology.get_nodes():
            raise ValueError(f"Specified user host '{user}' not in topology")

    def shortest_service_path(self, router, service):
        """ Returns an array of routers to visit to reach the nearest router hosting service """

        if self.hosting.is_hosting(router, service):
            return []

        paths = []

        for dest_router in self.hosting.get_hosts(service):
            paths.append(self.topology.shortest_path(router, dest_router))

        shortest_path = min(paths, key=len)

        return shortest_path

    def next_hop(self, router, service):
        """ Returns the next closest router to service """

        if self.hosting.is_hosting(router, service):
            return router

        return self.shortest_service_path(router, service)[0]

    def __str__(self):
        return '{}({}, {}, {})'.format(self.__class__.__name__, self.workflow, self.topology, self.hosting)
        

# https://stackoverflow.com/questions/19472530/representing-graphs-data-structure-in-python  
class Graph(object):
    """ Graph data structure, undirected by default. """

    def __init__(self, connections=[], directed=False, metadata=None, edge_metadata=None):
        self._graph = defaultdict(set)
        self._metadata = defaultdict(dict)
        self._edge_metadata = defaultdict(dict)
        self._directed = directed
        
        if metadata:
            self._metadata.update(metadata)
        if edge_metadata:
            self._edge_metadata.update(edge_metadata)
            
        self.add_connections(connections)

    def clone(self):
        """ Create a clone of the object """

        return self.__class__(
            [(node, service) for node, services in self._graph.items() for service in services], 
            self._directed, 
            copy.deepcopy(self._metadata),
            copy.deepcopy(self._edge_metadata)
        )

    def update_metadata(self, node, **kwargs):
        """ Update metadata for a specific node """
        self._metadata[node].update(kwargs)

    def update_edge_metadata(self, node1, node2, **kwargs):
        """ Update metadata for a specific node """
        self._edge_metadata[(node1, node2)].update(kwargs)

    def get_metadata(self, node):
        """ Get metadata dict for a node """
        return self._metadata[node]

    def get_edge_metadata(self, node1, node2):
        """ Get metadata for an edge, checking both directions if undirected """
        if (node1, node2) in self._edge_metadata:
            return self._edge_metadata[(node1, node2)]
        elif not self._directed and (node2, node1) in self._edge_metadata:
            return self._edge_metadata[(node2, node1)]
        return {}

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

        if node in self._metadata:
            del self._metadata[node]

        # Cleanup edges containing node in metadata
        self._edge_metadata = {k: v for k, v in self._edge_metadata.items() if node not in k}

    def remove_connection(self, upstream, downstream):
        """ Remove a downstream between upstreams """

        cxns = self._graph[upstream]

        if downstream in cxns:
            cxns.remove(downstream)

        if not cxns:
            del self._graph[upstream]

        if (upstream, downstream) in self._edge_metadata:
            del self._edge_metadata[(upstream, downstream)]

    def is_connected(self, upstream=None, downstream=None):
        """
            Is upstream directly connected to downstream
            If an argument is none, check against all nodes
        """

        if upstream is None and downstream is None:
            return bool(self._graph)

        if downstream is None:
            return bool(self.get_outgoing(upstream))

        if upstream is None:
            return bool(self.get_incoming(downstream))

        return upstream in self._graph and downstream in self._graph[upstream]

    def contains(self, node):
        """ Does graph contain node """

        if node in self._graph:
            return True

        if any(node in nodes for nodes in self._graph.values()):
            return True

        return False

    def get_outgoing(self, node=None):
        """
            Get all outgoing connections of node
            Passing no argument will get outgoing connections of all nodes
        """

        if node is None:
            return set(node for cxns in self._graph.values() for node in cxns)

        if node not in self._graph:
            return set()

        return self._graph[node]

    def get_incoming(self, node=None):
        """
            Get all incoming nodes of node
            Passing no argument will get outgoing nodes of all nodes
        """

        if node is None:
            return set(self._graph.keys())

        return set(incoming for incoming, cxns in self._graph.items() if node in cxns)

    def get_roots(self):
        """ Get roots, which are any nodes without incoming connections """

        return self.get_incoming() - self.get_outgoing()

    def get_sinks(self):
        """ Get sinks, which are any nodes without outgoing connections """
        return self.get_outgoing() - self.get_incoming()

    def get_nodes(self):
        """ Get all nodes in graph """

        # if a node is in the graph but has no outgoing connections, self._graph[node] will not exist
        return self.get_outgoing() | self.get_incoming()

    def get_contiguous_nodes(self, nodes):
        fringe = nodes
        closed = set()

        while fringe:
            n = fringe.pop()
            if n in closed:
                continue
            closed.add(n)
            for m in self.get_outgoing(n):
                fringe.append(m)

        return closed

    def is_contiguous(self):
        all_nodes = self.get_nodes()
        
        if not self._directed:
            closed = self.get_contiguous_nodes(self._graph.keys()[0])
        else:
            raise "not implemented"

        return all_nodes == closed

    # TODO: some sort of state to not re-do work each time we call this
    # may or may not be necessary for complex topologies
    # only implement caching if we have a performance issue
    # premature optimization is the root of all evil
    def shortest_path(self, source, target):
        """ Find the shortest path using Djikstra's algorithm """

        unvisited_nodes = self.get_incoming() & self.get_outgoing()
        dist = {node: float('inf') for node in unvisited_nodes}
        dist[source] = 0
        prev = dict()

        while len(unvisited_nodes) > 0:
            node = min(unvisited_nodes, key=dist.get)
            unvisited_nodes.remove(node)

            if node == target:
                break

            for neighbor in self.get_outgoing(node):
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

    def __init__(self, connections=[], metadata=None):
        super().__init__(connections, True, metadata)

    def clone(self):
        """ Create a clone of the object """

        return self.__class__([(node, service) for node, services in self._graph.items() for service in services], metadata=copy.deepcopy(self._metadata))

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

    def get_producers(self):
        return self.get_roots()

    def get_consumers(self):
        return self.get_sinks()

    def get_services(self):
        return self.get_incoming() & self.get_outgoing()


class Topology(Graph):
    """ Topology data structure, undirected graph """

    def get_dict(self):
        """ Returns a dictionary representation of the class """

        routers = []
        links = []
        for n in self.get_nodes():
            meta = self.get_metadata(n)
            meta["node"] = n
            routers.append(meta)

            for m in self.get_outgoing(n):
                # Check for existing link in reverse direction to avoid duplicates
                if any((link["from"] == m and link["to"] == n) or (link["from"] == n and link["to"] == m) for link in links):
                    continue

                edge_meta = self.get_edge_metadata(n, m)
                edge_meta["from"] = n
                edge_meta["to"] = m
                links.append(edge_meta)

        return {"router": routers, "link": links}

    def write_txt(self, file):
        # router
        router_fields = [ "comment", "y", "x", "mpi-partition" ]

        file.write("router\n")
        file.write("# ")
        file.write('\t'.join(["node"] + router_fields))
        file.write('\n')
        for node in self.get_nodes():
            file.write(node)
            file.write('\t')

            for field in router_fields:
                if field not in self.get_metadata(node):
                    break

                file.write(self.get_metadata(node)[field])
                file.write('\t')

            file.write('\n')

        # link
        link_fields = [ "capacity", "metric", "delay", "queue" ]

        file.write("\nlink\n")
        file.write("# ")
        file.write('\t'.join(["from", "to"] + link_fields))
        file.write('\n')
        links = []

        for n in self.get_nodes():
            for m in self.get_outgoing(n):
                if any((link[0] == n and link[1] == m) or (link[0] == m and link[1] == n) for link in links):
                    continue
                links.append((n, m))

                file.write(n)
                file.write('\t')
                file.write(m)
                file.write('\t')

                for field in link_fields:
                    if field not in self.get_edge_metadata(n, m):
                        break

                    file.write(self.get_edge_metadata(n, m)[field])
                    file.write('\t')

            file.write('\n')

    @classmethod
    def from_dict(cls, topo_dict, cs_size=None):
        topo = cls()
        routers = topo_dict["router"]
        links = topo_dict["link"]

        for link in links:
            topo.add(link["from"], link["to"])
            topo.update_edge_metadata(link["from"], link["to"], **link)

        for router in routers:
            if cs_size:
                link["cs-size"] = cs_size
            node = router.pop("node")
            topo.update_metadata(node, **router)

        return topo

    @classmethod
    def from_txt(cls, file, cs_size=None):
        topo = cls()
        section = ""

        line_num = 0

        for line in file.readlines():
            line_num += 1

            # remove trailing whitespace
            line = line.rstrip()

            # ignore empty lines
            if not line:
                continue

            # ignore comments
            if line.startswith('#'):
                continue

            # if label
            labels = [ "router", "link" ]
            if line in labels:
                section = line
                continue

            if not section:
                raise ValueError(f"line {line_num}: unexpected line in empty section")

            if section == "router":
                num_req_fields = 1
                fields = ["node", "comment", "y", "x", "mpi-partition"]
            elif section == "link":
                num_req_fields = 2
                fields = ["from", "to", "capacity", "metric", "delay", "queue"]

            values = line.split()
            if len(values) > len(fields):
                raise ValueError(f"line {line_num}: too many fields, maximum {len(fields)}, got {len(values)}")
            elif len(values) < num_req_fields:
                raise ValueError(f"line {line_num}: not enough fields, minimum {num_req_fields}, got {len(values)}")

            # TODO: validate contents of fields againt field-specific grammar
            entry = {field: value for field, value in zip(fields, values)}
            if section == "router":
                if cs_size:
                    entry["cs-size"] = cs_size
                node = entry.pop("node")
                topo.update_metadata(node, **entry)
            elif section == "link":
                topo.add(entry["from"], entry["to"])
                topo.update_edge_metadata(entry["from"], entry["to"], **entry)

        return topo

    def clone(self):
        """ Create a clone of the object """

        return self.__class__(
            [(node, service) for node, services in self._graph.items() for service in services], 
            metadata=copy.deepcopy(self._metadata), 
            edge_metadata=copy.deepcopy(self._edge_metadata)
        )

    def __init__(self, connections=[], metadata=None, edge_metadata=None):
        super().__init__(connections, False, metadata, edge_metadata)


class Hosting(Graph):
    """ Hosting data structure """

    def clone(self):
        """ Create a clone of the object """

        return self.__class__([(node, service) for node, services in self._graph.items() for service in services], metadata=copy.deepcopy(self._metadata))

    def __init__(self, connections=[], metadata=None):
        super().__init__(connections, True, metadata)
        self.get_hosts = self.get_incoming
        self.get_hosting = self.get_outgoing
        self.is_hosting = self.is_connected


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
        pattern = re.compile(r"^([a-zA-Z0-9_-]+)\s+([a-zA-Z0-9_-]+).*$")
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

# vim: tw=0 shiftwidth=4 expandtab tabstop=8 softtabstop=8 smarttab autoindent
