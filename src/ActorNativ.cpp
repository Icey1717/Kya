#include "ActorNativ.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "EventManager.h"
#include "TimeController.h"
#include "FileManager3D.h"
#include "ActorNativCmd.h"

void CActorNativ::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	CBehaviour* pCVar3;
	void* pvVar4;
	float fVar5;

	CActorAutonomous::Create(pByteCode);

	this->field_0x350 = pByteCode->GetU32();
	this->field_0x354 = pByteCode->GetU32();
	this->field_0x358 = pByteCode->GetU32();
	this->moveSpeedNoHeldObject = pByteCode->GetF32();
	this->moveSpeedHeldObject = pByteCode->GetF32();
	this->field_0x370 = pByteCode->GetU32();
	this->field_0x368 = pByteCode->GetS32();
	this->field_0x364 = pByteCode->GetS32();
	this->field_0x378 = pByteCode->GetU32();
	this->field_0x37c = pByteCode->GetU32();
	this->field_0x380 = pByteCode->GetU32();
	this->field_0x3f0 = (CActor*)0x0;
	this->nbActions = 0;

	pCVar3 = GetBehaviour(NATIVE_BEHAVIOUR_LIVE);
	if (pCVar3 != (CBehaviour*)0x0) {
		this->nbActions = this->nbActions + 1;
	}

	pCVar3 = GetBehaviour(NATIVE_BEHAVIOUR_SPEAK);
	if (pCVar3 != (CBehaviour*)0x0) {
		this->nbActions = this->nbActions + (this->behaviourSpeak).nbTrajectoryParams;
	}

	pCVar3 = GetBehaviour(NATIVE_BEHAVIOUR_TAKE_AND_PUT);
	if (pCVar3 != (CBehaviour*)0x0) {
		this->nbActions = this->nbActions + (this->behaviourTakeAndPut).nbTrajectoryParams;
	}

	if (this->nbActions < 1) {
		this->aActions = (ActionEntry*)0x0;
	}
	else {
		this->aActions = new ActionEntry[this->nbActions];
	}

	return;
}

void CActorNativ::Init()
{
	CAnimation* pAnimationController;
	bool bVar1;
	int iVar2;
	CBehaviour* pCVar3;
	edAnmLayer* peVar4;
	float fVar5;

	this->field_0x53c = 0;
	this->bHasObject = 0;
	this->pHeldItemHierarchy = (ed_3d_hierarchy*)0x0;
	this->pHeldItemNode = (edNODE*)0x0;

	if ((this->field_0x364 != -1) && (this->field_0x368 != -1)) {
		memset(&this->hierSetup, 0, sizeof(ed_3d_hierarchy_setup));
		(this->hierBoundingSphere).x = 0.0f;
		(this->hierBoundingSphere).y = 0.0f;
		(this->hierBoundingSphere).z = 0.0f;
		(this->hierBoundingSphere).w = 10.0f;
		this->hierSetup.pBoundingSphere = &this->hierBoundingSphere;
		this->hierClipInfo = 75.0f;
		this->hierSetup.clipping_0x0 = (char*)&this->hierClipInfo;
		ChangeDisplayState(0);
		this->pAnimationController->RegisterBone(this->field_0x350);
	}

	if (GetIdMacroAnim(3) != -1) {
		(this->emotionInfo).macroAnimId = GetIdMacroAnim(3);

		if ((this->emotionInfo).macroAnimId != -1) {
			(this->emotionInfo).field_0x8 = -1;
			(this->emotionInfo).field_0x4 = -1;
			(this->emotionInfo).field_0x1c = 0.0f;
			(this->emotionInfo).field_0xc = 0.0f;
			(this->emotionInfo).field_0x10 = 0.0f;
			(this->emotionInfo).field_0x14 = 0.0f;
			(this->emotionInfo).field_0x18 = 1.0f;

			if ((this->emotionInfo).macroAnimId != -1) {
				(this->emotionInfo).field_0x28 = 1.0f;
				(this->emotionInfo).field_0x2c = 2.0f;
				(this->emotionInfo).field_0x24 = 0.0f;
				(this->emotionInfo).field_0x30 = 0.3f;

				if (((this->emotionInfo).macroAnimId != -1) && (iVar2 = (this->emotionInfo).field_0x4, iVar2 != 0)) {
					(this->emotionInfo).field_0x8 = iVar2;
					(this->emotionInfo).field_0x4 = 0;
					(this->emotionInfo).field_0x1c = 0.0f;
					(this->emotionInfo).field_0x20 = 0.5f;

					if ((this->emotionInfo).field_0x4 == -1) {
						fVar5 = 0.0f;
					}
					else {
						fVar5 = 0.3f;
						if ((this->emotionInfo).field_0x8 == -1) {
							(this->emotionInfo).field_0x10 = 0.0f;
							fVar5 = 0.3f;
						}
					}

					if ((this->emotionInfo).macroAnimId != -1) {
						(this->emotionInfo).field_0xc = (this->emotionInfo).field_0x10;
						(this->emotionInfo).field_0x14 = fVar5;
						(this->emotionInfo).field_0x18 = 0.5f;
					}
				}
			}

			pAnimationController = this->pAnimationController;
			bVar1 = pAnimationController->IsLayerActive(2);
			if (bVar1 == false) {
				(this->emotionInfo).macroAnimId = -1;
			}
			else {
				iVar2 = pAnimationController->PhysicalLayerFromLayerId(2);
				peVar4 = (pAnimationController->anmBinMetaAnimator).aAnimData + iVar2;
				peVar4->blendOp = 3;
				peVar4->field_0x4 = 0.0f;
			}
		}

		if ((this->emotionInfo).macroAnimId != -1) {
			(this->emotionInfo).field_0x28 = 30.0f;
			(this->emotionInfo).field_0x2c = 60.0f;
			(this->emotionInfo).field_0x24 = 0.0f;
			(this->emotionInfo).field_0x30 = 1.0f;

			if (((this->emotionInfo).macroAnimId != -1) && (iVar2 = (this->emotionInfo).field_0x4, iVar2 != 0)) {
				(this->emotionInfo).field_0x8 = iVar2;
				(this->emotionInfo).field_0x4 = 0;
				(this->emotionInfo).field_0x1c = 0.0f;
				(this->emotionInfo).field_0x20 = 0.5f;

				if ((this->emotionInfo).field_0x4 == -1) {
					fVar5 = 0.0f;
				}
				else {
					fVar5 = 1.0f;
					if ((this->emotionInfo).field_0x8 == -1) {
						(this->emotionInfo).field_0x10 = 0.0f;
						fVar5 = 1.0f;
					}
				}

				if ((this->emotionInfo).macroAnimId != -1) {
					(this->emotionInfo).field_0xc = (this->emotionInfo).field_0x10;
					(this->emotionInfo).field_0x14 = fVar5;
					(this->emotionInfo).field_0x18 = 0.5f;
				}
			}
		}
	}

	InitActionsTable();
	ClearLocalData();
	this->pAnimationController->RegisterBone(this->field_0x354);
	this->pAnimationController->RegisterBone(this->field_0x358);

	SetLookingAtBones(this->field_0x358, this->field_0x354);
	SetLookingAtBounds(-0.3490658f, 0.3490658f, -0.7853982f, 0.7853982f);

	CActorAutonomous::Init();

	pCVar3 = CActor::GetBehaviour(NATIVE_BEHAVIOUR_SELLER);
	if (pCVar3 != (CBehaviour*)0x0) {
		IMPLEMENTATION_GUARD_LOG(
		pCVar3 = CActor::GetBehaviour(NATIVE_BEHAVIOUR_SELLER);
		FUN_003f2900((int)pCVar3);)
	}

	return;
}

void CActorNativ::Term()
{
	IMPLEMENTATION_GUARD();
}

void CActorNativ::Manage()
{
	uint uVar1;
	bool bVar2;
	edF32MATRIX4* peVar3;
	ed_zone_3d* peVar4;
	CBehaviour* pCVar5;
	edF32MATRIX4 eStack64;

	CActor::Manage();

	ManageOrientationAndLookingAt();

	uVar1 = this->field_0x370;
	if ((uVar1 != 1) && (this->pHeldItemNode != (edNODE*)0x0)) {
		if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
			bVar2 = true;
		}
		else {
			bVar2 = false;
		}

		if (bVar2) {
			if (this->bHasObject != 0) {
				peVar3 = this->pAnimationController->GetCurBoneMatrix(this->field_0x350);
				edF32Matrix4MulF32Matrix4Hard(&eStack64, peVar3, &this->pMeshTransform->base.transformA);
				edF32Matrix4CopyHard(&this->pHeldItemHierarchy->transformA, &eStack64);
			}
		}
		else {
			peVar3 = this->pAnimationController->GetCurBoneMatrix(this->field_0x350);
			edF32Matrix4MulF32Matrix4Hard(&eStack64, peVar3, &this->pMeshTransform->base.transformA);
			edF32Matrix4CopyHard(&this->pHeldItemHierarchy->transformA, &eStack64);
		}
	}

	peVar4 = (ed_zone_3d*)0x0;
	if (this->field_0x380 != 0xffffffff) {
		peVar4 = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->field_0x380);
	}

	if (peVar4 != (ed_zone_3d*)0x0) {
		SV_ComputeDiffMatrixFromInit(&this->field_0x580);
		edF32Matrix4GetInverseOrthoHard(&this->field_0x580, &this->field_0x580);
		peVar4->pMatrix = STORE_SECTION(&this->field_0x580);
	}

	pCVar5 = GetBehaviour(NATIVE_BEHAVIOUR_SELLER);
	if (pCVar5 != (CBehaviour*)0x0) {
		IMPLEMENTATION_GUARD(
		pCVar5 = CActor::GetBehaviour((CActor*)this, NATIVE_BEHAVIOUR_SELLER);
		FUN_003f2150((long)(int)pCVar5);)
	}

	pCVar5 = GetBehaviour(7);
	if ((pCVar5 != (CBehaviour*)0x0) && (this->curBehaviourId == 1)) {
		IMPLEMENTATION_GUARD(
		pCVar5 = CActor::GetBehaviour((CActor*)this, 7);
		if ((GameFlags & 0x4020) != 0) {
			(*(code*)(pCVar5->pVTable->macroAnimId).ManageFrozen)();
		})
	}

	return;
}

