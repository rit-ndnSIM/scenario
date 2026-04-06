import os
import json
import argparse

def process_files(input_dir, output_dir):
    # Create output directory if it doesn't exist
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"Created output directory: {output_dir}")

    # Count processed files
    count = 0

    for filename in os.listdir(input_dir):
        # Filter for relevant files
        if "nescoSCOPT" in filename and filename.endswith(".json"):
            input_path = os.path.join(input_dir, filename)
            
            # Load the JSON data
            try:
                with open(input_path, 'r') as f:
                    data = json.load(f)
            except Exception as e:
                print(f"Error reading {filename}: {e}")
                continue

            # Modify the 'prefix' parameter
            if "prefix" in data and data["prefix"] == "nescoSCOPT":
                data["prefix"] = "nesco"

            # Determine new filename and path
            new_filename = filename.replace("nescoSCOPT", "nesco")
            output_path = os.path.join(output_dir, new_filename)

            # Save the new file
            with open(output_path, 'w') as f:
                json.dump(data, f, indent=2)
            
            count += 1
            print(f"Generated: {new_filename}")

    print(f"\nDone! Successfully replicated {count} files to {output_dir}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Replicate and modify nescoSCOPT JSON files.")
    
    # Add command line arguments
    parser.add_argument("-i", "--input", required=True, help="Directory containing the source JSON files")
    parser.add_argument("-o", "--output", required=True, help="Directory where modified files will be saved")

    args = parser.parse_args()

    # Expand user paths (handles things like ~/Desktop)
    in_dir = os.path.abspath(os.path.expanduser(args.input))
    out_dir = os.path.abspath(os.path.expanduser(args.output))

    process_files(in_dir, out_dir)
