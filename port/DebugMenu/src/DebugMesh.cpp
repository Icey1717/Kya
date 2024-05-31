#include "DebugMesh.h"
#include <imgui.h>

#include "ed3D.h"
#include "Mesh.h"
#include "FileManager3D.h"
#include "DebugHelpers.h"
#include "DebugTexture.h"

namespace Debug
{
	namespace Mesh
	{
		static const Renderer::Kya::G3D* gSelectedMesh = nullptr;
		static ed_g3d_hierarchy* gSelectedHierarchy = nullptr;

		static void ShowList(bool* bOpen)
		{
			ImGui::Begin("Mesh List", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			auto& meshLibrary = Renderer::Kya::GetMeshLibrary();

			static bool bOnlyInUse = false;
			ImGui::Checkbox("Only in use", &bOnlyInUse);

			meshLibrary.ForEach([&](const Renderer::Kya::G3D& mesh) {
				bool bFound = true;

				if (bFound) {
					if (ImGui::Selectable(mesh.GetName().c_str())) {
						gSelectedMesh = &mesh;
						gSelectedHierarchy = nullptr;
					}
				}
				});

			ImGui::End();
		}

		static void ListHallDetails(ed_g3d_manager* pManager)
		{
			DebugHelpers::ListChunckDetails(pManager->HALL);

			ImGui::Spacing();
			ImGui::Spacing();

			ed_Chunck* pHASH = pManager->HALL + 1;

			DebugHelpers::ListChunckDetails(pHASH);

			ImGui::Spacing();
			ImGui::Spacing();

			ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(pHASH + 1);
			const int chunkNb = edChunckGetNb(pHASH, reinterpret_cast<char*>(pManager->HALL) + pManager->HALL->size);

			ImGui::Text("Number of Hashes: %d", chunkNb - 1);

			for (int curIndex = 0; curIndex < chunkNb - 1; curIndex = curIndex + 1) {
				char buff[256];
				sprintf_s(buff, 256, "%d - %s", curIndex, pHashCode->hash.ToString().c_str());

				if (gSelectedHierarchy == nullptr || ImGui::Selectable(buff)) {
					gSelectedHierarchy = ed3DG3DHierarchyGetFromIndex(pManager, curIndex);
				}

				pHashCode = pHashCode + 1;
			}

			if (gSelectedHierarchy) {
				if (ImGui::CollapsingHeader("Hierarchy", ImGuiTreeNodeFlags_DefaultOpen)) {
					ImGui::Text("Transform A: %s", gSelectedHierarchy->transformA.ToString().c_str());
					ImGui::Text("Transform B: %s", gSelectedHierarchy->transformA.ToString().c_str());

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Hash: %s", gSelectedHierarchy->hash.ToString().c_str());

					ImGui::Text("field_0x88: %d", gSelectedHierarchy->field_0x88);
					ImGui::Text("field_0x89: %d", gSelectedHierarchy->field_0x89);
					ImGui::Text("bRenderShadow: %u", gSelectedHierarchy->bRenderShadow);

					ImGui::Spacing();
					ImGui::Spacing();

					//DebugHelpers::TextValidValue("pShadowAnimMatrix: %p", LOAD_SECTION(gSelectedHierarchy->pShadowAnimMatrix));
					DebugHelpers::TextValidValue("pLinkTransformData: %p", LOAD_SECTION(gSelectedHierarchy->pLinkTransformData));
					DebugHelpers::TextValidValue("field_0x94: %p", LOAD_SECTION(gSelectedHierarchy->field_0x94));
					DebugHelpers::TextValidValue("pTextureInfo: %p", LOAD_SECTION(gSelectedHierarchy->pTextureInfo));

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("lodCount: %d", gSelectedHierarchy->lodCount);
					ImGui::Text("flags_0x9e: %u", gSelectedHierarchy->flags_0x9e);

					ImGui::Spacing();
					ImGui::Spacing();

					DebugHelpers::TextValidValue("pHierarchySetup: %p", LOAD_SECTION(gSelectedHierarchy->pHierarchySetup));
					DebugHelpers::TextValidValue("pMatrixPkt: %p", LOAD_SECTION(gSelectedHierarchy->pMatrixPkt));
					DebugHelpers::TextValidValue("pAnimMatrix: %p", LOAD_SECTION(gSelectedHierarchy->pAnimMatrix));

					ImGui::Text("subMeshParentCount_0xac: %d", gSelectedHierarchy->subMeshParentCount_0xac);
					ImGui::Text("desiredLod: %d", gSelectedHierarchy->desiredLod);
					ImGui::Text("GlobalAlhaON: %d", gSelectedHierarchy->GlobalAlhaON);

					ImGui::Spacing();
					ImGui::Spacing();

					for (int i = 0; i < gSelectedHierarchy->lodCount; i++) {
						char buff[256];
						sprintf_s(buff, "LOD %d", i);

						if (ImGui::CollapsingHeader(buff, ImGuiTreeNodeFlags_DefaultOpen)) {
							ed3DLod* pLod = gSelectedHierarchy->aLods + i;

							DebugHelpers::TextValidValue("pObj: %p", LOAD_SECTION(pLod->pObj));
							ImGui::Text("renderType: %u", pLod->renderType);
							ImGui::Text("sizeBias: %f", pLod->sizeBias);

							ed_hash_code* pHash = LOAD_SECTION_CAST(ed_hash_code*, pLod->pObj);
							ImGui::Text("Hash: %s", i, pHash->hash.ToString().c_str());
						}
					}
				}
			}

			return;
		}

		static void ListMbnaDetails(ed_g3d_manager* pManager)
		{
			DebugHelpers::ListChunckDetails(pManager->MBNA);

			ImGui::Spacing();
			ImGui::Spacing();

			ed_Chunck* pMBNK = pManager->MBNA + 1;
			DebugHelpers::ListChunckDetails(pMBNK);

			// Technically there can be stuff in between this, maybe should change this to get chunk call.
			assert(pMBNK->hash == HASH_CODE_MBNK);

			ImGui::Spacing();
			ImGui::Spacing();

			if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
				const int matHashNb = (pMBNK->size - sizeof(ed_Chunck)) / sizeof(ed_hash_code);

				ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(pMBNK + 1);

				for (int i = 0; i < matHashNb; i++) {
					char buff[256];
					sprintf_s(buff, 256, "%d - %s", i, pHashCode->hash.ToString().c_str());

					if (ImGui::Selectable(buff)) {
						ed_hash_code* pMatHash = LOAD_SECTION_CAST(ed_hash_code*, pHashCode->pData);
						ed_Chunck* pMAT = LOAD_SECTION_CAST(ed_Chunck*, pMatHash->pData);
						
						const ed_g2d_material* pMaterial = reinterpret_cast<ed_g2d_material*>(pMAT + 1);
						Texture::ShowMaterialDetails(pMaterial);
					}

					pHashCode++;
				}
			}
		}

