#include "ActorTeleporter.h"
#include "MemoryStream.h"
#include "FileManager3D.h"
#include "LevelScheduleManager.h"
#include "CollisionManager.h"
#include "PoolAllocators.h"
#include "EventManager.h"
#include "MathOps.h"
#include "DlistManager.h"
#include "ActorHero.h"
#include "ed3D/ed3DG3D.h"

void CActorTeleporter::Create(ByteCode* pByteCode)
{
	int* piVar1;
	S_NTF_TARGET_STREAM_REF* pSVar2;
	S_STREAM_EVENT_CAMERA* pSVar3;
	ACTOR_WAYPOINT_LIST* pAVar4;
	CCollision* pCVar5;
	C3DFileManager* pFileManager;
	int iVar6;
	uint uVar7;
	ed_zone_3d* peVar8;
	int iVar9;
	ParticleInfo* pPVar10;
	CActor* pCVar11;
	edDList_material* peVar12;
	ulong uVar13;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;

	CActor::Create(pByteCode);

	this->field_0x168 = pByteCode->GetS32();
	this->hash = pByteCode->GetU64();
	this->field_0x16c = pByteCode->GetU32();

	S_DESTINATION_LIST::Create(&this->pDestinationList, pByteCode);

	this->field_0x170.Create(pByteCode);
	this->field_0x178.Create(pByteCode);
	this->field_0x180.Create(pByteCode);

	this->field_0x188.index = pByteCode->GetS32();
	this->field_0x18c.index = pByteCode->GetS32();

	iVar6 = pByteCode->GetS32();
	this->cinematicId = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(iVar6);
	this->field_0x190 = pFileManager->GetG2DInfo(iVar6);

	iVar6 = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(iVar6);
	this->field_0x194 = pFileManager->GetG2DInfo(iVar6);

	this->field_0x1d4.index = pByteCode->GetS32();
	this->field_0x1d8.index = pByteCode->GetS32();

	iVar6 = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(iVar6);
	this->field_0x1dc = pFileManager->GetG2DInfo(iVar6);

	this->field_0x1e0.index = pByteCode->GetS32();

	ACTOR_WAYPOINT_LIST::Create(&this->pActorWaypointList, pByteCode);

	this->condOpArray.Create(pByteCode);

	if (this->cinematicId != -1) {
		S_DESTINATION_LIST* pSVar1 = this->pDestinationList;
		iVar6 = 0;
		if (pSVar1 != (S_DESTINATION_LIST*)0x0) {
			iVar6 = pSVar1->nbEntries;
		}

		if ((iVar6 != 0) && (this->field_0x190 == (ParticleInfo*)0x0)) {
			if (pSVar1 == (S_DESTINATION_LIST*)0x0) {
				iVar6 = 0;
			}
			else {
				iVar6 = pSVar1->nbEntries;
			}

			this->aMaterials = NewPool_edDLIST_MATERIAL(iVar6);
			goto LAB_002ef408;
		}
	}
	this->aMaterials = (edDList_material*)0x0;

LAB_002ef408:
	this->nbMaterials = 0;
	pCVar5 = this->pCollisionData;
	if (pCVar5 != (CCollision*)0x0) {
		pCVar5->flags_0x0 = pCVar5->flags_0x0 & 0xffffffc4;
		pCVar5->flags_0x0 = pCVar5->flags_0x0 | 0x40;
	}

	this->field_0x2ac = -1;
	this->field_0x2b0 = -1;

	return;
}

