#include "ActorBonus.h"
#include "MemoryStream.h"

#include "TimeController.h"
#include "MathOps.h"
#include "FileManager3D.h"
#include "DlistManager.h"
#include "CollisionRay.h"
#include "ActorHero.h"

CSharedLights<COmniLight, 3> CActorBonus::_gBNS_Lights;

StateConfig CActorBonus::_gStateCfg_BNS[6] = {
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x7, 0x0),
	StateConfig(0x7, 0x0),
};

void CActorBonus::Create(ByteCode* pByteCode)
{
	CActorMovable::Create(pByteCode);

	this->pFxTail = (CFxTail*)0x0;
	this->field_0x1dc.index = pByteCode->GetS32();
	this->flareMaterialId = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(this->flareMaterialId);
	this->animMaterialId = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(this->animMaterialId);
	this->field_0x1d8 = CreateActorSound(1);
	this->vector_0x1e0 = (this->subObjA)->boundingSphere;

	return;
}

void CActorBonus::Init()
{
	CShadow* pCVar1;
	KyaUpdateObjA* pKVar2;
	float fVar3;

	this->field_0xf0 = 20.0f;
	CActor::Init();
	this->field_0x1dc.Init();

	pCVar1 = this->pShadow;
	if (pCVar1 != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		(pCVar1->base).field_0x30 = 0x80808080;
		((this->pShadow)->base).field_0x48 = 1.3f;
		(this->pShadow)->field_0x50 = 1.3f;)
	}

	_gBNS_Lights.Init(0x40000000, 0x40800000, 0x41004, 0x10a010, 0);

	pKVar2 = this->subObjA;
	fVar3 = pKVar2->field_0x1c;
	if (pKVar2->visibilityDistance < fVar3) {
		(this->subObjA)->visibilityDistance = fVar3 + 5.0f;
	}

	(this->subObjA)->field_0x20 = 50000.0f;

	return;
}

// Should be in: D:/Projects/b-witch/ActorBonus.h
void CActorBonus::Term()
{
	CActor::Term();
	_gBNS_Lights.Term();
	return;
}

void CActorBonus::Draw()
{
	CActor::Draw();
}

void CActorBonus::CheckpointReset()
{
	CBehaviour* pCVar1;

	pCVar1 = GetBehaviour(this->curBehaviourId);
	if (pCVar1 != (CBehaviour*)0x0) {
		static_cast<CBehaviourBonusBase*>(GetBehaviour(this->curBehaviourId))->ChangeVisibleState(0);
	}
	return;
}

void CActorBonus::SaveContext(void* pData, uint mode, uint maxSize)
{
	CBehaviour* pBehaviour;

	pBehaviour = GetBehaviour(this->curBehaviourId);
	if (pBehaviour != (CBehaviour*)0x0) {
		CBehaviourBonusBase* pBehaviourBonus = static_cast<CBehaviourBonusBase*>(pBehaviour);
		pBehaviourBonus->SaveContext(pData, mode, maxSize);
	}
}

void CActorBonus::LoadContext(void* pData, uint mode, uint maxSize)
{
	CBehaviourBonusBase* pBehaviour;

	pBehaviour = static_cast<CBehaviourBonusBase*>(GetBehaviour(this->curBehaviourId));
	if (pBehaviour != (CBehaviourBonusBase*)0x0) {
		pBehaviour->LoadContext(pData, mode, maxSize);
	}

	return;
}

CBehaviour* CActorBonus::BuildBehaviour(int behaviourType)
{
	CBehaviour* pCVar1;

	if (behaviourType == 2) {
		pCVar1 = (CBehaviour*)0x0;
	}
	else {
		if (behaviourType == BONUS_BEHAVIOUR_ADD_ON) {
			pCVar1 = new CCBehaviourBonusAddOn;
		}
		else {
			if (behaviourType == BONUS_BEHAVIOUR_FLOCK) {
					pCVar1 = &this->behaviourBonusFlock;
			}
			else {
				if (behaviourType == BONUS_BEHAVIOUR_PATH) {
					pCVar1 = new CBehaviourBonusPath;
				}
				else {
					if (behaviourType == BONUS_BEHAVIOUR_TURN) {
						pCVar1 = new CBehaviourBonusTurn;
					}
					else {
						pCVar1 = CActor::BuildBehaviour(behaviourType);
					}
				}
			}
		}
	}
	return pCVar1;
}

StateConfig* CActorBonus::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActorMovable::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_BNS + state + -5;
	}

	return pStateConfig;
}

void CActorBonus::ChangeManageState(int state)
{
	int iVar1;
	CBehaviourBonusFlock* pCVar2;

	CActor::ChangeManageState(state);

	iVar1 = this->curBehaviourId;
	if ((((iVar1 == 6) || (iVar1 == 5)) || (iVar1 == 4)) || ((iVar1 == 3 || (iVar1 == 2)))) {
		if (state == 0) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
		}
		else {
			this->flags = this->flags & 0xffffff5f;
			EvaluateDisplayState();
		}
	}

	if ((((this->flags & 0x800000) != 0) && (state != 0)) && (pCVar2 = static_cast<CBehaviourBonusFlock*>(GetBehaviour(5)), pCVar2 != (CBehaviourBonusFlock*)0x0)) {
		CActInstance* pCVar3 = (CActInstance*)0x0;

		if (0 < pCVar2->nbInstances) {
			pCVar3 = pCVar2->aBnsInstances;
		}

		if (pCVar3 != (CActInstance*)0x0) {
			if (pCVar2->nbInstances < 1) {
				pCVar3 = (CActInstance*)0x0;
			}
			else {
				pCVar3 = pCVar2->aBnsInstances;
			}

			pCVar3->SetAlive(1);
		}
	}

	return;
}

void CActorBonus::ChangeVisibleState(int state)
{
	int iVar1;
	CBehaviourBonusBase* pCVar2;

	CActor::ChangeVisibleState(state);

	if ((this->flags & 0x800000) == 0) {
		iVar1 = this->curBehaviourId;
		if ((((iVar1 == 6) || (iVar1 == 5)) || (iVar1 == 4)) || ((iVar1 == 3 || (iVar1 == 2)))) {
			pCVar2 = static_cast<CBehaviourBonusBase*>(GetBehaviour(this->curBehaviourId));
			pCVar2->ChangeVisibleState(state);
		}
	}
	else {
		pCVar2 = static_cast<CBehaviourBonusBase*>(GetBehaviour(BONUS_BEHAVIOUR_FLOCK));
		if (pCVar2 != (CBehaviour*)0x0) {
			pCVar2->ChangeVisibleState(state);
		}
	}

	return;
}

void CActorBonus::CinematicMode_Leave(int behaviourId)
{
	CBnsInstance* pBnsInstance;

	CActor::CinematicMode_Leave(behaviourId);

	CBehaviourBonusFlock* pBehaviourFlock = static_cast<CBehaviourBonusFlock*>(GetBehaviour(BONUS_BEHAVIOUR_FLOCK));
	if (pBehaviourFlock != (CBehaviourBonusFlock*)0x0) {
		CBnsInstance* pBnsInstance = (pBehaviourFlock->nbInstances > 0) ? pBehaviourFlock->aBnsInstances : (CBnsInstance*)0x0;

		if (pBnsInstance != (CBnsInstance*)0x0) {
			UpdatePosition(&(pBnsInstance->currentPosition), false);
		}
	}

	return;
}

void CActorBonus::CinematicMode_Manage()
{
	CBehaviour* pBehaviourFlock = GetBehaviour(BONUS_BEHAVIOUR_FLOCK);
	if (pBehaviourFlock != (CBehaviour*)0x0) {
		pBehaviourFlock->Draw();
	}

	return;
}

