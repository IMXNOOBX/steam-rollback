import os
import zipfile

# Input directory containing the files
input_dir = "old_steam/"
zip_file_name = "old_steam"

# Output directory for generated header files
output_dir = "steam-rollback/resources/"

# Create the output directory if it doesn't exist
os.makedirs(output_dir, exist_ok=True)
# os.remove(output_dir + "*")

# Function to generate C-style array from binary content
def generate_binary_array(binary_content, filename):
	binary_array = ", ".join(f"{byte:#04x}" for byte in binary_content)
	result =  f"const unsigned char {filename}_data[] = {{ {binary_array} }};\n\n"
	result += f"const size_t {filename}_size = sizeof({filename}_data);"
	return result


with zipfile.ZipFile(zip_file_name + ".zip", 'w') as zipf:
    for root, _, filenames in os.walk(input_dir):
        for filename in filenames:
            file_path = os.path.join(root, filename)  # Construct the full path
            print(f"Adding file into zip: {file_path}")
            zipf.write(file_path, os.path.relpath(file_path, input_dir))  # Add the file to the zip archive

print(f"All files added to the zip, generating header file")

# Generate the index header file
header_content = f"#pragma once\n\n"

with open(zip_file_name + ".zip", "rb") as f:
	binary_content = f.read()

header_content += f"const char* file_name = \"{zip_file_name}.zip\";\n"
header_content += generate_binary_array(binary_content, zip_file_name.replace(".", "_").replace("-", "_"))

with open(output_dir + zip_file_name + ".h", "w") as header_file:
	header_file.write(header_content)


print(f"Generated index header: {output_dir + zip_file_name}.h")
print("All headers and index file generated.")
