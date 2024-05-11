#include "DebugTexture.h"

#include <imgui.h>

#include "Texture.h"
#include "ed3D.h"

namespace Debug
{
	namespace Texture
	{
		static const Renderer::Kya::Texture* gSelectedTexture = nullptr;

		static void ShowList(bool* bOpen)
		{
			ImGui::Begin("Texture List", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			auto& textureLibrary = Renderer::Kya::GetTextureLibrary();

			textureLibrary.ForEach([&](const Renderer::Kya::Texture& texture) {
				if (ImGui::Selectable(texture.GetName().c_str())) {
					gSelectedTexture = &texture;
				}
			});

			ImGui::End();
		}

		static void ShowTextureDetails()
		{
			bool bShouldBeOpen = true;
			ImGui::Begin("Texture Details", &bShouldBeOpen, ImGuiWindowFlags_AlwaysAutoResize);

			ImGui::Text("Name: %s", gSelectedTexture->GetName().c_str());

			ImGui::Spacing();
			ImGui::Spacing();

			auto* pManager = gSelectedTexture->GetManager();

			if (pManager) {
				static ImVec4 sValidColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
				static ImVec4 sInvalidColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

				auto DrawValidColor = [&](char* fmt, void* p) {
					ImGui::TextColored(p ? sValidColor : sInvalidColor, fmt, p);
				};

				DrawValidColor("File Buffer: %p", pManager->pFileBuffer);
				DrawValidColor("*2D* (Texture Chunk): %p", pManager->pTextureChunk);
				DrawValidColor("MATA (Hash): %p", pManager->pMATA_HASH);
				DrawValidColor("T2DA: %p", pManager->pT2DA);
				DrawValidColor("PALL: %p", pManager->pPALL);
				DrawValidColor("ANMA: %p", pManager->pANMA);

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
							ImGui::Selectable(pHashCode->hash.ToString().c_str());
						}
					}
				}
			}

			ImGui::End();

			if (!bShouldBeOpen) {
				gSelectedTexture = nullptr;
			}
		}
	}

	static bool gShowTextureList = false;
}

void Debug::Texture::ShowMenu(bool* bOpen)
{
	ImGui::Begin("Texture", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::Button("Toggle Texture List")) {
		gShowTextureList = !gShowTextureList;
	}

	ImGui::End();

	if (gShowTextureList) {
		ShowList(&gShowTextureList);
	}

	if (gSelectedTexture) {
		ShowTextureDetails();
	}
}
