#include "DebugCallstackPreviewer.h"
#include "Callstack.h"
#include "imgui.h"

void CallstackPreviewer::Show(CallstackPreviewerEntry& entry)
{
	if (!entry.backtrace.empty()) {
		if (ImGui::CollapsingHeader("Callstack")) {
			ImGui::BeginChild("Callstack Frame", ImVec2(640, 300), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::Text("Callstack");

			static bool bShowFileName = false;
			static bool bShowLineNumber = false;
			bool bUpdateCallstack = entry.callstack.empty();
			bUpdateCallstack |= ImGui::Checkbox("Show File Name", &bShowFileName);
			ImGui::SameLine();
			bUpdateCallstack |= ImGui::Checkbox("Show Line Number", &bShowLineNumber);

			if (bUpdateCallstack) {
				entry.callstack.clear();
				CollectCallstack(entry.callstack, entry.backtrace, bShowFileName, bShowLineNumber);
			}

			ImGui::BeginChild("Callstack List", ImVec2(640, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
			// Show callstack in Dear ImGui window
			for (const std::string& frame : entry.callstack) {
				ImGui::Text("%s", frame.c_str());
			}
			ImGui::EndChild();
			ImGui::EndChild();
		}
	}
	else {
		ImGui::Text("Back trace is empty.");
	}
}