		static void ShowMeshDetails()
		{
			bool bOpen = true;
			ImGui::Begin("Mesh Details", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			ImGui::Text("Name: %s", gSelectedMesh->GetName().c_str());

			ImGui::Spacing();
			ImGui::Spacing();

			auto* pManager = gSelectedMesh->GetManager();

			if (pManager) {
				DebugHelpers::TextValidValue("File Buffer: %p", pManager->fileBufferStart);
				DebugHelpers::TextValidValue("field_0x4: %p", pManager->field_0x4);

				ImGui::Spacing();
				ImGui::Spacing();

				DebugHelpers::TextValidValue("OBJA: %p", pManager->OBJA);
				DebugHelpers::TextValidValue("LIA.: %p", pManager->LIA);
				DebugHelpers::TextValidValue("CAMA: %p", pManager->CAMA);
				DebugHelpers::TextValidValue("SPRA: %p", pManager->SPRA);
				DebugHelpers::TextValidValue("HALL: %p", pManager->HALL);
				DebugHelpers::TextValidValue("CSTA: %p", pManager->CSTA);
				DebugHelpers::TextValidValue("GEOM: %p", pManager->GEOM);
				DebugHelpers::TextValidValue("MBNA: %p", pManager->MBNA);
				DebugHelpers::TextValidValue("INFA: %p", pManager->INFA);
				DebugHelpers::TextValidValue("CDZA: %p", pManager->CDZA);
				DebugHelpers::TextValidValue("ANMA: %p", pManager->ANMA);


				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Text("fileLengthA: 0x%x", pManager->fileLengthA);
				ImGui::Text("fileLengthB: 0x%x", pManager->fileLengthB);
				ImGui::Text("field_0xc: 0x%x", pManager->field_0xc);

				ImGui::Spacing();
				ImGui::Spacing();

				if (pManager->HALL) {
					if (ImGui::CollapsingHeader("HALL", ImGuiTreeNodeFlags_DefaultOpen)) {
						ListHallDetails(pManager);
					}
				}

				if (pManager->MBNA) {
					if (ImGui::CollapsingHeader("MBNA")) {
						ListMbnaDetails(pManager);
					}
				}
			}

			ImGui::End();

			if (!bOpen) {
				gSelectedMesh = nullptr;
			}
		}

		static bool gShowMeshList;
	}
}

void Debug::Mesh::ShowMenu(bool* bOpen)
{
	ImGui::Begin("Mesh", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

	auto& textureLibrary = Renderer::Kya::GetMeshLibrary();
	ImGui::Text("Count: %d", textureLibrary.GetMeshCount());

	ImGui::Spacing();
	ImGui::Spacing();

	if (ImGui::Button("Toggle Mesh List")) {
		gShowMeshList = !gShowMeshList;
	}

	ImGui::End();

	if (gShowMeshList) {
		ShowList(&gShowMeshList);
	}

	if (gSelectedMesh) {
		ShowMeshDetails();
	}
}