void CActorNativ::ComputeLighting()
{
	ed_3d_hierarchy* pHier;

	CScene::ptable.g_LightManager_004516b0->ComputeLighting(this->lightingFloat_0xe0, this, this->lightingFlags, &this->lightConfig);

	pHier = this->pHeldItemHierarchy;
	if (pHier != (ed_3d_hierarchy*)0x0) {
		CScene::ptable.g_LightManager_004516b0->ComputeLighting(1.0f, pHier, &(pHier->transformA).rowT, 1, &this->lightConfig, 0);
	}

	return;
}

void CActorNativ::Reset()
{
	CBehaviourNativTakeAndPut* pTakeAndPut;
	CBehaviourNativSpeak* pSpeak;
	CBehaviour* pCVar1;
	int curTrajectoryParamIndex;
	CTakePutTrajectoryParam* pTrajectoryParam;

	CActorAutonomous::Reset();

	ClearLocalData();

	pTakeAndPut = (CBehaviourNativTakeAndPut*)GetBehaviour(6);
	if ((pTakeAndPut != (CBehaviourNativTakeAndPut*)0x0) && (curTrajectoryParamIndex = 0, 0 < pTakeAndPut->nbTrajectoryParams)) {
		do {
			pTrajectoryParam = pTakeAndPut->aTrajectoryParams + curTrajectoryParamIndex;
			pTrajectoryParam->pathFollowReader.Reset();
			pTrajectoryParam->field_0x1c = pTrajectoryParam->field_0x14;
			curTrajectoryParamIndex = curTrajectoryParamIndex + 1;
		} while (curTrajectoryParamIndex < pTakeAndPut->nbTrajectoryParams);
	}

	pSpeak = (CBehaviourNativSpeak*)GetBehaviour(3);
	if (pSpeak != (CBehaviourNativSpeak*)0x0) {
		pSpeak->Reset();
	}

	pCVar1 = GetBehaviour(7);
	if (pCVar1 != (CBehaviour*)0x0) {
		IMPLEMENTATION_GUARD(
		pCVar1->Reset(); //FUN_003f3430
		)
	}
	return;
}

CBehaviour* CActorNativ::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	switch (behaviourType) {
	case NATIVE_BEHAVIOUR_SPEAK:
		pBehaviour = &behaviourSpeak;
		break;
	case NATIVE_BEHAVIOUR_EXORCISM:
		pBehaviour = &behaviourExorcism;
		break;
	case NATIVE_BEHAVIOUR_LIVE:
		pBehaviour = &behaviourLive;
		break;
	case NATIVE_BEHAVIOUR_TAKE_AND_PUT:
		pBehaviour = &behaviourTakeAndPut;
		break;
	case NATIVE_BEHAVIOUR_SELLER:
		pBehaviour = new CBehaviourNativSeller;
		break;
	default:
		assert(behaviourType < 3);
		pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		break;
	}
	return pBehaviour;
}

StateConfig CActorNativ::_gStateCfg_NTV[54] = {
	StateConfig(0x00000000, 0x00030900), // StateConfig [0]
	StateConfig(0x00000007, 0x00039100), // StateConfig [1]
	StateConfig(0x00000007, 0x00018100), // StateConfig [2]
	StateConfig(0x0000000D, 0x00061100), // StateConfig [3]
	StateConfig(0x00000000, 0x00061100), // StateConfig [4]
	StateConfig(0x00000007, 0x00061100), // StateConfig [5]
	StateConfig(0x0000000C, 0x00067100), // StateConfig [6]
	StateConfig(0x0000000C, 0x00067100), // StateConfig [7]
	StateConfig(0x0000001D, 0x00000000), // StateConfig [8]
	StateConfig(0x0000001E, 0x00000000), // StateConfig [9]
	StateConfig(0x0000001E, 0x00000000), // StateConfig [10]
	StateConfig(0x00000007, 0x00000000), // StateConfig [11]
	StateConfig(0x00000000, 0x00000000), // StateConfig [12]
	StateConfig(0x00000000, 0x00030100), // StateConfig [13]
	StateConfig(0x00000007, 0x0002D100), // StateConfig [14]
	StateConfig(0x00000007, 0x00039100), // StateConfig [15]
	StateConfig(0x00000000, 0x00031100), // StateConfig [16]
	StateConfig(0x00000000, 0x00001100), // StateConfig [17]
	StateConfig(0x00000000, 0x00021100), // StateConfig [18]
	StateConfig(0x00000018, 0x00001100), // StateConfig [19]
	StateConfig(0xFFFFFFFF, 0x00000100), // StateConfig [20]
	StateConfig(0x0000001A, 0x00000100), // StateConfig [21]
	StateConfig(0x0000001B, 0x00020100), // StateConfig [22]
	StateConfig(0x0000001B, 0x00020100), // StateConfig [23]
	StateConfig(0x0000001C, 0x00000100), // StateConfig [24]
	StateConfig(0x00000000, 0x00420100), // StateConfig [25]
	StateConfig(0x00000000, 0x00020100), // StateConfig [26]
	StateConfig(0x00000000, 0x00020100), // StateConfig [27]
	StateConfig(0x00000000, 0x000A0100), // StateConfig [28]
	StateConfig(0x00000000, 0x00080100), // StateConfig [29]
	StateConfig(0x00000000, 0x00080100), // StateConfig [30]
	StateConfig(0x00000000, 0x00080100), // StateConfig [31]
	StateConfig(0x00000000, 0x00020100), // StateConfig [32]
	StateConfig(0x00000000, 0x00400100), // StateConfig [33]
	StateConfig(0x00000000, 0x00300100), // StateConfig [34]
	StateConfig(0x00000000, 0x00300100), // StateConfig [35]
	StateConfig(0x00000000, 0x00300100), // StateConfig [36]
	StateConfig(0x00000000, 0x00300100), // StateConfig [37]
	StateConfig(0x00000000, 0x00300100), // StateConfig [38]
	StateConfig(0x00000000, 0x00300100), // StateConfig [39]
	StateConfig(0x00000000, 0x00300100), // StateConfig [40]
	StateConfig(0x00000000, 0x00300100), // StateConfig [41]
	StateConfig(0x00000000, 0x00300100), // StateConfig [42]
	StateConfig(0x00000000, 0x00300100), // StateConfig [43]
	StateConfig(0x00000000, 0x00300100), // StateConfig [44]
	StateConfig(0x00000000, 0x00300100), // StateConfig [45]
	StateConfig(0x00000000, 0x00300100), // StateConfig [46]
	StateConfig(0x00000000, 0x00300100), // StateConfig [47]
	StateConfig(0x00000000, 0x00380100), // StateConfig [48]
	StateConfig(0x00000000, 0x00380100), // StateConfig [49]
	StateConfig(0x00000000, 0x00380100), // StateConfig [50]
	StateConfig(0x00000000, 0x00220100), // StateConfig [51]
	StateConfig(0x00000007, 0x0002D100), // StateConfig [52]
	StateConfig(0x00000007, 0x00029100), // StateConfig [53]


	// Add more if needed
};

StateConfig* CActorNativ::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_NTV + state + -6;
	}

	return pStateConfig;
}

void CActorNativ::UpdateAnimEffects()
{
	Timer* pTVar1;

	pTVar1 = Timer::GetTimer();
	if ((this->p3DHierNode->base.desiredLod == 0) && (this->emotionInfo.macroAnimId != -1)) {
		this->emotionInfo.DoAnimation(pTVar1->cutsceneDeltaTime, 1.0f, this);
	}
	return;
}

void CActorNativ::ChangeManageState(int state)
{
	CActorAutonomous::ChangeManageState(state);

	if ((this->curBehaviourId == 7) && (state != 0)) {
		SetState(0x1a, -1);
	}
	return;
}

void CActorNativ::ChangeVisibleState(int bVisible)
{
	ed_g3d_manager* pMeshInfo;
	ed_g3d_hierarchy* peVar1;
	edNODE* peVar2;

	CActor::ChangeVisibleState(bVisible);

	if (this->field_0x370 != 1) {
		if (bVisible == 0) {
			if (this->pHeldItemNode != (edNODE*)0x0) {
				ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pHeldItemNode);
				this->pHeldItemNode = (edNODE*)0x0;
				this->pHeldItemHierarchy = (ed_3d_hierarchy*)0x0;
			}
		}
		else {
			if (this->pHeldItemNode == (edNODE*)0x0) {
				pMeshInfo = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(this->field_0x364, this->field_0x368);
				peVar1 = ed3DG3DHierarchyGetFromIndex(pMeshInfo, 0);
				this->field_0x444 = peVar1;
				ed3DG3DHierarchySetStripShadowCastFlag(this->field_0x444, 8);
				peVar2 = ed3DHierarchyAddToScene(CScene::_scene_handleA, pMeshInfo, (char*)0x0);
				this->pHeldItemNode = peVar2;
				this->pHeldItemHierarchy = (ed_3d_hierarchy*)this->pHeldItemNode->pData;
				ed3DHierarchySetSetup(this->pHeldItemHierarchy, &this->hierSetup);
			}
		}
	}

	return;
}

