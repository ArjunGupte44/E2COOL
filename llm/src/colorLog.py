# printing Colors
ESC="\033"
RED=ESC+"[91m"
GREEN=ESC+"[92m"
YELLOW=ESC+"[93m"
BLUE=ESC+"[94m"
RESET=ESC+"[0m"
BOLD=ESC+"[1m"

def print_green(msg: str):
    print(f"{GREEN}{msg}{RESET}")

def print_red(msg: str):
    print(f"{RED}{msg}{RESET}")

def print_yellow(msg: str):
    print(f"{YELLOW}{msg}{RESET}")

def print_blue(msg: str):
    print(f"{BLUE}{msg}{RESET}")

        
if __name__ == "__main__":
    print_blue("hello")