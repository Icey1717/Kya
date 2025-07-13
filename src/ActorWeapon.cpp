#include "ActorWeapon.h"
#include "MemoryStream.h"
#include "ActorWolfen.h"
#include "CollisionRay.h"
#include "MathOps.h"

void CActorWeapon::Create(ByteCode* pByteCode)
{
	CActorMovable::Create(pByteCode);

	this->field_0x1d4 = 0;
	this->field_0x1d8 = pByteCode->GetF32();
	this->field_0x1dc = pByteCode->GetF32();
	this->field_0x1d0 = 0;

	return;
}

void CActorWeapon::Init()
{
	CActor::Init();
	this->field_0x1d4 = 1;
	this->pOwningWolfen = (CActorWolfen*)0x0;
	this->flags = this->flags | 0x400;

	return;
}

void CActorWeapon::Term()
{
	CActor::Term();
	this->field_0x1d4 = 0;

	return;
}

void CActorWeapon::Reset()
{
	UpdatePosition(&this->baseLocation, true);
	CActorMovable::Reset();

	return;
}

void CActorWeapon::CheckpointReset()
{
	Reset();

	return;
}

CBehaviour* CActorWeapon::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == WEAPON_BEHAVIOUR_RPG) {
		pBehaviour = new CBehaviourWeaponRpg;
	}
	else {
		if (behaviourType == 5) {
			IMPLEMENTATION_GUARD();
		}
		else {
			if (behaviourType == WEAPON_BEHAVIOUR_SNIPER) {
				pBehaviour = new CBehaviourWeaponSniper;
			}
			else {
				if (behaviourType == WEAPON_BEHAVIOUR_PISTOL) {
					pBehaviour = new CBehaviourWeaponPistol;
				}
				else {
					if (behaviourType == 2) {
						pBehaviour = new CBehaviourWeapon;
					}
					else {
						pBehaviour = CActor::BuildBehaviour(behaviourType);
					}
				}
			}
		}
	}

	return pBehaviour;
}

StateConfig CActorWeapon::_gStateCfg_WEA[9] =
{
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x7, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
};

StateConfig* CActorWeapon::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActorMovable::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 9);
		pStateConfig = _gStateCfg_WEA + state + -5;
	}

	return pStateConfig;
}

void CActorWeapon::UnlinkWeapon()
{
	UnlinkFromActor();
	this->field_0x1d4 = 3;
	this->SetState(7, -1);
	this->pOwningWolfen = (CActorWolfen*)0x0;
	return;
}

void CActorWeapon::LinkWeapon(CActorWolfen* pWolfen, uint boneId)
{
	int iVar1;
	CBehaviourWeapon* pWeaponBehaviour;
	bool lVar2;

	LinkToActor(pWolfen, boneId, 0);
	this->field_0x1d4 = 1;

	iVar1 = this->curBehaviourId;
	if ((iVar1 < 2) || (6 < iVar1)) {
		lVar2 = false;
	}
	else {
		pWeaponBehaviour = static_cast<CBehaviourWeapon*>(GetBehaviour(this->curBehaviourId));
		lVar2 = pWeaponBehaviour->Func_0x50();
	}

	if (lVar2 == false) {
		SetState(WEAPON_STATE_IDLE, -1);
	}
	else {
		SetState(0xd, -1);
	}

	this->pOwningWolfen = pWolfen;

	return;
}

int CActorWeapon::FUN_002d57c0()
{
	CBehaviourWeaponBurst* pBurst = reinterpret_cast<CBehaviourWeaponBurst*>(GetBehaviour(this->curBehaviourId));
	return pBurst->field_0x38;
}