void CActorNativ::CinematicMode_Enter(bool bSetState)
{
	int iVar1;
	bool bVar2;
	float fVar3;

	this->cinematicCachedLocation = this->currentLocation;

	iVar1 = this->curBehaviourId;
	if (((iVar1 == NATIVE_BEHAVIOUR_SPEAK) || (iVar1 - 5U < 2)) || (iVar1 == 9)) {
		this->cinematicCachedBehaviour = iVar1;
		this->cinematicCachedState = this->actorState;
		this->cinematicCachedAnim = this->currentAnimType;
	}
	else {
		this->cinematicCachedBehaviour = -1;
		this->cinematicCachedState = -1;
		this->cinematicCachedAnim = -1;
	}

	CActor::CinematicMode_Enter(bSetState);

	bVar2 = (this->emotionInfo).macroAnimId != -1;
	if (bVar2) {
		if ((bVar2) && (iVar1 = (this->emotionInfo).field_0x4, iVar1 != -1)) {
			(this->emotionInfo).field_0x8 = iVar1;
			(this->emotionInfo).field_0x4 = -1;
			(this->emotionInfo).field_0x1c = 0.0f;
			(this->emotionInfo).field_0x20 = 0.5f;
			if ((this->emotionInfo).field_0x4 == -1) {
				fVar3 = 0.0f;
			}
			else {
				fVar3 = 1.0f;
				if ((this->emotionInfo).field_0x8 == -1) {
					(this->emotionInfo).field_0x10 = 0.0f;
					fVar3 = 1.0f;
				}
			}
			if ((this->emotionInfo).macroAnimId != -1) {
				(this->emotionInfo).field_0xc = (this->emotionInfo).field_0x10;
				(this->emotionInfo).field_0x14 = fVar3;
				(this->emotionInfo).field_0x18 = 0.5f;
			}
		}

		(this->emotionInfo).field_0x24 = -1.0f;
	}

	return;
}

void CActorNativ::CinematicMode_Leave(int behaviourId)
{
	CActor::CinematicMode_Leave(behaviourId);
	(this->emotionInfo).field_0x24 = 0.0f;
	return;
}

int CActorNativ::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	CActor* pReceiver;
	undefined4* puVar2;
	bool bVar3;
	//EnemySubStruct32* pEVar4;
	edF32VECTOR4* peVar5;
	CBehaviour* pCVar6;
	uint uVar7;
	long lVar8;
	undefined** ppuVar9;
	undefined4 uVar10;
	undefined4 uVar11;
	undefined4 uVar12;
	undefined4 local_20[7];
	undefined4* local_4;

	if (msg == 0x60) {
		IMPLEMENTATION_GUARD(
		/* WARNING: Load size is inaccurate */
		if (*pMsgParam == 3) {
			pCVar6 = CActor::GetBehaviour((CActor*)this, 7);
			if ((long)(int)pCVar6 != 0) {
				uVar10 = FUN_003f30d0((long)(int)pCVar6, (long)*(int*)((int)pMsgParam + 4));
				lVar8 = (*(code*)((pCVar6[3].pVTable)->field_0x0).SectorChange)(pCVar6 + 3, uVar10, 0, 1);
				if (lVar8 != 0) {
					pCVar6[10].pVTable = (CBehaviourMovingPlatformVTable*)0x1d;
				}
				return 1;
			}
		})
	}
	else {
		if (msg == 0x8a) {
			IMPLEMENTATION_GUARD(
			pCVar6 = CActor::GetBehaviour((CActor*)this, 6);
			if (pCVar6 != (CBehaviour*)0x0) {
				bVar3 = false;
				if (-1 < (int)pMsgParam) {
					bVar3 = (int)pMsgParam <= (int)&pCVar6[2].pVTable[-1].ChangeManageState + 3;
				}
				if (bVar3) {
					ppuVar9 = &((pCVar6[3].pVTable)->field_0x0)._vt + (int)pMsgParam * 8;
					ppuVar9[7] = (undefined*)((uint)ppuVar9[7] | 1);
					return 1;
				}
			})
		}
		else {
			if (msg == 0x89) {
				IMPLEMENTATION_GUARD(
				pCVar6 = CActor::GetBehaviour((CActor*)this, 6);
				if (pCVar6 != (CBehaviour*)0x0) {
					bVar3 = false;
					if (-1 < (int)pMsgParam) {
						bVar3 = (int)pMsgParam <= (int)&pCVar6[2].pVTable[-1].ChangeManageState + 3;
					}
					if (bVar3) {
						ppuVar9 = &((pCVar6[3].pVTable)->field_0x0)._vt + (int)pMsgParam * 8;
						ppuVar9[7] = (undefined*)((uint)ppuVar9[7] & 0xfffffffe);
						return 1;
					}
				})
			}
			else {
				if (msg != 0x4e) {
					if (msg != MESSAGE_GET_BONE_ID) {
						uVar7 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
						return uVar7;
					}

					if (pMsgParam != (void*)0xa) {
						return 1;
					}

					return this->field_0x350;
				}

				CActorNativMsgParam* pParms = reinterpret_cast<CActorNativMsgParam*>(pMsgParam);

				iVar1 = pParms->type;
				if (iVar1 == 0xf) {
					this->field_0x550 = 0;
					return 1;
				}

				if (iVar1 == 0xe) {
					IMPLEMENTATION_GUARD(
					puVar2 = *(undefined4**)((int)pMsgParam + 0x18);
					uVar12 = puVar2[1];
					uVar10 = puVar2[2];
					uVar11 = puVar2[3];
					*(undefined4*)&this->field_0x540.x = *puVar2;
					*(undefined4*)&this->field_0x540.y = uVar12;
					*(undefined4*)&this->field_0x540.z = uVar10;
					*(undefined4*)&this->field_0x540.w = uVar11;
					peVar5 = CActor::GetBottomPosition((CActor*)this);
					this->field_0x540.y = this->field_0x540.y + ((peVar5->y - this->currentLocation.y) - 0.3);
					this->field_0x550 = 1;
					return 1;)
				}

				if (iVar1 == 1) {
					iVar1 = pParms->field_0x4;
					CPathFinderClient* pPathFinderClient = GetPathfinderClient();
					pPathFinderClient->Init();
					pPathFinderClient->ChangePathfindingId(this, iVar1, &this->currentLocation);

					pReceiver = this->field_0x3f0;
					if ((((pReceiver != (CActor*)0x0) && (pReceiver != pSender)) && (this->curBehaviourId == 3)) && (local_4 = local_20, (this->behaviourSpeak).field_0x24 != -1)) {
						IMPLEMENTATION_GUARD(
						local_20[0] = 7;
						CActor::DoMessage((CActor*)this, pReceiver, 0x4e, (uint)local_4);)
					}

					this->field_0x3f0 = pSender;
					return 1;
				}
			}
		}
	}
	return 0;
}

CPathFinderClient* CActorNativ::GetPathfinderClient()
{
	return &this->pathFinderClient;
}

void CActorNativ::InitActionsTable()
{
	CBehaviour* pCVar1;
	undefined4* puVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	uint uVar6;

	pCVar1 = GetBehaviour(NATIVE_BEHAVIOUR_LIVE);
	if (pCVar1 != (CBehaviour*)0x0) {
		this->aActions->field_0x4 = (this->behaviourLive).field_0x8;
		this->aActions->field_0x0 = 0;
	}

	uVar6 = (uint)(pCVar1 != (CBehaviour*)0x0);

	pCVar1 = GetBehaviour(NATIVE_BEHAVIOUR_TAKE_AND_PUT);
	if (pCVar1 != (CBehaviour*)0x0) {
		iVar5 = 0;
		if (0 < this->behaviourTakeAndPut.nbTrajectoryParams) {
			do {
				this->aActions[iVar5].field_0x4 = this->behaviourTakeAndPut.aTrajectoryParams[iVar5].field_0x0;
				this->aActions[iVar5].field_0x0 = 2;
				iVar5 = iVar5 + 1;
				uVar6 = uVar6 + 1;
			} while (iVar5 < this->behaviourTakeAndPut.nbTrajectoryParams);
		}
	}

	pCVar1 = GetBehaviour(NATIVE_BEHAVIOUR_SPEAK);
	if (pCVar1 != (CBehaviour*)0x0) {
		iVar5 = 0;
		if (0 < (this->behaviourSpeak).nbTrajectoryParams) {
			IMPLEMENTATION_GUARD(
			iVar4 = 0;
			iVar3 = uVar6 << 3;
			do {
				iVar5 = iVar5 + 1;
				puVar2 = (undefined4*)((this->behaviourSpeak).field_0xc + iVar4);
				iVar4 = iVar4 + 8;
				*(undefined4*)((int)&this->aActions->field_0x4 + iVar3) = *puVar2;
				*(undefined4*)((int)&this->aActions->field_0x0 + iVar3) = 1;
				iVar3 = iVar3 + 8;
			} while (iVar5 < (this->behaviourSpeak).field_0x8);)
		}
	}

	return;
}

