#include "ActorFighter.h"
#include "MathOps.h"
#include "TimeController.h"
#include "kya.h"
#include "FileManager3D.h"
#include "InputManager.h"
#include "CollisionRay.h"
#include "ActorManager.h"
#include "ActorHero.h"
#include "LargeObject.h"
#include "CameraViewManager.h"
#include "ActorWeapon.h"

#define FIGHTER_LOG(level, format, ...) MY_LOG_CATEGORY("Fighter", level, format, ##__VA_ARGS__)

static std::unordered_map<uint, std::string> gDebugActionHashToFullName;

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

template<typename T>
T* _FindByHashcode(T* aDb, uint nbEntries, uint hash)
{
	uint uVar1;
	T* psVar2;
	uint uVar3;
	uint uVar4;

	psVar2 = (T*)0x0;
	if ((aDb != (T*)0x0) && (uVar3 = nbEntries >> 1, nbEntries != 0)) {
		uVar4 = 0;
		if (uVar3 != 0) {
			do {
				uVar1 = uVar3;
				if (aDb[uVar3].hash.hash <= hash) {
					uVar1 = nbEntries;
					uVar4 = uVar3;
				}
				nbEntries = uVar1;
				uVar3 = uVar4 + nbEntries >> 1;
			} while (uVar3 != uVar4);
		}

		if (hash == aDb[uVar3].hash.hash) {
			psVar2 = aDb + uVar3;
		}
	}

	return psVar2;
}

template<typename T>
void _SortIncreasing(T* aDb, uint nbEntries)
{
	uint uVar21 = 0;
	T* pItemA = aDb;
	if (nbEntries != 1) {
		do {
			uint uVar20 = uVar21 + 1;
			if (uVar20 < nbEntries) {
				T* pItemB = aDb + uVar20;
				do {
					if (pItemB->hash.hash < pItemA->hash.hash) {
						T copy = *pItemB;
						*pItemB = *pItemA;
						*pItemA = copy;
					}

					uVar20 = uVar20 + 1;
					pItemB = pItemB + 1;
				} while (uVar20 < nbEntries);
			}

			uVar21 = uVar21 + 1;
			pItemA = pItemA + 1;
		} while (uVar21 < nbEntries - 1);
	}

	return;
}

// Specialization for combo
template<>
void _SortIncreasing<s_fighter_combo>(s_fighter_combo* aDb, uint nbEntries)
{
	uint uVar21 = 0;
	s_fighter_combo* pItemA = aDb;
	if (nbEntries != 1) {
		do {
			uint uVar20 = uVar21 + 1;
			if (uVar20 < nbEntries) {
				s_fighter_combo* pItemB = aDb + uVar20;
				do {
					if ((((ulong)((pItemB->field_0x4.field_0x0ushort & 0x200U) != 0) ^ 1) << 0x20) + pItemB->hash.hash <
						(((ulong)((pItemA->field_0x4.field_0x0ushort & 0x200U) != 0) ^ 1) << 0x20) + pItemA->hash.hash) {
						s_fighter_combo copy = *pItemB;
						*pItemB = *pItemA;
						*pItemA = copy;
					}
					uVar20 = uVar20 + 1;
					pItemB = pItemB + 1;
				} while (uVar20 < nbEntries);
			}
			uVar21 = uVar21 + 1;
			pItemA = pItemA + 1;
		} while (uVar21 < nbEntries - 1);
	}
	return;
}

void CActorFighter::Init()
{
	int iVar2;
	float fVar3;

	this->pWeaponActor.Init();
	CActorAutonomous::Init();
	this->flags = this->flags | 0x100000;

	_InitBlowsDB();
	_InitGrabsDB();
	_InitCombosDB();

	this->field_0x6c0 = _GetFighterAnimationLength(0x29);
	this->field_0x6c0 = this->field_0x6c0 + _GetFighterAnimationLength(0x2a);
	this->field_0x6c0 = this->field_0x6c0 + _GetFighterAnimationLength(0x2b);

	return;
}

void CActorFighter::Term()
{
	undefined** ppuVar1;
	uint uVar2;

	CActorAutonomous::Term();

	IMPLEMENTATION_GUARD_LOG(
	uVar2 = 0;
	ppuVar1 = &PTR_ParticleVTable_0048f900;
	do {
		if (((uint)ppuVar1[0xb] & 1) != 0) {
			FUN_004013c0((int)ppuVar1);
		}
		uVar2 = uVar2 + 1;
		ppuVar1 = ppuVar1 + 0x12;
	} while (uVar2 < 4);)

	_DeleteCombosDB();
	_DeleteGrabsDB();
	_DeleteBlowsDB();

	if (this->pInputAnalyser != (CInputAnalyser*)0x0) {
		delete this->pInputAnalyser;
		this->pInputAnalyser = (CInputAnalyser*)0x0;
	}

	return;
}

