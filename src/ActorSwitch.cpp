#include "ActorSwitch.h"
#include "MemoryStream.h"
#include "CinematicManager.h"
#include "CollisionManager.h"
#include "MathOps.h"
#include "FileManager3D.h"
#include "CameraViewManager.h"

StateConfig CActorSwitch::_gStateCfg_SWT[5] = {
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
};

void CActorSwitch::Create(ByteCode* pByteCode)
{
	S_TARGET_STREAM_REF* pSVar1;
	S_STREAM_EVENT_CAMERA* pSVar2;
	CBehaviour* pCVar3;
	int* piVar4;

	CActor::Create(pByteCode);

	pSVar1 = (S_TARGET_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)pSVar1->aEntries;
	if (pSVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
	}
	this->pTargetStreamRef = pSVar1;

	pSVar2 = (S_STREAM_EVENT_CAMERA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pSVar2 + 1);
	this->pStreamEventCamera = pSVar2;

	pCVar3 = CActor::GetBehaviour(2);
	if (pCVar3 == (CBehaviour*)0x0) {
		this->pActorSound = (CActorSound*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD_AUDIO(
		piVar4 = CActor::CreateActorSound((CActor*)this, 1);
		this->pActorSound = piVar4;)
	}

	return;
}

void CActorSwitch::Init()
{
	for (int i = 0; i < this->pTargetStreamRef->entryCount; i++) {
		this->pTargetStreamRef->aEntries[i].Init();
	}

	this->pStreamEventCamera->Init();
	CActor::Init();

	return;
}

void CActorSwitch::Reset()
{
	CActor::Reset();

	for (int i = 0; i < this->pTargetStreamRef->entryCount; i++) {
		this->pTargetStreamRef->aEntries[i].Reset();
	}

	this->pStreamEventCamera->Reset(this);

	return;
}

CBehaviour* CActorSwitch::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType < 9) {
		switch (behaviourType) {
		case 0:
		case 1:
			pBehaviour = CActor::BuildBehaviour(behaviourType);
			break;
		case 2:
			pBehaviour = new CBehaviourSwitchLever;
			break;
		case 3:
			pBehaviour = new CBehaviourSwitchMagic;
			break;
		case 4:
			pBehaviour = new CBehaviourSwitchWolfenCounter;
			break;
		case 5:
			pBehaviour = &this->behaviourSwitchMultiCondition;
			break;
		case 6:
			pBehaviour = new CBehaviourSwitchTarget;
			break;
		case 7:
			pBehaviour = &this->behaviourSwitchSequence;
			break;
		case 8:
			pBehaviour = new CBehaviourSwitchNew;
			break;
		}

		return pBehaviour;
	}

	pBehaviour = CActor::BuildBehaviour(behaviourType);
	return pBehaviour;
}

StateConfig* CActorSwitch::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_SWT + state + -5;
	}

	return pStateConfig;
}

void CActorSwitch::ChangeManageState(int state)
{
	CBehaviourSwitch* pCVar1;

	CActor::ChangeManageState(state);

	switch (this->curBehaviourId) {
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		pCVar1 = static_cast<CBehaviourSwitch*>(GetBehaviour(this->curBehaviourId));
		if (pCVar1 != (CBehaviourSwitch*)0x0) {
			pCVar1->ChangeManageState(state);
		}
	}

	return;
}

int CActorSwitch::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return CActor::InterpretMessage(pSender, msg, pMsgParam);
}