void CActorNativ::ClearLocalData()
{
	float fVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	edF32MATRIX4* peVar5;
	edF32MATRIX4* peVar6;

	//*(undefined*)&this->field_0x4f0 = 1;
	//*(undefined*)&this->field_0x400 = 0;
	//SetHasObject(this, false);
	//*(undefined4*)&this->field_0x404 = 0;

	this->field_0x4f8 = -1;
	this->field_0x4fc = -1;
	this->field_0x500 = -1;

	this->cinematicCachedBehaviour = -1;
	this->cinematicCachedState = -1;
	this->cinematicCachedAnim = -1;
	this->field_0x530 = 1;
	this->field_0x534 = 0;
	
	this->lightAmbient = gF32Vector4Zero;
	this->lightDirections = gF32Matrix4Unit;
	this->lightColours = gF32Matrix4Unit;

	(this->lightConfig).pLightAmbient = &this->lightAmbient;
	(this->lightConfig).pLightDirections = &this->lightDirections;
	(this->lightConfig).pLightColorMatrix = &this->lightColours;

	this->field_0x504 = 0;
	this->field_0x550 = 0;
	this->field_0x551 = 0;
	return;
}

void CActorNativ::ChooseBehaviour()
{
	float fVar1;
	float fVar2;
	float fVar3;
	CPathFinderClient* piVar4;
	CPathFinderClient* pPathFinderClient;
	ulong uVar5;
	float fVar6;

	fVar1 = (this->cinematicCachedLocation).x - this->currentLocation.x;
	fVar2 = (this->cinematicCachedLocation).y - this->currentLocation.y;
	fVar3 = (this->cinematicCachedLocation).z - this->currentLocation.z;

	fVar6 = CActorMovable::FUN_00120250(0.2f);
	fVar6 = CActorMovable::FUN_00120250(fVar6);

	if (fVar6 < sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3)) {
		piVar4 = GetPathfinderClient();
		if (piVar4->id != -1) {
			pPathFinderClient = GetPathfinderClient();
			IMPLEMENTATION_GUARD(
			uVar5 = pPathFinderClient->IsValidPosition(&this->cinematicCachedLocation);
			if (uVar5 != 0) {
				this->field_0x504 = 0;
				SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 0x14, -1);
				return;
			})
		}

		this->field_0x504 = 0;
		SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 6, -1);
	}
	else {
		SetBehaviour(this->cinematicCachedBehaviour, this->cinematicCachedState, this->cinematicCachedAnim);
		this->cinematicCachedBehaviour = -1;
		this->cinematicCachedState = -1;
		this->cinematicCachedAnim = -1;
	}

	return;
}

void CActorNativ::ManageOrientationAndLookingAt()
{
	int iVar1;
	bool bVar2;
	StateConfig* pSVar3;
	long lVar4;
	uint uVar5;
	float fVar6;
	float puVar7;
	float puVar8;
	float fVar7;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	iVar1 = this->actorState;
	if (iVar1 == -1) {
		uVar5 = 0;
	}
	else {
		pSVar3 = GetStateCfg(iVar1);
		uVar5 = pSVar3->flags_0x4;
	}
	if (((uVar5 & 0x40000) != 0) && (this->field_0x550 != 0)) {
		fVar7 = 0.0f;
		edF32Vector4SubHard(&local_30, &this->field_0x540, &this->currentLocation);
		edF32Vector4NormalizeHard(&local_30, &local_30);

		local_10.x = this->rotationQuat.x;
		local_10.y = 0.0f;
		local_10.z = this->rotationQuat.z;
		local_10.w = 0.0f;

		edF32Vector4NormalizeHard(&local_10, &local_10);

		local_20.y = 0.0f;
		local_20.w = 0.0f;
		local_20.x = local_30.x;
		local_20.z = local_30.z;
		fVar6 = edF32Vector4GetDistHard(&local_20);
		if (0.0 < fVar6) {
			edF32Vector4NormalizeHard(&local_20, &local_20);
			GetTimer();
			puVar7 = edF32Vector4DotProductHard(&local_10, &local_20);
			if (1.0f < puVar7) {
				puVar8 = 1.0f;
			}
			else {
				puVar8 = -1.0f;
				if (-1.0f <= puVar7) {
					puVar8 = puVar7;
				}
			}

			fVar7 = acosf(puVar8);
		}

		if (0.0f < local_10.x * local_20.z - local_20.x * local_10.z) {
			if (0.7853982f < fVar7) {
				this->field_0x551 = 1;
			}
		}
		else {
			if (0.7853982f < fVar7) {
				this->field_0x551 = 1;
			}
		}

		if (this->field_0x551 != 1) {
			if ((this->field_0x370 - 5 < 2) || (this->field_0x370 == 7)) {
				bVar2 = true;
			}
			else {
				bVar2 = false;
			}

			if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
				bVar2 = false;
			}

			if (!bVar2) goto LAB_00162620;
		}

		bVar2 = SV_UpdateOrientationToPosition2D(5.0f, &this->field_0x540);
		if (bVar2 == true) {
			this->field_0x551 = 0;
		}

		UpdatePosition(&this->currentLocation, false);
	}

LAB_00162620:
	iVar1 = this->actorState;
	uVar5 = 0;
	if (iVar1 != -1) {
		pSVar3 = GetStateCfg(iVar1);
		uVar5 = pSVar3->flags_0x4;
	}

	if (((uVar5 & 0x20000) != 0) && (this->field_0x550 != 0)) {
		if ((this->field_0x370 - 5 < 2) || (this->field_0x370 == 7)) {
			bVar2 = true;
		}
		else {
			bVar2 = false;
		}
		if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
			bVar2 = false;
		}
		if (!bVar2) {
			lVar4 = IsLookingAt();
			if (lVar4 == 0) {
				SetLookingAtOn(0.2f);
			}

			SetLookingAtRotationHeight(1.5708f, &this->field_0x540);
			return;
		}
	}

	SetLookingAtOff();

	return;
}

void CActorNativ::BehaviourNativTakeAndPut_InitState(int newState)
{
	uint uVar1;

	if (newState != NATIVE_STATE_TAKE_PUT_WALK) {
		if (newState == 0x19) {
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;

			this->dynamicExt.field_0x6c = 0.0f;
			this->dynamic.speed = 0.0f;
		}
		else {
			if (newState == 0x18) {
				this->field_0x534 = 0;
				IMPLEMENTATION_GUARD(
				uVar1 = FUN_00162270(this);)
				if (uVar1 != 0) {
					SetState(NATIVE_STATE_TAKE_PUT_TURN_TO, -1);
				}
			}
			else {
				if (newState == 0x17) {
					this->field_0x534 = 0;
					IMPLEMENTATION_GUARD(
					uVar1 = FUN_00162270(this);)
					if (uVar1 != 0) {
						SetState(NATIVE_STATE_TAKE_PUT_TURN_TO, -1);
					}
				}
			}
		}
	}

	return;
}

void CActorNativ::BehaviourNativTakeAndPut_Manage(CBehaviourNativTakeAndPut* pBehaviour)
{
	int iVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	CTakePutTrajectoryParam* pCVar5;
	edF32VECTOR4* rotEuler;
	Timer* pTVar6;
	CPathFollowReader* pCVar7;
	edF32VECTOR4 eStack16;

	iVar1 = this->actorState;
	if (iVar1 == 0x19) {
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
		pCVar2 = this->pAnimationController;
		peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
		bVar4 = false;

		if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
			if (peVar3->animPlayState == 0) {
				bVar4 = false;
			}
			else {
				bVar4 = (peVar3->field_0xcc & 2) != 0;
			}
		}

		if (bVar4) {
			iVar1 = (this->behaviourTakeAndPut).curTrajectoryParamIndex;
			pCVar5 = (CTakePutTrajectoryParam*)0x0;
			if (iVar1 != -1) {
				pCVar5 = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
			}
			if (pCVar5 == (CTakePutTrajectoryParam*)0x0) {
				pCVar7 = (CPathFollowReader*)0x0;
			}
			else {
				pCVar5 = (CTakePutTrajectoryParam*)0x0;
				if (iVar1 != -1) {
					pCVar5 = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
				}
				pCVar7 = &pCVar5->pathFollowReader;
			}

			bVar4 = pCVar7->AtGoal(pCVar7->splinePointIndex, pCVar7->field_0xc);
			if (bVar4 == false) {
				SetState(NATIVE_STATE_TAKE_PUT_WALK, -1);
			}
			else {
				SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 6, -1);
			}
		}
	}
	else {
		if (iVar1 == 0x18) {
			ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
			pTVar6 = GetTimer();
			this->field_0x408 = this->field_0x408 + pTVar6->cutsceneDeltaTime;
			pCVar2 = this->pAnimationController;
			peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
			bVar4 = false;
			if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
				if (peVar3->animPlayState == 0) {
					bVar4 = false;
				}
				else {
					bVar4 = (peVar3->field_0xcc & 2) != 0;
				}
			}

			if (bVar4) {
				this->field_0x534 = this->field_0x534 + 1;
			}

			if (1 < this->field_0x534) {
				IMPLEMENTATION_GUARD(
				ChooseToolState();)
			}
		}
		else {
			if (iVar1 == 0x17) {
				IMPLEMENTATION_GUARD(
				StateNativTakePutUsedTool(pBehaviour);)
			}
			else {
				if (iVar1 == NATIVE_STATE_TAKE_PUT_WALK) {
					StateNativTakePutWalk(pBehaviour);
				}
				else {
					if (iVar1 == NATIVE_STATE_TAKE_PUT_TURN_TO) {
						StateNativTakePutTurnTo();
					}
				}
			}
		}
	}
	return;
}