void CActorTeleporter::Init()
{
	ed_zone_3d* pZone;
	CActor* this_00;
	undefined8* puVar2;
	CEventManager* pCVar3;
	int iVar4;
	edF32MATRIX4* peVar5;
	ed_hash_code* pHashCode;
	CCinematic* pCVar6;
	ulong uVar7;
	S_NTF_TARGET_STREAM_REF* pSVar8;
	int iVar9;
	ACTOR_WAYPOINT* pRef;
	float fVar10;
	float fVar11;
	float fVar12;
	edF32MATRIX4 eStack80;
	e_ed_event_prim3d_type eStack12;
	e_ed_event_prim3d_type eStack8;
	e_ed_event_prim3d_type eStack4;

	if ((this->field_0x16c & 1) == 0) {
		this->bOpen = 0;
	}
	else {
		this->bOpen = 1;
	}

	this->activeButtonIndex = 0;
	this->field_0x298 = 0;
	this->disabledDestinationMask = 0;
	this->field_0x2a4 = 0;

	for (iVar4 = this->pDestinationList->nbEntries; iVar4 != 0; iVar4 = iVar4 + -1) {
	}

	this->field_0x170.Init();
	this->field_0x178.Init();
	this->field_0x180.Init();

	this->field_0x188.Init();
	this->field_0x18c.Init();
	this->field_0x1d4.Init();
	this->field_0x1d8.Init();
	this->field_0x1e0.Init();

	pRef = this->pActorWaypointList->aEntries;
	pCVar3 = CScene::ptable.g_EventManager_006f5080;
	for (iVar4 = this->pActorWaypointList->nbEntries; CScene::ptable.g_EventManager_006f5080 = pCVar3, iVar4 != 0; iVar4 = iVar4 + -1) {
		pRef->actorRef.Init();
		pRef->wayPointRef.Init();
		pRef = pRef + 1;
	}

	pCVar3 = CScene::ptable.g_EventManager_006f5080;

	pZone = (this->field_0x188).Get();
	if (pZone == (ed_zone_3d*)0x0) {
		this->field_0x270 = this->currentLocation;
	}
	else {
		peVar5 = edEventGetChunkZonePrimitive(pCVar3->activeChunkId, pZone, 0, &eStack4);
		edF32Matrix4GetInverseGaussSoft(&eStack80, peVar5);
		this->field_0x270 = eStack80.rowT;
		(this->field_0x270).w = 1.0f;
	}

	pZone = (this->field_0x1d4).Get();
	if (pZone != (ed_zone_3d*)0x0) {
		peVar5 = edEventGetChunkZonePrimitive(pCVar3->activeChunkId, pZone, 0, &eStack8);
		edF32Matrix4GetInverseGaussSoft(&this->field_0x1f0, peVar5);
	}
	pZone = (this->field_0x1e0).Get();
	if (pZone != (ed_zone_3d*)0x0) {
		peVar5 = edEventGetChunkZonePrimitive(pCVar3->activeChunkId, pZone, 0, &eStack12);
		edF32Matrix4GetInverseGaussSoft(&this->field_0x230, peVar5);
	}

	CActor::Init();

	DetectDisabledDestinations(1);

	this->pSnapshotHashCode = (ed_hash_code*)0x0;

	this_00 = (this->field_0x1d8).Get();
	if ((this_00 != (CActor*)0x0) && ((this->field_0x1d4).Get() == (ed_zone_3d*)0x0)) {
		if (this_00->pMBNK == (void*)0x0) {
			this_00->SV_InstanciateMaterialBank();
		}
		pHashCode = ed3DHierarchyGetMaterialBank(&this_00->p3DHierNode->base);
		int nbMat = ed3DG2DGetG2DNbMaterials(pHashCode);
		uVar7 = ed3DComputeHashCode("Snapshot");
		int curMatIndex = 0;
		if (0 < nbMat) {
			do {
				if (uVar7 == pHashCode->hash.number) {
					this->pSnapshotHashCode = pHashCode;
					this->snapshotHashCode = *this->pSnapshotHashCode;
				}

				curMatIndex = curMatIndex + 1;
				pHashCode = pHashCode + 1;
			} while (curMatIndex < nbMat);
		}
	}

	pCVar6 = g_CinematicManager_0048efc->GetCinematic(this->cinematicId);
	if (pCVar6 != (CCinematic*)0x0) {
		pCVar6->pActor = (CActor*)this;
	}

	return;
}

void CActorTeleporter::Term()
{
	int curMatIndex;

	if (this->nbMaterials != 0) {
		curMatIndex = 0;
		if (0 < this->nbMaterials) {
			do {
				edDListTermMaterial(this->aMaterials + curMatIndex);
				curMatIndex = curMatIndex + 1;
			} while (curMatIndex < this->nbMaterials);
		}

		this->nbMaterials = 0;
		ed3DUnInstallG2D(&this->g2dManager);
	}

	CActor::Term();

	return;
}