void CActorSwitch::BehaviourSwitchMagic_Manage(CBehaviourSwitchMagic* pBehaviour)
{
	CActor* pCVar1;
	edNODE* peVar2;
	bool bVar3;
	int iVar4;
	float* pfVar6;
	ed_3d_hierarchy_node* peVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	float local_20;
	float local_1c;
	float local_18;
	float local_14;
	edF32VECTOR4 local_10;

	this->pStreamEventCamera->Manage(this);
	pBehaviour->pStreamEventCamera->Manage(this);
	iVar4 = this->actorState;
	if (iVar4 == 8) {
		IMPLEMENTATION_GUARD(
		FUN_00169530(this, pBehaviour);)
	}
	else {
		if (iVar4 == 6) {
			IMPLEMENTATION_GUARD(
			StateSwitchMagicOff2On(this, pBehaviour);)
		}
	}

	if (this->pTiedActor != (CActor*)0x0) {
		local_10 = this->baseLocation;

		SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		pCVar1 = pBehaviour->field_0x20;
		if (((pCVar1 == (CActor*)0x0) || (pBehaviour->field_0x1c == (CinNamedObject30*)0x0)) ||
			(bVar3 = true, pBehaviour->field_0x1c != pCVar1->pCinData)) {
			bVar3 = false;
		}

		if (bVar3) {
			if (((pCVar1 != (CActor*)0x0) && (pBehaviour->field_0x1c != (CinNamedObject30*)0x0)) &&
				(pBehaviour->field_0x1c == pCVar1->pCinData)) {
				pCVar1->currentLocation = this->currentLocation;
			}

			pCVar1 = pBehaviour->field_0x20;
			if (((pCVar1 != (CActor*)0x0) && (pBehaviour->field_0x1c != (CinNamedObject30*)0x0)) &&
				(pBehaviour->field_0x1c == pCVar1->pCinData)) {
				pCVar1->rotationEuler = this->rotationEuler;
			}
		}

		if (pBehaviour->pHierarchy != (edNODE*)0x0) {
			ed_3d_hierarchy* peVar5 = reinterpret_cast<ed_3d_hierarchy*>(pBehaviour->pHierarchy->pData);
			peVar5->transformA = this->pMeshTransform->base.transformA;
		}

		pCVar1 = pBehaviour->field_0x38;
		if (((pCVar1 == (CActor*)0x0) || (pBehaviour->field_0x34 == (CinNamedObject30*)0x0)) ||
			(bVar3 = true, pBehaviour->field_0x34 != pCVar1->pCinData)) {
			bVar3 = false;
		}

		if (bVar3) {
			if (((pCVar1 != (CActor*)0x0) && (pBehaviour->field_0x34 != (CinNamedObject30*)0x0)) &&
				(pBehaviour->field_0x34 == pCVar1->pCinData)) {
				pCVar1->currentLocation = this->currentLocation;
			}

			pCVar1 = pBehaviour->field_0x38;
			if (((pCVar1 != (CActor*)0x0) && (pBehaviour->field_0x34 != (CinNamedObject30*)0x0)) &&
				(pBehaviour->field_0x34 == pCVar1->pCinData)) {
				pCVar1->rotationEuler = this->rotationEuler;

			}
		}

		if (pBehaviour->field_0x44 != 0) {
			IMPLEMENTATION_GUARD(
			pfVar6 = *(float**)(pBehaviour->field_0x44 + 0xc);
			iVar4 = 8;
			peVar7 = this->pMeshTransform;
			do {
				iVar4 = iVar4 + -1;
				fVar8 = (peVar7->base).transformA.ab;
				*pfVar6 = (peVar7->base).transformA.aa;
				peVar7 = (ed_3d_hierarchy_node*)&(peVar7->base).transformA.ac;
				pfVar6[1] = fVar8;
				pfVar6 = pfVar6 + 2;
			} while (0 < iVar4);)
		}

		pCVar1 = pBehaviour->field_0x54;
		if (((pCVar1 == (CActor*)0x0) || (pBehaviour->field_0x50 == (CinNamedObject30*)0x0)) ||
			(bVar3 = true, pBehaviour->field_0x50 != pCVar1->pCinData)) {
			bVar3 = false;
		}

		if (bVar3) {
			local_20 = this->currentLocation.x;
			local_18 = this->currentLocation.z;
			local_14 = this->currentLocation.w;
			local_1c = this->currentLocation.y + pBehaviour->field_0x14;
			if (((pCVar1 != (CActor*)0x0) && (pBehaviour->field_0x50 != (CinNamedObject30*)0x0)) &&
				(pBehaviour->field_0x50 == pCVar1->pCinData)) {
				(pCVar1->currentLocation).x = local_20;
				(pCVar1->currentLocation).y = local_1c;
				(pCVar1->currentLocation).z = local_18;
				(pCVar1->currentLocation).w = local_14;
			}
			pCVar1 = pBehaviour->field_0x54;
			if (((pCVar1 != (CActor*)0x0) && (pBehaviour->field_0x50 != (CinNamedObject30*)0x0)) &&
				(pBehaviour->field_0x50 == pCVar1->pCinData)) {
				pCVar1->rotationEuler = this->rotationEuler;
			}
		}
	}

	if (((pBehaviour->field_0x54 == (CActor*)0x0) || (pBehaviour->field_0x50 == (CinNamedObject30*)0x0)) ||
		(pBehaviour->field_0x50 != pBehaviour->field_0x54->pCinData)) {
		bVar3 = false;
	}
	else {
		bVar3 = true;
	}

	if (bVar3) {
		local_30.x = this->currentLocation.x;
		local_30.z = this->currentLocation.z;
		local_30.w = this->currentLocation.w;
		local_30.y = this->currentLocation.y + pBehaviour->field_0x14;
		edF32Vector4SubHard(&eStack64, &local_30, &CCameraManager::_gThis->transformationMatrix.rowT);

		fVar8 = edF32Vector4GetDistHard(&eStack64);
		if (fVar8 <= 3.0f) {
			fVar9 = 0.0f;
		}
		else {
			fVar9 = 1.0f;
			if (fVar8 < 7.0f) {
				fVar9 = (fVar8 - 3.0f) / 4.0f;
			}
		}

		pCVar1 = pBehaviour->field_0x54;
		if (((pCVar1 != (CActor*)0x0) && (pBehaviour->field_0x50 != (CinNamedObject30*)0x0)) &&
			(pBehaviour->field_0x50 == pCVar1->pCinData)) {
			pCVar1->Reset();
		}
	}
	return;
}

