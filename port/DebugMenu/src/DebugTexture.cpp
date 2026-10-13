#include "DebugMenu.h"
#include "DebugTexture.h"
#include "DebugSetting.h"
#include "DrawTrace.h"
#include <optional>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <map>

#include <imgui.h>

#include "Texture/TextureCache.h"
#include "Texture/TextureUpdate.h"
#include "Texture/TextureUpscale.h"

#include "Texture.h"
#include "ed3D.h"
#include "ed3D/ed3DG2D.h"
#include "ed3D/ed3DG3D.h"
#include "port.h"
#include "backends/imgui_impl_vulkan.h"
#include "DebugHelpers.h"

namespace Debug
{
	namespace Texture
	{
		static Debug::Setting<bool> gLoadUpscaleCache = { "Load Upscaled Textures From Cache", false };
		static Debug::Setting<int> gUpscaleModelSetting = { "Texture Upscale Model", 0 };
		static void LoadCachedTextures()
		{
			using namespace Renderer::Native;
			static const bool initializedModel = [] {
				SetTextureUpscaleModel(static_cast<TextureUpscaleModel>(std::clamp(gUpscaleModelSetting.get(), 0, 1)));
				return true;
			}();
			(void)initializedModel;
			using WeakState = std::weak_ptr<Renderer::TextureUpdateState>;
			static std::map<WeakState, bool, std::owner_less<WeakState>> attempted;
			static auto lastModel = GetTextureUpscaleModel();
			const auto model = GetTextureUpscaleModel();
			SetTextureUpscaleCacheLoading(gLoadUpscaleCache.get());
			if (!gLoadUpscaleCache.get() || model != lastModel) { attempted.clear(); lastModel = model; }
			if (!gLoadUpscaleCache.get() || !GetTextureUpscaleStatus(model).ready) return;
			for (auto it = attempted.begin(); it != attempted.end(); ) {
				if (it->first.expired()) it = attempted.erase(it); else ++it;
			}
			size_t queued = 0;
			Renderer::Kya::GetTextureLibrary().ForEach([&](const Renderer::Kya::G2D& file) {
				for (const auto& material : file.GetMaterials())
					for (const auto& layer : material.layers)
						for (const auto& entry : layer.textures) {
							if (queued >= 16 || !entry.pSimpleTexture) continue;
							const WeakState state = entry.pSimpleTexture->GetUpdateState();
							if (attempted.find(state) == attempted.end() && LoadTextureUpscaleFromCache(entry.pSimpleTexture.get())) {
								attempted.emplace(state, true); ++queued;
							}
						}
			});
		}
		static void DrawUpscaleControls()
		{
			using namespace Renderer::Native;
			const auto progress = GetTextureUpscaleProgress();
			int selected = static_cast<int>(GetTextureUpscaleModel());
			ImGui::BeginDisabled(progress.pending != 0);
			if (ImGui::Combo("Upscale model", &selected, "UltraSharp\0SPAN\0")) {
				SetTextureUpscaleModel(static_cast<TextureUpscaleModel>(selected));
				gUpscaleModelSetting = selected;
			}
			ImGui::EndDisabled();
			if (gLoadUpscaleCache.DrawImguiControl()) {
				gLoadUpscaleCache.UpdateValue();
				SetTextureUpscaleCacheLoading(gLoadUpscaleCache.get());
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Apply cached textures for the selected model as they load. Upscale also reuses cached results; misses run inference only when you click Upscale. Fresh results are always saved in bin/WIN/texture-upscale-cache. Revert stays in effect until you toggle this option or change model.");
			const auto status = GetTextureUpscaleStatus(GetTextureUpscaleModel());
			if (status.loading) ImGui::TextDisabled("Loading model...");
			else if (status.ready) ImGui::Text("4x upscale (%s)", status.backend.c_str());
			else ImGui::TextWrapped("Upscaler unavailable: %s", status.error.c_str());
			if (progress.pending || progress.completed || progress.failed || progress.cacheHits || progress.cacheMisses || progress.cacheErrors) {
				ImGui::Text("Pending: %zu  Completed: %zu  Failed: %zu", progress.pending, progress.completed, progress.failed);
				ImGui::Text("Cache hits: %zu  Misses: %zu  Errors: %zu", progress.cacheHits, progress.cacheMisses, progress.cacheErrors);
				ImGui::TextWrapped("%s", progress.lastResult.c_str());
			}
		}
		static const Renderer::Kya::G2D* gSelectedTexture = nullptr;

		static const ed_g2d_material* gSelectedMaterial = nullptr;

		static bool bOpenFirstMaterial = false;
		static int gSelectedLayer = 0, gSelectedTextureIndex = 0;
		static bool gFocusDraw = false;
		struct DrawSelection {
			Renderer::DrawTrace::Source source;
			Renderer::DrawTrace::Draw draw;
		};
		static std::optional<DrawSelection> gDrawSelection;

		// Opt-in running-game smoke check. Uses a texture from the loaded game
		// library, goes through the same APIs as the buttons, and leaves SPAN on.
		static void ValidateUpscaling()
		{
			using namespace Renderer::Native;
			static const char* reportPath = std::getenv("KYA_UPSCALE_VERIFY");
			if (!reportPath || !*reportPath) return;
			static int phase = 0;
			static Renderer::SimpleTexture* texture = nullptr;
			static std::weak_ptr<Renderer::TextureUpdateState> lifetime;
			static uint32_t width = 0, height = 0;
			static auto started = std::chrono::steady_clock::now();
			static auto deadline = started;
			auto report = [&](const std::string& message) { std::ofstream(reportPath, std::ios::app) << message << '\n'; };
			if (phase == 7) return;
			if (std::chrono::steady_clock::now() - started > std::chrono::minutes(3)) {
				report("FAIL: running-game upscaling validation timed out"); phase = 7; return;
			}
			if (phase == 0) {
				for (auto model : {TextureUpscaleModel::UltraSharp, TextureUpscaleModel::SPAN}) {
					const auto status = GetTextureUpscaleStatus(model);
					if (!status.ready) {
						if (!status.loading && !status.error.empty()) { report("FAIL: " + status.error); phase = 7; }
						return;
					}
				}
				Renderer::Kya::GetTextureLibrary().ForEach([&](const Renderer::Kya::G2D& file) {
					if (texture || file.GetName().find("KIM") == std::string::npos) return;
					for (const auto& material : file.GetMaterials())
						for (size_t layer = 0; layer < material.layers.size(); ++layer)
							for (const auto& entry : material.layers[layer].textures) {
								if (texture || !entry.pSimpleTexture || !entry.pSimpleTexture->GetRenderer()) continue;
								auto* renderer = entry.pSimpleTexture->GetRenderer();
								if (renderer->imageData.canvasWidth < 128 || renderer->imageData.canvasHeight < 128) continue;
								texture = entry.pSimpleTexture.get(); lifetime = texture->GetUpdateState();
								width = renderer->imageData.canvasWidth; height = renderer->imageData.canvasHeight;
								gSelectedTexture = &file; gSelectedMaterial = material.pMaterial; gSelectedLayer = static_cast<int>(layer);
							}
				});
				if (!texture) return;
				report("Game texture: " + texture->GetName() + " " + std::to_string(width) + "x" + std::to_string(height));
				SetTextureUpscaleModel(TextureUpscaleModel::UltraSharp); UpscaleTexture(texture); phase = 1;
				return;
			}
			if (lifetime.expired()) { report("FAIL: validation texture unloaded"); phase = 7; return; }
			const auto* renderer = texture->GetRenderer();
			if (phase == 1 || phase == 4 || phase == 6) {
				if (GetTextureUpscaleProgress().pending || renderer->width != width * 4 || renderer->height != height * 4) return;
				const auto status = GetTextureUpscaleStatus(GetTextureUpscaleModel());
				report(std::string(phase == 1 ? "UltraSharp" : "SPAN") + " PASS: live Vulkan image " + std::to_string(renderer->width) + "x" + std::to_string(renderer->height) + " (" + status.backend + ")");
				if (phase == 6) { report("PASS: both models and Revert verified in the running game"); phase = 7; return; }
				deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
				phase = phase == 1 ? 2 : 5;
			} else if (phase == 2 || phase == 5) {
				if (std::chrono::steady_clock::now() < deadline) return;
				RevertTexture(texture); phase = phase == 2 ? 3 : 8;
			} else if (phase == 3 || phase == 8) {
				if (renderer->width != width || renderer->height != height) return;
				report("Revert PASS: original dimensions restored");
				SetTextureUpscaleModel(TextureUpscaleModel::SPAN); UpscaleTexture(texture); phase = phase == 3 ? 4 : 6;
			}
		}
		struct ResolvedDrawTexture {
			const Renderer::Kya::G2D* file = nullptr;
			const Renderer::Kya::G2D::Material* material = nullptr;
			Renderer::SimpleTexture* texture = nullptr;
			int layer = 0, index = 0;
		};

		static ResolvedDrawTexture ResolveDrawTexture(const Renderer::DrawTrace::Source& source, const Renderer::DrawTrace::Draw& draw)
		{
			ResolvedDrawTexture result;
			if (!Renderer::DrawTrace::IsSourceLive(source) || draw.framebuffer || !draw.texture[0]) return result;
			int matches = 0;
			Renderer::Kya::GetTextureLibrary().ForEach([&](const Renderer::Kya::G2D& file) {
				for (const auto& material : file.GetMaterials()) {
					for (size_t layer = 0; layer < material.layers.size(); ++layer) {
						const auto& textures = material.layers[layer].textures;
						for (size_t index = 0; index < textures.size(); ++index) {
							auto* texture = textures[index].pSimpleTexture.get();
							if (texture && texture->GetName() == draw.texture.data() &&
								texture->GetMaterialIndex() == draw.material && texture->GetLayerIndex() == draw.layer) {
								result = { &file, &material, texture, static_cast<int>(layer), static_cast<int>(index) };
								++matches;
							}
						}
					}
				}
			});
			return matches == 1 ? result : ResolvedDrawTexture{};
		}

		Renderer::SimpleTexture* FindDrawTexture(const Renderer::DrawTrace::Source& source, const Renderer::DrawTrace::Draw& draw)
		{
			return ResolveDrawTexture(source, draw).texture;
		}

		bool OpenDraw(const Renderer::DrawTrace::Source& source, const Renderer::DrawTrace::Draw& draw)
		{
			const auto resolved = ResolveDrawTexture(source, draw);
			if (!resolved.texture) return false;
			gDrawSelection = DrawSelection{ source, draw };
			gSelectedTexture = resolved.file;
			gSelectedMaterial = resolved.material->pMaterial;
			gSelectedLayer = resolved.layer;
			gSelectedTextureIndex = resolved.index;
			bOpenFirstMaterial = false;
			gFocusDraw = true;
			return true;
		}

		struct RemovedTextureEntry
		{
			VkDescriptorSet textureDescriptorSet = VK_NULL_HANDLE;
			int lifetime = MAX_FRAMES_IN_FLIGHT + 5;
		};

		static std::vector<RemovedTextureEntry> gPendingRemovedTextures;

		enum class SortMode
		{
			Loaded,
			Path
		};

		static SortMode gSortMode = SortMode::Path;

		static void AddPendingRemovedTexture(const VkDescriptorSet& descriptorSet)
		{
			gPendingRemovedTextures.push_back({ descriptorSet, MAX_FRAMES_IN_FLIGHT + 1 });
		}

		static void ShowSortModeControls()
		{
			ImGui::SameLine(0.0f, 3.0f);

			// Drop down box for Sort Mode
			const char* sortModeItems[] = { "Loaded", "Path" };
			const char* currentSortMode = sortModeItems[static_cast<int>(gSortMode)];

			if (ImGui::BeginCombo("Sort Mode", currentSortMode)) {
				for (int i = 0; i < IM_ARRAYSIZE(sortModeItems); i++) {
					bool bIsSelected = (gSortMode == static_cast<SortMode>(i));
					if (ImGui::Selectable(sortModeItems[i], bIsSelected)) {
						gSortMode = static_cast<SortMode>(i);
					}

					if (bIsSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
		}

		static void ShowList(bool* bOpen)
		{
			ImGui::Begin("Texture List", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			static char nameFilterBuff[256] = { '\0' };
			ImGui::InputText("Filter", nameFilterBuff, 256);

			ShowSortModeControls();

			std::string nameFilter = nameFilterBuff;

			auto& textureLibrary = Renderer::Kya::GetTextureLibrary();

			static bool bOnlyInUse = false;
			ImGui::Checkbox("Only in use", &bOnlyInUse);

			// Create a scrollable list of textures.
			ImGui::BeginChild("List", ImVec2(600, 600), true);

			std::vector<const Renderer::Kya::G2D*> textureList;
			textureList.reserve(textureLibrary.GetTextureCount());

			textureLibrary.ForEach([&](const Renderer::Kya::G2D& texture) {
				bool bFound = true;

				if (bOnlyInUse) {
					bFound = false;

					for (auto& material : texture.GetMaterials()) {
						bFound |= material.GetInUse();
					}
				}

				const bool bPassFilter = nameFilter.empty() || texture.GetName().find(nameFilter) != std::string::npos;

				if (bFound && bPassFilter) {
					textureList.push_back(&texture);
				}
			});

			// Sort the texture list based on the selected sort mode.
			if (gSortMode == SortMode::Path) {
				std::sort(textureList.begin(), textureList.end(), [](const Renderer::Kya::G2D* a, const Renderer::Kya::G2D* b) {
					return a->GetName() < b->GetName();
				});
			}

			for (size_t i = 0; auto* pTexture : textureList) {
				if (pTexture) {
					const Renderer::Kya::G2D& texture = *pTexture;

					ImGui::PushID(static_cast<int>(i));

					if (ImGui::Selectable(texture.GetName().c_str())) {
						gDrawSelection.reset();
						gSelectedLayer = gSelectedTextureIndex = 0;
						gSelectedTexture = &texture;
						gSelectedMaterial = nullptr;
						bOpenFirstMaterial = true;
					}

					if (texture.GetMaterials().size() > 0 && texture.GetMaterials().front().layers.size() > 0 && texture.GetMaterials().front().layers.front().textures.size() > 0) {
						ImGui::SameLine();

						auto bm = texture.GetMaterials().front().layers.front().textures.front().bitmap.GetBitmap();
						ImGui::Text("(%d, %d) psm: %d", bm->width, bm->height, bm->psm);
					}

					ImGui::PopID();
				}

				i++;
			}

			ImGui::EndChild();

			ImGui::End();
		}

		static int ListGifTagDetails(edpkt_data* aPkt)
		{
			// TAG
			Gif_Tag gifTag;
			gifTag.setTag((u8*)aPkt, false);

			ImGui::Text("GIF TAG");
			ImGui::Text("[mode=%s][pre=%d][prim=%d][nregs=%d][nloop=%d][qwc=%d][EOP=%d]\n",
				GifTag_ModeStr[gifTag.tag.FLG], gifTag.tag.PRE, gifTag.tag.PRIM,
				gifTag.nRegs, gifTag.nLoop, gifTag.len / 16, gifTag.tag.EOP);


			ImGui::Spacing();
			ImGui::Spacing();

			aPkt++;
	
			float textColorRed = 1.0f;

			for (int i = 0; i < gifTag.nLoop; i++) {
				edpkt_data pkt = *aPkt;

				const ImVec4 textColor = ImVec4(textColorRed, 1.0f, 1.0f, 1.0f);

				switch (pkt.asU32[2]) {
				case SCE_GS_TEX0_1:
				{
					const GIFReg::GSTex tex = *reinterpret_cast<GIFReg::GSTex*>(&pkt.cmdA);
					ImGui::TextColored(textColor, "TEX0_1");
					ImGui::TextColored(textColor, "CBP: 0x%x CLD: %d CPSM: %d CSA: %d CSM: %d",
						tex.CBP, tex.CLD, tex.CPSM, tex.CSA, tex.CSM);
					ImGui::TextColored(textColor, "PSM: %d TBP0: 0x%x TBW: %d TCC: %d TFX: %d TW: %d (%d) TH: %d (%d)",
						tex.PSM, tex.TBP0, tex.TBW, tex.TCC, tex.TFX, tex.TW, 1 << tex.TW, tex.TH, 1 << tex.TH);
				}
				break;
				case SCE_GS_ALPHA_1:
				{
					const GIFReg::GSAlpha alpha = *reinterpret_cast<GIFReg::GSAlpha*>(&pkt.cmdA);
					ImGui::TextColored(textColor, "ALPHA_1");
					ImGui::TextColored(textColor, "A: %d B: %d C: %d D: %d FIX: %d",
						alpha.A, alpha.B, alpha.C, alpha.D, alpha.FIX);
				}
				break;
				case SCE_GS_COLCLAMP:
				{
					const GIFReg::GSColClamp colClamp = *reinterpret_cast<GIFReg::GSColClamp*>(&pkt.cmdA);
					ImGui::TextColored(textColor, "COLCLAMP");
					ImGui::TextColored(textColor, "CLAMP: %d",
						colClamp.CLAMP);
				}
				break;
				case SCE_GS_TEST_1:
				{
					const GIFReg::GSTest test = *reinterpret_cast<GIFReg::GSTest*>(&pkt.cmdA);
					ImGui::TextColored(textColor, "TEST_1");
					ImGui::TextColored(textColor, "ATE: %d ATST: %d AFAIL: %d DATE: %d DATM: %d ZTE: %d ZTST: %d",
						test.ATE, test.ATST, test.AFAIL, test.DATE, test.DATM, test.ZTE, test.ZTST);
				}
				break;
				case SCE_GS_TEX1_1:
				{
					ImGui::TextColored(textColor, "TEX1_1");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands TEX1: {:x} ({:x})", pkt.cmdA, pkt.cmdB);

				}
				break;
				case SCE_GS_MIPTBP1_1:
				{
					ImGui::TextColored(textColor, "MIPTBP1_1");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands MIPTBP1: {:x} ({:x})", pkt.cmdA, pkt.cmdB);

				}
				break;
				case SCE_GS_MIPTBP2_1:
				{
					ImGui::TextColored(textColor, "MIPTBP2_1");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands MIPTBP2: {:x} ({:x})", pkt.cmdA, pkt.cmdB);

				}
				break;
				case SCE_GS_CLAMP_1:
				{
					const GIFReg::GSClamp clamp = *reinterpret_cast<GIFReg::GSClamp*>(&pkt.cmdA);
					ImGui::TextColored(textColor, "CLAMP_1");
					ImGui::TextColored(textColor, "WMS: %d WMT: %d MINU: %d MAXU: %d MAXV: %d MINV: %d",
						clamp.WMS, clamp.WMT, clamp.MINU, clamp.MAXU, clamp.MAXV, clamp.MINV);
				}
				break;
				case SCE_GS_BITBLTBUF:
				{
					GIFReg::GSBitBltBuf bitbltbuf = *reinterpret_cast<GIFReg::GSBitBltBuf*>(&pkt.cmdA);
					ImGui::TextColored(textColor, "SCE_GS_BITBLTBUF");
					ImGui::TextColored(textColor, "DBP: 0x%x DBW: %d DPSM: %d SBP: 0x%x SBW: %d SPSM: %d",
						bitbltbuf.DBP, bitbltbuf.DBW, bitbltbuf.DPSM, bitbltbuf.SBP, bitbltbuf.SBW, bitbltbuf.SPSM);
				}
				break;
				case SCE_GS_TRXPOS:
				{
					ImGui::TextColored(textColor, "SCE_GS_TRXPOS X: %d Y: %d", pkt.asU32[0], pkt.asU32[1]);
				}
				break;
				case SCE_GS_TRXREG:
				{
					ImGui::TextColored(textColor, "SCE_GS_TRXREG W: %d H: %d", pkt.asU32[0], pkt.asU32[1]);
				}
				break;
				case SCE_GS_TRXDIR:
				{
					ImGui::TextColored(textColor, "SCE_GS_TRXDIR W: %d H: %d", pkt.asU32[0], pkt.asU32[1]);
				}
				break;
				case SCE_GS_TEXFLUSH:
				{
					ImGui::TextColored(textColor, "SCE_GS_TEXFLUSH");
				}
				break;
				default:
				{
					ImGui::TextColored(textColor, "UNKNOWN");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands UNKNOWN ({:x}): {:x} ({:x})", pkt.asU32[2], pkt.cmdA, pkt.cmdB);
				}
				break;
				}

				aPkt++;

				textColorRed = (textColorRed == 1.0f) ? 0.0f : 1.0f;
			}

			return gifTag.nLoop;
		}

		static void ShowTextureCommandDetails(edpkt_data* aPkt, int size)
		{
			for (int i = 0; i < size; i++) {
				edpkt_data& pkt = aPkt[i];

				if (pkt.asU32[2] == 0xe) {
					i += ListGifTagDetails(aPkt + i);
				}
				else {
					const int cmd = (pkt.asU32[0] >> 24);

					switch (cmd)
					{
						case 0x10:
						{
							ImGui::Text("TO GIF");
						}
						break;
						case 0x30:
						{
							ImGui::Text("IMAGE");
							DebugHelpers::TextValidValue("Image: %p", LOAD_POINTER(pkt.asU32[1]));
						}
						break;
						case 0x50:
						{
							ImGui::Text("OTHER");
							DebugHelpers::TextValidValue("Other: %p", LOAD_POINTER(pkt.asU32[1]));
						}
						break;
						case 0x70:
						{
							ImGui::Text("70");
						}
						break;
						default:
							ImGui::Text("UNKNOWN");
						break;
					}
				}
			}
		}

		static void ListBitmapDetails(ed_g2d_bitmap* pBitmap)
		{
			ImGui::Text("Bitmap Details");
			ImGui::Text("Width: %d", pBitmap->width);
			ImGui::Text("Height: %d", pBitmap->height);
			ImGui::Text("PSM: %d", pBitmap->psm);
			ImGui::Text("Max Mip Level: %d", pBitmap->maxMipLevel);
			DebugHelpers::TextValidValue("PSX2: %p", LOAD_POINTER(pBitmap->pPSX2));

			if (pBitmap->pPSX2 && ImGui::CollapsingHeader("PSX2")) {
				// We have two header structures (double buffered).

				edPSX2Header* pHeader = LOAD_POINTER_CAST(edPSX2Header*, pBitmap->pPSX2);

				ImGui::Text("[0]");
				DebugHelpers::TextValidValue("List: %p", LOAD_POINTER(pHeader[0].pPkt));
				ImGui::Text("size: %d (0x%x)", pHeader[0].size, pHeader[0].size);

				if (pHeader[0].pPkt && ImGui::CollapsingHeader("[0] Command Buffer Contents")) {
					ShowTextureCommandDetails(LOAD_POINTER_CAST(edpkt_data*, pHeader[0].pPkt), pHeader[0].size);
				}

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Text("[1]");
				DebugHelpers::TextValidValue("List: %p", LOAD_POINTER(pHeader[1].pPkt));
				ImGui::Text("size: %d (0x%x)", pHeader[1].size, pHeader[1].size);

				if (pHeader[1].pPkt && ImGui::CollapsingHeader("[1] Command Buffer Contents")) {
					ShowTextureCommandDetails(LOAD_POINTER_CAST(edpkt_data*, pHeader[1].pPkt), pHeader[1].size);
				}
			}
		}

		static void ShowMaterialDetails()
		{
			bool bOpen = true;
			ImGui::Begin("Material Details", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			ImGui::Text("Count: %d", gSelectedMaterial->nbLayers);
			ImGui::Text("Flags: 0x%x", gSelectedMaterial->flags);

			ImGui::Spacing();
			ImGui::Spacing();

			DebugHelpers::TextValidValue("DMA Material: %p", LOAD_POINTER(gSelectedMaterial->pDMA_Material));
			DebugHelpers::TextValidValue("Command Buffer Texture: %p", LOAD_POINTER(gSelectedMaterial->pCommandBufferTexture));

			ImGui::Text("Command Buffer Texture Size: %d", gSelectedMaterial->commandBufferTextureSize);

			if ((gSelectedMaterial->pCommandBufferTexture && gSelectedMaterial->commandBufferTextureSize > 0) && ImGui::CollapsingHeader("Command Buffer Contents")) {
				ShowTextureCommandDetails(LOAD_POINTER_CAST(edpkt_data*, gSelectedMaterial->pCommandBufferTexture), gSelectedMaterial->commandBufferTextureSize);
			}

			ImGui::Spacing();
			ImGui::Spacing();

			for (int i = 0; i < gSelectedMaterial->nbLayers; ++i) {
				char buffer[256];
				sprintf_s(buffer, "Layer %d", i);
				if (gFocusDraw) ImGui::SetNextItemOpen(i == gSelectedLayer);
				if (ImGui::CollapsingHeader(buffer)) {
					ed_Chunck* pLAY = LOAD_POINTER_CAST(ed_Chunck*, gSelectedMaterial->aLayers[i]);

					DebugHelpers::ListChunckDetails(pLAY);

					ImGui::Spacing();
					ImGui::Spacing();

					ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);

					ImGui::Text("Layer Details");
					ImGui::Text("Flags 0x0: 0x%x", pLayer->flags_0x0);
					ImGui::Text("Flags 0x4: 0x%x", pLayer->flags_0x4);
					ImGui::Text("Field 0x1b: %d", pLayer->field_0x1b);
					ImGui::Text("bHasTexture: %d", pLayer->bHasTexture);
					ImGui::Text("Palette ID: %d", pLayer->paletteId);
					DebugHelpers::TextValidValue("TEX: %p", LOAD_POINTER(pLayer->pTex));

					ImGui::Spacing();
					ImGui::Spacing();

					if (pLayer->bHasTexture && ImGui::CollapsingHeader("Texture")) {
						ed_Chunck* pTEX = LOAD_POINTER_CAST(ed_Chunck*, pLayer->pTex);
						DebugHelpers::ListChunckDetails(pTEX);

						ImGui::Spacing();
						ImGui::Spacing();

						ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);

						ImGui::Text("Texture Details");
						ImGui::Text("Hash Code: %s", pTexture->hashCode.hash.ToString().c_str());
						ImGui::Text("bHasPalette: %d", pTexture->bHasPalette);
						ImGui::Text("Field 0x14: %f", pTexture->pAnimSpeedNormalExtruder);

						ImGui::Spacing();
						ImGui::Spacing();

						ed_hash_code* pBitmapHashCode = LOAD_POINTER_CAST(ed_hash_code*, pTexture->hashCode.pData);
						if (pBitmapHashCode != (ed_hash_code*)0x0) {
							if (ImGui::CollapsingHeader("Bitmap")) {
								ImGui::Text("Bitmap Details");
								ImGui::Text("Bitmap Hash Code: %s", pBitmapHashCode->hash.ToString().c_str());

								ImGui::Spacing();
								ImGui::Spacing();

								ed_Chunck* pT2D = LOAD_POINTER_CAST(ed_Chunck*, pBitmapHashCode->pData);

								DebugHelpers::ListChunckDetails(pT2D);

								ImGui::Spacing();
								ImGui::Spacing();

								ed_g2d_bitmap* pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
								ListBitmapDetails(pBitmap);
							}
						}

						ImGui::Spacing();
						ImGui::Spacing();

						if (pTexture->bHasPalette && ImGui::CollapsingHeader("Palette")) {
							ed_hash_code* pPaletteHashCodes = reinterpret_cast<ed_hash_code*>(pTexture + 1);
							ed_hash_code* pPaletteHashCode = LOAD_POINTER_CAST(ed_hash_code*, pPaletteHashCodes[pLayer->paletteId].pData);
							ed_Chunck* pT2D = LOAD_POINTER_CAST(ed_Chunck*, pPaletteHashCode->pData);

							DebugHelpers::ListChunckDetails(pT2D);

							ImGui::Spacing();
							ImGui::Spacing();

							ed_g2d_bitmap* pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);

							ListBitmapDetails(pBitmap);
						}
					}
				}
			}

			ImGui::End();

			if (gFocusDraw) ImGui::SetNextWindowFocus();
			ImGui::Begin("Preview", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);
			gFocusDraw = false;

			auto& textureLibrary = Renderer::Kya::GetTextureLibrary();
			const auto& texture = textureLibrary.FindMaterial(gSelectedMaterial);

			Renderer::SimpleTexture* pSimpleTexture = nullptr;
			if (texture && !texture->layers.empty()) {
				gSelectedLayer = std::clamp(gSelectedLayer, 0, static_cast<int>(texture->layers.size()) - 1);
				if (ImGui::SliderInt("Layer", &gSelectedLayer, 0, static_cast<int>(texture->layers.size()) - 1)) gSelectedTextureIndex = 0;
				const auto& textures = texture->layers[gSelectedLayer].textures;
				if (!textures.empty()) {
					gSelectedTextureIndex = std::clamp(gSelectedTextureIndex, 0, static_cast<int>(textures.size()) - 1);
					if (textures.size() > 1) ImGui::SliderInt("Texture", &gSelectedTextureIndex, 0, static_cast<int>(textures.size()) - 1);
					pSimpleTexture = textures[gSelectedTextureIndex].pSimpleTexture.get();
				}
			}

			if (pSimpleTexture && pSimpleTexture->GetRenderer()) {
				ImGui::TextUnformatted(pSimpleTexture->GetName().c_str());
				static PS2::GSSimpleTexture* pRenderer = nullptr;
				static VkImageView lastImageView = VK_NULL_HANDLE;
				static VkDescriptorSet textureId;

				static bool bLinearSampler = false;

				if (ImGui::Button("Whiteify")) {
					Renderer::Native::TurnTextureWhite(pSimpleTexture);
				}

				ImGui::SameLine();

				if (ImGui::Button("Revert")) {
					Renderer::Native::RevertTexture(pSimpleTexture);
				}

				ImGui::SameLine();

				ImGui::BeginDisabled(!Renderer::Native::IsUpscalerReady());
				if (ImGui::Button("Upscale")) {
					Renderer::Native::UpscaleTexture(pSimpleTexture);
				}
				ImGui::EndDisabled();
				DrawUpscaleControls();


				PS2::GSSimpleTexture* pCurrentRenderer = pSimpleTexture->GetRenderer();
				if (ImGui::Checkbox("Linear", &bLinearSampler) || pRenderer != pCurrentRenderer || lastImageView != pCurrentRenderer->imageView) {
					if (pRenderer != nullptr) {
						AddPendingRemovedTexture(textureId);
					}

					pRenderer = pCurrentRenderer;
					lastImageView = pRenderer->imageView;

					const VkSampler sampler = PS2::GetSampler(PS2::GetPreviewSamplerDescription(bLinearSampler));

					textureId = ImGui_ImplVulkan_AddTexture(sampler, pRenderer->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}
				
				//ImGui::Image(textureId, ImVec2(pRenderer->width, pRenderer->height));
				float aspectRatio = static_cast<float>(pRenderer->width) / static_cast<float>(pRenderer->height);
				ImGui::Image((ImTextureID)(uintptr_t)textureId, ImVec2(256.0f, 256.0f / aspectRatio));
			}

			ImGui::End();

			if (!bOpen) {
				gSelectedMaterial = nullptr;
				gDrawSelection.reset();
			}
		}

		void ShowMaterialDetails(const ed_g2d_material* pMaterial)
		{
			gDrawSelection.reset();
			gSelectedLayer = gSelectedTextureIndex = 0;
			gSelectedMaterial = pMaterial;

			// Try find the texture related to the material.
			auto& textureLibrary = Renderer::Kya::GetTextureLibrary();
			textureLibrary.ForEach([&](const Renderer::Kya::G2D& texture) {
				auto* pManager = texture.GetManager();

				if (pManager->pMATA_HASH) {
					const int nbMaterials = ed3DG2DGetG2DNbMaterials(pManager->pMATA_HASH);

					for (int i = 0; i < nbMaterials; ++i) {
						if (ed_g2d_material* pTextureMaterial = ed3DG2DGetG2DMaterialFromIndex(pManager, i); pMaterial == pTextureMaterial) {
							gSelectedTexture = &texture;
						}
					}
				}
			});
		}

		static void ShowTextureDetails()
		{
			bool bOpen = true;
			ImGui::Begin("Texture Details", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			ImGui::Text("Name: %s", gSelectedTexture->GetName().c_str());

			ImGui::Spacing();
			ImGui::Spacing();

			auto* pManager = gSelectedTexture->GetManager();

			if (pManager) {
				DebugHelpers::TextValidValue("File Buffer: %p", pManager->pFileBuffer);
				DebugHelpers::TextValidValue("*2D* (Texture Chunk): %p", pManager->pTextureChunk);
				DebugHelpers::TextValidValue("MATA (Hash): %p", pManager->pMATA_HASH);
				DebugHelpers::TextValidValue("T2DA: %p", pManager->pT2DA);
				DebugHelpers::TextValidValue("PALL: %p", pManager->pPALL);
				DebugHelpers::TextValidValue("ANMA: %p", pManager->pANMA);

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Text("Field 0x18: %d", pManager->field_0x18);
				ImGui::Text("Field 0x19: %d", pManager->field_0x19);
				ImGui::Text("Field 0x1a: %d", pManager->field_0x1a);
				ImGui::Text("Field 0x1b: %d", pManager->field_0x1b);

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Text("Length A %d (0x%x)", pManager->textureFileLengthA, pManager->textureFileLengthA);
				ImGui::Text("Length B %d (0x%x)", pManager->textureFileLengthB, pManager->textureFileLengthB);

				ImGui::Spacing();
				ImGui::Spacing();

				// Take the MATA hash and get the nb of materials.
				if (pManager->pMATA_HASH) {
					const int nbMaterials = ed3DG2DGetG2DNbMaterials(pManager->pMATA_HASH);

					ImGui::Text("Nb Materials: %d", nbMaterials);

					ImGui::Spacing();
					ImGui::Spacing();

					const ed_hash_code* pHashCodes = reinterpret_cast<ed_hash_code*>(pManager->pMATA_HASH + 1);

					if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
						for (int i = 0; i < nbMaterials; ++i) {
							const ed_hash_code* pHashCode = pHashCodes + i;

							char buff[256];
							sprintf_s(buff, 256, "%d - %s", i, pHashCode->hash.ToString().c_str());

							if (ImGui::Selectable(buff) || bOpenFirstMaterial) {
								gDrawSelection.reset();
								gSelectedLayer = gSelectedTextureIndex = 0;
								gSelectedMaterial = ed3DG2DGetG2DMaterialFromIndex(pManager, i);
								bOpenFirstMaterial = false;
							}

							if (gSelectedTexture->GetMaterials()[i].GetInUse()) {
								ImGui::SameLine();
								ImGui::TextColored(DebugHelpers::sValidColor, "(In Use)");
							}

							for (int j = 0; j < edDlistConfig.bankMaterialCount; j++) {
								if (gBankMaterial[j].hash.number == pHashCode->hash.number) {
									ImGui::SameLine();
									ImGui::TextColored(DebugHelpers::sValidColor, "gBankMaterial ID: 0x%x", j);
								}
							}
						}
					}
				}
			}

			ImGui::End();

			if (!bOpen) {
				gSelectedTexture = nullptr;
				gSelectedMaterial = nullptr;
			}
		}
	}

	static bool gShowTextureList = false;
}

void Debug::Texture::ShowMenu(bool* bOpen)
{
	ImGui::Begin("Texture", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

	auto& textureLibrary = Renderer::Kya::GetTextureLibrary();
	ImGui::Text("Count: %d", textureLibrary.GetTextureCount());
	DrawUpscaleControls();

	ImGui::Spacing();
	ImGui::Spacing();

	if (ImGui::Button("Toggle Texture List")) {
		gShowTextureList = !gShowTextureList;
	}

	ImGui::SameLine();

	const bool bUpscalerReady = Renderer::Native::IsUpscalerReady();
	if (!bUpscalerReady) {
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Upscale All")) {
		textureLibrary.ForEach([](const Renderer::Kya::G2D& texture) {
			for (const auto& material : texture.GetMaterials()) {
				for (const auto& layer : material.layers) {
					for (const auto& texEntry : layer.textures) {
						if (texEntry.pSimpleTexture) {
							Renderer::Native::UpscaleTexture(texEntry.pSimpleTexture.get());
						}
					}
				}
			}
		});
	}

	if (!bUpscalerReady) {
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::TextDisabled("(upscaler not ready)");
	}

	ImGui::End();

	if (gShowTextureList) {
		ShowList(&gShowTextureList);
	}
}

void Debug::Texture::Update()
{
	LoadCachedTextures();
	ValidateUpscaling();
	if (gDrawSelection) {
		const auto resolved = ResolveDrawTexture(gDrawSelection->source, gDrawSelection->draw);
		gSelectedTexture = resolved.file;
		gSelectedMaterial = resolved.material ? resolved.material->pMaterial : nullptr;
		if (!resolved.texture) gDrawSelection.reset();
	}
	if (gSelectedTexture) {
		ShowTextureDetails();
	}

	if (gSelectedMaterial) {
		ShowMaterialDetails();
	}

	for (auto& entry : gPendingRemovedTextures) {
		entry.lifetime--;
		if (entry.lifetime <= 0) {
			ImGui_ImplVulkan_RemoveTexture(entry.textureDescriptorSet);
		}
	}

	// Remove entries that have expired.
	gPendingRemovedTextures.erase(std::remove_if(gPendingRemovedTextures.begin(), gPendingRemovedTextures.end(),
		[](const RemovedTextureEntry& entry) { return entry.lifetime <= 0; }), gPendingRemovedTextures.end());
}

namespace Debug {
    MenuRegisterer sDebugTextureMenuReg("Texture", Debug::Texture::ShowMenu);
    UpdateRegisterer sDebugTextureUpdateReg(Debug::Texture::Update);
}

