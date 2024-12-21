#include "ActorFighter.h"
#include "MathOps.h"
#include "TimeController.h"
#include "kya.h"
#include "FileManager3D.h"
#include "InputManager.h"
#include "CollisionRay.h"

#define FIGHTER_LOG(level, format, ...) MY_LOG_CATEGORY("Fighter", level, format, ##__VA_ARGS__)

StateConfig CActorFighter::_gStateCfg_FIG[0x6c] =
{
	StateConfig(0xC, 0x1F00900),
	StateConfig(0xD, 0x900),
	StateConfig(0x11, 0xF00902),
	StateConfig(0x15, 0x1F00900),
	StateConfig(0x16, 0xA00800),
	StateConfig(0x16, 0x800800),
	StateConfig(0x17, 0xA00902),
	StateConfig(0x18, 0x5B00800),
	StateConfig(0x18, 0x5B00800),
	StateConfig(0x19, 0x402000),
	StateConfig(0x1C, 0x2000),
	StateConfig(0x1D, 0x2000),
	StateConfig(0x1A, 0x2000),
	StateConfig(0x1B, 0xA02000),
	StateConfig(0x1E, 0x2000900),
	StateConfig(0x1F, 0x900),
	StateConfig(0x1F, 0x2000900),
	StateConfig(0x20, 0x1F00900),
	StateConfig(0x21, 0x100900),
	StateConfig(0x27, 0x4900800),
	StateConfig(0x28, 0x800902),
	StateConfig(0x27, 0x100800),
	StateConfig(0x29, 0x900),
	StateConfig(0x2A, 0x900),
	StateConfig(0x2B, 0x900),
	StateConfig(0x2C, 0x900),
	StateConfig(0x2D, 0x1F04500),
	StateConfig(0x2E, 0x104502),
	StateConfig(0x2F, 0x4400),
	StateConfig(0x30, 0xC00),
	StateConfig(0x31, 0x304100),
	StateConfig(0x32, 0x4100),
	StateConfig(0x33, 0x4100),
	StateConfig(0x36, 0x204100),
	StateConfig(0x37, 0x4100),
	StateConfig(0x38, 0x1F08100),
	StateConfig(0x38, 0x1F08100),
	StateConfig(0x39, 0x1E08102),
	StateConfig(0x3A, 0x8100),
	StateConfig(0x3B, 0x8100),
	StateConfig(0x3B, 0x8100),
	StateConfig(0x3C, 0x8100),
	StateConfig(0x3D, 0x8100),
	StateConfig(0x3E, 0x8100),
	StateConfig(0x3E, 0x8100),
	StateConfig(0x3F, 0x8100),
	StateConfig(0x40, 0x2008102),
	StateConfig(0x41, 0x902),
	StateConfig(0x42, 0x900),
	StateConfig(0x43, 0x900),
	StateConfig(0x47, 0x2008100),
	StateConfig(0x45, 0x8100),
	StateConfig(0x46, 0x2008100),
	StateConfig(0x4D, 0x1F10102),
	StateConfig(0x4D, 0x10102),
	StateConfig(0x4E, 0x10100),
	StateConfig(0x4F, 0x120000),
	StateConfig(0x50, 0x120000),
	StateConfig(0x50, 0x120000),
	StateConfig(0x51, 0x120000),
	StateConfig(0x52, 0x120000),
	StateConfig(0x58, 0x20000),
	StateConfig(0x53, 0x110100),
	StateConfig(0x53, 0x110100),
	StateConfig(0x54, 0x110100),
	StateConfig(0x5B, 0x10102),
	StateConfig(0x55, 0x10102),
	StateConfig(0xFFFFFFFF, 0x10100),
	StateConfig(0xFFFFFFFF, 0x810100),
	StateConfig(0xFFFFFFFF, 0x10100),
	StateConfig(0xFFFFFFFF, 0x40100),
	StateConfig(0xFFFFFFFF, 0x40100),
	StateConfig(0xFFFFFFFF, 0x40100),
	StateConfig(0x60, 0x900),
	StateConfig(0x5C, 0x100900),
	StateConfig(0x5D, 0x100900),
	StateConfig(0x5E, 0x900),
	StateConfig(0x5F, 0x100900),
	StateConfig(0x60, 0x4001100),
	StateConfig(0x70, 0x4101000),
	StateConfig(0x70, 0x4101000),
	StateConfig(0x73, 0x6001100),
	StateConfig(0x75, 0x2001100),
	StateConfig(0x77, 0x2001100),
	StateConfig(0x7B, 0x1100),
	StateConfig(0x7B, 0x1100),
	StateConfig(0x7C, 0x1100),
	StateConfig(0x7D, 0x1100),
	StateConfig(0x7E, 0x2001100),
	StateConfig(0x34, 0x2001100),
	StateConfig(0x35, 0x2001100),
	StateConfig(0xFFFFFFFF, 0x2101000),
	StateConfig(0x80, 0x2101101),
	StateConfig(0xFFFFFFFF, 0x2001101),
	StateConfig(0x60, 0x4001100),
	StateConfig(0xFFFFFFFF, 0x1800900),
	StateConfig(0xFFFFFFFF, 0x800900),
	StateConfig(0xFFFFFFFF, 0x800900),
	StateConfig(0xFFFFFFFF, 0x1000900),
	StateConfig(0xFFFFFFFF, 0x900),
	StateConfig(0xFFFFFFFF, 0x900),
	StateConfig(0xFFFFFFFF, 0x900),
	StateConfig(0xFFFFFFFF, 0x900),
	StateConfig(0xFFFFFFFF, 0x2000900),
	StateConfig(0xFFFFFFFF, 0x2800900),
	StateConfig(0xFFFFFFFF, 0x900),
	StateConfig(0xFFFFFFFF, 0x2080100),
	StateConfig(0xFFFFFFFF, 0x2080100),
};

bool CActorFighter::IsKindOfObject(ulong kind)
{
	return (kind & 0xf) != 0;
}

void CActorFighter::Init()
{
	CAnimation* pCVar1;
	int iVar2;
	float fVar3;

	this->pWeaponActor.Init();
	CActorAutonomous::Init();
	this->flags = this->flags | 0x100000;

	IMPLEMENTATION_GUARD_LOG(
	_InitBlowsDB(this);
	FUN_00302b20(this);
	fVar3 = (float)_InitGrabsDB(this);
	pCVar1 = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim(this, New_Name_(41));
	if (iVar2 < 0) {
		fVar3 = 0.0;
		this->field_0x6c0 = 0.0;
	}
	else {
		CAnimation::GetAnimLength(pCVar1, iVar2, 1);
		this->field_0x6c0 = fVar3;
	}
	pCVar1 = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim(this, New_Name_(42));
	if (iVar2 < 0) {
		fVar3 = 0.0;
	}
	else {
		CAnimation::GetAnimLength(pCVar1, iVar2, 1);
	}
	fVar3 = this->field_0x6c0 + fVar3;
	this->field_0x6c0 = fVar3;
	pCVar1 = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim(this, New_Name_(43));
	if (iVar2 < 0) {
		fVar3 = 0.0;
	}
	else {
		CAnimation::GetAnimLength(pCVar1, iVar2, 1);
	}

	this->field_0x6c0 = this->field_0x6c0 + fVar3;)
	return;
}

void CActorFighter::Term()
{
	undefined** ppuVar1;
	uint uVar2;

	CActorAutonomous::Term();

	IMPLEMENTATION_GUARD(
	uVar2 = 0;
	ppuVar1 = &PTR_ParticleVTable_0048f900;
	do {
		if (((uint)ppuVar1[0xb] & 1) != 0) {
			FUN_004013c0((int)ppuVar1);
		}
		uVar2 = uVar2 + 1;
		ppuVar1 = ppuVar1 + 0x12;
	} while (uVar2 < 4);
	_DeleteCombosDB((int)this);
	_DeleteGrabsDB((int)this);
	_DeleteBlowsDB((int)this);
	if (this->field_0x470 != (FighterSubObj_40*)0x0) {
		Free(this->field_0x470);
		this->field_0x470 = (FighterSubObj_40*)0x0;
	})

	return;
}

void CActorFighter::Reset()
{
	ClearLocalData();

	CActorAutonomous::Reset();

	this->flags = this->flags | 0x100000;
	if ((this->flags & 0x1000) != 0) {
		Compute2DOrientationFromAngles();
		this->flags = this->flags & 0xffffefff;
	}

	if (this->field_0x470 != (FighterSubObj_40*)0x0) {
		*this->field_0x470 = FighterSubObj_40();
	}

	return;
}

CBehaviour* CActorFighter::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == FIGHTER_BEHAVIOUR_SLAVE) {
		pNewBehaviour = new CBehaviourFighterSlave;
	}
	else {
		if (behaviourType == FIGHTER_BEHAVIOUR_RIDDEN) {
			pNewBehaviour = new CBehaviourFighterRidden;
		}
		else {
			if (behaviourType == FIGHTER_BEHAVIOUR_PROJECTED) {
				pNewBehaviour = new CBehaviourFighterProjected;
			}
			else {
				if (behaviourType == FIGHTER_BEHAVIOUR_DEFAULT) {
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

StateConfig* CActorFighter::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		assert((state - 6) < 0x6c);
		pStateConfig = _gStateCfg_FIG + state + -6;
	}

	return pStateConfig;
}

static uint _gBehaviourFlags[7] =
{
	0x3DCCCCCD,
	0x0,
	0x0,
	0x1e,
	0xf,
	0x0,
	0x0,
};

uint CActorFighter::GetBehaviourFlags(int state)
{
	uint flags;

	if (state < 3) {
		flags = CActorAutonomous::GetBehaviourFlags(state);
	}
	else {
		flags = _gBehaviourFlags[state];
	}
	return flags;
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
	this->field_0x72a = this->field_0x72a & 0xf | 0x20;)

	(this->field_0x730).field_0x4 = 0x6a;
	(this->field_0x730).field_0x0[0] = (this->field_0x730).field_0x0[0] & 0xf0;
	(this->field_0x730).field_0x0[1] = (this->field_0x730).field_0x0[1] & 0xf0 | 3;
	(this->field_0x730).field_0x0[2] = (this->field_0x730).field_0x0[2] & 0xf0 | 5;
	(this->field_0x730).field_0x0[0] = (this->field_0x730).field_0x0[0] & 0xf | 0x10;
	(this->field_0x730).field_0x0[1] = (this->field_0x730).field_0x0[1] & 0xf | 0x20;
	(this->field_0x730).field_0x0[2] = (this->field_0x730).field_0x0[2] & 0xf | 0x40;

	CActorAutonomous::Create(pByteCode);

	uVar2 = pByteCode->GetU32();
	if (uVar2 == 0) {
		this->field_0x470 = (FighterSubObj_40*)0x0;
	}
	else {
		this->field_0x470 = new FighterSubObj_40;
	}


	uVar2 = pByteCode->GetU32();
	//this->field_0x444 = 0xffffffff;
	//this->field_0x448 = 0xffffffff;
	//this->field_0x440 = 0x3f800000;
	//if ((uVar2 & 1) == 0) {
	//	this->field_0x448 =
	//		this->field_0x448 & 0xf | (byte)(((uint)(((ulong)this->field_0x448 << 0x38) >> 0x3c) & 0xb) << 4);
	//}
	//if ((uVar2 & 2) == 0) {
	//	this->field_0x448 = this->field_0x448 & 0xf | (byte)(((uint)(((ulong)this->field_0x448 << 0x38) >> 0x3c) & 7) << 4);
	//}
	//if ((uVar2 & 4) == 0) {
	//	this->field_0x449 = this->field_0x449 & 0xfd;
	//}
	//if ((uVar2 & 0x10) == 0) {
	//	this->field_0x449 = this->field_0x449 & 0xcf | (byte)(((uint)(((ulong)this->field_0x449 << 0x3a) >> 0x3e) & 2) << 4)
	//		;
	//}
	//if ((uVar2 & 0x20) == 0) {
	//	this->field_0x449 = this->field_0x449 & 0xcf | (byte)(((uint)(((ulong)this->field_0x449 << 0x3a) >> 0x3e) & 1) << 4)
	//		;
	//}
	//if ((uVar2 & 8) == 0) {
	//	this->field_0x448 =
	//		this->field_0x448 & 0xf | (byte)(((uint)(((ulong)this->field_0x448 << 0x38) >> 0x3c) & 0xd) << 4);
	//}
	//if ((uVar2 & 0x40) == 0) {
	//	this->field_0x444 = this->field_0x444 & 0xfffffffe;
	//}
	//if ((uVar2 & 0x80) == 0) {
	//	this->field_0x444 = this->field_0x444 & 0xfffffffd;
	//}
	this->field_0x7c0 = pByteCode->GetF32();
	this->field_0x7c4 = pByteCode->GetF32();
	this->field_0x7d4 = pByteCode->GetF32();
	this->field_0x7cc = pByteCode->GetF32();

	peVar1 = (this->pCollisionData)->pObbPrim;

	this->field_0x410 = peVar1->field_0xb0;
	this->field_0x400 = this->field_0x410;

	this->field_0x420 = pByteCode->GetF32() * 2.0f;
	this->field_0x428 = pByteCode->GetF32() * 2.0f;
	this->field_0x424 = pByteCode->GetF32() * 2.0f;
	this->field_0x42c = pByteCode->GetF32() * 2.0f;

	this->field_0x3f8 = ((this->subObjA)->boundingSphere).w;

	this->field_0x810 = pByteCode->GetF32();
	this->field_0x814 = pByteCode->GetF32();
	this->field_0x818 = pByteCode->GetF32();
	this->field_0x81c = 1.0f;

	this->field_0x820 = pByteCode->GetF32();
	this->field_0x824 = pByteCode->GetF32();
	this->field_0x828 = pByteCode->GetF32();
	this->field_0x82c = 1.0f;

	uVar4 = pByteCode->GetU32();
	if (uVar4 == 0) {
		this->field_0x800 = 1.0f;
	}
	else {
		this->field_0x800 = -1.0f;
	}

	uVar4 = pByteCode->GetU32();
	if (uVar4 == 0) {
		this->fightFlags = (this->fightFlags & 0xffffffef);
	}
	else {
		this->fightFlags = (this->fightFlags | 0x10);
	}

	this->field_0x50c = pByteCode->GetF32();
	this->field_0x510 = pByteCode->GetF32();
	this->field_0x514 = pByteCode->GetF32();
	this->field_0x518 = pByteCode->GetU32();

	pByteCode->GetU32();

	this->field_0x51c = pByteCode->GetU32();
	this->field_0x520 = pByteCode->GetU32();
	this->field_0x524 = pByteCode->GetU32();
	this->field_0x528 = pByteCode->GetU32();
	this->field_0x52c = pByteCode->GetU32();
	this->field_0x530 = pByteCode->GetU32();
	this->field_0x534 = pByteCode->GetU32();
	this->field_0x538 = pByteCode->GetU32();
	this->field_0x53c = pByteCode->GetU32();
	this->field_0x540 = pByteCode->GetU32();
	this->field_0x544 = pByteCode->GetU32();
	this->field_0x548 = pByteCode->GetU32();
	this->field_0x54c = pByteCode->GetU32();

	this->staticMeshComponentAdvanced.Create(pByteCode);
	this->field_0x5f0 = pByteCode->GetU32();
	if ((uVar2 & 0x20) == 0) {
		staticMeshComponentAdvanced.ResetInternal(-1, -1);
	}

	(this->field_0x600).x = 0.0f;
	(this->field_0x600).y = 0.85f;
	(this->field_0x600).z = 0.4f;

	//uVar2 = 0;
	//ppuVar6 = &PTR_ParticleVTable_0048f900;
	//(this->field_0x600).w = 0.0;
	//do {
	//	if (((uint)ppuVar6[0xb] & 1) == 0) {
	//		FUN_00401470((int)ppuVar6, 4);
	//	}
	//	uVar2 = uVar2 + 1;
	//	ppuVar6 = ppuVar6 + 0x12;
	//} while (uVar2 < 4);
	//FUN_00407690(&this->field_0x550, this->field_0x52c);
	//FUN_00407690(&this->field_0x55c, this->field_0x52c);
	_CreateBlowsDB(pByteCode);
	_CreateGrabsDB(pByteCode);
	_CreateCombosDB(pByteCode);

	this->pWeaponActor.index = pByteCode->GetS32();

	return;
}

bool CActorFighter::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	uint uVar1;
	int iVar2;

	//iVar2 = &this->field_0x880;
	CActorAutonomous::CarriedByActor(pActor, m0);

	uVar1 = 0;
	if (this->field_0x8e0 != 0) {
		IMPLEMENTATION_GUARD(
		do {
			edF32Matrix4MulF32Vector4Hard((edF32VECTOR4*)(iVar2 + 0x20), m0, (edF32VECTOR4*)(iVar2 + 0x20));
			uVar1 = uVar1 + 1;
			iVar2 = iVar2 + 0x30;
		} while (uVar1 < (uint)this->field_0x8e0);)
	}

	if ((this->fightFlags & 0x1000) != 0) {
		edF32Matrix4MulF32Vector4Hard(&this->field_0x4a0, m0, &this->field_0x4a0);
		edF32Matrix4MulF32Vector4Hard(&this->field_0x4b0, m0, &this->field_0x4b0);
	}

	if (this->actorState - 0x52U < 2) {
		edF32Matrix4MulF32Vector4Hard(&this->field_0x690, m0, &this->field_0x690);
	}

	return true;
}

