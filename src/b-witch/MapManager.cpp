#include "MapManager.h"
#include "MemoryStream.h"
#include "Kya.h"
#include "MathOps.h"
#include "ActorHero.h"
#include "ActorTeleporter.h"
#include "ActorCompanion.h"
#include "ActorRune.h"
#include "ActorWolfen.h"
#include "ActorManager.h"
#include "InputManager.h"
#include "edVideo/VideoD.h"
#include "EventManager.h"
#include "FrontendDisp.h"
#include "FrontendMoney.h"
#include "LevelScheduler.h"
#include "TranslatedTextData.h"
#include "TimeController.h"
#include "BootData.h"
#include "edStr.h"
#include "edText.h"
#include "DlistManager.h"
#include "edBank/edBankFile.h"
#include "ed3D/ed3DG3D.h"
#include "Rendering/edCTextFont.h"

void CMapManager::OnLoadLevelBnk_003f9a60(ByteCode* pMemoryStream)
{
	int* piVar1;
	int iVar2;
	uint uVar3;
	float fVar4;

	this->field_0x4 = pMemoryStream->GetS32();
	this->field_0x8 = pMemoryStream->GetU32();
	this->field_0xc = (pMemoryStream->GetF32() * 3.141593f) / 180.0f;
	piVar1 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar1 * 4;
	}

	this->field_0x10 = piVar1;
	this->field_0x14 = pMemoryStream->GetS32();
	this->field_0x18.index = pMemoryStream->GetS32();
	if (this->field_0x18.index == -1) {
		this->field_0x394 = 1;
	}
	else {
		this->field_0x394 = 0;
	}

	return;
}

struct MapStruct_8
{
	edF32VECTOR4* pVector;
	int field_0x4;
};

MapStruct_8 MapStruct_8_ARRAY_0049ca70[32];
int INT_004497ec;

void FUN_003f9b40(edCEventMessage* pMsg, int)
{
	int iVar1;
	int iVar2;
	bool bVar3;
	int iVar4;

	iVar4 = INT_004497ec;
	iVar1 = CLevelScheduler::gThis->currentLevelID;
	iVar2 = LOAD_POINTER_CAST(EventSendInfo*, pMsg->pEventCollider->aSendInfo[pMsg->colliderId])[2].field_0x0;
	if (((iVar2 != 0x10) && (iVar2 != 0xe)) && (iVar2 != 0xf)) {
		bVar3 = iVar1 == iVar2;
		if (!bVar3) {
			bVar3 = iVar2 == INT_ARRAY_0048ed60[iVar1];
		}

		if (!bVar3) {
			bVar3 = iVar1 == INT_ARRAY_0048ed60[iVar2];
		}

		if (((!bVar3) && ((iVar1 != 0xc || (iVar2 != 0)))) && (INT_004497ec < 0x20)) {
			MapStruct_8_ARRAY_0049ca70[INT_004497ec].pVector = &LOAD_POINTER_CAST(ed_zone_3d*, pMsg->pEventCollider->field_0x1c)->boundSphere;
			MapStruct_8_ARRAY_0049ca70[iVar4].field_0x4 = iVar2;
			INT_004497ec = INT_004497ec + 1;
		}
	}

	return;
}

int INT_ARRAY_004279e0[3] = { 1, 0, 5 };

void CMapManager::Level_PostInit()
{
	int iVar1;
	int iVar2;
	bool bVar3;
	CCamera* pCVar4;
	int iVar5;
	CCinematic* pCinematic;
	int iVar6;
	edF32VECTOR4* peVar7;
	ed_zone_3d* pZone;
	int iVar8;
	CActor** pEntryIt;
	float fVar9;
	float fVar10;
	CActorsTable actorsTable;
	MapInitParams uStack16;
	MapInitParams* local_4;
	CActorTeleporter* pTeleporter;
	CCinematicManager* pCinematicManager;

	this->pLevelMap->Level_PostInit();
	this->field_0x24 = 0;
	this->pWorldMap->Level_PostInit();
	this->field_0x24 = 0;

	pCVar4 = CScene::ptable.g_CameraManager_0045167c->GetScenaricCamera(this->field_0x4);
	if (pCVar4 != (CCamera*)0x0) {
		this->field_0x2c.field_0x0 = pCVar4->transformationMatrix.rowT.xyz;
		this->field_0x2c.field_0xc = pCVar4->lookAt.xyz;
		this->field_0x2c.field_0x18 = pCVar4->transformationMatrix.rowY.xyz;
		this->field_0x2c.field_0x24 = pCVar4->fov;
		this->field_0x2c.field_0x28 = this->field_0x8;
	}

	if (CActorHero::_gThis != (CActorHero*)0x0) {
		local_4 = &uStack16;
		uStack16.field_0x0[2] = INT_ARRAY_004279e0[2];
		uStack16.field_0x0[0] = INT_ARRAY_004279e0[0];
		uStack16.field_0x0[1] = CActorHero::_gThis->actorManagerIndex;

		edEvent_00259c50((CScene::ptable.g_EventManager_006f5080)->activeChunkId, -1, 0xf, &local_4, 1, 3, FUN_003f9b40, 1);
	}

	pCinematicManager = g_CinematicManager_0048efc;
	iVar5 = g_CinematicManager_0048efc->GetNumCutscenes_001c50b0();
	iVar8 = 0;
	if (0 < iVar5) {
		do {
			pCinematic = pCinematicManager->GetCinematic(iVar8);
			iVar2 = INT_004497ec;
			pZone = (pCinematic->zoneRefA).Get();
			peVar7 = &gF32Vertex4Zero;

			if (pZone == (ed_zone_3d*)0x0) {
				pZone = (pCinematic->zoneRefB).Get();
			}

			if (pZone != (ed_zone_3d*)0x0) {
				peVar7 = &pZone->boundSphere;
			}

			iVar6 = pCinematic->endLevelId;
			iVar1 = CLevelScheduler::gThis->currentLevelID;
			if (((iVar6 != 0x10) && (iVar6 != 0xe)) && (iVar6 != 0xf)) {
				bVar3 = iVar1 == iVar6;
				if (!bVar3) {
					bVar3 = iVar6 == INT_ARRAY_0048ed60[iVar1];
				}

				if (!bVar3) {
					bVar3 = iVar1 == INT_ARRAY_0048ed60[iVar6];
				}

				if (((!bVar3) && ((iVar1 != 0xc || (iVar6 != 0)))) && (INT_004497ec < 0x20)) {
					MapStruct_8_ARRAY_0049ca70[INT_004497ec].pVector = peVar7;
					MapStruct_8_ARRAY_0049ca70[iVar2].field_0x4 = iVar6;
					INT_004497ec = INT_004497ec + 1;
				}
			}
			iVar8 = iVar8 + 1;
		} while (iVar8 < iVar5);
	}

	actorsTable.nbEntries = 0;
	CScene::ptable.g_ActorManager_004516a4->GetActorsByClassID(TELEPORTER, &actorsTable);
	iVar5 = 0;
	if (0 < actorsTable.nbEntries) {
		pEntryIt = actorsTable.aEntries;
		do {
			pTeleporter = static_cast<CActorTeleporter*>(*pEntryIt);
			bVar3 = pTeleporter->FUN_002edba0();
			if (bVar3 != false) {
				iVar6 = pTeleporter->FUN_002edaa0();
				iVar2 = INT_004497ec;
				iVar8 = CLevelScheduler::gThis->currentLevelID;
				if (((iVar6 != 0x10) && (iVar6 != 0xe)) && (iVar6 != 0xf)) {
					bVar3 = iVar8 == iVar6;
					if (!bVar3) {
						bVar3 = iVar6 == INT_ARRAY_0048ed60[iVar8];
					}

					if (!bVar3) {
						bVar3 = iVar8 == INT_ARRAY_0048ed60[iVar6];
					}

					if ((!bVar3) && (((iVar8 != 0xc || (iVar6 != 0)) && (INT_004497ec < 0x20)))) {
						MapStruct_8_ARRAY_0049ca70[INT_004497ec].pVector = &pTeleporter->field_0x270;
						MapStruct_8_ARRAY_0049ca70[iVar2].field_0x4 = iVar6;
						INT_004497ec = INT_004497ec + 1;
					}
				}
			}

			iVar5 = iVar5 + 1;
			pEntryIt = pEntryIt + 1;
		} while (iVar5 < actorsTable.nbEntries);
	}

	return;
}

void CMapManager::OnFileLoaded()
{
	bool bVar1;
	uint uVar2;
	edDList_material* pMapManager;
	uint uVar3;
	uint inFileIndex;
	edBANK_ENTRY_INFO bankEntryInfo;
	int iStack8;
	int iStack4;

	uVar2 = this->pBankBufferEntry->get_element_count();
	inFileIndex = 0;
	if (uVar2 != 0) {
		do {
			bVar1 = this->pBankBufferEntry->get_info(inFileIndex, &bankEntryInfo, (char*)0x0);

			if (bVar1 == false) break;

			uVar3 = bankEntryInfo.type << 0x10 | bankEntryInfo.stype;
			if (uVar3 == 0x40001) {
				ed3DInstallG2D(bankEntryInfo.fileBufferStart, bankEntryInfo.size, &iStack8, &this->field_0x1e0, 0);
				edDListCreatMaterialFromIndex(this->field_0x210, 0, &this->field_0x1e0, 2);
			}
			else {
				if (uVar3 == 0x50001) {
					NAME_NEXT_OBJECT("MapManager::OnFileLoaded - MapData");
					ed3DInstallG2D(bankEntryInfo.fileBufferStart, bankEntryInfo.size, &iStack4, &this->field_0xc0, 0);
					uVar3 = 0;
					pMapManager = this->field_0xf0;
					do {
						edDListCreatMaterialFromIndex(pMapManager, uVar3, &this->field_0xc0, 2);
						uVar3 = uVar3 + 1;
						pMapManager = pMapManager + 1;
					} while (uVar3 < 0xf);
				}
			}
			inFileIndex = inFileIndex + 1;
		} while (inFileIndex < uVar2);
	}

	this->field_0x300 = 1;

	return;
}

void CMapManager::Close()
{
	bool bVar1;
	edDList_material* pCVar2;
	uint uVar3;

	this->field_0x300 = 0;
	if ((this->pBankBufferEntry != (edCBankBufferEntry*)0x0) && (bVar1 = this->pBankBufferEntry->is_loaded(), bVar1 == false)) {
		this->pBankBufferEntry->wait();
	}

	edDListTermMaterial(this->field_0x210);
	ed3DUnInstallG2D(&this->field_0x1e0);

	uVar3 = 0;
	pCVar2 = this->field_0xf0;
	do {
		edDListTermMaterial(pCVar2);
		uVar3 = uVar3 + 1;
		pCVar2 = pCVar2 + 1;
	} while (uVar3 < 0xf);

	ed3DUnInstallG2D(&this->field_0xc0);

	return;
}

void CMapManager::Func_003f8d80()
{
	int iVar1;
	int iVar2;
	NativShopLevelSubObj* pNVar4;
	Episode* pEVar5;
	int iVar6;
	NativShopLevelSubObjSubObj* pNVar7;
	ulong uVar8;
	ObjectiveEntry* pLVar9;
	ObjectiveEntry* pLVar10;
	ObjectiveEntry* pLVar11;
	CLevelScheduler* pLevelScheduler;

	pLevelScheduler = CLevelScheduler::gThis;
	iVar2 = 6;
	iVar6 = this->field_0x338.field_0x0;
	pLVar11 = CLevelScheduler::gThis->field_0x4220;
	pLVar10 = pLVar11 + CLevelScheduler::gThis->objCount_0x4218;
	pLVar9 = (ObjectiveEntry*)0x0;
	for (; pLVar11 < pLVar10; pLVar11 = pLVar11 + 1) {
		if (((pLVar11->field_0x4 == 0) && (pLVar11->messageKey != 0)) && (iVar1 = CLevelScheduler::ScenVar_Get(pLVar11->field_0x0), iVar1 == 1)) {
			iVar2 = CLevelScheduler::MapFunc_002d8dc0(pLVar11);
			pLVar9 = pLVar11;
		}
	}

	if (pLVar9 == (ObjectiveEntry*)0x0) {
		iVar1 = 0x62;
	}
	else {
		iVar1 = pLVar9->field_0x0;
	}

	if (iVar1 == iVar6) {
		if ((iVar1 != 0x62) && ((iVar2 != this->field_0x338.field_0x4 || (this->field_0x338.hash != pLVar9->field_0x40)))) {
			this->field_0x334 = 1;
			this->field_0x330 = 0.0f;
			this->field_0x37c = GetTimer()->totalTime;
			this->field_0x338.hash = pLVar9->field_0x40;
			this->field_0x338.field_0x4 = iVar2;
		}
	}
	else {
		this->field_0x358 = this->field_0x338;
		this->field_0x334 = 1;
		this->field_0x330 = 0.0f;
		this->field_0x37c = GetTimer()->totalTime;
		this->field_0x338.field_0x0 = iVar1;
		if (this->field_0x338.field_0x0 == 0x62) {
			this->field_0x338.hash = 0;
			this->field_0x338.field_0x4 = 6;
			if (10.0f < GetTimer()->scaledTotalTime) {
				IMPLEMENTATION_GUARD(
				pLevelScheduler->SaveGame_InitiateAutoSave(1.0f, 0);)
			}
		}
		else {
			this->field_0x338.hash = pLVar9->field_0x40;
			this->field_0x338.field_0x4 = iVar2;
		}
	}

	iVar6 = 0x10;
	this->field_0x338.field_0x10 = 0x10;
	this->field_0x338.field_0x14 = 0;
	this->field_0x338.hash_2 = 0;
	if (pLVar9 != (ObjectiveEntry*)0x0) {
		iVar2 = 0;
		if (this->field_0x338.field_0x4 == 2) {
			iVar1 = 0;

			do {
				IMPLEMENTATION_GUARD(
				uVar8 = pLevelScheduler->FUN_002ddfb0(iVar1);)
				if (uVar8 == 0) {
					iVar2 = iVar2 + 1;
					iVar6 = iVar1;
				}

				iVar1 = iVar1 + 1;
			} while (iVar1 < 0x10);

			if (iVar2 == 1) {
				this->field_0x338.field_0x10 = iVar6;
			}
		}

		if (pLVar9->field_0x24 != 0x20) {
			IMPLEMENTATION_GUARD(
			pNVar4 = pLevelScheduler->FUN_002d9510(pLVar9->field_0x24);
			if (pNVar4 != (NativShopLevelSubObj*)0x0) {
				pEVar5 = pLevelScheduler->GetEpisode(pNVar4->episodeThresholdLink);
				iVar6 = CLevelScheduler::ScenVar_Get(0);
				this->field_0x34c = pEVar5->minWolfen - iVar6;
			}
			pNVar7 = pLevelScheduler->FUN_002d9590(pLVar9->field_0x24);
			if (pNVar7 != (NativShopLevelSubObjSubObj*)0x0) {
				this->field_0x350 = pNVar7->field_0x8;
			})
		}
	}

	return;
}

void CMapManager::DrawMapPromptHint()
{
	bool bVar1;
	uint uVar3;
	float fVar4;
	float fVar5;

	if ((((this->field_0x334 != 0) && ((GameFlags & 0x1c) == 0)) && (CLevelScheduler::gThis->bShouldLoad == 0)) && ((GameFlags & 0x180) == 0)) {
		if (CActorCompanion::_gThis != (CActorCompanion*)0x0) {
			bVar1 = (CActorCompanion::_gThis->flags & 4) != 0;
			if (bVar1) {
				bVar1 = CActorCompanion::_gThis->behaviourCompanion.FUN_001e6d60();
				bVar1 = bVar1 != false;
			}

			if (bVar1) {
				return;
			}
		}

		bVar1 = g_CinematicManager_0048efc->FUN_001c5c60();
		if ((bVar1 == false) && (CActorHero::_gThis != (CActorHero*)0x0)) {
			fVar5 = this->field_0x330 + GetTimer()->cutsceneDeltaTime;
			this->field_0x330 = fVar5;
			if (fVar5 < 15.0f) {
				fVar4 = fVar5 * 256.0f;
				if (fVar4 < 2.147484e+09f) {
					uVar3 = static_cast<uint>(fVar4);
				}
				else {
					uVar3 = static_cast<int>(fVar4 - 2.147484e+09f) | 0x80000000;
				}

				uVar3 = static_cast<int>(uVar3) % 0x140;
				if (0xff < uVar3) {
					uVar3 = (0x40 - (uVar3 - 0x100)) * 0xff >> 6;
				}

				fVar4 = 1.0f;
				if (14.5f < fVar5) {
					fVar4 = (15.0f - fVar5) / 0.5f;
				}

				bVar1 = GuiDList_BeginCurrent();
				if (bVar1 != false) {
					fVar5 = static_cast<float>(uVar3 >> 1) * fVar4;
					if (fVar5 < 2.147484e+09f) {
						BootBitmaps[20].rgba[3] = (byte)static_cast<int>(fVar5);
					}
					else {
						BootBitmaps[20].rgba[3] = (byte)static_cast<int>(fVar5 - 2.147484e+09f);
					}

					BootBitmaps[0x14].Draw(1.0f, 64.0f, (static_cast<float>(gVideoConfig.screenHeight) * 392.0f) / 512.0f, 0x12);
					uVar3 = 0x80 - (uVar3 >> 1);
					BootBitmaps[20].rgba[3] = 0x7f;

					if (static_cast<int>(uVar3) < 0) {
						fVar5 = static_cast<float>(uVar3 >> 1 | uVar3 & 1);
						fVar5 = fVar5 + fVar5;
					}
					else {
						fVar5 = static_cast<float>(uVar3);
					}

					fVar5 = fVar5 * fVar4;
					if (fVar5 < 2.147484e+09f) {
						BootBitmaps[4].rgba[3] = (byte)static_cast<int>(fVar5);
					}
					else {
						BootBitmaps[4].rgba[3] = (byte)static_cast<int>(fVar5 - 2.147484e+09f);
					}

					BootBitmaps[4].Draw(1.0f, 64.0f, (static_cast<float>(gVideoConfig.screenHeight) * 392.0f) / 512.0f + 6.0f, 0x12);
					BootBitmaps[4].rgba[3] = 0x7f;
					GuiDList_EndCurrent();
				}
			}
			else {
				this->field_0x334 = 0;
				this->field_0x330 = 0.0f;
			}
		}
	}

	return;
}

float CMapManager::ComputeMapSlideFactor(float param_1, int param_3)
{
	float fVar1;

	if (param_3 == 0) {
		if (param_1 < 1.0f) {
			fVar1 = cosf((1.0f - param_1) * 1.570796f - 1.570796f);
		}
		else {
			fVar1 = 0.0f;
		}
	}
	else {
		fVar1 = 1.0f;

		if (param_1 < 1.0f) {
			fVar1 = cosf(param_1 * 1.570796f - 1.570796f);
		}
	}

	return fVar1;
}

