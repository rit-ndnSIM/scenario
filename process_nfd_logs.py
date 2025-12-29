import os
import re
import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import random



# Regex to capture the timestamp at the very beginning of the line
# Matches: +1.011524240s ...
LINE_TIMESTAMP_PATTERN = re.compile(r"^\+(?P<seconds>\d+\.\d+)s")

def get_line_seconds(line):
    """Extracts the floating point timestamp from the start of the log line."""
    match = LINE_TIMESTAMP_PATTERN.match(line)
    if match:
        return float(match.group('seconds'))
    return None

'''
def count_specific_lines(output_file_path, keyword1, keyword2):
    count = 0
    with open(output_file_path, 'r', encoding='utf-8') as file:
        for line in file:
            if keyword1 in line and keyword2 in line:
                count += 1
            if "serviceOrchestration/reset" in line:
                count = 0
    return count
'''
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


'''


# --- 2. Function to Process Log Data ---
def analyze_resource_utilization(file_path):
    if not os.path.exists(file_path):
        print(f"Error: Log file not found at {file_path}")
        return

    try:
        with open(file_path, 'r') as f:
            log_data = f.read()
    except Exception as e:
        print(f"Error reading file: {e}")
        return

    # Initialize variables for time extraction
    workflow_start_time_ns = 0
    final_service_latency_ns = 0

    # Initialize data structures (Dictionaries for accumulation, list for sorting)
    active_runs_us = {} 
    node_busy_time_ns = {}
    
    # Iterate line by line to extract required metrics
    for line in log_data.splitlines():
        # A. Extract Simulation Times
        match_start = WORKFLOW_START_PATTERN.search(line)
        if match_start:
            workflow_start_time_ns = int(match_start.group('start_time'))

        match_latency = FINAL_LATENCY_PATTERN.search(line)
        if match_latency:
            final_service_latency_ns = int(match_latency.group('latency'))

        # B. Extract Node Activity and Calculate Busy Time
        match_activity = NODE_ACTIVITY_PATTERN.search(line)
        if match_activity:
            data = match_activity.groupdict()
            node_id = data['node_id']
            service_name = data['service_name']
            action = data['action']
            timestamp_ns = int(data['timestamp'])
            
            run_key = (service_name, node_id) 

            if action == 'started':
                active_runs_ns[run_key] = timestamp_ns
                if node_id not in node_busy_time_ns:
                    node_busy_time_ns[node_id] = 0

            elif action == 'finished':
                if run_key in active_runs_ns:
                    start_time_ns = active_runs_ns.pop(run_key)
                    busy_time_ns = timestamp_ns - start_time_ns
                    
                    busy_time_us = busy_time_ns / US_TO_NS
                    node_busy_time_ns[node_id] += busy_time_ns

    # Total duration is the final latency relative to the start
    total_workflow_duration_ns = final_service_latency_ns

    # --- 3. Calculate Overall Metrics ---
    overall_total_busy_time_ns = 0
    total_utilization_percentage = 0.0
    number_of_nodes = 0
    results = []

    if total_workflow_duration_ns > 0:
        for node_id, busy_time_ns in node_busy_time_ns.items():
            overall_total_busy_time_ns += busy_time_ns
            
            utilization_percentage = (busy_time_ns / total_workflow_duration_ns) * 100
            total_utilization_percentage += utilization_percentage
            number_of_nodes += 1
            
            # Store for printing in sorted order
            results.append((int(node_id), busy_time_ns, utilization_percentage))
            
        results.sort(key=lambda x: x[0])
        
        average_utilization_percentage = total_utilization_percentage / number_of_nodes if number_of_nodes > 0 else 0.0
    
    # --- 4. Print Section ---
    print("\n## Simulation Time Summary")
    print("----------------------------")
    print(f"Workflow Start Time: {workflow_start_time_ns:,} nanoseconds")
    print(f"Total Workflow Service Latency: {final_service_latency_ns:,} nanoseconds")
    #print(f"Total Workflow Duration: {total_workflow_duration_ns:,} nanoseconds")

    print("\n## Individual Node Resource Utilization")
    print("------------------------------------------")

    if total_workflow_duration_ns > 0:
        for node_id_int, busy_time_ns, percentage in results:
            print(f"\nNode {node_id_int}:")
            print(f"  Total Busy Time: {busy_time_ns:,} nanoseconds")
            print(f"  Utilization: {percentage:.4f}%")
    else:
        print("Error: Total workflow duration is zero or was not found.")

    overall_total_busy_time_us = overall_total_busy_time_ns / US_TO_NS

    print("\n## Overall Resource Utilization Summary")
    print("------------------------------------------")
    print(f"Overall Total Busy Time (All Nodes): {overall_total_busy_time_us} microseconds")
    print(f"Average Utilization (All Nodes): {average_utilization_percentage:.4f}%")



def analyze_and_plot_resources(file_path, figure_filename):
    if not os.path.exists(file_path):
        print(f"Error: Log file not found at {file_path}")
        return

    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            log_data = f.read()
    except Exception as e:
        print(f"Error reading file: {e}")
        return

    # --- Data Extraction ---
    workflow_start_time_ns = 0
    final_service_latency_ns = 0
    
    active_runs_ns = {} 
    node_busy_time_ns = {}
    
    # List to store completed runs for the Plot: tuples of (Node, Service, Start_ns, End_ns)
    completed_runs_for_plot = [] 

    for line in log_data.splitlines():
        # Times
        match_start = WORKFLOW_START_PATTERN.search(line)
        if match_start:
            workflow_start_time_ns = int(match_start.group('start_time'))

        match_latency = FINAL_LATENCY_PATTERN.search(line)
        if match_latency:
            final_service_latency_ns = int(match_latency.group('latency'))

        # Activity
        match_activity = NODE_ACTIVITY_PATTERN.search(line)
        if match_activity:
            data = match_activity.groupdict()
            node_id = int(data['node_id'])
            service_name = data['service_name']
            action = data['action']
            timestamp_ns = int(data['timestamp'])
            
            run_key = (service_name, node_id) 

            if action == 'started':
                active_runs_ns[run_key] = timestamp_ns
                if node_id not in node_busy_time_ns:
                    node_busy_time_ns[node_id] = 0

            elif action == 'finished':
                if run_key in active_runs_ns:
                    start_time_ns = active_runs_ns.pop(run_key)
                    busy_time_ns = timestamp_ns - start_time_ns
                    
                    busy_time_us = busy_time_ns / US_TO_NS
                    node_busy_time_ns[node_id] += busy_time_ns
                    
                    # Store data for the timeline plot
                    completed_runs_for_plot.append({
                        'node': node_id,
                        'service': service_name,
                        'start_ns': start_time_ns,
                        'end_ns': timestamp_ns,
                        'duration_ns': busy_time_ns
                    })

    # Duration Calculation
    total_simulation_duration_ns = final_service_latency_ns

    # --- Metrics Output (Text) ---
    overall_total_busy_time_ns = 0
    total_utilization_percentage = 0.0
    number_of_nodes = 0
    results = []

    if total_simulation_duration_ns > 0:
        for node_id, busy_time_ns in node_busy_time_ns.items():
            overall_total_busy_time_ns += busy_time_ns
            utilization_percentage = (busy_time_ns / total_simulation_duration_ns) * 100
            total_utilization_percentage += utilization_percentage
            number_of_nodes += 1
            results.append((node_id, busy_time_ns, utilization_percentage))
            
        results.sort(key=lambda x: x[0])
        average_utilization_percentage = total_utilization_percentage / number_of_nodes if number_of_nodes > 0 else 0.0
    else:
        average_utilization_percentage = 0.0
    
    print("\n## Simulation Time Summary")
    print("----------------------------")
    print(f"Workflow Start Time: {workflow_start_time_ns} nanoseconds")
    print(f"Final Service Latency: {final_service_latency_ns} nanoseconds")
    print(f"Total Simulation Duration: {total_simulation_duration_ns} nanoseconds")

    print("\n## Individual Node Resource Utilization")
    print("------------------------------------------")

    if total_simulation_duration_ns > 0:
        for node_id_int, busy_time_ns, percentage in results:
            print(f"\nNode {node_id_int}:")
            print(f"  Total Busy Time: {busy_time_ns} nanoseconds")
            print(f"  Utilization: {percentage:.4f}%")
    else:
        print("Error: Total simulation duration is zero or was not found.")

    overall_total_busy_time_us = overall_total_busy_time_ns / US_TO_NS
    print("\n## Overall Resource Utilization Summary")
    print("------------------------------------------")
    print(f"Overall Total Busy Time (All Nodes): {overall_total_busy_time_us} microseconds")
    print(f"Average Utilization (All Nodes): {average_utilization_percentage:.4f}%")

    # --- 4. Graphical Plotting Section ---
    print("\nGenerating Graphical Timeline (resource_timeline.png)...")
    
    if not completed_runs_for_plot:
        print("No completed runs found to plot.")
        return

    # A. Normalize Time
    # Convert Workflow Start to microseconds
    workflow_start_us = workflow_start_time_ns // 1000
    
    # B. Setup Colors
    # Get unique service names to assign colors
    unique_services = sorted(list(set(run['service'] for run in completed_runs_for_plot)))
    # Create a color map using matplotlib colormaps
    cmap = plt.get_cmap('tab10') # qualitative colormap
    service_colors = {service: cmap(i % 10) for i, service in enumerate(unique_services)}

    # C. Prepare Plot
    fig, ax = plt.subplots(figsize=(12, 6))

    # Y-Axis Settings
    # We want Node 0 at the top, or ordered 0, 1, 2. Let's list all possible nodes.
    all_nodes = sorted(list(set(run['node'] for run in completed_runs_for_plot)))
    # We will plot bars at y=node_id * 10 (spacing)
    y_positions = [n * 10 for n in all_nodes]
    y_labels = [f"Node {n}" for n in all_nodes]
    
    bar_height = 8 # thickness of the bar

    # D. Plot Bars
    for run in completed_runs_for_plot:
        node_y = run['node'] * 10
        
        # Calculate Start relative to Workflow Start (in milliseconds for readability)
        rel_start_ms = (run['start_ns'] - workflow_start_ns) / 1000000.0
        duration_ms = run['duration_ns'] / 1000000.0
       
        # Draw the bar 
        ax.broken_barh(
            [(rel_start_ms, duration_ms)], # list of (start, width)
            (node_y - bar_height/2, bar_height), # (y_start, height)
            facecolors=service_colors[run['service']],
            edgecolor='black',
            linewidth=0.5
        )

        # Add the label inside the bar
        # Calculate center position for the text
        text_x = rel_start_ms + (duration_ms / 2)
        text_y = node_y
        
        ax.text(
            text_x, 
            text_y, 
            run['service'],          # Text to display
            ha='center', va='center', # Center horizontally and vertically
            color='white',            # White text for contrast
            fontsize=9,               # A slightly smaller font fits better
            weight='bold',            # Bold text is easier to read
            clip_on=True              # Ensure text doesn't spill outside the plot area
        )


    # E. Formatting
    ax.set_yticks(y_positions)
    ax.set_yticklabels(y_labels)
    ax.set_xlabel('Time (milliseconds since Simulation Start)')
    #ax.set_title('Resource Utilization Timeline by Node')
    ax.set_title('Resource Utilization Timeline  -  ' + figure_filename)
    ax.grid(True, axis='x', linestyle='--', alpha=0.7)
    ax.set_xlim(left=0)

    # Legend
    legend_patches = [mpatches.Patch(color=color, label=service) for service, color in service_colors.items()]
    ax.legend(handles=legend_patches, title="Services", loc='upper right')

    # F. Save Plot
    plt.tight_layout()
    plt.savefig(figure_filename)
    print(f"Plot saved successfully to: {figure_filename}")

'''







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