void CActorNativ::BehaviourNativTakeAndPut_TermState(int oldState)
{
	int iVar1;
	CTakePutTrajectoryParam* pCVar2;
	CPathFollowReader* pPathFollowReader;

	if (oldState == 0x19) {
		IMPLEMENTATION_GUARD(
		SetHasObject(false);)

		iVar1 = (this->behaviourTakeAndPut).curTrajectoryParamIndex;
		if (iVar1 == -1) {
			pCVar2 = (CTakePutTrajectoryParam*)0x0;
		}
		else {
			pCVar2 = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
		}

		if (pCVar2 == (CTakePutTrajectoryParam*)0x0) {
			pPathFollowReader = (CPathFollowReader*)0x0;
		}
		else {
			pCVar2 = (CTakePutTrajectoryParam*)0x0;
			if (iVar1 != -1) {
				pCVar2 = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
			}
			pPathFollowReader = &pCVar2->pathFollowReader;
		}

		pPathFollowReader->Reset();
	}
	else {
		if (oldState == 0x18) {
			this->field_0x530 = 1;
		}
		else {
			if (oldState == 0x17) {
				this->field_0x530 = 1;
			}
			else {
				if (oldState == NATIVE_STATE_TAKE_PUT_WALK) {
					SV_AUT_PathfindingEnd();
				}
			}
		}
	}

	return;
}

void CActorNativ::StateNativTakePutTurnTo()
{
	IMPLEMENTATION_GUARD(
		iVar1 = (this->behaviourTakeAndPut).curTrajectoryParamIndex;
	pCVar5 = (CTakePutTrajectoryParam*)0x0;
	if (iVar1 != -1) {
		pCVar5 = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
	}
	if (pCVar5 == (CTakePutTrajectoryParam*)0x0) {
		pCVar7 = (CPathFollowReader*)0x0;
	}
	else {
		pCVar5 = (CTakePutTrajectoryParam*)0x0;
		if (iVar1 != -1) {
			pCVar5 = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
		}
		pCVar7 = &pCVar5->pathFollowReader;
	}
	rotEuler = CPathFollowReader::GetWayPointAngles(pCVar7);
	SetVectorFromAngles(&eStack16, (edF32VECTOR3*)rotEuler);
	bVar4 = CActor::SV_UpdateOrientation2D(2.0, (CActor*)this, &eStack16, 0);
	(*(this->pVTable)->ManageDyn)(4.0, (CActorHero*)this, 0x400, (CActorsTable*)0x0);
	if (bVar4 == true) {
		if ((this->field_0x370 - 5 < 2) || (this->field_0x370 == 7)) {
			bVar4 = true;
		}
		else {
			bVar4 = false;
		}
		if (bVar4) {
			(*(this->pVTable)->SetState)((CActor*)this, 0x17, 0x16);
		}
		else {
			ChooseToolState(this);
		}
	})
		return;
}

void CActorNativ::StateNativTakePutWalk(CBehaviourNativTakeAndPut* pBehaviour)
{
	int iVar1;
	bool bVar2;
	bool bVar3;
	CTakePutTrajectoryParam* pTrajectoryParam;
	edF32VECTOR4* peVar5;
	int* piVar6;
	CPathFinderClient* pPathFinderClient;
	CPathFollowReader* pCVar7;
	uint uVar8;
	CPathFollowReader* pPathFollowReader;
	float fVar10;
	float fVar11;
	float fVar12;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;

	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.rotSpeed = 5.0f;

	if (this->bHasObject == 0) {
	LAB_00160ed0:
		movParamsIn.speed = this->moveSpeedNoHeldObject;
	}
	else {
		uVar8 = this->field_0x370;
		if (((uVar8 == 6) || (uVar8 == 5)) || (uVar8 == 7)) {
			bVar3 = true;
		}
		else {
			bVar3 = false;
		}
		if (!bVar3) goto LAB_00160ed0;
		movParamsIn.speed = this->moveSpeedHeldObject;
	}

	movParamsIn.flags = 0x406;
	movParamsIn.acceleration = 5.0f;

	iVar1 = (this->behaviourTakeAndPut).curTrajectoryParamIndex;
	bVar3 = false;
	if (iVar1 == -1) {
		pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
	}
	else {
		pTrajectoryParam = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
	}

	if (pTrajectoryParam == (CTakePutTrajectoryParam*)0x0) {
		pPathFollowReader = (CPathFollowReader*)0x0;
	}
	else {
		iVar1 = (this->behaviourTakeAndPut).curTrajectoryParamIndex;
		if (iVar1 == -1) {
			pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
		}
		else {
			pTrajectoryParam = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
		}

		pPathFollowReader = &pTrajectoryParam->pathFollowReader;
	}

	this->curWayPointLocation = *pPathFollowReader->GetWayPoint();

	iVar1 = (this->behaviourTakeAndPut).curTrajectoryParamIndex;
	if (iVar1 == -1) {
		pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
	}
	else {
		pTrajectoryParam = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
	}

	pPathFinderClient = GetPathfinderClient();
	bVar2 = pTrajectoryParam->IsWayPointValidPosition(pPathFinderClient);

	if (bVar2 != false) {
		if (pBehaviour->curTrajectoryParamIndex == -1) {
			pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
		}
		else {
			pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
		}

		if (pTrajectoryParam == (CTakePutTrajectoryParam*)0x0) {
			pPathFollowReader = (CPathFollowReader*)0x0;
		}
		else {
			if (pBehaviour->curTrajectoryParamIndex == -1) {
				pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
			}
			else {
				pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
			}

			pPathFollowReader = &pTrajectoryParam->pathFollowReader;
		}

		if (pPathFollowReader->field_0xc != 0) {
			if (pBehaviour->curTrajectoryParamIndex == -1) {
				pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
			}
			else {
				pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
			}

			if (pTrajectoryParam == (CTakePutTrajectoryParam*)0x0) {
				pPathFollowReader = (CPathFollowReader*)0x0;
			}
			else {
				if (pBehaviour->curTrajectoryParamIndex == -1) {
					pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
				}
				else {
					pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
				}

				pPathFollowReader = &pTrajectoryParam->pathFollowReader;
			}

			if (pPathFollowReader->splinePointIndex == 0) {
				if (GetPathfinderClient()->id != -1) {
					pPathFinderClient = GetPathfinderClient();
					bVar2 = pPathFinderClient->IsValidPosition(&this->currentLocation);
					if (bVar2 != false) {
						SV_AUT_MoveTo(&movParamsOut, &movParamsIn, &this->curWayPointLocation);
						bVar3 = true;
						ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
					}
				}
				goto LAB_00161180;
			}
		}

		SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &this->curWayPointLocation);

		bVar3 = true;
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
	}

LAB_00161180:
	fVar10 = (this->curWayPointLocation).x - this->currentLocation.x;
	fVar11 = (this->curWayPointLocation).z - this->currentLocation.z;
	if (!bVar3) {
		uVar8 = this->field_0x370;
		if (((uVar8 == 6) || (uVar8 == 5)) || (uVar8 == 7)) {
			bVar3 = true;
		}
		else {
			bVar3 = false;
		}

		if ((!bVar3) || (bVar3 = true, this->bHasObject == 0)) {
			bVar3 = false;
		}

		if (!bVar3) {
			SetHasObject(false);
			SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 0x13, -1);
		}
	}

	fVar12 = CActorMovable::FUN_00120250(0.2f);
	fVar12 = CActorMovable::FUN_00120250(fVar12);
	if (sqrtf(fVar10 * fVar10 + 0.0f + fVar11 * fVar11) < fVar12) {
		if (pBehaviour->curTrajectoryParamIndex == -1) {
			pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
		}
		else {
			pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
		}

		if (pTrajectoryParam == (CTakePutTrajectoryParam*)0x0) {
			pPathFollowReader = (CPathFollowReader*)0x0;
		}
		else {
			if (pBehaviour->curTrajectoryParamIndex == -1) {
				pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
			}
			else {
				pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
			}
			pPathFollowReader = &pTrajectoryParam->pathFollowReader;
		}

		bVar3 = pPathFollowReader->AtGoal(pPathFollowReader->splinePointIndex, pPathFollowReader->field_0xc);
		if (bVar3 == false) {
			if (pBehaviour->curTrajectoryParamIndex == -1) {
				pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
			}
			else {
				pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
			}

			if (pTrajectoryParam == (CTakePutTrajectoryParam*)0x0) {
				pPathFollowReader = (CPathFollowReader*)0x0;
			}
			else {
				if (pBehaviour->curTrajectoryParamIndex == -1) {
					pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
				}
				else {
					pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
				}
				pPathFollowReader = &pTrajectoryParam->pathFollowReader;
			}

			if (pBehaviour->curTrajectoryParamIndex == -1) {
				pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
			}
			else {
				pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
			}

			if (pTrajectoryParam == (CTakePutTrajectoryParam*)0x0) {
				pCVar7 = (CPathFollowReader*)0x0;
			}
			else {
				if (pBehaviour->curTrajectoryParamIndex == -1) {
					pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
				}
				else {
					pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
				}
				pCVar7 = &pTrajectoryParam->pathFollowReader;
			}

			if (pPathFollowReader->splinePointIndex == pCVar7->pPathFollow->splinePointCount + -1) {
				this->dynamicExt.normalizedTranslation.x = 0.0f;
				this->dynamicExt.normalizedTranslation.y = 0.0f;
				this->dynamicExt.normalizedTranslation.z = 0.0f;
				this->dynamicExt.normalizedTranslation.w = 0.0f;
				this->dynamicExt.field_0x6c = 0.0f;
				this->dynamic.speed = 0.0f;
				this->field_0x408 = 0.0f;

				uVar8 = FUN_00162270();
				if (uVar8 == 0) {
					uVar8 = this->field_0x370;
					if (((uVar8 == 6) || (uVar8 == 5)) || (uVar8 == 7)) {
						bVar3 = true;
					}
					else {
						bVar3 = false;
					}
					if (bVar3) {
						SetState(0x17, 0x16);
					}
					else {
						ChooseToolState();
					}
				}
				else {
					SetState(NATIVE_STATE_TAKE_PUT_TURN_TO, -1);
				}
			}
			else {
				if (pBehaviour->curTrajectoryParamIndex == -1) {
					pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
				}
				else {
					pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
				}
				if (pTrajectoryParam == (CTakePutTrajectoryParam*)0x0) {
					pPathFollowReader = (CPathFollowReader*)0x0;
				}
				else {
					if (pBehaviour->curTrajectoryParamIndex == -1) {
						pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
					}
					else {
						pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
					}
					pPathFollowReader = &pTrajectoryParam->pathFollowReader;
				}
				if ((pPathFollowReader->splinePointIndex == 0) && (this->bHasObject != 0)) {
					uVar8 = this->field_0x370;
					if ((uVar8 == 6) || ((uVar8 == 5 || (uVar8 == 7)))) {
						bVar3 = true;
					}
					else {
						bVar3 = false;
					}
					if (bVar3) {
						this->dynamicExt.normalizedTranslation.x = 0.0f;
						this->dynamicExt.normalizedTranslation.y = 0.0f;
						this->dynamicExt.normalizedTranslation.z = 0.0f;
						this->dynamicExt.normalizedTranslation.w = 0.0f;
						this->dynamicExt.field_0x6c = 0.0f;
						this->dynamic.speed = 0.0f;
						this->field_0x408 = 0.0f;
						uVar8 = FUN_00162270();
						if (uVar8 != 0) {
							SetState(0x16, -1);
							return;
						}

						uVar8 = this->field_0x370;
						if (((uVar8 == 6) || (uVar8 == 5)) || (uVar8 == 7)) {
							bVar3 = true;
						}
						else {
							bVar3 = false;
						}

						if (!bVar3) {
							ChooseToolState();
							return;
						}

						SetState(0x17, 0x16);
						return;
					}

					SetHasObject(false);
				}

				if (pBehaviour->curTrajectoryParamIndex == -1) {
					pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
				}
				else {
					pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
				}

				if (pTrajectoryParam == (CTakePutTrajectoryParam*)0x0) {
					pPathFollowReader = (CPathFollowReader*)0x0;
				}
				else {
					if (pBehaviour->curTrajectoryParamIndex == -1) {
						pTrajectoryParam = (CTakePutTrajectoryParam*)0x0;
					}
					else {
						pTrajectoryParam = pBehaviour->aTrajectoryParams + pBehaviour->curTrajectoryParamIndex;
					}
					pPathFollowReader = &pTrajectoryParam->pathFollowReader;
				}

				pPathFollowReader->NextWayPoint();
			}
		}
		else {
			uVar8 = this->field_0x370;
			if (((uVar8 == 6) || (uVar8 == 5)) || (uVar8 == 7)) {
				bVar3 = true;
			}
			else {
				bVar3 = false;
			}

			if ((!bVar3) || (bVar3 = true, this->bHasObject == 0)) {
				bVar3 = false;
			}

			if (bVar3) {
				this->dynamicExt.normalizedTranslation.x = 0.0f;
				this->dynamicExt.normalizedTranslation.y = 0.0f;
				this->dynamicExt.normalizedTranslation.z = 0.0f;
				this->dynamicExt.normalizedTranslation.w = 0.0f;
				this->dynamicExt.field_0x6c = 0.0f;
				this->dynamic.speed = 0.0f;
				this->field_0x408 = 0.0f;
				SetState(0x19, -1);
			}
			else {
				SetHasObject(false);
				SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 0x13, -1);
			}
		}
	}

	return;
}

