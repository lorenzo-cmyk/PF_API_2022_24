import os
import re

# The file structure is centered around this file.
current_dir = os.getcwd()
working_dir = current_dir + "/../"

# Regex to split the mutual parts of both input and output files.
filenames_pattern = r"open_(\d+)\.txt"

# Get all the files in the test-cases folder.
files = os.listdir(current_dir + "/test-cases/")

# Get all the input files that have a corresponding output file.
file_pairs = []
for file in files:
    match = re.match(filenames_pattern, file)
    if match:
        file_number = match.group(1)
        output_file = f"open_{file_number}.output.txt"
        if output_file in files:
            file_pairs.append((file, output_file))

# Sort the file pairs by the number in the filename.
file_pairs.sort(key=lambda x: int(re.match(filenames_pattern, x[0]).group(1)))

# Go in the working directory and compile the program.
os.chdir(working_dir)
os.system("make clean")
os.system("make")

# Create the folder "results" if it doesn't exist, if it does remove all files in it.
os.system(f"rm -rf {current_dir}/results")
os.system(f"mkdir -p {current_dir}/results")

# Run the program for each file pair and compare the output with the corresponding output file.
for file_pair in file_pairs:
    input_file = file_pair[0]
    output_file = file_pair[1]
    os.system("echo ''")
    os.system(f"echo 'Running test case {input_file}...'")
    os.system(
        f"./main.o < {current_dir}/test-cases/{input_file} > {current_dir}/results/{input_file}.tmp")
    os.system(
        f"diff {current_dir}/results/{input_file}.tmp {current_dir}/test-cases/{output_file}")
    os.system("echo ''")
