#include "ActorNativ.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "EventManager.h"
#include "TimeController.h"
#include "FileManager3D.h"
#include "ActorHero.h"
#include "CinematicManager.h"
#include "FrontEndDisp.h"
#include "LevelScheduleManager.h"
#include "kya.h"
#include "Rendering/edCTextStyle.h"
#include "Pause.h"
#include "Rendering/DisplayList.h"
#include "edText.h"
#include "FrontEndMoney.h"
#include "ActorNativShop.h"

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
		this->nbActions = this->nbActions + (this->behaviourSpeak).nbSubObjs;
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
		this->hierSetup.clipping_0x0 = &this->hierClipInfo;
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

	pCVar3 = CActor::GetBehaviour(NATIVE_BEHAVIOUR_AKASA);
	if (pCVar3 != (CBehaviour*)0x0) {
		IMPLEMENTATION_GUARD_LOG(
		pCVar3 = CActor::GetBehaviour(NATIVE_BEHAVIOUR_AKASA);
		FUN_003f2900((int)pCVar3);)
	}

	return;
}

void CActorNativ::Term()
{
	CActorAutonomous::Term();

	if (this->pHeldItemNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pHeldItemNode);
	}

	return;
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

	pCVar5 = GetBehaviour(NATIVE_BEHAVIOUR_AKASA);
	if (pCVar5 != (CBehaviour*)0x0) {
		CBehaviourNativAkasa* pBehaviourAkasa = static_cast<CBehaviourNativAkasa*>(GetBehaviour(NATIVE_BEHAVIOUR_AKASA));
		pBehaviourAkasa->ManageComboTutorial();
	}

	pCVar5 = GetBehaviour(NATIVE_BEHAVIOUR_SELLER);
	if ((pCVar5 != (CBehaviour*)0x0) && (this->curBehaviourId == 1)) {
		CBehaviourNativSeller* pBehaviourSeller = static_cast<CBehaviourNativSeller*>(GetBehaviour(NATIVE_BEHAVIOUR_SELLER));
		if ((GameFlags & 0x4020) != 0) {
			pBehaviourSeller->ManageFrozen();
		}
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
	case NATIVE_BEHAVIOUR_AKASA:
		pBehaviour = new CBehaviourNativAkasa;
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
	CActorNativMsgParam_0xe local_20;

	if (msg == 0x60) {
		int* pIntParam = (int*)pMsgParam;
		if (*pIntParam == 3) {
			pCVar6 = GetBehaviour(7);
			if (pCVar6 != (CBehaviour*)0x0) {
				CBehaviourNativSeller* pSeller = static_cast<CBehaviourNativSeller*>(pCVar6);
				uint uVar10 = pSeller->GetPurchaseCutsceneId(pIntParam[1]);
				if (pSeller->addOn.Func_0x20(uVar10, 0, 1) != 0) {
					pSeller->cachedStateId = 0x1d;
				}

				return 1;
			}
		}
	}
	else {
		if (msg == 0x8a) {
			CBehaviourNativTakeAndPut* pTakeAndPut = reinterpret_cast<CBehaviourNativTakeAndPut*>(GetBehaviour(NATIVE_BEHAVIOUR_TAKE_AND_PUT));
			if (pTakeAndPut != (CBehaviourNativTakeAndPut*)0x0) {
				bVar3 = false;
				int newTrajectoryParamIndex = reinterpret_cast<int>(pMsgParam);

				if (-1 < newTrajectoryParamIndex) {
					bVar3 = newTrajectoryParamIndex <= pTakeAndPut->nbTrajectoryParams + -1;
				}

				if (bVar3) {
					pTakeAndPut->aTrajectoryParams[newTrajectoryParamIndex].field_0x1c = pTakeAndPut->aTrajectoryParams[newTrajectoryParamIndex].field_0x1c | 1;
					return 1;
				}
			}
		}
		else {
			if (msg == 0x89) {	
				CBehaviourNativTakeAndPut* pTakeAndPut = reinterpret_cast<CBehaviourNativTakeAndPut*>(GetBehaviour(NATIVE_BEHAVIOUR_TAKE_AND_PUT));
				if (pCVar6 != (CBehaviour*)0x0) {
					bVar3 = false;
					int newTrajectoryParamIndex = reinterpret_cast<int>(pMsgParam);

					if (-1 < newTrajectoryParamIndex) {
						bVar3 = newTrajectoryParamIndex <= pTakeAndPut->nbTrajectoryParams + -1;
					}

					if (bVar3) {
						pTakeAndPut->aTrajectoryParams[newTrajectoryParamIndex].field_0x1c = pTakeAndPut->aTrajectoryParams[newTrajectoryParamIndex].field_0x1c & 0xfffffffe;
						return 1;
					}
				}
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

				CActorNativMsgParam_0xe* pParams = reinterpret_cast<CActorNativMsgParam_0xe*>(pMsgParam);

				iVar1 = pParams->type;
				if (iVar1 == 0xf) {
					this->field_0x550 = 0;
					return 1;
				}

				if (iVar1 == 0xe) {
					this->field_0x540 = *pParams->field_0x18;
					this->field_0x540.y = this->field_0x540.y + ((CActor::GetBottomPosition()->y - this->currentLocation.y) - 0.3f);
					this->field_0x550 = 1;

					return 1;
				}

				if (iVar1 == 1) {
					iVar1 = pParams->field_0x4;
					CPathFinderClient* pPathFinderClient = GetPathfinderClient();
					pPathFinderClient->Init();
					pPathFinderClient->ChangePathfindingId(this, iVar1, &this->currentLocation);

					pReceiver = this->field_0x3f0;
					if ((((pReceiver != (CActor*)0x0) && (pReceiver != pSender)) && (this->curBehaviourId == 3)) && ((this->behaviourSpeak).field_0x24 != -1)) {
						local_20.type = 7;
						DoMessage(pReceiver, MESSAGE_NATIV_CMD, &local_20);
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
		if (0 < (this->behaviourSpeak).nbSubObjs) {
			iVar3 = uVar6;
			do {
				this->aActions[iVar3].field_0x4 = (this->behaviourSpeak).aSubObjs[iVar5].field_0x0;
				this->aActions[iVar3].field_0x0 = 1;
				iVar3 = iVar3 + 1;
				iVar5 = iVar5 + 1;
			} while (iVar5 < (this->behaviourSpeak).nbSubObjs);
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

	this->field_0x4f0 = 1;
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
			if (pPathFinderClient->IsValidPosition(&this->cinematicCachedLocation) != false) {
				this->field_0x504 = 0;
				SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 0x14, -1);
				return;
			}
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

	if (((GetStateFlags(this->actorState) & 0x40000) != 0) && (this->field_0x550 != 0)) {
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
	if (((GetStateFlags(this->actorState) & 0x20000) != 0) && (this->field_0x550 != 0)) {
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
			if (IsLookingAt() == 0) {
				SetLookingAtOn(0.2f);
			}

			SetLookingAtRotationHeight(1.5708f, &this->field_0x540);
			return;
		}
	}

	SetLookingAtOff();

	return;
}

void CActorNativ::BehaviourExorcisme_Manage()
{
	int iVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;

	iVar1 = this->actorState;
	if ((iVar1 != 0x12) && (iVar1 != 0x11)) {
		if (iVar1 == 0x10) {
			ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
			if ((this->flags & 0x4000) == 0) {
				this->field_0x53c = 1;
				this->flags = this->flags & 0xfffffffd;
				this->flags = this->flags | 1;
				this->flags = this->flags & 0xffffff7f;
				this->flags = this->flags | 0x20;
				EvaluateDisplayState();
				SetState(0, -1);
			}
		}
		else {
			if (iVar1 == 0xf) {
				ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
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
					SetState(0x10, -1);
				}
			}
			else {
				if (iVar1 == 0xe) {
					ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
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

					if ((bVar4) && (((this->pCollisionData)->flags_0x4 & 2) != 0)) {
						SetState(0xf, -1);
					}
				}
			}
		}
	}

	bVar4 = ColWithCactus();
	if (bVar4 != false) {
		this->field_0x53c = 1;
		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;
		EvaluateDisplayState();
		SetState(0, -1);
	}

	// Duplicated code is a mistake? 
	bVar4 = ColWithCactus();
	if (bVar4 != false) {
		this->field_0x53c = 1;
		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;
		EvaluateDisplayState();
		SetState(0, -1);
	}

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
				uVar1 = FUN_00162270();
				if (uVar1 != 0) {
					SetState(NATIVE_STATE_TAKE_PUT_TURN_TO, -1);
				}
			}
			else {
				if (newState == 0x17) {
					this->field_0x534 = 0;
					uVar1 = FUN_00162270();
					if (uVar1 != 0) {
						SetState(NATIVE_STATE_TAKE_PUT_TURN_TO, -1);
					}
				}
			}
		}
	}

	return;
}

void CActorNativ::BehaviourNativLive_Manage()
{
	int iVar1;

	iVar1 = this->actorState;
	if (iVar1 == 0x14) {
		IMPLEMENTATION_GUARD(
		ActorFunc_001617c0(this);)
	}
	else {
		if (iVar1 == 0x13) {
			if ((this->behaviourLive).field_0xc < this->timeInAir) {
				SetState(6, -1);
			}
		}
		else {
			if (iVar1 == 7) {
				IMPLEMENTATION_GUARD(
				StateNativWalk(this);)
			}
		}
	}

	return;
}


void CActorNativ::BehaviourNativLive_TermState(int oldState)
{
	if (oldState == 7) {
		SV_AUT_PathfindingEnd();
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
			bVar4 = this->behaviourTakeAndPut.GetCurrentPathFollowReader()->AtGoal(pCVar7->splinePointIndex, pCVar7->field_0xc);
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
				ChooseToolState();
			}
		}
		else {
			if (iVar1 == NATIVE_STATE_TAKE_PUT_USED_TOOL) {
				StateNativTakePutUsedTool(pBehaviour);
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
		SetHasObject(false);
		this->behaviourTakeAndPut.GetCurrentPathFollowReader()->Reset();
	}
	else {
		if (oldState == 0x18) {
			this->field_0x530 = 1;
		}
		else {
			if (oldState == NATIVE_STATE_TAKE_PUT_USED_TOOL) {
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

void CActorNativ::BehaviourNativAkasa_InitState(int newState, CBehaviourNativAkasa* pBehaviour)
{
	undefined4 local_20;
	undefined4 local_1c;
	undefined4 local_18;
	undefined4 local_14;
	undefined4* local_4;

	if (newState == 0x38) {
		pBehaviour->comboDesiredDisplayWidth = 0.0f;
		pBehaviour->comboDesiredDisplayHeight = 0.0f;
		pBehaviour->comboDesiredDisplayAlpha = 0.0f;
	}
	else {
		if (newState == 0x20) {
			CScene::ptable.g_FrontendManager_00451680->pMoney->field_0x74 = 0;
		}
		else {
			if ((newState != 0x39) && (newState == 0x26)) {
				IMPLEMENTATION_GUARD(
				local_4 = &local_20;
				local_1c = 1;
				local_18 = 1;
				local_14 = 1;
				local_20 = 0;
				CActor::DoMessage((CActor*)this, (CActor*)CActorHero::_gThis, 0x23, (uint)local_4);)
			}
		}
	}
	return;
}

void CActorNativ::BehaviourNativAkasa_Manage(CBehaviourNativAkasa* pBehaviour)
{
	uint** ppuVar1;
	int iVar2;
	int iVar3;

	switch (this->actorState) {
	case 0x20:
		CScene::ptable.g_FrontendManager_00451680->pMoney->field_0x74 = 0;
		break;
	case NATIVE_STATE_SELLER_INIT_ARENA_DISPLAY:
		StateInitArenaDisplay(pBehaviour);
		break;
	case 0x26:
		IMPLEMENTATION_GUARD(
		iVar3 = CBehaviourNativAkasa::FUN_003efdf0(pBehaviour);
		CBehaviourNativAkasa::FUN_003eef80(pBehaviour);
		FUN_003eed60();
		if (iVar3 == 1) {
			if (pBehaviour->field_0x8 == 0xffffffff) {
				SetState(0x20, -1);
				CBehaviourNativAkasa::FUN_003f1e90(pBehaviour, 0x28);
			}
			else {
				SetState(0x27, -1);
			}
		}
		else {
			if (iVar3 == 2) {
				SetState(0x1f, -1);
			}
		})
		break;
	case 0x27:
		IMPLEMENTATION_GUARD(
		ppuVar1 = CBehaviourNativAkasa::IsEventActive(pBehaviour);
		if (ppuVar1 != (uint**)0x0) {
			FUN_003ebee0(pBehaviour, 1);
			SetState(0x20, -1);
			CBehaviourNativAkasa::FUN_003f1e90(pBehaviour, 0x28);
			iVar3 = 0;
			if (0 < pBehaviour->field_0x48->entryCount) {
				iVar2 = 0;
				do {
					S_STREAM_NTF_TARGET_SWITCH::Switch
					((S_STREAM_NTF_TARGET_SWITCH*)((int)&pBehaviour->field_0x48->aEntries[0].base.pRef + iVar2),
						(CActor*)this);
					iVar3 = iVar3 + 1;
					iVar2 = iVar2 + 0x1c;
				} while (iVar3 < pBehaviour->field_0x48->entryCount);
			}
			S_STREAM_EVENT_CAMERA::SwitchOn(pBehaviour->streamEventCamera_0x4c, (CActor*)this);
		})
	}

	return;
}

void CActorNativ::BehaviourNativAkasa_TermState(int oldState)
{
	if (oldState == 0x20) {
		(CScene::ptable.g_FrontendManager_00451680)->pMoney->field_0x74 = 1;
	}
	else {
		if (oldState == 0x26) {
			DoMessage(CActorHero::_gThis, MESSAGE_LEAVE_SHOP, 0);
		}
	}

	return;
}

void CActorNativ::BehaviourNativSeller_InitState(int newState)
{
	if (newState == 0x1a) {
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;
		EvaluateDisplayState();
	}

	return;
}

void CActorNativ::BehaviourNativSeller_Manage(CBehaviourNativSeller* pBehaviour)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	uint zoneId;
	bool bVar3;
	float fVar4;
	CActorHero* pCVar5;
	CEventManager* pCVar6;
	ed_zone_3d* pZone;
	int iVar7;
	long lVar8;
	float fVar9;
	float fVar10;

	iVar7 = this->actorState;
	if (iVar7 == 0x1e) {
		fVar10 = pBehaviour->field_0x8;
		fVar9 = pBehaviour->GetWayPointDistance();
		if (fVar9 < fVar10) {
			SetState(0x1b, -1);
		}
		else {
			pCVar1 = this->pAnimationController;
			peVar2 = pCVar1->anmBinMetaAnimator.aAnimData;
			if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
				bVar3 = false;
				if (peVar2->animPlayState != 0) {
					bVar3 = (peVar2->field_0xcc & 2) != 0;
				}
			}
			else {
				bVar3 = false;
			}
			if (bVar3) {
				SetState(0x1a, -1);
			}
		}
	}
	else {
		if (iVar7 == 0x1d) {
			if ((20.0f < this->timeInAir) && (pBehaviour->addOn.Func_0x20(3, (CActor*)0x0, 0) != 0)) {
				pBehaviour->cachedStateId = 0x1d;
			}
		}
		else {
			if (iVar7 == 0x1c) {
				StateNativSellerStandCome(pBehaviour);
			}
			else {
				if (iVar7 == 0x1b) {
					fVar10 = pBehaviour->field_0x8;
					fVar9 = pBehaviour->GetWayPointDistance();
					if (fVar10 * 1.1f < fVar9) {
						SetState(0x1e, -1);
					}
					else {
						pCVar1 = this->pAnimationController;
						peVar2 = pCVar1->anmBinMetaAnimator.aAnimData;
						if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
							bVar3 = false;
							if (peVar2->animPlayState != 0) {
								bVar3 = (peVar2->field_0xcc & 2) != 0;
							}
						}
						else {
							bVar3 = false;
						}

						if (bVar3) {
							if (pBehaviour->addOn.Func_0x20(1, (CActor*)0x0, 0) == 0) {
								SetState(0x1c, -1);
							}
							else {
								pBehaviour->cachedStateId = 0x1c;
							}
						}
					}
				}
				else {
					if ((iVar7 == 0x1a) && (fVar10 = pBehaviour->field_0x8, fVar9 = pBehaviour->GetWayPointDistance(), fVar9 < fVar10))
					{
						SetState(0x1b, -1);
					}
				}
			}
		}
	}
	
	pBehaviour->addOn.Manage();

	pCVar6 = CScene::ptable.g_EventManager_006f5080;
	pCVar5 = CActorHero::_gThis;
	zoneId = this->field_0x380;

	if (zoneId == 0xffffffff) {
		return;
	}

	if (CActorHero::_gThis != (CActorHero*)0x0) {
		pZone = (ed_zone_3d*)0x0;
		if (zoneId != 0xffffffff) {
			pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, zoneId);
		}
		if (pZone == (ed_zone_3d*)0x0) {
			fVar9 = pCVar5->currentLocation.x - this->currentLocation.x;
			fVar10 = pCVar5->currentLocation.y - this->currentLocation.y;
			fVar4 = pCVar5->currentLocation.z - this->currentLocation.z;
			if (sqrtf(fVar9 * fVar9 + fVar10 * fVar10 + fVar4 * fVar4) < 5.0f) {
				bVar3 = true;
				goto LAB_0015e1f0;
			}
		}
		else {
			iVar7 = edEventComputeZoneAgainstVertex(pCVar6->activeChunkId, pZone, &pCVar5->currentLocation, 0);
			bVar3 = true;

			if (iVar7 == 1) goto LAB_0015e1f0;
		}
	}

	bVar3 = false;

LAB_0015e1f0:
	if (bVar3) {
		pBehaviour->FUN_003f3070();
	}
	else {
		pBehaviour->FUN_003f3020();
	}

	return;
}

void CActorNativ::BehaviourNativSeller_TermState(int oldState)
{
	if (oldState == 0x1a) {
		this->flags = this->flags & 0xffffff5f;
		EvaluateDisplayState();
	}

	return;
}

void CActorNativ::BehaviourNativSpeak_InitState(int newState)
{
	if (newState != 10) {
		if (newState == NATIVE_STATE_SPEAK_FACE_TOWARDS) {
			this->field_0x504 = 1;
		}
		else {
			if (newState == 0xb) {
				this->field_0x4f0 = 0;
				this->field_0x530 = 0;
			}
		}
	}
	return;
}

void CActorNativ::BehaviourNativSpeak_Manage(CBehaviourNativSpeak* pBehaviour)
{
	float fVar2;
	CActorNativMsgParam_0xe local_90;
	CActorNativMsgParam_0xe local_70;
	CActorNativMsgParam_0xe local_50;
	CActorNativMsgParam_0xe local_30;


	switch (this->actorState) {
	case 0xb:
		IMPLEMENTATION_GUARD(
		ActorInStateFunc_001619e0((Actor*)this, (int)pBehaviour);)
		break;
	case NATIVE_STATE_SPEAK_FACE_TOWARDS:
		fVar2 = this->field_0x508 - GetTimer()->cutsceneDeltaTime;
		this->field_0x508 = fVar2;
		if (fVar2 < 0.0f) {
			this->field_0x504 = 0;
			local_30.field_0x8 = pBehaviour->field_0x24;
			local_30.field_0x18 = &pBehaviour->field_0x10;
			if (local_30.field_0x8 == -1) {
				local_50.type = 3;
				DoMessage(this->field_0x3f0, MESSAGE_NATIV_CMD, &local_50);
			}
			else {
				local_30.type = 8;
				DoMessage(this->field_0x3f0, MESSAGE_NATIV_CMD, &local_30);
			}
		}
		break;
	case NATIVE_STATE_SPEAK_FACE_AWAY:
		fVar2 = this->field_0x508 - GetTimer()->cutsceneDeltaTime;
		this->field_0x508 = fVar2;
		if (fVar2 < 0.0f) {
			this->field_0x504 = 0;
			local_70.field_0x8 = pBehaviour->field_0x24;
			local_70.field_0x18 = &pBehaviour->field_0x10;

			if (local_70.field_0x8 == -1) {
				local_90.type = 3;
				DoMessage(this->field_0x3f0, MESSAGE_NATIV_CMD, &local_90);
			}
			else {
				local_70.type = 8;
				DoMessage(this->field_0x3f0, MESSAGE_NATIV_CMD, &local_70);
			}
		}
	}

	return;
}

void CActorNativ::BehaviourNativSpeak_TermState(int oldState)
{
	if (oldState == 0xb) {
		SV_AUT_PathfindingEnd();
	}
	else {
		if (oldState == 9) {
			this->field_0x4f0 = 1;
		}
	}

	return;
}

void CActorNativ::StateNativTakePutTurnTo()
{
	edF32VECTOR4* rotEuler = this->behaviourTakeAndPut.GetCurrentPathFollowReader()->GetWayPointAngles();
	edF32VECTOR4 eStack16;
	SetVectorFromAngles(&eStack16, &rotEuler->xyz);
	bool bVar4 = CActor::SV_UpdateOrientation2D(2.0f, &eStack16, 0);

	ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);

	if (bVar4 == true) {
		if ((this->field_0x370 - 5 < 2) || (this->field_0x370 == 7)) {
			bVar4 = true;
		}
		else {
			bVar4 = false;
		}

		if (bVar4) {
			SetState(NATIVE_STATE_TAKE_PUT_USED_TOOL, 0x16);
		}
		else {
			ChooseToolState();
		}
	}

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

	bVar3 = false;

	this->curWayPointLocation = *pBehaviour->GetCurrentPathFollowReader()->GetWayPoint();

	pPathFinderClient = GetPathfinderClient();
	bVar2 = pBehaviour->GetCurrentTrajectoryParam()->IsWayPointValidPosition(pPathFinderClient);

	if (bVar2 != false) {
		if (pBehaviour->GetCurrentPathFollowReader()->field_0xc != 0) {
			if (pBehaviour->GetCurrentPathFollowReader()->splinePointIndex == 0) {
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
		pPathFollowReader = pBehaviour->GetCurrentPathFollowReader();

		bVar3 = pPathFollowReader->AtGoal(pPathFollowReader->splinePointIndex, pPathFollowReader->field_0xc);
		if (bVar3 == false) {
			if (pBehaviour->GetCurrentPathFollowReader()->splinePointIndex == pBehaviour->GetCurrentPathFollowReader()->pPathFollow->splinePointCount + -1) {
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
						SetState(NATIVE_STATE_TAKE_PUT_USED_TOOL, 0x16);
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
				if ((pBehaviour->GetCurrentPathFollowReader()->splinePointIndex == 0) && (this->bHasObject != 0)) {
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

						SetState(NATIVE_STATE_TAKE_PUT_USED_TOOL, 0x16);
						return;
					}

					SetHasObject(false);
				}

				pBehaviour->GetCurrentPathFollowReader()->NextWayPoint();
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


void CActorNativ::StateNativTakePutUsedTool(CBehaviourNativTakeAndPut* pBehaviour)
{
	int iVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	edF32VECTOR4* rotEuler;
	edF32VECTOR4 eStack16;

	rotEuler = pBehaviour->GetCurrentPathFollowReader()->GetWayPointAngles();
	SetVectorFromAngles(&eStack16, &rotEuler->xyz);
	SV_UpdateOrientation2D(1.0f, &eStack16, 0);

	ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
	this->field_0x408 = this->field_0x408 + GetTimer()->cutsceneDeltaTime;

	if ((this->field_0x370 - 5 < 2) || (this->field_0x370 == 7)) {
		bVar4 = true;
	}
	else {
		bVar4 = false;
	}

	if (bVar4) {
		if (this->bHasObject == 0) {
			if ((this->pAnimationController->anmBinMetaAnimator.aAnimData)->animPlayState == 1) {
				SetHasObject(true);
			}
		}
		else {
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
				pBehaviour->GetCurrentPathFollowReader()->NextWayPoint();
				SetState(NATIVE_STATE_TAKE_PUT_WALK, 0x17);
			}
		}
	}
	else {
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

		if (2 < this->field_0x534) {
			if (pBehaviour->GetCurrentTrajectoryParam()->field_0x18 < this->field_0x408) {
				pBehaviour->GetCurrentPathFollowReader()->NextWayPoint();

				SetHasObject(true);
				SetState(NATIVE_STATE_TAKE_PUT_WALK, -1);
			}
			else {
				ChooseToolState();
			}
		}
	}

	return;
}

void CActorNativ::StateNativSellerStandCome(CBehaviourNativSeller* pBehaviour)
{
	CActor* pActor;
	float fVar1;
	float fVar2;
	float fVar3;
	CActorHero* pCVar4;
	bool bVar5;
	CBehaviourNativShopSell* pCVar6;
	edF32VECTOR4* peVar7;
	float in_f0;
	edF32VECTOR4 local_10;

	pCVar4 = CActorHero::_gThis;
	pActor = pBehaviour->pNativShop;
	if (pActor == (CActor*)0x0) {
		fVar1 = CActorHero::_gThis->currentLocation.x - this->currentLocation.x;
		fVar2 = CActorHero::_gThis->currentLocation.y - this->currentLocation.y;
		fVar3 = CActorHero::_gThis->currentLocation.z - this->currentLocation.z;
		in_f0 = sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3);
	}
	else {
		if (pActor->curBehaviourId == 2) {
			pCVar6 = static_cast<CBehaviourNativShopSell*>(pActor->GetBehaviour(pActor->curBehaviourId));
			local_10 = *pCVar6->pathFollowReader.GetWayPoint(0);
			edF32Vector4SubHard(&local_10, &local_10, &pCVar4->currentLocation);
			local_10.y = 0.0f;
			in_f0 = edF32Vector4GetDistHard(&local_10);
		}
	}

	if (pBehaviour->field_0x8 * 1.1f < in_f0) {
		SetState(0x1e, -1);
	}
	else {
		if ((20.0f < this->timeInAir) && (((pActor == (CActor*)0x0 || (pActor->actorState != 7)) &&
				(bVar5 = pBehaviour->addOn.Func_0x20(2, (CActor*)0x0, 0), bVar5 != false)))) {
			pBehaviour->cachedStateId = 0x1c;
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
	this->field_0x534 = 0;
	switch (this->field_0x370) {
	case 1:
		SetState(NATIVE_STATE_TAKE_PUT_USED_TOOL, -1);
		break;
	case 2:
		if (this->actorState == NATIVE_STATE_TAKE_PUT_USED_TOOL) {
			SetState(0x18, 0x14);
		}
		else {
			SetState(NATIVE_STATE_TAKE_PUT_USED_TOOL, ((int)(CScene::Rand() * 2) >> 0xf) + 0x12);
		}
		break;
	case 3:
		SetState(NATIVE_STATE_TAKE_PUT_USED_TOOL, ((int)(CScene::Rand() * 2) >> 0xf) + 0x12);
		break;
	case 4:
		SetState(NATIVE_STATE_TAKE_PUT_USED_TOOL, 0x12);
		break;
	case 5:
	case 6:
	case 7:
		SetState(NATIVE_STATE_TAKE_PUT_USED_TOOL, 0);
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

	rotEuler = this->behaviourTakeAndPut.GetCurrentPathFollowReader()->GetWayPointAngles();
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


int CActorNativ::FUN_00162a70()
{
	uint uVar1;
	bool bVar2;
	int iVar3;

	switch (this->field_0x378) {
	case 0:
	case 1:
		uVar1 = this->field_0x37c;
		if (uVar1 == 3) {
			uVar1 = this->field_0x370;
			switch (uVar1) {
			case 1:
				return 4;
			case 2:
			case 3:
			case 4:
				return 8;
			case 5:
				if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
					bVar2 = true;
				}
				else {
					bVar2 = false;
				}
				if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
					bVar2 = false;
				}
				if (!bVar2) {
					return 4;
				}
				return 9;
			case 6:
				if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
					bVar2 = true;
				}
				else {
					bVar2 = false;
				}
				if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
					bVar2 = false;
				}
				if (!bVar2) {
					return 4;
				}
				return 10;
			case 7:
				if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
					bVar2 = true;
				}
				else {
					bVar2 = false;
				}
				if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
					bVar2 = false;
				}
				if (!bVar2) {
					return 4;
				}
				return 0xb;
			}
		}
		else {
			if (uVar1 == 2) {
				uVar1 = this->field_0x370;
				switch (uVar1) {
				case 1:
					return 3;
				case 2:
				case 3:
				case 4:
					return 7;
				case 5:
					if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
						bVar2 = true;
					}
					else {
						bVar2 = false;
					}
					if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
						bVar2 = false;
					}
					if (!bVar2) {
						return 3;
					}
					return 9;
				case 6:
					if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
						bVar2 = true;
					}
					else {
						bVar2 = false;
					}
					if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
						bVar2 = false;
					}
					if (!bVar2) {
						return 3;
					}
					return 10;
				case 7:
					if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
						bVar2 = true;
					}
					else {
						bVar2 = false;
					}
					if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
						bVar2 = false;
					}
					if (!bVar2) {
						return 3;
					}
					return 0xb;
				}
			}
			else {
				if (uVar1 == 1) {
					uVar1 = this->field_0x370;
					switch (uVar1) {
					case 1:
						return 2;
					case 2:
					case 3:
					case 4:
						return 6;
					case 5:
						if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
							bVar2 = true;
						}
						else {
							bVar2 = false;
						}
						if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
							bVar2 = false;
						}
						if (!bVar2) {
							return 2;
						}
						return 9;
					case 6:
						if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
							bVar2 = true;
						}
						else {
							bVar2 = false;
						}
						if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
							bVar2 = false;
						}
						if (!bVar2) {
							return 2;
						}
						return 10;
					case 7:
						if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
							bVar2 = true;
						}
						else {
							bVar2 = false;
						}
						if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
							bVar2 = false;
						}
						if (!bVar2) {
							return 2;
						}
						return 0xb;
					}
				}
				else {
					if (uVar1 == 0) {
						uVar1 = this->field_0x370;
						switch (uVar1) {
						case 1:
							return 1;
						case 2:
						case 3:
						case 4:
							return 5;
						case 5:
							if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
								bVar2 = true;
							}
							else {
								bVar2 = false;
							}
							if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
								bVar2 = false;
							}
							if (bVar2) {
								return 9;
							}
							return 1;
						case 6:
							if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
								bVar2 = true;
							}
							else {
								bVar2 = false;
							}
							if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
								bVar2 = false;
							}
							if (bVar2) {
								return 10;
							}
							return 1;
						case 7:
							if ((uVar1 - 5 < 2) || (uVar1 == 7)) {
								bVar2 = true;
							}
							else {
								bVar2 = false;
							}
							if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
								bVar2 = false;
							}
							if (bVar2) {
								return 0xb;
							}
							return 1;
						}
					}
				}
			}
		}
	default:
		iVar3 = 0;
		break;
	case 2:
		iVar3 = 0xc;
		break;
	case 3:
		iVar3 = 0xd;
		break;
	case 4:
		iVar3 = 0xe;
		break;
	case 5:
		iVar3 = 0xf;
		break;
	case 6:
		iVar3 = 0x10;
	}

	return iVar3;
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