int CActorFighter::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	StateConfig* pAVar2;
	CBehaviour* pCVar3;
	CLifeInterface* pCVar4;
	int iVar5;
	uint uVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	if (msg == MESSAGE_GET_ACTION) {
		GetActionMsgParams* pResolvedMsg = reinterpret_cast<GetActionMsgParams*>(pMsgParam);

		iVar5 = this->actorState;
		uVar6 = 0;
		if (iVar5 != -1) {
			uVar6 = GetStateCfg(iVar5)->flags_0x4;
		}

		if ((uVar6 & 0x2000000) == 0) {
			fVar7 = GetLifeInterface()->GetValue();
			if (((0.0f < fVar7) && (fVar7 = (pSender->currentLocation).x - this->currentLocation.x,
					fVar8 = (pSender->currentLocation).z - this->currentLocation.z, fVar7 * fVar7 + fVar8 * fVar8 <= 1.0f)) &&
					(fabs((pSender->currentLocation).y - this->currentLocation.y) <= 0.5f)) {
				pResolvedMsg->field_0x10 = this->rotationQuat;
				pResolvedMsg->field_0x0 = this->currentLocation;
				return 10;
			}
		}

		iVar5 = 0;
	}
	else {
		if (msg == 0x27) {
			IMPLEMENTATION_GUARD(
			bVar1 = (*(this->pVTable)->IsFightRelated)
				(this, this->curBehaviourId);
			if (bVar1 == false) {
				(*(code*)(this->pVTable)->SetFightBehaviour)(this);
				pCVar3 = CActor::GetBehaviour(this, this->curBehaviourId);
				iVar5 = (*pCVar3->pVTable->InterpretMessage)(pCVar3, pSender, 0x27, (int)pMsgParam);
			}
			else {
				iVar5 = 0;
			})
		}
		else {
			if ((msg == 0x66) || (msg == 0x65)) {
				IMPLEMENTATION_GUARD(
				iVar5 = this->actorState;
				uVar6 = 0;
				if (iVar5 != -1) {
					pAVar2 = (*(this->pVTable)->GetStateCfg)(this, iVar5);
					uVar6 = pAVar2->flags_0x4;
				}
				if (((uVar6 & 0x2000000) == 0) &&
					(bVar1 = (*(this->pVTable)->IsFightRelated)
						(this, this->curBehaviourId), bVar1 == false)) {
					(*(code*)(this->pVTable)->SetFightBehaviour)(this);
					pCVar3 = CActor::GetBehaviour(this, this->curBehaviourId);
					iVar5 = (*pCVar3->pVTable->InterpretMessage)(pCVar3, pSender, msg, (int)pMsgParam);
				}
				else {
					iVar5 = 0;
				})
			}
			else {
				if (msg == 2) {
					iVar5 = this->actorState;
					uVar6 = 0;
					if (iVar5 != -1) {
						pAVar2 = GetStateCfg(iVar5);
						uVar6 = pAVar2->flags_0x4;
					}

					if (((uVar6 & 0x2000000) != 0) ||
						(bVar1 = IsFightRelated(this->curBehaviourId), bVar1 != false)) {
						return 0;
					}

					_msg_hit_param* pMsgHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
					iVar5 = pMsgHitParam->projectileType;

					if (((iVar5 == 7) || (iVar5 == 8)) || (iVar5 == 10)) {
						SetFightBehaviour();
						pCVar3 = GetBehaviour(this->curBehaviourId);
						iVar5 = pCVar3->InterpretMessage(pSender, 2, pMsgParam);
						return iVar5;
					}
				}

				iVar5 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
			}
		}
	}
	return iVar5;
}

int CActorFighter::ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam)
{
	int result;

	if ((msg == 2) || (msg == 3)) {
		this->field_0x634 = pSender;
	}

	result = CActor::ReceiveMessage(pSender, msg, pMsgParam);

	return result;
}

void CActorFighter::TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* param_5)
{
	int iVar1;
	CActor* pCVar2;
	StateConfig* pSVar3;
	uint uVar4;

	if (this->curBehaviourId == 3) {
		iVar1 = this->actorState;
		if (iVar1 == -1) {
			uVar4 = 0;
		}
		else {
			pSVar3 = GetStateCfg(iVar1);
			uVar4 = pSVar3->flags_0x4;
		}

		if ((uVar4 & 0xff800) == 0x8000) {
			pCVar2 = this->field_0x354;
			if (pCVar2 != (CActor*)0x0) {
				pCVar2->TieToActor(pTieActor, carryMethod, param_4, param_5);
			}
		}
		else {
			if (((uVar4 & 0xff800) == 0x800) &&
				((iVar1 = this->actorState, iVar1 - 0x14U < 2 || (iVar1 == 0x16)))) {
				pAdversary->TieToActor(pTieActor, carryMethod, param_4, param_5);
			}
		}
	}

	CActor::TieToActor(pTieActor, carryMethod, param_4, param_5);

	return;
}

bool CActorFighter::IsFightRelated(int behaviourId)
{
	bool bFightRelated;

	if ((behaviourId - 3U < 3) || (behaviourId == 6)) {
		bFightRelated = true;
	}
	else {
		bFightRelated = false;
	}

	return bFightRelated;
}

bool CActorFighter::IsAlive()
{
	return 0.0f < GetLifeInterface()->GetValue();
}

void CActorFighter::Func_0x194(float param_1)
{
	CLifeInterface* pCVar1;
	float fVar2;

	if (this->field_0x7cc < param_1) {
		fVar2 = GetLifeInterface()->GetValue();
		LifeDecrease(fVar2);
	}
	return;
}

int CActorFighter::Func_0x198()
{
	return 0;
}

void CActorFighter::_Proj_GetPossibleExit()
{
	int iVar1;

	iVar1 = this->actorState;
	if (iVar1 != -1) {
		GetStateCfg(iVar1);
	}

	return;
}

void CActorFighter::AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	edANM_HDR* peVar1;
	int* piVar2;
	int iVar3;
	int iVar4;
	float* pfVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float puVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 local_c0;
	edF32MATRIX4 eStack176;
	edF32VECTOR4 eStack112;
	edF32MATRIX4 auStack96;
	edANM_HDR* local_c;
	edANM_HDR* local_8;

	if (newAnim == 0x42) {
		IMPLEMENTATION_GUARD(
		peVar1 = pAnimator->pAnimKeyTableEntry;
		if (0.0 <= this->field_0xa5c) {
			(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = 0x3f800000;
			(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = 0;
		}
		else {
			(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = 0;
			(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = 0x3f800000;
		})
	}
	else {
		if ((newAnim == 0x4e) || (newAnim == 0x39)) {
			IMPLEMENTATION_GUARD(
			peVar1 = pAnimator->pAnimKeyTableEntry;
			if (peVar1->keyIndex_0x8 == 7) {
				iVar4 = 1;
				if (0.0 <= this->field_0xa5c) {
					iVar3 = 4;
				}
				else {
					iVar4 = 4;
					iVar3 = 1;
				}
				(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8 + iVar4] = (int)this->field_0x4e0;
				(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8 + iVar4] = (int)this->field_0x4e4;
				(&peVar1[1].keyIndex_0x8)[peVar1->keyIndex_0x8 + iVar4] = (int)this->field_0x4e8;
				piVar2 = &peVar1[1].count_0x0 + peVar1->keyIndex_0x8 + iVar3;
				piVar2[2] = 0;
				piVar2[1] = 0;
				*piVar2 = 0;
				(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = (int)this->field_0x4ec;
			})
		}
		else {
			if ((newAnim == 0x4d) || (newAnim == 0x38)) {
				IMPLEMENTATION_GUARD(
				peVar1 = pAnimator->pAnimKeyTableEntry;
				if (peVar1->keyIndex_0x8 == 4) {
					peVar1[2].field_0x4 = (int)this->field_0x4e4;
					(&peVar1[2].count_0x0)[peVar1->keyIndex_0x8] = (int)this->field_0x4e8;
					fVar6 = this->field_0x4e0;
					if (0.0 <= fVar6) {
						(&peVar1[1].keyIndex_0x8)[peVar1->keyIndex_0x8] = (int)fVar6;
						(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = 0;
					}
					else {
						(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = (int)-fVar6;
						(&peVar1[1].keyIndex_0x8)[peVar1->keyIndex_0x8] = 0;
					}
				})
			}
			else {
				if (((newAnim == 0x27) || (newAnim == 0x18)) || (newAnim == 0x16)) {
					IMPLEMENTATION_GUARD(
					peVar1 = pAnimator->pAnimKeyTableEntry;
					if (peVar1->keyIndex_0x8 == 2) {
						peVar1[2].count_0x0 = *(int*)&this->field_0x4fc;
						(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = (int)(1.0 - *(float*)&this->field_0x4fc);
					})
				}
				else {
					if ((newAnim == 0x74) || (newAnim == 0x73)) {
						char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
						AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

						local_c = pAnimator->pAnimKeyTableEntry;
						if (local_c->keyIndex_0x8.asKey == 5) {
							if ((this->flags & 0x1000) == 0) {
								GetAnglesFromVector(&this->rotationEuler, &this->rotationQuat);
							}

							edF32Matrix4FromEulerSoft(&eStack176, &this->rotationEuler.xyz, "XYZ");
							edF32Matrix4GetTransposeHard(&eStack176, &eStack176);
							edF32Matrix4MulF32Vector4Hard(&local_c0, &eStack176, &this->dynamic.velocityDirectionEuler);

							if (1.0f < local_c0.x) {
								puVar10 = 1.0f;
							}
							else {
								puVar10 = -1.0f;
								if (-1.0f <= local_c0.x) {
									puVar10 = local_c0.x;
								}
							}

							fVar6 = acosf(puVar10);
							fVar6 = (fVar6 - 1.570796f) / -1.570796f;
							if (0.0f <= local_c0.y) {
								puVar10 = 1.0f;
							}
							else {
								puVar10 = -1.0f;
							}
							fVar8 = puVar10 * (1.0f - fabs(fVar6));
							fVar7 = edFIntervalUnitDstLERP(this->dynamic.linearAcceleration, 0.8f, 4.0f);
							pValue->field_0x1c = 1.0f - fVar7;

							if (0.0 <= fVar6) {
								pValue->field_0x10 = fVar7 * fVar6;
								pValue->field_0xc = 0.0f;
							}
							else {
								pValue->field_0xc = -fVar7 * fVar6;
								pValue->field_0x10 = 0.0f;
							}

							if (0.0 <= fVar8) {
								pValue->field_0x14 = fVar7 * fVar8;
								pValue->field_0x18 = 0.0f;
							}
							else {
								pValue->field_0x18 = -fVar7 * fVar8;
								pValue->field_0x14 = 0.0f;
							}
						}
					}
					else {
						if (newAnim == 0x70) {
							char* pBase = (char*)pAnimator->pAnimKeyTableEntry;

							local_8 = pAnimator->pAnimKeyTableEntry;
							if ((this->flags & 0x1000) == 0) {
								GetAnglesFromVector(&this->rotationEuler, &this->rotationQuat);
							}
							edF32Matrix4FromEulerSoft(&auStack96, &this->rotationEuler.xyz, "XYZ");
							fVar6 = edF32Vector4SafeNormalize0Hard(&eStack112, &this->field_0x750);
							fVar7 = edF32Vector4DotProductHard(&eStack112, &auStack96.rowX);
							fVar8 = edF32Vector4DotProductHard(&eStack112, &auStack96.rowY);
							fVar9 = edF32Vector4DotProductHard(&eStack112, &auStack96.rowZ);

							if (((fVar7 != 0.0f) || (fVar8 != 0.0f)) || (fVar9 != 0.0f)) {
								AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);
								if (0.0f <= fVar7) {
									fVar12 = 0.5f;
									fVar11 = fVar7 * ((fVar6 / 7.0f) / 2.0f);
									if (fVar11 <= 0.5f) {
										fVar12 = fVar11;
									}

									pValue->field_0xc = (fVar12 + 0.5f) * 1000.0f * fVar7;
									pValue->field_0x10 = (1.0f - (fVar12 + 0.5f)) * 1000.0f * fVar7;
								}
								else {
									fVar12 = 0.5f;
									fVar11 = -fVar7 * ((fVar6 / 7.0f) / 2.0f);
									if (fVar11 <= 0.5f) {
										fVar12 = fVar11;
									}

									pValue->field_0xc = (1.0f - (fVar12 + 0.5f)) * 1000.0f * -fVar7;
									pValue->field_0x10 = (fVar12 + 0.5f) * 1000.0f * -fVar7;
								}

								pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);
								if (0.0f <= fVar8) {
									fVar7 = 0.5f;
									fVar12 = fVar8 * ((fVar6 / 7.0f) / 2.0f);
									if (fVar12 <= 0.5f) {
										fVar7 = fVar12;
									}

									pValue->field_0x18 = (fVar7 + 0.5f) * 1000.0f * fVar8;
									pValue->field_0x14 = (1.0f - (fVar7 + 0.5f)) * 1000.0f * fVar8;
								}
								else {
									fVar7 = 0.5f;
									fVar12 = -fVar8 * ((fVar6 / 7.0f) / 2.0f);
									if (fVar12 <= 0.5f) {
										fVar7 = fVar12;
									}

									pValue->field_0x18 = (1.0f - (fVar7 + 0.5f)) * 1000.0f * -fVar8;
									pValue->field_0x14 = (fVar7 + 0.5f) * 1000.0f * -fVar8;
								}

								pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);
								if (0.0f <= fVar9) {
									fVar7 = 0.5f;
									fVar6 = fVar9 * ((fVar6 / 7.0f) / 2.0f);
									if (fVar6 <= 0.5f) {
										fVar7 = fVar6;
									}

									pValue->field_0x20 = (fVar7 + 0.5f) * 1000.0f * fVar9;
									pValue->field_0x1c = (1.0f - (fVar7 + 0.5f)) * 1000.0f * fVar9;
								}
								else {
									fVar7 = 0.5f;
									fVar6 = -fVar9 * ((fVar6 / 7.0f) / 2.0f);
									if (fVar6 <= 0.5f) {
										fVar7 = fVar6;
									}

									pValue->field_0x20 = (1.0f - (fVar7 + 0.5f)) * 1000.0f * -fVar9;
									pValue->field_0x1c = (fVar7 + 0.5f) * 1000.0f * -fVar9;
								}
							}
						}
						else {
							if (newAnim == 0x15) {
								IMPLEMENTATION_GUARD(
								peVar1 = pAnimator->pAnimKeyTableEntry;
								if ((this->fightFlags & 4) == 0) {
									fVar6 = this->field_0x4e0;
									if (0.0 <= fVar6) {
										(&peVar1[2].field_0x4)[peVar1->keyIndex_0x8] = (int)fVar6;
										(&peVar1[2].keyIndex_0x8)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = 0;
									}
									else {
										(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = (int)-fVar6;
										(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[2].field_0x4)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[2].keyIndex_0x8)[peVar1->keyIndex_0x8] = 0;
									}
									fVar6 = this->field_0x4e8;
									if (0.0 <= fVar6) {
										(&peVar1[2].count_0x0)[peVar1->keyIndex_0x8] = (int)fVar6;
										(&peVar1[1].keyIndex_0x8)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[3].count_0x0)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[3].field_0x4)[peVar1->keyIndex_0x8] = 0;
									}
									else {
										(&peVar1[3].count_0x0)[peVar1->keyIndex_0x8] = (int)-fVar6;
										(&peVar1[3].field_0x4)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[1].keyIndex_0x8)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[2].count_0x0)[peVar1->keyIndex_0x8] = 0;
									}
								}
								else {
									fVar6 = this->field_0x4e0;
									if (0.0 <= fVar6) {
										(&peVar1[2].keyIndex_0x8)[peVar1->keyIndex_0x8] = (int)fVar6;
										(&peVar1[2].field_0x4)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = 0;
									}
									else {
										(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = (int)-fVar6;
										(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[2].field_0x4)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[2].keyIndex_0x8)[peVar1->keyIndex_0x8] = 0;
									}
									fVar6 = this->field_0x4e8;
									if (0.0 <= fVar6) {
										(&peVar1[1].keyIndex_0x8)[peVar1->keyIndex_0x8] = (int)fVar6;
										(&peVar1[2].count_0x0)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[3].count_0x0)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[3].field_0x4)[peVar1->keyIndex_0x8] = 0;
									}
									else {
										(&peVar1[3].field_0x4)[peVar1->keyIndex_0x8] = (int)-fVar6;
										(&peVar1[3].count_0x0)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[1].keyIndex_0x8)[peVar1->keyIndex_0x8] = 0;
										(&peVar1[2].count_0x0)[peVar1->keyIndex_0x8] = 0;
									}
								})
							}
							else {
								CActor::AnimEvaluate(param_2, pAnimator, newAnim);
							}
						}
					}
				}
			}
		}
	}
	return;
}

