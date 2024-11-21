from pydantic import BaseModel
from openai import OpenAI
from dotenv import load_dotenv
import os
load_dotenv()


openai_key = os.getenv('API_KEY')
USER_PREFIX = os.getenv('USER_PREFIX')

prompt = """You are tasked with optimizing the following C++ code to improve its energy efficiency. This involves reducing CPU cycles, minimizing memory access, and optimizing I/O operations. Please follow these steps and guidelines:

 Task Instructions:

                - Analyze the code: Examine the provided C++ code in detail.
                - Identify optimization opportunities, determine where you can:
                    - Reduce or eliminate nested loops to lower computational overhead.
                    - Select more efficient data structures to minimize memory access and improve performance.
                    - Apply dynamic programming or memoization to avoid redundant calculations and reduce CPU usage where applicable.
                    - Implement specialized algorithms if they can significantly improve performance and reduce energy consumption.
                    - Optimize I/O operations to reduce their impact on overall performance.
                - Suggest optimization strategies: Propose multiple methods to improve energy efficiency. For each method:
                    - Provide a detailed explanation of how the optimization reduces energy usage.
                    - Discuss the trade-offs in terms of complexity, maintainability, and performance gains.
                - Choose the most effective optimization: After evaluating each proposed strategy, select the approach that yields the best balance of energy efficiency, performance, readability, and maintainability.
                - Implement the chosen optimization: Rewrite the code with the chosen optimization strategies, ensuring:
                    - The optimized code produces the same output as the original code for all valid inputs.
                    - Code remains clear and understandable.
                - Provide a detailed explanation: For each change you make, explain:
                    - Why this change optimizes energy efficiency.
                    - The improvement in performance and energy usage expected.
                    - How this change preserves the correctness and format of the output.
                - Output Requirements:
                    - Begin with a step-by-step analysis of the original code and identify inefficiencies.
                    - Outline each proposed optimization strategy in detail, including the reasoning behind it and its potential impact on energy usage.
                    - Implement the best optimization strategies directly into the code.
                    - Ensure the final code is energy-efficient, correct in terms of functionality, and maintains similar output formatting.
                                
                Here is an example of desirable response:
                Example of cpp code to be optimized:
                ```
                #include <iostream>
                #include <vector>

                using namespace std;

                // Inefficient code for finding duplicates in a vector of user IDs
                vector<int> findDuplicates(const vector<int>& userIds) {
                    vector<int> duplicates;
                    for (size_t i = 0; i < userIds.size(); ++i) {
                        for (size_t j = i + 1; j < userIds.size(); ++j) {
                            if (userIds[i] == userIds[j]) {
                                duplicates.push_back(userIds[i]);
                            }
                        }
                    }
                    return duplicates;
                }

                int main() {
                    vector<int> userIds = {1, 2, 3, 2, 4, 5, 1, 3, 5};
                    vector<int> duplicates = findDuplicates(userIds);

                    cout << "Duplicate user IDs: ";
                    for (int id : duplicates) {
                        cout << id << " ";
                    }
                    cout << endl;

                    return 0;
                }
                ```
                Example of Analysis:
                Analysis and reasoning example:
                    - Reduction of Nested Loops: Uses an O(n²) approach with nested loops.
                    - Efficient Data Structure Selection: Using an unordered_set to track seen user IDs reduces complexity to O(n).
                    - Dynamic Programming or Memoization: Not required here as each ID is processed once with an efficient data structure.
                    - Specialized Algorithms: Using hashing-based data structures effectively reduces redundant checks.
                    - I/O Optimization: Minimal I/O operations are used. Keep output operations efficient and straightforward.
                    - Code Simplicity and Readability: The optimized approach using unordered_set is both efficient and easy to understand.
                
                Here is the actual optimized code: 
                ```
                #include <iostream>
                #include <vector>
                #include <unordered_set>

                using namespace std;

                // Optimized code for finding duplicates in a vector of user IDs
                vector<int> findDuplicates(const vector<int>& userIds) {
                    unordered_set<int> seen;  // Set to track seen user IDs
                    unordered_set<int> duplicates;  // Set to store duplicates
                    for (int id : userIds) {
                        if (seen.find(id) != seen.end()) {
                            duplicates.insert(id);  // Add to duplicates if already seen
                        } else {
                            seen.insert(id);  // Mark as seen
                        }
                    }
                    return vector<int>(duplicates.begin(), duplicates.end());  // Convert set to vector
                }

                int main() {
                    vector<int> userIds = {1, 2, 3, 2, 4, 5, 1, 3, 5};
                    vector<int> duplicates = findDuplicates(userIds);

                    cout << "Duplicate user IDs: ";
                    for (int id : duplicates) {
                        cout << id << " ";
                    }
                    cout << endl;

                    return 0;
                }
                ```
"""

