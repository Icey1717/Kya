#include "ActorFighter.h"
#include "MathOps.h"

bool CActorFighter::IsKindOfObject(ulong kind)
{
	return (kind & 0xf) != 0;
}

void CActorFighter::Init()
{
	CAnimation* pCVar1;
	int iVar2;
	float fVar3;

	IMPLEMENTATION_GUARD_LOG(
	S_STREAM_REF<CActor>::Init(this->pWeaponActor);)
	CActorAutonomous::Init();
	this->flags = this->flags | 0x100000;

	IMPLEMENTATION_GUARD_LOG(
	_InitBlowsDB(this);
	FUN_00302b20(this);
	fVar3 = (float)_InitGrabsDB(this);
	pCVar1 = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim((CActor*)this, New_Name_(41));
	if (iVar2 < 0) {
		fVar3 = 0.0;
		this->field_0x6c0 = 0.0;
	}
	else {
		CAnimation::GetAnimLength(pCVar1, iVar2, 1);
		this->field_0x6c0 = fVar3;
	}
	pCVar1 = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim((CActor*)this, New_Name_(42));
	if (iVar2 < 0) {
		fVar3 = 0.0;
	}
	else {
		CAnimation::GetAnimLength(pCVar1, iVar2, 1);
	}
	fVar3 = this->field_0x6c0 + fVar3;
	this->field_0x6c0 = fVar3;
	pCVar1 = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim((CActor*)this, New_Name_(43));
	if (iVar2 < 0) {
		fVar3 = 0.0;
	}
	else {
		CAnimation::GetAnimLength(pCVar1, iVar2, 1);
	}

	this->field_0x6c0 = this->field_0x6c0 + fVar3;)
	return;
}

CBehaviour* CActorFighter::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == 6) {
		pNewBehaviour = new CBehaviourFighterSlave;
	}
	else {
		if (behaviourType == 5) {
			pNewBehaviour = new CBehaviourFighterRidden;
		}
		else {
			if (behaviourType == 4) {
				pNewBehaviour = new CBehaviourFighterProjected;
			}
			else {
				if (behaviourType == 3) {
					pNewBehaviour = new CBehaviourFighter;
				}
				else {
					pNewBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
				}
			}
		}
	}
	return pNewBehaviour;
}

int INT_00449738 = 0;

