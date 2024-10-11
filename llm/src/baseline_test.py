import sys
import os
import time
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))
from energy.src.measure_energy import get_evaluator_feedback

def baseline_test():
    for filename in os.listdir("../llm_input_files/input_code"):
        if filename.split(".")[-1] == "txt" or "." not in filename:
            continue
        print(f"Running evaluator on {filename}")
        output = get_evaluator_feedback(filename)
        with open(f"./baseline_test_logs/{filename}_feedback.txt", "a") as f:
            f.write("Original:\n")
            f.write(f"Average Energy: {output['original']['avg_energy']}\n")
            f.write(f"Average Runtime: {output['original']['avg_runtime']}\n\n")
            f.write("Lowest Average Energy:\n")
            f.write(f"Average Energy: {output['lowest_avg_energy']['avg_energy']}\n")
            f.write(f"Average Runtime: {output['lowest_avg_energy']['avg_runtime']}\n\n")
            f.write("Current:\n")
            f.write(f"Average Energy: {output['current']['avg_energy']}\n")
            f.write(f"Average Runtime: {output['current']['avg_runtime']}\n\n")

if __name__ == "__main__":
    baseline_test()