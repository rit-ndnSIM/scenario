#!/bin/bash
# vim: sw=8 noet

set -e

# --- 1. Define Directories ---
export NDNSIM_HOME="$HOME/ndnSIM"
export SCENARIO_DIR="$HOME/ndnSIM/scenario"
export CPM_DIR="$HOME/CPM"

#TYPE="cascon_main"
#TYPE="cascon_cpm"
TYPE="cascon_cpm_random"
#TYPE="cascon_intervals"
#TYPE="fwdOptSD"
#TYPE="cascon_random_test"

export SCENARIO_JSON_DIR="$SCENARIO_DIR/scenario_json/$TYPE"
export csv_out="$SCENARIO_DIR/perf-results-simulation-generic_${TYPE}.csv"

# --- 2. Safety Checks & Backup ---
if [ ! -f "$csv_out" ]; then
	echo "Error: Could not find CSV file at $csv_out"
	exit 1
fi

echo "Backing up original CSV to ${csv_out}.backup_before_cpm..."
cp "$csv_out" "${csv_out}.backup_before_cpm"

# --- 3. Define the Worker Function ---
update_cpm() {
	local scenario="$1"
	local scenario_json="${SCENARIO_JSON_DIR}/${scenario}.json"

	if [ ! -f "$scenario_json" ]; then
		echo "Warning: JSON not found for $scenario. Skipping."
		return
	fi

	# Run the CPM tool
	set +e
	local cpm_output=$(${CPM_DIR}/cpm --scenarioJSON "${scenario_json}" 2>&1)
	local cpm_status=$?
	set -e

	local cpm cpm_t
	if  [ $cpm_status -ne 0 ]; then
		echo "Warning: CPM failed with exit code $cpm_status on scenario $scenario"
		cpm=-1
		cpm_t=-1
	else
		cpm=$(echo "$cpm_output" | sed -n 's/^metric: \([0-9]*\)/\1/p' | tr -d '\n')
		cpm_t=$(echo "$cpm_output" | sed -n 's/^time: \([0-9]*\) ns/\1/p' | tr -d '\n')
	fi

	# Lock the CSV file and precisely update columns 10 and 11
	(
		flock -x 200
		
		# Find the exact line number for this scenario
		local line_num=$(grep -n -F "$scenario," "$csv_out" | cut -d: -f1 | head -1)
		
		if [ -n "$line_num" ]; then
			# Extract the current line
			local current_line=$(sed -n "${line_num}p" "$csv_out")
			
			# Use awk to replace just the 10th and 11th fields, keeping the original formatting
			local new_line=$(echo "$current_line" | awk -F',' -v OFS=',' -v c="$cpm" -v ct="$cpm_t" '{$10=" "c; $11=" "ct; print}')
			
			# Overwrite the old line with the newly updated line
			sed --in-place -e "${line_num}c\\$new_line" "$csv_out"
			echo "Updated $scenario -> CPM: $cpm, Time: $cpm_t"
		else
			echo "Warning: Scenario $scenario not found in CSV."
		fi
	) 200> "${csv_out}.lock"
}
export -f update_cpm

# --- 4. Dispatch Jobs ---
echo "Extracting scenarios from CSV and dispatching CPM updates..."

# This awk command skips the first row (NR>1) to ignore the header,
# grabs the first column ($1), and pipes those scenario names into parallel.
awk -F',' 'NR>1 && $1 != "" {print $1}' "$csv_out" | parallel --jobs 0 update_cpm {}

echo "All CPM updates completed at $(date '+%H:%M:%S')."