void CActorTeleporter::Draw()
{
	S_DESTINATION_LIST* pSVar1;
	ed_hash_code* peVar2;
	bool bVar3;
	S_DESTINATION_ENTRY* puVar4;
	edDList_material* pMaterialInfo;
	ed_hash_code* puVar5;
	int iVar6;
	uint uVar8;
	int iVar9;
	S_SUBSECTOR_INFO SStack240;
	edF32MATRIX4 eStack192;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 eStack64;
	CActor* pActor;

	CActor::Draw();

	if (this->bOpen != 0) {
		pSVar1 = this->pDestinationList;
		iVar6 = 0;
		if (pSVar1 != (S_DESTINATION_LIST*)0x0) {
			iVar6 = pSVar1->nbEntries;
		}

		if (this->activeButtonIndex < iVar6) {
			puVar4 = pSVar1->aEntries + this->activeButtonIndex;
			bVar3 = GameDList_BeginCurrent();
			if (bVar3 != false) {
				pActor = (this->field_0x1d8).Get();
				if (pActor != (CActor*)0x0) {
					pActor->SV_ComputeDiffMatrixFromInit(&eStack64);
				}

				pMaterialInfo = (edDList_material*)0x0;
				uVar8 = 0;
				iVar6 = 0;
				iVar9 = 0;
				if (puVar4->field_0x0 != 0x10) {
					pMaterialInfo = GetDestinationMaterial(puVar4->field_0x0, puVar4->field_0x4);
					CLevelScheduler::gThis->Level_GetSubSectorInfo(puVar4->field_0x0, puVar4->field_0x4, &SStack240);
					uVar8 = SStack240.flags & 1;
					iVar6 = SStack240.field_0x14;
					iVar9 = SStack240.nbExorcisedWolfen;
				}

				peVar2 = this->pSnapshotHashCode;
				if (peVar2 == (ed_hash_code*)0x0) {
					if ((this->field_0x1d4).Get() != (ed_zone_3d*)0x0) {
						if (pActor == (CActor*)0x0) {
							edDListLoadMatrix(&this->field_0x1f0);
						}
						else {
							edF32Matrix4MulF32Matrix4Hard(&eStack128, &this->field_0x1f0, &eStack64);
							edDListLoadMatrix(&eStack128);
						}

						if (pMaterialInfo != (edDList_material*)0x0) {
							edDListUseMaterial(pMaterialInfo);
							edDListBegin(0.0f, 0.0f, 0.0f, 8, 4);

							if (uVar8 == 0) {
								edDListColor4u8(0x30, 0x30, 0x30, 0x80);
							}
							else {
								edDListColor4u8(0x80, 0x80, 0x80, 0x80);
							}

							edDListTexCoo2f(0.0f, 1.0f);
							edDListVertex4f(-0.5f, -0.5f, 0.0f, 0.0f);
							edDListTexCoo2f(1.0f, 1.0f);
							edDListVertex4f(0.5f, -0.5f, 0.0f, 0.0f);
							edDListTexCoo2f(0.0f, 0.0f);
							edDListVertex4f(-0.5f, 0.5f, 0.0f, 0.0f);
							edDListTexCoo2f(1.0f, 0.0f);
							edDListVertex4f(0.5f, 0.5f, 0.0f, 0.0f);
							edDListEnd();
						}
					}
				}
				else {
					if (pMaterialInfo == (edDList_material*)0x0) {
						*peVar2 = this->snapshotHashCode;
					}
					else {
						puVar5 = ed3DG2DGetHashCode(pMaterialInfo->pManager, pMaterialInfo->pMaterial);
						peVar2 = this->pSnapshotHashCode;
						*peVar2 = *puVar5;
					}
				}

				if ((puVar4->field_0x0 != 0x10) && ((this->field_0x1e0).Get() != (ed_zone_3d*)0x0)) {
					if (pActor == (CActor*)0x0) {
						edDListLoadMatrix(&this->field_0x230);
					}
					else {
						edF32Matrix4MulF32Matrix4Hard(&eStack192, &this->field_0x230, &eStack64);
						edDListLoadMatrix(&eStack192);
					}

					if (this->field_0x1dc != 0) {
						iVar6 = iVar6 - iVar9;
						if (puVar4->field_0x8 < 1) {
							iVar6 = 0;
						}
						if (iVar6 < 1) {
							iVar9 = 0;
							iVar6 = 0;
						}
						else {
							iVar9 = iVar6 / 10;
							iVar6 = iVar6 % 10;
						}

						DisplayDigit(0.2f, 0.0f, 0.2f, 1.0f, iVar9);
						DisplayDigit(0.4f, 0.0f, 0.2f, 1.0f, iVar6);
					}
				}

				GameDList_EndCurrent();
			}
		}
	}

	return;
}

void CActorTeleporter::Reset()
{
	IMPLEMENTATION_GUARD();
}

struct S_SAVE_CLASS_TELEPORTER
{
	int bOpen;
	int activeButtonIndex;
	int field_0x8;
	int field_0xc;
};

void CActorTeleporter::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_TELEPORTER* pSaveData = reinterpret_cast<S_SAVE_CLASS_TELEPORTER*>(pData);

	if (this->bOpen == 0) {
		pSaveData->bOpen = 0;
	}
	else {
		pSaveData->bOpen = 1;
	}

	pSaveData->activeButtonIndex = this->activeButtonIndex;
	pSaveData->field_0x8 = this->field_0x2ac;
	pSaveData->field_0xc = this->field_0x2b0;

	return;
}

