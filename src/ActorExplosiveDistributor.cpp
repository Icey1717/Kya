#include "ActorExplosiveDistributor.h"
#include "MemoryStream.h"
#include "CollisionManager.h"
#include "CinematicManager.h"
#include "MathOps.h"
#include "ActorProjectile.h"

CActorExplosiveDistributor::CActorExplosiveDistributor()
{
	this->pStreamEventCamera = (S_STREAM_EVENT_CAMERA*)0x0;
	this->field_0x168 = 0;
	this->nbProjectiles = 0;
	this->aProjectiles = (_s_projectile*)0x0;
	this->field_0x190 = 0;

	return;
}

void CActorExplosiveDistributor::Create(ByteCode* pByteCode)
{
	bool bVar1;
	S_STREAM_EVENT_CAMERA* pSVar2;
	CCollision* pCollision;
	float fVar4;
	float fVar5;
	uint uVar6;
	int* pBase;
	_s_projectile* peVar7;
	CActor* pCVar7;
	_s_projectile* pCurProjectile;
	float fVar9;
	char acStack1024[1024];

	CActor::Create(pByteCode);

	this->field_0x160 = pByteCode->GetF32();
	this->field_0x160 = this->field_0x160 * this->field_0x160;

	this->field_0x164 = pByteCode->GetF32();
	this->field_0x164 = cosf(this->field_0x164);

	this->nbProjectiles = pByteCode->GetU32();
	uVar6 = this->nbProjectiles;
	if (uVar6 != 0) {
		this->aProjectiles = new _s_projectile[uVar6];

		pCurProjectile = this->aProjectiles;
		uVar6 = this->nbProjectiles;
		while (bVar1 = uVar6 != 0, uVar6 = uVar6 - 1, bVar1) {
			pCurProjectile->actorRef.index = pByteCode->GetS32();

			if (CScene::_pinstance->field_0x1c < 2.25f) {
				pByteCode->GetF32();
				pByteCode->GetF32();
				pByteCode->GetF32();
				(pCurProjectile->field_0x0).x = 0.0f;
				(pCurProjectile->field_0x0).y = 1.0f;
				(pCurProjectile->field_0x0).z = 1.0f;
				(pCurProjectile->field_0x0).w = 0.0f;
			}
			else {
				(pCurProjectile->field_0x0).x = pByteCode->GetF32();
				(pCurProjectile->field_0x0).y = pByteCode->GetF32();
				(pCurProjectile->field_0x0).z = pByteCode->GetF32();
				(pCurProjectile->field_0x0).w = 0.0f;

				fVar9 = edF32Vector4GetDistHard(&pCurProjectile->field_0x0);
				if (fVar9 == 0.0) {
					pCurProjectile->field_0x0 = gF32Vector4UnitZ;
				}
				else {
					edF32Vector4ScaleHard(1.0f / fVar9, &pCurProjectile->field_0x0, &pCurProjectile->field_0x0);
				}
			}

			pCurProjectile->ejectForce = pByteCode->GetF32() * 1.414213;
			if (pCurProjectile->ejectForce < 100.0f) {
				sprintf(acStack1024, "eject force too low for proj. %d\n", this->nbProjectiles - uVar6);
				pCurProjectile->ejectForce = 100.0f;
			}

			pCurProjectile = pCurProjectile + 1;
		}

		(this->projectileSpawnLocation).x = 0.75f;
		(this->projectileSpawnLocation).y = 1.75f;
		(this->projectileSpawnLocation).z = 1.8f;
		(this->projectileSpawnLocation).w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&this->projectileSpawnLocation, &this->pMeshTransform->base.transformA, &this->projectileSpawnLocation);
	}

	pSVar2 = (S_STREAM_EVENT_CAMERA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pSVar2 + 1);
	this->pStreamEventCamera = pSVar2;

	pCollision = this->pCollisionData;
	pCollision->flags_0x0 = pCollision->flags_0x0 | 0x5403;
	pCollision->flags_0x0 = pCollision->flags_0x0 & 0xfffc5e06;

	return;
}

void CActorExplosiveDistributor::Init()
{
	CCollision* pCollision;
	uint projectileIndex;
	_s_projectile* pCurProjectile;

	CActor::Init();

	projectileIndex = this->nbProjectiles;
	pCurProjectile = this->aProjectiles;

	while (projectileIndex != 0) {
		projectileIndex = projectileIndex - 1;
		pCurProjectile->actorRef.Init();

		pCollision = (pCurProjectile->actorRef).Get()->pCollisionData;
		if (pCollision != (CCollision*)0x0) {
			CCollision::PatchObbTreeFlagsRecurse(pCollision->pObbTree, 0x200, -1, 0);
		}

		pCurProjectile = pCurProjectile + 1;
	}

	this->pStreamEventCamera->Init();
	this->pProjectile = (_s_projectile*)0x0;

	return;
}

void CActorExplosiveDistributor::Draw()
{
	return;
}

