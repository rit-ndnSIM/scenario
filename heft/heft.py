#!/usr/bin/env python3

import json
import re
import math
import os
import glob
import csv
import argparse
import fnmatch
import time
import subprocess
import concurrent.futures
import multiprocessing

class EdgeScheduler:
    def __init__(self, json_data):
        self.nodes = []
        self.links = {}
        self.dag = {}
        self.tasks = []
        
        self.delay_matrix = {}
        self.comp_matrix = {} # comp_matrix[task][node] = time_in_ns
        self.energy_matrix = {} # energy_matrix[task][node] = joules
        self.quality_matrix = {} # quality_matrix[task][node] = quality in (0, 1]
        self.quality_tasks = set() # tasks whose hosting entries actually carry a quality
        
        # Cap on the number of re-ranking passes used by schedule_heft_iter_mod2()
        self.max_heft_iterations = 100

        # Budget for schedule_exhaustive(): maximum number of candidate task
        # placements the search may evaluate before it gives up on proving optimality.
        self.max_exhaustive_evaluations = 10_000_000

        # A-Beam beam width k: how many branches survive the prune at each DAG step.
        # An int is a constant width; a callable k(step_index, total_steps) -> int lets
        # the width vary with depth (e.g. wider early, narrower near the end).
        self.abeam_beam_width = 10

        # A step's hosting combinations are enumerated exactly while their product stays
        # at or below this; above it the step is expanded service by service with the
        # beam applied inside the step too. See schedule_abeam().
        self.abeam_max_step_combinations = 20_000

        # Hard ceiling on how many branches A-Beam may hold at once. Enumerating a step
        # exactly multiplies the live branch list by the beam width, so memory grows as
        # beam_width x step_combinations - at beam_width 1000 that reached ~2.5 GB per
        # worker and got processes OOM-killed. When an expansion would exceed this, the
        # branch list is trimmed to the best f first and the step is no longer exact.
        # Budget roughly 2 KB per branch PER WORKER PROCESS, so the default is about
        # 400 MB per worker; lower it if you run many workers on a small machine.
        # The default leaves beam widths up to 10 untouched (10 x 20_000 = 200_000).
        self.abeam_max_open_branches = 200_000

        # Objective weights. The A_Beam<k>_LEQ schemes minimize a weighted sum of three
        # normalized terms - latency, quality and energy (see evaluate_schedule() for how
        # each is normalized so that the weights are comparable); the A_Beam<k>_L schemes
        # ignore these and optimize latency alone. evaluate_schedule() also uses them to
        # score every scheme's Cost column. Only their ratios affect which placement wins.
        # All equal for now; to be tuned per context later.
        self.weight_latency = 1.0 / 3.0
        self.weight_quality = 1.0 / 3.0
        self.weight_energy = 1.0 / 3.0

        self._parse_json(json_data)
        self._compute_all_pairs_shortest_path()
        
    def _parse_time_to_ns(self, time_str):
        """Converts strings like '1ms', '100us' into nanoseconds."""
        match = re.match(r"([\d\.]+)([a-zA-Z]+)", time_str)
        if not match: return 0
        val = float(match.group(1))
        unit = match.group(2).lower()
        if unit == "ms": return int(val * 1_000_000)
        elif unit == "us": return int(val * 1_000)
        elif unit == "ns": return int(val)
        elif unit == "s": return int(val * 1_000_000_000)
        return int(val)

    def _parse_json(self, data):
        # Parse Routers (Nodes)
        self.nodes = [r['node'] for r in data.get('router', [])]
        
        # Parse Links and Delays
        for link in data.get('link', []):
            u, v = link['from'], link['to']
            delay_ns = self._parse_time_to_ns(link['delay'])
            if u not in self.links: self.links[u] = {}
            if v not in self.links: self.links[v] = {}
            self.links[u][v] = delay_ns
            self.links[v][u] = delay_ns # Assuming bidirectional
            
        # Parse DAG
        dag_data = data.get('dag', {}).get('dag1', {})
        self.tasks = list(dag_data.keys())
        
        # Add tasks that are targets but not sources in the dict
        for targets in dag_data.values():
            for t in targets.keys():
                if t not in self.tasks:
                    self.tasks.append(t)
                    
        self.dag = {t: [] for t in self.tasks}
        self.dag_parents = {t: [] for t in self.tasks}
        
        for parent, children in dag_data.items():
            for child in children.keys():
                self.dag[parent].append(child)
                self.dag_parents[child].append(parent)

        # Parse Router Hosting (Computation, Energy and Quality Matrices)
        self.comp_matrix = {t: {} for t in self.tasks}
        self.energy_matrix = {t: {} for t in self.tasks}
        self.quality_matrix = {t: {} for t in self.tasks}
        for rh in data.get('routerHosting', []):
            svc = rh['service']
            if svc in self.comp_matrix:
                self._add_hosting(svc, rh)

        # Fallback for instance-suffixed hosting names. The DAG names the consumer
        # sink task "/consumer", but routerHosting pins it under its instance name
        # ("/consumer1", "/consumer2", ...). Match any still-unhosted task against a
        # routerHosting service that is the task name followed only by digits, so
        # "/consumer" picks up "/consumer1" and lands on its pinned router.
        for task, hosts in self.comp_matrix.items():
            if hosts:
                continue
            for rh in data.get('routerHosting', []):
                svc = rh['service']
                if re.fullmatch(re.escape(task) + r"\d+", svc):
                    self._add_hosting(task, rh)

    def _add_hosting(self, task, rh):
        """Records one routerHosting entry for `task`. A missing field falls back to the
        neutral value for its objective: makespanNS 0, energy 0 J (adds nothing to the
        total), quality 1.0 (multiplies the total quality by 1). The consumer entry, for
        instance, carries none of the three."""
        rtr = rh['router']
        self.comp_matrix[task][rtr] = rh.get('makespanNS', 0)
        self.energy_matrix[task][rtr] = rh.get('energy', 0.0)
        self.quality_matrix[task][rtr] = rh.get('quality', 1.0)
        if 'quality' in rh:
            self.quality_tasks.add(task)

    def _compute_all_pairs_shortest_path(self):
        """Floyd-Warshall to get shortest path delay between all routers."""
        self.delay_matrix = {u: {v: math.inf for v in self.nodes} for u in self.nodes}
        for u in self.nodes:
            self.delay_matrix[u][u] = 0
            if u in self.links:
                for v, delay in self.links[u].items():
                    self.delay_matrix[u][v] = delay
                    
        for k in self.nodes:
            for i in self.nodes:
                for j in self.nodes:
                    if self.delay_matrix[i][j] > self.delay_matrix[i][k] + self.delay_matrix[k][j]:
                        self.delay_matrix[i][j] = self.delay_matrix[i][k] + self.delay_matrix[k][j]

        # Compute average communication cost for rank calculations
        total_delay, count = 0, 0
        for i in self.nodes:
            for j in self.nodes:
                if i != j and self.delay_matrix[i][j] != math.inf:
                    total_delay += self.delay_matrix[i][j]
                    count += 1
        self.avg_comm = total_delay / count if count > 0 else 0

    def _get_specific_avg_comm(self, parent_task, child_task):
        """Calculates the average link delay only between eligible hosting routers."""
        
        # Check if we already calculated this specific edge
        edge_key = (parent_task, child_task)
        if not hasattr(self, '_edge_comm_cache'):
            self._edge_comm_cache = {}
        if edge_key in self._edge_comm_cache:
            return self._edge_comm_cache[edge_key]

        parent_nodes = list(self.comp_matrix.get(parent_task, {}).keys())
        child_nodes = list(self.comp_matrix.get(child_task, {}).keys())

        # If either task has no eligible nodes, communication is effectively 0/impossible
        if not parent_nodes or not child_nodes:
            return 0

        total_delay = 0
        count = 0
        
        # The M x M loop
        for p_node in parent_nodes:
            for c_node in child_nodes:
                delay = self.delay_matrix[p_node][c_node]
                if delay != math.inf:
                    total_delay += delay
                    count += 1

        # Cache and return the result
        avg_delay = total_delay / count if count > 0 else 0
        self._edge_comm_cache[edge_key] = avg_delay
        return avg_delay

    def _get_avg_comp(self, task):
        """Calculates average computation cost strictly across eligible nodes."""
        valid_costs = self.comp_matrix.get(task, {}).values()
        if not valid_costs:
            return 0 
        return sum(valid_costs) / len(valid_costs)


    # helper functions for calculating SLR (service latency ratio)
    def _get_min_comp(self, task):
        """Calculates the minimum computation cost strictly across eligible nodes."""
        valid_costs = self.comp_matrix.get(task, {}).values()
        if not valid_costs:
            return 0 
        return min(valid_costs)
    def calculate_cp_min(self):
        """Calculates CP_min: the longest path in the DAG using only minimum computation costs."""
        rank_min = {}
        
        # Recursive function to find the longest path to an exit node
        def calc_rank_min(task):
            if task in rank_min: return rank_min[task]
            max_succ = 0
            for succ in self.dag.get(task, []):
                max_succ = max(max_succ, calc_rank_min(succ))
            
            rank_min[task] = self._get_min_comp(task) + max_succ
            return rank_min[task]

        # Compute for all tasks
        for t in self.tasks:
            calc_rank_min(t)
            
        # CP_min is the maximum value found (the length of the critical path from entry to exit)
        if not rank_min:
            return 0
        return max(rank_min.values())
    def calculate_cp_min_comm(self):
        """Like calculate_cp_min(), but also charges every edge its cheapest possible
        transfer (_get_specific_min_comm): the longest path in the DAG where each task
        costs its minimum makespan and each hop its minimum link delay. Every chain in
        the DAG must be executed in order, and no placement can make a task or a transfer
        cheaper than its minimum, so this is still a true lower bound on the makespan of
        ANY schedule - just a much tighter one when communication matters."""
        placeable_set = {t for t in self.tasks if self.comp_matrix.get(t)}
        rank = {}

        def calc(task):
            if task in rank: return rank[task]
            longest_succ = 0
            for succ in self.dag.get(task, []):
                if succ in placeable_set:
                    longest_succ = max(longest_succ,
                                       self._get_specific_min_comm(task, succ) + calc(succ))
            rank[task] = self._get_min_comp(task) + longest_succ
            return rank[task]

        for t in self.tasks:
            if t in placeable_set:
                calc(t)
        return max(rank.values()) if rank else 0

    def get_makespan(self, schedule):
        """Returns the total makespan of a given schedule."""
        if not schedule:
            return 0
        return max(end for node, start, end in schedule.values())


    # --- MULTI-OBJECTIVE EVALUATION (latency, quality, energy) ---

    def _objective_refs(self):
        """Per-scenario reference ("ideal") value for each objective, used to turn the
        three raw objectives into comparable, dimensionless terms. Computed once.

          latency_ref  : calculate_cp_min_comm() - the critical path with every task at
                         its cheapest makespan AND every edge at its cheapest transfer; no
                         schedule can finish sooner. CP_min alone is far too loose for
                         this: it ignores communication, which on high-CCR scenarios is
                         most of the makespan, so even the OPTIMAL schedule sits at a
                         median 2.7x (p90 ~23x) CP_min - which would inflate the latency
                         term by that factor and swamp quality and energy regardless of
                         the weights. With communication included the optimum sits at a
                         median ~1.4x.
          energy_ref   : the sum of every placeable task's cheapest energy; the lowest
                         total energy any placement can reach.
          quality_best : each task's best quality over its eligible nodes; their
                         product is the highest total quality any placement can reach.
          n_quality    : how many placeable tasks actually carry a quality value."""
        if hasattr(self, '_objective_refs_cache'):
            return self._objective_refs_cache

        placeable = [t for t in self.tasks if self.comp_matrix.get(t)]
        latency_ref = self.calculate_cp_min_comm()
        self._objective_refs_cache = {
            "latency_ref": latency_ref if latency_ref > 0 else 1,
            "energy_ref": sum(min(self.energy_matrix[t].values()) for t in placeable),
            "quality_best": {t: max(self.quality_matrix[t].values()) for t in placeable},
            "n_quality": sum(1 for t in placeable if t in self.quality_tasks),
        }
        return self._objective_refs_cache

    def _resolve_weights(self, weights=None):
        """The objective weights to use: `weights` if given (a dict with any of the keys
        "latency", "energy", "quality"; missing keys fall back to 0), otherwise the
        configured self.weight_latency / self.weight_energy / self.weight_quality."""
        if weights is None:
            return {"latency": self.weight_latency,
                    "energy": self.weight_energy,
                    "quality": self.weight_quality}
        return {k: weights.get(k, 0.0) for k in ("latency", "energy", "quality")}

    def _energy_quality_cost(self, task, node, weights=None):
        """The weighted energy + quality cost of hosting `task` on `node` - its share of
        w_energy * energy_term + w_quality * quality_term in evaluate_schedule(), minus
        the energy term's constant offset. Both terms are additive per service, so a
        schedule's energy + quality cost is exactly the sum of this over its services.
        An objective whose weight is 0 is skipped outright."""
        w = self._resolve_weights(weights)
        refs = self._objective_refs()
        cost = 0.0
        if w["energy"] and refs["energy_ref"] > 0:
            cost += w["energy"] * self.energy_matrix[task][node] / refs["energy_ref"]
        if w["quality"] and refs["n_quality"] > 0 and task in self.quality_tasks:
            q = self.quality_matrix[task][node]
            if q <= 0:
                return math.inf # a zero-quality service zeroes the whole product
            cost += w["quality"] * math.log(refs["quality_best"][task] / q) / refs["n_quality"]
        return cost

    def _cost_offset(self, weights=None):
        """Constant subtracted so the combined cost is 0 at the ideal: the "- 1" of the
        energy ratio term (the latency ratio's "- 1" is applied where it is computed)."""
        w = self._resolve_weights(weights)
        return w["energy"] if self._objective_refs()["energy_ref"] > 0 else 0.0

    def evaluate_schedule(self, schedule):
        """Scores a schedule on all three objectives and combines them into one cost.

        Raw values:
          latency : the makespan (ns) - service makespans plus link delays along the
                    schedule's longest path
          energy  : the SUM of every placed service's energy (J)
          quality : the PRODUCT of every placed service's quality, in (0, 1]; 1 only if
                    every service runs at full quality, lower for any degraded one

        The raw values live on wildly different scales (~10^7 ns, a few tens of J, and a
        product that can be ~10^-2), so weighting them directly would let latency swamp
        the other two no matter the weights. Each is instead turned into a dimensionless
        term that is 0 at its ideal and grows as the schedule gets worse:

          latency_term = latency / latency_ref - 1        (latency_ref includes comms)
          energy_term  = energy / energy_ref - 1
          quality_term = ln(quality_best_total / quality) / n_quality

        so a schedule 10% worse than ideal in any one objective scores ~0.1 in that term.

        quality_term is the per-service (geometric-mean) quality loss. The product shrinks
        geometrically as the DAG grows, so without dividing by the number of services
        quality would automatically outweigh latency and energy on bigger DAGs. Being a
        log it is additive per service, which A-Beam's heuristic relies on, and it is a
        monotone function of the product, so ranking by it IS ranking by total quality.

          cost = w_latency * latency_term + w_quality * quality_term + w_energy * energy_term

        Returns a dict with the raw values, the three terms and the cost."""
        refs = self._objective_refs()
        latency = self.get_makespan(schedule)
        energy = 0.0
        quality = 1.0
        quality_term = 0.0
        for task, (node, _start, _end) in schedule.items():
            energy += self.energy_matrix[task][node]
            q = self.quality_matrix[task][node]
            quality *= q
            if refs["n_quality"] > 0 and task in self.quality_tasks:
                quality_term = (math.inf if q <= 0 else
                                quality_term + math.log(refs["quality_best"][task] / q) / refs["n_quality"])

        latency_term = latency / refs["latency_ref"] - 1
        energy_term = energy / refs["energy_ref"] - 1 if refs["energy_ref"] > 0 else 0.0
        cost = (self.weight_latency * latency_term
                + self.weight_quality * quality_term
                + self.weight_energy * energy_term)
        return {"latency": latency, "energy": energy, "quality": quality,
                "latency_term": latency_term, "quality_term": quality_term,
                "energy_term": energy_term, "cost": cost}


    # --- ALGORITHMS ---

    def compute_ranks(self):
        self.rank_u = {}
        self.rank_d = {}
        
        # Upward Rank (computed from exit nodes to entry nodes)
        def calc_upward(task):
            if task in self.rank_u: return self.rank_u[task]
            max_succ = 0
            for succ in self.dag[task]:
                max_succ = max(max_succ, self.avg_comm + calc_upward(succ))    # this uses the average of all links in the entire topology
            self.rank_u[task] = self._get_avg_comp(task) + max_succ
            return self.rank_u[task]

        for t in self.tasks: calc_upward(t)
            
        # Downward Rank (computed from entry nodes to exit nodes)
        def calc_downward(task):
            if task in self.rank_d: return self.rank_d[task]
            if not self.dag_parents[task]: 
                self.rank_d[task] = 0
                return 0
            max_pred = 0
            for pred in self.dag_parents[task]:
                max_pred = max(max_pred, calc_downward(pred) + self._get_avg_comp(pred) + self.avg_comm)
            self.rank_d[task] = max_pred
            return self.rank_d[task]

        for t in self.tasks: calc_downward(t)


    def compute_ranks_rlc(self):
        """Uses the average of all links between two specific services, rather than average of all links in the entire topology.
        i.e.: uses mean shortest-path delay over only the routers that can actually host the parent × routers that can host the child, computed per edge"""
        
        self.rank_u = {}
        self.rank_d = {}
        
        # Upward Rank (computed from exit nodes to entry nodes)
        def calc_upward(task):
            if task in self.rank_u: return self.rank_u[task]
            max_succ = 0
            for succ in self.dag[task]:
                specific_comm = self._get_specific_avg_comm(task, succ)         # this uses the average of all links between two specific services
                max_succ = max(max_succ, specific_comm + calc_upward(succ))
            self.rank_u[task] = self._get_avg_comp(task) + max_succ
            return self.rank_u[task]

        for t in self.tasks: calc_upward(t)
            
        # Downward Rank (computed from entry nodes to exit nodes)
        def calc_downward(task):
            if task in self.rank_d: return self.rank_d[task]
            if not self.dag_parents[task]: 
                self.rank_d[task] = 0
                return 0
            max_pred = 0
            for pred in self.dag_parents[task]:
                max_pred = max(max_pred, calc_downward(pred) + self._get_avg_comp(pred) + self.avg_comm)
            self.rank_d[task] = max_pred
            return self.rank_d[task]

        for t in self.tasks: calc_downward(t)



    def compute_ranks_ic(self):
        """Like RLC ranks, but each task is also charged its most expensive input transfer cost."""
        self.rank_u = {}
        self.rank_d = {}

        def max_input_comm(task):
            # Highest incoming (parent -> task) communication cost of all in-edges
            parents = self.dag_parents.get(task, [])
            if not parents:
                return 0
            return max(self._get_specific_avg_comm(p, task) for p in parents)

        # Upward Rank (computed from exit nodes to entry nodes)
        def calc_upward(task):
            if task in self.rank_u: return self.rank_u[task]
            max_succ = 0
            for succ in self.dag[task]:
                specific_comm = self._get_specific_avg_comm(task, succ)         # this uses the average of all links between two specific services
                max_succ = max(max_succ, specific_comm + calc_upward(succ))
            # mod1: add the highest input cost of this task on top of the RLC rank
            self.rank_u[task] = self._get_avg_comp(task) + max_input_comm(task) + max_succ
            return self.rank_u[task]

        for t in self.tasks: calc_upward(t)

        # Downward Rank (computed from entry nodes to exit nodes)
        def calc_downward(task):
            if task in self.rank_d: return self.rank_d[task]
            if not self.dag_parents[task]:
                self.rank_d[task] = 0
                return 0
            max_pred = 0
            for pred in self.dag_parents[task]:
                max_pred = max(max_pred, calc_downward(pred) + self._get_avg_comp(pred) + self.avg_comm)
            self.rank_d[task] = max_pred
            return self.rank_d[task]

        for t in self.tasks: calc_downward(t)

    def compute_ranks_iter(self, schedule=None):
        """RLC ranks. If a schedule is given, every task that was placed uses the
        makespan on its ASSIGNED node instead of the average over all eligible nodes."""
        self.rank_u = {}
        self.rank_d = {}

        def comp_cost(task):
            if schedule and task in schedule:
                assigned_node = schedule[task][0]
                # Fall back to the average if the assigned node somehow has no entry
                return self.comp_matrix.get(task, {}).get(assigned_node, self._get_avg_comp(task))
            return self._get_avg_comp(task)

        # Upward Rank (computed from exit nodes to entry nodes)
        def calc_upward(task):
            if task in self.rank_u: return self.rank_u[task]
            max_succ = 0
            for succ in self.dag[task]:
                specific_comm = self._get_specific_avg_comm(task, succ)         # this uses the average of all links between two specific services
                max_succ = max(max_succ, specific_comm + calc_upward(succ))
            self.rank_u[task] = comp_cost(task) + max_succ
            return self.rank_u[task]

        for t in self.tasks: calc_upward(t)

        # Downward Rank (computed from entry nodes to exit nodes)
        def calc_downward(task):
            if task in self.rank_d: return self.rank_d[task]
            if not self.dag_parents[task]:
                self.rank_d[task] = 0
                return 0
            max_pred = 0
            for pred in self.dag_parents[task]:
                max_pred = max(max_pred, calc_downward(pred) + comp_cost(pred) + self.avg_comm)
            self.rank_d[task] = max_pred
            return self.rank_d[task]

        for t in self.tasks: calc_downward(t)




    def schedule_heft(self):
        self.compute_ranks()
        # Sort by upward rank descending. rank_u(parent) is always greater than
        # rank_u(child), so this ordering is inherently topological.
        ordered_tasks = sorted(self.tasks, key=lambda x: self.rank_u[x], reverse=True)
        return self._place_tasks(ordered_tasks)

    def schedule_heft_rlc(self):
        self.compute_ranks_rlc()
        # Sort by upward rank descending
        ordered_tasks = sorted(self.tasks, key=lambda x: self.rank_u[x], reverse=True)
        return self._place_tasks(ordered_tasks)

    def schedule_heft_ic(self):
        self.compute_ranks_ic()
        # Sort by upward rank descending
        ordered_tasks = sorted(self.tasks, key=lambda x: self.rank_u[x], reverse=True)
        return self._place_tasks(ordered_tasks)

    def schedule_heft_iter(self, max_iterations=None):
        """Runs RLC, then re-ranks using the computation cost each task actually got on
        its assigned node, re-placing until the task priority order stops changing.
        Returns (schedule, iterations) where iterations counts the placement passes."""
        if max_iterations is None:
            max_iterations = self.max_heft_iterations

        # Pass 1: no schedule yet, so this is exactly the RLC ranking
        self.compute_ranks_iter()
        ordered_tasks = sorted(self.tasks, key=lambda x: self.rank_u[x], reverse=True)
        schedule = self._place_tasks(ordered_tasks)
        iterations = 1

        while iterations < max_iterations:
            # Re-rank with the makespan of each task on the node it was placed on
            self.compute_ranks_iter(schedule)
            new_order = sorted(self.tasks, key=lambda x: self.rank_u[x], reverse=True)

            if new_order == ordered_tasks:
                break   # priorities did not change -> converged, the schedule would repeat

            ordered_tasks = new_order
            schedule = self._place_tasks(ordered_tasks)
            iterations += 1

        return schedule, iterations

    def schedule_exhaustive(self, max_evaluations=None):
        """Exhaustively searches EVERY combination of task placements AND every valid
        ordering of tasks that share a node, and returns the schedule with the lowest
        makespan.

        A fixed, single topological order (as a prior version of this search used) is not
        enough: two tasks with no dependency between them can still be forced onto the
        same node, and if the search always ran them in one predetermined order, it would
        never consider slotting the later-in-order-but-independent one into an earlier
        idle gap on that node. That is a real gap, not a theoretical one - CPOP's own
        (differently tie-broken) topological order does exactly this kind of reordering
        and has been observed to beat this search's old fixed-order result by finding a
        schedule this search never even visited. So at every step the search branches over
        BOTH which currently-ready task to place next AND which node to place it on.

        "Ready" means every placeable predecessor has already been placed - predecessors
        with no eligible node anywhere are skipped, matching _earliest_start()/_place_tasks().
        Every ready task is branched on, with no symmetry breaking by node-contention
        group. Grouping tasks that can never share a node and forcing a fixed order
        between groups LOOKS safe and is not: a cross-group dependency can hold one group
        member back until after a group it was forced to follow has been placed, so the
        interleaving where it runs first on their shared node is never explored - the
        exact class of omission this search exists to avoid.

        The search is a depth-first branch and bound rather than a flat product loop: a
        partial placement is abandoned as soon as its makespan lower bound is no better
        than the best complete schedule found so far. Ordering the candidate nodes
        cheapest-first, and seeding the incumbent with the best schedule any heuristic
        produces, makes that pruning bite early.

        Sets self.exhaustive_evaluations (placements examined) and self.exhaustive_complete
        (False if the evaluation budget ran out before the search finished, in which case
        the result is the best schedule found rather than a proven optimum)."""
        if max_evaluations is None:
            max_evaluations = self.max_exhaustive_evaluations

        chain_after = self._min_chain_after()

        # Tasks with no eligible node cannot be placed at all (same as the heuristics).
        # Kept as a list in self.tasks order: the branch order decides which schedule is
        # returned when several tie on makespan, and iterating a set of task names would
        # make that depend on Python's per-process string hash seed.
        placeable = []
        placeable_set = set()
        for task in self.tasks:
            if self.comp_matrix.get(task):
                placeable.append(task)
                placeable_set.add(task)
            else:
                print(f"Warning: Task {task} could not be scheduled (No eligible nodes).")

        # Only placeable predecessors gate readiness or contribute an arrival delay - an
        # unplaceable predecessor is skipped entirely, exactly like _earliest_start().
        placeable_parents = {t: [p for p in self.dag_parents.get(t, []) if p in placeable_set] for t in placeable}
        placeable_children = {t: [] for t in placeable}
        for t in placeable:
            for p in placeable_parents[t]:
                placeable_children[p].append(t)

        # Cheapest node first: finds good schedules early, which prunes harder
        options = {t: sorted(self.comp_matrix[t].items(), key=lambda kv: kv[1]) for t in placeable}

        combinations = 1
        for opt in options.values():
            combinations *= len(opt)
        self.exhaustive_combinations = combinations
        self.exhaustive_evaluations = 0
        self.exhaustive_complete = True

        # Seed the incumbent with the best schedule every heuristic can produce. Two
        # reasons: a tight incumbent prunes the very first branches hard, and if the
        # evaluation budget runs out the result returned is still no worse than any
        # heuristic - without this, a truncated search can report a makespan that one of
        # the heuristics it is supposed to be the ground truth for already beat.
        best = {"makespan": math.inf, "schedule": None}
        for seed_schedule in (self.schedule_heft(),
                              self.schedule_heft_rlc(),
                              self.schedule_heft_ic(),
                              self.schedule_heft_iter()[0],
                              self.schedule_cpop()):
            if not seed_schedule:
                continue
            seed_makespan = self.get_makespan(seed_schedule)
            if seed_makespan < best["makespan"]:
                best["makespan"] = seed_makespan
                best["schedule"] = seed_schedule

        avail = {n: 0 for n in self.nodes}
        placement = {} # task -> (node, start_time, end_time)
        indegree = {t: len(placeable_parents[t]) for t in placeable}
        n_tasks = len(placeable)

        def search(ready, makespan_so_far, bound_so_far):
            if len(placement) == n_tasks:
                if makespan_so_far < best["makespan"]:
                    best["makespan"] = makespan_so_far
                    best["schedule"] = dict(placement)
                return

            for task in list(ready):
                preds = placeable_parents[task]
                tail = chain_after[task]

                for node, comp_time in options[task]:
                    if self.exhaustive_evaluations >= max_evaluations:
                        self.exhaustive_complete = False
                        return
                    self.exhaustive_evaluations += 1

                    # Earliest start on this node given everything placed so far
                    est = avail[node]
                    for pred in preds:
                        pred_node, _, pred_aft = placement[pred]
                        arrival = pred_aft + self.delay_matrix[pred_node][node]
                        if arrival > est:
                            est = arrival

                    eft = est + comp_time
                    makespan = makespan_so_far if makespan_so_far > eft else eft

                    # Lower bound: nothing already committed can shrink, and every successor
                    # chain of this task still has to run after it finishes
                    bound = bound_so_far
                    if makespan > bound: bound = makespan
                    if eft + tail > bound: bound = eft + tail

                    if bound >= best["makespan"]:
                        continue # this branch cannot beat the incumbent - prune it

                    previous_avail = avail[node]
                    avail[node] = eft
                    placement[task] = (node, est, eft)
                    ready.remove(task)
                    newly_ready = []
                    for child in placeable_children[task]:
                        indegree[child] -= 1
                        if indegree[child] == 0:
                            ready.append(child)
                            newly_ready.append(child)

                    search(ready, makespan, bound)

                    for child in newly_ready:
                        ready.remove(child)
                    # Every child's indegree decrement above must be undone on backtrack,
                    # not just the ones that crossed zero - otherwise a child sharing this
                    # task as one of several parents keeps a stale (too-low) indegree the
                    # next time this task is tried on a different node, and can become
                    # "ready" before all of its real predecessors are placed.
                    for child in placeable_children[task]:
                        indegree[child] += 1
                    ready.append(task)
                    del placement[task]
                    avail[node] = previous_avail

                    if not self.exhaustive_complete:
                        return

        initial_ready = [t for t in placeable if indegree[t] == 0]
        search(initial_ready, 0, 0)

        return best["schedule"]

    def schedule_cpop(self):
        self.compute_ranks()
        priority = {t: self.rank_u[t] + self.rank_d[t] for t in self.tasks}
        
        # Identify Critical Path
        entry_nodes = [t for t in self.tasks if not self.dag_parents[t]]
        cp_node = max(entry_nodes, key=lambda x: priority.get(x, 0))
        critical_path = [cp_node]
        
        while self.dag.get(cp_node):
            cp_node = max(self.dag[cp_node], key=lambda x: priority.get(x, 0))
            critical_path.append(cp_node)
            
        # Select Critical Path Processor (minimizes sum of CP task computation times for tasks it CAN run)
        best_cp_proc, min_cp_cost = None, math.inf
        for node in self.nodes:
            cost = 0
            capable = False
            for t in critical_path:
                if node in self.comp_matrix.get(t, {}):
                    cost += self.comp_matrix[t][node]
                    capable = True
            if capable and cost < min_cp_cost:
                min_cp_cost = cost
                best_cp_proc = node

        # Walk the DAG in topological order, breaking ties by CPOP priority, rather than
        # sorting on priority alone. rank_u + rank_d is CONSTANT along the critical path
        # by construction, so a plain priority sort leaves the whole critical path tied
        # and lets floating-point noise order a task ahead of its own ancestors - which
        # silently drops that dependency and reports an impossibly low makespan.
        ordered_tasks = self._topological_order(priority)
        avail = {n: 0 for n in self.nodes}
        schedule = {}

        for task in ordered_tasks:
            best_node, min_eft, best_est = None, math.inf, 0

            # Restrict to CP Processor if it's a CP task AND the CP processor is eligible to run it
            eligible_nodes = list(self.comp_matrix.get(task, {}).keys())
            if task in critical_path and best_cp_proc in eligible_nodes:
                target_nodes = [best_cp_proc]
            else:
                target_nodes = eligible_nodes

            for node in target_nodes:
                est = self._earliest_start(task, node, schedule, avail)

                comp_time = self.comp_matrix[task][node]
                eft = est + comp_time

                if eft < min_eft:
                    min_eft = eft
                    best_est = est
                    best_node = node

            if best_node is not None:
                schedule[task] = (best_node, best_est, min_eft)
                avail[best_node] = min_eft
            else:
                print(f"Warning: Task {task} could not be scheduled (No eligible nodes).")
            
        return schedule


    # --- A-Beam: step-wise beam search ordered by f = g + h ---

    def _dag_steps(self):
        """Splits the DAG into "steps" (ASAP levels): a task sits in the earliest step
        that is still later than every one of its predecessors, so step(t) = 0 when t has
        no placeable parent and 1 + max(step(parents)) otherwise. The number of steps is
        therefore the length of the longest dependency chain in the DAG.

        Because an edge always increases the level, no two tasks in the same step depend
        on each other - they are mutually independent and could in principle run in
        parallel. Unplaceable tasks (no eligible node anywhere) are left out, matching
        _earliest_start()/_place_tasks(). Tasks keep self.tasks order inside a step so the
        search is deterministic."""
        level = {}
        placeable_set = {t for t in self.tasks if self.comp_matrix.get(t)}

        def calc_level(task):
            if task in level: return level[task]
            parents = [p for p in self.dag_parents.get(task, []) if p in placeable_set]
            level[task] = 0 if not parents else 1 + max(calc_level(p) for p in parents)
            return level[task]

        for t in self.tasks:
            if t in placeable_set:
                calc_level(t)

        if not level:
            return []
        steps = [[] for _ in range(max(level.values()) + 1)]
        for t in self.tasks:
            if t in placeable_set:
                steps[level[t]].append(t)
        return steps

    def _get_specific_min_comm(self, parent_task, child_task):
        """Cheapest possible transfer for this edge: the MINIMUM shortest-path delay over
        the routers that can host the parent x the routers that can host the child.

        _get_specific_avg_comm() returns an average, which can overshoot the delay an
        actual placement pays. The A-Beam heuristic needs a value no placement can ever
        beat, so it uses this minimum instead."""
        edge_key = (parent_task, child_task)
        if not hasattr(self, '_edge_min_comm_cache'):
            self._edge_min_comm_cache = {}
        if edge_key in self._edge_min_comm_cache:
            return self._edge_min_comm_cache[edge_key]

        parent_nodes = list(self.comp_matrix.get(parent_task, {}).keys())
        child_nodes = list(self.comp_matrix.get(child_task, {}).keys())
        if not parent_nodes or not child_nodes:
            return 0

        best = math.inf
        for p_node in parent_nodes:
            for c_node in child_nodes:
                delay = self.delay_matrix[p_node][c_node]
                if delay < best:
                    best = delay
        if best == math.inf:
            best = 0
        self._edge_min_comm_cache[edge_key] = best
        return best

    def compute_ranks_min(self):
        """Admissible variant of the HEFT-RLC upward rank, used as the A-Beam
        heuristic h. Three changes make it a strict lower bound on the time still needed
        from the moment a task starts:

          * every task is charged its CHEAPEST computation cost over its eligible nodes
            (_get_min_comp) instead of the average,
          * every edge is charged its CHEAPEST possible transfer
            (_get_specific_min_comm) instead of the average, and
          * a task takes the MINIMUM over its successors instead of the maximum.

        No real placement can finish the sub-DAG below a task faster than this, so
        f = g + h never overestimates the true makespan and the ranking stays
        admissible.

        Taking the MAXIMUM over successors instead would also be admissible, and makes
        individual ranks larger - but it cannot change A-Beam at all, so it is not
        offered. _abeam_latency_bound() maximizes est_lb(u) + rank(u) over EVERY unplaced
        task u, and every descendant of an unplaced task is itself unplaced. For u's
        longest downstream chain ending at an exit task w, the forward est_lb pass gives
        est_lb(w) + min_comp(w) >= est_lb(u) + max-rank(u), and an exit task's rank is
        min_comp(w) under either rule. So the maximum over tasks already recovers the
        longest chain, and the bound is identical either way (verified: identical results
        on 400 scenarios x 6 beam widths, and on 8,000 random partial placements)."""
        self.rank_min_u = {}
        placeable_set = {t for t in self.tasks if self.comp_matrix.get(t)}

        def calc_upward(task):
            if task in self.rank_min_u: return self.rank_min_u[task]
            successors = [s for s in self.dag.get(task, []) if s in placeable_set]
            if successors:
                tail = min(self._get_specific_min_comm(task, s) + calc_upward(s)
                           for s in successors)
            else:
                tail = 0
            self.rank_min_u[task] = self._get_min_comp(task) + tail
            return self.rank_min_u[task]

        for t in self.tasks:
            if t in placeable_set:
                calc_upward(t)

    def _abeam_latency_bound(self, placement, makespan, pending):
        """The latency part of A-Beam's f = g + h for a partial placement, in ns.

        g is the makespan already committed by the tasks in `placement`. h is the extra
        time still unavoidably needed on top of g, so g + h collapses to

            max( g, max over unplaced u of ( est_lb(u) + rank_min_u(u) ) )

        which is the right form for a makespan (a max-metric, not a sum: work still to
        come can overlap with work already scheduled, so literally adding the two would
        overestimate and break admissibility). The result is a lower bound on the final
        makespan of any completion of this placement.

        est_lb(u) is a forward lower bound on when u could possibly start - each
        predecessor's earliest finish plus that edge's cheapest transfer, with already
        placed predecessors contributing their real finish time. `pending` must be in
        topological order."""
        finish_lb = {t: entry[2] for t, entry in placement.items()}
        f = makespan

        for task in pending:
            est = 0
            for pred in self.dag_parents.get(task, []):
                if pred not in finish_lb:
                    continue # unplaceable predecessor, skipped like everywhere else
                arrival = finish_lb[pred] + self._get_specific_min_comm(pred, task)
                if arrival > est:
                    est = arrival
            finish_lb[task] = est + self._get_min_comp(task)
            chain_end = est + self.rank_min_u[task]
            if chain_end > f:
                f = chain_end

        return f

    def schedule_abeam_l(self, beam_width=None, max_step_combinations=None, heuristic="rank"):
        """A_Beam<k>_L: A-Beam optimizing latency (makespan) alone - schedule_abeam() with
        weights latency 1, energy 0, quality 0. This is the A-Beam from before energy and
        quality were added: with those weights f is a positive rescaling of the old
        makespan-only f, so every branch ranks exactly as it used to and the placements
        are identical (verified on 150 scenarios)."""
        return self.schedule_abeam(beam_width, max_step_combinations,
                                   weights={"latency": 1.0, "energy": 0.0, "quality": 0.0},
                                   heuristic=heuristic)

    def schedule_abeam_leq(self, beam_width=None, max_step_combinations=None, heuristic="rank"):
        """A_Beam<k>_LEQ: A-Beam optimizing the weighted Latency + Energy + Quality cost,
        with the configured self.weight_latency / weight_energy / weight_quality."""
        return self.schedule_abeam(beam_width, max_step_combinations, heuristic=heuristic)

    def _abeam_rollout(self, placement, avail, order, options):
        """Completes a partial placement greedily, HEFT-style: every still-unplaced task,
        taken in `order` (topological), goes on the eligible node with the earliest finish
        time. Returns the completed placement. Used by the "rollout_min"/"rollout_avg"
        A-Beam heuristics, where f is the cost of this completed schedule.

        Unlike the rank heuristic this is NOT a lower bound: it is the cost of one real
        completion, so it can overestimate the best completion (it is an upper bound)."""
        pl = dict(placement)
        av = dict(avail)
        for task in order:
            if task in pl:
                continue
            best_node, best_est, best_eft = None, 0, math.inf
            for node, comp_time in options[task]:
                est = self._earliest_start(task, node, pl, av)
                eft = est + comp_time
                if eft < best_eft:
                    best_node, best_est, best_eft = node, est, eft
            pl[task] = (best_node, best_est, best_eft)
            av[best_node] = best_eft
        return pl

    def schedule_abeam(self, beam_width=None, max_step_combinations=None, weights=None,
                       heuristic="rank"):
        """A-Beam: an A*-flavoured beam search that walks the DAG one "step" at a time.
        This is the engine behind both schedule_abeam_l() and schedule_abeam_leq(); they
        differ only in the objective weights passed in `weights` (see
        _resolve_weights(); None means the configured self.weight_*).

        The DAG is split into steps by _dag_steps() (the critical path sets how many there
        are). At each step every hosting combination for that step's services is explored
        as a separate branch; each branch is scored by f = g + h. Once the whole step has
        been expanded, the frontier is pruned down to the k branches with the lowest f,
        and the search moves to the next step.

        The objective is the weighted latency + quality + energy cost of
        evaluate_schedule(), and f is an admissible (never too high) estimate of the final
        cost of any completion of a branch:

          g : the cost already committed by the placed services - the makespan so far,
              plus the energy and quality of every placed service
          h : the least cost the unplaced services can still add -
                latency : the extra makespan they force, from _abeam_latency_bound()
                          (admissible via compute_ranks_min())
                energy + quality : for each unplaced service, the cheapest
                          w_energy * energy + w_quality * quality-loss over its eligible
                          nodes. Taking the minimum of the COMBINED per-node cost, rather
                          than the best energy node plus the best quality node separately,
                          is still admissible (both are additive per service) and is
                          tighter whenever a service's greenest node is not its
                          highest-quality one.

        beam_width (k) may be an int for a constant width, or a callable
        k(step_index, total_steps) -> int for a schedule that varies with depth (wider
        early, narrower later). Defaults to self.abeam_beam_width.

        Two separate limits keep this affordable. self.abeam_max_open_branches caps how
        many branches may be held at once, because enumerating a step exactly multiplies
        the live branch list by the beam width; when an expansion would exceed it, the
        branches are trimmed to the best f first, which also makes that step inexact.

        A step's full hosting product is enumerated whenever it is small enough
        (max_step_combinations, default self.abeam_max_step_combinations). It often is
        not: a map-reduce DAG with 15 independent services on 17 eligible routers each has
        17^15 ~ 2.9e18 combinations in a SINGLE step, which cannot be enumerated at any
        budget. Above the cap the step is instead expanded one service at a time, pruning
        the partial combinations back to k after each service. That explores the same
        space with the same scoring, just with the beam applied inside the step as well as
        at its boundary. self.abeam_steps_enumerated records how many steps got the exact
        treatment and self.abeam_complete is True only when every step did.

        Services within a step are mutually independent, so they are placed in self.tasks
        order; that order only matters when two of them land on the same node, where it
        decides which runs first.

        `heuristic` chooses how h is computed:
          "rank"        : the admissible bound above (UpRank_min + forward pass). Default.
          "rollout_min" : complete the branch greedily with HEFT placement
                          (_abeam_rollout), taking the remaining services in topological
                          order ranked by UpRank_min, and use the completed schedule's
                          cost as f.
          "rollout_avg" : the same rollout, ranked by the standard HEFT upward rank
                          (average compute cost, average communication cost).
        Rollouts are an upper bound rather than a lower bound, and cost O(H x e) per
        evaluation instead of O(e), so for them f is only computed where pruning
        actually reads it: inside an exactly enumerated step no pruning happens until
        the step ends, so mid-step branches are scored lazily, only if the open-branch
        ceiling forces a trim. The "rank" heuristic keeps its original behavior and
        scores every branch.

        self.abeam_heuristic_evals counts f evaluations and self.abeam_heuristic_time
        is the wall time spent in them (seconds), so the cost of one evaluation can be
        compared across heuristics."""
        if heuristic not in ("rank", "rollout_min", "rollout_avg"):
            raise ValueError(f"unknown A-Beam heuristic {heuristic!r}")
        if beam_width is None:
            beam_width = self.abeam_beam_width
        if max_step_combinations is None:
            max_step_combinations = self.abeam_max_step_combinations

        width_of = beam_width if callable(beam_width) else (lambda i, n: beam_width)

        steps = self._dag_steps()
        for task in self.tasks:
            if not self.comp_matrix.get(task):
                print(f"Warning: Task {task} could not be scheduled (No eligible nodes).")
        self.abeam_heuristic_evals = 0
        self.abeam_heuristic_time = 0.0
        if not steps:
            self.abeam_expansions = 0
            self.abeam_steps_enumerated = 0
            self.abeam_complete = True
            return {}

        self.compute_ranks_min()

        w = self._resolve_weights(weights)
        latency_ref = self._objective_refs()["latency_ref"]
        w_latency = w["latency"]
        offset = self._cost_offset(w)

        # Cheapest node first, so the partial-combination prune inside a wide step keeps
        # sensible branches even before f has much to go on.
        options = {t: sorted(self.comp_matrix[t].items(), key=lambda kv: kv[1])
                   for step in steps for t in step}

        # Weighted energy + quality cost of every (service, node) pairing, and each
        # service's cheapest one - the latter is the energy/quality part of h.
        eq_cost = {t: {n: self._energy_quality_cost(t, n, w) for n in self.comp_matrix[t]}
                   for step in steps for t in step}
        eq_min = {t: min(costs.values()) for t, costs in eq_cost.items()}

        # Everything still unplaced after step i, in topological order (steps are
        # topological by construction), precomputed once for _abeam_latency_bound().
        pending_after = [[t for later in steps[i + 1:] for t in later] for i in range(len(steps))]
        pending_after_eq = [sum(eq_min[t] for t in pending) for pending in pending_after]

        self.abeam_expansions = 0
        self.abeam_steps_enumerated = 0
        self.abeam_complete = True

        # Rollout heuristics complete each branch in a fixed topological order of all
        # services, ranked by UpRank_min ("rollout_min") or by the standard HEFT upward
        # rank ("rollout_avg"). Ranking by UpRank_min alone is not always topological
        # (it takes the minimum over successors), hence the topological sort.
        rollout = heuristic != "rank"
        if rollout:
            if heuristic == "rollout_min":
                priority = dict(self.rank_min_u)
            else:
                self.compute_ranks()
                priority = dict(self.rank_u)
            rollout_order = [t for t in self._topological_order(priority)
                             if self.comp_matrix.get(t)]

        # Heuristic timing is accumulated in locals and stored on self at the end. A rank
        # evaluation takes only a few microseconds, so even two clock reads per evaluation
        # slowed the rank-only schemes by ~3% - enough to distort their measured runtimes.
        # Rank evaluations are therefore timed 1 in RANK_TIMING_STRIDE and scaled up
        # (an estimate, but the sample spans every depth of the search); rollouts cost
        # hundreds of microseconds each, so every one is timed exactly.
        RANK_TIMING_STRIDE = 16
        clock = time.perf_counter
        h_time = 0.0       # rollouts: exact total; rank: total over the sampled evaluations
        h_evals = 0
        h_sampled = 0      # rank evaluations actually timed

        def score_rollout(placement, avail, eq_placed):
            """f = the cost of completing this branch with a greedy HEFT placement."""
            nonlocal h_time, h_evals
            t0 = clock()
            done = self._abeam_rollout(placement, avail, rollout_order, options)
            latency = 0
            eq_total = eq_placed
            for t, (node, _start, end) in done.items():
                if end > latency:
                    latency = end
                if t not in placement:
                    eq_total += eq_cost[t][node]
            f = w_latency * (latency / latency_ref - 1) + eq_total - offset
            h_time += clock() - t0
            h_evals += 1
            return f

        def scored(b):
            """A branch with its f filled in, if it was deferred (rollouts only)."""
            if b[0] is not None:
                return b
            return (score_rollout(b[2], b[3], b[4]),) + b[1:]

        # A branch is (f, makespan, placement, avail, eq_placed), where eq_placed is the
        # weighted energy + quality cost of the services it has placed so far. f is None
        # while a rollout score has been deferred (see the docstring).
        beam = [(0, 0, {}, {n: 0 for n in self.nodes}, 0.0)]

        for i, step_tasks in enumerate(steps):
            k = max(1, int(width_of(i, len(steps))))

            combinations = 1
            for t in step_tasks:
                combinations *= len(options[t])
            exact = combinations <= max_step_combinations
            if exact:
                self.abeam_steps_enumerated += 1
            else:
                self.abeam_complete = False

            # Expand the step service by service. When the whole product fits under the
            # cap nothing is dropped mid-step, so this enumerates every combination; when
            # it does not, the partial branches are pruned back to k after each service.
            branches = beam
            step_exact = exact
            for position, task in enumerate(step_tasks):
                # Expanding multiplies the branch list by this service's eligible node
                # count, so trim to the best f BEFORE expanding whenever that product
                # would blow past the ceiling. Without this, an exactly enumerated step
                # holds beam_width x step_combinations branches at once.
                fanout = len(options[task])
                if len(branches) * fanout > self.abeam_max_open_branches:
                    keep = max(1, self.abeam_max_open_branches // fanout)
                    if keep < len(branches):
                        if rollout:
                            branches = [scored(b) for b in branches]
                        branches = sorted(branches, key=lambda b: (b[0], b[1]))[:keep]
                        if step_exact:
                            # this step no longer saw every combination
                            step_exact = False
                            self.abeam_steps_enumerated -= 1
                            self.abeam_complete = False

                grown = []
                remaining_in_step = step_tasks[position + 1:]
                pending = remaining_in_step + pending_after[i]
                pending_eq = sum(eq_min[t] for t in remaining_in_step) + pending_after_eq[i]
                mid_step = position < len(step_tasks) - 1
                # Nothing reads f before this exact step ends, so defer rollout scores.
                defer = rollout and step_exact and mid_step
                for _f, makespan, placement, avail, eq_placed in branches:
                    for node, comp_time in options[task]:
                        self.abeam_expansions += 1

                        est = self._earliest_start(task, node, placement, avail)
                        eft = est + comp_time

                        new_placement = dict(placement)
                        new_placement[task] = (node, est, eft)
                        new_avail = dict(avail)
                        new_avail[node] = eft
                        new_makespan = makespan if makespan > eft else eft
                        new_eq = eq_placed + eq_cost[task][node]

                        # f = g + h, in the units of evaluate_schedule()'s cost. At a
                        # leaf (nothing pending) this is exactly the schedule's cost.
                        if not rollout:
                            h_evals += 1
                            if (h_evals - 1) % RANK_TIMING_STRIDE == 0:   # 1st, 17th, ...
                                t0 = clock()
                                latency = self._abeam_latency_bound(new_placement,
                                                                    new_makespan, pending)
                                h_time += clock() - t0
                                h_sampled += 1
                            else:
                                latency = self._abeam_latency_bound(new_placement,
                                                                    new_makespan, pending)
                            new_f = (w_latency * (latency / latency_ref - 1)
                                     + new_eq + pending_eq - offset)
                        elif defer:
                            new_f = None
                        else:
                            new_f = score_rollout(new_placement, new_avail, new_eq)
                        grown.append((new_f, new_makespan, new_placement, new_avail, new_eq))

                # Prune to the k lowest f. Skipped mid-step while the step is being
                # enumerated exactly, so that an exact step really does see every
                # combination before the beam closes at its boundary.
                if not (step_exact and mid_step):
                    grown.sort(key=lambda b: (b[0], b[1]))
                    grown = grown[:k]
                branches = grown

            beam = branches

        if not rollout and h_sampled:
            h_time *= h_evals / h_sampled
        self.abeam_heuristic_time = h_time
        self.abeam_heuristic_evals = h_evals

        # Every branch is now a complete schedule, and with nothing pending its f is
        # exactly its cost. Return the cheapest, ties going to the one that finishes first.
        best = min(beam, key=lambda b: (b[0], b[1]))
        return best[2]

    def _earliest_start(self, task, node, schedule, avail):
        """Earliest time `task` can start on `node`: the node's own availability, plus
        the arrival of every predecessor's result over the shortest path.

        A predecessor missing from `schedule` is only legitimate when it has no eligible
        node anywhere and so could not be scheduled at all. A predecessor that simply has
        not been placed YET means the task ordering is not topological: its dependency
        would be silently dropped, producing a schedule that violates precedence and an
        impossibly low makespan. That is a hard error rather than a silent skip."""
        est = avail[node]
        for pred in self.dag_parents.get(task, []):
            if pred not in schedule:
                if self.comp_matrix.get(pred):
                    raise ValueError(
                        f"Task ordering is not topological: '{task}' is being placed "
                        f"before its predecessor '{pred}'. Scheduling a task before its "
                        f"parent drops that dependency and understates the makespan.")
                continue # genuinely unschedulable predecessor (no eligible nodes)
            pred_node, _, pred_aft = schedule[pred]
            arrival = pred_aft + self.delay_matrix[pred_node][node]
            if arrival > est:
                est = arrival
        return est

    def _place_tasks(self, ordered_tasks):
        """Greedy earliest-finish-time placement of an already prioritized task list.
        Identical to the placement loop in schedule_heft()/schedule_heft_rlc()."""
        avail = {n: 0 for n in self.nodes}
        schedule = {} # task -> (node, start_time, end_time)

        for task in ordered_tasks:
            best_node, min_eft, best_est = None, math.inf, 0

            # Only iterate over ELIGIBLE nodes for this specific task
            eligible_nodes = self.comp_matrix.get(task, {})

            for node, comp_time in eligible_nodes.items():
                est = self._earliest_start(task, node, schedule, avail)

                eft = est + comp_time
                if eft < min_eft:
                    min_eft = eft
                    best_est = est
                    best_node = node

            if best_node is not None:
                schedule[task] = (best_node, best_est, min_eft)
                avail[best_node] = min_eft
            else:
                print(f"Warning: Task {task} could not be scheduled (No eligible nodes).")

        return schedule



    def _topological_order(self, priority=None):
        """Kahn topological sort. Ties are broken by descending priority (RLC rank_u by
        default), so the resulting order matches the HEFT task ordering whenever that
        ordering is itself topological - which keeps the exhaustive search comparable."""
        if priority is None:
            self.compute_ranks_rlc()
            priority = dict(self.rank_u)

        indegree = {t: len(self.dag_parents.get(t, [])) for t in self.tasks}
        ready = [t for t in self.tasks if indegree[t] == 0]
        order = []

        while ready:
            ready.sort(key=lambda x: priority.get(x, 0), reverse=True)
            task = ready.pop(0)
            order.append(task)
            for child in self.dag.get(task, []):
                indegree[child] -= 1
                if indegree[child] == 0:
                    ready.append(child)

        if len(order) != len(self.tasks):
            # Not a DAG (should not happen); fall back to the raw task list
            print("Warning: DAG contains a cycle, exhaustive search is using the raw task order.")
            order = list(self.tasks)
        return order

    def _min_chain_after(self):
        """chain_after[t] = lower bound on the time still needed AFTER t finishes: the
        longest successor chain, each task counted at its cheapest computation cost and
        with all communication ignored. Used to prune the exhaustive search."""
        min_tail = {}

        def calc(task):
            if task in min_tail: return min_tail[task]
            longest_succ = 0
            for succ in self.dag.get(task, []):
                longest_succ = max(longest_succ, calc(succ))
            min_tail[task] = self._get_min_comp(task) + longest_succ
            return min_tail[task]

        for t in self.tasks: calc(t)

        chain_after = {}
        for t in self.tasks:
            successors = self.dag.get(t, [])
            chain_after[t] = max([min_tail[s] for s in successors]) if successors else 0
        return chain_after


'''
# --- Execution Entry Point ---
if __name__ == "__main__":
    # You can load this directly from the file in your environment
    #with open("../scenario_json/cascon_main/ndn-cabeee-8dag-nesco.json", "r") as f:
    #with open("ndn-cabeee-8dag-nesco.json", "r") as f:
    with open("heft_DAG.json", "r") as f:
        json_data = json.load(f)
        
    scheduler = EdgeScheduler(json_data)
    
    # Compute ranks explicitly so we can print them
    scheduler.compute_ranks()
   
    # Calculate the theoretical lower bound (CP_min)
    cp_min = scheduler.calculate_cp_min()
    print(f"=== Baseline Metrics ===")
    print(f"Theoretical CP_min: {cp_min} ns\n")
    
    print("=== Task Ranks ===")
    print(f"{'Task':<15} | {'Upward Rank':<15} | {'Downward Rank':<15}")
    print("-" * 52)
    for t in scheduler.tasks:
        u_rank = scheduler.rank_u.get(t, 0)
        d_rank = scheduler.rank_d.get(t, 0)
        print(f"{t:<15} | {u_rank:<15.2f} | {d_rank:<15.2f}")
    print()


    print("=== HEFT Schedule ===")
    heft_sched = scheduler.schedule_heft()
    for t in scheduler.tasks: 
        if t in heft_sched:
            node, start, end = heft_sched[t]
            print(f"Task: {t:15} | Node: {node:8} | Start: {start:10} ns | End: {end:10} ns")
    makespan_heft = scheduler.get_makespan(heft_sched)
    slr_heft = makespan_heft / cp_min if cp_min > 0 else 0
    print(f">> HEFT Makespan: {makespan_heft} ns | SLR: {slr_heft:.4f}\n")

    print("=== HEFT-cabeee Schedule ===")
    heft_cabeee_sched = scheduler.schedule_heft_cabeee()
    for t in scheduler.tasks: 
        if t in heft_cabeee_sched:
            node, start, end = heft_cabeee_sched[t]
            print(f"Task: {t:15} | Node: {node:8} | Start: {start:10} ns | End: {end:10} ns")
    makespan_cabeee = scheduler.get_makespan(heft_cabeee_sched)
    slr_cabeee = makespan_cabeee / cp_min if cp_min > 0 else 0
    print(f">> HEFT-cabeee Makespan: {makespan_cabeee} ns | SLR: {slr_cabeee:.4f}\n")

    print("=== CPOP Schedule ===")
    cpop_sched = scheduler.schedule_cpop()
    for t in scheduler.tasks:
        if t in cpop_sched:
            node, start, end = cpop_sched[t]
            print(f"Task: {t:15} | Node: {node:8} | Start: {start:10} ns | End: {end:10} ns")
    makespan_cpop = scheduler.get_makespan(cpop_sched)
    slr_cpop = makespan_cpop / cp_min if cp_min > 0 else 0
    print(f">> CPOP Makespan: {makespan_cpop} ns | SLR: {slr_cpop:.4f}\n")
'''

# --- Execution Entry Point (Batch Processor) ---

# CSV columns, shared by the worker and the writer in the parent process.
# Every "... Time ms" column is wall-clock milliseconds for that scheme alone, measured
# with time.perf_counter(). Milliseconds is the one scale that covers the whole range
# seen here: the HEFT variants finish in well under a millisecond on small DAGs, while
# a budget-hit exhaustive search runs for tens of seconds.
#
# "Energy J", "Quality" and "Cost" are every scheme's schedule scored by the SAME
# EdgeScheduler.evaluate_schedule() and weights: total energy (sum over services), total
# quality (product over services) and the weighted, normalized latency + quality + energy
# cost that the A_Beam<k>_LEQ schemes minimize. Only they optimize that cost; the
# A_Beam<k>_L schemes and the other schemes optimize makespan alone, so their Cost shows what ignoring quality and
# energy costs them.
FIELDNAMES = [
    "Scenario File",
    "Total Tasks",
    "Total Nodes",
    "CP min ns",
    "HEFT Makespan ns",
    "HEFT SLR",
    "HEFT Energy J",
    "HEFT Quality",
    "HEFT Cost",
    "HEFT Time ms",
    "HEFT RLC Makespan ns",
    "HEFT RLC SLR",
    "HEFT RLC Energy J",
    "HEFT RLC Quality",
    "HEFT RLC Cost",
    "HEFT RLC Time ms",
    "HEFT IC Makespan ns",
    "HEFT IC SLR",
    "HEFT IC Energy J",
    "HEFT IC Quality",
    "HEFT IC Cost",
    "HEFT IC Time ms",
    "HEFT Iter Makespan ns",
    "HEFT Iter SLR",
    "HEFT Iter Energy J",
    "HEFT Iter Quality",
    "HEFT Iter Cost",
    "HEFT Iter iterations",
    "HEFT Iter Time ms",
    "CPOP Makespan ns",
    "CPOP SLR",
    "CPOP Energy J",
    "CPOP Quality",
    "CPOP Cost",
    "CPOP Time ms",
    "Exhaustive Makespan ns",
    "Exhaustive SLR",
    "Exhaustive Energy J",
    "Exhaustive Quality",
    "Exhaustive Cost",
    "Exhaustive Status",
    "Exhaustive Time ms",
    "Weight Latency",
    "Weight Quality",
    "Weight Energy",
    "Setup Time ms",
    "Scenario Time ms"
]

# A-Beam is run at every beam width in this list, once optimizing latency alone
# (A_Beam<k>_L) and once optimizing latency + energy + quality (A_Beam<k>_LEQ).
# Override from the command line with -k/--beam-widths.
ABEAM_BEAM_WIDTHS = [1, 10, 100]
ABEAM_OBJECTIVES = ("L", "LEQ")

# Extra A-Beam variants whose heuristic completes each branch with a greedy HEFT
# placement (a rollout) instead of using the UpRank_min bound. Each entry is
# (tag, heuristic, beam width) and adds A_Beam<tag><k>_L and A_Beam<tag><k>_LEQ:
#   PM - rollout order ranked by UpRank_min (minimum costs)
#   PA - rollout order ranked by the standard HEFT upward rank (average costs)
# These run at their own fixed width, independent of -k/--beam-widths.
ABEAM_ROLLOUT_SCHEMES = [("PM", "rollout_min", 10), ("PA", "rollout_avg", 10)]

# "Expansions" counts (branch, node) pairs generated; "Heuristic Evals" counts f
# evaluations (rollout variants defer them inside exactly enumerated steps, so they can
# be fewer); "Heuristic Time ms" is the wall time spent evaluating f, and
# "Heuristic us per Eval" the mean cost of one evaluation.
ABEAM_METRICS = ("Makespan ns", "SLR", "Energy J", "Quality", "Cost", "Status", "Time ms",
                 "Expansions", "Heuristic Evals", "Heuristic Time ms", "Heuristic us per Eval")


def abeam_specs(widths):
    """Every A-Beam scheme to run, in run order, as (label, heuristic, k, objective)."""
    specs = [(f"A_Beam{k}_{obj}", "rank", k, obj)
             for k in widths for obj in ABEAM_OBJECTIVES]
    specs += [(f"A_Beam{tag}{k}_{obj}", heuristic, k, obj)
              for tag, heuristic, k in ABEAM_ROLLOUT_SCHEMES for obj in ABEAM_OBJECTIVES]
    return specs


def abeam_labels(widths):
    """Scheme labels for every A-Beam scheme, in run order."""
    return [label for label, _, _, _ in abeam_specs(widths)]


def build_fieldnames(widths):
    """CSV columns: the fixed schemes, then one block per A-Beam variant, then the tail
    (weights and setup/scenario times)."""
    tail_start = FIELDNAMES.index("Weight Latency")
    abeam_cols = [f"{label} {m}" for label in abeam_labels(widths) for m in ABEAM_METRICS]
    return FIELDNAMES[:tail_start] + abeam_cols + FIELDNAMES[tail_start:]


# Minimal scenario used only to read EdgeScheduler's default limits back out for
# diagnostics, without needing a real scenario file.
EMPTY_SCENARIO = {"router": [], "link": [], "routerHosting": [], "dag": {"dag1": {}}}


def _stamp():
    """[HH:MM:SS] prefix for progress lines. The full start date is printed once in the
    run header, so the per-line stamp stays short."""
    return time.strftime("[%H:%M:%S]")


def _format_duration(seconds):
    """Human-readable elapsed time: sub-minute stays in seconds, longer runs get
    Hh Mm Ss so a multi-hour sweep is readable at a glance."""
    if seconds < 60:
        return f"{seconds:.2f}s"
    total = int(seconds)
    h, rem = divmod(total, 3600)
    m, s = divmod(rem, 60)
    if h:
        return f"{h}h {m:02d}m {s:02d}s"
    return f"{m}m {s:02d}s"

# SLR is the schedule-length-ratio.
# It is a normalized metric designed to measure scheduling efficiency independent of the size of the graph.
# It divides the actual makespan by the theoretical critical path time of the graph.
# The critical path represents the absolute minimum time required to complete the graph if you assume tasks run sequentially
# along the longest path, ignoring communication delays and assuming the fastest possible processing speeds.
# A smaller SLR means a more efficient algorithm.

# Shared across worker processes via the pool initializer: a counter of how many
# scenarios have started, and the total number of scenarios.
_started_counter = None
_total_scenarios = None
_beam_widths = None


def _init_worker(started_counter, total_scenarios, beam_widths=None):
    """Pool initializer: hand every worker process the shared start counter and the
    A-Beam beam widths to run."""
    global _started_counter, _total_scenarios, _beam_widths
    _started_counter = started_counter
    _total_scenarios = total_scenarios
    _beam_widths = beam_widths


def process_scenario(file_path, beam_widths=None):
    """Runs every scheduling algorithm over a single JSON scenario file.

    This is the unit of work handed to each worker process, so it must be a
    module-level function (picklable). It reports its own "Starting" line as it
    begins on a core; the parent reports the matching "Finished" line.
    Returns a dict: {"file": <name>, "row": <csv row dict or None>,
    "warning": <str or None>, "error": <str or None>}.
    """
    file_name = os.path.basename(file_path)

    # Bump the shared "started" counter and announce this scenario as it begins.
    if _started_counter is not None:
        with _started_counter.get_lock():
            _started_counter.value += 1
            started_idx = _started_counter.value
        print(f"{_stamp()} Starting scenario {started_idx}/{_total_scenarios}: {file_name}", flush=True)

    scenario_t0 = time.perf_counter()
    try:
        with open(file_path, "r") as f:
            json_data = json.load(f)

        # Initialize scheduler. Setup is timed separately from the schemes: it covers the
        # JSON parse plus the Floyd-Warshall all-pairs delay matrix, which is shared work
        # none of the individual algorithms should be charged for.
        t0 = time.perf_counter()
        scheduler = EdgeScheduler(json_data)
        cp_min = scheduler.calculate_cp_min()   # theoretical lower bound
        time_setup = (time.perf_counter() - t0) * 1000.0

        # Run HEFT
        t0 = time.perf_counter()
        heft_sched = scheduler.schedule_heft()
        time_heft = (time.perf_counter() - t0) * 1000.0
        makespan_heft = scheduler.get_makespan(heft_sched)
        slr_heft = makespan_heft / cp_min if cp_min > 0 else 0

        # Run HEFT-RLC (Your optimized version)
        t0 = time.perf_counter()
        heft_rlc_sched = scheduler.schedule_heft_rlc()
        time_rlc = (time.perf_counter() - t0) * 1000.0
        makespan_rlc = scheduler.get_makespan(heft_rlc_sched)
        slr_rlc = makespan_rlc / cp_min if cp_min > 0 else 0

        # Run HEFT-IC (Your optimized version with modification 1)
        t0 = time.perf_counter()
        heft_ic_sched = scheduler.schedule_heft_ic()
        time_ic = (time.perf_counter() - t0) * 1000.0
        makespan_ic = scheduler.get_makespan(heft_ic_sched)
        slr_ic = makespan_ic / cp_min if cp_min > 0 else 0

        # Run HEFT-Iter (Your optimized version with modification 2)
        t0 = time.perf_counter()
        heft_iter_sched, iterations_iter = scheduler.schedule_heft_iter()
        time_iter = (time.perf_counter() - t0) * 1000.0
        makespan_iter = scheduler.get_makespan(heft_iter_sched)
        slr_iter = makespan_iter / cp_min if cp_min > 0 else 0

        # Run CPOP
        t0 = time.perf_counter()
        cpop_sched = scheduler.schedule_cpop()
        time_cpop = (time.perf_counter() - t0) * 1000.0
        makespan_cpop = scheduler.get_makespan(cpop_sched)
        slr_cpop = makespan_cpop / cp_min if cp_min > 0 else 0

        # Run the exhaustive search over every possible combination of placements.
        # NOTE: this time includes the five heuristic schedules the search runs internally
        # to seed its incumbent - that seeding is part of what the method costs.
        t0 = time.perf_counter()
        exhaustive_sched = scheduler.schedule_exhaustive()
        time_exhaustive = (time.perf_counter() - t0) * 1000.0
        makespan_exhaustive = scheduler.get_makespan(exhaustive_sched)
        slr_exhaustive = makespan_exhaustive / cp_min if cp_min > 0 else 0

        # Run every A-Beam scheme with the same engine: A_Beam<k>_L optimizes latency
        # alone, A_Beam<k>_LEQ the weighted latency + energy + quality cost, and the
        # PM/PA variants swap the UpRank_min heuristic for a HEFT-placement rollout.
        widths = beam_widths or _beam_widths or ABEAM_BEAM_WIDTHS
        abeam_runs = []
        for label, heuristic, k, obj in abeam_specs(widths):
            run = scheduler.schedule_abeam_l if obj == "L" else scheduler.schedule_abeam_leq
            t0 = time.perf_counter()
            sched = run(beam_width=k, heuristic=heuristic)
            elapsed = (time.perf_counter() - t0) * 1000.0
            abeam_runs.append({
                "label": label, "sched": sched, "time_ms": elapsed,
                "complete": scheduler.abeam_complete,
                "expansions": scheduler.abeam_expansions,
                "evals": scheduler.abeam_heuristic_evals,
                "heuristic_ms": scheduler.abeam_heuristic_time * 1000.0,
            })

        elapsed_ms = (time.perf_counter() - scenario_t0) * 1000.0

        warning = None
        if not scheduler.exhaustive_complete:
            warning = (f"exhaustive search hit its budget of "
                       f"{scheduler.max_exhaustive_evaluations:,} evaluations. Reported "
                       f"makespan is the best found, NOT a proven optimum.")

        row = {
            "Scenario File": file_name,
            "Total Tasks": len(scheduler.tasks),
            "Total Nodes": len(scheduler.nodes),
            "CP min ns": cp_min,
            "HEFT Makespan ns": makespan_heft,
            "HEFT SLR": f"{slr_heft:.4f}",
            "HEFT Time ms": f"{time_heft:.3f}",
            "HEFT RLC Makespan ns": makespan_rlc,
            "HEFT RLC SLR": f"{slr_rlc:.4f}",
            "HEFT RLC Time ms": f"{time_rlc:.3f}",
            "HEFT IC Makespan ns": makespan_ic,
            "HEFT IC SLR": f"{slr_ic:.4f}",
            "HEFT IC Time ms": f"{time_ic:.3f}",
            "HEFT Iter Makespan ns": makespan_iter,
            "HEFT Iter SLR": f"{slr_iter:.4f}",
            "HEFT Iter iterations": iterations_iter,
            "HEFT Iter Time ms": f"{time_iter:.3f}",
            "CPOP Makespan ns": makespan_cpop,
            "CPOP SLR": f"{slr_cpop:.4f}",
            "CPOP Time ms": f"{time_cpop:.3f}",
            "Exhaustive Makespan ns": makespan_exhaustive,
            "Exhaustive SLR": f"{slr_exhaustive:.4f}",
            # 1 = the search finished, so the makespan is a proven optimum.
            # 0 = it ran out of evaluations, so it is only the best found.
            "Exhaustive Status": 1 if scheduler.exhaustive_complete else 0,
            "Exhaustive Time ms": f"{time_exhaustive:.3f}",
            "Setup Time ms": f"{time_setup:.3f}",
            "Scenario Time ms": f"{elapsed_ms:.3f}"
        }

        # Energy, quality and combined cost for every scheme, all scored identically
        for label, sched in (("HEFT", heft_sched),
                             ("HEFT RLC", heft_rlc_sched),
                             ("HEFT IC", heft_ic_sched),
                             ("HEFT Iter", heft_iter_sched),
                             ("CPOP", cpop_sched),
                             ("Exhaustive", exhaustive_sched),
                             *((r["label"], r["sched"]) for r in abeam_runs)):
            metrics = scheduler.evaluate_schedule(sched)
            row[f"{label} Energy J"] = f"{metrics['energy']:.4f}"
            row[f"{label} Quality"] = f"{metrics['quality']:.6g}"
            row[f"{label} Cost"] = f"{metrics['cost']:.6f}"
        for r in abeam_runs:
            label = r["label"]
            makespan = scheduler.get_makespan(r["sched"])
            row[f"{label} Makespan ns"] = makespan
            row[f"{label} SLR"] = f"{(makespan / cp_min if cp_min > 0 else 0):.4f}"
            # 1 = every DAG step had its full hosting product enumerated exactly.
            # 0 = at least one step was too wide and was expanded service by service.
            row[f"{label} Status"] = 1 if r["complete"] else 0
            row[f"{label} Time ms"] = f"{r['time_ms']:.3f}"
            row[f"{label} Expansions"] = r["expansions"]
            row[f"{label} Heuristic Evals"] = r["evals"]
            row[f"{label} Heuristic Time ms"] = f"{r['heuristic_ms']:.3f}"
            per_eval = r["heuristic_ms"] * 1000.0 / r["evals"] if r["evals"] else 0.0
            row[f"{label} Heuristic us per Eval"] = f"{per_eval:.3f}"
        row["Weight Latency"] = f"{scheduler.weight_latency:.4f}"
        row["Weight Quality"] = f"{scheduler.weight_quality:.4f}"
        row["Weight Energy"] = f"{scheduler.weight_energy:.4f}"
        return {"file": file_name, "row": row, "warning": warning,
                "error": None, "elapsed_ms": elapsed_ms}

    except Exception as e:
        elapsed_ms = (time.perf_counter() - scenario_t0) * 1000.0
        return {"file": file_name, "row": None, "warning": None,
                "error": str(e), "elapsed_ms": elapsed_ms}


if __name__ == "__main__":
    # 1. Setup command line arguments
    parser = argparse.ArgumentParser(description="Run scheduling algorithms over a directory of JSON scenarios.")
    parser.add_argument("-d", "--dir", required=True, help="Directory containing JSON scenario files")
    parser.add_argument("-o", "--out", default="results.csv", help="Output CSV file name")
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count(),
                        help="Number of worker processes / CPU cores to use in parallel "
                             "(default: all available cores). Use 1 to run serially.")
    parser.add_argument("-k", "--beam-widths", default=",".join(map(str, ABEAM_BEAM_WIDTHS)),
                        help="Comma-separated A-Beam beam widths (default: %(default)s). "
                             "Each width k adds two schemes, A_Beam<k>_L (latency only) and "
                             "A_Beam<k>_LEQ (latency + energy + quality), with their own CSV "
                             "columns. k is how many branches survive the prune at each DAG "
                             "step; larger k searches more and runs slower.")
    parser.add_argument("-p", "--pattern", default="*.json",
                        help="Only run scenarios whose file name matches this glob pattern "
                             "(default: '*.json', i.e. every scenario in the directory). A "
                             "pattern with no wildcard is treated as a suffix, so "
                             "-p 1-noSD2-multicast.json runs every file ending in that. "
                             "Quote patterns containing * so the shell does not expand them.")
    args = parser.parse_args()

    try:
        beam_widths = [int(x) for x in args.beam_widths.split(",") if x.strip()]
    except ValueError:
        parser.error(f"-k/--beam-widths must be comma-separated integers, got {args.beam_widths!r}")
    if not beam_widths or any(k < 1 for k in beam_widths):
        parser.error("-k/--beam-widths needs at least one width, all >= 1")
    beam_widths = sorted(set(beam_widths))
    fieldnames = build_fieldnames(beam_widths)

    # 2. Find the JSON files in the specified directory, then keep the ones matching
    #    the requested pattern. A pattern with no glob wildcard is a plain suffix.
    all_json_files = sorted(glob.glob(os.path.join(args.dir, "*.json")))
    if not all_json_files:
        print(f"No JSON files found in directory: {args.dir}")
        exit(1)

    pattern = args.pattern
    if not any(ch in pattern for ch in "*?["):
        pattern = "*" + pattern

    json_files = [f for f in all_json_files if fnmatch.fnmatch(os.path.basename(f), pattern)]
    if not json_files:
        print(f"No JSON files in {args.dir} match pattern: {args.pattern}")
        exit(1)

    workers = max(1, min(args.jobs, len(json_files)))
    print(f"Run started {time.strftime('%Y-%m-%d %H:%M:%S')}")
    if len(json_files) == len(all_json_files):
        print(f"Found {len(json_files)} JSON scenarios. Evaluating on {workers} core(s)...\n")
    else:
        print(f"Found {len(all_json_files)} JSON scenarios, {len(json_files)} match "
              f"'{args.pattern}'. Evaluating on {workers} core(s)...\n")

    # 3. Fan the scenarios out across worker processes. Each JSON file is an
    #    independent unit of work, so every scenario runs on its own core and the
    #    parent process collects the finished rows.
    #
    #    Rows are written to the CSV (and flushed) as each scenario finishes, so
    #    interrupting the run part way through still leaves a usable results file
    #    with every scenario completed so far. If the whole run finishes, the file
    #    is rewritten once at the end in sorted file order for a deterministic
    #    result.
    results = {}
    total = len(json_files)
    finished = 0
    interrupted = False
    pool_broken = False
    started_counter = multiprocessing.Value('i', 0)

    csv_file = open(args.out, mode='w', newline='')
    writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
    writer.writeheader()
    csv_file.flush()

    run_t0 = time.perf_counter()
    scenario_seconds = 0.0   # summed per-scenario time, for the parallel-speedup line

    # The executor is driven manually rather than with a "with" block: on Ctrl-C we want
    # to drop the queued scenarios so the run stops promptly. A plain "with" would call
    # shutdown(wait=True), which drains the entire remaining queue before exiting.
    future_to_file = {}
    executor = concurrent.futures.ProcessPoolExecutor(
        max_workers=workers,
        initializer=_init_worker,
        initargs=(started_counter, total, beam_widths),
    )
    try:
        future_to_file = {
            executor.submit(process_scenario, fp): os.path.basename(fp)
            for fp in json_files
        }
        for future in concurrent.futures.as_completed(future_to_file):
            res = future.result()
            results[res["file"]] = res
            finished += 1
            took = res.get("elapsed_ms", 0.0)
            scenario_seconds += took / 1000.0
            print(f"{_stamp()} Finished scenario {finished}/{total} "
                  f"in {took / 1000.0:.2f}s: {res['file']}", flush=True)
            if res["error"] is not None:
                print(f"{_stamp()}   -> Error processing {res['file']}: {res['error']}")
                continue
            if res["warning"] is not None:
                print(f"{_stamp()}   -> WARNING ({res['file']}): {res['warning']}")
            if res["row"] is not None:
                writer.writerow(res["row"])
                csv_file.flush()
    except KeyboardInterrupt:
        # Ctrl-C in the parent only
        interrupted = True
    except concurrent.futures.process.BrokenProcessPool:
        # A worker process died. Either Ctrl-C from a terminal reached the whole process
        # group (so the workers died before the parent saw its own KeyboardInterrupt), or
        # a worker was killed outright - most often by the kernel's OOM killer, since
        # A-Beam's memory grows with the beam width and every worker pays it. Report it
        # instead of silently calling it an interrupt.
        interrupted = True
        pool_broken = True
    finally:
        if interrupted:
            if pool_broken:
                print(f"\n{_stamp()} A worker process died - stopping.", flush=True)
            else:
                print(f"\n{_stamp()} Stopping (waiting for the scenarios already running "
                      f"to finish)...", flush=True)
            # Drop everything still queued, then wait only for what is already running.
            # shutdown(cancel_futures=True) does this in one call, but that argument only
            # exists in Python 3.9+, so cancel the pending futures explicitly first -
            # which has the same effect on every version. Futures already running cannot
            # be cancelled and simply finish.
            for pending in future_to_file:
                pending.cancel()
            executor.shutdown(wait=True)
        else:
            executor.shutdown(wait=True)
        csv_file.close()

    run_elapsed = time.perf_counter() - run_t0

    if pool_broken:
        print(f"\nSTOPPED after {finished}/{total} scenarios in "
              f"{_format_duration(run_elapsed)}: a worker process died, which breaks the "
              f"pool and ends the run.")
        print("  The usual cause is the kernel's OOM killer. Check with:")
        print("    dmesg -T | grep -i 'killed process'")
        print(f"  A-Beam holds up to abeam_max_open_branches "
              f"({EdgeScheduler(EMPTY_SCENARIO).abeam_max_open_branches:,}) branches of "
              f"~2 KB in EVERY worker, so if that is the cause, use fewer workers (-j), "
              f"smaller beam widths (-k), or lower abeam_max_open_branches.")
        print(f"  Partial results (unsorted) saved in: {args.out}")
        exit(1)

    if interrupted:
        print(f"\nInterrupted after {finished}/{total} scenarios in "
              f"{_format_duration(run_elapsed)}. "
              f"Partial results (unsorted) saved in: {args.out}")
        exit(1)

    # 4. Full run finished: rewrite the CSV in the original (sorted) file order so
    #    the output is deterministic regardless of the order the workers finished.
    with open(args.out, mode='w', newline='') as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
        writer.writeheader()
        for file_path in json_files:
            res = results.get(os.path.basename(file_path))
            if res is not None and res["row"] is not None:
                writer.writerow(res["row"])

    print(f"\nEvaluation complete! Results tabulated in: {args.out}")
    print(f"Run finished {time.strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"Total wall-clock time for all {total} scenarios: "
          f"{_format_duration(run_elapsed)} ({run_elapsed:.2f}s)")
    print(f"  summed scenario time : {_format_duration(scenario_seconds)}"
          f"  (mean {scenario_seconds / total:.2f}s per scenario)")
    if run_elapsed > 0:
        print(f"  parallel speedup     : {scenario_seconds / run_elapsed:.1f}x "
              f"on {workers} core(s)")

    # Push a completion notification through ntfy.sh. The total is the same string as
    # the "Total wall-clock time" line above. Best effort only: the results are already
    # saved, so a failed notification must not turn a finished run into an error.
    ntfy_topic = "ntfy.sh/cabeee-dag_run"
    ntfy_message = f"A-Beam runs complete! Total exec time: {_format_duration(run_elapsed)}."
    try:
        notify = subprocess.run(["curl", "-d", ntfy_message, ntfy_topic],
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                timeout=30)
        if notify.returncode == 0:
            print(f"Sent completion notification to {ntfy_topic}")
        else:
            print(f"(completion notification failed: curl exited with {notify.returncode})")
    except (OSError, subprocess.TimeoutExpired) as e:
        print(f"(completion notification failed: {e})")