void CActorNativ::SetHasObject(bool bHasObject)
{
	bool bVar1;
	ed_g3d_manager* pG3D;
	edNODE* pNode;

	if ((this->field_0x370 - 5 < 2) || (this->field_0x370 == 7)) {
		bVar1 = true;
	}
	else {
		bVar1 = false;
	}
	if (bVar1) {
		if (this->bHasObject == 0) {
			if ((bHasObject != false) && (this->pHeldItemNode == (edNODE*)0x0)) {
				pG3D = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(this->field_0x364, this->field_0x368);
				pNode = ed3DHierarchyAddToScene(CScene::_scene_handleA, pG3D, (char*)0x0);
				this->pHeldItemNode = pNode;
				this->pHeldItemHierarchy = (ed_3d_hierarchy*)this->pHeldItemNode->pData;
				ed3DHierarchySetSetup(this->pHeldItemHierarchy, &this->hierSetup);
			}
		}
		else {
			if ((bHasObject == false) && (this->pHeldItemNode != (edNODE*)0x0)) {
				ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pHeldItemNode);
				this->pHeldItemNode = (edNODE*)0x0;
				this->pHeldItemHierarchy = (ed_3d_hierarchy*)0x0;
			}
		}
	}

	this->bHasObject = bHasObject;
	return;
}

void CActorNativ::ChooseToolState()
{
	ulong uVar1;

	this->field_0x534 = 0;
	switch (this->field_0x370) {
	case 1:
		SetState(0x17, -1);
		break;
	case 2:
		if (this->actorState == 0x17) {
			SetState(0x18, 0x14);
		}
		else {
			uVar1 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
			CScene::_pinstance->field_0x38 = uVar1;
			SetState(0x17, ((int)(((uint)(uVar1 >> 0x10) & 0x7fff) * 2) >> 0xf) + 0x12);
		}
		break;
	case 3:
		uVar1 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
		CScene::_pinstance->field_0x38 = uVar1;
		SetState(0x17, ((int)(((uint)(uVar1 >> 0x10) & 0x7fff) * 2) >> 0xf) + 0x12);
		break;
	case 4:
		SetState(0x17, 0x12);
		break;
	case 5:
	case 6:
	case 7:
		SetState(0x17, 0);
	}

	return;
}

uint CActorNativ::FUN_00162270()
{
	int iVar1;
	CTakePutTrajectoryParam* pCVar2;
	edF32VECTOR4* rotEuler;
	Timer* pTVar3;
	CPathFollowReader* pPathFollowReader;
	float puVar4;
	float puVar5;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	iVar1 = (this->behaviourTakeAndPut).curTrajectoryParamIndex;
	pCVar2 = (CTakePutTrajectoryParam*)0x0;
	if (iVar1 != -1) {
		pCVar2 = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
	}
	if (pCVar2 == (CTakePutTrajectoryParam*)0x0) {
		pPathFollowReader = (CPathFollowReader*)0x0;
	}
	else {
		pCVar2 = (CTakePutTrajectoryParam*)0x0;
		if (iVar1 != -1) {
			pCVar2 = (this->behaviourTakeAndPut).aTrajectoryParams + iVar1;
		}
		pPathFollowReader = &pCVar2->pathFollowReader;
	}

	rotEuler = pPathFollowReader->GetWayPointAngles();
	SetVectorFromAngles(&local_30, &rotEuler->xyz);
	local_10.x = this->rotationQuat.x;
	local_10.y = 0.0f;
	local_10.z = this->rotationQuat.z;
	local_10.w = 0.0f;
	edF32Vector4NormalizeHard(&local_10, &local_10);
	local_20.y = 0.0f;
	local_20.w = 0.0f;
	local_20.x = local_30.x;
	local_20.z = local_30.z;
	edF32Vector4NormalizeHard(&local_20, &local_20);
	pTVar3 = GetTimer();
	fVar5 = pTVar3->cutsceneDeltaTime;

	puVar4 = edF32Vector4DotProductHard(&local_10, &local_20);
	if (1.0f < puVar4) {
		puVar5 = 1.0f;
	}
	else {
		puVar5 = -1.0f;
		if (-1.0f <= puVar4) {
			puVar5 = puVar4;
		}
	}

	fVar4 = acosf(puVar5);
	return fVar4 < fVar5 * 2.0f * 10.0f ^ 1;
}

float CActorNativ::FUN_00164070()
{
	int iVar1;
	CTakePutTrajectoryParam* pCVar2;
	bool bVar3;
	CPathFinderClient* pPathFinderClient;
	ActionEntry* pAction;
	int unaff_s1_lo;
	int iVar5;
	int iVar6;
	float fVar7;

	iVar6 = 0;
	fVar7 = 0.0f;
	iVar5 = -1;
	if (0 < this->nbActions) {
		do {
			pAction = this->aActions + iVar6;
			iVar1 = pAction->field_0x0;
			if (iVar5 != iVar1) {
				unaff_s1_lo = 0;
				iVar5 = iVar1;
			}

			if (iVar1 == 2) {
				pCVar2 = (this->behaviourTakeAndPut).aTrajectoryParams;
				pPathFinderClient = GetPathfinderClient();
				bVar3 = pCVar2[unaff_s1_lo].IsWayPointValidPosition(pPathFinderClient);
				if (bVar3 != false) {
					fVar7 = fVar7 + pAction->field_0x4;
				}
			}
			else {
				fVar7 = fVar7 + pAction->field_0x4;
			}

			iVar6 = iVar6 + 1;
			unaff_s1_lo = unaff_s1_lo + 1;
		} while (iVar6 < this->nbActions);
	}

	return fVar7;
}

CBehaviourNativTakeAndPut::CBehaviourNativTakeAndPut()
	: nbTrajectoryParams(0), aTrajectoryParams(0)
{

}