void CActorFighter::Create(ByteCode* pByteCode)
{
	edColPRIM_OBJECT* peVar1;
	uint uVar2;
	undefined4* puVar3;
	uint uVar4;
	int iVar5;
	undefined** ppuVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	INT_00449738 = INT_00449738 + 1;
	ClearLocalData();

	IMPLEMENTATION_GUARD_LOG(
	this->field_0x358 = 5.5;
	this->field_0x35c = 2.5;
	this->field_0x360 = 5.0;
	this->field_0x364 = 2.0;
	this->field_0x488 = 0xd;
	this->field_0x484 = this->field_0x484 & 0xf0;
	this->field_0x484 = this->field_0x484 & 0xf | 0x10;
	this->field_0x485 = this->field_0x485 & 0xf0 | 2;
	this->field_0x485 = this->field_0x485 & 0xf;
	this->field_0x486 = this->field_0x486 & 0xf0 | 1;
	this->field_0x486 = this->field_0x486 & 0xf | 0x30;
	this->field_0x487 = this->field_0x487 & 0xf0;
	this->field_0x487 = this->field_0x487 & 0xf | 0x10;
	this->field_0x490 = 0x11;
	this->field_0x48c = this->field_0x48c & 0xf0;
	this->field_0x48c = this->field_0x48c & 0xf | 0x10;
	this->field_0x48d = this->field_0x48d & 0xf0 | 2;
	this->field_0x48d = this->field_0x48d & 0xf;
	this->field_0x48e = this->field_0x48e & 0xf0 | 1;
	this->field_0x48e = this->field_0x48e & 0xf | 0x30;
	this->field_0x48f = this->field_0x48f & 0xf0;
	this->field_0x48f = this->field_0x48f & 0xf | 0x10;
	this->field_0x6d4 = 0x60;
	this->field_0x6d0 = this->field_0x6d0 & 0xf0;
	this->field_0x6d1 = this->field_0x6d1 & 0xf0 | 4;
	this->field_0x6d2 = this->field_0x6d2 & 0xf0 | 3;
	this->field_0x6d0 = this->field_0x6d0 & 0xf | 0x10;
	this->field_0x6d1 = this->field_0x6d1 & 0xf | 0x50;
	this->field_0x6d2 = this->field_0x6d2 & 0xf | 0x20;
	this->field_0x6dc = 0x66;
	this->field_0x6d8 = this->field_0x6d8 & 0xf0;
	this->field_0x6d9 = this->field_0x6d9 & 0xf0 | 2;
	this->field_0x6da = this->field_0x6da & 0xf0 | 3;
	this->field_0x6d8 = this->field_0x6d8 & 0xf | 0x10;
	this->field_0x6d9 = this->field_0x6d9 & 0xf | 0x20;
	this->field_0x6da = this->field_0x6da & 0xf | 0x20;
	this->field_0x6e4 = 0x66;
	this->field_0x6e0 = this->field_0x6e0 & 0xf0;
	this->field_0x6e1 = this->field_0x6e1 & 0xf0 | 2;
	this->field_0x6e2 = this->field_0x6e2 & 0xf0 | 3;
	this->field_0x6e0 = this->field_0x6e0 & 0xf | 0x10;
	this->field_0x6e1 = this->field_0x6e1 & 0xf | 0x20;
	this->field_0x6e2 = this->field_0x6e2 & 0xf | 0x20;
	this->field_0x6ec = 0x47;
	this->field_0x6e8 = this->field_0x6e8 & 0xf0;
	this->field_0x6e9 = this->field_0x6e9 & 0xf0 | 2;
	this->field_0x6ea = this->field_0x6ea & 0xf0 | 2;
	this->field_0x6e8 = this->field_0x6e8 & 0xf | 0x10;
	this->field_0x6e9 = this->field_0x6e9 & 0xf | 0x20;
	this->field_0x6ea = this->field_0x6ea & 0xf | 0x20;
	this->field_0x6f4 = 0x4a;
	this->field_0x6f0 = this->field_0x6f0 & 0xf0;
	this->field_0x6f1 = this->field_0x6f1 & 0xf0 | 2;
	this->field_0x6f2 = this->field_0x6f2 & 0xf0 | 2;
	this->field_0x6f0 = this->field_0x6f0 & 0xf | 0x10;
	this->field_0x6f1 = this->field_0x6f1 & 0xf | 0x20;
	this->field_0x6f2 = this->field_0x6f2 & 0xf | 0x20;
	this->field_0x6fc = 0x4a;
	this->field_0x6f8 = this->field_0x6f8 & 0xf0;
	this->field_0x6f9 = this->field_0x6f9 & 0xf0 | 2;
	this->field_0x6fa = this->field_0x6fa & 0xf0 | 2;
	this->field_0x6f8 = this->field_0x6f8 & 0xf | 0x10;
	this->field_0x6f9 = this->field_0x6f9 & 0xf | 0x20;
	this->field_0x6fa = this->field_0x6fa & 0xf | 0x20;
	this->field_0x704 = 0x55;
	this->field_0x700 = this->field_0x700 & 0xf0;
	this->field_0x701 = this->field_0x701 & 0xf0 | 2;
	this->field_0x702 = this->field_0x702 & 0xf0 | 2;
	this->field_0x700 = this->field_0x700 & 0xf | 0x10;
	this->field_0x701 = this->field_0x701 & 0xf | 0x20;
	this->field_0x702 = this->field_0x702 & 0xf | 0x20;
	this->field_0x70c = 0x58;
	this->field_0x708 = this->field_0x708 & 0xf0;
	this->field_0x709 = this->field_0x709 & 0xf0 | 2;
	this->field_0x70a = this->field_0x70a & 0xf0 | 2;
	this->field_0x708 = this->field_0x708 & 0xf | 0x10;
	this->field_0x709 = this->field_0x709 & 0xf | 0x20;
	this->field_0x70a = this->field_0x70a & 0xf | 0x20;
	this->field_0x714 = 0x58;
	this->field_0x710 = this->field_0x710 & 0xf0;
	this->field_0x711 = this->field_0x711 & 0xf0 | 2;
	this->field_0x712 = this->field_0x712 & 0xf0 | 2;
	this->field_0x710 = this->field_0x710 & 0xf | 0x10;
	this->field_0x711 = this->field_0x711 & 0xf | 0x20;
	this->field_0x712 = this->field_0x712 & 0xf | 0x20;
	this->field_0x71c = 0x21;
	this->field_0x718 = this->field_0x718 & 0xf0;
	this->field_0x719 = this->field_0x719 & 0xf0 | 2;
	this->field_0x71a = this->field_0x71a & 0xf0 | 2;
	this->field_0x718 = this->field_0x718 & 0xf | 0x10;
	this->field_0x719 = this->field_0x719 & 0xf | 0x20;
	this->field_0x71a = this->field_0x71a & 0xf | 0x20;
	this->field_0x724 = 0x24;
	this->field_0x720 = this->field_0x720 & 0xf0;
	this->field_0x721 = this->field_0x721 & 0xf0 | 2;
	this->field_0x722 = this->field_0x722 & 0xf0 | 2;
	this->field_0x720 = this->field_0x720 & 0xf | 0x10;
	this->field_0x721 = this->field_0x721 & 0xf | 0x20;
	this->field_0x722 = this->field_0x722 & 0xf | 0x20;
	this->field_0x72c = 0x24;
	this->field_0x728 = this->field_0x728 & 0xf0;
	this->field_0x729 = this->field_0x729 & 0xf0 | 2;
	this->field_0x72a = this->field_0x72a & 0xf0 | 2;
	this->field_0x728 = this->field_0x728 & 0xf | 0x10;
	this->field_0x729 = this->field_0x729 & 0xf | 0x20;
	this->field_0x72a = this->field_0x72a & 0xf | 0x20;
	this->field_0x734 = 0x6a;
	this->field_0x730 = this->field_0x730 & 0xf0;
	this->field_0x731 = this->field_0x731 & 0xf0 | 3;
	this->field_0x732 = this->field_0x732 & 0xf0 | 5;
	this->field_0x730 = this->field_0x730 & 0xf | 0x10;
	this->field_0x731 = this->field_0x731 & 0xf | 0x20;
	this->field_0x732 = this->field_0x732 & 0xf | 0x40;)
	CActorAutonomous::Create(pByteCode);

	IMPLEMENTATION_GUARD_LOG(
	uVar2 = ByteCode::GetU32(pByteCode);
	if (uVar2 == 0) {
		this->field_0x470 = (undefined4*)0x0;
	}
	else {
		puVar3 = (undefined4*)operator.new(0x40);
		if (puVar3 != (undefined4*)0x0) {
			FUN_00337a10(puVar3);
		}
		this->field_0x470 = puVar3;
	}
	uVar2 = ByteCode::GetU32(pByteCode);
	this->field_0x444 = 0xffffffff;
	*(undefined4*)&this->field_0x448 = 0xffffffff;
	this->field_0x440 = 0x3f800000;
	if ((uVar2 & 1) == 0) {
		this->field_0x448 =
			this->field_0x448 & 0xf | (byte)(((uint)(((ulong)this->field_0x448 << 0x38) >> 0x3c) & 0xb) << 4);
	}
	if ((uVar2 & 2) == 0) {
		this->field_0x448 = this->field_0x448 & 0xf | (byte)(((uint)(((ulong)this->field_0x448 << 0x38) >> 0x3c) & 7) << 4);
	}
	if ((uVar2 & 4) == 0) {
		this->field_0x449 = this->field_0x449 & 0xfd;
	}
	if ((uVar2 & 0x10) == 0) {
		this->field_0x449 = this->field_0x449 & 0xcf | (byte)(((uint)(((ulong)this->field_0x449 << 0x3a) >> 0x3e) & 2) << 4)
			;
	}
	if ((uVar2 & 0x20) == 0) {
		this->field_0x449 = this->field_0x449 & 0xcf | (byte)(((uint)(((ulong)this->field_0x449 << 0x3a) >> 0x3e) & 1) << 4)
			;
	}
	if ((uVar2 & 8) == 0) {
		this->field_0x448 =
			this->field_0x448 & 0xf | (byte)(((uint)(((ulong)this->field_0x448 << 0x38) >> 0x3c) & 0xd) << 4);
	}
	if ((uVar2 & 0x40) == 0) {
		this->field_0x444 = this->field_0x444 & 0xfffffffe;
	}
	if ((uVar2 & 0x80) == 0) {
		this->field_0x444 = this->field_0x444 & 0xfffffffd;
	}
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x7c0 = fVar7;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x7c4 = fVar7;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x7d4 = fVar7;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x7cc = fVar7;
	peVar1 = ((this->characterBase).base.base.pCollisionData)->pObbPrim;
	fVar9 = (peVar1->field_0xb0).y;
	fVar7 = (peVar1->field_0xb0).z;
	fVar8 = (peVar1->field_0xb0).w;
	(this->field_0x410).x = (peVar1->field_0xb0).x;
	(this->field_0x410).y = fVar9;
	(this->field_0x410).z = fVar7;
	(this->field_0x410).w = fVar8;
	fVar9 = (this->field_0x410).y;
	fVar7 = (this->field_0x410).z;
	fVar8 = (this->field_0x410).w;
	(this->field_0x400).x = (this->field_0x410).x;
	(this->field_0x400).y = fVar9;
	(this->field_0x400).z = fVar7;
	(this->field_0x400).w = fVar8;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x420 = fVar7 * 2.0;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x428 = fVar7 * 2.0;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x424 = fVar7 * 2.0;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x42c = fVar7 * 2.0;
	this->field_0x3f8 = (((this->characterBase).base.base.subObjA)->boundingSphere).w;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x810 = fVar7;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x814 = fVar7;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x818 = fVar7;
	this->field_0x81c = 1.0;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x820 = fVar7;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x824 = fVar7;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x828 = fVar7;
	this->field_0x82c = 1.0;
	uVar4 = ByteCode::GetU32(pByteCode);
	if (uVar4 == 0) {
		this->field_0x800 = 1.0;
	}
	else {
		this->field_0x800 = -1.0f;
	}
	uVar4 = ByteCode::GetU32(pByteCode);
	if (uVar4 == 0) {
		this->field_0x368 = (this->field_0x368 & 0xffffffef);
	}
	else {
		this->field_0x368 = (this->field_0x368 | 0x10);
	}
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x50c = fVar7;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x510 = fVar7;
	fVar7 = ByteCode::GetF32(pByteCode);
	this->field_0x514 = fVar7;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x518 = uVar4;
	ByteCode::GetU32(pByteCode);
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x51c = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x520 = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x524 = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x528 = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x52c = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x530 = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x534 = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x538 = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x53c = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x540 = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x544 = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x548 = uVar4;
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x54c = uVar4;
	FUN_004071f0((long)(int)&this->field_0x570, pByteCode);
	uVar4 = ByteCode::GetU32(pByteCode);
	this->field_0x5f0 = uVar4;
	if ((uVar2 & 0x20) == 0) {
		FUN_00407160(&this->field_0x570, -1, -1);
	}
	(this->field_0x600).x = 0.0;
	(this->field_0x600).y = 0.85;
	(this->field_0x600).z = 0.4;
	uVar2 = 0;
	ppuVar6 = &PTR_ParticleVTable_0048f900;
	(this->field_0x600).w = 0.0;
	do {
		if (((uint)ppuVar6[0xb] & 1) == 0) {
			FUN_00401470((int)ppuVar6, 4);
		}
		uVar2 = uVar2 + 1;
		ppuVar6 = ppuVar6 + 0x12;
	} while (uVar2 < 4);
	FUN_00407690(&this->field_0x550, this->field_0x52c);
	FUN_00407690(&this->field_0x55c, this->field_0x52c);
	_CreateBlowsDB(this, pByteCode);
	_CreateGrabsDB(this, pByteCode);
	_CreateCombosDB(this, pByteCode);
	iVar5 = ByteCode::GetS32(pByteCode);
	this->pWeaponActor = iVar5;)
	return;
}

