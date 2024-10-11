import ollama
from ollama import Client
import subprocess 

#Setup ollama client
client = Client(host='http://172.18.36.115:11434')

##Generator file info##
generator_instructions_path = "/home/agupte/VIP_PTM_DL/LLM_Energy_Efficiency/EEDC/llm/llm_input_files/nl_prompts/generator_instructions.txt"
input_code_path = "/home/agupte/VIP_PTM_DL/LLM_Energy_Efficiency/EEDC/llm/llm_input_files/input_code/nested_loops.txt"
optimized_code_path = "/home/agupte/VIP_PTM_DL/LLM_Energy_Efficiency/EEDC/llm/llm_output_files/generator_output_code.txt"

##Evaluator file info##
evaluator_instructions_path = "/home/agupte/VIP_PTM_DL/LLM_Energy_Efficiency/EEDC/llm/llm_input_files/nl_prompts/evaluator_instructions.txt"
evaluator_feedback_path = "/home/agupte/VIP_PTM_DL/LLM_Energy_Efficiency/EEDC/llm/llm_input_files/nl_prompts/evaluator_feedback.txt"

##Energy usage data file info##
energy_usage_path = "/home/agupte/VIP_PTM_DL/LLM_Energy_Efficiency/EEDC/llm/energy_usage_files/energy_usage_data.txt"


def store_optimized_code():
  return optimized_code_path[optimized_code_path.index("/generator"):len(optimized_code_path)]


def run_evaluator():
  pass
  #call done_optimizing = run_statistical_test()

  if not done_optimizing:
    instructions_file = open(evaluator_instructions_path, "r")
    instructions = "".join(instructions_file.readlines())

    optimized_code_file = open(optimized_code_path, "r")
    optimized_code = "".join(optimized_code_file.readlines())

    energy_data_file = open(energy_usage_path, "r")
    energy_usage_data = "".join(energy_data_file.readlines())

    evaluator_prompt = instructions + "\n" + optimized_code + "\n" + energy_usage_data
    print(f"\n\nEVALUATOR PROMPT: \n{evaluator_prompt}")

    #Run codellama 7B
    response = client.chat(model='codellama:70b-instruct', messages=[
      {
        'role': 'user',
        'content': evaluator_prompt,
      },
    ])

    evaluator_output = response['message']['content']
    feedback_file = open(evaluator_feedback_path, "w")
    feedback_file.write(evaluator_output)
    feedback_file.close()

    return True

  else:
    return True


def run_energy_test():
  path = "/Users/lambert/VIP/EEDC/energy_estimation.sh"
  subprocess.run(['bash', path])

def run_generator(pass_num):
  instructions_file = open(generator_instructions_path, "r")
  instructions = "".join(instructions_file.readlines())

  if pass_num == 0:
    evaluator_feedback = ""
    input_code_file = open(input_code_path, "r")
  else:
    feedback_file = open(evaluator_feedback_path, "r")
    evaluator_feedback = "".join(feedback_file.readlines())
    input_code_file = open(optimized_code_path, "r")

  input_code = "".join(input_code_file.readlines())

  generator_prompt = instructions + "\n" + evaluator_feedback + "\n" + input_code
  #print(f"\n\nGENERATOR PROMPT: \n{generator_prompt}")


  #Run codellama 7B
  response = client.chat(model='codellama:70b-instruct', messages=[
    {
      'role': 'user',
      'content': generator_prompt,
    },
  ])

  generator_output = response['message']['content']
  output_code_file = open(optimized_code_path, "w")
  output_code_file.write(generator_output)
  output_code_file.close()


if __name__ == "__main__":
  done_optimizing = False
  iter_num = 0

  #while not done_optimizing:
  for i in range(5): 
    run_generator(iter_num)
    run_energy_test()
    #done_optimizing = run_evaluator()
    run_evaluator()
    iter_num += 1
  
  optimized_code_path = store_optimized_code()
  print(f"The final optimized code can be found in {optimized_code_path}")