void CActorWeapon::FUN_002d57e0(int index)
{
	undefined4* puVar1;

	CBehaviourWeaponBurst* pBurst = reinterpret_cast<CBehaviourWeaponBurst*>(GetBehaviour(this->curBehaviourId));
	if (index != pBurst->field_0x38) {
		pBurst->field_0x8[index].field_0x10 = 0;
		pBurst->field_0x8[index].field_0x14 = 0.0f;
		pBurst->field_0x38 = index;
	}

	return;
}

bool CActorWeapon::FUN_002d58a0()
{
	int iVar1;
	bool uVar2;

	iVar1 = this->curBehaviourId;
	if ((iVar1 < 2) || (WEAPON_BEHAVIOUR_RPG < iVar1)) {
		uVar2 = false;
	}
	else {
		CBehaviourWeapon* pWeaponBehaviour = reinterpret_cast<CBehaviourWeapon*>(GetBehaviour(this->curBehaviourId));
		uVar2 = pWeaponBehaviour->Func_0x50();
	}

	return uVar2;
}

bool CActorWeapon::FUN_002d5830()
{
	CBehaviourWeaponBurst* pBurst = reinterpret_cast<CBehaviourWeaponBurst*>(GetBehaviour(this->curBehaviourId));
	return pBurst->Func_0x68();
}

float CActorWeapon::FUN_002d5710()
{
	CBehaviourWeaponBurst* pBurst = reinterpret_cast<CBehaviourWeaponBurst*>(GetBehaviour(this->curBehaviourId));
	return pBurst->field_0x8[pBurst->field_0x38].field_0xc;
}


float CActorWeapon::GetLaunchSpeed(int index)
{
	CBehaviourWeaponBurst* pBurst = static_cast<CBehaviourWeaponBurst*>(GetBehaviour(this->curBehaviourId));
	return pBurst->GetLaunchSpeed(index);
}

int CActorWeapon::GetBurstState()
{
	CBehaviourWeaponBurst* pBurst = static_cast<CBehaviourWeaponBurst*>(GetBehaviour(this->curBehaviourId));
	return pBurst->GetBurstState();
}

bool CActorWeapon::Action(edF32VECTOR4* pPosition, CActor* pActor)
{
	CBehaviourWeaponBurst* pBurst = static_cast<CBehaviourWeaponBurst*>(GetBehaviour(this->curBehaviourId));
	return pBurst->Action(pPosition, pActor);
}

void CBehaviourWeapon::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorWeapon*>(pOwner);

	return;
}

void CBehaviourWeapon::Manage()
{
	CActorWeapon* pWeapon;
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	CCollision* pCVar3;
	bool bVar4;
	float fVar5;

	pWeapon = this->pOwner;
	switch (pWeapon->actorState) {
	case 6:
		if (pWeapon->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pWeapon->SetState(WEAPON_STATE_IDLE, -1);
		}
		break;
	case 7:
		pWeapon->ManageDyn(4.0f, 0x2022b, (CActorsTable*)0x0);

		const static float FADE_DURATION = 0.5f;
		fVar5 = 1.0f - pWeapon->timeInAir / FADE_DURATION;
		if (fVar5 < 0.0f) {
			fVar5 = 0.0f;
		}

		pWeapon->SetAlpha((byte)(int)(fVar5 * 127.0f));

		pCVar3 = pWeapon->pCollisionData;
		if (((pCVar3 != (CCollision*)0x0) && ((pCVar3->flags_0x4 & 2) != 0)) || (fVar5 == 0.0f)) {
			pWeapon->ToggleMeshAlpha();
			pWeapon->SetState(8, -1);
		}
		break;
	case 8:
		pWeapon->ManageDyn(4.0f, 0x20, (CActorsTable*)0x0);
		break;
	case WEAPON_STATE_RELOAD:
		Func_0x5c();
		break;
	case 0xb:
		Func_0x54();
		break;
	case 0xc:
		Func_0x58();
	}
	return;
}