void CActorFighter::ClearLocalData()
{
	float fVar1;
	float fVar2;
	float fVar3;

	this->fightFlags = (this->fightFlags | 2);
	this->fightFlags = (this->fightFlags | 4);
	this->fightFlags = (this->fightFlags & 0xfffffffe);
	this->fightFlags = (this->fightFlags & 0xffffffdf);
	this->fightFlags = (this->fightFlags & 0xffffffbf);
	this->fightFlags = (this->fightFlags & 0xffffff7f);
	this->fightFlags = (this->fightFlags & 0xfffffeff);
	this->fightFlags = (this->fightFlags & 0xfffffdff);
	this->fightFlags = (this->fightFlags & 0xfffffbff);
	this->fightFlags = (this->fightFlags & 0xffffefff);
	this->fightFlags = (this->fightFlags & 0xffffdfff);
	this->fightFlags = (this->fightFlags & 0xffffbfff);
	this->fightFlags = (this->fightFlags & 0xffff7fff);
	this->fightFlags = (this->fightFlags & 0xfffeffff);

	this->pAdversary = (CActorFighter*)0x0;
	this->field_0x354 = (CActor*)0x0;

	this->field_0x36c = 1;
	this->field_0x370 = 0;
	this->field_0x374 = 0;
	this->field_0x378 = 0;

	this->field_0x37c.Reset();
	this->field_0x3a4.Reset();
	this->scalarDynJump.Reset();

	this->field_0x3f4 = 1.0f;
	this->field_0x478 = 0.0f;
	IMPLEMENTATION_GUARD_LOG(
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
	this->field_0x474 = 0;)
	this->field_0x634 = (CActor*)0x0;

	this->field_0x738 = 1;
	this->field_0x6a0 = g_xVector;
	
	this->field_0x690 = gF32Vertex4Zero;

	this->field_0x6b0 = 0.0f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x6b4 = 0;)
	this->field_0x684 = 0;
	IMPLEMENTATION_GUARD_LOG(
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
	this->field_0x7fc = fVar3;)

	this->fighterAnatomyZones.field_0x0 = this->fighterAnatomyZones.field_0x10;

	IMPLEMENTATION_GUARD_LOG(
	this->field_0x830 = 0;
	this->field_0x840 = 0;
	this->field_0x834 = 0;
	this->field_0x860 = 0;
	this->field_0x864 = 0;
	this->field_0x868 = 0xffffffff;)

	this->pFighterCombo = (s_fighter_combo*)0x0;

	IMPLEMENTATION_GUARD_LOG(
	this->field_0x630 = 0;)
	this->field_0x8e0 = 0;
	IMPLEMENTATION_GUARD_LOG(
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

void CActorFighter::SetAdversary(CActorFighter* pNewAdversary)
{
	CActor* pCVar1;
	edF32VECTOR4* v1;
	float fVar2;
	edF32VECTOR4 eStack16;

	if (pNewAdversary == (CActor*)0x0) {
		if (this->pAdversary != (CActorFighter*)0x0) {
			this->pAdversary->pAnimationController->UnRegisterBone(0x45544554);
		}

		this->pAdversary = (CActorFighter*)0x0;
		this->field_0x370 = 0;
		this->field_0x378 = 0;
	}
	else {
		pCVar1 = this->pAdversary;
		if (pNewAdversary != pCVar1) {
			if (pCVar1 != (CActor*)0x0) {
				this->pAdversary->pAnimationController->UnRegisterBone(0x45544554);
			}

			this->pAdversary = pNewAdversary;
			this->pAdversary->pAnimationController->RegisterBone(0x45544554);

			IMPLEMENTATION_GUARD(
			v1 = (edF32VECTOR4*)(*(code*)(this->pVTable)->field_0x188)();
			edF32Vector4SubHard(&eStack16, v1, &this->currentLocation);
			eStack16.y = 0.0;
			fVar2 = edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
			this->field_0x370 = fVar2;
			if (fVar2 != 0.0) {
				fVar2 = GetAngleYFromVector(&eStack16);
				this->field_0x378 = fVar2;
			})
		}
	}

	return;
}

void CActorFighter::ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	edF32VECTOR4* peVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edColPRIM_OBJECT* pPrimObj;

	CActorAutonomous::ChangeCollisionSphere(param_1, param_3, param_4);

	pPrimObj = this->pCollisionData->pObbPrim;
	this->fighterAnatomyZones.field_0x10 = pPrimObj->field_0xb0;
	fVar2 = edF32Vector4GetDistHard(&this->fighterAnatomyZones.field_0x10);
	peVar1 = &this->fighterAnatomyZones.field_0x0;
	edF32Vector4NormalizeHard(peVar1, peVar1);
	peVar1 = &this->fighterAnatomyZones.field_0x0;
	edF32Vector4ScaleHard(fVar2, peVar1, peVar1);
	this->fighterAnatomyZones.field_0x0.w = 1.0f;

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
	this->fighterAnatomyZones.field_0x10 = peVar1->field_0xb0;

	fVar2 = edF32Vector4GetDistHard(&this->fighterAnatomyZones.field_0x10);
	edF32Vector4NormalizeHard(&this->fighterAnatomyZones.field_0x0, &this->fighterAnatomyZones.field_0x0);
	edF32Vector4ScaleHard(fVar2, &this->fighterAnatomyZones.field_0x0, &this->fighterAnatomyZones.field_0x0);
	(this->fighterAnatomyZones.field_0x0).w = 1.0f;

	return;
}

void CActorFighter::SetFightBehaviour()
{
	IMPLEMENTATION_GUARD_FIGHT(
	SetBehaviour(FIGHTER_BEHAVIOUR_DEFAULT, -1, -1);)
}

int CActorFighter::GetFightBehaviour()
{
	return FIGHTER_BEHAVIOUR_DEFAULT;
}

void CActorFighter::_InterpretCollisions(int param_2)
{
	CCollision* pCVar1;
	bool bVar2;
	Timer* pTVar3;
	int inAnimType;
	int iVar4;
	int iVar5;
	uint uVar6;
	float puVar7;
	float puVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float unaff_f20;
	edF32VECTOR4 eStack384;
	edF32VECTOR4 eStack368;
	edF32MATRIX4 eStack352;
	edF32VECTOR4 eStack288;
	edF32VECTOR4 eStack272;
	edF32VECTOR4 local_100;
	edF32VECTOR4 eStack240;
	edF32VECTOR4 eStack224;
	edF32VECTOR4 eStack208;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 eStack176;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 local_80;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CAnimation* pAnim;

	pCVar1 = this->pCollisionData;
	local_10 = gF32Vector4Zero;

	this->field_0x690 = local_10;

	bVar2 = (pCVar1->flags_0x4 & 2) != 0;
	if (bVar2) {
		edF32Vector4AddHard(&local_10, &local_10, &pCVar1->aCollisionContact[1].location);
		edF32Vector4AddHard(&this->field_0x690, &this->field_0x690, &pCVar1->aCollisionContact[1].field_0x10);
	}

	uVar6 = (uint)bVar2;
	if ((pCVar1->flags_0x4 & 1) != 0) {
		edF32Vector4AddHard(&local_10, &local_10, &pCVar1->aCollisionContact[0].location);
		edF32Vector4AddHard(&this->field_0x690, &this->field_0x690, &pCVar1->aCollisionContact[0].field_0x10);
		uVar6 = uVar6 + 1;
	}

	if ((pCVar1->flags_0x4 & 4) != 0) {
		edF32Vector4AddHard(&local_10, &local_10, &pCVar1->aCollisionContact[2].location);
		edF32Vector4AddHard(&this->field_0x690, &this->field_0x690, &pCVar1->aCollisionContact[2].field_0x10);
		uVar6 = uVar6 + 1;
	}

	if (this->field_0x8e4 != 0) {
		edF32Vector4AddHard(&local_10, &local_10, &this->field_0x8f0);
		edF32Vector4AddHard(&this->field_0x690, &this->field_0x690, &this->field_0x900);
		uVar6 = uVar6 + 1;
	}

	fVar11 = 1.0f / (float)uVar6;
	edF32Vector4ScaleHard(fVar11, &local_10, &local_10);
	edF32Vector4ScaleHard(fVar11, &this->field_0x690, &this->field_0x690);
	fVar11 = edF32Vector4DotProductHard(&this->field_0x740, &local_10);
	if ((fVar11 <= -0.195f) || (param_2 == 1)) {
		edF32Vector4AddHard(&local_30, &this->currentLocation, &this->fighterAnatomyZones.field_0x0);
		local_30.w = 1.0f;

		if ((this->field_0x738 & 2) == 0) {
			local_20 = this->field_0x740;
		}
		else {
			IMPLEMENTATION_GUARD(
			_SV_VOLDYN_PointSpeedFromVolumeSpeed
			(this->field_0x7b4, this, &local_20, &local_30, &this->field_0x740, &this->field_0x7a0, &this->field_0x690);)
		}

		fVar11 = edF32Vector4DotProductHard(&local_20, &local_20);
		iVar5 = 8;
		if (36.0f < fVar11) {
			iVar5 = 7;
		}

		if ((this->field_0x738 & 1) == 0) {
			IMPLEMENTATION_GUARD(
			fVar11 = edF32Vector4DotProductHard(&local_20, &local_10);
			edF32Vector4ScaleHard(-fVar11, &eStack80, &local_10);
			edF32Vector4AddHard(&local_40, &local_20, &eStack80);
			local_80.x = local_40.x;
			local_80.y = local_40.y;
			local_80.z = local_40.z;
			local_80.w = local_40.w;
			edF32Vector4ScaleHard(1.0 - this->field_0x7c4, &local_40, &local_40);
			edF32Vector4SubHard(&local_80, &local_80, &local_40);
			fVar11 = edF32Vector4DotProductHard(&this->field_0x740, &local_10);
			edF32Vector4ScaleHard(-fVar11, &eStack144, &local_10);
			edF32Vector4AddHard(&this->field_0x740, &this->field_0x740, &eStack144);
			edF32Vector4ScaleHard(this->field_0x7c0, &eStack80, &eStack80);
			edF32Vector4ScaleHard(this->field_0x7c4, &this->field_0x740, &this->field_0x740);
			edF32Vector4AddHard(&this->field_0x740, &this->field_0x740, &eStack80);
			edF32Vector4SubHard(&eStack96, &local_30, &this->field_0x690);
			edF32Vector4AddHard(&eStack112, &local_30, &eStack96);
			_SV_VOLDYN_VolumeSpeedFromPointSpeed
			(this, &eStack160, &this->field_0x7a0, &this->field_0x7b4, &local_30, &eStack112, &local_80);)
		}
		else {
			fVar11 = edF32Vector4DotProductHard(&local_20, &local_10);
			edF32Vector4ScaleHard(-fVar11, &eStack80, &local_10);
			edF32Vector4AddHard(&local_40, &local_20, &eStack80);
			edF32Vector4ScaleHard(this->field_0x7c0, &eStack80, &eStack80);
			edF32Vector4ScaleHard(1.0f - this->field_0x7c4, &local_40, &local_40);
			edF32Vector4AddHard(&local_20, &eStack80, &local_40);
			edF32Vector4SubHard(&eStack96, &local_30, &this->field_0x690);
			edF32Vector4AddHard(&eStack112, &local_30, &eStack96);
			_SV_VOLDYN_PointSpeedFromVolumeSpeed(this->field_0x7b4, &eStack176, &local_30, &this->field_0x740, &this->field_0x7a0, &eStack112);

			pTVar3 = GetTimer();
			edF32Vector4ScaleHard(pTVar3->cutsceneDeltaTime, &eStack192, &local_20);
			edF32Vector4AddHard(&eStack192, &eStack192, &this->field_0x690);
			pTVar3 = GetTimer();
			edF32Vector4ScaleHard(pTVar3->cutsceneDeltaTime, &eStack208, &eStack176);
			edF32Vector4AddHard(&eStack208, &eStack208, &eStack112);
			fVar11 = edF32Vector4NormalizeHard(&eStack96, &eStack96);
			edF32Vector4SubHard(&eStack224, &eStack208, &eStack192);
			edF32Vector4NormalizeHard(&eStack224, &eStack224);
			edF32Vector4ScaleHard(fVar11, &eStack240, &eStack224);
			edF32Vector4AddHard(&eStack240, &eStack240, &eStack192);
			edF32Vector4SubHard(&this->field_0x740, &eStack240, &local_30);
			pTVar3 = GetTimer();
			edF32Vector4ScaleHard(1.0f / pTVar3->cutsceneDeltaTime, &this->field_0x740, &this->field_0x740);
			edF32Vector4CrossProductHard(&this->field_0x7a0, &eStack96, &eStack224);
			edF32Vector4SafeNormalize0Hard(&this->field_0x7a0, &this->field_0x7a0);
			pTVar3 = GetTimer();
			fVar11 = pTVar3->cutsceneDeltaTime;
			puVar7 = edF32Vector4DotProductHard(&eStack96, &eStack224);
			if (1.0f < puVar7) {
				puVar8 = 1.0f;
			}
			else {
				puVar8 = -1.0f;
				if (-1.0f <= puVar7) {
					puVar8 = puVar7;
				}
			}

			fVar9 = acosf(puVar8);

			static float FLOAT_00448b08 = 15.707964;

			this->field_0x7b4 = fVar9 / fVar11;
			if (FLOAT_00448b08 < this->field_0x7b4) {
				this->field_0x7b4 = FLOAT_00448b08;
			}
			else {
				this->field_0x7b4 = this->field_0x7b4;
			}

			if (0.0f < local_10.y) {
				fVar9 = local_10.y * 3.0f;

				fVar11 = edF32Vector4DotProductHard(&this->field_0x740, &local_10);
				if (0.0f < fVar9 - fVar11) {
					edF32Vector4ScaleHard(fVar9 - fVar11, &eStack80, &local_10);
					edF32Vector4AddHard(&this->field_0x740, &this->field_0x740, &eStack80);
				}
			}
		}

		this->field_0x7b0 = 0.0f;
		edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");

		if ((this->field_0x6b8 & 0x40U) != 0) {
			this->field_0x7b4 = 0.0;
			this->field_0x6b8 = this->field_0x6b8 & 0xffffffbf;
		}

		inAnimType = _SV_ANM_GetMultiWaysAnim3D(&this->field_0x730, &local_10, &local_100);
		pAnim = this->pAnimationController;
		iVar4 = CActor::GetIdMacroAnim(inAnimType);
		if (iVar4 < 0) {
			fVar11 = 0.0f;
		}
		else {
			fVar11 = pAnim->GetAnimLength(iVar4, 1);
		}

		if (local_100.x == 0.0f) {
			if (local_100.y == 0.0f) {
				if (local_100.z != 0.0f) {
					edF32Vector4ScaleHard(local_100.z, &eStack272, &this->field_0x760.rowZ);
				}
			}
			else {
				edF32Vector4ScaleHard(local_100.y, &eStack272, &this->field_0x760.rowY);
			}
		}
		else {
			edF32Vector4ScaleHard(local_100.x, &eStack272, &this->field_0x760.rowX);
		}

		edF32Vector4CrossProductHard(&eStack288, &eStack272, &local_10);
		edF32Vector4SafeNormalize1Hard(&eStack288, &eStack288);
		puVar7 = edF32Vector4DotProductHard(&eStack272, &local_10);
		if (1.0f < puVar7) {
			puVar8 = 1.0f;
		}
		else {
			puVar8 = -1.0f;
			if (-1.0f <= puVar7) {
				puVar8 = puVar7;
			}
		}

		fVar9 = acosf(puVar8);
		MatrixRotationFromVectorAndAngle(-fVar9, &eStack352, &eStack288);
		edF32Matrix4MulF32Matrix4Hard(&this->field_0x760, &this->field_0x760, &eStack352);
	
		_SV_DYN_SetRotationAroundMassCenter(&this->field_0x760);

		edF32Vector4SubHard(&eStack368, &local_30, &this->field_0x690);

		fVar9 = edF32Vector4DotProductHard(&eStack368, &local_10);
		if (local_100.x == 0.0f) {
			if (local_100.y == 0.0f) {
				if (local_100.z != 0.0f) {
					unaff_f20 = ((this->pCollisionData)->pObbPrim->field_0x90).z;
				}
			}
			else {
				unaff_f20 = ((this->pCollisionData)->pObbPrim->field_0x90).y;
			}
		}
		else {
			unaff_f20 = ((this->pCollisionData)->pObbPrim->field_0x90).x;
		}

		edF32Vector4ScaleHard(unaff_f20 - (fVar9 - 0.05f), &eStack384, &local_10);
		edF32Vector4AddHard(&eStack384, &eStack384, &this->currentLocation);
		UpdatePosition(&eStack384, true);

		static float f32TMax = 0.075f;
		static float f32TMin = 0.055f;
		static float f32S = 1.75f;

		fVar9 = edF32Vector4GetDistHard(&this->field_0x740);
		fVar10 = edFIntervalUnitSrcLERP(fVar9 / 14.0f, f32TMax, f32TMin);
		this->field_0x7d8 = fVar10;
		fVar9 = f32S;
		if (f32S <= fVar11 / fVar10) {
			fVar9 = fVar11 / fVar10;
		}

		this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar9, 0);

		if (this->field_0x8e4 == 0) {
			IMPLEMENTATION_GUARD_FX(
			PlayOrientedFx(&this->field_0x690, &local_10, (&this->field_0x51c)[iVar5], (int*)0x0);)
		}

		SetState(0x54, inAnimType);
	}

	return;
}

