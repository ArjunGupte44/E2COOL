from colorLog import *
from datetime import datetime
from dotenv import load_dotenv
import json
import ollama
from ollama import Client
import os
import pickle
import shutil
import subprocess
import sys
from utils import setup_logger


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

# attaching date and time to make names unique
run_start_time = datetime.now()
def attach_datetime(string):
    return string + run_start_time.strftime(f"_%d-%m-%H-%M")
log_dir = os.path.join("logs", sys.argv[2])
if not os.path.exists(log_dir):
    os.makedirs(log_dir)
log_file_path = os.path.join(log_dir, attach_datetime('run') + '.log')
logger = setup_logger(log_file_path)

def master_script(filename, client, model_name):

    global total_compilation_errors, compilation_errors_fixed

    # Keep a copy of a compiling file for re-optimization
    # copy original code to benchmarks_out/ as filename.compiled.gpp-x.c++
    shutil.copyfile(f"{USER_PREFIX}/llm/llm_input_files/input_code/{filename}", f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
    logger.info(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
    
    # keep track of errors
    regression_test_result = -3
    compilation_errors, success = 0, 0
    i, occurence_of_compilation_error = 0, -2
    reoptimize_lastly_flag = 0

    while True:
        # optimization step
        # reoptimize latest working opimized file if logic/compile error
        if reoptimize_lastly_flag == 0:
            logger.info(f"Optimizing {filename}, iteration {success}")
            llm_optimize(client, model_name, filename, success)
        else:
            logger.info("re-optimizing from latest working optimization")
            llm_optimize(client, model_name, f"{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}", success)
            reoptimize_lastly_flag = 0
        
        # regression test step
        logger.info(f"Running regression test on optimized_{filename}")
        regression_test_result = regression_test(f"optimized_{filename}")
        i += 1
        
        # Log compilation fixed errors
        if occurence_of_compilation_error + 1 == i and regression_test_result != -1:
            compilation_errors_fixed += 1

        # Compilation error in unoptimized file, exit script
        if regression_test_result == -2:
            logger.error("Error in unoptimized file, exiting script")
            return

        # Compilation error in optimized file, re-prompt
        if regression_test_result == -1:
            total_compilation_errors += 1
            occurence_of_compilation_error = i
            if compilation_errors == 3:
                logger.error("Could not compile optimized file after 3 attempts, will re-optimize from lastest working optimized file")
                logger.info(f"{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
                reoptimize_lastly_flag = 1
                compilation_errors = 0
                continue

            logger.error("Error in optimized file, re-optimizing")
            handle_compilation_error(client, model_name, filename)
            compilation_errors += 1

        # Output difference in optimized file, re-prompt
        if regression_test_result == 0:
            logger.error("Output difference in optimized file, will re-optimize from lastest working optimized file")
            logger.info(f"{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
            reoptimize_lastly_flag = 1
            continue
        
        # Success
        if regression_test_result == 1:
            logger.info("Regression test successful, getting evaluator feedback")
            get_evaluator_feedback(client, model_name, filename, success)
            # print_green("Got evaluator feedback")
            logger.info("Got evaluator feedback")
            success += 1

            # Copy lastest optimized code for logic error re-optimization
            logger.info("Saving lastest working optimized file")
            os.makedirs(os.path.dirname(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}"), exist_ok=True)
            shutil.copyfile(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}", f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}.compiled.{'.'.join(filename.split('.')[1:])}")
            
            # Hard code to run 5 times
            if success == 5:
                logger.info("Optimized 5 times successfully, exiting script")
                break
        
if __name__ == "__main__":
    
    #Check if requested benchmark is valid
    benchmark = sys.argv[2]
    if benchmark in valid_benchmarks:
        logger.info(f"Running benchmark: {benchmark}")
        # Call the function or code to execute the benchmark here
    else:
        logger.error(f"Error: Invalid benchmark '{benchmark}'.")
        print("Please provide one of the following valid benchmarks:")
        for valid in valid_benchmarks:
            print(f" - {valid}")
        sys.exit(1)
    
    #Check if requested LLM is valid
    model_name = sys.argv[3]
    if model_name == "openai":
        client = model_name
    else:
        try:
            subprocess.run(["ollama", "pull", model_name], check=True)
        except Exception as e:
            logger.error(f"Error: Invalid LLM requested: {model_name}")
            print("Please provide the name of an LLM suppported by Ollama")
            sys.exit(1)
        else:
            client = Client(host="http://localhost:11434")
            
    #run benchmark
    master_script(benchmark, client, model_name)

    # print_green(f"Total compilation errors: {total_compilation_errors}, fixed: {compilation_errors_fixed}")
    logger.info(f"Total compilation errors: {total_compilation_errors}, fixed: {compilation_errors_fixed}")
    logger.info("EEDC Optimization Complete, writing results to file.....")

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
            logger.info(f"{file_path} has been removed successfully.")
        else:
            logger.error(f"{file_path} does not exist.")
    except Exception as e:
        logger.error(f"An error occurred while trying to remove the file: {e}")