void CActorFighter::Manage()
{
	CActorFighter* pCVar1;
	BoneData* pBVar2;
	Timer* pTVar3;
	long lVar4;
	s_fighter_fight_collision* pFightCollision;
	uint uVar6;
	edF32VECTOR4 local_10;

	pFightCollision = this->aFightCollisions;

	if ((this->fightFlags & 0x8000) == 0) {
		if ((this->fightFlags & 0x10000) != 0) {
			ed3DUnLockLOD(this->p3DHierNode);
			this->fightFlags = this->fightFlags & 0xfffeffff;
		}
	}
	else {
		ed3DLockLOD(this->p3DHierNode, 0);
		this->fightFlags = this->fightFlags & 0xffff7fff;
	}

	CActor::Manage();

	uVar6 = 0;
	if (this->nbFightCollisions != 0) {
		do {
			pFightCollision->prevBonePosition = pFightCollision->curBonePosition;
			pFightCollision->pActor->SV_GetBoneWorldPosition(pFightCollision->boneId, &pFightCollision->curBonePosition);
			uVar6 = uVar6 + 1;
			pFightCollision = pFightCollision + 1;
		} while (uVar6 < this->nbFightCollisions);
	}

	if ((IsLookingAt() != 0) && (this->pAdversary != (CActorFighter*)0x0)) {
		pBVar2 = this->pAdversary->pAnimationController->FindReggedBone(0x45544554);
		if (pBVar2 == (BoneData*)0x0) {
			pCVar1 = this->pAdversary;
			local_10.x = pCVar1->currentLocation.x;
			local_10.z = pCVar1->currentLocation.z;
			local_10.w = pCVar1->currentLocation.w;
			local_10.y = pCVar1->currentLocation.y + 1.5f;
		}
		else {
			this->pAdversary->SV_GetBoneWorldPosition(0x45544554, &local_10);
		}

		SetLookingAtRotationHeight(4.71239f, &local_10);
	}

	this->actorsExcludeTable.EmptyByTime(GetTimer()->cutsceneDeltaTime);

	FUN_0031a7c0(this->actorState);

	if (Func_0x19c() == 0) {
		this->field_0x450.all = 0;
	}

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

	if (this->pInputAnalyser != (CInputAnalyser*)0x0) {
		*this->pInputAnalyser = CInputAnalyser();
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

bool CActorFighter::SetBehaviour(int behaviourId, int newState, int animationType)
{
	CCollision* pCVar1;
	ulong uVar2;

	if (behaviourId != this->curBehaviourId) {
		uVar2 = GetBehaviourFlags(behaviourId);
		if ((uVar2 & 1) == 0) {
			if ((this->flags & 0x1000) != 0) {
				Compute2DOrientationFromAngles();
				this->flags = this->flags & 0xffffefff;
			}

			pCVar1 = this->pCollisionData;
			pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xfffff7ff;
		}
		else {
			if ((this->flags & 0x1000) == 0) {
				GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
				this->flags = this->flags | 0x1000;
			}

			pCVar1 = this->pCollisionData;
			pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x800;
		}
	}

	return CActor::SetBehaviour(behaviourId, newState, animationType);
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

static uint _gBehaviourFlags[4] =
{
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
		assert((state - 3) < 4);
		flags = _gBehaviourFlags[state - 3];
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

	this->field_0x358 = 5.5;
	IMPLEMENTATION_GUARD_FIGHT(
	this->field_0x35c = 2.5;)
	this->field_0x360 = 5.0f;
	IMPLEMENTATION_GUARD_FIGHT(
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
	this->field_0x6d4 = 0x60;)

	this->field_0x6d0[0].field_0x4 = 0x60;
	this->field_0x6d0[0].field_0x0[0] = this->field_0x6d0[0].field_0x0[0] & 0xf0;
	this->field_0x6d0[0].field_0x0[1] = this->field_0x6d0[0].field_0x0[1] & 0xf0 | 4;
	this->field_0x6d0[0].field_0x0[2] = this->field_0x6d0[0].field_0x0[2] & 0xf0 | 3;
	this->field_0x6d0[0].field_0x0[0] = this->field_0x6d0[0].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[0].field_0x0[1] = this->field_0x6d0[0].field_0x0[1] & 0xf | 0x50;
	this->field_0x6d0[0].field_0x0[2] = this->field_0x6d0[0].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[1].field_0x4 = 0x66;
	this->field_0x6d0[1].field_0x0[0] = this->field_0x6d0[1].field_0x0[0] & 0xf0;
	this->field_0x6d0[1].field_0x0[1] = this->field_0x6d0[1].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[1].field_0x0[2] = this->field_0x6d0[1].field_0x0[2] & 0xf0 | 3;
	this->field_0x6d0[1].field_0x0[0] = this->field_0x6d0[1].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[1].field_0x0[1] = this->field_0x6d0[1].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[1].field_0x0[2] = this->field_0x6d0[1].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[2].field_0x4 = 0x66;
	this->field_0x6d0[2].field_0x0[0] = this->field_0x6d0[2].field_0x0[0] & 0xf0;
	this->field_0x6d0[2].field_0x0[1] = this->field_0x6d0[2].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[2].field_0x0[2] = this->field_0x6d0[2].field_0x0[2] & 0xf0 | 3;
	this->field_0x6d0[2].field_0x0[0] = this->field_0x6d0[2].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[2].field_0x0[1] = this->field_0x6d0[2].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[2].field_0x0[2] = this->field_0x6d0[2].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[3].field_0x4 = 0x47;
	this->field_0x6d0[3].field_0x0[0] = this->field_0x6d0[3].field_0x0[0] & 0xf0;
	this->field_0x6d0[3].field_0x0[1] = this->field_0x6d0[3].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[3].field_0x0[2] = this->field_0x6d0[3].field_0x0[2] & 0xf0 | 2;
	this->field_0x6d0[3].field_0x0[0] = this->field_0x6d0[3].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[3].field_0x0[1] = this->field_0x6d0[3].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[3].field_0x0[2] = this->field_0x6d0[3].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[4].field_0x4 = 0x4a;
	this->field_0x6d0[4].field_0x0[0] = this->field_0x6d0[4].field_0x0[0] & 0xf0;
	this->field_0x6d0[4].field_0x0[1] = this->field_0x6d0[4].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[4].field_0x0[2] = this->field_0x6d0[4].field_0x0[2] & 0xf0 | 2;
	this->field_0x6d0[4].field_0x0[0] = this->field_0x6d0[4].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[4].field_0x0[1] = this->field_0x6d0[4].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[4].field_0x0[2] = this->field_0x6d0[4].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[5].field_0x4 = 0x4a;
	this->field_0x6d0[5].field_0x0[0] = this->field_0x6d0[5].field_0x0[0] & 0xf0;
	this->field_0x6d0[5].field_0x0[1] = this->field_0x6d0[5].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[5].field_0x0[2] = this->field_0x6d0[5].field_0x0[2] & 0xf0 | 2;
	this->field_0x6d0[5].field_0x0[0] = this->field_0x6d0[5].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[5].field_0x0[1] = this->field_0x6d0[5].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[5].field_0x0[2] = this->field_0x6d0[5].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[6].field_0x4 = 0x55;
	this->field_0x6d0[6].field_0x0[0] = this->field_0x6d0[6].field_0x0[0] & 0xf0;
	this->field_0x6d0[6].field_0x0[1] = this->field_0x6d0[6].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[6].field_0x0[2] = this->field_0x6d0[6].field_0x0[2] & 0xf0 | 2;
	this->field_0x6d0[6].field_0x0[0] = this->field_0x6d0[6].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[6].field_0x0[1] = this->field_0x6d0[6].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[6].field_0x0[2] = this->field_0x6d0[6].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[7].field_0x4 = 0x58;
	this->field_0x6d0[7].field_0x0[0] = this->field_0x6d0[7].field_0x0[0] & 0xf0;
	this->field_0x6d0[7].field_0x0[1] = this->field_0x6d0[7].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[7].field_0x0[2] = this->field_0x6d0[7].field_0x0[2] & 0xf0 | 2;
	this->field_0x6d0[7].field_0x0[0] = this->field_0x6d0[7].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[7].field_0x0[1] = this->field_0x6d0[7].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[7].field_0x0[2] = this->field_0x6d0[7].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[8].field_0x4 = 0x58;
	this->field_0x6d0[8].field_0x0[0] = this->field_0x6d0[8].field_0x0[0] & 0xf0;
	this->field_0x6d0[8].field_0x0[1] = this->field_0x6d0[8].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[8].field_0x0[2] = this->field_0x6d0[8].field_0x0[2] & 0xf0 | 2;
	this->field_0x6d0[8].field_0x0[0] = this->field_0x6d0[8].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[8].field_0x0[1] = this->field_0x6d0[8].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[8].field_0x0[2] = this->field_0x6d0[8].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[9].field_0x4 = 0x21;
	this->field_0x6d0[9].field_0x0[0] = this->field_0x6d0[9].field_0x0[0] & 0xf0;
	this->field_0x6d0[9].field_0x0[1] = this->field_0x6d0[9].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[9].field_0x0[2] = this->field_0x6d0[9].field_0x0[2] & 0xf0 | 2;
	this->field_0x6d0[9].field_0x0[0] = this->field_0x6d0[9].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[9].field_0x0[1] = this->field_0x6d0[9].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[9].field_0x0[2] = this->field_0x6d0[9].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[10].field_0x4 = 0x24;
	this->field_0x6d0[10].field_0x0[0] = this->field_0x6d0[10].field_0x0[0] & 0xf0;
	this->field_0x6d0[10].field_0x0[1] = this->field_0x6d0[10].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[10].field_0x0[2] = this->field_0x6d0[10].field_0x0[2] & 0xf0 | 2;
	this->field_0x6d0[10].field_0x0[0] = this->field_0x6d0[10].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[10].field_0x0[1] = this->field_0x6d0[10].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[10].field_0x0[2] = this->field_0x6d0[10].field_0x0[2] & 0xf | 0x20;

	this->field_0x6d0[0xb].field_0x4 = 0x24;
	this->field_0x6d0[0xb].field_0x0[0] = this->field_0x6d0[0xb].field_0x0[0] & 0xf0;
	this->field_0x6d0[0xb].field_0x0[1] = this->field_0x6d0[0xb].field_0x0[1] & 0xf0 | 2;
	this->field_0x6d0[0xb].field_0x0[2] = this->field_0x6d0[0xb].field_0x0[2] & 0xf0 | 2;
	this->field_0x6d0[0xb].field_0x0[0] = this->field_0x6d0[0xb].field_0x0[0] & 0xf | 0x10;
	this->field_0x6d0[0xb].field_0x0[1] = this->field_0x6d0[0xb].field_0x0[1] & 0xf | 0x20;
	this->field_0x6d0[0xb].field_0x0[2] = this->field_0x6d0[0xb].field_0x0[2] & 0xf | 0x20;

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
		this->pInputAnalyser = (CInputAnalyser*)0x0;
	}
	else {
		this->pInputAnalyser = new CInputAnalyser;
	}


	uVar2 = pByteCode->GetU32();
	this->field_0x444 = 0xffffffff;
	this->validCommandMask.all = 0xffffffff;
	this->hitMultiplier = 1.0f;
	if ((uVar2 & 1) == 0) {
		this->validCommandMask.flags[0] = this->validCommandMask.flags[0] & 0xf | (byte)(((uint)(((ulong)this->validCommandMask.flags[0] << 0x38) >> 0x3c) & 0xb) << 4);
	}

	if ((uVar2 & 2) == 0) {
		this->validCommandMask.flags[0] = this->validCommandMask.flags[0] & 0xf | (byte)(((uint)(((ulong)this->validCommandMask.flags[0] << 0x38) >> 0x3c) & 7) << 4);
	}

	if ((uVar2 & 4) == 0) {
		this->validCommandMask.flags[1] = this->validCommandMask.flags[1] & 0xfd;
	}

	if ((uVar2 & 0x10) == 0) {
		this->validCommandMask.flags[1] = this->validCommandMask.flags[1] & 0xcf | (byte)(((uint)(((ulong)this->validCommandMask.flags[1] << 0x3a) >> 0x3e) & 2) << 4);
	}

	if ((uVar2 & 0x20) == 0) {
		this->validCommandMask.flags[1] = this->validCommandMask.flags[1] & 0xcf | (byte)(((uint)(((ulong)this->validCommandMask.flags[1] << 0x3a) >> 0x3e) & 1) << 4);
	}

	if ((uVar2 & 8) == 0) {
		this->validCommandMask.flags[0] = this->validCommandMask.flags[0] & 0xf | (byte)(((uint)(((ulong)this->validCommandMask.flags[0] << 0x38) >> 0x3c) & 0xd) << 4);
	}

	if ((uVar2 & 0x40) == 0) {
		this->field_0x444 = this->field_0x444 & 0xfffffffe;
	}

	if ((uVar2 & 0x80) == 0) {
		this->field_0x444 = this->field_0x444 & 0xfffffffd;
	}

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

	this->field_0x810[0].x = pByteCode->GetF32();
	this->field_0x810[0].y = pByteCode->GetF32();
	this->field_0x810[0].z = pByteCode->GetF32();
	this->field_0x810[0].w = 1.0f;

	this->field_0x810[1].x = pByteCode->GetF32();
	this->field_0x810[1].y = pByteCode->GetF32();
	this->field_0x810[1].z = pByteCode->GetF32();
	this->field_0x810[1].w = 1.0f;

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
	(this->field_0x600).w = 0.0f;

	//uVar2 = 0;
	//ppuVar6 = &PTR_ParticleVTable_0048f900;
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
	s_fighter_fight_collision* pFightCollision;

	pFightCollision = this->aFightCollisions;
	CActorAutonomous::CarriedByActor(pActor, m0);

	uVar1 = 0;
	if (this->nbFightCollisions != 0) {
		do {
			edF32Matrix4MulF32Vector4Hard(&pFightCollision->curBonePosition, m0, &pFightCollision->curBonePosition);
			uVar1 = uVar1 + 1;
			pFightCollision = pFightCollision + 1;
		} while (uVar1 < this->nbFightCollisions);
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

		if ((GetStateFlags(this->actorState) & 0x2000000) == 0) {
			fVar7 = GetLifeInterface()->GetValue();
			if (((0.0f < fVar7) && (fVar7 = (pSender->currentLocation).x - this->currentLocation.x,
					fVar8 = (pSender->currentLocation).z - this->currentLocation.z, fVar7 * fVar7 + fVar8 * fVar8 <= 1.0f)) &&
					(fabs((pSender->currentLocation).y - this->currentLocation.y) <= 0.5f)) {
				pResolvedMsg->rotationQuat = this->rotationQuat;
				pResolvedMsg->position = this->currentLocation;
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
				if (((GetStateFlags(this->actorState) & 0x2000000) == 0) &&
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
					if (((GetStateFlags(this->actorState) & 0x2000000) != 0) ||
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
	uint stateFlags;

	if (this->curBehaviourId == 3) {
		stateFlags = GetStateFlags(this->actorState);

		if ((stateFlags & 0xff800) == 0x8000) {
			pCVar2 = this->field_0x354;
			if (pCVar2 != (CActor*)0x0) {
				pCVar2->TieToActor(pTieActor, carryMethod, param_4, param_5);
			}
		}
		else {
			if (((stateFlags & 0xff800) == 0x800) &&
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

bool CActorFighter::IsInHitState()
{
	int iVar1;
	bool bVar2;
	uint uVar3;
	bool bVar4;

	iVar1 = this->actorState;
	bVar4 = true;
	uVar3 = iVar1 - 0x14;
	bVar2 = false;

	if ((uVar3 < 0xc) && ((1 << (uVar3 & 0x1f) & 0x897U) != 0)) {
		bVar2 = true;
	}

	uVar3 = iVar1 - 0x38;
	if (!bVar2) {
		bVar2 = false;

		if ((uVar3 < 0x1c) && ((1 << (uVar3 & 0x1f) & 0xc800001U) != 0)) {
			bVar2 = true;
		}

		if (!bVar2) {
			bVar4 = false;
		}
	}

	return bVar4;
}

bool CActorFighter::IsAlive()
{
	return 0.0f < GetLifeInterface()->GetValue();
}

bool Criterion_AcquireAdversary(CActor* pActor, void* pParams)
{
	bool bVar1;
	StateConfig* pSVar2;
	uint uVar3;

	if ((pParams != pActor) && (bVar1 = pActor->IsKindOfObject(OBJ_TYPE_FIGHTER), bVar1 != false)) {
		if (pActor->actorState == -1) {
			uVar3 = 0;
		}
		else {
			pSVar2 = pActor->GetStateCfg(pActor->actorState);
			uVar3 = pSVar2->flags_0x4 & 1;
		}

		if (uVar3 == 0) {
			return true;
		}
	}

	return false;
}

void CActorFighter::AcquireAdversary()
{
	CActorFighter* pNewAdversary;
	float fVar1;
	bool bVar3;
	int iVar4;
	float fVar5;
	edF32VECTOR4 local_120;
	CActorsTable local_110;

	local_110.nbEntries = 0;
	SetAdversary((CActorFighter*)0x0);
	local_120.xyz = this->currentLocation.xyz;
	local_120.w = 5.5f;
	bVar3 = CScene::ptable.g_ActorManager_004516a4->cluster.GetActorsIntersectingSphereWithCriterion(&local_110, &local_120, Criterion_AcquireAdversary, this);
	iVar4 = 0;
	if (bVar3 != false) {
		for (; iVar4 < local_110.nbEntries; iVar4 = iVar4 + 1) {
			pNewAdversary = (CActorFighter*)local_110.aEntries[iVar4];
			if ((pNewAdversary != this->field_0x354) &&
				(((fVar5 = pNewAdversary->currentLocation.x - this->currentLocation.x,
					fVar1 = pNewAdversary->currentLocation.z - this->currentLocation.z,
					fVar5 = sqrtf(fVar5 * fVar5 + 0.0f + fVar1 * fVar1),
					this->pAdversary == (CActorFighter*)0x0 && (fVar5 < this->field_0x360)) ||
					((this->pAdversary != (CActorFighter*)0x0 && (fVar5 < this->adversaryDistance)))))) {
				SetAdversary(pNewAdversary);
			}
		}
	}

	return;
}

edF32VECTOR4* CActorFighter::GetAdversaryPos()
{
	edF32VECTOR4* pAdversaryPosition;

	if (this->pAdversary == (CActorFighter*)0x0) {
		pAdversaryPosition = &this->currentLocation;
	}
	else {
		pAdversaryPosition = &this->field_0x4b0;
		if ((this->fightFlags & 0x1000) == 0) {
			this->pAdversary->_ComputeLogicalPos(&this->currentLocation);
			pAdversaryPosition = &this->pAdversary->logicalPosition;
		}
	}

	return pAdversaryPosition;
}

int CActorFighter::Func_0x18c()
{
	return 1;
}

bool CActorFighter::Func_0x190(CActor* pOther)
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;
	float fVar4;

	fVar4 = edF32Vector4DotProductHard(&pOther->rotationQuat, &this->rotationQuat);
	if (-0.5f < fVar4) {
		uVar3 = GetStateFlags(this->actorState);

		if ((uVar3 & 0xff800) != 0x800) {
			uVar3 = GetStateFlags(this->actorState);
			if ((uVar3 & 0xff800) != 0) {
				return false;
			}
		}

		if (fabs((pOther->currentLocation).y - this->currentLocation.y) < 0.8f) {
			return true;
		}
	}

	return false;
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

bool CActorFighter::Func_0x1ac()
{
	return (((ulong)this->validCommandMask.flags[0] << 0x38) >> 0x3c & 2) != 0;
}

bool CActorFighter::Func_0x1b0(CActor* pOther)
{
	return true;
}

bool CActorFighter::Func_0x1c0(s_fighter_combo* pCombo)
{
	return true;
}

// Should be in: D:/Projects/b-witch/ActorFighter.h
void CActorFighter::UpdateFightCommand()
{
	return;
}

bool CActorFighter::Func_0x19c()
{
	bool bVar2;
	uint uVar5;

	if ((GetBehaviourFlags(this->curBehaviourId) & 4) == 0) {
		bVar2 = false;
	}
	else {
		bVar2 = true;
		if (this->curBehaviourId == 3) {
			uVar5 = GetStateFlags(this->actorState) & 0xff800;
			if (((uVar5 == 0x8000) || (uVar5 == 0x4000)) || (uVar5 == 0x2000)) {
				bVar2 = false;
			}
		}
	}

	return bVar2;
}

bool CActorFighter::Func_0x1a0()
{
	bool bVar2;
	uint uVar5;

	if ((GetBehaviourFlags(this->curBehaviourId) & 2) == 0) {
		bVar2 = false;
	}
	else {
		bVar2 = true;
		if (this->curBehaviourId == 3) {
			uVar5 = GetStateFlags(this->actorState);
			if (((uVar5 == 0x8000) & 0xff800) || ((uVar5 & 0xff800) == 0x4000)) {
				bVar2 = false;
			}
		}
	}

	return bVar2;
}

bool CActorFighter::Func_0x1a4()
{
	return true;
}

bool CActorFighter::Func_0x1a8()
{
	bool bVar1;

	bVar1 = this->validCommandMask.all != 0;
	if (bVar1) {
		bVar1 = this->field_0x444 != 0;
	}

	if (bVar1 != false) {
		bVar1 = (this->flags & 0x800000) == 0;
	}

	return bVar1;
}

void CActorFighter::_Execute_Std(s_fighter_action* pAction, s_fighter_action_param* pParam)
{
	byte bVar1;
	bool bVar2;
	bool bVar3;
	float* pfVar4;
	StateConfig* pSVar5;
	float* pfVar6;
	Timer* pTVar7;
	int iVar8;
	int iVar9;
	float* pfVar10;
	ulong uVar11;
	uint uVar12;
	float fVar13;
	float distance;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32MATRIX4 auStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR3 auStack16;
	s_fighter_action local_4;
	CAnimation* pAnim;

	bVar1 = pAction->field_0x1;
	uVar11 = ((ulong)bVar1 << 0x3a) >> 0x3e;
	if (uVar11 == 2) {
		if ((GetStateFlags(this->actorState) & 0x100) != 0) {
			SetState(0x1c, -1);
		}
	}
	else {
		bVar3 = true;
		if (uVar11 == 1) {
			if ((GetStateFlags(this->actorState) & 0x100) == 0) {
				SetState(0x19, -1);
			}
			else {
				this->field_0x6bc = GetTimer()->cutsceneDeltaTime;
				SetState(0x17, -1);
			}
		}
		else {
			uVar11 = (ulong)bVar1 & 0xf;
			if (uVar11 == 8) {
				iVar8 = this->actorState;
				uVar12 = iVar8 - 0x49;
				bVar2 = false;

				if ((uVar12 < 0x20) && ((1 << (uVar12 & 0x1f) & 0xf000003fU) != 0)) {
					bVar2 = bVar3;
				}

				if ((!bVar2) && (6 < iVar8 - 0x69U)) {
					bVar3 = false;
				}

				if (bVar3) {
					this->pFighterCombo = (s_fighter_combo*)0x0;
					this->field_0x864 = (s_fighter_blow*)0x0;
					this->field_0x860 = 0;
					SetState(6, this->standAnim);
				}
			}
			else {
				if ((uVar11 == 0xc) || (uVar11 == 4)) {
					uVar12 = this->actorState - 0x65;
					bVar3 = false;
					if ((uVar12 < 10) && ((1 << (uVar12 & 0x1f) & 0x303U) != 0)) {
						bVar3 = true;
					}

					if ((!bVar3) || ((bVar1 & 8) != 0)) {
						if ((bVar1 & 8) != 0) {
							SetState(-1, -1);
						}

						this->field_0x864 = (s_fighter_blow*)0x0;
						this->field_0x860 = 0;
						IMPLEMENTATION_GUARD(
						this->field_0x840 = reinterpret_cast<s_fighter_blow*>(pParam->pData);
						this->field_0x474 = field_0x840->field_0xc;
						SetState(0x6b, *(int*)(this->field_0x840 + 0x80));)
						return;
					}

					this->field_0x864 = reinterpret_cast<s_fighter_blow*>(pParam->pData);
					this->field_0x860 = 2;
				}
				else {
					if ((uVar11 == 10) || (uVar11 == 2)) {
						IMPLEMENTATION_GUARD(
						this->pBlow = pParam->field_0x4;
						this->field_0x474 = *(float*)(this->pBlow + 0xc);
						(*(this->pVTable)->SetState)
							((CActor*)this, 0x68, *(int*)(this->pBlow + 0x70));
						return;)
					}
					if ((uVar11 == 9) || (uVar11 == 1)) {
						uVar12 = this->actorState - 0x65;
						bVar3 = false;

						if ((uVar12 < 10) && ((1 << (uVar12 & 0x1f) & 0x303U) != 0)) {
							bVar3 = true;
						}

						if ((!bVar3) || ((bVar1 & 8) != 0)) {
							if ((bVar1 & 8) != 0) {
								SetState(-1, -1);
							}

							this->field_0x864 = (s_fighter_blow*)0x0;
							this->field_0x860 = 0;
							this->pBlow = reinterpret_cast<s_fighter_blow*>(pParam->pData);
							this->field_0x474 = this->pBlow->field_0xc;
							SetState(FIGHTER_BLOW_BEGIN, this->pBlow->blowStageBegin.animId);
							return;
						}

						this->field_0x864 = reinterpret_cast<s_fighter_blow*>(pParam->pData);
						this->field_0x860 = 1;
					}
				}
			}

			bVar1 = pAction->field_0x0;
			if (((ulong)bVar1 & 0xf) == 0xc) {
				if (((ulong)bVar1 << 0x38) >> 0x3c == 2) {
					IMPLEMENTATION_GUARD(
					this->field_0x36c = this->field_0x36c | 1;
					_BuildCommandFromAbsoluteVector(this, pParam->field_0x0, &local_4, &eStack128);
					this->field_0x44c = this->field_0x44c & 0xf0 | local_4.field_0x0 & 0xf;)
				}
				else {
					this->field_0x36c = this->field_0x36c & 0xfffffffe;
					local_20.x = pParam->field_0x0->x;
					local_20.y = 0.0f;
					local_20.z = pParam->field_0x0->z;
					local_20.w = 0.0f;
					edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
					edF32Vector4ScaleHard(this->field_0x4cc, &local_20, &local_20);
					memset(&local_70, 0, sizeof(edF32VECTOR4));

					if ((this->flags & 0x1000) == 0) {
						GetAnglesFromVector(&auStack16, &this->rotationQuat);
						edF32Matrix4FromEulerSoft(&auStack96, &auStack16, "XYZ");
					}
					else {
						edF32Matrix4FromEulerSoft(&auStack96, &this->rotationEuler.xyz, "XYZ");
					}

					local_70.z = edF32Vector4DotProductHard(&local_20, &auStack96.rowZ);
					local_70.x = edF32Vector4DotProductHard(&local_20, &auStack96.rowX);

					_SetRelativeSpeedOnGround(GetTimer()->cutsceneDeltaTime, &local_70);
				}
			}
			else {
				if (((bVar1 & 0xf) != 0) &&
					(this->field_0x36c = this->field_0x36c | 1, ((ulong)pAction->field_0x0 << 0x38) >> 0x3c != 2)) {
					IMPLEMENTATION_GUARD(
					this->scalarDynA.BuildFromSpeedTime(this->field_0x4cc, this->field_0x4cc, GetTimer()->cutsceneDeltaTime);)
				}
			}

			uVar11 = ((ulong)pAction->field_0x0 << 0x38) >> 0x3c;
			if (uVar11 != 8) {
				if (uVar11 == 4) {
					this->scalarDynJump.BuildFromSpeedDistTime(this->field_0x4f8, 0.0f, this->field_0x4f4, 0.22f);
					this->field_0x3f4 = 1.0f;
					this->field_0x474 = 0.0f;
					SetState(10, -1);
				}
				else {
					if (uVar11 == 2) {
						if ((pAction->field_0x1 & 0xf) == 0) {
							IMPLEMENTATION_GUARD(
							iVar8 = _SV_ANM_GetMultiWaysAnim2D(this, (s_fighter_multiways_anim*)&this->field_0x484, this->field_0x44c & 0xf);
							fVar13 = _GetFighterAnimationLength(iVar8);
							bVar1 = this->field_0x44c;
							uVar12 = bVar1 & 0xf;
							if ((uVar12 == 3) || (uVar12 == 6)) {
								if (uVar12 < 6) {
									distance = (float)this->field_0x4c0;
								}
								else {
									distance = (float)this->field_0x4c4;
								}
							}
							else {
								if (uVar12 % 3 == 1) {
									this->field_0x44c = bVar1 & 0xf0 | 1;
								}
								else {
									this->field_0x44c = bVar1 & 0xf0 | 2;
								}
								if (this->adversaryDistance < 0.75) {
									distance = this->adversaryDistance * 1.570796;
								}
								else {
									distance = this->field_0x4c8;
								}
							}
							this->scalarDynA.BuildFromSpeedDistTime(v1$12742, v2$12743, distance, fVar13);
							CScalarDyn::Reset(&this->scalarDynB);
							this->field_0x474 = 0.0;
							SetState(7, iVar8);)
						}
						else {
							this->field_0x44c = this->field_0x44c & 0xf;
						}
					}
					else {
						if (uVar11 == 1) {
							this->field_0x474 = 0.0f;
							SetState(FIGHTER_DEFAULT_STATE_RUN, -1);
						}
					}
				}
			}
		}
	}

	return;
}

void CActorFighter::_Std_GetPossibleHit(bool bPlayImpact)
{
	CActor* pCVar1;
	bool bVar2;
	bool bVar3;
	int iVar4;
	CPlayerInput* pCVar5;
	StateConfig* pSVar6;
	uint uVar7;
	byte bVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 local_60;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	s_fighter_action local_8;
	uint local_4;

	bVar8 = (GetStateFlags(this->actorState) & 0x2000000) != 0;
	bVar2 = false;

	if ((iVar4 - 0x3fU < 7) && ((1 << (iVar4 - 0x3fU & 0x1f) & 99U) != 0)) {
		bVar2 = true;
	}

	if (bVar2) {
		bVar8 = bVar8 | 2;
	}

	if (iVar4 - 0x17U < 5) {
		bVar8 = bVar8 | 4;
	}

	bVar2 = false;
	if ((iVar4 - 0x1cU < 4) && ((1 << (iVar4 - 0x1cU & 0x1f) & 0xbU) != 0)) {
		bVar2 = true;
	}

	if (bVar2) {
		bVar8 = bVar8 | 8;
	}

	if (bVar8 != 0) {
		if ((this->flags & 0x1000) == 0) {
			local_20 = this->rotationQuat;
		}
		else {
			SetVectorFromAngleY(this->rotationEuler.y, &local_20);
		}

		if (fabs((this->field_0x6a0).y) == 1.0f) {
			pCVar1 = this->field_0x634;
			if ((pCVar1->flags & 0x1000) == 0) {
				local_30 = pCVar1->rotationQuat;
			}
			else {
				SetVectorFromAngleY((pCVar1->rotationEuler).y, &local_30);
			}
		}
		else {
			local_30.x = (this->field_0x6a0).x;
			local_30.z = (this->field_0x6a0).z;
			local_30.w = (this->field_0x6a0).w;
			local_30.y = 0.0f;
			edF32Vector4NormalizeHard(&local_30, &local_30);
		}

		fVar9 = edF32Vector4DotProductHard(&local_20, &local_30);
		if (fVar9 < 0.5f) {
			bVar8 = (GetStateFlags(this->actorState) & 0x2000000) != 0;
			bVar2 = false;
			if ((iVar4 - 0x3fU < 7) && ((1 << (iVar4 - 0x3fU & 0x1f) & 99U) != 0)) {
				bVar2 = true;
			}

			if (bVar2) {
				bVar8 = bVar8 | 2;
			}

			if (iVar4 - 0x17U < 5) {
				bVar8 = bVar8 | 4;
			}

			bVar2 = false;
			if ((iVar4 - 0x1cU < 4) && ((1 << (iVar4 - 0x1cU & 0x1f) & 0xbU) != 0)) {
				bVar2 = true;
			}

			if (bVar2) {
				bVar8 = bVar8 | 8;
			}

			if (bVar8 == 8) {
				if ((this->hitFlags & 0x20U) == 0) {
					this->field_0x474 = 0.0f;
					DoMessage(this->field_0x634, (ACTOR_MESSAGE)100, &this->field_0x6c4);
					iVar4 = GetIdMacroAnim(0x2c);

					if (iVar4 < 0) {
						fVar9 = 0.0f;
					}
					else {
						fVar9 = this->pAnimationController->GetAnimLength(iVar4, 1);
					}

					edF32Vector4SubHard(&local_40, &this->currentLocation, &this->field_0x634->currentLocation);
					local_40.y = 0.0f;
					edF32Vector4NormalizeHard(&local_40, &local_40);
					_BuildCommandFromAbsoluteVector(&local_40, &local_8, &eStack80);

					this->field_0x44c = this->field_0x44c & 0xf0 | local_8.field_0x0 & 0xf;
					this->field_0x36c = this->field_0x36c & 0xfffffffe;

					static float FLOAT_00448b28 = 2.0f;
					edF32Vector4ScaleHard(this->field_0x6b0 * FLOAT_00448b28 * 0.040625f, &local_40, &eStack80);
					edF32Vector4ScaleHard(this->field_0x6b0, &local_60, &eStack80);
					this->scalarDynA.BuildFromSpeedDistTime(fabs(local_60.z), 0.0f, fabs(local_40.z), fVar9);
					this->scalarDynB.BuildFromSpeedDistTime(fabs(local_60.x), 0.0f, fabs(local_40.x), fVar9);
					this->scalarDynJump.Reset();
					SetState(0x1f, -1);

					return;
				}
			}
			else {
				if ((bVar8 == 4) && ((this->hitFlags & 0x10U) == 0)) {
					if (this->distanceToGround <= 0.3f) {
						SetState(0x18, -1);
					}
					else {
						SetState(0x1b, -1);
					}

					if (bPlayImpact != true) {
						return;
					}

					IMPLEMENTATION_GUARD_FX(
					PlayImpactFx(this, (long)(int)&this->field_0x690, &this->field_0x6a0, (ulong)((this->hitFlags & 1U) != 0),
						'\x01');)

					return;
				}
			}
		}
	}

	SetLookingAtOff();

	if ((1.0f <= this->hitDamage) && (this->typeID == ACTOR_HERO_PRIVATE)) {
		pCVar5 = GetInputManager(1, 0);
		if (pCVar5 != (CPlayerInput*)0x0) {
			CPlayerInput::FUN_001b66f0(1.0f, 0.0f, 0.03f, 0.0f, &pCVar5->field_0x1c, 0);
		}
	}

	LifeDecrease(this->hitDamage);
	this->hitDamage = 0.0f;

	if ((this->hitFlags & 1U) == 0) {
		if ((GetStateFlags(this->actorState) & 0x100) != 0) {
			iVar4 = this->actorState;
			bVar3 = true;
			uVar7 = iVar4 - 0x49;
			bVar2 = false;

			if ((uVar7 < 0x20) && ((1 << (uVar7 & 0x1f) & 0xf000003fU) != 0)) {
				bVar2 = true;
			}

			if ((!bVar2) && (6 < iVar4 - 0x69U)) {
				bVar3 = false;
			}

			if ((!bVar3) || (this->distanceToGround <= 0.4)) {
				if ((this->hitFlags & 8U) != 0) {
					iVar4 = this->actorState;
					bVar2 = true;
					if ((iVar4 != 0x52) && (iVar4 != 0x53)) {
						bVar2 = false;
					}

					if (!bVar2) {
						SetState(0x52, -1);
						goto LAB_00319fb0;
					}
				}

				SetState(FIGHTER_HIT_STEP_BACK, -1);
				goto LAB_00319fb0;
			}
		}
	}

	if ((this->hitFlags & 1U) == 0) {
		this->field_0x6b0 = 4.0f;
		this->field_0x684 = 1;
		this->field_0x7a0 = g_xVector;
		this->field_0x7b4 = 0.0;
		pCVar1 = this->field_0x634;
		if (pCVar1 != (CActor*)0x0) {
			this->field_0x6a0 = pCVar1->rotationQuat;
			(this->field_0x6a0).y = 0.3f;
			this->field_0x6b0 = 8.0f;
		}
	}

	SetBehaviour(FIGHTER_BEHAVIOUR_PROJECTED, -1, -1);

LAB_00319fb0:
	if (bPlayImpact == true) {
		IMPLEMENTATION_GUARD_FX(
		PlayImpactFx(this, (long)(int)&this->field_0x690, &this->field_0x6a0, (ulong)((this->hitFlags & 1U) != 0), '\0');)
	}

	return;
}

void CActorFighter::_Std_GetPossibleExit()
{
	CBehaviourFighter* pCVar2;
	int iVar3;

	if ((GetStateFlags(this->actorState) & 0x100000) != 0) {
		if ((Func_0x1a8() == 0) && ((this->fightFlags & 1) == 0)) {
			pCVar2 = reinterpret_cast<CBehaviourFighter*>(GetBehaviour(this->curBehaviourId));
			SetBehaviour(pCVar2->behaviourId, -1, -1);
		}
		else {
			if ((this->actorState - 0x17U < 5) || (AcquireAdversary(), this->pAdversary != (CActorFighter*)0x0)) {
				if ((this->fightFlags & 2) != 0) {
					iVar3 = FUN_0030a6a0();
					if (iVar3 != -1) {
						SetBehaviour(iVar3, -1, -1);
					}
				}
			}
			else {
				pCVar2 = reinterpret_cast<CBehaviourFighter*>(GetBehaviour(this->curBehaviourId));
				SetBehaviour(pCVar2->behaviourId, -1, -1);
			}
		}
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighter.h
void CActorFighter::_Std_OnFightActionSuccess()
{
	return;
}

CActorFighter* CActorFighter::_Std_GetCaughtAdversary()
{
	IMPLEMENTATION_GUARD();
}

void CActorFighter::_StateFighterRun(CActorsTable* pTable)
{
	_ManageFighterDyn(0x1d, 0x1002023b, pTable);

	if (this->dynamic.speed != 0.0) {
		RunInternal(this->adversaryAngleDiff, &this->dynamic.rotationQuat);
	}

	if (this->scalarDynA.IsFinished() != false) {
		SetState(6, this->standAnim);
	}
	return;
}

void CActorFighter::_Ride_GetPossibleHit(bool bPlayImpact)
{
	IMPLEMENTATION_GUARD();
}

// Should be in: D:/Projects/b-witch/ActorFighterRide.cpp
void CActorFighter::_Ride_GetPossibleExit()
{
	byte bVar1;
	int iVar2;
	CActorFighter* pCVar3;
	CCollision* pCVar4;
	StateConfig* pSVar5;
	CBehaviour* pCVar6;
	uint uVar7;
	undefined4 local_110[2];
	undefined4 local_108;
	undefined4 local_104;
	undefined4 local_100;
	edF32VECTOR4 local_f0;
	float local_e0;
	undefined2 local_c0;
	float local_b0;
	float local_ac;
	float local_a8;
	float local_a4;
	undefined4 local_a0;
	undefined4 local_90[2];
	undefined4 local_88;
	undefined4 local_84;
	undefined4 local_80;
	edF32VECTOR4 local_70;
	float local_60;
	undefined2 local_40;
	float local_30;
	undefined4 local_2c;
	float local_28;
	undefined4 local_24;
	undefined4 local_20;
	undefined4* local_c;
	undefined4* local_8;
	undefined4* local_4;

	if ((GetStateFlags(this->actorState) & 0x100000) != 0) {
		pCVar3 = this->field_0x354;

		if (((pCVar3->fightFlags & 0x200) != 0) || (bVar1 = pCVar3->pCollisionData->flags_0x4, (bVar1 & 2) == 0 && (0.3f < pCVar3->timeInAir))) {
			IMPLEMENTATION_GUARD(
			local_90[0] = 7;
			local_88 = 1;
			local_40 = 1;
			local_30 = this->rotationQuat.z;
			local_2c = 0;
			local_28 = -this->rotationQuat.x;
			local_24 = 0;
			local_70.x = this->rotationQuat.x;
			local_70.y = -1.0f;
			local_70.z = this->rotationQuat.z;
			local_70.w = 0.0;
			edF32Vector4NormalizeHard(&local_70, &local_70);
			local_60 = this->field_0x504;
			local_4 = local_90;
			local_20 = 0x40000000;
			local_84 = 0;
			local_80 = 0;
			CActor::DoMessage((CActor*)this, (CActor*)this->field_0x354, 2, (uint)local_4);
			pCVar6 = CActor::GetBehaviour((CActor*)this, this->curBehaviourId);
			(*(this->pVTable)->SetBehaviour)((CActor*)this, (int)pCVar6[2].pVTable, -1, -1);)
		}
		else {
			if ((bVar1 & 1) != 0) {
				IMPLEMENTATION_GUARD(
				local_110[0] = 7;
				local_108 = 1;
				local_c0 = 1;
				local_e0 = this->field_0x504;
				local_b0 = g_xVector.x;
				local_ac = g_xVector.y;
				local_a8 = g_xVector.z;
				local_a4 = g_xVector.w;
				local_a0 = 0;
				local_104 = 0;
				local_100 = 0;
				pCVar4 = this->pOwner->pCollisionData;
				local_f0.x = pCVar4->aCollisionContact[0].location.x;
				local_f0.z = pCVar4->aCollisionContact[0].location.z;
				local_f0.w = pCVar4->aCollisionContact[0].location.w;
				local_f0.y = pCVar4->aCollisionContact[0].location.y + 0.1;
				edF32Vector4NormalizeHard(&local_f0, &local_f0);
				local_c = local_110;
				local_8 = local_c;
				CActor::DoMessage((CActor*)this, (CActor*)this->field_0x354, 2, (uint)local_c);
				pCVar4 = this->pOwner->pCollisionData;
				local_f0.x = pCVar4->aCollisionContact[0].location.x;
				local_f0.z = pCVar4->aCollisionContact[0].location.z;
				local_f0.w = pCVar4->aCollisionContact[0].location.w;
				local_f0.y = pCVar4->aCollisionContact[0].location.y + 0.4;
				edF32Vector4NormalizeHard(&local_f0, &local_f0);
				CActor::DoMessage((CActor*)this, (CActor*)this, 2, (uint)local_c);)
			}
		}
	}

	return;
}

void CActorFighter::_Hold_GetPossibleHit(bool bPlayImpact)
{
	IMPLEMENTATION_GUARD();
}

// Should be in: D:/Projects/b-witch/ActorFighterSlave.cpp
void CActorFighter::_Hold_GetPossibleExit()
{
	int iVar1;
	CActorFighter* pCVar2;
	bool bVar3;
	bool bVar4;
	StateConfig* pSVar5;
	edF32VECTOR4* v1;
	uint uVar6;
	undefined4 local_a0[2];
	undefined4 local_98;
	undefined4 local_94;
	undefined4 local_90;
	edF32VECTOR4 eStack128;
	undefined4 local_70;
	undefined2 local_50;
	undefined2 local_4e;
	edF32VECTOR4 local_20;
	undefined4* local_4;

	if ((GetStateFlags(this->actorState) & 0x100000) != 0) {
		pCVar2 = this->field_0x354;
		bVar3 = false;
		local_20 = pCVar2->currentLocation;

		uVar6 = this->field_0x354->fightFlags;
		bVar4 = false;
		if ((uVar6 & 0x400) == 0) {
			if ((0.8f < fabs(this->currentLocation.y - local_20.y)) &&
				(bVar3 = true, 0.5f < this->field_0x354->distanceToGround)) {
				bVar4 = true;
			}
		}
		else {
			if (((uVar6 & 0x100) != 0) && (0.3f < fabs(this->currentLocation.y - local_20.y))) {
				bVar3 = true;
				bVar4 = true;
			}
		}

		if (bVar3) {
			if (bVar4) {
				IMPLEMENTATION_GUARD(
				local_a0[0] = 7;
				local_98 = 1;
				v1 = GetAdversaryHeldPos();
				edF32Vector4SubHard(&eStack128, v1, &this->currentLocation);
				edF32Vector4NormalizeHard(&eStack128, &eStack128);
				local_4e = 0;
				local_70 = 0x40800000;
				local_94 = 0;
				local_50 = 0xe;
				local_90 = 0;
				local_4 = local_a0;
				CActor::DoMessage((CActor*)this, (CActor*)this->field_0x354, 2, (uint)local_4);)
			}

			SetState(0x37, -1);
		}
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterSlave.cpp
void CActorFighter::_BeginFighterHold()
{
	IMPLEMENTATION_GUARD();
}

void CActorFighter::_EndFighterHold()
{
	IMPLEMENTATION_GUARD();
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_Proj_GetPossibleExit()
{
	int iVar1;

	iVar1 = this->actorState;
	if (iVar1 != -1) {
		GetStateCfg(iVar1);
	}

	return;
}

void CActorFighter::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
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
			pValue->field_0xc = 0x3f800000;
			pValue->field_0x10 = 0;
		}
		else {
			pValue->field_0xc = 0;
			pValue->field_0x10 = 0x3f800000;
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
				pValue->field_0xc = (int)this->field_0x4ec;
			})
		}
		else {
			if ((newAnim == 0x4d) || (newAnim == 0x38)) {
				IMPLEMENTATION_GUARD(
				peVar1 = pAnimator->pAnimKeyTableEntry;
				if (peVar1->keyIndex_0x8 == 4) {
					peVar1[2].field_0x4 = (int)this->field_0x4e4;
					pValue->field_0x18 = (int)this->field_0x4e8;
					fVar6 = this->field_0x4e0;
					if (0.0 <= fVar6) {
						pValue->field_0x14 = (int)fVar6;
						pValue->field_0x10 = 0;
					}
					else {
						pValue->field_0x10 = (int)-fVar6;
						pValue->field_0x14 = 0;
					}
				})
			}
			else {
				if (((newAnim == 0x27) || (newAnim == 0x18)) || (newAnim == 0x16)) {
					peVar1 = pAnimator->pAnimKeyTableEntry;
					if (peVar1->keyIndex_0x8.asKey == 2) {
						char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
						AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);
						pValue->field_0x10 = this->field_0x4fc;
						pValue->field_0xc = 1.0f - this->field_0x4fc;
					}
				}
				else {
					if ((newAnim == 0x74) || (newAnim == 0x73)) {
						char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
						AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

						local_c = pAnimator->pAnimKeyTableEntry;
						if (local_c->keyIndex_0x8.asKey == 5) {
							if ((this->flags & 0x1000) == 0) {
								GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
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

							if (0.0f <= fVar6) {
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
								GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
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
								char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
								AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

								peVar1 = pAnimator->pAnimKeyTableEntry;
								if ((this->fightFlags & 4) == 0) {
									fVar6 = this->field_0x4e0.x;
									if (0.0f <= fVar6) {
										pValue->field_0x1c = fVar6;
										pValue->field_0x20 = 0.0f;
										pValue->field_0xc = 0.0f;
										pValue->field_0x10 = 0.0f;
									}
									else {
										pValue->field_0x10 = -fVar6;
										pValue->field_0xc = 0.0f;
										pValue->field_0x1c = 0.0f;
										pValue->field_0x20 = 0.0f;
									}

									fVar6 = this->field_0x4e0.z;
									if (0.0f <= fVar6) {
										pValue->field_0x18 = fVar6;
										pValue->field_0x14 = 0.0f;
										pValue->field_0x24 = 0.0f;
										pValue->field_0x28 = 0.0f;
									}
									else {
										pValue->field_0x24 = -fVar6;
										pValue->field_0x28 = 0.0f;
										pValue->field_0x14 = 0.0f;
										pValue->field_0x18 = 0.0f;
									}
								}
								else {
									fVar6 = this->field_0x4e0.x;
									if (0.0f <= fVar6) {
										pValue->field_0x20 = fVar6;
										pValue->field_0x1c = 0.0f;
										pValue->field_0xc = 0.0f;
										pValue->field_0x10 = 0.0f;
									}
									else {
										pValue->field_0xc = -fVar6;
										pValue->field_0x10 = 0.0f;
										pValue->field_0x1c = 0.0f;
										pValue->field_0x20 = 0.0f;
									}

									fVar6 = this->field_0x4e0.z;
									if (0.0f <= fVar6) {
										pValue->field_0x14 = fVar6;
										pValue->field_0x18 = 0.0f;
										pValue->field_0x24 = 0.0f;
										pValue->field_0x28 = 0.0f;
									}
									else {
										pValue->field_0x28 = -fVar6;
										pValue->field_0x24 = 0.0f;
										pValue->field_0x14 = 0.0f;
										pValue->field_0x18 = 0.0f;
									}
								}
							}
							else {
								CActor::AnimEvaluate(layerId, pAnimator, newAnim);
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
	this->field_0x354 = (CActorFighter*)0x0;

	this->field_0x36c = 1;
	this->adversaryDistance = 0.0f;
	this->field_0x374 = 0;
	this->adversaryAngleDiff = 0.0f;

	this->scalarDynA.Reset();
	this->scalarDynB.Reset();
	this->scalarDynJump.Reset();

	this->field_0x3f4 = 1.0f;
	this->field_0x478 = 0.0f;

	this->field_0x4a0 = gF32Vertex4Zero;
	this->field_0x4b0 = gF32Vertex4Zero;
	this->field_0x4e0 = gF32Vector4Zero;

	this->field_0x44cuint = 0;
	this->field_0x450.all = 0;
	this->field_0x474 = 0;
	this->field_0x634 = (CActor*)0x0;

	this->field_0x738 = 1;
	this->field_0x6a0 = g_xVector;
	
	this->field_0x690 = gF32Vertex4Zero;

	this->field_0x6b0 = 0.0f;
	this->hitDamage = 0.0f;
	this->field_0x684 = 0;
	IMPLEMENTATION_GUARD_FIGHT(
	this->field_0x7d8 = 0x3db851ec;)
	this->field_0x6c4 = 0.1f;
	this->field_0x6c8 = 1.0f;
	this->field_0x6cc = 1.0f;
	this->field_0x7e4 = 0;
	this->field_0x7f0 = gF32Vector4UnitZ;

	this->fighterAnatomyZones.field_0x0 = this->fighterAnatomyZones.field_0x10;

	this->pBlow = 0;
	this->field_0x840 = (s_fighter_blow*)0;
	this->field_0x834 = (s_fighter_blow*)0;
	this->field_0x860 = 0;
	this->field_0x864 = (s_fighter_blow*)0x0;
	this->comboFlags = 0xffffffff;

	this->pFighterCombo = (s_fighter_combo*)0x0;

	this->field_0x630 = 0;
	this->nbFightCollisions = 0;
	IMPLEMENTATION_GUARD_FIGHT(
	this[1].characterBase.base.base.field_0x13c = 0;
	this[1].characterBase.base.base.aActorSounds = (CActorSound*)0x0;
	*(undefined4*)&this[1].characterBase.base.base.field_0x144 = 0;
	this[1].characterBase.base.base.field_0x140 = 0;
	this[1].characterBase.base.base.field_0x14c = 0x3f800000;
	this[1].characterBase.base.base.field_0x15c = 0x3dcccccd;
	this[1].characterBase.base.dynamic.rotationQuat.x = 0.1;
	*(undefined2*)&this[1].characterBase.base.dynamic.rotationQuat.y = 3;)
	this->field_0x47c = 23.56194f;
	this->field_0x4c0 = 1.75f;
	this->field_0x4c4 = 2.5f;
	this->field_0x4c8 = 1.178097f;
	this->field_0x4cc = 4.0f;
	this->field_0x4f4 = 1.2f;
	this->field_0x4f8 = 17.0f;
	IMPLEMENTATION_GUARD_FIGHT(
	this->field_0x508 = 0x40600000;)
	this->field_0x504 = 7.0f;
	IMPLEMENTATION_GUARD_FIGHT(
	this->field_0x500 = 0;)
	this->standAnim = -1;
	return;
}

bool CActorFighter::Execute(s_fighter_action* pAction, s_fighter_action_param* pParams)
{
	int iVar1;
	bool bVar2;
	StateConfig* pSVar3;
	uint uVar4;
	s_fighter_action local_4;

	if ((this->fightFlags & 1) == 0) {
		_ValidateCommand(pAction, &local_4);

		iVar1 = this->actorState;
		uVar4 = GetStateFlags(this->actorState);
		uVar4 = uVar4 & 0xff800;
		if (uVar4 == 0x8000) {
			IMPLEMENTATION_GUARD(
			(*(code*)(this->pVTable)->_Execute_Hold)(&local_4, pParams);)
		}
		else {
			if (uVar4 == 0x4000) {
				IMPLEMENTATION_GUARD(
				(*(code*)(this->pVTable)->_Execute_Ride)(&local_4, pParams);)
			}
			else {
				if (uVar4 == 0x2000) {
					IMPLEMENTATION_GUARD(
					(*(code*)(this->pVTable)->_Execute_Flip)(&local_4, pParams);)
				}
				else {
					if (uVar4 == 0x800) {
						_Execute_Std(&local_4, pParams);
					}
				}
			}
		}
		bVar2 = local_4.all != 0x0;
	}
	else {
		bVar2 = false;
	}
	return bVar2;
}

void CActorFighter::RunInternal(float param_1, edF32VECTOR4* pRotation)
{
	float fVar1;
	float fVar2;
	float puVar3;
	float puVar4;
	float puVar5;
	float puVar6;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	edF32Matrix4RotateYHard(param_1, &eStack64, &gF32Matrix4Unit);
	edF32Matrix4TransposeHard(&eStack64);
	edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, pRotation);

	if (1.0f < local_50.z) {
		puVar6 = 1.0f;
	}
	else {
		puVar6 = -1.0f;
		if (-1.0f <= local_50.z) {
			puVar6 = local_50.z;
		}
	}

	fVar2 = acosf(puVar6);
	puVar4 = 1.0f;
	(this->field_0x4d0).z = 1.0f - fVar2 / 1.570796f;
	if (local_50.x < 0.0f) {
		puVar4 = -1.0f;
	}

	puVar3 = 1.0f;
	(this->field_0x4d0).x = puVar4 * (1.0f - fabs((this->field_0x4d0).z));
	fVar2 = (this->field_0x4d0).x - (this->field_0x4e0).x;
	fVar1 = (this->field_0x4d0).z - (this->field_0x4e0).z;

	if (fabs(fVar2) <= 0.1f) {
		(this->field_0x4e0).x = (this->field_0x4d0).x;
	}
	else {
		if (fVar2 < 0.0f) {
			puVar3 = -1.0f;
		}

		(this->field_0x4e0).x = (this->field_0x4e0).x + puVar3 * 0.1f;
	}

	if (fabs(fVar1) <= 0.1f) {
		(this->field_0x4e0).z = (this->field_0x4d0).z;
	}
	else {
		if (0.0f <= fVar1) {
			puVar5 = 1.0f;
		}
		else {
			puVar5 = -1.0f;
		}

		(this->field_0x4e0).z = (this->field_0x4e0).z + puVar5 * 0.1f;
	}

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
		this->adversaryDistance = 0.0f;
		this->adversaryAngleDiff = 0.0f;
	}
	else {
		pCVar1 = this->pAdversary;
		if (pNewAdversary != pCVar1) {
			if (pCVar1 != (CActor*)0x0) {
				this->pAdversary->pAnimationController->UnRegisterBone(0x45544554);
			}

			this->pAdversary = pNewAdversary;
			this->pAdversary->pAnimationController->RegisterBone(0x45544554);

			v1 = GetAdversaryPos();
			edF32Vector4SubHard(&eStack16, v1, &this->currentLocation);
			eStack16.y = 0.0f;
			fVar2 = edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
			this->adversaryDistance = fVar2;
			if (fVar2 != 0.0f) {
				fVar2 = GetAngleYFromVector(&eStack16);
				this->adversaryAngleDiff = fVar2;
			}
		}
	}

	return;
}

void CActorFighter::_ManageFighterDyn(uint flags, uint dynFlags, CActorsTable* pTable)
{
	undefined* puVar1;
	edF32VECTOR4* peVar2;
	bool bVar3;
	undefined* puVar5;
	edF32VECTOR4* peVar6;
	byte bVar8;
	float puVar11;
	uint uVar12;
	float fVar13;
	float puVar9;
	float puVar10;
	float fVar14;
	float t;
	edF32VECTOR4 eStack256;
	edF32VECTOR4 local_f0;
	edF32MATRIX4 eStack224;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 newDynRotQuat;
	edF32VECTOR4 local_30;
	edF32VECTOR4 toAdversaryNormalized;
	edF32VECTOR4 toAdversary;

	t = GetTimer()->cutsceneDeltaTime;

	memset(&local_30, 0, sizeof(edF32VECTOR4));
	memset(&newDynRotQuat, 0, sizeof(edF32VECTOR4));

	edF32Vector4SubHard(&toAdversary, GetAdversaryPos(), &this->currentLocation);
	toAdversary.y = 0.0f;
	const float adversaryDist = edF32Vector4SafeNormalize0Hard(&toAdversaryNormalized, &toAdversary);
	this->adversaryDistance = adversaryDist;
	if (0.0f != adversaryDist) {
		this->adversaryAngleDiff = GetAngleYFromVector(&toAdversary);
	}

	if ((flags & 8) != 0) {
		float newDynSpeed = 0.0f;

		if ((flags & 1) != 0) {
			if ((this->field_0x36c & 1) == 0) {
				memset(&local_90, 0, sizeof(edF32VECTOR4));
				memset(&local_a0, 0, sizeof(edF32VECTOR4));

				local_90.x = this->rotationQuat.x;
				local_90.z = this->rotationQuat.z;

				local_a0.x = this->rotationQuat.z;
				local_a0.z = -this->rotationQuat.x;

				bVar8 = this->field_0x44c & 0xf;
				if (((bVar8 != 2) && (bVar8 != 1)) && (5 < bVar8)) {
					edF32Vector4GetNegHard(&local_90, &local_90);
				}

				uVar12 = this->field_0x44c & 0xf;
				if (((uVar12 != 3) && (uVar12 != 6)) && (uVar12 % 3 == 1)) {
					edF32Vector4GetNegHard(&local_a0, &local_a0);
				}

				this->scalarDynA.Integrate(t);
				this->scalarDynB.Integrate(t);

				if ((flags & 0x10) != 0) {
					bVar3 = this->scalarDynA.IsFinished();
					if (bVar3 != false) {
						this->scalarDynA.Stop();
					}

					bVar3 = this->scalarDynB.IsFinished();
					if (bVar3 != false) {
						this->scalarDynB.Stop();
					}
				}

				edF32Vector4ScaleHard((this->scalarDynA).field_0x20, &local_90, &local_90);
				edF32Vector4ScaleHard((this->scalarDynB).field_0x20, &local_a0, &local_a0);
				edF32Vector4AddHard(&newDynRotQuat, &local_90, &local_a0);
				edF32Vector4ScaleHard(t, &toAdversaryNormalized, &newDynRotQuat);
				edF32Vector4SubHard(&toAdversaryNormalized, &toAdversary, &toAdversaryNormalized);
			}
			else {
				this->scalarDynA.Integrate(t);

				if ((flags & 0x10) != 0) {
					bVar3 = this->scalarDynA.IsFinished();
					if (bVar3 != false) {
						this->scalarDynA.Stop();
					}
				}

				bVar8 = this->field_0x44c & 0xf;
				fVar13 = (this->scalarDynA).field_0x20 * t;
				if ((this->field_0x44c & 0xf) != 0) {
					if (((bVar8 != 2) && (bVar8 != 1)) && ((bVar8 != 3 && (bVar8 != 6)))) {
						fVar13 = fVar13 / 2.0f;
					}

					bVar8 = this->field_0x44c & 0xf;
					if ((bVar8 != 2) && (bVar8 != 1)) {
						if (bVar8 < 6) {
							this->adversaryDistance = this->adversaryDistance - fVar13;
						}
						else {
							this->adversaryDistance = this->adversaryDistance + fVar13;
						}

						if (this->adversaryDistance < 0.0f) {
							this->adversaryDistance = this->adversaryDistance * -1.0f;
							this->adversaryAngleDiff = this->adversaryAngleDiff + 3.141593f;
						}
					}

					uVar12 = this->field_0x44c & 0xf;
					if (((uVar12 != 3) && (uVar12 != 6)) && (this->adversaryDistance != 0.0f)) {
						if (uVar12 % 3 == 1) {
							this->adversaryAngleDiff = this->adversaryAngleDiff + fVar13 / this->adversaryDistance;
						}
						else {
							this->adversaryAngleDiff = this->adversaryAngleDiff - fVar13 / this->adversaryDistance;
						}

						this->adversaryAngleDiff = edF32Between_2Pi(this->adversaryAngleDiff);
					}

					edF32MATRIX4 advRotationMatrix;
					edF32Matrix4SetIdentityHard(&advRotationMatrix);
					edF32Matrix4RotateYHard(this->adversaryAngleDiff, &advRotationMatrix, &advRotationMatrix);
					edF32Matrix4MulF32Vector4Hard(&toAdversaryNormalized, &advRotationMatrix, &gF32Vector4UnitZ);
					edF32Vector4ScaleHard(this->adversaryDistance, &newDynRotQuat, &toAdversaryNormalized);
					edF32Vector4SubHard(&newDynRotQuat, &toAdversary, &newDynRotQuat);
					edF32Vector4ScaleHard(1.0f / t, &newDynRotQuat, &newDynRotQuat);
				}
			}

			newDynSpeed = edF32Vector4SafeNormalize0Hard(&newDynRotQuat, &newDynRotQuat);
		}

		if (((flags & 2) != 0) && (((flags & 0x20) == 0 || (bVar3 = this->scalarDynJump.IsFinished(), bVar3 == false)))) {
			this->scalarDynJump.Integrate(t);

			if ((flags & 0x20) != 0) {
				bVar3 = this->scalarDynJump.OnLastValidSample();
				if (bVar3 != false) {
					this->scalarDynJump.Stop();
				}
			}

			this->dynamicExt.field_0x60.y = (this->scalarDynJump).field_0x20 * this->field_0x3f4;
			this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x8000;
		}

		this->dynamic.rotationQuat = newDynRotQuat;
		this->dynamic.speed = newDynSpeed;
	}

	if (((flags & 4) != 0) && (((toAdversaryNormalized.x != 0.0f || (toAdversaryNormalized.y != 0.0f)) || (toAdversaryNormalized.z != 0.0f)))) {
		edF32Vector4NormalizeHard(&local_f0, &toAdversaryNormalized);

		puVar9 = edF32Vector4DotProductHard(&local_f0, &this->rotationQuat);
		if (1.0f < puVar9) {
			puVar11 = 1.0f;
		}
		else {
			puVar11 = -1.0f;
			if (-1.0f <= puVar9) {
				puVar11 = puVar9;
			}
		}

		fVar13 = acosf(puVar11);
		edF32Vector4CrossProductHard(&eStack256, &g_xVector, &this->rotationQuat);
		fVar14 = edF32Vector4DotProductHard(&local_f0, &eStack256);
		if (0.0f <= fVar14) {
			puVar10 = 1.0f;
		}
		else {
			puVar10 = -1.0f;
		}

		fVar13 = edF32Between_Pi(fVar13 * puVar10);
		fVar13 = fVar13 / t;
		if (this->field_0x47c < fabs(fVar13)) {
			if (0.0f <= fVar13) {
				puVar11 = 1.0f;
			}
			else {
				puVar11 = -1.0f;
			}
			fVar13 = this->field_0x47c * puVar11;
		}

		edF32Matrix4RotateYHard(fVar13 * t, &eStack224, &gF32Matrix4Unit);
		edF32Matrix4MulF32Vector4Hard(&local_f0, &eStack224, &this->rotationQuat);
		edF32Vector4NormalizeHard(&local_f0, &local_f0);
		this->rotationQuat = local_f0;
	}

	ManageDyn(4.0f, dynFlags, pTable);

	return;
}

void CActorFighter::_ComputeLogicalPos(edF32VECTOR4* pPosition)
{
	int iVar1;
	StateConfig* pSVar3;
	uint uVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	this->logicalPosition = this->currentLocation;

	iVar1 = this->curBehaviourId;
	if (iVar1 == 6) {
		if (((GetStateFlags(this->actorState) & 0xff800) == 0x8000) && ((this->fightFlags & 0x400) != 0)) {
			edF32Vector4AddHard(&this->logicalPosition, &this->currentLocation, &this->fighterAnatomyZones.field_0x0);
			(this->logicalPosition).xyz = (this->logicalPosition).xyz;
			(this->logicalPosition).w = 1.0f;
		}
	}
	else {
		if (iVar1 == 4) {
			iVar1 = this->actorState;
			if (((iVar1 != FIGHTER_PROJECTED_HIT_WAKING_UP) && (iVar1 != 0x62)) && (iVar1 != 99)) {
				edF32Vector4AddHard(&this->logicalPosition, &this->currentLocation, &this->fighterAnatomyZones.field_0x0);
				(this->logicalPosition).xyz = (this->logicalPosition).xyz;
				(this->logicalPosition).w = 1.0f;
			}
		}
		else {
			uVar4 = GetStateFlags(this->actorState) & 0xff800;
			if (uVar4 == 0x800) {
				if ((this->fightFlags & 0x1000) != 0) {
					this->logicalPosition = this->field_0x4a0;
				}
			}
			else {
				if (uVar4 == 0x8000) {
					if (this->field_0x354 != (CActorFighter*)0x0) {
						if (pPosition == (edF32VECTOR4*)0x0) {
							this->logicalPosition = this->field_0x354->currentLocation;
						}
						else {
							edF32Vector4SubHard(&eStack16, pPosition, &this->currentLocation);
							edF32Vector4SubHard(&eStack32, &this->field_0x354->currentLocation, &this->currentLocation);
							edF32Vector4NormalizeHard(&eStack16, &eStack16);
							edF32Vector4NormalizeHard(&eStack32, &eStack32);
							fVar5 = edF32Vector4DotProductHard(&eStack16, &eStack32);
							if (0.81f <= fVar5) {
								this->logicalPosition = this->field_0x354->currentLocation;
							}
						}
					}
				}
				else {
					if (uVar4 == 0x4000) {
						this->logicalPosition = this->field_0x354->currentLocation;
					}
					else {
						if (uVar4 == 0x2000) {
							this->logicalPosition = this->pAdversary->currentLocation;
						}
					}
				}
			}
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

void CActorFighter::SetInitialState()
{
	if ((0.4f < this->distanceToGround) || ((this->fightFlags & 0x4000) != 0)) {
		if (this->pAdversary != (CActorFighter*)0x0) {
			this->fightFlags = this->fightFlags & 0xfffffffd;
			this->field_0x478 = 0.0f;
			this->field_0x4f0 = this->pAdversary->currentLocation.y;
		}

		if (((validCommandMask.flags[0] >> 2) & 1) == 0) {
			this->field_0x684 = 1;
			this->hitFlags = 4;
			this->field_0x6b0 = 0.0f;
			SetBehaviour(FIGHTER_BEHAVIOUR_PROJECTED, -1, -1);
		}
		else {
			if ((this->field_0x444 & 1) == 0) {
				SetState(0xd, -1);
			}
			else {
				SetState(0xe, -1);
			}
		}
	}
	else {
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;

		this->dynamicExt.field_0x6c = 0.0f;
		SetState(6, this->standAnim);
	}

	this->fightFlags = this->fightFlags & 0xffffbfff;

	return;
}

void CActorFighter::SetFightBehaviour()
{
	SetBehaviour(FIGHTER_BEHAVIOUR_DEFAULT, -1, -1);
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

		if ((this->hitFlags & 0x40U) != 0) {
			this->field_0x7b4 = 0.0;
			this->hitFlags = this->hitFlags & 0xffffffbf;
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
		edF32Matrix4FromAngAxisSoft(-fVar9, &eStack352, &eStack288);
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

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
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
	edF32Matrix4FromAngAxisSoft(-this->field_0x7b0, &eStack128, &this->field_0x7a0);
	edF32Matrix4MulF32Matrix4Hard(&eStack128, &local_40, &eStack128);

	_SV_DYN_SetRotationAroundMassCenter(&eStack128);

	pCVar1 = this->pCollisionData;
	if ((pCVar1->flags_0x4 & 2) == 0) {
		this->field_0x7b4 = 0.0f;
		this->field_0x7b0 = 0.0f;
		if (0.15f < this->timeInAir) {
			edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");
			edF32Matrix4MulF32Vector4Hard(&this->fighterAnatomyZones.field_0x0, &this->field_0x760, &(this->fighterAnatomyZones).field_0x10);
			SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
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

	this->hitFlags = 2;
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

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitFly()
{
	int iVar2;
	float fVar3;
	float fVar5;
	edF32MATRIX4 eStack336;
	CActorsTable local_110;

	local_110.nbEntries = 0;
	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &this->field_0x740, &this->dynamicExt.normalizedTranslation);

	this->field_0x7d0 = (this->fighterAnatomyZones).field_0x0.y + this->currentLocation.y;

	ManageDyn(4.0f, 0x8000120, &local_110);

	this->field_0x7b0 = this->field_0x7b0 + this->field_0x7b4 * GetTimer()->cutsceneDeltaTime;
	fVar3 = this->field_0x7b4;
	this->field_0x7b4 = fVar3 - FLOAT_00448b04 * fVar3 * fVar3 * GetTimer()->cutsceneDeltaTime * GetTimer()->cutsceneDeltaTime;

	edF32Matrix4FromAngAxisSoft(-this->field_0x7b0, &eStack336, &this->field_0x7a0);
	edF32Matrix4MulF32Matrix4Hard(&eStack336, &this->field_0x760, &eStack336);
	_SV_DYN_SetRotationAroundMassCenter(&eStack336);

	this->field_0x8e4 = _SV_HIT_ProcessActorsCollisions(0.0f, &this->field_0x8f0, &this->field_0x900, &this->field_0x740, &local_110, &this->actorsExcludeTable, true, 6, 0);

	if ((((this->pCollisionData)->flags_0x4 & 7) == 0) && (this->field_0x8e4 == 0)) {
		this->field_0x750 = this->field_0x740;
	}
	else {
		_InterpretCollisions(0);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitWakingUpInit()
{
	edF32VECTOR4* v0;
	float puVar3;
	float puVar4;
	float fVar2;
	edF32VECTOR4 eStack256;
	edF32VECTOR4 eStack240;
	edF32MATRIX4 eStack224;
	edF32MATRIX4 eStack160;
	edF32VECTOR4 eStack96;
	edF32VECTOR3 local_50;
	edF32MATRIX4 eStack64;

	edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");
	edF32Matrix4FromAngAxisSoft(-(this->field_0x7dc * 1.570796f), &eStack64, &this->field_0x760.rowX);
	edF32Matrix4MulF32Matrix4Hard(&this->field_0x760, &this->field_0x760, &eStack64);
	edF32Matrix4ToEulerSoft(&this->field_0x760, &local_50, "XYZ");
	this->rotationEuler.xyz = local_50;

	SV_GetBoneWorldPosition(0x53538f8b, &eStack96);
	ComputeRotTransMatrix(&eStack160);
	ComputeLocalMatrix(&eStack224, &eStack160);
	edF32Matrix4MulF32Vector4Hard(&eStack256, &eStack224, this->field_0x810 + ((int)this->field_0x7dc + 1 >> 1));
	edF32Vector4SubHard(&eStack240, &eStack96, &eStack256);
	v0 = &this->currentLocation;
	edF32Vector4AddHard(v0, v0, &eStack240);
	edF32Vector4CrossProductHard(&this->field_0x7a0, &(this->field_0x760).rowY, &g_xVector);
	edF32Vector4SafeNormalize0Hard(&this->field_0x7a0, &this->field_0x7a0);

	puVar3 = edF32Vector4DotProductHard(&(this->field_0x760).rowY, &g_xVector);
	if (1.0f < puVar3) {
		puVar4 = 1.0f;
	}
	else {
		puVar4 = -1.0f;
		if (-1.0f <= puVar3) {
			puVar4 = puVar3;
		}
	}

	fVar2 = acosf(puVar4);
	this->field_0x7b4 = fVar2;
	this->field_0x7b0 = 0.0f;

	int macroAnim = GetIdMacroAnim(this->currentAnimType);

	if (macroAnim < 0) {
		fVar2 = 0.0f;
	}
	else {
		fVar2 = this->pAnimationController->GetAnimLength(macroAnim, 1);
	}

	this->field_0x7b4 = this->field_0x7b4 / fVar2;

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
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

	local_120.nbEntries = 0;
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
		uVar4 = _Stack16.hitMaterialFlags & 0xf;
		if (uVar4 == 0) {
			uVar4 = CScene::_pinstance->defaultMaterialIndex;
		}

		if (CCollisionManager::_material_table[uVar4].field_0x8 <= fabs(eStack352.y)) {
			uVar4 = _Stack16.hitMaterialFlags & 0xf;
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
				edF32Matrix4FromAngAxisSoft(-(this->field_0x7b0 * (1.0f - fVar5)), &local_1c0, &this->field_0x7a0);
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
	edF32Matrix4FromAngAxisSoft(-this->field_0x7b0, &eStack528, &this->field_0x7a0);
	edF32Matrix4MulF32Matrix4Hard(&eStack528, &this->field_0x760, &eStack528);
	_SV_DYN_SetRotationAroundMassCenter(&eStack528);
	this->field_0x7dc = 0.0f;
LAB_00306b50:
	
	this->field_0x8e4 = _SV_HIT_ProcessActorsCollisions(0.0f, &this->field_0x8f0, &this->field_0x900, &this->field_0x740, &local_120, &this->actorsExcludeTable, 1, 6, 0);
	if ((((this->pCollisionData)->flags_0x4 & 2) == 0) || (this->field_0x7dc == 0.0f)) {
		if (((((this->pCollisionData)->flags_0x4 & 7) == 0) && (this->field_0x8e4 == 0)) ||
			(this->timeInAir <= 0.15f)) {
			this->field_0x750 = this->field_0x740;
			if ((((this->hitFlags & 4U) == 0) && (15.0f < this->dynamicExt.field_0x6c)) &&
				(this->dynamicExt.normalizedTranslation.y < 0.0f)) {
				SetState(FIGHTER_PROJECTED_HIT_FLY, -1);
			}
		}
		else {
			_InterpretCollisions(0);
		}
	}
	else {
		edProjectVectorOnPlane(0.0f, &this->field_0x740, &this->field_0x740, &eStack352, 0);
		iVar3 = _SV_ANM_GetTwoSidedAnim(0x71, (int)this->field_0x7dc);
		SetState(FIGHTER_PROJECTED_HIT_SLIDE, iVar3);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
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

	if (pCVar1->IsCurrentLayerAnimEndReached(0)) {
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

void CActorFighter::_StateFighterHitStepBackInit(int animationId, int param_3)
{
	int iVar1;
	float time;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	s_fighter_action local_4;

	time = _GetFighterAnimationLength(animationId);

	if (param_3 != 0) {
		this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(5.0f, 0);
	}

	PlayAnim(animationId);
	edF32Vector4SubHard(&local_30, &this->currentLocation, &this->field_0x634->currentLocation);
	local_30.y = 0.0f;
	edF32Vector4NormalizeHard(&local_30, &local_30);
	_BuildCommandFromAbsoluteVector(&local_30, &local_4, &eStack32);
	this->field_0x44c = this->field_0x44c & 0xf0 | local_4.field_0x0 & 0xf;
	this->field_0x36c = this->field_0x36c & 0xfffffffe;
	edF32Vector4ScaleHard(this->field_0x6b0 * 0.040625f, &local_30, &eStack32);
	edF32Vector4ScaleHard(this->field_0x6b0, &local_40, &eStack32);
	this->scalarDynA.BuildFromSpeedDistTime(fabs(local_40.z), 0.0f, fabs(local_30.z), time);
	this->scalarDynB.BuildFromSpeedDistTime(fabs(local_40.x), 0.0f, fabs(local_30.x), time);
	this->scalarDynJump.Reset();

	return;
}

void CActorFighter::_StateFighterHitStepBack(int nextState, int animationId, int param_4)
{
	bool bVar1;
	int iVar4;
	float fVar5;
	float fVar6;
	CAnimation* pAnim;

	_ManageFighterDyn(0x19, 0x1006023b, (CActorsTable*)0x0);

	if (this->scalarDynA.IsFinished() != false) {
		this->field_0x36c = this->field_0x36c | 1;

		fVar5 = GetLifeInterface()->GetValue();
		if (fVar5 <= 0.0f) {
			SetBehaviour(FIGHTER_BEHAVIOUR_PROJECTED, FIGHTER_PROJECTED_HIT_STAGGER_CHECK, -1);
		}
		else {
			if (param_4 != 0) {
				pAnim = this->pAnimationController;
				if (animationId == -1) {
					iVar4 = GetIdMacroAnim(GetStateCfg(nextState)->animId);
				}
				else {
					iVar4 = CActor::GetIdMacroAnim(animationId);
				}

				if (iVar4 < 0) {
					fVar5 = 0.0f;
				}
				else {
					fVar5 = pAnim->GetAnimLength(iVar4, 1);
					fVar6 = pAnim->GetAnimLength(iVar4, 2);
					fVar5 = fVar5 - fVar6;
				}

				pAnim->anmBinMetaAnimator.SetLayerTimeWarper(fVar5 / this->field_0x474, 0);
			}

			SetState(nextState, animationId);
		}
	}

	return;
}

float CActorFighter::_StateFighterFightActionDynInit(_s_fighter_blow_stage* pStage)
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float speed;
	edF32VECTOR4 eStack16;

	this->field_0x6c8 = this->field_0x6cc;
	this->field_0x6cc = 1.0f;

	fVar2 = pStage->field_0x8;
	fVar4 = this->field_0x6c8;
	fVar5 = pStage->field_0x4 * fVar4;

	fVar3 = _GetFighterAnimationLength(pStage->animId);
	speed = 1.0f;
	if (0.0f < fVar3 / fVar5) {
		speed = fVar3 / fVar5;
	}

	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar5, 0);

	if ((pStage->flags & 0x40) != 0) {
		this->field_0x36c = this->field_0x36c & 0xfffffffe;
		edF32Vector4ScaleHard(1.0f / speed, &eStack16, &pStage->field_0x10);
		_SetRelativeSpeedOnGround(speed, &eStack16);
	}

	if ((pStage->flags & 0x80) != 0) {
		fVar5 = (pStage->field_0x10).y;

		if (0.0f <= fVar5) {
			this->scalarDynJump.BuildFromDistTimeNoAccel(fVar5, speed);
			this->field_0x3f4 = 1.0f;
		}
		else {
			this->scalarDynJump.BuildFromDistTimeNoAccel(-fVar5, speed);
			this->field_0x3f4 = -1.0f;
		}
	}

	this->field_0x47c = fVar2 * fVar4 * 23.56194f;

	return speed;
}

void CActorFighter::_StateFighterExecuteBlow(int nextStateA, int nextStateB, int param_4)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	void* pcVar3;
	bool bVar4;
	uint uVar5;
	uint uVar6;
	s_fighter_collision_desc* psVar7;
	s_fighter_collision_desc local_80[2];

	_ManageFighterDyn((this->pBlow->blowStageExecute).flags, 0x40323, (CActorsTable*)0x0);

	uVar5 = _SV_HIT_FightCollisionsGet(local_80, 2, 1);
	uVar6 = 0;
	if (uVar5 != 0) {
		psVar7 = local_80;
		do {
			_SV_HIT_FightCollisionProcessHit(psVar7->pActor, this->pBlow, &psVar7->field_0x10, param_4);
			uVar6 = uVar6 + 1;
			psVar7 = psVar7 + 1;
		} while (uVar6 < uVar5);
	}

	pCVar1 = this->pAnimationController;

	if (pCVar1->IsCurrentLayerAnimEndReached(0)) {
		if ((this->fightFlags & 0x40) != 0) {
			_Std_OnFightActionSuccess();
		}

		pcVar3 = this->pBlow->field_0xd0;
		if (pcVar3 != (void*)0x0) {
			IMPLEMENTATION_GUARD(
			(*pcVar3)(this, 1);)
		}

		if (this->field_0x860 == 2) {
			this->field_0x860 = 0;
			IMPLEMENTATION_GUARD(
			this->field_0x840 = this->field_0x864;
			this->field_0x474 = this->field_0x840->field_0xc;
			SetState(0x6b, (int)(this->field_0x840->blowStageBegin).field_0x10.x);)
		}
		else {
			if (this->field_0x860 == 1) {
				this->field_0x860 = 0;
				this->pBlow = this->field_0x864;
				this->field_0x474 = this->pBlow->field_0xc;
				SetState(nextStateB, (this->pBlow->blowStageBegin).animId);
			}
			else {
				this->field_0x6cc = this->field_0x6c8;
				SetState(nextStateA, (this->pBlow->blowStageEnd).animId);
			}
		}
	}

	return;
}

float CActorFighter::_pStagger_check[8] = {
	2.3561945f,
	1.5707964f,
	0.7853982f,
	0.0f,
	3.1415927f,
	3.926991f,
	4.712389f,
	5.4977875f,
};

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitStaggerCheck()
{
	float fVar1;
	float f0;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	if ((this->fightFlags & 0x20) == 0) {
		if (this->field_0x7e4 < 8) {
			edF32Matrix4RotateYHard(_pStagger_check[this->field_0x7e4], &eStack64, &gF32Matrix4Unit);
			edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, &this->field_0x7f0);
			edF32Vector4AddHard(&local_60, &this->currentLocation, &(this->fighterAnatomyZones).field_0x10);
			local_60.w = 1.0f;
			f0 = ((this->pCollisionData)->pObbPrim->field_0x90).y * 2.0f * 1.1f;
			CCollisionRay CStack128 = CCollisionRay(f0, &local_60, &local_50);
			fVar1 = CStack128.Intersect(3, (CActor*)this, (CActor*)0x0, 0x7fffffff, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
			if (fVar1 == 1e+30f) {
				edF32Vector4ScaleHard(f0, &local_50, &local_50);
				edF32Vector4AddHard(&local_60, &local_60, &local_50);
				local_50.y = -1.0f;
				local_50.x = 0.0f;
				local_50.z = 0.0f;
				local_50.w = 0.0f;
				CStack128.ChangeOrigin(&local_60);
				CStack128.ChangeLeadVector(&local_50);
				fVar1 = CStack128.Intersect(3, (CActor*)this, (CActor*)0x0, 0x7fffffff, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);

				if (fVar1 == 1e+30) {
					this->field_0x7e4 = this->field_0x7e4 + 1;
				}
				else {
					this->fightFlags = this->fightFlags | 0x20;
				}
			}
			else {
				this->field_0x7e4 = this->field_0x7e4 + 1;
			}
		}
		else {
			this->fightFlags = this->fightFlags | 0x20;
			this->field_0x7e4 = 0;
		}
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if ((this->fightFlags & 0x20) != 0) {
		SetState(0x5b, -1);
	}
	return;
}

edF32VECTOR4 edF32VECTOR4_00425760 = { 0.0f, -1.0f, 0.0f, 0.0f };

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitStaggerFallInit(float param_1)
{
	edColPRIM_OBJECT* peVar1;
	StateConfig* pSVar2;
	int iVar3;
	float fVar4;
	float puVar5;
	float puVar6;
	float fVar7;
	float fVar8;
	float t;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	_ray_info_out _Stack16;

	local_50 = edF32VECTOR4_00425760;
	peVar1 = this->pCollisionData->pObbPrim;
	t = (peVar1->field_0x90).y;
	fVar8 = (peVar1->field_0x90).z;
	this->field_0x7dc = param_1;
	SetVectorFromAngleY(this->rotationEuler.y, &eStack64);
	edF32Vector4ScaleHard(this->field_0x7dc, &eStack64, &eStack64);
	edF32Vector4ScaleHard(-t, &local_30, &eStack64);
	edF32Vector4AddHard(&local_30, &local_30, &this->currentLocation);
	edF32Vector4AddHard(&local_30, &local_30, &(this->fighterAnatomyZones).field_0x10);
	local_30.w = 1.0f;
	CCollisionRay CStack112 = CCollisionRay(t * 2.0f, &local_30, &local_50);
	CStack112.Intersect(3, (CActor*)this, (CActor*)0x0, 0x40000008, &eStack32, &_Stack16);
	pSVar2 = GetStateCfg(this->actorState);
	fVar4 = _GetFighterAnimationLength(pSVar2->animId);
	edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");
	edF32Vector4CrossProductHard(&this->field_0x7a0, &eStack64, &eStack32);
	edF32Vector4SafeNormalize0Hard(&this->field_0x7a0, &this->field_0x7a0);
	puVar5 = edF32Vector4DotProductHard(&eStack64, &eStack32);
	if (1.0f< puVar5) {
		puVar6 = 1.0f;
	}
	else {
		puVar6 = -1.0f;
		if (-1.0f <= puVar5) {
			puVar6 = puVar5;
		}
	}

	fVar7 = acosf((float)puVar6);
	this->field_0x7b0 = fVar7;
	this->field_0x7b4 = fVar7 / fVar4;
	edF32Vector4ScaleHard(fVar8, &eStack64, &eStack32);
	edF32Vector4CrossProductHard(&local_50, &eStack32, &this->field_0x7a0);
	edF32Vector4NormalizeHard(&local_50, &local_50);
	edF32Vector4ScaleHard(t, &local_50, &local_50);
	edF32Vector4AddHard(&local_30, &local_30, &local_50);
	edF32Vector4SubHard(&this->field_0x740, &local_30, &this->currentLocation);
	edF32Vector4ScaleHard(1.0f / fVar4, &this->field_0x740, &this->field_0x740);
	fVar8 = edF32Vector4NormalizeHard(&this->field_0x740, &this->field_0x740);
	this->dynamic.speed = fVar8;
	this->dynamic.rotationQuat = this->field_0x740;

	return;
}

void CActorFighter::_StateFighter_0xaInit()
{
	CCollision* pCVar1;
	int iVar2;
	byte bVar3;
	float time;
	CAnimation* pAnim;

	pAnim = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim(0x16);
	if (iVar2 < 0) {
		time = 0.0f;
	}
	else {
		time = pAnim->GetAnimLength(iVar2, 1);
	}

	pAnim->anmBinMetaAnimator.SetLayerTimeWarper(time / (this->scalarDynJump).duration, 0);

	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffffcf;
	this->field_0x4f0 = this->currentLocation.y;
	this->fightFlags = this->fightFlags & 0xfffffffd;

	bVar3 = this->field_0x44c & 0xf;
	if ((((this->field_0x44c & 0xf) == 0) || (bVar3 == 2)) || (bVar3 == 1)) {
		this->field_0x4fc = 0.5f;
	}
	else {
		if (bVar3 < 6) {
			this->field_0x4fc = 0.0f;
		}
		else {
			this->field_0x4fc = 1.0f;
		}
	}

	return;
}

void CActorFighter::_StateFighter_0xbInit(float dist)
{
	CCollision* pCVar1;
	int iVar2;
	byte bVar3;
	float time;
	CAnimation* pAnim;

	pAnim = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim(0x16);
	if (iVar2 < 0) {
		time = 0.0f;
	}
	else {
		time = pAnim->GetAnimLength(iVar2, 1);
	}

	pAnim->anmBinMetaAnimator.SetLayerTimeWarper(time / (this->scalarDynJump).duration, 0);

	this->field_0x36c = this->field_0x36c & 0xfffffffe;
	this->scalarDynA.BuildFromDistTime(dist, time);
	this->field_0x44c = this->field_0x44c & 0xf0 | 3;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffffcf;
	this->field_0x4f0 = this->currentLocation.y;
	this->fightFlags = this->fightFlags & 0xfffffffd;

	bVar3 = this->field_0x44c & 0xf;
	if ((((this->field_0x44c & 0xf) == 0) || (bVar3 == 2)) || (bVar3 == 1)) {
		this->field_0x4fc = 0.5f;
	}
	else {
		if (bVar3 < 6) {
			this->field_0x4fc = 0.0f;
		}
		else {
			this->field_0x4fc = 1.0f;
		}
	}

	return;
}

void CActorFighter::_StateFighter_0xb()
{
	CActorFighter::_ManageFighterDyn(0x3b, 0x129, (CActorsTable*)0x0);

	if (this->scalarDynJump.IsFinished() == true) {
		if ((this->field_0x444 & 1) == 0) {
			SetState(0xd, -1);
		}
		else {
			SetState(0xe, -1);
		}
	}

	return;
}

uint CreateHashFromName(char* szInput)
{
	union
	{
		uint fullHash;
		byte bytes[4];
	} local_8;

	local_8.fullHash = 0;

	uint index = 0;
	char curCharacter = *szInput;

	while (curCharacter != '\0') {
		szInput = szInput + 1;
		local_8.bytes[index & 3] = local_8.bytes[(index & 3)] + curCharacter;
		index = index + 1;
		curCharacter = *szInput;
	}

	return local_8.fullHash;
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
	s_fighter_blow_bone_ref* pBoneRef;
	float fVar10;
	edF32VECTOR4 eStack32;
	undefined4 local_4;

	pBoneRef = pBlow->aBoneRefs;
	pcVar2 = pByteCode->GetString();

	FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadBlow {}", pcVar2);

	pBlow->hash.hash = CreateHashFromName(pcVar2);

#ifdef PLATFORM_WIN
	gDebugActionHashToFullName[pBlow->hash.hash] = pcVar2;
#endif

	pBlow->field_0x4.field_0x0byte = pByteCode->GetU32();
	pBlow->field_0x4.field_0x1byte = pByteCode->GetU32();
	pBlow->field_0x4.field_0x2ushort = pByteCode->GetU32();

	pBlow->field_0x8 = pByteCode->GetF32();
	pBlow->field_0xc = pByteCode->GetF32();
	pBlow->field_0x10 = pByteCode->GetF32();
	pBlow->damage = pByteCode->GetF32();
	pBlow->field_0x18 = pByteCode->GetF32();

	pBlow->field_0x20.x = pByteCode->GetF32();
	pBlow->field_0x20.y = pByteCode->GetF32();
	pBlow->field_0x20.z = pByteCode->GetF32();
	pBlow->field_0x20.w = 0.0f;
	
	(pBlow->field_0x30).x = pByteCode->GetF32() * 0.02f;
	(pBlow->field_0x30).y = pByteCode->GetF32() * 0.02f;
	(pBlow->field_0x30).z = pByteCode->GetF32() * 0.02f;
	(pBlow->field_0x30).w = 0.0f;

	edQuatFromEuler(&eStack32, (pBlow->field_0x30).x, (pBlow->field_0x30).y, (pBlow->field_0x30).z);
	edQuatToAngAxis(&eStack32, &pBlow->field_0x1c, (edF32VECTOR3*)&pBlow->field_0x30);
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
			do {
				iVar8 = pBlow->field_0x48 + uVar6;
				iVar8->boneRefA.boneId = pByteCode->GetU32();
				iVar8->boneRefB.boneId = pByteCode->GetU32();
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
			} while (uVar6 < pBlow->nbSubObjs);
		}
	}

	iVar7 = 0;
	uVar6 = 0;
	do {
		uVar4 = pByteCode->GetU32();
		if (uVar4 != 0) {
			pBoneRef->boneId = uVar4;
			iVar7 = iVar7 + 1;
			pBoneRef = pBoneRef + 1;
		}

		uVar6 = uVar6 + 1;
	} while (uVar6 < 2);
	pBlow->nbBoneRefs = iVar7;
	
	pBlow->canActivateRange = pByteCode->GetF32();

	pBlow->blowStageBegin.animId = pByteCode->GetS32();	
	pBlow->blowStageBegin.field_0x4 = pByteCode->GetF32();	
	pBlow->blowStageBegin.field_0x8 = pByteCode->GetF32();	
	pBlow->blowStageBegin.field_0x10.x = pByteCode->GetF32();
	pBlow->blowStageBegin.field_0x10.y = pByteCode->GetF32();
	pBlow->blowStageBegin.field_0x10.z = pByteCode->GetF32();
	pBlow->blowStageBegin.field_0x10.w = 0.0f;
	pBlow->blowStageBegin.flags = pByteCode->GetU32();

	pBlow->blowStageExecute.animId = pByteCode->GetS32();
	pBlow->blowStageExecute.field_0x4 = pByteCode->GetF32();
	pBlow->blowStageExecute.field_0x8 = pByteCode->GetF32();
	pBlow->blowStageExecute.field_0x10.x = pByteCode->GetF32();
	pBlow->blowStageExecute.field_0x10.y = pByteCode->GetF32();
	pBlow->blowStageExecute.field_0x10.z = pByteCode->GetF32();
	pBlow->blowStageExecute.field_0x10.w = 0.0f;
	pBlow->blowStageExecute.flags = pByteCode->GetU32();

	pBlow->blowStageEnd.animId = pByteCode->GetS32();
	pBlow->blowStageEnd.field_0x4 = pByteCode->GetF32();
	pBlow->blowStageEnd.field_0x8 = pByteCode->GetF32();
	pBlow->blowStageEnd.field_0x10.x = pByteCode->GetF32();
	pBlow->blowStageEnd.field_0x10.y = pByteCode->GetF32();
	pBlow->blowStageEnd.field_0x10.z = pByteCode->GetF32();
	pBlow->blowStageEnd.field_0x10.w = 0.0f;
	pBlow->blowStageEnd.flags = pByteCode->GetU32();

	pBlow->field_0xd0 = (void*)0x0;
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

		_SortIncreasing<s_fighter_blow>(this->aBlows, uVar1);
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

	pGrab->hash.hash = CreateHashFromName(pcVar2);

#ifdef PLATFORM_WIN
	gDebugActionHashToFullName[pGrab->hash.hash] = pcVar2;
#endif

	pGrab->field_0x4.field_0x0byte = pByteCode->GetU32();
	pGrab->field_0x4.field_0x1byte = pByteCode->GetU32();
	pGrab->field_0x4.field_0x2ushort = pByteCode->GetU32();

	
	pGrab->field_0x8 = pByteCode->GetF32();
	
	pGrab->field_0xc = pByteCode->GetF32();
	
	pGrab->field_0x10 = pByteCode->GetF32();
	
	pGrab->field_0x14.boneId = pByteCode->GetU32();
	
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
	
	(pGrab->field_0x50).x = pByteCode->GetF32() * 0.02f;
	
	(pGrab->field_0x50).y = pByteCode->GetF32() * 0.02f;
	
	(pGrab->field_0x50).z = pByteCode->GetF32() * 0.02f;
	pGrab->field_0x5c = 0;
	edQuatFromEuler(&eStack32, (pGrab->field_0x50).x, (pGrab->field_0x50).y, (pGrab->field_0x50).z);
	edQuatToAngAxis(&eStack32, &pGrab->field_0x64, &pGrab->field_0x50);
	pGrab->field_0x64 = pGrab->field_0x64 / 0.02f;
	
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

		_SortIncreasing<s_fighter_grab>(this->aGrabs, uVar2);
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
				this->aGrabReacts[uVar7].hash.hash = CreateHashFromName(pcVar5);
				this->aGrabReacts[uVar7].field_0x4 = pByteCode->GetU32();
				this->aGrabReacts[uVar7].field_0x8 = pByteCode->GetU32();
				uVar7 = uVar7 + 1;
				uVar2 = this->nbGrabReacts;
			} while (uVar7 < uVar2);
		}

		_SortIncreasing<s_fighter_grab_react>(this->aGrabReacts, uVar2);
	}

	return;
}

uint UINT_ARRAY_00425710[12] =
{
	0x2, 0x8, 0x1, 0x4,
	0x3, 0x6, 0x9, 0xC,
	0xA, 0x0, 0x0, 0x0,
};

#define ACTION_FLAG_COMBO_ROOT 0x200

int CActorFighter::_MarkCombosRoots(s_fighter_combo* aCombos, int nbCombos)
{
	bool bVar1;
	int nbComboRoots;
	uint uVar3;
	s_fighter_combo* psVar4;
	s_fighter_combo* psVar5;
	s_fighter_combo* psVar6;

	psVar5 = aCombos + nbCombos;
	nbComboRoots = 0;
	psVar6 = aCombos;

	if (aCombos != psVar5) {
		do {
			bVar1 = false;
			psVar4 = aCombos;
			while ((psVar4 != psVar5 && (!bVar1))) {
				if (psVar4 != psVar6) {
					uVar3 = 0;
					while ((uVar3 < psVar4->nbBranches && (!bVar1))) {
						if (psVar6->hash.hash == psVar4->aBranches[uVar3].hash) {
							bVar1 = true;
						}

						uVar3 = uVar3 + 1;
					}
				}

				psVar4 = psVar4 + 1;
			}

			if (bVar1) {
				psVar6->field_0x4.field_0x0ushort = psVar6->field_0x4.field_0x0ushort & 0xfdff;
			}
			else {
				nbComboRoots = nbComboRoots + 1;
				psVar6->field_0x4.field_0x0ushort = psVar6->field_0x4.field_0x0ushort | ACTION_FLAG_COMBO_ROOT;
			}

			psVar6 = psVar6 + 1;
		} while (psVar6 != psVar5);
	}

	return nbComboRoots;
}

void CActorFighter::_LoadCombo(s_fighter_combo* pCombo, ByteCode* pByteCode)
{
	char* pcVar2;
	uint uVar3;
	uint uVar5;
	uint uVar6;

	pcVar2 = pByteCode->GetString();
	FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadCombo Root {}", pcVar2);

	pCombo->hash.hash = CreateHashFromName(pcVar2);

#ifdef PLATFORM_WIN
	gDebugActionHashToFullName[pCombo->hash.hash] = pcVar2;
#endif

	pCombo->field_0x4.field_0x0ushort = pByteCode->GetU32();
	pCombo->field_0x4.field_0x2ushort = pByteCode->GetU32();
	pCombo->field_0x8 = pByteCode->GetF32();
	pCombo->pattern.field_0x0uint = pCombo->pattern.field_0x0uint & 0xfff00000;
	pCombo->pattern.nbInputs = pByteCode->GetU32();

	FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadCombo pattern field_0x0uint 0x{:x}", pCombo->pattern.field_0x0uint);
	FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadCombo pattern field_0x4 0x{:x}", pCombo->pattern.nbInputs);

	uVar5 = 0;
	if (pCombo->pattern.nbInputs != 0) {
		uVar6 = 0;
		do {
			uVar3 = pByteCode->GetU32();
			uVar5 = uVar5 + 1;
			pCombo->pattern.field_0x0uint = pCombo->pattern.field_0x0uint & 0xfff00000 |
				((pCombo->pattern.field_0x0uint & 0xfffffU |
					((ulong)(UINT_ARRAY_00425710[uVar3] << (uVar6 & 0x1f)) & 0xffffU) << 0x2c) >> 0x2c);
			uVar6 = uVar6 + 4;
		} while (uVar5 < pCombo->pattern.nbInputs);
	}

	pCombo->pattern.field_0x2ushort = pCombo->pattern.field_0x2ushort & 0xf00f | (ushort)((pByteCode->GetU32() & 0xff) << 4);
	pCombo->pattern.field_0x3byte = pCombo->pattern.field_0x3byte & 0xf | (byte)((pByteCode->GetU32() & 0xf) << 4);
	pCombo->field_0xc = pByteCode->GetF32();

	pcVar2 = pByteCode->GetString();
	FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadCombo Action {}", pcVar2);
	
	pCombo->actionHash.hash = CreateHashFromName(pcVar2);
	pCombo->nbBranches = pByteCode->GetU32();

	if (pCombo->nbBranches != 0) {
		pCombo->aBranches = reinterpret_cast<s_fighter_action_hash*>(edMemAlloc(TO_HEAP(H_MAIN), pCombo->nbBranches << 2));
		uVar5 = 0;
		if (pCombo->nbBranches != 0) {
			do {
				pcVar2 = pByteCode->GetString();
				FIGHTER_LOG(LogLevel::Info, "CActorFighter::_LoadCombo ---- Branch {}", pcVar2);
				pCombo->aBranches[uVar5].hash = CreateHashFromName(pcVar2);
				uVar5 = uVar5 + 1;
			} while (uVar5 < pCombo->nbBranches);
		}
	}

	return;
}

void CActorFighter::_CreateCombosDB(ByteCode* pByteCode)
{
	uint comboCount = pByteCode->GetU32();
	if (comboCount == 0) {
		this->aCombos = (s_fighter_combo*)0x0;
		this->nbCombos = 0;
		this->nbComboRoots = 0;
	}
	else {
		this->aCombos = (s_fighter_combo*)edMemAlloc(TO_HEAP(H_MAIN), comboCount * sizeof(s_fighter_combo));;
		uint curComboIndex = 0;
		if (comboCount != 0) {
			do {
				_LoadCombo(this->aCombos + curComboIndex, pByteCode);
				curComboIndex = curComboIndex + 1;
			} while (curComboIndex < comboCount);
		}

		this->nbComboRoots = _MarkCombosRoots(this->aCombos, comboCount);
		this->nbCombos = comboCount - this->nbComboRoots;
		_SortIncreasing<s_fighter_combo>(this->aCombos, comboCount);
	}

	return;
}

uint CActorFighter::_SV_HIT_GetHitZoneFromImpact(s_fighter_anatomy_zones* pAnatomyZones, edF32VECTOR4* pHitPosition, edF32VECTOR4* pActorPosition)
{
	uint uVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, pHitPosition, pActorPosition);
	fVar2 = edF32Vector4SafeNormalize0Hard(&eStack32, &pAnatomyZones->field_0x0);
	fVar3 = edF32Vector4DotProductHard(&eStack32, &eStack16);
	uVar1 = 2;

	if (pAnatomyZones->field_0x20 <= fVar3 / fVar2) {
		uVar1 = pAnatomyZones->field_0x24 < fVar3 / fVar2 ^ 1;
	}

	return uVar1;
}

// Should be in: D:/Projects/b-witch/ActorFighterServices.cpp
void CActorFighter::_SV_HIT_FightCollisionsBegin()
{
	uint curColIndex;
	s_fighter_fight_collision* pCurFightCollision;

	pCurFightCollision = this->aFightCollisions;

	if ((this->pBlow->field_0x4.field_0x0byte & 8U) == 0) {
		this->pAnimationController->RegisterBone(0x53538f8b);
		this->fightFlags = this->fightFlags | 0x80;
	}

	curColIndex = 0;
	if (this->nbFightCollisions != 0) {
		do {
			pCurFightCollision->pActor->pAnimationController->RegisterBone(pCurFightCollision->boneId);

			this->aFightCollisions[curColIndex].prevBonePosition.x = 0.0f;
			this->aFightCollisions[curColIndex].prevBonePosition.y = 0.0f;
			this->aFightCollisions[curColIndex].prevBonePosition.z = 0.0f;
			this->aFightCollisions[curColIndex].prevBonePosition.w = 1.0f;

			this->aFightCollisions[curColIndex].curBonePosition.x = 0.0f;
			this->aFightCollisions[curColIndex].curBonePosition.y = 0.0f;
			this->aFightCollisions[curColIndex].curBonePosition.z = 0.0f;
			this->aFightCollisions[curColIndex].curBonePosition.w = 1.0f;

			pCurFightCollision = pCurFightCollision + 1;
			curColIndex = curColIndex + 1;
		} while (curColIndex < this->nbFightCollisions);
	}

	return;
}


bool CActorFighter::_SV_HIT_FightCollisionCheckIntersect(s_fighter_collision_desc* pCollisionDesc, edF32VECTOR4* param_3, edF32VECTOR4* param_4, uint kindMask)
{
	bool bVar1;
	bool bIntersection;
	float f0;
	float intersectAlpha;
	edF32VECTOR4 local_20;
	_ray_info_out rayInfo;

	bIntersection = false;

	edF32Vector4SubHard(&local_20, param_4, param_3);
	f0 = edF32Vector4SafeNormalize1Hard(&local_20, &local_20);
	if (f0 == 0.0f) {
		f0 = 1e-20f;
	}

	CCollisionRay colRay = CCollisionRay(f0, param_3, &local_20);
	intersectAlpha = colRay.IntersectActors(this, (CActor*)0x0, 0x7fffffff, (edF32VECTOR4*)0x0, &rayInfo);
	if ((intersectAlpha != 1e+30f) && (rayInfo.pActor_0x0->IsKindOfObject(kindMask) != false)) {
		pCollisionDesc->pActor = rayInfo.pActor_0x0;

		bIntersection = true;

		pCollisionDesc->field_0x30 = f0 / GetTimer()->cutsceneDeltaTime;
		pCollisionDesc->field_0x10 = *param_4;
		pCollisionDesc->field_0x20 = local_20;
	}

	return bIntersection;
}



uint CActorFighter::_SV_HIT_FightCollisionsGet(s_fighter_collision_desc* aCollisionDescs, uint param_3, uint kindMask)
{
	bool bVar1;
	uint nbCollisions;
	uint uVar2;
	s_fighter_collision_desc* pCollisionDesc;
	s_fighter_fight_collision* pCollision;
	int iVar4;
	edF32VECTOR4 eStack16;

	uVar2 = 0;
	nbCollisions = 0;
	pCollision = this->aFightCollisions;
	while ((uVar2 < this->nbFightCollisions && (nbCollisions < param_3))) {
		bVar1 = _SV_HIT_FightCollisionCheckIntersect(aCollisionDescs + nbCollisions, &pCollision->prevBonePosition, &pCollision->curBonePosition, kindMask);
		if (bVar1 == false) {
			if ((this->pBlow->field_0x4.field_0x0byte & 8U) == 0) {
				CActor::SV_GetBoneWorldPosition(0x53538f8b, &eStack16);
				bVar1 = _SV_HIT_FightCollisionCheckIntersect(aCollisionDescs + nbCollisions, &eStack16, &pCollision->curBonePosition, kindMask);
				if (bVar1 != false) {
					nbCollisions = nbCollisions + 1;
				}
			}
		}
		else {
			nbCollisions = nbCollisions + 1;
		}

		pCollision = pCollision + 1;
		uVar2 = uVar2 + 1;
	}

	if ((this->pBlow->field_0x4.field_0x0byte & 0x10U) != 0) {
		uVar2 = 0;
		iVar4 = 0;
		pCollisionDesc = aCollisionDescs + nbCollisions;
		pCollision = this->aFightCollisions;
		while ((uVar2 < this->nbFightCollisions - 1 && (nbCollisions < param_3))) {
			IMPLEMENTATION_GUARD(
			bVar1 = _SV_HIT_FightCollisionCheckIntersect(this, pCollisionDesc, &pCollision->curBonePosition, &this->aFightCollisions[1].curBonePosition + iVar4, kindMask);)
			if (bVar1 != false) {
				pCollisionDesc = pCollisionDesc + 1;
				nbCollisions = nbCollisions + 1;
			}

			iVar4 = iVar4 + 0x30;
			pCollision = pCollision + 1;
			uVar2 = uVar2 + 1;
		}
	}

	if ((this->pBlow->field_0x4.field_0x0byte & 0x20U) != 0) {
		uVar2 = 0;
		iVar4 = 0;
		pCollisionDesc = aCollisionDescs + nbCollisions;
		pCollision = this->aFightCollisions;
		while ((uVar2 < this->nbFightCollisions - 1 && (nbCollisions < param_3))) {
			IMPLEMENTATION_GUARD(
			bVar1 = _SV_HIT_FightCollisionCheckIntersect(this, pCollisionDesc, &pCollision->curBonePosition, &this->aFightCollisions[1].prevBonePosition + iVar4, kindMask);)
			if (bVar1 != false) {
				pCollisionDesc = pCollisionDesc + 1;
				nbCollisions = nbCollisions + 1;
			}
			iVar4 = iVar4 + 0x30;
			pCollision = pCollision + 1;
			uVar2 = uVar2 + 1;
		}
		uVar2 = 0;
		iVar4 = 0;
		pCollisionDesc = aCollisionDescs + nbCollisions;
		pCollision = this->aFightCollisions;
		while ((uVar2 < this->nbFightCollisions - 1 && (nbCollisions < param_3))) {
			IMPLEMENTATION_GUARD(
			bVar1 = _SV_HIT_FightCollisionCheckIntersect(this, pCollisionDesc, &pCollision->prevBonePosition, &this->aFightCollisions[1].curBonePosition + iVar4, kindMask);)
			if (bVar1 != false) {
				pCollisionDesc = pCollisionDesc + 1;
				nbCollisions = nbCollisions + 1;
			}
			iVar4 = iVar4 + 0x30;
			pCollision = pCollision + 1;
			uVar2 = uVar2 + 1;
		}
	}

	return nbCollisions;
}

// Should be in: D:/Projects/b-witch/ActorFighterServices.cpp
void CActorFighter::_SV_HIT_FightCollisionsEnd()
{
	s_fighter_fight_collision* psVar1;
	uint uVar2;

	psVar1 = this->aFightCollisions;
	if ((this->fightFlags & 0x80) != 0) {
		this->pAnimationController->UnRegisterBone(0x53538f8b);
		this->fightFlags = this->fightFlags & 0xffffff7f;
	}

	uVar2 = 0;
	if (this->nbFightCollisions != 0) {
		do {
			psVar1->pActor->pAnimationController->UnRegisterBone(psVar1->boneId);
			uVar2 = uVar2 + 1;
			psVar1 = psVar1 + 1;
		} while (uVar2 < this->nbFightCollisions);
	}

	this->nbFightCollisions = 0;

	return;
}


void CActorFighter::_SV_HIT_FightCollisionProcessHit(CActor* pHitActor, s_fighter_blow* pBlow, edF32VECTOR4* pPosition, int param_5)
{
	int iVar1;
	bool bVar2;
	uint uVar3;
	long lVar4;
	s_fighter_hit_exclusion* pCVar5;
	int iVar6;
	float angle;
	_msg_hit_param hitMsg;
	edF32MATRIX4 rotMatrix;

	bVar2 = this->actorsExcludeTable.IsInList(pHitActor);

	if (!bVar2) {
		hitMsg.projectileType = 7;
		hitMsg.field_0x10 = pBlow->field_0x10 - this->timeInAir;
		hitMsg.field_0x30 = pBlow->field_0x18;
		hitMsg.field_0x70 = pBlow->field_0x1c;
		hitMsg.damage = pBlow->damage;
		hitMsg.field_0x50 = 1;
		hitMsg.flags = pBlow->field_0x4.field_0x2ushort;

		if (Func_0x1b0(pHitActor) == 0) {
			hitMsg.flags = hitMsg.flags & 0xfffffff8;
		}

		if (param_5 == 1) {
			hitMsg.field_0x30 = hitMsg.field_0x30 * this->hitMultiplier;
			hitMsg.field_0x70 = hitMsg.field_0x70 * this->hitMultiplier;
		}

		hitMsg.damage = hitMsg.damage * this->hitMultiplier;

		if ((this->flags & 0x1000) == 0) {
			angle = GetAngleYFromVector(&this->rotationQuat);
			edF32Matrix4RotateYHard(angle, &rotMatrix, &gF32Matrix4Unit);
		}
		else {
			edF32Matrix4RotateYHard(this->rotationEuler.y, &rotMatrix, &gF32Matrix4Unit);
		}

		edF32Matrix4MulF32Vector4Hard(&hitMsg.field_0x20, &rotMatrix, &pBlow->field_0x20);
		edF32Matrix4MulF32Vector4Hard(&hitMsg.field_0x60, &rotMatrix, &pBlow->field_0x30);
		hitMsg.field_0x40 = *pPosition;

		CActor::DoMessage(pHitActor, MESSAGE_KICKED, &hitMsg);

		bVar2 = pHitActor->IsKindOfObject(OBJ_TYPE_FIGHTER);
		if (bVar2 == true) {
			CActorFighter* pFighter = static_cast<CActorFighter*>(pHitActor);
			uVar3 = pFighter->FUN_0031b4d0(pFighter->actorState);
			if (uVar3 == 0) {
				this->fightFlags = this->fightFlags | 0x40;
			}
		}
		else {
			this->fightFlags = this->fightFlags | 0x40;
		}

		iVar1 = (this->actorsExcludeTable).nbEntries;
		if (iVar1 < 6) {
			(this->actorsExcludeTable).aEntries[iVar1].pActor = pHitActor;
			(this->actorsExcludeTable).aEntries[(this->actorsExcludeTable).nbEntries].maxDuration = -1.0f;
			(this->actorsExcludeTable).aEntries[(this->actorsExcludeTable).nbEntries].currentDuration = 0.0f;
			(this->actorsExcludeTable).nbEntries = (this->actorsExcludeTable).nbEntries + 1;
		}
	}

	return;
}



int CActorFighter::_SV_HIT_ProcessActorsCollisions(float damage, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5, CActorsTable* param_6, CFighterExcludedTable* pExclusionTable, bool param_8, ushort param_9, ushort param_10)
{
	CActor* pReceiver;
	CCollision* pCVar1;
	edColPRIM_OBJECT* peVar2;
	ed_3d_hierarchy_node* peVar3;
	bool bVar4;
	int iVar6;
	float fVar8;
	CActorsTable* local_200;
	int local_1f0;
	edF32VECTOR4 eStack464;
	edF32VECTOR4 eStack448;
	_msg_hit_param local_1b0;
	edF32MATRIX4 local_130;
	edF32VECTOR4 eStack240;
	edF32VECTOR4 local_e0[2];
	_msg_hit_param local_c0;
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
	if (param_6->nbEntries != 0) {
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
		while (local_1f0 < param_6->nbEntries) {
			pReceiver = local_200->aEntries[0];
			local_1f0 = local_1f0 + 1;
			local_200 = (CActorsTable*)local_200->aEntries;
			bVar4 = pReceiver->IsKindOfObject(OBJ_TYPE_FIGHTER);

			if (bVar4 != false) {
				if (pExclusionTable != (CFighterExcludedTable*)0x0) goto code_r0x0030b96c;
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

			local_1b0.projectileType = 8;
			local_1b0.flags = 0;
			local_1b0.field_0x30 = edF32Vector4SafeNormalize0Hard(&local_1b0.field_0x20, param_5);
			pCVar1 = pReceiver->pCollisionData;
			local_1b0.field_0x40 = pCVar1->field_0x90;
			local_1b0.field_0x10 = 0.0f;
			local_1b0.field_0x50 = 0;
			local_1b0.field_0x52 = 0;
			local_1b0.damage = damage;
			DoMessage(pReceiver, MESSAGE_KICKED, &local_1b0);
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
	s_fighter_hit_exclusion* pCVar5 = pExclusionTable->aEntries;
	if (0 < pExclusionTable->nbEntries) {
		do {
			local_18 = pCVar5->pActor;
			//local_14 = pCVar5->field_0x8; UNUSED?
			//local_10 = pCVar5->field_0xc; UNUSED?
			if (local_18 == pReceiver) {
				bVar4 = true;
				goto LAB_0030b9d0;
			}

			iVar6 = iVar6 + 1;
			pCVar5 = pCVar5 + 1;
		} while (iVar6 < pExclusionTable->nbEntries);
	}

	bVar4 = false;

LAB_0030b9d0:
	if (!bVar4) {
	LAB_0030b9e0:
		// Receiver is a CActorFighter
		CActorFighter* pFighterReceiver = static_cast<CActorFighter*>(pReceiver);

		pCVar1 = pFighterReceiver->pCollisionData;
		local_40 = pCVar1->field_0x90;

		edF32Vector4AddHard(&eStack448, &pFighterReceiver->currentLocation, &pFighterReceiver->fighterAnatomyZones.field_0x0);

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
			local_c0.projectileType = 8;
			local_c0.field_0x10 = 0.3f;
			local_c0.field_0x30 = edF32Vector4SafeNormalize0Hard(&local_c0.field_0x20, param_5);
			local_c0.flags = (uint)(this->field_0x7d4 <= local_c0.field_0x30);
			local_c0.field_0x40 = local_40;
			local_c0.field_0x52 = param_10;
			local_c0.damage = damage;
			local_c0.field_0x50 = param_9;
			CActor::DoMessage(pReceiver, MESSAGE_KICKED, &local_c0);

			if ((pExclusionTable != (CFighterExcludedTable*)0x0) && (pExclusionTable->nbEntries < 6)) {
				pExclusionTable->aEntries[pExclusionTable->nbEntries].pActor = pReceiver;
				pExclusionTable->aEntries[pExclusionTable->nbEntries].maxDuration = 1.0f;
				pExclusionTable->aEntries[pExclusionTable->nbEntries].currentDuration = 0.0f;
				pExclusionTable->nbEntries = pExclusionTable->nbEntries + 1;
			}
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

int CActorFighter::_SV_ANM_GetMultiWaysAnim3D(s_fighter_multiways_anim* pAnimation, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
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
		GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
	}

	edF32Matrix4FromEulerSoft(&auStack64, &this->rotationEuler.xyz, "XYZ");

	fVar3 = edF32Vector4DotProductHard(param_3, &auStack64.rowX);
	fVar4 = edF32Vector4DotProductHard(param_3, &auStack64.rowY);
	fVar5 = edF32Vector4DotProductHard(param_3, &auStack64.rowZ);

	if (fabs(fVar5) <= fabs(fVar3)) {
		if (fabs(fVar4) <= fabs(fVar3)) {
			if (0.0f < fVar3) {
				iVar2 = pAnimation->field_0x4 + (pAnimation->field_0x0[0] & 0xf);
				local_50.x = 1.0f;
			}
			else {
				local_50.x = -1.0f;
				iVar2 = pAnimation->field_0x4 + (uint)(((ulong)pAnimation->field_0x0[0] << 0x38) >> 0x3c);
			}
		}
		else {
			if (0.0f < fVar4) {
				iVar2 = pAnimation->field_0x4 + (pAnimation->field_0x0[1] & 0xf);
				local_50.y = 1.0f;
			}
			else {
				local_50.y = -1.0f;
				iVar2 = pAnimation->field_0x4 + (uint)(((ulong)pAnimation->field_0x0[1] << 0x38) >> 0x3c);
			}
		}
	}
	else {
		if (fabs(fVar4) <= fabs(fVar5)) {
			if (0.0f < fVar5) {
				iVar2 = pAnimation->field_0x4 + (pAnimation->field_0x0[2] & 0xf);
				local_50.z = 1.0f;
			}
			else {
				local_50.z = -1.0f;
				iVar2 = pAnimation->field_0x4 + (uint)(((ulong)pAnimation->field_0x0[2] << 0x38) >> 0x3c);
			}
		}
		else {
			if (0.0f < fVar4) {
				iVar2 = pAnimation->field_0x4 + (pAnimation->field_0x0[1] & 0xf);
				local_50.y = 1.0f;
			}
			else {
				local_50.y = -1.0f;
				iVar2 = pAnimation->field_0x4 + (uint)(((ulong)pAnimation->field_0x0[1] << 0x38) >> 0x3c);
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

s_fighter_combo* CActorFighter::FindComboByName(char* szName)
{
	const uint hash = CreateHashFromName(szName);

	s_fighter_combo* pCombo = _FindByHashcode<s_fighter_combo>(this->aCombos, this->nbComboRoots, hash);
	if (pCombo == (s_fighter_combo*)0x0) {
		pCombo = _FindByHashcode<s_fighter_combo>(this->aCombos + this->nbComboRoots, this->nbCombos, hash);
	}

	return pCombo;
}

void CActorFighter::SetFighterCombo(s_fighter_combo* pCombo)
{
	this->pFighterCombo = pCombo;

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterBlowDB.cpp
s_fighter_blow* CActorFighter::FindBlowByName(char* szName)
{
	return _FindByHashcode<s_fighter_blow>(this->aBlows, this->nbBlows, CreateHashFromName(szName));
}

void CActorFighter::SetStandAnim(int newStandAnim)
{
	if ((this->standAnim != newStandAnim) && (this->standAnim = newStandAnim, this->actorState == FIGHTER_DEFAULT_STATE_IDLE)) {
		SetState(FIGHTER_DEFAULT_STATE_IDLE, this->standAnim);
	}

	return;
}

bool CActorFighter::FUN_0031b5d0(int state)
{
	return state - 0x17U < 5;
}

uint gBoneIds_004343a0[2] = { 0x7bafa38c, 0x7bafa389 };

void CActorFighter::FUN_0031a7c0(int inState)
{
	edNODE* pNode;
	long lVar1;
	int iVar2;
	CActorFighter* pCVar3;
	byte alpha;
	uint* puVar4;
	uint uVar5;
	CActorFighter* pCVar6;
	uint* piVar7;
	uint uVar8;
	float fVar9;
	edF32MATRIX4 eStack176;
	edF32VECTOR4 eStack112;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	if (this->actorState - 0x1cU < 4) {
		edF32Matrix4TranslateHard(&eStack176, &gF32Matrix4Unit, &this->field_0x600);
		edF32Matrix4MulF32Matrix4Hard(&eStack176, &eStack176, &this->pMeshTransform->base.transformA);

		IMPLEMENTATION_GUARD(
		(*(code*)(this->staticMeshComponentAdvanced).base.pVTable[1].field_0x4)
			(&this->staticMeshComponentAdvanced, &eStack176);
		lVar1 = (*(code*)((this->staticMeshComponentAdvanced).base.pVTable)->HasMesh)(&this->staticMeshComponentAdvanced);
		if ((lVar1 != 0) && (pNode = (this->staticMeshComponentAdvanced).base.pMeshTransformParent, pNode != (edNODE*)0x0))
		{
			if (DAT_00448b2c < 2.147484e+09) {
				alpha = (byte)(int)DAT_00448b2c;
			}
			else {
				alpha = (byte)(int)(DAT_00448b2c - 2.147484e+09);
			}
			ed3DHierarchyNodeSetAlpha(pNode, alpha);
		})
	}

	uVar8 = 0;
	if (this->field_0x630 != 0) {
		IMPLEMENTATION_GUARD(
		pCVar6 = this;
		do {
			CActor::SV_GetBoneWorldPosition
			((CActor*)(*(CActor**)&pCVar6->field_0x614)->pVTable, (int)(*(CActor**)&pCVar6->field_0x614)->objectId
				, &eStack32);
			CActor::SV_GetBoneWorldPosition
			(*(CActor**)(*(int*)&pCVar6->field_0x614 + 8), *(int*)(*(int*)&pCVar6->field_0x614 + 0xc), &eStack16);
			FUN_00400f10(0, *(int*)&pCVar6->field_0x610, (float*)&eStack32, 0);
			if ((*(uint*)(*(int*)&pCVar6->field_0x610 + 0x2c) & 2) == 0) {
				CAnimation::UnRegisterBone
				(*(CAnimation**)(**(int**)&pCVar6->field_0x614 + 0x28), (*(int**)&pCVar6->field_0x614)[1]);
				CAnimation::UnRegisterBone
				(*(CAnimation**)(*(int*)(*(int*)&pCVar6->field_0x614 + 8) + 0x28),
					*(uint*)(*(int*)&pCVar6->field_0x614 + 0xc));
				uVar5 = uVar8 + 1;
				if (uVar5 < (uint)this->field_0x630) {
					puVar4 = &this->flags + uVar8 * 2;
					do {
						iVar2 = uVar5 - 1;
						uVar5 = uVar5 + 1;
						pCVar3 = (CActorFighter*)((int)this + iVar2 * 8);
						*(uint*)&pCVar3->field_0x610 = puVar4[0x184];
						*(uint*)&pCVar3->field_0x614 = puVar4[0x185];
						puVar4 = puVar4 + 2;
					} while (uVar5 < (uint)this->field_0x630);
				}
				this->field_0x630 = this->field_0x630 + -1;
			}
			else {
				lVar1 = (*(code*)(this->pVTable)->field_0x1b4)(this, inState);
				if ((lVar1 == 0) && (((*(PlayerSubStruct_64**)&pCVar6->field_0x610)->field_0x2c & 0x10) == 0)) {
					FUN_004012a0(0.25, *(PlayerSubStruct_64**)&pCVar6->field_0x610);
				}
			}
			uVar8 = uVar8 + 1;
			pCVar6 = (CActorFighter*)&(pCVar6->characterBase).base.base.flags;
		} while (uVar8 < this->field_0x630);)
	}

	iVar2 = this->actorState;
	if ((iVar2 == FIGHTER_HIT_STEP_BACK) || (iVar2 == 0x18)) {
		fVar9 = GetAngleYFromVector(&this->dynamic.velocityDirectionEuler);
		fVar9 = edF32Between_0_2Pi(fVar9 + 3.141593f);
		uVar8 = 0;

		piVar7 = gBoneIds_004343a0;
		//pCVar6 = this;
		do {
			SV_GetBoneWorldPosition(*piVar7, &eStack112);
			edF32Matrix4RotateYHard(fVar9, &eStack96, &gF32Matrix4Unit);
			edF32Matrix4TranslateHard(&eStack96, &eStack96, &eStack112);

			IMPLEMENTATION_GUARD_FX(
			FUN_001a0600((long)(int)pCVar6->field_0x550, (long)(int)&eStack96, (long)(int)pCVar6->field_0x550);)
			uVar8 = uVar8 + 1;
			piVar7 = piVar7 + 1;
			//pCVar6 = (CActorFighter*)&(pCVar6->characterBase).base.base.actorFieldS;
		} while (uVar8 < 2);
	}

	return;
}

void CActorFighter::FUN_0031aad0(float animLength, s_fighter_blow* pBlow)
{
	int* piVar1;
	bool bVar2;
	int* piVar3;
	CActorFighter* pCVar4;
	uint uVar5;
	int iVar6;
	uint uVar7;

	if ((this->field_0x630 != 0) && (uVar7 = 0, pBlow->nbSubObjs != 0)) {
		IMPLEMENTATION_GUARD(
		iVar6 = 0;
		do {
			uVar5 = 0;
			bVar2 = false;
			pCVar4 = this;
			while ((uVar5 < this->field_0x630 && (!bVar2))) {
				piVar1 = *(int**)&pCVar4->field_0x614;
				piVar3 = (int*)((int)&(pBlow->field_0x48->boneRefA).pActor + iVar6);
				if ((*piVar1 == *piVar3) && (((piVar1[1] == piVar3[1] && (piVar1[2] == piVar3[2])) && (piVar1[3] == piVar3[3])))
					) {
					bVar2 = true;
				}
				else {
					pCVar4 = (CActorFighter*)&(pCVar4->characterBase).base.base.flags;
					uVar5 = uVar5 + 1;
				}
			}
			if (bVar2) {
				FUN_004012a0(animLength, *(PlayerSubStruct_64**)((int)(this->actorsExcludeTable).aEntries + uVar5 * 8 + -0x2c));
			}
			uVar7 = uVar7 + 1;
			iVar6 = iVar6 + 0x24;
		} while (uVar7 < pBlow->nbSubObjs);)
	}

	return;
}

bool CActorFighter::FUN_0031b790(int state)
{
	bool bVar1;
	bool bVar2;

	bVar2 = true;
	bVar1 = false;
	if ((state - 0x49U < 0x20) && ((1 << (state - 0x49U & 0x1f) & 0xf000003fU) != 0)) {
		bVar1 = true;
	}

	if ((!bVar1) && (6 < state - 0x69U)) {
		bVar2 = false;
	}

	return bVar2;
}

uint CActorFighter::FUN_0031b4d0(int state)
{
	bool bVar1;
	StateConfig* pSVar2;
	uint uVar3;

	uVar3 = (uint)((GetStateFlags(state) & 0x2000000) != 0);
	bVar1 = false;
	if ((state - 0x3fU < 7) && ((1 << (state - 0x3fU & 0x1f) & 99U) != 0)) {
		bVar1 = true;
	}

	if (bVar1) {
		uVar3 = uVar3 | 2;
	}

	if (state - 0x17U < 5) {
		uVar3 = uVar3 | 4;
	}

	bVar1 = false;
	if ((state - 0x1cU < 4) && ((1 << (state - 0x1cU & 0x1f) & 0xbU) != 0)) {
		bVar1 = true;
	}

	if (bVar1) {
		uVar3 = uVar3 | 8;
	}

	return uVar3;
}

bool CActorFighter::FUN_0031ac10(float param_1, s_fighter_blow* pBlow)
{
	uint uVar1;
	int* piVar2;
	//CActorFighterVTable* pCVar3;
	bool bVar4;
	bool bVar5;
	int iVar6;
	undefined** ppuVar7;
	//CActorFighterVTable** ppCVar8;
	uint uVar9;
	CActorFighter* pCVar10;
	int* piVar11;
	uint uVar12;
	int iVar13;
	int iVar14;

	iVar13 = 0;
	uVar12 = 0;
	if (pBlow->nbSubObjs != 0) {
		IMPLEMENTATION_GUARD(
		iVar14 = 0;
		do {
			uVar1 = this->field_0x630;
			uVar9 = 0;
			bVar5 = false;
			pCVar10 = this;
			while ((uVar9 < uVar1 && (!bVar5))) {
				piVar2 = *(int**)&pCVar10->field_0x614;
				piVar11 = (int*)((int)&(pBlow->field_0x48->boneRefA).pActor + iVar14);
				if ((*piVar2 != *piVar11) ||
					((((piVar2[1] != piVar11[1] || (piVar2[2] != piVar11[2])) || (piVar2[3] != piVar11[3])) ||
						((piVar2[5] != piVar11[5] || (bVar4 = true, piVar2[4] != piVar11[4])))))) {
					bVar4 = false;
				}
				if (bVar4) {
					bVar5 = true;
				}
				else {
					pCVar10 = (CActorFighter*)&(pCVar10->characterBase).base.base.flags;
					uVar9 = uVar9 + 1;
				}
			}
			if (bVar5) {
				iVar6 = (int)&(pBlow->field_0x48->boneRefA).pActor + iVar14;
				FUN_00401230(param_1, *(undefined4*)(iVar6 + 0x1c),
					*(int*)((int)(this->actorsExcludeTable).aEntries + uVar9 * 8 + -0x2c),
					*(undefined4*)(iVar6 + 0x20));
				iVar13 = iVar13 + 1;
			}
			else {
				if (uVar1 < 4) {
					uVar9 = 0;
					bVar5 = false;
					ppuVar7 = &PTR_ParticleVTable_0048f900;
					while ((uVar9 < 4 && (!bVar5))) {
						if (((uint)ppuVar7[0xb] & 2) == 0) {
							bVar5 = true;
						}
						else {
							ppuVar7 = ppuVar7 + 0x12;
							uVar9 = uVar9 + 1;
						}
					}
					if (bVar5) {
						ppCVar8 = &this->pVTable + uVar1 * 2;
						ppCVar8[0x185] = (CActorFighterVTable*)((int)&(pBlow->field_0x48->boneRefA).pActor + iVar14);
						ppCVar8[0x184] = (CActorFighterVTable*)(&PTR_ParticleVTable_0048f900 + uVar9 * 0x12);
						FUN_00401460(ppCVar8[0x185]->Init, (int)ppCVar8[0x184], ppCVar8[0x185]->Term);
						pCVar3 = ppCVar8[0x185];
						FUN_004012f0(param_1, (int)ppCVar8[0x184], (uint)pCVar3->field_0x18, (int)pCVar3->InitDlistPatchable,
							pCVar3->Create);
						CAnimation::RegisterBone(*(CAnimation**)(ppCVar8[0x185]->actorBase + 0x28), (uint)ppCVar8[0x185]->field_0x4)
							;
						CAnimation::RegisterBone(*(CAnimation**)(ppCVar8[0x185]->free + 0x28), (uint)ppCVar8[0x185]->IsKindOfObject)
							;
						iVar13 = iVar13 + 1;
						this->field_0x630 = this->field_0x630 + 1;
					}
				}
			}

			uVar12 = uVar12 + 1;
			iVar14 = iVar14 + 0x24;
		} while (uVar12 < pBlow->nbSubObjs);)
	}

	return iVar13 != 0;
}

int CActorFighter::FUN_0030a6a0()
{
	int iVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	iVar2 = -1;
	if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
		fVar3 = this->field_0x478 + GetTimer()->cutsceneDeltaTime;
		this->field_0x478 = fVar3;
		if (0.15f < fVar3) {
			iVar2 = 4;
			this->field_0x6a0 = this->dynamic.velocityDirectionEuler;
			this->field_0x6b0 = this->dynamic.linearAcceleration;
			this->field_0x684 = 1;
			this->field_0x7a0 = g_xVector;
			this->field_0x7b4 = 0.0f;
		}
	}
	else {
		this->field_0x478 = 0.0f;
	}

	return iVar2;
}

struct ConditionalExecuteEntry
{
	uint field_0x0;
	float field_0x4;
	int field_0x8;
	undefined4 field_0xc;
};

ConditionalExecuteEntry ConditionalExecuteEntry_ARRAY_004343b0[10]
{
	{ 0x1000, 0.1f, 0x1, 0x0 },
	{ 0x2000, 0.2f, 0x3, 0x0 },
	{ 0x100, 0.1f, 0x3, 0x0 },
	{ 0x400, 0.1f, 0x3, 0x0 },
	{ 0x200, 0.1f, 0x3, 0x0 },
	{ 0x800, 0.1f, 0x1, 0x0 },
	{ 0x20, 0.2f, 0x2, 0x0 },
	{ 0x40, 0.15f, 0x2, 0x0 },
	{ 0x10, 0.1f, 0x0, 0x0 },
	{ 0x80, 0.1f, 0x0, 0x0 },
};

bool CActorFighter::FUN_003175e0(s_fighter_action* pFighterAction, float* param_3)
{
	s_fighter_action sVar1;
	bool bVar2;
	ConditionalExecuteEntry* pCVar3;
	ConditionalExecuteEntry* pCVar4;
	uint uVar5;
	float fVar6;
	uint local_4;

	sVar1 = *pFighterAction;
	fVar6 = 0.0;
	bVar2 = false;
	if ((sVar1.all != 0x0) && (sVar1.all != this->field_0x450.all)) {
		uVar5 = 0;
		local_4 = sVar1.all & 0xfffffff0;
		pCVar3 = ConditionalExecuteEntry_ARRAY_004343b0;
		while ((pCVar4 = pCVar3, uVar5 < 10 && (!bVar2))) {
			pCVar3 = pCVar4 + 1;
			uVar5 = uVar5 + 1;
			if (((pCVar4->field_0x0 & local_4) != 0) && ((pCVar4->field_0x8 & 2U) != 0)) {
				fVar6 = pCVar4->field_0x4;
				bVar2 = true;
			}
		}
	}

	if (param_3 != (float*)0x0) {
		*param_3 = fVar6;
	}

	return bVar2;
}

bool CActorFighter::FUN_0031b7f0(s_fighter_action* pAction, s_fighter_action_param* pActionParam)
{
	edF32VECTOR4* peVar1;
	bool bVar2;
	Timer* pTVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float local_4;

	bVar2 = FUN_003175e0(pAction, &local_4);
	if (bVar2 == false) {
		pTVar3 = GetTimer();
		fVar4 = this->field_0x45c - pTVar3->cutsceneDeltaTime;
		this->field_0x45c = fVar4;
		if (fVar4 <= 0.0f) {
			this->field_0x450.all = 0x0;
		}
		bVar2 = false;
	}
	else {
		this->field_0x450 = *pAction;
		this->field_0x45c = local_4;

		if (pActionParam == (s_fighter_action_param*)0x0) {
			(this->field_0x454).field_0x0 = (edF32VECTOR4*)0x0;
			(this->field_0x454).field_0x4 = 0;
		}
		else {
			peVar1 = pActionParam->field_0x0;
			if (peVar1 == (edF32VECTOR4*)0x0) {
				this->field_0x460 = gF32Vector4Zero;
			}
			else {
				this->field_0x460 = *peVar1;
			}

			(this->field_0x454).field_0x0 = &this->field_0x460;
			(this->field_0x454).pData = pActionParam->pData;
		}

		bVar2 = true;
	}

	return bVar2;
}

bool CActorFighter::FUN_001740a0()
{
	int iVar1;
	bool bVar2;

	iVar1 = this->actorState;
	if (((iVar1 == 0x7c) || (iVar1 == 0x7b)) || (iVar1 == 0x7a)) {
		bVar2 = true;
	}
	else {
		bVar2 = false;
	}

	return bVar2;
}


void CActorFighter::PlayOrientedFx(edF32VECTOR4* param_2, edF32VECTOR4* param_3, uint param_4, int* param_5)
{
	IMPLEMENTATION_GUARD_FX();
}

void CActorFighter::PlayImpactFx(edF32VECTOR4* pPosition, edF32VECTOR4* param_3, int param_4, bool param_5)
{
	uint uVar1;

	if (param_5 == false) {
		uVar1 = _SV_HIT_GetHitZoneFromImpact(&this->fighterAnatomyZones, pPosition, &this->currentLocation);
		if (uVar1 == 0) {
			if (param_4 == 0) {
				uVar1 = this->field_0x524;
			}
			else {
				uVar1 = this->field_0x51c;
			}
		}
		else {
			if (param_4 == 0) {
				uVar1 = this->field_0x528;
			}
			else {
				uVar1 = this->field_0x520;
			}
		}
	}
	else {
		if (param_4 == 0) {
			uVar1 = this->field_0x534;
		}
		else {
			uVar1 = this->field_0x530;
		}
	}

	PlayOrientedFx(pPosition, param_3, uVar1, (int*)0x0);

	return;
}



float CActorFighter::_GetFighterAnimationLength(int animId)
{
	int idMacroAnim = CActor::GetIdMacroAnim(animId);
	if (idMacroAnim < 0) {
		return 0.0f;
	}
	else {
		return this->pAnimationController->GetAnimLength(idMacroAnim, 1);
	}
}

uint FUN_00337d20(edF32VECTOR4* pDir)
{
	bool bVar1;
	uint result;
	float fVar3;
	edF32VECTOR4 eStack16;

	result = 0;

	if (0.35f <= edF32Vector4SafeNormalize0Hard(&eStack16, pDir)) {
		fVar3 = edF32Between_Pi(GetAngleYFromVector(&eStack16));
		bVar1 = false;
		if ((0.0f <= fabs(fVar3)) && (bVar1 = false, fabs(fVar3) <= 1.047198f)) {
			bVar1 = true;
		}
		if (bVar1) {
			result = 1;
		}
		else {
			bVar1 = false;
			if ((2.094395f <= fabs(fVar3)) && (bVar1 = false, fabs(fVar3) <= 3.141593f)) {
				bVar1 = true;
			}

			if (bVar1) {
				result = 4;
			}
		}

		bVar1 = false;
		if ((0.5235988f <= fVar3) && (bVar1 = false, fVar3 <= 2.617994f)) {
			bVar1 = true;
		}

		if (bVar1) {
			result = result | 2;
		}
		else {
			bVar1 = false;
			if ((-2.617994f <= fVar3) && (bVar1 = false, fVar3 <= -0.5235988f)) {
				bVar1 = true;
			}

			if (bVar1) {
				result = result | 8;
			}
		}
	}

	return result;
}

void CActorFighter::_InitBoneRef(s_fighter_blow_bone_ref* pBoneRef)
{
	CAnimation* pWeaponAnim = (CAnimation*)0x0;
	if ((GetWeapon() != (CActorWeapon*)0x0) && (GetWeapon()->pAnimationController != (CAnimation*)0x0)) {
		pWeaponAnim = GetWeapon()->pAnimationController;
	}

	if (this->pAnimationController->IsBonePresent(pBoneRef->boneId) == false) {
		if ((pWeaponAnim != (CAnimation*)0x0) && (pWeaponAnim->IsBonePresent(pBoneRef->boneId) != false)) {
			pBoneRef->pActor = GetWeapon();
		}
	}
	else {
		pBoneRef->pActor = this;
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterBlowDB.cpp
void CActorFighter::_InitBlow(s_fighter_blow* pBlow)
{
	CAnimation* pCVar1;
	bool bVar2;
	CActorWeapon* pCVar4;
	int iVar5;
	CActorFighter* pCVar6;
	uint uVar7;
	s_fighter_blow_bone_ref* pBoneRef;
	CAnimation* pCVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;

	fVar11 = _GetFighterAnimationLength(pBlow->blowStageBegin.animId);
	fVar12 = _GetFighterAnimationLength(pBlow->blowStageExecute.animId);
	fVar13 = _GetFighterAnimationLength(pBlow->blowStageEnd.animId);
	fVar10 = pBlow->blowStageBegin.field_0x4;

	pBoneRef = pBlow->aBoneRefs;
	
	pBlow->field_0x50 = fVar11 / fVar10;
	pBlow->field_0x54 = pBlow->field_0xc + fVar13 / pBlow->blowStageEnd.field_0x4 + fVar11 / pBlow->blowStageBegin.field_0x4 + fVar12 / pBlow->blowStageExecute.field_0x4;
	pBlow->field_0x10 = fVar12 / pBlow->blowStageExecute.field_0x4 + pBlow->field_0x10 * (fVar13 / pBlow->blowStageEnd.field_0x4);
	pBlow->canActivateRange = pBlow->canActivateRange + pBlow->blowStageBegin.field_0x10.z + pBlow->blowStageExecute.field_0x10.z;

	uVar7 = 0;
	if (pBlow->nbBoneRefs != 0) {
		do {
			_InitBoneRef(pBoneRef);
			uVar7 = uVar7 + 1;
			pBoneRef = pBoneRef + 1;
		} while (uVar7 < (uint)pBlow->nbBoneRefs);
	}

	uVar7 = 0;
	if (pBlow->nbSubObjs != 0) {
		do {
			_InitBoneRef(&pBlow->field_0x48[uVar7].boneRefA);
			_InitBoneRef(&pBlow->field_0x48[uVar7].boneRefB);
			uVar7 = uVar7 + 1;
		} while (uVar7 < pBlow->nbSubObjs);
	}

	return;
}

void CActorFighter::_InitGrab(s_fighter_grab* pGrab)
{
	uint boneId;
	bool bVar1;
	int iVar2;
	CActorWeapon* pCVar3;
	float fVar4;
	float fVar5;
	CAnimation* pAnim;

	if (pGrab->field_0x34 == 0.0f) {
		pGrab->field_0x34 = pGrab->field_0x30;
		fVar4 = pGrab->field_0x34 - ((this->pCollisionData)->pObbPrim->field_0x90).z;
		pGrab->field_0x34 = fVar4;
		pGrab->field_0x34 = fVar4 - 0.4f;
	}

	pGrab->field_0x10 = pGrab->field_0x10 * (fVar4 / _GetFighterAnimationLength(pGrab->field_0x100));

	boneId = (pGrab->field_0x14).boneId;
	if (boneId == 0) {
		(pGrab->field_0x14).pActor = (CActor*)0x0;
	}
	else {
		_InitBoneRef(&pGrab->field_0x14);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterBlowDB.cpp
void CActorFighter::_InitBlowsDB()
{
	uint curBlowIndex;

	curBlowIndex = 0;
	if (this->nbBlows != 0) {
		do {
			_InitBlow(this->aBlows + curBlowIndex);
			curBlowIndex = curBlowIndex + 1;
		} while (curBlowIndex < this->nbBlows);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterBlowDB.cpp
void CActorFighter::_DeleteCombosDB()
{
	void* pAlloc;
	int iVar1;
	uint uVar3;

	if (this->aCombos != (s_fighter_combo*)0x0) {
		uVar3 = 0;

		if (this->nbComboRoots + this->nbCombos != 0) {
			do {
				s_fighter_combo* pCombo = this->aCombos + uVar3;
				if ((pCombo->nbBranches != 0) && (pAlloc = pCombo->aBranches, pAlloc != (void*)0x0)) {
					edMemFree(pAlloc);
					pCombo->aBranches = (s_fighter_action_hash*)0x0;
				}

				uVar3 = uVar3 + 1;
			} while (uVar3 < this->nbComboRoots + this->nbCombos);
		}

		edMemFree(this->aCombos);

		this->aCombos = (s_fighter_combo*)0x0;
		this->nbCombos = 0;
		this->nbComboRoots = 0;
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterBlowDB.cpp
void CActorFighter::_InitGrabsDB()
{
	uint curGrabIndex;

	curGrabIndex = 0;
	if (this->nbGrabs != 0) {
		do {
			_InitGrab(this->aGrabs + curGrabIndex);
			curGrabIndex = curGrabIndex + 1;
		} while (curGrabIndex < this->nbGrabs);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterBlowDB.cpp
void CActorFighter::_DeleteGrabsDB()
{
	if (this->aGrabs != (s_fighter_grab*)0x0) {
		edMemFree(this->aGrabs);

		this->aGrabs = (s_fighter_grab*)0x0;
		this->nbGrabs = 0;
	}

	if (this->aGrabReacts != (s_fighter_grab_react*)0x0) {
		edMemFree(this->aGrabReacts);

		this->aGrabReacts = (s_fighter_grab_react*)0x0;
		this->nbGrabReacts = 0;
	}

	return;
}

#define ACTION_FLAG_GRAB 0x400

// Should be in: D:/Projects/b-witch/ActorFighterBlowDB.cpp
void CActorFighter::_InitCombosDB()
{
	uint curBranchIndex;
	s_fighter_combo* pCombo;
	uint curComboIndex;

	curComboIndex = 0;
	if (this->nbComboRoots + this->nbCombos != 0) {
		do {
			pCombo = this->aCombos + curComboIndex;
			uint hash = pCombo->actionHash.hash;
			s_fighter_blow* pBlow = _FindByHashcode<s_fighter_blow>(this->aBlows, this->nbBlows, hash);
			pCombo->actionHash.pData = STORE_SECTION(pBlow);

			if (pCombo->actionHash.pData == 0x0) {
				s_fighter_grab* pGrab = _FindByHashcode<s_fighter_grab>(this->aGrabs, this->nbGrabs, hash);
				pCombo->actionHash.pData = STORE_SECTION(pGrab);
				pCombo->field_0x4.field_0x0ushort = pCombo->field_0x4.field_0x0ushort | ACTION_FLAG_GRAB;
			}

			curBranchIndex = 0;
			if (pCombo->nbBranches != 0) {
				do {
					s_fighter_action_hash* pBranch = pCombo->aBranches + curBranchIndex;
					s_fighter_combo* pCombo = _FindByHashcode<s_fighter_combo>(this->aCombos, this->nbComboRoots, pBranch->hash);
					if (pCombo == (s_fighter_combo*)0x0) {
						pCombo = _FindByHashcode<s_fighter_combo>((this->aCombos + this->nbComboRoots), this->nbCombos, pBranch->hash);
					}

					curBranchIndex = curBranchIndex + 1;
					pBranch->pData = STORE_SECTION(pCombo);
				} while (curBranchIndex < pCombo->nbBranches);
			}

			curComboIndex = curComboIndex + 1;
		} while (curComboIndex < this->nbComboRoots + this->nbCombos);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterBlowDB.cpp
void CActorFighter::_DeleteBlowsDB()
{
	uint uVar1;
	int iVar2;
	s_fighter_blow* iVar3;

	if (this->aBlows != (s_fighter_blow*)0x0) {
		uVar1 = 0;

		if (this->nbBlows != 0) {
			iVar2 = 0;
			do {
				iVar3 = this->aBlows + uVar1;
				if ((iVar3->nbSubObjs != 0) && (iVar3->field_0x48 != (s_fighter_blow_sub_obj*)0x0)) {
					edMemFree(iVar3->field_0x48);
					iVar3->field_0x48 = (s_fighter_blow_sub_obj*)0x0;
				}

				uVar1 = uVar1 + 1;
			} while (uVar1 < this->nbBlows);
		}

		edMemFree(this->aBlows);
		this->aBlows = (s_fighter_blow*)0x0;
		this->nbBlows = 0;
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterBlowDB.cpp
void CActorFighter::_UpdateComboSituation()
{
	CActorFighter* pCVar1;
	float fVar2;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	this->comboFlags = 0;
	if (this->pAdversary == (CActorFighter*)0x0) {
		this->comboFlags = 2;
	}
	else {
		if ((this->flags & 0x1000) == 0) {
			local_10 = this->rotationQuat;
		}
		else {
			SetVectorFromAngleY(this->rotationEuler.y, &local_10);
		}
		pCVar1 = this->pAdversary;
		if ((pCVar1->flags & 0x1000) == 0) {
			local_20 = pCVar1->rotationQuat;
		}
		else {
			SetVectorFromAngleY(pCVar1->rotationEuler.y, &local_20);
		}

		fVar2 = edF32Vector4DotProductHard(&local_10, &local_20);
		if (fVar2 <= -0.707f) {
			this->comboFlags = 2;
		}
		else {
			if (0.707f <= fVar2) {
				this->comboFlags = 1;
			}
			else {
				if (0.0f < local_10.x * local_20.z - local_10.z * local_20.x) {
					this->comboFlags = 0x40;
				}
				else {
					this->comboFlags = 0x20;
				}
			}
		}
	}

	if (this->distanceToGround <= 0.08f) {
		this->comboFlags = this->comboFlags | 4;
	}
	else {
		if (this->dynamic.velocityDirectionEuler.y < 0.0f) {
			this->comboFlags = this->comboFlags | 8;
		}
		else {
			this->comboFlags = this->comboFlags | 0x10;
		}
	}

	return;
}

s_fighter_combo* CActorFighter::_FindComboBestRoot(s_input_pattern* pPattern, uint flags)
{
	uint unaff_s1_lo;
	uint unaff_s2_lo;
	s_fighter_combo* pCombo;
	s_fighter_combo* pComboEnd;
	s_fighter_combo* pCurCombo;
	float unaff_f20;
	s_input_pattern_cmp inputPatternCmp;

	pCurCombo = this->aCombos;
	pComboEnd = pCurCombo + this->nbComboRoots;
	pCombo = (s_fighter_combo*)0x0;

	for (; pCurCombo < pComboEnd; pCurCombo = pCurCombo + 1) {
		FIGHTER_LOG(LogLevel::Info, "\nCActorFighter::_FindComboBestRoot Checking {}", gDebugActionHashToFullName[pCurCombo->hash.hash]);

		s_fighter_move* pAction = LOAD_SECTION_CAST(s_fighter_move*, pCurCombo->actionHash.pData);

		FIGHTER_LOG(LogLevel::Info, "CActorFighter::_FindComboBestRoot Action {}", pAction ? gDebugActionHashToFullName[pAction->hash.hash] : "NONE");

		if ((((Func_0x1c0(pCurCombo) != false) && ((flags & ~(uint)pAction->field_0x4.field_0x1byte) == 0)) &&
			(CInputAnalyser::Compare(pPattern, &pCurCombo->pattern, &inputPatternCmp) != 0)) &&
			(((((pCombo == (s_fighter_combo*)0x0 || (unaff_s2_lo < inputPatternCmp.field_0x0)) ||
				((inputPatternCmp.field_0x0 == unaff_s2_lo && (unaff_f20 < inputPatternCmp.field_0x8)))) ||
				((inputPatternCmp.field_0x8 == unaff_f20 && (inputPatternCmp.field_0x4 < unaff_s1_lo)))) ||
				(pCombo->field_0xc < pCurCombo->field_0xc)))) {
			unaff_f20 = inputPatternCmp.field_0x8;
			pCombo = pCurCombo;
			unaff_s2_lo = inputPatternCmp.field_0x0;
			unaff_s1_lo = inputPatternCmp.field_0x4;

			FIGHTER_LOG(LogLevel::Info, "CActorFighter::_FindComboBestRoot Updated combo to *{}*", gDebugActionHashToFullName[pCurCombo->hash.hash]);
		}
	}

	return pCombo;
}

s_fighter_combo* CActorFighter::_FindComboBestNext(s_input_pattern* pPattern, uint flags)
{
	uint unaff_s1_lo;
	uint unaff_s2_lo;
	s_fighter_combo* pCombo;
	s_fighter_action_hash* pBranchHashEnd;
	s_fighter_action_hash* pCurBranchHash;
	float unaff_f20;
	s_input_pattern_cmp inputPatternCmp;

	pCurBranchHash = this->pFighterCombo->aBranches;
	pBranchHashEnd = this->pFighterCombo->aBranches + this->pFighterCombo->nbBranches;
	pCombo = (s_fighter_combo*)0x0;

	for (; pCurBranchHash < pBranchHashEnd; pCurBranchHash = pCurBranchHash + 1) {
		s_fighter_combo* pCurCombo = LOAD_SECTION_CAST(s_fighter_combo*, pCurBranchHash->pData);
		FIGHTER_LOG(LogLevel::Info, "\nCActorFighter::_FindComboBestNext Checking {}", gDebugActionHashToFullName[pCurCombo->hash.hash]);

		s_fighter_move* pAction = LOAD_SECTION_CAST(s_fighter_move*, pCurCombo->actionHash.pData);

		FIGHTER_LOG(LogLevel::Info, "CActorFighter::_FindComboBestNext Action {}", pAction ? gDebugActionHashToFullName[pAction->hash.hash] : "NONE");

		if ((((Func_0x1c0(pCurCombo) != false) && ((flags & ~(uint)pAction->field_0x4.field_0x1byte) == 0)) &&
			(CInputAnalyser::Compare(pPattern, &pCurCombo->pattern, &inputPatternCmp) != 0)) &&
			(((((pCombo == (s_fighter_combo*)0x0 || (unaff_s2_lo < inputPatternCmp.field_0x0)) ||
				((inputPatternCmp.field_0x0 == unaff_s2_lo && (unaff_f20 < inputPatternCmp.field_0x8)))) ||
				((inputPatternCmp.field_0x8 == unaff_f20 && (inputPatternCmp.field_0x4 < unaff_s1_lo)))) ||
				(pCombo->field_0xc < pCurCombo->field_0xc)))) {
			unaff_f20 = inputPatternCmp.field_0x8;
			pCombo = pCurCombo;
			unaff_s2_lo = inputPatternCmp.field_0x0;
			unaff_s1_lo = inputPatternCmp.field_0x4;

			FIGHTER_LOG(LogLevel::Info, "CActorFighter::_FindComboBestRoot Updated combo to *{}*", gDebugActionHashToFullName[pCurCombo->hash.hash]);
		}
	}

	return pCombo;
}

s_fighter_combo* CActorFighter::_FindBestCombo(s_input_pattern* pPattern, uint flags)
{
	s_fighter_combo* pFighterCombo;

	if (this->pFighterCombo == (s_fighter_combo*)0x0) {
		pFighterCombo = _FindComboBestRoot(pPattern, flags);
	}
	else {
		pFighterCombo = _FindComboBestNext(pPattern, flags);
	}

	return pFighterCombo;
}

bool CActorFighter::_ValidateCommand(s_fighter_action* pActionA, s_fighter_action* pActionB)
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;

	pActionB->all = pActionA->all & this->validCommandMask.all;

	if ((this->actorState == FIGHTER_BLOW_BEGIN) && (this->pBlow->field_0x50 * this->pBlow->field_0x8 <= this->timeInAir)) {
		pActionB->field_0x1 = pActionB->field_0x1 & 0xcf;
	}

	uVar3 = GetStateFlags(this->actorState);
	if ((uVar3 & 0x200000) == 0) {
		pActionB->field_0x0 = pActionB->field_0x0 & 0xf0;
	}
	else {
		this->field_0x44c = this->field_0x44c & 0xf0 | pActionB->field_0x0 & 0xf;
	}

	if ((uVar3 & 0x400000) == 0) {
		pActionB->field_0x0 = pActionB->field_0x0 & 0xf;
	}
	else {
		this->field_0x44c = this->field_0x44c & 0xf | (byte)((uint)(((ulong)pActionB->field_0x0 << 0x38) >> 0x3c) << 4);
	}

	if ((uVar3 & 0x800000) == 0) {
		pActionB->field_0x1 = pActionB->field_0x1 & 0xf0;
	}
	else {
		this->field_0x44d = this->field_0x44d & 0xf0 | pActionB->field_0x1 & 0xf;
	}

	if ((uVar3 & 0x1000000) == 0) {
		pActionB->field_0x1 = pActionB->field_0x1 & 0xcf;
	}
	else {
		this->field_0x44d = this->field_0x44d & 0xcf | (byte)((uint)(((ulong)pActionB->field_0x1 << 0x3a) >> 0x3e) << 4);
	}

	return pActionB->all != 0x0;
}

void CActorFighter::_SetRelativeSpeedOnGround(float speed, edF32VECTOR4* pDirection)
{
	this->field_0x44c = this->field_0x44c & 0xf0;
	if (pDirection->z == 0.0f) {
		this->scalarDynA.Reset();
	}
	else {
		if (0.0f < pDirection->z) {
			this->field_0x44c = this->field_0x44c & 0xf0 | (this->field_0x44c & 0xf) + 3 & 0xf;
			this->scalarDynA.BuildFromSpeedTime(pDirection->z, pDirection->z, speed);
		}
		else {
			this->field_0x44c = this->field_0x44c & 0xf0 | (this->field_0x44c & 0xf) + 6 & 0xf;
			this->scalarDynA.BuildFromSpeedTime(-pDirection->z, -pDirection->z, speed);
		}
	}

	if (pDirection->x == 0.0f) {
		this->scalarDynB.Reset();
	}
	else {
		if (0.0 < pDirection->x) {
			this->field_0x44c = this->field_0x44c & 0xf0 | (this->field_0x44c & 0xf) + 2 & 0xf;
			this->scalarDynB.BuildFromSpeedTime(pDirection->x, pDirection->x, speed);
		}
		else {
			this->field_0x44c = this->field_0x44c & 0xf0 | (this->field_0x44c & 0xf) + 1 & 0xf;
			this->scalarDynB.BuildFromSpeedTime(-pDirection->x, -pDirection->x, speed);
		}
	}

	return;
}

void CActorFighter::_BuildCommandFromAbsoluteVector(edF32VECTOR4* param_2, s_fighter_action* pFighterAction, edF32VECTOR4* param_4)
{
	float fVar1;
	edF32MATRIX4 auStack64;

	if ((this->flags & 0x1000) == 0) {
		fVar1 = GetAngleYFromVector(&this->rotationQuat);
		edF32Matrix4RotateYHard(fVar1, &auStack64, &gF32Matrix4Unit);
	}
	else {
		edF32Matrix4FromEulerSoft(&auStack64, &this->rotationEuler.xyz, "XYZ");
	}

	fVar1 = edF32Vector4DotProductHard(&auStack64.rowX, param_2);
	param_4->x = fVar1;
	param_4->y = 0.0f;
	fVar1 = edF32Vector4DotProductHard(&auStack64.rowZ, param_2);
	param_4->z = fVar1;
	param_4->w = 0.0f;

	pFighterAction->all = 0x0;

	if (0.5f <= fabs(param_4->x)) {
		if (0.0f < param_4->x) {
			pFighterAction->field_0x0 = pFighterAction->field_0x0 & 0xf0 | (pFighterAction->field_0x0 & 0xf) + 2 & 0xf;
		}
		else {
			pFighterAction->field_0x0 = pFighterAction->field_0x0 & 0xf0 | (pFighterAction->field_0x0 & 0xf) + 1 & 0xf;
		}
	}
	if (0.5f <= fabs(param_4->z)) {
		if (0.0f < param_4->z) {
			pFighterAction->field_0x0 = pFighterAction->field_0x0 & 0xf0 | (pFighterAction->field_0x0 & 0xf) + 3 & 0xf;
		}
		else {
			pFighterAction->field_0x0 = pFighterAction->field_0x0 & 0xf0 | (pFighterAction->field_0x0 & 0xf) + 6 & 0xf;
		}
	}

	return;
}

void CActorFighter::_UpdateDynForExplosion(_msg_hit_param* pHitParams)
{
	ed_3d_hierarchy_node* peVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_10;

	this->field_0x690 = pHitParams->field_0x40;
	this->hitDamage = pHitParams->damage;
	edF32Vector4SafeNormalize0Hard(&this->field_0x6a0, &pHitParams->field_0x20);
	this->hitFlags = 5;
	this->field_0x684 = 1;
	this->field_0x474 = 0.3f;
	this->field_0x6b0 = pHitParams->field_0x30 / 60.0f;
	fVar2 = ((this->field_0x6a0).y * -2.0f * this->field_0x6b0 * 0.8f) / (CDynamicExt::gForceGravity.y * this->dynamicExt.gravityScale);
	if (0.0f < fVar2) {
		fVar3 = 4.712389f / fVar2;
		if (10.0f < fVar3) {
			fVar3 = 1.570796f / fVar2;
		}
		if (10.0f < fVar3) {
			fVar3 = 10.0f;
		}
	}
	else {
		fVar3 = 10.0f;
	}

	edF32Vector4SubHard(&local_10, &this->currentLocation, &this->field_0x634->currentLocation);
	local_10.y = 0.0f;
	fVar2 = edF32Vector4NormalizeHard(&local_10, &local_10);
	if (fVar2 == 0.0f) {
		peVar1 = this->pMeshTransform;
		this->field_0x7a0 = (peVar1->base).transformA.rowX;
		this->field_0x7b4 = fVar3;
	}
	else {
		(this->field_0x7a0).x = local_10.z;
		(this->field_0x7a0).y = 0.0f;
		(this->field_0x7a0).z = -local_10.x;
		(this->field_0x7a0).w = 0.0f;
		this->field_0x7b4 = fVar3;
	}

	return;
}

void CActorFighter::_UpdateDynForHit(float maxDuration, _msg_hit_param* pHitParams)
{
	CActor* pActor;
	bool bVar1;
	float fVar2;
	float fVar3;
	float fVar4;


	this->field_0x690 = pHitParams->field_0x40;

	this->hitDamage = pHitParams->damage;

	edF32Vector4SafeNormalize0Hard(&this->field_0x6a0, &pHitParams->field_0x20);
	pActor = this->field_0x634;
	bVar1 = this->actorsExcludeTable.IsInList(pActor);

	if (bVar1 == false) {
		this->actorsExcludeTable.Add(maxDuration, pActor);
	}

	this->hitFlags = pHitParams->flags;
	this->field_0x474 = pHitParams->field_0x10;
	this->field_0x6b0 = pHitParams->field_0x30;
	this->field_0x684 = pHitParams->field_0x50;
	this->field_0x686 = pHitParams->field_0x52;

	if ((this->field_0x684 & 1U) != 0) {
		this->field_0x7a0 = pHitParams->field_0x60;
		this->field_0x7b4 = pHitParams->field_0x70;
	}

	return;
}

void CActorFighter::UpdateFightCommandInternal(CPlayerInput* pPlayerInput, int param_3)
{
	s_fighter_action_param* psVar2;
	float fVar3;
	float fVar4;
	bool bVar5;
	s_fighter_action_param* psVar6;
	int iVar7;
	s_fighter_combo* psVar8;
	int iVar9;
	Timer* pTVar10;
	uint* puVar11;
	CBehaviourFighter* this_00;
	ulong uVar12;
	long lVar13;
	s_fighter_action_param* psVar14;
	uint uVar15;
	uint uVar16;
	s_fighter_combo* psVar17;
	uint uVar18;
	float fVar19;
	float fVar20;
	uint local_b0;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32MATRIX4 eStack112;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	s_fighter_action_param fighterActionParam;
	undefined* local_c;
	s_fighter_action local_4;

	if (this->pInputAnalyser == (CInputAnalyser*)0x0) {
		return;
	}

	if ((pPlayerInput == (CPlayerInput*)0x0) || (param_3 == 0)) {
		local_b0 = 0;
		local_20.x = 0.0f;
		fVar19 = 0.0f;
		uVar18 = 0;
		uVar16 = 0;
		local_20.z = 0.0f;
		uVar15 = 0;
		fVar20 = 0.0f;
	}
	else {
		local_20.z = pPlayerInput->aAnalogSticks[PAD_STICK_LEFT].y;
		local_20.x = pPlayerInput->aAnalogSticks[PAD_STICK_LEFT].x;
		fVar20 = pPlayerInput->aButtons[5].clickValue;
		fVar19 = pPlayerInput->aButtons[6].clickValue;
		uVar16 = pPlayerInput->releasedBitfield & 0x20;
		local_b0 = pPlayerInput->releasedBitfield & 0x40;
		uVar15 = pPlayerInput->pressedBitfield & 0x20;
		uVar18 = pPlayerInput->pressedBitfield & PAD_BITMASK_CROSS;
	}

	local_c = (undefined*)0x0;
	local_30 = gF32Vector4Zero;

	memset(&fighterActionParam, 0, sizeof(s_fighter_action));

	local_4.all = 0;
	iVar9 = 2;
	if ((local_20.z != 0.0f) || (fVar3 = local_30.x, local_20.y = local_30.y, fVar4 = local_30.z, local_20.w = local_30.w, local_20.x != 0.0f)) {
		local_20.y = 0.0f;
		local_20.w = 0.0f;
		edF32Matrix4GetTransposeHard(&eStack112, &(CScene::ptable.g_CameraManager_0045167c)->transMatrix_0x390);
		edF32Matrix4MulF32Vector4Hard(&local_20, &eStack112, &local_20);
		local_30.z = local_20.z * -1.0f;
		local_30.x = local_20.x;
		local_30.y = local_20.y;
		local_30.w = local_20.w;
		local_20.z = local_30.z;
		edF32Matrix4SetIdentityHard(&eStack112);
		edF32Matrix4RotateYHard(this->adversaryAngleDiff, &eStack112, &eStack112);
		edF32Matrix4TransposeHard(&eStack112);
		edF32Matrix4MulF32Vector4Hard(&local_20, &eStack112, &local_20);
		uVar12 = FUN_00337d20(&local_20);
		if ((uVar12 & 2) == 0) {
			if ((uVar12 & 8) != 0) {
				local_4.all = local_4.all & 0xffffff00 | (uint)(local_4.field_0x0 & 0xf0 | (local_4.field_0x0 & 0xf) + 1 & 0xf);
			}
		}
		else {
			local_4.all = local_4.all & 0xffffff00 | (uint)(local_4.field_0x0 & 0xf0 | (local_4.field_0x0 & 0xf) + 2 & 0xf);
		}

		if ((uVar12 & 1) == 0) {
			if ((uVar12 & 4) != 0) {
				local_4.all = local_4.all & 0xffffff00 | (uint)(local_4.field_0x0 & 0xf0 | (local_4.field_0x0 & 0xf) + 6 & 0xf);
			}
		}
		else {
			local_4.all = local_4.all & 0xffffff00 | (uint)(local_4.field_0x0 & 0xf0 | (local_4.field_0x0 & 0xf) + 3 & 0xf);
		}

		fVar3 = local_20.x;
		fVar4 = local_20.z;
		if ((local_4.all & 0xf) != 0) {
			if (((fVar20 == 0.0f) || ((this->pInputAnalyser->flags & 1) == 0)) || (Func_0x1ac() == 0)) {
				local_4.all = local_4.all & 0xffffff0f | 0x10;
				fVar3 = local_20.x;
				fVar4 = local_20.z;
			}
			else {
				local_4.all = local_4.all & 0xffffff0f | 0x20;
				this->pInputAnalyser->flags = this->pInputAnalyser->flags & 0xfffffffe;
				fVar3 = local_20.x;
				fVar4 = local_20.z;
			}
		}
	}

	local_20.z = fVar4;
	local_20.x = fVar3;
	if (uVar16 != 0) {
		this->pInputAnalyser->flags = this->pInputAnalyser->flags | 1;
	}

	if (uVar18 != 0) {
		local_4.all = local_4.all & 0xffffff0f | 0x40;
	}

	if (Func_0x1a4() == 0) goto LAB_0030a390;

	if (Func_0x1a0() == 0) {
		IMPLEMENTATION_GUARD(
		SetVectorFromAngleY(this->adversaryAngleDiff, &eStack144);
		iVar9 = this->pInputAnalyser->Cumulate(pPlayerInput, &eStack144, &local_20);
		if (iVar9 != 0) {
			uVar12 = ((ulong) * (ushort*)((int)&(this->pInputAnalyser->patternA).patternA + 2) << 0x34) >> 0x38;
			if ((uVar12 & 3) == 0) {
				if ((uVar12 & 4) != 0) {
					local_c = (undefined*)0x0;
					local_4 = local_4 & 0xfffff0ff | 0x200;
				}
			}
			else {
				local_c = (undefined*)0x0;
				local_4 = local_4 & 0xfffff0ff | 0x100;
			}
		}
		goto LAB_0030a390;)
	}

	SetVectorFromAngleY(this->adversaryAngleDiff, &eStack128);

	iVar7 = this->actorState;
	if (((iVar7 == 0x6b) || (iVar7 == FIGHTER_BLOW_BEGIN)) && (psVar8 = this->pInputAnalyser->pComboA, psVar8 != (s_fighter_combo*)0x0)) {
		SetFighterCombo(psVar8);
		this->pInputAnalyser->pComboA = (s_fighter_combo*)0x0;
		this->pInputAnalyser->pComboB = (s_fighter_combo*)0x0;
	}

	psVar8 = this->pFighterCombo;
	if ((psVar8 == (s_fighter_combo*)0x0) || ((psVar8->field_0x4.field_0x0ushort & 0x100U) == 0)) {
		iVar7 = this->pInputAnalyser->Cumulate(pPlayerInput, &eStack128, &local_20);
		if (iVar7 != 0) {
			if (this->pFighterCombo == (s_fighter_combo*)0x0) {
				_UpdateComboSituation();
				this->pInputAnalyser->pComboB = _FindBestCombo(&this->pInputAnalyser->patternA, this->comboFlags);
			}
			else {
				this->pInputAnalyser->pComboB = _FindBestCombo(&this->pInputAnalyser->patternA, 0);
			}

			psVar8 = this->pInputAnalyser->pComboB;
			if ((psVar8 == (s_fighter_combo*)0x0) || ((psVar8->field_0x4.field_0x0ushort & 0x100U) == 0)) {
				this->pInputAnalyser->Reset();
			}
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		iVar7 = this->actorState;
		if (((iVar7 == 0x6b) || (iVar7 == FIGHTER_EXECUTE_BLOW)) || (iVar7 == FIGHTER_BLOW_BEGIN)) {
			psVar17 = (s_fighter_combo*)psVar8->aBranches;
			uVar16 = psVar8->nbBranches;
			this->pInputAnalyser->pComboB = (s_fighter_combo*)0x0;
			uVar12 = CInputAnalyser::FUN_003381b0(this->pInputAnalyser, pPlayerInput, &eStack128, &local_20);
			if ((uVar12 & 0xff) == 0) {
				while ((uVar16 != 0 && (this->pInputAnalyser->pComboB == (s_fighter_combo*)0x0))) {
					if (((*(s_fighter_combo**)psVar17)->field_0x4 & 0x100U) == 0) {
						this->pInputAnalyser->pComboB = *(s_fighter_combo**)psVar17;
					}
					else {
						psVar17 = (s_fighter_combo*)((int)psVar17 + 4);
						uVar16 = uVar16 - 1;
					}
				}
				if ((this->pFighterCombo->field_0x4 & 0x200U) == 0) {
					iVar9 = 1;
					local_4 = local_4 & 0xfffff0ff | 0x800;
				}
			}
			else {
				while ((uVar16 != 0 && (this->pInputAnalyser->pComboB == (s_fighter_combo*)0x0))) {
					if (((*(s_fighter_combo**)psVar17)->field_0x4 & 0x100U) == 0) {
						psVar17 = (s_fighter_combo*)((int)psVar17 + 4);
						uVar16 = uVar16 - 1;
					}
					else {
						this->pInputAnalyser->pComboB = *(s_fighter_combo**)psVar17;
					}
				}
			}
		})
	}

	if (iVar9 == 2) {
		iVar7 = this->actorState;
		if ((iVar7 == 0x6e) || (iVar7 == FIGHTER_EXECUTE_BLOW)) {
		LAB_0030a1f0:
			iVar9 = 1;
		}
		else {
			if ((iVar7 == 0x6b) || (iVar7 == FIGHTER_BLOW_BEGIN)) {
				iVar9 = 2;
			}
			else {
				if (iVar7 == FIGHTER_HIT_STEP_BACK) {
					iVar9 = 0;
				}
				else {
					if (((iVar7 != 0x6c) && (iVar7 != 0x6f)) && (iVar7 != FIGHTER_BLOW_END)) goto LAB_0030a1f0;
					if (this->pFighterCombo != (s_fighter_combo*)0x0) {
						fVar20 = _GetFighterAnimationLength(this->currentAnimType);

						if ((fVar20 * this->pFighterCombo->field_0x8 <= this->timeInAir) ||
							(pTVar10 = GetTimer(), fVar20 - this->timeInAir <= pTVar10->cutsceneDeltaTime))
						{
							iVar9 = 0;
						}
						else {
							iVar9 = 1;
						}
					}
				}
			}
		}
	}

	if (iVar9 == 1) {
		psVar8 = this->pInputAnalyser->pComboB;
		if (psVar8 != (s_fighter_combo*)0x0) {
			s_fighter_blow* pCurrentBlow = LOAD_SECTION_CAST(s_fighter_blow*, psVar8->actionHash.pData);
			s_fighter_blow* pBaseCatch = FindBlowByName("BASE_CATCH");
			if (pCurrentBlow == pBaseCatch) {
				local_4.field_0x1 = local_4.field_0x1 & 0xf0 | local_4.field_0x1 & 0xf | 2, local_4.field_0x0;
				local_4.all = local_4.all & 0xffff0000 | (uint)local_4.field_0x0_2;
			}
			else {
				if ((psVar8->field_0x4.field_0x0ushort & 0x400U) == 0) {
					local_4.field_0x1 = local_4.field_0x1 & 0xf0 | local_4.field_0x1 & 0xf | 1, local_4.field_0x0;
					local_4.all = local_4.all & 0xffff0000 | (uint)local_4.field_0x0_2;
				}
				else {
					local_4.field_0x1 = local_4.field_0x1 & 0xf0 | local_4.field_0x1 & 0xf | 4, local_4.field_0x0;
					local_4.all = local_4.all & 0xffff0000 | (uint)local_4.field_0x0_2;
				}
			}

			psVar8 = this->pInputAnalyser->pComboB;
			fighterActionParam.pData = LOAD_SECTION(psVar8->actionHash.pData);
			this->pInputAnalyser->pComboA = psVar8;
			this->pInputAnalyser->pComboB = (s_fighter_combo*)0x0;
		}
	}
	else {
		if (iVar9 == 0) {
			SetFighterCombo((s_fighter_combo*)0x0);
			this->pInputAnalyser->pComboA = (s_fighter_combo*)0x0;
			this->pInputAnalyser->pComboB = (s_fighter_combo*)0x0;
		}
	}
LAB_0030a390:
	if (local_b0 != 0) {
		this->pInputAnalyser->flags = this->pInputAnalyser->flags | 2;
		this->pInputAnalyser->flags = this->pInputAnalyser->flags | 4;
	}

	if (fVar19 != 0.0f) {
		IMPLEMENTATION_GUARD(
		if ((((((ulong)this->validCommandMask[1] << 0x3a) >> 0x3e & 2) != 0) && ((this->pInputAnalyser->flags & 4) != 0)) &&
			(((local_4 & 0xf) != 0 && (((uVar16 = local_4 & 0xf, uVar16 != 2 && (uVar16 != 1)) && (5 < uVar16)))))) {
			local_4 = local_4 & 0xffffcfff | 0x2000;
			this->pInputAnalyser->flags = this->pInputAnalyser->flags & 0xfffffffb;
		}

		if (((((ulong)local_4.field_0x1 << 0x3a) >> 0x3e == 0) && ((((ulong)this->validCommandMask[1] << 0x3a) >> 0x3e & 1) != 0)
			) && ((this->pInputAnalyser->flags & 2) != 0)) {
			local_4 = local_4 & 0xffffcfff | 0x1000;
		})
	}

	if ((((local_4.all & 0xf00) != 0) && (psVar8 = this->pInputAnalyser->pComboA, psVar8 != (s_fighter_combo*)0x0)) &&
		((((ulong)(psVar8->pattern).field_0x3byte << 0x38) >> 0x3c & 1) != 0)) {
		local_4.all = local_4.all & 0xffffcf0f;
		this->pInputAnalyser->flags = this->pInputAnalyser->flags & 0xfffffffd;
		this->pInputAnalyser->flags = this->pInputAnalyser->flags & 0xfffffffb;
	}

	if (((ulong)local_4.field_0x1 << 0x3a) >> 0x3e != 0) {
		IMPLEMENTATION_GUARD(
		uVar12 = ((ulong)local_4.field_0x0 << 0x38) >> 0x3c;
		if (((uVar12 != 0) && (uVar12 != 1)) &&
			(bVar5 = FUN_0031b5d0(this, this->actorState), bVar5 != false)) {
			local_4 = local_4 & 0xffffcfff;
		})
	}

	if ((((ulong)local_4.field_0x0 << 0x38) >> 0x3c == 0) && (uVar15 != 0)) {
		local_4.all = local_4.all & 0xffffff0f | 0x80;
	}

	if ((((ulong)local_4.field_0x0 << 0x38) >> 0x3c != 2) && ((local_4.field_0x0 & 0xf) != 0)) {
		local_4.all = local_4.all & 0xfffffff0 | 0xc;
	}

	edF32Vector4SafeNormalize0Hard(&local_30, &local_30);
	fighterActionParam.field_0x0 = &local_30;
	this_00 = reinterpret_cast<CBehaviourFighter*>(GetBehaviour(this->curBehaviourId));
	this_00->Conditional_Execute(&local_4, &fighterActionParam);
	return;
}



void CBehaviourFighter::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorFighter*>(pOwner);
	this->behaviourId = FIGHTER_BEHAVIOUR_DEFAULT;

	return;
}

void CBehaviourFighter::Manage()
{
	byte bVar1;
	s_fighter_combo* psVar2;
	CActorFighter* pFighter;
	int iVar4;
	CAnimation* pCVar5;
	edAnmLayer* peVar6;
	//code* pcVar7;
	bool bVar8;
	int iVar9;
	CBehaviour* pCVar10;
	Timer* pTVar11;
	long lVar12;
	int iVar13;
	ulong uVar14;
	long lVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	CActorsTable local_110;
	undefined4 local_4;

	uVar14 = 8;

	iVar9 = this->pOwner->standAnim;
	iVar13 = 0;
	this->pOwner->UpdateFightCommand();

	psVar2 = this->pOwner->pFighterCombo;
	if ((psVar2 != (s_fighter_combo*)0x0) && (psVar2->nbBranches == 0)) {
		iVar13 = 1;
	}

	pFighter = this->pOwner;
	
	switch (this->pOwner->actorState) {
	case 0xc:
		pCVar5 = pFighter->pAnimationController;

		if (pCVar5->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(FIGHTER_DEFAULT_STATE_IDLE, pFighter->standAnim);
		}

		pFighter->_ManageFighterDyn(0, 0x1002023b, (CActorsTable*)0x0);
		break;
	case 0xd:
		pFighter->_ManageFighterDyn(0x19, 0x1002023b, (CActorsTable*)0x0);
		if ((pFighter->pCollisionData->flags_0x4 & 2) == 0) {
			pFighter->field_0x4fc = pFighter->dynamic.linearAcceleration * fabs(pFighter->dynamic.velocityDirectionEuler.y) / 8.0f;
			if (1.0f < pFighter->field_0x4fc) {
				pFighter->field_0x4fc = 1.0;
			}

			if (pFighter->currentLocation.y < pFighter->field_0x4f0) {
				pFighter->fightFlags = pFighter->fightFlags | 2;
			}
		}
		else {
			pFighter->SetState(0xc, -1);
		}
		break;
	case FIGHTER_DEFAULT_STATE_IDLE:
		if (pFighter->field_0x474 <= pFighter->timeInAir) {
			pFighter->AcquireAdversary();
			uVar14 = 0xc;
			if ((pFighter->fightFlags & 1) != 0) {
				pFighter->SetLookingAtOn();
				pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
				pFighter->fightFlags = pFighter->fightFlags & 0xfffffffe;
			}
		}

		pFighter->_ManageFighterDyn(uVar14, 0x100a023b, 0);
		break;
	case FIGHTER_DEFAULT_STATE_RUN:
		this->pOwner->_StateFighterRun((CActorsTable*)0x0);
		break;
	case FIGHTER_HIT_STEP_BACK:
		this->pOwner->_StateFighterHitStepBack(6, iVar9, 1);
		break;
	case FIGHTER_BLOW_BEGIN:
		this->pOwner->_ManageFighterDyn((this->pOwner->pBlow->blowStageBegin).flags, 0x40121, (CActorsTable*)0x0);

		pCVar5 = this->pOwner->pAnimationController;

		if (pCVar5->IsCurrentLayerAnimEndReached(0)) {
			this->pOwner->SetState(FIGHTER_EXECUTE_BLOW, (this->pOwner->pBlow->blowStageExecute).animId);
		}
		break;
	case FIGHTER_EXECUTE_BLOW:
		this->pOwner->_StateFighterExecuteBlow(FIGHTER_BLOW_END, FIGHTER_BLOW_BEGIN, iVar13);
		break;
	case FIGHTER_BLOW_END:
	{
		pFighter->_ManageFighterDyn((pFighter->pBlow->blowStageEnd).flags, 0x40323, (CActorsTable*)0x0);

		pCVar5 = pFighter->pAnimationController;

		if (pCVar5->IsCurrentLayerAnimEndReached(0)) {
			pFighter->scalarDynA.Reset();
			pFighter->scalarDynB.Reset();
			pFighter->scalarDynJump.Reset();

			CBehaviourFighter* pBehaviourFighter = reinterpret_cast<CBehaviourFighter*>(pFighter->GetBehaviour(pFighter->curBehaviourId));
			pBehaviourFighter->SetInitialState();
		}
	}
	break;
	case 0xa:
	case 0xb:
		pFighter->_StateFighter_0xb();
		break;
	default:
		IMPLEMENTATION_GUARD();
	}

	_ManageExit();

	return;
}

void CBehaviourFighter::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	bool bVar2;
	int iVar3;

	this->pOwner->field_0x474 = 0;
	this->pOwner->pAnimationController->RegisterBone(0x00554f43);
	this->pOwner->pAnimationController->RegisterBone(0x45544554);
	this->pOwner->pAnimationController->RegisterBone(0x4146416f);
	this->pOwner->SetLookingAtBones(0x45544554, 0x554f43);
	this->pOwner->flags = this->pOwner->flags | 0x800;

	if (newState == -1) {
		SetInitialState();
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->field_0x860 = 0;
	this->pOwner->field_0x864 = (s_fighter_blow*)0x0;
	this->pOwner->SetFighterCombo((s_fighter_combo*)0x0);

	(this->pOwner->fighterAnatomyZones).field_0x0 = (this->pOwner->fighterAnatomyZones).field_0x10;

	iVar3 = this->pOwner->prevBehaviourId;
	if (iVar3 == -1) {
		this->behaviourId = this->pOwner->GetFightBehaviour();
	}
	else {
		bVar2 = this->pOwner->IsFightRelated(iVar3);
		if (bVar2 == false) {
			this->behaviourId = this->pOwner->prevBehaviourId;
		}
	}

	if (CActorHero::_gThis != pOwner) {
		IMPLEMENTATION_GUARD_AUDIO(
		CScene::ptable.g_AudioManager_00451698->PlayCombatMusic();)
	}

	this->pOwner->GetLifeInterfaceOther()->SetPriority(4);

	return;
}

void CBehaviourFighter::End(int newBehaviourId)
{
	CInputAnalyser* pInputAnalyser;

	this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

	this->pOwner->flags = this->pOwner->flags & 0xfffff7ff;
	this->pOwner->pAnimationController->UnRegisterBone(0x00554f43);
	this->pOwner->pAnimationController->UnRegisterBone(0x45544554);
	this->pOwner->pAnimationController->UnRegisterBone(0x4146416f);

	this->pOwner->SetLookingAtOff();

	this->pOwner->field_0x44cuint = 0;
	this->pOwner->fightFlags = this->pOwner->fightFlags & 0xfffffffe;

	if ((this->pOwner->IsFightRelated(newBehaviourId) == false) && (pInputAnalyser = this->pOwner->pInputAnalyser, pInputAnalyser != (CInputAnalyser*)0x0)) {
		*pInputAnalyser = CInputAnalyser();
	}

	this->pOwner->GetLifeInterfaceOther()->SetPriority(0);

	IMPLEMENTATION_GUARD_AUDIO(
	if (CActorHero::_gThis != (CActorHero*)this->pOwner) {
		FUN_001844c0(CScene::ptable.g_AudioManager_00451698);
	})

	return;
}

void CBehaviourFighter::InitState(int newState)
{
	CActorFighter* pFighter;
	int iVar2;
	int* piVar3;
	CCollision* pCVar4;
	CActorFighter* pCVar5;
	CActor* pCVar6;
	edF32VECTOR4* peVar7;
	Timer* pTVar8;
	int iVar9;
	uint uVar10;
	StateConfig* pSVar11;
	//CActorFighterVTable** ppCVar12;
	byte bVar13;
	long in_a2;
	ulong uVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	edF32MATRIX4 eStack176;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	s_fighter_action local_8;
	s_fighter_action local_4;
	CAnimation* pAnim;

	pFighter = this->pOwner;

	if ((newState - 0x1cU < 4) && (3 < pFighter->prevActorState - 0x1cU)) {
		IMPLEMENTATION_GUARD(
		StaticMeshComponentAdvanced::FUN_00406ff0
		(&pFighter->staticMeshComponentAdvanced, (ed_3D_Scene*)0x0, (ed_g3d_manager*)0x0,
			(ed_3d_hierarchy_setup*)0x0, (char*)0x0);
		(*((pFighter->staticMeshComponentAdvanced).base.pVTable)->SetHidden)
			((StaticMeshComponent*)&pFighter->staticMeshComponentAdvanced, (ed_3D_Scene*)0x0);
		edF32Matrix4TranslateHard(&eStack176, &gF32Matrix4Unit, &pFighter->field_0x600);
		edF32Matrix4MulF32Matrix4Hard
		(&eStack176, &eStack176, (edF32MATRIX4*)pFighter->pMeshTransform);
		(*(code*)(pFighter->staticMeshComponentAdvanced).base.pVTable[1].field_0x4)
			(&pFighter->staticMeshComponentAdvanced, &eStack176);
		peVar7 = StaticMeshComponentAdvanced::GetTextureAnimSpeedNormalExtruder(&pFighter->staticMeshComponentAdvanced);
		pTVar8 = GetTimer();
		peVar7->x = (1.0f / pFighter->field_0x6c0) * pTVar8->cutsceneDeltaTime;
		peVar7->y = 0.0f;
		peVar7->z = 0.0f;
		peVar7->w = 0.0f;
		iVar9 = (*(code*)(pFighter->staticMeshComponentAdvanced).base.pVTable[1].field_0x0)
			(&pFighter->staticMeshComponentAdvanced);
		in_a2 = (long)(int)(edF32VECTOR4*)(iVar9 + 0x20);
		CActorFighter::FUN_0031aea0
		(pFighter, (long)(int)pFighter, (edF32VECTOR4*)(iVar9 + 0x20), pFighter->field_0x5f0, (int*)0x0, 0);)
	}

	if ((newState == FIGHTER_HIT_STEP_BACK) || (newState == 0x18)) {
		for (uVar10 = 0; uVar10 < 2; uVar10 = uVar10 + 1) {
			pFighter->pAnimationController->RegisterBone(gBoneIds_004343a0[uVar10]);
			IMPLEMENTATION_GUARD_FX(
			CFxHandle::FUN_004074f0(pFighter->field_0x550 + uVar10, 0, 0);)
		}
	}

	pFighter = this->pOwner;

	iVar2 = pFighter->actorState;
	if (iVar2 != -1) {
		pFighter->GetStateCfg(iVar2);
	}

	if (iVar2 - 0x17U < 5) {
		pFighter = this->pOwner;
		iVar2 = pFighter->prevActorState;
		if (iVar2 != -1) {
			pFighter->GetStateCfg(iVar2);
		}

		if (4 < iVar2 - 0x17U) {
			this->pOwner->SetLookingAtOff();
		}
	}

	uVar10 = this->pOwner->GetStateFlags(newState);
	uVar10 = uVar10 & 0xff800;

	if (uVar10 == 0x80000) {
		if ((this->pOwner->GetStateFlags(this->pOwner->prevActorState) & 0xff800) != 0x80000) {
			pFighter = this->pOwner;
			GetAnglesFromVector(&pFighter->rotationEuler.xyz, &pFighter->rotationQuat);
			pFighter->flags = pFighter->flags | 0x1000;
			pCVar4 = pFighter->pCollisionData;
			pCVar4->flags_0x0 = pCVar4->flags_0x0 | 0x800;
			(pFighter->pCollisionData)->actorFieldA = (CActor*)pFighter->pAdversary;

			IMPLEMENTATION_GUARD(
			piVar3 = *(int**)&pFighter->field_0x854;
			if ((piVar3 != (int*)0x0) && (*piVar3 != 0)) {
				CAnimation::RegisterBone(*(CAnimation**)(*piVar3 + 0x28), piVar3[1]);
			})
		}
	}
	else {
		if (uVar10 == 0x8000) {
			if ((this->pOwner->GetStateFlags(this->pOwner->prevActorState) & 0xff800) != 0x8000) {
				IMPLEMENTATION_GUARD(
				this->pOwner->_BeginFighterHold();)
			}
		}
		else {
			if (uVar10 == 0x4000) {
				if ((this->pOwner->GetStateFlags(this->pOwner->prevActorState) & 0xff800) != 0x4000) {
					IMPLEMENTATION_GUARD(
						this->pOwner->_BeginFighterRide();)
				}
			}
			else {
				if (uVar10 == 0x2000) {
					if ((this->pOwner->GetStateFlags(this->pOwner->prevActorState) & 0xff800) != 0x2000) {
						IMPLEMENTATION_GUARD(
							this->pOwner->_BeginFighterFlip();)
					}
				}
			}
		}
	}

	switch (newState) {
	case 0xc:
		pCVar4 = this->pOwner->pCollisionData;
		pCVar4->flags_0x0 = pCVar4->flags_0x0 | 0x30;
	case 0xd:
		break;
	case FIGHTER_DEFAULT_STATE_IDLE:
		pFighter = this->pOwner;
		pFighter->actorsExcludeTable.nbEntries = 0;
		pFighter->fightFlags = pFighter->fightFlags | 3;
		pFighter->field_0x44cuint = 0;
		pFighter->dynamic.speed = 0.0f;
		break;
	case FIGHTER_DEFAULT_STATE_RUN:
		pFighter = this->pOwner;

		bVar13 = pFighter->field_0x44c & 0xf;
		if (((bVar13 == 2) || (bVar13 == 1)) || (bVar13 < 6)) {
			pFighter->fightFlags = pFighter->fightFlags & 0xfffffffb;
		}
		else {
			pFighter->fightFlags = pFighter->fightFlags | 4;
		}

		pFighter->field_0x4e0 = gF32Vector4Zero;
		break;
	case FIGHTER_HIT_STEP_BACK:
	{
		pFighter = this->pOwner;

		uint uVar8;

		if ((pFighter->field_0x684 & 8U) == 0) {
			uVar8 = pFighter->_SV_HIT_GetHitZoneFromImpact(&pFighter->fighterAnatomyZones, &pFighter->field_0x690, &pFighter->currentLocation);
		}
		else {
			uVar8 = (uint)pFighter->field_0x686;
		}

		const int animationId = pFighter->_SV_ANM_GetMultiWaysAnim3D(pFighter->field_0x6d0 + uVar8, &pFighter->field_0x6a0, (edF32VECTOR4*)0x0);
		pFighter->_StateFighterHitStepBackInit(animationId, 1);
	}
		break;
	case FIGHTER_BLOW_BEGIN:
	case 0x68:
	{
		pFighter = this->pOwner;

		pFighter->nbFightCollisions = pFighter->pBlow->nbBoneRefs;

		for (uint curColIndex = 0; curColIndex < pFighter->nbFightCollisions; curColIndex = curColIndex + 1) {
			pFighter->aFightCollisions[curColIndex].pActor = pFighter->pBlow->aBoneRefs[curColIndex].pActor;
			pFighter->aFightCollisions[curColIndex].boneId = pFighter->pBlow->aBoneRefs[curColIndex].boneId;
		}

		pFighter->_SV_HIT_FightCollisionsBegin();

		pFighter->fightFlags = pFighter->fightFlags & 0xffffffbf;

		const float dynSpeed = pFighter->_StateFighterFightActionDynInit(&pFighter->pBlow->blowStageBegin);
		pFighter->FUN_0031ac10(dynSpeed, pFighter->pBlow);
	}
	break;
	case FIGHTER_EXECUTE_BLOW:
	case 0x69:
	{
		pFighter = this->pOwner;
		pFighter->field_0x834 = pFighter->pBlow;
		(pFighter->actorsExcludeTable).nbEntries = 0;
		pFighter->_StateFighterFightActionDynInit(&pFighter->pBlow->blowStageExecute);
	}
	break;
	case FIGHTER_BLOW_END:
	case 0x6a:
		this->pOwner->_StateFighterFightActionDynInit(&this->pOwner->pBlow->blowStageEnd);
		break;
	case 0xa:
		this->pOwner->_StateFighter_0xaInit();
	case 0xb:
		this->pOwner->_StateFighter_0xbInit(1.6f);
		break;
	default:
		IMPLEMENTATION_GUARD();
	}

	return;
}

void CBehaviourFighter::TermState(int oldState, int newState)
{
	CCollision* pCVar2;
	int* piVar3;
	CActorFighter* pFighter;
	bool bVar5;
	bool bVar6;
	Timer* pTVar7;
	int iVar8;
	StateConfig* pSVar9;
	CActor* pCVar10;
	CActorFighter* pCVar11;
	uint* puVar12;
	uint uVar13;
	ulong uVar14;
	float fVar15;
	CAnimation* pAnim;

	switch (oldState)
	{
	case FIGHTER_DEFAULT_STATE_IDLE:
	case FIGHTER_DEFAULT_STATE_RUN:
	case FIGHTER_HIT_STEP_BACK:
	case FIGHTER_BLOW_END:
	case 0xc:
		break;
	case 0xd:
		pFighter = this->pOwner;
		if ((pFighter->GetStateFlags(newState) & 0x100) != 0) {
			pFighter->fightFlags = pFighter->fightFlags | 2;
		}
		break;
	case FIGHTER_BLOW_BEGIN:
	case 0x68:
	{
		pFighter = this->pOwner;
		if (newState != -1) {
			pFighter->GetStateCfg(newState);
		}

		uVar14 = 0;
		if (newState - 0x17U < 5) {
			uVar14 = 4;
		}

		bVar5 = false;
		if ((newState - 0x1cU < 4) && ((1 << (newState - 0x1cU & 0x1f) & 0xbU) != 0)) {
			bVar5 = true;
		}

		if (bVar5) {
			uVar14 = uVar14 | 8;
		}

		if (uVar14 != 0) {
			pFighter->pFighterCombo = (s_fighter_combo*)0x0;
			pFighter->scalarDynA.Reset();
			pFighter->scalarDynB.Reset();
		}

		bVar6 = true;
		bVar5 = false;
		if ((newState - 0x49U < 0x20) && ((1 << (newState - 0x49U & 0x1f) & 0xf000003fU) != 0)) {
			bVar5 = true;
		}

		if ((!bVar5) && (6 < newState - 0x69U)) {
			bVar6 = false;
		}

		if (bVar6) {
			void* pcVar1 = pFighter->pBlow->field_0xd0;
			if (pcVar1 != (void*)0x0) {
				IMPLEMENTATION_GUARD(
				(*pcVar1)(pFighter, 0);)
			}
		}
		else {
			void* pcVar1 = pFighter->pBlow->field_0xd0;
			if (pcVar1 != (void*)0x0) {
				IMPLEMENTATION_GUARD(
				(*pcVar1)(pFighter, 3);)
			}

			pFighter->_SV_HIT_FightCollisionsEnd();
		}

		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		pFighter->field_0x36c = pFighter->field_0x36c | 1;
		pFighter->field_0x47c = 23.56194f;
	}
	break;
	case FIGHTER_EXECUTE_BLOW:
	case 0x69:
	{
		pFighter = this->pOwner;
		pFighter->_SV_HIT_FightCollisionsEnd();

		void* pcVar1 = pFighter->pBlow->field_0xd0;
		if (pcVar1 != (void*)0x0) {
			IMPLEMENTATION_GUARD(
			bVar5 = true;
			bVar4 = false;
			if ((newState - 0x49U < 0x20) && ((1 << (newState - 0x49U & 0x1f) & 0xf000003fU) != 0)) {
				bVar4 = true;
			}
			if ((!bVar4) && (6 < newState - 0x69U)) {
				bVar5 = false;
			}
			if (!bVar5) {
				(*pcVar1)(pFighter, 3);
			})
		}

		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		pFighter->field_0x36c = pFighter->field_0x36c | 1;
		pFighter->field_0x47c = 23.56194f;

		pFighter->FUN_0031aad0(pFighter->_GetFighterAnimationLength(pFighter->currentAnimType), pFighter->field_0x834);
	}
		break;
	case 0xa:
	case 0xb:
		pFighter = this->pOwner;
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		break;
	default:
		IMPLEMENTATION_GUARD();
		break;
	}

	uVar13 = this->pOwner->GetStateFlags(oldState) & 0xff800;
	if (uVar13 == 0x80000) {
		if ((this->pOwner->GetStateFlags(newState) & 0xff800) != 0x80000) {
			IMPLEMENTATION_GUARD(
				pFighter = this->pOwner;
			if (newState != -1) {
				CActor::Compute2DOrientationFromAngles((CActor*)pFighter);
				pFighter->flags = pFighter->flags & 0xffffefff;
			}
			pCVar2 = pFighter->pCollisionData;
			pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfffff7ff;
			(pFighter->pCollisionData)->actorFieldA = (CActor*)0x0;
			piVar3 = *(int**)&pFighter->field_0x854;
			if ((piVar3 != (int*)0x0) && (*piVar3 != 0)) {
				CAnimation::UnRegisterBone(*(CAnimation**)(*piVar3 + 0x28), piVar3[1]);
			})
		}
	}
	else {
		if (uVar13 == 0x8000) {
			if ((this->pOwner->GetStateFlags(newState) & 0xff800) != 0x8000) {
				IMPLEMENTATION_GUARD(
					_EndFighterHold(newState);)
			}
		}
		else {
			if (uVar13 == 0x4000) {
				if ((this->pOwner->GetStateFlags(newState) & 0xff800) != 0x4000) {
					IMPLEMENTATION_GUARD(
						_EndFighterRide(newState);)
				}
			}
			else {
				if (uVar13 == 0x2000) {
					if (newState == -1) {
						if ((this->pOwner->GetStateFlags(newState) & 0xff800) != 0x2000) {
							IMPLEMENTATION_GUARD(
								_EndFighterFlip();)
						}
					}
				}
			}
		}
	}

	if ((oldState - 0x17U < 5) && (4 < newState - 0x17U)) {
		this->pOwner->fightFlags = this->pOwner->fightFlags & 0xffffcfff;
	}

	pFighter = this->pOwner;
	if ((oldState - 0x1cU < 4) && (3 < newState - 0x1cU)) {
		IMPLEMENTATION_GUARD(
			StaticMeshComponentAdvanced::FUN_00406f90(&pFighter->staticMeshComponentAdvanced);)
	}

	if ((oldState == FIGHTER_HIT_STEP_BACK) || (oldState == 0x18)) {
		uVar13 = 0;
		puVar12 = gBoneIds_004343a0;
		//pCVar11 = pFighter;
		do {
			pFighter->pAnimationController->UnRegisterBone(*puVar12);

			IMPLEMENTATION_GUARD_FX(
			CFxHandle::FUN_00407310(pCVar11->field_0x550);)
			uVar13 = uVar13 + 1;
			puVar12 = puVar12 + 1;
			//pCVar11 = (CActorFighter*)&(pCVar11->characterBase).base.base.actorFieldS;
		} while (uVar13 < 2);
	}

	return;
}

int CBehaviourFighter::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorFighter* pCVar1;
	int iVar2;
	StateConfig* pSVar3;
	CBehaviour* pCVar4;
	uint uVar5;
	edF32VECTOR4* v0;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack16;

	uVar5 = this->pOwner->GetStateFlags(this->pOwner->actorState);

	if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
		GetPositionMsgParams* pPositionParams = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
		if (pPositionParams->field_0x0 == 5) {
			if ((this->pOwner->GetStateFlags(this->pOwner->actorState) & 0xff800) == 0x4000) {
				edF32Vector4ScaleHard(2.0f, &pPositionParams->vectorFieldB, &(this->pOwner->field_0x354->fighterAnatomyZones).field_0x10);
				pPositionParams->vectorFieldB.w = 0.0f;
				return 1;
			}
		}
	}
	else {
		if (msg == MESSAGE_REQUEST_CAMERA_TARGET) {
			if ((this->pOwner->GetStateFlags(this->pOwner->actorState) & 0xff800) == 0x4000) {
				edF32VECTOR4* pVectorParam = reinterpret_cast<edF32VECTOR4*>(pMsgParam);
				*pVectorParam = this->pOwner->field_0x354->currentLocation;
				return 1;
			}
		}
		else {
			if (msg == 0x66) {
				pCVar4 = this->pOwner->GetBehaviour(6);
				if (pCVar4 != (CBehaviour*)0x0) {
					return 1;
				}
			}
			else {
				if (msg == 0x65) {
					IMPLEMENTATION_GUARD(
					FUN_00311f80((long)(int)this->pOwner, (edF32VECTOR4*)pMsgParam, (int)pSender);)
					return 1;
				}

				if (msg == 100) {
					IMPLEMENTATION_GUARD(
					FUN_003178a0(pMsgParam, (int)this->pOwner);)
					return 1;
				}

				if (msg == 0x27) {
					return 1;
				}

				if (msg == 0x16) {
					IMPLEMENTATION_GUARD(
					uVar5 = this->pOwner->GetStateFlags(this->pOwner->actorState);
					edF32Vector4ScaleHard(*(float*)((int)pMsgParam + 0x10), &eStack16, (edF32VECTOR4*)pMsgParam);
					pCVar1 = this->pOwner;
					v0 = (pCVar1->characterBase).dynamicExt.aImpulseVelocities + 2;
					edF32Vector4AddHard(v0, v0, &eStack16);
					fVar6 = edF32Vector4GetDistHard(v0);
					(pCVar1->characterBase).dynamicExt.aImpulseVelocityMagnitudes[2] = fVar6;
					(*(this->pOwner->pVTable)->SetBehaviour)
						((CActor*)this->pOwner, this->behaviourId, -1, -1);
					return 1;)
				}

				if (msg == 2) {
					_msg_hit_param* pMsgHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
					iVar2 = pMsgHitParam->projectileType;
					if (iVar2 == 10) {
						this->pOwner->_UpdateDynForExplosion(pMsgHitParam);
						this->pOwner->hitFlags = this->pOwner->hitFlags | 0x30;
						_ManageHit(false);
						return 1;
					}

					if (iVar2 == 8) {
						if (((uVar5 & 0x2000000) == 0) ||
							(((uVar5 & 0x80000) != 0 && (pSender == this->pOwner->pAdversary)))) {
							this->pOwner->_UpdateDynForHit(1.0f, pMsgHitParam);
							_ManageHit((this->pOwner->hitFlags & 0x80U) == 0);
							return 1;
						}
					}
					else {
						if (iVar2 != 7) {
							return 0;
						}

						if (((uVar5 & 0x2000000) == 0) ||
							(((uVar5 & 0x80000) != 0 && (pSender == this->pOwner->pAdversary)))) {
							this->pOwner->_UpdateDynForHit(-1.0f, pMsgHitParam);
							_ManageHit((this->pOwner->hitFlags & 0x80U) == 0);
							return 1;
						}
					}
				}
			}
		}
	}

	return 0;
}

void CBehaviourFighter::SetInitialState()
{
	this->pOwner->SetInitialState();

	return;
}

bool CBehaviourFighter::Execute(s_fighter_action* param_2, s_fighter_action_param* param_3)
{
	int iVar1;
	bool bVar2;
	StateConfig* pSVar3;
	uint uVar4;
	s_fighter_action local_4;
	CActorFighter* pFighter;

	pFighter = this->pOwner;
	if ((pFighter->fightFlags & 1) == 0) {
		pFighter->_ValidateCommand(param_2, &local_4);

		uVar4 = pFighter->GetStateFlags(pFighter->actorState) & 0xff800;
		if (uVar4 == 0x8000) {
			IMPLEMENTATION_GUARD(
				pFighter->_Execute_Hold(&local_4, param_3);)
		}
		else {
			if (uVar4 == 0x4000) {
				IMPLEMENTATION_GUARD(
					pFighter->_Execute_Ride(&local_4, param_3);)
			}
			else {
				if (uVar4 == 0x2000) {
					IMPLEMENTATION_GUARD(
						pFighter->_Execute_Flip(&local_4, param_3);)
				}
				else {
					if (uVar4 == 0x800) {
						pFighter->_Execute_Std(&local_4, param_3);
					}
				}
			}
		}

		bVar2 = local_4.all != 0x0;
	}
	else {
		bVar2 = false;
	}

	return bVar2;
}

void CBehaviourFighter::_ManageHit(bool bPlayImpact)
{
	uint flags = this->pOwner->GetStateFlags(this->pOwner->actorState) & 0xff800;

	if (flags == 0x4000) {
		this->pOwner->_Ride_GetPossibleHit(bPlayImpact);
	}
	else {
		if (flags == 0x8000) {
			this->pOwner->_Hold_GetPossibleHit(bPlayImpact);
		}
		else {
			if ((flags == 0x80000) || (flags == 0x800)) {
				this->pOwner->_Std_GetPossibleHit(bPlayImpact);
			}
		}
	}
}

void CBehaviourFighter::_ManageExit()
{
	uint flags = this->pOwner->GetStateFlags(this->pOwner->actorState) & 0xff800;

	if (flags == 0x1000) {
		this->pOwner->_Proj_GetPossibleExit();
	}
	else {
		if (flags == 0x8000) {
			this->pOwner->_Hold_GetPossibleExit();
		}
		else {
			if (flags == 0x4000) {
				this->pOwner->_Ride_GetPossibleExit();
			}
			else {
				if ((flags == 0x80000) || (flags == 0x800)) {
					this->pOwner->_Std_GetPossibleExit();
				}
			}
		}
	}

	return;
}

bool CBehaviourFighter::Conditional_Execute(s_fighter_action* pAction, s_fighter_action_param* pActionParam)
{
	byte bVar1;
	bool bVar3;
	bool bVar4;
	long lVar5;

	this->pOwner->GetBehaviourFlags(this->pOwner->curBehaviourId);

	bVar4 = this->pOwner->FUN_003175e0(pAction, (float*)0x0);
	if ((bVar4 == false) && (this->pOwner->field_0x450.all != 0x0)) {
		*pAction = this->pOwner->field_0x450;
		pActionParam = &this->pOwner->field_0x454;
	}

	bVar4 = Execute(pAction, pActionParam);
	if (bVar4 != false) {
		if (((ulong)(pAction->field_0x0 & this->pOwner->field_0x44c) << 0x38) >> 0x3c == 0) {
			bVar1 = this->pOwner->field_0x44d;
			if (((pAction->field_0x1 & 0xf & bVar1 & 0xf) != 0) ||
				(bVar3 = false, ((ulong)(pAction->field_0x1 & bVar1) << 0x3a) >> 0x3e != 0)) goto LAB_00313210;
		}
		else {
		LAB_00313210:
			bVar3 = true;
		}

		if (bVar3) goto LAB_00313260;
	}

	if (this->pOwner->Func_0x19c() != 0) {
		this->pOwner->FUN_0031b7f0(pAction, pActionParam);
		return bVar4;
	}

LAB_00313260:
	this->pOwner->field_0x450.all = 0x0;
	return bVar4;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CBehaviourFighterProjected::Manage()
{
	bool bVar1;
	Timer* pTVar2;
	CLifeInterface* pLifeInterface;
	int iVar3;
	CBehaviourFighter* pCVar4;
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

	this->pOwner->UpdateFightCommand();

	pFighter = this->pOwner;
	switch (pFighter->actorState) {
	case 0x54:
		pFighter->ManageDyn(4.0f, 0xb, (CActorsTable*)0x0);
		if (pFighter->field_0x7d8 <= pFighter->timeInAir) {
			pFighter->SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
		}
		break;
	case FIGHTER_PROJECTED_HIT_FLY:
		pFighter->_StateFighterHitFly();
		break;
	case FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE:
		pFighter->_StateFighterHitFlyToSlide();
		break;
	case FIGHTER_PROJECTED_HIT_SLIDE:
		pFighter->_StateFighterHitSlide();
		break;
	case 0x58:
		pCVar4 = static_cast<CBehaviourFighter*>(pFighter->GetBehaviour(pFighter->curBehaviourId));
		pFighter->ManageDyn(4.0f, 0x233, (CActorsTable*)0x0);

		bVar1 = pFighter->IsFightRelated(pCVar4->behaviourId);
		if ((bVar1 == false) || ((pFighter->field_0x444 & 2) == 0)) {
			iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x77, (int)pFighter->field_0x7dc);
			fVar6 = 0.6f;
		}
		else {
			iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x79, (int)pFighter->field_0x7dc);
			fVar6 = 0.1f;
		}

		if (fVar6 < pFighter->timeInAir) {
			pFighter->SetState(FIGHTER_PROJECTED_HIT_WAKING_UP, iVar3);
		}
		break;
	case FIGHTER_PROJECTED_HIT_WAKING_UP:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pTVar2 = GetTimer();
		fVar7 = pFighter->field_0x7b0 + pFighter->field_0x7b4 * pTVar2->cutsceneDeltaTime;
		pFighter->field_0x7b0 = fVar7;

		edF32Matrix4FromAngAxisSoft(-fVar7, &eStack64, &pFighter->field_0x7a0);
		edF32Matrix4MulF32Matrix4Hard(&eStack64, &pFighter->field_0x760, &eStack64);
		edF32Matrix4ToEulerSoft(&eStack64, &local_50, "XYZ");

		pFighter->rotationEuler.xyz = local_50;

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pCVar4 = static_cast<CBehaviourFighter*>(pFighter->GetBehaviour(pFighter->curBehaviourId));
			pFighter->SetBehaviour(pCVar4->behaviourId, -1, -1);
		}
		break;
	case FIGHTER_PROJECTED_HIT_STAGGER_CHECK:
		pFighter->_StateFighterHitStaggerCheck();
		break;
	case 0x5b:
		fVar7 = edF32Between_2Pi(pFighter->rotationEuler.y + pFighter->field_0x7b4 * GetTimer()->cutsceneDeltaTime);
		pFighter->rotationEuler.y = fVar7;
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x5c, -1);
		}
	break;
	case 0x5c:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x5d, -1);
		}
		break;
	case 0x5d:
		fVar7 = pFighter->field_0x7b4 * pFighter->timeInAir;
		if (fVar7 <= pFighter->field_0x7b0) {
			pAnimation = pFighter->pAnimationController;

			if (pAnimation->IsCurrentLayerAnimEndReached(0)) goto LAB_00305528;
		}
		else {
		LAB_00305528:
			fVar7 = pFighter->field_0x7b0;
		}

		edF32Matrix4FromAngAxisSoft(-fVar7, &eStack144, &pFighter->field_0x7a0);
		edF32Matrix4MulF32Matrix4Hard(&eStack144, &pFighter->field_0x760, &eStack144);
		edF32Matrix4ToEulerSoft(&eStack144, &local_a0, "XYZ");

		pFighter->rotationEuler.xyz = local_a0;
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x5e, -1);
		}
		break;
	case 0x5e:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			fVar7 = pFighter->GetLifeInterface()->GetValue();
			if (fVar7 <= 0.0f) {
				iVar3 = 0x7f;
				if ((pFighter->fightFlags & 0x10) == 0) {
					iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x75, (int)pFighter->field_0x7dc);
				}
				else {
				}
				pFighter->SetState(0x62, iVar3);
			}
			else {
				iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x75, (int)pFighter->field_0x7dc);
				pFighter->SetState(0x58, iVar3);
			}
		}
		break;
	case 0x5f:
		fVar7 = pFighter->field_0x7b4 * pFighter->timeInAir;
		if (fVar7 <= pFighter->field_0x7b0) {
			pAnimation = pFighter->pAnimationController;
			if (pAnimation->IsCurrentLayerAnimEndReached(0)) goto LAB_00305798;
		}
		else {
		LAB_00305798:
			fVar7 = pFighter->field_0x7b0;
		}

		edF32Matrix4FromAngAxisSoft(-fVar7, &eStack224, &pFighter->field_0x7a0);
		edF32Matrix4MulF32Matrix4Hard(&eStack224, &pFighter->field_0x760, &eStack224);
		edF32Matrix4ToEulerSoft(&eStack224, &local_f0, "XYZ");
		pFighter->rotationEuler.xyz = local_f0;

		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x60, -1);
		}
		break;
	case 0x60:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			fVar7 = pFighter->GetLifeInterface()->GetValue();
			if (fVar7 <= 0.0f) {
				iVar3 = 0x7f;
				if ((pFighter->fightFlags & 0x10) == 0) {
					iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x75, (int)pFighter->field_0x7dc);
				}
				else {
				}
				pFighter->SetState(0x62, iVar3);
			}
			else {
				iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x75, (int)pFighter->field_0x7dc);
				pFighter->SetState(0x58, iVar3);
			}
		}
		break;
	case 0x62:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->ProcessDeath();
		}
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

	this->pOwner->scalarDynA.Reset();
	this->pOwner->scalarDynB.Reset();
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
		if ((pFighter->hitFlags & 2U) == 0) {
			pFighter->SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
		}
		else {
			pFighter->SetState(FIGHTER_PROJECTED_HIT_FLY, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->GetLifeInterfaceOther()->SetPriority(2);

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
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

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
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
	case FIGHTER_PROJECTED_HIT_FLY:
	case FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE:
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
	case FIGHTER_PROJECTED_HIT_SLIDE:
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
	case 0x58:
		pFighter = this->pOwner;
		pFighter->dynamic.speed = 0.0f;
		pFighter->ClearAllSumForceExt();
		pFighter->dynamicExt.normalizedTranslation.x = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.y = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.z = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.w = 0.0f;
		pFighter->dynamicExt.field_0x6c = 0.0f;
		break;
	case FIGHTER_PROJECTED_HIT_WAKING_UP:
		this->pOwner->_StateFighterHitWakingUpInit();
		break;
	case FIGHTER_PROJECTED_HIT_STAGGER_CHECK:
		pFighter = this->pOwner;
		pFighter->fightFlags = pFighter->fightFlags & 0xffffffdf;
		pFighter->field_0x7e4 = 0;
		pFighter->field_0x7dc = pFighter->field_0x800;
		SetVectorFromAngleY(pFighter->rotationEuler.y, &pFighter->field_0x7f0);
		edF32Vector4ScaleHard(-pFighter->field_0x7dc, &pFighter->field_0x7f0, &pFighter->field_0x7f0);
		pFighter->dynamic.speed = 0.0f;
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		break;
	case 0x5b:
		pFighter = this->pOwner;
		pFighter->field_0x7b4 = CActorFighter::_pStagger_check[pFighter->field_0x7e4] / pFighter->_GetFighterAnimationLength(pFighter->currentAnimType);
		break;
	case 0x5d:
		pFighter = this->pOwner;
		pFighter->_StateFighterHitStaggerFallInit(this->pOwner->field_0x7dc);
		break;
	case 0x5e:
		this->pOwner->dynamic.speed = 0.0f;
		break;
	case 0x5f:
		pFighter->_StateFighterHitStaggerFallInit(-1.0f);
		break;
	case 0x60:
		this->pOwner->dynamic.speed = 0.0f;
		break;
	case 0x62:
		pFighter = this->pOwner;
		pFighter->dynamic.speed = 0.0;
		pFighter->ClearAllSumForceExt();
		pFighter->dynamicExt.normalizedTranslation.x = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.y = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.z = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.w = 0.0f;
		pFighter->dynamicExt.field_0x6c = 0.0f;
		break;
	case 0x64:
		this->pOwner->_InterpretCollisions(1);
		break;
	case 0x5c:
		// Nothing
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
			if ((oldState == FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE) || (oldState == FIGHTER_PROJECTED_HIT_FLY)) {
				pFighter = this->pOwner;
				if ((newState == FIGHTER_PROJECTED_HIT_SLIDE) || (newState == 0x54)) {
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
	CActorFighter* pFighter;
	CActor* pCVar2;
	StateConfig* pSVar3;
	int result;
	uint uVar5;
	edF32VECTOR4* v0;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack16;

	if (msg == 0x27) {
		result = 0;
	}
	else {
		if (msg == 0x16) {
			IMPLEMENTATION_GUARD(
			edF32Vector4ScaleHard(*(float*)((int)pMsgParam + 0x10), &eStack16, (edF32VECTOR4*)pMsgParam);
			pFighter = this->pOwner;
			v0 = (pFighter->characterBase).dynamicExt.aImpulseVelocities + 2;
			edF32Vector4AddHard(v0, v0, &eStack16);
			fVar6 = edF32Vector4GetDistHard(v0);
			(pFighter->characterBase).dynamicExt.aImpulseVelocityMagnitudes[2] = fVar6;
			result = 1;)
		}
		else {
			if (msg == 2) {
				pFighter = this->pOwner;
				uVar5 = pFighter->GetStateFlags(pFighter->actorState);

				if ((uVar5 & 0x2000000) == 0) {
					_msg_hit_param* pMsgHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);

					int projType = pMsgHitParam->projectileType;

					if (projType == 10) {
						this->pOwner->_UpdateDynForExplosion(pMsgHitParam);
						_ManageHit(false);
						result = 1;
					}
					else {
						if (projType == 8) {
							pFighter = this->pOwner;
							pFighter->field_0x690 = pMsgHitParam->field_0x40;
							pFighter->hitDamage = pMsgHitParam->damage;
							edF32Vector4SafeNormalize0Hard(&pFighter->field_0x6a0, &pMsgHitParam->field_0x20);

							pCVar2 = pFighter->field_0x634;
							if (pFighter->actorsExcludeTable.IsInList(pCVar2) == false) {
								pFighter->actorsExcludeTable.Add(1.0f, pCVar2);
							}

							pFighter->hitFlags = pMsgHitParam->flags;
							pFighter->field_0x474 = pMsgHitParam->field_0x10;
							pFighter->field_0x6b0 = pMsgHitParam->field_0x30;
							pFighter->field_0x684 = pMsgHitParam->field_0x50;
							pFighter->field_0x686 = pMsgHitParam->field_0x52;

							if ((pFighter->field_0x684 & 1U) != 0) {
								pFighter->field_0x7a0 = pMsgHitParam->field_0x60;
								pFighter->field_0x7b4 = pMsgHitParam->field_0x70;
							}

							_ManageHit((pMsgHitParam->flags & 0x80) == 0);

							result = 1;
						}
						else {
							if (projType == 7) {
								pFighter = this->pOwner;
								pFighter->field_0x690 = pMsgHitParam->field_0x40;
								pFighter->hitDamage = pMsgHitParam->damage;
								edF32Vector4SafeNormalize0Hard(&pFighter->field_0x6a0, &pMsgHitParam->field_0x20);

								pCVar2 = pFighter->field_0x634;
								if (pFighter->actorsExcludeTable.IsInList(pCVar2) == false) {
									pFighter->actorsExcludeTable.Add(-1.0f, pCVar2);
								}

								pFighter->hitFlags = pMsgHitParam->flags;
								pFighter->field_0x474 = pMsgHitParam->field_0x10;
								pFighter->field_0x6b0 = pMsgHitParam->field_0x30;
								pFighter->field_0x684 = pMsgHitParam->field_0x50;
								pFighter->field_0x686 = pMsgHitParam->field_0x52;

								if ((pFighter->field_0x684 & 1U) != 0) {
									pFighter->field_0x7a0 = pMsgHitParam->field_0x60;
									pFighter->field_0x7b4 = pMsgHitParam->field_0x70;
								}

								_ManageHit((pMsgHitParam->flags & 0x80) == 0);

								result = 1;
							}
							else {
								result = 0;
							}
						}
					}
				}
				else {
					result = 0;
				}
			}
			else {
				result = 0;
			}
		}
	}

	return result;
}

bool CBehaviourFighterProjected::Execute(s_fighter_action* param_2, s_fighter_action_param* param_3)
{
	return false;
}

void CBehaviourFighterProjected::_ManageHit(bool bPlayImpact)
{
	this->pOwner->LifeDecrease(this->pOwner->hitDamage);
	this->pOwner->hitDamage = 0.0f;

	if ((this->pOwner->hitFlags & 1U) == 0) {
		(this->pOwner->field_0x6a0).y = 2.0f;
		edF32Vector4NormalizeHard(&this->pOwner->field_0x6a0, &this->pOwner->field_0x6a0);
		this->pOwner->field_0x6b0 = 5.0f;
		this->pOwner->field_0x686 = 1;
		this->pOwner->field_0x684 = 0xe;

		_ComputeDynamics();

		this->pOwner->SetState(-1, -1);
		this->pOwner->SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
	}
	else {
		_ComputeDynamics();

		this->pOwner->SetState(-1, -1);

		if (((this->pOwner->hitFlags & 2U) == 0) || ((this->pOwner->hitFlags & 4U) != 0)) {
			this->pOwner->SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
		}
		else {
			this->pOwner->SetState(FIGHTER_PROJECTED_HIT_FLY, -1);
		}
	}

	if (bPlayImpact == true) {
		IMPLEMENTATION_GUARD_FX(
		pCVar1 = this->pOwner;
		CActorFighter::PlayImpactFx
		(pCVar1, (long)(int)&pCVar1->field_0x690, &pCVar1->field_0x6a0, (ulong)((pCVar1->hitFlags & 1U) != 0), '\0');)
	}
	return;
}


// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
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
				pFighter = this->pOwner;
				uVar3 = pFighter->_SV_HIT_GetHitZoneFromImpact(&pFighter->fighterAnatomyZones, &pFighter->field_0x690, &pFighter->currentLocation);
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

static float _last_dir_time = 0.0f;

CInputAnalyser::CInputAnalyser()
{
	_last_dir_time = GetTimer()->scaledTotalTime;

	this->patternA.field_0x0uint = 0;
	this->patternA.nbInputs = 0;

	this->patternB.field_0x0uint = 0;
	this->patternB.nbInputs = 0;

	this->lastStickDirFlags = 0;
	
	this->field_0x10 = gF32Vector4UnitZ;

	this->flags = this->flags | 7;
	this->pComboB = (s_fighter_combo*)0x0;
	this->pComboA = (s_fighter_combo*)0x0;

	return;
}

float _max_dir_time;

void CInputAnalyser::_CumulateDirections(CPlayerInput* pInput, edF32VECTOR4* pDir)
{
	uint dirFlags;
	edF32VECTOR4 local_10;

	if (pDir == (edF32VECTOR4*)0x0) {
		local_10.x = pInput->aAnalogSticks[PAD_STICK_LEFT].x;
		local_10.y = 0.0f;
		local_10.z = pInput->aAnalogSticks[PAD_STICK_LEFT].y;
		local_10.w = 0.0f;
	}
	else {
		local_10.x = pDir->x;
		local_10.y = pDir->y;
		local_10.z = pDir->z;
		local_10.w = pDir->w;
	}

	dirFlags = FUN_00337d20(&local_10);
	if (dirFlags == this->lastStickDirFlags) {
		if (_max_dir_time <= GetTimer()->scaledTotalTime - _last_dir_time) {
			this->patternB.field_0x0uint = this->patternB.field_0x0uint & 0xfff00000 | (uint)((ulong)((ulong)(int)this->lastStickDirFlags << 0x2c) >> 0x2c);
			this->patternB.nbInputs = 1;
		}
	}
	else {
		_last_dir_time = GetTimer()->scaledTotalTime;
		this->lastStickDirFlags = dirFlags;

		if (4 < this->patternB.nbInputs) {
			this->patternB.field_0x0uint = this->patternB.field_0x0uint & 0xfff00000 | (uint)((ulong)((ulong)(int)(uint)((ulong)((ulong)(int)this->patternB.field_0x0uint << 0x2c) >> 0x30) << 0x2c) >> 0x2c);
			this->patternB.nbInputs = this->patternB.nbInputs - 1;
		}

		this->patternB.field_0x0uint = this->patternB.field_0x0uint & 0xfff00000 | (uint)((((ulong)(int)this->patternB.field_0x0uint & 0xfffffU | (ulong)(int)(this->lastStickDirFlags << ((this->patternB.nbInputs & 7) << 2))) << 0x2c) >> 0x2c);
		this->patternB.nbInputs = this->patternB.nbInputs + 1;
	}

	return;
}

int CInputAnalyser::Cumulate(CPlayerInput* pPlayerInput, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	byte bVar1;
	ushort uVar2;
	float puVar4;
	float puVar3;
	float puVar5;
	float fVar3;
	int iVar4;
	float fVar5;
	float fVar6;

	if (pPlayerInput != (CPlayerInput*)0x0) {
		if (this->patternB.nbInputs != 0) {
			puVar3 = edF32Vector4DotProductHard(param_3, &this->field_0x10);
			if (1.0f < puVar3) {
				puVar5 = 1.0f;
			}
			else {
				puVar5 = -1.0f;
				if (-1.0f <= puVar3) {
					puVar5 = puVar3;
				}
			}

			fVar3 = acosf(puVar5);
			if (0.0f <= param_3->z * (this->field_0x10).x - param_3->x * (this->field_0x10).z) {
				puVar4 = 1.0f;
			}
			else {
				puVar4 = -1.0f;
			}

			iVar4 = (int)((fVar3 * puVar4) / 0.7853982f + 0.5f);
			if (iVar4 != 0) {
				_SV_RotatePattern(iVar4);
			}
		}

		this->field_0x10 = *param_3;
		_CumulateDirections(pPlayerInput, param_4);

		if (pPlayerInput->aButtons[6].clickValue == 0.0f) {
			bVar1 = this->patternB.field_0x3byte;
			this->patternB.field_0x3byte = bVar1 & 0xf | (byte)(((uint)(((ulong)bVar1 << 0x38) >> 0x3c) & 0xe) << 4);
		}
		else {
			bVar1 = this->patternB.field_0x3byte;
			this->patternB.field_0x3byte = bVar1 & 0xf | (byte)(((uint)(((ulong)bVar1 << 0x38) >> 0x3c) | 1) << 4);
		}

		if (pPlayerInput->aButtons[5].clickValue == 0.0f) {
			bVar1 = this->patternB.field_0x3byte;
			this->patternB.field_0x3byte = bVar1 & 0xf | (byte)(((uint)(((ulong)bVar1 << 0x38) >> 0x3c) & 0xd) << 4);
		}
		else {
			bVar1 = this->patternB.field_0x3byte;
			this->patternB.field_0x3byte = bVar1 & 0xf | (byte)(((uint)(((ulong)bVar1 << 0x38) >> 0x3c) | 2) << 4);
		}

		if ((pPlayerInput->pressedBitfield & PAD_BITMASK_SQUARE) != 0) {
			uVar2 = this->patternB.field_0x2ushort;
			this->patternB.field_0x2ushort = uVar2 & 0xf00f | (ushort)(((uint)(((ulong)uVar2 << 0x34) >> 0x38) | 1) << 4);
		}

		if ((pPlayerInput->pressedBitfield & 0x80) != 0) {
			uVar2 = this->patternB.field_0x2ushort;
			this->patternB.field_0x2ushort = uVar2 & 0xf00f | (ushort)(((uint)(((ulong)uVar2 << 0x34) >> 0x38) | 2) << 4);
		}

		if ((pPlayerInput->pressedBitfield & 0x100) != 0) {
			uVar2 = this->patternB.field_0x2ushort;
			this->patternB.field_0x2ushort = uVar2 & 0xf00f | (ushort)(((uint)(((ulong)uVar2 << 0x34) >> 0x38) | 4) << 4);
		}

		if ((pPlayerInput->pressedBitfield & PAD_BITMASK_CROSS) != 0) {
			uVar2 = this->patternB.field_0x2ushort;
			this->patternB.field_0x2ushort = uVar2 & 0xf00f | (ushort)(((uint)(((ulong)uVar2 << 0x34) >> 0x38) | 8) << 4);
		}

		if ((this->patternB.field_0x2ushort << 0x34) >> 0x38 != 0) {
			this->patternA = this->patternB;
			this->patternB.field_0x0uint = 0;
			this->patternB.nbInputs = 0;

			this->lastStickDirFlags = 0;
			return 1;
		}
	}

	return 0;
}

// Should be in: D:/Projects/b-witch/ActorFighterInputServices.cpp
void CInputAnalyser::Reset()
{
	_last_dir_time = GetTimer()->scaledTotalTime;

	(this->patternA).field_0x0uint = 0;
	(this->patternA).nbInputs = 0;

	(this->patternB).field_0x0uint = 0;
	(this->patternB).nbInputs = 0;

	this->lastStickDirFlags = 0;

	this->field_0x10 = gF32Vector4UnitZ;

	return;
}

bool CInputAnalyser::Compare(s_input_pattern* pPatternA, s_input_pattern* pPatternB, s_input_pattern_cmp* pPatternCmp)
{
	bool bVar1;
	uint uVar2;
	uint uVar3;
	ulong uVar4;
	uint uVar5;
	int iVar6;
	uint uVar7;
	uint uVar8;
	uint uVar9;
	float fVar10;

	// Log out patterns
	FIGHTER_LOG(LogLevel::Info, "CInputAnalyser::Compare Pattern A: 0x{:x} 0x{:x}", pPatternA->field_0x0uint, pPatternA->nbInputs);
	FIGHTER_LOG(LogLevel::Info, "CInputAnalyser::Compare Pattern B: 0x{:x} 0x{:x}", pPatternB->field_0x0uint, pPatternB->nbInputs);

	pPatternCmp->field_0x0 = 0;
	pPatternCmp->field_0x8 = 0.0f;
	pPatternCmp->field_0x4 = pPatternA->nbInputs;
	iVar6 = 0;

	if ((pPatternA->field_0x2ushort & pPatternB->field_0x2ushort) == 0 ||
		(pPatternA->field_0x3byte != pPatternB->field_0x3byte) ||
		(pPatternA->nbInputs < pPatternB->nbInputs)) {
		bVar1 = false;
	}
	else {
		pPatternCmp->field_0x0 = 1;
		uVar5 = 0;
		for (uVar4 = ((ulong)((pPatternA->field_0x2ushort) ^ *(ushort*)((ulong)&pPatternB->field_0x0uint + 2)) <<
			0x34) >> 0x38; uVar4 != 0; uVar4 = ((uint)uVar4 >> 1)) {
			if ((uVar4 & 1) != 0) {
				uVar5 = uVar5 + 1;
			}
		}

		if ((int)uVar5 < 0) {
			fVar10 = (float)(uVar5 >> 1 | uVar5 & 1);
			fVar10 = fVar10 + fVar10;
		}
		else {
			fVar10 = (float)uVar5;
		}

		pPatternCmp->field_0x8 = -fVar10;
		uVar5 = 0;
		if (pPatternA->nbInputs != 0) {
			uVar3 = 0;
			uVar2 = 0;
			do {
				uVar8 = (uint)((ulong)((ulong)(uint)pPatternA->field_0x0uint << 0x2c) >> 0x2c) >> (uVar3 & 0x1f) & 0xf;
				uVar7 = (uint)((ulong)((ulong)(uint)pPatternB->field_0x0uint << 0x2c) >> 0x2c) >> (uVar2 & 0x1f) & 0xf;
				if ((uVar8 & uVar7) == 0) {
					if (pPatternA->nbInputs - uVar5 <= pPatternB->nbInputs - iVar6) {
						pPatternCmp->field_0x0 = 0;
						pPatternCmp->field_0x8 = 0.0f;
						pPatternCmp->field_0x4 = pPatternA->nbInputs;
						return false;
					}
				}
				else {
					uVar9 = 0;
					pPatternCmp->field_0x0 = pPatternCmp->field_0x0 + 1;
					for (uVar8 = uVar8 ^ uVar7; uVar8 != 0; uVar8 = uVar8 >> 1) {
						if ((uVar8 & 1) != 0) {
							uVar9 = uVar9 + 1;
						}
					}
					if ((int)uVar9 < 0) {
						fVar10 = (float)(uVar9 >> 1 | uVar9 & 1);
						fVar10 = fVar10 + fVar10;
					}
					else {
						fVar10 = (float)uVar9;
					}
					uVar2 = uVar2 + 4;
					iVar6 = iVar6 + 1;
					pPatternCmp->field_0x8 = pPatternCmp->field_0x8 - fVar10;
					pPatternCmp->field_0x4 = pPatternA->nbInputs - (uVar5 + 1);
				}

				uVar5 = uVar5 + 1;
				uVar3 = uVar3 + 4;
			} while (uVar5 < pPatternA->nbInputs);
		}

		uVar5 = pPatternCmp->field_0x0;
		if ((int)uVar5 < 0) {
			fVar10 = (float)(uVar5 >> 1 | uVar5 & 1);
			fVar10 = fVar10 + fVar10;
		}
		else {
			fVar10 = (float)uVar5;
		}

		bVar1 = true;
		pPatternCmp->field_0x8 = pPatternCmp->field_0x8 / fVar10;
	}

	return bVar1;
}

ushort _u16Rot_precompute[8] = { 0x1, 0x3, 0x2, 0x6, 0x4, 0xc, 0x8, 0x9 };

// Should be in: D:/Projects/b-witch/ActorFighterInputServices.cpp
void CInputAnalyser::_SV_RotatePattern(int index)
{
	uint uVar1;
	uint uVar2;
	uint uVar3;
	uint uVar4;
	ushort* puVar5;
	int iVar6;

	uVar4 = (this->patternB).field_0x0uint;
	(this->patternB).field_0x0uint = uVar4 & 0xfff00000;
	uVar3 = 0;
	if ((this->patternB).nbInputs != 0) {
		uVar2 = 0;
		do {
			if ((uVar4 & 0xf) != 0) {
				iVar6 = 0;
				for (puVar5 = _u16Rot_precompute; (iVar6 < 8 && ((uVar4 & 0xf) != *puVar5)); puVar5 = puVar5 + 1) {
					iVar6 = iVar6 + 1;
				}

				iVar6 = iVar6 + index;
				if (iVar6 < 0) {
					iVar6 = iVar6 + 8;
				}
				else {
					if (7 < iVar6) {
						iVar6 = iVar6 + -8;
					}
				}

				uVar1 = (this->patternB).field_0x0uint;

				(this->patternB).field_0x0uint = uVar1 & 0xfff00000 | (uint)((((ulong)(int)uVar1 & 0xfffffU | (ulong)(int)((uint)_u16Rot_precompute[iVar6] << (uVar2 & 0x1f)) & 0xffffU) << 0x2c) >> 0x2c);
			}
			uVar3 = uVar3 + 1;
			uVar4 = (uVar4 & 0xffff) >> 4;
			uVar2 = uVar2 + 4;
		} while (uVar3 < (this->patternB).nbInputs);
	}

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

void StaticMeshComponentAdvanced::SetHidden(ed_3D_Scene* pScene)
{
	StaticMeshComponent::SetHidden(pScene);
	this->field_0x61 = 0;

	return;
}

void StaticMeshComponentAdvanced::SetVisible(ed_3D_Scene* pScene)
{
	StaticMeshComponent::SetVisible(pScene);
	this->field_0x61 = 1;

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

// Should be in: D:/Projects/b-witch/ActorFighterServices.cpp
void CFighterExcludedTable::EmptyByTime(float time)
{
	s_fighter_hit_exclusion sStack16;
	s_fighter_hit_exclusion* pCVar1;
	int iVar2;

	iVar2 = 0;
	pCVar1 = this->aEntries;
	if (0 < this->nbEntries) {
		do {
			pCVar1->currentDuration = pCVar1->currentDuration + time;

			const float fVar3 = pCVar1->maxDuration;
			if ((fVar3 < pCVar1->currentDuration) && (fVar3 != -1.0f)) {
				Pop(&sStack16, iVar2);
			}

			iVar2 = iVar2 + 1;
			pCVar1 = pCVar1 + 1;
		} while (iVar2 < this->nbEntries);
	}

	return;
}

bool CFighterExcludedTable::IsInList(CActor* pActor)
{
	int iVar1 = this->nbEntries;
	int iVar6 = 0;
	s_fighter_hit_exclusion* pCVar5 = this->aEntries;
	if (0 < iVar1) {
		do {
			if (pCVar5->pActor == pActor) return true;

			iVar6 = iVar6 + 1;
			pCVar5 = pCVar5 + 1;
		} while (iVar6 < iVar1);
	}

	return false;
}

void CFighterExcludedTable::Add(float maxDuration, CActor* pActor)
{
	if (this->nbEntries < 6) {
		this->aEntries[this->nbEntries].pActor = pActor;
		this->aEntries[this->nbEntries].maxDuration = maxDuration;
		this->aEntries[this->nbEntries].currentDuration = 0.0f;

		this->nbEntries = this->nbEntries + 1;
	}

	return;
}