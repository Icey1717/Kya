#include "ActorAutonomous.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "MathOps.h"
#include "CollisionManager.h"

int INT_00448e08 = 0;

CDynamicExt::CDynamicExt()
{
	this->gravityScale = 1.0f;
	this->field_0x4 = 0.0f;
	this->field_0x8 = 0.0f;
}

void CDynamicExt::ClearLocalData()
{
	Timer* pTVar1;

	(this->velocity).x = 0.0f;
	(this->velocity).y = 0.0f;
	(this->velocity).z = 0.0f;
	(this->velocity).w = 0.0f;
	this->field_0x10[0] = 0.0f;
	(this->field_0x30).x = 0.0f;
	(this->field_0x30).y = 0.0f;
	(this->field_0x30).z = 0.0f;
	(this->field_0x30).w = 0.0f;
	this->field_0x10[1] = 0.0f;
	(this->field_0x40).x = 0.0f;
	(this->field_0x40).y = 0.0f;
	(this->field_0x40).z = 0.0f;
	(this->field_0x40).w = 0.0f;
	this->field_0x10[2] = 0.0f;
	(this->field_0x50).x = 0.0f;
	(this->field_0x50).y = 0.0f;
	(this->field_0x50).z = 0.0f;
	(this->field_0x50).w = 0.0f;
	(this->field_0x60).x = 0.0f;
	(this->field_0x60).y = 0.0f;
	(this->field_0x60).z = 0.0f;
	this->field_0x6c = 0.0f;
	this->field_0xc = 1.7f;
	this->field_0x10[3] = 0.7f;
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
	this->field_0x2cc = 0;
	//this->field_0x2c8 = 0;
	if (this->pCollisionData != (CCollision*)0x0) {
		StoreCollisionSphere();
	}
	uVar4 = pByteCode->GetU32();
	//CLifeInterface::SetValueMax((float)uVar4, &this->field_0x2d0);
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
	//this->field_0x308 = 3.141593;
	//this->field_0x318 = -1.047198;
	//this->field_0x31c = 1.047198;
	//this->field_0x314 = 0.0f;
	//this->field_0x324 = -1.047198;
	//this->field_0x328 = 1.047198;
	//this->field_0x320 = 0.0f;
	//this->field_0x32c = 3396.333;
	//this->field_0x330 = 3396.333;
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
	pCVar1 = (*(((this->base).base.pVTable)->actorBase).getPerception)((CActorWolfen*)this);
	if (pCVar1 != (CVision*)0x0) {
		pCVar1 = (*(((this->base).base.pVTable)->actorBase).getPerception)((CActorWolfen*)this);
		fVar5 = this->currentLocation.y;
		fVar3 = this->currentLocation.z;
		fVar4 = this->currentLocation.w;
		(pCVar1->baseLocation_0x10).x = this->currentLocation.x;
		(pCVar1->baseLocation_0x10).y = fVar5;
		(pCVar1->baseLocation_0x10).z = fVar3;
		(pCVar1->baseLocation_0x10).w = fVar4;
		pCVar1 = (*(((this->base).base.pVTable)->actorBase).getPerception)((CActorWolfen*)this);
		fVar5 = this->rotationQuat.y;
		fVar3 = this->rotationQuat.z;
		fVar4 = this->rotationQuat.w;
		(pCVar1->vectorB_0x20).x = this->rotationQuat.x;
		(pCVar1->vectorB_0x20).y = fVar5;
		(pCVar1->vectorB_0x20).z = fVar3;
		(pCVar1->vectorB_0x20).w = fVar4;
	}
	iVar2 = (*(code*)((this->base).base.pVTable)->field_0x13c)(this);
	CLifeInterface::SetPriority(iVar2, 0);
	(*(code*)((this->base).base.pVTable)->field_0x130)();
	*(undefined4*)&this->field_0x2e4 = 0;
	)
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

