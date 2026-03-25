#include "ActorWeapon.h"
#include "MemoryStream.h"
#include "ActorWolfen.h"
#include "FileManager3D.h"
#include "CollisionRay.h"
#include "MathOps.h"
#include "DlistManager.h"

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
		if (((pCVar3 != (CCollision*)0x0) && ((pCVar3->flags_0x4 & COLLISION_GROUND_FLAG) != 0)) || (fVar5 == 0.0f)) {
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
		pBullet->pInstance3d = (CInstance3D*)0x0;
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
		pBullet->pInstance3d = (CInstance3D*)0x0;
		pBullet->field_0x40 = 0;
		ResetBullet(pBullet);
		iVar4 = iVar4 + 1;
		pBullet = pBullet + 1;
	} while (iVar4 < 8);

	this->nbBullets = 0;
	this->angle = 0.0f;

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
	float impulseMagnitude;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 local_90;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 bulletImpulse;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 bulletPosition;
	bool bSuccess;

	edF32Vector4AddHard(&bulletPosition, &this->pOwner->currentLocation, &this->pOwner->rotationQuat);
	edF32Vector4SubHard(&local_20, pPosition, &bulletPosition);
	edF32Vector4NormalizeHard(&local_20, &local_20);

	bSuccess = true;
	if (pActor != (CActor*)0x0) {
		pActor->SV_GetActorColCenter(&eStack48);
		edF32Vector4SubHard(&eStack64, &bulletPosition, &eStack48);
		impulseMagnitude = edF32Vector4NormalizeHard(&eStack64, &eStack64);
		CCollisionRay CStack96 = CCollisionRay(impulseMagnitude, &eStack48, &eStack64);
		impulseMagnitude = CStack96.IntersectScenery((edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
		if (impulseMagnitude != 1e+30f) {
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

		edF32Vector4AddHard(&bulletImpulse, &local_90, &eStack128);
		impulseMagnitude = edF32Vector4NormalizeHard(&bulletImpulse, &bulletImpulse);
		pBVar2->position = bulletPosition;
		pBVar2->field_0x10 = local_20;
		pBVar2->direction = bulletImpulse;

		pBVar2->speed = impulseMagnitude;

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
	if (pBullet->bulletState != 0) {
		this->nbBullets = this->nbBullets + -1;
	}

	pBullet->bulletState = 0;
	pBullet->currentLifetime = 0;

	return;
}

void CBehaviourWeaponBullets::FireBullet(Bullet* pBullet)
{
	if (pBullet->bulletState == 0) {
		this->nbBullets = this->nbBullets + 1;
	}

	pBullet->bulletState = 1;
	pBullet->currentLifetime = 3.0f;

	return;
}

void CBehaviourWeaponBullets::HitBullet(float param_1, Bullet* pBullet)
{
	if (pBullet->bulletState == 0) {
		this->nbBullets = this->nbBullets + 1;
	}

	pBullet->bulletState = 2;
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

	for (pCVar1 = this->aBullets; (freeBulletIndex < 8 && (pCVar1->bulletState != 0)); pCVar1 = pCVar1 + 1) {
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
	CActor* pRayHitActor;
	bool bVar3;
	Timer* pTVar4;
	Bullet* pBullet;
	int index;
	float intersectionTimeScenery;
	float intersectionTimeActors;
	float deltaTime;
	float rayDist;
	_msg_hit_param msgHitParam;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	_ray_info_out rayInfoOut;
	undefined4* local_4;

	deltaTime = GetTimer()->cutsceneDeltaTime;
	index = 0;
	pBullet = this->aBullets;
	do {
		if (pBullet->bulletState != 0) {
			const float newLifeTime = pBullet->currentLifetime - deltaTime;
			rayDist = deltaTime * pBullet->speed;
			pBullet->currentLifetime = newLifeTime;
			if (newLifeTime <= 0.0f) {
				if (pBullet->bulletState == 1) {
					HitBullet(0.5f, pBullet);
				}

				if (pBullet->bulletState == 2) {
					ResetBullet(pBullet);
				}
			}

			if (pBullet->bulletState == 1) {
				CCollisionRay hitRay = CCollisionRay(rayDist, &pBullet->position, &pBullet->direction);
				intersectionTimeScenery = hitRay.Intersect(1, this->pOwner, (CActor*)0x0, 0, &eStack48, (_ray_info_out*)0x0);
				intersectionTimeActors = hitRay.Intersect(2, this->pOwner, (CActor*)0x0, 0x40000400, (edF32VECTOR4*)0x0, &rayInfoOut);

				pRayHitActor = rayInfoOut.pActor_0x0;
				if (intersectionTimeActors <= intersectionTimeScenery) {
					if (intersectionTimeActors != 1e+30f) {
						bVar1 = false;
						if ((rayInfoOut.pActor_0x0 != (CActor*)0x0) && ((bVar3 = rayInfoOut.pActor_0x0->IsKindOfObject(OBJ_TYPE_WOLFEN), bVar3 == false || (pRayHitActor->curBehaviourId == 6)))) {
							bVar1 = true;
						}

						if (bVar1) {
							edF32Vector4ScaleHard(intersectionTimeActors, &msgHitParam.field_0x80, &pBullet->direction);
							edF32Vector4AddHard(&msgHitParam.field_0x80, &pBullet->position, &msgHitParam.field_0x80);
							msgHitParam.projectileType = 9;
							msgHitParam.damage = this->pOwner->field_0x1d8;
							msgHitParam.field_0x20 = pBullet->field_0x10;
							msgHitParam.field_0x40 = msgHitParam.field_0x80;
							msgHitParam.field_0x30 = this->pOwner->field_0x1dc;
							this->pOwner->DoMessage(rayInfoOut.pActor_0x0, MESSAGE_KICKED, &msgHitParam);
							StartFx(pBullet, this->field_0x44);
							HitBullet(0.5f, pBullet);
							rayDist = intersectionTimeActors;
						}
					}
				}
				else {
					if (intersectionTimeScenery != 1e+30f) {
						StartFx(pBullet, this->field_0x40);
						HitBullet(0.5f, pBullet);
						rayDist = intersectionTimeScenery;
					}
				}

				edF32Vector4ScaleHard(rayDist, &eStack32, &pBullet->direction);
				edF32Vector4AddHard(&pBullet->position, &pBullet->position, &eStack32);
			}
		}

		index = index + 1;
		pBullet = pBullet + 1;
	} while (index < 8);

	static const float DBG_ANG_SPEED = 40.840706f;
	this->angle = edF32Between_0_2Pi_Incr(this->angle + DBG_ANG_SPEED * GetTimer()->cutsceneDeltaTime);

	return;
}

void CBehaviourWeaponBullets::StartFx(Bullet* pBullet, uint param_3)
{
	bool bVar1;
	float angle;
	float puVar2;
	float puVar3;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	CFxHandle fxHandle;

	fxHandle.id = 0;
	fxHandle.pFx = (CNewFx*)0x0;
	if (param_3 != 0xffffffff) {
		CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&fxHandle, param_3, FX_MATERIAL_SELECTOR_NONE);
		if (((fxHandle.pFx == (CNewFx*)0x0) || (fxHandle.id == 0)) ||
			(fxHandle.id != (fxHandle.pFx)->id)) {
			bVar1 = false;
		}
		else {
			bVar1 = true;
		}

		if (bVar1) {
			local_20.x = (pBullet->field_0x10).z;
			local_20.y = 0.0f;
			local_20.z = -(pBullet->field_0x10).x;
			local_20.w = 0.0f;

			puVar2 = pBullet->field_0x10.y;
			if (1.0f < puVar2) {
				puVar3 = 1.0f;
			}
			else {
				puVar3 = -1.0f;
				if (-1.0f <= puVar2) {
					puVar3 = puVar2;
				}
			}

			angle = acosf(puVar3);
			edF32Matrix4FromAngAxisSoft(angle, &eStack96, &local_20);
			edF32Matrix4ToEulerSoft(&eStack96, &local_20.xyz, "XYZ");
			fxHandle.SetPosition(&pBullet->position);
			fxHandle.SetRotationEuler(&local_20);
			fxHandle.Start();
		}

		pBullet->currentLifetime = 0.0f;
	}

	return;
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

void CSniperBullet::FUN_002d3be0(float param_1)
{
	// The overall shape is a spiral of 12 points in spherical coordinates, with z linearly interpolating from param_1
	// toward 1.0.

	const int rand0 = CScene::_pinstance->Rand(); // polar angle seed
	const int rand1 = CScene::_pinstance->Rand(); // azimuth angle seed
	const int rand2 = CScene::_pinstance->Rand(); // azimuth sign seed

	this->field_0xc4 = param_1;

	const float sign = (rand2 & 0x10000) ? -1.0f : 1.0f;
	const float polarAngle = (rand0 * (M_PI / 4.0f)) / 32767.0f + (3.0f * M_PI / 4.0f); // [3π/4, π]
	const float azimuthAngle = (rand1 * (2.0f * M_PI)) / 32767.0f + M_PI;                  // [π, 3π]

	for (int i = 0; i < 0xc; i++)
	{
		const float t = (float)i / 12.0f;
		const float phi = polarAngle * t;
		const float theta = azimuthAngle * t * sign;

		const float sinPhi = sinf(phi);
		this->field_0x0[i].x = sinPhi * cosf(theta);
		this->field_0x0[i].y = sinPhi * sinf(theta);
		this->field_0x0[i].z = param_1 + t * (1.0f - param_1);
		this->field_0x0[i].w = 1.0f;
	}
}


void CSniperBullet::Draw(float param_1, edF32VECTOR4* pPosition, edF32VECTOR4* param_4, edF32VECTOR4* param_5, int param_6, uint param_7)
{
	bool bVar1;
	edF32VECTOR4* v1;
	uint uVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR4* peVar4;
	edF32VECTOR4* v1_00;
	float fVar5;
	float fVar6;
	float puVar7;
	float puVar8;
	byte local_244;
	byte bStack579;
	byte bStack578;
	byte bStack577;
	edF32VECTOR4 local_240;
	edF32VECTOR4 local_230;
	edF32MATRIX4 eStack544;
	edF32MATRIX4 eStack480;
	edF32VECTOR4 aeStack416[13];
	edF32VECTOR4 local_d0[13];

	local_230.x = -param_4->y;
	local_230.y = param_4->x;
	local_230.z = 0.0f;
	local_230.w = 0.0f;
	edF32Vector4SafeNormalize1Hard(&local_230, &local_230);
	puVar7 = param_4->z;
	if (1.0f < puVar7) {
		puVar8 = 1.0f;
	}
	else {
		puVar8 = -1.0f;
		if (-1.0f <= puVar7) {
			puVar8 = puVar7;
		}
	}
	fVar5 = acosf(puVar8);
	edF32Matrix4FromAngAxisSoft(-fVar5, &eStack480, &local_230);
	edF32Matrix4TranslateHard(&eStack480, &eStack480, pPosition);
	edF32Matrix4ScaleHard(&eStack544, &gF32Matrix4Unit, param_5);
	edF32Matrix4MulF32Matrix4Hard(&eStack480, &eStack544, &eStack480);
	local_d0[0].x = pPosition->x;
	local_d0[0].y = pPosition->y;
	local_d0[0].z = pPosition->z;
	local_d0[0].w = pPosition->w;

	uVar2 = 0;

	v1 = this->field_0x0;
	do {
		edF32Matrix4MulF32Vector4Hard(local_d0 + uVar2 + 1, &eStack480, v1);
		uVar2 = uVar2 + 1;
		v1 = v1 + 1;
	} while (uVar2 < 0xc);

	edF32Matrix4CopyHard(&eStack480, &CCameraManager::_gThis->worldToCamera_0x3d0);

	v1_00 = local_d0;
	uVar2 = 0;
	peVar3 = aeStack416;
	peVar4 = v1_00;
	do {
		edF32Matrix4MulF32Vector4Hard(peVar3, &eStack480, peVar4);
		uVar2 = uVar2 + 1;
		peVar4 = peVar4 + 1;
		peVar3 = peVar3 + 1;
	} while (uVar2 < 0xd);

	eStack480.rowT = gF32Vertex4Zero;

	edF32Matrix4TransposeHard(&eStack480);

	bVar1 = GameDList_BeginCurrent();
	if (bVar1 != false) {
		fVar5 = this->field_0xc4;
		edDListLoadIdentity();
		edDListUseMaterial((edDList_material*)0x0);
		edDListBlendSet(1);
		edDListBlendFunc_002ca830();
		edDListBegin(0.0, 0.0, 0.0, 4, 0x1a);
		bStack577 = (byte)(param_7 >> 0x18);
		uVar2 = 0;
		bStack578 = (byte)(param_7 >> 0x10);
		peVar3 = aeStack416;
		bStack579 = (byte)(param_7 >> 8);
		peVar4 = local_d0;
		do {
			local_244 = (byte)param_7;
			if (uVar2 == 0) {
				edDListColor4u8(local_244, bStack579, bStack578, 0);
				edF32Vector4SubHard(&local_230, aeStack416 + 1, peVar3);
			}
			else {
				if (uVar2 == 0xc) {
					edDListColor4u8(local_244, bStack579, bStack578, 0);
					edF32Vector4SubHard(&local_230, peVar3, aeStack416 + 0xb);
				}
				else {
					edDListColor4u8(local_244, bStack579, bStack578, bStack577);
					edF32Vector4SubHard(&local_240, peVar3, aeStack416 + (uVar2 - 1));
					edF32Vector4SubHard(&local_230, aeStack416 + uVar2 + 1, peVar3);
					edF32Vector4AddHard(&local_230, &local_230, &local_240);
				}
			}

			local_240.x = local_230.y;
			local_240.y = -local_230.x;
			local_240.z = 0.0f;
			local_240.w = 0.0f;

			fVar6 = edF32Vector4NormalizeHard(&local_230, &local_240);
			if (fVar6 == 0.0f) {
				local_230 = gF32Vector4UnitX;
			}

			edF32Vector4NormalizeHard(&local_240, &local_230);
			edF32Vector4ScaleHard(param_1, &local_230, &local_230);
			edF32Matrix4MulF32Vector4Hard(&local_230, &eStack480, &local_230);
			local_240 = *peVar4;
			edDListTexCoo2f(fVar5, 0.0f);
			edDListVertex4f(local_240.x, local_240.y, local_240.z, 0.0f);
			edF32Vector4AddHard(&local_240, v1_00, &local_230);
			edDListTexCoo2f(fVar5, 1.0f);
			edDListVertex4f(local_240.x, local_240.y, local_240.z, 0.0f);
			uVar2 = uVar2 + 1;
			peVar3 = peVar3 + 1;
			peVar4 = peVar4 + 1;
			v1_00 = v1_00 + 1;
			fVar5 = fVar5 + (1.0f - this->field_0xc4) / 12.0f;
		} while (uVar2 < 0xd);

		edDListEnd();
		GameDList_EndCurrent();
	}

	return;
}

void CBehaviourWeaponRpg::Create(ByteCode* pByteCode)
{
	CBehaviourWeaponBurst::Create(pByteCode);

	this->field_0x3c = S_ACTOR_STREAM_REF::Create(pByteCode);

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
CSniperBullet gSniperBullets[0x10];

void CBehaviourWeaponSniper::Create(ByteCode* pByteCode)
{
	CBehaviourWeaponBullets::Create(pByteCode);

	WeaponGlobal_0048dd60.g3dTextureIndex = pByteCode->GetS32();
	WeaponGlobal_0048dd60.g3dMeshIndex = pByteCode->GetS32();

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
		WeaponGlobal_0048dd60.Init(0.0f, 0.0f, 0x10, (ed_3D_Scene*)0x0);
		uint uVar2 = 0;
		do {
			gSniperBullets[uVar2].bInUse = 0;
			uVar2 = uVar2 + 1;
		} while (uVar2 < 0x10);
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

void CBehaviourWeaponSniper::ResetBullet(Bullet* pBullet)
{
	CInstance3D* pInstance;

	pInstance = pBullet->pInstance3d;
	if (pInstance != (CInstance3D*)0x0) {
		pInstance->SetDraw(false, WeaponGlobal_0048dd60.field_0x8);
		WeaponGlobal_0048dd60.Remove(pInstance);
		pBullet->pInstance3d = (CInstance3D*)0x0;
	}

	if (pBullet->bulletState != 0) {
		this->nbBullets = this->nbBullets + -1;
	}

	pBullet->bulletState = 0;
	pBullet->currentLifetime = 0.0;
	if (pBullet->field_0x40 != (CSniperBullet*)0x0) {
		pBullet->field_0x40->bInUse = 0;
		pBullet->field_0x40 = (CSniperBullet*)0x0;
	}

	return;
}

void CBehaviourWeaponSniper::FireBullet(Bullet * pBullet)
{
	CInstance3D* pInstance;
	CSniperBullet* pSniperBullet;
	uint index;
	edF32VECTOR3 newRotation;

	if (pBullet->bulletState == 0) {
		this->nbBullets = this->nbBullets + 1;
	}

	pBullet->bulletState = 1;
	pBullet->currentLifetime = 3.0f;
	pInstance = WeaponGlobal_0048dd60.Add();
	pInstance->position = pBullet->position;
	GetAnglesFromVector(&newRotation, &pBullet->field_0x10);
	(pInstance->rotation).xyz = newRotation;

	pInstance->SetDraw(true, WeaponGlobal_0048dd60.field_0x8);

	pBullet->pInstance3d = pInstance;
	if (pBullet->field_0x40 == (CSniperBullet*)0x0) {
		index = 0;
		pSniperBullet = gSniperBullets;
		while ((index < 0x10 && (pBullet->field_0x40 == (CSniperBullet*)0x0))) {
			if (pSniperBullet->bInUse == 0) {
				pBullet->field_0x40 = pSniperBullet;
				pBullet->field_0x40->bInUse = 1;
				pBullet->field_0x40->FUN_002d3be0(0.15f);
			}

			pSniperBullet = pSniperBullet + 1;
			index = index + 1;
		}
	}
	else {
		pBullet->field_0x40->FUN_002d3be0(0.15f);
	}

	return;
}

void CBehaviourWeaponSniper::HitBullet(float param_1, Bullet* pBullet)
{
	CInstance3D* pInstance;

	pInstance = pBullet->pInstance3d;
	if (pInstance != (CInstance3D*)0x0) {
		pInstance->SetDraw(false, WeaponGlobal_0048dd60.field_0x8);
		WeaponGlobal_0048dd60.Remove(pInstance);
		pBullet->pInstance3d = (CInstance3D*)0x0;
	}

	if (pBullet->bulletState == 0) {
		this->nbBullets = this->nbBullets + 1;
	}

	pBullet->bulletState = 2;
	pBullet->currentLifetime = 1.5f;

	return;
}

uint UINT_00448ac4 = 0x80808080;

void CBehaviourWeaponSniper::DrawBullets()
{
	int iVar1;
	Bullet* pBullet;
	int iVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 eStack144;
	edF32MATRIX4 eStack128;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	uint local_4;
	CInstance3D* pInstance;

	iVar2 = 0;
	local_4 = UINT_00448ac4;
	pBullet = this->aBullets;
	do {
		iVar1 = pBullet->bulletState;
		if (iVar1 == 1) {
			pInstance = pBullet->pInstance3d;
			pInstance->position = pBullet->position;
			pInstance->ComputeObjectMatrix();
			if (pBullet->field_0x40 != (CSniperBullet*)0x0) {
				edF32Vector4GetNegHard(&eStack48, &pBullet->field_0x10);
				local_40.x = 0.0f;
				local_40.y = 0.0f;
				local_40.z = 3.0f;
				local_40.w = 1.0f;
				pBullet->field_0x40->Draw(0.035f, &pBullet->position, &eStack48, &local_40, this->field_0x344, local_4);
			}
		}
		else {
			if ((iVar1 == 2) && (pBullet->field_0x40 != (CSniperBullet*)0x0)) {
				fVar5 = 1.0f - pBullet->currentLifetime / 1.5f;
				fVar3 = fVar5 * 0.05f;

				local_40.x = fVar5 * 0.5f;
				local_40.z = local_40.x + 3.0f;
				local_40.w = 1.0f;
				local_40.y = local_40.x;

				edF32Vector4GetNegHard(&eStack48, &pBullet->field_0x10);
				edF32Vector4CrossProductHard(&eStack144, &gF32Vector4UnitY, &eStack48);
				fVar4 = edF32Vector4NormalizeHard(&eStack144, &eStack144);
				if (fVar4 != 0.0f) {
					edF32Matrix4FromAngAxisSoft(fVar5 * 0.3141593f, &eStack128, &eStack144);
					edF32Matrix4MulF32Vector4Hard(&eStack48, &eStack128, &eStack48);
				}

				edF32Vector4ScaleHard(fVar5 * 0.15f, &eStack144, &gF32Vector4UnitY);
				edF32Vector4AddHard(&eStack32, &pBullet->position, &eStack144);
				fVar4 = (1.0f - fVar5) * (float)(local_4 >> 0x18);
				if (fVar4 < 2.147484e+09f) {
					local_4 = local_4 & 0xffffff | (int)fVar4 << 0x18;
				}
				else {
					local_4 = local_4 & 0xffffff | (int)(fVar4 - 2.147484e+09f) << 0x18;
				}

				pBullet->field_0x40->Draw(fVar3 + 0.035f, &eStack32, &eStack48, &local_40, this->field_0x344, local_4);
			}
		}

		iVar2 = iVar2 + 1;
		pBullet = pBullet + 1;
	} while (iVar2 < 8);

	return;
}

WeaponGlobal::WeaponGlobal()
{
	this->field_0x8 = (ed_3D_Scene*)0x0;
	this->g3dMeshIndex = -1;
	this->g3dTextureIndex = -1;
	this->field_0x10 = 0;
	this->nbEntries = 0;
	this->field_0x18 = (CInstance3D*)0x0;
	this->field_0x1c = 75.0f;
	(this->boundingSphere).x = 0.0f;
	(this->boundingSphere).y = 0.0f;
	(this->boundingSphere).z = 0.0f;
	(this->boundingSphere).w = 2.0f;
	memset(&this->hierarchySetup, 0, sizeof(this->hierarchySetup));

	return;
}

void WeaponGlobal::Init(float param_1, float param_2, uint param_4, ed_3D_Scene* pScene)
{
	int* pBase;
	CInstance3D* pCVar1;
	CInstance3D** ppCVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	int iVar6;

	if (pScene == (ed_3D_Scene*)0x0) {
		this->field_0x8 = CScene::_scene_handleA;
	}
	else {
		this->field_0x8 = pScene;
	}

	if (param_1 != 0.0f) {
		this->boundingSphere.w = param_1;
	}

	this->hierarchySetup.pBoundingSphere = &this->boundingSphere;
	if (param_2 != 0.0f) {
		this->field_0x1c = param_2;
	}
	this->hierarchySetup.clipping_0x0 = &this->field_0x1c;

	this->nbEntries = param_4;
	this->field_0x10 = param_4;

	this->field_0x18 = new CInstance3D[param_4];
	this->pEntries = new CInstance3D*[param_4];

	for (uint i = 0; i < param_4; i++) {
		pEntries[i] = &field_0x18[i];
	}

	return;
}

CInstance3D* WeaponGlobal::Add()
{
	float fVar1;
	float fVar2;
	float fVar3;
	ed_g3d_manager* pG3D;
	edNODE* peVar4;
	CInstance3D* pInstance;

	pInstance = (CInstance3D*)0x0;
	if (0 < (int)this->nbEntries) {
		this->nbEntries = this->nbEntries - 1;
		pInstance = this->pEntries[this->nbEntries];
		pInstance->rotation = gF32Vector4Zero;
		pInstance->position = gF32Vector4Zero;
		(pInstance->scale).z = 1.0f;
		(pInstance->scale).y = 1.0f;
		(pInstance->scale).x = 1.0f;
		(pInstance->scale).w = 0.0f;

		pG3D = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(this->g3dMeshIndex, this->g3dTextureIndex);
		peVar4 = ed3DHierarchyAddToScene(this->field_0x8, pG3D, (char*)0x0);
		pInstance->pNode = peVar4;
		ed3DHierarchySetSetup(static_cast<ed_3d_hierarchy*>(pInstance->pNode->pData), &this->hierarchySetup);
	}

	return pInstance;
}

void WeaponGlobal::Remove(CInstance3D* pInstance)
{
	if (pInstance->pNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(this->field_0x8, pInstance->pNode);
	}

	this->pEntries[this->nbEntries] = pInstance;
	this->nbEntries = this->nbEntries + 1;

	return;
}

void WeaponGlobal::Term()
{
	CInstance3D* pWVar1;
	
	pWVar1 = this->field_0x18;
	if (pWVar1 != (CInstance3D*)0x0) {
		if (pWVar1 != (CInstance3D*)0x0) {
			delete(this->field_0x18);
		}
		this->field_0x18 = (CInstance3D*)0x0;
	}

	if (this->pEntries != (CInstance3D**)0x0) {
		delete(this->pEntries);
		this->pEntries = (CInstance3D**)0x0;
	}

	this->field_0x10 = 0;
	this->nbEntries = 0;

	return;
}