void CBehaviourWeapon::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (this->pOwner->GetLinkFather() != (CActor*)0x0) {
		this->pOwner->GetLinkFather();
		this->pOwner->UnlinkFromActor();
		this->pOwner->field_0x1d4 = 3;
		this->pOwner->SetState(7, -1);
		this->pOwner->pOwningWolfen = (CActorWolfen*)0x0;
	}

	if (newState == -1) {
		this->pOwner->SetState(8, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	CCollision* pCVar1 = this->pOwner->pCollisionData;
	if (pCVar1 != (CCollision*)0x0) {
		pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffeffd;
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x6001;
	}

	this->pOwner->field_0x1d0 = 1;

	return;
}

void CBehaviourWeapon::InitState(int newState)
{
	if (newState == 8) {
		this->pOwner->flags = this->pOwner->flags & 0xfffffffd;
		this->pOwner->flags = this->pOwner->flags | 1;

		this->pOwner->flags = this->pOwner->flags & 0xffffff7f;
		this->pOwner->flags = this->pOwner->flags | 0x20;

		this->pOwner->EvaluateDisplayState();
	}
	else {
		if ((newState == 7) && (this->pOwner->pMeshNode != (edNODE*)0x0)) {
			this->pOwner->SetBFCulling(1);
		}
	}

	return;
}

void CBehaviourWeapon::TermState(int oldState, int newState)
{
	if (oldState == 8) {
		this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
		this->pOwner->flags = this->pOwner->flags & 0xffffff5f;

		this->pOwner->EvaluateDisplayState();
	}
	else {
		if ((oldState == 7) && (this->pOwner->pMeshNode != (edNODE*)0x0)) {
			this->pOwner->SetBFCulling(0);
		}
	}

	return;
}

void CBehaviourWeaponBurst::Create(ByteCode* pByteCode)
{
	this->field_0x38 = 0;

	this->field_0x8[0].field_0xc = pByteCode->GetF32();
	this->field_0x8[0].field_0x0 = pByteCode->GetS32();
	this->field_0x8[0].field_0x4 = pByteCode->GetF32();
	this->field_0x8[0].field_0x8 = pByteCode->GetF32();

	this->field_0x8[1].field_0xc = pByteCode->GetF32();
	this->field_0x8[1].field_0x0 = pByteCode->GetS32();
	this->field_0x8[1].field_0x4 = pByteCode->GetF32();
	this->field_0x8[1].field_0x8 = pByteCode->GetF32();
	return;
}

void CBehaviourWeaponBurst::Manage()
{
	CBehaviourWeapon::Manage();

	UpdateBurst();

	return;
}

void CBehaviourWeaponBurst::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourWeapon::Begin(pOwner, newState, newAnimationType);

	this->field_0x8[this->field_0x38].field_0x10 = 0;
	this->field_0x8[this->field_0x38].field_0x14 = 0.0f;

	return;
}

int CBehaviourWeaponBurst::GetBurstState()
{
	int iVar1 = this->pOwner->actorState;
	if (iVar1 == WEAPON_STATE_RELOAD) {
		iVar1 = 3;
	}
	else {
		if (iVar1 == WEAPON_STATE_FIRE) {
			iVar1 = 2;
		}
		else {
			if (iVar1 == WEAPON_STATE_IDLE) {
				iVar1 = 1;
			}
			else {
				iVar1 = 0;
			}
		}
	}

	return iVar1;
}

float CBehaviourWeaponBurst::GetLaunchSpeed(int index)
{
	float launchSpeed = 1.0f;

	if (index == 3) {
		launchSpeed = this->field_0x8[this->field_0x38].field_0x8;
	}
	else {
		if (index == 2) {
			launchSpeed = 1.0f / this->field_0x8[this->field_0x38].field_0x0;
		}
	}

	return launchSpeed;
}

bool CBehaviourWeaponBurst::Action(edF32VECTOR4* pPosition, CActor* pActor)
{
	return false;
}