bool CActorNativ::FUN_00162750()
{
	int iVar1;
	bool bVar2;
	CBehaviour* pCVar3;

	if (this->field_0x530 != 0) {
		if ((this->field_0x370 - 5 < 2) || (this->field_0x370 == 7)) {
			bVar2 = true;
		}
		else {
			bVar2 = false;
		}

		if ((!bVar2) || (bVar2 = true, this->bHasObject == 0)) {
			bVar2 = false;
		}

		if ((((!bVar2) && (pCVar3 = GetBehaviour(NATIVE_BEHAVIOUR_SPEAK), pCVar3 != (CBehaviour*)0x0)) &&
			((this->flags & 4) != 0)) && (((iVar1 = this->curBehaviourId, iVar1 == 3 || (iVar1 == 5)) || (iVar1 == 6)))) {
			return true;
		}
	}

	return false;
}



bool CActorNativ::CanSpeak()
{
	bool bVar1;

	if ((this->field_0x370 - 5 < 2) || (this->field_0x370 == 7)) {
		bVar1 = true;
	}
	else {
		bVar1 = false;
	}

	if ((!bVar1) || (bVar1 = true, this->bHasObject == 0)) {
		bVar1 = false;
	}

	return bVar1;
}

bool CBehaviourNativAkasa::AdvanceTutorial(int param_2)
{
	ArenaTutorial* pTutorial;
	int iVar2;
	int iVar3;

	if (param_2 == 0) {
		pTutorial = GetActiveComboTutorial();

		if (this->activeSubObjBIndex == pTutorial->nbRequiredMoves + -1) {
			this->activeSubObjBIndex = 0;
			return true;
		}

		iVar3 = (this->comboTutorialManager).activeTutorialIndex;

		if (iVar3 == -1) {
			pTutorial = (ArenaTutorial*)0x0;
		}
		else {
			pTutorial = (this->comboTutorialManager).aTutorials + iVar3;
		}

		if ((pTutorial->aRequiredMoves[this->activeSubObjBIndex].aRequiredCombos[0]->field_0x4.field_0x0ushort & 0x100U) != 0) {
			this->activeSubObjBIndex = this->activeSubObjBIndex + 1;
			AdvanceTutorial(0);
			return false;
		}
	}

	if (param_2 == 1) {
		IMPLEMENTATION_GUARD(
		iVar3 = *(int*)&this->field_0x1690;
		iVar2 = 0;
		if (iVar3 != 0) {
			iVar2 = iVar3;
		}

		if (this->activeSubObjBIndex == *(int*)(iVar2 + 0x80) + -1) {
			this->activeSubObjBIndex = 0;
			return true;
		}

		if (iVar3 == 0) {
			iVar3 = 0;
		}

		if ((*(ushort*)(*(int*)(iVar3 + this->activeSubObjBIndex * 0x10 + 4) + 4) & 0x100) != 0) {
			this->activeSubObjBIndex = this->activeSubObjBIndex + 1;
			AdvanceTutorial(1);
			return false;
		})
	}

	this->activeSubObjBIndex = this->activeSubObjBIndex + 1;
	return false;

}