CBehaviourSwitchMagic::CBehaviourSwitchMagic()
{
	//this->field_0x1c = 0;
	//this->field_0x20 = 0;
	//this->field_0x34 = 0;
	//this->field_0x38 = 0;
	(this->fxDigits).field_0x0 = (ParticleInfo*)0x0;
	//this->field_0x50 = 0;
	//this->field_0x54 = 0;
	this->pStreamEventCamera = (S_STREAM_EVENT_CAMERA*)0x0;
}

void CBehaviourSwitchMagic::Create(ByteCode* pByteCode)
{
	S_STREAM_EVENT_CAMERA* pSVar1;

	this->field_0x8 = pByteCode->GetF32();

	this->field_0x10 = pByteCode->GetF32();
	if (0.0f < this->field_0x10) {
		this->field_0x10 = this->field_0x8 / this->field_0x10;
	}
	else {
		this->field_0x10 = this->field_0x8;
	}

	this->fxDigits.Init(pByteCode->GetS32());
	this->field_0x14 = pByteCode->GetF32();
	this->field_0x4c = pByteCode->GetS32();
	this->field_0x18 = pByteCode->GetS32();
	this->field_0x28 = pByteCode->GetS32();
	this->field_0x24 = pByteCode->GetS32();
	this->field_0x30 = pByteCode->GetS32();
	this->field_0x40 = pByteCode->GetS32();
	this->field_0x3c = pByteCode->GetS32();

	pSVar1 = (S_STREAM_EVENT_CAMERA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pSVar1 + 1);
	this->pStreamEventCamera = pSVar1;

	this->pHierarchy = (edNODE*)0x0;
	this->field_0x44 = 0;

	return;

}

void CBehaviourSwitchMagic::Init(CActor* pOwner)
{
	this->pStreamEventCamera->Init();

	return;
}

void CBehaviourSwitchMagic::Manage()
{
	this->pOwner->BehaviourSwitchMagic_Manage(this);
	return;
}

void CBehaviourSwitchMagic::Draw()
{
	CActorSwitch* pCVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	if (this->field_0xc < this->field_0x8) {
		pCVar1 = this->pOwner;
		local_10.x = pCVar1->currentLocation.x;
		local_10.z = pCVar1->currentLocation.z;
		local_10.w = pCVar1->currentLocation.w;
		local_10.y = pCVar1->currentLocation.y + this->field_0x14;

		edF32Vector4SubHard(&eStack32, &local_10, &CCameraManager::_gThis->transformationMatrix.rowT);

		fVar2 = edF32Vector4GetDistHard(&eStack32);
		if (fVar2 <= 3.0f) {
			fVar3 = 0.0f;
		}
		else {
			fVar3 = 1.0f;
			if (fVar2 < 7.0f) {
				fVar3 = (fVar2 - 3.0f) / 4.0f;
			}
		}

		this->fxDigits.Draw(this->field_0x8, this->field_0xc, 1.0f, fVar3, &local_10, this->pOwner->actorState == 6);
	}
	return;
}

