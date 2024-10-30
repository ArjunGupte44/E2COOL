from pydantic import BaseModel
from openai import OpenAI
from dotenv import load_dotenv
import os
load_dotenv()


openai_key = os.getenv('API_KEY')
USER_PREFIX = os.getenv('USER_PREFIX')

prompt = """You are tasked with optimizing the following C++ code for energy efficiency, specifically focusing on reducing CPU cycles, minimizing memory access, and optimizing I/O operations. Analyze the code thoroughly and suggest multiple optimization strategies, considering the following aspects:

                Reduction of nested loops: Identify opportunities to simplify or eliminate nested loops to reduce computational overhead.
                Efficient data structure selection: Propose data structures that minimize memory access and improve performance.
                Dynamic programming or memoization: Look for opportunities to avoid redundant calculations and reduce CPU cycles.
                Specialized algorithms: Explore if more efficient algorithms can be applied to lower resource usage.
                I/O optimization: Suggest ways to optimize input/output operations, reducing their impact on performance.
                Code simplicity and readability: Ensure that the optimized code remains understandable while achieving energy efficiency.
                
                Provide a detailed step-by-step explanation of your analysis and the reasoning behind each optimization strategy. After evaluating the pros and cons of each approach, choose the most effective strategy and implement the necessary changes directly into the code. Make sure the optimized code and the original code provide the same output in the same format for any given input. This is critical to the optimization.
                
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

                Reduction of Nested Loops: The original code uses a nested loop with O(n²) complexity, which is highly inefficient for large inputs. We can significantly reduce CPU cycles by eliminating the second loop and using a more efficient approach.
                Efficient Data Structure Selection: To eliminate nested loops, we can use a hash-based data structure like unordered_set, which provides O(1) average-time complexity for both insertions and lookups. This ensures we only traverse the list once, lowering time complexity to O(n).
                Dynamic Programming or Memoization: Since we are only identifying duplicates without overlapping subproblems, dynamic programming or memoization is not needed here. Using a hash set avoids redundant calculations, as each element is processed exactly once.
                Specialized Algorithms: Given that the problem is about finding duplicates, no specialized algorithm is required beyond leveraging the efficient properties of a hash set for tracking elements.
                I/O Optimization: The I/O operations are minimal and don't present significant overhead. However, maintaining simplicity in how duplicates are printed ensures no unnecessary additional operations are introduced.
                Code Simplicity and Readability: Using an unordered_set makes the code both efficient and readable. The logic remains easy to follow, while offering an energy-efficient solution by reducing computational and memory overhead.

                After considering various strategies, we opted for using unordered_set because it effectively reduces the time complexity from O(n²) to O(n) by eliminating nested loops. The set also ensures minimal memory usage and fast lookups, making it an ideal fit for this problem.
                While dynamic programming or memoization wasn't necessary, the simplicity and clarity of using a hash-based approach meant the code remained both energy-efficient and easy to maintain.
                Other alternatives, like sorting-based algorithms, were discarded since they either increased complexity or were unnecessary for the problem at hand.
                This approach provided the best balance between energy efficiency, performance, and code simplicity.

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

    # get original code
    source_path = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}/{filename}"

    # get optimized file if is not first iteration
    if optim_iter != 0:
        source_path = f"/home/jimmy/VIP_PTM/E2COOL/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}"

    # get lastly compiled code
    if filename.split('.')[1] == "compiled":
        source_path = f"/home/jimmy/VIP_PTM/E2COOL/llm/benchmarks_out/{filename.split('.')[0]}/{filename}"
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
        # print("File content:", content)
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

    if completion.choices[0].message.parsed.final_code == "":
        print("Error in llm completion")
        return
    
    final_code = completion.choices[0].message.parsed.final_code
    
    print(f"llm_optimize: : writing optimized code to llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}")
    destination_path = f"{USER_PREFIX}/llm/benchmarks_out/{filename.split('.')[0]}"
    with open(destination_path+"/optimized_"+filename, "w") as file:
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
        

        #if regression test too large, usally is because of syntax error
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

