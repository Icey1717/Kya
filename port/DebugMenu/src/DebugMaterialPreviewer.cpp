#include "DebugMaterialPreviewer.h"
#include "Callstack.h"
#include "DebugMaterial.h"
#include "imgui_internal.h"

#include <optional>
#include <memory>
#include "DebugRenderer.h"

namespace DebugMenu_Internal {
	using DebugMaterialPtr = std::shared_ptr<DebugHelpers::DebugMaterial>;
	DebugMaterialPtr gOpenMaterial;

	// Get the current zoom and scroll values
	static float zoomLevel = 1.0f;
	static ImVec2 scrollOffset(0.0f, 0.0f);

	bool Neq(float a, float b, float epsilon) {
		return std::abs(a - b) < epsilon;
	}

	struct ColorGrid
	{
	private:
		struct Filter
		{
			float tolerance = 0.001f;

			bool bActive = false;
			bool bFilterR = false;
			bool bFilterG = false;
			bool bFilterB = false;
			bool bFilterA = false;

			ImVec4 filterColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		} filter;
		
		float customCellSize = 0.0f;

		const float panelSize = 640.0f;

	public:
		void Show(const char* name, bool& bShow, const uint32_t* pixelIndexes, const uint32_t* paletteColors, const int width, const int height)
		{
			ImGui::Begin(name, &bShow, ImGuiWindowFlags_AlwaysAutoResize);

			float cellSize = panelSize / static_cast<float>(std::max(width, height));

			if (customCellSize != 0.0f) {
				cellSize = customCellSize;
			}

			// Draw table with fixed cell heights
			if (ImGui::InputFloat("Cell Height", &cellSize)) {
				customCellSize = cellSize;
			}

			ImGui::SameLine();

			if (ImGui::Button("Reset")) {
				customCellSize = 0.0f;
			}

			ImGui::Checkbox("Filter", &filter.bActive);

			static float tolerance = 0.001f;
			if (filter.bActive) {
				ImGui::InputFloat("Tolerance", &tolerance);

				ImGui::InputFloat("R", &filter.filterColor.x);
				ImGui::SameLine();
				ImGui::Checkbox("Filter R", &filter.bFilterR);

				ImGui::InputFloat("G", &filter.filterColor.y);
				ImGui::SameLine();
				ImGui::Checkbox("Filter G", &filter.bFilterG);

				ImGui::InputFloat("B", &filter.filterColor.z);
				ImGui::SameLine();
				ImGui::Checkbox("Filter B", &filter.bFilterB);

				ImGui::InputFloat("A", &filter.filterColor.w);
				ImGui::SameLine();
				ImGui::Checkbox("Filter A", &filter.bFilterA);
			}

			//ImGui::SetWindowFontScale(0.5f);

			const ImGuiTableFlags flags = ImGuiTableFlags_Borders;
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
			if (ImGui::BeginTable("GridTable", width, flags, ImVec2((cellSize * width) + width, 0))) {
				for (int row = 0; row < height; ++row) {
					ImGui::TableNextRow(0);
					for (int col = 0; col < width; ++col) {
						ImGui::TableSetColumnIndex(col);
						const int paletteIndex = pixelIndexes[col + (row * width)];
						ImU32 cell_bg_color = paletteColors[paletteIndex];

						// Extract color components
						ImVec4 colorRGBA = ImVec4(
							(float)(cell_bg_color & 0xFF) / 255.0f,                 // Red
							(float)((cell_bg_color >> 8) & 0xFF) / 255.0f,          // Green
							(float)((cell_bg_color >> 16) & 0xFF) / 255.0f,         // Blue
							(float)((cell_bg_color >> 24) & 0xFF) / 255.0f          // Alpha
						);

						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);

						if (filter.bActive) {
							if ((Neq(colorRGBA.x, filter.filterColor.x, tolerance) || !filter.bFilterR) &&
								(Neq(colorRGBA.y, filter.filterColor.y, tolerance) || !filter.bFilterG) &&
								(Neq(colorRGBA.z, filter.filterColor.z, tolerance) || !filter.bFilterB) &&
								(Neq(colorRGBA.w, filter.filterColor.w, tolerance) || !filter.bFilterA)) {
								ImGui::ColorButton("", colorRGBA, 0, ImVec2(cellSize, cellSize));
							}
							else {
								ImGui::ColorButton("", { 0.0f, 0.0f, 0.0f, 0.0f }, 0, ImVec2(cellSize, cellSize));
							}
						}
						else {
							ImGui::ColorButton("", colorRGBA, 0, ImVec2(cellSize, cellSize));
						}
						//ImGui::Text("%02X", paletteIndex); // Use %04X for 4 digits and leading zeros
						// Open the tooltip when hovering the cell
						if (ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("");
							ImGui::Text("Cell (%d, %d)\nPalette Index: %02X", col, row, paletteIndex);
							ImGui::EndTooltip();
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();
			ImGui::End();
		}
	};

	static void Show(DebugHelpers::DebugMaterial& debugMaterial, bool& bOpen) 
	{
		const auto& imageData = debugMaterial.texture.image.imageData;
		const auto& paletteData = debugMaterial.texture.paletteImage.imageData;

		ImGui::Begin("Previewer", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		ImVec2 imageSize(imageData.canvasWidth, imageData.canvasHeight); // Set the image size

		static bool bShowTextureColorGrid = false;
		static bool bShowFinalPaletteColorGrid = false;
		static bool bShowRawPaletteColorGrid = false;

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
			if (ImGui::Button("Show Texture Color Grid")) {
				bShowTextureColorGrid = !bShowTextureColorGrid;
			}

			ImGui::Text("Image Width: %d Height: %d Read Width: %d Read Height: %d BPP: %d MXL: %d", imageData.canvasWidth, imageData.canvasHeight, imageData.readWidth, imageData.readHeight, imageData.bpp, imageData.maxMipLevel);
			ImGui::Text("Palette Width: %d Height: %d Read Width: %d Read Height: %d BPP: %d MXL: %d", paletteData.canvasWidth, paletteData.canvasHeight, paletteData.readWidth, paletteData.readHeight, paletteData.bpp, paletteData.maxMipLevel);

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

		if (bShowTextureColorGrid) {
			const auto& debugData = debugMaterial.texture.debugData;
			const auto& imageData = debugMaterial.texture.image.imageData;

			static ColorGrid textureColorGrid;
			textureColorGrid.Show("Texture Color Grid", bShowTextureColorGrid, debugData.paletteIndexes.data(), (uint32_t*)debugMaterial.texture.paletteImage.readBuffer.data(), imageData.canvasWidth, imageData.canvasHeight);
		}		

		const int scale = 20;
		ImGui::Begin("Palette", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		ImVec2 paletteSize = { paletteData.readWidth * scale, paletteData.readHeight * scale };
		ImGui::Image(debugMaterial.paletteTexID, paletteSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));

		std::vector<uint32_t> indexes;
		indexes.resize(paletteData.readWidth * paletteData.readHeight);
		for (int i = 0; i < paletteData.readWidth * paletteData.readHeight; i++) {
			indexes[i] = i;
		}

		if (bShowRawPaletteColorGrid) {
			static ColorGrid rawPaletteColorGrid;
			rawPaletteColorGrid.Show("Raw Palette", bShowRawPaletteColorGrid, indexes.data(), (uint32_t*)paletteData.pImage, paletteData.readWidth, paletteData.readHeight);
		}

		if (bShowFinalPaletteColorGrid) {
			static ColorGrid finalPaletteColorGrid;
			finalPaletteColorGrid.Show("Final Palette", bShowFinalPaletteColorGrid, indexes.data(), (uint32_t*)debugMaterial.texture.paletteImage.readBuffer.data(), paletteData.readWidth, paletteData.readHeight);
		}

		ImGui::End();

		CallstackPreviewer::Show(debugMaterial.callstackEntry);

		ImGui::End();
	}
}

using namespace DebugMenu_Internal;


namespace PaletteSelector
{
	std::optional<MaterialPreviewerEntry> gOpenEntry;
	Renderer::PaletteMap gPaletteMap;

	void Show(const MaterialPreviewerEntry& entry, bool& bOpen)
	{
		ImGui::Begin("Palette Selector", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		if (gPaletteMap.size() > 0) {

			auto paletteEnd = --gPaletteMap.end();

			for (auto& palette : gPaletteMap) {
				std::string label = "Palette ";
				label += std::to_string(palette.first);
				if (ImGui::Selectable(label.c_str()) || (!gOpenMaterial && palette.first == paletteEnd->first)) {
					MaterialPreviewer::Reset();
					gOpenMaterial = DebugMaterialPtr(new DebugHelpers::DebugMaterial(entry.pMaterial, palette.first));
					gOpenMaterial->callstackEntry = entry.callstackEntry;
					gOpenMaterial->name = entry.name + " - " + label;
				}
			}
		}

		ImGui::End();
	}

	void Update()
	{
		if (gOpenEntry.has_value()) {
			bool bOpen = true;
			Show(*gOpenEntry, bOpen);

			if (!bOpen) {
				gOpenEntry.reset();
			}
		}
	}
}

void MaterialPreviewer::Update()
{
	PaletteSelector::Update();

	if (gOpenMaterial) {
		bool bOpen = true;
		Show(*gOpenMaterial, bOpen);

		if (!bOpen) {
			gOpenMaterial.reset();
		}
	}
}

void MaterialPreviewer::Open(MaterialPreviewerEntry& entry)
{
	Reset();
	PaletteSelector::gOpenEntry = entry;

	const auto textureData = DebugMenu::LoadTextureData(entry.pMaterial);
	PaletteSelector::gPaletteMap = textureData.palettes;
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
	gOpenMaterial.reset();
	zoomLevel = 1.0f;
	scrollOffset = ImVec2(0.0f, 0.0f);
}
