import os
import re
import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import random
import argparse
from collections import defaultdict
import numpy as np



# Regex to capture the timestamp at the very beginning of the line
# Matches: +1.011524240s ...
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
                # Check stop time
                if stop_time is not None:
                    t = get_line_seconds(line)
                    if t is not None and t > stop_time:
                        break # Stop reading file

                if "serviceOrchestration/reset" in line:
                    count = 0
                    continue
                if keyword1 in line and keyword2 in line:
                    count += 1
    except FileNotFoundError:
        return 0
    return count

def clean_service_name(name):
    """
    Shortens service names for display. 
    Ex: '/sensor/params-sha256=...' -> '/sensor'
    """
    if '/params' in name:
        return name.split('/params')[0]
    return name

def normalize_to_ns(value, unit):
    """
    Crucial fix: Converts value to nanoseconds based on the unit string.
    """
    if 'microsecond' in unit:
        return int(value) * 1000
    return int(value)




# Determine the resource utilization


# --- Configuration ---
US_TO_NS = 1000

# --- 1. Define Regex Patterns ---
NODE_ACTIVITY_PATTERN = re.compile(
    #r".*?NFDServiceDiscovery - (WF|SD)?resourceAllocation: Service (?P<service_name>\S+) (?P<action>started|finished) running on node (?P<node_id>\d+)\. .*? at (?P<timestamp>\d+) (nanoseconds|microseconds)"
    r".*?NFDServiceDiscovery - (WF|SD)?resourceAllocation: Service (?P<service_name>\S+) (?P<action>started|finished) running on node (?P<node_id>\d+)\. .*? at (?P<timestamp>\d+) (?P<unit>nanoseconds|microseconds)"
)
SCHEDULING_PATTERN = re.compile(
    #r".*?NFDServiceDiscovery - SDresourceAllocation: Service (?P<service_name>\S+) (?P<status>scheduled|no longer scheduled) on node (?P<node_id>\d+) starting at (?P<start_ns>\d+) and ending at (?P<end_ns>\d+) nanoseconds"
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
POISSON_DETECTOR_PATTERN = re.compile(
    r"Poisson consumer is being used\.\s+Number of interests\s*=\s*(?P<interests>\d+),\s+frequency\s*=\s*(?P<frequency>\d+)"
)

# Regex for: Final answer for consumer node 21, service /consumerP, interest # 1/100: 100
FINAL_ANSWER_PATTERN = re.compile(
    r"Final answer for consumer node\s+\d+, service\s+(/[^,]+), interest # \d+/\d+: ([\d\.]+)"
)
# Regex for: Service Latency for consumer node 21, service /consumerP, interest # 1/100: 158815 microseconds.
# Note: We ignore the "milliseconds" line to maintain microsecond precision.
LATENCY_PATTERN = re.compile(
    r"Service Latency for consumer node\s+\d+, service\s+(/[^,]+), interest # \d+/\d+: (\d+) microseconds"
)

SD_LATENCY_PATTERN = re.compile(
    r"^\s*Service Discovery Latency:\s*(?P<latency>\d+)\s+microseconds"
)



def generate_gantt_chart(runs_data, output_filename, title, workflow_start_ns, total_duration_ns):
    """
    Generates a Gantt chart from a list of run dictionaries.
    runs_data: list of dicts {node, service, start_ns, duration_ns}
    """
    print(f"\nGenerating {title} ({output_filename})...")
    
    if not runs_data:
        print(f"No data found for {title}.")
        return

    # A. Setup Colors
    unique_services = sorted(list(set(clean_service_name(run['service']) for run in runs_data)))
    cmap = plt.get_cmap('tab10') 
    service_colors = {service: cmap(i % 10) for i, service in enumerate(unique_services)}

    # B. Prepare Plot
    fig, ax = plt.subplots(figsize=(12, 6))

    all_nodes = sorted(list(set(run['node'] for run in runs_data)))
    y_positions = [n * 10 for n in all_nodes]
    y_labels = [f"Node {n}" for n in all_nodes]
    
    bar_height = 8

    # C. Plot Bars
    for run in runs_data:
        node_y = run['node'] * 10
        
        # Normalize Time: (Current_NS - WorkflowStart_NS) / 1,000,000 => Milliseconds
        rel_start_ms = (run['start_ns'] - workflow_start_ns) / 1000000.0
        duration_ms = run['duration_ns'] / 1000000.0
        
        svc_clean = clean_service_name(run['service'])
        
        # 1. Draw Bar
        ax.broken_barh(
            [(rel_start_ms, duration_ms)],
            (node_y - bar_height/2, bar_height),
            facecolors=service_colors[svc_clean],
            edgecolor='black',
            linewidth=0.5
        )
        
        # 2. Add Label
        text_x = rel_start_ms + (duration_ms / 2)
        text_y = node_y
        
        # Only add text if the bar is wide enough (e.g., > 0.1ms)
        if duration_ms > 0.1:
            ax.text(
                text_x, text_y, svc_clean,
                ha='center', va='center',
                color='white', fontsize=8, weight='bold', clip_on=True
            )

    # D. Formatting
    ax.set_yticks(y_positions)
    ax.set_yticklabels(y_labels)
    ax.set_xlabel('Time (milliseconds since Workflow Start)')
    ax.set_title(title)
    ax.grid(True, axis='x', linestyle='--', alpha=0.7)

    # X-Axis limits
    ax.set_xlim(left=0)
    if total_duration_ns > 0:
        sim_duration_ms = total_duration_ns / 1000000.0
        current_max = ax.get_xlim()[1]
        ax.set_xlim(right=max(current_max, sim_duration_ms))

    # Legend
    legend_patches = [mpatches.Patch(color=color, label=service) for service, color in service_colors.items()]
    ax.legend(handles=legend_patches, title="Services", loc='upper right')

    plt.tight_layout()
    plt.savefig(output_filename)
    plt.close(fig) 
    print(f"Plot saved successfully to: {output_filename}")



def analyze_and_plot(file_path, output_filename_base, poisson_detected, num_interests, stop_time=None):
    if not os.path.exists(file_path):
        print(f"Error: Log file not found at {file_path}")
        return

    try:
        # Use 'errors=ignore' to handle potential encoding issues with binary data in logs
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            # We must iterate line by line to support the stop_time check efficiently
            log_lines = f.readlines()
    except Exception as e:
        print(f"Error reading file: {e}")
        return

    # --- Data Containers ---
    workflow_start_time_ns = 0
    final_service_latency_ns = 0
    
    # Utilization Containers
    active_runs_ns = {} 
    node_busy_time_ns = {}
    completed_utilization_runs = [] 
    
    # Scheduling Containers
    scheduled_items = {} 

    number_of_nodes = 0

    print(f"Analyzing log data{' (stopping at ' + str(stop_time) + 's)' if stop_time else ''}...")

#TODO: if poisson detected, then scan entire file storing info in trial_data (like process_nfd_logs_intervals.py), and print the max, min, avg, quartiles, etc.

    if (poisson_detected == True):


       # Dictionary to store trial results by type
        trial_data = defaultdict(list)
        final_answers = {}
        
        sd_latencies_us = []

        try:
            for line in log_lines:

                # 0. Check Stop Time
                if stop_time is not None:
                    t = get_line_seconds(line)
                    if t is not None and t > stop_time:
                        # We have reached the user-defined time limit. Stop processing further lines.
                        break

                # 1. NFD node counter
                match_node_count = NFD_NODE_PATTERN.search(line)
                if match_node_count:
                    number_of_nodes += 1

                # 3. Resource Utilization (Actual)
                match_utilization = NODE_ACTIVITY_PATTERN.search(line)
                if match_utilization:
                    data = match_utilization.groupdict()
                    node_id = int(data['node_id'])
                    service_name = data['service_name']
                    action = data['action']
            
                    # Normalize timestamp to nanoseconds immediately
                    timestamp_ns = normalize_to_ns(data['timestamp'], data['unit'])

                    final_service_latency_ns = timestamp_ns
            
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


                # Look for latency
                #latency_match = re.search(LATENCY_PATTERN, line)
                latency_match = LATENCY_PATTERN.search(line)
                if latency_match:
                    # Extract the type and trial result (z) as a float
                    trial_type = latency_match.group(1).strip()
                    trial_result = int(latency_match.group(2))
                    trial_data[trial_type].append(trial_result)                   
                    continue

                    #if trial_type not in trial_data:
                        #trial_data[trial_type] = []
                    #trial_data[trial_type].append(trial_result)

                # Look for Final Answer 
                #final_answer_match = re.search(FINAL_ANSWER_PATTERN, line)
                final_answer_match = FINAL_ANSWER_PATTERN.search(line)
                if final_answer_match:
                    # Extract the type and final result as a float
                    trial_type = final_answer_match.group(1).strip()
                    final_result = int(final_answer_match.group(2))
                    
                    if trial_type in final_answers and final_answers[trial_type] != final_result:
                        print(f"Warning: Inconsistent final result for type {trial_type}!")
                    final_answers[trial_type] = final_result

                match_sd_latency = SD_LATENCY_PATTERN.search(line)
                if match_sd_latency:
                    sd_latencies_us.append(int(match_sd_latency.group('latency')))
                    continue





            # --- Text Metrics ---
    
            # If we stopped early, we might not have found the final latency line.
            # In that case, use the last known activity as the duration, or just 0 if nothing happened.
            if stop_time is not None and final_service_latency_ns == 0 and completed_utilization_runs:
                # Use the end time of the last completed run as a proxy for duration so far
                max_end_ns = max(run['start_ns'] + run['duration_ns'] for run in completed_utilization_runs)
                total_simulation_duration_ns = max_end_ns
                print(f"Note: Stop time reached. Using last recorded activity ({total_simulation_duration_ns}ns) as total duration.")
            else:
                total_simulation_duration_ns = final_service_latency_ns

            overall_total_busy_time_ns = 0
            total_utilization_percentage = 0.0
            #number_of_nodes = 0
            text_results = []

            if total_simulation_duration_ns > 0:
                for node_id, busy_time_ns in node_busy_time_ns.items():
                    overall_total_busy_time_ns += busy_time_ns
                    utilization_percentage = (busy_time_ns / total_simulation_duration_ns) * 100
                    total_utilization_percentage += utilization_percentage
                    #number_of_nodes += 1
                    text_results.append((node_id, busy_time_ns, utilization_percentage))
        
                text_results.sort(key=lambda x: x[0])
                average_utilization_percentage = total_utilization_percentage / number_of_nodes if number_of_nodes > 0 else 0.0
            else:
                average_utilization_percentage = 0.0


            # Calculate Standard Deviation and Coefficient of Variation (CV = StandardDeviation / Mean)
            variance = 0.0
            stdev = 0.0
            coef_of_variation = 0.0

            if number_of_nodes > 0 and average_utilization_percentage > 0:
                sum_squared_diff = 0.0
                for _, _, util_pct in text_results:
                    diff = util_pct - average_utilization_percentage
                    sum_squared_diff += diff * diff
        
                # Population Variance = sum((x - mean)^2) / N
                variance = sum_squared_diff / number_of_nodes
                stdev = variance ** 0.5  # Square root (equivalent to math.sqrt)
        
                coef_of_variation = stdev / average_utilization_percentage





            # If no results were found, handle the empty dictionary case
            if not trial_data:
                print("No matching lines were found in the file.")
                return

            # Print statistics for each type
            #for trial_type, trial_results in trial_data.items():
            for trial_type, trial_results in sorted(trial_data.items()):
                # Calculate statistics
                total = sum(trial_results)
                minimum = min(trial_results)
                low_quartile = np.quantile(trial_results, 0.25)
                mid_quartile = np.quantile(trial_results, 0.5)
                high_quartile = np.quantile(trial_results, 0.75)
                maximum = max(trial_results)
                average = total / len(trial_results)
                req_fulfilled = len(trial_results)

                
                print(f"Statistics for type: {trial_type}")
                #print(f"  Minimum trial result: {minimum:.2f} microseconds")
                #print(f"  Maximum trial result: {maximum:.2f} microseconds")
                #print(f"  Total of all trial results: {total:.2f} microseconds")
                #print(f"  Average trial result: {average:.2f} microseconds")
                #print(f"  {trial_type} min latency: {int(minimum)} microseconds")
                #print(f"  {trial_type} max latency: {int(maximum)} microseconds")
                #print(f"  {trial_type} Total of all trial results: {int(total)} microseconds")
                #print(f"  {trial_type} Average trial result: {int(average)} microseconds")

                print(f"  {trial_type} min latency: {int(minimum)} microseconds")
                print(f"  {trial_type} low latency: {int(low_quartile)} microseconds")
                print(f"  {trial_type} mid latency: {int(mid_quartile)} microseconds")
                print(f"  {trial_type} high latency: {int(high_quartile)} microseconds")
                print(f"  {trial_type} max latency: {int(maximum)} microseconds")
                print(f"  {trial_type} total latency: {int(total)} microseconds")
                print(f"  {trial_type} avg latency: {int(average)} microseconds")
                print(f"  {trial_type} requests fulfilled: {int(req_fulfilled)} total requests")
                if (req_fulfilled != num_interests):
                    print(f"Warning: Inconsistent number of requests fulfilled!")
                
                if trial_type in final_answers:
                    #print(f"  Final answer: {final_answers[trial_type]:.2f} microseconds")
                    print(f"  {trial_type} Final answer: {int(final_answers[trial_type])} numerical")
                print()



            # --- Global Aggregation Across All Consumers ---
            all_combined_trials = []
            global_final_answers_sum = 0
            global_req_fulfilled = 0

            # Dynamically loop through whatever consumer data your dictionary captured
            for t_type, results in trial_data.items():
                all_combined_trials.extend(results)
                global_req_fulfilled += len(results)
                global_final_answers_sum += int(final_answers.get(t_type, 0))

            print("\n## Global Consumer Statistics Summary (All Trials Combined)")
            print("------------------------------------------------------------")
            if all_combined_trials:
                print(f"Global workflow min latency: {int(np.min(all_combined_trials))} microseconds")
                print(f"Global workflow low latency: {int(np.quantile(all_combined_trials, 0.25))} microseconds")
                print(f"Global workflow mid latency: {int(np.median(all_combined_trials))} microseconds")
                print(f"Global workflow high latency: {int(np.quantile(all_combined_trials, 0.75))} microseconds")
                print(f"Global workflow max latency: {int(np.max(all_combined_trials))} microseconds")
                print(f"Global workflow total latency: {int(np.sum(all_combined_trials))} microseconds")
                print(f"Global workflow avg latency: {int(np.mean(all_combined_trials))} microseconds")
                print(f"Global workflow total requests fulfilled: {global_req_fulfilled} total requests")
                print(f"Global workflow final answers sum: {global_final_answers_sum} numerical")

                total_expected_interests = num_interests * len(trial_data)
            
                if global_req_fulfilled != total_expected_interests:
                    print(f"  Warning: Inconsistent number of global requests fulfilled! Expected: {total_expected_interests}")
            else:
                print("  No trial data discovered to compile global summaries.")
            print()

            global_sd_avg = int(np.mean(sd_latencies_us)) if sd_latencies_us else 0
            print(f"Global service discovery average latency: {global_sd_avg} microseconds")



            print("\n## Simulation Time Summary")
            print("----------------------------")
            if stop_time:
                print(f"Log Analysis Stopped At: {stop_time} seconds")
            print(f"Total Simulation Duration: {total_simulation_duration_ns} nanoseconds")

            print("\n## Individual Node Resource Utilization")
            print("------------------------------------------")
            if total_simulation_duration_ns > 0:
                for node_id_int, busy_time_ns, percentage in text_results:
                    print(f"\nNode {node_id_int}:")
                    print(f"  Total Busy Time: {busy_time_ns} nanoseconds")
                    print(f"  Utilization: {percentage:.4f}%")
            else:
                print("Warning: Total simulation duration is zero (possibly no finished jobs before stop time).")

            overall_total_busy_time_us = overall_total_busy_time_ns / US_TO_NS
            print("\n## Overall Resource Utilization Summary")
            print("------------------------------------------")
            print(f"Overall Total Busy Time (All Nodes): {overall_total_busy_time_us} microseconds")
            print(f"Average Utilization (All Nodes): {average_utilization_percentage:.4f}%")
            print(f"Coefficient of Variation (load distribution): {coef_of_variation:.4f}")
            print()
            print()



        except FileNotFoundError:
            print(f"Error: The file '{file_path}' was not found.")
        except Exception as e:
            print(f"An error occurred: {e}")


    if (poisson_detected == False):

        # --- Parse Log ---
        for line in log_lines:
            
            # 0. Check Stop Time
            if stop_time is not None:
                t = get_line_seconds(line)
                if t is not None and t > stop_time:
                    # We have reached the user-defined time limit. Stop processing further lines.
                    break

            # 1. NFD node counter
            match_node_count = NFD_NODE_PATTERN.search(line)
            if match_node_count:
                number_of_nodes += 1

            # 2. Simulation Times
            match_start = WORKFLOW_START_PATTERN.search(line)
            if match_start:
                workflow_start_time_ns = int(match_start.group('start_time'))

            match_latency = FINAL_LATENCY_PATTERN.search(line)
            if match_latency:
                final_service_latency_ns = int(match_latency.group('latency'))

            # 3. Resource Utilization (Actual)
            match_utilization = NODE_ACTIVITY_PATTERN.search(line)
            if match_utilization:
                data = match_utilization.groupdict()
                node_id = int(data['node_id'])
                service_name = data['service_name']
                action = data['action']
                
                # Normalize timestamp to nanoseconds immediately
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

            # 4. Resource Scheduling (Planned)
            match_sched = SCHEDULING_PATTERN.search(line)
            if match_sched:
                data = match_sched.groupdict()
                node_id = int(data['node_id'])
                service_name = data['service_name']
                status = data['status']
                unit = data['unit'] # Capture unit (micro/nano)
                
                # Normalize to nanoseconds immediately
                start_ns = normalize_to_ns(data['start_time'], unit)
                end_ns = normalize_to_ns(data['end_time'], unit)
                
                sched_key = (node_id, service_name, start_ns, end_ns)
                
                if status == 'scheduled':
                    scheduled_items[sched_key] = {
                        'node': node_id,
                        'service': service_name,
                        'start_ns': start_ns,
                        'duration_ns': end_ns - start_ns
                    }
                elif status == 'no longer scheduled':
                    if sched_key in scheduled_items:
                        del scheduled_items[sched_key]

        # --- Text Metrics ---
        
        # If we stopped early, we might not have found the final latency line.
        # In that case, use the last known activity as the duration, or just 0 if nothing happened.
        if stop_time is not None and final_service_latency_ns == 0 and completed_utilization_runs:
            # Use the end time of the last completed run as a proxy for duration so far
            max_end_ns = max(run['start_ns'] + run['duration_ns'] for run in completed_utilization_runs)
            total_simulation_duration_ns = max_end_ns
            print(f"Note: Stop time reached. Using last recorded activity ({total_simulation_duration_ns}ns) as total duration.")
        else:
            total_simulation_duration_ns = final_service_latency_ns

        overall_total_busy_time_ns = 0
        total_utilization_percentage = 0.0
        #number_of_nodes = 0
        text_results = []

        if total_simulation_duration_ns > 0:
            for node_id, busy_time_ns in node_busy_time_ns.items():
                overall_total_busy_time_ns += busy_time_ns
                utilization_percentage = (busy_time_ns / total_simulation_duration_ns) * 100
                total_utilization_percentage += utilization_percentage
                #number_of_nodes += 1
                text_results.append((node_id, busy_time_ns, utilization_percentage))
            
            text_results.sort(key=lambda x: x[0])
            average_utilization_percentage = total_utilization_percentage / number_of_nodes if number_of_nodes > 0 else 0.0
        else:
            average_utilization_percentage = 0.0


        # Calculate Standard Deviation and Coefficient of Variation (CV = StandardDeviation / Mean)
        variance = 0.0
        stdev = 0.0
        coef_of_variation = 0.0

        if number_of_nodes > 0 and average_utilization_percentage > 0:
            sum_squared_diff = 0.0
            for _, _, util_pct in text_results:
                diff = util_pct - average_utilization_percentage
                sum_squared_diff += diff * diff
            
            # Population Variance = sum((x - mean)^2) / N
            variance = sum_squared_diff / number_of_nodes
            stdev = variance ** 0.5  # Square root (equivalent to math.sqrt)
            
            coef_of_variation = stdev / average_utilization_percentage


        print("\n## Simulation Time Summary")
        print("----------------------------")
        print(f"Workflow Start Time: {workflow_start_time_ns} nanoseconds")
        if stop_time:
            print(f"Log Analysis Stopped At: {stop_time} seconds")
        print(f"Final Service Latency: {final_service_latency_ns} nanoseconds")
        print(f"Total Simulation Duration: {total_simulation_duration_ns} nanoseconds")

        print("\n## Individual Node Resource Utilization")
        print("------------------------------------------")
        if total_simulation_duration_ns > 0:
            for node_id_int, busy_time_ns, percentage in text_results:
                print(f"\nNode {node_id_int}:")
                print(f"  Total Busy Time: {busy_time_ns} nanoseconds")
                print(f"  Utilization: {percentage:.4f}%")
        else:
            print("Warning: Total simulation duration is zero (possibly no finished jobs before stop time).")

        overall_total_busy_time_us = overall_total_busy_time_ns / US_TO_NS
        print("\n## Overall Resource Utilization Summary")
        print("------------------------------------------")
        print(f"Overall Total Busy Time (All Nodes): {overall_total_busy_time_us} microseconds")
        print(f"Average Utilization (All Nodes): {average_utilization_percentage:.4f}%")
        print(f"Coefficient of Variation (load distribution): {coef_of_variation:.4f}")

        # --- Generate Plots ---
        
        if generate_graph == "true":
            # 1. Utilization Plot (Actual)
            generate_gantt_chart(
                completed_utilization_runs, 
                output_filename_base, 
                #f"Resource Utilization (Actual){' - Stopped at ' + str(stop_time) + 's' if stop_time else ''}", 
                f"Resource Utilization (Actual) - {output_filename_base}{' - Stopped at ' + str(stop_time) + 's' if stop_time else ''}", 
                workflow_start_time_ns, 
                total_simulation_duration_ns
            )
        
            # 2. Scheduling Plot (Planned)
            completed_schedule_runs = list(scheduled_items.values())
            base, ext = os.path.splitext(output_filename_base)
            sched_filename = f"{base}_scheduling{ext}"
        
            generate_gantt_chart(
                completed_schedule_runs, 
                sched_filename, 
                #f"Resource Scheduling (Planned){' - Stopped at ' + str(stop_time) + 's' if stop_time else ''}", 
                f"Resource Scheduling (Planned) - {output_filename_base}{' - Stopped at ' + str(stop_time) + 's' if stop_time else ''}", 
                workflow_start_time_ns, 
                total_simulation_duration_ns
            )



# =====================================================================
# CORE EXECUTION ENTRY ROUTINE
# =====================================================================

#TODO: Delete any leading spaces to make sure all entries have timestamp at the beginning of the line!
#TODO: Add node name after the timestamp when merging!

if __name__ == "__main__":

    print("Processing NFD logs!")
    print("")

    # Parse Arguments flexibly
    # Usage: python script.py [image_name] [stop_time_float]

    # 1. Initialize the parser
    parser = argparse.ArgumentParser(description="Process NFD logs to count packets, calculate utilization values, and create allocation graphs.")

    # 2. Add your arguments
    # '-l' or '--logfile' for the input logfile filename (String)
    parser.add_argument("-l", "--logfile", type=str, default="scenario.log", help="The name of the input log file.")

    # '-o' or '--output' for the output filename (String)
    parser.add_argument("-o", "--output", type=str, default="default_image.png", help="The name of the target image file.")

    # '-s' or '--stop' for the time limit (Float)
    parser.add_argument("-s", "--stop", type=float, help="The stop time limit in seconds.")

    # '--graph' flag (Boolean)
    # 'action="store_true"' means if the flag is present, the value is True. 
    # If absent, it's False.
    parser.add_argument("--graph", type=str, default="false", help="Enable graph generation. Pass true or false.")

    # 3. Parse the arguments
    args = parser.parse_args()

    if not os.path.exists(args.logfile):
        print(f"Critical System Error: Specified target file path unreadable: {args.logfile}")
        sys.exit(1)

    # 4. Use the values
    target_image_name = args.output
    generate_graph = args.graph

    #output_file_path = 'scenario.log'
    output_file_path = args.logfile

    stop_time_limit = None
    if args.stop:
        stop_time_limit = args.stop
        print(f"Configuration: Output='{target_image_name}', Stop Time={stop_time_limit}s")
    else:
        print(f"Configuration: Output='{target_image_name}', Stop Time=End of File")

    print(f"Graph Generation: {'Enabled' if generate_graph=="true" else 'Disabled'}")


     ## Count the number of Service Discovery interests

    # Count interest packets coming from application face
    keyword1 = 'CABEEE'
    keyword2 = 'onIncomingSDInterestFromApp'
    count = count_specific_lines(output_file_path, keyword1, keyword2)
    print(f"SD Interest Packets Generated: {count} interests")
    # Count data packets going to application face
    keyword1 = 'CABEEE'
    keyword2 = 'onOutgoingSDDataToApp'
    count = count_specific_lines(output_file_path, keyword1, keyword2)
    print(f"SD Data Packets Generated: {count} data")

    print("")

    # we can't trust the transmitted packets in ndnSIM because it uses an older version of NFD, which seems to end up with a higher count of transmitted packets than the real NFD
    # Count interest packets coming from any face
    keyword1 = 'CABEEE'
    keyword2 = 'onIncomingSDInterestFromFace'
    count = count_specific_lines(output_file_path, keyword1, keyword2)
    print(f"SD Interest Packets Transmitted: {count} interests")
    # Count data packets going to any face
    keyword1 = 'CABEEE'
    keyword2 = 'onOutgoingSDDataToFace'
    count = count_specific_lines(output_file_path, keyword1, keyword2)
    print(f"SD Data Packets Transmitted: {count} data")

    print("")


     ## Count the number of Workflow interests

    # Count interest packets coming from application face
    keyword1 = 'CABEEE'
    keyword2 = 'onIncomingWFInterestFromApp'
    count = count_specific_lines(output_file_path, keyword1, keyword2)
    print(f"WF Interest Packets Generated: {count} interests")
    # Count data packets going to application face
    keyword1 = 'CABEEE'
    keyword2 = 'onOutgoingWFDataToApp'
    count = count_specific_lines(output_file_path, keyword1, keyword2)
    print(f"WF Data Packets Generated: {count} data")

    print("")

    # we can't trust the transmitted packets in ndnSIM because it uses an older version of NFD, which seems to end up with a higher count of transmitted packets than the real NFD
    # Count interest packets coming from any face
    keyword1 = 'CABEEE'
    keyword2 = 'onIncomingWFInterestFromFace'
    count = count_specific_lines(output_file_path, keyword1, keyword2)
    print(f"WF Interest Packets Transmitted: {count} interests")
    # Count data packets going to any face
    keyword1 = 'CABEEE'
    keyword2 = 'onOutgoingWFDataToFace'
    count = count_specific_lines(output_file_path, keyword1, keyword2)
    print(f"WF Data Packets Transmitted: {count} data")

    print("")

    # Upfront log analysis scan pass to see if Poisson Process is used for requests, or if it was a single request.
    poisson_detected = False
    num_interests = 1 # default, in case this isn't a Poisson Process experiment
    frequency = 0
    with open(args.logfile, 'r', encoding='utf-8', errors='ignore') as scan_file:
        for line in scan_file:
            match = POISSON_DETECTOR_PATTERN.search(line)
            if match:
                poisson_detected = True
                num_interests = int(match.group('interests'))
                frequency = int(match.group('frequency'))
                break

    analyze_and_plot(output_file_path, target_image_name, poisson_detected, num_interests, stop_time_limit)