void CActorExplosiveDistributor::Reset()
{
	CActor::Reset();
	this->pStreamEventCamera->Reset(this);
	this->pProjectile = (_s_projectile*)0x0;

	return;
}

void CActorExplosiveDistributor::CheckpointReset()
{
	return;
}

CBehaviour* CActorExplosiveDistributor::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourExplosiveDistributor;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorExplosiveDistributor::_gStateCfg_ExD[2] = {
	StateConfig(0x6, 0x0),
	StateConfig(0x0, 0x0),
};

StateConfig* CActorExplosiveDistributor::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_ExD + state + -5;
	}

	return pStateConfig;
}

bool CActorExplosiveDistributor::DistributeExplosive()
{
	bool bVar1;

	this->pProjectile = this->aProjectiles;
	int uVar4 = this->nbProjectiles;

	while ((bVar1 = uVar4 != 0, uVar4 = uVar4 - 1, bVar1 && (this->pProjectile->actorRef.Get()->actorState != 6))) {
		this->pProjectile = this->pProjectile + 1;
	}

	if (uVar4 < 0) {
		this->pProjectile = (_s_projectile*)0x0;
	}

	return this->pProjectile != (_s_projectile*)0x0;
}

void CBehaviourExplosiveDistributor::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourExplosiveDistributor::Manage()
{
	CAnimation* pAnimationController;
	CActorProjectile* pProjectile;
	edAnmLayer* peVar2;
	bool bVar3;
	float fVar4;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;
	CActorExplosiveDistributor* pExplosiveDistributor;

	pExplosiveDistributor = this->pOwner;

	pExplosiveDistributor->pStreamEventCamera->Manage(pExplosiveDistributor);

	if (pExplosiveDistributor->pTiedActor != (CActor*)0x0) {
		local_10 = pExplosiveDistributor->baseLocation;
		pExplosiveDistributor->SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		(pExplosiveDistributor->projectileSpawnLocation).x = 0.75f;
		(pExplosiveDistributor->projectileSpawnLocation).y = 1.75f;
		(pExplosiveDistributor->projectileSpawnLocation).z = 1.8f;
		(pExplosiveDistributor->projectileSpawnLocation).w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&pExplosiveDistributor->projectileSpawnLocation, &pExplosiveDistributor->pMeshTransform->base.transformA, &pExplosiveDistributor->projectileSpawnLocation);
	}

	if (pExplosiveDistributor->actorState == 5) {
		pAnimationController = pExplosiveDistributor->pAnimationController;
		if (pAnimationController == (CAnimation*)0x0) {
			pProjectile = static_cast<CActorProjectile*>(pExplosiveDistributor->pProjectile->actorRef.Get());
			pProjectile->pCollisionData->Reset();

			pProjectile->UpdatePosition(&pExplosiveDistributor->projectileSpawnLocation, true);

			edF32Matrix4MulF32Vector4Hard(&eStack48, &pExplosiveDistributor->pMeshTransform->base.transformA, &pExplosiveDistributor->pProjectile->field_0x0);
			edF32Vector4NormalizeHard(&eStack48, &eStack48);
			pProjectile->Project(pExplosiveDistributor->pProjectile->ejectForce, &eStack48, true, (CActor*)0x0);
			pExplosiveDistributor->SetState(0, -1);
		}
		else {
			pAnimationController->anmBinMetaAnimator.GetAnimType_00242330(0);

			if (((pAnimationController->anmBinMetaAnimator).aAnimData)->animPlayState == 1) {
				fVar4 = pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0, 1);
			}
			else {
				fVar4 = 0.0f;
			}

			if (((0.7f <= fVar4) && ((((pAnimationController->anmBinMetaAnimator).aAnimData)->currentAnimDesc).animType == pAnimationController->currentAnimType_0x30)) &&
				(pExplosiveDistributor->field_0x190 == 0)) {
				pProjectile = static_cast<CActorProjectile*>(pExplosiveDistributor->pProjectile->actorRef.Get());

				pProjectile->pCollisionData->Reset();
				pProjectile->UpdatePosition(&pExplosiveDistributor->projectileSpawnLocation, true);
				edF32Matrix4MulF32Vector4Hard(&eStack32, &pExplosiveDistributor->pMeshTransform->base.transformA, &pExplosiveDistributor->pProjectile->field_0x0);
				edF32Vector4NormalizeHard(&eStack32, &eStack32);
				pProjectile->Project(pExplosiveDistributor->pProjectile->ejectForce, &eStack32, true, (CActor*)0x0);
				pExplosiveDistributor->field_0x190 = 1;
			}

			if (pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				pExplosiveDistributor->field_0x190 = 0;
				pExplosiveDistributor->SetState(0, -1);
			}
		}
	}

	return;
}

void CBehaviourExplosiveDistributor::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorExplosiveDistributor*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