void CActorTeleporter::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_NTF_TARGET_STREAM_REF* pSVar1;
	int iVar2;
	int iVar3;

	S_SAVE_CLASS_TELEPORTER* pSaveData = reinterpret_cast<S_SAVE_CLASS_TELEPORTER*>(pData);

	this->field_0x2ac = -1;
	this->field_0x2b0 = -1;

	if (mode == 3) {
		this->field_0x2ac = pSaveData->field_0x8;
		this->field_0x2b0 = pSaveData->field_0xc;
		this->bOpen = pSaveData->bOpen;

		this->activeButtonIndex = pSaveData->activeButtonIndex;
		if (this->activeButtonIndex < 0) {
			this->activeButtonIndex = 0;
		}

		this->field_0x298 = this->activeButtonIndex;

		if (this->bOpen != 0) {
			SetState(6, -1);

			this->field_0x2a4 = (this->disabledDestinationMask & 1 << (this->activeButtonIndex & 0x1fU)) != 0 ^ 1;

			if (this->field_0x2a4 == 0) {
				this->field_0x170.pTargetStreamRef->SwitchOff(this);

				this->field_0x180.pTargetStreamRef->SwitchOn(this);
			}
			else {
				this->field_0x170.SwitchOn(this);
				this->field_0x180.pTargetStreamRef->SwitchOff(this);
			}

			this->field_0x178.pTargetStreamRef->SwitchOff(this);
		}
	}

	return;
}

CBehaviour* CActorTeleporter::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == TELEPORTER_BEHAVIOUR_DEFAULT) {
		pBehaviour = &this->behaviourTeleporterDefault;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorTeleporter::_gStateCfg_TLP[3] =
{
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
};

StateConfig* CActorTeleporter::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 3);
		pStateConfig = _gStateCfg_TLP + state + -5;
	}

	return pStateConfig;
}

int CActorTeleporter::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CCinematic* pCVar1;
	int iVar2;

	if (msg != MESSAGE_CINEMATIC_INSTALL) {
		iVar2 = CActor::InterpretMessage(pSender, msg, pMsgParam);
		return iVar2;
	}

	_msg_cinematic_install_param* pMsg = reinterpret_cast<_msg_cinematic_install_param*>(pMsgParam);

	pCVar1 = g_CinematicManager_0048efc->GetCinematic(this->cinematicId);
	if ((this->aMaterials != (edDList_material*)0x0) && (pCVar1 == pMsg->pCinematic)) {
		if (pMsg->action == 1) {
			if (this->nbMaterials != 0) {
				iVar2 = 0;
				if (0 < this->nbMaterials) {
					do {
						edDListTermMaterial(this->aMaterials + iVar2);
						iVar2 = iVar2 + 1;
					} while (iVar2 < this->nbMaterials);
				}

				this->nbMaterials = 0;
				CLevelScheduler::gThis->Level_TeleporterChanged();
				ed3DUnInstallG2D(&this->g2dManager);

				return 1;
			}

			return 0;
		}

		if (pMsg->action == 0) {
			iVar2 = FUN_002ee670();
			return iVar2;
		}
	}

	return 0;
}

void CActorTeleporter::StateTeleporterActive()
{
	ed_zone_3d* pZone;
	int levelId;
	int index;
	float fVar1;
	float fVar2;
	float fVar3;
	bool bInsideTeleporter;
	uint uVar5;
	CCinematic* pCinematic;
	int iVar7;
	int iVar8;
	int iVar9;
	int cutsceneId;
	int** piVar10;
	CAudioManager* pAudioManager;
	CCinematicManager* pCinematicManager;

	uVar5 = (this->disabledDestinationMask & 1 << (this->activeButtonIndex & 0x1fU)) != 0 ^ 1;
	if (this->field_0x2a4 != uVar5) {
		this->field_0x2a4 = uVar5;
		iVar8 = 0;

		if (this->field_0x2a4 == 0) {
			this->field_0x170.pTargetStreamRef->SwitchOff(this);
			this->field_0x180.SwitchOn(this);
		}
		else {
			this->field_0x170.SwitchOn(this);
			this->field_0x180.pTargetStreamRef->SwitchOff(this);
		}
	}

	if (this->field_0x2a4 != 0) {
		pZone = (this->field_0x188).Get();
		if (pZone == (ed_zone_3d*)0x0) {
			fVar1 = CActorHero::_gThis->currentLocation.x - this->currentLocation.x;
			fVar2 = CActorHero::_gThis->currentLocation.y - this->currentLocation.y;
			fVar3 = CActorHero::_gThis->currentLocation.z - this->currentLocation.z;
			bInsideTeleporter = fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3 <= 1.0f;
		}
		else {
			uVar5 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0);
			bInsideTeleporter = (uVar5 & 1) != 0;
		}

		pCinematicManager = g_CinematicManager_0048efc;
		if (bInsideTeleporter) {
			if (this->pDestinationList == (S_DESTINATION_LIST*)0x0) {
				iVar8 = 0;
			}
			else {
				iVar8 = this->pDestinationList->nbEntries;
			}

			if (this->activeButtonIndex < iVar8) {
				S_DESTINATION_ENTRY* pEntry = &this->pDestinationList->aEntries[this->activeButtonIndex];
				iVar8 = pEntry->field_0x4;
				levelId = pEntry->field_0x0;
				if (iVar8 < 1) {
					iVar8 = -1;
				}
				cutsceneId = pEntry->field_0x8;
				if (levelId == 0) {
					if (CScene::_pinstance->pElevatorCutsceneList == (S_ELEVATOR_CUTSCENE_LIST*)0x0) {
						iVar7 = 0;
					}
					else {
						iVar7 = CScene::_pinstance->pElevatorCutsceneList->nbEntries;
					}

					for (iVar9 = 0; iVar9 < iVar7; iVar9 = iVar9 + 1) {
						index = CScene::_pinstance->pElevatorCutsceneList->aEntries[iVar9];
						pCinematic = pCinematicManager->GetCinematic(index);
						if ((pCinematic != (CCinematic*)0x0) && (bInsideTeleporter = pCinematic->CanBePlayed(), bInsideTeleporter != false)) {
							cutsceneId = index;
						}
					}
				}

				IMPLEMENTATION_GUARD_AUDIO(
				this->pOwner->field_0x2ac = CScene::ptable.g_AudioManager_00451698->FUN_001819b0();
				this->pOwner->field_0x2b0 = CScene::ptable.g_AudioManager_00451698->FUN_00184580();)

				this->condOpArray.Perform();
				CLevelScheduler::gThis->Level_Teleport(this, levelId, iVar8, cutsceneId, this->field_0x168);
			}
		}
	}
	return;
}