bool CBehaviourWeaponBurst::Func_0x68()
{
	int weaponState;
	bool bReturn;

	weaponState = this->pOwner->actorState;
	if ((weaponState == 0xd) || (weaponState == 0xc)) {
		bReturn = true;
	}
	else {
		bReturn = false;
	}

	return bReturn;
}

bool CBehaviourWeaponBurst::Func_0x74()
{
	return true;
}

bool CBehaviourWeaponBurst::Func_0x78()
{
	return true;
}

void CBehaviourWeaponBurst::UpdateBurst()
{
	this->field_0x8[this->field_0x38].field_0x14 = this->field_0x8[this->field_0x38].field_0x14 + GetTimer()->cutsceneDeltaTime;

	switch (this->pOwner->actorState) {
	case WEAPON_STATE_IDLE:
	case 0xd:
		this->field_0x8[this->field_0x38].field_0x14 = 0.0f;
		break;
	case WEAPON_STATE_FIRE:
		if (1.0f / this->field_0x8[this->field_0x38].field_0x0 <= this->field_0x8[this->field_0x38].field_0x14) {
			if (this->field_0x8[this->field_0x38].field_0x10 < this->field_0x8[this->field_0x38].field_0x4) {
				this->field_0x8[this->field_0x38].field_0x14 = 0.0f;
				this->pOwner->SetState(WEAPON_STATE_IDLE, -1);
			}
			else {
				if (this->field_0x8[this->field_0x38].field_0x8 == 0.0f) {
					this->field_0x8[this->field_0x38].field_0x14 = 0.0f;
					this->field_0x8[this->field_0x38].field_0x10 = 0;
					this->pOwner->SetState(WEAPON_STATE_IDLE, -1);
				}
				else {
					this->field_0x8[this->field_0x38].field_0x14 = 0.0f;
					this->pOwner->SetState(WEAPON_STATE_RELOAD, -1);
				}
			}
		}
		break;
	case WEAPON_STATE_RELOAD:
		if (this->field_0x8[this->field_0x38].field_0x8 <= this->field_0x8[this->field_0x38].field_0x14) {
			this->field_0x8[this->field_0x38].field_0x14 = 0.0f;
			this->field_0x8[this->field_0x38].field_0x10 = 0;
			this->pOwner->SetState(WEAPON_STATE_IDLE, -1);
		}
		break;
	case 0xb:
		this->field_0x8[this->field_0x38].field_0x14 = 0.0f;
		if (Func_0x74() != 0) {
			this->pOwner->SetState(WEAPON_STATE_IDLE, -1);
		}
		break;
	case 0xc:
		this->field_0x8[this->field_0x38].field_0x14 = 0.0f;
		if (Func_0x78() != 0) {
			this->pOwner->SetState(0xd, -1);
		}
	}
	return;
}

void CBehaviourWeaponBullets::Create(ByteCode* pByteCode)
{
	CBehaviourWeaponBurst::Create(pByteCode);

	this->field_0x40 = pByteCode->GetU32();
	this->field_0x44 = pByteCode->GetU32();
	this->field_0x3c.index = pByteCode->GetS32();

	return;
}

void CBehaviourWeaponBullets::Init(CActor* pOwner)
{
	Bullet* pBullet;
	int iVar3;

	this->pOwner = reinterpret_cast<CActorWeapon*>(pOwner);

	if (this->field_0x3c.index == -1) {
		this->field_0x50 = (CActorSound*)0x0;
	}
	else {
		this->field_0x3c.Init();
		this->field_0x50 = pOwner->CreateActorSound(1);
	}

	this->field_0x48 = -1;

	iVar3 = 0;
	pBullet = this->aBullets;
	do {
		pBullet->field_0x3c = 0;
		pBullet->field_0x40 = 0;
		ResetBullet(pBullet);
		iVar3 = iVar3 + 1;
		pBullet = pBullet + 1;
	} while (iVar3 < 8);

	this->nbBullets = 0;
	return;
}