int CBehaviourExplosiveDistributor::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	CActorExplosiveDistributor* pExplosiveDistributor;
	CAnimation* pCVar3;
	int uVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32MATRIX4 auStack256;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 local_b0;
	edF32MATRIX4 eStack160;
	undefined auStack96[52];
	float local_2c;
	float local_28;
	float local_24;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
		GetPositionMsgParams* pGetPosMsgParam = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
		if (((pGetPosMsgParam->field_0x0 == 1) || (pGetPosMsgParam->field_0x0 == 0)) && (pCVar3 = this->pOwner->pAnimationController, pCVar3 != (CAnimation*)0x0)) {
			pCVar3->GetDefaultBoneMatrix(0x48535550, &auStack256);
			edF32Matrix4MulF32Matrix4Hard(&auStack256, &auStack256, &this->pOwner->pMeshTransform->base.transformA);
			edF32Vector4SubHard(&auStack256.rowZ, &auStack256.rowZ, &this->pOwner->currentLocation);
			pGetPosMsgParam->vectorFieldB = auStack256.rowT;
		}
	}
	else {
		if (((msg != 0xf) && (msg != 0xe)) && (msg != 0x14)) {
			if (msg != 2) {
				if (msg != 0x12) {
					return 0;
				}

				IMPLEMENTATION_GUARD(
				edF32Vector4SubHard(&local_10, &this->pOwner->currentLocation, &pSender->currentLocation);
				fVar5 = edF32Vector4NormalizeHard(&eStack32, &local_10);
				if (3.0 <= fVar5) {
					return 0;
				}
				if (ABS((float)(uint)(local_10.y < 0.5)) == 0.0) {
					return 0;
				}
				pExplosiveDistributor = this->pOwner;
				if (pExplosiveDistributor->pAnimationController == (CAnimation*)0x0) {
					return 0;
				}

				if (!this->pOwner->DistributeExplosive()) {
					return 0;
				}

				CAnimation::GetDefaultBoneMatrix(this->pOwner->pAnimationController, 0x48535550, (edF32MATRIX4*)auStack96);
				edF32Matrix4MulF32Matrix4Hard
				((edF32MATRIX4*)auStack96, (edF32MATRIX4*)auStack96, (edF32MATRIX4*)this->pOwner->pMeshTransform);
				edF32Vector4SubHard(&local_10, (edF32VECTOR4*)(auStack96 + 0x30), &pSender->currentLocation);
				local_10.y = 0.0;
				if (this->pOwner->field_0x160 < local_10.z * local_10.z + local_10.x * local_10.x) {
					return 0;
				}
				edF32Vector4NormalizeHard(&local_10, &local_10);
				fVar5 = edF32Vector4DotProductHard(&local_10, &pSender->rotationQuat);
				pExplosiveDistributor = this->pOwner;
				if (fVar5 < pExplosiveDistributor->field_0x164) {
					return 0;
				}
				fVar7 = (pExplosiveDistributor->rotationQuat).y;
				fVar5 = (pExplosiveDistributor->rotationQuat).z;
				fVar6 = (pExplosiveDistributor->rotationQuat).w;
				pMsgParam->field_0x10 = (pExplosiveDistributor->rotationQuat).x;
				*(float*)&pMsgParam->field_0x14 = fVar7;
				*(float*)&pMsgParam->field_0x18 = fVar5;
				*(float*)&pMsgParam->field_0x1c = fVar6;
				edF32Vector4GetNegHard((edF32VECTOR4*)&pMsgParam->field_0x10, (edF32VECTOR4*)&pMsgParam->field_0x10);
				pMsgParam->projectileType = (int)auStack96._48_4_;
				pMsgParam->field_0x4 = (int)local_2c;
				pMsgParam->flags = (uint)local_28;
				pMsgParam->damage = local_24;
				return 8;)
			}

			_msg_hit_param* pHitMsgParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
			
			if (pHitMsgParam->projectileType == 8) {
				pCVar3 = this->pOwner->pAnimationController;
				if (pCVar3 != (CAnimation*)0x0) {
					pCVar3->GetDefaultBoneMatrix(0x48535550, &eStack160);
					edF32Matrix4MulF32Matrix4Hard(&eStack160, &eStack160, &this->pOwner->pMeshTransform->base.transformA);
					local_b0 = eStack160.rowT;
					edF32Vector4SubHard(&eStack192, &local_b0, &pHitMsgParam->field_0x40);
					fVar5 = edF32Vector4GetDistHard(&eStack192);
					if (1.0f < fVar5) {
						return 0;
					}
				}
			}
			else {
				if (pHitMsgParam->projectileType != 4) {
					return 0;
				}
			}
		}

		pExplosiveDistributor = this->pOwner;
		if (pExplosiveDistributor->actorState == 0) {
			this->pOwner->DistributeExplosive();

			if (this->pOwner->DistributeExplosive()) {
				this->pOwner->SetState(5, -1);
				this->pOwner->pStreamEventCamera->SwitchOn(this->pOwner);
			}

			return 1;
		}
	}

	return 0;
}