int CBehaviourNativAkasa::FUN_003f1b90(int param_2)
{
	CActorNativ* pActor;
	bool bVar1;
	int iVar2;
	long lVar3;
	undefined8 uVar4;
	int iVar5;

	bVar1 = AdvanceTutorial(param_2);
	if (bVar1 == false) {
		iVar2 = 1;
	}
	else {
		if (this->field_0x16a4 != 0) {
			this->field_0x16a4 = 0;
		}
		if ((param_2 == 0) && (iVar2 = this->comboTutorialManager.StepRequiredCombo(), iVar2 == -1)) {
			bVar1 = true;
		}
		else {
			if ((param_2 == 1) && (this->field_0x60.Set_0x1630(this->field_0x16b0) == false)) {
				bVar1 = true;
			}
			else {
				this->comboDisplayAlpha = 0.0f;
				bVar1 = false;
			}
		}

		if (bVar1) {
			pActor = this->pOwner;
			iVar2 = 0;
			if (0 < this->field_0x40->entryCount) {
				do {
					this->field_0x40->aEntries[iVar2].Switch(pActor);
					iVar2 = iVar2 + 1;
				} while (iVar2 < this->field_0x40->entryCount);
			}

			this->streamEventCamera_0x44->SwitchOn(pActor);

			if (this->addOn.Func_0x20(5, (this->addOn).pOwner, 1) == 0) {
				iVar2 = 4;
			}
			else {
				SetBehaviourState(-1);
				this->initialAnimId = 0x39;
				iVar2 = 0;
			}
		}
		else {
			uVar4 = 0;
			if (this->field_0x16ac == 2) {
				uVar4 = 2;
			}

			if (this->field_0x16ac == 1) {
				uVar4 = 1;
			}

			this->field_0x16ac = 0;

			if (this->addOn.Func_0x20(uVar4, (this->addOn).pOwner, 0) != 0) {
				this->initialAnimId = 0x20;
			}

			ArenaUpdateDisplayBorderSize();
			iVar2 = 2;
		}
	}

	return iVar2;
}

