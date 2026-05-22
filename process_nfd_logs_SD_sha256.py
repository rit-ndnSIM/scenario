import os
import re
import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import random
import argparse

# Regex to capture the timestamp at the very beginning of the line
LINE_TIMESTAMP_PATTERN = re.compile(r"^\+(?P<seconds>\d+\.\d+)s")

def get_line_seconds(line):
    """Extracts the floating point timestamp from the start of the log line."""
    match = LINE_TIMESTAMP_PATTERN.match(line)
    if match:
        return float(match.group('seconds'))
    return None

def count_specific_lines(output_file_path, keyword1, keyword2, stop_time=None):
    count = 0
    try:
        with open(output_file_path, 'r', encoding='utf-8', errors='ignore') as file:
            for line in file:
                if stop_time is not None:
                    t = get_line_seconds(line)
                    if t is not None and t > stop_time:
                        break

                if "serviceOrchestration/reset" in line:
                    count = 0
                    continue
                if keyword1 in line and keyword2 in line:
                    count += 1
    except FileNotFoundError:
        return 0
    return count

def clean_service_name(name):
    """Shortens service names for display. Ex: '/sensor/params-sha256=...' -> '/sensor'"""
    if '/params' in name:
        return name.split('/params')[0]
    return name

def extract_sha256(name):
    """Extracts the sha256 hash substring if present in the service name."""
    if 'params-sha256=' in name:
        return name.split('params-sha256=')[-1]
    return "unknown"

def normalize_to_ns(value, unit):
    if 'microsecond' in unit:
        return int(value) * 1000
    return int(value)

US_TO_NS = 1000

NODE_ACTIVITY_PATTERN = re.compile(
    r".*?NFDServiceDiscovery - (WF|SD)?resourceAllocation: Service (?P<service_name>\S+) (?P<action>started|finished) running on node (?P<node_id>\d+)\. .*? at (?P<timestamp>\d+) (?P<unit>nanoseconds|microseconds)"
)

SCHEDULING_PATTERN = re.compile(
    r".*?NFDServiceDiscovery - SDresourceAllocation: Service (?P<service_name>\S+) (?P<status>scheduled|no longer scheduled) on node (?P<node_id>\d+) starting at (?P<start_time>\d+) and ending at (?P<end_time>\d+) (?P<unit>nanoseconds|microseconds)"
)

WORKFLOW_START_PATTERN = re.compile(
    r"- workflow start: (?P<start_time>\d+) nanoseconds"
)
FINAL_LATENCY_PATTERN = re.compile(
    r"Service Latency: (?P<latency>\d+) nanoseconds\."
)
NFD_NODE_PATTERN = re.compile(
    r"NFD is running on node (?P<node_id>\d+)"
)

def generate_gantt_chart(runs_data, output_filename, title, workflow_start_ns, total_duration_ns):
    print(f"\nGenerating {title} ({output_filename})...")
    if not runs_data:
        print(f"No data found for {title}.")
        return

    unique_services = sorted(list(set(clean_service_name(run['service']) for run in runs_data)))
    cmap = plt.get_cmap('tab10') 
    service_colors = {service: cmap(i % 10) for i, service in enumerate(unique_services)}

    fig, ax = plt.subplots(figsize=(12, 6))

    all_nodes = sorted(list(set(run['node'] for run in runs_data)))
    y_positions = [n * 10 for n in all_nodes]
    y_labels = [f"Node {n}" for n in all_nodes]
    bar_height = 8

    for run in runs_data:
        node_y = run['node'] * 10
        rel_start_ms = (run['start_ns'] - workflow_start_ns) / 1000000.0
        duration_ms = run['duration_ns'] / 1000000.0
        svc_clean = clean_service_name(run['service'])
        
        ax.broken_barh(
            [(rel_start_ms, duration_ms)],
            (node_y - bar_height/2, bar_height),
            facecolors=service_colors[svc_clean],
            edgecolor='black',
            linewidth=0.5
        )
        
        text_x = rel_start_ms + (duration_ms / 2)
        text_y = node_y
        
        if duration_ms > 0.1:
            ax.text(
                text_x, text_y, svc_clean,
                ha='center', va='center',
                color='white', fontsize=8, weight='bold', clip_on=True
            )

    ax.set_yticks(y_positions)
    ax.set_yticklabels(y_labels)
    ax.set_xlabel('Time (milliseconds since Workflow Start)')
    ax.set_title(title)
    ax.grid(True, axis='x', linestyle='--', alpha=0.7)

    ax.set_xlim(left=0)
    if total_duration_ns > 0:
        sim_duration_ms = total_duration_ns / 1000000.0
        current_max = ax.get_xlim()[1]
        ax.set_xlim(right=max(current_max, sim_duration_ms))

    legend_patches = [mpatches.Patch(color=color, label=service) for service, color in service_colors.items()]
    ax.legend(handles=legend_patches, title="Services", loc='upper right')

    plt.tight_layout()
    plt.savefig(output_filename)
    plt.close(fig) 
    print(f"Plot saved successfully to: {output_filename}")