void CActorTeleporter::UpdateCurTeleporterState(int levelId, int param_3)
{
	IMPLEMENTATION_GUARD();
}

void CActorTeleporter::NotifyLevelTeleporterChanged()
{
	DetectDisabledDestinations(0);

	return;
}

bool CActorTeleporter::LevelHasTeleporters()
{
	S_DESTINATION_LIST* pSVar1;
	S_DESTINATION_ENTRY* iVar2;
	int iVar3;
	int iVar4;
	int curIndex;
	bool bHasTeleporters;
	S_SUBSECTOR_INFO SStack48;
	CLevelScheduler* pLevel;

	pLevel = CLevelScheduler::gThis;
	bHasTeleporters = false;

	if (((CLevelScheduler::gThis->currentLevelID == 0) && (this->bOpen != 0)) && (curIndex = 0, (this->field_0x1e0).Get() != (ed_zone_3d*)0x0)) {
		bHasTeleporters = false;
		while (true) {
			pSVar1 = this->pDestinationList;
			iVar3 = 0;
			if (pSVar1 != (S_DESTINATION_LIST*)0x0) {
				iVar3 = pSVar1->nbEntries;
			}

			if (iVar3 <= curIndex) break;

			iVar2 = pSVar1->aEntries + curIndex;
			iVar3 = iVar2->field_0x4;
			if (((iVar3 != 0x10) && (iVar4 = iVar2->field_0x8, 0 < iVar4)) && (pLevel->Level_GetSubSectorInfo(iVar3, iVar4, &SStack48), (SStack48.flags & 1) != 0)) {
				bHasTeleporters = true;
			}

			curIndex = curIndex + 1;
		}
	}

	return bHasTeleporters;
}

int CActorTeleporter::FUN_002ee670()
{
	S_DESTINATION_LIST* pSVar1;
	bool bVar2;
	CCinematic* pCinematic;
	int iVar3;
	edBANK_ENTRY_INFO bankEntryInfo;
	int iStack4;

	pCinematic = g_CinematicManager_0048efc->GetCinematic(this->cinematicId);
	bVar2 = pCinematic->LoadEntryByFile(&bankEntryInfo, "G2D", 0);
	if (bVar2 == false) {
		iVar3 = 0;
	}
	else {
		ObjectNaming::SetNextObjectName("Cin G2D");
		ed3DInstallG2D(bankEntryInfo.fileBufferStart, bankEntryInfo.size, &iStack4, &this->g2dManager, 1);
		iVar3 = ed3DG2DGetG2DNbMaterials(&this->g2dManager);
		this->nbMaterials = iVar3;

		pSVar1 = this->pDestinationList;
		if (pSVar1 == (S_DESTINATION_LIST*)0x0) {
			iVar3 = 0;
		}
		else {
			iVar3 = pSVar1->nbEntries;
		}

		if (iVar3 < this->nbMaterials) {
			iVar3 = 0;
			if (pSVar1 != (S_DESTINATION_LIST*)0x0) {
				iVar3 = pSVar1->nbEntries;
			}

			this->nbMaterials = iVar3;
		}

		iVar3 = 0;
		if (0 < this->nbMaterials) {
			do {
				edDListCreatMaterialFromIndex(&this->aMaterials[iVar3], iVar3, &this->g2dManager, 2);
				iVar3 = iVar3 + 1;
			} while (iVar3 < this->nbMaterials);
		}

		CLevelScheduler::gThis->Level_TeleporterChanged();
		iVar3 = 1;
	}

	return iVar3;
}