void CBehaviourNativAkasa::FUN_003f1810(int param_2, int param_3, int param_4)
{
	CActorHero* pHero;

	pHero = CActorHero::_gThis;
	if (param_2 == 4) {
		SetBehaviourState(-1);

		if (param_3 == 0) {
			this->pOwner->SetBehaviour(8, 0x39, -1);
		}
		else {
			this->pOwner->SetBehaviour(8, 0x1f, -1);
		}
	}
	else {
		if (param_2 == 3) {
			SetBehaviourState(NATIVE_STATE_SELLER_INPUT_FAILED);
		}
		else {
			if (param_2 == 2) {
				SetBehaviourState(0x38);
			}
			else {
				if (param_2 == 1) {
					if (param_4 == 2) {
						SetBehaviourState(0x32);
					}
					else {
						if (param_4 == 1) {
							SetBehaviourState(0x2f);
						}
						else {
							if (param_4 == 0) {
								if (param_3 == 0) {
									if (CActorHero::_gThis->pFighterCombo == (s_fighter_combo*)0x0) {
										SetBehaviourState(0x25);
									}
									else {
										SetBehaviourState(0x22);
									}
								}
								if (param_3 == 1) {
									if (pHero->pFighterCombo == (s_fighter_combo*)0x0) {
										SetBehaviourState(0x2b);
									}
									else {
										SetBehaviourState(0x28);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

void CActorNativ::StateInitArenaDisplay(CBehaviourNativAkasa* pBehaviour)
{
	int curSwitchIndex;

	if (pBehaviour->IsEventActive() != false) {
		pBehaviour->FUN_003ebee0(0);
		SetState(0x20, -1);
		pBehaviour->SetBehaviourState(0x22);

		curSwitchIndex = 0;
		if (0 < pBehaviour->field_0x48->entryCount) {
			do {
				pBehaviour->field_0x48->aEntries[curSwitchIndex].Switch(this);
				curSwitchIndex = curSwitchIndex + 1;
			} while (curSwitchIndex < pBehaviour->field_0x48->entryCount);
		}

		pBehaviour->streamEventCamera_0x4c->SwitchOn(this);
	}

	return;
}

void CActorNativ::State_0x22(CBehaviourNativAkasa* pBehaviour)
{
	s_fighter_combo* pCombo;
	bool bVar3;
	ArenaTutorial* pSubObjA;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;

	pSubObjA = pBehaviour->GetActiveComboTutorial();

	if (((pBehaviour->field_0x16ac != 2) &&
		(bVar3 = CActorHero::_gThis->FUN_0031b790(CActorHero::_gThis->actorState), bVar3 != false)) && (pCombo = pHero->pFighterCombo, pCombo != (s_fighter_combo*)0x0)) {
		pBehaviour->FUN_003f1da0(pCombo);
	}

	bVar3 = pHero->FUN_0031b790(pHero->actorState);
	if (bVar3 != false) {
		if (((pBehaviour->field_0x16ac != 2) &&
			(bVar3 = pHero->FUN_0031b790(pHero->actorState), bVar3 != false)) && (pCombo = pHero->pFighterCombo, pCombo != (s_fighter_combo*)0x0)) {
			pBehaviour->FUN_003f1da0(pCombo);
		}

		if ((pSubObjA != (ArenaTutorial*)0x0) && (pCombo = pHero->pFighterCombo, pCombo != (s_fighter_combo*)0x0)) {
			bVar3 = (pSubObjA->aRequiredMoves + pBehaviour->activeSubObjBIndex)->IsRequiredCombo(pCombo);
			if (bVar3 == false) {
				pBehaviour->FUN_003f1810(3, 0, 0);
			}
			else {
				if (((pHero->fightFlags & 0x40) == 0) &&
					(((pHero->pFighterCombo)->field_0x4.field_0x0ushort & 0x100U) == 0)) {
					pBehaviour->SetBehaviourState(0x23);
				}
				else {
					pBehaviour->SetBehaviourState(NATIVE_STATE_SELLER_INPUT_SUCCEEDED);
				}
			}
		}
	}

	return;
}

void CActorNativ::State_0x23(CBehaviourNativAkasa* pBehaviour)
{
	s_fighter_combo* pCombo;
	ArenaTutorial* pTutorial;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;
	pTutorial = pBehaviour->GetActiveComboTutorial();

	if (CActorHero::_gThis->FUN_0031b790(CActorHero::_gThis->actorState) == false) {
		pBehaviour->FUN_003ebd90();
	}
	else {
		if ((pTutorial != (ArenaTutorial*)0x0) && (pCombo = pHero->pFighterCombo, pCombo != (s_fighter_combo*)0x0)) {
			if ((pTutorial->aRequiredMoves + pBehaviour->activeSubObjBIndex)->IsRequiredCombo(pCombo) == false) {
				pBehaviour->SetBehaviourState(NATIVE_STATE_SELLER_INPUT_FAILED);
			}
			else {
				if ((pHero->fightFlags & 0x40) != 0) {
					pBehaviour->SetBehaviourState(NATIVE_STATE_SELLER_INPUT_SUCCEEDED);
				}
			}
		}
	}

	return;
}

void CActorNativ::StateInputSucceeded(CBehaviourNativAkasa* pBehaviour)
{
	ArenaTutorial* pTutorial;

	pTutorial = pBehaviour->GetActiveComboTutorial();

	if (pTutorial->aRequiredMoves[pBehaviour->activeSubObjBIndex].IsRequiredCombo(CActorHero::_gThis->pFighterCombo) == false) {
		pBehaviour->FUN_003f1810(pBehaviour->FUN_003f1b90(0), 0, 0);
	}

	return;
}

void CActorNativ::State_0x25(CBehaviourNativAkasa* pBehaviour)
{
	s_fighter_combo* pCombo;
	bool bVar1;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;

	if (((pBehaviour->field_0x16ac != 2) && (CActorHero::_gThis->FUN_0031b790(CActorHero::_gThis->actorState) != false)) &&
		(pCombo = pHero->pFighterCombo, pCombo != (s_fighter_combo*)0x0)) {
		pBehaviour->FUN_003f1da0(pCombo);
	}

	bVar1 = pHero->FUN_0031b790(pHero->actorState);
	if (bVar1 == false) {
		pBehaviour->field_0x16a4 = 0;
		(pBehaviour->field_0x60).activeSubObjIndex = 0;
		pBehaviour->SetBehaviourState(0x22);
	}

	return;
}

void CActorNativ::StateInputFailed(CBehaviourNativAkasa* pBehaviour)
{
	s_fighter_combo* pCombo;
	int iVar1;
	bool bVar2;
	StateConfig* pSVar3;
	uint uVar4;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;
	if (((pBehaviour->field_0x16ac != 2) && (bVar2 = CActorHero::_gThis->FUN_0031b790(CActorHero::_gThis->actorState), bVar2 != false)) &&
		(pCombo = pHero->pFighterCombo, pCombo != (s_fighter_combo*)0x0)) {
		pBehaviour->FUN_003f1da0(pCombo);
	}

	bVar2 = pHero->FUN_0031b790(pHero->actorState);
	if (((bVar2 == false) && (iVar1 = pHero->actorState, iVar1 != 0x68)) && ((iVar1 != 0x69 && (iVar1 != NATIVE_STATE_SELLER_INPUT_FAILED)))) {
		uVar4 = GetStateFlags(iVar1);

		if ((uVar4 & 0x8000) == 0) {
			uVar4 = GetStateFlags(iVar1) & 0x4000;

			if (((uVar4 == 0) && ((iVar1 = pHero->actorState, iVar1 != 10 || (pHero->prevActorState != 0x20)))) && (iVar1 != 0x23)) {
				pBehaviour->activeSubObjBIndex = 0;
				pBehaviour->FUN_003ebd90();
			}
		}
	}

	return;
}

void CActorNativ::State_0x38(CBehaviourNativAkasa* pBehaviour)
{
	s_fighter_combo* pCombo;
	bool bVar1;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;

	if (((pBehaviour->field_0x16ac != 2) && (bVar1 = CActorHero::_gThis->FUN_0031b790(CActorHero::_gThis->actorState), bVar1 != false)) &&
		(pCombo = pHero->pFighterCombo, pCombo != (s_fighter_combo*)0x0)) {
		pBehaviour->FUN_003f1da0(pCombo);
	}

	if (0.8f < pBehaviour->field_0x16c0) {
		pBehaviour->ArenaUpdateDisplayBorderSize();

		if (pBehaviour->field_0x16b4 == 0) {
			pBehaviour->SetBehaviourState(0x22);
		}
		else {
			if (pBehaviour->field_0x16b4 == 1) {
				pBehaviour->SetBehaviourState(0x28);
			}
		}
	}

	return;
}

bool CActorNativ::InZone_00162970(CActor* pActor)
{
	float fVar1;
	float fVar2;
	float fVar3;
	CEventManager* pCVar4;
	ed_zone_3d* pZone;
	int iVar5;

	pCVar4 = CScene::ptable.g_EventManager_006f5080;
	if (pActor != (CActor*)0x0) {
		pZone = (ed_zone_3d*)0x0;
		if (this->field_0x380 != 0xffffffff) {
			pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->field_0x380);
		}
		if (pZone == (ed_zone_3d*)0x0) {
			fVar1 = (pActor->currentLocation).x - this->currentLocation.x;
			fVar2 = (pActor->currentLocation).y - this->currentLocation.y;
			fVar3 = (pActor->currentLocation).z - this->currentLocation.z;
			if (sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) < 5.0f) {
				return true;
			}
		}
		else {
			iVar5 = edEventComputeZoneAgainstVertex(pCVar4->activeChunkId, pZone, &pActor->currentLocation, 0);
			if (iVar5 == 1) {
				return true;
			}
		}
	}

	return false;
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

	this->curTrajectoryParamIndex = -1;
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
	CActorNativ* pNativ;

	this->pOwner = reinterpret_cast<CActorNativ*>(pOwner);

	if (newState == -1) {
		pNativ = this->pOwner;
		if ((pNativ->cinematicCachedState == -1) || (pNativ->cinematicCachedBehaviour == -1)) {
			if (this->GetCurrentPathFollowReader() == (CPathFollowReader*)0x0) {
				pNativ->SetState(6, -1);
			}
			else {
				if (this->GetCurrentPathFollowReader()->splinePointIndex != 0) {
					this->GetCurrentPathFollowReader()->SetToClosestSplinePoint(&pNativ->currentLocation);
				}

				this->pOwner->SetState(NATIVE_STATE_TAKE_PUT_WALK, -1);
			}
		}
		else {
			pNativ->ChooseBehaviour();
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
	CActorNativ* pNativ;
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
		CActorNativMsgParam_0xe* pParams = reinterpret_cast<CActorNativMsgParam_0xe*>(pMsgParam);
		iVar7 = pParams->type;
		if (iVar7 == NATIVE_MESSAGE_FACE_AWAY) {
			bVar4 = this->pOwner->CanSpeak();
			if (bVar4 == false) {
				pNativ = this->pOwner;
				pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_AWAY, -1);
			}
			else {
				pNativ = this->pOwner;
				pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_AWAY, 0x15);
			}
		}
		else {
			if ((iVar7 == NATIVE_MESSAGE_FACE_TOWARDS) || (iVar7 == 7)) {
				bVar4 = this->pOwner->CanSpeak();
				if (bVar4 == false) {
					pNativ = this->pOwner;
					pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_TOWARDS, -1);
				}
				else {
					pNativ = this->pOwner;
					pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_TOWARDS, 0x15);
				}
			}
			else {
				if (iVar7 == 0xd) {
					CActorNativMsgParam_0xe* pMsgParam_0xe = reinterpret_cast<CActorNativMsgParam_0xe*>(pMsgParam);
					pNativ = this->pOwner;
					pNativ->field_0x4f8 = pNativ->curBehaviourId;
					pNativ = this->pOwner;
					pNativ->field_0x4fc = pNativ->actorState;
					pNativ = this->pOwner;
					pNativ->field_0x500 = pNativ->currentAnimType;
					pNativ = this->pOwner;

					pNativ->behaviourSpeak.field_0x10 = *pMsgParam_0xe->field_0x18;
					
					((this->pOwner)->behaviourSpeak).field_0x24 = pMsgParam_0xe->field_0x8;
					((this->pOwner)->behaviourSpeak).field_0x28 = pMsgParam_0xe->field_0x14;
					pNativ = this->pOwner;
					pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, 9, pMsgParam_0xe->field_0xc + 0xd);
					(this->pOwner)->field_0x530 = 0;
				}
				else {
					if (iVar7 == 6) {
						IMPLEMENTATION_GUARD(
						*(undefined4*)&((this->pOwner)->behaviourSpeak).field_0x20 = *(undefined4*)((int)pMsgParam + 4);
						pNativ = this->pOwner;
						puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
						uVar15 = puVar3[1];
						uVar11 = puVar3[2];
						uVar13 = puVar3[3];
						*(undefined4*)&(pNativ->behaviourSpeak).field_0x10 = *puVar3;
						*(undefined4*)&(pNativ->behaviourSpeak).field_0x14 = uVar15;
						*(undefined4*)&(pNativ->behaviourSpeak).field_0x18 = uVar11;
						*(undefined4*)&(pNativ->behaviourSpeak).field_0x1c = uVar13;
						((this->pOwner)->behaviourSpeak).field_0x24 = *(undefined4*)((int)pMsgParam + 8);
						((this->pOwner)->behaviourSpeak).field_0x28 = *(undefined**)((int)pMsgParam + 0x14);
						pNativ = this->pOwner;
						(pNativ->field_0x570).x = pNativ->currentLocation.x;
						(pNativ->field_0x570).y = pNativ->currentLocation.y;
						(pNativ->field_0x570).z = pNativ->currentLocation.z;
						(pNativ->field_0x570).w = pNativ->currentLocation.w;
						pNativ = this->pOwner;
						pNativ->SetBehaviour(3, 10, -1);)
					}
					else {
						if (iVar7 == 9) {
							IMPLEMENTATION_GUARD(
							*(undefined4*)&((this->pOwner)->behaviourSpeak).field_0x20 = *(undefined4*)((int)pMsgParam + 4);
							pNativ = this->pOwner;
							puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
							uVar15 = puVar3[1];
							uVar11 = puVar3[2];
							uVar13 = puVar3[3];
							*(undefined4*)&(pNativ->behaviourSpeak).field_0x10 = *puVar3;
							*(undefined4*)&(pNativ->behaviourSpeak).field_0x14 = uVar15;
							*(undefined4*)&(pNativ->behaviourSpeak).field_0x18 = uVar11;
							*(undefined4*)&(pNativ->behaviourSpeak).field_0x1c = uVar13;
							((this->pOwner)->behaviourSpeak).field_0x24 = *(undefined4*)((int)pMsgParam + 8);
							((this->pOwner)->behaviourSpeak).field_0x28 = *(undefined**)((int)pMsgParam + 0x14);
							pNativ = this->pOwner;
							(*(pNativ->pVTable)->SetBehaviour)
								((CActor*)pNativ, 3, 9, *(int*)((int)pMsgParam + 0xc) + 0xd);)
						}
						else {
							if (iVar7 == 5) {
								IMPLEMENTATION_GUARD(
								*(undefined4*)&((this->pOwner)->behaviourSpeak).field_0x20 = *(undefined4*)((int)pMsgParam + 4);
								pNativ = this->pOwner;
								puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
								uVar15 = puVar3[1];
								uVar11 = puVar3[2];
								uVar13 = puVar3[3];
								*(undefined4*)&(pNativ->behaviourSpeak).field_0x10 = *puVar3;
								*(undefined4*)&(pNativ->behaviourSpeak).field_0x14 = uVar15;
								*(undefined4*)&(pNativ->behaviourSpeak).field_0x18 = uVar11;
								*(undefined4*)&(pNativ->behaviourSpeak).field_0x1c = uVar13;
								((this->pOwner)->behaviourSpeak).field_0x24 = *(undefined4*)((int)pMsgParam + 8);
								((this->pOwner)->behaviourSpeak).field_0x28 = *(undefined**)((int)pMsgParam + 0x14);
								pNativ = this->pOwner;
								(pNativ->field_0x570).x = pNativ->currentLocation.x;
								(pNativ->field_0x570).y = pNativ->currentLocation.y;
								(pNativ->field_0x570).z = pNativ->currentLocation.z;
								(pNativ->field_0x570).w = pNativ->currentLocation.w;
								pNativ = this->pOwner;
								pNativ->SetBehaviour(3, 0xb, -1);
								pNativ = this->pOwner;
								(pNativ->field_0x570).x = pNativ->currentLocation.x;
								(pNativ->field_0x570).y = pNativ->currentLocation.y;
								(pNativ->field_0x570).z = pNativ->currentLocation.z;
								(pNativ->field_0x570).w = pNativ->currentLocation.w;)
							}
							else {
								if (iVar7 == 4) {
									this->curTrajectoryParamIndex = pParams->field_0x4;
									pNativ = this->pOwner;

									pNativ->behaviourTakeAndPut.GetCurrentPathFollowReader()->Reset();
									pNativ = this->pOwner;
									pNativ->SetState(NATIVE_STATE_TAKE_PUT_WALK, -1);
								}
								else {
									if (iVar7 == 3) {
										IMPLEMENTATION_GUARD(
										((this->pOwner)->behaviourSpeak).field_0x24 = *(undefined4*)((int)pMsgParam + 8);
										pNativ = this->pOwner;
										if ((pNativ->field_0x4f8 == -1) && (pNativ->field_0x4fc == -1)) {
											pNativ->SetBehaviour(5, 6, -1);
										}
										else {
											(*(pNativ->pVTable)->SetBehaviour)
												((CActor*)pNativ, pNativ->field_0x4f8, pNativ->field_0x4fc, pNativ->field_0x500);
											(this->pOwner)->field_0x4f8 = -1;
											(this->pOwner)->field_0x4fc = -1;
											(this->pOwner)->field_0x500 = -1;
										})
									}
									else {
										if (iVar7 == 2) {
											this->curTrajectoryParamIndex = -1;
											pNativ = this->pOwner;
											pNativ->SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 0x13, -1);
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
												pNativ = this->pOwner;
												pNativ->SetBehaviour(9, 0x3a, -1);)
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
			pNativ = this->pOwner;
			local_4 = local_20;
			iVar7 = CActor::DoMessage((CActor*)pNativ, (CActor*)pNativ->field_0x3f0, 0x4e, (uint)local_4);
			if (iVar7 != 0) {
				pNativ = this->pOwner;
				pNativ->cinematicCachedBehaviour = pNativ->curBehaviourId;
				pNativ = this->pOwner;
				pNativ->cinematicCachedState = pNativ->actorState;
				pNativ = this->pOwner;
				pNativ->cinematicCachedAnim = pNativ->currentAnimType;
				return 1;
			})
			return 0;
		}
		if (msg == MESSAGE_GET_ACTION) {
			this->pOwner->field_0x508 = 1.0f;
			pNativ = this->pOwner;
			uVar6 = pNativ->GetStateFlags(pNativ->actorState) & 0x4000;

			if (uVar6 == 0) {
				this->pOwner->field_0x4f8 = this->pOwner->curBehaviourId;
				this->pOwner->field_0x4fc = this->pOwner->actorState;
				this->pOwner->field_0x500 = this->pOwner->currentAnimType;
				this->pOwner->field_0x504 = 1;
			}

			if (this->pOwner->actorState == NATIVE_STATE_SPEAK_FACE_TOWARDS) {
				return 0xf;
			}

			return 0;
		}
	}

	return 0;
}

CTakePutTrajectoryParam* CBehaviourNativTakeAndPut::GetCurrentTrajectoryParam()
{
	if (this->curTrajectoryParamIndex == -1) {
		return (CTakePutTrajectoryParam*)0x0;
	}
	else {
		return this->aTrajectoryParams + this->curTrajectoryParamIndex;
	}
}

CPathFollowReader* CBehaviourNativTakeAndPut::GetCurrentPathFollowReader()
{
	if (GetCurrentTrajectoryParam() == (CTakePutTrajectoryParam*)0x0) {
		return (CPathFollowReader*)0x0;
	}
	else {
		return &GetCurrentTrajectoryParam()->pathFollowReader;
	}
}

CBehaviourNativSpeak::CBehaviourNativSpeak()
	: nbSubObjs(0), aSubObjs(0)
{

}

void CBehaviourNativSpeak::Create(ByteCode* pByteCode)
{
	uint count;
	int iVar1;
	int* pBase;
	SpeakSubObj* pSVar2;
	float* pfVar4;
	float fVar5;

	this->field_0x20 = 0;

	this->nbSubObjs = pByteCode->GetS32();

	count = this->nbSubObjs;
	if (count != 0) {
		this->aSubObjs = new SpeakSubObj[count];
		iVar1 = 0;
		if (0 < this->nbSubObjs) {
			do {
				this->aSubObjs[iVar1].field_0x0 = pByteCode->GetF32();
				this->aSubObjs[iVar1].field_0x4 = pByteCode->GetF32();
				iVar1 = iVar1 + 1;
			} while (iVar1 < this->nbSubObjs);
		}
	}

	return;
}

void CBehaviourNativSpeak::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorNativ*>(pOwner);
	this->field_0x24 = -1;
	this->field_0x28 = -1.0f;

	return;
}

void CBehaviourNativSpeak::Manage()
{
	this->pOwner->BehaviourNativSpeak_Manage(this);

	return;
}

void CBehaviourNativSpeak::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorNativ* pNativ;

	if (newState == -1) {
		pNativ = this->pOwner;

		if ((pNativ->cinematicCachedState == -1) || (pNativ->cinematicCachedBehaviour == -1)) {
			pNativ->SetState(6, -1);
		}
		else {
			pNativ->ChooseBehaviour();
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourNativSpeak::End(int newBehaviourId)
{
	return;
}

void CBehaviourNativSpeak::InitState(int newState)
{
	this->pOwner->BehaviourNativSpeak_InitState(newState);

	return;
}

void CBehaviourNativSpeak::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourNativSpeak_TermState(oldState);

	return;
}

int CBehaviourNativSpeak::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorNativ* pNativ;
	//CBehaviourFighterVTable* pCVar2;
	float* pfVar3;
	bool bVar4;
	StateConfig* pSVar5;
	uint uVar6;
	int iVar7;
	CBehaviour* pCVar8;
	CTakePutTrajectoryParam* pCVar9;
	CPathFollowReader* pPathFollowReader;
	//CBehaviourFighterVTable* pCVar10;
	float fVar11;
	//CBehaviourFighterVTable* pCVar12;
	float fVar13;
	//CBehaviourFighterVTable* pCVar14;
	float fVar15;
	CActorNativMsgParam_0xe local_20;
	undefined4* local_4;

	if (msg == 0x4e) {
		CActorNativMsgParam_0xe* pParams = reinterpret_cast<CActorNativMsgParam_0xe*>(pMsgParam);
		switch (pParams->type) {
		case 2:
			this->field_0x24 = -1;

			(this->field_0x10).x = 0.0f;
			(this->field_0x10).y = 0.0f;
			(this->field_0x10).z = 0.0f;
			(this->field_0x10).w = 1.0f;

			this->field_0x20 = -1;
			this->pOwner->SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 0x13, -1);
			break;
		case 3:
			this->field_0x24 = -1;

			(this->field_0x10).x = 0.0f;
			(this->field_0x10).y = 0.0f;
			(this->field_0x10).z = 0.0f;
			(this->field_0x10).w = 1.0f;

			this->field_0x20 = -1;
			pNativ = this->pOwner;
			if ((pNativ->field_0x4f8 == -1) && (pNativ->field_0x4fc == -1)) {
				pNativ->SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 6, -1);
			}
			else {
				pNativ->SetBehaviour(pNativ->field_0x4f8, pNativ->field_0x4fc, pNativ->field_0x500);
				this->pOwner->field_0x4f8 = -1;
				this->pOwner->field_0x4fc = -1;
				this->pOwner->field_0x500 = -1;
			}
			break;
		case 4:
			pNativ->behaviourTakeAndPut.GetCurrentPathFollowReader()->Reset();
			this->pOwner->SetBehaviour(NATIVE_BEHAVIOUR_TAKE_AND_PUT, -1, -1);
			break;
		case 5:
			IMPLEMENTATION_GUARD(
			this->field_0x20 = *(undefined4*)((int)pMsgParam + 4);
			pfVar3 = *(float**)((int)pMsgParam + 0x18);
			fVar15 = pfVar3[1];
			fVar11 = pfVar3[2];
			fVar13 = pfVar3[3];
			(this->field_0x10).x = *pfVar3;
			(this->field_0x10).y = fVar15;
			(this->field_0x10).z = fVar11;
			(this->field_0x10).w = fVar13;
			this->field_0x24 = *(int*)((int)pMsgParam + 8);
			this->field_0x28 = *(float*)((int)pMsgParam + 0x14);
			this->pOwner->SetState(0xb, -1);
			pNativ = this->pOwner;
			(pNativ->field_0x570).x = (pNativ->base).base.base.currentLocation.x;
			(pNativ->field_0x570).y = (pNativ->base).base.base.currentLocation.y;
			(pNativ->field_0x570).z = (pNativ->base).base.base.currentLocation.z;
			(pNativ->field_0x570).w = (pNativ->base).base.base.currentLocation.w;)
			break;
		case 6:
			IMPLEMENTATION_GUARD(
			this->field_0x20 = *(undefined4*)((int)pMsgParam + 4);
			pfVar3 = *(float**)((int)pMsgParam + 0x18);
			fVar15 = pfVar3[1];
			fVar11 = pfVar3[2];
			fVar13 = pfVar3[3];
			(this->field_0x10).x = *pfVar3;
			(this->field_0x10).y = fVar15;
			(this->field_0x10).z = fVar11;
			(this->field_0x10).w = fVar13;
			pNativ = this->pOwner;
			(pNativ->field_0x570).x = (pNativ->base).base.base.currentLocation.x;
			(pNativ->field_0x570).y = (pNativ->base).base.base.currentLocation.y;
			(pNativ->field_0x570).z = (pNativ->base).base.base.currentLocation.z;
			(pNativ->field_0x570).w = (pNativ->base).base.base.currentLocation.w;
			this->field_0x24 = *(int*)((int)pMsgParam + 8);
			this->pOwner->SetState(10, -1);)
			break;
		case 7:
			this->pOwner->SetState(10, -1);
			break;
		case 9:
			IMPLEMENTATION_GUARD(
			this->field_0x20 = *(undefined4*)((int)pMsgParam + 4);
			pfVar3 = *(float**)((int)pMsgParam + 0x18);
			fVar15 = pfVar3[1];
			fVar11 = pfVar3[2];
			fVar13 = pfVar3[3];
			(this->field_0x10).x = *pfVar3;
			(this->field_0x10).y = fVar15;
			(this->field_0x10).z = fVar11;
			(this->field_0x10).w = fVar13;
			this->field_0x24 = *(int*)((int)pMsgParam + 8);
			this->field_0x28 = *(float*)((int)pMsgParam + 0x14);
			this->pOwner->SetState(9, *(int*)((int)pMsgParam + 0xc) + 0xd);)
			break;
		case NATIVE_MESSAGE_FACE_TOWARDS:
			bVar4 = this->pOwner->CanSpeak();
			if (bVar4 == false) {
				pNativ = this->pOwner;
				pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_TOWARDS, -1);
			}
			else {
				pNativ = this->pOwner;
				pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_TOWARDS, 0x15);
			}
			break;
		case NATIVE_MESSAGE_FACE_AWAY:
			bVar4 = this->pOwner->CanSpeak();
			if (bVar4 == false) {
				pNativ = this->pOwner;
				pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_AWAY, -1);
			}
			else {
				pNativ = this->pOwner;
				pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_AWAY, 0x15);
			}
			break;
		case 0xc:
			this->field_0x24 = -1;
			(this->field_0x10).x = 0.0f;
			(this->field_0x10).y = 0.0f;
			(this->field_0x10).z = 0.0f;
			(this->field_0x10).w = 1.0f;
			this->field_0x20 = -1;
			this->field_0x28 = -1.0f;
			this->pOwner->SetBehaviour(NATIVE_BEHAVIOUR_LIVE, 7, -1);
			break;
		case 0xd:
			IMPLEMENTATION_GUARD(
			this->pOwner->field_0x4f8 = this->pOwner->curBehaviourId;
			this->pOwner->field_0x4fc = this->pOwner->actorState;
			this->pOwner->field_0x500 = this->pOwner->currentAnimType;
			pfVar3 = *(float**)((int)pMsgParam + 0x18);
			fVar15 = pfVar3[1];
			fVar11 = pfVar3[2];
			fVar13 = pfVar3[3];
			(this->field_0x10).x = *pfVar3;
			(this->field_0x10).y = fVar15;
			(this->field_0x10).z = fVar11;
			(this->field_0x10).w = fVar13;
			this->field_0x24 = *(int*)((int)pMsgParam + 8);
			this->field_0x28 = *(float*)((int)pMsgParam + 0x14);
			pNativ = this->pOwner;
			iVar7 = *(int*)((int)pMsgParam + 0xc);
			(pNativ->field_0x570).x = (pNativ->base).base.base.currentLocation.x;
			(pNativ->field_0x570).y = (pNativ->base).base.base.currentLocation.y;
			(pNativ->field_0x570).z = (pNativ->base).base.base.currentLocation.z;
			(pNativ->field_0x570).w = (pNativ->base).base.base.currentLocation.w;
			this->pOwner->SetState(9, iVar7 + 0xd);
			this->pOwner->field_0x530 = 0;)
			break;
		case 0x10:
			IMPLEMENTATION_GUARD(
			pCVar8 = CActor::GetBehaviour((CActor*)this->pOwner, 9);
			pCVar2 = pCVar8[1].pVTable;
			if (((pCVar2[0xd].SetInitialState == (undefined*)0xffffffff) &&
				(pCVar2[0xd].Execute == (ExecuteFunc*)0xffffffff)) && (pCVar2[0xd]._ManageHit == (undefined*)0xffffffff)) {
				pCVar2[0xd].SetInitialState = pCVar2[2].InitDlistPatchable;
				pCVar8[1].pVTable[0xd].Execute = (ExecuteFunc*)pCVar8[1].pVTable[2].SetInitialState;
				pCVar8[1].pVTable[0xd]._ManageHit = pCVar8[1].pVTable[3]._dt;
				pCVar2 = pCVar8[1].pVTable;
				pCVar14 = (CBehaviourFighterVTable*)pCVar2->InitState;
				pCVar10 = (CBehaviourFighterVTable*)pCVar2->TermState;
				pCVar12 = (CBehaviourFighterVTable*)pCVar2->InterpretMessage;
				pCVar8[4].pVTable = (CBehaviourFighterVTable*)pCVar2->End;
				pCVar8[5].pVTable = pCVar14;
				pCVar8[6].pVTable = pCVar10;
				pCVar8[7].pVTable = pCVar12;
			})

			this->pOwner->SetBehaviour(9, 0x3a, -1);
		}
		iVar7 = 0;
	}
	else {
		if (msg == 0x14) {
			local_20.type = 9;
			local_20.field_0x10 = this->pOwner->FUN_00162a70();
			iVar7 = this->pOwner->DoMessage(this->pOwner->field_0x3f0, MESSAGE_NATIV_CMD, &local_20);
			if (iVar7 == 0) {
				iVar7 = 0;
			}
			else {
				iVar7 = 1;
				this->pOwner->cinematicCachedBehaviour = this->pOwner->curBehaviourId;
				this->pOwner->cinematicCachedState = this->pOwner->actorState;
				this->pOwner->cinematicCachedAnim = this->pOwner->currentAnimType;
			}
		}
		else {
			if (msg == 0x12) {
				this->pOwner->field_0x508 = 1.0f;
				pNativ = this->pOwner;
				uVar6 = pNativ->GetStateFlags(pNativ->actorState) & 0x4000;

				if (uVar6 == 0) {
					this->pOwner->field_0x4f8 = this->pOwner->curBehaviourId;
					this->pOwner->field_0x4fc = this->pOwner->actorState;
					this->pOwner->field_0x500 = this->pOwner->currentAnimType;
					this->pOwner->field_0x504 = 1;
				}

				iVar7 = 0xf;
				if (this->pOwner->actorState != NATIVE_STATE_SPEAK_FACE_TOWARDS) {
					iVar7 = 0;
				}
			}
			else {
				iVar7 = 0;
			}
		}
	}

	return iVar7;
}

int CBehaviourNativSpeak::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourNativSpeak::Reset()
{
	this->field_0x24 = -1;
	this->field_0x28 = -1.0f;

	return;
}

void CEmotionInfo::DoAnimation(float, float, CActor*)
{
	IMPLEMENTATION_GUARD_EMOTION();
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

void CBehaviourNativExorcisme::Manage()
{
	this->pOwner->BehaviourExorcisme_Manage();
}

void CBehaviourNativExorcisme::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorNativ*>(pOwner);

	this->pOwner->field_0x53c = 1;
	this->pOwner->flags = this->pOwner->flags & 0xfffffffd;
	this->pOwner->flags = this->pOwner->flags | 1;
	this->pOwner->flags = this->pOwner->flags & 0xffffff7f;
	this->pOwner->flags = this->pOwner->flags | 0x20;
	this->pOwner->EvaluateDisplayState();

	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourNativExorcisme::InitState(int newState)
{
	ulong uVar2;
	edF32VECTOR4 local_10;

	if (newState == 0x10) {
		this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(((float)CScene::Rand() * 0.4f) / 32767.0f + 0.8f, 0);
	}
	else {
		if (newState == 0xe) {
			edF32Vector4SubHard(&local_10, &CActorHero::_gThis->currentLocation, &this->pOwner->currentLocation);
			local_10.y = 0.0f;
			edF32Vector4NormalizeHard(&local_10, &local_10);
			this->pOwner->rotationQuat = local_10;
		}
	}

	return;
}

void CBehaviourNativExorcisme::TermState(int oldState, int newState)
{
	if (oldState == 0x10) {
		this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
	}

	return;
}

int CBehaviourNativExorcisme::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar2;

	if (msg == 0x19) {
		IMPLEMENTATION_GUARD(
		/* WARNING: Load size is inaccurate */
		iVar2 = *pMsgParam;
		if (iVar2 == 3) {
			iVar2 = (this->pOwner)->field_0x53c;
		}
		else {
			if (iVar2 == 1) {
				pCVar1 = this->pOwner;
				(*(this->pOwner->pVTable)->SetState)((CActor*)pCVar1, 0x12, -1);
			}
			else {
				if (iVar2 == 0) {
					(this->pOwner)->field_0x53c = 0;
					pCVar1 = this->pOwner;
					this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
					pCVar1 = this->pOwner;
					this->pOwner->flags = this->pOwner->flags & 0xffffff5f;
					CActor::EvaluateDisplayState((CActor*)pCVar1);
					CActor::UpdatePosition((CActor*)this->pOwner, (edF32VECTOR4*)((int)pMsgParam + 0x10), true);
					pCVar1 = this->pOwner;
					(*(this->pOwner->pVTable)->SetState)((CActor*)pCVar1, 0xe, -1);
				}
			}

			iVar2 = 1;
		})
	}
	else {
		iVar2 = 0;
	}

	return iVar2;
}

void CBehaviourNativLive::Create(ByteCode* pByteCode)
{
	this->field_0x8 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
}

void CBehaviourNativLive::Manage()
{
	this->pOwner->BehaviourNativLive_Manage();

	return;
}

void CBehaviourNativLive::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = reinterpret_cast<CActorNativ*>(pOwner);
	this->pOwner = reinterpret_cast<CActorNativ*>(pOwner);

	if (newState == -1) {
		if ((this->pOwner->cinematicCachedState == -1) || (this->pOwner->cinematicCachedBehaviour == -1)) {
			this->pOwner->SetState(6, -1);
		}
		else {
			this->pOwner->ChooseBehaviour();
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourNativLive::End(int newBehaviourId)
{
	return;
}

void CBehaviourNativLive::InitState(int newState)
{
	if ((newState != 7) && (newState == 6)) {
		this->pOwner->field_0x530 = 1;
	}
	return;
}

void CBehaviourNativLive::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourNativLive_TermState(oldState);
}

int CBehaviourNativLive::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorNativ* pNativ;
	//CBehaviourMovingPlatformVTable* pCVar2;
	undefined4* puVar3;
	bool bVar4;
	StateConfig* pSVar5;
	uint uVar6;
	int iVar7;
	CBehaviour* pCVar8;
	CTakePutTrajectoryParam* pCVar9;
	CPathFollowReader* pPathFollow;
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
		CActorNativMsgParam_0xe* pParams = reinterpret_cast<CActorNativMsgParam_0xe*>(pMsgParam);
		iVar7 = pParams->type;
		if (iVar7 == NATIVE_MESSAGE_FACE_AWAY) {
			bVar4 = this->pOwner->CanSpeak();
			if (bVar4 == false) {
				pNativ = this->pOwner;
				pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_AWAY, -1);
			}
			else {
				pNativ = this->pOwner;
				pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_AWAY, 0x15);
			}
		}
		else {
			if (iVar7 == NATIVE_MESSAGE_FACE_TOWARDS) {
				bVar4 = this->pOwner->CanSpeak();
				if (bVar4 == false) {
					pNativ = this->pOwner;
					pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_TOWARDS, -1);
				}
				else {
					pNativ = this->pOwner;
					pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, NATIVE_STATE_SPEAK_FACE_TOWARDS, 0x15);
				}
			}
			else {
				if (iVar7 == 0xd) {
					CActorNativMsgParam_0xe* pMsgParam_0xe = reinterpret_cast<CActorNativMsgParam_0xe*>(pMsgParam);
					pNativ = this->pOwner;
					pNativ->field_0x4f8 = pNativ->curBehaviourId;
					pNativ = this->pOwner;
					pNativ->field_0x4fc = pNativ->actorState;
					pNativ = this->pOwner;
					pNativ->field_0x500 = pNativ->currentAnimType;
					pNativ = this->pOwner;

					pNativ->behaviourSpeak.field_0x10 = *pMsgParam_0xe->field_0x18;

					((this->pOwner)->behaviourSpeak).field_0x24 = pMsgParam_0xe->field_0x8;
					((this->pOwner)->behaviourSpeak).field_0x28 = pMsgParam_0xe->field_0x14;
					pNativ = this->pOwner;
					pNativ->SetBehaviour(NATIVE_BEHAVIOUR_SPEAK, 9, pMsgParam_0xe->field_0xc + 0xd);
					(this->pOwner)->field_0x530 = 0;
				}
				else {
					if (iVar7 != 7) {
						if (iVar7 == 9) {
							IMPLEMENTATION_GUARD(
							*(undefined4*)&(((this->base).pOwner)->behaviourSpeak).field_0x20 = *(undefined4*)((int)pMsgParam + 4);
							pNativ = (this->base).pOwner;
							puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
							uVar15 = puVar3[1];
							uVar11 = puVar3[2];
							uVar13 = puVar3[3];
							*(undefined4*)&(pNativ->behaviourSpeak).field_0x10 = *puVar3;
							*(undefined4*)&(pNativ->behaviourSpeak).field_0x14 = uVar15;
							*(undefined4*)&(pNativ->behaviourSpeak).field_0x18 = uVar11;
							*(undefined4*)&(pNativ->behaviourSpeak).field_0x1c = uVar13;
							(((this->base).pOwner)->behaviourSpeak).field_0x24 = *(int*)((int)pMsgParam + 8);
							(((this->base).pOwner)->behaviourSpeak).field_0x28 = *(float*)((int)pMsgParam + 0x14);
							pNativ = (this->base).pOwner;
							(*(pNativ->pVTable)->SetBehaviour)
								((CActor*)pNativ, 3, 9, *(int*)((int)pMsgParam + 0xc) + 0xd);)
						}
						else {
							if (iVar7 == 6) {
								IMPLEMENTATION_GUARD(
								*(undefined4*)&(((this->base).pOwner)->behaviourSpeak).field_0x20 = *(undefined4*)((int)pMsgParam + 4)
									;
								pNativ = (this->base).pOwner;
								puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
								uVar15 = puVar3[1];
								uVar11 = puVar3[2];
								uVar13 = puVar3[3];
								*(undefined4*)&(pNativ->behaviourSpeak).field_0x10 = *puVar3;
								*(undefined4*)&(pNativ->behaviourSpeak).field_0x14 = uVar15;
								*(undefined4*)&(pNativ->behaviourSpeak).field_0x18 = uVar11;
								*(undefined4*)&(pNativ->behaviourSpeak).field_0x1c = uVar13;
								(((this->base).pOwner)->behaviourSpeak).field_0x24 = *(int*)((int)pMsgParam + 8);
								(((this->base).pOwner)->behaviourSpeak).field_0x28 = *(float*)((int)pMsgParam + 0x14);
								pNativ = (this->base).pOwner;
								(pNativ->field_0x570).x = pNativ->currentLocation.x;
								(pNativ->field_0x570).y = pNativ->currentLocation.y;
								(pNativ->field_0x570).z = pNativ->currentLocation.z;
								(pNativ->field_0x570).w = pNativ->currentLocation.w;
								pNativ = (this->base).pOwner;
								(*(pNativ->pVTable)->SetBehaviour)((CActor*)pNativ, 3, 10, -1);)
							}
							else {
								if (iVar7 == 5) {
									IMPLEMENTATION_GUARD(
									*(undefined4*)&(((this->base).pOwner)->behaviourSpeak).field_0x20 =
										*(undefined4*)((int)pMsgParam + 4);
									pNativ = (this->base).pOwner;
									puVar3 = *(undefined4**)((int)pMsgParam + 0x18);
									uVar15 = puVar3[1];
									uVar11 = puVar3[2];
									uVar13 = puVar3[3];
									*(undefined4*)&(pNativ->behaviourSpeak).field_0x10 = *puVar3;
									*(undefined4*)&(pNativ->behaviourSpeak).field_0x14 = uVar15;
									*(undefined4*)&(pNativ->behaviourSpeak).field_0x18 = uVar11;
									*(undefined4*)&(pNativ->behaviourSpeak).field_0x1c = uVar13;
									(((this->base).pOwner)->behaviourSpeak).field_0x24 = *(int*)((int)pMsgParam + 8);
									(((this->base).pOwner)->behaviourSpeak).field_0x28 = *(float*)((int)pMsgParam + 0x14);
									pNativ = (this->base).pOwner;
									(*(pNativ->pVTable)->SetBehaviour)((CActor*)pNativ, 3, 0xb, -1);
									pNativ = (this->base).pOwner;
									(pNativ->field_0x570).x = pNativ->currentLocation.x;
									(pNativ->field_0x570).y = pNativ->currentLocation.y;
									(pNativ->field_0x570).z = pNativ->currentLocation.z;
									(pNativ->field_0x570).w = pNativ->currentLocation.w;)
								}
								else {
									if (iVar7 == 4) {
										CActorNativMsgParam_0xe* pTajectoryParam = reinterpret_cast<CActorNativMsgParam_0xe*>(pMsgParam);
										this->pOwner->behaviourTakeAndPut.curTrajectoryParamIndex = pTajectoryParam->field_0x4;
										this->pOwner->behaviourTakeAndPut.GetCurrentPathFollowReader()->Reset();
										pNativ = this->pOwner;
										pNativ->SetBehaviour(NATIVE_BEHAVIOUR_TAKE_AND_PUT, -1, -1);
									}
									else {
										if (iVar7 == 3) {
											IMPLEMENTATION_GUARD(
											(((this->base).pOwner)->behaviourSpeak).field_0x24 = *(int*)((int)pMsgParam + 8);
											pNativ = (this->base).pOwner;
											if ((pNativ->field_0x4f8 == -1) && (pNativ->field_0x4fc == -1)) {
												(*(pNativ->pVTable)->SetState)((CActor*)pNativ, 6, -1);
											}
											else {
												(*(pNativ->pVTable)->SetBehaviour)
													((CActor*)pNativ, pNativ->field_0x4f8, pNativ->field_0x4fc, pNativ->field_0x500);
												((this->base).pOwner)->field_0x4f8 = -1;
												((this->base).pOwner)->field_0x4fc = -1;
												((this->base).pOwner)->field_0x500 = -1;
											})
										}
										else {
											if (iVar7 == 2) {
												pNativ = this->pOwner;
												pNativ->SetState(0x13, -1);
											}
											else {
												if (iVar7 == 0x10) {
													IMPLEMENTATION_GUARD(
													pCVar8 = CActor::GetBehaviour((CActor*)(this->base).pOwner, 9);
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
													pNativ = (this->base).pOwner;
													(*(pNativ->pVTable)->SetBehaviour)((CActor*)pNativ, 9, 0x3a, -1);)
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
	}
	else {
		if (msg == 0x14) {
			IMPLEMENTATION_GUARD(
			local_20[0] = 9;
			local_10 = CActorNativ::FUN_00162a70((this->base).pOwner);
			pNativ = (this->base).pOwner;
			local_4 = local_20;
			iVar7 = CActor::DoMessage((CActor*)pNativ, pNativ->field_0x3f0, 0x4e, (uint)local_4);
			if (iVar7 != 0) {
				pNativ = (this->base).pOwner;
				pNativ->cinematicCachedBehaviour = pNativ->curBehaviourId;
				pNativ = (this->base).pOwner;
				pNativ->cinematicCachedState = pNativ->actorState;
				pNativ = (this->base).pOwner;
				pNativ->cinematicCachedAnim = pNativ->currentAnimType;
				return 1;
			})
			return 0;
		}
		if (msg == 0x12) {
			this->pOwner->field_0x508 = 1.0f;
			pNativ = this->pOwner;
			uVar6 = pNativ->GetStateFlags(pNativ->actorState) & 0x4000;

			if (uVar6 == 0) {
				this->pOwner->field_0x4f8 = this->pOwner->curBehaviourId;
				this->pOwner->field_0x4fc = this->pOwner->actorState;
				this->pOwner->field_0x500 = this->pOwner->currentAnimType;
				this->pOwner->field_0x504 = 1;
			}

			if (this->pOwner->actorState == NATIVE_STATE_SPEAK_FACE_TOWARDS) {
				return 0xf;
			}
			return 0;
		}
	}

	return 0;
}

CBehaviourNativAkasa::CBehaviourNativAkasa()
{
	//this->field_0x60 = 0;
	//this->field_0x64 = 0;
}

void CBehaviourNativAkasa::Create(ByteCode* pByteCode)
{
	S_TARGET_STREAM_REF* pSVar1;
	S_STREAM_EVENT_CAMERA* pSVar2;
	int uVar3;
	int iVar4;
	int iVar8;

	if (2.26f <= CScene::_pinstance->field_0x1c) {
		this->field_0xc = pByteCode->GetS32();
	}
	else {
		this->field_0xc = -1;
	}

	this->field_0x8 = pByteCode->GetU32();

	this->comboTutorialManager.nbTutorials = pByteCode->GetS32();
	uVar3 = this->comboTutorialManager.nbTutorials;
	if (uVar3 != 0) {
		this->comboTutorialManager.aTutorials = new ArenaTutorial[uVar3];

		iVar4 = 0;
		if (0 < this->comboTutorialManager.nbTutorials) {
			do {
				ArenaTutorial* pSubObjA = this->comboTutorialManager.aTutorials + iVar4;
				pSubObjA->nbRequiredMoves = pByteCode->GetS32();
				uVar3 = pSubObjA->nbRequiredMoves;
				if (uVar3 != 0) {
					pSubObjA->aRequiredMoves = new NativSubObjB[uVar3];
					iVar8 = 0;
					if (0 < pSubObjA->nbRequiredMoves) {
						do {
							pSubObjA->aRequiredMoves[iVar8].Create(pByteCode);
							iVar8 = iVar8 + 1;
						} while (iVar8 < pSubObjA->nbRequiredMoves);
					}
				}

				iVar4 = iVar4 + 1;
			} while (iVar4 < this->comboTutorialManager.nbTutorials);
		}
	}

	this->addOn.Create(pByteCode);

	{
		S_TARGET_STREAM_REF* pTargetStreamRef = reinterpret_cast<S_TARGET_STREAM_REF*>(pByteCode->currentSeekPos);
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
		if (pTargetStreamRef->entryCount != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
		}
		this->field_0x38 = pTargetStreamRef;
	}

	pSVar2 = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pSVar2 + 1);
	this->streamEventCamera_0x3c = pSVar2;


	{
		S_TARGET_STREAM_REF* pTargetStreamRef = reinterpret_cast<S_TARGET_STREAM_REF*>(pByteCode->currentSeekPos);
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
		if (pTargetStreamRef->entryCount != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
		}
		this->field_0x40 = pTargetStreamRef;
	}

	pSVar2 = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pSVar2 + 1);
	this->streamEventCamera_0x44 = pSVar2;

	{
		S_TARGET_STREAM_REF* pTargetStreamRef = reinterpret_cast<S_TARGET_STREAM_REF*>(pByteCode->currentSeekPos);
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
		if (pTargetStreamRef->entryCount != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
		}
		this->field_0x48 = pTargetStreamRef;
	}

	pSVar2 = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pSVar2 + 1);
	this->streamEventCamera_0x4c = pSVar2;

	{
		S_TARGET_STREAM_REF* pTargetStreamRef = reinterpret_cast<S_TARGET_STREAM_REF*>(pByteCode->currentSeekPos);
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
		if (pTargetStreamRef->entryCount != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
		}
		this->field_0x50 = pTargetStreamRef;
	}

	pSVar2 = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pSVar2 + 1);
	this->streamEventCamera_0x54 = pSVar2;

	CActor::SV_InstallMaterialId(this->field_0xc);

	return;
}

void CBehaviourNativAkasa::Init(CActor* pOwner)
{
	S_TARGET_STREAM_REF* pSVar1;
	int iVar2;
	int iVar3;

	this->pOwner = static_cast<CActorNativ*>(pOwner);

	this->activeSubObjBIndex = 0;

	this->addOn.Init(pOwner);

	this->comboTutorialManager.Init();

	{
		S_TARGET_STREAM_REF* pTargetStreamRef = this->field_0x38;
		iVar3 = 0;
		if (0 < pTargetStreamRef->entryCount) {
			do {
				pTargetStreamRef->aEntries[iVar3].Init();
				iVar3 = iVar3 + 1;
			} while (iVar3 < pTargetStreamRef->entryCount);
		}
	}

	this->streamEventCamera_0x3c->Init();

	{
		S_TARGET_STREAM_REF* pTargetStreamRef = this->field_0x40;
		iVar3 = 0;
		if (0 < pTargetStreamRef->entryCount) {
			do {
				pTargetStreamRef->aEntries[iVar3].Init();
				iVar3 = iVar3 + 1;
			} while (iVar3 < pTargetStreamRef->entryCount);
		}
	}

	this->streamEventCamera_0x44->Init();
	{
		S_TARGET_STREAM_REF* pTargetStreamRef = this->field_0x48;
		iVar3 = 0;
		if (0 < pTargetStreamRef->entryCount) {
			do {
				pTargetStreamRef->aEntries[iVar3].Init();
				iVar3 = iVar3 + 1;
			} while (iVar3 < pTargetStreamRef->entryCount);
		}
	}

	this->streamEventCamera_0x4c->Init();
	{
		S_TARGET_STREAM_REF* pTargetStreamRef = this->field_0x50;
		iVar3 = 0;
		if (0 < pTargetStreamRef->entryCount) {
			do {
				pTargetStreamRef->aEntries[iVar3].Init();
				iVar3 = iVar3 + 1;
			} while (iVar3 < pTargetStreamRef->entryCount);
		}
	}

	this->streamEventCamera_0x54->Init();

	this->addOn.Reset();

	this->initialAnimId = -1;
	this->field_0x16ac = 0;
	this->field_0x16b0 = 0;
	this->field_0x16b4 = 0;
	//FUN_003fffa0((int)&this->field_0x60);

	this->currentBehaviourState = -1;

	this->field_0x16c0 = 0.0f;

	this->comboDesiredDisplayWidth = 0.0f;
	this->comboDesiredDisplayHeight = 0.0f;
	this->comboDesiredDisplayAlpha = 0.0f;

	this->comboDisplayWidth = 0.0f;
	this->comboDisplayHeight = 0.0f;
	this->comboDisplayAlpha = 0.0f;

	this->field_0x16b8 = 0;
	this->field_0x16a4 = 0;

	return;
}

void CBehaviourNativAkasa::Manage()
{
	this->pOwner->BehaviourNativAkasa_Manage(this);

	return;
}

void CBehaviourNativAkasa::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorNativ*>(pOwner);

	if (newState == -1) {
		if (this->initialAnimId == -1) {
			this->pOwner->SetState(0x1f, -1);
		}
		else {
			this->pOwner->SetState(this->initialAnimId, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourNativAkasa::End(int newBehaviourId)
{
	return;
}

void CBehaviourNativAkasa::InitState(int newState)
{
	this->pOwner->BehaviourNativAkasa_InitState(newState, this);

	return;
}

void CBehaviourNativAkasa::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourNativAkasa_TermState(oldState);

	return;
}

int CBehaviourNativAkasa::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorNativ* pCVar1;
	int iVar2;
	int iVar3;
	StateConfig* pSVar4;
	uint uVar5;

	if (msg == 0x7a) {
		IMPLEMENTATION_GUARD(
		CLevelScheduler::ScenVar_Get(0);)
	}
	else {
		if (msg == 0x14) {
			this->field_0x16b4 = 1;
			this->pOwner->SetState(0x26, -1);
			return 1;
		}

		if (msg == 0x12) {
			CLevelScheduler::ScenVar_Get(0);
			iVar2 = CLevelScheduler::ScenVar_Get(0);
			if (0 < iVar2) {
				pCVar1 = this->pOwner;
				uVar5 = pCVar1->GetStateFlags(pCVar1->actorState);
				if ((uVar5 & 0x400000) != 0) {
					return 0xf;
				}
			}

			return 0;
		}

		if (msg != 0xf) {
			return 0;
		}

		CLevelScheduler::ScenVar_Get(0);
		iVar2 = CLevelScheduler::ScenVar_Get(0);
		if (iVar2 == 0) {
			this->field_0x16b4 = 0;
			if (this->field_0x8 == 0xffffffff) {
				SetBehaviourState(0x22);

				iVar2 = this->field_0x16b4;
				if (iVar2 == 0) {
					(this->comboTutorialManager).activeTutorialIndex = -1;
					if (this->comboTutorialManager.StepRequiredCombo() == -1) {
						return 0;
					}
				}

				if (iVar2 == 1) {
					IMPLEMENTATION_GUARD(
					this->field_0x60.FUN_003fff90();
					if (this->field_0x60.FUN_003ffea0(this->field_0x16b0) == 0) {
						return 0;
					})
				}

				ArenaUpdateDisplayBorderSize();
			}
			else {
				SetBehaviourState(NATIVE_STATE_SELLER_INIT_ARENA_DISPLAY);
			}
		}
	}

	return 0;
}


void CBehaviourNativAkasa::ManageComboTutorial()
{
	CActorNativ* pNativ;
	bool bVar2;
	CEventManager* pEventManager;
	ed_zone_3d* pZone;
	int iVar6;
	uint uVar7;
	int iVar9;

	this->comboDisplayWidth = this->comboDisplayWidth + (this->comboDesiredDisplayWidth - this->comboDisplayWidth) * 0.15f;
	this->comboDisplayHeight = this->comboDisplayHeight + (this->comboDesiredDisplayHeight - this->comboDisplayHeight) * 0.05f;
	this->comboDisplayAlpha = this->comboDisplayAlpha + (this->comboDesiredDisplayAlpha - this->comboDisplayAlpha) * 0.04f;

	this->field_0x16c0 = this->field_0x16c0 + GetTimer()->cutsceneDeltaTime;
	iVar6 = this->currentBehaviourState;
	if (iVar6 != -1) {
		uVar7 = pOwner->GetStateFlags(iVar6);

		pEventManager = CScene::ptable.g_EventManager_006f5080;
		if ((uVar7 & 0x200000) != 0) {
			uVar7 = this->field_0x8;
			iVar6 = 0;
			if (uVar7 == 0xffffffff) {
				bVar2 = false;
			}
			else {
				pZone = (ed_zone_3d*)0x0;
				if (uVar7 != 0xffffffff) {
					pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, uVar7);
				}

				if (pZone != (ed_zone_3d*)0x0) {
					iVar6 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0);
				}

				if (iVar6 == 1) {
					bVar2 = true;
				}
				else {
					bVar2 = false;
				}
			}

			if (!bVar2) {
				this->comboDesiredDisplayWidth = 0.0f;
				this->comboDesiredDisplayHeight = 0.0f;
				this->comboDesiredDisplayAlpha = 0.0f;

				pNativ = this->pOwner;
				iVar6 = 0;
				if (0 < this->field_0x50->entryCount) {
					do {
						this->field_0x50->aEntries[iVar6].Switch(pNativ);
						iVar6 = iVar6 + 1;
					} while (iVar6 < this->field_0x50->entryCount);
				}

				this->streamEventCamera_0x54->SwitchOn(pNativ);

				this->currentBehaviourState = -1;

				pNativ = this->pOwner;
				if (pNativ->curBehaviourId == 1) {
					this->initialAnimId = 0x1f;
				}
				else {
					pNativ->SetBehaviour(NATIVE_BEHAVIOUR_AKASA, 0x1f, -1);
				}
			}
		}
	}

	switch (this->currentBehaviourState) {
	case -1:
		break;
	case NATIVE_STATE_SELLER_INIT_ARENA_DISPLAY:
		this->pOwner->StateInitArenaDisplay(this);
		break;
	case 0x22:
		this->pOwner->State_0x22(this);
		break;
	case 0x23:
		this->pOwner->State_0x23(this);
		break;
	case NATIVE_STATE_SELLER_INPUT_SUCCEEDED:
		this->pOwner->StateInputSucceeded(this);
		break;
	case 0x25:
		this->pOwner->State_0x25(this);
		break;
	case NATIVE_STATE_SELLER_INPUT_FAILED:
		this->pOwner->StateInputFailed(this);
		break;
	case 0x38:
		this->pOwner->State_0x38(this);
		break;
	default:
		IMPLEMENTATION_GUARD();
		break;
	}

	if (((this->field_0x16a4 == 0) && (this->field_0x16ac == 2)) && ((((CActorHero::_gThis->fightFlags & 0x40) != 0 ||
			((((iVar6 = CActorHero::_gThis->actorState, iVar6 == 0x6f || (iVar6 == 0x34)) || (iVar6 == NATIVE_STATE_SELLER_INPUT_FAILED)) || ((iVar6 == 10 &&
				(CActorHero::_gThis->prevActorState == 0x20)))))) || (iVar6 == 0x22)))) {
		pNativ = this->pOwner;

		iVar6 = 0;
		if (0 < this->field_0x38->entryCount) {
			do {
				this->field_0x38->aEntries[iVar6].Switch(pNativ);
				iVar6 = iVar6 + 1;
			} while (iVar6 < this->field_0x38->entryCount);
		}

		this->streamEventCamera_0x3c->SwitchOn(pNativ);

		this->field_0x16a4 = 1;
	}

	this->streamEventCamera_0x3c->Manage(this->pOwner);
	this->streamEventCamera_0x44->Manage(this->pOwner);
	this->streamEventCamera_0x4c->Manage(this->pOwner);
	this->streamEventCamera_0x54->Manage(this->pOwner);

	this->addOn.Manage();

	if (((this->comboDisplayWidth != 0.0f) || (this->comboDisplayHeight != 0.0f)) || (this->comboDisplayAlpha != 0.0f)) {
		if (this->field_0x16b4 == 0) {
			DrawButtonPromptA();
		}

		if (this->field_0x16b4 == 1) {
			IMPLEMENTATION_GUARD(
			FUN_003ec0e0(this);)
		}
	}

	return;
}



void CBehaviourNativAkasa::ArenaUpdateDisplayBorderSize()
{
	byte bVar1;
	s_fighter_combo* pCurrentCombo;
	ArenaTutorial* pReqCombo;
	int iVar4;
	int iVar5;
	float totalComboDisplayHeight;
	float fVar7;
	float totalComboDisplayWidth;
	float local_10;
	float local_c;
	float comboButtonWidth;
	float comboButtonHeight;

	fVar7 = 0.0f;
	totalComboDisplayWidth = (float)gVideoConfig.screenWidth * 0.13f;
	totalComboDisplayHeight = (float)gVideoConfig.screenHeight * 0.2f;

	if (CActorHero::_gThis != (CActorHero*)0x0) {
		CActorHero::_gThis->_UpdateComboSituation();

		if (this->field_0x16b4 == 0) {
			pReqCombo = GetActiveComboTutorial();

			iVar4 = 0;
			if (pReqCombo != (ArenaTutorial*)0x0) {
				for (; iVar4 < pReqCombo->nbRequiredMoves; iVar4 = iVar4 + 1) {
					pCurrentCombo = pReqCombo->aRequiredMoves[iVar4].aRequiredCombos[0];

					s_fighter_move* pMove = LOAD_SECTION_CAST(s_fighter_move*, pCurrentCombo->actionHash.pData);

					bVar1 = pMove->field_0x4.field_0x0byte;
					if (((bVar1 & 4) == 0) && (((bVar1 & 8) != 0 || ((bVar1 & 0x10) != 0)))) {
						totalComboDisplayWidth = totalComboDisplayWidth + (float)gVideoConfig.screenWidth * 0.08f;
					}

					GetComboButtonDisplaySize(pCurrentCombo, &comboButtonWidth, &comboButtonHeight);

					totalComboDisplayHeight = std::max<float>(comboButtonHeight, totalComboDisplayHeight);
					totalComboDisplayWidth = totalComboDisplayWidth + (float)gVideoConfig.screenWidth * 0.05f + comboButtonWidth;
				}
			}
		}

		if (this->field_0x16b4 == 1) {
			IMPLEMENTATION_GUARD(
			iVar4 = *(int*)&this->field_0x1690;
			if (iVar4 == 0) {
				iVar4 = 0;
			}
			iVar5 = 0;
			if (iVar4 != 0) {
				for (; iVar5 < *(int*)(iVar4 + 0x80); iVar5 = iVar5 + 1) {
					pCurrentCombo = *(s_fighter_combo**)(iVar4 + iVar5 * 0x10 + 4);
					bVar1 = *(byte*)((int)&pCurrentCombo->actionHash->field_0x4 + 1);
					if (((bVar1 & 4) == 0) && (((bVar1 & 8) != 0 || ((bVar1 & 0x10) != 0)))) {
						totalComboDisplayWidth = totalComboDisplayWidth + (float)gVideoConfig.screenWidth * 0.08f;
					}
					GetComboButtonDisplaySize(this, pCurrentCombo, &local_10, &local_c);
					fVar7 = (float)((int)fVar7 * (uint)(local_c < fVar7) | (int)local_c * (uint)(local_c >= fVar7));
					totalComboDisplayWidth = totalComboDisplayWidth + (float)gVideoConfig.screenWidth * 0.05f + local_10;
				}
			})
		}

		totalComboDisplayHeight = totalComboDisplayHeight + fVar7;
		totalComboDisplayWidth = totalComboDisplayWidth + ((float)gVideoConfig.screenWidth * 0.05f) / 2.0f;
	}

	this->comboDesiredDisplayWidth = totalComboDisplayWidth;
	this->comboDesiredDisplayHeight = totalComboDisplayHeight;
	this->comboDesiredDisplayAlpha = 1.0f;

	return;
}

void CBehaviourNativAkasa::FUN_003ebd90()
{
	CActorHero* pHero;
	long lVar2;
	undefined8 unaff_s1;

	pHero = CActorHero::_gThis;
	if (this->field_0x16ac == 2) {
		unaff_s1 = 4;
	}

	if (this->field_0x16ac == 1) {
		unaff_s1 = 3;
	}

	if (this->field_0x16a4 != 0) {
		this->field_0x16a4 = 0;
	}

	this->field_0x16ac = 0;
	this->activeSubObjBIndex = 0;

	if (this->field_0x16b4 == 0) {
		SetBehaviourState(0x22);
	}
	else {
		SetBehaviourState(0x28);
	}

	if (((pHero->fightFlags & 0x40) != 0) && (lVar2 = this->addOn.Func_0x20(unaff_s1, (CActor*)0x0, 0), lVar2 != 0)) {
		this->initialAnimId = 0x20;
	}
	return;
}

void CBehaviourNativAkasa::GetComboButtonDisplaySize(s_fighter_combo* pCombo, float* param_3, float* param_4)
{
	uint uVar1;
	char cVar2;
	ulong uVar3;
	uint uVar4;
	float fVar5;
	float fVar6;

	if (pCombo != (s_fighter_combo*)0x0) {
		fVar6 = 1.0f;
		fVar5 = 0.0f;

		uVar3 = ((ulong)pCombo->pattern.field_0x2ushort << 0x34) >> 0x38;
		if ((uVar3 & 1) != 0) {
			fVar5 = 1.0f;
		}

		if ((uVar3 & 2) != 0) {
			fVar5 = fVar5 + 1.0f;
		}

		if ((uVar3 & 4) != 0) {
			fVar5 = fVar5 + 1.0f;
		}

		if ((uVar3 & 8) != 0) {
			fVar5 = fVar5 + 1.0f;
		}

		if ((uVar3 & 0x10) != 0) {
			fVar5 = fVar5 + 1.0f;
		}

		for (uVar4 = 0; uVar4 < (pCombo->pattern).nbInputs; uVar4 = uVar4 + 1) {
			uVar1 = (uint)((ulong)(pCombo->pattern.field_0x0uint << 0x2c) >> 0x2c) >> ((uVar4 & 7) << 2) & 0xf;
			if (uVar1 == 1) {
				fVar5 = fVar5 + 1.0f;
			}

			if (uVar1 == 2) {
				fVar5 = fVar5 + 1.0f;
			}

			if (uVar1 == 4) {
				fVar5 = fVar5 + 1.0f;
			}

			if (uVar1 == 8) {
				fVar5 = fVar5 + 1.0f;
			}

			if (uVar1 == 3) {
				fVar5 = fVar5 + 1.0f;
			}

			if (uVar1 == 9) {
				fVar5 = fVar5 + 1.0f;
			}

			if (uVar1 == 6) {
				fVar5 = fVar5 + 1.0f;
			}

			if (uVar1 == 0xc) {
				fVar5 = fVar5 + 1.0f;
			}

			if (uVar1 == 10) {
				fVar5 = fVar5 + 1.0f;
			}
		}

		s_fighter_move* pMove = LOAD_SECTION_CAST(s_fighter_move*, pCombo->actionHash.pData);

		cVar2 = (pMove->field_0x4.field_0x0byte & 1) != 0;
		if ((pMove->field_0x4.field_0x0byte & 2) != 0) {
			cVar2 = cVar2 + '\x02';
		}

		if ((pMove->field_0x4.field_0x0byte & 0x20) != 0) {
			cVar2 = cVar2 + '\x04';
		}

		if ((pMove->field_0x4.field_0x0byte & 0x40) != 0) {
			cVar2 = cVar2 + '\b';
		}

		if (cVar2 == '\b') {
			fVar6 = 2.0f;
			fVar5 = std::max<float>(fVar5, 1.0f);
		}
		else {
			if (cVar2 == '\x04') {
				fVar6 = 2.0f;
				fVar5 = std::max<float>(fVar5, 1.0f);
			}
			else {
				if (cVar2 == '\x02') {
					fVar6 = 2.0f;
					fVar5 = std::max<float>(fVar5, 1.0f);
				}
				else {
					if (cVar2 == '\x01') {
						fVar6 = 2.0f;
						fVar5 = std::max<float>(fVar5, 1.0f);
					}
				}
			}
		}

		uVar3 = ((ulong)pCombo->pattern.field_0x3byte << 0x38) >> 0x3c;
		if ((uVar3 & 1) != 0) {
			fVar5 = fVar5 + 1.0f;
		}

		if ((uVar3 & 2) != 0) {
			fVar5 = fVar5 + 1.0f;
		}

		if (fVar5 <= 0.0f) {
			fVar6 = 0.0f;
		}

		*param_3 = (float)gVideoConfig.screenWidth * 0.068f * std::max<float>(fVar5, 1.0f) + (float)gVideoConfig.screenWidth * 0.08f * std::max(fVar5 - 1.0f, 0.0f);

		*param_4 = (float)gVideoConfig.screenHeight * 0.07f * std::max(fVar6 - 1.0f, 0.0f);
	}

	return;
}

void CBehaviourNativAkasa::FUN_003f1da0(s_fighter_combo* pCombo)
{
	if ((pCombo->field_0x4.field_0x0ushort & 0x400U) == 0) {
		s_fighter_move* pMove = LOAD_SECTION_CAST(s_fighter_move*, pCombo->actionHash.pData);
		if ((pMove->field_0x4.field_0x2ushort & 1) == 0) {
			this->field_0x16ac = 1;
		}
		else {
			this->field_0x16ac = 2;
		}
	}
	else {
		this->field_0x16ac = 2;
	}

	return;
}

bool CBehaviourNativAkasa::FUN_003ebee0(int param_2)
{
	int iVar1;

	if (param_2 == 0) {
		(this->comboTutorialManager).activeTutorialIndex = -1;

		iVar1 = this->comboTutorialManager.StepRequiredCombo();
		if (iVar1 == -1) {
			return true;
		}
	}

	if (param_2 == 1) {
		IMPLEMENTATION_GUARD(
		FUN_003fff90((int)&this->field_0x60);
		iVar1 = FUN_003ffea0(&this->field_0x60, (long)(int)this->field_0x16b0);
		if (iVar1 == 0) {
			return true;
		})
	}
	
	ArenaUpdateDisplayBorderSize();

	return false;
}

bool CBehaviourNativAkasa::IsEventActive()
{
	uint zoneId;
	CEventManager* pEventManager;
	bool bEventActive;
	ed_zone_3d* pZone;
	int iVar2;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	zoneId = this->field_0x8;
	iVar2 = 0;

	if (zoneId == 0xffffffff) {
		bEventActive = false;
	}
	else {
		pZone = (ed_zone_3d*)0x0;
		if (zoneId != 0xffffffff) {
			pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, zoneId);
		}

		if (pZone != (ed_zone_3d*)0x0) {
			iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0);
		}

		bEventActive = true;

		if (iVar2 != 1) {
			bEventActive = false;
		}
	}

	return bEventActive;
}

void CBehaviourNativAkasa::SetBehaviourState(int newState)
{
	if (newState != this->currentBehaviourState) {
		TermState(this->currentBehaviourState, -1);
		InitState(newState);

		this->field_0x16c0 = 0.0f;
		this->currentBehaviourState = newState;
	}

	return;
}

#define INPUT_STATE_NORMAL 0x1
#define INPUT_STATE_CORRECT 0x2
#define INPUT_STATE_CORRECT_HIT 0x3
#define INPUT_STATE_INCORRECT 0x4

int CBehaviourNativAkasa::GetInputState(int currentIndex, int param_3)
{
	int iVar1;
	CActorHero* pCVar2;
	bool bVar3;
	int inputState;
	long lVar5;

	pCVar2 = CActorHero::_gThis;
	inputState = this->currentBehaviourState;
	if ((inputState == 0x38) || (inputState == -1)) {
		inputState = 0;
	}
	else {
		if (inputState == 0x37) {
			inputState = INPUT_STATE_CORRECT_HIT;
		}
		else {
			iVar1 = this->activeSubObjBIndex;
			if (iVar1 + -1 < currentIndex) {
				if (currentIndex == iVar1) {
					if (inputState == NATIVE_STATE_SELLER_INPUT_FAILED) {
						return INPUT_STATE_INCORRECT;
					}

					if (((((inputState == 0x22) || (inputState == 0x28)) || (inputState == 0x32)) || ((inputState - 0x2fU < 2 || (inputState == 0x2c)))) || (inputState == 0x34)) {
						return INPUT_STATE_NORMAL;
					}

					if (((inputState == NATIVE_STATE_SELLER_INPUT_SUCCEEDED) || (inputState == 0x2a)) || ((inputState == 0x31 || (inputState == 0x35)))) {
						return INPUT_STATE_CORRECT_HIT;
					}

					if ((((inputState == 0x23) || (inputState == 0x29)) || (inputState - 0x2dU < 2)) || (inputState == 0x33)) {
						return INPUT_STATE_CORRECT;
					}

					if ((inputState == 0x25) || (inputState == 0x2b)) {
						lVar5 = CActorHero::_gThis->FUN_0031c9e0();
						if (lVar5 != 0) {
							return INPUT_STATE_NORMAL;
						}

						return 0;
					}
				}

				if (currentIndex == iVar1 + 1) {
					bVar3 = CActorHero::_gThis->FUN_0031b790(CActorHero::_gThis->actorState);
					if (((bVar3 == false) || (lVar5 = pCVar2->FUN_0031c9e0(), lVar5 == 0)) || (inputState = 1, param_3 != 0)) {
						inputState = 0;
					}
				}
				else {
					inputState = 0;
				}
			}
			else {
				inputState = 5;
			}
		}
	}

	return inputState;
}

uint INPUT_COLOR_NORMAL = 0x80806060;
uint INPUT_COLOR = 0x80303030;
uint INPUT_COLOR_CORRECT = 0x80007000;
uint INPUT_COLOR_INCORRECT = 0x80000070;

void CBehaviourNativAkasa::SetupTextStyle(int funcIndex, edCTextStyle* pTextStyle)
{
	uint inputColorNormal;
	uint inputColor;
	uint inputColorCorrect;
	uint inputColorIncorrect;

	inputColorNormal = INPUT_COLOR_NORMAL & 0xffffff | (int)(this->comboDisplayAlpha * 128.0f) << 0x18;
	inputColor = INPUT_COLOR & 0xffffff | (int)(this->comboDisplayAlpha * 128.0f) << 0x18;
	inputColorCorrect = INPUT_COLOR_CORRECT & 0xffffff | (int)(this->comboDisplayAlpha * 128.0f) << 0x18;
	inputColorIncorrect = INPUT_COLOR_INCORRECT & 0xffffff | (int)(this->comboDisplayAlpha * 128.0f) << 0x18;

	if ((uint)funcIndex < 6) {
		switch (funcIndex) {
		case 0:
			pTextStyle->SetScale(1.0f, 1.0f);
			pTextStyle->rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
			pTextStyle->altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
			pTextStyle->alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
			this->field_0x16b8 = inputColor;
			break;
		case INPUT_STATE_NORMAL:
			pTextStyle->SetScale(1.3f, 1.3f);
			pTextStyle->rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
			this->field_0x16b8 = inputColorNormal;
			break;
		case INPUT_STATE_CORRECT:
			pTextStyle->SetScale(1.3f, 1.3f);
			pTextStyle->rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
			this->field_0x16b8 = inputColorCorrect;
			break;
		case INPUT_STATE_CORRECT_HIT:
			pTextStyle->SetScale(1.3f, 1.3f);
			pTextStyle->rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
			this->field_0x16b8 = inputColorCorrect;
			break;
		case INPUT_STATE_INCORRECT:
			pTextStyle->SetScale(1.3f, 1.3f);
			pTextStyle->rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
			this->field_0x16b8 = inputColorIncorrect;
			break;
		case 5:
			pTextStyle->SetScale(1.3f, 1.3f);
			pTextStyle->rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
			pTextStyle->alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
			this->field_0x16b8 = inputColorCorrect;
			break;
		}
	}
	return;
}

uint UINT_00448d68 = 0x80007000;
uint UINT_00448d6c = 0x80303030;

void DrawInputBorder(edF32VECTOR2* v0, edF32VECTOR2* v1, _rgba* color, edDList_material* pMaterial, int param_5)
{
	float fVar1;
	float fVar2;
	float y;
	float x;

	fVar1 = v1->x / 2.0f;
	fVar2 = v1->y / 2.0f;
	x = v0->x - fVar1;
	fVar1 = v0->x + fVar1;
	y = v0->y - fVar2;
	fVar2 = v0->y + fVar2;

	edDListUseMaterial(pMaterial);
	edDListLoadIdentity();

	if (param_5 != 0) {
		edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);
		if (color != (_rgba*)0x0) {
			edDListColor4u8(0, 0, 0, 0x80);
		}

		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f(x, y, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f(fVar1, y, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f(x, fVar2, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(fVar1, fVar2, 0.0f, 0.0f);
		edDListEnd();
	}

	edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);
	if (color != (_rgba*)0x0) {
		edDListColor4u8(color->r, color->g, color->b, color->a);
	}

	edDListTexCoo2f(0.0f, 0.0f);
	edDListVertex4f(x, y, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 0.0f);
	edDListVertex4f(fVar1, y, 0.0f, 0.0f);
	edDListTexCoo2f(0.0f, 1.0f);
	edDListVertex4f(x, fVar2, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 1.0f);
	edDListVertex4f(fVar1, fVar2, 0.0f, 0.0f);
	edDListEnd();

	return;
}

bool CBehaviourNativAkasa::DrawButton(s_fighter_combo* pCombo, float* pOutWidth, int param_4)
{
	byte bVar1;
	uint uVar2;
	bool bVar3;
	edCTextStyle* pTextStyle;
	edDList_material* pMaterial;
	int iVar4;
	float fVar5;
	edCTextStyle textStyle;
	edF32VECTOR2 local_20;
	edF32VECTOR2 local_18;
	uint textColor;
	float textScale;
	uint local_8;
	uint local_4;
	CActorHero* pHero;

	iVar4 = (int)(this->comboDisplayAlpha * 128.0f);
	local_8 = UINT_00448d6c & 0xffffff | iVar4 << 0x18;
	local_4 = UINT_00448d68 & 0xffffff | iVar4 << 0x18;

	pTextStyle = edTextStyleGetCurrent();
	textStyle = *pTextStyle;
	pTextStyle = edTextStyleSetCurrent(&textStyle);

	pHero = CActorHero::_gThis;
	textStyle.GetScale(&textScale, &textScale);
	textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;

	s_fighter_move* pMove = LOAD_SECTION_CAST(s_fighter_move*, pCombo->actionHash.pData);

	bVar1 = pMove->field_0x4.field_0x1byte;
	bVar3 = true;

	if (((bVar1 & 4) == 0) && (((bVar1 & 8) != 0 || ((bVar1 & 0x10) != 0)))) {
		textColor = this->field_0x16b8;
		if (param_4 == 1) {
			uVar2 = pHero->comboFlags;
			if (((uVar2 & 8) == 0) && ((uVar2 & 0x10) == 0)) {
				bVar3 = false;
			}
			else {
				bVar3 = true;
				textColor = local_4;
			}
		}

		local_18.x = *pOutWidth;
		fVar5 = (float)gVideoConfig.screenHeight;
		local_18.y = fVar5 * 0.92f - fVar5 * 0.008f;
		local_20.x = (float)gVideoConfig.screenWidth * 0.068f * textScale;
		local_20.y = fVar5 * 0.075f * textScale;

		pMaterial = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->field_0xc, 0);

		DrawInputBorder(&local_18, &local_20, reinterpret_cast<_rgba*>(&textColor), pMaterial, 0);
		edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[VALID]b");
		*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f * textScale;
	}

	if (!bVar3) {
		this->field_0x16b8 = local_8;
		pTextStyle->SetScale(1.0f, 1.0f);
		pTextStyle->rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
		pTextStyle->altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
		pTextStyle->alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
	}

	edTextStyleSetCurrent(pTextStyle);

	return bVar3;
}


void CBehaviourNativAkasa::DrawInventoryCrouch(uint flags, float* pOutWidth)
{
	byte bVar1;
	CInputAnalyser* pCVar2;
	CActorHero* pHero;
	edCTextStyle* pTextStyle;
	edCTextStyle textStyle;
	float fStack4;

	pHero = CActorHero::_gThis;
	pTextStyle = edTextStyleGetCurrent();
	textStyle = *pTextStyle;
	pTextStyle = edTextStyleSetCurrent(&textStyle);

	if ((flags & 1) != 0) {
		pCVar2 = pHero->pInputAnalyser;
		bVar1 = 0xb;
		if (pCVar2 != (CInputAnalyser*)0x0) {
			bVar1 = pCVar2->patternB.field_0x3byte;
		}

		if ((((ulong)bVar1 << 0x38) >> 0x3c & 1) == 0) {
			textStyle.SetScale(1.0f, 1.0f);
			textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
			textStyle.altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
			textStyle.alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
		}

		textStyle.GetScale(&fStack4, &fStack4);
		edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[INVENT]b");
		*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
	}

	if ((flags & 2) != 0) {
		pCVar2 = pHero->pInputAnalyser;
		bVar1 = 0xb;
		if (pCVar2 != (CInputAnalyser*)0x0) {
			bVar1 = pCVar2->patternB.field_0x3byte;
		}
		if ((((ulong)bVar1 << 0x38) >> 0x3c & 2) == 0) {
			textStyle.SetScale(1.0f, 1.0f);
			textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
			textStyle.altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
			textStyle.alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
		}

		textStyle.GetScale(&fStack4, &fStack4);
		edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[CROUCH]b");
		*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
	}

	edTextStyleSetCurrent(pTextStyle);

	return;
}

void CBehaviourNativAkasa::DrawDirectionArrow(s_input_pattern* pPattern, float* pOutWidth)
{
	edCTextStyle* pNewFont;
	edCTextStyle* pNewFont_00;
	uint uVar1;
	uint uVar2;
	uint uVar3;
	float fonScale;

	pNewFont = edTextStyleGetCurrent();
	pNewFont_00 = edTextStyleSetCurrent(pNewFont);
	pNewFont->GetScale(&fonScale, &fonScale);

	uVar3 = 0;
	if (pPattern->nbInputs != 0) {
		uVar1 = 0;
		do {
			uVar2 = (uint)((ulong)((ulong)(uint)pPattern->field_0x0uint << 0x2c) >> 0x2c) >> (uVar1 & 0x1f) & 0xf;
			if (uVar2 == 1) {
				/* %[RIGHT]b */
				edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[RIGHT]b");
				*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
			}

			if ((uVar2 == 2) || (uVar2 == 10)) {
				/* %[UP]b */
				edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[UP]b");
				*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
			}

			if (uVar2 == 4) {
				/* %[LEFT]b */
				edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[LEFT]b");
				*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
			}

			if (uVar2 == 8) {
				/* %[DOWN]b */
				edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[DOWN]b");
				*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
			}

			if (uVar2 == 3) {
				/* %[UP]b */
				pNewFont->SetRotation(0.7853982f);
				edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[UP]b");
				pNewFont->SetRotation(0.0f);
				*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
			}

			if (uVar2 == 9) {
				/* %[LEFT]b */
				pNewFont->SetRotation(0.7853982f);
				edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[LEFT]b");
				pNewFont->SetRotation(0.0f);
				*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
			}

			if (uVar2 == 6) {
				/* %[RIGHT]b */
				pNewFont->SetRotation(0.7853982f);
				edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[RIGHT]b");
				pNewFont->SetRotation(0.0f);
				*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
			}

			if (uVar2 == 0xc) {
				/* %[DOWN]b */
				pNewFont->SetRotation(0.7853982f);
				edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[DOWN]b");
				pNewFont->SetRotation(0.0f);
				*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
			}

			uVar3 = uVar3 + 1;
			uVar1 = uVar1 + 4;
		} while (uVar3 < pPattern->nbInputs);
	}

	edTextStyleSetCurrent(pNewFont_00);

	return;
}

bool CBehaviourNativAkasa::FUN_003ede90(s_fighter_combo* pCombo, float* pOutWidth, int param_4)
{
	byte bVar1;
	CActorHero* pHero;
	byte cVar3;
	edCTextStyle* pTextStyle;
	bool bVar5;
	float y;
	edCTextStyle textStyle;
	float textScale;

	pTextStyle = edTextStyleGetCurrent();
	textStyle = *pTextStyle;
	pTextStyle = edTextStyleSetCurrent(&textStyle);

	pHero = CActorHero::_gThis;

	textStyle.GetScale(&textScale, &textScale);
	bVar5 = true;
	textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;

	s_fighter_move* pMove = LOAD_SECTION_CAST(s_fighter_move*, pCombo->actionHash.pData);
	bVar1 = pMove->field_0x4.field_0x1byte;
	cVar3 = (bVar1 & 1) != 0;

	y = (float)gVideoConfig.screenHeight * 0.92f - (float)gVideoConfig.screenHeight * 0.07f * textScale;

	if ((bVar1 & 2) != 0) {
		cVar3 = cVar3 + 2;
	}

	if ((bVar1 & 0x20) != 0) {
		cVar3 = cVar3 + 4;
	}

	if ((bVar1 & 0x40) != 0) {
		cVar3 = cVar3 + 8;
	}

	if (cVar3 == 8) {
		if ((param_4 == 1) && ((pHero->comboFlags & 0x40) == 0)) {
			textStyle.SetScale(0.8f, 0.8f);
			textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
			textStyle.altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
			textStyle.alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
			textStyle.GetScale(&textScale, &textScale);
			bVar5 = false;
		}
		else {
			textStyle.SetScale(1.0f, 1.0f);
			textStyle.GetScale(&textScale, &textScale);
		}

		edTextDraw(*pOutWidth, y, gMessageManager.get_message(0x9101806180e064f));
	}
	else {
		if (cVar3 == 4) {
			if ((param_4 == 1) && ((pHero->comboFlags & 0x20) == 0)) {
				textStyle.SetScale(0.8f, 0.8f);
				textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
				textStyle.altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
				textStyle.alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
				textStyle.GetScale(&textScale, &textScale);
				bVar5 = false;
			}
			else {
				textStyle.SetScale(1.0f, 1.0f);
				textStyle.GetScale(&textScale, &textScale);
			}

			edTextDraw(*pOutWidth, y, gMessageManager.get_message(0x910060a1912524f));
		}
		else {
			if (cVar3 == 2) {
				if ((param_4 == 1) && ((pHero->comboFlags & 2) == 0)) {
					textStyle.SetScale(0.8f, 0.8f);
					textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
					textStyle.altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
					textStyle.alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
					textStyle.GetScale(&textScale, &textScale);
					bVar5 = false;
				}
				else {
					textStyle.SetScale(1.0f, 1.0f);
					textStyle.GetScale(&textScale, &textScale);
				}

				edTextDraw(*pOutWidth, y, gMessageManager.get_message(0x9100c1d1008064f));
			}
			else {
				if (cVar3 == 1) {
					if ((param_4 == 1) && ((pHero->comboFlags & 1) == 0)) {
						textStyle.SetScale(0.8f, 0.8f);
						textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
						textStyle.altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
						textStyle.alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
						textStyle.GetScale(&textScale, &textScale);
						bVar5 = false;
					}
					else {
						textStyle.SetScale(1.0f, 1.0f);
						textStyle.GetScale(&textScale, &textScale);
					}

					edTextDraw(*pOutWidth, y, gMessageManager.get_message(0x910080e1c0d524f));
				}
			}
		}
	}

	edTextStyleSetCurrent(pTextStyle);

	return bVar5;
}

void CBehaviourNativAkasa::DrawButton(uint flags, float* pOutWidth, int param_4, int param_5)
{
	edCTextStyle* pTextStyle;
	edCTextStyle textStyle;
	float textScale;

	pTextStyle = edTextStyleGetCurrent();
	textStyle = *pTextStyle;
	pTextStyle = edTextStyleSetCurrent(&textStyle);

	if ((param_4 == 1) && (param_5 == 0)) {
		textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
		textStyle.altColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x80808000;
		textStyle.alpha = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0x40404000;
	}

	textStyle.GetScale(&textScale, &textScale);

	if ((flags & 1) != 0) {
		edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[ACTION]b");
		*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
	}

	if ((flags & 2) != 0) {
		edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[BACK]b");
		*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
	}

	if ((flags & 4) != 0) {
		edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[CATCH]b");
		*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
	}

	if ((flags & 8) != 0) {
		edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[VALID]b");
		*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
	}

	if ((flags & 0x10) != 0) {
		edTextDraw(*pOutWidth, (float)gVideoConfig.screenHeight * 0.92f, "%[STIK_ROT]b");
		*pOutWidth = *pOutWidth + (float)gVideoConfig.screenWidth * 0.08f;
	}

	edTextStyleSetCurrent(pTextStyle);

	return;
}

void CBehaviourNativAkasa::FUN_003ed820(s_fighter_combo* pCombo, float* param_3, int param_4)
{
	bool bVar1;
	edCTextStyle* pTextStyle;
	edDList_material* pMaterial;
	bool uVar3;
	edCTextStyle textStyle;
	edF32VECTOR2 local_20;
	edF32VECTOR2 local_18;
	float local_c;
	float fStack8;
	float local_4;

	if (pCombo != (s_fighter_combo*)0x0) {
		GetComboButtonDisplaySize(pCombo, &local_4, &fStack8);
		local_18.x = *param_3 + (local_4 - (float)gVideoConfig.screenWidth * 0.068f) / 2.0f;
		local_18.y = (float)gVideoConfig.screenHeight * 0.92f - (float)gVideoConfig.screenHeight * 0.008f;
		pTextStyle = edTextStyleGetCurrent();
		textStyle = *pTextStyle;
		textStyle.GetScale(&local_c, &local_c);
		bVar1 = GuiDList_BeginCurrent();
		if (bVar1 != false) {
			local_20.x = local_4 * local_c;
			local_20.y = (float)gVideoConfig.screenHeight * 0.075f * local_c;
			pMaterial = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->field_0xc, 0);
			DrawInputBorder(&local_18, &local_20, reinterpret_cast<_rgba*>(&this->field_0x16b8), pMaterial, 0);
			DrawInventoryCrouch(pCombo->pattern.field_0x3byte, param_3);
			DrawDirectionArrow(&pCombo->pattern, param_3);
			uVar3 = FUN_003ede90(pCombo, param_3, param_4);
			DrawButton(((ulong)pCombo->pattern.field_0x2ushort << 0x34) >> 0x38, param_3, param_4, uVar3);
			GuiDList_EndCurrent();
		}
	}
	return;
}