bool CActorFighter::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	uint uVar1;
	int iVar2;

	//iVar2 = &this->field_0x880;
	CActorAutonomous::CarriedByActor(pActor, m0);

	IMPLEMENTATION_GUARD_LOG(
	uVar1 = 0;
	if (this->field_0x8e0 != 0) {
		do {
			edF32Matrix4MulF32Vector4Hard((edF32VECTOR4*)(iVar2 + 0x20), m0, (edF32VECTOR4*)(iVar2 + 0x20));
			uVar1 = uVar1 + 1;
			iVar2 = iVar2 + 0x30;
		} while (uVar1 < (uint)this->field_0x8e0);
	}

	if ((this->field_0x368 & 0x1000) != 0) {
		edF32Matrix4MulF32Vector4Hard((edF32VECTOR4*)&this->field_0x4a0, m0, (edF32VECTOR4*)&this->field_0x4a0);
		edF32Matrix4MulF32Vector4Hard((edF32VECTOR4*)&this->field_0x4b0, m0, (edF32VECTOR4*)&this->field_0x4b0);
	}

	if (this->actorState - 0x52U < 2) {
		edF32Matrix4MulF32Vector4Hard((edF32VECTOR4*)&this->field_0x690, m0, (edF32VECTOR4*)&this->field_0x690);
	})

	return true;
}