'''
# --- 4. Main Analysis Logic ---
def analyze_and_plot_all(file_path, output_filename_base):
    if not os.path.exists(file_path):
        print(f"Error: Log file not found at {file_path}")
        return

    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            log_data = f.read()
    except Exception as e:
        print(f"Error reading file: {e}")
        return

    # --- Data Containers ---
    workflow_start_time_ns = 0
    final_service_latency_ns = 0
    
    # For Utilization (Actual Runs)
    active_runs_ns = {} 
    node_busy_time_ns = {}
    completed_utilization_runs = [] 
    
    # For Scheduling (Planned Runs)
    # Key: (node, service, start_ns, end_ns) -> Value: dict with details
    scheduled_items = {} 

    # --- Parse Log ---
    for line in log_data.splitlines():
        # A. Simulation Times
        match_start = WORKFLOW_START_PATTERN.search(line)
        if match_start:
            workflow_start_time_ns = int(match_start.group('start_time'))

        match_latency = FINAL_LATENCY_PATTERN.search(line)
        if match_latency:
            final_service_latency_ns = int(match_latency.group('latency'))

        # B. Resource Utilization
        match_util = NODE_ACTIVITY_PATTERN.search(line)
        if match_util:
            data = match_util.groupdict()
            node_id = int(data['node_id'])
            service_name = data['service_name']
            action = data['action']
            timestamp_ns = int(data['timestamp'])
            
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

        # C. Resource Scheduling
        match_sched = SCHEDULING_PATTERN.search(line)
        if match_sched:
            data = match_sched.groupdict()
            node_id = int(data['node_id'])
            service_name = data['service_name']
            status = data['status']
            start_ns = int(data['start_ns'])
            end_ns = int(data['end_ns'])
            
            # Unique key for this specific scheduled block
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
    total_simulation_duration_ns = final_service_latency_ns
    overall_total_busy_time_ns = 0
    total_utilization_percentage = 0.0
    number_of_nodes = 0
    text_results = []

    if total_simulation_duration_ns > 0:
        for node_id, busy_time_ns in node_busy_time_ns.items():
            overall_total_busy_time_ns += busy_time_ns
            utilization_percentage = (busy_time_ns / total_simulation_duration_ns) * 100
            total_utilization_percentage += utilization_percentage
            number_of_nodes += 1
            text_results.append((node_id, busy_time_ns, utilization_percentage))
        
        text_results.sort(key=lambda x: x[0])
        average_utilization_percentage = total_utilization_percentage / number_of_nodes if number_of_nodes > 0 else 0.0
    else:
        average_utilization_percentage = 0.0

    print("\n## Simulation Time Summary")
    print("----------------------------")
    print(f"Workflow Start Time: {workflow_start_time_ns} nanoseconds")
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
        print("Error: Total simulation duration is zero or was not found.")

    overall_total_busy_time_us = overall_total_busy_time_ns / US_TO_NS
    print("\n## Overall Resource Utilization Summary")
    print("------------------------------------------")
    print(f"Overall Total Busy Time (All Nodes): {overall_total_busy_time_us} microseconds")
    print(f"Average Utilization (All Nodes): {average_utilization_percentage:.4f}%")

    # --- Generate Plots ---
    
    # 1. Utilization Plot (Filename as provided)
    generate_gantt_chart(
        completed_utilization_runs, 
        output_filename_base, 
        f"Resource Utilization Timeline (Actual) - {output_filename_base}", 
        workflow_start_time_ns, 
        total_simulation_duration_ns
    )
    
    # 2. Scheduling Plot (Filename + _scheduling.png)
    completed_schedule_runs = list(scheduled_items.values())
    base, ext = os.path.splitext(output_filename_base)
    sched_filename = f"{base}_scheduling{ext}"
    
    generate_gantt_chart(
        completed_schedule_runs, 
        sched_filename, 
        f"Resource Scheduling Timeline (Planned) - {output_filename_base}", 
        workflow_start_time_ns, 
        total_simulation_duration_ns
    )
'''

