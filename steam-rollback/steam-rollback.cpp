#include <iostream>
#include <fstream>
#include <filesystem> // Include the filesystem library
#include <locale>
#include <codecvt>

#include "resources/old_steam.h"
#include "utils/miniz.h"
#include "process/process.hpp"
#include "utils/debug.hpp"

int main()
{
	HKEY hKey;
	WCHAR steam_path_w[256]; // Adjust the size according to your needs
	DWORD steam_path_s = sizeof(steam_path_w);
	bool should_rollback = true;

	// Get steam path and check version

	LONG openRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_ALL_ACCESS, &hKey);
	if (openRes != ERROR_SUCCESS) {
		debug::log::error("Could not open the Steam registry key.");
		return 0;
	}

	debug::log::success("Opened the Steam registry key.");

	LONG getName = RegQueryValueExW(hKey, L"SteamPath", 0, NULL, (LPBYTE)steam_path_w, &steam_path_s);
	if (getName != ERROR_SUCCESS) {
		debug::log::error("Could not read the SteamPath.");
		return 0;
	}

	for (int i = 0; i < size(proc::steamProcesses); i++)
		proc::terminate_process(proc::steamProcesses[i]);

	debug::log::warn("Steam has been closed.");

	std::this_thread::sleep_for(std::chrono::seconds(1));

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, steam_path_w, -1, nullptr, 0, nullptr, nullptr);
	std::string steam_path(size_needed - 1, 0); // Subtract 1 to exclude the null character
	WideCharToMultiByte(CP_UTF8, 0, steam_path_w, -1, &steam_path[0], size_needed, nullptr, nullptr);

	std::string steam_version = proc::get_file_version(steam_path + "/steam.exe");
	if (steam_version.find("Error") != std::string::npos) {
		debug::log::error("Could not get steam version: " + steam_version);
	}
	else {
		debug::log::success("Steam version: " + steam_version);

		if (steam_version > "7.56.33.36") {
			debug::log::success("Steam version is with the new ui, rolling back to the old UI...\n");
		}
		else {
			should_rollback = false;
			debug::log::warn("Steam is with the old UI, restoring to the new version...\n");
		}
	}

	// Unzip files and restore old steam

	if (should_rollback) {
		//for (const embedded_files::binary_info& file : embedded_files::file_array) {

		//	std::string absolute_path = steam_path + file.relative_path;
		//	//std::cout << "absolute_path " << absolute_path << std::endl;

		//	// Create the directory path if it doesn't exist
		//	std::filesystem::path directoryPath(absolute_path);
		//	directoryPath.remove_filename(); // Get the directory path without the filename
		//	std::filesystem::create_directories(directoryPath);

		//	std::ofstream outFile(absolute_path, std::ios::out | std::ios::binary);

		//	if (!outFile) {
		//		debug::log::error("Failed to open output file for: " + file.name);
		//		continue;
		//	}

		//	// Write binary data to the file
		//	outFile.write(reinterpret_cast<const char*>(file.data), file.size);

		//	// Close the file
		//	outFile.close();

		//	debug::log::success("Binary data written to " + file.relative_path);
		//}

		std::string file_name_ = file_name;
		std::string absolute_path = steam_path + "/" + file_name_;
		std::ofstream outFile(absolute_path, std::ios::out | std::ios::binary);

		if (!outFile) {
			debug::log::error("Failed to open output file for: " + absolute_path);
			return -1;
		}

		outFile.write(reinterpret_cast<const char*>(old_steam_data), old_steam_size);
		outFile.close();

		debug::log::success("ZIP File build successfully: " + absolute_path + "\n");

		//std::this_thread::sleep_for(std::chrono::seconds(5));

		mz_zip_archive zip_archive;
		memset(&zip_archive, 0, sizeof(zip_archive));

		if (mz_zip_reader_init_file(&zip_archive, absolute_path.c_str(), 0)) {
			debug::log::success("Extracting files...\n");

			int num_files = mz_zip_reader_get_num_files(&zip_archive);

			for (int i = 0; i < num_files; ++i) {
				mz_zip_archive_file_stat file_stat;
				if (mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
					std::string output_filename = steam_path + std::string("/") + file_stat.m_filename;

					// Check if the file already exists
					if (proc::file_exists(output_filename)) {
						//std::cout << "Replacing existing file: " << output_filename << std::endl;
						debug::log::success("replacing existing file -> " + output_filename);
					}

					if (mz_zip_reader_extract_to_file(&zip_archive, i, output_filename.c_str(), 0)) {
						//std::cout << "Extracted: " << output_filename << std::endl;
						debug::log::success("extracted -> " + output_filename);
					}
					else {
						//std::cerr << "Failed to extract: " << output_filename << std::endl;
						debug::log::error("failed to extract -> " + output_filename);
					}
				}
			}

			mz_zip_reader_end(&zip_archive);
			debug::log::success("Files extraction clompleted!");
		}
		else {
			//std::cerr << "Failed to open zip file." << std::endl;
			debug::log::error("Failed to open zip file.");
			return -1;
		}


		debug::log::success("Deleting zip file...");
		std::remove(absolute_path.c_str());
	}
	else {
		const std::string file_array[] = { "steam.cfg", "tier0_s.dll", "tier0_s64.dll" };

		for (std::string file : file_array)
			std::remove((steam_path + "/" + file).c_str());

		debug::log::success("Steam restored successfully, starting updater!\n");
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cin.get();

	debug::log::success("Starting steam...");

	HINSTANCE hInstance = ShellExecuteW(NULL, L"open", L"steam://open/main", NULL, NULL, SW_SHOWNORMAL);
	if ((int)hInstance <= 32) {
		debug::log::error("Could not open Steam.");
		return 0;
	}

	std::cin.get();

	return 0;
}