static edF32VECTOR4 gMagicSwitchBoundingSphere = { 0.0f, 0.0f, 0.0f, 1.0f };

void CBehaviourSwitchMagic::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorSwitch* pCVar1;
	edF32VECTOR4 local_10;

	this->pOwner = static_cast<CActorSwitch*>(pOwner);

	this->field_0xc = 0.0f;

	local_10.x = gMagicSwitchBoundingSphere.x;
	local_10.z = gMagicSwitchBoundingSphere.z;
	local_10.w = gMagicSwitchBoundingSphere.w;
	local_10.y = this->field_0x14;
	this->pOwner->SetLocalBoundingSphere(fabs(this->field_0x14 * 2.0f), &local_10);

	this->field_0x1c = 0;
	this->field_0x20 = 0;
	this->field_0x34 = 0;
	this->field_0x38 = 0;
	this->field_0x50 = 0;
	this->field_0x54 = 0;

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	if ((this->pOwner->flags & 4) != 0) {
		this->ChangeManageState(1);
	}

	this->pStreamEventCamera->Reset(this->pOwner);

	return;
}

void CBehaviourSwitchMagic::InitState(int newState)
{
	if (newState == 6) {
		this->pStreamEventCamera->SwitchOn(this->pOwner);
	}

	return;
}

