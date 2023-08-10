#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <windows.h>
#include <iostream>

namespace debug {
	enum class DebugType {
		None,
		Success,
		Warning,
		Error
	};

	std::string current_date_time();

	void allocate_console();

	void cout(DebugType type, const std::string& message);

	namespace log {
		void success(const std::string& text);

		void warn(const std::string& text);

		void error(const std::string& text);

		void out(const std::string& text);
	}
}

namespace debug {
	std::string current_date_time() {
		SYSTEMTIME st;
		GetLocalTime(&st);
		char buffer[32];
		sprintf_s(buffer, sizeof(buffer), "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
		return std::string(buffer);
	}

	void allocate_console() {
		Beep(750, 300);
		AllocConsole();
		SetConsoleTitle("Debug Console");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		log::success("Debug console allocated!");
	}

	void cout(DebugType type, const std::string& message) {
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		WORD color = 0;
		std::string prefix;

		switch (type) {
		case DebugType::None:
			color = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			prefix = "[" + current_date_time() + "]";
			break;
		case DebugType::Success:
			color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			prefix = "[" + current_date_time() + "][Success]";
			break;
		case DebugType::Warning:
			color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			prefix = "[" + current_date_time() + "][Warning]";
			break;
		case DebugType::Error:
			color = FOREGROUND_RED | FOREGROUND_INTENSITY;
			prefix = "[" + current_date_time() + "][Error]";
			break;
		}

		SetConsoleTextAttribute(console, color);
		std::cout << prefix << " " << message << std::endl;
		SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}

	namespace log {
		void success(const std::string& text) {
			cout(DebugType::Success, text);
		}

		void warn(const std::string& text) {
			cout(DebugType::Warning, text);
		}

		void error(const std::string& text) {
			cout(DebugType::Error, text);
		}

		void out(const std::string& text) {
			cout(DebugType::None, text);
		}
	}
}

#endif // DEBUG_HPP