void CActorFighter::_InterpretSlides()
{
	CCollision* pCVar1;
	uint uVar3;
	edF32VECTOR4* peVar4;
	float fVar5;
	float puVar6;
	float puVar7;
	edF32VECTOR4 eStack176;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 eStack144;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 local_40;

	this->field_0x7b0 = this->field_0x7b0 + this->field_0x7b4 * GetTimer()->cutsceneDeltaTime;
	edF32Matrix4FromEulerSoft(&local_40, &this->rotationEuler.xyz, "XYZ");
	MatrixRotationFromVectorAndAngle(-this->field_0x7b0, &eStack128, &this->field_0x7a0);
	edF32Matrix4MulF32Matrix4Hard(&eStack128, &local_40, &eStack128);

	_SV_DYN_SetRotationAroundMassCenter(&eStack128);

	pCVar1 = this->pCollisionData;
	if ((pCVar1->flags_0x4 & 2) == 0) {
		this->field_0x7b4 = 0.0f;
		this->field_0x7b0 = 0.0f;
		if (0.15f < this->timeInAir) {
			edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");
			edF32Matrix4MulF32Vector4Hard(&this->fighterAnatomyZones.field_0x0, &this->field_0x760, &(this->fighterAnatomyZones).field_0x10);
			SetState(0x56, -1);
		}
		else {
			this->field_0x7e0 = 0x20;
		}
	}
	else {
		edF32Vector4ScaleHard(this->field_0x7dc, &eStack144, &pCVar1->aCollisionContact[1].location);

		uVar3 = (this->pCollisionData)->aCollisionContact[1].materialFlags & 0xf;
		if (uVar3 == 0) {
			uVar3 = CScene::_pinstance->defaultMaterialIndex;
		}

		if (uVar3 == 5) {
			ComputeFrictionForceWithSpeedMax(12.0f, &eStack160, 1);
		}
		else {
			ComputeFrictionForce(90.0f, &eStack160, 0);
		}

		peVar4 = this->dynamicExt.aImpulseVelocities + 1;
		edF32Vector4AddHard(peVar4, peVar4, &eStack160);
		fVar5 = edF32Vector4GetDistHard(peVar4);
		this->dynamicExt.aImpulseVelocityMagnitudes[1] = fVar5;

		ComputeSlidingForce(&eStack176, 0);

		peVar4 = this->dynamicExt.aImpulseVelocities;
		edF32Vector4AddHard(peVar4, peVar4, &eStack176);
		fVar5 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
		this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar5;
		this->timeInAir = 0.0f;

		edF32Vector4CrossProductHard(&this->field_0x7a0, &local_40.rowZ, &eStack144);
		puVar6 = edF32Vector4DotProductHard(&local_40.rowZ, &eStack144);
		if (1.0f < puVar6) {
			puVar7 = 1.0f;
		}
		else {
			puVar7 = -1.0f;;
			if (-1.0 <= puVar6) {
				puVar7 = puVar6;
			}
		}

		fVar5 = acosf(puVar7);
		fVar5 = edF32Between_2Pi(fVar5);
		this->field_0x7b4 = fVar5;
		this->field_0x7b4 = (this->field_0x7b4 - this->field_0x7b0) / (GetTimer()->cutsceneDeltaTime * 2.0f);
	}

	return;
}

void CActorFighter::_StateFighterHitFall(float param_1, edF32VECTOR4* pRotation, edF32VECTOR4* param_4, int bProcessCollisions)
{
	int* piVar1;
	edF32VECTOR4* v0;
	float fVar2;
	float fVar3;
	float fVar4;
	edF32MATRIX4 eStack112;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	this->field_0x6a0 = *param_4;
	this->field_0x6b0 = param_1;

	this->field_0x7a0 = g_xVector;
	this->field_0x7b4 = 0.0f;

	if ((this->flags & 0x1000) == 0) {
		this->flags = this->flags | 0x1000;
	
		this->rotationEuler.xyz = pRotation->xyz;

		edF32Matrix4FromEulerSoft(&eStack112, &pRotation->xyz, "XYZ");
		edF32Matrix4MulF32Vector4Hard(&local_20, &eStack112, &(this->fighterAnatomyZones).field_0x10);
		edF32Vector4SubHard(&eStack48, &this->fighterAnatomyZones.field_0x0, &local_20);
		v0 = &this->currentLocation;
		edF32Vector4AddHard(v0, v0, &eStack48);
		(this->fighterAnatomyZones).field_0x0 = local_20;
	}
	else {
		this->rotationEuler.xyz = pRotation->xyz;
	}

	this->field_0x6b8 = 2;
	this->field_0x684 = 1;

	if (bProcessCollisions == 0) {
		this->field_0x8e4 = 0;
	}
	else {
		IMPLEMENTATION_GUARD(
		edF32Vector4ScaleHard(param_1, &eStack16, param_4);
		piVar1 = _SV_HIT_ProcessActorsCollisions
		(0.0, this, (edF32VECTOR4*)&this->field_0x8f0, (edF32VECTOR4*)&this->field_0x900, &eStack16,
			(int*)bProcessCollisions, (int*)0x0, 1, 6, 3);
		this->field_0x8e4 = piVar1;)
	}

	SetBehaviour(4, 100, -1);

	return;
}

static float FLOAT_00448b04 = 10.0f;

void CActorFighter::_StateFighterHitFlyToSlide()
{
	Timer* pTVar1;
	int iVar3;
	uint uVar4;
	float fVar5;
	float puVar6;
	float puVar7;
	float fVar8;
	float fVar9;
	edF32MATRIX4 eStack528;
	edF32VECTOR4 local_1d0;
	edF32MATRIX4 local_1c0;
	edF32VECTOR4 eStack352;
	edF32VECTOR4 eStack336;
	edF32VECTOR4 eStack320;
	edF32VECTOR4 local_130;
	CActorsTable local_120;
	_ray_info_out _Stack16;

	local_120.entryCount = 0;
	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &this->field_0x740, &this->dynamicExt.normalizedTranslation);

	this->field_0x7d0 = (this->fighterAnatomyZones).field_0x0.y + this->currentLocation.y;

	ManageDyn(4.0f, 0x8000120, &local_120);

	edF32Vector4AddHard(&local_130, &this->fighterAnatomyZones.field_0x0, &this->currentLocation);
	local_130.w = 1.0f;

	fVar5 = edF32Vector4DotProductHard(&this->field_0x740, &this->field_0x740);
	if (fVar5 == 0.0f) {
		edF32Vector4GetNegHard(&eStack336, &g_xVector);
	}
	else {
		edF32Vector4NormalizeHard(&eStack336, &this->field_0x740);
	}

	_SV_VOLDYN_GetCollisionPosFromVector(&local_130, &eStack336);

	edF32Vector4ScaleHard(-0.1f, &eStack320, &eStack336);
	edF32Vector4AddHard(&local_130, &local_130, &eStack320);

	CCollisionRay CStack384 = CCollisionRay(1.2f, &local_130, &eStack336);
	fVar5 = CStack384.Intersect(RAY_FLAG_ACTOR_SCENERY, (CActor*)0x0, (CActor*)0x0, 0x40000008, &eStack352, &_Stack16);
	if (fVar5 != 1e+30f) {
		uVar4 = (uint)_Stack16.pVector_0x4 & 0xf;
		if (uVar4 == 0) {
			uVar4 = CScene::_pinstance->defaultMaterialIndex;
		}

		if (CCollisionManager::_material_table[uVar4].field_0x8 <= fabs(eStack352.y)) {
			uVar4 = (uint)_Stack16.pVector_0x4 & 0xf;
			if (uVar4 == 0) {
				uVar4 = CScene::_pinstance->defaultMaterialIndex;
			}

			if (-CCollisionManager::_material_table[uVar4].field_0x8 < eStack352.y) {
				edF32Matrix4FromEulerSoft(&local_1c0, &this->rotationEuler.xyz, "XYZ");
				fVar9 = edF32Vector4DotProductHard(&eStack352, &local_1c0.rowZ);
				if (fVar9 < 0.0f) {
					this->field_0x7dc = -1.0f;
					edF32Vector4ScaleHard(this->field_0x7dc, &eStack352, &eStack352);
				}
				else {
					this->field_0x7dc = 1.0f;
				}

				edF32Vector4CrossProductHard(&local_1d0, &local_1c0.rowZ, &eStack352);
				edF32Vector4SafeNormalize0Hard(&local_1d0, &local_1d0);
				puVar6 = edF32Vector4DotProductHard(&local_1c0.rowZ, &eStack352);
				if (1.0f < puVar6) {
					puVar7 = 1.0f;
				}
				else {
					puVar7 = -1.0f;
					if (-1.0f <= puVar6) {
						puVar7 = puVar6;
					}
				}

				fVar9 = acosf(puVar7);
				fVar8 = edF32Vector4DotProductHard(&local_1d0, &this->field_0x7a0);
				if ((fabs(fVar8) < 0.99f) || (0.08726647f < fabs(fVar9 - this->field_0x7b0))) {
					edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");
					this->field_0x7a0 = local_1d0;

					this->field_0x7b0 = fVar9;
					this->field_0x7b4 = fVar5;
				}

				fVar5 = edFIntervalUnitDstLERP(fVar5, 0.2f, 1.1f);
				MatrixRotationFromVectorAndAngle(-(this->field_0x7b0 * (1.0f - fVar5)), &local_1c0, &this->field_0x7a0);
				edF32Matrix4MulF32Matrix4Hard(&local_1c0, &this->field_0x760, &local_1c0);
				_SV_DYN_SetRotationAroundMassCenter(&local_1c0);

				goto LAB_00306b50;
			}
		}
	}

	this->field_0x7b0 = this->field_0x7b0 + this->field_0x7b4 * GetTimer()->cutsceneDeltaTime;
	fVar9 = GetTimer()->cutsceneDeltaTime;
	fVar5 = this->field_0x7b4;
	this->field_0x7b4 = fVar5 - FLOAT_00448b04 * fVar5 * fVar5 * GetTimer()->cutsceneDeltaTime * fVar9;
	MatrixRotationFromVectorAndAngle(-this->field_0x7b0, &eStack528, &this->field_0x7a0);
	edF32Matrix4MulF32Matrix4Hard(&eStack528, &this->field_0x760, &eStack528);
	_SV_DYN_SetRotationAroundMassCenter(&eStack528);
	this->field_0x7dc = 0.0f;
LAB_00306b50:
	
	this->field_0x8e4 = _SV_HIT_ProcessActorsCollisions(0.0f, &this->field_0x8f0, &this->field_0x900, &this->field_0x740, &local_120, &this->actorsExcludeTable, 1, 6, 0);
	if ((((this->pCollisionData)->flags_0x4 & 2) == 0) || (this->field_0x7dc == 0.0f)) {
		if (((((this->pCollisionData)->flags_0x4 & 7) == 0) && (this->field_0x8e4 == 0)) ||
			(this->timeInAir <= 0.15f)) {
			this->field_0x750 = this->field_0x740;
			if ((((this->field_0x6b8 & 4U) == 0) && (15.0f < this->dynamicExt.field_0x6c)) &&
				(this->dynamicExt.normalizedTranslation.y < 0.0f)) {
				SetState(0x55, -1);
			}
		}
		else {
			_InterpretCollisions(0);
		}
	}
	else {
		edProjectVectorOnPlane(0.0f, &this->field_0x740, &this->field_0x740, &eStack352, 0);
		iVar3 = _SV_ANM_GetTwoSidedAnim(0x71, (int)this->field_0x7dc);
		SetState(0x57, iVar3);
	}

	return;
}

