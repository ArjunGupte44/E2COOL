import os
import sys
import json
import shutil
import pickle
from dotenv import load_dotenv
load_dotenv()
USER_PREFIX = os.getenv('USER_PREFIX')

from regression_test import regression_test
from new_llm_optimize import llm_optimize, handle_compilation_error
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))
from energy.src.measure_energy import get_evaluator_feedback

valid_benchmarks = [
    "binarytrees.gpp-9.c++",
    "chameneosredux.gpp-5.c++",
    "fannkuchredux.gpp-5.c++",
    "fasta.gpp-5.c++",
    "knucleotide.gpp-3.c++",
    "mandelbrot.gpp-6.c++",
    "nbody.gpp-8.c++",
    "pidigits.gpp-4.c++",
    "regexredux.gpp-3.c++",
    "revcomp.gpp-4.c++",
    "spectralnorm.gpp-6.c++"
]

total_compilation_errors, compilation_errors_fixed = 0, 0

def master_script(filename):

    global total_compilation_errors, compilation_errors_fixed

    # Keep a copy of a compiling file for re-optimization
    shutil.copyfile(f"{USER_PREFIX}/llm/llm_input_files/input_code/{filename}", f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
    print(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
    
    print(f"Optimizing {filename}, longest step")
    if llm_optimize(filename) != 0:
        print("Error in optimization, exiting script")
        return
    
    print(f"Running regression test on {filename}")
    regression_test_result = -3
    compilation_errors, success = 0, 0
    i, occurence_of_compilation_error = 0, -2

    # Run llm optimization until successful regression test
    while True:
        i += 1
        regression_test_result = regression_test(f"optimized_{filename}")
        
        # Log compilation fixed errors
        if occurence_of_compilation_error + 1 == i and regression_test_result != -1:
            compilation_errors_fixed += 1

        # Compilation error in unoptimized file, exit script
        if regression_test_result == -2:
            print("Error in unoptimized file, exiting script")
            return

        # Compilation error in optimized file, re-prompt
        if regression_test_result == -1:
            total_compilation_errors += 1
            occurence_of_compilation_error = i
            if compilation_errors == 3:
                print("Could not compile optimized file after 3 attempts, will re-optimize from lastly compiling file")
                print(f"{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
                llm_optimize(f"{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
                compilation_errors = 0
                continue
            print("Error in optimized file, re-optimizing")
            handle_compilation_error(filename)
            compilation_errors += 1

        # Output difference in optimized file, re-prompt
        if regression_test_result == 0:
            print("Output difference in optimized file, will re-optimize from lastly compiling file")
            print(f"{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
            llm_optimize(f"{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
            continue
        
        # Success
        if regression_test_result == 1:
            get_evaluator_feedback(filename, success)
            success += 1
            print("Got evaluator feedback")
            # Copy compiling file
            print(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
            os.makedirs(os.path.dirname(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}"), exist_ok=True)
            shutil.copyfile(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}", f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
            print("Regression test successful")
            
            # Hard code to run 5 times
            if success == 5:
                print("Optimized 5 times successfully, exiting script")
                break
        
if __name__ == "__main__":
    
    benchmark = sys.argv[2]
    if benchmark in valid_benchmarks:
        print(f"Running benchmark: {benchmark}")
        # Call the function or code to execute the benchmark here
    else:
        print(f"Error: Invalid benchmark '{benchmark}'.")
        print("Please provide one of the following valid benchmarks:")
        for valid in valid_benchmarks:
            print(f" - {valid}")
        sys.exit(1)

    #run benchmark
    master_script(benchmark)

    print(f"Total compilation errors: {total_compilation_errors}, fixed: {compilation_errors_fixed}")
    print("EEDC Optimization Complete, writing results to file.....")

    with open(f"{USER_PREFIX}/energy/c++/benchmark_data.pkl", "rb") as file:
        contents = pickle.load(file)
    
    dict_str = json.dumps(contents, indent=4)
    with open("result_file.txt", "w+") as file:
        file.write(str(dict_str))

    # Delete evaluator feedback
    file_path = f"{USER_PREFIX}/energy/src/evaluator_feedback.txt"
    try:
        # Check if file exists
        if os.path.isfile(file_path):
            os.remove(file_path)
            print(f"{file_path} has been removed successfully.")
        else:
            print(f"{file_path} does not exist.")
    except Exception as e:
        print(f"An error occurred while trying to remove the file: {e}")