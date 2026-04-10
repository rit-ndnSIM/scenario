import os
import json
import sys

def update_json_files(directory):
    # Define the new data to insert
    new_entries = {
        "rateTrace": 1,
        "csTrace": 1,
        "csUsage": 1
    }

    # Verify the directory exists
    if not os.path.isdir(directory):
        print(f"Error: The directory '{directory}' does not exist.")
        return

    # Iterate through all files in the directory
    for filename in os.listdir(directory):
        if filename.endswith(".json"):
            file_path = os.path.join(directory, filename)
            
            with open(file_path, 'r') as f:
                try:
                    data = json.load(f)
                except json.JSONDecodeError:
                    print(f"Skipping {filename}: Invalid JSON format.")
                    continue

            # Skip if "router" is missing or if keys already exist (optional safety)
            if "router" not in data:
                print(f"Skipping {filename}: 'router' key not found.")
                continue

            # Rebuild the dictionary to ensure order
            updated_data = {}
            for key, value in data.items():
                if key == "router":
                    updated_data.update(new_entries)
                updated_data[key] = value

            # Write the updated content back
            with open(file_path, 'w') as f:
                json.dump(updated_data, f, indent=4)
                print(f"Successfully updated: {filename}")

if __name__ == "__main__":
    # Check if the user provided the argument
    if len(sys.argv) < 2:
        print("Usage: python script_name.py /path/to/directory")
    else:
        target_dir = sys.argv[1]
        update_json_files(target_dir)