void CActorBonus::CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition)
{
	CBehaviourBonusFlock* pBehaviourBonusFlock;
	CBnsInstance* pBnsInstance;
	float angleRotY, deltaTime, fieldA0, fieldA4;
	edF32VECTOR4 localPosition;
	CShadow* pShadow;

	pBehaviourBonusFlock = static_cast<CBehaviourBonusFlock*>(GetBehaviour(BONUS_BEHAVIOUR_FLOCK));
	if (pBehaviourBonusFlock != (CBehaviourBonusFlock*)0x0) {
		pBnsInstance = (CBnsInstance*)0x0;
		if (0 < pBehaviourBonusFlock->nbInstances) {
			pBnsInstance = pBehaviourBonusFlock->aBnsInstances;
		}

		if (pBnsInstance != (CBnsInstance*)0x0) {
			if ((pBnsInstance->flags & 4) == 0) {
				pBnsInstance->flags &= 0xfffffffd;
			}
			else {
				pBnsInstance->flags |= 2;
			}

			if ((pBnsInstance->flags & 1) != 0) {
				deltaTime = GetTimer()->cutsceneDeltaTime;
				angleRotY = pBnsInstance->angleRotY + deltaTime * 24.0f;
				pBnsInstance->angleRotY = (angleRotY >= 16.0f) ? angleRotY - 16.0f : angleRotY;
				pBnsInstance->FUN_003982c0();
				pBnsInstance->field_0x5c += deltaTime;
				pBnsInstance->field_0x64 = pBnsInstance->currentPosition.xyz;
			}

			fieldA0 = pBnsInstance->field_0xa0 + GetTimer()->cutsceneDeltaTime * 1.8f;
			pBnsInstance->field_0xa0 = (fieldA0 >= 6.283185f) ? fieldA0 - 6.283185f : fieldA0;
			fieldA4 = pBnsInstance->field_0xa4 + GetTimer()->cutsceneDeltaTime * 2.2f;
			pBnsInstance->field_0xa4 = (fieldA4 >= 6.283185f) ? fieldA4 - 6.283185f : fieldA4;

			pBnsInstance->SetBasePosition(&pPosition->rowT);

			localPosition.x = cosf(pBnsInstance->field_0xa4);
			localPosition.y = cosf(pBnsInstance->field_0xa0);
			localPosition.z = sinf(pBnsInstance->field_0xa4);
			localPosition.w = 0.0f;

			edF32Vector4ScaleHard(0.25f, &localPosition, &localPosition);
			edF32Vector4AddHard(&localPosition, &pBnsInstance->basePosition, &localPosition);
			pBnsInstance->SetPosition(&localPosition);
			pBnsInstance->SetVisible(this->flags & 0x100);

			if (this->pShadow != (CShadow*)0x0) {
				IMPLEMENTATION_GUARD_SHADOW(
				(*(void(**)())(this->pShadow->pVTable + 0x14))();
				pShadow = this->pShadow;
				pShadow->field_0x10.x = pBnsInstance->currentPosition.x;
				pShadow->field_0x10.z = pBnsInstance->currentPosition.z;)
			}
		}

		UpdatePosition(&pPosition->rowT, false);
	}

	return;
}

void CBehaviourBonusBase::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorBonus*>(pOwner);

	this->field_0x8 = ((float)rand() / 2.147484e+09f) * 6.283185f;
	this->field_0xc = ((float)rand() / 2.147484e+09f) * 6.283185f;
	this->field_0x10 = ((float)rand() / 2.147484e+09f) * 6.283185f;

	return;
}

void CBehaviourBonusBase::Manage()
{
	edF32VECTOR4 local_10;

	this->field_0x8 = edF32Between_0_2Pi(this->field_0x8 + GetTimer()->cutsceneDeltaTime * 1.1f);
	this->field_0xc = edF32Between_0_2Pi(this->field_0xc + GetTimer()->cutsceneDeltaTime * 1.3f);
	this->field_0x10 = edF32Between_0_2Pi(this->field_0x10 + GetTimer()->cutsceneDeltaTime * 1.6f);

	local_10.x = cosf(this->field_0x8);
	local_10.y = sinf(this->field_0xc);
	local_10.z = cosf(this->field_0x10);
	local_10.w = 0.0f;

	edF32Vector4ScaleHard(0.2f, &local_10, &local_10);
	edF32Vector4AddHard(&local_10, &local_10, &this->pOwner->baseLocation);
	this->pOwner->UpdatePosition(&local_10, true);

	return;
}

