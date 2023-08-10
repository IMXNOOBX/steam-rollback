import os

folder_path = "."  # Replace with the actual folder path

def rename_files(folder_path, target_string):
    for filename in os.listdir(folder_path):
        if target_string in filename:
            new_filename = filename.replace(target_string, "")
            old_path = os.path.join(folder_path, filename)
            new_path = os.path.join(folder_path, new_filename)
            os.rename(old_path, new_path)
            print(f"Renamed: {filename} -> {new_filename}")

target_string = "RollbackSteam.Resources."

rename_files(folder_path, target_string)
print("Renaming complete.")