edF32VECTOR4 edF32VECTOR4_00427a30 = { 0.0f, 0.0f, 0.2f, 0.8f };
edF32VECTOR4 edF32VECTOR4_00427a40 = { 0.0f, 0.0f, 0.8f, 0.8f };
edF32VECTOR4 edF32VECTOR4_00427a50 = { 0.0f, 0.0f, 0.8f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00427a60 = { 0.0f, 0.0f, 0.2f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00427a70 = { 0.0f, 0.0f, 0.2f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00427a80 = { 0.0f, 0.0f, 1.0f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00427a90 = { 0.0f, 0.0f, 1.0f, 1.0f };
edF32VECTOR4 edF32VECTOR4_00427aa0 = { 0.0f, 0.0f, 0.2f, 1.0f };
edF32VECTOR4 edF32VECTOR4_00427ab0 = { 1.0f, 1.0f, 1.0f, 1.0f };

void CMapManager::DrawObjectivePanel(float param_1)
{
	float t;
	float t_00;
	float t_01;
	float x;
	float s;
	float t_02;
	uint uVar2;
	uint uVar3;
	edCTextStyle* pNewFont;
	char* pcVar4;
	edDList_material* pMaterialInfo;
	float x_00;
	float fVar5;
	float x_01;
	float y;
	float y_00;
	float s_00;
	float y_01;
	float s_01;
	edCTextStyle eStack272;
	float local_50;
	float local_4c;
	float local_48;
	float local_44;
	float local_40;
	float local_3c;
	float local_38;
	float local_34;
	float local_30;
	float local_2c;
	float local_28;
	float local_24;
	float local_20;
	float local_1c;
	float local_18;
	float local_14;
	_rgba local_4;

	fVar5 = this->field_0x378;
	if (fVar5 != 0.0f) {
		if (0.0f < param_1) {
			local_4c = fVar5 * (param_1 - 1.0f);
			local_50 = edF32VECTOR4_00427a30.x;
			local_48 = edF32VECTOR4_00427a30.z;
			local_44 = edF32VECTOR4_00427a30.w;
			local_38 = edF32VECTOR4_00427a40.z;
			local_34 = edF32VECTOR4_00427a40.w;
			local_28 = edF32VECTOR4_00427a50.z;
			local_24 = edF32VECTOR4_00427a50.w;
			local_20 = edF32VECTOR4_00427a60.x;
			local_18 = edF32VECTOR4_00427a60.z;
			local_14 = edF32VECTOR4_00427a60.w;
			local_4.rgba = 0x80808080;
			local_2c = (local_4c + fVar5) * 1.2f;
			local_40 = static_cast<float>(gVideoConfig.screenWidth);
			local_3c = local_4c;
			local_30 = local_40;
			local_1c = local_2c;
			edDListUseMaterial(&MenuBitmaps[8].materialInfo);
			edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);
			edDListColor4u8(local_4.r, local_4.g, local_4.b, local_4.a);
			edDListLoadIdentity();
			edDListTexCoo2f(local_48, local_44);
			edDListVertex4f(local_50, local_4c, 0.0f, 0.0f);
			edDListTexCoo2f(local_38, local_34);
			edDListVertex4f(local_40, local_3c, 0.0f, 0.0f);
			edDListTexCoo2f(local_18, local_14);
			edDListVertex4f(local_20, local_1c, 0.0f, 0.0f);
			edDListTexCoo2f(local_28, local_24);
			edDListVertex4f(local_30, local_2c, 0.0f, 0.0f);
			edDListEnd();
			fVar5 = 1.0f;
			uVar2 = 0;
			if (this->field_0x37c != 0.0f) {
				fVar5 = GetTimer()->totalTime - this->field_0x37c;
				if (fVar5 <= 2.0f) {
					fVar5 = (fVar5 / 2.0f) * 0.5f;
				}
				else {
					fVar5 = ((fVar5 - 2.0f) * 0.5f) / 3.5f + 0.5f;
				}
				uVar2 = this->field_0x358.GetObjectiveLineMask();
				uVar3 = this->field_0x338.GetObjectiveLineMask();
				uVar2 = uVar2 ^ uVar3;
				if (((uVar2 & 0x40) == 0) && ((this->field_0x358).field_0x0 == (this->field_0x338).field_0x0)) {
					if (((this->field_0x358).hash != (this->field_0x338).hash) || ((uVar2 & 1) != 0)) {
						uVar2 = uVar2 | 0x3f;
					}
				}
				else {
					uVar2 = uVar2 | 0x7f;
				}
			}

			if (fVar5 < 0.5f) {
				this->field_0x358.DrawObjectiveDetails(this->field_0x378 * (param_1 - 1.0f), (0.5f - fVar5) * 2.0f, uVar2, 0);
			}
			else {
				this->field_0x338.DrawObjectiveDetails(this->field_0x378 * (param_1 - 1.0f), (fVar5 - 0.5f) * 2.0f, uVar2, 1);
			}
		}
		eStack272.Reset();
		eStack272.SetShadow(0x100);
		eStack272.alpha = 0xff;
		eStack272.rgbaColour = 0xb6bbfcff;
		eStack272.SetVerticalAlignment(8);
		eStack272.SetHorizontalAlignment(0);
		eStack272.SetFont(BootDataFont, false);
		eStack272.SetScale(0.80000001f, 0.80000001f);
		pNewFont = edTextStyleSetCurrent(&eStack272);
		y_01 = (static_cast<float>(gVideoConfig.screenHeight) * 30.0f) / 512.0f;
		edCTextFormat auStack5664;
		pcVar4 = gMessageManager.get_message(0xe1519090a114a45);
		auStack5664.FormatString("%[ACTION]b   %s", pcVar4);
		t_02 = edF32VECTOR4_00427aa0.w;
		s = edF32VECTOR4_00427aa0.z;
		x = edF32VECTOR4_00427aa0.x;
		t_01 = edF32VECTOR4_00427a90.w;
		s_01 = edF32VECTOR4_00427a90.z;
		t_00 = edF32VECTOR4_00427a80.w;
		s_00 = edF32VECTOR4_00427a80.z;
		t = edF32VECTOR4_00427a70.w;
		fVar5 = edF32VECTOR4_00427a70.z;
		y_00 = y_01 - 18.0f;
		x_00 = edF32VECTOR4_00427a70.x;
		y = y_01 + 15.0f;
		x_01 = auStack5664.field_0x8 * 0.8f + 25.0f + 24.0f;
		pMaterialInfo = (edDList_material*)0x0;
		if (this->field_0x300 != 0) {
			pMaterialInfo = this->field_0xf0 + 0xb;
		}

		edDListUseMaterial(pMaterialInfo);
		edDListBegin(0.0f, 0.0f, 0.0f, 4, 6);
		edDListColor4u8(0x80, 0x80, 0x80, 0x80);
		edDListLoadIdentity();
		edDListTexCoo2f(fVar5, t);
		edDListVertex4f(x_00, y_00, 0.0f, 0.0f);
		edDListTexCoo2f(s, t_02);
		edDListVertex4f(x, y, 0.0f, 0.0f);
		edDListTexCoo2f(s_00 - 0.2f, t_00);
		fVar5 = x_01;
		edDListVertex4f(x_01 - 24.0f, y_00, 0.0f, 0.0f);
		edDListTexCoo2f(s_01 - 0.2f, t_01);
		edDListVertex4f(x_01 - 24.0f, y, 0.0f, 0.0f);
		edDListTexCoo2f(s_00, t_00);
		edDListVertex4f(fVar5, y_00, 0.0f, 0.0f);
		edDListTexCoo2f(s_01, t_01);
		edDListVertex4f(x_01, y, 0.0f, 0.0f);
		edDListEnd();
		auStack5664.Display(25.0f, y_01);
		edTextStyleSetCurrent(pNewFont);
	}

	return;
}

bool CMapManager::LoadMarkerPositionsForLevel(int levelId, MapDataSizes* pMarkerCounts, MapPosition* pMarkerBuffer, int param_5)
{
	int iVar1;
	byte bVar2;
	bool bVar3;
	SaveDataChunk_BLMP* pCVar4;
	MapPosition** ppMVar5;
	int* piVar6;
	int iVar7;
	int iVar8;
	int iVar9;
	bool bVar10;
	uint uVar11;
	uint uVar12;
	MapPosition aMStack4144[128];
	MapPosition aMStack2096[128];
	MapDataSizes local_30;
	MapDataSizes local_20;
	MapPosition* local_8[2];

	iVar8 = INT_ARRAY_0048ed60[levelId];
	if (iVar8 == levelId) {
		iVar8 = INT_ARRAY_0048eda0[levelId];
	}

	pMarkerCounts->field_0x0 = 0;
	pMarkerCounts->nbTeleporters = 0;
	bVar10 = false;
	pMarkerCounts->nbWolfen = 0;
	pMarkerCounts->nbRunes = 0;
	if (iVar8 == levelId) {
		if (levelId == CLevelScheduler::gThis->currentLevelID) {
			GetMarkerPositions(pMarkerCounts, pMarkerBuffer);
			bVar10 = true;
		}
		else {
			pCVar4 = CLevelScheduler::gThis->LoadMapSaveChunk(levelId);
			if (pCVar4 != (SaveDataChunk_BLMP*)0x0) {
				*pMarkerCounts = pCVar4->mapDataSizes;
				iVar8 = pCVar4->mapDataSizes.field_0x0 + pCVar4->mapDataSizes.nbTeleporters + pCVar4->mapDataSizes.nbWolfen + pCVar4->mapDataSizes.nbRunes;
				if (param_5 < iVar8) {
					iVar8 = param_5;
				}

				memcpy(pMarkerBuffer, &pCVar4->aPositions, iVar8 * sizeof(MapPosition));
				bVar10 = true;
			}
		}
	}
	else {
		bVar3 = false;
		if (levelId == CLevelScheduler::gThis->currentLevelID) {
			GetMarkerPositions(&local_30, aMStack4144);
			bVar3 = true;
		}
		else {
			pCVar4 = CLevelScheduler::gThis->LoadMapSaveChunk(levelId);
			if (pCVar4 != (SaveDataChunk_BLMP*)0x0) {
				local_30.field_0x0 = pCVar4->mapDataSizes.field_0x0;
				local_30.nbTeleporters = pCVar4->mapDataSizes.nbTeleporters;
				local_30.nbWolfen = pCVar4->mapDataSizes.nbWolfen;
				local_30.nbRunes = pCVar4->mapDataSizes.nbRunes;
				iVar9 = pCVar4->mapDataSizes.field_0x0 + pCVar4->mapDataSizes.nbTeleporters + pCVar4->mapDataSizes.nbWolfen + pCVar4->mapDataSizes.nbRunes;
				if (0x80 < iVar9) {
					iVar9 = 0x80;
				}

				memcpy(aMStack4144, &pCVar4->aPositions, iVar9 * sizeof(MapPosition));
				bVar3 = true;
			}
		}

		bVar2 = 0;
		if (iVar8 == CLevelScheduler::gThis->currentLevelID) {
			GetMarkerPositions(&local_20, aMStack2096);
			bVar2 = 1;
		}
		else {
			pCVar4 = CLevelScheduler::gThis->LoadMapSaveChunk(iVar8);
			if (pCVar4 != (SaveDataChunk_BLMP*)0x0) {
				local_20.field_0x0 = pCVar4->mapDataSizes.field_0x0;
				local_20.nbTeleporters = pCVar4->mapDataSizes.nbTeleporters;
				local_20.nbWolfen = pCVar4->mapDataSizes.nbWolfen;
				local_20.nbRunes = pCVar4->mapDataSizes.nbRunes;
				iVar8 = pCVar4->mapDataSizes.field_0x0 + pCVar4->mapDataSizes.nbTeleporters + pCVar4->mapDataSizes.nbWolfen + pCVar4->mapDataSizes.nbRunes;
				if (0x80 < iVar8) {
					iVar8 = 0x80;
				}
				memcpy(aMStack2096, &pCVar4->aPositions, iVar8 * sizeof(MapPosition));
				bVar2 = 1;
			}
		}

		if (bVar3 && bVar2) {
			if (local_30.field_0x0 == local_20.field_0x0) {
				if (local_30.nbTeleporters == local_20.nbTeleporters) {
					if (local_30.nbWolfen == local_20.nbWolfen) {
						if (local_30.nbRunes == local_20.nbRunes) {
							iVar8 = 0;
							while (iVar8 < 128) {
								if ((aMStack4144[iVar8].position.x != aMStack2096[iVar8].position.x) || (aMStack4144[iVar8].position.y != aMStack2096[iVar8].position.y)) {
									break;
								}

								iVar8 = iVar8 + 1;
							}

							if (iVar8 == 128) {
								memcpy(pMarkerBuffer, aMStack4144, 128 << 4);
								pMarkerCounts->field_0x0 = local_30.field_0x0;
								pMarkerCounts->nbTeleporters = local_30.nbTeleporters;
								pMarkerCounts->nbWolfen = local_30.nbWolfen;
								pMarkerCounts->nbRunes = local_30.nbRunes;
								bVar10 = true;
							}
						}
					}
				}
			}
		}

		if (static_cast<bool>(bVar3 | bVar2)) {
			if (static_cast<bool>(bVar3 & bVar2)) {
				IMPLEMENTATION_GUARD(
				local_8[0] = aMStack4144;
				iVar8 = 0;
				local_8[1] = aMStack2096;
				iVar9 = 0;
				do {
					pMarkerCounts->field_0x0 = 0;
					iVar7 = 0;
					piVar6 = static_cast<int*>((int)&local_30.field_0x0 + iVar9);
					ppMVar5 = local_8;
					do {
						pMarkerCounts->field_0x0 = pMarkerCounts->field_0x0 + *piVar6;
						memcpy(pMarkerBuffer, *ppMVar5, *piVar6 << 4);
						iVar7 = iVar7 + 1;
						*ppMVar5 = *ppMVar5 + *piVar6;
						iVar1 = *piVar6;
						ppMVar5 = ppMVar5 + 1;
						piVar6 = piVar6 + 4;
						pMarkerBuffer = pMarkerBuffer + iVar1;
					} while (iVar7 < 2);
					iVar8 = iVar8 + 1;
					pMarkerCounts = (MapDataSizes*)&pMarkerCounts->nbTeleporters;
					iVar9 = iVar9 + 4;
				} while (iVar8 < 4);)
			}
			else {
				if (bVar3) {
					pMarkerCounts->field_0x0 = local_30.field_0x0;
					pMarkerCounts->nbTeleporters = local_30.nbTeleporters;
					pMarkerCounts->nbWolfen = local_30.nbWolfen;
					pMarkerCounts->nbRunes = local_30.nbRunes;
					iVar8 = local_30.field_0x0 + local_30.nbTeleporters + local_30.nbWolfen + local_30.nbRunes;
					if (param_5 < iVar8) {
						iVar8 = param_5;
					}

					memcpy(pMarkerBuffer, aMStack4144, iVar8 * sizeof(MapPosition));
				}
				else {
					pMarkerCounts->field_0x0 = local_20.field_0x0;
					pMarkerCounts->nbTeleporters = local_20.nbTeleporters;
					pMarkerCounts->nbWolfen = local_20.nbWolfen;
					pMarkerCounts->nbRunes = local_20.nbRunes;
					iVar8 = local_20.field_0x0 + local_20.nbTeleporters + local_20.nbWolfen + local_20.nbRunes;
					if (param_5 < iVar8) {
						iVar8 = param_5;
					}

					memcpy(pMarkerBuffer, aMStack2096, iVar8 * sizeof(MapPosition));
				}
			}

			bVar10 = true;
		}
	}

	return bVar10;
}

void CMapManager::GetMarkerPositions(MapDataSizes* pMarkerCounts, MapPosition* pMarkerBuffer)
{
	edF32VECTOR4* peVar1;
	CActorTeleporter* pTeleporter;
	CActorRune* pRune;
	CActorManager* pActorManager;
	bool bVar2;
	int iVar3;
	long lVar4;
	MapStruct_8* pMVar5;
	int iVar6;
	CActor** pCVar7;
	CActor* piVar8;
	float fVar9;
	float fVar10;
	CActorsTable local_220;
	CActorsTable local_110;

	iVar6 = 0;

	pMarkerCounts->field_0x0 = 0;
	pMarkerCounts->nbTeleporters = 0;
	pMarkerCounts->nbWolfen = 0;
	pMarkerCounts->nbRunes = 0;
	pMarkerCounts->field_0x0 = INT_004497ec;

	if (0 < INT_004497ec) {
		pMVar5 = MapStruct_8_ARRAY_0049ca70;
		do {
			iVar6 = iVar6 + 1;
			pMarkerBuffer->field_0xc = pMVar5->field_0x4;
			peVar1 = pMVar5->pVector;
			pMarkerBuffer->position = peVar1->xyz;
			pMarkerBuffer = pMarkerBuffer + 1;
		} while (iVar6 < INT_004497ec);
	}

	// Teleporters.
	pActorManager = CScene::ptable.g_ActorManager_004516a4;
	local_110.nbEntries = 0;
	CScene::ptable.g_ActorManager_004516a4->GetActorsByClassID(TELEPORTER, &local_110);
	iVar6 = 0;
	if (0 < local_110.nbEntries) {
		pCVar7 = local_110.aEntries;
		do {
			pTeleporter = static_cast<CActorTeleporter*>(*pCVar7);
			bVar2 = pTeleporter->FUN_002edba0();
			if (bVar2 == false) {
				iVar3 = pTeleporter->FUN_002edb10();
				if (iVar3 < 1) {
					pMarkerBuffer->field_0xc = MARKER_TYPE_RUNE;
				}
				else {
					iVar3 = pTeleporter->FUN_002edaa0();
					pMarkerBuffer->field_0xc = iVar3;
				}

				pMarkerBuffer->position = (pTeleporter->field_0x270).xyz;
				pMarkerBuffer = pMarkerBuffer + 1;
				pMarkerCounts->nbTeleporters = pMarkerCounts->nbTeleporters + 1;
			}

			iVar6 = iVar6 + 1;
			pCVar7 = pCVar7 + 1;
		} while (iVar6 < local_110.nbEntries);
	}

	// Wolfen.
	if ((this->bHasWolfenMap != 0) && (iVar6 = 0, 0 < pActorManager->nbActors)) {
		do {
			piVar8 = (CActor*)0x0;
			if (iVar6 != -1) {
				piVar8 = pActorManager->aActors[iVar6];
			}

			CActorWolfen* pWolfen = static_cast<CActorWolfen*>(piVar8);
			if ((((piVar8->IsKindOfObject(0x10) != false) && (pWolfen->exorcisedState != 2)) && ((pWolfen->flags & 0x2000000U) == 0)) && (pWolfen->field_0xb74 != 5)) {
				pMarkerBuffer->field_0xc = MARKER_TYPE_RUNE;
				pMarkerBuffer->position = pWolfen->currentLocation.xyz;
				pMarkerBuffer = pMarkerBuffer + 1;
				pMarkerCounts->nbWolfen = pMarkerCounts->nbWolfen + 1;
			}

			iVar6 = iVar6 + 1;
		} while (iVar6 < pActorManager->nbActors);
	}

	// Runes.
	local_220.nbEntries = 0;
	pActorManager->GetActorsByClassID(RUNE, &local_220);
	iVar6 = 0;
	if (0 < local_220.nbEntries) {
		pCVar7 = local_220.aEntries;
		do {
			pRune = static_cast<CActorRune*>(*pCVar7);
			bVar2 = pRune->HasBeenCollected();
			if (bVar2 == false) {
				pMarkerBuffer->field_0xc = MARKER_TYPE_RUNE;
				pMarkerBuffer->position = pRune->currentLocation.xyz;
				pMarkerBuffer = pMarkerBuffer + 1;
				pMarkerCounts->nbRunes = pMarkerCounts->nbRunes + 1;
			}

			iVar6 = iVar6 + 1;
			pCVar7 = pCVar7 + 1;
		} while (iVar6 < local_220.nbEntries);
	}

	return;
}

bool CMapManager::HasRouteToLevel(int levelId, int markerType, int param_4)
{
	int iVar1;
	int iVar2;
	bool bVar3;
	SaveDataChunk_BLMP* pSVar4;
	MapPosition* pMVar5;
	int iVar6;
	MapPosition* pMVar7;

	if ((CLevelScheduler::gThis->currentLevelID != levelId) && (markerType != 0x10)) {
		bVar3 = levelId == markerType;
		if (!bVar3) {
			bVar3 = markerType == INT_ARRAY_0048ed60[levelId];
		}

		if (!bVar3) {
			bVar3 = levelId == INT_ARRAY_0048ed60[markerType];
		}

		if (bVar3) {
			return true;
		}

		if ((param_4 != 0) && (pSVar4 = CLevelScheduler::gThis->LoadMapSaveChunk(markerType), pSVar4 != (SaveDataChunk_BLMP*)0x0)) {
			iVar1 = (pSVar4->mapDataSizes).field_0x0;
			pMVar5 = pSVar4->aPositions;
			iVar6 = 0;
			pMVar7 = pMVar5 + iVar1;
			if (0 < (pSVar4->mapDataSizes).nbTeleporters) {
				do {
					iVar2 = pMVar7->field_0xc;
					if (iVar2 != 0x10) {
						bVar3 = levelId == iVar2;

						if (!bVar3) {
							bVar3 = iVar2 == INT_ARRAY_0048ed60[levelId];
						}

						if (!bVar3) {
							bVar3 = levelId == INT_ARRAY_0048ed60[iVar2];
						}

						if (bVar3) {
							return true;
						}
					}

					iVar6 = iVar6 + 1;
					pMVar7 = pMVar7 + 1;
				} while (iVar6 < (pSVar4->mapDataSizes).nbTeleporters);
			}

			iVar6 = 0;
			if (0 < iVar1) {
				do {
					iVar1 = pMVar5->field_0xc;

					if (iVar1 != 0x10) {
						bVar3 = levelId == iVar1;
						if (!bVar3) {
							bVar3 = iVar1 == INT_ARRAY_0048ed60[levelId];
						}

						if (!bVar3) {
							bVar3 = levelId == INT_ARRAY_0048ed60[iVar1];
						}

						if (bVar3) {
							return true;
						}
					}

					iVar6 = iVar6 + 1;
					pMVar5 = pMVar5 + 1;
				} while (iVar6 < (pSVar4->mapDataSizes).field_0x0);
			}
		}
	}
	return false;
}

int CMapManager::ClipPoly(S_2DVERTEX* param_2, S_2DVERTEX* param_3, int nbCount, uint index)
{
	bool bVar1;
	int* piVar2;
	S_2DVERTEX* pSVar3;
	S_2DVERTEX* pSVar4;
	int* piVar5;
	S_2DVERTEX* pSVar6;
	S_2DVERTEX* pSVar7;
	uint uVar8;
	int* piVar9;
	float* pfVar10;
	float* pfVar11;
	int iVar12;
	float* pfVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float pMVar17;
	float pMVar18;
	float local_10[2];
	int local_8[2];

	pMVar17 = this->field_0x310.raw[index];
	pMVar18 = this->field_0x320.raw[index];

	uVar8 = index ^ 1;
	pSVar4 = param_2;

	if (param_3 < param_3 + nbCount) {
		pfVar11 = param_3->raw + index;
		pfVar13 = param_3->raw + uVar8;
		pSVar3 = param_3;
		pSVar7 = param_3 + nbCount + -1;

		do {
			pSVar6 = pSVar3;
			memset(local_8, 0, sizeof(local_8));
			
			fVar14 = *pfVar11;
			fVar15 = pSVar7->raw[index];
			bVar1 = false;
			if (fVar14 < pMVar17) {
				if (pMVar17 <= fVar15) {
					if (pMVar18 < fVar15) {
						local_8[0] = 1;
						local_10[0] = pMVar18;
					}

					local_8[1] = 1;
					local_10[1] = pMVar17;
				}
			}
			else {
				if (pMVar18 < fVar14) {
					if (fVar15 <= pMVar18) {
						if (fVar15 < pMVar17) {
							local_8[0] = 1;
							local_10[0] = pMVar17;
						}

						local_8[1] = 1;
						local_10[1] = pMVar18;
					}
				}
				else {
					if (fVar15 < pMVar17) {
						local_8[0] = 1;
						local_10[0] = pMVar17;
					}
					else {
						if (pMVar18 < fVar15) {
							local_8[0] = 1;
							local_10[0] = pMVar18;
						}
					}

					bVar1 = true;
				}
			}

			iVar12 = 0;
			piVar9 = local_8;
			pfVar10 = local_10;
			do {
				if (*piVar9 != 0) {
					fVar16 = (*pfVar10 - fVar15) / (fVar14 - fVar15);
					pSVar4->raw[index] = *pfVar10;
					pSVar4->raw[uVar8] = pSVar7->raw[uVar8] + fVar16 * (*pfVar13 - pSVar7->raw[uVar8]);
					pSVar4->s1 = pSVar7->s1 + fVar16 * (pSVar6->s1 - pSVar7->s1);
					pSVar4->t1 = pSVar7->t1 + fVar16 * (pSVar6->t1 - pSVar7->t1);
					pSVar4 = pSVar4 + 1;
				}

				iVar12 = iVar12 + 1;
				piVar9 = piVar9 + 1;
				pfVar10 = pfVar10 + 1;
			} while (iVar12 < 2);

			if (bVar1) {
				fVar14 = pSVar6->y1;
				pSVar4->x1 = pSVar6->x1;
				pSVar4->y1 = fVar14;
				fVar14 = pSVar6->t1;
				pSVar4->s1 = pSVar6->s1;
				pSVar4->t1 = fVar14;
				pSVar4 = pSVar4 + 1;
			}

			pfVar11 = pfVar11 + 4;
			pfVar13 = pfVar13 + 4;
			pSVar3 = pSVar6 + 1;
			pSVar7 = pSVar6;
		} while (pSVar6 + 1 < param_3 + nbCount);
	}

	int diff = ((char*)pSVar4 - (char*)param_2);
	if (diff < 0) {
		pSVar4 = (S_2DVERTEX*)((int)&pSVar4->t1 + 3);
	}

	return diff / sizeof(S_2DVERTEX);
}

void CMapManager::DrawSprite(edDList_material* pMaterial, S_2DRECT* pRect, uint color)
{
	byte r;
	byte g;
	byte b;
	byte a;

	edDListUseMaterial(pMaterial);
	edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);

	g = (byte)(color >> 8);
	b = (byte)(color >> 0x10);
	a = (byte)(color >> 0x18);
	r = (byte)color;
	edDListColor4u8(r, g, b, a);

	edDListLoadIdentity();
	edDListTexCoo2f(pRect->aVertices[0].s1, pRect->aVertices[0].t1);
	edDListVertex4f(pRect->aVertices[0].x1, pRect->aVertices[0].y1, 0.0f, 0.0f);
	edDListTexCoo2f(pRect->aVertices[1].s1, pRect->aVertices[1].t1);
	edDListVertex4f(pRect->aVertices[1].x1, pRect->aVertices[1].y1, 0.0f, 0.0f);
	edDListTexCoo2f(pRect->aVertices[3].s1, pRect->aVertices[3].t1);
	edDListVertex4f(pRect->aVertices[3].x1, pRect->aVertices[3].y1, 0.0f, 0.0f);
	edDListTexCoo2f(pRect->aVertices[2].s1, pRect->aVertices[2].t1);
	edDListVertex4f(pRect->aVertices[2].x1, pRect->aVertices[2].y1, 0.0f, 0.0f);
	edDListEnd();

	return;
}

void CMapManager::DrawClippedSprite(edDList_material* pMaterial, S_2DRECT* pRect, uint color)
{
	int nbVertices;
	S_2DVERTEX* pVertexIt;
	S_2DVERTEX outVerticesB[12];
	S_2DVERTEX outVerticesA[12];
	_rgba rgbColor;

	rgbColor = color;

	nbVertices = ClipPoly(outVerticesA, pRect->aVertices, 4, 0);
	nbVertices = ClipPoly(outVerticesB, outVerticesA, nbVertices, 1);

	if (nbVertices != 0) {
		edDListUseMaterial(pMaterial);
		edDListBegin(0.0f, 0.0f, 0.0f, 5, nbVertices);
		edDListColor4u8(rgbColor.r, rgbColor.g, rgbColor.b, rgbColor.a);
		edDListLoadIdentity();

		for (pVertexIt = outVerticesB; pVertexIt < outVerticesB + nbVertices; pVertexIt = pVertexIt + 1) {
			edDListTexCoo2f(pVertexIt->s1, pVertexIt->t1);
			edDListVertex4f(pVertexIt->x1, pVertexIt->y1, 0.0f, 0.0f);
		}

		edDListEnd();
	}

	return;
}

int CMapManager::FUN_003f9040()
{
	int iVar1;

	iVar1 = 0x62;
	if (this->field_0x394 != 0) {
		iVar1 = this->field_0x14;
	}

	return iVar1;
}

edDList_material* CMapManager::GetDlistMaterial(int index)
{
	edDList_material* peVar1;

	peVar1 = (edDList_material*)0x0;
	if (this->field_0x300 != 0) {
		peVar1 = this->field_0xf0 + index;
	}

	return peVar1;
}

edF32VECTOR4 edF32VECTOR4_004279f0 = { 0.0f, 0.0f, 0.2f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00427a00 = { 0.0f, 0.0f, 0.8f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00427a10 = { 0.0f, 0.0f, 0.8f, 1.0f };
edF32VECTOR4 edF32VECTOR4_00427a20 = { 0.0f, 0.0f, 0.2f, 1.0f };

void CMapManager::DrawButtons(char* pText)
{
	edCTextStyle* pNewFont;
	char* pcVar1;
	edCTextStyle textStyle;
	float local_50;
	float local_4c;
	float local_48;
	float local_44;
	float local_40;
	float local_3c;
	float local_38;
	float local_34;
	float local_30;
	float local_2c;
	float local_28;
	float local_24;
	float local_20;
	float local_1c;
	float local_18;
	float local_14;
	_rgba local_4;

	local_50 = edF32VECTOR4_004279f0.x;
	local_48 = edF32VECTOR4_004279f0.z;
	local_44 = edF32VECTOR4_004279f0.w;
	local_38 = edF32VECTOR4_00427a00.z;
	local_34 = edF32VECTOR4_00427a00.w;
	local_28 = edF32VECTOR4_00427a10.z;
	local_24 = edF32VECTOR4_00427a10.w;
	local_20 = edF32VECTOR4_00427a20.x;
	local_18 = edF32VECTOR4_00427a20.z;
	local_14 = edF32VECTOR4_00427a20.w;
	local_4 = 0x80808080;
	local_4c = (static_cast<float>(gVideoConfig.screenHeight) * 463.0f) / 512.0f;
	local_2c = (static_cast<float>(gVideoConfig.screenHeight) * 502.0f) / 512.0f;
	local_40 = static_cast<float>(gVideoConfig.screenWidth);
	local_3c = local_4c;
	local_30 = local_40;
	local_1c = local_2c;
	edDListUseMaterial(&MenuBitmaps[8].materialInfo);
	edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);
	edDListColor4u8(local_4.r, local_4.g, local_4.b, local_4.a);
	edDListLoadIdentity();
	edDListTexCoo2f(local_48, local_44);
	edDListVertex4f(local_50, local_4c, 0.0f, 0.0f);
	edDListTexCoo2f(local_38, local_34);
	edDListVertex4f(local_40, local_3c, 0.0f, 0.0f);
	edDListTexCoo2f(local_18, local_14);
	edDListVertex4f(local_20, local_1c, 0.0f, 0.0f);
	edDListTexCoo2f(local_28, local_24);
	edDListVertex4f(local_30, local_2c, 0.0f, 0.0f);
	edDListEnd();
	textStyle.Reset();
	textStyle.SetShadow(0x100);
	textStyle.alpha = 0xff;
	textStyle.rgbaColour = 0xb6bbfcff;
	textStyle.SetVerticalAlignment(8);
	textStyle.SetFont(BootDataFont, false);
	textStyle.SetScale(0.80000001f, 0.80000001f);
	pNewFont = edTextStyleSetCurrent(&textStyle);
	textStyle.SetHorizontalAlignment(0);
	edTextDraw(25.0f, ((static_cast<float>(gVideoConfig.screenHeight) * 463.0f) / 512.0f + (static_cast<float>(gVideoConfig.screenHeight) * 502.0f) / 512.0f) / 2.0f, pText);
	textStyle.SetHorizontalAlignment(1);
	edCTextFormat textFormat;
	pcVar1 = gMessageManager.get_message(0x1203111c03454c50);
	textFormat.FormatString("%[BLINK]k%[MAP]b%[RESET]K %s", pcVar1);
	textFormat.Display(static_cast<float>(gVideoConfig.screenWidth) - 25.0f, ((static_cast<float>(gVideoConfig.screenHeight) * 463.0f) / 512.0f + (static_cast<float>(gVideoConfig.screenHeight) * 502.0f) / 512.0f) / 2.0f);
	edTextStyleSetCurrent(pNewFont);

	return;
}

void CMapManager::Game_Init()
{
	(this->field_0x58).field_0x0.x = 0.0f;
	(this->field_0x58).field_0x0.y = -1.0f;
	(this->field_0x58).field_0x0.z = 0.0f;

	(this->field_0x58).field_0xc.x = 0.0f;
	(this->field_0x58).field_0xc.y = 0.0f;
	(this->field_0x58).field_0xc.z = 0.0f;

	(this->field_0x58).field_0x18.x = 0.0f;
	(this->field_0x58).field_0x18.y = 1.0f;
	(this->field_0x58).field_0x18.z = 0.0f;

	(this->field_0x58).field_0x24 = 1.0f;
	(this->field_0x58).field_0x28 = 0;

	this->field_0x310.x = 0.0f;
	this->field_0x310.y = 0.0f;
	this->field_0x310.z = 0.0f;
	this->field_0x310.w = 1.0f;

	this->field_0x320.x = static_cast<float>(gVideoConfig.screenWidth);
	this->field_0x320.y = static_cast<float>(gVideoConfig.screenHeight);
	this->field_0x320.z = 0.0f;
	this->field_0x320.w = 1.0f;

	this->field_0x24 = 0;
	this->field_0x28 = 0;
	this->field_0x2c = this->field_0x58;
	INT_004497ec = 0;
	this->field_0x300 = 0;
	this->field_0x334 = 0;
	this->field_0x330 = 0.0f;
	this->field_0x378 = 0.0f;
	this->field_0x10 = 0;
	this->field_0x14 = 0x62;
	this->field_0x18.index = -1;
	this->field_0x394 = 0;

	this->field_0x338.field_0x0 = 0x62;
	this->field_0x338.field_0x4 = 6;
	this->field_0x338.hash = 0;

	this->field_0x338.field_0x10 = 0x10;
	this->field_0x338.field_0x14 = 0;
	this->field_0x338.hash_2 = 0;

	this->field_0x358 = this->field_0x338;

	this->field_0x334 = 0;
	this->field_0x330 = 0.0f;
	this->field_0x37c = 0.0f;
	this->field_0x380 = 1;
	this->field_0x384 = 0.0f;
	this->pLevelMap = new CLevelMap;
	this->pWorldMap = new CWorldMap;
	this->field_0x390 = 0;
	this->bHasWolfenMap = 0;
	this->field_0x388 = 1.0f;

	return;
}

void CMapManager::Game_Term()
{
	IMPLEMENTATION_GUARD();
}

void CMapManager::Level_Init()
{
	uint uVar1;
	int iVar2;

	this->field_0x388 = (static_cast<float>(gVideoConfig.screenWidth) / static_cast<float>(gVideoConfig.screenHeight)) / (CScene::ptable.g_CameraManager_0045167c)->aspectRatio;
	this->field_0x18.Init();
	if ((((this->field_0x394 == 0) && (this->field_0x18.Get() != (ed_zone_3d*)0x0)) && (CActorHero::_gThis != (CActorHero*)0x0)) &&
		(uVar1 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->field_0x18.Get(), &CActorHero::_gThis->currentLocation, 0), (uVar1 & 1) != 0)) {
		this->field_0x394 = 1;
	}

	if (this->field_0x14 == 0x62) {
		iVar2 = 2;
	}
	else {
		iVar2 = CLevelScheduler::ScenVar_Get(this->field_0x14);
	}

	this->bHasWolfenMap = static_cast<uint>(iVar2 == 2);
	this->field_0x390 = 0;

	return;
}

void CMapManager::Level_Term()
{
	//FUN_003c0a40(); Stub
	//FUN_003d5100(); Stub

	this->field_0x24 = 0;
	this->field_0x28 = 0;
	this->field_0x2c = this->field_0x58;
	INT_004497ec = 0;
	this->field_0x300 = 0;
	this->field_0x334 = 0;
	this->field_0x330 = 0.0f;
	this->field_0x378 = 0.0f;
	this->field_0x10 = 0;
	this->field_0x14 = 0x62;
	(this->field_0x18).index = -1;
	this->field_0x394 = 0;

	return;
}

void CMapManager::Level_ClearAll()
{
	return;
}

float CObjectivesPanel::GetPanelHeight()
{
	bool bVar1;
	float fVar2;
	float fVar3;

	fVar2 = 0.0f;
	if (this->field_0x0 != 0x62) {
		bVar1 = false;
		fVar3 = (static_cast<float>(gVideoConfig.screenHeight) * 20.0f) / 512.0f;
		fVar2 = fVar3 + fVar3 / 4.0f + (static_cast<float>(gVideoConfig.screenHeight) * 30.0f) / 512.0f + fVar3 + 0.0f;
		switch (this->field_0x4) {
		case 0:
			fVar2 = fVar2 + fVar3;
			break;
		case 1:
			bVar1 = true;
			fVar2 = fVar2 + fVar3;
			break;
		case 2:
			bVar1 = true;
			fVar2 = fVar2 + fVar3 * 2.0f;
			break;
		case 3:
			fVar2 = fVar2 + fVar3;
		case 4:
			fVar2 = fVar2 + fVar3;
		default:
			bVar1 = true;
		}
		if ((bVar1) && (this->hash != 0)) {
			fVar2 = fVar2 + fVar3;
		}
		fVar2 = fVar2 - (static_cast<float>(gVideoConfig.screenHeight) * 4.0f) / 512.0f;
	}

	return fVar2;
}

void CObjectivesPanel::DrawTextRevealUnderline(float param_2, float param_3, float param_4, edCTextFormat* pTextFormat, uint color)
{
	float x;
	float x_00;
	byte local_4;
	byte bStack3;
	byte bStack2;
	byte bStack1;

	edDListUseMaterial((edDList_material*)0x0);
	edDListBlendSet(1);
	edDListBlendFunc_002ca830();
	edDListBegin(0.0f, 0.0f, 0.0f, 8, 4);
	bStack3 = (byte)(color >> 8);
	bStack2 = (byte)(color >> 0x10);
	bStack1 = (byte)(color >> 0x18);
	local_4 = (byte)color;
	edDListColor4u8(local_4, bStack3, bStack2, bStack1);
	edDListLoadIdentity();
	edTextStyleGetCurrent();
	x_00 = (param_2 + pTextFormat->offsetX_0x0 * 0.7f) - 2.0f;
	x = x_00 + param_4 * pTextFormat->field_0x8 * 0.7f + 2.0f;
	edDListVertex4f(x_00, param_3 - 1.0f, 0.0f, 0.0f);
	edDListVertex4f(x_00, param_3 + 1.0f, 0.0f, 0.0f);
	edDListVertex4f(x, param_3 - 1.0f, 0.0f, 0.0f);
	edDListVertex4f(x, param_3 + 1.0f, 0.0f, 0.0f);
	edDListEnd();

	return;
}

void CObjectivesPanel::DrawObjectiveTextLine(float param_1, float param_2, char* pText, int param_5, uint color, uint param_7, int param_8)
{
	edCTextStyle* pTextStyle;
	uint uVar2;
	long lVar4;
	float fVar5;
	float fVar6;
	char acStack6432[1024];
	_rgba local_4;

	lVar4 = static_cast<long>(param_5);
	pTextStyle = edTextStyleGetCurrent();
	edCTextFormat textFormat;
	local_4 = color;

	if (param_7 != 0) {
		if (param_8 == 0) {
			if (param_2 < 0.1f) {
				fVar5 = (param_2 * 255.0f) / 0.1f;

				if (2.147484e+09f <= fVar5) {
					fVar5 = fVar5 - 2.147484e+09f;
				}

				uVar2 = static_cast<uint>(fVar5);
			}
			else {
				uVar2 = 0xff;
			}
		}
		else {
			fVar5 = param_2 * 896.0f;
			if (fVar5 < 2.147484e+09f) {
				uVar2 = static_cast<uint>(fVar5);
			}
			else {
				uVar2 = static_cast<int>(fVar5 - 2.147484e+09f) | 0x80000000;
			}

			uVar2 = static_cast<int>(uVar2) % 0x140;
			if (0xff < uVar2) {
				uVar2 = (0x40 - (uVar2 - 0x100)) * 0xff >> 6;
			}
		}

		local_4 = color & 0xffffff | (uVar2 >> 1) << 0x18;
	}

	pTextStyle->rgbaColour = static_cast<uint>(local_4.a) << 1 | (local_4.rgba >> 0x10 & 0xff) << 9 | local_4.rgba << 0x19 | (local_4.rgba >> 8 & 0xff) << 0x11;
	pTextStyle->alpha = static_cast<uint>(local_4.a) << 1;

	if (static_cast<long>(param_5) == 0) {
		fVar5 = 37.0f;
		textFormat.FormatString("- %s", pText);
	}
	else {
		fVar5 = 27.0f;
		edStrCopyUpper(acStack6432, pText);
		textFormat.FormatString(acStack6432);
	}

	textFormat.Display(fVar5, param_1);

	if ((param_8 == 0) && (param_7 != 0)) {
		if (param_2 < 0.5f) {
			fVar6 = 1.0f;
		}
		else {
			fVar6 = cosf((1.0f - param_2) * 1.570796f * 2.0f - 1.570796f);
		}

		DrawTextRevealUnderline(fVar5, param_1, fVar6, &textFormat, local_4.rgba);
	}

	return;
}

void CObjectivesPanel::DrawObjectiveDetails(float param_1, float param_2, uint param_4, int param_5)
{
	int iVar1;
	int iVar2;
	ObjectiveEntry* pLoadLoopObjj;
	edCTextStyle* pNewFont;
	char* pcVar3;
	char* pcVar4;
	float fVar5;
	float fVar6;
	char acStack1216[1024];
	edCTextStyle textStyle;
	CLevelScheduler* pLevelScheduler;

	pLevelScheduler = CLevelScheduler::gThis;
	pLoadLoopObjj = (ObjectiveEntry*)0x0;
	if (this->field_0x0 != 0x62) {
		pLoadLoopObjj = CLevelScheduler::gThis->FUN_002d9f30(this->field_0x0);
	}

	if (pLoadLoopObjj != (ObjectiveEntry*)0x0) {
		textStyle.Reset();
		textStyle.SetShadow(0x100);
		textStyle.SetVerticalAlignment(8);
		textStyle.SetHorizontalAlignment(0);
		textStyle.SetFont(BootDataFont, false);
		textStyle.SetScale(0.69999999f, 0.69999999f);
		pNewFont = edTextStyleSetCurrent(&textStyle);
		fVar6 = (static_cast<float>(gVideoConfig.screenHeight) * 20.0f) / 512.0f;
		fVar6 = param_1 + fVar6 / 4.0f + (static_cast<float>(gVideoConfig.screenHeight) * 30.0f) / 512.0f + fVar6;
		pcVar3 = gMessageManager.get_message(pLoadLoopObjj->messageKey);
		DrawObjectiveTextLine(fVar6, param_2, pcVar3, 1, 0x7f7f7f7f, param_4 & 0x40, param_5);
		fVar6 = fVar6 + (static_cast<float>(gVideoConfig.screenHeight) * 20.0f) / 512.0f;
		switch (this->field_0x4) {
		case 0:
			iVar2 = pLoadLoopObjj->levelId;
			pcVar3 = gMessageManager.get_message(0x44544b46584e454e);
			pcVar4 = gMessageManager.get_message(pLevelScheduler->aLevelInfo[iVar2].titleMsgHash);
			sprintf(acStack1216, pcVar3, pcVar4);
			DrawObjectiveTextLine(fVar6, param_2, acStack1216, 0, 0x7f7f7f7f, param_4 & 1, param_5);
			break;
		case 1:
			pcVar3 = gMessageManager.get_message(0x424f140a1d18100d);
			DrawObjectiveTextLine(fVar6, param_2, pcVar3, 0, 0x7f613021, param_4 & 2, param_5);
			fVar5 = static_cast<float>(gVideoConfig.screenHeight);
			if (this->hash != 0) {
				pcVar3 = gMessageManager.get_message(this->hash);
				DrawObjectiveTextLine(fVar6 + (fVar5 * 20.0f) / 512.0f, param_2, pcVar3, 0, 0x7f7f7f7f, param_4 & 0x20, param_5);
			}
			break;
		case 2:
			iVar2 = this->field_0x10;
			if (iVar2 == 0x10) {
				iVar2 = this->field_0x14;
				if (iVar2 == 1) {
					pcVar3 = gMessageManager.get_message(0x11061c140211164b);
					sprintf(acStack1216, pcVar3);
				}
				else {
					pcVar3 = gMessageManager.get_message(0x11061c4b4d5f534b);
					sprintf(acStack1216, pcVar3, iVar2);
				}

				DrawObjectiveTextLine(fVar6, param_2, acStack1216, 0, 0x7f112e7c, param_4 & 4, param_5);
			}
			else {
				iVar1 = this->field_0x14;
				if (iVar1 == 1) {
					pcVar3 = gMessageManager.get_message(0x514d46550d1b0517);
					pcVar4 = gMessageManager.get_message(pLevelScheduler->aLevelInfo[iVar2].titleMsgHash);
					sprintf(acStack1216, pcVar3, pcVar4);
				}
				else {
					pcVar3 = gMessageManager.get_message(0xe0208100d1b0517);
					pcVar4 = gMessageManager.get_message(pLevelScheduler->aLevelInfo[iVar2].titleMsgHash);
					sprintf(acStack1216, pcVar3, pcVar4, iVar1);
				}

				DrawObjectiveTextLine(fVar6, param_2, acStack1216, 0, 0x7f112e7c, param_4 & 4, param_5);
			}
			fVar6 = fVar6 + (static_cast<float>(gVideoConfig.screenHeight) * 20.0f) / 512.0f;
		case 3:
			if (this->field_0x4 != 2) {
				iVar2 = this->field_0x14;
				if (iVar2 == 1) {
					pcVar3 = gMessageManager.get_message(0x1d0c140e49454155);
					sprintf(acStack1216, pcVar3);
				}
				else {
					pcVar3 = gMessageManager.get_message(0x42435a4b49454155);
					sprintf(acStack1216, pcVar3, iVar2);
				}
				DrawObjectiveTextLine(fVar6, param_2, acStack1216, 0, 0x7f112e7c, param_4 & 8, param_5);
				fVar6 = fVar6 + (static_cast<float>(gVideoConfig.screenHeight) * 20.0f) / 512.0f;
			}
		case 4:
			pcVar3 = "UNKNOWN";
			if (this->hash_2 != 0) {
				pcVar3 = gMessageManager.get_message(this->hash_2);
			}

			pcVar4 = gMessageManager.get_message(0x585d504e5246101a);
			sprintf(acStack1216, pcVar4, pcVar3);
			DrawObjectiveTextLine(fVar6, param_2, acStack1216, 0, 0x7f2f777f, param_4 & 0x10, param_5);
			fVar6 = fVar6 + (static_cast<float>(gVideoConfig.screenHeight) * 20.0f) / 512.0f;
		default:
			if (this->hash != 0) {
				pcVar3 = gMessageManager.get_message(this->hash);
				DrawObjectiveTextLine(fVar6, param_2, pcVar3, 0, 0x7f7f7f7f, param_4 & 0x20, param_5);
			}
		}

		edTextStyleSetCurrent(pNewFont);
	}

	return;
}

uint CObjectivesPanel::GetObjectiveLineMask()
{
	bool bVar1;
	uint uVar2;

	uVar2 = 0;
	if (this->field_0x0 != 0x62) {
		uVar2 = 0x40;
		bVar1 = false;

		switch (this->field_0x4) {
		case 0:
			uVar2 = 0x41;
			break;
		case 1:
			uVar2 = 0x42;
			bVar1 = true;
			break;
		case 2:
			uVar2 = 0x44;
		case 3:
			uVar2 = uVar2 | 8;
		case 4:
			uVar2 = uVar2 | 0x10;
		default:
			bVar1 = true;
		}

		if ((bVar1) && (this->hash != 0)) {
			uVar2 = uVar2 | 0x20;
		}
	}

	return uVar2;
}

void CMapManager::Level_Manage()
{
	ed_zone_3d* pZone;
	uint uVar1;
	int iVar2;
	ulong uVar4;
	ulong uVar5;
	ulong uVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	Func_003f8d80();

	if (this->bHasWolfenMap == 0) {
		if ((((this->field_0x394 == 0) && (pZone = (this->field_0x18).Get(), pZone != (ed_zone_3d*)0x0)) && (CActorHero::_gThis != (CActorHero*)0x0)) &&
			(uVar1 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0), (uVar1 & 1) != 0)) {
			this->field_0x394 = 1;
		}

		if (this->field_0x14 == 0x62) {
			iVar2 = 2;
		}
		else {
			iVar2 = CLevelScheduler::ScenVar_Get(this->field_0x14);
		}

		this->bHasWolfenMap = static_cast<uint>(iVar2 == 2);
		if (this->bHasWolfenMap != 0) {
			this->field_0x390 = 1;
		}
	}

	if ((GameFlags & 0x10) != 0) {
		this->field_0x334 = 0;
		this->field_0x330 = 0.0f;
		fVar9 = GetTimer()->totalTime;
		fVar8 = 1.0f;

		if (this->field_0x37c != 0.0f) {
			fVar8 = GetTimer()->totalTime - this->field_0x37c;
			if (fVar8 <= 2.0f) {
				fVar8 = (fVar8 / 2.0f) * 0.5f;
			}
			else {
				fVar8 = ((fVar8 - 2.0f) * 0.5f) / 3.5f + 0.5f;
			}

			uVar4 = this->field_0x338.GetObjectiveLineMask();
			uVar5 = this->field_0x358.GetObjectiveLineMask();
			uVar6 = this->field_0x338.GetObjectiveLineMask();
			uVar5 = uVar5 ^ uVar6;
			if (((uVar5 & 0x40) == 0) && (this->field_0x358.field_0x0 == this->field_0x338.field_0x0)) {
				if ((this->field_0x358.hash != this->field_0x338.hash) || ((uVar5 & 1) != 0)) {
					uVar5 = uVar5 | 0x3f;
				}
			}
			else {
				uVar5 = uVar5 | 0x7f;
			}

			if ((1.0f < fVar8) || ((0.6f <= fVar8 && ((uVar5 & uVar4) == 0)))) {
				this->field_0x37c = 0.0f;
				fVar8 = 1.0f;
				this->field_0x358 = this->field_0x338;
				if (this->field_0x380 == 0) {
					this->field_0x384 = fVar9;
				}
			}
		}

		fVar7 = this->field_0x338.GetPanelHeight();
		this->field_0x378 = fVar7;
		if (fVar8 < 1.0f) {
			fVar9 = this->field_0x358.GetPanelHeight();
			if (fVar8 < 0.4f) {
				this->field_0x378 = fVar9;
			}
			else {
				fVar7 = this->field_0x378;
				if (fVar9 < fVar7) {
					if (fVar8 < 0.5f) {
						fVar8 = (fVar8 - 0.4f) * 10.0f;
						this->field_0x378 = fVar9 * (1.0f - fVar8) + fVar7 * fVar8;
					}
				}
				else {
					if (fVar8 < 0.5f) {
						this->field_0x378 = fVar9;
					}
					else {
						if ((0.5f < fVar8) && (fVar8 < 0.6f)) {
							fVar8 = (fVar8 - 0.5f) * 10.0f;
							this->field_0x378 = fVar9 * (1.0f - fVar8) + fVar7 * fVar8;
						}
					}
				}
			}
		}
		else {
			if (((1.0f <= (fVar9 - this->field_0x384) / 0.5f) && (this->field_0x37c == 0.0f)) && ((gPlayerInput.pressedBitfield & PAD_BITMASK_SQUARE_ALT) != 0)) {
				(CScene::ptable.g_FrontendManager_00451680)->pFrontendSamplePlayer->PlaySample(1.0f, 2, 0);
				this->field_0x380 = this->field_0x380 ^ 1;
				this->field_0x384 = fVar9;
			}
		}

		if (this->field_0x24 == 0) {
			if (this->field_0x28 != 0) {
				IMPLEMENTATION_GUARD(
				this->pWorldMap->FUN_003d4220();)
			}
		}
		else {
			this->pLevelMap->Update();
		}
	}

	return;
}

void CMapManager::Level_ManagePaused()
{
	Level_Manage();
}

void CMapManager::Level_Draw()
{
	if ((GameFlags & 0x10) == 0) {
		DrawMapPromptHint();
	}
	else {
		if (this->field_0x24 == 0) {
			if (this->field_0x28 != 0) {
				this->pWorldMap->Draw();
			}
		}
		else {
			this->pLevelMap->Draw();
		}
	}
	return;
}

void CMapManager::Level_SaveContext()
{
	SaveDataChunk_BLMP* pMapChunk;
	int iVar1;
	float fVar2;
	float fVar3;
	CLevelScheduler* pLevelScheduler;

	pLevelScheduler = CLevelScheduler::gThis;
	if (this->field_0x4 != -1) {
		pMapChunk = static_cast<SaveDataChunk_BLMP*>(CLevelScheduler::gThis->SaveGame_BeginChunk(0x504d4c42));

		pMapChunk->saveChunk.field_0x0 = this->field_0x2c.field_0x0;
		pMapChunk->saveChunk.field_0xc = this->field_0x2c.field_0xc;
		pMapChunk->saveChunk.field_0x18 = this->field_0x2c.field_0x18;
		pMapChunk->saveChunk.field_0x24 = this->field_0x2c.field_0x24;
		pMapChunk->saveChunk.field_0x28 = this->field_0x2c.field_0x28;

		pMapChunk->field_0x2c = this->field_0xc;

		if (this->field_0x394 == 0) {
			iVar1 = 0x62;
		}
		else {
			iVar1 = this->field_0x14;
		}

		pMapChunk->field_0x30 = iVar1;
		GetMarkerPositions(&pMapChunk->mapDataSizes, pMapChunk->aPositions);
		pLevelScheduler->SaveGame_EndChunk(pMapChunk->aPositions + (pMapChunk->mapDataSizes).field_0x0 + (pMapChunk->mapDataSizes).nbTeleporters + (pMapChunk->mapDataSizes).nbWolfen + (pMapChunk->mapDataSizes).nbRunes);
	}

	return;
}

void CMapManager::Level_LoadContext()
{
	CChunk* pCVar1;
	CLevelScheduler* pLevelScheduler;

	pLevelScheduler = CLevelScheduler::gThis;
	pCVar1 = CLevelScheduler::gThis->SaveGame_OpenChunk(0x504d4c42);
	if (pCVar1 != (CChunk*)0x0) {
		if (this->field_0x14 != 0x62) {
			// Is this chunk + 1 or just chunk?
			SaveDataChunk_BLMP* pMapChunk = reinterpret_cast<SaveDataChunk_BLMP*>(pCVar1 + 1);
			if (pMapChunk->field_0x30 == 0x62) {
				this->field_0x394 = 0;
			}
			else {
				this->field_0x394 = 1;
			}
		}

		pLevelScheduler->SaveGame_CloseChunk();
	}

	return;
}

void CLevelMap::Level_PostInit()
{
	this->pMaterials = (edDList_material*)0x0;
	this->field_0x78 = 0;
	this->field_0x7c = 0;
	this->field_0x0 = 0x10;
	this->levelId = 0x10;
	this->field_0xd8 = 0.0f;
	this->field_0xe4 = 0;
	this->field_0xdc = 0;
	this->field_0xe0 = 0;

	return;
}

edF32VECTOR2 edF32VECTOR2_00426968 = { 0.0f, 0.0f };
edF32VECTOR2 edF32VECTOR2_00426978 = { 1.0f, 0.0f };
edF32VECTOR2 edF32VECTOR2_00426988 = { 1.0f, 1.0f };
edF32VECTOR2 edF32VECTOR2_00426998 = { 0.0f, 1.0f };

struct MapKeyEntry
{
	ulong key;
	int materialId;
};

MapKeyEntry MapKeyEntry_ARRAY_00437480[8] =
{
	{ 0x4D41505F4B594100, 0x0 },
	{ 0x1F141E1A4F424A5F, 0x6 },
	{ 0x1D0E020B11174C45, 0x4 },
	{ 0x080F505F574F4C46, 0x3 },
	{ 0x091E111C140A011F, 0x5 },
	{ 0x1D1319120E10135F, 0xd },
	{ 0x0841505F41424F56, 0x2 },
	{ 0x1A41505F42454C4F, 0x1 }
};

edF32VECTOR4 edF32VECTOR4_00426a30 = { 0.0f, 0.0f, 0.0f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00426a40 = { 0.0f, 0.0f, 0.0f, 0.8f };
edF32VECTOR4 edF32VECTOR4_00426a50 = { 0.0f, 0.0f, 1.0f, 0.8f };
edF32VECTOR4 edF32VECTOR4_00426a60 = { 0.0f, 0.0f, 1.0f, 0.0f };

edF32VECTOR4 edF32VECTOR4_00426a70 = { 0.0f, 0.0f, 0.0f, 0.2f };
edF32VECTOR4 edF32VECTOR4_00426a80 = { 0.0f, 0.0f, 1.0f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00426a90 = { 0.0f, 0.0f, 1.0f, 1.0f };
edF32VECTOR4 edF32VECTOR4_00426aa0 = { 0.0f, 0.0f, 0.2f, 1.0f };


void CLevelMap::DrawLegend(float param_1, float param_2)
{
	CMapManager* pMapManager;
	edCTextStyle* pNewFont;
	char* pcVar1;
	edDList_material* pDlistMaterial;
	MapKeyEntry* plVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float y;
	float fVar6;
	float x;
	float fVar7;
	float s;
	float x_00;
	S_2DRECT local_1690;
	float local_1650;
	float local_164c;
	float local_1648;
	float local_1644;
	float local_1640;
	float local_163c;
	float local_1638;
	float local_1634;
	float local_1630;
	float local_162c;
	float local_1628;
	float local_1624;
	float local_1620;
	float local_161c;
	float local_1618;
	float local_1614;
	S_2DRECT local_100;
	edCTextStyle textStyle;

	pMapManager = CScene::ptable.g_MapManager_0045168c;
	fVar6 = (CScene::ptable.g_MapManager_0045168c)->field_0x378;
	fVar7 = fVar6 * (param_2 - 1.0f);
	fVar3 = static_cast<float>(gVideoConfig.screenWidth) - param_1 * 168.0f;
	textStyle.Reset();
	textStyle.SetShadow(0x100);
	textStyle.alpha = 0xff;
	textStyle.SetVerticalAlignment(8);
	textStyle.SetFont(BootDataFont, false);
	pNewFont = edTextStyleSetCurrent(&textStyle);
	if (0.0f < param_1) {
		local_100.aVertices[0].x1 = fVar3 - 16.0f;
		local_100.aVertices[0].s1 = edF32VECTOR4_00426a30.z;
		local_100.aVertices[0].t1 = edF32VECTOR4_00426a30.w;
		local_100.aVertices[1].s1 = edF32VECTOR4_00426a40.z;
		local_100.aVertices[1].t1 = edF32VECTOR4_00426a40.w;
		fVar4 = fVar6 + fVar7;
		local_100.aVertices[2].s1 = edF32VECTOR4_00426a50.z;
		local_100.aVertices[2].t1 = edF32VECTOR4_00426a50.w;
		local_100.aVertices[3].s1 = edF32VECTOR4_00426a60.z;
		local_100.aVertices[3].t1 = edF32VECTOR4_00426a60.w;
		local_100.aVertices[1].x1 = static_cast<float>(gVideoConfig.screenWidth);
		local_100.aVertices[2].y1 = (static_cast<float>(gVideoConfig.screenHeight) * 472.0f) / 512.0f;
		local_100.aVertices[0].y1 = fVar4;
		local_100.aVertices[1].y1 = fVar4;
		local_100.aVertices[2].x1 = local_100.aVertices[1].x1;
		local_100.aVertices[3].x1 = local_100.aVertices[0].x1;
		local_100.aVertices[3].y1 = local_100.aVertices[2].y1;
		pMapManager->DrawClippedSprite(&MenuBitmaps[8].materialInfo, &local_100, 0x80808080);
		textStyle.SetHorizontalAlignment(0);
		textStyle.SetScale(0.69999999f, 0.69999999f);
		textStyle.field_0x8c = -9.0f;
		textStyle.rgbaColour = 0xffffffff;
		plVar2 = MapKeyEntry_ARRAY_00437480;
		fVar4 = (static_cast<float>(gVideoConfig.screenHeight) * 58.0f) / 512.0f + fVar4;
		fVar3 = fVar3 + 16.0f;
		fVar5 = fVar3 + 28.0f;
		do {
			pMapManager = CScene::ptable.g_MapManager_0045168c;
			local_1690.aVertices[1].x1 = (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f * 28.0f;
			local_1690.aVertices[0].s1 = edF32VECTOR2_00426968.x;
			local_1690.aVertices[0].t1 = edF32VECTOR2_00426968.y;
			local_1690.aVertices[1].s1 = edF32VECTOR2_00426978.x;
			local_1690.aVertices[1].t1 = edF32VECTOR2_00426978.y;
			local_1690.aVertices[2].s1 = edF32VECTOR2_00426988.x;
			local_1690.aVertices[2].t1 = edF32VECTOR2_00426988.y;
			local_1690.aVertices[3].s1 = edF32VECTOR2_00426998.x;
			local_1690.aVertices[3].t1 = edF32VECTOR2_00426998.y;
			local_1690.aVertices[0].x1 = fVar3 - local_1690.aVertices[1].x1;
			local_1690.aVertices[1].x1 = fVar3 + local_1690.aVertices[1].x1;
			local_1690.aVertices[0].y1 = fVar4 - 14.0f;
			local_1690.aVertices[2].y1 = fVar4 + 14.0f;
			local_1690.aVertices[1].y1 = local_1690.aVertices[0].y1;
			local_1690.aVertices[2].x1 = local_1690.aVertices[1].x1;
			local_1690.aVertices[3].x1 = local_1690.aVertices[0].x1;
			local_1690.aVertices[3].y1 = local_1690.aVertices[2].y1;
			if ((((local_1690.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_1690.aVertices[1].x1)) ||
				(local_1690.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y)) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_1690.aVertices[2].y1)) {
				pDlistMaterial = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(plVar2->materialId);
				pMapManager->DrawClippedSprite(pDlistMaterial, &local_1690, 0x80808080);
			}
			else {
				pDlistMaterial = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(plVar2->materialId);
				pMapManager->DrawSprite(pDlistMaterial, &local_1690, 0x80808080);
			}

			pcVar1 = gMessageManager.get_message(plVar2->key);
			edTextDraw(fVar5, fVar4 - 2.0f, pcVar1);
			plVar2 = plVar2 + 1;
			fVar4 = fVar4 + (static_cast<float>(gVideoConfig.screenHeight) * 32.0f) / 512.0f;
		} while (plVar2 < MapKeyEntry_ARRAY_00437480 + 8);
	}

	y = (static_cast<float>(gVideoConfig.screenHeight) * 30.0f) / 512.0f + ((fVar6 + fVar7) - (1.0f - param_1) * fVar6 * param_2);
	textStyle.SetHorizontalAlignment(1);
	textStyle.SetScale(0.80000001f, 0.80000001f);
	textStyle.rgbaColour = 0xb6bbfcff;
	edCTextFormat textFormat;
	pcVar1 = gMessageManager.get_message(0x305505f4c454745);
	textFormat.FormatString("%[VALDSAFE]b   %s", pcVar1);
	local_164c = y - 18.0f;
	local_1648 = edF32VECTOR4_00426a70.z;
	local_1644 = edF32VECTOR4_00426a70.w;
	local_1638 = edF32VECTOR4_00426a80.z;
	local_1634 = edF32VECTOR4_00426a80.w;
	local_162c = y + 15.0f;
	local_1628 = edF32VECTOR4_00426a90.z;
	local_1624 = edF32VECTOR4_00426a90.w;
	local_1618 = edF32VECTOR4_00426aa0.z;
	local_1614 = edF32VECTOR4_00426aa0.w;
	local_1650 = static_cast<float>(gVideoConfig.screenWidth);
	local_1640 = ((local_1650 - 25.0f) - textFormat.field_0x8 * 0.8f) - 24.0f;
	local_163c = local_164c;
	local_1630 = local_1640;
	local_1620 = local_1650;
	local_161c = local_162c;
	pDlistMaterial = pMapManager->GetDlistMaterial(0xb);
	edDListUseMaterial(pDlistMaterial);
	edDListBegin(0.0f, 0.0f, 0.0f, 4, 6);
	edDListColor4u8(0x80, 0x80, 0x80, 0x80);
	edDListLoadIdentity();
	edDListTexCoo2f(local_1648, local_1644);
	edDListVertex4f(local_1650, local_164c, 0.0f, 0.0f);
	edDListTexCoo2f(local_1618, local_1614);
	edDListVertex4f(local_1620, local_161c, 0.0f, 0.0f);
	fVar6 = local_1634;
	fVar5 = local_1638;
	edDListTexCoo2f(local_1638 - 0.2f, local_1634);
	fVar3 = local_163c;
	x_00 = local_1640;
	edDListVertex4f(local_1640 + 24.0f, local_163c, 0.0f, 0.0f);
	fVar4 = local_1624;
	s = local_1628;
	edDListTexCoo2f(local_1628 - 0.2f, local_1624);
	fVar7 = local_162c;
	x = local_1630;
	edDListVertex4f(local_1630 + 24.0f, local_162c, 0.0f, 0.0f);
	edDListTexCoo2f(fVar5, fVar6);
	edDListVertex4f(x_00, fVar3, 0.0f, 0.0f);
	edDListTexCoo2f(s, fVar4);
	edDListVertex4f(x, fVar7, 0.0f, 0.0f);
	edDListEnd();
	textFormat.Display(static_cast<float>(gVideoConfig.screenWidth) - 25.0f, y);

	edTextStyleSetCurrent(pNewFont);

	return;
}

void CLevelMap::Draw()
{
	int iVar1;
	CMapManager* pMapManager;
	bool bVar3;
	ObjectiveEntry* pLVar4;
	edDList_material* peVar5;
	char* pcVar7;
	char* pcVar8;
	uint uVar9;
	float fVar11;
	float fVar12;
	S_2DRECT local_300;
	S_2DRECT local_2c0;
	S_2DRECT local_280;
	S_2DRECT local_240;
	char acStack512[512];

	pMapManager = CScene::ptable.g_MapManager_0045168c;
	uVar9 = static_cast<uint>(this->field_0x7c != (LevelMapFileData*)0x0);
	if (uVar9 != 0) {
		uVar9 = static_cast<uint>(this->pMaterials != (edDList_material*)0x0);
	}

	if ((uVar9 | (CScene::ptable.g_MapManager_0045168c)->field_0x300) != 0) {
		iVar1 = ((CScene::ptable.g_MapManager_0045168c)->field_0x338).field_0x0;
		pLVar4 = (ObjectiveEntry*)0x0;
		if (iVar1 != 0x62) {
			pLVar4 = CLevelScheduler::gThis->FUN_002d9f30(iVar1);
		}

		ClearDisplay();

		bVar3 = GuiDList_BeginCurrent();
		if (bVar3 != false) {
			if (uVar9 != 0) {
				DrawMapTilesAndMarkers(pLVar4, (pMapManager->field_0x338).field_0x4);
			}

			pMapManager = CScene::ptable.g_MapManager_0045168c;
			if (pMapManager->field_0x300 != 0) {
				local_240.aVertices[1].x1 = (this->field_0x80).x;
				local_240.aVertices[2].y1 = (this->field_0x80).y;
				fVar11 = (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f * 32.0f;
				local_240.aVertices[0].s1 = edF32VECTOR2_00426968.x;
				local_240.aVertices[0].t1 = edF32VECTOR2_00426968.y;
				local_240.aVertices[1].s1 = edF32VECTOR2_00426978.x;
				local_240.aVertices[1].t1 = edF32VECTOR2_00426978.y;
				local_240.aVertices[2].s1 = edF32VECTOR2_00426988.x;
				local_240.aVertices[2].t1 = edF32VECTOR2_00426988.y;
				local_240.aVertices[3].s1 = edF32VECTOR2_00426998.x;
				local_240.aVertices[3].t1 = edF32VECTOR2_00426998.y;
				local_240.aVertices[0].x1 = local_240.aVertices[1].x1 - fVar11;
				local_240.aVertices[1].x1 = local_240.aVertices[1].x1 + fVar11;
				local_240.aVertices[0].y1 = local_240.aVertices[2].y1 - 16.0f;
				local_240.aVertices[2].y1 = local_240.aVertices[2].y1 + 16.0f;
				local_240.aVertices[1].y1 = local_240.aVertices[0].y1;
				local_240.aVertices[2].x1 = local_240.aVertices[1].x1;
				local_240.aVertices[3].x1 = local_240.aVertices[0].x1;
				local_240.aVertices[3].y1 = local_240.aVertices[2].y1;
				if ((((local_240.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_240.aVertices[1].x1)) ||
					(local_240.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y)) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_240.aVertices[2].y1)) {
					peVar5 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(10);
					CScene::ptable.g_MapManager_0045168c->DrawClippedSprite(peVar5, &local_240, 0x80808080);
				}
				else {
					peVar5 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(10);
					CScene::ptable.g_MapManager_0045168c->DrawSprite(peVar5, &local_240, 0x80808080);
				}
				pMapManager = CScene::ptable.g_MapManager_0045168c;
				local_280.aVertices[2].y1 = (this->field_0x80).y;
				local_280.aVertices[1].x1 = (this->field_0x90).x;
				fVar11 = (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f * -32.0f;
				local_280.aVertices[0].y1 = local_280.aVertices[2].y1 - 16.0f;
				local_280.aVertices[0].s1 = edF32VECTOR2_00426968.x;
				local_280.aVertices[0].t1 = edF32VECTOR2_00426968.y;
				local_280.aVertices[1].s1 = edF32VECTOR2_00426978.x;
				local_280.aVertices[1].t1 = edF32VECTOR2_00426978.y;
				local_280.aVertices[2].s1 = edF32VECTOR2_00426988.x;
				local_280.aVertices[2].t1 = edF32VECTOR2_00426988.y;
				local_280.aVertices[0].x1 = local_280.aVertices[1].x1 - fVar11;
				local_280.aVertices[3].s1 = edF32VECTOR2_00426998.x;
				local_280.aVertices[3].t1 = edF32VECTOR2_00426998.y;
				local_280.aVertices[2].y1 = local_280.aVertices[2].y1 + 16.0f;
				local_280.aVertices[1].x1 = local_280.aVertices[1].x1 + fVar11;
				local_280.aVertices[1].y1 = local_280.aVertices[0].y1;
				local_280.aVertices[2].x1 = local_280.aVertices[1].x1;
				local_280.aVertices[3].x1 = local_280.aVertices[0].x1;
				local_280.aVertices[3].y1 = local_280.aVertices[2].y1;
				if (((local_280.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_280.aVertices[1].x1)) ||
					((local_280.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_280.aVertices[2].y1)))) {
					peVar5 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(10);
					pMapManager->DrawClippedSprite(peVar5, &local_280, 0x80808080);
				}
				else {
					peVar5 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(10);
					CScene::ptable.g_MapManager_0045168c->DrawSprite(peVar5, &local_280, 0x80808080);
				}
				pMapManager = CScene::ptable.g_MapManager_0045168c;
				local_2c0.aVertices[1].x1 = (this->field_0x90).x;
				local_2c0.aVertices[2].y1 = (this->field_0x90).y;
				fVar11 = (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f * -32.0f;
				local_2c0.aVertices[0].s1 = edF32VECTOR2_00426968.x;
				local_2c0.aVertices[0].t1 = edF32VECTOR2_00426968.y;
				local_2c0.aVertices[1].s1 = edF32VECTOR2_00426978.x;
				local_2c0.aVertices[1].t1 = edF32VECTOR2_00426978.y;
				local_2c0.aVertices[2].s1 = edF32VECTOR2_00426988.x;
				local_2c0.aVertices[2].t1 = edF32VECTOR2_00426988.y;
				local_2c0.aVertices[3].s1 = edF32VECTOR2_00426998.x;
				local_2c0.aVertices[3].t1 = edF32VECTOR2_00426998.y;
				local_2c0.aVertices[0].x1 = local_2c0.aVertices[1].x1 - fVar11;
				local_2c0.aVertices[1].x1 = local_2c0.aVertices[1].x1 + fVar11;
				local_2c0.aVertices[0].y1 = local_2c0.aVertices[2].y1 - -16.0f;
				local_2c0.aVertices[2].y1 = local_2c0.aVertices[2].y1 + -16.0f;
				local_2c0.aVertices[1].y1 = local_2c0.aVertices[0].y1;
				local_2c0.aVertices[2].x1 = local_2c0.aVertices[1].x1;
				local_2c0.aVertices[3].x1 = local_2c0.aVertices[0].x1;
				local_2c0.aVertices[3].y1 = local_2c0.aVertices[2].y1;
				if (((local_2c0.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_2c0.aVertices[1].x1)) ||
					((local_2c0.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_2c0.aVertices[2].y1)))) {
					peVar5 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(10);
					pMapManager->DrawClippedSprite(peVar5, &local_2c0, 0x80808080);
				}
				else {
					peVar5 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(10);
					pMapManager->DrawSprite(peVar5, &local_2c0, 0x80808080);
				}
				pMapManager = CScene::ptable.g_MapManager_0045168c;
				local_300.aVertices[2].y1 = (this->field_0x90).y;
				local_300.aVertices[1].x1 = (this->field_0x80).x;
				fVar11 = (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f * 32.0f;
				local_300.aVertices[0].y1 = local_300.aVertices[2].y1 - -16.0f;
				local_300.aVertices[0].s1 = edF32VECTOR2_00426968.x;
				local_300.aVertices[0].t1 = edF32VECTOR2_00426968.y;
				local_300.aVertices[1].s1 = edF32VECTOR2_00426978.x;
				local_300.aVertices[1].t1 = edF32VECTOR2_00426978.y;
				local_300.aVertices[2].s1 = edF32VECTOR2_00426988.x;
				local_300.aVertices[2].t1 = edF32VECTOR2_00426988.y;
				local_300.aVertices[0].x1 = local_300.aVertices[1].x1 - fVar11;
				local_300.aVertices[3].s1 = edF32VECTOR2_00426998.x;
				local_300.aVertices[3].t1 = edF32VECTOR2_00426998.y;
				local_300.aVertices[2].y1 = local_300.aVertices[2].y1 + -16.0f;
				local_300.aVertices[1].x1 = local_300.aVertices[1].x1 + fVar11;
				local_300.aVertices[1].y1 = local_300.aVertices[0].y1;
				local_300.aVertices[2].x1 = local_300.aVertices[1].x1;
				local_300.aVertices[3].x1 = local_300.aVertices[0].x1;
				local_300.aVertices[3].y1 = local_300.aVertices[2].y1;
				if ((((local_300.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_300.aVertices[1].x1)) ||
					(local_300.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y)) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_300.aVertices[2].y1)) {
					peVar5 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(10);
					pMapManager->DrawClippedSprite(peVar5, &local_300, 0x80808080);
				}
				else {
					peVar5 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(10);
					pMapManager->DrawSprite(peVar5, &local_300, 0x80808080);
				}
				fVar11 = GetTimer()->totalTime;
				uVar9 = static_cast<uint>(pMapManager->field_0x380 != 0);
				if ((uVar9 == 0) && (uVar9 = 1, pMapManager->field_0x37c == 0.0f)) {
					uVar9 = 0;
				}
				fVar12 = pMapManager->ComputeMapSlideFactor((fVar11 - pMapManager->field_0x384) / 0.5f, uVar9);
				pMapManager->DrawObjectivePanel(fVar12);
				fVar11 = pMapManager->ComputeMapSlideFactor((fVar11 - this->field_0xd8) / 0.5f, this->field_0xd4);
				DrawLegend(fVar11, fVar12);
				pcVar7 = gMessageManager.get_message(0x120c1f090d454c50);
				pcVar8 = gMessageManager.get_message(0x5f474c100a04000f);
				sprintf(acStack512, "%%[UDLR]b %s   %%[VALID]b %s", pcVar7, pcVar8);
				pMapManager->DrawButtons(acStack512);
			}

			GuiDList_EndCurrent();
		}
	}

	if ((gPlayerInput.pressedBitfield & PAD_BITMASK_CROSS_ALT) != 0) {
		(CScene::ptable.g_FrontendManager_00451680)->pFrontendSamplePlayer->PlaySample(1.0f, 0, 0);
		pMapManager = CScene::ptable.g_MapManager_0045168c;
		IMPLEMENTATION_GUARD(
		CScene::ptable.g_MapManager_0045168c->FUN_003f8560();
		pMapManager->WorldMap_Enter(this->field_0x0);)
	}

	if ((gPlayerInput.pressedBitfield & PAD_BITMASK_13) != 0) {
		(CScene::ptable.g_FrontendManager_00451680)->pFrontendSamplePlayer->PlaySample(1.0f, 3, 0);
		MapLeave();
	}

	return;
}

edCBankCallback _gLevelMapBlankCallback[1] = { -1, -1, 0x0, 0, 0, 0, 0, 0 };

void _gLevelMapCallback(bool param_1, void* pParams)
{
	if (param_1 != false) {
		reinterpret_cast<CLevelMap*>(pParams)->InstallMapBank();
	}

	return;
}

bool CLevelMap::LoadMapBank(int levelId)
{
	int iVar1;
	int* piVar2;
	CLevelScheduler* pCVar3;
	CMapManager* pCVar4;
	bool bVar5;
	edCBankBufferEntry* peVar6;
	int iVar8;
	int iVar9;
	int iVar10;
	edCBankInstall mapBankInstall;
	char mapFilePath[1024];
	char mapFileName[16];

	this->field_0xdc = 0;
	this->field_0xe0 = 0;
	pCVar4 = CScene::ptable.g_MapManager_0045168c;
	this->field_0x0 = 0x10;
	this->levelId = levelId;

	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);

	pCVar3 = CLevelScheduler::gThis;
	iVar9 = -1;
	if (CLevelScheduler::gThis->currentLevelID == levelId) {
		iVar1 = ((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID;
		if (iVar1 == -1) {
			iVar1 = ((CScene::ptable.g_SectorManager_00451670)->baseSector).sectorIndex;
		}

		if (0 < iVar1) {
			piVar2 = pCVar4->field_0x10;
			iVar10 = 0;

			while (true) {
				if (piVar2 == (int*)0x0) {
					bVar5 = iVar10 < 0;
				}
				else {
					bVar5 = iVar10 < *piVar2;
				}

				if (!bVar5) break;

				if (iVar1 == piVar2[iVar10 + 1]) {
					iVar9 = iVar1;
				}

				iVar10 = iVar10 + 1;
			}
		}
	}

	if (iVar9 == -1) {
		/* Load default map map.bnk */
		edStrCatMulti(mapFilePath, CLevelScheduler::gThis->levelPath, CLevelScheduler::gThis->aLevelInfo[this->levelId].levelName, "/", "map.bnk", NULL);
	}
	else {
		/* map_%d.bnk */
		sprintf(mapFileName, "map_%d.bnk");
		edStrCatMulti(mapFilePath, pCVar3->levelPath, pCVar3->aLevelInfo[this->levelId].levelName, "/", mapFileName, NULL);
	}

	memset(&mapBankInstall, 0, sizeof(edCBankInstall));
	this->bankBuffer.initialize(0xc5800, 1, &mapBankInstall);
	this->bankBuffer.bank_buffer_setcb(_gLevelMapBlankCallback);
	mapBankInstall.filePath = mapFilePath;
	mapBankInstall.fileFunc = _gLevelMapCallback;
	mapBankInstall.pObjectReference = this;
	peVar6 = this->bankBuffer.get_free_entry();
	this->pBankBufferEntry = peVar6;
	mapBankInstall.fileFlagA = 0;
	bVar5 = this->pBankBufferEntry->load(&mapBankInstall);
	if (bVar5 == false) {
		this->pBankBufferEntry = (edCBankBufferEntry*)0x0;
	}

	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);

	return bVar5;
}

void CLevelMap::InstallMapBank()
{
	bool bVar1;
	uint uVar2;
	uint uVar3;
	edDList_material* peVar4;
	int iVar6;
	uint inFileIndex;
	edBANK_ENTRY_INFO local_20;
	int iStack4;

	this->field_0x0 = this->levelId;
	this->levelId = 0x10;

	uVar2 = this->pBankBufferEntry->get_element_count();
	inFileIndex = 0;

	if (uVar2 != 0) {
		do {
			bVar1 = this->pBankBufferEntry->get_info(inFileIndex, &local_20, (char*)0x0);

			if (bVar1 == false) break;

			uVar3 = local_20.type << 0x10 | local_20.stype;
			if (uVar3 == 0) {
				this->field_0x7c = reinterpret_cast<LevelMapFileData*>(local_20.fileBufferStart);
			}
			else {
				if (uVar3 == 0x50001) {
					ed3DInstallG2D(local_20.fileBufferStart, local_20.size, &iStack4, &this->field_0x44, 0);
					uVar3 = ed3DG2DGetG2DNbMaterials(&this->field_0x44);
					this->field_0x78 = uVar3;
					if (0 < (int)this->field_0x78) {
						this->pMaterials = new edDList_material[this->field_0x78];
						uVar3 = 0;
						if (this->field_0x78 != 0) {
							do {
								edDListCreatMaterialFromIndex(this->pMaterials + uVar3, uVar3, &this->field_0x44, 2);
								uVar3 = uVar3 + 1;
							} while (uVar3 < this->field_0x78);
						}
					}
				}
			}

			inFileIndex = inFileIndex + 1;
		} while (inFileIndex < uVar2);
	}

	if (this->field_0xd4 != 0) {
		this->field_0xd8 = GetTimer()->totalTime;
	}

	UpdateViewportBounds();
	SetupMatrices();

	bVar1 = (CScene::ptable.g_MapManager_0045168c)->bHasWolfenMap != 0;
	if (bVar1) {
		bVar1 = (CScene::ptable.g_MapManager_0045168c)->field_0x390 != 0;
	}

	if (bVar1) {
		this->field_0xe4 = GetTimer()->totalTime;
		this->field_0xc4 = this->field_0xc8 + (this->field_0xcc - this->field_0xc8) * 0.25f;
	}
	else {
		this->field_0xe4 = 0;
	}

	return;
}

edF32VECTOR4 edF32VECTOR4_004268d0 = { 0.0f, 0.0f, 1.0f, 1.0f };

void CLevelMap::SetupMatrices()
{
	edF32VECTOR4* peVar1;
	CLevelScheduler* pLevelScheduler;
	CMapManager* pMapManager;
	CChunk* pCVar2;
	ObjectiveEntry* pLVar3;
	MapPosition* pMVar4;
	MapSaveChunk* pCVar5;
	int iVar6;
	ObjectiveEntry* pLVar7;
	MapPosition* pMVar8;
	NativShopLevelSubObj* pNVar9;
	edF32VECTOR4* peVar10;
	edF32VECTOR4* peVar11;
	int iVar12;
	int currentLevelID;
	edF32MATRIX4* peVar14;
	edF32MATRIX4* peVar15;
	uint uVar16;
	uint uVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	edF32VECTOR4 local_a00;
	edF32VECTOR4 local_9f0;
	edF32VECTOR4 local_9e0;
	edF32MATRIX4 eStack2512;
	edF32VECTOR4 local_990;
	edF32VECTOR4 local_980;
	edF32MATRIX4 eStack2416;
	edF32VECTOR4 local_930;
	edF32VECTOR4 local_920;
	edF32VECTOR4 local_910;
	float local_900;
	float local_8fc;
	float fStack2296;
	float fStack2292;
	edF32VECTOR4 local_8f0;
	edF32VECTOR4 local_8e0;
	edF32VECTOR4 local_8d0;
	MapPosition aMStack2240[128];
	MapDataSizes local_c0;
	MapPosition* local_b0[16];
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	pMapManager = CScene::ptable.g_MapManager_0045168c;
	pLevelScheduler = CLevelScheduler::gThis;
	currentLevelID = CLevelScheduler::gThis->currentLevelID;
	pCVar5 = &(CScene::ptable.g_MapManager_0045168c)->field_0x58;
	if (this->field_0x0 == currentLevelID) {
		pCVar5 = &(CScene::ptable.g_MapManager_0045168c)->field_0x2c;
	}
	else {
		iVar12 = CLevelScheduler::gThis->OpenLevelChunk(this->field_0x0);
		if (iVar12 != 0) {
			pCVar2 = CLevelScheduler::gThis->SaveGame_OpenChunk(SAVEGAME_CHUNK_BLMP);
			if (pCVar2 != (CChunk*)0x0) {
				pCVar5 = reinterpret_cast<MapSaveChunk*>(pCVar2 + 1);
				CLevelScheduler::gThis->SaveGame_CloseChunk();

			}

			CLevelScheduler::gThis->CloseLevelChunk();
		}
	}

	this->field_0xd0 = pCVar5->field_0x28 & 1;
	local_10.xyz = pCVar5->field_0x0;
	local_10.w = 1.0f;
	local_20.xyz = pCVar5->field_0xc;
	local_20.w = 1.0f;
	local_30.xyz = pCVar5->field_0x18;
	local_30.w = 0.0f;

	edF32Vector4SubHard(&eStack64, &local_20, &local_10);
	edF32Vector4NormalizeHard(&eStack64, &eStack64);
	edF32Vector4CrossProductHard(&eStack80, &local_30, &eStack64);
	edF32Vector4SubHard(&this->field_0xf0.rowZ, &local_20, &local_10);
	fVar18 = edF32Vector4GetDistHard(&this->field_0xf0.rowZ);
	fVar18 = fVar18 * pCVar5->field_0x24;
	edF32Vector4ScaleHard(fVar18, &eStack96, &local_30);
	edF32Vector4ScaleHard(-fVar18, &eStack112, &eStack80);
	edF32Vector4ScaleHard(2.0f, &this->field_0xf0.rowX, &eStack112);
	edF32Vector4ScaleHard(-2.0f, &this->field_0xf0.rowY, &eStack96);
	edF32Vector4SubHard(&this->field_0xf0.rowT, &local_20, &eStack112);
	peVar11 = &this->field_0xf0.rowT;
	edF32Vector4AddHard(peVar11, peVar11, &eStack96);
	edF32Matrix4GetInverseGaussSoft(&this->field_0x130, &this->field_0xf0);
	(this->field_0xb0).x = 0.5f;
	(this->field_0xb0).y = 0.5f;
	(this->field_0xb0).z = 0.0f;
	(this->field_0xb0).w = 1.0f;
	if (this->field_0x0 == currentLevelID) {
		this->field_0xc0 = pMapManager->field_0xc;
	}
	else {
		SaveDataChunk_BLMP* pSaveDataChunk_BLMP = pLevelScheduler->LoadMapSaveChunk(this->field_0x0);
		if (pSaveDataChunk_BLMP == (SaveDataChunk_BLMP*)0x0) {
			this->field_0xc0 = 0.0f;
		}
		else {
			this->field_0xc0 = pSaveDataChunk_BLMP->field_0x2c;
		}
	}

	this->field_0xc4 = 1.0f;
	this->scrollMatrix = gF32Matrix4Unit;
	
	fVar18 = (this->field_0xb0).y;
	fVar19 = (this->field_0xb0).z;
	fVar20 = (this->field_0xb0).w;
	(this->scrollMatrix).da = 0.0f - (this->field_0xb0).x;
	(this->scrollMatrix).db = 0.0f - fVar18;
	(this->scrollMatrix).dc = 0.0f - fVar19;
	(this->scrollMatrix).dd = fVar20;
	local_930.x = this->field_0xc4;
	local_930.y = this->field_0xc4;
	local_930.z = 1.0f;
	local_930.w = 1.0f;
	edF32Matrix4ScaleHard(&this->scrollMatrix, &this->scrollMatrix, &local_930);
	edF32Matrix4RotateZHard(this->field_0xc0, &eStack2416, &gF32Matrix4Unit);
	edF32Matrix4MulF32Matrix4Hard(&this->scrollMatrix, &this->scrollMatrix, &eStack2416);
	local_980.z = edF32VECTOR4_004268d0.z;
	local_980.w = edF32VECTOR4_004268d0.w;
	local_980.x = static_cast<float>(gVideoConfig.screenWidth) / (CScene::ptable.g_CameraManager_0045167c)->aspectRatio;
	local_980.y = static_cast<float>(gVideoConfig.screenHeight);
	edF32Matrix4ScaleHard(&this->scrollMatrix, &this->scrollMatrix, &local_980);
	peVar11 = &(this->scrollMatrix).rowT;
	edF32Vector4AddHard(peVar11, peVar11, &this->field_0xa0);
	edF32Matrix4MulF32Matrix4Hard(&this->field_0x1b0, &this->field_0x130, &this->scrollMatrix);
	uVar16 = 0;
	if ((this->field_0x0 == currentLevelID) && (CActorHero::_gThis != (CActorHero*)0x0)) {
		local_b0[0] = (MapPosition*)&CActorHero::_gThis->currentLocation;
		uVar16 = 1;
	}

	currentLevelID = (pMapManager->field_0x338).field_0x0;
	pLVar3 = (ObjectiveEntry*)0x0;
	if (currentLevelID != 0x62) {
		pLVar3 = pLevelScheduler->FUN_002d9f30(currentLevelID);
	}

	pMapManager->LoadMarkerPositionsForLevel(this->field_0x0, &local_c0, aMStack2240, 0x80);
	pMVar8 = aMStack2240 + local_c0.field_0x0 + local_c0.nbTeleporters + local_c0.nbWolfen;
	uVar17 = uVar16;
	if ((pLVar3 != (ObjectiveEntry*)0x0) && (uVar16 < 0x10)) {
		if (this->field_0x0 == pLVar3->levelId) {
			if (pLVar3->field_0x40 == 0) {
				local_b0[uVar16] = (MapPosition*)&pLVar3->field_0x10;
				uVar17 = uVar16 + 1;
			}
			else {
				local_b0[uVar16] = (MapPosition*)&pLVar3->field_0x30;
				uVar17 = uVar16 + 1;
			}
		}
		else {
			pMVar4 = FindMarkerForLevel(pLVar3->levelId, &local_c0, aMStack2240, 1);
			if (pMVar4 != (MapPosition*)0x0) {
				uVar17 = uVar16 + 1;
				local_b0[uVar16] = pMVar4;
			}
		}
	}
	currentLevelID = 0;
	if ((pMapManager->field_0x338).field_0x4 == 1) {
		for (; currentLevelID < local_c0.nbRunes; currentLevelID = currentLevelID + 1) {
			if (static_cast<int>(uVar17) < 0x10) {
				local_b0[uVar17] = pMVar8;
				uVar17 = uVar17 + 1;
			}

			pMVar8 = pMVar8 + 1;
		}

		for (currentLevelID = 0; currentLevelID < 0x10; currentLevelID = currentLevelID + 1) {
			SaveDataChunk_BLMP* pSaveDataChunk_BLMP;
			if (((currentLevelID != this->field_0x0) && (pSaveDataChunk_BLMP = pLevelScheduler->LoadMapSaveChunk(currentLevelID), pSaveDataChunk_BLMP != (SaveDataChunk_BLMP*)0x0)) && (0 < pSaveDataChunk_BLMP->mapDataSizes.nbRunes)) {
				IMPLEMENTATION_GUARD(
				pMVar8 = FindMarkerForLevel(currentLevelID, &local_c0, aMStack2240, 0);
				if ((pMVar8 != (MapPosition*)0x0) && (static_cast<int>(uVar17) < 0x10)) {
					local_b0[uVar17] = pMVar8;
					uVar17 = uVar17 + 1;
				})
			}
		}
	}

	currentLevelID = 0;
	if ((pMapManager->field_0x338).field_0x4 == 2) {
		for (; currentLevelID < 0x10; currentLevelID = currentLevelID + 1) {
			IMPLEMENTATION_GUARD(
			if (((static_cast<int>(uVar17) < 0x10) && (iVar12 = pLevelScheduler->IsMapCollectedInLevel(currentLevelID), iVar12 != 0x62)) && (iVar6 = CLevelScheduler::ScenVar_Get(iVar12), iVar6 != 2)) {
				if (currentLevelID == this->field_0x0) {
					pLVar7 = pLevelScheduler->FUN_002d9f30(iVar12);
					if (pLVar7 != (ObjectiveEntry*)0x0) {
						local_b0[uVar17] = (MapPosition*)&pLVar7->field_0x10;
						uVar17 = uVar17 + 1;
					}
				}
				else {
					pMVar8 = FindMarkerForLevel(currentLevelID, &local_c0, aMStack2240, 0);
					if (pMVar8 != (MapPosition*)0x0) {
						local_b0[uVar17] = pMVar8;
						uVar17 = uVar17 + 1;
					}
				}
			})
		}
	}

	uVar16 = uVar17;
	if ((((pLVar3 != (ObjectiveEntry*)0x0) && ((pMapManager->field_0x338).field_0x4 == 4)) && (pNVar9 = pLevelScheduler->FUN_002d9510(pLVar3->field_0x24), pNVar9 != (NativShopLevelSubObj*)0x0))
		&& ((pNVar9->currentLevelId == this->field_0x0 && (static_cast<int>(uVar17) < 0x10)))) {
		uVar16 = uVar17 + 1;
		local_b0[uVar17] = (MapPosition*)&pNVar9->currentLocation;
	}

	if (uVar16 != 0) {
		local_8d0 = gF32Vector4Zero;
		for (currentLevelID = 0; currentLevelID < static_cast<int>(uVar16); currentLevelID = currentLevelID + 1) {
			local_8d0.xyz = local_8d0.xyz + local_b0[currentLevelID]->position;
		}

		edF32Vector4ScaleHard(1.0f / static_cast<float>(uVar16), &local_8d0, &local_8d0);
		local_8d0.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&this->field_0xb0, &this->field_0x130, &local_8d0);

		if (this->field_0xd0 != 0) {
			(this->field_0xb0).x = ((this->field_0xb0).x - 0.5f) / ((this->field_0xb0).z - -1.0f) + 0.5f;
			(this->field_0xb0).y = ((this->field_0xb0).y - 0.5f) / ((this->field_0xb0).z - -1.0f) + 0.5f;
		}
	}

	// Clear
	local_8e0 = {};

	local_8e0.x = 1.0f / (float)this->field_0x7c->field_0x10;
	local_8e0.y = 1.0f / (float)this->field_0x7c->field_0x14;
	edF32Matrix4MulF32Vector4Hard(&local_8e0, &this->scrollMatrix, &local_8e0);
	fVar18 = edF32Vector4GetDistHard(&local_8e0);
	this->field_0xc8 = 0.80000001f;
	this->field_0xcc = 1.414214f / fVar18;
	if (this->field_0xcc < this->field_0xc8) {
		this->field_0xcc = this->field_0xc8;
	}

	this->scrollMatrix = gF32Matrix4Unit;

	this->field_0xcc = this->field_0xcc * 1.5f;
	this->field_0xc4 = this->field_0xc8;

	fVar18 = (this->field_0xb0).y;
	fVar19 = (this->field_0xb0).z;
	fVar20 = (this->field_0xb0).w;
	(this->scrollMatrix).da = 0.0f - (this->field_0xb0).x;
	(this->scrollMatrix).db = 0.0f - fVar18;
	(this->scrollMatrix).dc = 0.0f - fVar19;
	(this->scrollMatrix).dd = fVar20;

	local_990.x = this->field_0xc4;
	local_990.y = this->field_0xc4;
	local_990.z = 1.0f;
	local_990.w = 1.0f;

	edF32Matrix4ScaleHard(&this->scrollMatrix, &this->scrollMatrix, &local_990);
	edF32Matrix4RotateZHard(this->field_0xc0, &eStack2512, &gF32Matrix4Unit);
	edF32Matrix4MulF32Matrix4Hard(&this->scrollMatrix, &this->scrollMatrix, &eStack2512);

	local_9e0.z = edF32VECTOR4_004268d0.z;
	local_9e0.w = edF32VECTOR4_004268d0.w;
	local_9e0.x = static_cast<float>(gVideoConfig.screenWidth) / (CScene::ptable.g_CameraManager_0045167c)->aspectRatio;
	local_9e0.y = static_cast<float>(gVideoConfig.screenHeight);

	edF32Matrix4ScaleHard(&this->scrollMatrix, &this->scrollMatrix, &local_9e0);
	peVar11 = (edF32VECTOR4*)&(this->scrollMatrix).da;
	edF32Vector4AddHard(peVar11, peVar11, &this->field_0xa0);
	edF32Matrix4MulF32Matrix4Hard(&this->field_0x1b0, &this->field_0x130, &this->scrollMatrix);

	if (uVar16 != 0) {
		local_910.xyz = local_b0[0]->position;
		local_910.w = 1.0f;

		if (this->field_0xd0 == 0) {
			edF32Matrix4MulF32Vector4Hard(&local_8f0, &this->field_0x1b0, &local_910);
		}
		else {
			edF32Matrix4MulF32Vector4Hard(&local_9f0, &this->field_0x130, &local_910);
			local_9f0.x = (local_9f0.x - 0.5f) / (local_9f0.z - -1.0f) + 0.5f;
			local_9f0.y = (local_9f0.y - 0.5f) / (local_9f0.z - -1.0f) + 0.5f;
			edF32Matrix4MulF32Vector4Hard(&local_8f0, &this->scrollMatrix, &local_9f0);
		}

		local_900 = local_8f0.x;
		local_8fc = local_8f0.y;
		fStack2296 = local_8f0.z;
		fStack2292 = local_8f0.w;

		for (currentLevelID = 1; currentLevelID < static_cast<int>(uVar16); currentLevelID = currentLevelID + 1) {
			pMVar8 = local_b0[currentLevelID];
			local_910.xyz = pMVar8->position;
			local_910.w = 1.0f;

			if (this->field_0xd0 == 0) {
				edF32Matrix4MulF32Vector4Hard(&local_920, &this->field_0x1b0, &local_910);
			}
			else {
				edF32Matrix4MulF32Vector4Hard(&local_a00, &this->field_0x130, &local_910);
				local_a00.x = (local_a00.x - 0.5f) / (local_a00.z - -1.0f) + 0.5f;
				local_a00.y = (local_a00.y - 0.5f) / (local_a00.z - -1.0f) + 0.5f;
				edF32Matrix4MulF32Vector4Hard(&local_920, &this->scrollMatrix, &local_a00);
			}

			if (local_920.x < local_8f0.x) {
				local_8f0.x = local_920.x;
			}
			else {
				if (local_900 < local_920.x) {
					local_900 = local_920.x;
				}
			}

			if (local_920.y < local_8f0.y) {
				local_8f0.y = local_920.y;
			}
			else {
				if (local_8fc < local_920.y) {
					local_8fc = local_920.y;
				}
			}
		}

		fVar18 = 0.0f;
		fVar19 = 0.0f;

		if (local_8f0.x != local_900) {
			fVar18 = (local_900 - local_8f0.x) / ((this->field_0x90).x - (this->field_0x80).x);
		}

		if (local_8f0.y != local_8fc) {
			fVar19 = (local_8fc - local_8f0.y) / ((this->field_0x90).y - (this->field_0x80).y);
		}

		if (fVar18 <= fVar19) {
			fVar18 = fVar19;
		}

		if (0.0f < fVar18 * 1.2f) {
			fVar18 = this->field_0xc4 / (fVar18 * 1.2f);
			this->field_0xc4 = fVar18;
			fVar19 = this->field_0xcc / 1.2f;
			if (fVar19 < fVar18) {
				this->field_0xc4 = fVar19;
			}

			if (this->field_0xc4 < this->field_0xc8) {
				this->field_0xc4 = this->field_0xc8;
			}
		}
		else {
			this->field_0xc4 = this->field_0xcc / 1.2f;
		}
	}

	return;
}

void CLevelMap::Update()
{
	ManagePad();

	if ((1.0f <= (GetTimer()->totalTime - this->field_0xd8) / 0.5f) && ((gPlayerInput.pressedBitfield & PAD_BITMASK_12) != 0)) {
		(CScene::ptable.g_FrontendManager_00451680)->pFrontendSamplePlayer->PlaySample(1.0f, 2, 0);
		this->field_0xd4 = this->field_0xd4 ^ 1;
		this->field_0xd8 = GetTimer()->totalTime;
	}

	UpdateViewportBounds();

	return;
}

void CLevelMap::Close()
{
	bool bVar1;
	int iVar2;
	uint uVar3;

	if ((this->pBankBufferEntry != (edCBankBufferEntry*)0x0) && (bVar1 = this->pBankBufferEntry->is_loaded(), bVar1 == false)) {
		this->pBankBufferEntry->wait();
	}

	if (this->pMaterials != (edDList_material*)0x0) {
		uVar3 = 0;
		if (this->field_0x78 != 0) {
			do {
				edDListTermMaterial(&this->pMaterials[uVar3]);
				uVar3 = uVar3 + 1;
			} while (uVar3 < this->field_0x78);
		}

		this->field_0x78 = 0;
		delete(this->pMaterials);
		this->pMaterials = (edDList_material*)0x0;
		ed3DUnInstallG2D(&this->field_0x44);
	}

	GlobalDList_AddToView();
#ifdef PLATFORM_PS2
	edVideoFlip();
#endif

	if (this->pBankBufferEntry != (edCBankBufferEntry*)0x0) {
		this->pBankBufferEntry->close();
		this->pBankBufferEntry = (edCBankBufferEntry*)0x0;
	}

	this->bankBuffer.terminate();

	return;
}

void CLevelMap::ManagePad()
{
	int iVar2;
	edF32VECTOR4* v0;
	edF32MATRIX4* peVar3;
	edF32MATRIX4* peVar4;
	float fVar5;
	float fVar6;
	float puVar8;
	float fVar7;
	float fVar8;
	float puVar10;
	edF32VECTOR4 local_80;
	edF32MATRIX4 eStack112;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if ((this->field_0x7c != (LevelMapFileData*)0x0) && (this->pMaterials != (edDList_material*)0x0)) {
		fVar6 = GetTimer()->lastFrameTime;
		fVar7 = gPlayerInput.aAnalogSticks[0].x;
		if (fabsf(gPlayerInput.aAnalogSticks[0].x) < 0.2f) {
			fVar7 = 0.0f;
		}

		fVar5 = gPlayerInput.aAnalogSticks[0].y;
		if (fabsf(gPlayerInput.aAnalogSticks[0].y) < 0.2f) {
			fVar5 = 0.0f;
		}

		fVar8 = gPlayerInput.aAnalogSticks[1].y;
		if (fabsf(gPlayerInput.aAnalogSticks[1].y) < 0.2f) {
			fVar8 = 0.0f;
		}

		puVar10 = fVar7;
		if (((fVar7 == 0.0f) && (puVar10 = -1.0f, gPlayerInput.aButtons[INPUT_BUTTON_DPAD_L].clickValue == 0.0f)) && (puVar10 = fVar7, gPlayerInput.aButtons[INPUT_BUTTON_DPAD_R].clickValue != 0.0f)) {
			puVar10 = 1.0f;
		}

		puVar8 = fVar5;
		if (((fVar5 == 0.0f) && (puVar8 = 1.0f, gPlayerInput.aButtons[INPUT_BUTTON_DPAD_U].clickValue == 0.0f)) && (puVar8 = fVar5, gPlayerInput.aButtons[INPUT_BUTTON_DPAD_D].clickValue != 0.0f)) {
			puVar8 = -1.0f;
		}

		fVar7 = puVar8 * fVar6 * -1.0f;
		if ((gPlayerInput.pressedBitfield & PAD_BITMASK_15) != 0) {
			this->field_0xdc = 1;
		}

		if ((gPlayerInput.pressedBitfield & PAD_BITMASK_16) != 0) {
			this->field_0xe0 = 1;
		}

		if ((this->field_0xdc != 0) && (gPlayerInput.aButtons[INPUT_BUTTON_INDEX_1E].clickValue != 0.0f)) {
			this->field_0xc0 = this->field_0xc0 - fVar6 * 3.141593f * 0.5f;
		}

		if ((this->field_0xe0 != 0) && (gPlayerInput.aButtons[INPUT_BUTTON_INDEX_SQUARE_ALT].clickValue != 0.0f)) {
			this->field_0xc0 = this->field_0xc0 + fVar6 * 3.141593f * 0.5f;
		}

		fVar5 = edF32Between_Pi(this->field_0xc0);
		this->field_0xc0 = fVar5;
		local_10.x = cosf(this->field_0xc0 - 1.570796f);
		local_20.y = -local_10.x;
		local_20.x = cosf(this->field_0xc0);
		local_20.w = 0.0f;
		local_10.z = 0.0f;
		local_10.w = 0.0f;
		local_20.z = 0.0f;

		this->field_0xc4 = this->field_0xc4 * (fVar8 * fVar6 + 1.0f);
		if (this->field_0xcc < this->field_0xc4) {
			this->field_0xc4 = this->field_0xcc;
		}

		if (this->field_0xc4 < this->field_0xc8) {
			this->field_0xc4 = this->field_0xc8;
		}

		local_10.y = local_20.x;
		edF32Vector4ScaleHard(fVar7 / this->field_0xc4, &local_10, &local_10);
		edF32Vector4ScaleHard((puVar10 * fVar6) / this->field_0xc4, &local_20, &local_20);
		edF32Vector4AddHard(&this->field_0xb0, &this->field_0xb0, &local_10);
		edF32Vector4AddHard(&this->field_0xb0, &this->field_0xb0, &local_20);

		if (((this->field_0x0 == CLevelScheduler::gThis->currentLevelID) && (CActorHero::_gThis != (CActorHero*)0x0)) && ((gPlayerInput.pressedBitfield & PAD_BITMASK_R1_ALT) != 0)) {
			(CScene::ptable.g_FrontendManager_00451680)->pFrontendSamplePlayer->PlaySample(1.0f, 0, 0);
			edF32Matrix4MulF32Vector4Hard(&this->field_0xb0, &this->field_0x130, &CActorHero::_gThis->currentLocation);

			if (this->field_0xd0 != 0) {
				(this->field_0xb0).x = ((this->field_0xb0).x - 0.5f) / ((this->field_0xb0).z - -1.0f) + 0.5f;
				(this->field_0xb0).y = ((this->field_0xb0).y - 0.5f) / ((this->field_0xb0).z - -1.0f) + 0.5f;
			}
		}

		fVar7 = (this->field_0xb0).x;
		if (fVar7 < 0.0f) {
			(this->field_0xb0).x = 0.0f;
		}
		else {
			if (1.0f < fVar7) {
				(this->field_0xb0).x = 1.0f;
			}
		}

		fVar7 = (this->field_0xb0).y;
		if (fVar7 < 0.0f) {
			(this->field_0xb0).y = 0.0f;
		}
		else {
			if (1.0f < fVar7) {
				(this->field_0xb0).y = 1.0f;
			}
		}

		this->scrollMatrix = gF32Matrix4Unit;

		fVar7 = (this->field_0xb0).y;
		fVar6 = (this->field_0xb0).z;
		fVar5 = (this->field_0xb0).w;
		(this->scrollMatrix).da = 0.0f - (this->field_0xb0).x;
		(this->scrollMatrix).db = 0.0f - fVar7;
		(this->scrollMatrix).dc = 0.0f - fVar6;
		(this->scrollMatrix).dd = fVar5;
		local_30.x = this->field_0xc4;
		local_30.y = this->field_0xc4;
		local_30.z = 1.0f;
		local_30.w = 1.0f;
		edF32Matrix4ScaleHard(&this->scrollMatrix, &this->scrollMatrix, &local_30);
		edF32Matrix4RotateZHard(this->field_0xc0, &eStack112, &gF32Matrix4Unit);
		edF32Matrix4MulF32Matrix4Hard(&this->scrollMatrix, &this->scrollMatrix, &eStack112);
		local_80.z = edF32VECTOR4_004268d0.z;
		local_80.w = edF32VECTOR4_004268d0.w;
		local_80.x = static_cast<float>(gVideoConfig.screenWidth) / (CScene::ptable.g_CameraManager_0045167c)->aspectRatio;
		local_80.y = static_cast<float>(gVideoConfig.screenHeight);
		edF32Matrix4ScaleHard(&this->scrollMatrix, &this->scrollMatrix, &local_80);
		v0 = &(this->scrollMatrix).rowT;
		edF32Vector4AddHard(v0, v0, &this->field_0xa0);
		edF32Matrix4MulF32Matrix4Hard(&this->field_0x1b0, &this->field_0x130, &this->scrollMatrix);
	}

	return;
}

void CLevelMap::UpdateViewportBounds()
{
	bool bVar1;
	uint uVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	CMapManager* pMapManager;

	pMapManager = CScene::ptable.g_MapManager_0045168c;
	fVar6 = GetTimer()->totalTime;
	bVar1 = true;
	if ((pMapManager->field_0x380 == 0) && (pMapManager->field_0x37c == 0.0f)) {
		bVar1 = false;
	}

	if (bVar1) {
		fVar5 = (fVar6 - pMapManager->field_0x384) / 0.3333333f;
	}
	else {
		fVar5 = (fVar6 - pMapManager->field_0x384) / 0.6f;
	}

	uVar3 = static_cast<uint>(pMapManager->field_0x380 != 0);
	if (uVar3 == 0) {
		uVar3 = static_cast<uint>(pMapManager->field_0x37c != 0.0f);
	}

	fVar5 = pMapManager->ComputeMapSlideFactor(fVar5, uVar3);
	if (this->field_0xd4 == 0) {
		fVar6 = (fVar6 - this->field_0xd8) / 0.6f;
	}
	else {
		fVar6 = (fVar6 - this->field_0xd8) / 0.3333333f;
	}

	fVar6 = pMapManager->ComputeMapSlideFactor(fVar6, this->field_0xd4);
	fVar4 = pMapManager->field_0x378;

#ifdef PLATFORM_WIN
	this->field_0x80 = {};
	this->field_0x90 = {};
#endif

	(this->field_0x80).x = 24.0f;
	(this->field_0x80).y = (static_cast<float>(gVideoConfig.screenHeight) * 6.0f) / 512.0f + (static_cast<float>(gVideoConfig.screenHeight) * 30.0f) / 512.0f + fVar5 * fVar4;
	(this->field_0x90).x = (static_cast<float>(gVideoConfig.screenWidth) - fVar6 * 168.0f) - 24.0f;
	(this->field_0x90).y = (static_cast<float>(gVideoConfig.screenHeight) * 463.0f) / 512.0f - (static_cast<float>(gVideoConfig.screenHeight) * 20.0f) / 512.0f;

	fVar6 = (static_cast<float>(gVideoConfig.screenHeight) * 30.0f) / 512.0f + (static_cast<float>(gVideoConfig.screenHeight) * 28.0f) / 512.0f;
	if ((this->field_0x80).y < fVar6) {
		(this->field_0x80).y = fVar6;
	}

	edF32Vector4AddHard(&this->field_0xa0, &this->field_0x80, &this->field_0x90);
	edF32Vector4ScaleHard(0.5f, &this->field_0xa0, &this->field_0xa0);

	return;
}

MapPosition* CLevelMap::FindMarkerForLevel(int inLevelId, MapDataSizes* pMapSizes, MapPosition* pMarkerBuffer, long param_5)
{
	bool bVar1;
	MapPosition* pMVar2;
	int iVar3;
	MapPosition* pMVar4;
	MapPosition* pMVar5;
	CMapManager* pMapManager;

	pMapManager = CScene::ptable.g_MapManager_0045168c;
	iVar3 = 0;
	pMVar4 = (MapPosition*)0x0;
	pMVar2 = pMarkerBuffer + pMapSizes->field_0x0;
	pMVar5 = pMVar2;

	while ((iVar3 < pMapSizes->nbTeleporters && (pMVar4 == (MapPosition*)0x0))) {
		bVar1 = pMapManager->HasRouteToLevel(inLevelId, pMVar5->field_0xc, 0);
		if (bVar1 != false) {
			pMVar4 = pMVar5;
		}
		iVar3 = iVar3 + 1;
		pMVar5 = pMVar5 + 1;
	}

	iVar3 = 0;
	pMVar5 = pMarkerBuffer;
	while ((iVar3 < pMapSizes->field_0x0 && (pMVar4 == (MapPosition*)0x0))) {
		bVar1 = pMapManager->HasRouteToLevel(inLevelId, pMVar5->field_0xc, 0);
		if (bVar1 != false) {
			pMVar4 = pMVar5;
		}

		iVar3 = iVar3 + 1;
		pMVar5 = pMVar5 + 1;
	}

	if (param_5 != 0) {
		iVar3 = 0;
		while ((iVar3 < pMapSizes->nbTeleporters && (pMVar4 == (MapPosition*)0x0))) {
			bVar1 = pMapManager->HasRouteToLevel(inLevelId, pMVar2->field_0xc, 1);
			if (bVar1 != false) {
				pMVar4 = pMVar2;
			}
			iVar3 = iVar3 + 1;
			pMVar2 = pMVar2 + 1;
		}

		iVar3 = 0;
		while ((iVar3 < pMapSizes->field_0x0 && (pMVar4 == (MapPosition*)0x0))) {
			bVar1 = pMapManager->HasRouteToLevel(inLevelId, pMarkerBuffer->field_0xc, 1);
			if (bVar1 != false) {
				pMVar4 = pMarkerBuffer;
			}
			iVar3 = iVar3 + 1;
			pMarkerBuffer = pMarkerBuffer + 1;
		}
	}

	return pMVar4;
}

edF32VECTOR4 edF32VECTOR4_004269a0 = { 0.0f, 0.0f, 0.0f, 1.0f };

void CLevelMap::DrawMapTilesAndMarkers(ObjectiveEntry* pObjectiveEntry, int param_3)
{
	int mapSpriteIndex;
	bool bVar2;
	bool bVar3;
	bool bVar4;
	edF32VECTOR4* peVar5;
	edF32VECTOR4* peVar6;
	edF32VECTOR4* peVar7;
	uint uVar8;
	int xIndex;
	int* pMapSpriteIndexIt;
	int yIndex;
	float x;
	float y;
	S_2DRECT rect;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CMapManager* pMapManager;

	local_c0.x = 1.0f / (float)this->field_0x7c->field_0x8;
	local_c0.y = 1.0f / (float)this->field_0x7c->field_0xc;

	local_10 = {};
	local_20 = {};
	local_30 = {};

	local_10.x = local_c0.x;
	local_20.y = local_c0.y;
	
	local_30.x = local_c0.x * 0.5f;
	local_30.y = local_c0.y * 0.5f;

	edF32Matrix4MulF32Vector4Hard(&local_10, &this->scrollMatrix, &local_10);
	edF32Matrix4MulF32Vector4Hard(&local_20, &this->scrollMatrix, &local_20);
	edF32Matrix4MulF32Vector4Hard(&local_30, &this->scrollMatrix, &local_30);

	local_50 = gF32Vector4Zero;

	local_40.z = gF32Vector4Zero.z;
	local_40.w = gF32Vector4Zero.w;
	local_40.x = local_50.x;
	local_40.y = local_50.y;

	for (uVar8 = 0; static_cast<int>(uVar8) < 4; uVar8 = uVar8 + 1) {
		if ((uVar8 & 1) == 0) {
			local_60 = local_10;
		}
		else {
			edF32Vector4GetNegHard(&local_60, &local_10);
		}

		if (static_cast<int>(uVar8) < 2) {
			edF32Vector4SubHard(&local_60, &local_60, &local_20);
		}
		else {
			edF32Vector4AddHard(&local_60, &local_60, &local_20);
		}

		if (local_60.x < local_40.x) {
			local_40.x = local_60.x;
		}

		if (local_50.x < local_60.x) {
			local_50.x = local_60.x;
		}

		if (local_60.y < local_40.y) {
			local_40.y = local_60.y;
		}

		if (local_50.y < local_60.y) {
			local_50.y = local_60.y;
		}
	}

	edF32Vector4ScaleHard(0.5f, &local_40, &local_40);
	edF32Vector4ScaleHard(0.5f, &local_50, &local_50);
	edF32Vector4SubHard(&eStack112, &local_50, &local_40);
	pMapManager = CScene::ptable.g_MapManager_0045168c;
	local_80 = local_40;
	local_90 = local_50;
	local_a0 = local_40;
	local_b0 = local_50;
	peVar6 = &(CScene::ptable.g_MapManager_0045168c)->field_0x310;
	peVar7 = &(CScene::ptable.g_MapManager_0045168c)->field_0x320;
	edF32Vector4SubHard(&local_80, peVar6, &local_50);
	edF32Vector4SubHard(&local_90, peVar7, &local_40);
	edF32Vector4SubHard(&local_a0, peVar6, &local_40);
	edF32Vector4SubHard(&local_b0, peVar7, &local_50);
	edDListLoadIdentity();

	local_c0.z = edF32VECTOR4_004269a0.z;
	local_c0.w = edF32VECTOR4_004269a0.w;
	local_c0.x = local_c0.x * 0.5f;
	local_c0.y = local_c0.y * 0.5f;
	edF32Matrix4MulF32Vector4Hard(&local_c0, &this->scrollMatrix, &local_c0);

	rect.aVertices[0].s1 = 0.0f;
	rect.aVertices[0].t1 = 0.0f;
	rect.aVertices[1].s1 = 1.0f;
	rect.aVertices[1].t1 = 0.0f;
	rect.aVertices[2].s1 = 1.0f;
	rect.aVertices[2].t1 = 1.0f;
	rect.aVertices[3].t1 = 1.0f;
	rect.aVertices[3].s1 = 0.0f;

	// Tiles:
	for (yIndex = 0; yIndex < this->field_0x7c->field_0xc; yIndex = yIndex + 1) {
		x = local_c0.x;
		y = local_c0.y;

		pMapSpriteIndexIt = this->field_0x7c->aTileIndices + yIndex * this->field_0x7c->field_0x8;

		for (xIndex = 0; xIndex < this->field_0x7c->field_0x8; xIndex = xIndex + 1) {
			mapSpriteIndex = *pMapSpriteIndexIt;
			if ((((mapSpriteIndex != -1) && (x <= local_90.x)) && (local_80.x <= x)) && ((y <= local_90.y && (local_80.y <= y)))) {
				bVar4 = false;
				bVar3 = false;
				bVar2 = false;

				rect.aVertices[0].x1 = x - local_30.x;
				rect.aVertices[0].y1 = y - local_30.y;
				rect.aVertices[1].x1 = rect.aVertices[0].x1 + local_10.x;
				rect.aVertices[1].y1 = rect.aVertices[0].y1 + local_10.y;
				rect.aVertices[2].x1 = local_20.x + rect.aVertices[1].x1;
				rect.aVertices[3].x1 = rect.aVertices[0].x1 + local_20.x;
				rect.aVertices[2].y1 = local_20.y + rect.aVertices[1].y1;
				rect.aVertices[3].y1 = rect.aVertices[0].y1 + local_20.y;

				if ((x <= local_b0.x) && (local_a0.x <= x)) {
					bVar2 = true;
				}

				if ((bVar2) && (y <= local_b0.y)) {
					bVar3 = true;
				}

				if ((bVar3) && (local_a0.y <= y)) {
					bVar4 = true;
				}

				if (bVar4) {
					pMapManager->DrawSprite(this->pMaterials + mapSpriteIndex, &rect, 0x80808080);
				}
				else {
					pMapManager->DrawClippedSprite(this->pMaterials + mapSpriteIndex, &rect, 0x80808080);
				}
			}

			x = x + local_10.x;
			y = y + local_10.y;
			pMapSpriteIndexIt = pMapSpriteIndexIt + 1;
		}

		local_c0 = local_c0 + local_20;
	}

	DrawMapOverlays(pObjectiveEntry, param_3);

	return;
}

void CLevelMap::DrawMapOverlays(ObjectiveEntry* pObjectiveEntry, int param_3)
{
	CActorHero* pHero;
	CLevelScheduler* pCVar2;
	CMapManager* pCVar3;
	uint color;
	bool bVar4;
	NativShopLevelSubObj* pNVar5;
	MapPosition* pMVar6;
	edDList_material* peVar8;
	int iVar9;
	int iVar10;
	ObjectiveEntry* pLVar11;
	ObjectiveEntry* pLVar12;
	undefined uVar13;
	float fVar14;
	float fVar15;
	S_2DRECT local_990;
	edF32VECTOR4 local_950;
	edF32VECTOR4 local_940;
	edF32VECTOR4 local_930;
	MapPosition aMStack2336[128];
	MapDataSizes MStack288;
	CActorsTable local_110;
	uint local_8;
	uint local_4;

	pCVar3 = CScene::ptable.g_MapManager_0045168c;
	pCVar2 = CLevelScheduler::gThis;
	pHero = CActorHero::_gThis;
	iVar9 = CLevelScheduler::gThis->currentLevelID;

	edDListLoadIdentity();

	pNVar5 = DrawAndSelectNearestShopMarker();
	if (this->field_0x0 == iVar9) {
		local_110.nbEntries = 0;
		CScene::ptable.g_ActorManager_004516a4->GetActorsByClassID(JAMGUT, &local_110);
		for (iVar10 = 0; iVar10 < local_110.nbEntries; iVar10 = iVar10 + 1) {
			DrawWorldMarkerIcon(25.0f, 25.0f, 7, &local_110.aEntries[iVar10]->currentLocation, 0x80808080);
		}
	}

	pMVar6 = (MapPosition*)0x0;
	bVar4 = pCVar3->LoadMarkerPositionsForLevel(this->field_0x0, &MStack288, aMStack2336, 0x80);
	if (bVar4 != false) {
		pMVar6 = DrawCollectedMarkers(pObjectiveEntry, param_3, &MStack288, aMStack2336);
	}

	if (pNVar5 != (NativShopLevelSubObj*)0x0) {
		if (this->field_0xd0 == 0) {
			edF32Matrix4MulF32Vector4Hard(&local_940, &this->field_0x1b0, &pNVar5->currentLocation);
		}
		else {
			edF32Matrix4MulF32Vector4Hard(&local_950, &this->field_0x130, &pNVar5->currentLocation);
			local_950.x = (local_950.x - 0.5f) / (local_950.z - -1.0f) + 0.5f;
			local_950.y = (local_950.y - 0.5f) / (local_950.z - -1.0f) + 0.5f;
			edF32Matrix4MulF32Vector4Hard(&local_940, &this->scrollMatrix, &local_950);
		}

		fVar15 = fmodf(GetTimer()->totalTime, 0.9f);
		pCVar3 = CScene::ptable.g_MapManager_0045168c;
		fVar15 = fVar15 / 0.9f;
		fVar14 = 1.0f;
		if (fVar15 < 0.2f) {
			fVar14 = fVar15 * 5.0f;
		}
		else {
			if (0.8f < fVar15) {
				fVar14 = cosf((1.0f - fVar15) / 0.2f);
			}
		}

		fVar14 = fVar14 * 128.0f;
		if (fVar14 < 2.147484e+09f) {
			uVar13 = (undefined)static_cast<int>(fVar14);
		}
		else {
			uVar13 = (undefined)static_cast<int>(fVar14 - 2.147484e+09f);
		}
		local_8 = 0x7e5d5b | (uint)uVar13 << 0x18;

		color = local_8;
		local_990.aVertices[2].y1 = ((fVar15 * 2.0f - fVar15 * fVar15 * fVar15) * 0.3f + 1.7f) * 25.0f;
		local_990.aVertices[1].x1 = local_990.aVertices[2].y1 * (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f;
		local_990.aVertices[0].s1 = edF32VECTOR2_00426968.x;
		local_990.aVertices[0].t1 = edF32VECTOR2_00426968.y;
		local_990.aVertices[1].s1 = edF32VECTOR2_00426978.x;
		local_990.aVertices[1].t1 = edF32VECTOR2_00426978.y;
		local_990.aVertices[2].s1 = edF32VECTOR2_00426988.x;
		local_990.aVertices[2].t1 = edF32VECTOR2_00426988.y;
		local_990.aVertices[3].s1 = edF32VECTOR2_00426998.x;
		local_990.aVertices[3].t1 = edF32VECTOR2_00426998.y;
		local_990.aVertices[2].y1 = local_990.aVertices[2].y1 * 0.5f;
		local_990.aVertices[0].x1 = local_940.x - local_990.aVertices[1].x1;
		local_990.aVertices[1].x1 = local_940.x + local_990.aVertices[1].x1;
		local_990.aVertices[0].y1 = local_940.y - local_990.aVertices[2].y1;
		local_990.aVertices[2].y1 = local_940.y + local_990.aVertices[2].y1;
		local_990.aVertices[1].y1 = local_990.aVertices[0].y1;
		local_990.aVertices[2].x1 = local_990.aVertices[1].x1;
		local_990.aVertices[3].x1 = local_990.aVertices[0].x1;
		local_990.aVertices[3].y1 = local_990.aVertices[2].y1;
		if ((((local_990.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_990.aVertices[1].x1)) ||
			(local_990.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y)) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_990.aVertices[2].y1)) {
			peVar8 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(0xe);
			pCVar3->DrawClippedSprite(peVar8, &local_990, color);
		}
		else {
			peVar8 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(0xe);
			pCVar3->DrawSprite(peVar8, &local_990, color);
		}
	}

	if ((this->field_0x0 == iVar9) && (pHero != (CActorHero*)0x0)) {
		local_4 = 0x80808080;
		fVar15 = (cosf(GetTimer()->totalTime * 6.283185f - 1.570796f)) * 0.5f;
		fVar14 = fVar15 * 128.0f;
		if (fVar14 < 2.147484e+09f) {
			local_4 = local_4 & 0xffffff | static_cast<int>(fVar14) << 0x18;
		}
		else {
			local_4 = local_4 & 0xffffff | static_cast<int>(fVar14 - 2.147484e+09f) << 0x18;
		}

		DrawOrientedMarkerIcon(37.5f, 37.5f, 0, &pHero->currentLocation, &pHero->rotationQuat, local_4);

		fVar14 = (1.0f - fVar15) * 128.0f;
		if (fVar14 < 2.147484e+09f) {
			local_4 = local_4 & 0xffffff | static_cast<int>(fVar14) << 0x18;
		}
		else {
			local_4 = local_4 & 0xffffff | static_cast<int>(fVar14 - 2.147484e+09f) << 0x18;
		}

		DrawOrientedMarkerIcon(37.5f, 37.5f, 0xc, &pHero->currentLocation, &pHero->rotationQuat, local_4);
		DrawOffscreenMarkerArrow(25.0f, 25.0f, &pHero->currentLocation, 0x1d2378);
	}

	pLVar11 = pCVar2->field_0x4220;
	pLVar12 = pLVar11 + pCVar2->objCount_0x4218;
	for (; pLVar11 < pLVar12; pLVar11 = pLVar11 + 1) {
		iVar9 = CLevelScheduler::ScenVar_Get(pLVar11->field_0x0);
		if (((iVar9 == 1) && (this->field_0x0 == pLVar11->levelId)) && (pLVar11->field_0x4 == 1)) {
			DrawPulsingMarkerHighlight(12.5f, 12.5f, &pLVar11->field_0x10, 0x606060);
		}
	}

	if (pMVar6 != (MapPosition*)0x0) {
		local_930.xyz = pMVar6->position;
		local_930.w = 1.0f;
		DrawPulsingMarkerHighlight(25.0f, 25.0f, &local_930, 0x7f7f7f);
		DrawOffscreenMarkerArrow(25.0f, 25.0f, &local_930, 0x7f7f7f);
	}

	if (pObjectiveEntry != (ObjectiveEntry*)0x0) {
		DrawObjectiveTargetMarkers(pObjectiveEntry, param_3, &MStack288, aMStack2336);
	}

	if (pNVar5 != (NativShopLevelSubObj*)0x0) {
		DrawSelectedShop(pNVar5, pObjectiveEntry, param_3);
	}

	return;
}

void CLevelMap::DrawWorldMarkerIcon(float param_1, float param_2, int param_4, edF32VECTOR4* param_5, uint param_6)
{
	CMapManager* pCVar1;
	uint color;
	edDList_material* peVar2;
	int index;
	float fVar3;
	S_2DRECT local_b0;
	S_2DRECT local_70;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	uint local_4;

	if (this->field_0xd0 == 0) {
		edF32Matrix4MulF32Vector4Hard(&local_20, &this->field_0x1b0, param_5);
	}
	else {
		edF32Matrix4MulF32Vector4Hard(&local_30, &this->field_0x130, param_5);
		local_30.x = (local_30.x - 0.5f) / (local_30.z - -1.0f) + 0.5f;
		local_30.y = (local_30.y - 0.5f) / (local_30.z - -1.0f) + 0.5f;
		edF32Matrix4MulF32Vector4Hard(&local_20, &this->scrollMatrix, &local_30);
	}

	pCVar1 = CScene::ptable.g_MapManager_0045168c;
	local_70.aVertices[0].y1 = local_20.y - param_2 * 0.5f;
	local_70.aVertices[2].y1 = local_20.y + param_2 * 0.5f;
	local_70.aVertices[0].s1 = edF32VECTOR2_00426968.x;
	local_70.aVertices[0].t1 = edF32VECTOR2_00426968.y;
	local_70.aVertices[1].s1 = edF32VECTOR2_00426978.x;
	local_70.aVertices[1].t1 = edF32VECTOR2_00426978.y;
	local_70.aVertices[2].s1 = edF32VECTOR2_00426988.x;
	local_70.aVertices[2].t1 = edF32VECTOR2_00426988.y;
	local_70.aVertices[3].s1 = edF32VECTOR2_00426998.x;
	local_70.aVertices[3].t1 = edF32VECTOR2_00426998.y;
	local_70.aVertices[1].x1 = param_1 * (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f;
	local_70.aVertices[0].x1 = local_20.x - local_70.aVertices[1].x1;
	local_70.aVertices[1].x1 = local_20.x + local_70.aVertices[1].x1;
	local_70.aVertices[1].y1 = local_70.aVertices[0].y1;
	local_70.aVertices[2].x1 = local_70.aVertices[1].x1;
	local_70.aVertices[3].x1 = local_70.aVertices[0].x1;
	local_70.aVertices[3].y1 = local_70.aVertices[2].y1;
	if ((((local_70.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_70.aVertices[1].x1)) ||
		(local_70.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y)) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_70.aVertices[2].y1)) {
		peVar2 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(param_4);
		pCVar1->DrawClippedSprite(peVar2, &local_70, param_6);
	}
	else {
		peVar2 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(param_4);
		pCVar1->DrawSprite(peVar2, &local_70, param_6);
	}

	pCVar1 = CScene::ptable.g_MapManager_0045168c;
	if ((this->field_0x0 == CLevelScheduler::gThis->currentLevelID) && (CActorHero::_gThis != (CActorHero*)0x0)) {
		local_4 = param_6 & 0xff000000 | 0x7f7f7f;
		color = local_4;
		fVar3 = CActorHero::_gThis->currentLocation.y;
		index = 0xf;
		if (param_5->y <= fVar3 - 10.0f) {
			index = 1;
		}
		else {
			if (fVar3 + 10.0f <= param_5->y) {
				index = 2;
			}
		}

		if (index != 0xf) {
			local_b0.aVertices[0].y1 = local_20.y - param_2 * 0.5f;
			local_b0.aVertices[2].y1 = local_20.y + param_2 * 0.5f;
			local_b0.aVertices[0].s1 = edF32VECTOR2_00426968.x;
			local_b0.aVertices[0].t1 = edF32VECTOR2_00426968.y;
			local_b0.aVertices[1].s1 = edF32VECTOR2_00426978.x;
			local_b0.aVertices[1].t1 = edF32VECTOR2_00426978.y;
			local_b0.aVertices[2].s1 = edF32VECTOR2_00426988.x;
			local_b0.aVertices[2].t1 = edF32VECTOR2_00426988.y;
			local_b0.aVertices[3].s1 = edF32VECTOR2_00426998.x;
			local_b0.aVertices[3].t1 = edF32VECTOR2_00426998.y;
			local_b0.aVertices[1].x1 = param_1 * (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f;
			local_b0.aVertices[0].x1 = local_20.x - local_b0.aVertices[1].x1;
			local_b0.aVertices[1].x1 = local_20.x + local_b0.aVertices[1].x1;
			local_b0.aVertices[1].y1 = local_b0.aVertices[0].y1;
			local_b0.aVertices[2].x1 = local_b0.aVertices[1].x1;
			local_b0.aVertices[3].x1 = local_b0.aVertices[0].x1;
			local_b0.aVertices[3].y1 = local_b0.aVertices[2].y1;

			if (((local_b0.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_b0.aVertices[1].x1)) ||
				((local_b0.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_b0.aVertices[2].y1)))) {
				peVar2 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(index);
				pCVar1->DrawClippedSprite(peVar2, &local_b0, color);
			}
			else {
				peVar2 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(index);
				pCVar1->DrawSprite(peVar2, &local_b0, color);
			}
		}
	}

	return;
}

MapPosition* CLevelMap::DrawCollectedMarkers(ObjectiveEntry* pObjectiveEntry, int param_3, MapDataSizes* param_4, MapPosition* param_5)
{
	float fVar1;
	CActorHero* pCVar2;
	MapPosition* pMVar3;
	int iVar5;
	MapPosition* pTeleporterBuffer;
	MapPosition* pRuneBuffer;
	MapPosition* pWolfenBuffer;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 local_20;
	uint local_4;

	pCVar2 = CActorHero::_gThis;
	pMVar3 = (MapPosition*)0x0;
	pTeleporterBuffer = param_5 + param_4->field_0x0;
	pWolfenBuffer = pTeleporterBuffer + param_4->nbTeleporters;
	pRuneBuffer = pWolfenBuffer + param_4->nbWolfen;
	if ((pObjectiveEntry != (ObjectiveEntry*)0x0) && (pObjectiveEntry->levelId != this->field_0x0)) {
		pMVar3 = FindMarkerForLevel(pObjectiveEntry->levelId, param_4, param_5, 1);
	}

	fVar9 = GetTimer()->totalTime - this->field_0xe4;
	if (((this->field_0xe4 == 0.0f) || (100.0f < fVar9)) || (pCVar2 == (CActorHero*)0x0)) {
		for (iVar5 = 0; iVar5 < param_4->nbWolfen; iVar5 = iVar5 + 1) {
			local_20.xyz = pWolfenBuffer->position;
			local_20.w = 1.0f;
			DrawWorldMarkerIcon(25.0f, 25.0f, 3, &local_20, 0x80808080);
			pWolfenBuffer = pWolfenBuffer + 1;
		}
	}
	else {
		fVar9 = fVar9 + 1.0f;
		fVar11 = fVar9 * fVar9 * 4.0f - 4.0f;
		fVar9 = fVar11 * fVar9 * 7.0f;
		for (iVar5 = 0; iVar5 < param_4->nbWolfen; iVar5 = iVar5 + 1) {
			local_20.xyz = pWolfenBuffer->position;
			local_20.w = 1.0f;
			fVar10 = local_20.x - pCVar2->currentLocation.x;
			fVar1 = local_20.z - pCVar2->currentLocation.z;
			fVar10 = sqrtf(fVar10 * fVar10 + 0.0f + fVar1 * fVar1);
			if (fVar10 < fVar11) {
				local_4 = 0x807f7f7f;
			}
			else {
				if (fVar10 < fVar9) {
					fVar10 = ((fVar9 - fVar10) * 128.0f) / (fVar9 - fVar11);
					if (2.147484e+09f <= fVar10) {
						fVar10 = fVar10 - 2.147484e+09f;
					}

					local_4 = static_cast<uint>((int)fVar10 << 0x18 | 0x7f7f7f);
				}
				else {
					local_4 = 0x007f7f7f;
				}
			}

			DrawWorldMarkerIcon(25.0f, 25.0f, 3, &local_20, local_4);
			pWolfenBuffer = pWolfenBuffer + 1;
		}
	}

	for (iVar5 = 0; iVar5 < param_4->nbTeleporters; iVar5 = iVar5 + 1) {
		local_20.xyz = pTeleporterBuffer->position;
		local_20.w = 1.0f;
		DrawWorldMarkerIcon(25.0f, 25.0f, 4, &local_20, 0x80808080);
		pTeleporterBuffer = pTeleporterBuffer + 1;
	}

	iVar5 = CLevelScheduler::ScenVar_Get(0xf);
	if (iVar5 != 0) {
		for (iVar5 = 0; iVar5 < param_4->nbRunes; iVar5 = iVar5 + 1) {
			local_20.xyz = pRuneBuffer->position;
			local_20.w = 1.0f;
			DrawWorldMarkerIcon(25.0f, 25.0f, 6, &local_20, 0x80808080);

			if (param_3 == 1) {
				DrawPulsingMarkerHighlight(25.0f, 25.0f, &local_20, 0x613021);
				DrawOffscreenMarkerArrow(25.0f, 25.0f, &local_20, 0x613021);
			}

			pRuneBuffer = pRuneBuffer + 1;
		}
	}

	pWolfenBuffer = param_5;
	for (iVar5 = 0; iVar5 < param_4->field_0x0; iVar5 = iVar5 + 1) {
		local_20.xyz = pWolfenBuffer->position;
		local_20.w = 1.0f;
		DrawWorldMarkerIcon(25.0f, 25.0f, 5, &local_20, 0x80808080);
		pWolfenBuffer = pWolfenBuffer + 1;
	}

	DrawMarkerLabels(param_5, param_4->field_0x0);

	return pMVar3;
}

void CLevelMap::DrawPulsingMarkerHighlight(float param_1, float param_2, edF32VECTOR4* param_4, undefined3 param_5)
{
	CMapManager* pCVar1;
	uint uVar2;
	edDList_material* peVar4;
	int index;
	undefined uVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	S_2DRECT local_b0;
	S_2DRECT local_70;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	uint local_8;
	uint local_4;

	if (this->field_0xd0 == 0) {
		edF32Matrix4MulF32Vector4Hard(&local_20, &this->field_0x1b0, param_4);
	}
	else {
		edF32Matrix4MulF32Vector4Hard(&local_30, &this->field_0x130, param_4);
		local_30.x = (local_30.x - 0.5f) / (local_30.z - -1.0f) + 0.5f;
		local_30.y = (local_30.y - 0.5f) / (local_30.z - -1.0f) + 0.5f;
		edF32Matrix4MulF32Vector4Hard(&local_20, &this->scrollMatrix, &local_30);
	}

	fVar7 = fmodf(GetTimer()->totalTime, 0.9f);
	pCVar1 = CScene::ptable.g_MapManager_0045168c;
	fVar6 = 1.0f;
	fVar7 = fVar7 / 0.9f;
	fVar8 = (fVar7 * 2.0f - fVar7 * fVar7 * fVar7) * 2.0f;
	if (0.66f < fVar7) {
		fVar6 = cosf((((1.0f - fVar7) / 0.34f) * 1.570796f - 1.570796f));
	}

	fVar6 = fVar6 * 128.0f;
	if (fVar6 < 2.147484e+09f) {
		uVar5 = (undefined)static_cast<int>(fVar6);
	}
	else {
		uVar5 = (undefined)static_cast<int>(fVar6 - 2.147484e+09f);
	}

	local_4 = param_5 & 0xffffff | (uint)uVar5 << 0x18;

	uVar2 = local_4;
	local_70.aVertices[2].y1 = param_2 * fVar8 * 0.5f;
	local_70.aVertices[0].x1 = param_1 * fVar8 * (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f;
	local_70.aVertices[0].s1 = edF32VECTOR2_00426968.x;
	local_70.aVertices[0].t1 = edF32VECTOR2_00426968.y;
	local_70.aVertices[1].x1 = local_20.x + local_70.aVertices[0].x1;
	local_70.aVertices[1].s1 = edF32VECTOR2_00426978.x;
	local_70.aVertices[1].t1 = edF32VECTOR2_00426978.y;
	local_70.aVertices[2].s1 = edF32VECTOR2_00426988.x;
	local_70.aVertices[2].t1 = edF32VECTOR2_00426988.y;
	local_70.aVertices[3].s1 = edF32VECTOR2_00426998.x;
	local_70.aVertices[3].t1 = edF32VECTOR2_00426998.y;
	local_70.aVertices[0].x1 = local_20.x - local_70.aVertices[0].x1;
	local_70.aVertices[0].y1 = local_20.y - local_70.aVertices[2].y1;
	local_70.aVertices[2].y1 = local_20.y + local_70.aVertices[2].y1;
	local_70.aVertices[1].y1 = local_70.aVertices[0].y1;
	local_70.aVertices[2].x1 = local_70.aVertices[1].x1;
	local_70.aVertices[3].x1 = local_70.aVertices[0].x1;
	local_70.aVertices[3].y1 = local_70.aVertices[2].y1;

	if ((((local_70.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_70.aVertices[1].x1)) ||
		(local_70.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y)) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_70.aVertices[2].y1)) {
		peVar4 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(0xd);
		pCVar1->DrawClippedSprite(peVar4, &local_70, uVar2);
	}
	else {
		peVar4 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(0xd);
		pCVar1->DrawSprite(peVar4, &local_70, uVar2);
	}

	pCVar1 = CScene::ptable.g_MapManager_0045168c;
	if ((this->field_0x0 == CLevelScheduler::gThis->currentLevelID) && (CActorHero::_gThis != (CActorHero*)0x0)) {
		local_8 = local_4 & 0xff000000 | 0x7f7f7f;
		uVar2 = local_8;
		fVar6 = CActorHero::_gThis->currentLocation.y;
		index = 0xf;
		if (param_4->y <= fVar6 - 10.0f) {
			index = 1;
		}
		else {
			if (fVar6 + 10.0f <= param_4->y) {
				index = 2;
			}
		}

		if (index != 0xf) {
			fVar6 = param_1 * fVar8 * 0.5f;
			fVar7 = param_2 * fVar8 * 0.5f;
			local_b0.aVertices[2].y1 = fVar7 * 0.5f;
			local_20.y = local_20.y - fVar7 * 0.5f;
			local_b0.aVertices[0].s1 = edF32VECTOR2_00426968.x;
			local_b0.aVertices[0].t1 = edF32VECTOR2_00426968.y;
			local_b0.aVertices[1].s1 = edF32VECTOR2_00426978.x;
			local_b0.aVertices[1].t1 = edF32VECTOR2_00426978.y;
			local_b0.aVertices[2].s1 = edF32VECTOR2_00426988.x;
			local_b0.aVertices[2].t1 = edF32VECTOR2_00426988.y;
			local_b0.aVertices[3].s1 = edF32VECTOR2_00426998.x;
			local_b0.aVertices[3].t1 = edF32VECTOR2_00426998.y;
			local_b0.aVertices[0].x1 = fVar6 * (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f;
			local_20.x = local_20.x + fVar6 * 0.5f;
			local_b0.aVertices[1].x1 = local_20.x + local_b0.aVertices[0].x1;
			local_b0.aVertices[0].x1 = local_20.x - local_b0.aVertices[0].x1;
			local_b0.aVertices[0].y1 = local_20.y - local_b0.aVertices[2].y1;
			local_b0.aVertices[2].y1 = local_20.y + local_b0.aVertices[2].y1;
			local_b0.aVertices[1].y1 = local_b0.aVertices[0].y1;
			local_b0.aVertices[2].x1 = local_b0.aVertices[1].x1;
			local_b0.aVertices[3].x1 = local_b0.aVertices[0].x1;
			local_b0.aVertices[3].y1 = local_b0.aVertices[2].y1;
			if (((local_b0.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_b0.aVertices[1].x1)) ||
				((local_b0.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_b0.aVertices[2].y1)))) {
				peVar4 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(index);
				pCVar1->DrawClippedSprite(peVar4, &local_b0, uVar2);
			}
			else {
				peVar4 = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(index);
				pCVar1->DrawSprite(peVar4, &local_b0, uVar2);
			}
		}
	}

	return;
}

edF32VECTOR2 edF32VECTOR2_004268e8 = { 0.0f, 0.0f };
edF32VECTOR2 edF32VECTOR2_004268f8 = { 1.0f, 0.0f };
edF32VECTOR2 edF32VECTOR2_00426908 = { 1.0f, 1.0f };
edF32VECTOR2 edF32VECTOR2_00426918 = { 0.0f, 1.0f };

void CLevelMap::DrawOffscreenMarkerArrow(float param_1, float param_2, edF32VECTOR4* param_4, uint param_5)
{
	bool bVar1;
	bool bVar2;
	bool bVar3;
	float* pfVar4;
	CMapManager* this_00;
	float* pfVar5;
	float* pfVar7;
	uint uVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 local_e0;
	S_2DRECT local_d0;
	float local_90;
	float local_8c;
	float fStack136;
	float fStack132;
	float local_80;
	float local_7c;
	float fStack120;
	float fStack116;
	float local_70;
	float local_6c;
	float fStack104;
	float fStack100;
	float local_60;
	float local_5c;
	float fStack88;
	float fStack84;
	float local_50;
	float local_4c;
	float fStack72;
	float fStack68;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	_rgba local_4;

	if (this->field_0xd0 == 0) {
		edF32Matrix4MulF32Vector4Hard(&local_20, &this->field_0x1b0, param_4);
	}
	else {
		edF32Matrix4MulF32Vector4Hard(&local_e0, &this->field_0x130, param_4);
		local_e0.x = (local_e0.x - 0.5f) / (local_e0.z - -1.0f) + 0.5f;
		local_e0.y = (local_e0.y - 0.5f) / (local_e0.z - -1.0f) + 0.5f;
		edF32Matrix4MulF32Vector4Hard(&local_20, &this->scrollMatrix, &local_e0);
	}
	bVar3 = true;
	bVar2 = true;
	fVar9 = local_20.x;
	fVar10 = local_20.y;
	local_30.z = local_20.z;
	local_30.w = local_20.w;
	bVar1 = true;
	fVar11 = (static_cast<float>(gVideoConfig.screenHeight) * 20.0f) / 512.0f;
	if (((this->field_0x80).x - 24.0f <= fVar9) && (fVar9 <= (this->field_0x90).x + 24.0f)) {
		bVar1 = false;
	}

	if ((!bVar1) && ((this->field_0x80).y - fVar11 <= fVar10)) {
		bVar2 = false;
	}

	if ((!bVar2) && (fVar10 <= (this->field_0x90).y + fVar11)) {
		bVar3 = false;
	}

	if (bVar3) {
		local_30.x = (this->field_0x80).x;
		if ((local_30.x <= fVar9) && (local_30.x = (this->field_0x90).x, fVar9 <= local_30.x)) {
			local_30.x = fVar9;
		}

		local_30.y = (this->field_0x80).y;
		if ((local_30.y <= fVar10) && (local_30.y = (this->field_0x90).y, fVar10 <= local_30.y)) {
			local_30.y = fVar10;
		}

		edF32Vector4SubHard(&local_40, &local_30, &local_20);
		local_40.z = 0.0f;
		edF32Vector4SafeNormalize1Hard(&local_40, &local_40);
		this_00 = CScene::ptable.g_MapManager_0045168c;
		local_50 = {};
		
		fVar11 = param_1 * 0.5f;
		fVar9 = -local_40.y;
		local_40.y = local_40.y * fVar11;
		local_40.z = local_40.z * fVar11;
		local_40.w = local_40.w * fVar11;
		fVar10 = param_2 * 0.5f;
		local_4c = local_40.x * fVar10;
		fStack72 = fStack72 * fVar10;
		fStack68 = fStack68 * fVar10;
		local_40.x = local_40.x * fVar11 * (CScene::ptable.g_MapManager_0045168c)->field_0x388;
		local_50 = fVar9 * fVar10 * (CScene::ptable.g_MapManager_0045168c)->field_0x388;
		local_d0.aVertices[0].x1 = (local_30.x - local_40.x) - local_50;
		local_d0.aVertices[0].y1 = (local_30.y - local_40.y) - local_4c;
		fStack136 = (local_30.z - local_40.z) - fStack72;
		fStack132 = (local_30.w - local_40.w) - fStack68;
		local_d0.aVertices[1].x1 = (local_30.x + local_40.x) - local_50;
		local_d0.aVertices[1].y1 = (local_30.y + local_40.y) - local_4c;
		fStack120 = (local_30.z + local_40.z) - fStack72;
		fStack116 = (local_30.w + local_40.w) - fStack68;
		local_d0.aVertices[2].x1 = local_30.x + local_40.x + local_50;
		local_d0.aVertices[2].y1 = local_30.y + local_40.y + local_4c;
		fStack104 = local_30.z + local_40.z + fStack72;
		fStack100 = local_30.w + local_40.w + fStack68;
		local_d0.aVertices[3].x1 = (local_30.x - local_40.x) + local_50;
		local_d0.aVertices[3].y1 = (local_30.y - local_40.y) + local_4c;
		fStack88 = (local_30.z - local_40.z) + fStack72;
		fStack84 = (local_30.w - local_40.w) + fStack68;

		local_d0.aVertices[0].s1 = edF32VECTOR2_004268e8.x;
		local_d0.aVertices[0].t1 = edF32VECTOR2_004268e8.y;
		local_d0.aVertices[1].s1 = edF32VECTOR2_004268f8.x;
		local_d0.aVertices[1].t1 = edF32VECTOR2_004268f8.y;
		local_d0.aVertices[2].s1 = edF32VECTOR2_00426908.x;
		local_d0.aVertices[2].t1 = edF32VECTOR2_00426908.y;
		local_d0.aVertices[3].s1 = edF32VECTOR2_00426918.x;
		local_d0.aVertices[3].t1 = edF32VECTOR2_00426918.y;

		local_90 = local_d0.aVertices[0].x1;
		local_8c = local_d0.aVertices[0].y1;
		local_80 = local_d0.aVertices[1].x1;
		local_7c = local_d0.aVertices[1].y1;
		local_70 = local_d0.aVertices[2].x1;
		local_6c = local_d0.aVertices[2].y1;
		local_60 = local_d0.aVertices[3].x1;
		local_5c = local_d0.aVertices[3].y1;
		fVar9 = GetTimer()->totalTime * 256.0f;
		if (fVar9 < 2.147484e+09f) {
			uVar8 = static_cast<uint>(fVar9);
		}
		else {
			uVar8 = static_cast<int>(fVar9 - 2.147484e+09f) | 0x80000000;
		}

		uVar8 = static_cast<int>(uVar8) % 0xa0;
		local_4.a = (byte)uVar8;
		if (0x80 < local_4.a) {
			uVar8 = (uint)(byte)((' ' - (local_4.a + 0x80)) * '\x04');
		}
		local_4 = param_5 & 0xffffff | uVar8 << 0x18;
		this_00->DrawSprite(&MenuBitmaps[10].materialInfo, &local_d0, local_4.rgba);
	}

	return;
}

edF32VECTOR2 edF32VECTOR2_00426928 = { 0.0f, 1.0f };
edF32VECTOR2 edF32VECTOR2_00426938 = { 0.0f, 0.0f };
edF32VECTOR2 edF32VECTOR2_00426948 = { 1.0f, 0.0f };
edF32VECTOR2 edF32VECTOR2_00426958 = { 1.0f, 1.0f };

void CLevelMap::DrawOrientedMarkerIcon(float param_1, float param_2, int param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, uint param_7)
{
	float* pfVar1;
	float* pfVar2;
	edDList_material* pMaterial;
	float* pfVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 local_c0;
	S_2DRECT local_b0;
	float local_70;
	float local_6c;
	float fStack104;
	float fStack100;
	float local_60;
	float local_5c;
	float fStack88;
	float fStack84;
	float local_50;
	float local_4c;
	float fStack72;
	float fStack68;
	float local_40;
	float local_3c;
	float fStack56;
	float fStack52;
	float local_30;
	float local_2c;
	float fStack40;
	float fStack36;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CMapManager* pMapManager;

	if (this->field_0xd0 == 0) {
		edF32Matrix4MulF32Vector4Hard(&local_10, &this->field_0x1b0, param_5);
	}
	else {
		edF32Matrix4MulF32Vector4Hard(&local_c0, &this->field_0x130, param_5);
		local_c0.x = (local_c0.x - 0.5f) / (local_c0.z - -1.0f) + 0.5f;
		local_c0.y = (local_c0.y - 0.5f) / (local_c0.z - -1.0f) + 0.5f;
		edF32Matrix4MulF32Vector4Hard(&local_10, &this->scrollMatrix, &local_c0);
	}
	edF32Matrix4MulF32Vector4Hard(&local_20, &this->field_0x1b0, param_6);
	local_20.z = 0.0f;
	edF32Vector4SafeNormalize1Hard(&local_20, &local_20);
	pMapManager = CScene::ptable.g_MapManager_0045168c;
	local_30 = {};
	fVar6 = param_1 * 0.5f;
	fVar4 = -local_20.y;
	local_20.y = local_20.y * fVar6;
	local_20.z = local_20.z * fVar6;
	local_20.w = local_20.w * fVar6;
	fVar5 = param_2 * 0.5f;
	local_2c = local_20.x * fVar5;
	fStack40 = fStack40 * fVar5;
	fStack36 = fStack36 * fVar5;
	local_20.x = local_20.x * fVar6 * (CScene::ptable.g_MapManager_0045168c)->field_0x388;
	local_30 = fVar4 * fVar5 * (CScene::ptable.g_MapManager_0045168c)->field_0x388;
	local_b0.aVertices[0].x1 = (local_10.x - local_20.x) - local_30;
	local_b0.aVertices[0].y1 = (local_10.y - local_20.y) - local_2c;
	fStack104 = (local_10.z - local_20.z) - fStack40;
	fStack100 = (local_10.w - local_20.w) - fStack36;
	local_b0.aVertices[1].x1 = (local_10.x + local_20.x) - local_30;
	local_b0.aVertices[1].y1 = (local_10.y + local_20.y) - local_2c;
	fStack88 = (local_10.z + local_20.z) - fStack40;
	fStack84 = (local_10.w + local_20.w) - fStack36;
	local_b0.aVertices[2].x1 = local_10.x + local_20.x + local_30;
	local_b0.aVertices[2].y1 = local_10.y + local_20.y + local_2c;
	fStack72 = local_10.z + local_20.z + fStack40;
	fStack68 = local_10.w + local_20.w + fStack36;
	local_b0.aVertices[3].x1 = (local_10.x - local_20.x) + local_30;
	local_b0.aVertices[3].y1 = (local_10.y - local_20.y) + local_2c;
	fStack56 = (local_10.z - local_20.z) + fStack40;
	fStack52 = (local_10.w - local_20.w) + fStack36;
	local_b0.aVertices[0].s1 = edF32VECTOR2_00426928.x;
	local_b0.aVertices[0].t1 = edF32VECTOR2_00426928.y;
	local_b0.aVertices[1].s1 = edF32VECTOR2_00426938.x;
	local_b0.aVertices[1].t1 = edF32VECTOR2_00426938.y;
	local_b0.aVertices[2].s1 = edF32VECTOR2_00426948.x;
	local_b0.aVertices[2].t1 = edF32VECTOR2_00426948.y;
	local_b0.aVertices[3].s1 = edF32VECTOR2_00426958.x;
	local_b0.aVertices[3].t1 = edF32VECTOR2_00426958.y;
	local_70 = local_b0.aVertices[0].x1;
	local_6c = local_b0.aVertices[0].y1;
	local_60 = local_b0.aVertices[1].x1;
	local_5c = local_b0.aVertices[1].y1;
	local_50 = local_b0.aVertices[2].x1;
	local_4c = local_b0.aVertices[2].y1;
	local_40 = local_b0.aVertices[3].x1;
	local_3c = local_b0.aVertices[3].y1;
	pMaterial = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(param_4);
	pMapManager->DrawClippedSprite(pMaterial, &local_b0, param_7);

	return;
}

NativShopLevelSubObj* CLevelMap::DrawAndSelectNearestShopMarker()
{
	int iVar1;
	bool bVar2;
	edDList_material* pDlistMaterial;
	int iVar3;
	NativShopLevelSubObj* this_00;
	int iVar4;
	NativShopLevelSubObj* pNVar5;
	NativShopLevelSubObj* pNVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	S_2DRECT local_b0;
	S_2DRECT local_70;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;
	CMapManager* pMapManager;

	fVar9 = 1e+10;
	pNVar6 = (NativShopLevelSubObj*)0x0;
	iVar3 = 0;
	iVar1 = CLevelScheduler::gThis->nbNativShopSubObjs;
	this_00 = CLevelScheduler::gThis->aNativShopSubObjs;
	pNVar5 = pNVar6;
	if (0 < iVar1) {
		do {
			fVar10 = fVar9;
			pNVar6 = pNVar5;
			if (this_00->currentLevelId == this->field_0x0) {
				bVar2 = this_00->FUN_002d8cb0();
				iVar4 = 8;
				if (bVar2 == false) {
					iVar4 = 9;
				}

				if (this->field_0xd0 == 0) {
					edF32Matrix4MulF32Vector4Hard(&local_10, &this->field_0x1b0, &this_00->currentLocation);
				}
				else {
					edF32Matrix4MulF32Vector4Hard(&local_30, &this->field_0x130, &this_00->currentLocation);
					local_30.x = (local_30.x - 0.5f) / (local_30.z - -1.0f) + 0.5f;
					local_30.y = (local_30.y - 0.5f) / (local_30.z - -1.0f) + 0.5f;
					edF32Matrix4MulF32Vector4Hard(&local_10, &this->scrollMatrix, &local_30);
				}

				pMapManager = CScene::ptable.g_MapManager_0045168c;
				fVar10 = this->field_0xc4 * 25.0f;
				local_70.aVertices[0].s1 = edF32VECTOR2_00426968.x;
				local_70.aVertices[0].t1 = edF32VECTOR2_00426968.y;
				local_70.aVertices[1].s1 = edF32VECTOR2_00426978.x;
				local_70.aVertices[1].t1 = edF32VECTOR2_00426978.y;
				local_70.aVertices[2].s1 = edF32VECTOR2_00426988.x;
				local_70.aVertices[2].t1 = edF32VECTOR2_00426988.y;
				local_70.aVertices[3].s1 = edF32VECTOR2_00426998.x;
				local_70.aVertices[3].t1 = edF32VECTOR2_00426998.y;
				local_70.aVertices[3].x1 = fVar10 * (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f;
				local_70.aVertices[0].y1 = local_10.y - fVar10 * 0.5f;
				local_70.aVertices[2].y1 = local_10.y + fVar10 * 0.5f;
				local_70.aVertices[0].x1 = local_10.x - local_70.aVertices[3].x1;
				local_70.aVertices[1].x1 = local_10.x + local_70.aVertices[3].x1;
				local_70.aVertices[2].x1 = local_10.x + local_70.aVertices[3].x1;
				local_70.aVertices[3].x1 = local_10.x - local_70.aVertices[3].x1;
				local_70.aVertices[1].y1 = local_70.aVertices[0].y1;
				local_70.aVertices[3].y1 = local_70.aVertices[2].y1;
				if ((((local_70.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_70.aVertices[1].x1)) ||
					(local_70.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y)) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_70.aVertices[2].y1)) {
					pDlistMaterial = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(iVar4);
					CScene::ptable.g_MapManager_0045168c->DrawClippedSprite(pDlistMaterial, &local_70, 0x80808080);
				}
				else {
					pDlistMaterial = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(iVar4);
					CScene::ptable.g_MapManager_0045168c->DrawSprite(pDlistMaterial, &local_70, 0x80808080);
				}

				pMapManager = CScene::ptable.g_MapManager_0045168c;
				if ((this->field_0x0 == CLevelScheduler::gThis->currentLevelID) && (CActorHero::_gThis != (CActorHero*)0x0)) {
					fVar7 = CActorHero::_gThis->currentLocation.y;
					fVar8 = (this_00->currentLocation).y;
					iVar4 = 0xf;

					if (fVar8 <= fVar7 - 10.0f) {
						iVar4 = 1;
					}
					else {
						if (fVar7 + 10.0f <= fVar8) {
							iVar4 = 2;
						}
					}

					if (iVar4 != 0xf) {
						fVar8 = fVar10 * 0.5f;
						local_b0.aVertices[0].s1 = edF32VECTOR2_00426968.x;
						local_b0.aVertices[0].t1 = edF32VECTOR2_00426968.y;
						local_b0.aVertices[1].s1 = edF32VECTOR2_00426978.x;
						local_b0.aVertices[1].t1 = edF32VECTOR2_00426978.y;
						local_b0.aVertices[2].s1 = edF32VECTOR2_00426988.x;
						local_b0.aVertices[2].t1 = edF32VECTOR2_00426988.y;
						local_b0.aVertices[3].s1 = edF32VECTOR2_00426998.x;
						local_b0.aVertices[3].t1 = edF32VECTOR2_00426998.y;
						fVar7 = fVar8 * (CScene::ptable.g_MapManager_0045168c)->field_0x388 * 0.5f;
						local_b0.aVertices[2].y1 = fVar8 * 0.5f + (local_10.y - fVar10 * 0.4f);
						local_b0.aVertices[1].x1 = (local_10.x + fVar10 * 0.4f) - fVar8 * 0.5f;
						local_b0.aVertices[0].x1 = local_b0.aVertices[1].x1 - fVar7;
						local_b0.aVertices[1].x1 = local_b0.aVertices[1].x1 + fVar7;
						local_b0.aVertices[0].y1 = local_b0.aVertices[2].y1 - fVar8 * 0.5f;
						local_b0.aVertices[2].y1 = local_b0.aVertices[2].y1 + fVar8 * 0.5f;
						local_b0.aVertices[1].y1 = local_b0.aVertices[0].y1;
						local_b0.aVertices[2].x1 = local_b0.aVertices[1].x1;
						local_b0.aVertices[3].x1 = local_b0.aVertices[0].x1;
						local_b0.aVertices[3].y1 = local_b0.aVertices[2].y1;
						if (((local_b0.aVertices[0].x1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).x) || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).x < local_b0.aVertices[1].x1)) ||
							((local_b0.aVertices[0].y1 < ((CScene::ptable.g_MapManager_0045168c)->field_0x310).y || (((CScene::ptable.g_MapManager_0045168c)->field_0x320).y < local_b0.aVertices[2].y1)))) {
							pDlistMaterial = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(iVar4);
							pMapManager->DrawClippedSprite(pDlistMaterial, &local_b0, 0x80808080);
						}
						else {
							pDlistMaterial = CScene::ptable.g_MapManager_0045168c->GetDlistMaterial(iVar4);
							pMapManager->DrawSprite(pDlistMaterial, &local_b0, 0x80808080);
						}
					}
				}

				fVar10 = fVar9;
				if (((((this->field_0x80).x + 70.0f <= local_10.x) && (local_10.x <= (this->field_0x90).x - 70.0f)) && ((this->field_0x80).y - (static_cast<float>(gVideoConfig.screenHeight) * 25.0f) / 512.0f <= local_10.y))
					&& (local_10.y <= (this->field_0x90).y - (static_cast<float>(gVideoConfig.screenHeight) * 70.0f) / 512.0f)) {
					edF32Vector4SubHard(&eStack32, &local_10, &this->field_0xa0);
					fVar10 = edF32Vector4GetDistHard(&eStack32);
					pNVar6 = this_00;
					if (fVar9 <= fVar10) {
						fVar10 = fVar9;
						pNVar6 = pNVar5;
					}
				}
			}

			iVar3 = iVar3 + 1;
			this_00 = this_00 + 1;
			fVar9 = fVar10;
			pNVar5 = pNVar6;
		} while (iVar3 < iVar1);
	}

	return pNVar6;
}