void CBehaviourWeaponBullets::Term()
{
	for (int i = 0; i < 8; i++) {
		ResetBullet(&this->aBullets[i]);
	}

	return;
}

void CBehaviourWeaponBullets::Manage()
{
	CBehaviourWeapon::Manage();

	UpdateBurst();

	if (0 < this->nbBullets) {
		UpdateBulletsLife();
	}

	return;
}

void CBehaviourWeaponBullets::Draw()
{
	if (0 < this->nbBullets) {
		DrawBullets();
	}

	return;
}

void CBehaviourWeaponBullets::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	Bullet* pBullet;
	int iVar4;

	CBehaviourWeaponBurst::Begin(pOwner, newState, newAnimationType);

	this->field_0x48 = -1;
	pBullet = this->aBullets;
	do {
		pBullet->field_0x3c = 0;
		pBullet->field_0x40 = 0;
		ResetBullet(pBullet);
		iVar4 = iVar4 + 1;
		pBullet = pBullet + 1;
	} while (iVar4 < 8);

	this->nbBullets = 0;
	this->field_0x4c = 0.0f;

	return;
}

bool CBehaviourWeaponBullets::Action(edF32VECTOR4* pPosition, CActor* pActor)
{
	bool bVar1;
	Bullet* pBVar2;
	CActor* pLinkedActor;
	CActor* pCVar3;
	CBehaviourWeaponBurst* pCVar5;
	undefined4* puVar4;
	float fVar5;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 local_90;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	bool bSuccess;

	edF32Vector4AddHard(&local_10, &this->pOwner->currentLocation, &this->pOwner->rotationQuat);
	edF32Vector4SubHard(&local_20, pPosition, &local_10);
	edF32Vector4NormalizeHard(&local_20, &local_20);

	bSuccess = true;
	if (pActor != (CActor*)0x0) {
		pActor->SV_GetActorColCenter(&eStack48);
		edF32Vector4SubHard(&eStack64, &local_10, &eStack48);
		fVar5 = edF32Vector4NormalizeHard(&eStack64, &eStack64);
		CCollisionRay CStack96 = CCollisionRay(fVar5, &eStack48, &eStack64);
		fVar5 = CStack96.IntersectScenery((edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
		if (fVar5 != 1e+30f) {
			bSuccess = false;
		}
	}

	if (bSuccess) {
		pBVar2 = GetFreeBullet();
		local_90 = gF32Vector4Zero;
		pLinkedActor = this->pOwner->GetLinkFather();
		pCVar3 = pLinkedActor->GetCollidingActor();
		while (pLinkedActor != (CActor*)0x0) {
			if (pCVar3 == pLinkedActor) {
				pLinkedActor = (CActor*)0x0;
			}
			else {
				bVar1 = pLinkedActor->IsKindOfObject(2);
				if (bVar1 != false) {
					CActorMovable* pMovable = static_cast<CActorMovable*>(pLinkedActor);
					edF32Vector4ScaleHard(pMovable->dynamic.linearAcceleration, &eStack160, &pMovable->dynamic.velocityDirectionEuler);
					edF32Vector4AddHard(&local_90, &local_90, &eStack160);
				}

				pLinkedActor = pLinkedActor->pTiedActor;
			}
		}

		pCVar5 = static_cast<CBehaviourWeaponBurst*>(this->pOwner->GetBehaviour(this->pOwner->curBehaviourId));
		edF32Vector4ScaleHard(pCVar5->field_0x8[pCVar5->field_0x38].field_0xc, &eStack128, &local_20);

		edF32Vector4AddHard(&local_70, &local_90, &eStack128);
		fVar5 = edF32Vector4NormalizeHard(&local_70, &local_70);
		pBVar2->position = local_10;
		pBVar2->field_0x10 = local_20;
		pBVar2->direction = local_70;

		pBVar2->field_0x38 = fVar5;

		FireBullet(pBVar2);

		if (this->field_0x50 != (CActorSound*)0x0) {
			IMPLEMENTATION_GUARD_AUDIO(
			CActorSound::SoundStart
			(this->field_0x50, (CActor*)this->pOwner, 0, (CSound*)this->field_0x3c, 1, 0,
				(SOUND_SPATIALIZATION_PARAM*)0x0);)
		}

		this->field_0x8[this->field_0x38].field_0x10 = this->field_0x8[this->field_0x38].field_0x10 + 1;

		this->pOwner->SetState(WEAPON_STATE_FIRE, -1);
	}

	return bSuccess;
}

void CBehaviourWeaponBullets::ResetBullet(Bullet* pBullet)
{
	if (pBullet->field_0x30 != 0) {
		this->nbBullets = this->nbBullets + -1;
	}

	pBullet->field_0x30 = 0;
	pBullet->currentLifetime = 0;

	return;
}

void CBehaviourWeaponBullets::FireBullet(Bullet* pBullet)
{
	if (pBullet->field_0x30 == 0) {
		this->nbBullets = this->nbBullets + 1;
	}

	pBullet->field_0x30 = 1;
	pBullet->currentLifetime = 3.0f;

	return;
}

void CBehaviourWeaponBullets::HitBullet(float param_1, Bullet* pBullet)
{
	if (pBullet->field_0x30 == 0) {
		this->nbBullets = this->nbBullets + 1;
	}

	pBullet->field_0x30 = 2;
	pBullet->currentLifetime = param_1;

	return;
}

Bullet* CBehaviourWeaponBullets::GetFreeBullet()
{
	Bullet* pCVar1;
	int freeBulletIndex;
	float fVar3;
	float fVar4;

	fVar3 = 0.0f;
	freeBulletIndex = 0;

	for (pCVar1 = this->aBullets; (freeBulletIndex < 8 && (pCVar1->field_0x30 != 0)); pCVar1 = pCVar1 + 1) {
		freeBulletIndex = freeBulletIndex + 1;
	}

	if (7 < freeBulletIndex) {
		freeBulletIndex = 0;
		pCVar1 = this->aBullets;
		do {
			fVar4 = pCVar1->currentLifetime;
			if (fVar3 < fVar4) {
				this->field_0x48 = freeBulletIndex;
				fVar3 = fVar4;
			}

			freeBulletIndex = freeBulletIndex + 1;
			pCVar1 = pCVar1 + 1;
		} while (freeBulletIndex < 8);

		ResetBullet(this->aBullets + this->field_0x48);
		freeBulletIndex = this->field_0x48;
	}

	return this->aBullets + freeBulletIndex;
}


void CBehaviourWeaponBullets::UpdateBulletsLife()
{
	bool bVar1;
	CActor* pCVar2;
	bool bVar3;
	Timer* pTVar4;
	Bullet* pCVar5;
	int iVar6;
	float fVar7;
	float t;
	float fVar8;
	float fVar9;
	_msg_hit_param local_e0;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	_ray_info_out local_10;
	undefined4* local_4;

	fVar8 = GetTimer()->cutsceneDeltaTime;
	iVar6 = 0;
	pCVar5 = this->aBullets;
	do {
		if (pCVar5->field_0x30 != 0) {
			fVar7 = pCVar5->currentLifetime - fVar8;
			fVar9 = fVar8 * pCVar5->field_0x38;
			pCVar5->currentLifetime = fVar7;
			if (fVar7 <= 0.0f) {
				if (pCVar5->field_0x30 == 1) {
					HitBullet(0.5f, pCVar5);
				}

				if (pCVar5->field_0x30 == 2) {
					ResetBullet(pCVar5);
				}
			}

			if (pCVar5->field_0x30 == 1) {
				CCollisionRay CStack80 = CCollisionRay(fVar9, &pCVar5->position, &pCVar5->direction);
				fVar7 = CStack80.Intersect(1, this->pOwner, (CActor*)0x0, 0, &eStack48, (_ray_info_out*)0x0);
				t = CStack80.Intersect(2, this->pOwner, (CActor*)0x0, 0x40000400, (edF32VECTOR4*)0x0, &local_10);
				pCVar2 = local_10.pActor_0x0;
				if (t <= fVar7) {
					if (t != 1e+30f) {
						bVar1 = false;
						if ((local_10.pActor_0x0 != (CActor*)0x0) && ((bVar3 = local_10.pActor_0x0->IsKindOfObject(0x10), bVar3 == false || (pCVar2->curBehaviourId == 6)))) {
							bVar1 = true;
						}

						if (bVar1) {
							edF32Vector4ScaleHard(t, &local_e0.field_0x80, &pCVar5->direction);
							edF32Vector4AddHard(&local_e0.field_0x80, &pCVar5->position, &local_e0.field_0x80);
							local_e0.projectileType = 9;
							local_e0.damage = this->pOwner->field_0x1d8;
							local_e0.field_0x20 = pCVar5->field_0x10;
							local_e0.field_0x40 = local_e0.field_0x80;
							local_e0.field_0x30 = this->pOwner->field_0x1dc;
							this->pOwner->DoMessage(local_10.pActor_0x0, MESSAGE_KICKED, &local_e0);
							FUN_002d3f60(pCVar5, this->field_0x44);
							HitBullet(0.5f, pCVar5);
							fVar9 = t;
						}
					}
				}
				else {
					if (fVar7 != 1e+30f) {
						FUN_002d3f60(pCVar5, this->field_0x40);
						HitBullet(0.5f, pCVar5);
						fVar9 = fVar7;
					}
				}

				edF32Vector4ScaleHard(fVar9, &eStack32, &pCVar5->direction);
				edF32Vector4AddHard(&pCVar5->position, &pCVar5->position, &eStack32);
			}
		}

		static const float DBG_ANG_SPEED = 40.840706f;
		fVar9 = DBG_ANG_SPEED;
		iVar6 = iVar6 + 1;
		pCVar5 = pCVar5 + 1;
	} while (iVar6 < 8);

	this->field_0x4c = edF32Between_0_2Pi_Incr(this->field_0x4c + fVar9 * GetTimer()->cutsceneDeltaTime);;

	return;
}

void CBehaviourWeaponBullets::FUN_002d3f60(Bullet* pBullet, uint param_3)
{
	IMPLEMENTATION_GUARD_FX();
}

void CBehaviourWeaponPistol::Create(ByteCode* pByteCode)
{
	CBehaviourWeaponBullets::Create(pByteCode);

	this->field_0x2e0 = pByteCode->GetS32();
	this->field_0x2e4 = pByteCode->GetS32();

	if (this->field_0x2e4 != -1) {
		CActor::SV_InstallMaterialId(this->field_0x2e4);
	}

	return;
}

void CBehaviourWeaponPistol::Init(CActor* pOwner)
{
	CBehaviourWeaponBullets::Init(pOwner);
	CActor::SV_InstallMaterialId(this->field_0x2e0);

	return;
}

void CBehaviourWeaponPistol::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourWeaponBullets::Begin(pOwner, newState, newAnimationType);

	this->pOwner->field_0x1d0 = 2;

	return;
}

void CBehaviourWeaponPistol::DrawBullets()
{
	IMPLEMENTATION_GUARD_FX();
}

void CBehaviourWeaponRpg::Create(ByteCode* pByteCode)
{
	CBehaviourWeaponBurst::Create(pByteCode);

	S_ACTOR_STREAM_REF* pSVar1 = (S_ACTOR_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)pSVar1->aEntries;
	if (pSVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * 4;
	}
	this->field_0x3c = pSVar1;

	return;
}

void CBehaviourWeaponRpg::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorWeapon*>(pOwner);

	for (int i = 0; i < this->field_0x3c->entryCount; i++) {
		this->field_0x3c->aEntries[i].Init();
	}

	return;
}