void CBehaviourNativTakeAndPut::Create(ByteCode* pByteCode)
{
	int iVar1;
	int* pBase;
	uint uVar3;
	CTakePutTrajectoryParam* pfVar3;
	float fVar5;

	this->curTrajectoryParamIndex = 0xffffffff;
	this->nbTrajectoryParams = pByteCode->GetS32();
	uVar3 = this->nbTrajectoryParams;
	if (uVar3 != 0) {
		this->aTrajectoryParams = new CTakePutTrajectoryParam[uVar3];

		iVar1 = 0;
		if (0 < this->nbTrajectoryParams) {
			do {
				pfVar3 = this->aTrajectoryParams + iVar1;
				pfVar3->field_0x0 = pByteCode->GetF32();

				pfVar3->pathFollowReader.Create(pByteCode);

				if (CScene::_pinstance->field_0x1c < 2.21f) {
					pByteCode->GetF32();
				}
				else {
					pfVar3->field_0x14 = pByteCode->GetU32();
				}

				pfVar3->field_0x18 = pByteCode->GetF32();
				iVar1 = iVar1 + 1;
				pfVar3->field_0x1c = pfVar3->field_0x14;
			} while (iVar1 < this->nbTrajectoryParams);
		}
	}

	return;
}

void CBehaviourNativTakeAndPut::Init(CActor* pOwner)
{
	int iVar1;
	CTakePutTrajectoryParam* iVar3;

	this->pOwner = reinterpret_cast<CActorNativ*>(pOwner);

	iVar1 = 0;
	if (0 < this->nbTrajectoryParams) {
		do {
			iVar3 = this->aTrajectoryParams + iVar1;
			iVar3->pathFollowReader.Init();
			iVar1 = iVar1 + 1;
			iVar3->field_0x1c = iVar3->field_0x14;
		} while (iVar1 < this->nbTrajectoryParams);
	}

	return;
}

void CBehaviourNativTakeAndPut::Manage()
{
	this->pOwner->BehaviourNativTakeAndPut_Manage(this);
}

