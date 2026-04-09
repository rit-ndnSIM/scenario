import os
import json
import argparse

def update_path(old_path, subfolder="cpm_100"):
    """
    Takes a path like /path/to/file.txt 
    Returns /path/to/cpm_100/file.txt
    """
    if not old_path or not isinstance(old_path, str):
        return old_path
        
    directory, filename = os.path.split(old_path)
    return os.path.join(directory, subfolder, filename)

def modify_json_paths(input_dir, output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    processed_count = 0

    for filename in os.listdir(input_dir):
        if filename.endswith(".json"):
            file_path = os.path.join(input_dir, filename)
            
            with open(file_path, 'r') as f:
                try:
                    data = json.load(f)
                except json.JSONDecodeError:
                    continue

            # 1. Update the 'topofile' key
            if "topofile" in data:
                data["topofile"] = update_path(data["topofile"])

            # 2. Update 'workflowFile' inside 'routerHosting' list
            if "routerHosting" in data and isinstance(data["routerHosting"], list):
                for item in data["routerHosting"]:
                    if "workflowFile" in item:
                        item["workflowFile"] = update_path(item["workflowFile"])

            # Save the modified file
            output_path = os.path.join(output_dir, filename)
            with open(output_path, 'w') as f:
                json.dump(data, f, indent=2)
            
            processed_count += 1

    print(f"Success: {processed_count} files updated and saved to: {output_dir}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Inject 'cpm_100' into JSON file paths.")
    parser.add_argument("-i", "--input", required=True, help="Source directory")
    parser.add_argument("-o", "--output", required=True, help="Destination directory")

    args = parser.parse_args()
    
    modify_json_paths(
        os.path.abspath(os.path.expanduser(args.input)),
        os.path.abspath(os.path.expanduser(args.output))
    )