void CBehaviourNativAkasa::DrawButtonPromptA()
{
	int iVar1;
	bool bVar2;
	edCTextStyle* pPrevTextStyle;
	int inputState;
	ArenaTutorial* pNVar4;
	int iVar5;
	long lVar6;
	float screenHeight;
	float fVar8;
	float fVar9;
	float fVar10;
	edCTextStyle textStyle;
	float local_4;

	pNVar4 = GetActiveComboTutorial();

	local_4 = (float)gVideoConfig.screenWidth * 0.13f;
	textStyle.Reset();
	textStyle.SetFont(BootDataFont, false);
	textStyle.SetHorizontalAlignment(2);
	textStyle.SetVerticalAlignment(8);
	textStyle.rgbaColour = (int)(this->comboDisplayAlpha * 255.0f) & 0xffU | 0xffffff00;
	textStyle.SetShadow(0x100);

	pPrevTextStyle = edTextStyleSetCurrent(&textStyle);
	lVar6 = 0;
	if ((pNVar4 != (ArenaTutorial*)0x0) && (CActorHero::_gThis != (CActorHero*)0x0)) {
		fVar8 = this->comboDisplayWidth;
		screenHeight = (float)gVideoConfig.screenHeight;
		fVar9 = this->comboDisplayHeight;
		fVar10 = fVar8 / 2.0f - (float)gVideoConfig.screenWidth * 0.02f;
		bVar2 = GuiDList_BeginCurrent();
		if (bVar2 != false) {
			CPauseManager::DrawRectangleBorder(fVar10, screenHeight * 1.04f - fVar9 / 2.0f, fVar8, fVar9, (float)gVideoConfig.screenWidth * 0.01f, (float)gVideoConfig.screenHeight * 0.01f, 0x40101030, 0, 0);
			GuiDList_EndCurrent();
		}

		CActorHero::_gThis->_UpdateComboSituation();

		iVar5 = 0;
		if (0 < pNVar4->nbRequiredMoves) {
			do {
				s_fighter_combo* pCombo = pNVar4->aRequiredMoves[iVar5].aRequiredCombos[0];

				inputState = GetInputState(iVar5, lVar6);
				SetupTextStyle(inputState, &textStyle);
				DrawButton(pCombo, &local_4, inputState);

				if (inputState == 4) {
					lVar6 = 1;
				}

				FUN_003ed820(pCombo, &local_4, inputState);
				iVar5 = iVar5 + 1;
				local_4 = local_4 + (float)gVideoConfig.screenWidth * 0.05f;
			} while (iVar5 < pNVar4->nbRequiredMoves);
		}
	}

	edTextStyleSetCurrent(pPrevTextStyle);

	return;
}