void CBehaviourBonusBase::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner->field_0xf0 = 20.0f;
	this->pOwner->ComputeAltitude();
	this->pOwner->field_0xf0 = this->pOwner->distanceToGround + 0.2f;

	if (newState == -1) {
		this->pOwner->SetState(6, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourBonusBase::Func_0x4c()
{
	return;
}

// Should be in: D:/Projects/b-witch/ActorBonus.h
void CBehaviourBonusBase::ChangeVisibleState(int state)
{
	return;
}

void CBehaviourBonusBase::SaveContext(void* pData, uint mode, uint maxSize)
{
	return;
}

void CBehaviourBonusBase::LoadContext(void* pData, uint mode, uint maxSize)
{
	return;
}

void CBehaviourBonusAlone::Create(ByteCode* pByteCode)
{
	uint materialId;
	uint fxTailCount;
	int g2dIndex;
	C3DFileManager* p3DManager;

	p3DManager = CScene::ptable.g_C3DFileManager_00451664;
	this->field_0x14 = pByteCode->GetF32();
	this->field_0x1b8 = pByteCode->GetF32();
	this->field_0x1b4 = pByteCode->GetF32();
	materialId = pByteCode->GetU32();
	this->field_0x1b0 = pByteCode->GetF32();
	fxTailCount = pByteCode->GetU32();
	g2dIndex = pByteCode->GetS32();

	if (g2dIndex != -1) {
		p3DManager->InstanciateG2D(g2dIndex);
	}

	this->fxTail.Create(0.4f, fxTailCount, 2, g2dIndex);
	this->fxTail.field_0xb0 = materialId;

	return;
}

void CBehaviourBonusAlone::Init(CActor* pOwner)
{
	CShadow* pShadow;
	int randValue;
	CActorBonus* pBonus;

	CBehaviourBonusBase::Init(pOwner);

	pBonus = this->pOwner;
	this->actInstance.InitBns(pBonus, &pBonus->baseLocation, &pBonus->vector_0x1e0);

	this->fxTail.Init(0.5f, pOwner->sectorId);

	pShadow = pBonus->pShadow;
	if (pShadow != nullptr) {
		IMPLEMENTATION_GUARD_SHADOW(
		pShadow->field_0x48 = 1.3f;)
	}

	this->bonusFlarePatchId = GameDListPatch_Register(pOwner, 0, 1);
	this->bonusAnimPatchId = GameDListPatch_Register(pOwner, 0, 1);

	return;
}

void CBehaviourBonusAlone::Term()
{
	this->actInstance.Term();

	return;
}

void CBehaviourBonusAlone::SectorChange(int oldSectorId, int newSectorId)
{
	CFxTail* pFxTail;

	this->actInstance.CheckpointReset();

	this->pOwner->flags &= 0xfffffffc;
	this->pOwner->flags &= 0xffffff5f;
	this->pOwner->EvaluateDisplayState();

	pFxTail = this->pOwner->pFxTail;
	if (pFxTail != nullptr) {
		pFxTail->Reset();
	}

	return;
}

void CBehaviourBonusAlone::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorBonus* pBonus;
	CShadow* pShadow;
	uint flags;
	uint8_t shadowVisible = 0;

	CBehaviourBonusBase::Begin(pOwner, newState, newAnimationType);

	this->pOwner->pFxTail = &this->fxTail;
	this->actInstance.Reset();
	this->actInstance.field_0x98 = 0.0f;

	pShadow = this->pOwner->pShadow;
	if (pShadow != nullptr) {
		flags = this->actInstance.flags;
		if ((flags & 1) != 0 && (flags & 6) != 0) {
			shadowVisible = 1;
		}

		IMPLEMENTATION_GUARD_SHADOW(
		pShadow->SetVisible(shadowVisible);
		pShadow->field_0x10.x = this->actInstance.currentPosition.x;
		pShadow->field_0x10.z = this->actInstance.currentPosition.z;)
	}

	this->field_0x1c4 = 1;

	return;
}

bool CBehaviourBonusAlone::InitDlistPatchable(int patchId)
{
	bool bSuccess = false;
	edDList_material* pMaterial;
	C3DFileManager* p3DManager = CScene::ptable.g_C3DFileManager_00451664;

	if (patchId == this->bonusFlarePatchId) {
		edDListLoadIdentity();
		pMaterial = p3DManager->GetMaterialFromId(this->pOwner->flareMaterialId, 0);
		edDListUseMaterial(pMaterial);
		edDListBegin(0.0f, 0.0f, 0.0f, 0xb, 1);
		this->actInstance.InitDlistPatchable(0);
		edDListEnd();
		bSuccess = true;
	}
	else if (patchId == this->bonusAnimPatchId) {
		edDListLoadIdentity();
		pMaterial = p3DManager->GetMaterialFromId(this->pOwner->animMaterialId, 0);
		edDListUseMaterial(pMaterial);
		edDListBegin(0.0f, 0.0f, 0.0f, 0xb, 1);
		this->actInstance.InitDlistPatchable(1);
		edDListEnd();
		bSuccess = true;
	}

	return bSuccess;
}

void CBehaviourBonusAlone::KillInstance(CBnsInstance* pInstance)
{
	if ((this->actInstance.flags & 1) != 0) {
		this->actInstance.SetAlive(0);
		GameDListPatch_HideSprite(this->bonusFlarePatchId, 0);
		GameDListPatch_HideSprite(this->bonusAnimPatchId, 0);
	}

	return;
}

void CBehaviourBonusAlone::ChangeVisibleState(int state)
{
	CGlobalDListManager* pDlist;

	pDlist = (CGlobalDListManager*)CScene::GetManager(MO_GlobalDListManager);

	if (((this->actInstance).flags & 1) != 0) {
		pDlist->SetActive(this->bonusFlarePatchId, state);
		pDlist->SetActive(this->bonusAnimPatchId, state);
		(this->pOwner)->pFxTail->SetPatchActive(state);
	}

	return;
}

void CBehaviourBonusAlone::SaveContext(void* pData, uint mode, uint maxSize)
{
	uint* pSaveData = (uint*)pData;
	*pSaveData = (uint)(((this->actInstance).flags & 1) != 0);

	return;
}

void CBehaviourBonusAlone::LoadContext(void* pData, uint mode, uint maxSize)
{
	CActorBonus* pBonus;

	if (mode == 1) {
		if (*reinterpret_cast<uint*>(pData) == 0) {
			pBonus = this->pOwner;
			pBonus->flags = (pBonus->flags & 0xffffff7f) | 0x20;
			pBonus->EvaluateDisplayState();
			pBonus->flags = (pBonus->flags & 0xfffffffd) | 1;
		}

		this->actInstance.SetAlive(*reinterpret_cast<uint*>(pData));
	}

	return;
}

edF32VECTOR4 edF32VECTOR4_0040e1c0 = { 0.0f, -1.0f, 0.0f, 0.0f };
edF32VECTOR4 edF32VECTOR4_0040e1d0 = { 0.0f, 1.0f, 0.0f, 0.0f };

float CBehaviourBonusBase::UpdateGroundDistanceAndShadow(edF32VECTOR4* pOrigin)
{
	CActorBonus* pBonus;
	float intersectResult, groundHeight;
	CCollisionRay collisionRay;
	edF32VECTOR4 rayOrigin, rayDir, rayNormal;
	edF32VECTOR4 local_10;

	// Unpack currentPosition into rayOrigin
	rayOrigin.x = pOrigin->x;
	rayOrigin.y = pOrigin->y + 0.3f;
	rayOrigin.z = pOrigin->z;
	rayOrigin.w = pOrigin->w;

	local_10 = rayOrigin;

	pBonus = this->pOwner;
	pBonus->field_0xf0 = 15.0f;
	groundHeight = pBonus->field_0xf0;

	// edF32VECTOR4_0040e1c0 and edF32VECTOR4_0040e1d0 are likely static direction/normal vectors
	rayDir = edF32VECTOR4_0040e1c0;

	rayNormal = edF32VECTOR4_0040e1d0;

	collisionRay = CCollisionRay(groundHeight, &rayOrigin, &rayDir);
	intersectResult = collisionRay.Intersect(1, nullptr, nullptr, 0x40000008, &rayNormal, nullptr);
	if (intersectResult == 1e+30f) {
		intersectResult = groundHeight + 0.3f;
	}
	
	return intersectResult;
}

void CBehaviourBonusTurn::Create(ByteCode* pByteCode)
{
	CBehaviourBonusAlone::Create(pByteCode);

	float fVar2 = pByteCode->GetF32();
	this->field_0x1d4 = fVar2;
	this->field_0x1d0 = fVar2;
	this->field_0x1d8 = 0;

	return;
}

void CBehaviourBonusTurn::Manage()
{
	CShadow* pShadow;
	CActorBonus* pBonus;
	CFxTail* pFxTail;
	Timer* pTimer;
	uint flags, nextFlags;
	float prevY;
	edF32VECTOR4 local_10;

	if (this->field_0x1c4 != 0) {
		this->field_0x1c4 = 0;
		local_10 = this->actInstance.currentPosition;
		float intersectResult = UpdateGroundDistanceAndShadow(&local_10);

		pBonus = this->pOwner;
		pBonus->distanceToGround = intersectResult - 0.3f;
		local_10.y = local_10.y - pBonus->distanceToGround;

		pShadow = pBonus->pShadow;
		if (pShadow != nullptr) {
			IMPLEMENTATION_GUARD_SHADOW(
				pShadow->field_0x10 = local_10;
			CShadowShared::SetIntensity(1.0f - pBonus->distanceToGround / pBonus->field_0xf0, pShadow);)
		}
	}

	pFxTail = this->pOwner->pFxTail;
	pFxTail->field_0x30 = 128.0f / static_cast<float>(pFxTail->count_0x34 << 2);

	if (this->field_0x1dc == 0) {
		pFxTail->Manage(&this->actInstance.currentPosition, 1, ((this->actInstance.flags & 1) == 0));
	}
	else {
		pFxTail->Manage(nullptr, 0, 1);
	}

	nextFlags = (this->field_0x1dc + 1) & 3;
	if (static_cast<int>(this->field_0x1dc + 1) < 0 && nextFlags != 0) {
		nextFlags -= 4;
	}
	this->field_0x1dc = nextFlags;

	if ((this->actInstance.flags & 1) == 0) {
		pShadow = this->pOwner->pShadow;
		if (pShadow != nullptr) {
			IMPLEMENTATION_GUARD_SHADOW(
			pShadow->SetVisible(0);)
		}

		pBonus = this->pOwner;
		if ((reinterpret_cast<CActor*>(pBonus->pFxTail)->state_0x10 & 0x1000) == 0) {
			pBonus->SetState(6, -1);
			pBonus->flags = (pBonus->flags & 0xfffffffd) | 1;
			pBonus->flags = (pBonus->flags & 0xffffff7f) | 0x20;
			pBonus->EvaluateDisplayState();
		}
	}
	else {
		prevY = this->actInstance.currentPosition.y;
		this->actInstance.BehaviourTurn_Manage(this);

		pBonus = this->pOwner;
		pBonus->distanceToGround += (this->actInstance.currentPosition.y - prevY);

		pShadow = pBonus->pShadow;
		if (pShadow != nullptr) {
			flags = this->actInstance.flags;
			uint8_t shadowVisible = 0;
			if ((flags & 1) != 0 && (flags & 6) != 0) {
				shadowVisible = 1;
			}

			IMPLEMENTATION_GUARD_SHADOW(
			pShadow->SetVisible(shadowVisible);
			pShadow->field_0x10.x = this->actInstance.currentPosition.x;
			pShadow->field_0x10.z = this->actInstance.currentPosition.z;
			CShadowShared::SetIntensity(1.0f - pBonus->distanceToGround / pBonus->field_0xf0, pShadow);)
		}

		this->field_0x1d0 = this->field_0x1d4 + sinf(this->field_0x1d8) * 0.12f;
		this->field_0x1d8 = edF32Between_2Pi(this->field_0x1d8 + GetTimer()->cutsceneDeltaTime * 7.0f);
	}

	return;
}

void CBehaviourBonusTurn::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorBonus* pBonus;
	CShadow* pShadow;
	uint randValue, randMod;

	CBehaviourBonusAlone::Begin(pOwner, newState, newAnimationType);

	this->pOwner->field_0xf0 = 15.0f;
	this->pOwner->ComputeAltitude();
	pBonus = this->pOwner;
	pBonus->field_0xf0 = this->field_0x1d4 + pBonus->distanceToGround;
	pBonus->SetLocalBoundingSphere(this->field_0x1d4 + 0.1714286f, &gF32Vertex4Zero);

	randValue = static_cast<uint>(rand());
	randMod = randValue & 3;
	if (static_cast<int>(randValue) < 0 && randMod != 0) {
		randMod -= 4;
	}
	this->field_0x1dc = randMod;

	return;
}

void CBehaviourBonusPath::Create(ByteCode* pByteCode)
{
	CBehaviourBonusAlone::Create(pByteCode);

	this->pathPlane.pathFollowReader.Create(pByteCode);
	this->field_0x1f4 = pByteCode->GetF32();
	this->field_0x1f8 = pByteCode->GetF32();
	this->field_0x1fc = pByteCode->GetF32();
	this->field_0x200 = pByteCode->GetF32();
	this->field_0x204 = pByteCode->GetU32();
	this->field_0x1f0 = 0;

	return;
}

void CBehaviourBonusPath::Init(CActor* pOwner)
{
	CBehaviourBonusAlone::Init(pOwner);

	this->pathPlane.Init();

	return;
}