float CActorAutonomous::ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable)
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
	edF32VECTOR4 eStack208;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 translation;
	CCollision* pCollision;

	pCollision = this->pCollisionData;
	pTVar5 = GetTimer();
	bVar3 = false;
	uVar2 = this->dynamic.flags;

	local_40 = this->currentLocation;

	this->dynamic.flags = 0;
	uVar10 = flags | this->dynamic.field_0x4c;

	local_60 = (pCollision->aCollisionContact[1]).field_0x0;
	local_50 = (pCollision->aCollisionContact[0]).field_0x0;
	local_70 = (pCollision->aCollisionContact[2]).field_0x0;

	bVar4 = false;
	if (((pCollision->flags_0x4 & 1) != 0) && ((uVar10 & 0x10) != 0)) {
		local_50.y = 0.0f;
		edF32Vector4SafeNormalize1Hard(&local_50, &local_50);
	}

	// Gravity
	if ((((uVar10 & 0x20) != 0) && ((pCollision->flags_0x4 & 2) == 0)) || ((uVar10 & 0x8000000) != 0)) {
		edF32Vector4ScaleHard(this->dynamicExt.gravityScale, &eStack128, &CDynamicExt::gForceGravity);
		peVar9 = &this->dynamicExt.velocity;
		edF32Vector4AddHard(peVar9, peVar9, &eStack128);
		fVar15 = edF32Vector4GetDistHard(&this->dynamicExt.velocity);
		this->dynamicExt.field_0x10[0] = fVar15;
	}

	if (((uVar10 & 0x100) != 0) && ((pCollision->flags_0x4 & 2) == 0)) {
		if ((uVar10 & 0x100000) == 0) {
			fVar15 = this->dynamic.field_0x44;
			pTVar6 = GetTimer();
			fVar15 = pTVar6->cutsceneDeltaTime * -param_1 * fVar15 * fVar15;
		}
		else {
			fVar15 = this->dynamicExt.field_0x6c;
			pTVar6 = GetTimer();
			fVar15 = pTVar6->cutsceneDeltaTime * -param_1 * fVar15 * fVar15;
		}

		edF32Vector4ScaleHard(fVar15, &eStack144, &this->dynamicExt.field_0x50);
		peVar9 = &this->dynamicExt.field_0x30;
		edF32Vector4AddHard(peVar9, peVar9, &eStack144);
		fVar15 = edF32Vector4GetDistHard(&this->dynamicExt.field_0x30);
		this->dynamicExt.field_0x10[1] = fVar15;
	}

	if (this->pTiedActor == (CActor*)0x0) {
		pTVar6 = GetTimer();
		edF32Vector4ScaleHard(0.02f / pTVar6->cutsceneDeltaTime, aeStack544, &this->dynamic.field_0x10);
		peVar9 = &this->dynamicExt.velocity;
		edF32Vector4AddHard(peVar9, peVar9, aeStack544);
		fVar15 = edF32Vector4GetDistHard(&this->dynamicExt.velocity);
		this->dynamicExt.field_0x10[0] = fVar15;
		this->dynamic.field_0x10.x = 0.0f;
		this->dynamic.field_0x10.y = 0.0f;
		this->dynamic.field_0x10.z = 0.0f;
		this->dynamic.field_0x10.w = 0.0f;
	}

	local_a0.x = 0.0f;
	local_a0.y = 0.0f;
	local_a0.z = 0.0f;
	local_a0.w = 0.0f;

	for (iVar8 = 0; iVar8 < 3; iVar8 = iVar8 + 1) {
		// Dodgy, should this be matrix / vector[4]?
		IMPLEMENTATION_GUARD_LOG();
		edF32Vector4AddHard(&local_a0, &local_a0, &this->dynamicExt.velocity + iVar8);
	}

	fVar15 = pTVar5->cutsceneDeltaTime;
	local_a0.x = local_a0.x * fVar15;
	local_a0.y = local_a0.y * fVar15;
	local_a0.z = local_a0.z * fVar15;
	local_a0.w = local_a0.w * fVar15;
	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &translation, &this->dynamicExt.field_0x50);
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

	fVar15 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.field_0x50, &translation);

	this->dynamicExt.field_0x6c = fVar15;
	local_30.x = translation.x;
	local_30.y = translation.y;
	local_30.z = translation.z;
	local_30.w = translation.w;
	edF32Vector4ScaleHard(this->dynamic.intensity, &eStack32, &this->dynamic.rotationQuat);
	edF32Vector4AddHard(&translation, &translation, &eStack32);
	if ((pCollision->flags_0x4 & 2) != 0) {
		if ((uVar10 & 0x1000) == 0) {
			if (((uVar10 & 2) != 0) || ((uVar10 & 4) != 0)) {
				if ((uVar10 & 0x20000) != 0) {
					fVar12 = this->dynamicExt.field_0xc;
					fVar13 = this->dynamicExt.field_0x10[3];
					local_230.y = 0.0f;
					local_230.x = translation.x;
					local_230.z = translation.z;
					local_230.w = translation.w;
					edF32Vector4SafeNormalize0Hard(&local_230, &local_230);
					fVar15 = edF32Vector4DotProductHard(&local_230, &local_60);
					if (0.0 < fVar15) {
						unaff_f20 = edFIntervalUnitSrcLERP(fVar15, 1.0f, fVar12);
					}
					else {
						unaff_f20 = edFIntervalUnitSrcLERP(fVar15 + 1.0f, fVar13, 1.0f);
					}
				}
				edProjectVectorOnPlane(0.0f, &translation, &translation, &local_60, uVar10 & 4);
				if ((uVar10 & 0x20000) != 0) {
					edF32Vector4ScaleHard(unaff_f20, &translation, &translation);
				}
			}
		}
		else {
			edReflectVectorOnPlane(this->dynamicExt.field_0x4, &translation, &translation, &local_60, 1);
			bVar3 = true;
		}
		if ((uVar10 & 0x200000) != 0) {
			edProjectVectorOnPlane(0.0f, &local_30, &local_30, &local_60, 0);
			bVar4 = true;
		}
	}
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
	uVar7 = pCollision->flags_0x0;
	if ((uVar10 & 0x40000) == 0) {
		local_b0.y = 0.0f;
		local_b0.x = translation.x;
		local_b0.z = translation.z;
		local_b0.w = translation.w;
		fVar12 = edF32Vector4SafeNormalize0Hard(&local_b0, &local_b0);
		fVar15 = CDynamic::gMaxSpeed_Horiz;
		if (fVar12 <= CDynamic::gMaxSpeed_Horiz) {
			fVar15 = fVar12;
		}
		edF32Vector4ScaleHard(fVar15, &local_b0, &local_b0);
		translation.z = local_b0.z;
		translation.x = local_b0.x;
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
	if (((uVar10 & 0x40) != 0) || (bVar3)) {
		fVar15 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.field_0x50, &translation);
		this->dynamicExt.field_0x6c = fVar15;
	}
	if (bVar4) {
		fVar15 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.field_0x50, &local_30);
		this->dynamicExt.field_0x6c = fVar15;
	}
	if (((uVar10 & 0x10) != 0) &&
		(((0.001f < fabs(translation.x) || (0.001f < fabs(translation.z))) || ((pCollision->flags_0x0 & 0x800) != 0)))) {
		edF32Vector4ScaleHard(-1.0f, &eStack192, &this->collisionContact.field_0x0);
		edF32Vector4AddHard(&translation, &translation, &eStack192);
	}
	fVar15 = pTVar5->cutsceneDeltaTime;
	translation.x = translation.x * fVar15;
	translation.y = translation.y * fVar15;
	translation.z = translation.z * fVar15;
	translation.w = translation.w * fVar15;
	if ((uVar10 & 0x400) == 0) {
		if ((((uVar10 & 0x80000) == 0) || ((pCollision->flags_0x4 & 2) == 0)) ||
			(pTVar5->scaledTotalTime - this->dynamicExt.scaledTotalTime < 0.5f)) {

			local_1e0.entryCount = 0;
			if (pActorsTable == (CActorsTable*)0x0) {
				pActorsTable = &local_1e0;
			}
			pCollision->CheckCollisions_TranslateActor(this, &translation, pActorsTable, 0, 1);
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
		edF32Vector4AddHard(&eStack208, &this->currentLocation, &translation);
		UpdatePosition(&eStack208, true);
	}

	if ((uVar7 & 0x10000) == 0) {
		pCollision->flags_0x0 = pCollision->flags_0x0 & 0xfffeffff;
	}

	if (((pCollision->flags_0x4 & 2) != 0) &&
		(uVar7 = CCollisionManager::IsASlidingGround(&pCollision->aCollisionContact[1]), uVar7 == 0)) {
		if ((uVar10 & 0x200) == 0) {
			if (((uVar10 & 0x1000000) != 0) && (0.99f < (pCollision->aCollisionContact[1]).field_0x0.y)) {
				fVar15 = this->dynamicExt.field_0x50.y * this->dynamicExt.field_0x6c;
				this->dynamicExt.field_0x6c = sqrtf(this->dynamicExt.field_0x6c * this->dynamicExt.field_0x6c - fVar15 * fVar15);
				this->dynamicExt.field_0x50.y = 0.0f;

				this->dynamic.field_0x10.x = 0.0f;
				this->dynamic.field_0x10.y = 0.0f;
				this->dynamic.field_0x10.z = 0.0f;
				this->dynamic.field_0x10.w = 0.0f;
			}
		}
		else {
			this->dynamicExt.field_0x50.x = 0.0f;
			this->dynamicExt.field_0x50.y = 0.0f;
			this->dynamicExt.field_0x50.z = 0.0f;
			this->dynamicExt.field_0x50.w = 0.0f;

			this->dynamicExt.field_0x6c = 0.0f;

			this->dynamic.field_0x10.x = 0.0f;
			this->dynamic.field_0x10.y = 0.0f;
			this->dynamic.field_0x10.z = 0.0f;
			this->dynamic.field_0x10.w = 0.0f;
		}
	}
	if (((uVar10 & 0x2000000) != 0) && ((pCollision->flags_0x4 & 1) != 0)) {
		edF32Vector4NormalizeHard(&eStack496, &local_30);
		fVar15 = edF32Vector4DotProductHard(&pCollision->aCollisionContact[0].field_0x0, &eStack496);

		if (fVar15 < -0.99f) {
			this->dynamicExt.field_0x6c = this->dynamicExt.field_0x6c * this->dynamicExt.field_0x50.y;

			this->dynamicExt.field_0x50.x = 0.0f;
			this->dynamicExt.field_0x50.z = 0.0f;

			this->dynamic.field_0x10.x = 0.0f;
			this->dynamic.field_0x10.y = 0.0f;
			this->dynamic.field_0x10.z = 0.0f;
			this->dynamic.field_0x10.w = 0.0f;
		}
	}

	if ((((uVar10 & 0x4000000) != 0) && ((pCollision->flags_0x4 & 4) != 0)) && ((pCollision->aCollisionContact[2].field_0x0).y < -0.99)) {
		fVar15 = (this->dynamicExt).field_0x50.y * (this->dynamicExt).field_0x6c;
		(this->dynamicExt).field_0x6c = sqrtf((this->dynamicExt).field_0x6c * (this->dynamicExt).field_0x6c - fVar15 * fVar15);
		(this->dynamicExt).field_0x50.y = 0.0f;
		this->dynamic.field_0x10.x = 0.0f;
		this->dynamic.field_0x10.y = 0.0f;
		this->dynamic.field_0x10.z = 0.0f;
		this->dynamic.field_0x10.w = 0.0f;
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
		fVar15 = this->dynamicExt.base.intensity;
		iVar8 = (*(code*)(this->base.pVTable)->field_0x100)(this);
		*(float*)(iVar8 + 0x48) = fVar15;
		uVar16 = this->dynamicExt.base.field_0x4c;
		iVar8 = (*(code*)(this->base.pVTable)->field_0x100)(this);
		*(undefined4*)(iVar8 + 0x4c) = uVar16;
		fVar15 = this->dynamicExt.base.field_0x10.z;
		iVar8 = (*(code*)(this->base.pVTable)->field_0x100)(this);
		*(float*)(iVar8 + 0x38) = fVar15;
	})

		IMPLEMENTATION_GUARD_LOG(
	for (iVar8 = 0; iVar8 < 3; iVar8 = iVar8 + 1) {
		ppCVar14 = &this->base.pVTable + iVar8 * 4;
		ppCVar14[0x7c] = (CActorFighterVTable*)0x0;
		ppCVar14[0x7d] = (CActorFighterVTable*)0x0;
		ppCVar14[0x7e] = (CActorFighterVTable*)0x0;
		ppCVar14[0x7f] = (CActorFighterVTable*)0x0;
		this->dynamicExt.field_0x10[iVar8] = 0.0f;
	})

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
				IMPLEMENTATION_GUARD(
				ActorGroundFunc_00104b70
				((Actor*)this, (edF32VECTOR4*)&pCollision->collisionContact, 1,
					(short)((pCollision->collisionContact).field_0x20 >> 0x10));)
			}
			else {
				IMPLEMENTATION_GUARD(
				ActorGroundFunc_00104b70
				((Actor*)this, (edF32VECTOR4*)&pCollision->collisionContact, 0, this->field_0xf0);)
			}
		}
	}

	if ((((pCollision->flags_0x4 & 2) != 0) && ((pCollision->flags_0x0 & 4) == 0)) &&
		(uVar10 = CCollisionManager::IsASlidingGround(&pCollision->aCollisionContact[1]), uVar10 != 0)) {
		this->dynamic.flags = this->dynamic.flags | 2;
	}

	fVar15 = edF32Vector4DotProductHard(&translation, &eStack32);
	if (fVar15 < 0.0) {
		this->dynamic.flags = this->dynamic.flags | 1;
	}

	if ((this->pTiedActor != (CActor*)0x0) ||
		(((uVar2 & 4) != 0 && ((pCollision->flags_0x4 & 2) == 0)))) {
		this->dynamic.flags = this->dynamic.flags | 4;
	}

	ComputeRealMoving(&local_40, fVar15);

	fVar15 = 0.05f;
	if (0.05f < this->dynamic.field_0x44) {
		pTVar5 = GetTimer();
		fVar15 = pTVar5->scaledTotalTime;
		this->dynamicExt.scaledTotalTime = fVar15;
	}

	if (this->field_0x2cc != 0) {
		IMPLEMENTATION_GUARD(
		fVar15 = (float)(*(code*)(this->base.pVTable)->field_0x110)(this);)
	}
	return fVar15;
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
			this->field_0x2cc = 0;
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
			this->field_0x2cc = 1;
		}
		pCollision->InvalidatePrims();
	}
	return;
}