void CBehaviourWeaponRpg::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourWeaponBurst::Begin(pOwner, newState, newAnimationType);

	this->pOwner->field_0x1d0 = 4;

	return;
}

WeaponGlobal WeaponGlobal_0048dd60;

void CBehaviourWeaponSniper::Create(ByteCode* pByteCode)
{
	CBehaviourWeaponBullets::Create(pByteCode);

	WeaponGlobal_0048dd60.field_0x0 = pByteCode->GetS32();
	WeaponGlobal_0048dd60.field_0x4 = pByteCode->GetS32();

	this->field_0x344 = pByteCode->GetS32();
	if (this->field_0x344 != -1) {
		CActor::SV_InstallMaterialId(this->field_0x344);
	}

	return;
}

int INT_00449718 = 0;

void CBehaviourWeaponSniper::Init(CActor* pOwner)
{
	CBehaviourWeaponBullets::Init(pOwner);

	if (INT_00449718 == 0) {
		IMPLEMENTATION_GUARD_SNIPER_BULLET(
		WeaponGlobal::Init(0.0, 0.0, &WeaponGlobal_0048dd60, 0x10, (ed_3D_Scene*)0x0);
		uVar2 = 0;
		puVar1 = &DAT_0048ddb0;
		do {
			*(undefined4*)(puVar1 + 0xc0) = 0;
			*(undefined4*)(puVar1 + 400) = 0;
			uVar2 = uVar2 + 8;
			*(undefined4*)(puVar1 + 0x260) = 0;
			*(undefined4*)(puVar1 + 0x330) = 0;
			*(undefined4*)(puVar1 + 0x400) = 0;
			*(undefined4*)(puVar1 + 0x4d0) = 0;
			*(undefined4*)(puVar1 + 0x5a0) = 0;
			*(undefined4*)(puVar1 + 0x670) = 0;
			puVar1 = puVar1 + 0x680;
		} while (uVar2 < 0x10);)
	}

	INT_00449718 = INT_00449718 + 1;

	return;

}