ArenaTutorial* CBehaviourNativAkasa::GetActiveComboTutorial()
{
	ArenaTutorial* pReqCombo;
	const int index = (this->comboTutorialManager).activeTutorialIndex;
	if (index == -1) {
		pReqCombo = (ArenaTutorial*)0x0;
	}
	else {
		pReqCombo = (this->comboTutorialManager).aTutorials + index;
	}

	return pReqCombo;
}

NativSubObjB::NativSubObjB()
{
	this->nbRequiredCombos = 0;
	this->aRequiredCombos[0] = (s_fighter_combo*)0x0;
	this->aRequiredCombos[1] = (s_fighter_combo*)0x0;
	this->field_0xc = 0;

	return;
}

void NativSubObjB::Create(ByteCode* pByteCode)
{
	int iVar2;
	uint* puVar4;
	s_fighter_combo** pCombo;

	this->nbRequiredCombos = pByteCode->GetS32();
	assert(this->nbRequiredCombos <= 2);

	iVar2 = 0;
	pCombo = this->aRequiredCombos;
	if (0 < this->nbRequiredCombos) {
		do {
			*pCombo = CActorHero::_gThis->FindComboByName(pByteCode->GetString());
			iVar2 = iVar2 + 1;
			pCombo = pCombo + 1;
		} while (iVar2 < this->nbRequiredCombos);
	}

	return;
}

