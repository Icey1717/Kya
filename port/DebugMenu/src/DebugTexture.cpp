#include "DebugTexture.h"

#include <imgui.h>

#include "TextureCache.h"

#include "Texture.h"
#include "ed3D.h"
#include "port.h"
#include "backends/imgui_impl_vulkan.h"
#include "DebugHelpers.h"

namespace Debug
{
	namespace Texture
	{
		static const Renderer::Kya::G2D* gSelectedTexture = nullptr;

		static const ed_g2d_material* gSelectedMaterial = nullptr;

		static bool bOpenFirstMaterial = false;

		static void ShowList(bool* bOpen)
		{
			ImGui::Begin("Texture List", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			static char nameFilterBuff[256] = { '\0' };
			ImGui::InputText("Filter", nameFilterBuff, 256);

			std::string nameFilter = nameFilterBuff;

			auto& textureLibrary = Renderer::Kya::GetTextureLibrary();

			static bool bOnlyInUse = false;
			ImGui::Checkbox("Only in use", &bOnlyInUse);

			// Create a scrollable list of textures.
			ImGui::BeginChild("List", ImVec2(600, 600), true);

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
					if (ImGui::Selectable(texture.GetName().c_str())) {
						gSelectedTexture = &texture;
						gSelectedMaterial = nullptr;
						bOpenFirstMaterial = true;
					}

					if (texture.GetMaterials().size() > 0 && texture.GetMaterials().front().layers.size() > 0 && texture.GetMaterials().front().layers.front().textures.size() > 0) {
						ImGui::SameLine();

						auto bm = texture.GetMaterials().front().layers.front().textures.front().bitmap.GetBitmap();
						ImGui::Text("(%d, %d) psm: %d", bm->width, bm->height, bm->psm);
					}
				}
			});

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

