#include "DebugMenu.h"
#include "DebugMenuLog.h"

#include <imgui.h>
#include <cfloat>
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
		static bool expanded = false;
		// Keep the original window docked. Undocking it can destroy an empty
		// split, so restoring just its DockId cannot reliably restore the layout.
		if (ImGui::Begin("Log Window", bOpen) && !expanded) {
			if (ImGui::Button("Expand")) expanded = true;
			ImGui::SameLine();
			DrawLogContents();
		}
		ImGui::End();
		if (!*bOpen) expanded = false;
		if (expanded) {
			const auto* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			const auto flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
			if (ImGui::Begin("Log Window (Expanded)", &expanded, flags)) {
				if (ImGui::Button("Restore")) expanded = false;
				ImGui::SameLine();
				DrawLogContents();
			}
			ImGui::End();
		}
	}

	void DrawLogContents()
	{
		static ImGuiTextFilter filter;
		static ImGuiTextFilter categoryFilter;
		static bool paused = false;
		static bool follow = true;
		static double nextRefresh = 0.0;
		static std::vector<std::string> messages;
		if (ImGui::Button("Capture categories")) ImGui::OpenPopup("Categories");
		if (ImGui::BeginPopup("Categories")) {
			ImGui::TextWrapped("Controls which categories are written to the UI and log files.");
			categoryFilter.Draw("Find category", ImGui::GetFontSize() * 16);
			ImGui::BeginChild("CategoryList", ImVec2(ImGui::GetFontSize() * 24, ImGui::GetFontSize() * 15));
			sLogCache.Refresh(Log::GetInstance().logs);
			for (auto& [pName, pEntry] : sLogCache.entries) {
				if (categoryFilter.PassFilter(pName->c_str()) && ImGui::Checkbox(pName->c_str(), &pEntry->bEnabled)) {
					UpdateCategoryInConfigFile(*pName, pEntry->bEnabled);
				}
			}
			ImGui::EndChild();
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Pause", &paused);
		ImGui::SameLine();
		ImGui::Checkbox("Follow", &follow);
		ImGui::SetNextItemWidth(-FLT_MIN);
		filter.Draw("##Search", -1.0f);
		ImGui::SetItemTooltip("Filter messages. Commas separate terms; prefix a term with - to exclude it.");
		if (!paused && ImGui::GetTime() >= nextRefresh) {
			messages = Log::GetRecentMessages();
			nextRefresh = ImGui::GetTime() + 0.25;
		}
		ImGui::TextDisabled("Latest %zu / 512 messages%s", messages.size(), paused ? " (paused)" : "");
		ImGui::BeginChild("Messages", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
		const bool atBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;
		if (messages.empty()) ImGui::TextDisabled("No captured messages yet. Enable a category to capture its output.");
		for (const auto& message : messages) {
			if (filter.PassFilter(message.c_str())) ImGui::TextUnformatted(message.c_str());
		}
		if (follow && atBottom && !paused) ImGui::SetScrollHereY(1.0f);
		ImGui::EndChild();
	}

} // namespace Debug

namespace Debug {
    MenuRegisterer sDebugLogMenuReg("Log", Debug::ShowLogWindow);
}