void CBehaviourBonusPath::Manage()
{
	CActorBonus* pBonus;
	CShadow* pShadow;
	uint flags;

	if ((this->pathPlane).pathFollowReader.pPathFollow != (CPathFollow*)0x0) {
		if (this->field_0x1c4 != 0) {
			this->field_0x1c4 = 0;
			edF32VECTOR4 local_10 = this->actInstance.currentPosition;
			float intersectResult = UpdateGroundDistanceAndShadow(&local_10);

			pBonus = this->pOwner;
			pBonus->distanceToGround = intersectResult - 0.3f;
			local_10.y = local_10.y - pBonus->distanceToGround;

			pShadow = pBonus->pShadow;
			if (pShadow != nullptr) {
				IMPLEMENTATION_GUARD_SHADOW(
					pShadow->field_0x10 = local_10;
				CShadowShared::SetIntensity(1.0f - pBonus->distanceToGround / pBonus->field_0xf0, pShadow);)
			}
		}

		this->pOwner->pFxTail->Manage(&this->actInstance.currentPosition, 1, (this->actInstance.flags & 1) != 0 ^ 1);

		if ((this->actInstance.flags & 1) == 0) {
			pShadow = this->pOwner->pShadow;
			if (pShadow != nullptr) {
				IMPLEMENTATION_GUARD_SHADOW(
					pShadow->SetVisible(0);)
			}

			pBonus = this->pOwner;
			if ((reinterpret_cast<CActor*>(pBonus->pFxTail)->state_0x10 & 0x1000) == 0) {
				pBonus->SetState(6, -1);
				pBonus->flags = (pBonus->flags & 0xfffffffd) | 1;
				pBonus->flags = (pBonus->flags & 0xffffff7f) | 0x20;
				pBonus->EvaluateDisplayState();
			}
		}
		else {
			float prevY = this->actInstance.currentPosition.y;
			this->actInstance.BehaviourPath_Manage(this);

			pBonus = this->pOwner;
			pBonus->distanceToGround += (this->actInstance.currentPosition.y - prevY);

			pShadow = pBonus->pShadow;
			if (pShadow != nullptr) {
				flags = this->actInstance.flags;
				uint8_t shadowVisible = 0;
				if ((flags & 1) != 0 && (flags & 6) != 0) {
					shadowVisible = 1;
				}

				IMPLEMENTATION_GUARD_SHADOW(
				pShadow->SetVisible(shadowVisible);
				pShadow->field_0x10.x = this->actInstance.currentPosition.x;
				pShadow->field_0x10.z = this->actInstance.currentPosition.z;
				CShadowShared::SetIntensity(1.0f - pBonus->distanceToGround / pBonus->field_0xf0, pShadow);)
			}

			this->pOwner->UpdatePosition(&this->actInstance.currentPosition, true);
		}
	}

	return;
}

void CBehaviourBonusPath::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourBonusAlone::Begin(pOwner, newState, newAnimationType);

	this->pOwner->field_0xf0 = 15.0f;
	this->pOwner->ComputeAltitude();

	this->pOwner->field_0xf0 = this->pOwner->distanceToGround;
	if (this->pathPlane.pathFollowReader.pPathFollow != (CPathFollow*)0x0) {
		this->pathPlane.Reset();

		this->pOwner->UpdatePosition(this->pathPlane.pathFollowReader.GetWayPoint(), true);
		this->pathPlane.pathFollowReader.NextWayPoint();
	}

	this->actInstance.SetState(0);

	return;
}

CBehaviourBonusFlock::CBehaviourBonusFlock()
{
	this->nbInstances = 0;
	this->nbSharedShadows = 0;
	this->field_0x38 = 0;
	this->aBnsInstances = (CBnsInstance*)0x0;
	this->aShadowShared = (CShadowShared*)0x0;

	return;
}

void CBehaviourBonusFlock::Create(ByteCode* pByteCode)
{
	this->pathFollowRef.index = pByteCode->GetS32();
	this->field_0x38 = 0;

	return;
}

void CBehaviourBonusFlock::Init(CActor* pOwner)
{
	CPathFollow* pCVar1;
	uint uVar2;
	CActorBonus* pCVar3;
	CShadow* pCVar4;
	int iVar5;
	int* piVar6;
	CBnsInstance* pCVar7;
	CShadowShared* pCVar8;
	int iVar9;
	edF32VECTOR4* peVar10;
	int iVar11;
	float fVar12;
	float fVar13;
	float fVar14;

	this->pathFollowRef.Init();

	pCVar1 = (this->pathFollowRef).Get();
	if (pCVar1 == (CPathFollow*)0x0) {
		this->nbInstances = 1;
	}
	else {
		this->nbInstances = pCVar1->splinePointCount;
	}

	CBehaviourBonusBase::Init(pOwner);

	this->aBnsInstances = new CBnsInstance[this->nbInstances];

	if ((this->pOwner->pShadow != (CShadow*)0x0) && ((this->pathFollowRef).Get() != (CPathFollow*)0x0)) {
		this->nbSharedShadows = this->nbInstances + -1;
		uVar2 = this->nbSharedShadows;
		this->aShadowShared = new CShadowShared[uVar2];
	}

	if (this->nbInstances == 1) {
		this->aBnsInstances->InitBns(this->pOwner, &this->pOwner->baseLocation, &this->pOwner->vector_0x1e0);
	}
	else {
		pCVar7 = this->aBnsInstances;
		iVar5 = 0;
		if (0 < this->nbInstances) {
			do {
				peVar10 = (this->pathFollowRef).Get()->aSplinePoints;

				if (peVar10 == (edF32VECTOR4*)0x0) {
					peVar10 = &gF32Vertex4Zero;
				}
				else {
					peVar10 = peVar10 + iVar5;
				}

				pCVar3 = this->pOwner;
				pCVar7->InitBns(pCVar3, peVar10, &pCVar3->vector_0x1e0);
				pCVar7 = pCVar7 + 1;
				iVar5 = iVar5 + 1;
			} while (iVar5 < this->nbInstances);
		}

		if (this->nbSharedShadows != 0) {
			pCVar8 = this->aShadowShared;
			iVar5 = 0;
			if (0 < this->nbSharedShadows) {
				do {
					IMPLEMENTATION_GUARD_SHADOW(
					CShadowShared::Init(pCVar8, ((this->pOwner)->base).base.objectId);
					fVar14 = gF32Vector4UnitY.w;
					fVar13 = gF32Vector4UnitY.z;
					fVar12 = gF32Vector4UnitY.y;
					(pCVar8->field_0x20).x = gF32Vector4UnitY.x;
					(pCVar8->field_0x20).y = fVar12;
					(pCVar8->field_0x20).z = fVar13;
					(pCVar8->field_0x20).w = fVar14;
					pCVar8->field_0x48 = 1.3;
					pCVar8->field_0x30 = 0x80808080;
					peVar10 = ((this->pathFollowRef).pPathFollow)->aSplinePoints;
					if (peVar10 == (edF32VECTOR4*)0x0) {
						peVar10 = &gF32Vertex4Zero;
					}
					else {
						peVar10 = peVar10 + iVar5 + 1;
					}
					iVar5 = iVar5 + 1;
					fVar14 = peVar10->y;
					fVar12 = peVar10->z;
					fVar13 = peVar10->w;
					(pCVar8->field_0x10).x = peVar10->x;
					(pCVar8->field_0x10).y = fVar14;
					(pCVar8->field_0x10).z = fVar12;
					(pCVar8->field_0x10).w = fVar13;)
					pCVar8 = pCVar8 + 1;
				} while (iVar5 < this->nbSharedShadows);
			}
		}
	}

	pCVar4 = this->pOwner->pShadow;
	if (pCVar4 != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD_SHADOW(
		pCVar4->field_0x48 = 1.3f;)
	}

	this->pOwner->UpdateBoundingSphere(this->aBnsInstances, this->nbInstances);

	this->flarePatchId = GameDListPatch_Register(pOwner, 0, this->nbInstances);
	this->animPatchId = GameDListPatch_Register(pOwner, 0, this->nbInstances);

	if (this->aShadowShared != (CShadowShared*)0x0) {
		this->field_0x2c = GameDListPatch_Register(pOwner, this->nbSharedShadows << 2, 0);
	}

	return;
}

void CBehaviourBonusFlock::Term()
{
	int instanceIndex = 0;

	if (this->nbInstances > 0) {
		do {
			CActInstance* pInstance = this->aBnsInstances + instanceIndex;
			pInstance->Term();
			instanceIndex++;
		} while (instanceIndex < this->nbInstances);
	}

	if (this->aBnsInstances != (CBnsInstance*)0x0) {
		delete[] this->aBnsInstances;
	}

	if (this->aShadowShared != (CShadowShared*)0x0) {
		delete[] this->aShadowShared;
	}

	return;
}

typedef void (CBnsInstance::* BNS_PTMF_TYPE)(CBehaviourBonusFlock*);

BNS_PTMF_TYPE gBnsBhvAddOnFunc = &CBnsInstance::BehaviourAddOn_Manage;
BNS_PTMF_TYPE gBnsBhvFlockFunc = &CBnsInstance::BehaviourFlock_Manage;