edF32VECTOR4 edF32VECTOR4_004269b0 = { 0.0f, 0.0f, 0.0f, 1.0f };
edF32VECTOR4 edF32VECTOR4_004269c0 = { 0.0f, 0.0f, 0.0f, 0.0f };
edF32VECTOR4 edF32VECTOR4_004269d0 = { 0.0f, 0.0f, 1.0f, 0.0f };
edF32VECTOR4 edF32VECTOR4_004269e0 = { 0.0f, 0.0f, 1.0f, 1.0f };

void CLevelMap::DrawMarkerLabels(MapPosition* pMapPosition, int param_3)
{
	int iVar1;
	edCTextStyle* pTextStyle;
	char* pText;
	edF32VECTOR4* v1;
	int iVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 local_1630;
	S_2DRECT local_1620;
	edF32VECTOR4 local_d0;
	CLevelScheduler* pLevelScheduler;
	CMapManager* pMapManager;
	edCTextStyle textStyle;

	pMapManager = CScene::ptable.g_MapManager_0045168c;
	pLevelScheduler = CLevelScheduler::gThis;
	textStyle.Reset();
	textStyle.SetShadow(0x100);
	textStyle.alpha = 0xff;
	textStyle.rgbaColour = 0xd0e0ffff;
	textStyle.SetHorizontalAlignment(0);
	textStyle.SetVerticalAlignment(8);
	textStyle.SetFont(BootDataFont, false);
	textStyle.SetScale(0.69999999f, 0.69999999f);
	pTextStyle = edTextStyleSetCurrent(&textStyle);
	iVar2 = 0;
	if (0 < param_3) {
		do {
			v1 = &local_d0;
			local_d0.xyz = pMapPosition->position;
			local_d0.w = 1.0f;
			if (this->field_0xd0 == 0) {
				edF32Matrix4MulF32Vector4Hard(v1, &this->field_0x1b0, v1);
			}
			else {
				edF32Matrix4MulF32Vector4Hard(&local_1630, &this->field_0x130, v1);
				v1 = &local_1630;
				local_1630.x = (local_1630.x - 0.5f) / (local_1630.z - -1.0f) + 0.5f;
				local_1630.y = (local_1630.y - 0.5f) / (local_1630.z - -1.0f) + 0.5f;
				edF32Matrix4MulF32Vector4Hard(&local_d0, &this->scrollMatrix, v1);
			}

			local_d0.x = local_d0.x + 20.0f;
			iVar1 = pMapPosition->field_0xc;
			if (pLevelScheduler->aLevelInfo[iVar1].titleMsgHash != 0) {
				edCTextFormat textFormat;
				pText = gMessageManager.get_message(pLevelScheduler->aLevelInfo[iVar1].titleMsgHash);
				textFormat.FormatString(pText, v1);
				fVar3 = (textFormat.offsetX_0x0 + textFormat.field_0x8) * 0.7f;
				fVar5 = textFormat.offsetY_0x4 * 0.7f;
				fVar4 = (textFormat.offsetY_0x4 + textFormat.field_0xc) * 0.7f;
				if ((((0.0f <= local_d0.x + fVar3) && (fVar6 = local_d0.x + textFormat.offsetX_0x0 * 0.7f, fVar6 - 16.0f <= static_cast<float>(gVideoConfig.screenWidth))) && (0.0f <= local_d0.y + fVar4)) &&
					(local_d0.y + fVar5 <= static_cast<float>(gVideoConfig.screenHeight))) {
					local_1620.aVertices[0].x1 = fVar6 - 12.0f;
					local_1620.aVertices[0].s1 = edF32VECTOR4_004269b0.z;
					local_1620.aVertices[0].t1 = edF32VECTOR4_004269b0.w;
					local_1620.aVertices[1].s1 = edF32VECTOR4_004269c0.z;
					local_1620.aVertices[1].t1 = edF32VECTOR4_004269c0.w;
					local_1620.aVertices[2].s1 = edF32VECTOR4_004269d0.z;
					local_1620.aVertices[2].t1 = edF32VECTOR4_004269d0.w;
					local_1620.aVertices[3].s1 = edF32VECTOR4_004269e0.z;
					local_1620.aVertices[3].t1 = edF32VECTOR4_004269e0.w;
					local_1620.aVertices[0].y1 = (local_d0.y + fVar5) - 1.0f;
					local_1620.aVertices[1].x1 = local_d0.x + fVar3 + 12.0f;
					local_1620.aVertices[1].y1 = (local_d0.y + fVar5) - 1.0f;
					local_1620.aVertices[2].x1 = local_d0.x + fVar3 + 12.0f;
					local_1620.aVertices[2].y1 = local_d0.y + fVar4 + 1.0f;
					local_1620.aVertices[3].x1 = (local_d0.x + textFormat.offsetX_0x0 * 0.7f) - 12.0f;
					local_1620.aVertices[3].y1 = local_d0.y + fVar4 + 1.0f;
					pMapManager->DrawClippedSprite(&MenuBitmaps[8].materialInfo, &local_1620, 0x80808080);
					textFormat.Display(local_d0.x, local_d0.y);
				}
			}

			iVar2 = iVar2 + 1;
			pMapPosition = pMapPosition + 1;
		} while (iVar2 < param_3);
	}

	edTextStyleSetCurrent(pTextStyle);

	return;
}