int CActorFighter::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	AnimResult* pAVar2;
	CBehaviour* pCVar3;
	CLifeInterface* pCVar4;
	int iVar5;
	uint uVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	if (msg == 0x12) {
		IMPLEMENTATION_GUARD(
		iVar5 = (this->characterBase).base.base.actorState;
		uVar6 = 0;
		if (iVar5 != -1) {
			pAVar2 = (*((this->characterBase).base.base.pVTable)->GetStateCfg)((CActor*)this, iVar5);
			uVar6 = pAVar2->flags_0x4;
		}
		if ((uVar6 & 0x2000000) == 0) {
			pCVar4 = (*((this->characterBase).base.base.pVTable)->GetLifeInterface)((CActor*)this);
			fVar7 = (float)(*(code*)pCVar4->pVtable->GetValue)();
			if (((0.0 < fVar7) &&
				(fVar7 = (pSender->currentLocation).x - (this->characterBase).base.base.currentLocation.x,
					fVar8 = (pSender->currentLocation).z - (this->characterBase).base.base.currentLocation.z,
					fVar7 * fVar7 + fVar8 * fVar8 <= 1.0)) &&
				(ABS((pSender->currentLocation).y - (this->characterBase).base.base.currentLocation.y) <= 0.5)) {
				fVar9 = (this->characterBase).base.base.rotationQuat.y;
				fVar7 = (this->characterBase).base.base.rotationQuat.z;
				fVar8 = (this->characterBase).base.base.rotationQuat.w;
				*(float*)((int)pMsgParam + 0x10) = (this->characterBase).base.base.rotationQuat.x;
				*(float*)((int)pMsgParam + 0x14) = fVar9;
				*(float*)((int)pMsgParam + 0x18) = fVar7;
				*(float*)((int)pMsgParam + 0x1c) = fVar8;
				fVar9 = (this->characterBase).base.base.currentLocation.y;
				fVar7 = (this->characterBase).base.base.currentLocation.z;
				fVar8 = (this->characterBase).base.base.currentLocation.w;
				*(float*)pMsgParam = (this->characterBase).base.base.currentLocation.x;
				*(float*)((int)pMsgParam + 4) = fVar9;
				*(float*)((int)pMsgParam + 8) = fVar7;
				*(float*)((int)pMsgParam + 0xc) = fVar8;
				return 10;
			}
		}
		iVar5 = 0;)
	}
	else {
		if (msg == 0x27) {
			IMPLEMENTATION_GUARD(
			bVar1 = (*((this->characterBase).base.base.pVTable)->IsFightRelated)
				((CActor*)this, (this->characterBase).base.base.curBehaviourId);
			if (bVar1 == false) {
				(*(code*)((this->characterBase).base.base.pVTable)->SetFightBehaviour)(this);
				pCVar3 = CActor::GetBehaviour((CActor*)this, (this->characterBase).base.base.curBehaviourId);
				iVar5 = (*pCVar3->pVTable->InterpretMessage)(pCVar3, pSender, 0x27, (int)pMsgParam);
			}
			else {
				iVar5 = 0;
			})
		}
		else {
			if ((msg == 0x66) || (msg == 0x65)) {
				IMPLEMENTATION_GUARD(
				iVar5 = (this->characterBase).base.base.actorState;
				uVar6 = 0;
				if (iVar5 != -1) {
					pAVar2 = (*((this->characterBase).base.base.pVTable)->GetStateCfg)((CActor*)this, iVar5);
					uVar6 = pAVar2->flags_0x4;
				}
				if (((uVar6 & 0x2000000) == 0) &&
					(bVar1 = (*((this->characterBase).base.base.pVTable)->IsFightRelated)
						((CActor*)this, (this->characterBase).base.base.curBehaviourId), bVar1 == false)) {
					(*(code*)((this->characterBase).base.base.pVTable)->SetFightBehaviour)(this);
					pCVar3 = CActor::GetBehaviour((CActor*)this, (this->characterBase).base.base.curBehaviourId);
					iVar5 = (*pCVar3->pVTable->InterpretMessage)(pCVar3, pSender, msg, (int)pMsgParam);
				}
				else {
					iVar5 = 0;
				})
			}
			else {
				if (msg == 2) {
					IMPLEMENTATION_GUARD(
					iVar5 = (this->characterBase).base.base.actorState;
					uVar6 = 0;
					if (iVar5 != -1) {
						pAVar2 = (*((this->characterBase).base.base.pVTable)->GetStateCfg)((CActor*)this, iVar5);
						uVar6 = pAVar2->flags_0x4;
					}
					if (((uVar6 & 0x2000000) != 0) ||
						(bVar1 = (*((this->characterBase).base.base.pVTable)->IsFightRelated)
							((CActor*)this, (this->characterBase).base.base.curBehaviourId), bVar1 != false)) {
						return 0;
					}
					/* WARNING: Load size is inaccurate */
					iVar5 = *pMsgParam;
					if (((iVar5 == 7) || (iVar5 == 8)) || (iVar5 == 10)) {
						(*(code*)((this->characterBase).base.base.pVTable)->SetFightBehaviour)(this);
						pCVar3 = CActor::GetBehaviour((CActor*)this, (this->characterBase).base.base.curBehaviourId);
						iVar5 = (*pCVar3->pVTable->InterpretMessage)(pCVar3, pSender, 2, (int)pMsgParam);
						return iVar5;
					})
				}

				iVar5 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
			}
		}
	}
	return iVar5;
}

