setup:
	@echo "Installing system and python dependencies"
	sudo apt install libapr1-dev
	pip install -r requirements.txt

run:
	@echo "Running main"
	python3 llm/src/main.py

all: setup run
