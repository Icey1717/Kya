#include "MapManager.h"
#include "MemoryStream.h"
#include "Kya.h"
#include "MathOps.h"
#include "ActorHero.h"
#include "ActorTeleporter.h"
#include "ActorManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "FrontendDisp.h"
#include "LevelScheduler.h"
#include "TimeController.h"

void CMapManager::OnLoadLevelBnk_003f9a60(ByteCode* pMemoryStream)
{
	int* piVar1;
	int iVar2;
	uint uVar3;
	float fVar4;

	iVar2 = pMemoryStream->GetS32();
	this->field_0x4 = iVar2;
	uVar3 = pMemoryStream->GetU32();
	this->field_0x8 = uVar3;
	fVar4 = pMemoryStream->GetF32();
	this->field_0xc = (fVar4 * 3.141593f) / 180.0f;
	piVar1 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar1 * 4;
	}

	this->field_0x10 = piVar1;
	iVar2 = pMemoryStream->GetS32();
	this->field_0x14 = iVar2;
	iVar2 = pMemoryStream->GetS32();
	this->field_0x18.index = iVar2;
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
	edF32VECTOR4* field_0x0;
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
			MapStruct_8_ARRAY_0049ca70[INT_004497ec].field_0x0 = &LOAD_POINTER_CAST(ed_zone_3d*, pMsg->pEventCollider->field_0x1c)->boundSphere;
			MapStruct_8_ARRAY_0049ca70[iVar4].field_0x4 = iVar2;
			INT_004497ec = INT_004497ec + 1;
		}
	}

	return;
}

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
		fVar9 = (pCVar4->transformationMatrix).db;
		fVar10 = (pCVar4->transformationMatrix).dc;
		this->field_0x2c = (pCVar4->transformationMatrix).da;
		this->field_0x30 = fVar9;
		this->field_0x34 = fVar10;
		fVar9 = (pCVar4->lookAt).y;
		fVar10 = (pCVar4->lookAt).z;
		this->field_0x38 = (pCVar4->lookAt).x;
		this->field_0x3c = fVar9;
		this->field_0x40 = fVar10;
		fVar9 = (pCVar4->transformationMatrix).bb;
		fVar10 = (pCVar4->transformationMatrix).bc;
		this->field_0x44 = (pCVar4->transformationMatrix).ba;
		this->field_0x48 = fVar9;
		this->field_0x4c = fVar10;
		this->field_0x50 = pCVar4->fov;
		this->field_0x54 = this->field_0x8;
	}

	if (CActorHero::_gThis != (CActorHero*)0x0) {
		local_4 = &uStack16;
		//uStack16.field_0x8 = DAT_004279e8;
		//uStack16.wide = DAT_004279e0 & 0xffffffff | static_cast<ulong>(CActorHero::_gThis->actorManagerIndex << 0x20);
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
					MapStruct_8_ARRAY_0049ca70[INT_004497ec].field_0x0 = peVar7;
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
						MapStruct_8_ARRAY_0049ca70[INT_004497ec].field_0x0 = &pTeleporter->field_0x270;
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

void CMapManager::Func_003f8d80()
{
	int iVar1;
	int iVar2;
	NativShopLevelSubObj* pNVar4;
	Episode* pEVar5;
	int iVar6;
	NativShopLevelSubObjSubObj* pNVar7;
	ulong uVar8;
	LoadLoopObject_50* pLVar9;
	LoadLoopObject_50* pLVar10;
	LoadLoopObject_50* pLVar11;
	CLevelScheduler* pLevelScheduler;

	pLevelScheduler = CLevelScheduler::gThis;
	iVar2 = 6;
	iVar6 = this->field_0x338;
	pLVar11 = CLevelScheduler::gThis->field_0x4220;
	pLVar10 = pLVar11 + CLevelScheduler::gThis->objCount_0x4218;
	pLVar9 = (LoadLoopObject_50*)0x0;
	for (; pLVar11 < pLVar10; pLVar11 = pLVar11 + 1) {
		if (((pLVar11->field_0x4 == 0) && (pLVar11->messageKey != 0)) && (iVar1 = CLevelScheduler::ScenVar_Get(pLVar11->field_0x0), iVar1 == 1)) {
			iVar2 = MapFunc_002d8dc0(pLVar11);
			pLVar9 = pLVar11;
		}
	}

	if (pLVar9 == (LoadLoopObject_50*)0x0) {
		iVar1 = 0x62;
	}
	else {
		iVar1 = pLVar9->field_0x0;
	}

	if (iVar1 == iVar6) {
		if ((iVar1 != 0x62) && ((iVar2 != this->field_0x33c || (this->field_0x340 != pLVar9->field_0x40)))) {
			this->field_0x334 = 1;
			this->field_0x330 = 0;
			this->field_0x37c = GetTimer()->totalTime;
			this->field_0x340 = pLVar9->field_0x40;
			this->field_0x33c = iVar2;
		}
	}
	else {
		this->field_0x358 = this->field_0x338;
		this->field_0x35c = this->field_0x33c;
		this->field_0x360 = this->field_0x340;
		this->field_0x368 = this->field_0x348;
		this->field_0x36c = this->field_0x34c;
		this->field_0x370 = this->field_0x350;
		this->field_0x334 = 1;
		this->field_0x330 = 0;
		this->field_0x37c = GetTimer()->totalTime;
		this->field_0x338 = iVar1;
		if (this->field_0x338 == 0x62) {
			this->field_0x340 = 0;
			this->field_0x33c = 6;
			if (10.0f < GetTimer()->scaledTotalTime) {
				IMPLEMENTATION_GUARD(
				pLevelScheduler->SaveGame_InitiateAutoSave(1.0f, 0);)
			}
		}
		else {
			this->field_0x340 = pLVar9->field_0x40;
			this->field_0x33c = iVar2;
		}
	}

	iVar6 = 0x10;
	this->field_0x348 = 0x10;
	this->field_0x34c = 0;
	this->field_0x350 = 0;
	if (pLVar9 != (LoadLoopObject_50*)0x0) {
		iVar2 = 0;
		if (this->field_0x33c == 2) {
			iVar1 = 0;

			do {
				uVar8 = pLevelScheduler->FUN_002ddfb0(iVar1);
				if (uVar8 == 0) {
					iVar2 = iVar2 + 1;
					iVar6 = iVar1;
				}

				iVar1 = iVar1 + 1;
			} while (iVar1 < 0x10);

			if (iVar2 == 1) {
				this->field_0x348 = iVar6;
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

void CMapManager::Game_Init()
{
	this->field_0x58 = 0;
	this->field_0x5c = -1.0f;
	this->field_0x60 = 0;
	this->field_0x64 = 0;
	this->field_0x68 = 0;
	this->field_0x6c = 0;
	this->field_0x70 = 0;
	this->field_0x74 = 1.0f;
	this->field_0x78 = 0;
	this->field_0x7c = 1.0f;
	this->field_0x80 = 0;
	this->field_0x310 = 0;
	this->field_0x314 = 0;
	this->field_0x318 = 0;
	this->field_0x31c = 1.0f;
	this->screenWidth_0x320 = static_cast<float>(gVideoConfig.screenWidth);
	this->screenHeight_0x324 = static_cast<float>(gVideoConfig.screenHeight);
	this->field_0x328 = 0;
	this->field_0x32c = 1.0f;
	this->field_0x24 = 0;
	this->field_0x28 = 0;
	this->field_0x2c = this->field_0x58;
	this->field_0x30 = this->field_0x5c;
	this->field_0x34 = this->field_0x60;
	this->field_0x38 = this->field_0x64;
	this->field_0x3c = this->field_0x68;
	this->field_0x40 = this->field_0x6c;
	this->field_0x44 = this->field_0x70;
	this->field_0x48 = this->field_0x74;
	this->field_0x4c = this->field_0x78;
	this->field_0x50 = this->field_0x7c;
	this->field_0x54 = this->field_0x80;
	INT_004497ec = 0;
	this->field_0x300 = 0;
	this->field_0x334 = 0;
	this->field_0x330 = 0;
	this->field_0x378 = 0;
	this->field_0x10 = 0;
	this->field_0x14 = 0x62;
	this->field_0x18.index = -1;
	this->field_0x394 = 0;
	this->field_0x338 = 0x62;
	this->field_0x33c = 6;
	this->field_0x340 = 0;
	this->field_0x348 = 0x10;
	this->field_0x34c = 0;
	this->field_0x350 = 0;
	this->field_0x358 = this->field_0x338;
	this->field_0x35c = this->field_0x33c;
	this->field_0x360 = this->field_0x340;
	this->field_0x368 = this->field_0x348;
	this->field_0x36c = this->field_0x34c;
	this->field_0x370 = this->field_0x350;
	this->field_0x334 = 0;
	this->field_0x330 = 0;
	this->field_0x37c = 0;
	this->field_0x380 = 1;
	this->field_0x384 = 0.0f;
	this->pLevelMap = new CLevelMap;
	this->pWorldMap = new CWorldMap;
	this->field_0x390 = 0;
	this->field_0x38c = 0;
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

	this->field_0x38c = static_cast<uint>(iVar2 == 2);
	this->field_0x390 = 0;

	return;
}

void CMapManager::Level_Term()
{
	IMPLEMENTATION_GUARD();
}

void CMapManager::Level_ClearAll()
{
	IMPLEMENTATION_GUARD();
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

	if (this->field_0x38c == 0) {
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

		this->field_0x38c = static_cast<uint>(iVar2 == 2);
		if (this->field_0x38c != 0) {
			this->field_0x390 = 1;
		}
	}

	if ((GameFlags & 0x10) != 0) {
		IMPLEMENTATION_GUARD(
		this->field_0x334 = 0;
		this->field_0x330 = 0;
		fVar9 = GetTimer()->totalTime;
		fVar8 = 1.0f;
		if ((float)this->field_0x37c != 0.0f) {
			fVar8 = GetTimer()->totalTime - (float)this->field_0x37c;
			if (fVar8 <= 2.0f) {
				fVar8 = (fVar8 / 2.0f) * 0.5f;
			}
			else {
				fVar8 = ((fVar8 - 2.0f) * 0.5f) / 3.5f + 0.5f;
			}
			uVar4 = FUN_003f6910(&this->field_0x338);
			uVar5 = FUN_003f6910(&this->field_0x358);
			uVar6 = FUN_003f6910(&this->field_0x338);
			uVar5 = uVar5 ^ uVar6;
			if (((uVar5 & 0x40) == 0) && (this->field_0x358 == this->field_0x338)) {
				if ((this->field_0x360 != this->field_0x340) || ((uVar5 & 1) != 0)) {
					uVar5 = uVar5 | 0x3f;
				}
			}
			else {
				uVar5 = uVar5 | 0x7f;
			}
			if ((1.0f < fVar8) || ((0.6f <= fVar8 && ((uVar5 & uVar4) == 0)))) {
				this->field_0x37c = 0;
				fVar8 = 1.0f;
				this->field_0x358 = this->field_0x338;
				this->field_0x35c = this->field_0x33c;
				this->field_0x360 = this->field_0x340;
				this->field_0x368 = this->field_0x348;
				this->field_0x36c = this->field_0x34c;
				this->field_0x370 = this->field_0x350;
				if (this->field_0x380 == 0) {
					this->field_0x384 = fVar9;
				}
			}
		}

		fVar7 = FUN_003f69b0(&this->field_0x338);
		this->field_0x378 = fVar7;
		if (fVar8 < 1.0f) {
			fVar9 = FUN_003f69b0(&this->field_0x358);
			if (fVar8 < 0.4f) {
				this->field_0x378 = fVar9;
			}
			else {
				fVar7 = (float)this->field_0x378;
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
			if (((1.0f <= (fVar9 - this->field_0x384) / 0.5f) && ((float)this->field_0x37c == 0.0f)) && ((gPlayerInput.pressedBitfield & PAD_BITMASK_17) != 0)) {
				(CScene::ptable.g_FrontendManager_00451680)->pFrontendSamplePlayer->PlaySample(1.0f, 2, 0);
				this->field_0x380 = this->field_0x380 ^ 1;
				this->field_0x384 = fVar9;
			}
		}

		if (this->field_0x24 == 0) {
			if (this->field_0x28 != 0) {
				this->pWorldMap->FUN_003d4220();
			}
		}
		else {
			this->pLevelMap->Update();
		})
	}

	return;
}

void CMapManager::Level_ManagePaused()
{
	IMPLEMENTATION_GUARD();
}

void CMapManager::Level_Draw()
{
	IMPLEMENTATION_GUARD();
}

void CMapManager::Level_SaveContext()
{
	IMPLEMENTATION_GUARD();
}

void CMapManager::Level_LoadContext()
{
	IMPLEMENTATION_GUARD();
}

void CLevelMap::Level_PostInit()
{
	this->pMaterials = (edDList_material*)0x0;
	this->field_0x78 = 0;
	this->field_0x7c = 0;
	this->field_0x0 = 0x10;
	this->field_0x4 = 0x10;
	this->field_0xd8 = 0.0f;
	this->field_0xe4 = 0;
	this->field_0xdc = 0;
	this->field_0xe0 = 0;

	return;
}

void CWorldMap::Level_PostInit()
{
	this->currentLevelId = CLevelScheduler::gThis->currentLevelID;

	return;
}
