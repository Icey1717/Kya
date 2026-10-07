#include "DebugSetting.h"

#include <fstream>
#include <optional>
#include <imgui.h>
#include "DebugUi.h"

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

bool Debug::ComboSetting::DrawImguiControl()
{
	int current = value;
	if (Ui::Field(name.c_str(), [&](const char* id) { return ImGui::Combo(id, &current, labels.data(), static_cast<int>(labels.size())); })) {
		value = current;
		return UpdateValue();
	}
	return false;
}

template<>
bool Debug::Setting<bool>::DrawImguiControl()
{
	if (ImGui::Checkbox(name.c_str(), &value)) {
		return UpdateValue();
	}

	return false;
}

template<>
bool Debug::Setting<float>::DrawImguiControl()
{
	if (Ui::InputFloat(name.c_str(), &value)) {
		return UpdateValue();
	}

	return false;
}

template<>
bool Debug::Setting<int>::DrawImguiControl()
{
	if (Ui::Field(name.c_str(), [&](const char* id) { return ImGui::InputInt(id, &value); })) {
		return UpdateValue();
	}
	return false;
}

template<>
bool Debug::Setting<std::array<float, 4>>::DrawImguiControl()
{
	if (ImGui::ColorEdit4(name.c_str(), value.data(), ImGuiColorEditFlags_NoInputs)) {
		return UpdateValue();
	}
	return false;
}

template<>
bool Debug::Setting<std::string>::DrawImguiControl()
{
	char buffer[256];
	strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination

	if (Ui::Field(name.c_str(), [&](const char* id) { return ImGui::InputText(id, buffer, sizeof(buffer)); })) {
		value = std::string(buffer);
		return UpdateValue();
	}

	return false;
}
