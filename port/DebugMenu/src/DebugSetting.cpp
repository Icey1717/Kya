#include "DebugSetting.h"

#include <fstream>
#include <optional>
#include <imgui.h>

namespace Debug {
	const char* gSettingsFile = "settings.json";
}

void Debug::SaveSettings(Settings settings)
{
	std::ofstream file(gSettingsFile);
	file << *settings;
	file.close();
}

Debug::Settings Debug::LoadSettings(bool bCreateIfNotExisting /*= false*/)
{
	Settings settings;

	std::ifstream file(gSettingsFile);
	if (file) {
		settings = nlohmann::json::parse(file);
		file.close();
	}
	else if (bCreateIfNotExisting) {
		std::ofstream outfile(gSettingsFile);
		nlohmann::json j;
		outfile << j;
		outfile.close();
		return LoadSettings(false);
	}
	return settings;
}

template<>
bool Debug::Setting<bool>::DrawImguiControl()
{
	if (ImGui::Checkbox(name.c_str(), &value)) {
		return UpdateValue();
	}

	return true;
}

template<>
bool Debug::Setting<float>::DrawImguiControl()
{
	if (ImGui::InputFloat(name.c_str(), &value)) {
		return UpdateValue();
	}

	return true;
}

template<>
bool Debug::Setting<int>::DrawImguiControl()
{
	if (ImGui::InputInt(name.c_str(), &value)) {
		return UpdateValue();
	}
	return true;
}