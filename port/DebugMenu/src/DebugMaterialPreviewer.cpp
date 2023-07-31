#include "DebugMaterialPreviewer.h"
#include "Callstack.h"
#include "DebugHelpers.h"
#include "imgui_internal.h"

namespace DebugMenu_Internal {
	std::vector<DebugHelpers::DebugMaterial> debugMaterials;

	template <typename... Args>
	DebugHelpers::DebugMaterial& FindOrAddMaterial(Args... args)
	{
		for (auto& material : debugMaterials) {
			if (material.key == DebugHelpers::DebugMaterialKey(args...)) {
				return material;
			}
		}

		debugMaterials.emplace_back(args...);
		return debugMaterials.back();
	}

	// Get the current zoom and scroll values
	static float zoomLevel = 1.0f;
	static ImVec2 scrollOffset(0.0f, 0.0f);

	static void Show(DebugHelpers::DebugMaterial& material, CallstackPreviewerEntry& callstackEntry, std::string name, bool& bOpen) {
		ImGui::Begin("Previewer", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		ImVec2 imageSize(material.texture.width, material.texture.height); // Set the image size

		if (ImGui::CollapsingHeader("Material Image", ImGuiTreeNodeFlags_DefaultOpen)) {

			// Enable zooming and scrolling for the image
			ImGui::BeginChild("ImageContainer", ImVec2(640, 480), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav);

			ImGui::Text(name.c_str());

			// Calculate the available width and height for the image
			const ImVec2 availableSize = ImGui::GetContentRegionAvail();

			if (ImGui::Button("Zoom to fill")) {

				// Calculate the zoom level based on the available space
				DebugMenu_Internal::zoomLevel = std::min<float>(availableSize.x / imageSize.x, availableSize.y / imageSize.y);
			}

			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				MaterialPreviewer::Reset();
			}

			ImGui::Text("Width: %d Height: %d BPP: %d", material.texture.width, material.texture.height, material.texture.bpp);

			// Calculate the scaled size of the image
			ImVec2 scaledSize(imageSize.x * DebugMenu_Internal::zoomLevel, imageSize.y * DebugMenu_Internal::zoomLevel);

			// Calculate the offset for centering the image
			ImVec2 offset((availableSize.x - scaledSize.x) * 0.5f, (availableSize.y - scaledSize.y) * 0.5f);

			offset = ImVec2(offset.x + DebugMenu_Internal::scrollOffset.x, offset.y + DebugMenu_Internal::scrollOffset.y);

			// Use ImGui::Image to display the image with the calculated size and offset
			ImGui::SetCursorPos(offset);
			ImGui::Image(material.texID, scaledSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));

			// Handle mouse interaction for zooming and scrolling
			ImGuiIO& io = ImGui::GetIO();

			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (ImGui::IsMouseHoveringRect(window->Rect().Min, window->Rect().Max)) {
				// Zoom in/out using the mouse wheel
				if (io.MouseWheel != 0.0f) {
					DebugMenu_Internal::zoomLevel += io.MouseWheel * 0.1f;
					DebugMenu_Internal::zoomLevel = std::max<float>(DebugMenu_Internal::zoomLevel, 0.1f);
				}

				// Scroll the image using the right mouse button
				if (io.MouseDown[ImGuiMouseButton_Left]) {
					DebugMenu_Internal::scrollOffset.x += io.MouseDelta.x;
					DebugMenu_Internal::scrollOffset.y += io.MouseDelta.y;
				}
			}

			ImGui::EndChild();
		}

		CallstackPreviewer::Show(callstackEntry);

		ImGui::End();
	}
}

void MaterialPreviewer::Show(MaterialPreviewerEntry& entry, std::string name, bool& bOpen)
{
	auto material = DebugMenu_Internal::FindOrAddMaterial(entry.pMaterial);
	DebugMenu_Internal::Show(material, entry.callstackEntry, name, bOpen);
}

void MaterialPreviewer::Show(const PS2::GSTexValue& texValue, const ImTextureID& texID, std::string name, bool& bOpen)
{
	auto material = DebugMenu_Internal::FindOrAddMaterial(texValue, texID);
	auto emptyCallstack = CallstackPreviewerEntry();
	DebugMenu_Internal::Show(material, emptyCallstack, name, bOpen);
}

void MaterialPreviewer::Reset()
{
	DebugMenu_Internal::zoomLevel = 1.0f;
	DebugMenu_Internal::scrollOffset = ImVec2(0.0f, 0.0f);
}

void MaterialPreviewer::RemoveMaterial(edDList_material* pMaterial)
{
	auto itr = std::find(DebugMenu_Internal::debugMaterials.begin(), DebugMenu_Internal::debugMaterials.end(), pMaterial);
	if (itr != DebugMenu_Internal::debugMaterials.end()) {
		DebugMenu_Internal::debugMaterials.erase(itr);
	}
}
