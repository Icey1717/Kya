#include "DebugMaterialPreviewer.h"
#include "Callstack.h"
#include "DebugMaterial.h"
#include "imgui_internal.h"

#include <optional>
#include <memory>

namespace DebugMenu_Internal {
	using DebugMaterialPtr = std::shared_ptr<DebugHelpers::DebugMaterial>;
	DebugMaterialPtr gOpenMaterial;

	// Get the current zoom and scroll values
	static float zoomLevel = 1.0f;
	static ImVec2 scrollOffset(0.0f, 0.0f);

	static void Show(DebugHelpers::DebugMaterial& debugMaterial, bool& bOpen) {
		const auto& imageData = debugMaterial.texture.image.imageData;
		const auto& paletteData = debugMaterial.texture.paletteImage.imageData;

		ImGui::Begin("Previewer", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		ImVec2 imageSize(imageData.canvasWidth, imageData.canvasHeight); // Set the image size

		static bool bShowPalette = false;

		if (ImGui::CollapsingHeader("Material Image", ImGuiTreeNodeFlags_DefaultOpen)) {

			// Enable zooming and scrolling for the image
			ImGui::BeginChild("ImageContainer", ImVec2(640, 480), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav);

			ImGui::Text(debugMaterial.name.c_str());

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

			ImGui::SameLine();
			if (ImGui::Button("Show Palette")) {
				bShowPalette = !bShowPalette;
			}

			ImGui::Text("Image Width: %d Height: %d Read Width: %d Read Height: %d BPP: %d", imageData.canvasWidth, imageData.canvasHeight, imageData.readWidth, imageData.readHeight, imageData.bpp);
			ImGui::Text("Palette Width: %d Height: %d Read Width: %d Read Height: %d BPP: %d", paletteData.canvasWidth, paletteData.canvasHeight, paletteData.readWidth, paletteData.readHeight, paletteData.bpp);

			// Calculate the scaled size of the image
			ImVec2 scaledSize(imageSize.x * zoomLevel, imageSize.y * zoomLevel);

			// Calculate the offset for centering the image
			ImVec2 offset((availableSize.x - scaledSize.x) * 0.5f, (availableSize.y - scaledSize.y) * 0.5f);

			offset = ImVec2(offset.x + scrollOffset.x, offset.y + scrollOffset.y);

			// Use ImGui::Image to display the image with the calculated size and offset
			ImGui::SetCursorPos(offset);

			ImGui::Image(debugMaterial.texID, scaledSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));


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

		if (bShowPalette) {
			ImGui::Begin("PaletteIndexes", &bShowPalette, ImGuiWindowFlags_AlwaysAutoResize);
			// Draw table with fixed cell heights
			float cellHeight = 22.0f;

			const ImGuiTableFlags flags = ImGuiTableFlags_Borders;
			const auto& paletteIndexes = debugMaterial.texture.debugData.paletteIndexes;

			if (ImGui::BeginTable("GridTable", imageData.canvasWidth, flags, ImVec2(0, 0))) {
				for (int row = 0; row < imageData.canvasHeight; ++row) {
					ImGui::TableNextRow(0, cellHeight);
					for (int col = 0; col < imageData.canvasWidth; ++col) {
						ImGui::TableSetColumnIndex(col);
						const int paletteIndex = paletteIndexes[col + (row * imageData.canvasWidth)];
						ImU32 cell_bg_color = debugMaterial.texture.debugData.convertedPalette[paletteIndex];
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
						ImGui::Text("%02X", paletteIndex); // Use %04X for 4 digits and leading zeros

					}
				}
				ImGui::EndTable();
			}
			ImGui::End();
		}		

		const int scale = 20;
		ImGui::Begin("Palette", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		ImVec2 paletteSize = { paletteData.readWidth * scale, paletteData.readHeight * scale };
		ImGui::Image(debugMaterial.paletteTexID, paletteSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));

		ImGui::End();

		CallstackPreviewer::Show(debugMaterial.callstackEntry);

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
	gOpenMaterial = DebugMaterialPtr(new DebugHelpers::DebugMaterial(entry.pMaterial));
	gOpenMaterial->callstackEntry = entry.callstackEntry;
	gOpenMaterial->name = name;
}

void MaterialPreviewer::Open(const PS2::GSTexValue& texValue, const ImageTextureID& texIDs, std::string name)
{
	Reset();
	gOpenMaterial = DebugMaterialPtr(new DebugHelpers::DebugMaterial(texValue, texIDs.image));
	gOpenMaterial->paletteTexID = texIDs.palette;
	gOpenMaterial->name = name;
}

void MaterialPreviewer::Reset()
{
	zoomLevel = 1.0f;
	scrollOffset = ImVec2(0.0f, 0.0f);
}