def analyze_and_plot_all(file_path, output_filename_base, stop_time=None):
    if not os.path.exists(file_path):
        print(f"Error: Log file not found at {file_path}")
        return

    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            log_lines = f.readlines()
    except Exception as e:
        print(f"Error reading file: {e}")
        return

    workflow_start_time_ns = 0
    final_service_latency_ns = 0
    
    active_runs_ns = {} 
    node_busy_time_ns = {}
    completed_utilization_runs = [] 
    
    # Track planned layout items dynamically
    scheduled_items = {} 

    number_of_nodes = 0
    print(f"Analyzing log data{' (stopping at ' + str(stop_time) + 's)' if stop_time else ''}...")

    for line in log_lines:
        if stop_time is not None:
            t = get_line_seconds(line)
            if t is not None and t > stop_time:
                break

        match_node_count = NFD_NODE_PATTERN.search(line)
        if match_node_count:
            number_of_nodes += 1

        match_start = WORKFLOW_START_PATTERN.search(line)
        if match_start:
            workflow_start_time_ns = int(match_start.group('start_time'))

        match_latency = FINAL_LATENCY_PATTERN.search(line)
        if match_latency:
            final_service_latency_ns = int(match_latency.group('latency'))

        # Resource Utilization
        match_util = NODE_ACTIVITY_PATTERN.search(line)
        if match_util:
            data = match_util.groupdict()
            node_id = int(data['node_id'])
            service_name = data['service_name']
            action = data['action']
            timestamp_ns = normalize_to_ns(data['timestamp'], data['unit'])
            run_key = (service_name, node_id) 

            if action == 'started':
                active_runs_ns[run_key] = timestamp_ns
                if node_id not in node_busy_time_ns:
                    node_busy_time_ns[node_id] = 0
            elif action == 'finished':
                if run_key in active_runs_ns:
                    start_ns = active_runs_ns.pop(run_key)
                    duration_ns = timestamp_ns - start_ns
                    node_busy_time_ns[node_id] += duration_ns
                    completed_utilization_runs.append({
                        'node': node_id,
                        'service': service_name,
                        'start_ns': start_ns,
                        'duration_ns': duration_ns
                    })

        # Resource Scheduling
        match_sched = SCHEDULING_PATTERN.search(line)
        if match_sched:
            data = match_sched.groupdict()
            node_id = int(data['node_id'])
            raw_service_name = data['service_name']
            status = data['status']
            unit = data['unit']
            
            start_ns = normalize_to_ns(data['start_time'], unit)
            end_ns = normalize_to_ns(data['end_time'], unit)
            
            sched_key = (node_id, raw_service_name, start_ns, end_ns)

            if status == 'scheduled':
                scheduled_items[sched_key] = {
                    'node': node_id,
                    'service': raw_service_name,
                    'start_ns': start_ns,
                    'duration_ns': end_ns - start_ns
                }
            elif status == 'no longer scheduled':
                if sched_key in scheduled_items:
                    del scheduled_items[sched_key]

    if stop_time is not None and final_service_latency_ns == 0 and completed_utilization_runs:
        max_end_ns = max(run['start_ns'] + run['duration_ns'] for run in completed_utilization_runs)
        total_simulation_duration_ns = max_end_ns
        print(f"Note: Stop time reached. Using last recorded activity ({total_simulation_duration_ns}ns) as total duration.")
    else:
        total_simulation_duration_ns = final_service_latency_ns

    overall_total_busy_time_ns = 0
    total_utilization_percentage = 0.0
    text_results = []

    if total_simulation_duration_ns > 0:
        for node_id, busy_time_ns in node_busy_time_ns.items():
            overall_total_busy_time_ns += busy_time_ns
            utilization_percentage = (busy_time_ns / total_simulation_duration_ns) * 100
            total_utilization_percentage += utilization_percentage
            text_results.append((node_id, busy_time_ns, utilization_percentage))
        text_results.sort(key=lambda x: x[0])
        average_utilization_percentage = total_utilization_percentage / number_of_nodes if number_of_nodes > 0 else 0.0
    else:
        average_utilization_percentage = 0.0

    variance = 0.0
    stdev = 0.0
    coef_of_variation = 0.0

    if number_of_nodes > 0 and average_utilization_percentage > 0:
        sum_squared_diff = 0.0
        for _, _, util_pct in text_results:
            diff = util_pct - average_utilization_percentage
            sum_squared_diff += diff * diff
        variance = sum_squared_diff / number_of_nodes
        stdev = variance ** 0.5
        coef_of_variation = stdev / average_utilization_percentage

    print("\n## Simulation Time Summary")
    print("----------------------------")
    print(f"Workflow Start Time: {workflow_start_time_ns} nanoseconds")
    if stop_time:
        print(f"Log Analysis Stopped At: {stop_time} seconds")
    print(f"Final Service Latency: {final_service_latency_ns} nanoseconds")
    print(f"Total Simulation Duration: {total_simulation_duration_ns} nanoseconds")

    # MODIFIED: Extract and print only the active final allocations from scheduled_items
    print("\n## Final Active Scheduled Service Allocations (Never Unscheduled)")
    print("------------------------------------------------------------------")
    
    # Process remaining allocations into a reporting registry
    final_registry = {}
    for item in scheduled_items.values():
        svc_base = clean_service_name(item['service'])
        sha_hash = extract_sha256(item['service'])
        node_id = item['node']
        
        if svc_base not in final_registry:
            final_registry[svc_base] = {}
        if node_id not in final_registry[svc_base]:
            final_registry[svc_base][node_id] = set()
            
        final_registry[svc_base][node_id].add(sha_hash)

    if not final_registry:
        print("No services are currently left scheduled on any node (all were cleared/unscheduled).")
    else:
        for service, nodes_map in sorted(final_registry.items()):
            print(f"Service: {service}")
            for node_id, hashes in sorted(nodes_map.items()):
                hash_str = ", ".join(sorted(list(hashes)))
                print(f"  -> Actively Scheduled on Node {node_id}: [{hash_str}]")

    print("\n## Individual Node Resource Utilization")
    print("------------------------------------------")
    if total_simulation_duration_ns > 0:
        for node_id_int, busy_time_ns, percentage in text_results:
            print(f"\nNode {node_id_int}:")
            print(f"  Total Busy Time: {busy_time_ns} nanoseconds")
            print(f"  Utilization: {percentage:.4f}%")
    else:
        print("Warning: Total simulation duration is zero.")

    overall_total_busy_time_us = overall_total_busy_time_ns / US_TO_NS
    print("\n## Overall Resource Utilization Summary")
    print("------------------------------------------")
    print(f"Overall Total Busy Time (All Nodes): {overall_total_busy_time_us} microseconds")
    print(f"Average Utilization (All Nodes): {average_utilization_percentage:.4f}%")
    print(f"Coefficient of Variation (load distribution): {coef_of_variation:.4f}")

    if generate_graph == "true":
        generate_gantt_chart(
            completed_utilization_runs, 
            output_filename_base, 
            f"Resource Utilization (Actual) - {output_filename_base}{' - Stopped at ' + str(stop_time) + 's' if stop_time else ''}", 
            workflow_start_time_ns, 
            total_simulation_duration_ns
        )
    
        completed_schedule_runs = list(scheduled_items.values())
        base, ext = os.path.splitext(output_filename_base)
        sched_filename = f"{base}_scheduling{ext}"
    
        generate_gantt_chart(
            completed_schedule_runs, 
            sched_filename, 
            f"Resource Scheduling (Planned) - {output_filename_base}{' - Stopped at ' + str(stop_time) + 's' if stop_time else ''}", 
            workflow_start_time_ns, 
            total_simulation_duration_ns
        )

