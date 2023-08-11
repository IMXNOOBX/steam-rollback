import os
import zipfile

# Input directory containing the files
input_dir = "old_steam/"
zip_file_name = "old_steam"

# Output directory for generated header files
output_dir = "steam-rollback/resources/"

# Create the output directory if it doesn't exist
os.makedirs(output_dir, exist_ok=True)

with zipfile.ZipFile(output_dir + zip_file_name + ".zip", 'w') as zipf:
    for root, _, filenames in os.walk(input_dir):
        for filename in filenames:
            file_path = os.path.join(root, filename)  # Construct the full path
            print(f"Adding file into zip: {file_path}")
            zipf.write(file_path, os.path.relpath(file_path, input_dir))  # Add the file to the zip archive

print(f"All files added to the zip, generating file...")

print(f"Generated zip file: {output_dir + zip_file_name}.zip")