void CBehaviourBonusFlock::Manage()
{
	CActorBonus* pBonus;
	CShadow* pCVar2;
	uint uVar3;
	edF32MATRIX4* peVar4;
	edF32VECTOR4 fVar5;
	CBnsInstance* iVar9;
	CShadowShared* piVar10;
	undefined8 uVar5;
	long lVar6;
	int iVar7;
	CBnsInstance* pCVar8;
	CShadowShared* iVar11;
	int iVar10;
	int iVar12;
	float fVar13;
	float fVar14;
	CCollisionRay CStack208;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	CCollisionRay CStack128;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	BNS_PTMF_TYPE local_10;

	this->field_0x38 = 0;
	if (this->aBnsInstances != (CBnsInstance*)0x0) {
		if (this->field_0x20 != 0) {
			this->field_0x20 = 0;
			pCVar8 = this->aBnsInstances;
			fVar5 = pCVar8->currentPosition;
			
			fVar13 = UpdateGroundDistanceAndShadow(&fVar5);

			this->pOwner->distanceToGround = fVar13 - 0.3f;
			pBonus = this->pOwner;
			fVar13 = pBonus->distanceToGround;
			pCVar2 = pBonus->pShadow;
			if (pCVar2 != (CShadow*)0x0) {
				IMPLEMENTATION_GUARD_SHADOW(
				pCVar2->field_0x10.x = fVar5.x;
				pCVar2->field_0x10.y = fVar5.y - fVar13;
				pCVar2->field_0x10.z = fVar5.z;
				pCVar2->field_0x10.w = fVar5.w;
				pBonus = this->pOwner;
				CShadowShared::SetIntensity
				(1.0f - pBonus->distanceToGround / pBonus->field_0xf0,
					pBonus->pShadow);)
			}

			iVar12 = 0;
			if (0 < this->nbSharedShadows) {
				IMPLEMENTATION_GUARD_SHADOW(
				iVar7 = 0;
				do {
					iVar11 = (CShadowShared*)((int)&this->aShadowShared->pVTable + iVar7);
					local_60 = (iVar11->field_0x10);
					fVar13 = UpdateGroundDistanceAndShadow(&local_60);
					iVar12 = iVar12 + 1;
					local_30.y = local_30.y - (fVar13 - 0.3);
					(iVar11->field_0x10).x = local_30.x;
					(iVar11->field_0x10).y = local_30.y;
					(iVar11->field_0x10).z = local_30.z;
					(iVar11->field_0x10).w = local_30.w;
					iVar7 = iVar7 + 0x50;
				} while (iVar12 < this->nbSharedShadows);)
			}
		}

		pCVar2 = this->pOwner->pShadow;
		if (pCVar2 != (CShadow*)0x0) {
			IMPLEMENTATION_GUARD_SHADOW(
			uVar3 = (this->aBnsInstances->base).flags;
			uVar5 = 0;
			if (((uVar3 & 1) != 0) && ((uVar3 & 6) != 0)) {
				uVar5 = 1;
			}
			(**(code**)((pCVar2->base).pVTable + 0x14))(pCVar2, uVar5);)
		}

		pCVar8 = this->aBnsInstances;
		if (((pCVar8->flags & 1) != 0) && (pCVar2 = this->pOwner->pShadow, pCVar2 != (CShadow*)0x0)) {
			IMPLEMENTATION_GUARD_SHADOW(
			(pCVar2->base).field_0x10.x = (pCVar8->base).currentPosition.x;
			(pCVar2->base).field_0x10.z = (pCVar8->base).currentPosition.z;)
		}
	}

	pBonus = this->pOwner;
	lVar6 = (long)(int)pBonus;
	pCVar8 = this->aBnsInstances;
	peVar4 = &CCameraManager::_gThis->transformationMatrix;
	fVar13 = (pBonus->subObjA)->visibilityDistance;
	if (pBonus->curBehaviourId == BONUS_BEHAVIOUR_ADD_ON) {
		local_10 = gBnsBhvAddOnFunc;
	}
	else {
		local_10 = gBnsBhvFlockFunc;
	}

	iVar12 = 0;
	if (0 < this->nbInstances) {
		do {
			if ((pCVar8->flags & 1) != 0) {
				this->field_0x38 = this->field_0x38 + 1;
				fVar14 = pCVar8->DistSquared(&peVar4->rowT);
				if (fVar14 < fVar13 * fVar13) {
					(pCVar8->*local_10)(this);
				}
			}

			iVar12 = iVar12 + 1;
			pCVar8 = pCVar8 + 1;
		} while (iVar12 < this->nbInstances);
	}

	iVar12 = 0;
	if (0 < this->nbSharedShadows) {
		IMPLEMENTATION_GUARD_SHADOW(
		iVar10 = 0;
		iVar7 = 0;
		do {
			iVar9 = (CBnsInstance*)((int)&this->aBnsInstances[1].base.pVTable + iVar10);
			piVar10 = (CShadowShared*)((int)&this->aShadowShared->pVTable + iVar7);
			if (((iVar9->base).flags & 1) != 0) {
				(piVar10->field_0x10).x = (iVar9->base).currentPosition.x;
				(piVar10->field_0x10).z = (iVar9->base).currentPosition.z;
			}

			uVar3 = (iVar9->base).flags;
			uVar5 = 0;
			if (((uVar3 & 1) != 0) && ((uVar3 & 6) != 0)) {
				uVar5 = 1;
			}

			(**(code**)(piVar10->pVTable + 0x14))(piVar10, uVar5);
			iVar12 = iVar12 + 1;
			iVar10 = iVar10 + 0xc0;
			iVar7 = iVar7 + 0x50;
		} while (iVar12 < this->nbSharedShadows);)
	}

	if (this->field_0x38 == 0) {
		pBonus = this->pOwner;
		pBonus->SetState(6, -1);
		pBonus = this->pOwner;
		pBonus->flags = pBonus->flags & 0xfffffffd;
		pBonus->flags = pBonus->flags | 1;
		pBonus = this->pOwner;
		pBonus->flags = pBonus->flags & 0xffffff7f;
		pBonus->flags = pBonus->flags | 0x20;
		pBonus->EvaluateDisplayState();
		pBonus = this->pOwner;
		pBonus->flags = pBonus->flags & 0xfffffbff;
	}

	return;
}

void CBehaviourBonusFlock::SectorChange(int oldSectorId, int newSectorId)
{
	CActorBonus* pActorBonus;
	CGlobalDListPatch* pDlistPatch;
	CGlobalDListManager* pDlistManager;
	uint sectorFlag;
	int patchIndex;
	int instanceOffset;

	// If leaving the sector that matches this actor's objectId
	if (oldSectorId != -1 && oldSectorId == this->pOwner->sectorId) {
		for (int i = 0; i < this->nbInstances; ++i) {
			CActInstance* pInstance = this->aBnsInstances + i;
			pInstance->SetVisible(0);
		}

		pActorBonus = this->pOwner;
		pActorBonus->flags = (pActorBonus->flags & 0xffffff7f) | 0x20;
		pActorBonus->EvaluateDisplayState();
		pActorBonus->flags = (pActorBonus->flags & 0xfffffffd) | 1;
	}

	if (newSectorId == -1) {
		return;
	}

	int objectId = this->pOwner->sectorId;
	if (newSectorId != objectId && objectId > 0) {
		return;
	}

	pDlistManager = (CGlobalDListManager*)CScene::GetManager(MO_GlobalDListManager);
	pActorBonus = this->pOwner;

	if (pActorBonus->sectorId > 0) goto LAB_00127130;

	if (pDlistManager->ppGlobalDlist == (GlobalDlistEntry*)0x0 ||
		((patchIndex = this->flarePatchId >> 16, pDlistManager->field_0x14 != patchIndex && patchIndex != 0))) {
	LAB_00127120:
		sectorFlag = 0xffffffff;
	}
	else {
		pDlistPatch = pDlistManager->ppGlobalDlist[patchIndex].pDlistPatch;

		if (pDlistPatch == (CGlobalDListPatch*)0x0) goto LAB_00127120;

		sectorFlag = (uint)((*(uint*)pDlistPatch->aPatches[this->flarePatchId & 0xffff] & 1) != 0);
	}

	if (sectorFlag != 0) {
		return;
	}

LAB_00127130:
	pActorBonus->flags &= 0xfffffffc;
	pActorBonus->flags &= 0xffffff5f;
	pActorBonus->EvaluateDisplayState();

	return;
}

