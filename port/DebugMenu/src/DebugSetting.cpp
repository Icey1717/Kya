#include "DebugSetting.h"

#include <fstream>
#include <optional>
#include <imgui.h>

namespace DebugSetting {
	const char* gSettingsFile = "settings.json";

	template<typename SettingType>
	bool UpdateValue(const char* name, SettingType value)
	{
		Settings settings = LoadSettings(true);

		if (settings) {
			(*settings)[name] = value;
			std::ofstream file(gSettingsFile);
			file << *settings;
			file.close();
			return true;
		}

		return false;
	}
}

DebugSetting::Settings DebugSetting::LoadSettings(bool bCreateIfNotExisting /*= false*/)
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
bool DebugSetting::Setting<bool>::DrawImguiControl()
{
	if (ImGui::Checkbox(name.c_str(), &value)) {
		return UpdateValue(name.c_str(), value);
	}

	return true;
}

template<>
bool DebugSetting::Setting<float>::DrawImguiControl()
{
	if (ImGui::InputFloat(name.c_str(), &value)) {
		return UpdateValue(name.c_str(), value);
	}

	return true;
}