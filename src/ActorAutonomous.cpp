#include "ActorAutonomous.h"

#include <string.h>
#include <math.h>

#include "MemoryStream.h"
#include "TimeController.h"
#include "MathOps.h"
#include "CollisionManager.h"
#include "ActorManager.h"
#include "Vision.h"
#include "ActorFighter.h"

int INT_00448e08 = 0;

#define AUTONOMOUS_LOG(level, format, ...) MY_LOG_CATEGORY("Autonomous", level, format, ##__VA_ARGS__)

void CActorWindState::AddWind(_wind_param_in* pParams)
{
	if (0.0f <= pParams->field_0x0) {
		this->field_0xc = (pParams->field_0x0 + (float)this->field_0x0 * this->field_0xc) / (float)(this->field_0x0 + 1);
	}

	if (0.0f <= pParams->field_0x4) {
		this->field_0x10 = (pParams->field_0x4 + (float)this->field_0x0 * this->field_0x10) / (float)(this->field_0x0 + 1);
	}

	if (0.0f <= pParams->field_0x10) {
		this->field_0x14 = (pParams->field_0x10 + (float)this->field_0x0 * this->field_0x14) / (float)(this->field_0x0 + 1);
	}

	if (0.0f <= pParams->field_0x14) {
		this->field_0x18 = (pParams->field_0x14 + (float)this->field_0x0 * this->field_0x18) / (float)(this->field_0x0 + 1);
	}

	if (pParams->field_0x8 != 0) {
		this->field_0x4 = this->field_0x4 + 1;
	}

	if (pParams->field_0xc != 0) {
		this->field_0x8 = this->field_0x8 + 1;
	}

	this->field_0x1c = (pParams->field_0x18 + (float)this->field_0x0 * this->field_0x1c) / (float)(this->field_0x0 + 1);
	this->field_0x20 = (pParams->field_0x1c + (float)this->field_0x0 * this->field_0x20) / (float)(this->field_0x0 + 1);
	this->field_0x24 = (pParams->field_0x20 + (float)this->field_0x0 * this->field_0x24) / (float)(this->field_0x0 + 1);

	if (pParams->field_0x24 != 0) {
		this->field_0x28 = this->field_0x28 + 1;
	}

	if (pParams->pWayPoint != (CWayPoint*)0x0) {
		this->pWayPoint = pParams->pWayPoint;
		this->field_0x30 = pParams->field_0x2c;
		this->field_0x34 = pParams->field_0x30;
	}

	this->field_0x0 = this->field_0x0 + 1;

	return;
}

void CActorWindState::SubWind(_wind_param_in* pParams)
{
	int iVar1;

	iVar1 = this->field_0x0;
	if (iVar1 < 2) {
		if (iVar1 == 1) {
			Reset();
		}
	}
	else {
		if (0.0f <= pParams->field_0x0) {
			this->field_0xc = ((float)iVar1 * this->field_0xc - pParams->field_0x0) / (float)(iVar1 + -1);
		}

		if (0.0f <= pParams->field_0x4) {
			this->field_0x10 = ((float)this->field_0x0 * this->field_0x10 - pParams->field_0x4) / (float)(this->field_0x0 + -1);
		}

		if (pParams->field_0x8 != 0) {
			this->field_0x4 = this->field_0x4 + -1;
		}

		if (pParams->field_0xc != 0) {
			this->field_0x8 = this->field_0x8 + -1;
		}

		if (0.0f <= pParams->field_0x10) {
			this->field_0x14 = ((float)this->field_0x0 * this->field_0x14 - pParams->field_0x10) / (float)(this->field_0x0 + -1);
		}

		if (0.0f <= pParams->field_0x14) {
			this->field_0x18 = ((float)this->field_0x0 * this->field_0x18 - pParams->field_0x14) / (float)(this->field_0x0 + -1);
		}

		this->field_0x1c = ((float)this->field_0x0 * this->field_0x1c - pParams->field_0x18) / (float)(this->field_0x0 + -1);
		this->field_0x20 = ((float)this->field_0x0 * this->field_0x20 - pParams->field_0x1c) / (float)(this->field_0x0 + -1);
		this->field_0x24 = ((float)this->field_0x0 * this->field_0x24 - pParams->field_0x20) / (float)(this->field_0x0 + -1);

		if (pParams->field_0x24 != 0) {
			this->field_0x28 = this->field_0x28 + -1;
		}

		this->pWayPoint = (CWayPoint*)0x0;
		this->field_0x30 = 0.0f;
		this->field_0x34 = 0.0f;
		this->field_0x0 = this->field_0x0 + -1;
	}

	return;
}

void CActorWindState::Reset()
{
	this->field_0x0 = 0;
	this->field_0x4 = 0;
	this->field_0x8 = 0;
	this->field_0xc = 0.0f;
	this->field_0x10 = 4.0f;
	this->field_0x14 = 0.0f;
	this->field_0x18 = 0.0f;
	this->field_0x1c = 0.0f;
	this->field_0x20 = 1.0f;
	this->field_0x24 = 1.0f;
	this->field_0x28 = 0;
	this->pWayPoint = (CWayPoint*)0x0;
	this->field_0x30 = 0.0f;
	this->field_0x34 = 0.0f;
	this->field_0x40.x = 0.0f;
	this->field_0x40.y = 0.0f;
	this->field_0x40.z = 0.0f;
	this->field_0x40.w = 0.0f;
	this->field_0x38 = 0.0f;
	return;
}

CDynamicExt::CDynamicExt()
{
	this->gravityScale = 1.0f;
	this->field_0x4 = 0.0f;
	this->field_0x8 = 0.0f;
}

void CDynamicExt::ClearLocalData()
{
	Timer* pTVar1;

	(this->aImpulseVelocities[0]).x = 0.0f;
	(this->aImpulseVelocities[0]).y = 0.0f;
	(this->aImpulseVelocities[0]).z = 0.0f;
	(this->aImpulseVelocities[0]).w = 0.0f;
	this->aImpulseVelocityMagnitudes[0] = 0.0f;
	(this->aImpulseVelocities[1]).x = 0.0f;
	(this->aImpulseVelocities[1]).y = 0.0f;
	(this->aImpulseVelocities[1]).z = 0.0f;
	(this->aImpulseVelocities[1]).w = 0.0f;
	this->aImpulseVelocityMagnitudes[1] = 0.0f;
	(this->aImpulseVelocities[2]).x = 0.0f;
	(this->aImpulseVelocities[2]).y = 0.0f;
	(this->aImpulseVelocities[2]).z = 0.0f;
	(this->aImpulseVelocities[2]).w = 0.0f;
	this->aImpulseVelocityMagnitudes[2] = 0.0f;
	(this->normalizedTranslation).x = 0.0f;
	(this->normalizedTranslation).y = 0.0f;
	(this->normalizedTranslation).z = 0.0f;
	(this->normalizedTranslation).w = 0.0f;
	(this->field_0x60).x = 0.0f;
	(this->field_0x60).y = 0.0f;
	(this->field_0x60).z = 0.0f;
	this->field_0x6c = 0.0f;
	this->field_0xc = 1.7f;
	this->field_0x1c = 0.7f;
	pTVar1 = GetTimer();
	this->scaledTotalTime = pTVar1->scaledTotalTime;
	return;
}

edF32VECTOR4 CDynamicExt::gForceGravity = { 0.0f, -30.0f, 0.0f, 0.0f };

CActorAutonomous::CActorAutonomous()
{
	this->dynamicExt.gravityScale = 1.0f;
	this->dynamicExt.field_0x4 = 0.0f;
	this->dynamicExt.field_0x8 = 0.0f;
	this->lifeInterface = CLifeInterface();
}

bool CActorAutonomous::IsKindOfObject(ulong kind)
{
	return (kind & 7) != 0;
}