			for (int i = 0; i < gifTag.nLoop; i++) {
				edpkt_data pkt = *aPkt;

				switch (pkt.asU32[2]) {
				case SCE_GS_TEX0_1:
				{
					GIFReg::GSTex tex = *reinterpret_cast<GIFReg::GSTex*>(&pkt.cmdA);
					ImGui::Text("TEX0_1");
					ImGui::Text("CBP: 0x%x CLD: %d CPSM: %d CSA: %d CSM: %d",
						tex.CBP, tex.CLD, tex.CPSM, tex.CSA, tex.CSM);
					ImGui::Text("PSM: %d TBP0: 0x%x TBW: %d TCC: %d TFX: %d TW: %d (%d) TH: %d (%d)",
						tex.PSM, tex.TBP0, tex.TBW, tex.TCC, tex.TFX, tex.TW, 1 << tex.TW, tex.TH, 1 << tex.TH);

					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands TEX0: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
					//SendTextureCommandsFromPacked(pkt.cmdA);
				}
				break;
				case SCE_GS_ALPHA_1:
				{
					ImGui::Text("ALPHA_1");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands ALPHA: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
					//GIFReg::GSAlpha alpha = *reinterpret_cast<GIFReg::GSAlpha*>(&pkt.cmdA);
					//Renderer::SetAlpha(alpha.A, alpha.B, alpha.C, alpha.D, alpha.FIX);
				}
				break;
				case SCE_GS_COLCLAMP:
				{
					ImGui::Text("COLCLAMP");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands COLCLAMP: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
					//GIFReg::GSColClamp colClamp = *reinterpret_cast<GIFReg::GSColClamp*>(&pkt.cmdA);
					//Renderer::SetColClamp(colClamp);
				}
				break;
				case SCE_GS_TEST_1:
				{
					const GIFReg::GSTest test = *reinterpret_cast<GIFReg::GSTest*>(&pkt.cmdA);
					ImGui::Text("TEST_1");
					ImGui::Text("ATE: %d ATST: %d AFAIL: %d DATE: %d DATM: %d ZTE: %d ZTST: %d",
						test.ATE, test.ATST, test.AFAIL, test.DATE, test.DATM, test.ZTE, test.ZTST);
				}
				break;
				case SCE_GS_TEX1_1:
				{
					ImGui::Text("TEX1_1");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands TEX1: {:x} ({:x})", pkt.cmdA, pkt.cmdB);

				}
				break;
				case SCE_GS_MIPTBP1_1:
				{
					ImGui::Text("MIPTBP1_1");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands MIPTBP1: {:x} ({:x})", pkt.cmdA, pkt.cmdB);

				}
				break;
				case SCE_GS_MIPTBP2_1:
				{
					ImGui::Text("MIPTBP2_1");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands MIPTBP2: {:x} ({:x})", pkt.cmdA, pkt.cmdB);

				}
				break;
				case SCE_GS_CLAMP_1:
				{
					ImGui::Text("CLAMP_1");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands CLAMP: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
					//const GIFReg::GSClamp clamp = *reinterpret_cast<GIFReg::GSClamp*>(&pkt.cmdA);
					//Renderer::SetClamp(clamp);
				}
				break;
				case SCE_GS_BITBLTBUF:
				{
					GIFReg::GSBitBltBuf bitbltbuf = *reinterpret_cast<GIFReg::GSBitBltBuf*>(&pkt.cmdA);
					ImGui::Text("SCE_GS_BITBLTBUF");
					ImGui::Text("DBP: 0x%x DBW: %d DPSM: %d SBP: 0x%x SBW: %d SPSM: %d",
						bitbltbuf.DBP, bitbltbuf.DBW, bitbltbuf.DPSM, bitbltbuf.SBP, bitbltbuf.SBW, bitbltbuf.SPSM);
				}
				break;
				case SCE_GS_TRXPOS:
				{
					ImGui::Text("SCE_GS_TRXPOS X: %d Y: %d", pkt.asU32[0], pkt.asU32[1]);
				}
				break;
				case SCE_GS_TRXREG:
				{
					ImGui::Text("SCE_GS_TRXREG W: %d H: %d", pkt.asU32[0], pkt.asU32[1]);
				}
				break;
				case SCE_GS_TRXDIR:
				{
					ImGui::Text("SCE_GS_TRXDIR W: %d H: %d", pkt.asU32[0], pkt.asU32[1]);
				}
				break;
				case SCE_GS_TEXFLUSH:
				{
					ImGui::Text("SCE_GS_TEXFLUSH");
				}
				break;
				default:
				{
					ImGui::Text("UNKNOWN");
					//ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands UNKNOWN ({:x}): {:x} ({:x})", pkt.asU32[2], pkt.cmdA, pkt.cmdB);
				}
				break;
				}

				aPkt++;
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
							DebugHelpers::TextValidValue("Image: %p", LOAD_SECTION(pkt.asU32[1]));
						}
						break;
						case 0x50:
						{
							ImGui::Text("OTHER");
							DebugHelpers::TextValidValue("Other: %p", LOAD_SECTION(pkt.asU32[1]));
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
			DebugHelpers::TextValidValue("PSX2: %p", LOAD_SECTION(pBitmap->pPSX2));

			if (pBitmap->pPSX2 && ImGui::CollapsingHeader("PSX2")) {
				// We have two header structures (double buffered).

				edPSX2Header* pHeader = LOAD_SECTION_CAST(edPSX2Header*, pBitmap->pPSX2);

				ImGui::Text("[0]");
				DebugHelpers::TextValidValue("List: %p", LOAD_SECTION(pHeader[0].pPkt));
				ImGui::Text("size: %d (0x%x)", pHeader[0].size, pHeader[0].size);

				if (pHeader[0].pPkt && ImGui::CollapsingHeader("[0] Command Buffer Contents")) {
					ShowTextureCommandDetails(LOAD_SECTION_CAST(edpkt_data*, pHeader[0].pPkt), pHeader[0].size);
				}

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Text("[1]");
				DebugHelpers::TextValidValue("List: %p", LOAD_SECTION(pHeader[1].pPkt));
				ImGui::Text("size: %d (0x%x)", pHeader[1].size, pHeader[1].size);

				if (pHeader[1].pPkt && ImGui::CollapsingHeader("[1] Command Buffer Contents")) {
					ShowTextureCommandDetails(LOAD_SECTION_CAST(edpkt_data*, pHeader[1].pPkt), pHeader[1].size);
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

			DebugHelpers::TextValidValue("DMA Material: %p", LOAD_SECTION(gSelectedMaterial->pDMA_Material));
			DebugHelpers::TextValidValue("Command Buffer Texture: %p", LOAD_SECTION(gSelectedMaterial->pCommandBufferTexture));

			ImGui::Text("Command Buffer Texture Size: %d", gSelectedMaterial->commandBufferTextureSize);

			if ((gSelectedMaterial->pCommandBufferTexture && gSelectedMaterial->commandBufferTextureSize > 0) && ImGui::CollapsingHeader("Command Buffer Contents")) {
				ShowTextureCommandDetails(LOAD_SECTION_CAST(edpkt_data*, gSelectedMaterial->pCommandBufferTexture), gSelectedMaterial->commandBufferTextureSize);
			}

			ImGui::Spacing();
			ImGui::Spacing();

			for (int i = 0; i < gSelectedMaterial->nbLayers; ++i) {
				char buffer[256];
				sprintf_s(buffer, "Layer %d", i);
				if (ImGui::CollapsingHeader(buffer)) {
					ed_Chunck* pLAY = LOAD_SECTION_CAST(ed_Chunck*, gSelectedMaterial->aLayers[i]);

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
					DebugHelpers::TextValidValue("TEX: %p", LOAD_SECTION(pLayer->pTex));

					ImGui::Spacing();
					ImGui::Spacing();

					if (pLayer->bHasTexture && ImGui::CollapsingHeader("Texture")) {
						ed_Chunck* pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
						DebugHelpers::ListChunckDetails(pTEX);

						ImGui::Spacing();
						ImGui::Spacing();

						ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);

						ImGui::Text("Texture Details");
						ImGui::Text("Hash Code: %s", pTexture->hashCode.hash.ToString().c_str());
						ImGui::Text("bHasPalette: %d", pTexture->bHasPalette);
						ImGui::Text("Field 0x14: %f", pTexture->field_0x14);

						ImGui::Spacing();
						ImGui::Spacing();

						ed_hash_code* pBitmapHashCode = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);
						if (pBitmapHashCode != (ed_hash_code*)0x0) {
							if (ImGui::CollapsingHeader("Bitmap")) {
								ImGui::Text("Bitmap Details");
								ImGui::Text("Bitmap Hash Code: %s", pBitmapHashCode->hash.ToString().c_str());

								ImGui::Spacing();
								ImGui::Spacing();

								ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pBitmapHashCode->pData);

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
							ed_hash_code* pPaletteHashCode = LOAD_SECTION_CAST(ed_hash_code*, pPaletteHashCodes[pLayer->paletteId].pData);
							ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pPaletteHashCode->pData);

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

			ImGui::Begin("Preview", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			auto& textureLibrary = Renderer::Kya::GetTextureLibrary();
			const auto& texture = textureLibrary.FindMaterial(gSelectedMaterial);

			if (texture->layers.begin()->textures.begin()->pSimpleTexture) {
				static PS2::GSSimpleTexture* pRenderer = nullptr;
				static VkDescriptorSet textureId;

				static bool bLinearSampler = false;

				if (ImGui::Checkbox("Linear", &bLinearSampler) || pRenderer != texture->layers.begin()->textures.begin()->pSimpleTexture->GetRenderer()) {
					if (pRenderer != nullptr) {
						ImGui_ImplVulkan_RemoveTexture(textureId);
					}

					pRenderer = texture->layers.begin()->textures.begin()->pSimpleTexture->GetRenderer();

					PS2::PSSamplerSelector sel;

					sel.ltf = bLinearSampler;

					VkSampler& sampler = PS2::GetSampler(sel);

					textureId = ImGui_ImplVulkan_AddTexture(sampler, pRenderer->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}
				
				//ImGui::Image(textureId, ImVec2(pRenderer->width, pRenderer->height));
				float aspectRatio = static_cast<float>(pRenderer->width) / static_cast<float>(pRenderer->height);
				ImGui::Image(textureId, ImVec2(256.0f, 256.0f / aspectRatio));
			}

			ImGui::End();

			if (!bOpen) {
				gSelectedMaterial = nullptr;
			}
		}

		void ShowMaterialDetails(const ed_g2d_material* pMaterial)
		{
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
								gSelectedMaterial = ed3DG2DGetG2DMaterialFromIndex(pManager, i);
								bOpenFirstMaterial = false;
							}

							if (gSelectedTexture->GetMaterials()[i].GetInUse()) {
								ImGui::SameLine();
								ImGui::TextColored(DebugHelpers::sValidColor, "(In Use)");
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

	ImGui::Spacing();
	ImGui::Spacing();

	if (ImGui::Button("Toggle Texture List")) {
		gShowTextureList = !gShowTextureList;
	}

	ImGui::End();

	if (gShowTextureList) {
		ShowList(&gShowTextureList);
	}
}

void Debug::Texture::Update()
{
	if (gSelectedTexture) {
		ShowTextureDetails();
	}

	if (gSelectedMaterial) {
		ShowMaterialDetails();
	}
}
