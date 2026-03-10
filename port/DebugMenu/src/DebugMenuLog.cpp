#include "DebugMenu.h"
#include "DebugMenuLog.h"

#include <imgui.h>
#include "log.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Debug {

	struct CategoryEntry {
		std::string name;
		bool enabled;
	};

	static void AddCategoryToConfigFile(const std::string& category, bool enabled)
	{
		std::ofstream outFile("category_config.txt", std::ios::app);

		if (!outFile.is_open()) {
			std::cerr << "Error: Unable to open config file for writing." << std::endl;

			std::ofstream createFile("category_config.txt");
			if (!createFile.is_open()) {
				std::cerr << "Error: Unable to create config file." << std::endl;
				return;
			}
			createFile.close();

			outFile.open("category_config.txt", std::ios::app);
			if (!outFile.is_open()) {
				std::cerr << "Error: Unable to open config file for writing." << std::endl;
				return;
			}
		}

		outFile << category << " " << std::boolalpha << enabled << std::endl;
		outFile.close();
	}

	static void UpdateCategoryInConfigFile(const std::string& category, bool enabled)
	{
		std::ifstream inFile("category_config.txt");
		if (!inFile.is_open()) {
			std::cerr << "Error: Config file not found. Creating a new one." << std::endl;

			std::ofstream createFile("category_config.txt");
			if (!createFile.is_open()) {
				std::cerr << "Error: Unable to create config file." << std::endl;
				return;
			}
			createFile.close();

			inFile.open("category_config.txt");
			if (!inFile.is_open()) {
				std::cerr << "Error: Unable to open config file for reading." << std::endl;
				return;
			}
		}

		bool bFound = false;

		std::vector<CategoryEntry> entries;
		std::string line;
		while (std::getline(inFile, line)) {
			std::istringstream iss(line);
			std::string categoryName;
			bool categoryEnabled;
			if (iss >> categoryName >> std::boolalpha >> categoryEnabled) {
				if (categoryName == category) {
					bFound = true;
					categoryEnabled = enabled;
				}
				entries.push_back({ categoryName, categoryEnabled });
			}
		}
		inFile.close();

		if (!bFound) {
			AddCategoryToConfigFile(category, enabled);
			return;
		}

		std::ofstream outFile("category_config.txt");
		if (!outFile.is_open()) {
			std::cerr << "Error: Unable to open config file for writing." << std::endl;
			return;
		}

		for (const auto& entry : entries) {
			outFile << entry.name << " " << std::boolalpha << entry.enabled << std::endl;
		}
		outFile.close();
	}

	void ShowLogWindow(bool* bOpen)
	{
		ImGui::Begin("Log Window", bOpen);

		for (int level = static_cast<int>(LogLevel::VeryVerbose); level < static_cast<int>(LogLevel::Max); ++level)
		{
			LogLevel logLevel = static_cast<LogLevel>(level);
			if (ImGui::Selectable(LogLevelToString(logLevel).c_str())) {
			}
		}

		for (auto& [category, log] : Log::GetInstance().GetLogs())
		{
			if (ImGui::Checkbox(category.c_str(), &log.bEnabled)) {
				UpdateCategoryInConfigFile(category, log.bEnabled);
			}
		}

		ImGui::End();
	}

	void DrawLogContents()
	{
		ImGui::Text("Log Categories");
		ImGui::Separator();

		for (int level = static_cast<int>(LogLevel::VeryVerbose); level < static_cast<int>(LogLevel::Max); ++level) {
			LogLevel logLevel = static_cast<LogLevel>(level);
			ImGui::BulletText("%s", LogLevelToString(logLevel).c_str());
		}

		ImGui::Separator();

		for (auto& [category, log] : Log::GetInstance().GetLogs()) {
			if (ImGui::Checkbox(category.c_str(), &log.bEnabled)) {
				UpdateCategoryInConfigFile(category, log.bEnabled);
			}
		}
	}

} // namespace Debug

namespace Debug {
    MenuRegisterer sDebugLogMenuReg("Log", Debug::ShowLogWindow);
}

