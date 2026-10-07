#pragma once

#include <imgui.h>
#include <cfloat>

namespace Debug::Ui {
	// Labels share a column; editors fill the remaining width at any UI scale.
	template<class Draw>
	bool Field(const char* label, Draw draw)
	{
		bool changed = false;
		ImGui::PushID(label);
		if (ImGui::BeginTable("Field", 2, ImGuiTableFlags_SizingStretchProp)) {
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.44f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.56f);
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			ImGui::TextWrapped("%s", label);
			ImGui::PopStyleColor();
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			changed = draw("##Value");
			ImGui::EndTable();
		}
		ImGui::PopID();
		return changed;
	}

	inline bool InputFloat(const char* label, float* value)
	{
		return Field(label, [value](const char* id) { return ImGui::InputFloat(id, value); });
	}

	inline void Readout(const char* label, const char* value)
	{
		Field(label, [value](const char*) { ImGui::TextWrapped("%s", value); return false; });
	}

	template<class... Args>
	void Readoutf(const char* label, const char* format, Args... args)
	{
		Field(label, [&](const char*) { ImGui::TextWrapped(format, args...); return false; });
	}

	inline bool ActionButton(const char* label)
	{
		return ImGui::Button(label, ImVec2(-FLT_MIN, 0));
	}
}
