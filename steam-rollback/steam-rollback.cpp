#include <iostream>
#include <fstream>
#include <filesystem> // Include the filesystem library
#include <locale>

#include "resource.h"
#include "utils/miniz.h"
#include "utils/debug.hpp"
#include "utils/utilities.hpp"

int main()
{
	SetConsoleTitle("Steam Rollback");

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

	for (int i = 0; i < size(util::steamProcesses); i++)
		util::terminate_process(util::steamProcesses[i]);

	debug::log::warn("Steam has been closed.");

	std::this_thread::sleep_for(std::chrono::seconds(1));

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, steam_path_w, -1, nullptr, 0, nullptr, nullptr);
	std::string steam_path(size_needed - 1, 0); // Subtract 1 to exclude the null character
	WideCharToMultiByte(CP_UTF8, 0, steam_path_w, -1, &steam_path[0], size_needed, nullptr, nullptr);

	std::string steam_version = util::get_file_version(steam_path + "/steam.exe");
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

	// Extract and unzip files to restore old steam

	if (should_rollback) {
		std::string absolute_path = steam_path + "/" + "old_steam.zip";

		util::extract_resource(absolute_path);

		debug::log::success("ZIP File build successfully: " + absolute_path + "\n");

		std::this_thread::sleep_for(std::chrono::seconds(1));

		mz_zip_archive zip_archive;
		memset(&zip_archive, 0, sizeof(zip_archive));

		if (mz_zip_reader_init_file(&zip_archive, absolute_path.c_str(), 0)) {
			debug::log::success("Extracting files...\n");

			int num_files = mz_zip_reader_get_num_files(&zip_archive);

			for (int i = 0; i < num_files; ++i) {
				mz_zip_archive_file_stat file_stat;
				if (mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
					std::string output_filename = steam_path + std::string("/") + file_stat.m_filename;

					util::display_progress_bar((i + 1) * 100 / num_files);

					if (!mz_zip_reader_extract_to_file(&zip_archive, i, output_filename.c_str(), 0)) {
						debug::log::error("Failed to extract -> \t\t" + output_filename);
					}
				}
			}

			mz_zip_reader_end(&zip_archive);
			debug::log::success("Files extraction clompleted!");
		}
		else {
			debug::log::error("Failed to open zip file.");
			return -1;
		}

		debug::log::success("Deleting zip file...");
		std::remove(absolute_path.c_str());
	}
	else { // Delete essential files and let steam update them
		const std::string file_array[] = { "steam.cfg", "tier0_s.dll", "tier0_s64.dll" };

		for (std::string file : file_array)
			std::remove((steam_path + "/" + file).c_str());

		debug::log::success("Steam restored successfully, starting updater!\n");
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));

	debug::log::success("Starting steam...\n");

	HINSTANCE hInstance = ShellExecuteW(NULL, L"open", L"steam://open/main", NULL, NULL, SW_SHOWNORMAL);
	if ((int)hInstance <= 32) {
		debug::log::error("Could not open Steam.");
		return 0;
	}

	debug::log::warn("Program finished, click any key to close.");
	std::cin.get();

	return 0;
}