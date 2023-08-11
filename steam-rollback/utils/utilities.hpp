#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>

#define size(arr) sizeof(arr) / sizeof(arr[0])

namespace util {
	const char* steamProcesses[] = { "csgo.exe", "Steam.exe", "SteamClient.exe", "SteamService.exe", "SteamWebHelper.exe", "steamwebhelper.exe" };

	bool terminate_process(const char* pName) {
		PROCESSENTRY32 processEntry;
		processEntry.dwSize = sizeof(PROCESSENTRY32);

		// Create a snapshot of the current running processes
		HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (processSnapshot == INVALID_HANDLE_VALUE) {
			return false;
		}

		// Iterate through the running processes
		if (Process32First(processSnapshot, &processEntry)) {
			do {
				if (_stricmp(processEntry.szExeFile, pName) == 0) {
					// Get the process ID and open the process
					DWORD processId = processEntry.th32ProcessID;
					HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);

					if (hProcess != NULL) {
						// Terminate the process
						if (TerminateProcess(hProcess, 0)) {
							CloseHandle(hProcess);
							CloseHandle(processSnapshot);
							return true;
						}

						CloseHandle(hProcess);
					}
				}
			} while (Process32Next(processSnapshot, &processEntry));
		}

		// Clean up and return false if the process was not found or couldn't be terminated
		CloseHandle(processSnapshot);
		return false;
	}

	std::string get_file_version(std::string filePath) {
		DWORD handle;
		DWORD versionSize = GetFileVersionInfoSizeA(filePath.c_str(), &handle);
		if (versionSize == 0) {
			return "Error getting version info size";
		}

		std::vector<char> versionData(versionSize);
		if (!GetFileVersionInfoA(filePath.c_str(), handle, versionSize, versionData.data())) {
			return "Error getting version info";
		}

		VS_FIXEDFILEINFO* fileInfo;
		UINT fileInfoSize;
		if (!VerQueryValueA(versionData.data(), "\\", reinterpret_cast<LPVOID*>(&fileInfo), &fileInfoSize)) {
			return "Error querying version value";
		}

		WORD majorVersion = HIWORD(fileInfo->dwFileVersionMS);
		WORD minorVersion = LOWORD(fileInfo->dwFileVersionMS);
		WORD buildNumber = HIWORD(fileInfo->dwFileVersionLS);
		WORD revisionNumber = LOWORD(fileInfo->dwFileVersionLS);

		return std::to_string(majorVersion) + "." +
			std::to_string(minorVersion) + "." +
			std::to_string(buildNumber) + "." +
			std::to_string(revisionNumber);
	}

	std::string fix_path_slashes(const std::string& path) {
		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');
		return fixedPath;
	}

	bool file_exists(const std::string& filename) {
		std::ifstream file(filename);
		return file.good();
	}

	void display_progress_bar(int progress) {
		std::cout << "\r";

		int barWidth = 110;
		int pos = barWidth * progress / 100;

		std::cout << "  " << progress << "% [";
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) std::cout << "|";
			else if (i == pos) std::cout << ">";
			else std::cout << "-";
		}
		std::cout << "] " << std::flush;

		if (progress == 100)
			std::cout << "\n" << std::endl;
	}

	void extract_resource(const std::string& outputFilePath)
	{
		HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(IDR_OLDSTEAM1), "oldsteam");
		if (hResource)
		{
			HGLOBAL hResourceData = LoadResource(NULL, hResource);
			if (hResourceData)
			{
				void* pResourceData = LockResource(hResourceData);
				DWORD dataSize = SizeofResource(NULL, hResource);

				std::ofstream outputFile(outputFilePath, std::ios::binary);
				outputFile.write(reinterpret_cast<char*>(pResourceData), dataSize);
				outputFile.close();
			}
		}
	}

	bool check_open_processname(const char* pName) {
		PROCESSENTRY32 processEntry;
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		bool isOpen = false;
		// Create a snapshot of the current running processes
		HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (processSnapshot == INVALID_HANDLE_VALUE) {
			return false;
		}

		// Iterate through the running processes
		if (Process32First(processSnapshot, &processEntry)) {
			do {
				if (_stricmp(processEntry.szExeFile, pName) == 0) {
					isOpen = true;
				}
			} while (Process32Next(processSnapshot, &processEntry));
		}

		// Clean up and return false if the process was not found or couldn't be terminated
		CloseHandle(processSnapshot);

		return isOpen;
	}
}