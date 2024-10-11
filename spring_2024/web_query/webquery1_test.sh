#!/bin/bash

# Define file to store powermetrics data
# output_file="powermetrics_output.txt"
time_factor=5
output_file="powermetrics_output.txt"


# Function to start powermetrics
start_powermetrics() {
    # Start powermetrics with a sampling interval of 100ms (0.1 second)
    sudo powermetrics -i $time_factor -f text -o $output_file &
    # Save the PID of powermetrics
    POWERMETRICS_PID=$!
    # echo "Powermetrics started with PID: $POWERMETRICS_PID"
}

# Function to stop powermetrics
stop_powermetrics() {
    # Kill the powermetrics process
    # echo "Stopping powermetrics (PID: $POWERMETRICS_PID)..."
    sudo kill $POWERMETRICS_PID
}

# Start powermetrics
start_powermetrics
# Sleep for powermetrics to start running
sleep 2
# Run executable
node webquery1.js

# Estimated one second sleep to cover runtime of program.
# For programs with longer runtime, it needs to be increased. 
sleep 1
# Once the Node.js program completes, stop powermetrics
stop_powermetrics

if [[ -s "$output_file" ]]; then
    # Print CPU Power values at each time interval (in milliseconds)
    echo "CPU Power (mW) every $time_factor ms:"
    grep "CPU Power" "$output_file" | awk '{print $3}'

    # Calculate and print the total estimated energy consumption
    total=$(grep "CPU Power" "$output_file" | awk -v factor=$time_factor '{sum += $3} END {print (sum * factor) / 1000000}')
    echo "Estimated energy consumption: $total J"
    
    # Clear the contents of the output file
    > "$output_file"
else
    echo "Error: Output file not found or is empty."
fi