void CActorFighter::_StateFighterHitSlide()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	float fVar6;
	edF32VECTOR4 eStack16;

	this->field_0x740 = this->dynamic.velocityDirectionEuler;

	edF32Vector4ScaleHard(this->dynamic.linearAcceleration, &this->field_0x740, &this->field_0x740);
	ManageDyn(4.0f, this->field_0x7e0 | 0x53, (CActorsTable*)0x0);
	pCVar1 = this->pAnimationController;
	peVar2 = (pCVar1->anmBinMetaAnimator).aAnimData;
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
		edF32Vector4ScaleHard(this->field_0x7dc, &eStack16, &this->pCollisionData->aCollisionContact[1].location);
		if ((0.8f <= this->dynamic.linearAcceleration) || (fVar6 = edF32Vector4DotProductHard(&eStack16, &g_xVector), fabs(fVar6) <= 0.8f)) {
			PlayAnim(_SV_ANM_GetTwoSidedAnim(0x73, (int)this->field_0x7dc));
		}
		else {
			if (IsAlive() == false) {
				if ((this->field_0x738 & 4) == 0) {
					SetState(0x62, _SV_ANM_GetTwoSidedAnim(0x80, (int)this->field_0x7dc));
				}
				else {
					ProcessDeath();
				}
			}
			else {
				SetState(0x58, _SV_ANM_GetTwoSidedAnim(0x75, (int)this->field_0x7dc));
			}
		}
	}

	_InterpretSlides();

	return;
}

void CActorFighter::_LoadBlow(s_fighter_blow* pBlow, ByteCode* pByteCode)
{
	char cVar1;
	char* pcVar2;
	s_fighter_blow_sub_obj* psVar3;
	uint uVar4;
	int iVar5;
	uint uVar6;
	int iVar7;
	s_fighter_blow_sub_obj* iVar8;
	int* iVar9;
	float fVar10;
	edF32VECTOR4 eStack32;
	undefined4 local_4;

	iVar9 = &pBlow->field_0x60;
	pcVar2 = pByteCode->GetString();

	FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadBlow {}", pcVar2);

	local_4 = 0;
	uVar6 = 0;
	cVar1 = *pcVar2;
	//while (cVar1 != '\0') {
	//	pcVar2 = pcVar2 + 1;
	//	*(char*)((int)&local_4 + (uVar6 & 3)) = *(char*)((int)&local_4 + (uVar6 & 3)) + cVar1;
	//	uVar6 = uVar6 + 1;
	//	cVar1 = *pcVar2;
	//}
	//pBlow->field_0x0 = local_4;
	pBlow->field_0x4 = pByteCode->GetU32();
	pBlow->field_0x5 = pByteCode->GetU32();
	pBlow->field_0x6 = pByteCode->GetU32();
	pBlow->field_0x8 = pByteCode->GetF32();
	pBlow->field_0xc = pByteCode->GetF32();
	pBlow->field_0x10 = pByteCode->GetF32();
	pBlow->field_0x14 = pByteCode->GetF32();
	pBlow->field_0x18 = pByteCode->GetF32();
	pBlow->field_0x20 = pByteCode->GetF32();
	pBlow->field_0x24 = pByteCode->GetF32();
	pBlow->field_0x28 = pByteCode->GetF32();
	pBlow->field_0x2c = 0;
	
	(pBlow->field_0x30).x = pByteCode->GetF32() * 0.02f;
	(pBlow->field_0x30).y = pByteCode->GetF32() * 0.02f;
	(pBlow->field_0x30).z = pByteCode->GetF32() * 0.02f;
	(pBlow->field_0x30).w = 0.0f;
	//edQuatFromEuler(&eStack32, (pBlow->field_0x30).x, (pBlow->field_0x30).y, (pBlow->field_0x30).z);
	//edQuatToAngAxis(&eStack32, &pBlow->field_0x1c, (edF32VECTOR3*)&pBlow->field_0x30);
	pBlow->field_0x1c = pBlow->field_0x1c / 0.02f;
	pBlow->field_0x40 = pByteCode->GetU32();

	pBlow->nbSubObjs = pByteCode->GetU32();
	if (pBlow->nbSubObjs == 0) {
		pBlow->field_0x48 = (s_fighter_blow_sub_obj*)0x0;
	}
	else {
		psVar3 = (s_fighter_blow_sub_obj*)edMemAlloc(TO_HEAP(H_MAIN), pBlow->nbSubObjs * sizeof(s_fighter_blow_sub_obj));
		pBlow->field_0x48 = psVar3;
		uVar6 = 0;
		if (pBlow->nbSubObjs != 0) {
			iVar7 = 0;
			do {
				iVar8 = pBlow->field_0x48 + uVar6;
				iVar8->field_0x4 = pByteCode->GetU32();
				iVar8->field_0xc = pByteCode->GetU32();
				iVar8->materialId = pByteCode->GetS32();
				iVar8->field_0x14uint = pByteCode->GetU32();
				iVar8->field_0x18 = 0;
				uVar4 = pByteCode->GetU32();
				if ((uVar4 & 1) != 0) {
					iVar8->field_0x18 = iVar8->field_0x18 | 0x20;
				}

				if ((uVar4 & 2) != 0) {
					iVar8->field_0x18 = iVar8->field_0x18 | 0x40;
				}
				
				iVar8->field_0x1c = pByteCode->GetF32();
				iVar8->field_0x20 = pByteCode->GetU32();
				if (iVar8->field_0x14 < 0x80) {
					iVar8->field_0x14 = iVar8->field_0x14 << 1;
				}
				else {
					iVar8->field_0x14 = 0xff;
				}

				if (iVar8->field_0x15 < 0x80) {
					iVar8->field_0x15 = iVar8->field_0x15 << 1;
				}
				else {
					iVar8->field_0x15 = 0xff;
				}

				if (iVar8->field_0x16 < 0x80) {
					iVar8->field_0x16 = iVar8->field_0x16 << 1;
				}
				else {
					iVar8->field_0x16 = 0xff;
				}

				CActor::SV_InstallMaterialId(iVar8->materialId);
				uVar6 = uVar6 + 1;
				iVar7 = iVar7 + 0x24;
			} while (uVar6 < pBlow->nbSubObjs);
		}
	}
	iVar7 = 0;
	uVar6 = 0;
	do {
		uVar4 = pByteCode->GetU32();
		//if (uVar4 != 0) {
		//	*(uint*)(iVar9 + 4) = uVar4;
		//	iVar7 = iVar7 + 1;
		//	iVar9 = iVar9 + 8;
		//}
		uVar6 = uVar6 + 1;
	} while (uVar6 < 2);
	pBlow->field_0x5c = iVar7;
	
	pBlow->field_0x4c = pByteCode->GetF32();
	pBlow->field_0x70 = pByteCode->GetS32();
	
	pBlow->field_0x74 = pByteCode->GetF32();
	
	pBlow->field_0x78 = pByteCode->GetF32();
	
	pBlow->field_0x80 = pByteCode->GetF32();
	
	pBlow->field_0x84 = pByteCode->GetF32();
	
	pBlow->field_0x88 = pByteCode->GetF32();
	pBlow->field_0x8c = 0;
	pBlow->field_0x7c = pByteCode->GetU32();
	pBlow->field_0x90 = pByteCode->GetS32();
	
	pBlow->field_0x94 = pByteCode->GetF32();
	
	pBlow->field_0x98 = pByteCode->GetF32();
	
	pBlow->field_0xa0 = pByteCode->GetF32();
	
	pBlow->field_0xa4 = pByteCode->GetF32();
	
	pBlow->field_0xa8 = pByteCode->GetF32();
	pBlow->field_0xac = 0;
	pBlow->field_0x9c = pByteCode->GetU32();
	pBlow->field_0xb0 = pByteCode->GetS32();
	
	pBlow->field_0xb4 = pByteCode->GetF32();
	
	pBlow->field_0xb8 = pByteCode->GetF32();
	
	pBlow->field_0xc0 = pByteCode->GetF32();
	
	pBlow->field_0xc4 = pByteCode->GetF32();
	
	pBlow->field_0xc8 = pByteCode->GetF32();
	pBlow->field_0xcc = 0;
	pBlow->field_0xbc = pByteCode->GetU32();
	pBlow->field_0xd0 = 0;
	return;
}

void CActorFighter::_CreateBlowsDB(ByteCode* pByteCode)
{
	uint uVar1;
	uint uVar4;

	this->nbBlows = pByteCode->GetU32();
	if (this->nbBlows == 0) {
		this->aBlows = (s_fighter_blow*)0x0;
	}
	else {
		this->aBlows = (s_fighter_blow*)edMemAlloc(TO_HEAP(H_MAIN), this->nbBlows * sizeof(s_fighter_blow));;
		uVar1 = this->nbBlows;
		uVar4 = 0;
		if (uVar1 != 0) {
			do {
				_LoadBlow(this->aBlows + uVar4, pByteCode);
				uVar1 = this->nbBlows;
				uVar4 = uVar4 + 1;
			} while (uVar4 < uVar1);
		}

		//_SortIncreasing<14s_fighter_blow>(this->aBlows, uVar1);
	}

	return;
}


void CActorFighter::_LoadGrab(s_fighter_grab* pGrab, ByteCode* pByteCode)
{
	char cVar1;
	char* pcVar2;
	int iVar3;
	uint uVar4;
	float fVar5;
	edF32VECTOR4 eStack32;
	undefined4 local_4;

	pcVar2 = pByteCode->GetString();

	FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadGrab {}", pcVar2);

	local_4 = 0;
	uVar4 = 0;
	cVar1 = *pcVar2;
	//while (cVar1 != '\0') {
	//	pcVar2 = pcVar2 + 1;
	//	*(char*)((int)&local_4 + (uVar4 & 3)) = *(char*)((int)&local_4 + (uVar4 & 3)) + cVar1;
	//	uVar4 = uVar4 + 1;
	//	cVar1 = *pcVar2;
	//}
	pGrab->field_0x0 = local_4;

	pGrab->field_0x4 = pByteCode->GetU32();
	pGrab->field_0x5 = pByteCode->GetU32();
	pGrab->field_0x6 = pByteCode->GetU32();

	
	pGrab->field_0x8 = pByteCode->GetF32();
	
	pGrab->field_0xc = pByteCode->GetF32();
	
	pGrab->field_0x10 = pByteCode->GetF32();
	
	pGrab->field_0x18 = pByteCode->GetU32();
	
	pGrab->field_0x20 = pByteCode->GetF32();
	
	pGrab->field_0x24 = pByteCode->GetF32();
	
	pGrab->field_0x28 = pByteCode->GetF32();
	pGrab->field_0x2c = 0;
	
	pGrab->field_0x30 = pByteCode->GetF32();
	
	pGrab->field_0x34 = pByteCode->GetF32();
	
	pGrab->field_0x38 = pByteCode->GetU32();
	
	pGrab->field_0x3c = pByteCode->GetF32();
	
	pGrab->field_0x60 = pByteCode->GetF32();
	
	pGrab->field_0x40 = pByteCode->GetF32();
	
	pGrab->field_0x44 = pByteCode->GetF32();
	
	pGrab->field_0x48 = pByteCode->GetF32();
	pGrab->field_0x4c = 0;
	
	(pGrab->field_0x50).x = pByteCode->GetF32() * 0.02;
	
	(pGrab->field_0x50).y = pByteCode->GetF32() * 0.02;
	
	(pGrab->field_0x50).z = pByteCode->GetF32() * 0.02;
	pGrab->field_0x5c = 0;
	//edQuatFromEuler(&eStack32, (pGrab->field_0x50).x, (pGrab->field_0x50).y, (pGrab->field_0x50).z);
	//edQuatToAngAxis(&eStack32, &pGrab->field_0x64, &pGrab->field_0x50);
	pGrab->field_0x64 = pGrab->field_0x64 / 0.02;
	
	pGrab->field_0x68 = pByteCode->GetU32();
	pByteCode->GetU32();

	pGrab->field_0x80 = pByteCode->GetS32();
	
	pGrab->field_0x84 = pByteCode->GetF32();
	
	pGrab->field_0x88 = pByteCode->GetF32();
	
	pGrab->field_0x90 = pByteCode->GetF32();
	
	pGrab->field_0x94 = pByteCode->GetF32();
	
	pGrab->field_0x98 = pByteCode->GetF32();
	pGrab->field_0x9c = 0;
	
	pGrab->field_0x8c = pByteCode->GetU32();
	pGrab->field_0xa0 = pByteCode->GetS32();
	
	pGrab->field_0xa4 = pByteCode->GetF32();
	
	pGrab->field_0xa8 = pByteCode->GetF32();
	
	pGrab->field_0xb0 = pByteCode->GetF32();
	
	pGrab->field_0xb4 = pByteCode->GetF32();
	
	pGrab->field_0xb8 = pByteCode->GetF32();
	pGrab->field_0xbc = 0;
	
	pGrab->field_0xac = pByteCode->GetU32();
	pGrab->field_0xc0 = pByteCode->GetS32();
	
	pGrab->field_0xc4 = pByteCode->GetF32();
	
	pGrab->field_0xc8 = pByteCode->GetF32();
	
	pGrab->field_0xd0 = pByteCode->GetF32();
	
	pGrab->field_0xd4 = pByteCode->GetF32();
	
	pGrab->field_0xd8 = pByteCode->GetF32();
	pGrab->field_0xdc = 0;
	
	pGrab->field_0xcc = pByteCode->GetU32();
	pGrab->field_0xe0 = pByteCode->GetS32();
	
	pGrab->field_0xe4 = pByteCode->GetF32();
	
	pGrab->field_0xe8 = pByteCode->GetF32();
	
	pGrab->field_0xf0 = pByteCode->GetF32();
	
	pGrab->field_0xf4 = pByteCode->GetF32();
	
	pGrab->field_0xf8 = pByteCode->GetF32();
	pGrab->field_0xfc = 0;
	
	pGrab->field_0xec = pByteCode->GetU32();
	pGrab->field_0x100 = pByteCode->GetS32();
	
	pGrab->field_0x104 = pByteCode->GetF32();
	
	pGrab->field_0x108 = pByteCode->GetF32();
	
	pGrab->field_0x110 = pByteCode->GetF32();
	
	pGrab->field_0x114 = pByteCode->GetF32();
	
	pGrab->field_0x118 = pByteCode->GetF32();
	pGrab->field_0x11c = 0;
	
	pGrab->field_0x10c = pByteCode->GetU32();
	return;
}

void CActorFighter::_CreateGrabsDB(ByteCode* pByteCode)
{
	char cVar1;
	uint uVar2;
	char* pcVar5;
	uint uVar7;
	undefined4 local_4;

	this->nbGrabs = pByteCode->GetU32();
	if (this->nbGrabs == 0) {
		this->aGrabs = (s_fighter_grab*)0x0;
	}
	else {
		this->aGrabs = (s_fighter_grab*)edMemAlloc(TO_HEAP(H_MAIN), this->nbGrabs * sizeof(s_fighter_grab));;
		uVar2 = this->nbGrabs;
		uVar7 = 0;
		if (uVar2 != 0) {
			do {
				_LoadGrab(this->aGrabs + uVar7, pByteCode);
				uVar2 = this->nbGrabs;
				uVar7 = uVar7 + 1;
			} while (uVar7 < uVar2);
		}

		//_SortIncreasing<14s_fighter_grab>((uint*)this->aGrabs, uVar2);
	}

	this->nbGrabReacts = pByteCode->GetU32();
	if (this->nbGrabReacts == 0) {
		this->aGrabReacts = (s_fighter_grab_react*)0x0;
	}
	else {
		this->aGrabReacts = (s_fighter_grab_react*)edMemAlloc(TO_HEAP(H_MAIN), this->nbGrabReacts * sizeof(s_fighter_grab_react));;
		uVar2 = this->nbGrabReacts;
		uVar7 = 0;
		if (uVar2 != 0) {
			do {
				pcVar5 = pByteCode->GetString();
				local_4 = 0;
				uVar2 = 0;
				cVar1 = *pcVar5;
				//while (cVar1 != '\0') {
				//	pcVar5 = pcVar5 + 1;
				//	*(char*)((int)&local_4 + (uVar2 & 3)) = *(char*)((int)&local_4 + (uVar2 & 3)) + cVar1;
				//	uVar2 = uVar2 + 1;
				//	cVar1 = *pcVar5;
				//}
				this->aGrabReacts[uVar7].field_0x0 = local_4;
				this->aGrabReacts[uVar7].field_0x4 = pByteCode->GetU32();
				this->aGrabReacts[uVar7].field_0x8 = pByteCode->GetU32();
				uVar7 = uVar7 + 1;
				uVar2 = this->nbGrabReacts;
			} while (uVar7 < uVar2);
		}

		//_SortIncreasing<20s_fighter_grab_react>((uint*)this->aGrabReacts, uVar2);
	}

	return;
}