void CActorAutonomous::Create(ByteCode* pByteCode)
{
	float fVar1;
	float fVar2;
	edF32MATRIX4* peVar3;
	uint uVar4;
	long lVar5;
	ByteCode* pBVar6;
	float fVar7;

	INT_00448e08 = INT_00448e08 + 1;
	pBVar6 = pByteCode;
	CActorMovable::Create(pByteCode);
	this->dynamicExt.ClearLocalData();
	CActorWindState* pCVar3 = GetWindState();
	if (pCVar3 != (CActorWindState*)0x0) {
		pCVar3 = GetWindState();
		pCVar3->Reset();
	}
	this->bCollisionSphereDirty = 0;
	this->field_0x2c8 = 0.0f;
	if (this->pCollisionData != (CCollision*)0x0) {
		StoreCollisionSphere();
	}
	uVar4 = pByteCode->GetU32();
	this->lifeInterface.SetValueMax((float)uVar4);
	fVar7 = pByteCode->GetF32();
	(this->dynamicExt).gravityScale = fVar7;
	fVar7 = pByteCode->GetF32();
	(this->dynamicExt).field_0x4 = fVar7;
	fVar7 = pByteCode->GetF32();
	(this->dynamicExt).field_0x8 = fVar7;
	//fVar2 = gF32Vector4Zero.w;
	//fVar1 = gF32Vector4Zero.z;
	//fVar7 = gF32Vector4Zero.y;
	//*(float*)&this->field_0x2f0 = gF32Vector4Zero.x;
	//*(float*)&this->field_0x2f4 = fVar7;
	//*(float*)&this->field_0x2f8 = fVar1;
	//this->field_0x2fc = fVar2;
	//(this->lookAtLocation).z = 3.141593;
	//this->field_0x318 = -1.047198;
	//this->field_0x31c = 1.047198;
	//this->field_0x314 = 0.0f;
	//this->field_0x324 = -1.047198;
	//this->field_0x328 = 1.047198;
	//this->field_0x320 = 0.0f;
	//this->lookingAtBoneLeft = 0x45544554;
	//this->lookingAtBoneRight = 0x45544554;
	return;
}

void CActorAutonomous::Reset()
{
	CActorMovable::Reset();

	this->dynamicExt.ClearLocalData();

	if (GetWindState() != (CActorWindState*)0x0) {
		GetWindState()->Reset();
	}

	memset(&this->collisionContact, 0, sizeof(s_collision_contact));

	this->field_0x344 = this->field_0x344 & 0xfe;

	if (this->pCollisionData != (CCollision*)0x0) {
		RestoreCollisionSphere(0.0f);
	}

	LifeRestore();

	this->field_0x2e4 = 0.0f;

	if (GetVision() != (CVision*)0x0) {
		GetVision()->Reset();
	}

	return;
}

void CActorAutonomous::Init()
{
	int iVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	if (GetVision() != (CVision*)0x0) {
		GetVision()->location = this->currentLocation;
		GetVision()->rotationQuat = this->rotationQuat;
	}

	GetLifeInterfaceOther()->SetPriority(0);
	LifeRestore();
	this->field_0x2e4 = 0.0f;

	memset(&this->collisionContact, 0, sizeof(s_collision_contact));
	this->field_0x344 = this->field_0x344 & 0xfe;
	CActor::Init();
	return;
}

void CActorAutonomous::Term()
{
	SV_AUT_PathfindingEnd();
	CActor::Term();

	return;
}

void CActorAutonomous::CheckpointReset()
{
	CActorMovable::CheckpointReset();

	this->field_0x344 = this->field_0x344 & 0xfe;

	RestoreCollisionSphere(0);
	
	return;
}

CBehaviour* CActorAutonomous::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == 2) {
		pNewBehaviour = new CBehaviourCatchByTrap;
	}
	else {
		pNewBehaviour = CActor::BuildBehaviour(behaviourType);
	}
	return pNewBehaviour;
}

bool CActorAutonomous::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	bool bVar1;
	Timer* pTVar2;

	pTVar2 = GetTimer();
	(this->dynamicExt).scaledTotalTime = pTVar2->scaledTotalTime;
	bVar1 = CActorMovable::CarriedByActor(pActor, m0);
	return bVar1;
}

int CActorAutonomous::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	int* piVar2;
	edF32MATRIX4* peVar3;
	CBehaviour* pCVar4;
	int iVar5;
	long lVar6;

	if (msg == 0x3f) {
		bVar1 = SetBehaviour(2, -1, -1);
		if (bVar1 == false) {
			iVar5 = 0;
		}
		else {
			pCVar4 = GetBehaviour(this->curBehaviourId);

			CBehaviourCatchByTrap* pCatchByTrap = reinterpret_cast<CBehaviourCatchByTrap*>(pCVar4);
			pCatchByTrap->field_0x10 = pSender;
			iVar5 = 1;
		}
	}
	else {
		if (msg == MESSAGE_LEAVE_WIND) {
			if (GetWindState() != (CActorWindState*)0x0) {
				GetWindState()->SubWind((_wind_param_in*)pMsgParam);
			}

			iVar5 = 1;
		}
		else {
			if (msg == MESSAGE_ENTER_WIND) {
				if (GetWindState() != (CActorWindState*)0x0) {
					GetWindState()->AddWind((_wind_param_in*)pMsgParam);
				}

				iVar5 = 1;
			}
			else {
				iVar5 = CActorMovable::InterpretMessage(pSender, msg, pMsgParam);
			}
		}
	}

	return iVar5;
}

StateConfig CActorAutonomous::gStateCfg_AUT[1] = {
	StateConfig(0x8, 0x410),
};

StateConfig* CActorAutonomous::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActorMovable::GetStateCfg(state);
	}
	else {
		assert(state == 5);
		pStateConfig = gStateCfg_AUT + state + -5;
	}

	return pStateConfig;
}

void CActorAutonomous::ChangeManageState(int state)
{
	int iVar1;
	CPathFinderClient* lVar2;

	CActorMovable::ChangeManageState(state);

	if ((state == 0) && (lVar2 = GetPathfinderClient(), lVar2 != 0)) {
		GetPathfinderClient()->CleanPathDynamic();
	}

	return;
}

bool CActorAutonomous::IsLockable()
{
	bool bVar1;
	bool bLockable;

	bLockable = false;
	bVar1 = CActor::IsLockable();

	if (bVar1 != false) {
		if (0.0f < GetLifeInterface()->GetValue()) {
			bLockable = true;
		}
	}

	return bLockable;
}

void CActorAutonomous::SetLookingAtOn(float param_1)
{
	IMPLEMENTATION_GUARD_LOG();
}

void CActorAutonomous::SetLookingAtOn()
{
	this->SetLookingAtOn(0.0f);
}

void CActorAutonomous::SetLookingAtRotationHeight(float height, edF32VECTOR4* pRotation)
{
	IMPLEMENTATION_GUARD_LOOK_AT();
}

void CActorAutonomous::SetLookingAt(float x, float y, float z)
{
	IMPLEMENTATION_GUARD_LOOK_AT();
}

void CActorAutonomous::SetLookingAtBounds(float param_1, float param_2, float param_3, float param_4)
{
	IMPLEMENTATION_GUARD_LOOK_AT();
}

void CActorAutonomous::SetLookingAtBones(uint leftBoneId, uint rightBoneId)
{
	IMPLEMENTATION_GUARD_LOOK_AT();
}

void CActorAutonomous::_ManageDynamicFence(CActorsTable* pActorsTable)
{
	CCollision* pCVar1;
	edColPRIM_OBJECT* peVar2;
	long lVar3;
	CActorsTable* pCVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	pCVar1 = this->pCollisionData;
	if (pActorsTable->nbEntries < 1) {
		this->field_0x348 = 0;
	}
	else {
		iVar5 = 0;
		local_10 = gF32Vector4Zero;

		this->field_0x348 = 0;
		this->field_0x34c = (CActor*)0x0;
		peVar2 = pCVar1->pObbPrim;
		fVar7 = (peVar2->field_0xb0).y;
		fVar6 = (peVar2->field_0x90).y * 0.5f;
		fVar8 = fVar7 - fVar6;
		if (0 < pActorsTable->nbEntries) {
			do {
				if (((pActorsTable->aEntries[iVar5] != this->pTiedActor) &&
					(edF32Vector4SubHard(&eStack32, &(pActorsTable->aEntries[iVar5]->pCollisionData)->field_0x90, &this->currentLocation), fVar8 < eStack32.y)) &&
					(eStack32.y < fVar7 + fVar6)) {
					eStack32.y = 0.0f;
					edF32Vector4SafeNormalize0Hard(&eStack32, &eStack32);
					edF32Vector4AddHard(&local_10, &local_10, &eStack32);
					if (this->field_0x34c == (CActor*)0x0) {
						this->field_0x34c = pActorsTable->aEntries[iVar5];
					}
					this->field_0x348 = this->field_0x348 + 1;
					lVar3 = pActorsTable->aEntries[iVar5]->IsKindOfObject(OBJ_TYPE_AUTONOMOUS);
					if (lVar3 != 0) {
						CActorAutonomous* pAutonomous = reinterpret_cast<CActorAutonomous*>(pActorsTable->aEntries[iVar5]);
						pAutonomous->field_0x344 = pAutonomous->field_0x344 & 0xfe;
					}
				}
				iVar5 = iVar5 + 1;
		
			} while (iVar5 < pActorsTable->nbEntries);
		}

		if (0 < this->field_0x348) {
			edF32Vector4SafeNormalize0Hard(&local_30, &local_10);
			this->field_0x2e8 = local_30.x;
			this->vector_0x2f0 = local_30.z;
		}
	}
	return;
}