bool NativSubObjB::IsRequiredCombo(s_fighter_combo* pCombo)
{
	int iVar2;

	iVar2 = 0;
	if (0 < this->nbRequiredCombos) {
		do {
			if (this->aRequiredCombos[iVar2] == pCombo) {
				return true;
			}

			iVar2 = iVar2 + 1;
		} while (iVar2 < this->nbRequiredCombos);
	}

	return false;
}

ArenaTutorial::ArenaTutorial()
{
	this->nbRequiredMoves = 0;
	this->aRequiredMoves = (NativSubObjB*)0x0;

	return;
}

NativSellerSubObjA::NativSellerSubObjA()
{
	this->field_0x88 = 0;
	this->field_0x8c = 0;

	return;
}

ComboTutorialManager::ComboTutorialManager()
{
	this->nbTutorials = 0;
	this->aTutorials = (ArenaTutorial*)0x0;

	return;
}

void ComboTutorialManager::Init()
{
	int iVar3;
	ArenaTutorial* pTutorial;
	int curTutorialIndex;

	curTutorialIndex = 0;
	if (0 < this->nbTutorials) {
		do {
			pTutorial = this->aTutorials + curTutorialIndex;
			iVar3 = 0;
			if (0 < pTutorial->nbRequiredMoves) {
				do {
					pTutorial->aRequiredMoves[iVar3].field_0xc = 0;
					iVar3 = iVar3 + 1;
				} while (iVar3 < pTutorial->nbRequiredMoves);
			}

			curTutorialIndex = curTutorialIndex + 1;
		} while (curTutorialIndex < this->nbTutorials);
	}

	this->activeTutorialIndex = -1;

	return;
}

