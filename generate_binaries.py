import os

# Input directory containing the files
input_dir = "old_steam/"

# Output directory for generated header files
output_dir = "steam-rollback/resources/"

# Create the output directory if it doesn't exist
os.makedirs(output_dir, exist_ok=True)
os.remove(output_dir + "*")
# Function to generate C-style array from binary content
def generate_binary_array(binary_content, filename):
	binary_array = ", ".join(f"{byte:#04x}" for byte in binary_content)
	result =  f"const unsigned char {filename}_data[] = {{ {binary_array} }};\n\n"
	result += f"const size_t {filename}_size = sizeof({filename}_data);"
	return result

# Recursively walk through the input directory and its subdirectories
header_entries = []

for root, _, filenames in os.walk(input_dir):
	for filename in filenames:
		file_path = os.path.join(root, filename)
		raw_relative_path = os.path.relpath(file_path, input_dir)
		relative_path_parts = raw_relative_path.split(os.sep)
		relative_path_parts[-1] = relative_path_parts[-1].replace(".", "_").replace("-", "_")
		relative_path = os.path.join(*relative_path_parts)
		output_path = os.path.join(output_dir, raw_relative_path) + ".h"

		with open(file_path, "rb") as f:
			binary_content = f.read()

		header_content = f"#pragma once\n\n"
		header_content += generate_binary_array(binary_content, filename.replace(".", "_").replace("-", "_"))

		header_subdir = os.path.dirname(output_path)
		os.makedirs(header_subdir, exist_ok=True)

		with open(output_path, "w") as header_file:
			header_file.write(header_content)

		print(f"Generated header: {output_path}")

		# Append entry to header_entries list
		header_entries.append({
			"relative_path": raw_relative_path,
			"raw_relative_path": raw_relative_path.replace("\\", "\\\\"),
			"filename": filename.replace(".", "_").replace("-", "_"),
			"raw_filename": filename,
			"variable_name": f"data_{relative_path.replace('/', '_').replace('.', '_')}",
		})

# Generate the index header file
index_header_content = "#pragma once\n\n"
index_header_content += "#include <cstddef>\n"
index_header_content += "#include <string>\n\n"

for entry in header_entries:
	index_header_content += f"#include \"{entry['relative_path']}.h\"\n"

index_header_content += "\nnamespace embedded_files {\n"

index_header_content += """
\tstruct binary_info {
\t    const unsigned char* data;
\t    size_t size;
\t    std::string name;
\t    std::string relative_path;
\t};

"""

index_header_content += "\tconst binary_info file_array[] = {\n"

for entry in header_entries:
	index_header_content += "\t\t{ " + entry['filename'] + "_data, " + entry['filename'] + "_size, \"" + entry['raw_filename'] + "\", \"" + entry['raw_relative_path'] + "\" },\n"

index_header_content += "\t};\n"

index_header_content += "}\n"

index_header_path = os.path.join(output_dir, "embedded_files.h")
with open(index_header_path, "w") as index_header_file:
	index_header_file.write(index_header_content)

print(f"Generated index header: {index_header_path}")
print("All headers and index file generated.")
