#include "DebugMaterialPreviewer.h"
#include "Callstack.h"
#include "DebugMaterial.h"
#include "imgui_internal.h"

#include <optional>
#include <memory>
#include "DebugRenderer.h"
#include "magic_enum.hpp"
/*
namespace DebugMenu_Internal
{
	using DebugMaterialPtr = std::shared_ptr<DebugHelpers::DebugMaterial>;
	DebugMaterialPtr gOpenMaterial;
}

namespace PaletteSelector
{
	std::optional<MaterialPreviewerEntry> gOpenEntry;
	Renderer::PaletteMap gPaletteMap;

	void Show(const MaterialPreviewerEntry& entry, bool& bOpen, bool bNewWindow = false)
	{
		if (bNewWindow) {
			ImGui::Begin("Palette Selector", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		}

		auto SelectPalette = [&entry](int CBP, std::string label)
			{
				DebugMenu_Internal::gOpenMaterial = DebugMenu_Internal::DebugMaterialPtr(new DebugHelpers::DebugMaterial(entry.pMaterial, CBP));
				DebugMenu_Internal::gOpenMaterial->callstackEntry = entry.callstackEntry;
				DebugMenu_Internal::gOpenMaterial->name = entry.name + " - " + label;
			};

		if (gPaletteMap.size() > 0) {

			if (!DebugMenu_Internal::gOpenMaterial) {
				const int CPB = gPaletteMap.begin()->first;
				std::string label = "Palette ";
				label += std::to_string(CPB);

				SelectPalette(CPB, label);
			}

			auto paletteEnd = --gPaletteMap.end();

			if (ImGui::BeginCombo("##PaletteList", DebugMenu_Internal::gOpenMaterial.get() ? DebugMenu_Internal::gOpenMaterial->name.c_str() : "None")) {

				for (auto& palette : gPaletteMap) {
					const int CBP = palette.first;
					std::string label = "Palette ";
					label += std::to_string(CBP);
					if (ImGui::Selectable(label.c_str()) && DebugMenu_Internal::gOpenMaterial->CBP != CBP) {
						MaterialPreviewer::Reset();
						SelectPalette(CBP, label);
					}
				}

				ImGui::EndCombo();
			}
		}

		if (bNewWindow) {
			ImGui::End();
		}
	}

	void Update(bool bNewWindow = false)
	{
		if (gOpenEntry.has_value()) {
			bool bOpen = true;
			Show(*gOpenEntry, bOpen, bNewWindow);

			if (!bOpen) {
				gOpenEntry.reset();
			}
		}
	}
}

namespace DebugMenu_Internal 
{
	// Get the current zoom and scroll values
	static float zoomLevel = 1.0f;
	static ImVec2 scrollOffset(0.0f, 0.0f);

	bool Neq(float a, float b, float epsilon) {
		return std::abs(a - b) < epsilon;
	}

	struct NumberGrid
	{
	private:
		struct Filter
		{
			bool bActive = false;
			int filteredNumber = 0;

			void Show() {
				ImGui::Checkbox("Filter", &bActive);
				if (bActive) {
					ImGui::InputInt("Number", &filteredNumber);
				}
			}

			bool IsFiltered(int number) {
				return number == filteredNumber;
			}
		} filter;

		float customCellSize = 0.0f;

		const float panelSize = 640.0f;

	public:
		template<typename NumberType>
		void Show(const char* name, bool& bShow, const NumberType* numbers, const int width, const int height, bool bNewWindow = false)
		{
			if (bNewWindow) {
				ImGui::Begin(name, &bShow, ImGuiWindowFlags_AlwaysAutoResize);
			}

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

			//ImGui::SetWindowFontScale(0.5f);

			const ImGuiTableFlags flags = ImGuiTableFlags_Borders;
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
			if (ImGui::BeginTable("GridTable", width, flags, ImVec2((cellSize * width) + width, 0))) {
				for (int row = 0; row < height; ++row) {
					ImGui::TableNextRow(0);
					for (int col = 0; col < width; ++col) {
						ImGui::TableSetColumnIndex(col);
						const int number = numbers[col + (row * width)];

						char label[256];
						sprintf_s(label, 256, "%02X", number);

						if (filter.bActive) {
							if (filter.IsFiltered(number)) {
								ImGui::Button(label, ImVec2(cellSize, cellSize));
							}
							else {
								ImGui::Button("", ImVec2(cellSize, cellSize));
							}
						}
						else {
							ImGui::Button(label, ImVec2(cellSize, cellSize));
						}

						// Open the tooltip when hovering the cell
						if (ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("");
							ImGui::Text("Cell (%d, %d)\nPalette Index: %02X", col, row, number);
							ImGui::EndTooltip();
						}

						if (ImGui::IsItemClicked()) {
							filter.bActive = true;
							filter.filteredNumber = number;
						}

						if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
							filter.bActive = false;
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			filter.Show();

			if (bNewWindow) {
				ImGui::End();
			}
		}
	};

	struct ColorGrid
	{
	private:
		struct Filter
		{
			float tolerance = 0.001f;

			bool bActive = false;
			bool bFilterR = true;
			bool bFilterG = true;
			bool bFilterB = true;
			bool bFilterA = true;

			ImVec4 filterColor = { 0.0f, 0.0f, 0.0f, 0.0f };

			void Show() {
				ImGui::Checkbox("Filter", &bActive);

				static float tolerance = 0.001f;
				if (bActive) {
					ImGui::InputFloat("Tolerance", &tolerance);

					ImGui::InputFloat("R", &filterColor.x);
					ImGui::SameLine();
					ImGui::Checkbox("Filter R", &bFilterR);

					ImGui::InputFloat("G", &filterColor.y);
					ImGui::SameLine();
					ImGui::Checkbox("Filter G", &bFilterG);

					ImGui::InputFloat("B", &filterColor.z);
					ImGui::SameLine();
					ImGui::Checkbox("Filter B", &bFilterB);

					ImGui::InputFloat("A", &filterColor.w);
					ImGui::SameLine();
					ImGui::Checkbox("Filter A", &bFilterA);
				}
			}

			bool IsFiltered(ImVec4 colorRGBA) {
				return (Neq(colorRGBA.x, filterColor.x, tolerance) || !bFilterR) &&
					(Neq(colorRGBA.y, filterColor.y, tolerance) || !bFilterG) &&
					(Neq(colorRGBA.z, filterColor.z, tolerance) || !bFilterB) &&
					(Neq(colorRGBA.w, filterColor.w, tolerance) || !bFilterA);
			}
		} filter;
		
		float customCellSize = 0.0f;

		const float panelSize = 640.0f;

	public:
		template<typename IndexType = uint32_t, typename ColorType = uint32_t>
		void Show(const char* name, bool& bShow, const IndexType* pixelIndexes, const ColorType* paletteColors, const int width, const int height, bool bNewWindow = false)
		{
			if (bNewWindow) {
				ImGui::Begin(name, &bShow, ImGuiWindowFlags_AlwaysAutoResize);
			}

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
							if (filter.IsFiltered(colorRGBA)) {
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

						if (ImGui::IsItemClicked()) {
							filter.bActive = true;
							filter.filterColor = colorRGBA;
						}

						if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
							filter.bActive = false;
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			filter.Show();

			if (bNewWindow) {
				ImGui::End();
			}
		}
	};

	enum class EPaletteOption {
		PaletteTexture,
		RawPalette,
		FinalPalette,
		RawImageIndexes,
		ResolvedImageIndexes,
		RawImage,
		ResolvedImage,
		Max,
	};

	static void ShowPaletteWindow(DebugHelpers::DebugMaterial& debugMaterial, bool& bOpen)
	{
		const auto& imageData = debugMaterial.texture.image.imageData;
		const auto& paletteData = debugMaterial.texture.paletteImage.imageData;

		const int scale = 20;
		ImGui::Begin("Palette", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		PaletteSelector::Update();

		static int paletteOption = 0;

		static std::array<const char*, static_cast<int>(EPaletteOption::Max)> paletteOptions = {
			magic_enum::enum_name(EPaletteOption::PaletteTexture).data(),
			magic_enum::enum_name(EPaletteOption::RawPalette).data(),
			magic_enum::enum_name(EPaletteOption::FinalPalette).data(),
			magic_enum::enum_name(EPaletteOption::RawImageIndexes).data(),
			magic_enum::enum_name(EPaletteOption::ResolvedImageIndexes).data(),
			magic_enum::enum_name(EPaletteOption::RawImage).data(),
			magic_enum::enum_name(EPaletteOption::ResolvedImage).data(),
		};

		static ColorGrid paletteColorGrid;
		static NumberGrid imageGrid;

		ImGui::Text("Palette Type: ");
		ImGui::SameLine();
		ImGui::Combo("##PaletteType", &paletteOption, paletteOptions.data(), paletteOptions.size());

		std::vector<uint32_t> indexes;
		indexes.resize(paletteData.readWidth * paletteData.readHeight);
		for (int i = 0; i < paletteData.readWidth * paletteData.readHeight; i++) {
			indexes[i] = i;
		}

		switch (static_cast<EPaletteOption>(paletteOption))
		{
		case EPaletteOption::PaletteTexture:
		{
			ImVec2 paletteSize = { paletteData.readWidth * scale, paletteData.readHeight * scale };
			ImGui::Image(debugMaterial.paletteTexID, paletteSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));
		}
		break;
		case EPaletteOption::RawPalette:
		{
			paletteColorGrid.Show("Raw Palette", bOpen, indexes.data(), (uint32_t*)paletteData.pImage, paletteData.readWidth, paletteData.readHeight);
		}
			break;
		case EPaletteOption::FinalPalette:
		{
			paletteColorGrid.Show("Final Palette", bOpen, indexes.data(), (uint32_t*)debugMaterial.texture.paletteImage.readBuffer.data(), paletteData.readWidth, paletteData.readHeight);
		}
			break;
		case EPaletteOption::RawImageIndexes:
		{
			const auto& imageData = debugMaterial.texture.image.imageData;

			imageGrid.Show<uint8_t>("Texture Color Grid", bOpen, (uint8_t*)imageData.pImage, imageData.canvasWidth, imageData.canvasHeight);
		}
		break;
		case EPaletteOption::ResolvedImageIndexes:
		{
			const auto& debugData = debugMaterial.texture.debugData;
			const auto& imageData = debugMaterial.texture.image.imageData;
			imageGrid.Show("Texture Color Grid", bOpen, debugData.paletteIndexes.data(), imageData.canvasWidth, imageData.canvasHeight);
		}
		break;
		case EPaletteOption::RawImage:
		{
			const auto& debugData = debugMaterial.texture.debugData;
			const auto& imageData = debugMaterial.texture.image.imageData;

			paletteColorGrid.Show<uint8_t>("Texture Color Grid", bOpen, (uint8_t*)imageData.pImage, (uint32_t*)debugMaterial.texture.paletteImage.readBuffer.data(), imageData.canvasWidth, imageData.canvasHeight);
		}
		break;
		case EPaletteOption::ResolvedImage:
		{
			const auto& debugData = debugMaterial.texture.debugData;
			const auto& imageData = debugMaterial.texture.image.imageData;

			paletteColorGrid.Show("Texture Color Grid", bOpen, debugData.paletteIndexes.data(), (uint32_t*)debugMaterial.texture.paletteImage.readBuffer.data(), imageData.canvasWidth, imageData.canvasHeight);
		}
			break;
		default:
			break;
		}

		ImGui::End();
	}

	static void Show(DebugHelpers::DebugMaterial& debugMaterial, bool& bOpen) 
	{
		const auto& imageData = debugMaterial.texture.image.imageData;
		const auto& paletteData = debugMaterial.texture.paletteImage.imageData;

		ImGui::Begin("Previewer", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		ImVec2 imageSize(imageData.canvasWidth, imageData.canvasHeight); // Set the image size

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

		CallstackPreviewer::Show(debugMaterial.callstackEntry);

		ImGui::End();

		ShowPaletteWindow(debugMaterial, bOpen);
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
	else {
		PaletteSelector::Update(true);
	}
}

void MaterialPreviewer::Open(MaterialPreviewerEntry& entry)
{
	Reset();
	PaletteSelector::gOpenEntry = entry;

	const auto textureData = DebugMenu::LoadTextureData(entry.pMaterial);
	PaletteSelector::gPaletteMap = textureData.palettes;
}

void MaterialPreviewer::Open(ed_g2d_material* pMaterial)
{
	Reset();

	const auto textureData = DebugMenu::LoadTextureData(pMaterial);
	PaletteSelector::gPaletteMap = textureData.palettes;

	gOpenMaterial = DebugMaterialPtr(new DebugHelpers::DebugMaterial(pMaterial, textureData.palettes.begin()->first));
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
*/