bool CActorFighter::IsFightRelated(int param_2)
{
	bool bVar1;

	if ((param_2 - 3U < 3) || (param_2 == 6)) {
		bVar1 = true;
	}
	else {
		bVar1 = false;
	}
	return bVar1;
}

void CActorFighter::ClearLocalData()
{
	float fVar1;
	float fVar2;
	float fVar3;

	IMPLEMENTATION_GUARD_LOG(

		this->field_0x368 = (this->field_0x368 | 2);
	this->field_0x368 = (this->field_0x368 | 4);
	this->field_0x368 = (this->field_0x368 & 0xfffffffe);
	this->field_0x368 = (this->field_0x368 & 0xffffffdf);
	this->field_0x368 = (this->field_0x368 & 0xffffffbf);
	this->field_0x368 = (this->field_0x368 & 0xffffff7f);
	this->field_0x368 = (this->field_0x368 & 0xfffffeff);
	this->field_0x368 = (this->field_0x368 & 0xfffffdff);
	this->field_0x368 = (this->field_0x368 & 0xfffffbff);
	this->field_0x368 = (this->field_0x368 & 0xffffefff);
	this->field_0x368 = (this->field_0x368 & 0xffffdfff);
	this->field_0x368 = (this->field_0x368 & 0xffffbfff);
	this->field_0x368 = (this->field_0x368 & 0xffff7fff);
	this->field_0x368 = (this->field_0x368 & 0xfffeffff);
	this->field_0x350 = 0;
	this->field_0x354 = 0;
	this->field_0x36c = 1;
	this->field_0x370 = 0;
	this->field_0x374 = 0;
	this->field_0x378 = 0;);

	this->field_0x37c.Reset();
	this->field_0x3a4.Reset();
	this->scalarDynJump.Reset();

	IMPLEMENTATION_GUARD_LOG(
	this->field_0x3f4 = 1.0;
	this->field_0x478 = 0;
	fVar3 = gF32Vertex4Zero.w;
	fVar2 = gF32Vertex4Zero.z;
	fVar1 = gF32Vertex4Zero.y;
	this->field_0x4a0 = gF32Vertex4Zero.x;
	this->field_0x4a4 = fVar1;
	this->field_0x4a8 = fVar2;
	this->field_0x4ac = fVar3;
	fVar3 = gF32Vertex4Zero.w;
	fVar2 = gF32Vertex4Zero.z;
	fVar1 = gF32Vertex4Zero.y;
	this->field_0x4b0 = gF32Vertex4Zero.x;
	this->field_0x4b4 = fVar1;
	this->field_0x4b8 = fVar2;
	this->field_0x4bc = fVar3;
	fVar3 = gF32Vector4Zero.w;
	fVar2 = gF32Vector4Zero.z;
	fVar1 = gF32Vector4Zero.y;
	this->field_0x4e0 = gF32Vector4Zero.x;
	this->field_0x4e4 = fVar1;
	this->field_0x4e8 = fVar2;
	this->field_0x4ec = fVar3;
	this->field_0x44c = 0;
	this->field_0x450 = 0;
	this->field_0x474 = 0;
	this->field_0x634 = 0;
	this->field_0x738 = 1;
	fVar3 = g_xVector.w;
	fVar2 = g_xVector.z;
	fVar1 = g_xVector.y;
	this->field_0x6a0 = g_xVector.x;
	this->field_0x6a4 = fVar1;
	this->field_0x6a8 = fVar2;
	this->field_0x6ac = fVar3;
	fVar3 = gF32Vertex4Zero.w;
	fVar2 = gF32Vertex4Zero.z;
	fVar1 = gF32Vertex4Zero.y;
	this->field_0x690 = gF32Vertex4Zero.x;
	this->field_0x694 = fVar1;
	this->field_0x698 = fVar2;
	this->field_0x69c = fVar3;
	this->field_0x6b0 = 0;
	this->field_0x6b4 = 0;
	this->field_0x684 = 0;
	this->field_0x7d8 = 0x3db851ec;
	this->field_0x6c4 = 0x3dcccccd;
	this->field_0x6c8 = 0x3f800000;
	this->field_0x6cc = 0x3f800000;
	this->field_0x7e4 = 0;
	fVar3 = gF32Vector4UnitZ.w;
	fVar2 = gF32Vector4UnitZ.z;
	fVar1 = gF32Vector4UnitZ.y;
	this->field_0x7f0 = gF32Vector4UnitZ.x;
	this->field_0x7f4 = fVar1;
	this->field_0x7f8 = fVar2;
	this->field_0x7fc = fVar3;
	fVar3 = (this->field_0x410).y;
	fVar1 = (this->field_0x410).z;
	fVar2 = (this->field_0x410).w;
	(this->field_0x400).x = (this->field_0x410).x;
	(this->field_0x400).y = fVar3;
	(this->field_0x400).z = fVar1;
	(this->field_0x400).w = fVar2;
	this->field_0x830 = 0;
	this->field_0x840 = 0;
	this->field_0x834 = 0;
	this->field_0x860 = 0;
	this->field_0x864 = 0;
	this->field_0x868 = 0xffffffff;
	this->field_0x86c = 0;
	this->field_0x630 = 0;
	this->field_0x8e0 = 0;
	this[1].characterBase.base.base.field_0x13c = 0;
	this[1].characterBase.base.base.aActorSounds = (CActorSound*)0x0;
	*(undefined4*)&this[1].characterBase.base.base.field_0x144 = 0;
	this[1].characterBase.base.base.field_0x140 = 0;
	this[1].characterBase.base.base.field_0x14c = 0x3f800000;
	this[1].characterBase.base.base.field_0x15c = 0x3dcccccd;
	this[1].characterBase.base.dynamic.rotationQuat.x = 0.1;
	*(undefined2*)&this[1].characterBase.base.dynamic.rotationQuat.y = 3;
	this->field_0x47c = 0x41bc7edd;
	this->field_0x4c0 = 0x3fe00000;
	this->field_0x4c4 = 0x40200000;
	this->field_0x4c8 = 0x3f96cbe4;
	this->field_0x4cc = 0x40800000;
	this->field_0x4f4 = &DAT_3f99999a;
	this->field_0x4f8 = 0x41880000;
	this->field_0x508 = 0x40600000;
	this->field_0x504 = 0x40e00000;
	this->field_0x500 = 0;
	this->field_0x480 = 0xffffffff;)
	return;
}

