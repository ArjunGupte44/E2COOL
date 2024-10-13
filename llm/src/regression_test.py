import subprocess
import os
from dotenv import load_dotenv
load_dotenv()
USER_PREFIX = os.getenv('USER_PREFIX')

TEST_OUTPUT_FILE = f"{USER_PREFIX}/llm/src/output_logs/regression_test_log.txt"
UNOPTIMIZED_OUTPUT = f"{USER_PREFIX}/llm/src/output_logs/unoptimized_output.txt"
OPTIMIZED_OUTPUT = f"{USER_PREFIX}/llm/src/output_logs/optimized_output.txt"

def compile_program(output_log, optimized):
    try: 
        # Redirect stdout and stderr to the regression_test_log file
        if not optimized:
            subprocess.run(
                    ["make", "compile"], 
                    check=True,
                    stdout=output_log, 
                    stderr=output_log  # Redirect stderr to the same file as stdout
            )
        else:
            subprocess.run(
                    ["make", "compile_optimized"], 
                    check=True,
                    stdout=output_log, 
                    stderr=output_log  # Redirect stderr to the same file as stdout
            )
        print("regression_test: Makefile compile successfully.\n")
        return True
    except subprocess.CalledProcessError as e:
        print(f"regression_test: Makefile compile failed: {e}\n")
        return False

def run_program(exec_path, output_file, optimized):
    # Run the make command and capture the output in a variable
    if not optimized:
        result = subprocess.run(["make", "run"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    else:
        result = subprocess.run(["make", "run_optimized"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    # Check for errors
    if result.returncode != 0:
        with open(output_file, 'w+') as f:
            f.write(result.stderr)
        print(f"Runtime error on {exec_path} with error message: {result.stderr}")
        return False

    # Filter out the unwanted lines
    filtered_output = "\n".join(
        line for line in result.stdout.splitlines()
        if not (line.startswith("make[") or line.startswith("./"))
    )
    
    # Write the filtered output to the file
    with open(output_file, 'w+') as f:
        f.write(filtered_output)
    
    return True

def compare_outputs(file1, file2, output_log):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        file1_content = f1.readlines()
        file2_content = f2.readlines()
        if file1_content == file2_content:
            output_log.write("Outputs are the same.\n")
            return True
        else:
            output_log.write("Outputs are different.\n")
            output_log.write(f"Original program output:\n{file1_content}\n")
            output_log.write(f"Optimized program output:\n{file2_content}\n\n")
            return False

def regression_test(filename): 
    unoptimized_file_exec = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename.split('.')[0]}"
    optimized_file_exec = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}"

    # Needed for makefiles
    os.chdir(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0].split('_')[-1]}")   
    with open(TEST_OUTPUT_FILE, 'w+') as output_log:
        if not compile_program(output_log, False):
            # Return code when unoptimized file does not compile
            return -2
        if not compile_program(output_log, True):
            # Return code when optimized file does not compile
            return -1

        run_program(unoptimized_file_exec, UNOPTIMIZED_OUTPUT, False)
        run_program(optimized_file_exec, OPTIMIZED_OUTPUT, True)

        if not compare_outputs(UNOPTIMIZED_OUTPUT, OPTIMIZED_OUTPUT, output_log):
            return 0
        else:
            output_log.write("Regression test successful. Outputs are the same.\n\n")
            return 1

if __name__ == "__main__":
    regression_test("binarytrees.c", "optimized_binarytrees.c", "binarytrees")