int ComboTutorialManager::StepRequiredCombo()
{
	this->activeTutorialIndex = this->activeTutorialIndex + 1;
	if (this->nbTutorials <= this->activeTutorialIndex) {
		this->activeTutorialIndex = -1;
	}

	return this->activeTutorialIndex;
}

NativSellerSubObjA* NativSubObjD::Set_0x1630(int param_2)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourNativSeller::Create(ByteCode* pByteCode)
{
	this->field_0x8 = pByteCode->GetF32();
	this->addOn.Create(pByteCode);

	return;
}

void CBehaviourNativSeller::Init(CActor* pOwner)
{
	this->cachedStateId = -1;
	this->pOwner = static_cast<CActorNativ*>(pOwner);

	this->addOn.Init(pOwner);

	this->pNativShop = (CActorNativShop*)0x0;

	this->addOn.Reset();
	this->addOn.field_0xd = 1;

	return;
}

void CBehaviourNativSeller::Manage()
{
	this->pOwner->BehaviourNativSeller_Manage(this);

	return;
}

void CBehaviourNativSeller::ManageFrozen()
{
	this->addOn.Manage();

	return;
}

void CBehaviourNativSeller::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorNativ*>(pOwner);
	if (newState == -1) {
		if (this->cachedStateId == -1) {
			this->pOwner->SetState(0x1a, -1);
		}
		else {
			this->pOwner->SetState(this->cachedStateId, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourNativSeller::End(int newBehaviourId)
{
	return;
}

void CBehaviourNativSeller::InitState(int newState)
{
	this->pOwner->BehaviourNativSeller_InitState(newState);

	return;
}

void CBehaviourNativSeller::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourNativSeller_TermState(oldState);
}

int CBehaviourNativSeller::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	CActorNativ* pNativ;
	undefined4 uVar3;
	int iVar4;
	long lVar5;

	if (msg == 0x60) {
		int* pMsg = reinterpret_cast<int*>(pMsgParam);
		iVar4 = 1;
		iVar1 = *pMsg;
		if (iVar1 == 1) {
			if (this->addOn.Func_0x20(0x25, 0, 0) == 0) {
				pNativ = this->pOwner;
				if (pNativ->curBehaviourId == 1) {
					this->cachedStateId = 0x1c;
				}
				else {
					pNativ->SetState(0x1c, -1);
				}
			}
			else {
				this->cachedStateId = 0x1c;
			}

			iVar4 = 1;
		}
		else {
			if (iVar1 == 2) {
				if (this->addOn.Func_0x20(0x24, 0, 0) != 0) {
					this->cachedStateId = 0x1d;
				}

				iVar4 = 1;
			}
			else {
				if (iVar1 == 3) {
					if (this->addOn.Func_0x20(GetPurchaseCutsceneId(pMsg[1]), 0, 1) != 0) {
						this->cachedStateId = 0x1d;
					}
					iVar4 = 1;
				}
				else {
					if (iVar1 == 0) {
						if (this->addOn.Func_0x20(3, 0, 0) == 0) {
							pNativ = this->pOwner;
							if (pNativ->curBehaviourId == 1) {
								this->cachedStateId = 0x1d;
							}
							else {
								pNativ->SetState(0x1d, -1);
							}
						}
						else {
							this->cachedStateId = 0x1d;
						}

						iVar4 = 1;
					}
					else {
						if ((iVar1 == 4) && (pSender->typeID == 0x34)) {
							this->pNativShop = static_cast<CActorNativShop*>(pSender);
						}
						else {
							iVar4 = 0;
						}
					}
				}
			}
		}
	}
	else {
		iVar4 = 0;
	}
	return iVar4;
}

float CBehaviourNativSeller::GetWayPointDistance()
{
	CActor* pActor;
	CActorNativ* pCVar1;
	float fVar2;
	float fVar3;
	CActorHero* pCVar4;
	CBehaviourNativShopSell* pCVar5;
	edF32VECTOR4* peVar6;
	float fVar7;
	edF32VECTOR4 local_10;

	pCVar4 = CActorHero::_gThis;
	pActor = (CActor*)this->pNativShop;
	if ((pActor == (CActor*)0x0) || (pActor->curBehaviourId != 2)) {
		pCVar1 = this->pOwner;
		fVar7 = CActorHero::_gThis->currentLocation.x - pCVar1->currentLocation.x;
		fVar2 = CActorHero::_gThis->currentLocation.y - pCVar1->currentLocation.y;
		fVar3 = CActorHero::_gThis->currentLocation.z - pCVar1->currentLocation.z;
		fVar7 = sqrtf(fVar7 * fVar7 + fVar2 * fVar2 + fVar3 * fVar3);
	}
	else {
		pCVar5 = static_cast<CBehaviourNativShopSell*>(pActor->GetBehaviour(pActor->curBehaviourId));
		local_10 = *pCVar5->pathFollowReader.GetWayPoint(0);
		edF32Vector4SubHard(&local_10, &local_10, &pCVar4->currentLocation);
		local_10.y = 0.0f;
		fVar7 = edF32Vector4GetDistHard(&local_10);
	}

	return fVar7;
}

void CBehaviourNativSeller::FUN_003f3070()
{
	int iVar1;

	iVar1 = 0;
	do {
		this->addOn.Func_0x24(iVar1, (CActor*)0x0);
		iVar1 = iVar1 + 1;
	} while (iVar1 < 0x26);
	return;
}

void CBehaviourNativSeller::FUN_003f3020()
{
	int iVar1;

	iVar1 = 0;
	do {
		this->addOn.ClearCinematic(iVar1);
		iVar1 = iVar1 + 1;
	} while (iVar1 < 0x26);
	return;
}

int CBehaviourNativSeller::GetPurchaseCutsceneId(int objectId)
{
	if (objectId < 0x20) {
		switch (objectId) {
		case 0:
			return 0x4;
		case 1:
			return 0x5;
		case 2:
			return 0x6;
		case 3:
			return 0x7;
		case 4:
			return 0x8;
		case 5:
			return 0x9;
		case 6:
			return 0xa;
		case 7:
			return 0xe;
		case 8:
			return 0xb;
		case 9:
			return 0xc;
		case 10:
			return 0xd;
		case 11:
			return 0xf;
		case 12:
			return 0x10;
		case 13:
			return 0x11;
		case 14:
			return 0x12;
		case 15:
			return 0x13;
		case 16:
			return 0x14;
		case 17:
			return 0x15;
		case 18:
			return 0x16;
		case 19:
			return 0x17;
		case 20:
			return 0x18;
		case 21:
			return 0x19;
		case 22:
			return 0x1a;
		case 23:
			return 0x1b;
		case 24:
			return 0x1c;
		case 25:
			return 0x1d;
		case 26:
			return 0x1e;
		case 27:
			return 0x1f;
		case 28:
			return 0x20;
		case 29:
			return 0x21;
		case 30:
			return 0x22;
		case 31:
			return 0x23;
		}
	}

	return -1;
}