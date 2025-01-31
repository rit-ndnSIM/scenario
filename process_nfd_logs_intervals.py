import re
import numpy as np

# Function to parse the file and compute the stats
def process_latency_file(file_path):
    # Regular expression to match the desired lines
    pattern = r"Service Latency \d+/\d+: (\d+) microseconds"

    # List to store all extracted trial results
    trial_results = []

    try:
        # Open the file and process line by line
        with open(file_path, 'r') as file:
            for line in file:
                match = re.search(pattern, line)
                if match:
                    # Extract the trial result (z)
                    trial_result = int(match.group(1))
                    trial_results.append(trial_result)

        # If no results were found, handle the empty list case
        if not trial_results:
            print("No matching lines were found in the file.")
            return

        # Calculate statistics
        total = sum(trial_results)
        minimum = min(trial_results)
        low_quartile = np.quantile(trial_results, 0.25)
        average = total / len(trial_results)
        high_quartile = np.quantile(trial_results, 0.75)
        maximum = max(trial_results)

        # Print the results
        print("")
        print(f"min latency: {minimum} microseconds")
        print(f"low latency: {low_quartile} microseconds")
        print(f"avg latency: {average} microseconds")
        print(f"high latency: {high_quartile} microseconds")
        print(f"max latency: {maximum} microseconds")
        print(f"total latency: {total} microseconds")
        print("")

    except FileNotFoundError:
        print(f"Error: The file '{file_path}' was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

# Example usage
if __name__ == "__main__":
    print("Processing intervals log file!")
    file_path = "scenario.log"
    process_latency_file(file_path)