# --- 4. Main Analysis Logic ---
def analyze_and_plot_all(file_path, output_filename_base, stop_time=None):
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
        match_util = NODE_ACTIVITY_PATTERN.search(line)
        if match_util:
            data = match_util.groupdict()
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


# --- Execute the Analysis ---

#TODO: Delete any leading spaces to make sure all entries have timestamp at the beginning of the line!
#TODO: Add node name after the timestamp when merging!

print("Processing NFD logs!")

output_file_path = 'scenario.log'

print("")


# Parse Arguments flexibly
# Usage: python script.py [image_name] [stop_time_float]
target_image_name = 'resource_timeline.png'
stop_time_limit = None

args = sys.argv[1:]
for arg in args:
    try:
        # If the argument is a float number, treat it as stop time
        val = float(arg)
        stop_time_limit = val
    except ValueError:
        # If it's not a number, treat it as the filename
        target_image_name = arg

if stop_time_limit:
    print(f"Configuration: Output='{target_image_name}', Stop Time={stop_time_limit}s")
else:
    print(f"Configuration: Output='{target_image_name}', Stop Time=End of File")


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


#analyze_resource_utilization(output_file_path)

#analyze_and_plot_resources(output_file_path, target_image_name)

analyze_and_plot_all(output_file_path, target_image_name, stop_time_limit)