void CActorFighter::_LoadCombo(s_fighter_combo* pCombo, ByteCode* pByteCode)
{
	char cVar1;
	char* pcVar2;
	uint uVar3;
	void* pvVar4;
	uint uVar5;
	uint uVar6;
	int iVar7;
	float fVar8;
	undefined4 local_c;
	undefined4 local_8;
	undefined4 local_4;

	pcVar2 = pByteCode->GetString();

	FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadCombo {}", pcVar2);

	local_4 = 0;
	uVar5 = 0;
	cVar1 = *pcVar2;
	//while (cVar1 != '\0') {
	//	pcVar2 = pcVar2 + 1;
	//	*(char*)((int)&local_4 + (uVar5 & 3)) = *(char*)((int)&local_4 + (uVar5 & 3)) + cVar1;
	//	uVar5 = uVar5 + 1;
	//	cVar1 = *pcVar2;
	//}
	pCombo->field_0x0 = local_4;
	pCombo->field_0x4 = pByteCode->GetU32();
	pCombo->field_0x6 = pByteCode->GetU32();
	pCombo->field_0x8 = pByteCode->GetF32();
	*(uint*)&pCombo->field_0x10 = *(uint*)&pCombo->field_0x10 & 0xfff00000;
	pCombo->field_0x14 = pByteCode->GetU32();
	uVar5 = 0;
	if (pCombo->field_0x14 != 0) {
		uVar6 = 0;
		do {
			uVar3 = pByteCode->GetU32();
			uVar5 = uVar5 + 1;
			//*(uint*)&pCombo->field_0x10 =
			//	*(uint*)&pCombo->field_0x10 & 0xfff00000 |
			//	(uint)((((long)(int)*(uint*)&pCombo->field_0x10 & 0xfffffU |
			//		(long)(*(int*)(&DAT_00425710 + uVar3 * 4) << (uVar6 & 0x1f)) & 0xffffU) << 0x2c) >> 0x2c);
			uVar6 = uVar6 + 4;
		} while (uVar5 < pCombo->field_0x14);
	}

	uVar5 = pByteCode->GetU32();
	*(ushort*)&pCombo->field_0x12 = *(ushort*)&pCombo->field_0x12 & 0xf00f | (ushort)((uVar5 & 0xff) << 4);
	uVar5 = pByteCode->GetU32();
	pCombo->field_0x13 = pCombo->field_0x13 & 0xf | (byte)((uVar5 & 0xf) << 4);
	pCombo->field_0xc = pByteCode->GetF32();
	pcVar2 = pByteCode->GetString();

	FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadCombo B {}", pcVar2);

	local_8 = 0;
	uVar5 = 0;
	cVar1 = *pcVar2;
	//while (cVar1 != '\0') {
	//	pcVar2 = pcVar2 + 1;
	//	*(char*)((int)&local_8 + (uVar5 & 3)) = *(char*)((int)&local_8 + (uVar5 & 3)) + cVar1;
	//	uVar5 = uVar5 + 1;
	//	cVar1 = *pcVar2;
	//}
	pCombo->field_0x18 = local_8;
	uVar5 = pByteCode->GetU32();
	pCombo->field_0x20 = uVar5;
	if (pCombo->field_0x20 != 0) {
		pvVar4 = edMemAlloc(TO_HEAP(H_MAIN), pCombo->field_0x20 << 2);
		pCombo->field_0x1c = pvVar4;
		uVar5 = 0;
		if (pCombo->field_0x20 != 0) {
			iVar7 = 0;
			do {
				pcVar2 = pByteCode->GetString();
				local_c = 0;
				uVar6 = 0;
				cVar1 = *pcVar2;
				//while (cVar1 != '\0') {
				//	pcVar2 = pcVar2 + 1;
				//	*(char*)((int)&local_c + (uVar6 & 3)) = *(char*)((int)&local_c + (uVar6 & 3)) + cVar1;
				//	uVar6 = uVar6 + 1;
				//	cVar1 = *pcVar2;
				//}
				uVar5 = uVar5 + 1;
				//*(undefined4*)((int)pCombo->field_0x1c + iVar7) = local_c;
				iVar7 = iVar7 + 4;
			} while (uVar5 < pCombo->field_0x20);
		}
	}

	return;
}

void CActorFighter::_CreateCombosDB(ByteCode* pByteCode)
{
	uint uVar1;
	s_fighter_combo* psVar2;
	int iVar3;
	uint uVar4;

	uVar1 = pByteCode->GetU32();
	if (uVar1 == 0) {
		this->aCombos = (s_fighter_combo*)0x0;
		this->nbCombos = 0;
		this->nbComboRoots = 0;
	}
	else {
		this->aCombos = (s_fighter_combo*)edMemAlloc(TO_HEAP(H_MAIN), uVar1 * sizeof(s_fighter_combo));;
		uVar4 = 0;
		if (uVar1 != 0) {
			iVar3 = 0;
			do {
				_LoadCombo(this->aCombos + uVar4, pByteCode);
				uVar4 = uVar4 + 1;
				iVar3 = iVar3 + 0x24;
			} while (uVar4 < uVar1);
		}

		//uVar4 = _MarkCombosRoots((long)(int)this, (int*)this->aCombos, uVar1);
		this->nbComboRoots = uVar4;
		this->nbCombos = uVar1 - this->nbComboRoots;
		//_SortIncreasing<15s_fighter_combo>((uint*)this->aCombos, uVar1);
	}
	return;
}

int CActorFighter::_SV_HIT_ProcessActorsCollisions(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5, CActorsTable* param_6, CFighterExcludedTable* param_7, bool param_8, ushort param_9, ushort param_10)
{
	CActor* pReceiver;
	CCollision* pCVar1;
	edColPRIM_OBJECT* peVar2;
	ed_3d_hierarchy_node* peVar3;
	bool bVar4;
	CFighterExcludedTable* pCVar5;
	int iVar6;
	float fVar8;
	CActorsTable* local_200;
	int local_1f0;
	edF32VECTOR4 eStack464;
	edF32VECTOR4 eStack448;
	undefined4 local_1b0[2];
	undefined4 local_1a8;
	float local_1a4;
	undefined4 local_1a0;
	edF32VECTOR4 eStack400;
	float local_180;
	float local_170;
	float local_16c;
	float local_168;
	float local_164;
	undefined2 local_160;
	undefined2 local_15e;
	edF32MATRIX4 local_130;
	edF32VECTOR4 eStack240;
	edF32VECTOR4 local_e0[2];
	undefined4 local_c0[2];
	uint local_b8;
	float local_b4;
	undefined4 local_b0;
	edF32VECTOR4 eStack160;
	float local_90;
	float local_80;
	float local_7c;
	float local_78;
	float local_74;
	ushort local_70;
	ushort local_6e;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	CActor* local_18;
	int local_14;
	int local_10;
	undefined4* local_8;
	undefined4* local_4;
	int uVar7;

	uVar7 = 0;
	local_1f0 = 0;
	if (param_6->entryCount != 0) {
		if (param_3 != (edF32VECTOR4*)0x0) {
			param_3->x = 0.0f;
			param_3->y = 0.0f;
			param_3->z = 0.0f;
			param_3->w = 0.0f;
		}

		if (param_4 != (edF32VECTOR4*)0x0) {
			param_4->x = 0.0f;
			param_4->y = 0.0f;
			param_4->z = 0.0f;
			param_4->w = 0.0f;
		}

		local_200 = param_6;
	LAB_0030bdc0:
		while (local_1f0 < param_6->entryCount) {
			pReceiver = local_200->aEntries[0];
			local_1f0 = local_1f0 + 1;
			local_200 = (CActorsTable*)local_200->aEntries;
			bVar4 = pReceiver->IsKindOfObject(8);

			if (bVar4 != false) {
				if (param_7 != (CFighterExcludedTable*)0x0) goto code_r0x0030b96c;
				goto LAB_0030b9e0;
			}

			pCVar1 = pReceiver->pCollisionData;
			if (((pCVar1->pObbPrim != (edColPRIM_OBJECT*)0x0) && ((pCVar1->pObbPrim->flags_0x80 & 0x200) != 0)) &&
				(pCVar1->primType == 0xe)) {
				local_40 = pCVar1->field_0x90;

				peVar2 = pReceiver->pCollisionData->pObbPrim;
				local_e0[0] = pReceiver->pCollisionData->pObbPrim->field_0xb0;

				peVar3 = pReceiver->pMeshTransform;
				local_130.rowX = pReceiver->pMeshTransform->base.transformA.rowX;
				local_130.rowY = pReceiver->pMeshTransform->base.transformA.rowY;
				local_130.rowZ = pReceiver->pMeshTransform->base.transformA.rowZ;
				local_130.rowT = gF32Vertex4Zero;

				edF32Matrix4MulF32Vector4Hard(&eStack240, &local_130, local_e0);
				edF32Vector4AddHard(&eStack464, &pReceiver->currentLocation, &eStack240);
				eStack464.w = 1.0f;

				edF32Vector4SubHard(&eStack48, &local_40, &eStack464);
				edF32Vector4NormalizeHard(&eStack48, &eStack48);

				uVar7 = uVar7 + 1;
				if (param_3 != (edF32VECTOR4*)0x0) {
					edF32Vector4AddHard(param_3, param_3, &eStack48);
				}

				if (param_4 != (edF32VECTOR4*)0x0) {
					edF32Vector4AddHard(param_4, param_4, &local_40);
				}
			}

			local_1b0[0] = 8;
			local_1a8 = 0;
			local_180 = edF32Vector4SafeNormalize0Hard(&eStack400, param_5);
			pCVar1 = pReceiver->pCollisionData;
			local_170 = (pCVar1->field_0x90).x;
			local_16c = (pCVar1->field_0x90).y;
			local_168 = (pCVar1->field_0x90).z;
			local_164 = (pCVar1->field_0x90).w;
			local_1a0 = 0;
			local_160 = 0;
			local_15e = 0;
			local_8 = local_1b0;
			local_1a4 = param_1;
			DoMessage(pReceiver, (ACTOR_MESSAGE)2, (MSG_PARAM)local_8);
		}
		if (uVar7 != 0x0) {
			if (param_3 != (edF32VECTOR4*)0x0) {
				if (uVar7 < 0) {
					fVar8 = (float)((uint)uVar7 >> 1 | (uint)uVar7 & 1);
					fVar8 = fVar8 + fVar8;
				}
				else {
					fVar8 = (float)uVar7;
				}

				edF32Vector4ScaleHard(1.0f / fVar8, param_3, param_3);
			}

			if (param_4 != (edF32VECTOR4*)0x0) {
				if (uVar7 < 0) {
					fVar8 = (float)((uint)uVar7 >> 1 | (uint)uVar7 & 1);
					fVar8 = fVar8 + fVar8;
				}
				else {
					fVar8 = (float)uVar7;
				}

				edF32Vector4ScaleHard(1.0f / fVar8, param_4, param_4);
			}
		}
	}

	return uVar7;
code_r0x0030b96c:
	iVar6 = 0;
	pCVar5 = param_7;
	if (0 < param_7->field_0x0) {
		do {
			IMPLEMENTATION_GUARD(
			local_18 = pCVar5->field_0x4;
			local_14 = pCVar5->field_0x8;
			local_10 = pCVar5->field_0xc;
			if (local_18 == pReceiver) {
				bVar4 = true;
				goto LAB_0030b9d0;
			}
			iVar6 = iVar6 + 1;
			pCVar5 = (CFighterExcludedTable*)&pCVar5->field_0xc;)
		} while (iVar6 < param_7->field_0x0);
	}

	bVar4 = false;

LAB_0030b9d0:
	if (!bVar4) {
	LAB_0030b9e0:
		pCVar1 = pReceiver->pCollisionData;
		local_40 = pCVar1->field_0x90;

		IMPLEMENTATION_GUARD(
		edF32Vector4AddHard(&eStack448, &pReceiver->currentLocation, (edF32VECTOR4*)&pReceiver[2].field_0x140);)

		eStack448.w = 1.0f;
		edF32Vector4SubHard(&eStack48, &local_40, &eStack448);
		edF32Vector4NormalizeHard(&eStack48, &eStack48);
		uVar7 = uVar7 + 1;

		if (param_3 != (edF32VECTOR4*)0x0) {
			edF32Vector4AddHard(param_3, param_3, &eStack48);
		}

		if (param_4 != (edF32VECTOR4*)0x0) {
			edF32Vector4AddHard(param_4, param_4, &local_40);
		}

		if (param_8 == true) {
			local_c0[0] = 8;
			local_b0 = 0x3e99999a;
			local_90 = edF32Vector4SafeNormalize0Hard(&eStack160, param_5);
			local_b8 = (uint)(this->field_0x7d4 <= local_90);
			local_80 = local_40.x;
			local_7c = local_40.y;
			local_78 = local_40.z;
			local_74 = local_40.w;
			local_6e = param_10;
			local_4 = local_c0;
			local_b4 = param_1;
			local_70 = param_9;
			IMPLEMENTATION_GUARD(
			CActor::DoMessage((CActor*)this, pReceiver, 2, (uint)local_4);
			if ((param_7 != (CFighterExcludedTable*)0x0) && (param_7->field_0x0 < 6)) {
				(&param_7->field_0x4)[param_7->field_0x0 * 3] = pReceiver;
				(&param_7->field_0xc)[param_7->field_0x0 * 3] = 0x3f800000;
				(&param_7->field_0x8)[param_7->field_0x0 * 3] = 0;
				param_7->field_0x0 = param_7->field_0x0 + 1;
			})
		}
	}

	goto LAB_0030bdc0;
}

void CActorFighter::_SV_DYN_SetRotationAroundMassCenter(edF32MATRIX4* param_2)
{
	edF32VECTOR4* v0;
	edF32VECTOR3 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	edF32Matrix4ToEulerSoft(param_2, &local_30, "XYZ");

	this->rotationEuler.xyz = local_30;

	edF32Matrix4MulF32Vector4Hard(&local_10, param_2, &(this->fighterAnatomyZones).field_0x10);

	edF32Vector4SubHard(&eStack32, &this->fighterAnatomyZones.field_0x0, &local_10);

	v0 = &this->currentLocation;
	edF32Vector4AddHard(v0, v0, &eStack32);
	(this->fighterAnatomyZones).field_0x0 = local_10;

	return;
}

