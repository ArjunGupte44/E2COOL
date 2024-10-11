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
                
                Here is an example of desirable optimization:
                Example of optimization: 
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

                Here is the actual code to be optimized: 
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

def llm_optimize(filename):
    source_path = f"{USER_PREFIX}/EEDC/llm/benchmarks_out/{filename.split('.')[0]}/{filename}"

    if filename.split('.')[1] == "compiled":
        filename = filename.split('.')[0] + "." + filename.split('.')[2:]

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
        print("First optimization, no evaluator feedback yet")

    # add code content to prompt
    optimize_prompt = prompt + f" {code_content}" + f" {evaluator_feedback}"

    with open(f"{USER_PREFIX}/EEDC/llm/src/output_logs/optimize_prompt_log.txt", "w") as f:
        f.write(optimize_prompt)

    client = OpenAI(api_key=openai_key)

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
    
    print(f"\nnew_llm_optimize (llm_optimize): writing optimized code to llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}")
    destination_path = f"{USER_PREFIX}/EEDC/llm/benchmarks_out/{filename.split('.')[0]}"
    with open(destination_path+"/optimized_"+filename, "w") as file:
        file.write(final_code)

    # Success code
    return 0

def handle_compilation_error(filename):
    with open(f"{USER_PREFIX}/EEDC/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}", "r") as file:
        optimized_code = file.read()

    with open(f"{USER_PREFIX}/EEDC/llm/src/output_logs/regression_test_log.txt", "r") as file:
        error_message = file.read()
                      
        class ErrorReasoning(BaseModel):
            analysis: str
            final_code: str
        
        compilation_error_prompt = f"""You were tasked with the task outlined in the following prompt: {prompt}. You returned the following optimized code: {optimized_code}. However, the code failed to compile with the following error message: {error_message}. Analyze the error message and explicitly identify the issue in the code that caused the compilation error. Then, consider if there's a need to use a different optimization strategy to compile successfully or if there are code changes which can fix this implementation strategy. Finally, update the code accordingly and ensure it compiles successfully. Ensure that the optimized code is both efficient and error-free and return it. """   
        
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
        destination_path = f"{USER_PREFIX}/EEDC/llm/benchmarks_out/{filename.split('.')[0]}"
        with open(destination_path+"/optimized_"+filename, "w") as file:
            file.write(final_code)

def handle_logic_error(filename):
    with open(f"{USER_PREFIX}/EEDC/llm/benchmarks_out/{filename.split('.')[0]}/optimized_{filename}", "r") as file:
        optimized_code = file.read()

    with open(f"{USER_PREFIX}/EEDC/llm/src/output_logs/regression_test_log.txt", "r") as file:
        output_differences = file.read()
    
    class ErrorReasoning(BaseModel):
        analysis: str
        final_code: str
        
    # if not use_output_differences:
    #     logic_error_prompt = f"""You were tasked with the task outlined in the following prompt: {prompt}. You returned the following optimized code: {optimized_code}. However, the code failed to produce the same outputs as the original source code. Analyze the source code and the optimized code and explicitly identify the potential reasons that caused the logic error. Then, consider if there's a need to use a different optimization strategy to match the outputs or if there are code changes which can fix this implementation strategy. Finally, update the code accordingly and ensure it will match the source code's outputs for any input. Ensure that the optimized code is both efficient and error-free and return it. """   
    # else:
    #     logic_error_prompt = f"""You were tasked with the task outlined in the following prompt: {prompt}. You returned the following optimized code: {optimized_code}. However, the code failed to produce the same outputs as the original source code. Here are the output differences : {output_differences}. Analyze the source code and the optimized code and explicitly identify the potential reasons that caused the logic error. Then, consider if there's a need to use a different optimization strategy to match the outputs or if there are code changes which can fix this implementation strategy. Finally, update the code accordingly and ensure it will match the source code's outputs for any input. Ensure that the optimized code is both efficient and error-free and return it. """
    
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


    destination_path = f"{USER_PREFIX}/EEDC/llm/benchmarks_out/{filename.split('.')[0]}"
    with open(destination_path+"/optimized_"+filename, "w") as file:
        file.write(final_code)

