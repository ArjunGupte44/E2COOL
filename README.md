# Code Release for 2025 ICSE-NIER Submission: E2COOL - Towards Energy Efficient Code Optimizations with Large Language Models
## About
This repository contains all the artifacts of the project “E2COOL: Towards Energy Efficient Code Optimizations with Large Language Models.” It includes scripts, implementation details, and instructions necessary to reproduce the results and experiments discussed in our submission.
## Table of Contents
- [Setting up the pipeline](#setting-up-the-pipeline)
- [Running the pipeline](#running-the-pipeline)
- [Manual analysis and evaluation](#manual-analysis-and-evaluation)
- [Workflow](#workflow)
- [Data Availability](#data-availability)
- [Reproduce Results](#reproduce-results)
- [Included Scripts](#included-scripts)
- [Code Dependencies](#code-dependencies)
## Setting up the pipeline
To set up the pipeline for energy-efficient code optimization, follow these steps:
1. **Clone the repository:**
   ```bash
   git clone <repository-link>
   cd <project-directory>
2. **Install the required dependencies using the Makefile**
    ```bash
   make setup
3. **Create .env in root directory**
    This should include:
    ```bash
    API_KEY=your_openai_api_key_here
    USER_PREFIX=/path/to/EEDC
## Running the pipeline
4. **Run the main script**
    ```bash
    make run