void CActorFighter::_SV_VOLDYN_VolumeSpeedFromPointSpeed(edF32VECTOR4* param_2, edF32VECTOR4* param_3, float* param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, edF32VECTOR4* param_7)
{
	float fVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, param_5, param_6);

	fVar1 = edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
	if (fVar1 == 0.0f) {
		*param_2 = *param_7;
		*param_3 = g_xVector;
		*param_4 = 0.0f;
	}
	else {
		fVar2 = edF32Vector4DotProductHard(&eStack16, param_7);
		edF32Vector4ScaleHard(fVar2, &eStack32, &eStack16);
		edF32Vector4SubHard(&eStack48, param_7, &eStack32);
		fVar2 = edF32Vector4SafeNormalize0Hard(param_2, param_7);
		fVar3 = edF32Vector4GetDistHard(&eStack48);
		edF32Vector4ScaleHard(fVar2 - fVar3, param_2, param_2);
		fVar2 = edF32Vector4SafeNormalize0Hard(param_3, &eStack48);
		*param_4 = fVar2 / fVar1;
		if (*param_4 == 0.0f) {
			*param_3 = g_xVector;
		}
		edF32Vector4CrossProductHard(param_3, param_3, &eStack16);
	}

	return;
}

void CActorFighter::_SV_VOLDYN_PointSpeedFromVolumeSpeed(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, edF32VECTOR4* param_7)

{
	float fVar1;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack32, param_4, param_7);
	fVar1 = edF32Vector4NormalizeHard(&eStack32, &eStack32);
	if (fVar1 == 0.0f) {
		*param_3 = *param_5;
	}
	else {
		edF32Vector4CrossProductHard(&eStack16, &eStack32, param_6);
		edF32Vector4ScaleHard(fVar1 * param_1, &eStack16, &eStack16);
		edF32Vector4AddHard(param_3, param_5, &eStack16);
	}

	return;
}

void CActorFighter::_SV_VOLDYN_GetCollisionPosFromVector(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	edF32Matrix4MulF32Vector4Hard(v0, &this->pCollisionData->pObbPrim->worldTransform, v1);
	edF32Vector4NormalizeHard(v0, v0);
	v0->w = 1.0f;
	edF32Matrix4MulF32Vector4Hard(v0, &this->pCollisionData->pObbPrim->vertices, v0);

	return;
}

int CActorFighter::_SV_ANM_GetMultiWaysAnim3D(s_fighter_multiways_anim* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	float fVar1;
	int iVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_50;
	edF32MATRIX4 auStack64;

	fVar1 = gF32Vector4Zero.w;
	local_50.xyz = gF32Vector4Zero.xyz;

	if ((this->flags & 0x1000) == 0) {
		GetAnglesFromVector(&this->rotationEuler, &this->rotationQuat);
	}

	edF32Matrix4FromEulerSoft(&auStack64, &this->rotationEuler.xyz, "XYZ");

	fVar3 = edF32Vector4DotProductHard(param_3, &auStack64.rowX);
	fVar4 = edF32Vector4DotProductHard(param_3, &auStack64.rowY);
	fVar5 = edF32Vector4DotProductHard(param_3, &auStack64.rowZ);

	if (fabs(fVar5) <= fabs(fVar3)) {
		if (fabs(fVar4) <= fabs(fVar3)) {
			if (0.0f < fVar3) {
				iVar2 = param_2->field_0x4 + (param_2->field_0x0[0] & 0xf);
				local_50.x = 1.0f;
			}
			else {
				local_50.x = -1.0f;
				iVar2 = param_2->field_0x4 + (uint)(((ulong)param_2->field_0x0[0] << 0x38) >> 0x3c);
			}
		}
		else {
			if (0.0f < fVar4) {
				iVar2 = param_2->field_0x4 + (param_2->field_0x0[1] & 0xf);
				local_50.y = 1.0f;
			}
			else {
				local_50.y = -1.0f;
				iVar2 = param_2->field_0x4 + (uint)(((ulong)param_2->field_0x0[1] << 0x38) >> 0x3c);
			}
		}
	}
	else {
		if (fabs(fVar4) <= fabs(fVar5)) {
			if (0.0f < fVar5) {
				iVar2 = param_2->field_0x4 + (param_2->field_0x0[2] & 0xf);
				local_50.z = 1.0f;
			}
			else {
				local_50.z = -1.0f;
				iVar2 = param_2->field_0x4 + (uint)(((ulong)param_2->field_0x0[2] << 0x38) >> 0x3c);
			}
		}
		else {
			if (0.0f < fVar4) {
				iVar2 = param_2->field_0x4 + (param_2->field_0x0[1] & 0xf);
				local_50.y = 1.0f;
			}
			else {
				local_50.y = -1.0f;
				iVar2 = param_2->field_0x4 + (uint)(((ulong)param_2->field_0x0[1] << 0x38) >> 0x3c);
			}
		}
	}

	if (param_4 != (edF32VECTOR4*)0x0) {
		param_4->xyz = local_50.xyz;
		param_4->w = fVar1;
	}

	return iVar2;
}

int CActorFighter::_SV_ANM_GetTwoSidedAnim(int param_2, int param_3)
{
	return param_2 + (param_3 + 1 >> 1);
}

CActorWeapon* CActorFighter::GetWeapon()
{
	return reinterpret_cast<CActorWeapon*>(this->pWeaponActor.Get());
}

void CBehaviourFighter::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorFighter*>(pOwner);
	this->behaviourId = FIGHTER_BEHAVIOUR_DEFAULT;

	return;
}

void CBehaviourFighter::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighter::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighter::End(int newBehaviourId)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighter::InitState(int newState)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighter::TermState(int oldState, int newState)
{
	IMPLEMENTATION_GUARD();
}

int CBehaviourFighter::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighter::SetInitialState()
{
	IMPLEMENTATION_GUARD();
}

int CBehaviourFighter::Execute()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighter::_ManageHit()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighter::_ManageExit()
{
	CActor* pFighter;
	StateConfig* pSVar2;
	uint uVar3;

	pFighter = this->pOwner;
	if (pFighter->actorState == -1) {
		uVar3 = 0;
	}
	else {
		pSVar2 = pFighter->GetStateCfg(pFighter->actorState);
		uVar3 = pSVar2->flags_0x4;
	}
	uVar3 = uVar3 & 0xff800;
	if (uVar3 == 0x1000) {
		this->pOwner->_Proj_GetPossibleExit();
	}
	else {
		if (uVar3 == 0x8000) {
			IMPLEMENTATION_GUARD(
			(*(code*)this->pOwner->pVTable->field_0x1f4)();)
		}
		else {
			if (uVar3 == 0x4000) {
				IMPLEMENTATION_GUARD(
				(*(code*)this->pOwner->pVTable->field_0x1e8)();)
			}
			else {
				if ((uVar3 == 0x80000) || (uVar3 == 0x800)) {
					IMPLEMENTATION_GUARD(
					(*(code*)this->pOwner->pVTable->field_0x1cc)();)
				}
			}
		}
	}
	return;
}

void CBehaviourFighterProjected::Manage()
{
	bool bVar1;
	Timer* pTVar2;
	CLifeInterface* pLifeInterface;
	int iVar3;
	CBehaviour* pCVar4;
	//CActorFighterVTable* pCVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR3 local_f0;
	edF32MATRIX4 eStack224;
	edF32VECTOR3 local_a0;
	edF32MATRIX4 eStack144;
	edF32VECTOR3 local_50;
	edF32MATRIX4 eStack64;
	CAnimation* pAnimation;
	edAnmLayer* pAnmLayer;
	CActorFighter* pFighter;

	this->pOwner->Func_0x198();

	pFighter = this->pOwner;
	switch (pFighter->actorState) {
	case 0x54:
		pFighter->ManageDyn(4.0f, 0xb, (CActorsTable*)0x0);
		if (pFighter->field_0x7d8 <= pFighter->timeInAir) {
			pFighter->SetState(0x56, -1);
		}
		break;
	case 0x56:
		pFighter->_StateFighterHitFlyToSlide();
		break;
	case 0x57:
		pFighter->_StateFighterHitSlide();
		break;
	default:
		IMPLEMENTATION_GUARD();
		break;
	}

	_ManageExit();

	return;

}

void CBehaviourFighterProjected::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorFighter* pFighter;
	CCollision* pCol;
	bool bVar3;
	int iVar4;
	StateConfig* pSVar5;
	uint uVar6;
	float fVar7;

	this->pOwner = reinterpret_cast<CActorFighter*>(pOwner);

	pFighter = this->pOwner;

	pFighter->fightFlags = pFighter->fightFlags | 0x8000;
	pFighter->fightFlags = pFighter->fightFlags & 0xfffeffff;

	this->behaviourId = this->pOwner->prevBehaviourId;
	if (this->pOwner->IsFightRelated(this->behaviourId) != false) {
		this->behaviourId = this->pOwner->GetFightBehaviour();
	}

	this->pOwner->SetLookingAtOff();
	this->pOwner->pAnimationController->RegisterBone(0x53538f8b);

	pCol = this->pOwner->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 | 0x10800;

	this->pOwner->field_0x37c.Reset();
	this->pOwner->field_0x3a4.Reset();
	this->pOwner->scalarDynJump.Reset();

	_ComputeDynamics();

	pFighter = this->pOwner;
	fVar7 = (pFighter->fighterAnatomyZones).field_0x0.y + pFighter->currentLocation.y;
	pFighter->field_0x7d0 = fVar7;
	this->pOwner->field_0x7c8 = fVar7;

	pFighter = this->pOwner;
	iVar4 = pFighter->prevActorState;
	if (iVar4 == -1) {
		uVar6 = 0;
	}
	else {
		pSVar5 = pFighter->GetStateCfg(iVar4);
		uVar6 = pSVar5->flags_0x4 & 1;
	}

	if (uVar6 == 0) {
		this->pOwner->field_0x738 = this->pOwner->field_0x738 & 0xfffffffb;
	}
	else {
		this->pOwner->field_0x738 = this->pOwner->field_0x738 | 4;
	}

	if (newState == -1) {
		pFighter = this->pOwner;
		if ((pFighter->field_0x6b8 & 2U) == 0) {
			pFighter->SetState(0x56, -1);
		}
		else {
			pFighter->SetState(0x55, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->GetLifeInterfaceOther()->SetPriority(2);

	return;
}

void CBehaviourFighterProjected::End(int newBehaviourId)
{
	CActorFighter* pCVar2;

	this->pOwner->GetLifeInterfaceOther()->SetPriority(0);
	this->pOwner->pCollisionData->flags_0x0 = this->pOwner->pCollisionData->flags_0x0 & 0xfffeffff;
	this->pOwner->pCollisionData->flags_0x0 = this->pOwner->pCollisionData->flags_0x0 | 0x30;
	this->pOwner->field_0x7b0 = 0.0f;
	this->pOwner->field_0x7b4 = 0.0f;
	pCVar2 = this->pOwner;
	pCVar2->fightFlags = pCVar2->fightFlags & 0xffff7fff;
	pCVar2->fightFlags = pCVar2->fightFlags | 0x10000;

	this->pOwner->pAnimationController->UnRegisterBone(0x53538f8b);

	this->pOwner->fightFlags = this->pOwner->fightFlags | 0x2000;

	return;
}

void CBehaviourFighterProjected::InitState(int newState)
{
	CActorFighter* pFighter;
	CCollision* pCVar2;
	CPlayerInput* pCVar3;
	Timer* pTVar4;
	int iVar5;
	edF32VECTOR4* v0;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack16;
	CAnimation* pAnimation;

	switch (newState) {
	case 0x54:
		this->pOwner->LifeDecrease(2.0f);
		break;
	case 0x56:
		pFighter = this->pOwner;

		pFighter->dynamicExt.field_0x6c = edF32Vector4SafeNormalize0Hard(&pFighter->field_0x740, &pFighter->field_0x740);
		pFighter->dynamicExt.normalizedTranslation = pFighter->field_0x740;

		if ((pFighter->typeID == ACTOR_HERO_PRIVATE) && (pCVar3 = pFighter->GetInputManager(1, 0), pCVar3 != (CPlayerInput*)0x0)) {
			fVar7 = edFIntervalUnitDstLERP(pFighter->dynamicExt.field_0x6c, 2.0f, 12.0f);
			fVar6 = edFIntervalUnitSrcLERP(fVar7, 0.2f, 0.1f);
			CPlayerInput::FUN_001b66f0(fVar7, 0.0f, fVar6, 0.0f, &pCVar3->field_0x40, 0);
		}

		pCVar2 = pFighter->pCollisionData;
		pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffffcf;
		break;
	case 0x57:
		pFighter = this->pOwner;
		pCVar2 = pFighter->pCollisionData;
		pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x10;
		pFighter->ClearAllSumForceExt();
		pFighter->dynamicExt.normalizedTranslation.x = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.y = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.z = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.w = 0.0f;
		pFighter->dynamicExt.field_0x6c = 0.0f;

		edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &eStack16, &pFighter->field_0x740);

		v0 = pFighter->dynamicExt.aImpulseVelocities;
		edF32Vector4AddHard(v0, v0, &eStack16);
		fVar6 = edF32Vector4GetDistHard(pFighter->dynamicExt.aImpulseVelocities);
		pFighter->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar6;

		pFighter->field_0x7b0 = 0.0f;
		pFighter->field_0x7b4 = 0.0f;
		pFighter->field_0x7e0 = 0;
		pCVar2 = pFighter->pCollisionData;

		IMPLEMENTATION_GUARD_FX(
		CActorFighter::PlayOrientedFx
		((CActor*)pFighter, (undefined4*)&pCVar2->aCollisionContact[1].field_0x10,
			(edF32VECTOR4*)(pCVar2->aCollisionContact + 1), pFighter->field_0x53c, (int*)0x0);)
		break;
	case 0x64:
		this->pOwner->_InterpretCollisions(1);
		break;
	default:
		IMPLEMENTATION_GUARD();
		break;
	}
	return;
}

void CBehaviourFighterProjected::TermState(int oldState, int newState)
{
	CActorFighter* pFighter;
	CCollision* pCol;
	float fVar3;

	if (oldState == 99) {
		pFighter = this->pOwner;
		pCol = pFighter->pCollisionData;
		pCol->flags_0x0 = pCol->flags_0x0 | 0x81000;
		pFighter->flags = pFighter->flags & 0xffffff5f;

		pFighter->EvaluateDisplayState();
	}
	else {
		if (oldState != 0x61) {
			if ((oldState == 0x56) || (oldState == 0x55)) {
				pFighter = this->pOwner;
				if ((newState == 0x57) || (newState == 0x54)) {
					pFighter->Func_0x194(pFighter->field_0x7c8 - pFighter->field_0x7d0);
					fVar3 = (pFighter->fighterAnatomyZones).field_0x0.y + pFighter->currentLocation.y;
					pFighter->field_0x7d0 = fVar3;
					pFighter->field_0x7c8 = fVar3;
				}
			}
			else {
				if (oldState == 0x54) {
					this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
				}
			}
		}
	}

	return;
}

int CBehaviourFighterProjected::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorFighter* pCVar1;
	CActor* pCVar2;
	StateConfig* pSVar3;
	int iVar4;
	uint uVar5;
	edF32VECTOR4* v0;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack16;

	if (msg == 0x27) {
		iVar4 = 0;
	}
	else {
		if (msg == 0x16) {
			IMPLEMENTATION_GUARD(
			edF32Vector4ScaleHard(*(float*)((int)pMsgParam + 0x10), &eStack16, (edF32VECTOR4*)pMsgParam);
			pCVar1 = this->pOwner;
			v0 = (pCVar1->characterBase).dynamicExt.aImpulseVelocities + 2;
			edF32Vector4AddHard(v0, v0, &eStack16);
			fVar6 = edF32Vector4GetDistHard(v0);
			(pCVar1->characterBase).dynamicExt.aImpulseVelocityMagnitudes[2] = fVar6;
			iVar4 = 1;)
		}
		else {
			if (msg == 2) {
				IMPLEMENTATION_GUARD(
				pCVar1 = this->pOwner;
				iVar4 = (pCVar1->characterBase).base.base.actorState;
				uVar5 = 0;
				if (iVar4 != -1) {
					pSVar3 = (*((pCVar1->characterBase).base.base.pVTable)->GetStateCfg)((CActor*)pCVar1, iVar4);
					uVar5 = pSVar3->flags_0x4;
				}

				if ((uVar5 & 0x2000000) == 0) {
					/* WARNING: Load size is inaccurate */
					iVar4 = *pMsgParam;
					if (iVar4 == 10) {
						FUN_00306eb0((int)this->pOwner, (int)pMsgParam);
						(*(code*)this->pVTable->_ManageHit)(this, 0);
						iVar4 = 1;
					}
					else {
						if (iVar4 == 8) {
							pCVar1 = this->pOwner;
							fVar8 = *(float*)((int)pMsgParam + 0x44);
							fVar6 = *(float*)((int)pMsgParam + 0x48);
							fVar7 = *(float*)((int)pMsgParam + 0x4c);
							(pCVar1->field_0x690).x = *(float*)((int)pMsgParam + 0x40);
							(pCVar1->field_0x690).y = fVar8;
							(pCVar1->field_0x690).z = fVar6;
							(pCVar1->field_0x690).w = fVar7;
							pCVar1->field_0x6b4 = *(undefined4*)((int)pMsgParam + 0xc);
							edF32Vector4SafeNormalize0Hard(&pCVar1->field_0x6a0, (edF32VECTOR4*)((int)pMsgParam + 0x20));
							pCVar2 = pCVar1->field_0x634;
							iVar4 = FUN_0030c650(&pCVar1->actorsExcludeTable, (int)pCVar2);
							if (iVar4 == 0) {
								FUN_0030c5e0(0x3f800000, &pCVar1->actorsExcludeTable, (int)pCVar2);
							}
							pCVar1->field_0x6b8 = *(int*)((int)pMsgParam + 8);
							pCVar1->field_0x474 = *(undefined4*)((int)pMsgParam + 0x10);
							pCVar1->field_0x6b0 = *(float*)((int)pMsgParam + 0x30);
							pCVar1->field_0x684 = *(short*)((int)pMsgParam + 0x50);
							pCVar1->field_0x686 = *(ushort*)((int)pMsgParam + 0x52);
							if ((pCVar1->field_0x684 & 1U) != 0) {
								fVar8 = *(float*)((int)pMsgParam + 100);
								fVar6 = *(float*)((int)pMsgParam + 0x68);
								fVar7 = *(float*)((int)pMsgParam + 0x6c);
								(pCVar1->field_0x7a0).x = *(float*)((int)pMsgParam + 0x60);
								(pCVar1->field_0x7a0).y = fVar8;
								(pCVar1->field_0x7a0).z = fVar6;
								(pCVar1->field_0x7a0).w = fVar7;
								pCVar1->field_0x7b4 = *(float*)((int)pMsgParam + 0x70);
							}
							(*(code*)this->pVTable->_ManageHit)(this, (*(uint*)((int)pMsgParam + 8) & 0x80) == 0);
							iVar4 = 1;
						}
						else {
							if (iVar4 == 7) {
								pCVar1 = this->pOwner;
								fVar8 = *(float*)((int)pMsgParam + 0x44);
								fVar6 = *(float*)((int)pMsgParam + 0x48);
								fVar7 = *(float*)((int)pMsgParam + 0x4c);
								(pCVar1->field_0x690).x = *(float*)((int)pMsgParam + 0x40);
								(pCVar1->field_0x690).y = fVar8;
								(pCVar1->field_0x690).z = fVar6;
								(pCVar1->field_0x690).w = fVar7;
								pCVar1->field_0x6b4 = *(undefined4*)((int)pMsgParam + 0xc);
								edF32Vector4SafeNormalize0Hard(&pCVar1->field_0x6a0, (edF32VECTOR4*)((int)pMsgParam + 0x20));
								pCVar2 = pCVar1->field_0x634;
								iVar4 = FUN_0030c650(&pCVar1->actorsExcludeTable, (int)pCVar2);
								if (iVar4 == 0) {
									FUN_0030c5e0((int)&DAT_bf800000, &pCVar1->actorsExcludeTable, (int)pCVar2);
								}
								pCVar1->field_0x6b8 = *(int*)((int)pMsgParam + 8);
								pCVar1->field_0x474 = *(undefined4*)((int)pMsgParam + 0x10);
								pCVar1->field_0x6b0 = *(float*)((int)pMsgParam + 0x30);
								pCVar1->field_0x684 = *(short*)((int)pMsgParam + 0x50);
								pCVar1->field_0x686 = *(ushort*)((int)pMsgParam + 0x52);
								if ((pCVar1->field_0x684 & 1U) != 0) {
									fVar8 = *(float*)((int)pMsgParam + 100);
									fVar6 = *(float*)((int)pMsgParam + 0x68);
									fVar7 = *(float*)((int)pMsgParam + 0x6c);
									(pCVar1->field_0x7a0).x = *(float*)((int)pMsgParam + 0x60);
									(pCVar1->field_0x7a0).y = fVar8;
									(pCVar1->field_0x7a0).z = fVar6;
									(pCVar1->field_0x7a0).w = fVar7;
									pCVar1->field_0x7b4 = *(float*)((int)pMsgParam + 0x70);
								}
								(*(code*)this->pVTable->_ManageHit)(this, (*(uint*)((int)pMsgParam + 8) & 0x80) == 0);
								iVar4 = 1;
							}
							else {
								iVar4 = 0;
							}
						}
					}
				}
				else {
					iVar4 = 0;
				})
			}
			else {
				iVar4 = 0;
			}
		}
	}

	return iVar4;
}