void CBehaviourNativTakeAndPut::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorNativ* pCVar1;
	int iVar2;
	CTakePutTrajectoryParam* pCVar3;
	CPathFollowReader* pCVar4;

	this->pOwner = reinterpret_cast<CActorNativ*>(pOwner);

	if (newState == -1) {
		pCVar1 = this->pOwner;
		if ((pCVar1->cinematicCachedState == -1) || (pCVar1->cinematicCachedBehaviour == -1)) {
			iVar2 = this->curTrajectoryParamIndex;
			pCVar3 = (CTakePutTrajectoryParam*)0x0;
			if (iVar2 != -1) {
				pCVar3 = this->aTrajectoryParams + iVar2;
			}

			if (pCVar3 == (CTakePutTrajectoryParam*)0x0) {
				pCVar4 = (CPathFollowReader*)0x0;
			}
			else {
				pCVar3 = (CTakePutTrajectoryParam*)0x0;
				if (iVar2 != -1) {
					pCVar3 = this->aTrajectoryParams + iVar2;
				}
				pCVar4 = &pCVar3->pathFollowReader;
			}

			if (pCVar4 == (CPathFollowReader*)0x0) {
				pCVar1->SetState(6, -1);
			}
			else {
				pCVar3 = (CTakePutTrajectoryParam*)0x0;
				if (iVar2 != -1) {
					pCVar3 = this->aTrajectoryParams + iVar2;
				}

				if (pCVar3 == (CTakePutTrajectoryParam*)0x0) {
					pCVar4 = (CPathFollowReader*)0x0;
				}
				else {
					pCVar3 = (CTakePutTrajectoryParam*)0x0;
					if (iVar2 != -1) {
						pCVar3 = this->aTrajectoryParams + iVar2;
					}
					pCVar4 = &pCVar3->pathFollowReader;
				}

				if (pCVar4->splinePointIndex != 0) {
					if (iVar2 == -1) {
						pCVar3 = (CTakePutTrajectoryParam*)0x0;
					}
					else {
						pCVar3 = this->aTrajectoryParams + iVar2;
					}

					pCVar4 = (CPathFollowReader*)0x0;
					if (pCVar3 != (CTakePutTrajectoryParam*)0x0) {
						if (iVar2 == -1) {
							pCVar3 = (CTakePutTrajectoryParam*)0x0;
						}
						else {
							pCVar3 = this->aTrajectoryParams + iVar2;
						}
						pCVar4 = &pCVar3->pathFollowReader;
					}

					pCVar4->SetToClosestSplinePoint(&pCVar1->currentLocation);
				}

				this->pOwner->SetState(NATIVE_STATE_TAKE_PUT_WALK, -1);
			}
		}
		else {
			pCVar1->ChooseBehaviour();
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourNativTakeAndPut::End(int newBehaviourId)
{
	return;
}

void CBehaviourNativTakeAndPut::InitState(int newState)
{
	this->pOwner->BehaviourNativTakeAndPut_InitState(newState);
}

void CBehaviourNativTakeAndPut::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourNativTakeAndPut_TermState(oldState);
}

int CBehaviourNativTakeAndPut::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorNativ* pCVar1;
	//CBehaviourMovingPlatformVTable* pCVar2;
	undefined4* puVar3;
	bool bVar4;
	StateConfig* pSVar5;
	uint uVar6;
	int iVar7;
	CBehaviour* pCVar8;
	CTakePutTrajectoryParam* pCVar9;
	CPathFollowReader* pPathFollowReader;
	//CBehaviourMovingPlatformVTable* pCVar10;
	undefined4 uVar11;
	//CBehaviourMovingPlatformVTable* pCVar12;
	undefined4 uVar13;
	//CBehaviourMovingPlatformVTable* pCVar14;
	undefined4 uVar15;
	undefined4 local_20[4];
	int local_10;
	undefined4* local_4;

	if (msg == 0x4e) {
		CActorNativMsgParamOther* pParams = reinterpret_cast<CActorNativMsgParamOther*>(pMsgParam);
		iVar7 = pParams->type;
		if (iVar7 == 0xb) {
			IMPLEMENTATION_GUARD(
			bVar4 = CActorNativ::FUN_00162910(this->pOwner);
			if (bVar4 == false) {
				pCVar1 = this->pOwner;
				(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 3, 0xd, -1);
			}
			else {
				pCVar1 = this->pOwner;
				(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 3, 0xd, NATIVE_STATE_TAKE_PUT_WALK);
			})
		}
		else {
			if ((iVar7 == 10) || (iVar7 == 7)) {
				IMPLEMENTATION_GUARD(
				bVar4 = CActorNativ::FUN_00162910(this->pOwner);
				if (bVar4 == false) {
					pCVar1 = this->pOwner;
					(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 3, 0xc, -1);
				}
				else {
					pCVar1 = this->pOwner;
					(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 3, 0xc, NATIVE_STATE_TAKE_PUT_WALK);
				})
			}
			else {
				if (iVar7 == 0xd) {
					IMPLEMENTATION_GUARD(
					pCVar1 = this->pOwner;
					pCVar1->field_0x4f8 = (pCVar1->base).base.base.curBehaviourId;
					pCVar1 = this->pOwner;
					pCVar1->field_0x4fc = (pCVar1->base).base.base.actorState;
					pCVar1 = this->pOwner;
					pCVar1->field_0x500 = (pCVar1->base).base.base.currentAnimType;
					pCVar1 = this->pOwner;
					puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
					uVar15 = puVar3[1];
					uVar11 = puVar3[2];
					uVar13 = puVar3[3];
					*(undefined4*)&(pCVar1->behaviourSpeak).field_0x10 = *puVar3;
					*(undefined4*)&(pCVar1->behaviourSpeak).field_0x14 = uVar15;
					*(undefined4*)&(pCVar1->behaviourSpeak).field_0x18 = uVar11;
					*(undefined4*)&(pCVar1->behaviourSpeak).field_0x1c = uVar13;
					((this->pOwner)->behaviourSpeak).field_0x24 = *(undefined4*)((int)pMsgParam + 8);
					((this->pOwner)->behaviourSpeak).field_0x28 = *(undefined**)((int)pMsgParam + 0x14);
					pCVar1 = this->pOwner;
					(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 3, 9, *(int*)((int)pMsgParam + 0xc) + 0xd);
					(this->pOwner)->field_0x530 = 0;)
				}
				else {
					if (iVar7 == 6) {
						IMPLEMENTATION_GUARD(
						*(undefined4*)&((this->pOwner)->behaviourSpeak).field_0x20 = *(undefined4*)((int)pMsgParam + 4);
						pCVar1 = this->pOwner;
						puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
						uVar15 = puVar3[1];
						uVar11 = puVar3[2];
						uVar13 = puVar3[3];
						*(undefined4*)&(pCVar1->behaviourSpeak).field_0x10 = *puVar3;
						*(undefined4*)&(pCVar1->behaviourSpeak).field_0x14 = uVar15;
						*(undefined4*)&(pCVar1->behaviourSpeak).field_0x18 = uVar11;
						*(undefined4*)&(pCVar1->behaviourSpeak).field_0x1c = uVar13;
						((this->pOwner)->behaviourSpeak).field_0x24 = *(undefined4*)((int)pMsgParam + 8);
						((this->pOwner)->behaviourSpeak).field_0x28 = *(undefined**)((int)pMsgParam + 0x14);
						pCVar1 = this->pOwner;
						(pCVar1->field_0x570).x = (pCVar1->base).base.base.currentLocation.x;
						(pCVar1->field_0x570).y = (pCVar1->base).base.base.currentLocation.y;
						(pCVar1->field_0x570).z = (pCVar1->base).base.base.currentLocation.z;
						(pCVar1->field_0x570).w = (pCVar1->base).base.base.currentLocation.w;
						pCVar1 = this->pOwner;
						(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 3, 10, -1);)
					}
					else {
						if (iVar7 == 9) {
							IMPLEMENTATION_GUARD(
							*(undefined4*)&((this->pOwner)->behaviourSpeak).field_0x20 = *(undefined4*)((int)pMsgParam + 4);
							pCVar1 = this->pOwner;
							puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
							uVar15 = puVar3[1];
							uVar11 = puVar3[2];
							uVar13 = puVar3[3];
							*(undefined4*)&(pCVar1->behaviourSpeak).field_0x10 = *puVar3;
							*(undefined4*)&(pCVar1->behaviourSpeak).field_0x14 = uVar15;
							*(undefined4*)&(pCVar1->behaviourSpeak).field_0x18 = uVar11;
							*(undefined4*)&(pCVar1->behaviourSpeak).field_0x1c = uVar13;
							((this->pOwner)->behaviourSpeak).field_0x24 = *(undefined4*)((int)pMsgParam + 8);
							((this->pOwner)->behaviourSpeak).field_0x28 = *(undefined**)((int)pMsgParam + 0x14);
							pCVar1 = this->pOwner;
							(*((pCVar1->base).base.base.pVTable)->SetBehaviour)
								((CActor*)pCVar1, 3, 9, *(int*)((int)pMsgParam + 0xc) + 0xd);)
						}
						else {
							if (iVar7 == 5) {
								IMPLEMENTATION_GUARD(
								*(undefined4*)&((this->pOwner)->behaviourSpeak).field_0x20 = *(undefined4*)((int)pMsgParam + 4);
								pCVar1 = this->pOwner;
								puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
								uVar15 = puVar3[1];
								uVar11 = puVar3[2];
								uVar13 = puVar3[3];
								*(undefined4*)&(pCVar1->behaviourSpeak).field_0x10 = *puVar3;
								*(undefined4*)&(pCVar1->behaviourSpeak).field_0x14 = uVar15;
								*(undefined4*)&(pCVar1->behaviourSpeak).field_0x18 = uVar11;
								*(undefined4*)&(pCVar1->behaviourSpeak).field_0x1c = uVar13;
								((this->pOwner)->behaviourSpeak).field_0x24 = *(undefined4*)((int)pMsgParam + 8);
								((this->pOwner)->behaviourSpeak).field_0x28 = *(undefined**)((int)pMsgParam + 0x14);
								pCVar1 = this->pOwner;
								(pCVar1->field_0x570).x = (pCVar1->base).base.base.currentLocation.x;
								(pCVar1->field_0x570).y = (pCVar1->base).base.base.currentLocation.y;
								(pCVar1->field_0x570).z = (pCVar1->base).base.base.currentLocation.z;
								(pCVar1->field_0x570).w = (pCVar1->base).base.base.currentLocation.w;
								pCVar1 = this->pOwner;
								(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 3, 0xb, -1);
								pCVar1 = this->pOwner;
								(pCVar1->field_0x570).x = (pCVar1->base).base.base.currentLocation.x;
								(pCVar1->field_0x570).y = (pCVar1->base).base.base.currentLocation.y;
								(pCVar1->field_0x570).z = (pCVar1->base).base.base.currentLocation.z;
								(pCVar1->field_0x570).w = (pCVar1->base).base.base.currentLocation.w;)
							}
							else {
								if (iVar7 == 4) {
									this->curTrajectoryParamIndex = pParams->field_0x4;
									pCVar1 = this->pOwner;
									iVar7 = (pCVar1->behaviourTakeAndPut).curTrajectoryParamIndex;
									if (iVar7 == -1) {
										pCVar9 = (CTakePutTrajectoryParam*)0x0;
									}
									else {
										pCVar9 = (pCVar1->behaviourTakeAndPut).aTrajectoryParams + iVar7;
									}

									if (pCVar9 == (CTakePutTrajectoryParam*)0x0) {
										pPathFollowReader = (CPathFollowReader*)0x0;
									}
									else {
										pCVar9 = (CTakePutTrajectoryParam*)0x0;
										if (iVar7 != -1) {
											pCVar9 = (pCVar1->behaviourTakeAndPut).aTrajectoryParams + iVar7;
										}
										pPathFollowReader = &pCVar9->pathFollowReader;
									}

									pPathFollowReader->Reset();
									pCVar1 = this->pOwner;
									pCVar1->SetState(NATIVE_STATE_TAKE_PUT_WALK, -1);
								}
								else {
									if (iVar7 == 3) {
										IMPLEMENTATION_GUARD(
										((this->pOwner)->behaviourSpeak).field_0x24 = *(undefined4*)((int)pMsgParam + 8);
										pCVar1 = this->pOwner;
										if ((pCVar1->field_0x4f8 == -1) && (pCVar1->field_0x4fc == -1)) {
											(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 5, 6, -1);
										}
										else {
											(*((pCVar1->base).base.base.pVTable)->SetBehaviour)
												((CActor*)pCVar1, pCVar1->field_0x4f8, pCVar1->field_0x4fc, pCVar1->field_0x500);
											(this->pOwner)->field_0x4f8 = -1;
											(this->pOwner)->field_0x4fc = -1;
											(this->pOwner)->field_0x500 = -1;
										})
									}
									else {
										if (iVar7 == 2) {
											IMPLEMENTATION_GUARD(
											this->curTrajectoryParamIndex = 0xffffffff;
											pCVar1 = this->pOwner;
											(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 5, 0x13, -1);)
										}
										else {
											if (iVar7 == 0x10) {
												IMPLEMENTATION_GUARD(
												pCVar8 = CActor::GetBehaviour((CActor*)this->pOwner, 9);
												pCVar2 = pCVar8[1].pVTable;
												if (((pCVar2[0xe].field_0x0.Draw == (undefined*)0xffffffff) &&
													(pCVar2[0xe].field_0x0.Begin == (BehaviourBeginFunc*)0xffffffff)) &&
													(pCVar2[0xe].field_0x0.End == (undefined*)0xffffffff)) {
													pCVar2[0xe].field_0x0.Draw = pCVar2[2].SaveContext;
													pCVar8[1].pVTable[0xe].field_0x0.Begin =
														(BehaviourBeginFunc*)pCVar8[1].pVTable[2].ChangeManageState;
													pCVar8[1].pVTable[0xe].field_0x0.End = pCVar8[1].pVTable[3].field_0x0.Term;
													pCVar2 = pCVar8[1].pVTable;
													pCVar14 = (CBehaviourMovingPlatformVTable*)(pCVar2->field_0x0).InitState;
													pCVar10 = (CBehaviourMovingPlatformVTable*)(pCVar2->field_0x0).TermState;
													pCVar12 = (CBehaviourMovingPlatformVTable*)(pCVar2->field_0x0).InterpretMessage;
													pCVar8[4].pVTable = (CBehaviourMovingPlatformVTable*)(pCVar2->field_0x0).End;
													pCVar8[5].pVTable = pCVar14;
													pCVar8[6].pVTable = pCVar10;
													pCVar8[7].pVTable = pCVar12;
												}
												pCVar1 = this->pOwner;
												(*((pCVar1->base).base.base.pVTable)->SetBehaviour)((CActor*)pCVar1, 9, 0x3a, -1);)
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else {
		if (msg == 0x14) {
			IMPLEMENTATION_GUARD(
			local_20[0] = 9;
			local_10 = CActorNativ::FUN_00162a70(this->pOwner);
			pCVar1 = this->pOwner;
			local_4 = local_20;
			iVar7 = CActor::DoMessage((CActor*)pCVar1, (CActor*)pCVar1->field_0x3f0, 0x4e, (uint)local_4);
			if (iVar7 != 0) {
				pCVar1 = this->pOwner;
				pCVar1->cinematicCachedBehaviour = (pCVar1->base).base.base.curBehaviourId;
				pCVar1 = this->pOwner;
				pCVar1->cinematicCachedState = (pCVar1->base).base.base.actorState;
				pCVar1 = this->pOwner;
				pCVar1->cinematicCachedAnim = (pCVar1->base).base.base.currentAnimType;
				return 1;
			})
			return 0;
		}
		if (msg == MESSAGE_GET_ACTION) {
	
			(this->pOwner)->field_0x508 = 1.0f;

			pCVar1 = this->pOwner;
			iVar7 = pCVar1->actorState;
			if (iVar7 == -1) {
				uVar6 = 0;
			}
			else {
				pSVar5 = pCVar1->GetStateCfg(iVar7);
				uVar6 = pSVar5->flags_0x4 & 0x4000;
			}

			if (uVar6 == 0) {
				pCVar1 = this->pOwner;
				pCVar1->field_0x4f8 = pCVar1->curBehaviourId;
				pCVar1 = this->pOwner;
				pCVar1->field_0x4fc = pCVar1->actorState;
				pCVar1 = this->pOwner;
				pCVar1->field_0x500 = pCVar1->currentAnimType;
				(this->pOwner)->field_0x504 = 1;
			}

			if (this->pOwner->actorState == 0xc) {
				return 0xf;
			}

			return 0;
		}
	}

	return 0;
}

CBehaviourNativSpeak::CBehaviourNativSpeak()
	: nbTrajectoryParams(0), aTrajectoryParams(0)
{

}

void CBehaviourNativSpeak::Reset()
{
	this->field_0x24 = -1;
	this->field_0x28 = -1.0f;

	return;
}

void CEmotionInfo::DoAnimation(float, float, CActor*)
{
	IMPLEMENTATION_GUARD_FX();
}

bool CTakePutTrajectoryParam::IsWayPointValidPosition(CPathFinderClient* pPathFinderClient)
{
	bool bVar1;
	edF32VECTOR4* v0;

	if ((((this->field_0x1c & 1) == 0) && (pPathFinderClient != (CPathFinderClient*)0x0)) &&
		(pPathFinderClient->id != -1)) {
		v0 = this->pathFollowReader.GetWayPoint(0);
		bVar1 = pPathFinderClient->IsValidPosition(v0);
		if (bVar1 != false) {
			return true;
		}
	}
	return false;
}
