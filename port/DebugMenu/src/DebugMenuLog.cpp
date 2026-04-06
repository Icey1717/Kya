#include "DebugMenu.h"
#include "DebugMenuLog.h"

#include <imgui.h>
#include "log.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Debug {

	// Iterating std::unordered_map every frame is expensive in MSVC debug builds:
	// each iterator construction/increment/destruction acquires a critical section
	// and heap-allocates a _Container_proxy. Cache a sorted vector of raw pointers
	// instead and only rebuild it when the map grows (i.e. a new category is added).
	struct LogCategoryCache
	{
		struct Entry { const std::string* name; LogEntry* entry; };
		std::vector<Entry> entries;
		size_t lastMapSize = 0;

		void Refresh(LogMap& map)
		{
			if (map.size() == lastMapSize)
				return;
			entries.clear();
			entries.reserve(map.size());
			for (auto& [name, entry] : map)
				entries.push_back({ &name, &entry });
			std::sort(entries.begin(), entries.end(),
				[](const Entry& a, const Entry& b) { return *a.name < *b.name; });
			lastMapSize = map.size();
		}
	};

	static LogCategoryCache sLogCache;

	struct ConfigEntry { std::string name; bool enabled; };

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

		std::vector<ConfigEntry> entries;
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

		sLogCache.Refresh(Log::GetInstance().logs);
		for (auto& [pName, pEntry] : sLogCache.entries)
		{
			if (ImGui::Checkbox(pName->c_str(), &pEntry->bEnabled)) {
				UpdateCategoryInConfigFile(*pName, pEntry->bEnabled);
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

		sLogCache.Refresh(Log::GetInstance().logs);
		for (auto& [pName, pEntry] : sLogCache.entries) {
			if (ImGui::Checkbox(pName->c_str(), &pEntry->bEnabled)) {
				UpdateCategoryInConfigFile(*pName, pEntry->bEnabled);
			}
		}
	}

} // namespace Debug

namespace Debug {
    MenuRegisterer sDebugLogMenuReg("Log", Debug::ShowLogWindow);
}