void CBehaviourBonusFlock::Draw()
{
	_rgba* p_Var1;
	CCameraManager* pCVar2;
	CGlobalDListManager* this_00;
	CGlobalDListPatch* pCVar3;
	Timer* pTVar4;
	edVertex* peVar5;
	int iVar6;
	CGlobalDListManager* lVar6;
	uint uVar7;
	_rgba* p_Var8;
	undefined2 uVar9;
	undefined2 uVar10;
	undefined2* puVar11;
	int iVar12;
	undefined2* puVar13;
	undefined2* puVar14;
	undefined2* puVar15;
	uint uVar16;
	CBnsInstance* pCVar17;
	uint uVar18;
	int iVar19;
	float fVar20;
	float fVar21;
	edF32VECTOR4 local_70;
	edF32MATRIX4 eStack96;
	float local_18;
	float local_14;
	_rgba local_10;
	_rgba local_c;
	_rgba local_8;
	_rgba local_4;

	IMPLEMENTATION_GUARD(

	if (0 < this->nbInstances) {
		this_00 = (CGlobalDListManager*)CScene::GetManager(MO_GlobalDListManager);
		pCVar3 = GameDListPatch_BeginCurrent(this->flarePatchId);
		if (pCVar3 != (CGlobalDListPatch*)0x0) {
			pCVar17 = this->aBnsInstances;
			for (uVar16 = 0; pCVar2 = CCameraManager::_gThis, uVar16 < (uint)this->nbInstances; uVar16 = uVar16 + 1) {
				uVar7 = (pCVar17->base).flags;
				if ((uVar7 & 4) == 0) {
					if ((uVar7 & 2) != 0) {
						iVar19 = this->flarePatchId;
						p_Var1 = (this_00->ppGlobalDlist[iVar19 >> 0x10].pDlistPatch)->pCurrentPatch->pRgba;
						p_Var8 = p_Var1 + uVar16 * 4;
						local_c = (_rgba)((uint)p_Var1[uVar16 * 4] & 0xffffff);
						gpCurPatchRGBABuf = 0;
						*p_Var8 = local_c;
						p_Var8[1] = local_c;
						p_Var8[2] = local_c;
						p_Var8[3] = local_c;
						CGlobalDListManager::_AddCallFuncElement2(this_00, iVar19, 9, (short)uVar16, 0);
					}
				}
				else {
					pTVar4 = GetTimer();
					fVar21 = edF32Between_0_2Pi((pCVar17->base).field_0x90 + pTVar4->cutsceneDeltaTime * 7.0);
					(pCVar17->base).field_0x90 = fVar21;
					local_70.x = edFCosinus[(int)(ABS(fVar21 * 1303.797) + 0.5) & 0x1fff] * 0.05;
					local_70.y = edFCosinus[(int)(ABS(((pCVar17->base).field_0x90 - 1.570796) * 1303.797) + 0.5) & 0x1fff] * 0.05;
					local_70.z = 0.0;
					local_70.w = 1.0;
					edF32Matrix4CopyHard(&eStack96, &pCVar2->transMatrix_0x390);
					eStack96.da = (pCVar17->base).currentPosition.x;
					eStack96.db = (pCVar17->base).currentPosition.y;
					eStack96.dc = (pCVar17->base).currentPosition.z;
					eStack96.dd = (pCVar17->base).currentPosition.w;
					edF32Matrix4MulF32Vector4Hard(&local_70, &eStack96, &local_70);
					peVar5 = pCVar3->pCurrentPatch->pVertex + (pCVar17->base).field_0x60 * 4;
					peVar5->x = local_70.x;
					peVar5->y = local_70.y;
					peVar5->z = local_70.z;
					iVar19 = (pCVar17->base).field_0x60;
					iVar6 = rand();
					p_Var1 = pCVar3->pCurrentPatch->pRgba;
					p_Var8 = p_Var1 + iVar19 * 4;
					gpCurPatchRGBABuf = 0;
					local_4 = (_rgba)((uint)p_Var1[iVar19 * 4] & 0xffffff |
						(int)(((float)iVar6 / 2.147484e+09) * 96.0 + 64.0) << 0x18);
					*p_Var8 = local_4;
					p_Var8[1] = local_4;
					p_Var8[2] = local_4;
					p_Var8[3] = local_4;
				}
				pCVar17 = pCVar17 + 1;
			}
			GameDListPatch_EndCurrent(-1, 0);
		}
		pCVar3 = GameDListPatch_BeginCurrent(this->animPatchId);
		if (pCVar3 != (CGlobalDListPatch*)0x0) {
			pCVar17 = this->aBnsInstances;
			for (uVar16 = 0; uVar16 < (uint)this->nbInstances; uVar16 = uVar16 + 1) {
				uVar7 = (pCVar17->base).flags;
				if ((uVar7 & 4) == 0) {
					if ((uVar7 & 2) != 0) {
						iVar19 = this->animPatchId;
						p_Var1 = (this_00->ppGlobalDlist[iVar19 >> 0x10].pDlistPatch)->pCurrentPatch->pRgba;
						p_Var8 = p_Var1 + uVar16 * 4;
						local_10 = (_rgba)((uint)p_Var1[uVar16 * 4] & 0xffffff);
						gpCurPatchRGBABuf = 0;
						*p_Var8 = local_10;
						p_Var8[1] = local_10;
						p_Var8[2] = local_10;
						p_Var8[3] = local_10;
						CGlobalDListManager::_AddCallFuncElement2(this_00, iVar19, 9, (short)uVar16, 0);
					}
				}
				else {
					fVar21 = (pCVar17->base).angleRotY;
					uVar18 = (uint)fVar21;
					uVar7 = uVar18 & 7;
					if (((int)uVar18 < 0) && (uVar7 != 0)) {
						uVar7 = uVar7 - 8;
					}
					local_18 = (float)uVar7 * 0.125;
					iVar19 = (int)fVar21;
					if (iVar19 < 0) {
						iVar19 = iVar19 + 7;
					}
					iVar6 = (pCVar17->base).field_0x60;
					iVar12 = iVar6 * 4;
					uVar7 = pCVar3->pCurrentPatch->pSt;
					local_14 = (float)(iVar19 >> 3) * 0.5;
					puVar15 = (undefined2*)(uVar7 + iVar6 * 0x10);
					puVar14 = (undefined2*)(uVar7 + (iVar12 + 1) * 4);
					puVar11 = (undefined2*)(uVar7 + (iVar12 + 3) * 4);
					puVar13 = (undefined2*)(uVar7 + (iVar12 + 2) * 4);
					uVar10 = (undefined2)(int)((local_18 + 0.125) * 4096.0);
					*puVar15 = uVar10;
					puVar15[1] = (short)(int)((local_14 + 0.5) * 4096.0);
					uVar9 = (undefined2)(int)(local_18 * 4096.0);
					*puVar14 = uVar9;
					puVar14[1] = (short)(int)((local_14 + 0.5) * 4096.0);
					*puVar13 = uVar10;
					puVar13[1] = (short)(int)(local_14 * 4096.0);
					*puVar11 = uVar9;
					puVar11[1] = (short)(int)(local_14 * 4096.0);
					fVar21 = (pCVar17->base).currentPosition.y;
					fVar20 = (pCVar17->base).currentPosition.z;
					peVar5 = pCVar3->pCurrentPatch->pVertex + (pCVar17->base).field_0x60 * 4;
					peVar5->x = (pCVar17->base).currentPosition.x;
					peVar5->y = fVar21;
					peVar5->z = fVar20;
					iVar19 = (pCVar17->base).field_0x60;
					p_Var1 = pCVar3->pCurrentPatch->pRgba;
					p_Var8 = p_Var1 + iVar19 * 4;
					local_8 = (_rgba)CONCAT13(0x80, SUB43(p_Var1[iVar19 * 4], 0));
					gpCurPatchRGBABuf = 0;
					*p_Var8 = local_8;
					p_Var8[1] = local_8;
					p_Var8[2] = local_8;
					p_Var8[3] = local_8;
				}
				pCVar17 = pCVar17 + 1;
			}
			GameDListPatch_EndCurrent(-1, 0);
		}
		if (this->aShadowShared != (CShadowShared*)0x0) {
			pCVar3 = GameDListPatch_BeginCurrent(this->field_0x2c);
			uVar16 = 0;
			if (pCVar3 != (CGlobalDListPatch*)0x0) {
				for (; uVar16 < (uint)this->nbSharedShadows; uVar16 = uVar16 + 1) {
					CShadowShared::Draw(this->aShadowShared + uVar16, pCVar3, uVar16 * 4);
				}
				GameDListPatch_EndCurrent(-1, 0);
			}
		}
	}
	return;)
}

void CBehaviourBonusFlock::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourBonusBase::Begin(pOwner, newState, newAnimationType);

	CActorBonus* pActorBonus = this->pOwner;

	pActorBonus->field_0xf0 = 15.0f;
	pActorBonus->ComputeAltitude();
	pActorBonus->field_0xf0 = pActorBonus->distanceToGround;
	pActorBonus->flags &= 0xfffffffc;
	pActorBonus->flags &= 0xffffff5f;
	pActorBonus->EvaluateDisplayState();

	// Reset all instances
	for (int i = 0; i < this->nbInstances; ++i) {
		CActInstance* pInstance = reinterpret_cast<CActInstance*>(this->aBnsInstances + i);
		pInstance->Reset();
		pInstance->field_0x98 = 0.0f;
	}

	// Reset all shared shadows
	for (int i = 0; i < this->nbSharedShadows; ++i) {
		IMPLEMENTATION_GUARD_SHADOW(
		CShadowShared* pShadow = this->aShadowShared + i;
		pShadow->Reset();
		CBnsInstance* pNextInstance = this->aBnsInstances + i + 1;
		pShadow->field_0x10 = pNextInstance->field_0x30;
		pShadow->field_0x18 = pNextInstance->field_0x38;)
	}

	this->field_0x20 = 1;

	pOwner->rotationEuler.x = 0.0f;
	pOwner->rotationEuler.y = 0.0f;
	pOwner->rotationEuler.z = 0.0f;

	pOwner->UpdatePosition(&pOwner->baseLocation, true);

	this->ChangeVisibleState(1);

	return;
}