void CBehaviourWeaponSniper::Term()
{
	Bullet* pCVar1;
	int iVar2;

	iVar2 = 0;
	pCVar1 = this->aBullets;
	do {
		ResetBullet(pCVar1);
		iVar2 = iVar2 + 1;
		pCVar1 = pCVar1 + 1;
	} while (iVar2 < 8);

	INT_00449718 = INT_00449718 + -1;

	if (INT_00449718 == 0) {
		WeaponGlobal_0048dd60.Term();
	}
	return;
}

void CBehaviourWeaponSniper::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourWeaponBullets::Begin(pOwner, newState, newAnimationType);

	this->pOwner->field_0x1d0 = 5;

	return;
}

void CBehaviourWeaponSniper::DrawBullets()
{
	IMPLEMENTATION_GUARD();
}

void WeaponGlobal::Term()
{
	IMPLEMENTATION_GUARD_LOG();
	//WeaponGlobalSubObj* pWVar1;
	//
	//pWVar1 = this->field_0x18;
	//if (pWVar1 != (WeaponGlobalSubObj*)0x0) {
	//	if (pWVar1 != (WeaponGlobalSubObj*)0x0) {
	//		delete(&pWVar1[-1].field_0x30);
	//	}
	//	this->field_0x18 = (WeaponGlobalSubObj*)0x0;
	//}
	//if (this->pEntries != (WeaponGlobalSubObj**)0x0) {
	//	delete(this->pEntries);
	//	this->pEntries = (WeaponGlobalSubObj**)0x0;
	//}
	//this->field_0x10 = 0;
	//this->nbEntries = 0;
	return;
}