void CActorFighter::ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	edF32VECTOR4* peVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edColPRIM_OBJECT* iVar1;

	CActorAutonomous::ChangeCollisionSphere(param_1, param_3, param_4);
	iVar1 = this->pCollisionData->pObbPrim;
	this->field_0x410 = iVar1->field_0xb0;
	fVar2 = edF32Vector4GetDistHard(&this->field_0x410);
	peVar1 = &this->field_0x400;
	edF32Vector4NormalizeHard(peVar1, peVar1);
	peVar1 = &this->field_0x400;
	edF32Vector4ScaleHard(fVar2, peVar1, peVar1);
	this->field_0x400.w = 1.0f;
	return;
}

void CActorFighter::UpdateCollisionSphere()
{
	edColPRIM_OBJECT* peVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	CActorAutonomous::UpdateCollisionSphere();
	peVar1 = this->pCollisionData->pObbPrim;

	this->field_0x410 = peVar1->field_0xb0;

	fVar2 = edF32Vector4GetDistHard(&this->field_0x410);
	edF32Vector4NormalizeHard(&this->field_0x400, &this->field_0x400);
	edF32Vector4ScaleHard(fVar2, &this->field_0x400, &this->field_0x400);
	(this->field_0x400).w = 1.0f;

	return;
}

void CBehaviourFighter::Init(CActor* pOwner)
{
	this->pOwner = pOwner;
	this->field_0x8 = 3;
	return;
}