void CActorAutonomous::ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable)
{
	byte bVar1;
	uint uVar2;
	bool bVar3;
	bool bVar4;
	uint uVar7;
	int iVar8;
	edF32VECTOR4* peVar9;
	uint uVar10;
	long lVar11;
	float fVar12;
	float fVar13;
	//CActorFighterVTable** ppCVar14;
	float fVar15;
	float unaff_f20;
	undefined4 uVar16;
	edF32VECTOR4 local_230;
	edF32VECTOR4 aeStack544[3];
	edF32VECTOR4 eStack496;
	CActorsTable local_1e0;
	edF32VECTOR4 newLocation;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 horizontalTranslation;
	edF32VECTOR4 summedImpulseVelocities;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 movementSurfaceNormal;
	edF32VECTOR4 local_50;
	edF32VECTOR4 oldCurrentLocation;
	edF32VECTOR4 local_30;
	edF32VECTOR4 velocity;
	edF32VECTOR4 translation;
	CCollision* pCollision;

	AUTONOMOUS_LOG(LogLevel::Verbose, "CActorAutonomous::ManageDyn flags: 0x{:x}", this->pCollisionData->flags_0x4);

	pCollision = this->pCollisionData;
	bVar3 = false;
	uVar2 = this->dynamic.flags;

	oldCurrentLocation = this->currentLocation;

	this->dynamic.flags = 0;
	uVar10 = flags | this->dynamic.field_0x4c;

	movementSurfaceNormal = (pCollision->aCollisionContact[1]).location;
	local_50 = (pCollision->aCollisionContact[0]).location;
	local_70 = (pCollision->aCollisionContact[2]).location;

	bVar4 = false;
	if (((pCollision->flags_0x4 & 1) != 0) && ((uVar10 & 0x10) != 0)) {
		local_50.y = 0.0f;
		edF32Vector4SafeNormalize1Hard(&local_50, &local_50);
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "CActorAutonomous::ManageDyn other flags: 0x{:x}", uVar10);

	// Gravity
	if ((((uVar10 & 0x20) != 0) && ((pCollision->flags_0x4 & 2) == 0)) || ((uVar10 & 0x8000000) != 0)) {
		edF32Vector4ScaleHard(this->dynamicExt.gravityScale, &eStack128, &CDynamicExt::gForceGravity);
		peVar9 = this->dynamicExt.aImpulseVelocities;
		edF32Vector4AddHard(peVar9, peVar9, &eStack128);
		this->dynamicExt.aImpulseVelocityMagnitudes[0] = edF32Vector4GetDistHard(&this->dynamicExt.aImpulseVelocities[0]);
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Gravity: {}", this->dynamicExt.aImpulseVelocities[0].ToString());

	if (((uVar10 & 0x100) != 0) && ((pCollision->flags_0x4 & 2) == 0)) {
		if ((uVar10 & 0x100000) == 0) {
			fVar15 = this->dynamic.linearAcceleration;
			fVar15 = GetTimer()->cutsceneDeltaTime * -param_1 * fVar15 * fVar15;
			AUTONOMOUS_LOG(LogLevel::Verbose, "Acceleration: {} ({})", fVar15, this->dynamic.linearAcceleration);
		}
		else {
			fVar15 = this->dynamicExt.field_0x6c;
			fVar15 = GetTimer()->cutsceneDeltaTime * -param_1 * fVar15 * fVar15;
			AUTONOMOUS_LOG(LogLevel::Verbose, "dynamicExt.field_0x6c: {} ({})", fVar15, this->dynamicExt.field_0x6c);

		}

		edF32Vector4ScaleHard(fVar15, &eStack144, &this->dynamicExt.normalizedTranslation);
		peVar9 = &this->dynamicExt.aImpulseVelocities[1];

		AUTONOMOUS_LOG(LogLevel::Verbose, "eStack144 {} vel[1]: {} normalizedT: {}",
			eStack144.ToString(), this->dynamicExt.aImpulseVelocities[1].ToString(), this->dynamicExt.normalizedTranslation.ToString());

		edF32Vector4AddHard(peVar9, peVar9, &eStack144);

		AUTONOMOUS_LOG(LogLevel::Verbose, "vel[1] result: {}", this->dynamicExt.aImpulseVelocities[1].ToString());

		this->dynamicExt.aImpulseVelocityMagnitudes[1] = edF32Vector4GetDistHard(&this->dynamicExt.aImpulseVelocities[1]);
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Unknown: {}", this->dynamicExt.aImpulseVelocities[1].ToString());

	if (this->pTiedActor == (CActor*)0x0) {
		edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, aeStack544, &this->dynamic.field_0x10);
		peVar9 = this->dynamicExt.aImpulseVelocities;
		edF32Vector4AddHard(peVar9, peVar9, aeStack544);
		this->dynamicExt.aImpulseVelocityMagnitudes[0] = edF32Vector4GetDistHard(&this->dynamicExt.aImpulseVelocities[0]);
		this->dynamic.field_0x10.x = 0.0f;
		this->dynamic.field_0x10.y = 0.0f;
		this->dynamic.field_0x10.z = 0.0f;
		this->dynamic.field_0x10.w = 0.0f;
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Tied Actor: {}", this->dynamicExt.aImpulseVelocities[0].ToString());

	summedImpulseVelocities.x = 0.0f;
	summedImpulseVelocities.y = 0.0f;
	summedImpulseVelocities.z = 0.0f;
	summedImpulseVelocities.w = 0.0f;

	for (iVar8 = 0; iVar8 < 3; iVar8 = iVar8 + 1) {
		AUTONOMOUS_LOG(LogLevel::Verbose, "Vel {}: {}", iVar8, this->dynamicExt.aImpulseVelocities[iVar8].ToString());
		edF32Vector4AddHard(&summedImpulseVelocities, &summedImpulseVelocities, &this->dynamicExt.aImpulseVelocities[iVar8]);
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Summed Velocities: {} 3: {} scale: {}", summedImpulseVelocities.ToString(), this->dynamicExt.normalizedTranslation.ToString(), this->dynamicExt.field_0x6c);

	summedImpulseVelocities = summedImpulseVelocities * GetTimer()->cutsceneDeltaTime;

	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &translation, &this->dynamicExt.normalizedTranslation);
	edF32Vector4AddHard(&translation, &translation, &summedImpulseVelocities);

	if ((uVar10 & 0x4000) != 0) {
		translation.x = this->dynamicExt.field_0x60.x;
	}
	if ((uVar10 & 0x8000) != 0) {
		translation.y = this->dynamicExt.field_0x60.y;
	}
	if ((uVar10 & 0x10000) != 0) {
		translation.z = this->dynamicExt.field_0x60.z;
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Translation: {}", translation.ToString());

	this->dynamicExt.field_0x6c = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &translation);

	AUTONOMOUS_LOG(LogLevel::Verbose, "New normalizedTranslation.y: {}", this->dynamicExt.normalizedTranslation.ToString());

	local_30.x = translation.x;
	local_30.y = translation.y;
	local_30.z = translation.z;
	local_30.w = translation.w;

	edF32Vector4ScaleHard(this->dynamic.speed, &velocity, &this->dynamic.rotationQuat);

	AUTONOMOUS_LOG(LogLevel::Verbose, "Calculated velocity: {} speed: {} rotation: {}", velocity.ToString(), this->dynamic.speed, this->dynamic.rotationQuat.ToString());

	assert(std::isnan(velocity.x) == false);
	assert(std::isnan(velocity.y) == false);
	assert(std::isnan(velocity.z) == false);

	edF32Vector4AddHard(&translation, &translation, &velocity);

	AUTONOMOUS_LOG(LogLevel::Verbose, "Translation post velocity: {} ", translation.ToString());

	if ((pCollision->flags_0x4 & 2) != 0) {
		if ((uVar10 & 0x1000) == 0) {
			if (((uVar10 & 2) != 0) || ((uVar10 & 4) != 0)) {
				if ((uVar10 & 0x20000) != 0) {
					fVar12 = this->dynamicExt.field_0xc;
					fVar13 = this->dynamicExt.field_0x1c;
					local_230.y = 0.0f;
					local_230.x = translation.x;
					local_230.z = translation.z;
					local_230.w = translation.w;

					edF32Vector4SafeNormalize0Hard(&local_230, &local_230);

					fVar15 = edF32Vector4DotProductHard(&local_230, &movementSurfaceNormal);
					if (0.0f < fVar15) {
						unaff_f20 = edFIntervalUnitSrcLERP(fVar15, 1.0f, fVar12);
					}
					else {
						unaff_f20 = edFIntervalUnitSrcLERP(fVar15 + 1.0f, fVar13, 1.0f);
					}
				}

				edProjectVectorOnPlane(0.0f, &translation, &translation, &movementSurfaceNormal, uVar10 & 4);
				AUTONOMOUS_LOG(LogLevel::Verbose, "Translation edProjectVectorOnPlane: {} | {}", translation.ToString(), uVar10 & 4);

				if ((uVar10 & 0x20000) != 0) {
					edF32Vector4ScaleHard(unaff_f20, &translation, &translation);
				}
			}
		}
		else {
			edReflectVectorOnPlane(this->dynamicExt.field_0x4, &translation, &translation, &movementSurfaceNormal, 1);
			AUTONOMOUS_LOG(LogLevel::Verbose, "Translation edReflectVectorOnPlane: {}", translation.ToString());
			bVar3 = true;
		}

		if ((uVar10 & 0x200000) != 0) {
			edProjectVectorOnPlane(0.0f, &local_30, &local_30, &movementSurfaceNormal, 0);
			AUTONOMOUS_LOG(LogLevel::Verbose, "local_30 edProjectVectorOnPlane: {}", local_30.ToString());
			bVar4 = true;
		}
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Translation post plane project A: {} | {}", translation.ToString(), pCollision->flags_0x4 & 2);

	if ((pCollision->flags_0x4 & 1) != 0) {
		if ((uVar10 & 0x800) == 0) {
			if ((uVar10 & 1) != 0) {
				edProjectVectorOnPlane(0.0f, &translation, &translation, &local_50, 1);
			}
		}
		else {
			edReflectVectorOnPlane(this->dynamicExt.field_0x8, &translation, &translation, &local_50, 1);
			bVar3 = true;
		}
		if ((uVar10 & 0x400000) != 0) {
			edProjectVectorOnPlane(0.0f, &local_30, &local_30, &local_50, 1);
			bVar4 = true;
		}
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Translation plane project B: {} | {}", translation.ToString(), pCollision->flags_0x4 & 1);

	if ((pCollision->flags_0x4 & 4) != 0) {
		if ((uVar10 & 0x2000) == 0) {
			if ((uVar10 & 8) != 0) {
				edProjectVectorOnPlane(0.0f, &translation, &translation, &local_70, 1);
			}
		}
		else {
			edReflectVectorOnPlane(this->dynamicExt.field_0x8, &translation, &translation, &local_70, 1);
			bVar3 = true;
		}
		if ((uVar10 & 0x800000) != 0) {
			edProjectVectorOnPlane(0.0f, &local_30, &local_30, &local_70, 1);
			bVar4 = true;
		}
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Translation plane project C: {} | {} flags: {:x} touching ground: {}", translation.ToString(), pCollision->flags_0x4 & 4, uVar10, uVar10 & 0x40000);

	uVar7 = pCollision->flags_0x0;

	// Clamp to max h speed.
	if ((uVar10 & 0x40000) == 0) {
		horizontalTranslation.y = 0.0f;
		horizontalTranslation.x = translation.x;
		horizontalTranslation.z = translation.z;
		horizontalTranslation.w = translation.w;

		fVar12 = edF32Vector4SafeNormalize0Hard(&horizontalTranslation, &horizontalTranslation);

		fVar15 = CDynamic::gMaxSpeed_Horiz;
		if (fVar12 <= CDynamic::gMaxSpeed_Horiz) {
			fVar15 = fVar12;
		}

		edF32Vector4ScaleHard(fVar15, &horizontalTranslation, &horizontalTranslation);
		translation.z = horizontalTranslation.z;
		translation.x = horizontalTranslation.x;

		if ((translation.y < -CDynamic::gMaxSpeed_Vert) || (fVar15 = translation.y, CDynamic::gMaxSpeed_Vert < translation.y)) {
			if (CDynamic::gMaxSpeed_Vert < translation.y) {
				translation.y = CDynamic::gMaxSpeed_Vert;
				fVar15 = translation.y;
			}
			else {
				fVar15 = -CDynamic::gMaxSpeed_Vert;
				if (-CDynamic::gMaxSpeed_Vert <= translation.y) {
					fVar15 = translation.y;
				}
			}
		}
	}
	else {
		pCollision->flags_0x0 = pCollision->flags_0x0 | 0x10000;
		fVar15 = translation.y;
	}

	translation.y = fVar15;

	AUTONOMOUS_LOG(LogLevel::Verbose, "Translation: {}", translation.ToString());

	if (((uVar10 & 0x40) != 0) || (bVar3)) {
		this->dynamicExt.field_0x6c = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &translation);
		AUTONOMOUS_LOG(LogLevel::Verbose, "New normalizedTranslation.y: {}", this->dynamicExt.normalizedTranslation.ToString());
	}

	if (bVar4) {
		this->dynamicExt.field_0x6c = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &local_30);
		AUTONOMOUS_LOG(LogLevel::Verbose, "New normalizedTranslation.y: {}", this->dynamicExt.normalizedTranslation.ToString());
	}

	if (((uVar10 & 0x10) != 0) &&
		(((0.001f < fabs(translation.x) || (0.001f < fabs(translation.z))) || ((pCollision->flags_0x0 & 0x800) != 0)))) {
		edF32Vector4ScaleHard(-1.0f, &eStack192, &this->collisionContact.location);
		edF32Vector4AddHard(&translation, &translation, &eStack192);
		AUTONOMOUS_LOG(LogLevel::Verbose, "Translation result: {} collision contact: {}", translation.ToString(), this->collisionContact.location.ToString());
	}

	translation = translation * GetTimer()->cutsceneDeltaTime;

	AUTONOMOUS_LOG(LogLevel::Verbose, "Translation delta time: {}", translation.ToString());

	if ((uVar10 & 0x400) == 0) {
		if ((((uVar10 & 0x80000) == 0) || ((pCollision->flags_0x4 & 2) == 0)) ||
			(GetTimer()->scaledTotalTime - this->dynamicExt.scaledTotalTime < 0.5f)) {

			local_1e0.nbEntries = 0;
			if (pActorsTable == (CActorsTable*)0x0) {
				pActorsTable = &local_1e0;
			}

			AUTONOMOUS_LOG(LogLevel::Verbose, "Translation before CheckCollisions location: {}", this->currentLocation.ToString());

			pCollision->CheckCollisions_TranslateActor(this, &translation, pActorsTable, (CActor*)0x0, 1);

			AUTONOMOUS_LOG(LogLevel::Verbose, "Translation after CheckCollisions translation: {} location: {}", translation.ToString(), this->currentLocation.ToString());

			bVar1 = this->field_0x344;
			if ((bVar1 & 1) == 1) {
				_ManageDynamicFence(pActorsTable);
				this->field_0x344 = this->field_0x344 & 0xfdU | 2;
			}
			else {
				this->field_0x344 = bVar1 & 0xfd;
			}
		}
		else {
			translation.x = 0.0f;
			translation.y = 0.0f;
			translation.z = 0.0f;
			translation.w = 0.0f;
			UpdatePosition(&this->currentLocation, true);
			this->flags = this->flags | 0x200000;
		}
	}
	else {
		edF32Vector4AddHard(&newLocation, &this->currentLocation, &translation);
		UpdatePosition(&newLocation, true);
	}

	if ((uVar7 & 0x10000) == 0) {
		pCollision->flags_0x0 = pCollision->flags_0x0 & 0xfffeffff;
	}

	if (((pCollision->flags_0x4 & 2) != 0) && (uVar7 = CCollisionManager::IsASlidingGround(&pCollision->aCollisionContact[1]), uVar7 == 0)) {
		if ((uVar10 & 0x200) == 0) {
			if (((uVar10 & 0x1000000) != 0) && (0.99f < (pCollision->aCollisionContact[1]).location.y)) {
				fVar15 = this->dynamicExt.normalizedTranslation.y * this->dynamicExt.field_0x6c;
				this->dynamicExt.field_0x6c = sqrtf(this->dynamicExt.field_0x6c * this->dynamicExt.field_0x6c - fVar15 * fVar15);
				this->dynamicExt.normalizedTranslation.y = 0.0f;

				AUTONOMOUS_LOG(LogLevel::Verbose, "Reset normalizedTranslation.y: {}", this->dynamicExt.normalizedTranslation.ToString());

				this->dynamic.field_0x10.x = 0.0f;
				this->dynamic.field_0x10.y = 0.0f;
				this->dynamic.field_0x10.z = 0.0f;
				this->dynamic.field_0x10.w = 0.0f;
			}
		}
		else {
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;

			AUTONOMOUS_LOG(LogLevel::Verbose, "Reset normalizedTranslation: {}", this->dynamicExt.normalizedTranslation.ToString());

			this->dynamicExt.field_0x6c = 0.0f;

			this->dynamic.field_0x10.x = 0.0f;
			this->dynamic.field_0x10.y = 0.0f;
			this->dynamic.field_0x10.z = 0.0f;
			this->dynamic.field_0x10.w = 0.0f;
		}
	}

	if (((uVar10 & 0x2000000) != 0) && ((pCollision->flags_0x4 & 1) != 0)) {
		edF32Vector4NormalizeHard(&eStack496, &local_30);

		if (edF32Vector4DotProductHard(&pCollision->aCollisionContact[0].location, &eStack496) < -0.99f) {
			this->dynamicExt.field_0x6c = this->dynamicExt.field_0x6c * this->dynamicExt.normalizedTranslation.y;

			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;

			this->dynamic.field_0x10.x = 0.0f;
			this->dynamic.field_0x10.y = 0.0f;
			this->dynamic.field_0x10.z = 0.0f;
			this->dynamic.field_0x10.w = 0.0f;
		}
	}

	if ((((uVar10 & 0x4000000) != 0) && ((pCollision->flags_0x4 & 4) != 0)) && ((pCollision->aCollisionContact[2].location).y < -0.99)) {
		fVar15 = (this->dynamicExt).normalizedTranslation.y * (this->dynamicExt).field_0x6c;
		(this->dynamicExt).field_0x6c = sqrtf((this->dynamicExt).field_0x6c * (this->dynamicExt).field_0x6c - fVar15 * fVar15);
		(this->dynamicExt).normalizedTranslation.y = 0.0f;
		this->dynamic.field_0x10.x = 0.0f;
		this->dynamic.field_0x10.y = 0.0f;
		this->dynamic.field_0x10.z = 0.0f;
		this->dynamic.field_0x10.w = 0.0f;

		AUTONOMOUS_LOG(LogLevel::Verbose, "Reset normalizedTranslation.y: {}", this->dynamicExt.normalizedTranslation.ToString());
	}

	CActorWindState* pCVar7 = GetWindState();
	if (pCVar7 != (CActorWindState*)0x0) {
		GetWindState()->field_0x40 = (this->dynamicExt).aImpulseVelocities[2];
		GetWindState()->field_0x38 = (this->dynamicExt).aImpulseVelocityMagnitudes[2];
	}

	for (iVar8 = 0; iVar8 < 3; iVar8 = iVar8 + 1) {
		this->dynamicExt.aImpulseVelocities[iVar8].x = 0.0f;
		this->dynamicExt.aImpulseVelocities[iVar8].y = 0.0f;
		this->dynamicExt.aImpulseVelocities[iVar8].z = 0.0f;
		this->dynamicExt.aImpulseVelocities[iVar8].w = 0.0f;
		this->dynamicExt.aImpulseVelocityMagnitudes[iVar8] = 0.0f;
	}

	this->dynamic.field_0x4c = 0;

	if ((pCollision->flags_0x4 & 2) == 0) {
		if ((uVar10 & 0x10) == 0) {
			memset(&this->collisionContact, 0, sizeof(s_collision_contact));
		}
	}
	else {
		this->collisionContact = pCollision->aCollisionContact[1];
	}

	if (((uVar10 & 0x10000000) != 0) && ((pCollision->flags_0x4 & 2) != 0)) {
		peVar9 = &pCollision->highestVertex;

		if ((pCollision->flags_0x0 & 0x40000) == 0) {
			peVar9 = &this->currentLocation;
		}

		if (fabs(pCollision->aCollisionContact[1].field_0x10.y - peVar9->y) <= 0.03f) {
			if (pCollision->aCollisionContact[1].nbCollisionsA == 0) {
				UpdateShadow(&pCollision->aCollisionContact[1].location, 1, (short)(pCollision->aCollisionContact[1].materialFlags >> 0x10));
			}
			else {
				UpdateShadow(&pCollision->aCollisionContact[1].location, 0, this->field_0xf0);
			}
		}
	}

	if ((((pCollision->flags_0x4 & 2) != 0) && ((pCollision->flags_0x0 & 4) == 0)) &&
		(uVar10 = CCollisionManager::IsASlidingGround(&pCollision->aCollisionContact[1]), uVar10 != 0)) {
		this->dynamic.flags = this->dynamic.flags | 2;
	}

	const float directionDot = edF32Vector4DotProductHard(&translation, &velocity);
	AUTONOMOUS_LOG(LogLevel::Verbose, "Translation . velocity: {} velocity: {} translation: {}", directionDot, velocity.ToString(), translation.ToString());
	if (directionDot < 0.0) {
		this->dynamic.flags = this->dynamic.flags | 1;
	}

	if ((this->pTiedActor != (CActor*)0x0) ||
		(((uVar2 & 4) != 0 && ((pCollision->flags_0x4 & 2) == 0)))) {
		this->dynamic.flags = this->dynamic.flags | 4;
	}

	ComputeRealMoving(&oldCurrentLocation);

	fVar15 = 0.05f;
	if (0.05f < this->dynamic.linearAcceleration) {
		this->dynamicExt.scaledTotalTime = GetTimer()->scaledTotalTime;
	}

	if (this->bCollisionSphereDirty != 0) {
		UpdateCollisionSphere();
	}

	return;
}

void CActorAutonomous::StoreCollisionSphere()
{
	edColPRIM_OBJECT* pObbPrim;

	pObbPrim = this->pCollisionData->pObbPrim;

	this->field_0x280 = pObbPrim->field_0x90.xyz;
	this->field_0x28c = pObbPrim->field_0xb0.xyz;
	return;
}

void CActorAutonomous::ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	edColPRIM_OBJECT* peVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 local_10;
	CCollision* pCollision;

	pCollision = this->pCollisionData;
	if (pCollision != (CCollision*)0x0) {
		peVar1 = pCollision->pObbPrim;
		if (param_1 == 0.0) {
			peVar1->field_0x90 = *param_3;
			peVar1->field_0xb0 = *param_4;
			this->bCollisionSphereDirty = 0;
		}
		else {
			pTVar2 = GetTimer();
			this->field_0x298 = param_3->xyz;
			this->field_0x2a4 = param_4->xyz;
			edF32Vector4SubHard(&local_10, param_3, &peVar1->field_0x90);
			fVar3 = 1.0f / param_1;
			edF32Vector4ScaleHard(fVar3, &local_10, &local_10);
			this->field_0x2b0 = local_10.xyz;

			edF32Vector4SubHard(&local_10, param_4, &peVar1->field_0xb0);
			edF32Vector4ScaleHard(fVar3, &local_10, &local_10);

			this->field_0x2bc = local_10.xyz;
			this->field_0x2c8 = param_1 + pTVar2->scaledTotalTime;
			this->bCollisionSphereDirty = 1;
		}
		pCollision->InvalidatePrims();
	}
	return;
}

void CActorAutonomous::UpdateCollisionSphere()
{
	CCollision* pCol;
	edColPRIM_OBJECT* peVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 local_10;

	pCol = this->pCollisionData;
	peVar1 = pCol->pObbPrim;
	pTVar2 = GetTimer();

	if (this->field_0x2c8 < pTVar2->scaledTotalTime) {
		peVar1->field_0x90.xyz = this->field_0x298;
		(peVar1->field_0x90).w = 0.0f;

		(peVar1->field_0xb0).xyz = this->field_0x2a4;
		(peVar1->field_0xb0).w = 1.0f;
		this->bCollisionSphereDirty = 0;
	}
	else {
		pTVar2 = GetTimer();
		local_10.w = pTVar2->cutsceneDeltaTime;

		local_10.x = (this->field_0x2b0).x * local_10.w;
		local_10.y = (this->field_0x2b0).y * local_10.w;
		local_10.z = (this->field_0x2b0).z * local_10.w;

		local_10.w = local_10.w * 0.0f;
		edF32Vector4AddHard(&peVar1->field_0x90, &peVar1->field_0x90, &local_10);

		local_10.w = pTVar2->cutsceneDeltaTime;
		local_10.x = (this->field_0x2bc).x * local_10.w;
		local_10.y = (this->field_0x2bc).y * local_10.w;
		local_10.z = (this->field_0x2bc).z * local_10.w;
		local_10.w = local_10.w * 0.0f;

		edF32Vector4AddHard(&peVar1->field_0xb0, &peVar1->field_0xb0, &local_10);
	}

	pCol->InvalidatePrims();
}

void CActorAutonomous::RestoreCollisionSphere(float param_2)
{
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.xyz = this->field_0x280;
	local_10.w = 0.0f;

	local_20.xyz = this->field_0x28c;
	local_20.w = 1.0f;

	ChangeCollisionSphere(param_2, &local_10, &local_20);
	return;
}

CActorWindState* CActorAutonomous::GetWindState()
{
	return (CActorWindState*)0x0;
}

float CActorAutonomous::GetWalkSpeed()
{
	return 2.0f;
}

float CActorAutonomous::GetWalkRotSpeed()
{
	return 8.0f;
}

float CActorAutonomous::GetWalkAcceleration()
{
	return 4.0f;
}

float CActorAutonomous::GetRunSpeed()
{
	return 4.0f;
}

float CActorAutonomous::GetRunRotSpeed()
{
	return 4.712389f;
}

float CActorAutonomous::GetRunAcceleration()
{
	return 8.0f;
}

void CActorAutonomous::ComputeFrictionForceWithSpeedMax(float param_1, edF32VECTOR4* pFrictionForce, int param_4)
{
	Timer* pTVar1;
	float fVar2;
	float fVar3;

	fVar3 = 0.001f;
	if (0.001f <= param_1) {
		fVar3 = param_1;
	}

	fVar2 = this->dynamic.linearAcceleration;
	fVar3 = (fVar2 * fVar2) / (fVar3 * fVar3);

	if ((param_4 == 0) || (1.0f <= fVar3)) {
		pTVar1 = GetTimer();
		edF32Vector4ScaleHard(-fVar3 / pTVar1->cutsceneDeltaTime, pFrictionForce, &(this->dynamicExt).normalizedTranslation);
	}
	else {
		*pFrictionForce = gF32Vector4Zero;
	}

	return;
}

void CActorAutonomous::ComputeFrictionForce(float param_1, edF32VECTOR4* pFrictionForce, int param_4)
{
	Timer* pTVar1;
	float fVar2;

	if (param_4 == 0) {
		fVar2 = this->dynamic.linearAcceleration;
		pTVar1 = GetTimer();
		fVar2 = pTVar1->cutsceneDeltaTime * -param_1 * fVar2 * fVar2;
	}
	else {
		fVar2 = (this->dynamicExt).field_0x6c;
		pTVar1 = GetTimer();
		fVar2 = pTVar1->cutsceneDeltaTime * -param_1 * fVar2 * fVar2;
	}

	edF32Vector4ScaleHard(fVar2, pFrictionForce, &(this->dynamicExt).normalizedTranslation);

	return;
}

void CActorAutonomous::ComputeSlidingForce(edF32VECTOR4* pSlidingForce, int param_3)
{
	CCollision* pCVar1;
	bool bVar2;
	float t;
	float fVar3;
	edF32VECTOR4 slidingForce;

	pCVar1 = this->pCollisionData;

	if ((pCVar1->flags_0x4 & 7) == 0) {
		pSlidingForce->x = 0.0f;
		pSlidingForce->y = 0.0f;
		pSlidingForce->z = 0.0f;
		pSlidingForce->w = 0.0f;
	}
	else {
		edF32Vector4ScaleHard((this->dynamicExt).gravityScale, &slidingForce, &CDynamicExt::gForceGravity);

		if ((pCVar1->flags_0x4 & 2) != 0) {
			bVar2 = CCollisionManager::IsASlidingGround(pCVar1->aCollisionContact + 1);
			if ((bVar2 == false) && (param_3 == 0)) {
				slidingForce.x = 0.0f;
				slidingForce.y = 0.0f;
				slidingForce.z = 0.0f;
				slidingForce.w = 0.0f;
			}
			else {
				t = CCollisionManager::GetMaterialSlidingCoef(&pCVar1->aCollisionContact[1]);

				if (param_3 != 0) {
					fVar3 = edFIntervalDotSrcLERP(pCVar1->aCollisionContact[1].location.y, 1.5f, 1.0f);
					t = t * fVar3;
				}

				edF32Vector4ScaleHard(t, &slidingForce, &slidingForce);
				edProjectVectorOnPlane((this->dynamicExt).field_0x4, &slidingForce, &slidingForce, &pCVar1->aCollisionContact[1].location, 1 - param_3);
			}
		}

		*pSlidingForce = slidingForce;
	}
	return;
}

void CActorAutonomous::ClearAllSumForceExt()
{
	if (GetWindState() != (CActorWindState*)0x0) {
		GetWindState()->field_0x40 = (this->dynamicExt).aImpulseVelocities[2];
		GetWindState()->field_0x38 = (this->dynamicExt).aImpulseVelocityMagnitudes[2];
	}

	(this->dynamicExt).aImpulseVelocities[0].x = 0.0f;
	(this->dynamicExt).aImpulseVelocities[0].y = 0.0f;
	(this->dynamicExt).aImpulseVelocities[0].z = 0.0f;
	(this->dynamicExt).aImpulseVelocities[0].w = 0.0f;
	(this->dynamicExt).aImpulseVelocityMagnitudes[0] = 0.0f;

	(this->dynamicExt).aImpulseVelocities[1].x = 0.0f;
	(this->dynamicExt).aImpulseVelocities[1].y = 0.0f;
	(this->dynamicExt).aImpulseVelocities[1].z = 0.0f;
	(this->dynamicExt).aImpulseVelocities[1].w = 0.0f;
	(this->dynamicExt).aImpulseVelocityMagnitudes[1] = 0.0f;

	(this->dynamicExt).aImpulseVelocities[2].x = 0.0f;
	(this->dynamicExt).aImpulseVelocities[2].y = 0.0f;
	(this->dynamicExt).aImpulseVelocities[2].z = 0.0f;
	(this->dynamicExt).aImpulseVelocities[2].w = 0.0f;
	(this->dynamicExt).aImpulseVelocityMagnitudes[2] = 0.0f;

	return;
}

bool Wolfen_CriterionWarn(CActor* pActor, void* pParams)
{
	bool bVar1;

	bVar1 = pActor != (CActor*)pParams;
	if (bVar1) {
		bVar1 = pActor->IsKindOfObject(0x10);
		bVar1 = bVar1 != false;
	}

	return bVar1;
}

void CActorAutonomous::StateAutSoccer(float param_1, int param_3, int param_4, CActorMovable* param_5)

{
	if (param_3 == 0) {
		ManageDyn(4.0f, 0x40001, (CActorsTable*)0x0);
	}
	else {
		if (this->dynamic.speed == 0.0) {
			PlayAnim(0xa);
			this->timeInAir = 0.0f;
		}
		else {
			if ((param_5->dynamic).horizontalLinearAcceleration < param_1) {
				PlayAnim(0xa);
				this->timeInAir = 0.0f;
			}
			else {
				PlayAnim(param_4);
			}
		}

		ManageDyn(4.0f, 0x1006023b, (CActorsTable*)0x0);
	}

	return;
}

void CActorAutonomous::SV_AUT_WarnActors(float radius, float param_2, CActor* pActor)
{
	CActor* pReceiver;
	float fVar1;
	float fVar2;
	int iVar3;
	float fVar4;
	CActorsTable local_130;
	edF32VECTOR4 local_20;
	CActor* local_8;
	CActor* local_4;

	if ((radius != 0.0) && (radius != param_2)) {
		local_130.nbEntries = 0;
		local_20.xyz = this->currentLocation.xyz;
		local_20.w = radius;

		(CScene::ptable.g_ActorManager_004516a4)->cluster.GetActorsIntersectingSphereWithCriterion(&local_130, &local_20, Wolfen_CriterionWarn, this);
		iVar3 = 0;
		if (param_2 == 0.0) {
			for (; iVar3 < local_130.nbEntries; iVar3 = iVar3 + 1) {
				local_4 = pActor;
				DoMessage(local_130.aEntries[iVar3], (ACTOR_MESSAGE)0x3a, (MSG_PARAM)pActor);
			}
		}
		else {
			for (iVar3 = 0; iVar3 < local_130.nbEntries; iVar3 = iVar3 + 1) {
				pReceiver = local_130.aEntries[iVar3];
				fVar4 = (pReceiver->currentLocation).x - this->currentLocation.x;
				fVar1 = (pReceiver->currentLocation).y - this->currentLocation.y;
				fVar2 = (pReceiver->currentLocation).z - this->currentLocation.z;
				fVar4 = sqrtf(fVar4 * fVar4 + fVar1 * fVar1 + fVar2 * fVar2);

				if ((param_2 <= fVar4) && (fVar4 <= radius)) {
					local_8 = pActor;
					DoMessage(pReceiver, (ACTOR_MESSAGE)0x3a, (MSG_PARAM)pActor);
				}
			}
		}
	}
	return;
}

void CActorAutonomous::SV_AUT_MoveTo_Pathfinding(CActorMovParamsOut* pParamsOut, CActorMovParamsIn* pParamsIn, edF32VECTOR4* pPosition)
{
	int* piVar1;
	int iVar2;
	CPathFinderClient* pCVar3;
	edF32VECTOR4* pPathPosition;
	float fVar6;
	edF32VECTOR4 local_10;

	pPathPosition = pPosition;

	if (((GetPathfinderClient() == 0) || (GetPathfinderClient()->id == -1)) || ((pParamsIn->flags & 0x20) != 0)) {
		if (GetPathfinderClient() != 0) {
			GetPathfinderClient()->CleanPathDynamic();
		}
	}
	else {
		if (GetPathfinderClient()->pPathDynamic == (CPathDynamic*)0x0) {
			GetPathfinderClient()->FindPathDynamic(this, pPosition);
		}

		if (GetPathfinderClient()->pPathDynamic == (CPathDynamic*)0x0) {
			// Pathfinding failed.
			pParamsOut->flags = 2;
			pPathPosition = (edF32VECTOR4*)0x0;
		}
		else {
			int lVar4 = GetPathfinderClient()->CheckAndUpdatePathDynamic(this, pPosition);
			if (lVar4 == 0) {
				GetPathfinderClient()->ComputeSubTargetPathDynamic(this, &GetPathfinderClient()->field_0x20);
				pPathPosition = &GetPathfinderClient()->field_0x20;
			}
			else {
				if (lVar4 == 1) {
					GetPathfinderClient()->CleanPathDynamic();
				}
				else {
					if (lVar4 == 3) {
						pParamsOut->flags = 2;
					}

					GetPathfinderClient()->CleanPathDynamic();
					pPathPosition = (edF32VECTOR4*)0x0;
				}
			}
		}
	}

	if (pPathPosition != (edF32VECTOR4*)0x0) {
		SV_AUT_MoveTo_DynFence(pParamsOut, pParamsIn, pPathPosition);
		local_10 = *pPosition - *pPathPosition;
		fVar6 = edF32Vector4GetDistHard(&local_10);
		pParamsOut->moveVelocity = fVar6 + pParamsOut->moveVelocity;
	}

	return;
}

void CActorAutonomous::SV_AUT_MoveTo(CActorMovParamsOut* pParamsIn, CActorMovParamsIn* pParamsOut, edF32VECTOR4* pLocation)
{
	SV_AUT_MoveTo_Pathfinding(pParamsIn, pParamsOut, pLocation);
}

void CActorAutonomous::SV_AUT_MoveTo_DynFence(CActorMovParamsOut* pParamsIn, CActorMovParamsIn* pParamsOut, edF32VECTOR4* pLocation)
{
	float fVar1;
	float fVar2;
	bool bVar3;
	int* piVar4;
	CPathFinderClient* this_00;
	int iVar5;
	edF32VECTOR4* peVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	peVar6 = pLocation;

	if ((pParamsOut->flags & 0x100) == 0) {
		this->field_0x344 = this->field_0x344 & 0xfe;
	}
	else {
		if (((this->field_0x344 & 1) == 0) || (-1 < (long)((ulong)this->field_0x344 << 0x3e))) {
			this->field_0x344 = this->field_0x344 & 0xfe | 1;
			this->field_0x340 = 0;
		}
		else {
			if (this->field_0x348 == 0) {
				this->field_0x340 = 0;
			}
			else {
				// Where is vector_0x2f0 set?
				IMPLEMENTATION_GUARD();
				if (this->field_0x340 == 0) {
					if (0.0f < this->rotationQuat.x * this->vector_0x2f0 - this->field_0x2e8 * this->rotationQuat.z) {
						this->field_0x340 = 1;
					}
					else {
						this->field_0x340 = 2;
					}
				}
				if (this->field_0x340 == 2) {
					local_10.x = -this->vector_0x2f0 + this->field_0x2e8 * 0.25;
					local_10.z = this->field_0x2e8 + this->vector_0x2f0 * 0.25;
				}
				else {
					if (this->field_0x340 == 1) {
						local_10.x = this->vector_0x2f0 + this->field_0x2e8 * 0.25;
						local_10.z = -this->field_0x2e8 + this->vector_0x2f0 * 0.25;
					}
				}

				local_10.y = 0.0f;
				local_10.w = 0.0f;

				edF32Vector4AddHard(&local_10, &local_10, &this->currentLocation);
				if ((GetPathfinderClient()->id != -1) && ((pParamsOut->flags & 0x20) == 0)) {
					bVar3 = GetPathfinderClient()->IsValidPosition(&local_10);
					if (bVar3 == false) {
						iVar5 = 1;
						if (this->field_0x340 == 1) {
							iVar5 = 2;
						}
						this->field_0x340 = iVar5;
					}
				}

				fVar7 = this->field_0x34c->GetPosition_00117db0();
				fVar8 = GetPosition_00117db0();
				fVar9 = pLocation->x - (this->field_0x34c->currentLocation).x;
				fVar1 = pLocation->z - (this->field_0x34c->currentLocation).z;
				fVar10 = pLocation->x - this->currentLocation.x;
				fVar2 = pLocation->z - this->currentLocation.z;
				if (sqrtf(fVar9 * fVar9 + 0.0f + fVar1 * fVar1) - fVar7 < sqrtf(fVar10 * fVar10 + 0.0f + fVar2 * fVar2) - fVar8) {
					peVar6 = &local_10;
				}
			}
		}
	}

	SV_AUT_MoveTo_FixDyn(pParamsIn, pParamsOut, peVar6);

	if (pLocation != peVar6) {
		edF32Vector4SubHard(&eStack32, pLocation, peVar6);
		fVar10 = pParamsIn->moveVelocity;
		fVar9 = edF32Vector4GetDistHard(&eStack32);
		pParamsIn->moveVelocity = fVar10 + fVar9;
	}

	return;
}

void CActorAutonomous::SV_AUT_MoveTo_FixDyn(CActorMovParamsOut* pParamsIn, CActorMovParamsIn* pParamsOut, edF32VECTOR4* pLocation)
{
	uint uVar1;
	bool bVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	s_fighter_action_param local_10;
	s_fighter_action local_4;

	if ((pParamsOut->flags & 0xc0) != 0) {
		edF32Vector4SubHard(&local_20, pLocation, &this->currentLocation);
		local_20.y = 0.0;
		fVar3 = edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
		fVar5 = pParamsOut->speed / pParamsOut->rotSpeed;
		if ((fVar3 < fVar5 * 2.0f + 0.001f) && (0.001f < fVar3)) {
			local_30.z = this->rotationQuat.z;
			local_30.w = this->rotationQuat.w;
			local_30.x = this->rotationQuat.x;
			local_30.y = 0.0f;
			edF32Vector4SafeNormalize0Hard(&local_30, &local_30);

			if (0.0f < local_20.x * local_30.z - local_30.x * local_20.z) {
				local_40.x = this->rotationQuat.z;
				local_40.z = -this->rotationQuat.x;
			}
			else {
				local_40.x = -this->rotationQuat.z;
				local_40.z = this->rotationQuat.x;
			}

			local_40.w = 0.0f;
			local_40.y = 0.0f;
			edF32Vector4NormalizeHard(&local_40, &local_40);
			edF32Vector4ScaleHard(fVar5, &eStack80, &local_40);
			edF32Vector4AddHard(&eStack80, &this->currentLocation, &eStack80);
			edF32Vector4SubHard(&eStack96, pLocation, &eStack80);
			eStack96.y = 0.0f;
			fVar4 = edF32Vector4GetDistHard(&eStack96);
			if (fVar4 < fVar5) {
				fVar5 = edF32Vector4DotProductHard(&local_40, &local_20);
				if (fVar5 < 0.0f) {
					fVar5 = 0.0f;
				}
				fVar4 = 1.0f;
				if (fVar5 <= 1.0f) {
					fVar4 = fVar5;
				}
				uVar1 = pParamsOut->flags;
				fVar4 = (fVar3 * 0.5) / fVar4;
				if ((uVar1 & 0x40) == 0) {
					if ((uVar1 & 0x80) != 0) {
						fVar3 = fVar4 * pParamsOut->rotSpeed + 0.001f;
						pParamsOut->speed = fVar3;
						pParamsOut->acceleration = 3.402823e+38f;
						pParamsOut->flags = pParamsOut->flags | 0x400;
						this->dynamic.speed = fVar3;
					}
				}
				else {
					pParamsOut->rotSpeed = pParamsOut->speed / fVar4 + 0.001;
					pParamsOut->flags = pParamsOut->flags | 2;
				}
			}
		}
	}

	if ((this->curBehaviourId == 3) && (bVar2 = IsKindOfObject(OBJ_TYPE_FIGHTER), bVar2 != false)) {
		CActorFighter* pFighter = static_cast<CActorFighter*>(this);
		local_4.all = 0x1c;
		edF32Vector4SubHard(&eStack112, pLocation, &this->currentLocation);
		fVar3 = edF32Vector4SafeNormalize0Hard(&pParamsIn->moveDirection, &eStack112);
		pParamsIn->moveVelocity = fVar3;
		local_10.field_0x4 = 0;
		local_10.field_0x0 = &pParamsIn->moveDirection;
		pFighter->Execute(&local_4, &local_10);
	}
	else {
		SV_MOV_MoveTo(pParamsIn, pParamsOut, pLocation);
	}

	return;
}

void CActorAutonomous::SV_AUT_PathfindingEnd()
{
	if (this->GetPathfinderClient() != (CPathFinderClient*)0x0) {
		this->GetPathfinderClient()->CleanPathDynamic();
	}

	return;
}

bool CActorAutonomous::SV_AUT_CanMoveTo(edF32VECTOR4* v0)
{
	bool bCanMoveTo;

	if (GetPathfinderClientAlt()->id == -1) {
		bCanMoveTo = true;
	}
	else {
		bCanMoveTo = GetPathfinderClientAlt()->HasPathTo(this, v0);
	}

	return bCanMoveTo;
}

void CActorAutonomous::LifeRestore()
{
	ACTOR_LOG(LogLevel::Info, "CActorAutonomous::LifeRestore Restoring life to max {}", GetLifeInterfaceOther()->GetValueMax());
	GetLifeInterfaceOther()->SetValue(GetLifeInterfaceOther()->GetValueMax());
	return;
}

void CActorAutonomous::LifeAnnihilate()
{
	ACTOR_LOG(LogLevel::Info, "CActorAutonomous::LifeAnnihilate Setting life to 0.0f");
	GetLifeInterfaceOther()->SetValue(0.0f);
	return;
}

CLifeInterface* CActorAutonomous::GetLifeInterface()
{
	return &this->lifeInterface;
}

CLifeInterface* CActorAutonomous::GetLifeInterfaceOther()
{
	return &this->lifeInterface;
}

void CActorAutonomous::LifeDecrease(float amount)
{
	CLifeInterface* pCVar1;
	CLifeInterface* pCVar2;
	float fVar3;

	if (this->field_0x2e4 <= 0.0f) {
		pCVar1 = GetLifeInterfaceOther();
		fVar3 = pCVar1->GetValue();

		if (fVar3 <= amount) {
			pCVar1 = GetLifeInterfaceOther();
			pCVar1->SetValue(0);
		}
		else {
			pCVar1 = GetLifeInterfaceOther();
			pCVar2 = GetLifeInterfaceOther();

			fVar3 = pCVar2->GetValue();
			pCVar1->SetValue(fVar3 - amount);
		}
	}
	return;
}

void CActorAutonomous::LifeIncrease(float amount)
{
	if ((amount + GetLifeInterfaceOther()->GetValue()) < GetLifeInterfaceOther()->GetValueMax()) {
		GetLifeInterfaceOther()->SetValue(amount + GetLifeInterfaceOther()->GetValue());
	}
	else {
		GetLifeInterfaceOther()->SetValue(GetLifeInterfaceOther()->GetValueMax());
	}

	return;
}

CLifeInterface::CLifeInterface()
{
	this->priority = 2;
	this->currentValue = 1.0f;
	this->valueMax = 1.0f;
	this->field_0x10 = 0;
	return;
}

bool CLifeInterface::Manage()
{
	bool bVar1;

	bVar1 = CScene::_pinstance->IsFadeTermActive();
	bVar1 = bVar1 != false;

	if (!bVar1) {
		bVar1 = CScene::_pinstance->IsCutsceneFadeActive();
		bVar1 = bVar1 != false;
	}

	if (!bVar1) {
		bVar1 = CScene::_pinstance->IsResetFadeActive();
		bVar1 = bVar1 != false;
	}

	return (bVar1 ^ 1);
}

float CLifeInterface::GetValue()
{
	return this->currentValue;
}

void CLifeInterface::SetValue(float value)
{
	this->currentValue = value;
}

float CLifeInterface::GetValueMax()
{
	return this->valueMax;
}

void CLifeInterface::SetValueMax(float max)
{
	ACTOR_LOG(LogLevel::Info, "CLifeInterface::SetValueMax Setting max life to {}", max);
	this->valueMax = max;
	return;
}

void CLifeInterface::SetPriority(int newPriority)
{
	this->priority = newPriority;
	return;
}

float CLifeInterface::GetPriority()
{
	return this->priority;
}

float CLifeInterface::GetPercent()
{
	return GetValue() / this->valueMax;
}

void CBehaviourAutonomous::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorAutonomous*>(pOwner);
	return;
}

void CBehaviourCatchByTrap::Create(ByteCode* pByteCode)
{
	this->condOpArray.Create(pByteCode);
	return;
}

void CBehaviourCatchByTrap::Manage()
{
	uint key;
	CActorAutonomous* pAutonomous;

	pAutonomous = this->pOwner;
	if (pAutonomous->actorState == 5) {
		key = pAutonomous->DoMessage(this->field_0x10, MESSAGE_GET_BONE_ID, (MSG_PARAM)9);

		if (key != this->boneId) {
			if (this->boneId != 0) {
				pAutonomous->UnlinkFromActor();
				this->field_0x10->pAnimationController->UnRegisterBone(this->boneId);
			}

			if (key != 0) {
				this->field_0x10->pAnimationController->RegisterBone(key);
				pAutonomous->LinkToActor(this->field_0x10, key, 1);
			}

			this->boneId = key;
		}
	}

	return;
}

void CBehaviourCatchByTrap::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorAutonomous*>(pOwner);
	this->boneId = 0;

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourCatchByTrap::End(int newBehaviourId)
{
	undefined4 local_90[3];
	undefined4 local_84;
	undefined4 local_6c;
	undefined4 local_60;
	undefined4* local_4;

	local_90[0] = 10;
	local_60 = 0;
	local_84 = 0x42c80000;
	local_6c = 0x3f000000;
	local_4 = local_90;
	this->pOwner->DoMessage(this->field_0x10, (ACTOR_MESSAGE)2, local_4);
	return;
}

void CBehaviourCatchByTrap::TermState(int oldState, int newState)
{
	if ((oldState == 5) && (this->boneId != 0)) {
		this->pOwner->UnlinkFromActor();
		this->field_0x10->pAnimationController->UnRegisterBone(this->boneId);
		this->boneId = 0;
	}

	return;
}

int CBehaviourCatchByTrap::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorAutonomous* pCVar1;
	int bProcessed;

	if (msg == 0x53) {
		this->pOwner->UpdatePosition((edF32VECTOR4*)pMsgParam, true);
		bProcessed = 1;

		this->pOwner->dynamic.speed = 0.0f;

		pCVar1 = this->pOwner;
		(pCVar1->dynamicExt).normalizedTranslation.x = 0.0f;
		(pCVar1->dynamicExt).normalizedTranslation.y = 0.0f;
		(pCVar1->dynamicExt).normalizedTranslation.z = 0.0f;
		(pCVar1->dynamicExt).normalizedTranslation.w = 0.0f;
		(pCVar1->dynamicExt).field_0x6c = 0.0f;
	}
	else {
		if (msg == 0x40) {
			pCVar1 = this->pOwner;
			pCVar1->SetBehaviour(pCVar1->prevBehaviourId, -1, -1);
			bProcessed = 1;
		}
		else {
			bProcessed = 0;
		}
	}

	return bProcessed;
}