void CLevelMap::DrawObjectiveTargetMarkers(ObjectiveEntry* pObjectiveEntry, int param_3, MapDataSizes* pMapSize, MapPosition* pMapPosition)
{
	int iVar1;
	int iVar2;
	ObjectiveEntry* pLVar3;
	MapPosition* pMVar4;
	NativShopLevelSubObj* pNVar5;
	int iVar6;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CLevelScheduler* pLevelScheduler;

	pLevelScheduler = CLevelScheduler::gThis;
	switch (param_3) {
	case 1:
		iVar6 = 0;
		do {
			if (((iVar6 != this->field_0x0) && (iVar1 = pLevelScheduler->IsRuneCollectedInLevel(iVar6), iVar1 == 0)) &&
				(pMVar4 = FindMarkerForLevel(iVar6, pMapSize, pMapPosition, 0), pMVar4 != (MapPosition*)0x0)) {
				local_10.xyz = pMVar4->position;
				local_10.w = 1.0f;
				DrawPulsingMarkerHighlight(25.0f, 25.0f, &local_10, 0x7f613021);
				DrawOffscreenMarkerArrow(25.0f, 25.0f, &local_10, 0x7f613021);
			}

			iVar6 = iVar6 + 1;
		} while (iVar6 < 0x10);
		break;
	case 2:
		iVar6 = 0;
		do {
			iVar1 = pLevelScheduler->IsMapCollectedInLevel(iVar6);
			if ((iVar1 != 0x62) && (iVar2 = CLevelScheduler::ScenVar_Get(iVar1), iVar2 != 2)) {
				if (iVar6 == this->field_0x0) {
					pLVar3 = pLevelScheduler->FUN_002d9f30(iVar1);
					if (pLVar3 != (ObjectiveEntry*)0x0) {
						DrawPulsingMarkerHighlight(25.0f, 25.0f, &pLVar3->field_0x10, 0x7f112e7c);
						DrawOffscreenMarkerArrow(25.0f, 25.0f, &pLVar3->field_0x10, 0x7f112e7c);
					}
				}
				else {
					pMVar4 = FindMarkerForLevel(iVar6, pMapSize, pMapPosition, 0);
					if (pMVar4 != (MapPosition*)0x0) {
						local_20.xyz = pMVar4->position;
						local_20.w = 1.0f;

						DrawPulsingMarkerHighlight(25.0f, 25.0f, &local_20, 0x7f112e7c);
						DrawOffscreenMarkerArrow(25.0f, 25.0f, &local_20, 0x7f112e7c);
					}
				}
			}

			iVar6 = iVar6 + 1;
		} while (iVar6 < 0x10);
	case 3:
	case 4:
		pNVar5 = pLevelScheduler->FUN_002d9510(pObjectiveEntry->field_0x24);
		if ((pNVar5 != (NativShopLevelSubObj*)0x0) && (pNVar5->currentLevelId == this->field_0x0)) {
			DrawPulsingMarkerHighlight(25.0f, 25.0f, &pNVar5->currentLocation, 0x7f2f777f);
			DrawOffscreenMarkerArrow(25.0f, 25.0f, &pNVar5->currentLocation, 0x7f2f777f);
		}
	}
	if ((pObjectiveEntry != (ObjectiveEntry*)0x0) && (this->field_0x0 == pObjectiveEntry->levelId)) {
		if (pObjectiveEntry->field_0x40 == 0) {
			DrawPulsingMarkerHighlight(25.0f, 25.0f, &pObjectiveEntry->field_0x10, 0x7f7f7f7f);
			DrawOffscreenMarkerArrow(25.0f, 25.0f, &pObjectiveEntry->field_0x10, 0x7f7f7f7f);
		}
		else {
			DrawPulsingMarkerHighlight(25.0f, 25.0f, &pObjectiveEntry->field_0x30, 0x7f7f7f7f);
			DrawOffscreenMarkerArrow(25.0f, 25.0f, &pObjectiveEntry->field_0x30, 0x7f7f7f7f);
		}
	}

	return;
}