edDList_material* CActorTeleporter::GetMySubSectorMaterial(int levelId, int nbAreas)
{
	int* piVar1;
	bool bVar2;
	CLevelScheduler* pCVar3;
	edDList_material* pSubSectorMaterial;
	int iVar5;
	int iVar6;
	int iVar7;

	pCVar3 = CLevelScheduler::gThis;
	pSubSectorMaterial = (edDList_material*)0x0;

	IMPLEMENTATION_GUARD_LOG(
	bVar2 = levelId == 0 || levelId == INT_ARRAY_0048ed60[0];
	if (levelId != 0 && levelId != INT_ARRAY_0048ed60[0]) {
		bVar2 = INT_ARRAY_0048ed60[levelId] == 0;
	}

	if (bVar2) {
		pSubSectorMaterial = GetDestinationMaterial(0, 1);
	}
	else {
		iVar7 = 0;
		iVar6 = 0;
		while (true) {
			piVar1 = *(int**)&this->pDestinationList;
			iVar5 = 0;
			if (piVar1 != (int*)0x0) {
				iVar5 = *piVar1;
			}
			if ((iVar5 <= iVar7) || (pSubSectorMaterial != (edDList_material*)0x0)) break;
			iVar5 = *(int*)((int)piVar1 + iVar6 + 4);
			if (iVar5 != 0x10) {
				bVar2 = levelId == iVar5;
				if (!bVar2) {
					bVar2 = iVar5 == INT_ARRAY_0048ed60[levelId];
				}
				if (!bVar2) {
					bVar2 = levelId == INT_ARRAY_0048ed60[iVar5];
				}
				if (bVar2) {
					pSubSectorMaterial = GetDestinationMaterial(iVar5, nbAreas);
				}
			}
			iVar6 = iVar6 + 0xc;
			iVar7 = iVar7 + 1;
		}

		if ((pSubSectorMaterial == (edDList_material*)0x0) && (levelId == pCVar3->currentLevelID)) {
			pSubSectorMaterial = GetDestinationMaterial(this, levelId, nbAreas);
		}
	})

	return pSubSectorMaterial;
}

void CActorTeleporter::DetectDisabledDestinations(int param_2)
{
	S_DESTINATION_LIST* pSVar1;
	int iVar2;
	int iVar3;
	int curEntryIndex;
	uint shiftedIndex;
	uint nbEntries;
	S_SUBSECTOR_INFO SStack96;
	S_SUBSECTOR_INFO SStack48;
	CLevelScheduler* pLevelSched;

	pLevelSched = CLevelScheduler::gThis;
	if (param_2 == 0) {
		shiftedIndex = 1;
		curEntryIndex = 0;
		while (true) {
			pSVar1 = this->pDestinationList;
			iVar2 = 0;
			if (pSVar1 != (S_DESTINATION_LIST*)0x0) {
				iVar2 = pSVar1->nbEntries;
			}

			if (iVar2 <= curEntryIndex) break;

			S_DESTINATION_ENTRY* pEntry = pSVar1->aEntries;

			iVar2 = pEntry->field_0x0;
			if (iVar2 == pLevelSched->currentLevelID) {
				pLevelSched->Level_GetSubSectorInfo(iVar2, pEntry->field_0x4, &SStack96);
				if ((SStack96.flags & 1) == 0) {
					this->disabledDestinationMask = this->disabledDestinationMask | shiftedIndex;
				}
				else {
					this->disabledDestinationMask = this->disabledDestinationMask & ~shiftedIndex;
				}
			}

			curEntryIndex = curEntryIndex + 1;
			shiftedIndex = shiftedIndex << 1;
		}
	}
	else {
		nbEntries = 0;
		if (this->pDestinationList != (S_DESTINATION_LIST*)0x0) {
			nbEntries = this->pDestinationList->nbEntries;
		}

		shiftedIndex = 1;
		curEntryIndex = 0;
		this->disabledDestinationMask = -1 << (nbEntries & 0x1f);
		while (true) {
			pSVar1 = this->pDestinationList;
			iVar2 = 0;
			if (pSVar1 != (S_DESTINATION_LIST*)0x0) {
				iVar2 = pSVar1->nbEntries;
			}

			if (iVar2 <= curEntryIndex) break;

			S_DESTINATION_ENTRY* pEntry = pSVar1->aEntries + curEntryIndex;
			if (pEntry->field_0x0 == 0x10) {
				this->disabledDestinationMask = this->disabledDestinationMask | shiftedIndex;
			}
			else {
				pLevelSched->Level_GetSubSectorInfo(pEntry->field_0x0, pEntry->field_0x4, &SStack48);
				if ((0 < pEntry->field_0x4) && ((SStack48.flags & 1) == 0)) {
					this->disabledDestinationMask = this->disabledDestinationMask | shiftedIndex;
				}
			}

			curEntryIndex = curEntryIndex + 1;
			shiftedIndex = shiftedIndex << 1;
		}
	}

	return;
}