void CBehaviourSwitchMagic::ChangeManageState(int state)
{
	CActor* pCVar1;
	CActorSwitch* pCVar2;
	CinNamedObject30* pCVar3;
	bool bVar4;
	ed_g3d_manager* pG3D;
	edNODE* peVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	int iVar9;
	long lVar10;
	ed_3d_hierarchy* peVar11;
	ed_3d_hierarchy_node* peVar12;
	float fVar13;

	if (state == 0) {
		pCVar1 = this->field_0x20;
		
		if (((pCVar1 == (CActor*)0x0) || (this->field_0x1c == (CinNamedObject30*)0x0)) ||
			(bVar4 = true, this->field_0x1c != pCVar1->pCinData)) {
			bVar4 = false;
		}

		if (bVar4) {
			if (((pCVar1 != (CActor*)0x0) && (this->field_0x1c != (CinNamedObject30*)0x0)) &&
				(this->field_0x1c == pCVar1->pCinData)) {
				pCVar1->IsKindOfObject(0);
			}

			this->field_0x20 = (CActor*)0x0;
			this->field_0x1c = (CinNamedObject30*)0x0;
		}

		this->field_0x1c = (CinNamedObject30*)0x0;
		this->field_0x20 = (CActor*)0x0;

		peVar5 = this->pHierarchy;
		lVar10 = (long)(int)peVar5;
		if (lVar10 != 0) {
			ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, peVar5);
			peVar5 = (edNODE*)lVar10;
			this->pHierarchy = (edNODE*)0x0;
		}

		pCVar1 = this->field_0x38;
		if (((pCVar1 == (CActor*)0x0) || (this->field_0x34 == (CinNamedObject30*)0x0)) ||
			(bVar4 = true, this->field_0x34 != pCVar1->pCinData)) {
			bVar4 = false;
		}

		if (bVar4) {
			if (((pCVar1 != (CActor*)0x0) && (this->field_0x34 != (CinNamedObject30*)0x0)) &&
				(this->field_0x34 == pCVar1->pCinData)) {
				pCVar1->IsKindOfObject(0);
			}

			this->field_0x38 = (CActor*)0x0;
			this->field_0x34 = (CinNamedObject30*)0x0;
		}

		this->field_0x34 = (CinNamedObject30*)0x0;
		this->field_0x38 = (CActor*)0x0;

		IMPLEMENTATION_GUARD_FX(
		DisplayOpenFxModel(0);)

		pCVar1 = this->field_0x54;
		if (((pCVar1 == (CActor*)0x0) || (this->field_0x50 == (CinNamedObject30*)0x0)) ||
			(bVar4 = true, this->field_0x50 != pCVar1->pCinData)) {
			bVar4 = false;
		}
		if (bVar4) {
			if (((pCVar1 != (CActor*)0x0) && (pCVar3 = this->field_0x50, pCVar3 != (CinNamedObject30*)0x0)) &&
				(pCVar3 == pCVar1->pCinData)) {
				pCVar1->IsKindOfObject(0);
			}

			this->field_0x54 = (CActor*)0x0;
			this->field_0x50 = (CinNamedObject30*)0x0;
		}
		this->field_0x50 = (CinNamedObject30*)0x0;
		this->field_0x54 = (CActor*)0x0;
	}
	else {
		if (this->field_0xc < this->field_0x8) {
			if (this->field_0x18 != 0xffffffff) {
				if (((this->field_0x20 == (CActor*)0x0) || (this->field_0x1c == (CinNamedObject30*)0x0)) ||
					(this->field_0x1c != this->field_0x20->pCinData)) {
					bVar4 = false;
				}
				else {
					bVar4 = true;
				}
				if (!bVar4) {
					IMPLEMENTATION_GUARD_FX(
					CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x1c, this->field_0x18, 0xffffffffffffffff);)
				}
			}

			pCVar1 = this->field_0x20;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x1c == (CinNamedObject30*)0x0)) ||
				(bVar4 = true, this->field_0x1c != pCVar1->pCinData)) {
				bVar4 = false;
			}

			if (bVar4) {
				pCVar2 = this->pOwner;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1c != (CinNamedObject30*)0x0)) &&
					(this->field_0x1c == pCVar1->pCinData)) {
					pCVar1->currentLocation = pCVar2->currentLocation;
				}

				pCVar2 = this->pOwner;
				pCVar1 = this->field_0x20;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1c != (CinNamedObject30*)0x0)) &&
					(this->field_0x1c == pCVar1->pCinData)) {
					pCVar1->rotationEuler = pCVar2->rotationEuler;
				}

				pCVar1 = this->field_0x20;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1c != (CinNamedObject30*)0x0)) &&
					(this->field_0x1c == pCVar1->pCinData)) {
					pCVar1->InitDlistPatchable();
				}
			}
			if (((this->pHierarchy == (edNODE*)0x0) && (this->field_0x24 != -1)) && (this->field_0x28 != -1)) {
				pG3D = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(this->field_0x24, this->field_0x28);
				peVar5 = ed3DHierarchyAddToScene(CScene::_scene_handleA, pG3D, (char*)0x0);
				this->pHierarchy = peVar5;
				if (this->pHierarchy != (edNODE*)0x0) {
					peVar11 = reinterpret_cast<ed_3d_hierarchy*>(this->pHierarchy->pData);

					peVar11->transformA = this->pOwner->pMeshTransform->base.transformA;
				}
			}

			if (this->field_0x4c != 0xffffffff) {
				if (((this->field_0x54 == (CActor*)0x0) || (this->field_0x50 == (CinNamedObject30*)0x0)) ||
					(this->field_0x50 != this->field_0x54->pCinData)) {
					bVar4 = false;
				}
				else {
					bVar4 = true;
				}

				if (!bVar4) {
					IMPLEMENTATION_GUARD_FX(
					CParticlesManager::GetDynamicFx
					(CScene::ptable.g_EffectsManager_004516b8, &this->field_0x50, this->field_0x4c, 0xffffffffffffffff);)
				}
			}

			pCVar1 = this->field_0x54;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x50 == (CinNamedObject30*)0x0)) ||
				(bVar4 = true, this->field_0x50 != pCVar1->pCinData)) {
				bVar4 = false;
			}

			if (bVar4) {
				pCVar2 = this->pOwner;
				fVar6 = pCVar2->currentLocation.y;
				fVar7 = pCVar2->currentLocation.z;
				fVar8 = pCVar2->currentLocation.w;
				fVar13 = this->field_0x14;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x50 != (CinNamedObject30*)0x0)) &&
					(this->field_0x50 == pCVar1->pCinData)) {
					(pCVar1->currentLocation).x = pCVar2->currentLocation.x;
					(pCVar1->currentLocation).y = fVar6 + fVar13;
					(pCVar1->currentLocation).z = fVar7;
					(pCVar1->currentLocation).w = fVar8;
				}

				pCVar2 = this->pOwner;
				pCVar1 = this->field_0x54;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x50 != (CinNamedObject30*)0x0)) &&
					(this->field_0x50 == pCVar1->pCinData)) {
					pCVar1->rotationEuler = pCVar2->rotationEuler;

				}

				pCVar1 = this->field_0x54;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x50 != (CinNamedObject30*)0x0)) &&
					(this->field_0x50 == pCVar1->pCinData)) {
					pCVar1->InitDlistPatchable();
				}
			}
		}
	}

	return;
}