# --- Execute the Analysis ---
print("Processing NFD logs!\n")

parser = argparse.ArgumentParser(description="Process NFD logs to count packets, calculate utilization values, and create allocation graphs.")
parser.add_argument("-l", "--logfile", type=str, default="scenario.log", help="The name of the input log file.")
parser.add_argument("-o", "--output", type=str, default="default_image.png", help="The name of the target image file.")
parser.add_argument("-s", "--stop", type=float, help="The stop time limit in seconds.")
parser.add_argument("--graph", type=str, default="false", help="Enable graph generation. Pass true or false.")

args = parser.parse_args()
target_image_name = args.output
generate_graph = args.graph
output_file_path = args.logfile

stop_time_limit = None
if args.stop:
    stop_time_limit = args.stop

stop_desc = f"{stop_time_limit}s" if stop_time_limit is not None else "End of File"
print(f"Configuration: Output='{target_image_name}', Stop Time={stop_desc}")
print(f"Graph Generation: {'Enabled' if generate_graph == 'true' else 'Disabled'}")

# Packet Counters
keyword1 = 'CABEEE'
keyword2 = 'onIncomingSDInterestFromApp'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"SD Interest Packets Generated: {count} interests")
keyword1 = 'CABEEE'
keyword2 = 'onOutgoingSDDataToApp'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"SD Data Packets Generated: {count} data\n")

keyword1 = 'CABEEE'
keyword2 = 'onIncomingSDInterestFromFace'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"SD Interest Packets Transmitted: {count} interests")
keyword1 = 'CABEEE'
keyword2 = 'onOutgoingSDDataToFace'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"SD Data Packets Transmitted: {count} data\n")

keyword1 = 'CABEEE'
keyword2 = 'onIncomingWFInterestFromApp'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"WF Interest Packets Generated: {count} interests")
keyword1 = 'CABEEE'
keyword2 = 'onOutgoingWFDataToApp'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"WF Data Packets Generated: {count} data\n")

keyword1 = 'CABEEE'
keyword2 = 'onIncomingWFInterestFromFace'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"WF Interest Packets Transmitted: {count} interests")
keyword1 = 'CABEEE'
keyword2 = 'onOutgoingWFDataToFace'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"WF Data Packets Transmitted: {count} data\n")

analyze_and_plot_all(output_file_path, target_image_name, stop_time_limit)