edDList_material* CActorTeleporter::GetDestinationMaterial(int levelId, int elevatorId)
{
	ParticleInfo* pPVar1;
	bool bVar2;
	edDList_material* peVar3;
	int iVar4;
	int iVar5;
	edDList_material* peVar6;

	peVar6 = (edDList_material*)0x0;
	peVar3 = (edDList_material*)0x0;
	iVar5 = 0;
	bVar2 = levelId == 0 || levelId == INT_ARRAY_0048ed60[0];
	if (levelId != 0 && levelId != INT_ARRAY_0048ed60[0]) {
		bVar2 = INT_ARRAY_0048ed60[levelId] == 0;
	}
	if (bVar2) {
		pPVar1 = this->field_0x194;
		if (pPVar1 != (ParticleInfo*)0x0) {
			iVar5 = pPVar1->materialCount_0x4;
			elevatorId = 1;
			peVar6 = pPVar1->materialInfoArray_0x8;
		}
	}
	else {
		if (this->aMaterials == (edDList_material*)0x0) {
			pPVar1 = this->field_0x190;
			if (pPVar1 != (ParticleInfo*)0x0) {
				peVar6 = pPVar1->materialInfoArray_0x8;
				iVar5 = pPVar1->materialCount_0x4;
			}
		}
		else {
			iVar5 = this->nbMaterials;
			peVar6 = this->aMaterials;
		}
	}
	iVar4 = elevatorId + -1;
	if ((((peVar6 != (edDList_material*)0x0) && (0 < iVar5)) && (-1 < iVar4)) && (iVar4 < iVar5)) {
		peVar3 = peVar6 + iVar4;
	}
	return peVar3;
}

void CActorTeleporter::DisplayDigit(float param_2, float param_3, float param_4, float param_5, int digit)
{
	float y;
	float x;
	float fVar1;
	float x_00;

	fVar1 = param_5 * 0.5f;
	edDListUseMaterial(this->field_0x1dc->materialInfoArray_0x8 + digit);
	edDListBlendSet(1);
	edDListBlendFunc_002ca830();
	edDListColor4u8(0x80, 0x80, 0x80, 0x80);
	edDListBegin(0.0f, 0.0f, 0.0f, 8, 4);
	edDListTexCoo2f(0.0f, 1.0f);
	y = param_3 - fVar1;
	x_00 = param_2 - param_4 * 0.5f;
	edDListVertex4f(x_00, y, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 1.0f);
	x = param_2 + param_4 * 0.5f;
	edDListVertex4f(x, y, 0.0f, 0.0f);
	edDListTexCoo2f(0.0f, 0.0f);
	fVar1 = param_3 + fVar1;
	edDListVertex4f(x_00, fVar1, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 0.0f);
	edDListVertex4f(x, fVar1, 0.0f, 0.0f);
	edDListEnd();

	return;
}

void ACTOR_WAYPOINT_LIST::Create(ACTOR_WAYPOINT_LIST** pList, ByteCode* pByteCode)
{
	ACTOR_WAYPOINT_LIST* pAVar4 = (ACTOR_WAYPOINT_LIST*)pByteCode->currentSeekPos;

	pByteCode->currentSeekPos = (char*)pAVar4->aEntries;
	if (pAVar4->nbEntries != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pAVar4->nbEntries * sizeof(ACTOR_WAYPOINT);
	}
	(*pList) = pAVar4;

	return;
}

void S_DESTINATION_LIST::Create(S_DESTINATION_LIST** pList, ByteCode* pByteCode)
{
	S_DESTINATION_LIST* pSVar1 = (S_DESTINATION_LIST*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)pSVar1->aEntries;
	if (pSVar1->nbEntries != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->nbEntries * sizeof(S_DESTINATION_ENTRY);
	}
	(*pList) = pSVar1;
	return;
}