int CBehaviourFighterProjected::Execute()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighterProjected::_ManageHit()
{
	IMPLEMENTATION_GUARD();
}


void CBehaviourFighterProjected::_ComputeDynamics()
{
	ushort uVar1;
	CActorFighter* pFighter;
	ulong uVar3;
	edF32VECTOR4* v0;
	float fVar4;
	float fVar5;
	float fVar6;
	float unaff_f20;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	uVar1 = this->pOwner->field_0x684;
	this->pOwner->dynamic.speed = 0.0f;
	pFighter = this->pOwner;
	pFighter->dynamicExt.normalizedTranslation.x = 0.0f;
	pFighter->dynamicExt.normalizedTranslation.y = 0.0f;
	pFighter->dynamicExt.normalizedTranslation.z = 0.0f;
	pFighter->dynamicExt.normalizedTranslation.w = 0.0f;
	pFighter->dynamicExt.field_0x6c = 0.0f;

	edF32Vector4SafeNormalize0Hard(&this->pOwner->field_0x740, &this->pOwner->field_0x6a0);

	v0 = &this->pOwner->field_0x740;
	edF32Vector4ScaleHard(this->pOwner->field_0x6b0, v0, v0);
	edF32Matrix4FromEulerSoft(&this->pOwner->field_0x760, &this->pOwner->rotationEuler.xyz, "XYZ");

	pFighter = this->pOwner;
	edF32Matrix4MulF32Vector4Hard(&pFighter->fighterAnatomyZones.field_0x0, &pFighter->field_0x760, &(pFighter->fighterAnatomyZones).field_0x10);

	if ((uVar1 & 1) == 0) {
		pFighter = this->pOwner;
		local_10 = pFighter->field_0x740;

		if ((uVar1 & 2) != 0) {
			if ((uVar1 & 8) == 0) {
				IMPLEMENTATION_GUARD(
				pFighter = this->pOwner;
				uVar3 = CActorFighter::_SV_HIT_GetHitZoneFromImpact
				(pFighter, &pFighter->fighterAnatomyZones, (edF32VECTOR4*)&pFighter->field_0x690,
					&pFighter->base.base.currentLocation);)
			}
			else {
				uVar3 = (ulong)this->pOwner->field_0x686;
			}

			if (uVar3 == 1) {
				pFighter = this->pOwner;
				local_30.x = (pFighter->fighterAnatomyZones).field_0x10.x;
				local_30.y = (pFighter->fighterAnatomyZones).field_0x10.y;
				local_30.z = (pFighter->fighterAnatomyZones).field_0x10.z;
				local_30.w = (pFighter->fighterAnatomyZones).field_0x10.w;
				unaff_f20 = ((this->pOwner->pCollisionData)->pObbPrim->field_0x90).z;
			}
			else {
				if (uVar3 == 2) {
					edF32Vector4ScaleHard(this->pOwner->field_0x428, &local_30, &(this->pOwner->fighterAnatomyZones).field_0x10);
					local_30.w = 1.0f;
					unaff_f20 = ((this->pOwner->pCollisionData)->pObbPrim->field_0x90).z / 2.0f;
				}
				else {
					if (uVar3 == 0) {
						edF32Vector4ScaleHard(this->pOwner->field_0x42c, &local_30, &(this->pOwner->fighterAnatomyZones).field_0x10);
						local_30.w = 1.0f;
						unaff_f20 = ((this->pOwner->pCollisionData)->pObbPrim->field_0x90).z / 2.0f;
					}
				}
			}

			edF32Matrix4MulF32Vector4Hard(&local_30, &this->pOwner->pMeshTransform->base.transformA, &local_30);

			if ((uVar1 & 4) == 0) {
				pFighter = this->pOwner;
				local_40.xyz = (pFighter->fighterAnatomyZones).field_0x0.xyz;
				local_40.w = 0.0f;
				edF32Vector4NormalizeHard(&local_40, &local_40);
				edF32Vector4SubHard(&eStack80, &this->pOwner->field_0x634->currentLocation, &local_30);
				fVar4 = edF32Vector4DotProductHard(&local_40, &eStack80);
				edF32Vector4ScaleHard(fVar4, &local_40, &local_40);
				edF32Vector4SubHard(&eStack80, &eStack80, &local_40);
				edF32Vector4SafeNormalize0Hard(&eStack80, &eStack80);
				edF32Vector4ScaleHard(unaff_f20, &eStack80, &eStack80);
				edF32Vector4AddHard(&this->pOwner->field_0x690, &local_30, &eStack80);
			}
			else {
				pFighter = this->pOwner;
				pFighter->field_0x690 = local_30;
			}
		}

		edF32Matrix4MulF32Vector4Hard(&eStack32, &this->pOwner->pMeshTransform->base.transformA, &(this->pOwner->fighterAnatomyZones).field_0x10);
		pFighter = this->pOwner;
		pFighter->_SV_VOLDYN_VolumeSpeedFromPointSpeed(&pFighter->field_0x740, &pFighter->field_0x7a0, &pFighter->field_0x7b4, &eStack32, &pFighter->field_0x690, &local_10);
	}

	this->pOwner->field_0x7b0 = 0.0f;

	pFighter = this->pOwner;
	pFighter->field_0x750 = pFighter->field_0x740;

	return;
}

static float FLOAT_00449750 = 10.039942f;

FighterSubObj_40::FighterSubObj_40()
{
	FLOAT_00449750 = GetTimer()->scaledTotalTime;
	this->field_0x0 = 0;
	this->field_0x4 = 0;
	this->field_0x8 = 0;
	this->field_0xc = 0;
	this->field_0x20 = 0;
	
	this->field_0x10 = gF32Vector4UnitZ;

	this->field_0x30 = this->field_0x30 | 7;
	this->field_0x38 = 0;
	this->field_0x34 = 0;

	return;
}

void StaticMeshComponentAdvanced::Create(ByteCode* pByteCode)
{
	int iVar1;
	int iVar2;
	edF32VECTOR4* peVar3;

	this->textureIndex = pByteCode->GetS32();
	this->meshIndex = pByteCode->GetS32();

	this->Reset();

	if ((gVideoConfig.isNTSC == 0) && (peVar3 = GetTextureAnimSpeedNormalExtruder(), peVar3 != (edF32VECTOR4*)0x0)) {
		peVar3->x = peVar3->x * 0.8333333f;
		peVar3->y = peVar3->y * 0.8333333f;
	}

	this->field_0x60 = 0;
	this->field_0x61 = 0;
	this->field_0x60 = 0;
	this->field_0x61 = 0;

	return;
}

void StaticMeshComponentAdvanced::ResetInternal(int textureIndex, int meshIndex)

{
	edF32VECTOR4* peVar1;

	this->textureIndex = textureIndex;
	this->meshIndex = meshIndex;

	this->Reset();

	if ((gVideoConfig.isNTSC == 0) && (peVar1 = GetTextureAnimSpeedNormalExtruder(), peVar1 != (edF32VECTOR4*)0x0)) {
		peVar1->x = peVar1->x * 0.8333333;
		peVar1->y = peVar1->y * 0.8333333;
	}

	this->field_0x60 = 0;
	this->field_0x61 = 0;
	return;
}

void StaticMeshComponentAdvanced::Reset()

{
	edF32VECTOR4* peVar1;

	if ((this->field_0x64 == 1) && (peVar1 = GetTextureAnimSpeedNormalExtruder(), peVar1 != (edF32VECTOR4*)0x0)) {
		peVar1->x = this->field_0x74;
		peVar1->y = this->field_0x78;
	}

	this->field_0x64 = 0;
	this->field_0x70 = 0;
	this->field_0x6c = 0;
	this->field_0x68 = 0;

	if (this->field_0x60 == 0) {
		if (HasMesh() != false) {
			IMPLEMENTATION_GUARD(
			StaticMeshComponent::Unload_00114e80((StaticMeshComponent*)this, (ed_3D_Scene*)0x0);)
		}

		this->pMeshTransformParent = (edNODE*)0x0;
		this->pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	}
	else {
		SetVisible((ed_3D_Scene*)0x0);
	}

	return;
}

bool StaticMeshComponentAdvanced::HasMesh()
{
	bool bVar1;

	if (this->field_0x60 == 1) {
		bVar1 = false;
		if (this->pMeshTransformData != (ed_3d_hierarchy_node*)0x0) {
			bVar1 = this->field_0x61 == 0;
		}
	}
	else {
		bVar1 = this->pMeshTransformParent != (edNODE*)0x0;
	}

	return bVar1;
}

edF32VECTOR4* StaticMeshComponentAdvanced::GetTextureAnimSpeedNormalExtruder()
{
	int index;
	ed_3d_hierarchy_node* pHier;
	ed_g2d_manager* pManager;
	ed_g2d_material* peVar1;
	ed_hash_code* peVar2;
	ed_g2d_texture* peVar3;
	edF32VECTOR4* peVar4;

	peVar1 = (ed_g2d_material*)0x0;
	if (this->field_0x60 == 0) {
		index = this->textureIndex;
		if ((index != -1) && (pManager = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(index), pManager != (ed_g2d_manager*)0x0)) {
			peVar1 = ed3DG2DGetG2DMaterialFromIndex(pManager, 0);
		}
	}
	else {
		pHier = this->pMeshTransformData;
		if (pHier != (ed_3d_hierarchy_node*)0x0) {
			peVar2 = ed3DHierarchyGetMaterialBank(&pHier->base);
			if (peVar2->pData != 0x0) {
				
				ed_hash_code* pData = LOAD_SECTION_CAST(ed_hash_code*, peVar2->pData);

				ed_Chunck* pChunk = LOAD_SECTION_CAST(ed_Chunck*, pData->pData);

				peVar1 = reinterpret_cast<ed_g2d_material*>(pChunk + 1);
			}
		}
	}

	if (peVar1 == (ed_g2d_material*)0x0) {
		peVar4 = (edF32VECTOR4*)0x0;
	}
	else {
		peVar3 = ed3DG2DGetTextureFromMaterial(peVar1, 0);
		peVar4 = LOAD_SECTION_CAST(edF32VECTOR4*, peVar3->pAnimSpeedNormalExtruder);
	}

	return peVar4;
}

CFighterExcludedTable::CFighterExcludedTable()
{
	this->field_0x0 = 0;
}
