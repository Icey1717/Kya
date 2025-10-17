# Path to your input file
input_file = "FunctionToFilename.txt"

file_names = set()

# Try utf-16-le encoding (common for Windows)
with open(input_file, "r", encoding="utf-16-le") as f:
    for line in f:
        line = line.strip()
        if not line or "[WARN]" in line:
            continue
        if "->" in line:
            file_path = line.split("->")[1].strip()
            file_names.add(file_path)

# Convert to sorted list for readability
unique_files = sorted(file_names)

# Print the results
for file in unique_files:
    print(file)