void CBehaviourSwitchTarget::Create(ByteCode* pByteCode)
{
	this->field_0x8 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetF32();

	return;
}

void CBehaviourSwitchTarget::Init(CActor* pOwner)
{
	CCollision* pCVar1;
	CActorSwitch* pSwitch;

	this->pOwner = static_cast<CActorSwitch*>(pOwner);

	this->field_0xc = this->field_0xc * 100.0f;
	this->field_0x10 = this->field_0x10 * 100.0f;

	pCVar1 = this->pOwner->pCollisionData;
	if (pCVar1 != (CCollision*)0x0) {
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x800;
		pSwitch = this->pOwner;
		pSwitch->UpdatePosition(&pSwitch->currentLocation, true);
	}

	return;
}

void CBehaviourSwitchTarget::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->field_0x14 = 0.0f;
	this->field_0x18 = 0;

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourSwitchLever::Create(ByteCode* pByteCode)
{
	this->field_0x40.index = pByteCode->GetS32();
	this->field_0x44.index = pByteCode->GetS32();
	return;
}

void CBehaviourSwitchLever::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorSwitch*>(pOwner);

	this->field_0x40.Init();
	this->field_0x44.Init();

	return;
}

void CBehaviourSwitchLever::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	edF32MATRIX4 rotation;

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	edF32Matrix4FromEulerSoft(&rotation, &this->pOwner->rotationEuler.xyz, "XYZ");
	edQuatFromMatrix4(&this->field_0x20, &rotation);

	this->field_0x20 = gF32Vertex4Zero;
	this->field_0x30 = gF32Vertex4Zero;
	this->field_0x10 = gF32Vertex4Zero;

	this->pActor = (CActor*)0x0;
	return;
}

void CBehaviourSwitchLever::InitState(int newState)
{
	CSound* pSound;
	CActorSwitch* pActor;

	if (newState == 9) {
		IMPLEMENTATION_GUARD_AUDIO(
		pSound = (this->field_0x44).pSound;
		if (pSound != (CSound*)0x0) {
			pActor = this->pOwner;
			CActorSound::SoundStart
			((CActorSound*)pActor->pActorSound, (CActor*)pActor, 0, pSound, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
		})
	}
	else {
		if (newState == 6) {
			this->pActor->pAnimationController->RegisterBone(0x414dad8d);
		}
	}

	return;
}

void CBehaviourSwitchMultiCondition::Create(ByteCode* pByteCode)
{
	this->field_0x14 = pByteCode->GetU32();
	this->field_0x8 = pByteCode->GetU32();
	this->field_0xc = pByteCode->GetU32();
	this->field_0x10 = this->field_0x8;

	return;
}

void CBehaviourSwitchMultiCondition::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorSwitch*>(pOwner);

	return;
}