edF32VECTOR4 edF32VECTOR4_004269f0 = { 0.0f, 0.0f, 0.0f, 1.0f };
edF32VECTOR4 edF32VECTOR4_00426a00 = { 0.0f, 0.0f, 0.0f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00426a10 = { 0.0f, 0.0f, 1.0f, 0.0f };
edF32VECTOR4 edF32VECTOR4_00426a20 = { 0.0f, 0.0f, 1.0f, 1.0f };

void CLevelMap::DrawSelectedShop(NativShopLevelSubObj* pShopSubObj, ObjectiveEntry* pObjective, int param_4)
{
	int iVar1;
	bool bVar2;
	bool bVar3;
	Episode* pEVar4;
	edCTextStyle* pNewFont;
	char* pcVar5;
	char* format;
	char* uVar6;
	long lVar7;
	int iVar8;
	NativShopLevelSubObjSubObj* pNVar9;
	CSprite* pCVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 local_1a40;
	S_2DRECT local_1a30;
	char local_4e0[1024];
	edCTextStyle eStack224;
	edF32VECTOR4 local_20;
	undefined4 local_4;
	CLevelScheduler* pLevelScheduler;
	CMapManager* pMapManager;

	pMapManager = CScene::ptable.g_MapManager_0045168c;
	pLevelScheduler = CLevelScheduler::gThis;
	bVar3 = pShopSubObj->FUN_002d8cb0();
	pEVar4 = pLevelScheduler->GetEpisode(pShopSubObj->episodeThresholdLink);
	if (this->field_0xd0 == 0) {
		edF32Matrix4MulF32Vector4Hard(&local_20, &this->field_0x1b0, &pShopSubObj->currentLocation);
	}
	else {
		edF32Matrix4MulF32Vector4Hard(&local_1a40, &this->field_0x130, &pShopSubObj->currentLocation);
		local_1a40.x = (local_1a40.x - 0.5f) / (local_1a40.z - -1.0f) + 0.5f;
		local_1a40.y = (local_1a40.y - 0.5f) / (local_1a40.z - -1.0f) + 0.5f;
		edF32Matrix4MulF32Vector4Hard(&local_20, &this->scrollMatrix, &local_1a40);
	}

	bVar2 = false;
	if (((param_4 == 4) || (param_4 == 3)) || (param_4 == 2)) {
		bVar2 = true;
	}

	eStack224.Reset();
	eStack224.SetShadow(0x100);
	eStack224.rgbaColour = 0xd0e0ffff;
	eStack224.alpha = 0xff;
	eStack224.SetHorizontalAlignment(0);
	eStack224.SetVerticalAlignment(0);
	uVar6 = (char*)0x0;
	eStack224.SetFont(BootDataFont, false);
	eStack224.SetScale(0.80000001f, 0.80000001f);
	pNewFont = edTextStyleSetCurrent(&eStack224);

	pcVar5 = uVar6;
	local_4e0[0] = '\0';
	if ((bVar3 == false) || (iVar8 = 0, pNVar9 = pShopSubObj->aSubObjs, pShopSubObj->field_0x8 == 0)) {
		if (bVar3 == false) {
			iVar8 = CLevelScheduler::ScenVar_Get(0);
			lVar7 = static_cast<long>(pEVar4->minWolfen - iVar8);
			if (lVar7 == 1) {
				format = gMessageManager.get_message(0x474b4a0c0b031f0c);
				sprintf(local_4e0, format);
			}
			else {
				pcVar5 = gMessageManager.get_message(0x8050f0c0b031f53);
				sprintf(local_4e0, pcVar5);
				pcVar5 = reinterpret_cast<char*>(lVar7);
			}
		}
		else {
			pcVar5 = gMessageManager.get_message(0x1e091f0f4e45575f);
			sprintf(local_4e0, "%%[BLINK]k%s%%[RESET]K", pcVar5);
		}
	}
	else {
		do {
			if (pNVar9->purchaseId != 0x20) {
				if (local_4e0[0] != '\0') {
					strcat(local_4e0, "\n");
				}
				pcVar5 = gMessageManager.get_message(pNVar9->field_0x8);
				strcat(local_4e0, pcVar5);
			}

			pcVar5 = uVar6;
			iVar8 = iVar8 + 1;
			pNVar9 = pNVar9 + 1;
		} while (iVar8 < 5);
	}

	if (local_4e0[0] != '\0') {
		edCTextFormat local_19f0;
		local_19f0.FormatString(local_4e0, pcVar5);
		fVar12 = local_19f0.offsetX_0x0 * 0.8f;
		fVar11 = (local_19f0.offsetX_0x0 + local_19f0.field_0x8) * 0.8f;
		if ((bVar3 != false) && (pShopSubObj->field_0x8 != 0)) {
			fVar12 = fVar12 - 23.0f;
			fVar11 = fVar11 + 23.0f;
		}
		local_1a30.aVertices[0].s1 = edF32VECTOR4_004269f0.z;
		local_1a30.aVertices[0].t1 = edF32VECTOR4_004269f0.w;
		local_20.x = local_20.x - (fVar11 - fVar12) * 0.5f;
		local_1a30.aVertices[1].s1 = edF32VECTOR4_00426a00.z;
		local_1a30.aVertices[1].t1 = edF32VECTOR4_00426a00.w;
		local_1a30.aVertices[2].s1 = edF32VECTOR4_00426a10.z;
		local_1a30.aVertices[2].t1 = edF32VECTOR4_00426a10.w;
		local_1a30.aVertices[3].s1 = edF32VECTOR4_00426a20.z;
		local_1a30.aVertices[3].t1 = edF32VECTOR4_00426a20.w;
		local_20.y = local_20.y + 32.0f;
		fVar11 = local_20.x + (fVar11 - fVar12);
		local_1a30.aVertices[0].x1 = local_20.x - 16.0f;
		local_1a30.aVertices[1].x1 = fVar11 + 20.0f;
		local_1a30.aVertices[2].y1 = local_20.y + ((local_19f0.offsetY_0x4 + local_19f0.field_0xc) * 0.8f - local_19f0.offsetY_0x4 * 0.8f) + 2.0f;
		local_1a30.aVertices[0].y1 = local_20.y - 2.0f;
		local_1a30.aVertices[1].y1 = local_1a30.aVertices[0].y1;
		local_1a30.aVertices[2].x1 = local_1a30.aVertices[1].x1;
		local_1a30.aVertices[3].x1 = local_1a30.aVertices[0].x1;
		local_1a30.aVertices[3].y1 = local_1a30.aVertices[2].y1;

		pMapManager->DrawClippedSprite(&MenuBitmaps[8].materialInfo, &local_1a30, 0x80808080);
		if ((bVar3 == false) || (pShopSubObj->field_0x8 == 0)) {
			local_19f0.Display(local_20.x, local_20.y);
		}
		else {
			iVar8 = 0;
			eStack224.field_0x8c = (float)(uint)BootDataFont->field_0x16 * 0.8f + eStack224.field_0x8c;
			fVar12 = local_20.y;
			pNVar9 = pShopSubObj->aSubObjs;
			do {
				iVar1 = pNVar9->purchaseId;
				if (iVar1 != 0x20) {
					if ((bVar2) && (iVar1 == pObjective->field_0x24)) {
						local_4 = 0x7f2f777f;
						eStack224.rgbaColour = 0xfeee5efe;
					}
					else {
						eStack224.rgbaColour = 0xd0e0ffff;
					}

					pcVar5 = gMessageManager.get_message(pNVar9->field_0x8);
					edTextDraw(local_20.x, fVar12, pcVar5);
					fVar12 = fVar12 + eStack224.field_0x8c;
				}

				iVar8 = iVar8 + 1;
				pNVar9 = pNVar9 + 1;
			} while (iVar8 < 5);

			eStack224.SetHorizontalAlignment(1);
			iVar8 = 0;
			eStack224.rgbaColour = 0xd0e0ffff;
			pCVar10 = &(CScene::ptable.g_FrontendManager_00451680)->pMoney->sprite;
			do {
				if (pShopSubObj->aSubObjs[iVar8].purchaseId != 0x20) {
					edTextDraw(fVar11 - 16.0f, local_20.y, "%d", pShopSubObj->aSubObjs[iVar8].moneyCost);
					pCVar10->Draw(0.5f, fVar11 - 8.0f, local_20.y + 10.0f, 0x12);
					local_20.y = local_20.y + eStack224.field_0x8c;
				}

				iVar8 = iVar8 + 1;
			} while (iVar8 < 5);
		}

	}

	edTextStyleSetCurrent(pNewFont);

	return;
}

void CWorldMap::Level_PostInit()
{
	this->currentLevelId = CLevelScheduler::gThis->currentLevelID;

	return;
}

void CWorldMap::Draw()
{
	IMPLEMENTATION_GUARD();
}
