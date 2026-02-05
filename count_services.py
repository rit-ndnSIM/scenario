import re
from collections import Counter

def count_services(log_file_path):
    # Regex breakdown:
    # /shortcutOPT/ : Matches the literal prefix
    # ([^/]+)       : Capturing group: matches everything until the next slash (the service name)
    # /params       : Matches the literal suffix
    service_pattern = re.compile(r'/shortcutOPT/([^/]+)/params')
    
    service_counts = Counter()

    try:
        with open(log_file_path, 'r') as file:
            for line in file:
                match = service_pattern.search(line)
                if match:
                    service_name = match.group(1)
                    service_counts[service_name] += 1
                    
        return service_counts

    except FileNotFoundError:
        print(f"Error: The file '{log_file_path}' was not found.")
        return None

# --- Execution ---
log_path = 'log.txt'  # Change this to your actual filename
results = count_services(log_path)

if results:
    print(f"{'Service Name':<20} | {'Count':<5}")
    print("-" * 30)
    # Sorting by count (highest first)
    for service, count in results.most_common():
        print(f"{service:<20} | {count:<5}")
