#include "ActorAutonomous.h"

#include <string.h>
#include <math.h>

#include "MemoryStream.h"
#include "TimeController.h"
#include "MathOps.h"
#include "CollisionManager.h"

int INT_00448e08 = 0;

#define AUTONOMOUS_LOG(level, format, ...) MY_LOG_CATEGORY("Autonomous", level, format, ##__VA_ARGS__)

CDynamicExt::CDynamicExt()
{
	this->gravityScale = 1.0f;
	this->field_0x4 = 0.0f;
	this->field_0x8 = 0.0f;
}

void CDynamicExt::ClearLocalData()
{
	Timer* pTVar1;

	(this->aVelocity[0]).x = 0.0f;
	(this->aVelocity[0]).y = 0.0f;
	(this->aVelocity[0]).z = 0.0f;
	(this->aVelocity[0]).w = 0.0f;
	this->aVelocityMagnitudes[0] = 0.0f;
	(this->aVelocity[1]).x = 0.0f;
	(this->aVelocity[1]).y = 0.0f;
	(this->aVelocity[1]).z = 0.0f;
	(this->aVelocity[1]).w = 0.0f;
	this->aVelocityMagnitudes[1] = 0.0f;
	(this->aVelocity[2]).x = 0.0f;
	(this->aVelocity[2]).y = 0.0f;
	(this->aVelocity[2]).z = 0.0f;
	(this->aVelocity[2]).w = 0.0f;
	this->aVelocityMagnitudes[2] = 0.0f;
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
	//lVar5 = (*(code*)this->base.pVTable[1].free)(this);
	//if (lVar5 != 0) {
	//	peVar3 = (edF32MATRIX4*)(*(code*)this->base.pVTable[1].free)(this);
	//	FUN_0019e4a0(peVar3);
	//}
	this->bCollisionSphereDirty = 0;
	//this->field_0x2c8 = 0;
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

void CActorAutonomous::Init()
{
	//CVision* pCVar1;
	int iVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	IMPLEMENTATION_GUARD_LOG(
	pCVar1 = (*((this->pVTable)->actorBase).getPerception)((CActorWolfen*)this);
	if (pCVar1 != (CVision*)0x0) {
		pCVar1 = (*((this->pVTable)->actorBase).getPerception)((CActorWolfen*)this);
		fVar5 = this->currentLocation.y;
		fVar3 = this->currentLocation.z;
		fVar4 = this->currentLocation.w;
		(pCVar1->baseLocation_0x10).x = this->currentLocation.x;
		(pCVar1->baseLocation_0x10).y = fVar5;
		(pCVar1->baseLocation_0x10).z = fVar3;
		(pCVar1->baseLocation_0x10).w = fVar4;
		pCVar1 = (*((this->pVTable)->actorBase).getPerception)((CActorWolfen*)this);
		fVar5 = this->rotationQuat.y;
		fVar3 = this->rotationQuat.z;
		fVar4 = this->rotationQuat.w;
		(pCVar1->vectorB_0x20).x = this->rotationQuat.x;
		(pCVar1->vectorB_0x20).y = fVar5;
		(pCVar1->vectorB_0x20).z = fVar3;
		(pCVar1->vectorB_0x20).w = fVar4;
	})

	GetLifeInterfaceOther()->SetPriority(0);
	LifeRestore();
	this->field_0x2e4 = 0.0f;

	memset(&this->collisionContact, 0, sizeof(s_collision_contact));
	this->field_0x344 = this->field_0x344 & 0xfe;
	CActor::Init();
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

int CActorAutonomous::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	int* piVar2;
	edF32MATRIX4* peVar3;
	CBehaviour* pCVar4;
	int iVar5;
	long lVar6;

	if (msg == 0x3f) {
		IMPLEMENTATION_GUARD(
		bVar1 = (*(this->pVTable)->SetBehaviour)((CActor*)this, 2, -1, -1);
		if (bVar1 == false) {
			iVar5 = 0;
		}
		else {
			pCVar4 = CActor::GetBehaviour((CActor*)this, this->curBehaviourId);
			pCVar4[4].pVTable = (CBehaviourVtable*)pSender;
			iVar5 = 1;
		})
	}
	else {
		if (msg == 0x18) {
			IMPLEMENTATION_GUARD(
			lVar6 = (*(code*)(this->pVTable)->field_0x100)();
			if (lVar6 != 0) {
				peVar3 = (edF32MATRIX4*)(*(code*)(this->pVTable)->field_0x100)(this);
				FUN_00123580(peVar3, (edF32MATRIX4*)pMsgParam);
			}
			iVar5 = 1;)
		}
		else {
			if (msg == 0x17) {
				IMPLEMENTATION_GUARD(
				lVar6 = (*(code*)(this->pVTable)->field_0x100)();
				if (lVar6 != 0) {
					piVar2 = (int*)(*(code*)(this->pVTable)->field_0x100)(this);
					FUN_00123810(piVar2, (float*)pMsgParam);
				}
				iVar5 = 1;)
			}
			else {
				iVar5 = CActorMovable::InterpretMessage(pSender, msg, pMsgParam);
			}
		}
	}
	return iVar5;
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
	if (pActorsTable->entryCount < 1) {
		this->field_0x348 = 0;
	}
	else {
		iVar5 = 0;
		local_10 = gF32Vector4Zero;

		this->field_0x348 = 0;
		this->field_0x34c = (CActor*)0x0;
		peVar2 = pCVar1->pObbPrim;
		fVar7 = (peVar2->field_0xb0).y;
		fVar6 = (peVar2->field_0x90).y * 0.5;
		fVar8 = fVar7 - fVar6;
		if (0 < pActorsTable->entryCount) {
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
					lVar3 = pActorsTable->aEntries[iVar5]->IsKindOfObject(4);
					if (lVar3 != 0) {
						IMPLEMENTATION_GUARD(
						*(byte*)(pCVar4->field_0x4[0] + 0x344) = *(byte*)(pCVar4->field_0x4[0] + 0x344) & 0xfe;)
					}
				}
				iVar5 = iVar5 + 1;
		
			} while (iVar5 < pActorsTable->entryCount);
		}
		if (0 < this->field_0x348) {
			IMPLEMENTATION_GUARD(
			edF32Vector4SafeNormalize0Hard(&local_30, &local_10);
			this->field_0x2e8 = local_30.x;
			this->vector_0x2f0 = local_30.z;)
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
	Timer* pTVar5;
	Timer* pTVar6;
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
	edF32VECTOR4 local_a0;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 movementSurfaceNormal;
	edF32VECTOR4 local_50;
	edF32VECTOR4 currentLocation;
	edF32VECTOR4 local_30;
	edF32VECTOR4 velocity;
	edF32VECTOR4 translation;
	CCollision* pCollision;

	AUTONOMOUS_LOG(LogLevel::Verbose, "CActorAutonomous::ManageDyn flags: 0x{:x}", this->pCollisionData->flags_0x4);

	pCollision = this->pCollisionData;
	pTVar5 = GetTimer();
	bVar3 = false;
	uVar2 = this->dynamic.flags;

	currentLocation = this->currentLocation;

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

	// Gravity
	if ((((uVar10 & 0x20) != 0) && ((pCollision->flags_0x4 & 2) == 0)) || ((uVar10 & 0x8000000) != 0)) {
		edF32Vector4ScaleHard(this->dynamicExt.gravityScale, &eStack128, &CDynamicExt::gForceGravity);
		peVar9 = this->dynamicExt.aVelocity;
		edF32Vector4AddHard(peVar9, peVar9, &eStack128);
		fVar15 = edF32Vector4GetDistHard(&this->dynamicExt.aVelocity[0]);
		this->dynamicExt.aVelocityMagnitudes[0] = fVar15;
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Gravity: {}", this->dynamicExt.aVelocity[0].ToString());

	if (((uVar10 & 0x100) != 0) && ((pCollision->flags_0x4 & 2) == 0)) {
		if ((uVar10 & 0x100000) == 0) {
			fVar15 = this->dynamic.linearAcceleration;
			pTVar6 = GetTimer();
			fVar15 = pTVar6->cutsceneDeltaTime * -param_1 * fVar15 * fVar15;

			AUTONOMOUS_LOG(LogLevel::Verbose, "Acceleration: {} ({})", fVar15, this->dynamic.linearAcceleration);
		}
		else {
			fVar15 = this->dynamicExt.field_0x6c;
			pTVar6 = GetTimer();
			fVar15 = pTVar6->cutsceneDeltaTime * -param_1 * fVar15 * fVar15;

			AUTONOMOUS_LOG(LogLevel::Verbose, "Acceleration: {} ({})", fVar15, this->dynamicExt.field_0x6c);

		}

		edF32Vector4ScaleHard(fVar15, &eStack144, &this->dynamicExt.normalizedTranslation);
		peVar9 = &this->dynamicExt.aVelocity[1];

		AUTONOMOUS_LOG(LogLevel::Verbose, "eStack144 {} vel[1]: {} normalizedT: {}",
			eStack144.ToString(), this->dynamicExt.aVelocity[1].ToString(), this->dynamicExt.normalizedTranslation.ToString());

		edF32Vector4AddHard(peVar9, peVar9, &eStack144);

		AUTONOMOUS_LOG(LogLevel::Verbose, "vel[1] result: {}", this->dynamicExt.aVelocity[1].ToString());

		fVar15 = edF32Vector4GetDistHard(&this->dynamicExt.aVelocity[1]);
		this->dynamicExt.aVelocityMagnitudes[1] = fVar15;
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Unknown: {}", this->dynamicExt.aVelocity[1].ToString());

	if (this->pTiedActor == (CActor*)0x0) {
		pTVar6 = GetTimer();
		edF32Vector4ScaleHard(0.02f / pTVar6->cutsceneDeltaTime, aeStack544, &this->dynamic.field_0x10);
		peVar9 = this->dynamicExt.aVelocity;
		edF32Vector4AddHard(peVar9, peVar9, aeStack544);
		fVar15 = edF32Vector4GetDistHard(&this->dynamicExt.aVelocity[0]);
		this->dynamicExt.aVelocityMagnitudes[0] = fVar15;
		this->dynamic.field_0x10.x = 0.0f;
		this->dynamic.field_0x10.y = 0.0f;
		this->dynamic.field_0x10.z = 0.0f;
		this->dynamic.field_0x10.w = 0.0f;
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Tied Actor: {}", this->dynamicExt.aVelocity[0].ToString());

	local_a0.x = 0.0f;
	local_a0.y = 0.0f;
	local_a0.z = 0.0f;
	local_a0.w = 0.0f;

	for (iVar8 = 0; iVar8 < 3; iVar8 = iVar8 + 1) {
		AUTONOMOUS_LOG(LogLevel::Verbose, "Vel {}: {}", iVar8, this->dynamicExt.aVelocity[iVar8].ToString());
		edF32Vector4AddHard(&local_a0, &local_a0, &this->dynamicExt.aVelocity[iVar8]);
	}

	AUTONOMOUS_LOG(LogLevel::Verbose, "Summed Velocities: {} 3: {} scale: {}", local_a0.ToString(), this->dynamicExt.normalizedTranslation.ToString(), this->dynamicExt.field_0x6c);

	fVar15 = pTVar5->cutsceneDeltaTime;
	local_a0.x = local_a0.x * fVar15;
	local_a0.y = local_a0.y * fVar15;
	local_a0.z = local_a0.z * fVar15;
	local_a0.w = local_a0.w * fVar15;
	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &translation, &this->dynamicExt.normalizedTranslation);
	edF32Vector4AddHard(&translation, &translation, &local_a0);

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

	fVar15 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &translation);
	this->dynamicExt.field_0x6c = fVar15;

	AUTONOMOUS_LOG(LogLevel::Verbose, "New normalizedTranslation.y: {}", this->dynamicExt.normalizedTranslation.ToString());

	local_30.x = translation.x;
	local_30.y = translation.y;
	local_30.z = translation.z;
	local_30.w = translation.w;

	edF32Vector4ScaleHard(this->dynamic.speed, &velocity, &this->dynamic.rotationQuat);

	AUTONOMOUS_LOG(LogLevel::Verbose, "Calculated velocity: {} speed: {} rotation: {}", velocity.ToString(), this->dynamic.speed, this->dynamic.rotationQuat.ToString());

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
		fVar15 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &translation);
		this->dynamicExt.field_0x6c = fVar15;

		AUTONOMOUS_LOG(LogLevel::Verbose, "New normalizedTranslation.y: {}", this->dynamicExt.normalizedTranslation.ToString());
	}

	if (bVar4) {
		fVar15 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &local_30);
		this->dynamicExt.field_0x6c = fVar15;

		AUTONOMOUS_LOG(LogLevel::Verbose, "New normalizedTranslation.y: {}", this->dynamicExt.normalizedTranslation.ToString());
	}

	if (((uVar10 & 0x10) != 0) &&
		(((0.001f < fabs(translation.x) || (0.001f < fabs(translation.z))) || ((pCollision->flags_0x0 & 0x800) != 0)))) {
		edF32Vector4ScaleHard(-1.0f, &eStack192, &this->collisionContact.location);
		edF32Vector4AddHard(&translation, &translation, &eStack192);
		AUTONOMOUS_LOG(LogLevel::Verbose, "Translation result: {} collision contact: {}", translation.ToString(), this->collisionContact.location.ToString());
	}

	fVar15 = pTVar5->cutsceneDeltaTime;
	translation = translation * fVar15;

	AUTONOMOUS_LOG(LogLevel::Verbose, "Translation delta time: {}", translation.ToString());

	if ((uVar10 & 0x400) == 0) {
		if ((((uVar10 & 0x80000) == 0) || ((pCollision->flags_0x4 & 2) == 0)) ||
			(pTVar5->scaledTotalTime - this->dynamicExt.scaledTotalTime < 0.5f)) {

			local_1e0.entryCount = 0;
			if (pActorsTable == (CActorsTable*)0x0) {
				pActorsTable = &local_1e0;
			}

			AUTONOMOUS_LOG(LogLevel::Verbose, "Translation before CheckCollisions location: {}", this->currentLocation.ToString());

			pCollision->CheckCollisions_TranslateActor(this, &translation, pActorsTable, 0, 1);

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
		fVar15 = edF32Vector4DotProductHard(&pCollision->aCollisionContact[0].location, &eStack496);

		if (fVar15 < -0.99f) {
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

	IMPLEMENTATION_GUARD_LOG(
	lVar11 = (*(code*)(this->base.pVTable)->field_0x100)(this);
	if (lVar11 != 0) {
		fVar15 = this->dynamicExt.base.field_0x40;
		iVar8 = (*(code*)(this->base.pVTable)->field_0x100)(this);
		*(float*)(iVar8 + 0x40) = fVar15;
		fVar15 = this->dynamicExt.base.field_0x44;
		iVar8 = (*(code*)(this->base.pVTable)->field_0x100)(this);
		*(float*)(iVar8 + 0x44) = fVar15;
		fVar15 = this->dynamicExt.base.speed;
		iVar8 = (*(code*)(this->base.pVTable)->field_0x100)(this);
		*(float*)(iVar8 + 0x48) = fVar15;
		uVar16 = this->dynamicExt.base.field_0x4c;
		iVar8 = (*(code*)(this->base.pVTable)->field_0x100)(this);
		*(undefined4*)(iVar8 + 0x4c) = uVar16;
		fVar15 = this->dynamicExt.base.field_0x10.z;
		iVar8 = (*(code*)(this->base.pVTable)->field_0x100)(this);
		*(float*)(iVar8 + 0x38) = fVar15;
	})

	for (iVar8 = 0; iVar8 < 3; iVar8 = iVar8 + 1) {
		this->dynamicExt.aVelocity[iVar8].x = 0.0f;
		this->dynamicExt.aVelocity[iVar8].y = 0.0f;
		this->dynamicExt.aVelocity[iVar8].z = 0.0f;
		this->dynamicExt.aVelocity[iVar8].w = 0.0f;
		this->dynamicExt.aVelocityMagnitudes[iVar8] = 0.0f;
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

	ComputeRealMoving(&currentLocation);

	fVar15 = 0.05f;
	if (0.05f < this->dynamic.linearAcceleration) {
		pTVar5 = GetTimer();
		fVar15 = pTVar5->scaledTotalTime;
		this->dynamicExt.scaledTotalTime = fVar15;
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

void CActorAutonomous::LifeRestore()
{
	GetLifeInterfaceOther()->SetValueMax(GetLifeInterfaceOther()->GetValueMax());
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

CLifeInterface::CLifeInterface()
{
	this->priority = 2;
	this->currentValue = 1.0f;
	this->valueMax = 1.0f;
	this->field_0x10 = 0;
	return;
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
	this->valueMax = max;
	return;
}

void CLifeInterface::SetPriority(int newPriority)
{
	this->priority = newPriority;
	return;
}