bool CBehaviourBonusFlock::InitDlistPatchable(int patchId)
{
	C3DFileManager* pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	bool bSuccess = false;
	CBnsInstance* pInstance = this->aBnsInstances;
	CShadowShared* pShadowShared = this->aShadowShared;

	if (patchId == this->flarePatchId) {
		edDListLoadIdentity();
		edDList_material* pMaterial = pFileManager->GetMaterialFromId(this->pOwner->flareMaterialId, 0);
		edDListUseMaterial(pMaterial);
		edDListBegin(0.0f, 0.0f, 0.0f, 0xb, this->nbInstances);
		for (int i = 0; i < this->nbInstances; ++i) {
			pInstance->InitDlistPatchable(0);
			++pInstance;
		}

		edDListEnd();
		bSuccess = true;
	}
	else if (patchId == this->animPatchId) {
		edDListLoadIdentity();
		edDList_material* pMaterial = pFileManager->GetMaterialFromId(this->pOwner->animMaterialId, 0);
		edDListUseMaterial(pMaterial);
		edDListBegin(0.0f, 0.0f, 0.0f, 0xb, this->nbInstances);

		for (int i = 0; i < this->nbInstances; ++i) {
			pInstance->InitDlistPatchable(1);
			++pInstance;
		}

		edDListEnd();
		bSuccess = true;
	}
	else if ((this->nbSharedShadows != 0) && (patchId == this->field_0x2c)) {
		IMPLEMENTATION_GUARD_SHADOW(
		edDListLoadIdentity();
		edDList_material* pMaterial = pFileManager->GetMaterialFromId(this->pOwner->pShadow->field_0x54, 0);
		edDListUseMaterial(pMaterial);
		edDListBegin(0.0f, 0.0f, 0.0f, 8, this->nbSharedShadows << 2);
		for (int i = 0; i < this->nbSharedShadows; ++i) {
			uint color = pShadowShared->field_0x30;
			byte r = (byte)color;
			byte g = (byte)(color >> 8);
			byte b = (byte)(color >> 16);
			edDListColor4u8(r, g, b, 0);
			edDListTexCoo2f(0.0f, 0.0f);
			edDListVertex4f(-0.5f, 0.01f, -0.5f, 0.0f);
			edDListTexCoo2f(0.0f, 1.0f);
			edDListVertex4f(-0.5f, 0.01f, 0.5f, 0.0f);
			edDListTexCoo2f(1.0f, 0.0f);
			edDListVertex4f(0.5f, 0.01f, -0.5f, 0.0f);
			edDListTexCoo2f(1.0f, 1.0f);
			edDListVertex4f(0.5f, 0.01f, 0.5f, 0.0f);
			++pShadowShared;
		}

		edDListEnd();)
		bSuccess = true;
	}

	return bSuccess;
}

void CBehaviourBonusFlock::KillInstance(CBnsInstance* pInstance)
{
	uint instanceIndex;
	int iVar1;

	instanceIndex = (uint)((int)pInstance - (int)this->aBnsInstances) / 0xc0;
	if ((pInstance->flags & 1) != 0) {
		pInstance->SetAlive(0);

		GameDListPatch_HideSprite(this->flarePatchId, instanceIndex);
		GameDListPatch_HideSprite(this->animPatchId, instanceIndex);

		if ((instanceIndex != 0) && (this->nbSharedShadows != 0)) {
			iVar1 = (instanceIndex - 1) * 4;
			GameDListPatch_HideVertex(this->field_0x2c, iVar1 + 2);
			GameDListPatch_HideVertex(this->field_0x2c, iVar1 + 3);
		}
	}

	return;
}

void CBehaviourBonusFlock::ChangeVisibleState(int state)
{
	CGlobalDListManager* pDlist;

	if (this->nbInstances != 0) {
		pDlist = (CGlobalDListManager*)CScene::GetManager(MO_GlobalDListManager);
		pDlist->SetActive(this->flarePatchId, state);
		pDlist->SetActive(this->animPatchId, state);

		if (this->aShadowShared != (CShadowShared*)0x0) {
			pDlist->SetActive(this->field_0x2c, state);
		}
	}

	return;
}

void CBnsInstance::InitBns(CActor* pOwner, edF32VECTOR4* pPosition, edF32VECTOR4* pBoundSphere)
{
	this->Init(pOwner, pPosition, pBoundSphere, 0);

	this->flags |= 8;
	this->field_0x98 = 0.0f;
	this->field_0x90 = (static_cast<float>(rand()) / 2.147484e+09f) * 6.283185f;
	this->field_0x9c = (static_cast<float>(rand()) / 2.147484e+09f) * 6.283185f;
	this->field_0xa0 = (static_cast<float>(rand()) / 2.147484e+09f) * 6.283185f;
	this->field_0xa4 = (static_cast<float>(rand()) / 2.147484e+09f) * 6.283185f;
	this->angleRotY = (static_cast<float>(rand()) / 2.147484e+09f) * 16.0f;

	return;
}

static uint gBonusSpriteColor = 0x80808080;

bool CBnsInstance::InitDlistPatchable(int bIsAnim)
{
	bool bSuccess;
	uint color;
	uint texIndex;
	float s, t;
	byte r, g, b;

	color = gBonusSpriteColor;

	if (bIsAnim == 0) {
		edDListTexCoo2f(0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);

		edDListWidthHeight2f(0.1714286f, 0.1714286f);

		r = (byte)color;
		g = (byte)(color >> 8);
		b = (byte)(color >> 16);
		edDListColor4u8(r, g, b, 0);
		edDListVertex4f(this->currentPosition.x, this->currentPosition.y, this->currentPosition.z, 0.0f);

		bSuccess = true;
	}
	else if (bIsAnim == 1) {
		// Pick a random texture index from 0 to 6
		texIndex = rand();
		uint sIndex = texIndex & 7;

		if ((int)texIndex < 0 && sIndex != 0) {
			sIndex -= 8;
		}

		s = (float)sIndex * 0.125f;
		if ((int)texIndex < 0) {
			texIndex += 7;
		}

		t = (float)((int)texIndex >> 3) * 0.5f;
		edDListTexCoo2f(s, t);
		edDListTexCoo2f(s + 0.125f, t + 0.5f);
		edDListWidthHeight2f(0.6f, 0.6f);
		edDListColor4u8(0x80, 0x80, 0x80, 0);
		edDListVertex4f(this->currentPosition.x, this->currentPosition.y, this->currentPosition.z, 0.0f);

		bSuccess = true;
	}
	else {
		bSuccess = false;
	}

	return bSuccess;
}

void CBnsInstance::BehaviourTurn_Manage(CBehaviourBonusTurn* pBehaviour)
{
	int curState;
	CActorBonus* pOwner;
	CFxTail* pFxTail;
	bool isVisible;
	float angleRotY, currentY, currentZ, nextY, nextZ, nextW;

	if ((this->flags & 1) != 0) {
		curState = this->state;
		if ((curState == 5) || (curState == 4)) {
			IMPLEMENTATION_GUARD_LIGHT(
			CActorBonus::_gBNS_Lights.Update(this, &this->currentPosition);
			CActorBonus::_gBNS_Lights.Unregister(this);)
			pBehaviour->KillInstance(this);
		}
		else {
			if (curState == 3) {
				this->State_GotoKim();
			}
			else if ((curState == 2) || (curState == 1)) {
				IMPLEMENTATION_GUARD(
				State_Turn(pBehaviour);)
			}
		}

		if ((this->flags & 4) == 0) {
			this->flags = this->flags & 0xfffffffd;
		}
		else {
			this->flags = this->flags | 2;
		}

		if ((this->flags & 1) != 0) {
			angleRotY = this->angleRotY + GetTimer()->cutsceneDeltaTime * 24.0f;
			this->angleRotY = angleRotY;

			if (16.0f <= angleRotY) {
				this->angleRotY = angleRotY - 16.0f;
			}

			this->FUN_003982c0();
			this->field_0x5c = this->field_0x5c + GetTimer()->cutsceneDeltaTime;
			this->field_0x64 = this->currentPosition.xyz;
		}

		pOwner = static_cast<CActorBonus*>(this->pOwner);
		isVisible = ((pOwner->flags & 0x4400) != 0);
		if (isVisible) {
			isVisible = pOwner->distanceToCamera <= pOwner->subObjA->field_0x1c;
		}

		if (isVisible) {
			nextY = this->field_0x9c;
			nextZ = pOwner->rotationEuler.y;
			nextW = pOwner->rotationEuler.z;
			pFxTail = pOwner->pFxTail;
			pFxTail->field_0x60 = pFxTail->field_0x50;
			pFxTail->field_0x50.x = nextY;
			pFxTail->field_0x50.y = nextZ;
			pFxTail->field_0x50.z = nextW;
			pFxTail->field_0x50.w = 0.0f;
		}
	}

	return;
}

