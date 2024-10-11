import pickle
from dotenv import load_dotenv
load_dotenv()
import os
USER_PREFIX = os.getenv('USER_PREFIX')

# Convert keys to a sorted list to access the first and last elements
with open(f"{USER_PREFIX}/EEDC/energy/c++/benchmark_data.pkl", "rb") as file:
    contents = pickle.load(file)

keys = list(contents.keys())

#print all values
for key, (source_code, avg_energy, avg_runtime) in contents.items():
    print("key:", key)
    print("avg_energy:", avg_energy)
    print("avg_runtime:", avg_runtime)
print("\n")

# Extract the first(original) and last(current) elements
first_key = keys[0]
last_key = keys[-1]

first_value = contents[first_key]
last_value = contents[last_key]


# Loop through the contents to find the key with the lowest avg_energy
min_avg_energy = float('inf')
min_energy_key = None
for key, (source_code, avg_energy, avg_runtime) in contents.items():
    if avg_energy < min_avg_energy:
        min_avg_energy = avg_energy
        min_energy_key = key

min_value = contents[min_energy_key]

# Prepare results in a structured format (dictionary)
benchmark_info = {
    "original": {
        "source_code": first_value[0],
        "avg_energy": first_value[1],
        "avg_runtime": first_value[2]
    },
    "lowest_avg_energy": {
        "source_code": min_value[0],
        "avg_energy": min_value[1],
        "avg_runtime": min_value[2]
    },
    "current": {
        "source_code": last_value[0],
        "avg_energy": last_value[1],
        "avg_runtime": last_value[2]
    }
}