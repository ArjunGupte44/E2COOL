# run with "bash energy_estimation.sh"

time_factor=100
output_file="powermetrics_output.txt"

start_powermetrics() {
    # Start powermetrics with a sampling interval of 100ms (0.1 second)
    sudo powermetrics -i 100 -f text -o "powermetrics_output.txt" & # already automatically clears the file each time
    # Save the PID of powermetrics
    POWERMETRICS_PID=$!
    sleep 1.2
}

stop_powermetrics() {
    # Kill the powermetrics process
    sleep 0.2
    sudo kill $POWERMETRICS_PID
}

# Start powermetrics
start_powermetrics

# Run your program
python3 testing_programs/twosum1.py

# Once the execution completes, stop powermetrics
stop_powermetrics

if [[ -s "$output_file" ]]; then
    # Extract CPU Power values, print each, sum them, and multiply by the multiplication factor
    total=$(grep "CPU Power" "$output_file" | awk -v factor="$time_factor" '{
        print $0; sum += $3
    } 
    END {
        printf "Total energy: %.4f J\n", (sum * factor) / 1000000
    }')
else
    echo "Error: Output file not found or is empty." > "energy_data.txt"
fi
echo "$total" > "energy_data.txt"