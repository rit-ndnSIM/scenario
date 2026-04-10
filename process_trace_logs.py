import csv
import argparse

def analyze_cs_trace(filename):
    """Reports the grand total of Cache Hits and Cache Misses."""
    total_hits = 0
    total_misses = 0
    
    with open(filename, 'r') as f:
        reader = csv.DictReader(f, delimiter='\t')
        for row in reader:
            # Clean whitespace and split by '(' to handle names like 'CacheMisses(cabeee)'
            trace_type = row['Type(cabeee)'].strip()
            packets = int(row['Packets'])
            
            if "CacheHits" in trace_type:
                total_hits += packets
            elif "CacheMisses" in trace_type:
                total_misses += packets
                
    print("--- CS Trace Summary ---")
    print(f"Total Cache Hits:   {total_hits}")
    print(f"Total Cache Misses: {total_misses}")

def analyze_cs_usage(filename):
    """
    Averages the 'CsTotalUsage' values specifically for the 'All Nodes' entries.
    """
    all_nodes_usages = []
    
    with open(filename, 'r') as f:
        reader = csv.DictReader(f, delimiter='\t')
        for row in reader:
            # Only process the summary line provided in the log
            if row['Node'].strip() == "All Nodes":
                all_nodes_usages.append(int(row['CsTotalUsage']))
            
    if not all_nodes_usages:
        print("Error: No 'All Nodes' entries found in the file.")
        return

    avg_usage = sum(all_nodes_usages) / len(all_nodes_usages)
    
    print("--- CS Usage Summary ---")
    print(f"Average Total Usage (All Nodes) across {len(all_nodes_usages)} intervals: {avg_usage:.2f}")

def analyze_rate_trace(filename):
    """Calculates the grand total of the Kilobytes column."""
    total_kilobytes = 0.0
    
    with open(filename, 'r') as f:
        reader = csv.DictReader(f, delimiter='\t')
        for row in reader:
            total_kilobytes += float(row['Kilobytes'])
            
    print("--- Rate Trace Summary ---")
    print(f"Total Kilobytes: {total_kilobytes:.2f} KB")

def main():
    parser = argparse.ArgumentParser(description="Analyze NDN trace metrics.")
    parser.add_argument("filename", help="Path to the TSV file")
    parser.add_argument("type", choices=["cs-trace", "cs-usage", "rate-trace"], 
                        help="The format/type of the trace file")
    
    args = parser.parse_args()
    
    try:
        if args.type == "cs-trace":
            analyze_cs_trace(args.filename)
        elif args.type == "cs-usage":
            analyze_cs_usage(args.filename)
        elif args.type == "rate-trace":
            analyze_rate_trace(args.filename)
    except FileNotFoundError:
        print(f"Error: File '{args.filename}' not found.")
    except KeyError as e:
        print(f"Error: Missing expected column {e}. Check if the file is tab-spaced.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    main()