import os
import re


def count_specific_lines(output_file_path, keyword1, keyword2):
    count = 0
    with open(output_file_path, 'r', encoding='utf-8') as file:
        for line in file:
            if keyword1 in line and keyword2 in line:
                count += 1
            if "serviceOrchestration/reset" in line:
                count = 0
    return count



#TODO: Delete any leading spaces to make sure all entries have timestamp at the beginning of the line!
#TODO: Add node name after the timestamp when merging!

print("Processing NFD logs!")

output_file_path = 'scenario.log'

print("")



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



# Determine the resource utilization


# --- Configuration ---
US_TO_NS = 1000

# --- 1. Define Regex Patterns ---
NODE_ACTIVITY_PATTERN = re.compile(
    r"Service (?P<service_name>/[^ ]+) (?P<action>started|finished) running on node (?P<node_id>\d+)\. .*? at (?P<timestamp>\d+) microseconds"
)
WORKFLOW_START_PATTERN = re.compile(
    r"- workflow start: (?P<start_time>\d+) nanoseconds"
)
FINAL_LATENCY_PATTERN = re.compile(
    r"Service Latency: (?P<latency>\d+) nanoseconds\."
)

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
            timestamp_us = int(data['timestamp'])
            
            run_key = (service_name, node_id) 

            if action == 'started':
                active_runs_us[run_key] = timestamp_us
                if node_id not in node_busy_time_ns:
                    node_busy_time_ns[node_id] = 0

            elif action == 'finished':
                if run_key in active_runs_us:
                    start_time_us = active_runs_us.pop(run_key)
                    busy_time_us = timestamp_us - start_time_us
                    
                    busy_time_ns = busy_time_us * US_TO_NS
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


# --- Execute the Analysis ---
analyze_resource_utilization(output_file_path)