void CBehaviourTeleporterDefault::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourTeleporterDefault::Manage()
{
	ed_zone_3d* pZone;
	bool bVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	uint uVar5;
	int iVar6;
	CActorTeleporter* pTeleporter;

	pTeleporter = this->pOwner;
	(pTeleporter->field_0x170).pStreamEventCamera->Manage(pTeleporter);
	(pTeleporter->field_0x178).pStreamEventCamera->Manage(pTeleporter);
	(pTeleporter->field_0x180).pStreamEventCamera->Manage(pTeleporter);

	iVar6 = pTeleporter->actorState;
	if (iVar6 == 7) {
		pZone = (pTeleporter->field_0x18c).Get();
		if (pZone == (ed_zone_3d*)0x0) {
			fVar2 = CActorHero::_gThis->currentLocation.x -	pTeleporter->currentLocation.x;
			fVar3 = CActorHero::_gThis->currentLocation.y - pTeleporter->currentLocation.y;
			fVar4 = CActorHero::_gThis->currentLocation.z - pTeleporter->currentLocation.z;
			bVar1 = fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4 <= 16.0f;
		}
		else {
			uVar5 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0);
			bVar1 = (uVar5 & 1) != 0;
		}

		iVar6 = 0;
		if (!bVar1) {
			pTeleporter->field_0x178.pTargetStreamRef->SwitchOff(pTeleporter);

			pTeleporter->field_0x2a4 = (pTeleporter->disabledDestinationMask & 1 << (pTeleporter->activeButtonIndex & 0x1fU)) != 0 ^ 1;
			iVar6 = 0;
			if (pTeleporter->field_0x2a4 == 0) {
				pTeleporter->field_0x170.pTargetStreamRef->SwitchOff(pTeleporter);
				pTeleporter->field_0x180.SwitchOn(pTeleporter);
			}
			else {
				pTeleporter->field_0x170.SwitchOn(pTeleporter);
				pTeleporter->field_0x180.pTargetStreamRef->SwitchOff(pTeleporter);
			}

			pTeleporter->SetState(TELEPORTER_STATE_DEFAULT_ACTIVE, -1);
		}
	}
	else {
		if (iVar6 == TELEPORTER_STATE_DEFAULT_ACTIVE) {
			pTeleporter->StateTeleporterActive();
		}
	}
	return;
}

void CBehaviourTeleporterDefault::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorTeleporter* pCVar1;

	this->pOwner = static_cast<CActorTeleporter*>(pOwner);

	if (newState == -1) {
		pCVar1 = this->pOwner;
		if (pCVar1->bOpen == 0) {
			this->pOwner->SetState(TELEPORTER_STATE_DEFAULT_INACTIVE, -1);
		}
		else {
			this->pOwner->SetState(TELEPORTER_STATE_DEFAULT_ACTIVE, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

int CBehaviourTeleporterDefault::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	S_NTF_TARGET_STREAM_REF* pSVar1;
	int iVar2;
	int iVar3;
	CAudioManager* pAudioManager;
	CActorTeleporter* pTeleporter;

	if ((msg == 0xe) || (msg == 0xf)) {
		pTeleporter = this->pOwner;
		if (pTeleporter->bOpen == 0) {
			pTeleporter->SetState(TELEPORTER_STATE_DEFAULT_ACTIVE, -1);
			this->pOwner->bOpen = 1;
			pTeleporter = this->pOwner;
			pTeleporter->field_0x2a4 = (pTeleporter->disabledDestinationMask & 1 << (pTeleporter->activeButtonIndex & 0x1fU)) != 0 ^ 1;
			if (pTeleporter->field_0x2a4 == 0) {
				pTeleporter->field_0x170.pTargetStreamRef->SwitchOff(pTeleporter);
				pTeleporter->field_0x180.SwitchOn(pTeleporter);
			}
			else {
				pTeleporter->field_0x170.SwitchOn(pTeleporter);
				pTeleporter->field_0x180.pTargetStreamRef->SwitchOff(pTeleporter);
			}

			CLevelScheduler::gThis->Level_TeleporterChanged();

			IMPLEMENTATION_GUARD_AUDIO(
			this->pOwner->field_0x2ac = CScene::ptable.g_AudioManager_00451698->FUN_001819b0();
			this->pOwner->field_0x2b0 = CScene::ptable.g_AudioManager_00451698->FUN_00184580();)

			return 1;
		}
	}
	else {
		if (msg == 0x2e) {
			_msg_params_0x2e* pParams_0x2e = reinterpret_cast<_msg_params_0x2e*>(pMsgParam);
			if (this->pOwner->bOpen == 0) {
				pParams_0x2e->field_0x0 = 0xffffffff;
			}
			else {
				pParams_0x2e->field_0x0 = this->pOwner->disabledDestinationMask;
			}

			pParams_0x2e->field_0x4 = this->pOwner->field_0x298;

			return 1;
		}

		if (msg == 0x2d) {
			pTeleporter = this->pOwner;
			pTeleporter->activeButtonIndex = (int)pMsgParam;
			if (pTeleporter->activeButtonIndex < 0) {
				pTeleporter->activeButtonIndex = 0;
			}
			pTeleporter->field_0x298 = pTeleporter->activeButtonIndex;
			return 1;
		}
	}
	return 0;
}