void CBnsInstance::BehaviourPath_Manage(CBehaviourBonusPath* pBehaviour)
{
	CActorBonus* pOwner;
	CFxTail* pFxTail;
	CActorHero* pFleeActor;
	bool atGoal;
	int msgResult;
	float angleRotY, posY, posZ, tailY, tailZ, tailW;

	pFleeActor = CActorHero::_gThis;
	if ((this->flags & 1) == 0) {

		return;
	}
	switch (this->state) {
	case 0:
		if (pBehaviour->field_0x1b8 <= this->field_0x5c) {
			this->SetState(1);
		}
		break;
	case 1:
		ComputeDistanceToKim((edF32VECTOR4*)0x0);

		if (this->distanceToKim <= 2.0f) {
			msgResult = this->pOwner->DoMessage(CActorHero::_gThis, (ACTOR_MESSAGE)9, (MSG_PARAM)0xffffffff);
			if (msgResult != 0) {
				this->SetState(3);
				break;
			}
		}

		if (this->distanceToKim <= pBehaviour->field_0x1f8 * 3.0f) {
			this->SetState(2);
			pBehaviour->pathPlane.InitTargetPos(&CActorHero::_gThis->currentLocation, &pBehaviour->pathPlane.outData);
		}

		MoveOnPath(0.0f, pBehaviour->field_0x200, pBehaviour);
		break;
	case 2:
		this->field_0xa8.field_0x4 = 1;
		this->field_0xa8.delay = pBehaviour->field_0x1f8;
		pBehaviour->pOwner->SV_MOV_ComputeSpeedAccelerationToFleeActor(3.0f, pFleeActor, &pBehaviour->pathPlane, &this->field_0xa8);
		atGoal = pBehaviour->pathPlane.pathFollowReader.AtGoal(pBehaviour->pathPlane.pathFollowReader.splinePointIndex, pBehaviour->pathPlane.pathFollowReader.field_0xc);
		if (!atGoal) {
			MoveOnPath(this->field_0xa8.acceleration, this->field_0xa8.speed, pBehaviour);
		}

		ComputeDistanceToKim((edF32VECTOR4*)0x0);

		if (this->distanceToKim <= 2.0f) {
			msgResult = this->pOwner->DoMessage(CActorHero::_gThis, (ACTOR_MESSAGE)9, (MSG_PARAM)0xffffffff);
			if (msgResult != 0) {
				this->SetState(3);
			}
		}

		if (pBehaviour->field_0x1f8 * 3.0f <= this->distanceToKim) {
			this->SetState(1);
		}
		break;
	case 3:
		this->State_GotoKim();
		break;
	case 4:
	case 5:
		IMPLEMENTATION_GUARD_LIGHT(
		CActorBonus::_gBNS_Lights.Update(this, &this->currentPosition);
		CActorBonus::_gBNS_Lights.Unregister(this);)
		pBehaviour->KillInstance(this);
		break;
	}

	if ((this->flags & 4) == 0) {
		this->flags = this->flags & 0xfffffffd;
	}
	else {
		this->flags = this->flags | 2;
	}

	if ((this->flags & 1) != 0) {
		angleRotY = this->angleRotY + GetTimer()->cutsceneDeltaTime * 24.0f;
		this->angleRotY = angleRotY;

		if (16.0f <= angleRotY) {
			this->angleRotY = angleRotY - 16.0f;
		}

		this->FUN_003982c0();
		this->field_0x5c = this->field_0x5c + GetTimer()->cutsceneDeltaTime;
		this->field_0x64 = this->currentPosition.xyz;
	}

	pOwner = static_cast<CActorBonus*>(this->pOwner);
	atGoal = ((pOwner->flags & 0x4400) != 0);
	if (atGoal) {
		atGoal = pOwner->distanceToCamera <= pOwner->subObjA->field_0x1c;
	}

	if (atGoal) {
		float nextY = this->field_0x9c;
		float nextZ = pOwner->rotationEuler.y;
		float nextW = pOwner->rotationEuler.z;
		pFxTail = pOwner->pFxTail;

		tailY = pFxTail->field_0x50.y;
		tailZ = pFxTail->field_0x50.z;
		tailW = pFxTail->field_0x50.w;
		pFxTail->field_0x60 = pFxTail->field_0x50;
		pFxTail->field_0x50.x = nextY;
		pFxTail->field_0x50.y = nextZ;
		pFxTail->field_0x50.z = nextW;
		pFxTail->field_0x50.w = 0.0f;
	}

	return;
}

void CBnsInstance::BehaviourAddOn_Manage(CBehaviourBonusFlock* pBehaviour)
{
	IMPLEMENTATION_GUARD();
}

void CBnsInstance::BehaviourFlock_Manage(CBehaviourBonusFlock* pBehaviour)
{
	int curState;
	edF32VECTOR4 newPosition;

	if ((this->flags & 1) != 0) {
		curState = this->state;
		if ((curState == 5) || (curState == 4)) {
			IMPLEMENTATION_GUARD_LIGHT(
			CActorBonus::_gBNS_Lights.Update(this, &this->currentPosition);
			CActorBonus::_gBNS_Lights.Unregister(this);)
			pBehaviour->KillInstance(this);
		}
		else {
			if (curState == 3) {
				State_GotoKim();
			}
			else {
				if ((curState == 2) || (curState == 1)) {
					State_Wait();

					this->field_0xa0 += GetTimer()->cutsceneDeltaTime * 1.8f;
					if (6.283185f <= this->field_0xa0) {
						this->field_0xa0 = this->field_0xa0 - 6.283185f;
					}

					this->field_0xa4 += GetTimer()->cutsceneDeltaTime * 2.2f;
					if (6.283185f <= this->field_0xa4) {
						this->field_0xa4 = this->field_0xa4 - 6.283185f;
					}

					SetBasePosition(&this->basePosition);

					newPosition.x = cosf(this->field_0xa4);
					newPosition.y = cosf(this->field_0xa0);
					newPosition.z = sinf(this->field_0xa4);
					newPosition.w = 0.0f;

					edF32Vector4ScaleHard(0.25f, &newPosition, &newPosition);
					edF32Vector4AddHard(&newPosition, &this->basePosition, &newPosition);

					this->SetPosition(&newPosition);
				}
			}
		}

		if ((this->flags & 4) == 0) {
			this->flags = this->flags & 0xfffffffd;
		}
		else {
			this->flags = this->flags | 2;
		}

		if ((this->flags & 1) != 0) {
			float tmpf1 = this->angleRotY + GetTimer()->cutsceneDeltaTime * 24.0f;
			this->angleRotY = tmpf1;

			if (16.0f <= tmpf1) {
				this->angleRotY = tmpf1 - 16.0f;
			}

			this->FUN_003982c0();
			this->field_0x5c = this->field_0x5c + GetTimer()->cutsceneDeltaTime;
			this->field_0x64 = this->currentPosition.xyz;
		}
	}

	return;
}

void CBnsInstance::MoveOnPath(float param_1, float speed, CBehaviourBonusPath* pBehaviour)
{
	bool atGoal;
	edF32VECTOR4* pWayPoint;
	float distToWayPoint;
	float newField_0x98;
	edF32VECTOR4 newPosition;

	pWayPoint = pBehaviour->pathPlane.pathFollowReader.GetWayPoint();
	edF32Vector4SubHard(&this->pathDelta, pWayPoint, &this->currentPosition);
	distToWayPoint = edF32Vector4GetDistHard(&this->pathDelta);

	if (this->field_0x98 < param_1) {
		newField_0x98 = this->field_0x98 + speed * GetTimer()->cutsceneDeltaTime;
		this->field_0x98 = newField_0x98;
		if (param_1 < newField_0x98) {
			this->field_0x98 = param_1;
		}
	}
	else {
		newField_0x98 = this->field_0x98 - speed * GetTimer()->cutsceneDeltaTime;
		this->field_0x98 = newField_0x98;
		if (newField_0x98 < param_1) {
			this->field_0x98 = param_1;
		}
	}

	float max = this->field_0x98;
	float scale = (max * GetTimer()->cutsceneDeltaTime) / distToWayPoint;
	edF32Vector4ScaleHard(scale, &this->pathDelta, &this->pathDelta);
	edF32Vector4AddHard(&newPosition, &this->currentPosition, &this->pathDelta);
	this->SetPosition(&newPosition);
	this->field_0x64 = newPosition.xyz;
	max = this->field_0x98;
	if (distToWayPoint <= max * GetTimer()->cutsceneDeltaTime) {
		atGoal = pBehaviour->pathPlane.pathFollowReader.AtGoal(
			pBehaviour->pathPlane.pathFollowReader.splinePointIndex,
			pBehaviour->pathPlane.pathFollowReader.field_0xc);

		if (!atGoal) {
			pBehaviour->pathPlane.pathFollowReader.NextWayPoint();
		}
		else {
			this->SetState(1);
		}
	}

	return;
}
