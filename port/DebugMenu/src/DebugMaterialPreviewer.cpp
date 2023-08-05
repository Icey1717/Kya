#include "DebugMaterialPreviewer.h"
#include "Callstack.h"
#include "DebugMaterial.h"
#include "imgui_internal.h"

#include <optional>
#include <memory>

namespace DebugMenu_Internal {
	using DebugMaterialPtr = std::shared_ptr<DebugHelpers::DebugMaterial>;
	std::vector<DebugMaterialPtr> gDebugMaterials;
	DebugMaterialPtr gOpenMaterial;

	template <typename... Args>
	DebugMaterialPtr FindOrAddMaterial(Args... args)
	{
		for (auto& material : gDebugMaterials) {
			if (material->key == DebugHelpers::DebugMaterialKey(args...)) {
				return material;
			}
		}

		gDebugMaterials.emplace_back(new DebugHelpers::DebugMaterial(args...));
		return gDebugMaterials.back();
	}

	// Get the current zoom and scroll values
	static float zoomLevel = 1.0f;
	static ImVec2 scrollOffset(0.0f, 0.0f);

	static void Show(DebugHelpers::DebugMaterial& material, bool& bOpen) {
		ImGui::Begin("Previewer", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		ImVec2 imageSize(material.texture.image.imageData.canvasWidth, material.texture.image.imageData.canvasHeight); // Set the image size

		if (ImGui::CollapsingHeader("Material Image", ImGuiTreeNodeFlags_DefaultOpen)) {

			// Enable zooming and scrolling for the image
			ImGui::BeginChild("ImageContainer", ImVec2(640, 480), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav);

			ImGui::Text(material.name.c_str());

			// Calculate the available width and height for the image
			const ImVec2 availableSize = ImGui::GetContentRegionAvail();

			if (ImGui::Button("Zoom to fill")) {

				// Calculate the zoom level based on the available space
				zoomLevel = std::min<float>(availableSize.x / imageSize.x, availableSize.y / imageSize.y);
			}

			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				MaterialPreviewer::Reset();
			}

			ImGui::Text("Width: %d Height: %d BPP: %d", material.texture.image.imageData.canvasWidth, material.texture.image.imageData.canvasHeight, material.texture.image.imageData.bpp);

			// Calculate the scaled size of the image
			ImVec2 scaledSize(imageSize.x * zoomLevel, imageSize.y * zoomLevel);

			// Calculate the offset for centering the image
			ImVec2 offset((availableSize.x - scaledSize.x) * 0.5f, (availableSize.y - scaledSize.y) * 0.5f);

			offset = ImVec2(offset.x + scrollOffset.x, offset.y + scrollOffset.y);

			// Use ImGui::Image to display the image with the calculated size and offset
			ImGui::SetCursorPos(offset);
			ImGui::Image(material.texID, scaledSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));

			// Handle mouse interaction for zooming and scrolling
			ImGuiIO& io = ImGui::GetIO();

			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (ImGui::IsMouseHoveringRect(window->Rect().Min, window->Rect().Max)) {
				// Zoom in/out using the mouse wheel
				if (io.MouseWheel != 0.0f) {
					zoomLevel += io.MouseWheel * 0.1f;
					zoomLevel = std::max<float>(zoomLevel, 0.1f);
				}

				// Scroll the image using the right mouse button
				if (io.MouseDown[ImGuiMouseButton_Left]) {
					scrollOffset.x += io.MouseDelta.x;
					scrollOffset.y += io.MouseDelta.y;
				}
			}

			ImGui::EndChild();
		}

		CallstackPreviewer::Show(material.callstackEntry);

		ImGui::End();
	}
}

using namespace DebugMenu_Internal;

void MaterialPreviewer::Update()
{
	if (gOpenMaterial) {
		bool bOpen = true;
		Show(*gOpenMaterial, bOpen);

		if (!bOpen) {
			gOpenMaterial.reset();
		}
	}
}

void MaterialPreviewer::Open(MaterialPreviewerEntry& entry, std::string name)
{
	Reset();
	gOpenMaterial = FindOrAddMaterial(entry.pMaterial);
	gOpenMaterial->callstackEntry = entry.callstackEntry;
	gOpenMaterial->name = name;
}

void MaterialPreviewer::Open(const PS2::GSTexValue& texValue, const ImageTextureID& texIDs, std::string name)
{
	Reset();
	gOpenMaterial = FindOrAddMaterial(texValue, texIDs.palette);
	gOpenMaterial->paletteTexID = texIDs.palette;
	gOpenMaterial->name = name;
}

void MaterialPreviewer::Reset()
{
	zoomLevel = 1.0f;
	scrollOffset = ImVec2(0.0f, 0.0f);
}

void MaterialPreviewer::RemoveMaterial(edDList_material* pMaterial)
{
	auto predicate = [pMaterial](const DebugMaterialPtr& debugMaterial) {
		return debugMaterial && debugMaterial->key == DebugHelpers::DebugMaterialKey(pMaterial);
	};

	auto itr = std::find_if(gDebugMaterials.begin(), gDebugMaterials.end(), predicate);
	if (itr != gDebugMaterials.end()) {
		gDebugMaterials.erase(itr);
	}
}