void CBehaviourSwitchMultiCondition::Manage()
{
	CActorSwitch* pActor;
	edF32VECTOR4 local_10;

	pActor = this->pOwner;
	pActor->pStreamEventCamera->Manage(pActor);

	if (pActor->pTiedActor != (CActor*)0x0) {
		local_10 = pActor->baseLocation;
		pActor->SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	return;
}

void CBehaviourSwitchMultiCondition::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->field_0x10 = this->field_0x8;

	if (newState == -1) {
		if (this->field_0x10 == this->field_0xc) {
			this->pOwner->SetState(8, -1);

			for (int i = 0; i < this->pOwner->pTargetStreamRef->entryCount; i++) {
				this->pOwner->pTargetStreamRef->aEntries[i].Switch(this->pOwner);
			}

			this->pOwner->pStreamEventCamera->SwitchOn(this->pOwner);
		}
		else {
			this->pOwner->SetState(5, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

int CBehaviourSwitchMultiCondition::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	uint uVar1;
	CActorSwitch* pCVar2;
	uint uVar4;
	int iVar5;
	int iVar6;

	const uint msgParam = reinterpret_cast<uint>(pMsgParam);

	if (msg != 0xe) {
		if (msg != 0x10) {
			if (msg != 0xf) {
				return 0;
			}

			uVar1 = this->field_0xc;
			uVar4 = this->field_0x10 | 1 << (msgParam & 0x1f);

			if (uVar1 == this->field_0x10) {
				if (uVar1 != uVar4) {
					pCVar2 = this->pOwner;
					pCVar2->SetState(5, -1);
					pCVar2 = this->pOwner;
					for (int i = 0; i < pCVar2->pTargetStreamRef->entryCount; i++) {
						pCVar2->pTargetStreamRef->aEntries[i].PostSwitch(pCVar2);
					}
				}
			}
			else {
				if (uVar1 == uVar4) {
					pCVar2 = this->pOwner;
					pCVar2->SetState(8, -1);
					pCVar2 = this->pOwner;

					for (int i = 0; i < pCVar2->pTargetStreamRef->entryCount; i++) {
						pCVar2->pTargetStreamRef->aEntries[i].Switch(pCVar2);
					}

					pCVar2->pStreamEventCamera->SwitchOn(pCVar2);

					if ((this->field_0x14 & 1) != 0) {
						pCVar2 = this->pOwner;
						pCVar2->SetState(5, -1);
						pCVar2 = this->pOwner;

						for (int i = 0; i < pCVar2->pTargetStreamRef->entryCount; i++) {
							pCVar2->pTargetStreamRef->aEntries[i].PostSwitch(pCVar2);
						}

						this->field_0x10 = this->field_0x8;
						return 1;
					}
				}
			}

			this->field_0x10 = uVar4;
			return 1;
		}

		uVar1 = this->field_0xc;
		uVar4 = this->field_0x10 & ~(1 << (msgParam & 0x1f));
		if (uVar1 == this->field_0x10) {
			if (uVar1 != uVar4) {
				pCVar2 = this->pOwner;
				pCVar2->SetState(5, -1);
				pCVar2 = this->pOwner;

				for (int i = 0; i < pCVar2->pTargetStreamRef->entryCount; i++) {
					pCVar2->pTargetStreamRef->aEntries[i].PostSwitch(pCVar2);
				}
			}
		}
		else {
			if (uVar1 == uVar4) {
				pCVar2 = this->pOwner;
				pCVar2->SetState(8, -1);
				pCVar2 = this->pOwner;

				for (int i = 0; i < pCVar2->pTargetStreamRef->entryCount; i++) {
					pCVar2->pTargetStreamRef->aEntries[i].Switch(pCVar2);
				}

				pCVar2->pStreamEventCamera->SwitchOn(pCVar2);

				if ((this->field_0x14 & 1) != 0) {
					pCVar2 = this->pOwner;
					pCVar2->SetState(5, -1);
					pCVar2 = this->pOwner;

					for (int i = 0; i < pCVar2->pTargetStreamRef->entryCount; i++) {
						pCVar2->pTargetStreamRef->aEntries[i].PostSwitch(pCVar2);
					}

					this->field_0x10 = this->field_0x8;
					return 1;
				}
			}
		}

		this->field_0x10 = uVar4;
		return 1;
	}

	uVar1 = this->field_0xc;
	uVar4 = this->field_0x10 ^ 1 << (msgParam & 0x1f);
	if (uVar1 == this->field_0x10) {
		if (uVar1 != uVar4) {
			pCVar2 = this->pOwner;
			pCVar2->SetState(5, -1);
			pCVar2 = this->pOwner;

			for (int i = 0; i < pCVar2->pTargetStreamRef->entryCount; i++) {
				pCVar2->pTargetStreamRef->aEntries[i].PostSwitch(pCVar2);
			}
		}
	}
	else {
		if (uVar1 == uVar4) {
			pCVar2 = this->pOwner;
			pCVar2->SetState(8, -1);
			pCVar2 = this->pOwner;

			for (int i = 0; i < pCVar2->pTargetStreamRef->entryCount; i++) {
				pCVar2->pTargetStreamRef->aEntries[i].Switch(pCVar2);
			}

			pCVar2->pStreamEventCamera->SwitchOn(pCVar2);

			if ((this->field_0x14 & 1) != 0) {
				pCVar2 = this->pOwner;
				pCVar2->SetState(5, -1);
				pCVar2 = this->pOwner;

				for (int i = 0; i < pCVar2->pTargetStreamRef->entryCount; i++) {
					pCVar2->pTargetStreamRef->aEntries[i].PostSwitch(pCVar2);
				}

				this->field_0x10 = this->field_0x8;
				return 1;
			}
		}
	}

	this->field_0x10 = uVar4;

	return 1;
}