def llm_optimize(filename, optim_iter):
    print(f"***********************[ Optimizing {filename} ]***********************")
    # get original code
    source_path = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename}"

    # get optimized file if is not first iteration
    if optim_iter != 0:
        print(f"Getting optimized code from optimized_{filename}")
        source_path = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}"

    # get lastly compiled code
    if filename.split('.')[1] == "compiled":
        print(f"Getting lastly compiled code from {filename}")
        source_path = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename}"
        filename = filename.split('.')[0] + "." + ('.'.join(filename.split('.')[2:]))
    
    with open(source_path, "r") as file:
        code_content = file.read()

    class Strategy(BaseModel):
        Pros: str
        Cons: str

    class OptimizationReasoning(BaseModel):
        analysis: str
        strategies: list[Strategy] 
        selected_strategy: str
        final_code: str

    #checking for evaluator feedback
    # Get the current directory of the script
    current_dir = os.path.dirname(__file__)
    # Construct the absolute path to evaluator_feedback.txt
    feedback_file_path = os.path.abspath(os.path.join(current_dir, "../../energy/src/evaluator_feedback.txt"))

    if os.path.isfile(feedback_file_path):
        with open(feedback_file_path, 'r') as file:
            evaluator_feedback = file.read()
            evaluator_feedback = "Here's some suggestion on how you should optimize the code from the evaluator, keep these in mind when optimizing code\n" + evaluator_feedback
            print("llm_optimize: got evaluator feedback")

    else:
        evaluator_feedback = ""
        print("llm_optimize: First optimization, no evaluator feedback yet")

    # add code content to prompt
    optimize_prompt = prompt + f" {code_content}" + f" {evaluator_feedback}"

    with open(f"{USER_PREFIX}/llm/src/output_logs/optimize_prompt_log.txt", "w") as f:
        f.write(optimize_prompt)

    client = OpenAI(api_key=openai_key)

    print(f"llm_optimize: Generator LLM Optimizing ....")
    completion = client.beta.chat.completions.parse(
        model="gpt-4o-2024-08-06",
        messages=[
            {"role": "system", "content": "You are a helpful assistant. Think through the code optimizations strategies possible step by step"},
            {
                "role": "user",
                "content": optimize_prompt
            }
        ],
        response_format=OptimizationReasoning
    )
    print(f"llm_optimize: Generator LLM Completed Optimization")

    
    if completion.choices[0].message.parsed.final_code == "":
        print("Error in llm completion")
        return
    
    final_code = completion.choices[0].message.parsed.final_code
    
    #store generator final code
    generator_respond = f"{USER_PREFIX}/llm/src/output_logs/generator_respond.txt"
    with open(generator_respond, "w") as file:
        file.write(final_code)
    
    print(f"llm_optimize: : writing optimized code to llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}")
    destination_path = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}"
    with open(destination_path, "w") as file:
        file.write(final_code)

    # Success code
    return 0

def handle_compilation_error(filename):
    with open(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}", "r") as file:
        optimized_code = file.read()

    with open(f"{USER_PREFIX}/llm/src/output_logs/regression_test_log.txt", "r") as file:
        error_message = file.read()
    

        class ErrorReasoning(BaseModel):
            analysis: str
            final_code: str
        
        compilation_error_prompt = f"""You were tasked with the task outlined in the following prompt: {prompt}. You returned the following optimized code: {optimized_code}. However, the code failed to compile with the following error message: {error_message}. Analyze the error message and explicitly identify the issue in the code that caused the compilation error. Then, consider if there's a need to use a different optimization strategy to compile successfully or if there are code changes which can fix this implementation strategy. Finally, update the code accordingly and ensure it compiles successfully. Ensure that the optimized code is both efficient and error-free and return it. """   
        

        #if regression_test_log too large, usally is because of syntax error
        # Get the size of the file in bytes
        file_size_bytes = os.path.getsize(f"{USER_PREFIX}/llm/src/output_logs/regression_test_log.txt")
        file_size_kb = file_size_bytes / 1024
        if file_size_kb > 100:
            print("Syntax Error")
            compilation_error_prompt = f"""You were tasked with the task outlined in the following prompt: {prompt}. You returned the following optimized code: {optimized_code}. However, the code has syntax error, explicitly identify the issue in the code that caused the syntax error. Then, consider if there's a need to use a different optimization strategy to compile successfully or if there are code changes which can fix this implementation strategy. Finally, update the code accordingly and ensure it compiles successfully. Ensure that the optimized code is both efficient and error-free and return it. """   
        

        print("handle_compilation_error: promting for re-optimization")
        client = OpenAI(api_key=openai_key)
        completion = client.beta.chat.completions.parse(
            model="gpt-4o-2024-08-06",
            messages=[
                {"role": "system", "content": "You are a helpful assistant. Think through the code optimizations strategies possible step by step"},
                {
                    "role": "user",
                    "content": compilation_error_prompt
                }
            ],
            response_format=ErrorReasoning
        )

        final_code = completion.choices[0].message.parsed.final_code

        print(f"handle_compilation_error: writing re-optimized code to optimized_{filename}")
        destination_path = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}"
        with open(destination_path+"/optimized_"+filename, "w") as file:
            file.write(final_code)

def handle_logic_error(filename):
    with open(f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}", "r") as file:
        optimized_code = file.read()

    with open(f"{USER_PREFIX}/llm/src/output_logs/regression_test_log.txt", "r") as file:
        output_differences = file.read()
    
    class ErrorReasoning(BaseModel):
        analysis: str
        final_code: str
        

    #just prompting it to give output difference everytime
    logic_error_prompt = f"""You were tasked with the task outlined in the following prompt: {prompt}. You returned the following optimized code: {optimized_code}. However, the code failed to produce the same outputs as the original source code. Here are the output differences : {output_differences}. Analyze the source code and the optimized code and explicitly identify the potential reasons that caused the logic error. Then, consider if there's a need to use a different optimization strategy to match the outputs or if there are code changes which can fix this implementation strategy. Finally, update the code accordingly and ensure it will match the source code's outputs for any input. Ensure that the optimized code is both efficient and error-free and return it. """
    
    client = OpenAI(api_key=openai_key)
    completion = client.beta.chat.completions.parse(
        model="gpt-4o-2024-08-06",
        messages=[
            {"role": "system", "content": "You are a helpful assistant. Think through the code optimizations strategies possible step by step"},
            {
                "role": "user",
                "content": logic_error_prompt
            }
        ],
        response_format=ErrorReasoning
    )

    final_code = completion.choices[0].message.parsed.final_code


    destination_path = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}"
    with open(destination_path+"/optimized_"+filename, "w") as file:
        file.write(final_code)

