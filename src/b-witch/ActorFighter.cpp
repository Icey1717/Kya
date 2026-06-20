#include "ActorFighter.h"
#include "MathOps.h"
#include "TimeController.h"
#include "kya.h"
#include "FileManager3D.h"
#include "InputManager.h"
#include "CollisionRay.h"
#include "ActorManager.h"
#include "ActorWind.h"
#include "ActorHero.h"
#include "LargeObject.h"
#include "CameraViewManager.h"
#include "ActorWeapon.h"
#include "DlistManager.h"
#include "ed3D/ed3DG2D.h"

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

PlayerSubStruct_64 PlayerSubStruct_64_ARRAY_0048f900[4];

void CActorFighter::Term()
{
	PlayerSubStruct_64* ppuVar1;
	uint uVar2;

	CActorAutonomous::Term();

	uVar2 = 0;
	ppuVar1 = PlayerSubStruct_64_ARRAY_0048f900;
	do {
		if ((ppuVar1->flags & 1) != 0) {
			ppuVar1->Term();
		}

		uVar2 = uVar2 + 1;
		ppuVar1 = ppuVar1 + 1;
	} while (uVar2 < 4);

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

void CActorFighter::Draw()
{
	CActorFighter* pCVar1;
	uint uVar2;

	CActorAutonomous::Draw();

	uVar2 = 0;
	if (this->field_0x630 != 0) {
		do {
			this->field_0x610[uVar2].pPlayerSubStruct->Draw();
			uVar2 = uVar2 + 1;
		} while (uVar2 < this->field_0x630);
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
	PlayerSubStruct_64* pCurSubStruct;
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

	this->field_0x410 = peVar1->position;
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
		this->fightFlags = (this->fightFlags & ~FIGHT_FLAG_USE_GENERIC_DEATH_ANIM);
	}
	else {
		this->fightFlags = (this->fightFlags | FIGHT_FLAG_USE_GENERIC_DEATH_ANIM);
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

	uVar2 = 0;
	pCurSubStruct = PlayerSubStruct_64_ARRAY_0048f900;
	do {
		if ((pCurSubStruct->flags & 1) == 0) {
			pCurSubStruct->Init(4);
		}

		uVar2 = uVar2 + 1;
		pCurSubStruct = pCurSubStruct + 1;
	} while (uVar2 < 4);

	this->field_0x550[0].Init(this->field_0x52c);
	this->field_0x550[1].Init(this->field_0x52c);

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
			bVar1 = IsFightRelated(this->curBehaviourId);
			if (bVar1 == false) {
				SetFightBehaviour();
				pCVar3 = GetBehaviour(this->curBehaviourId);
				iVar5 = pCVar3->InterpretMessage(pSender, 0x27, pMsgParam);
			}
			else {
				iVar5 = 0;
			}
		}
		else {
			if ((msg == MESSAGE_CAUGHT) || (msg == 0x65)) {
				if (((GetStateFlags(this->actorState) & 0x2000000) == 0) &&
					(bVar1 = IsFightRelated(this->curBehaviourId), bVar1 == false)) {
					SetFightBehaviour();
					pCVar3 = GetBehaviour(this->curBehaviourId);
					iVar5 = pCVar3->InterpretMessage(pSender, msg, pMsgParam);
				}
				else {
					iVar5 = 0;
				}
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

void CActorFighter::TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* pTieReferenceMatrix)
{
	int iVar1;
	CActor* pCVar2;
	uint stateFlags;

	if (this->curBehaviourId == 3) {
		stateFlags = GetStateFlags(this->actorState);

		if ((stateFlags & 0xff800) == FIGHTER_EXECUTE_FLAGS_HOLD) {
			pCVar2 = this->field_0x354;
			if (pCVar2 != (CActor*)0x0) {
				pCVar2->TieToActor(pTieActor, carryMethod, param_4, pTieReferenceMatrix);
			}
		}
		else {
			if (((stateFlags & 0xff800) == FIGHTER_EXECUTE_FLAGS_STD) &&
				((iVar1 = this->actorState, iVar1 - 0x14U < 2 || (iVar1 == FIGHTER_FLIP_OFF_ME_B)))) {
				pAdversary->TieToActor(pTieActor, carryMethod, param_4, pTieReferenceMatrix);
			}
		}
	}

	CActor::TieToActor(pTieActor, carryMethod, param_4, pTieReferenceMatrix);

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

bool CActorFighter::FUN_0031cdb0()
{
	return this->actorState - 0x5a < 5;
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

void CActorFighter::Func_0x170()
{
	return;
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

edF32VECTOR4* CActorFighter::GetAdversaryHeldPos()
{
	edF32VECTOR4* pHeldPos;

	if (this->field_0x354 == (CActorFighter*)0x0) {
		pHeldPos = &this->currentLocation;
	}
	else {
		this->field_0x354->_ComputeLogicalPos(&this->currentLocation);
		pHeldPos = &this->field_0x354->logicalPosition;
	}
	return pHeldPos;
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

		if ((uVar3 & 0xff800) != FIGHTER_EXECUTE_FLAGS_STD) {
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

bool CActorFighter::Func_0x1b4(int state)
{
	return state - 0x65U < 3;
}

bool CActorFighter::Func_0x1c0(s_fighter_combo* pCombo)
{
	return true;
}

int CActorFighter::InterpretSlaveMessage(int msg, void* pParams)
{
	switch (msg) {
	case 0:
		SetState(0x37, -1);
		break;
	case 0xb:
		SetState(6, this->standAnim);
		break;
	default:
		IMPLEMENTATION_GUARD();
	}

	return 0;
}

int CActorFighter::NotifyMaster(int msg, void* pParams)
{
	return this->field_0x354->InterpretSlaveMessage(msg, pParams);
}

// Should be in: D:/Projects/b-witch/ActorFighter.h
int CActorFighter::UpdateFightCommand()
{
	return 0;
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
			if (((uVar5 == FIGHTER_EXECUTE_FLAGS_HOLD) || (uVar5 == FIGHTER_EXECUTE_FLAGS_RIDE)) || (uVar5 == FIGHTER_EXECUTE_FLAGS_FLIP)) {
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
			if (((uVar5 == FIGHTER_EXECUTE_FLAGS_HOLD) & 0xff800) || ((uVar5 & 0xff800) == FIGHTER_EXECUTE_FLAGS_RIDE)) {
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

	bVar1 = pAction->actionByte;
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
					this->pNextAction.pBlow = (s_fighter_blow*)0x0;
					this->nextActionType = NEXT_ACTION_TYPE_NONE;

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

						this->pNextAction.pBlow = (s_fighter_blow*)0x0;
						this->nextActionType = NEXT_ACTION_TYPE_NONE;
						this->pExecutingGrab = reinterpret_cast<s_fighter_grab*>(pParam->pData);
						this->field_0x474 = pExecutingGrab->field_0xc;
						SetState(0x6b, this->pExecutingGrab->field_0x80.animId);
						return;
					}

					this->pNextAction.pGrab = reinterpret_cast<s_fighter_grab*>(pParam->pData);
					this->nextActionType = NEXT_ACTION_TYPE_GRAB;
				}
				else {
					if ((uVar11 == 10) || (uVar11 == 2)) {
						this->pBlow = reinterpret_cast<s_fighter_blow*>(pParam->pData);
						this->field_0x474 = pBlow->field_0xc;
						SetState(FIGHTER_GRAB_BEGIN, pBlow->blowStageBegin.animId);
						return;
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

							this->pNextAction.pBlow = (s_fighter_blow*)0x0;
							this->nextActionType = NEXT_ACTION_TYPE_NONE;
							this->pBlow = reinterpret_cast<s_fighter_blow*>(pParam->pData);
							this->field_0x474 = this->pBlow->field_0xc;
							SetState(FIGHTER_BLOW_BEGIN, this->pBlow->blowStageBegin.animId);
							return;
						}

						this->pNextAction.pBlow = reinterpret_cast<s_fighter_blow*>(pParam->pData);
						this->nextActionType = NEXT_ACTION_TYPE_BLOW;
					}
				}
			}

			bVar1 = pAction->moveByte;
			if (((ulong)bVar1 & 0xf) == 0xc) {
				if (((ulong)bVar1 << 0x38) >> 0x3c == 2) {
					this->field_0x36c = this->field_0x36c | 1;
					_BuildCommandFromAbsoluteVector(pParam->field_0x0, &local_4, &eStack128);
					this->field_0x44c = this->field_0x44c & 0xf0 | local_4.moveByte & 0xf;
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
					(this->field_0x36c = this->field_0x36c | 1, ((ulong)pAction->moveByte << 0x38) >> 0x3c != 2)) {
					this->scalarDynForward.BuildFromSpeedTime(this->field_0x4cc, this->field_0x4cc, GetTimer()->cutsceneDeltaTime);
				}
			}

			uVar11 = ((ulong)pAction->moveByte << 0x38) >> 0x3c;
			if (uVar11 != 8) {
				if (uVar11 == 4) {
					this->scalarDynJump.BuildFromSpeedDistTime(this->field_0x4f8, 0.0f, this->field_0x4f4, 0.22f);
					this->field_0x3f4 = 1.0f;
					this->field_0x474 = 0.0f;
					SetState(10, -1);
				}
				else {
					if (uVar11 == 2) {
						if ((pAction->actionByte & 0xf) == 0) {
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
							this->scalarDynForward.BuildFromSpeedDistTime(v1$12742, v2$12743, distance, fVar13);
							CScalarDyn::Reset(&this->scalarDynLateral);
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

					this->field_0x44c = this->field_0x44c & 0xf0 | local_8.moveByte & 0xf;
					this->field_0x36c = this->field_0x36c & 0xfffffffe;

					static float FLOAT_00448b28 = 2.0f;
					edF32Vector4ScaleHard(this->field_0x6b0 * FLOAT_00448b28 * 0.040625f, &local_40, &eStack80);
					edF32Vector4ScaleHard(this->field_0x6b0, &local_60, &eStack80);
					this->scalarDynForward.BuildFromSpeedDistTime(fabs(local_60.z), 0.0f, fabs(local_40.z), fVar9);
					this->scalarDynLateral.BuildFromSpeedDistTime(fabs(local_60.x), 0.0f, fabs(local_40.x), fVar9);
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
		if ((Func_0x1a8() == 0) && ((this->fightFlags & FIGHT_FLAG_ACTION_LOCKED) == 0)) {
			pCVar2 = reinterpret_cast<CBehaviourFighter*>(GetBehaviour(this->curBehaviourId));
			SetBehaviour(pCVar2->behaviourId, -1, -1);
		}
		else {
			if ((this->actorState - 0x17U < 5) || (AcquireAdversary(), this->pAdversary != (CActorFighter*)0x0)) {
				if ((this->fightFlags & FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK) != 0) {
					iVar3 = GetProjectedBehaviour();
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
	CActorFighter* pColFighter;
	bool bVar2;
	uint nbCol;
	s_fighter_collision_desc* pDescIt;
	CActorFighter* pCaughtAdversary;
	uint curIndex;
	s_fighter_collision_desc aFighterCollisionDescs[2];

	curIndex = 0;
	pCaughtAdversary = (CActorFighter*)0x0;
	nbCol = _SV_HIT_FightCollisionsGet(aFighterCollisionDescs, 2, 8);
	pDescIt = aFighterCollisionDescs;

	while ((curIndex < nbCol && (pCaughtAdversary == (CActorFighter*)0x0))) {
		pColFighter = static_cast<CActorFighter*>(pDescIt->pActor);
		bVar2 = Func_0x190(pColFighter);
		if (bVar2 != false) {
			pCaughtAdversary = pColFighter;
		}

		pDescIt = pDescIt + 1;
		curIndex = curIndex + 1;
	}

	return pCaughtAdversary;
}

void CActorFighter::_StateFighterRun(CActorsTable* pTable)
{
	_ManageFighterDyn(0x1d, 0x1002023b, pTable);

	if (this->dynamic.speed != 0.0) {
		RunInternal(this->adversaryAngleDiff, &this->dynamic.rotationQuat);
	}

	if (this->scalarDynForward.IsFinished() != false) {
		SetState(6, this->standAnim);
	}
	return;
}

void CActorFighter::_Ride_GetPossibleHit(bool bPlayImpact)
{
	return _Std_GetPossibleHit(bPlayImpact);
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

void CActorFighter::_BeginFighterFlip()
{
	CCollision* pCVar1;

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffefff;
	(this->pCollisionData)->actorFieldA = this->pAdversary;
	this->fightFlags = this->fightFlags & ~FIGHT_FLAG_FLIP_TO_JUMP;

	if ((this->flags & 0x1000) != 0) {
		Compute2DOrientationFromAngles();
		this->flags = this->flags & 0xffffefff;
	}

	return;
}

void CActorFighter::_Execute_Flip(s_fighter_action* pAction, s_fighter_action_param* pParam)
{
	if ((static_cast<ulong>(pAction->moveByte) << 0x38) >> 0x3c == 4) {
		this->fightFlags = this->fightFlags | FIGHT_FLAG_FLIP_TO_JUMP;
	}

	if ((pAction->actionByte & 0xf) == 1) {
		this->pBlow = static_cast<s_fighter_blow*>(pParam->pData);
		this->field_0x474 = this->pBlow->field_0xc;
		SetState(0x65, (this->pBlow->blowStageBegin).animId);
	}

	return;
}

void CActorFighter::_EndFighterFlip()
{
	CCollision* pCVar1;

	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
	(this->pCollisionData)->actorFieldA = (CActor*)0x0;

	return;
}

void CActorFighter::_BeginFighterRide()
{
	CCollision* pCVar1;

	this->field_0x354 = this->pAdversary;
	this->pAdversary = (CActorFighter*)0x0;
	SetAdversary(this->field_0x354);
	this->field_0x354->SetBehaviour(FIGHTER_BEHAVIOUR_RIDDEN, -1, -1);

	LinkToActor(this->field_0x354, 0x4146416f, 1);

	if ((this->flags & 0x1000) == 0) {
		GetAnglesFromVector((edF32VECTOR3*)&this->rotationEuler, &this->rotationQuat);
		this->flags = this->flags | 0x1000;
	}

	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffefff;

	SetLookingAtOff();
	EnableFightCamera();

	return;
}

void CActorFighter::_Execute_Ride(s_fighter_action* pAction, s_fighter_action_param* pParam)
{
	CBehaviourFighter* pBehaviour;
	ulong uVar1;
	_msg_hit_param local_120;
	_msg_hit_param local_a0;
	s_fighter_action_param local_18;
	_msg_hit_param* local_c;
	_msg_hit_param* local_8;
	s_fighter_action local_4;

	local_4.all = (pAction->moveByte & 0xffffff0f);
	local_18.field_0x0 = pParam->field_0x0;
	local_18.field_0x4 = 0;
	pBehaviour = static_cast<CBehaviourFighter*>(this->field_0x354->GetBehaviour(this->field_0x354->curBehaviourId));
	pBehaviour->Conditional_Execute(&local_4, &local_18);
	uVar1 = (static_cast<ulong>(pAction->moveByte) << 0x38) >> 0x3c;
	if ((uVar1 == 8) || (uVar1 != 4)) {
		if ((pAction->actionByte & 0xf) == 1) {
			local_120.projectileType = 7;
			local_120.flags = 0;
			local_c = &local_120;
			local_120.field_0x50 = 0;
			local_120.field_0x30 = 0.0f;
			local_120.field_0x10 = 0.0f;
			local_120.damage = this->hitMultiplier;
			DoMessage(this->field_0x354, MESSAGE_KICKED, local_c);
			SetState(0x21, -1);
		}
	}
	else {
		local_a0.projectileType = 7;
		local_a0.flags = 1;
		local_a0.field_0x50 = 1;
		local_a0.field_0x60.x = this->rotationQuat.z;
		local_a0.field_0x60.y = 0.0f;
		local_a0.field_0x60.z = -this->rotationQuat.x;
		local_a0.field_0x60.w = 0.0f;

		local_a0.field_0x20.x = this->rotationQuat.x;
		local_a0.field_0x20.y = 0.3f;
		local_a0.field_0x20.z = this->rotationQuat.z;
		local_a0.field_0x20.w = 0.0f;

		edF32Vector4NormalizeHard(&local_a0.field_0x20, &local_a0.field_0x20);
		local_a0.damage = 0.0f;
		local_a0.field_0x30 = 10.0f;
		local_a0.field_0x10 = 0.0f;
		local_a0.field_0x70 = 5.0f;
		local_8 = &local_a0;
		DoMessage(this->field_0x354, MESSAGE_KICKED, local_8);
		this->scalarDynJump.BuildFromSpeedDist(this->field_0x4f8, 0.0f, this->field_0x4f4 / 2.0f);
		this->field_0x3f4 = 1.0f;
		SetState(10, -1);
	}

	return;
}

void CActorFighter::_EndFighterRide(int newState)
{
	CActorFighter* pActor;
	CCollision* pCVar1;
	CBehaviourFighterRidden* pCVar2;

	UnlinkFromActor();
	pActor = this->field_0x354;
	if ((pActor->curBehaviourId == FIGHTER_BEHAVIOUR_RIDDEN) && (pActor != this->field_0x634)) {
		pCVar2 = static_cast<CBehaviourFighterRidden*>(pActor->GetBehaviour(pActor->curBehaviourId));
		this->field_0x354->SetBehaviour(pCVar2->behaviourId, -1, -1);
	}

	this->pAdversary = this->field_0x354;
	this->field_0x354 = (CActorFighter*)0x0;

	if (((this->flags & 0x1000) != 0) && (newState != -1)) {
		Compute2DOrientationFromAngles();
		this->flags = this->flags & 0xffffefff;
	}

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;

	this->dynamicExt.field_0x6c = 0.0f;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
	EnableFightCamera();

	return;
}

void CActorFighter::_SetAbsoluteSpeedOnGround(float param_1, edF32VECTOR4* param_3)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR4 local_60;
	edF32MATRIX4 auStack80;
	edF32VECTOR3 eStack16;

	local_60 = {};

	if ((this->flags & 0x1000) == 0) {
		GetAnglesFromVector(&eStack16, &this->rotationQuat);
		edF32Matrix4FromEulerSoft(&auStack80, &eStack16, "XYZ");
	}
	else {
		edF32Matrix4FromEulerSoft(&auStack80, &this->rotationEuler.xyz, "XYZ");
	}

	local_60.z = edF32Vector4DotProductHard(param_3, &auStack80.rowY);
	local_60.x = edF32Vector4DotProductHard(param_3, &auStack80.rowX);
	_SetRelativeSpeedOnGround(param_1, &local_60);

	return;
}

void CActorFighter::_Execute_Hold(s_fighter_action* pAction, s_fighter_action_param* pParam)
{
	byte bVar2;
	ulong uVar3;
	edF32VECTOR4 local_10;

	if ((pAction->moveByte & 0xf) == 0xc) {
		this->field_0x36c = this->field_0x36c & 0xfffffffe;
		local_10.x = pParam->field_0x0->x;
		local_10.y = 0.0f;
		local_10.z = pParam->field_0x0->z;
		local_10.w = 0.0f;
		edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
		edF32Vector4ScaleHard(this->field_0x4cc, &local_10, &local_10);
		_SetAbsoluteSpeedOnGround(GetTimer()->cutsceneDeltaTime, &local_10);
	}
	else {
		if ((pAction->moveByte & 0xf) != 0) {
			this->field_0x36c = this->field_0x36c | 1;
			this->scalarDynForward.BuildFromSpeedTime(this->field_0x4cc, this->field_0x4cc, GetTimer()->cutsceneDeltaTime);
		}
	}

	uVar3 = (static_cast<ulong>(pAction->moveByte) << 0x38) >> 0x3c;
	if ((uVar3 != 4) && (uVar3 == 1)) {
		SetState(FIGHTER_HOLD_RUN, -1);
	}

	bVar2 = pAction->actionByte & 0xf;
	if (((bVar2 == 4) || (bVar2 == 2)) || (bVar2 == 1)) {
		SetState(FIGHTER_HOLD_PUSH_PREPARE, -1);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterSlave.cpp
void CActorFighter::_BeginFighterHold()
{
	this->field_0x354 = this->pAdversary;
	SetAdversary(this->field_0x354);

	this->field_0x354->SetBehaviour(FIGHTER_BEHAVIOUR_SLAVE, -1, -1);
	this->pAdversary = (CActorFighter*)0x0;
	this->fightFlags = this->fightFlags & 0xfffff8ff;
	if (this->field_0x518 != 0) {
		this->pAnimationController->RegisterBone(this->field_0x518);
	}

	SetLookingAtOff();
	EnableFightCamera();

	return;
}

void CActorFighter::_EndFighterHold()
{
	int iVar1;
	CBehaviourFighterSlave* pCVar2;
	CActorFighter* pFighter;

	pFighter = this->field_0x354;
	if (pFighter != (CActorFighter*)0x0) {
		if (pFighter->curBehaviourId == 6) {
			iVar1 = pFighter->actorState;
			if ((iVar1 == 0x40) || (iVar1 == 0x3f)) {
				pFighter->fightFlags = static_cast<uint>((uint)pFighter->fightFlags | 0x4000);
				this->field_0x354->SetBehaviour(FIGHTER_BEHAVIOUR_DEFAULT, -1, -1);
			}
			else {
				pCVar2 = static_cast<CBehaviourFighterSlave*>(pFighter->GetBehaviour(pFighter->curBehaviourId));
				this->field_0x354->SetBehaviour(pCVar2->behaviourId, -1, -1);
			}
		}

		this->pAdversary = this->field_0x354;
		this->field_0x354 = (CActorFighter*)0x0;
		EnableFightCamera();
	}

	this->fightFlags = this->fightFlags & 0xfffffbff;
	this->field_0xa5c = 0.0f;

	if (this->field_0x518 != 0) {
		this->pAnimationController->UnRegisterBone(this->field_0x518);
	}

	SetLookingAtOn();

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
				char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
				AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);
				peVar1 = pAnimator->pAnimKeyTableEntry;
				if (peVar1->keyIndex_0x8.asKey == 4) {
					pValue->field_0xc = this->field_0x4e0.y;
					pValue->field_0x18 = this->field_0x4e0.z;
					fVar6 = this->field_0x4e0.x;
					if (0.0f <= fVar6) {
						pValue->field_0x14 = fVar6;
						pValue->field_0x10 = 0.0f;
					}
					else {
						pValue->field_0x10 = -fVar6;
						pValue->field_0x14 = 0.0f;
					}
				}
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
								assert(pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey == 8);
								peVar1 = pAnimator->pAnimKeyTableEntry;
								if ((this->fightFlags & FIGHT_FLAG_MOVE_BLEND_MIRRORED) == 0) {
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

	this->fightFlags = (this->fightFlags | FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK);
	this->fightFlags = (this->fightFlags | FIGHT_FLAG_MOVE_BLEND_MIRRORED);
	this->fightFlags = (this->fightFlags & ~FIGHT_FLAG_ACTION_LOCKED);
	this->fightFlags = (this->fightFlags & ~FIGHT_FLAG_STAGGER_CHECK_COMPLETE);
	this->fightFlags = (this->fightFlags & ~FIGHT_FLAG_ATTACK_CONNECTED);
	this->fightFlags = (this->fightFlags & ~FIGHT_FLAG_EXTRA_HIT_TRACE_BONE_REGISTERED);
	this->fightFlags = (this->fightFlags & 0xfffffeff);
	this->fightFlags = (this->fightFlags & ~FIGHT_FLAG_IN_WIND_AREA);
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

	this->scalarDynForward.Reset();
	this->scalarDynLateral.Reset();
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
	this->field_0x7d8 = 0.09f;
	this->field_0x6c4 = 0.1f;
	this->field_0x6c8 = 1.0f;
	this->field_0x6cc = 1.0f;
	this->field_0x7e4 = 0;
	this->field_0x7f0 = gF32Vector4UnitZ;

	this->fighterAnatomyZones.field_0x0 = this->fighterAnatomyZones.field_0x10;

	this->pBlow = 0;
	this->pExecutingGrab = (s_fighter_grab*)0;
	this->field_0x834 = (s_fighter_blow*)0;
	this->nextActionType = NEXT_ACTION_TYPE_NONE;
	this->pNextAction.pBlow = (s_fighter_blow*)0x0;
	this->comboFlags = 0xffffffff;

	this->pFighterCombo = (s_fighter_combo*)0x0;

	this->field_0x630 = 0;
	this->nbFightCollisions = 0;
	this->field_0xa50 = 0;
	this->field_0xa5c = 0.0f;
	this->field_0xa58 = 0.0f;
	this->field_0xa54 = 0.0f;
	this->field_0xa60 = 1.0f;
	this->field_0xa70 = 0.1f;
	this->field_0xa74 = 0.1f;
	this->field_0xa78 = 3;
	this->field_0x47c = 23.56194f;
	this->field_0x4c0 = 1.75f;
	this->field_0x4c4 = 2.5f;
	this->field_0x4c8 = 1.178097f;
	this->field_0x4cc = 4.0f;
	this->field_0x4f4 = 1.2f;
	this->field_0x4f8 = 17.0f;
	this->field_0x508 = 3.5f;
	this->field_0x504 = 7.0f;
	this->field_0x500 = 0.0f;
	this->standAnim = -1;

	return;
}

bool CActorFighter::Execute(s_fighter_action* pAction, s_fighter_action_param* pParams)
{
	int iVar1;
	bool bSuccess;
	StateConfig* pSVar3;
	uint uVar4;
	s_fighter_action local_4;

	if ((this->fightFlags & FIGHT_FLAG_ACTION_LOCKED) == 0) {
		_ValidateCommand(pAction, &local_4);

		iVar1 = this->actorState;
		uVar4 = GetStateFlags(this->actorState);
		uVar4 = uVar4 & 0xff800;
		if (uVar4 == FIGHTER_EXECUTE_FLAGS_HOLD) {
			IMPLEMENTATION_GUARD(
			(*(code*)(this->pVTable)->_Execute_Hold)(&local_4, pParams);)
		}
		else {
			if (uVar4 == FIGHTER_EXECUTE_FLAGS_RIDE) {
				IMPLEMENTATION_GUARD(
				(*(code*)(this->pVTable)->_Execute_Ride)(&local_4, pParams);)
			}
			else {
				if (uVar4 == FIGHTER_EXECUTE_FLAGS_FLIP) {
					_Execute_Flip(&local_4, pParams);
				}
				else {
					if (uVar4 == FIGHTER_EXECUTE_FLAGS_STD) {
						_Execute_Std(&local_4, pParams);
					}
				}
			}
		}
		bSuccess = local_4.all != 0x0;
	}
	else {
		bSuccess = false;
	}

	return bSuccess;
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

				this->scalarDynForward.Integrate(t);
				this->scalarDynLateral.Integrate(t);

				if ((flags & 0x10) != 0) {
					bVar3 = this->scalarDynForward.IsFinished();
					if (bVar3 != false) {
						this->scalarDynForward.Stop();
					}

					bVar3 = this->scalarDynLateral.IsFinished();
					if (bVar3 != false) {
						this->scalarDynLateral.Stop();
					}
				}

				edF32Vector4ScaleHard((this->scalarDynForward).field_0x20, &local_90, &local_90);
				edF32Vector4ScaleHard((this->scalarDynLateral).field_0x20, &local_a0, &local_a0);
				edF32Vector4AddHard(&newDynRotQuat, &local_90, &local_a0);
				edF32Vector4ScaleHard(t, &toAdversaryNormalized, &newDynRotQuat);
				edF32Vector4SubHard(&toAdversaryNormalized, &toAdversary, &toAdversaryNormalized);
			}
			else {
				this->scalarDynForward.Integrate(t);

				if ((flags & 0x10) != 0) {
					bVar3 = this->scalarDynForward.IsFinished();
					if (bVar3 != false) {
						this->scalarDynForward.Stop();
					}
				}

				bVar8 = this->field_0x44c & 0xf;
				fVar13 = (this->scalarDynForward).field_0x20 * t;
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

			this->dynamicExt.instanceIndex.y = (this->scalarDynJump).field_0x20 * this->field_0x3f4;
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
		if (((GetStateFlags(this->actorState) & 0xff800) == FIGHTER_EXECUTE_FLAGS_HOLD) && ((this->fightFlags & 0x400) != 0)) {
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
			if (uVar4 == FIGHTER_EXECUTE_FLAGS_STD) {
				if ((this->fightFlags & 0x1000) != 0) {
					this->logicalPosition = this->field_0x4a0;
				}
			}
			else {
				if (uVar4 == FIGHTER_EXECUTE_FLAGS_HOLD) {
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
					if (uVar4 == FIGHTER_EXECUTE_FLAGS_RIDE) {
						this->logicalPosition = this->field_0x354->currentLocation;
					}
					else {
						if (uVar4 == FIGHTER_EXECUTE_FLAGS_FLIP) {
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
	this->fighterAnatomyZones.field_0x10 = pPrimObj->position;
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
	this->fighterAnatomyZones.field_0x10 = peVar1->position;

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
			this->fightFlags = this->fightFlags & ~FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK;
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
				SetState(FIGHTER_JUMP_FALL_TO_FLIP, -1);
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
	CCollision* pCol;
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

	pCol = this->pCollisionData;
	local_10 = gF32Vector4Zero;

	this->field_0x690 = local_10;

	bVar2 = (pCol->flags_0x4 & COLLISION_GROUND_FLAG) != 0;
	if (bVar2) {
		edF32Vector4AddHard(&local_10, &local_10, &pCol->aCollisionContact[1].location);
		edF32Vector4AddHard(&this->field_0x690, &this->field_0x690, &pCol->aCollisionContact[1].field_0x10);
	}

	uVar6 = (uint)bVar2;
	if ((pCol->flags_0x4 & COLLISION_WALL_FLAG) != 0) {
		edF32Vector4AddHard(&local_10, &local_10, &pCol->aCollisionContact[0].location);
		edF32Vector4AddHard(&this->field_0x690, &this->field_0x690, &pCol->aCollisionContact[0].field_0x10);
		uVar6 = uVar6 + 1;
	}

	if ((pCol->flags_0x4 & COLLISION_CEILING_FLAG) != 0) {
		edF32Vector4AddHard(&local_10, &local_10, &pCol->aCollisionContact[2].location);
		edF32Vector4AddHard(&this->field_0x690, &this->field_0x690, &pCol->aCollisionContact[2].field_0x10);
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

	static float FLOAT_00448b08 = 15.707964f;

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
					unaff_f20 = ((this->pCollisionData)->pObbPrim->scale).z;
				}
			}
			else {
				unaff_f20 = ((this->pCollisionData)->pObbPrim->scale).y;
			}
		}
		else {
			unaff_f20 = ((this->pCollisionData)->pObbPrim->scale).x;
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

void CActorFighter::_StateFighterJumpFallToFlip()
{
	float fVar1;
	bool bVar2;
	int iVar3;
	float fVar4;
	edF32VECTOR4 eStack32;

	_ManageFighterDyn(0x19, 0x1002023b, (CActorsTable*)0x0);

	if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
		this->field_0x4fc = (this->dynamic.linearAcceleration * fabsf(this->dynamic.velocityDirectionEuler.y)) / 8.0f;

		if (1.0f < this->field_0x4fc) {
			this->field_0x4fc = 1.0f;
		}

		if (this->currentLocation.y < this->field_0x4f0) {
			this->fightFlags = this->fightFlags | FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK;
		}

		if (this->pAdversary != (CActorFighter*)0x0) {
			edF32Vector4SubHard(&eStack32, &this->currentLocation, &this->pAdversary->currentLocation);

			fVar4 = ((this->pAdversary->pCollisionData)->pObbPrim->scale).y;
			bVar2 = false;
			if ((fVar4 * 0.75f <= eStack32.y) && (bVar2 = false, eStack32.y <= fVar4 * 1.25f)) {
				bVar2 = true;
			}

			if ((bVar2) &&
				(fVar4 = this->pAdversary->currentLocation.x - this->currentLocation.x,
					fVar1 = this->pAdversary->currentLocation.z - this->currentLocation.z,
					sqrtf(fVar4 * fVar4 + 0.0f + fVar1 * fVar1) <
					((this->pCollisionData)->pObbPrim->scale).z + ((this->pAdversary->pCollisionData)->pObbPrim->scale).z)) {
				eStack32.y = 0.0f;

				fVar4 = edF32Vector4NormalizeHard(&eStack32, &eStack32);
				if (fVar4 == 0.0f) {
					edF32Vector4GetNegHard(&eStack32, &this->rotationQuat);
				}

				fVar4 = edF32Vector4DotProductHard(&eStack32, &this->rotationQuat);
				if (fVar4 <= 0.707f) {
					iVar3 = DoMessage(this->pAdversary, MESSAGE_JUMP_ON, 0);
					if (iVar3 != 0) {
						SetState(FIGHTER_JUMP_FLIP_IMPULSE, -1);
						SetAdversary(this->pAdversary);
						this->pAdversary->SetState(FIGHTER_FLIP_ON_ME, -1);
					}
				}
			}
		}
	}
	else {
		SetState(0xc, -1);
	}

	return;
}

void CActorFighter::_StateFighterFlipImpulse()
{
	float newSpeed;
	edF32VECTOR4 bonePos;

	this->pAdversary->SV_GetBoneWorldPosition(0x4146416f, &bonePos);

	edF32Vector4SubHard(&bonePos, &bonePos, &this->currentLocation);
	edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &bonePos, &bonePos);

	newSpeed = edF32Vector4SafeNormalize0Hard(&bonePos, &bonePos);
	if (8.0f < newSpeed) {
		this->dynamic.speed = 8.0f;
	}
	else {
		this->dynamic.speed = newSpeed;
	}

	this->dynamic.rotationQuat = bonePos;

	ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

	if (((this->pAnimationController->anmBinMetaAnimator.aAnimData)->animPlayState == STATE_ANIM_PLAYING) && ((this->fightFlags & FIGHT_FLAG_FLIP_TO_JUMP) != 0)) {
		SetState(FIGHTER_JUMP_FLIP_PREPARE_JUMP, -1);
	}
	else {
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(FIGHTER_JUMP_FLIP_PREPARE_FALL, -1);
		}
	}

	return;
}

void CActorFighter::StateFighterHoldThrow()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	float fVar5;
	float puVar6;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	if (0.0f <= this->field_0xa5c) {
		puVar6 = 1.0f;
	}
	else {
		puVar6 = -1.0f;
	}
	fVar5 = this->scalarDynForward.Integrate(GetTimer()->cutsceneDeltaTime);

	edF32Matrix4RotateYHard(puVar6 * fVar5, &eStack64, &gF32Matrix4Unit);
	edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, &this->rotationQuat);
	this->rotationQuat = local_50;

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(6, this->standAnim);
	}

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
	this->field_0x44c = this->field_0x44c & 0xf0 | local_4.moveByte & 0xf;
	this->field_0x36c = this->field_0x36c & 0xfffffffe;
	edF32Vector4ScaleHard(this->field_0x6b0 * 0.040625f, &local_30, &eStack32);
	edF32Vector4ScaleHard(this->field_0x6b0, &local_40, &eStack32);
	this->scalarDynForward.BuildFromSpeedDistTime(fabs(local_40.z), 0.0f, fabs(local_30.z), time);
	this->scalarDynLateral.BuildFromSpeedDistTime(fabs(local_40.x), 0.0f, fabs(local_30.x), time);
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

	if (this->scalarDynForward.IsFinished() != false) {
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

void CActorFighter::_StateFighterHitPushedInit()
{
	float fVar11;
	edF32VECTOR4 eStack80;
	s_fighter_action local_8;

	this->rotationQuat = this->field_0x6a0;
	StateConfig* pSVar9 = this->GetStateCfg(this->actorState);
	int iVar7 = this->GetIdMacroAnim(pSVar9->animId);
	if (iVar7 < 0) {
		fVar11 = 0.0f;
	}
	else {
		fVar11 = this->pAnimationController->GetAnimLength(iVar7, 1);
	}

	this->_BuildCommandFromAbsoluteVector(&this->rotationQuat, &local_8, &eStack80);
	this->field_0x44c = this->field_0x44c & 0xf0 | local_8.moveByte & 0xf;
	this->field_0x36c = this->field_0x36c & 0xfffffffe;
	edF32Vector4ScaleHard(this->field_0x6b0, &eStack80, &eStack80);
	this->scalarDynForward.BuildFromSpeedDistTime(14.5f, 0.0f, fabsf(eStack80.z), fVar11);
	this->scalarDynLateral.Reset();
	this->scalarDynJump.Reset();

	return;
}

void CActorFighter::_StateFighterHitPushed()
{
	bool bVar1;
	int iVar2;
	int iVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_130;
	edF32VECTOR4 local_120;
	CActorsTable local_110;
	CCollision* pCol;

	local_110.nbEntries = 0;
	iVar3 = 0;
	local_120 = gF32Vector4Zero;
	fVar4 = this->timeInAir;
	fVar5 = (this->scalarDynForward).duration;

	_ManageFighterDyn(0x19, 0x1002023b, &local_110);

	if (local_110.nbEntries != 0) {
		local_130.x = this->dynamic.rotationQuat.x;
		local_130.y = 0.3f;
		local_130.z = this->dynamic.rotationQuat.z;
		local_130.w = 0.0f;

		edF32Vector4NormalizeHard(&local_130, &local_130);
		if (fVar4 / fVar5 < 0.55f) {
			edF32Vector4ScaleHard(9.0f, &local_130, &local_130);
		}
		else {
			edF32Vector4ScaleHard(0.1f, &local_130, &local_130);
		}
		iVar2 = _SV_HIT_ProcessActorsCollisions(0.0f, (edF32VECTOR4*)0x0, (edF32VECTOR4*)0x0, &local_130, &local_110, (CFighterExcludedTable*)0x0, true, 6, 0);
		if (iVar2 != 0) {
			edF32Vector4SubHard(&local_120, &local_120, (edF32VECTOR4*)&this->dynamic);
			iVar3 = 1;
		}
	}

	pCol = this->pCollisionData;
	if ((pCol->flags_0x4 & 1) != 0) {
		edF32Vector4AddHard(&local_120, &local_120, &pCol->aCollisionContact->location);
		if (iVar3 != 0) {
			edF32Vector4NormalizeHard(&local_120, &local_120);
		}

		iVar3 = iVar3 + 1;
	}

	if (iVar3 == 0) {
		bVar1 = this->scalarDynForward.IsFinished();
		if ((bVar1 != false) && (bVar1 = this->scalarDynLateral.IsFinished(), bVar1 != false)) {
			this->field_0x36c = this->field_0x36c | 1;
			SetState(0x51, -1);
		}
	}
	else {
		local_120.y = 0.5f;
		edF32Vector4NormalizeHard(&this->field_0x6a0, &local_120);
		edF32Matrix4MulF32Vector4Hard(&this->field_0x690, (edF32MATRIX4*)this->pMeshTransform, &(this->fighterAnatomyZones).field_0x10);
		this->field_0x6b0 = 3.0f;
		SetBehaviour(FIGHTER_BEHAVIOUR_PROJECTED, 0x56, -1);
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
		if ((this->fightFlags & FIGHT_FLAG_ATTACK_CONNECTED) != 0) {
			_Std_OnFightActionSuccess();
		}

		pcVar3 = this->pBlow->field_0xd0;
		if (pcVar3 != (void*)0x0) {
			IMPLEMENTATION_GUARD(
			(*pcVar3)(this, 1);)
		}

		if (this->nextActionType == NEXT_ACTION_TYPE_GRAB) {
			this->nextActionType = NEXT_ACTION_TYPE_NONE;
			this->pExecutingGrab = this->pNextAction.pGrab;
			this->field_0x474 = this->pExecutingGrab->field_0xc;
			SetState(0x6b, this->pExecutingGrab->field_0x80.animId);
		}
		else {
			if (this->nextActionType == NEXT_ACTION_TYPE_BLOW) {
				this->nextActionType = NEXT_ACTION_TYPE_NONE;
				this->pBlow = this->pNextAction.pBlow;
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

void CActorFighter::_StateFighterFlipPrepareJump()
{
	CAnimation* pCVar1;
	bool bVar3;
	StateConfig* pSVar4;
	int iVar5;
	float fVar7;
	float fVar8;
	edF32VECTOR4 local_60;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 local_10;

	pSVar4 = GetStateCfg(this->actorState);
	pCVar1 = this->pAnimationController;
	iVar5 = GetIdMacroAnim(pSVar4->animId);
	if (iVar5 < 0) {
		fVar7 = 0.0f;
	}
	else {
		fVar7 = pCVar1->GetAnimLength(iVar5, 1);
	}

	fVar8 = this->timeInAir;
	if ((fVar8 <= fVar7) || (fabsf(fVar8 - fVar7) < GetTimer()->cutsceneDeltaTime / 2.0f)) {
		edF32Matrix4RotateYHard((-3.141593f / fVar7) * GetTimer()->cutsceneDeltaTime, &eStack80, &gF32Matrix4Unit);
		edF32Matrix4MulF32Vector4Hard(&local_60, &eStack80, &this->rotationQuat);
		this->rotationQuat = local_60;
	}

	this->pAdversary->SV_GetBoneWorldPosition(0x4146416f, &local_10);
	edF32Vector4SubHard(&local_10, &local_10, &this->currentLocation);
	edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &local_10, &local_10);
	fVar7 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
	this->dynamic.speed = fVar7;
	this->dynamic.rotationQuat = local_10;

	ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(FIGHTER_JUMP_FLIP_JUMP, -1);
		this->pAdversary->SetState(FIGHTER_FLIP_OFF_ME_A, -1);
	}

	return;
}

void CActorFighter::_StateFighterFlipJumpInit()
{
	StateConfig* pSVar1;
	int iVar2;
	float time;
	CAnimation* pAnim;

	pSVar1 = GetStateCfg(FIGHTER_JUMP_FLIP_JUMP);
	pAnim = this->pAnimationController;
	iVar2 = GetIdMacroAnim(pSVar1->animId);
	if (iVar2 < 0) {
		time = 0.0f;
	}
	else {
		time = pAnim->GetAnimLength(iVar2, 1);
	}

	this->scalarDynJump.BuildFromSpeedDistTime(14.0f, 0.0f, 1.2f, time * 0.6666667f * 0.8f);
	this->scalarDynForward.BuildFromSpeedDistTime(0.0f, 0.0f, 1.333333f, time);
	this->scalarDynLateral.Reset();
	this->field_0x3f4 = 1.0f;
	this->field_0x36c = this->field_0x36c & 0xfffffffe;

	return;
}



void CActorFighter::_StateFighterFlipJump()
{
	this->field_0x44c = this->field_0x44c & 0xf0 | 6;

	_ManageFighterDyn(0x3b, 0x129, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		this->field_0x4fc = (this->dynamic.linearAcceleration * fabsf(this->dynamic.velocityDirectionEuler.y)) / 10.0f;
		SetState(0xd, -1);
	}

	return;
}

void CActorFighter::_StateFighterFlipPrepareFallInit()
{
	StateConfig* pSVar1;
	int iVar2;
	CActorFighter* pCVar3;
	float fVar5;
	float fVar6;
	float fVar7;
	CAnimation* pAnim;

	pSVar1 = GetStateCfg(this->actorState);
	pAnim = this->pAnimationController;
	iVar2 = GetIdMacroAnim(pSVar1->animId);
	if (iVar2 < 0) {
		fVar5 = 0.0f;
		pCVar3 = this->pAdversary;
	}
	else {
		fVar5 = pAnim->GetAnimLength(iVar2, 1);
		pCVar3 = this->pAdversary;
	}

	fVar6 = GetAngleYFromVector(&pCVar3->rotationQuat);
	fVar7 = GetAngleYFromVector(&this->rotationQuat);
	fVar6 = edF32Between_2Pi(fVar6 - fVar7);
	this->field_0x7b0 = fVar6;
	this->field_0x7b0 = (this->field_0x7b0 * GetTimer()->cutsceneDeltaTime) / fVar5;

	return;
}

void CActorFighter::_StateFighterFlipPrepareFall()
{
	CAnimation* pCVar1;
	bool bVar3;
	float fVar5;
	float fVar6;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	fVar5 = GetAngleYFromVector(&this->pAdversary->rotationQuat);
	fVar6 = GetAngleYFromVector(&this->rotationQuat);
	fVar5 = edF32Between_2Pi(fVar5 - fVar6);
	if (fabsf(fVar5) < fabsf(this->field_0x7b0)) {
		edF32Matrix4RotateYHard(fVar5, &eStack64, &gF32Matrix4Unit);
	}
	else {
		edF32Matrix4RotateYHard(this->field_0x7b0, &eStack64, &gF32Matrix4Unit);
	}

	edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, &this->rotationQuat);
	this->rotationQuat = local_50;
	this->pAdversary->SV_GetBoneWorldPosition(0x4146416f, &local_60);
	edF32Vector4SubHard(&local_60, &local_60, &this->currentLocation);
	edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &local_60, &local_60);
	fVar5 = edF32Vector4SafeNormalize0Hard(&local_60, &local_60);
	this->dynamic.speed = fVar5;
	this->dynamic.rotationQuat = local_60;

	ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(0x11, -1);
	}

	return;
}

void CActorFighter::_StateFighterFlipFallInit()
{
	edColPRIM_OBJECT* peVar1;
	CActorFighter* pCVar2;
	StateConfig* pSVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_10;
	CAnimation* pAnim;

	peVar1 = (this->pAdversary->pCollisionData)->pObbPrim;
	fVar4 = (peVar1->scale).x;
	fVar5 = (peVar1->scale).y;
	local_10.x = (peVar1->scale).z;
	if (fVar5 <= fVar4) {
		fVar5 = fVar4;
	}
	if (local_10.x <= fVar5) {
		local_10.x = fVar5;
	}

	local_10.w = 0.0f;
	pAnim = this->pAnimationController;
	peVar1 = (this->pAdversary->pCollisionData)->pObbPrim;
	local_10.y = local_10.x;
	local_10.z = local_10.x;

	pSVar3 = GetStateCfg(this->actorState);
	if (pSVar3->animId < 0) {
		fVar5 = 0.0f;
	}
	else {
		fVar5 = pAnim->GetAnimLength(pSVar3->animId, 1);
	}

	this->pAdversary->ChangeCollisionSphere(fVar5, &local_10, &peVar1->position);

	return;
}

void CActorFighter::_StateFighterFlipFall()
{
	bool bVar3;
	float fVar5;
	edF32VECTOR4 local_10;

	this->pAdversary->SV_GetBoneWorldPosition(0x4146416f, &local_10);
	edF32Vector4SubHard(&local_10, &local_10, &this->currentLocation);
	edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &local_10, &local_10);
	fVar5 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
	this->dynamic.speed = fVar5;
	this->dynamic.rotationQuat = local_10;

	ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(0x20, -1);
	}

	return;
}

void CActorFighter::_StateFighterRiddenEjectInit()
{
	_msg_hit_param msgHitParam;
	CCollision* pCol;

	this->dynamic.speed = 0.0f;
	pCol = this->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 & 0xffffefff;
	msgHitParam.projectileType = 7;
	msgHitParam.flags = 5;
	msgHitParam.field_0x50 = 1;
	msgHitParam.field_0x60.x = this->rotationQuat.z;
	msgHitParam.field_0x60.y = 0.0f;
	msgHitParam.field_0x60.z = -this->rotationQuat.x;
	msgHitParam.field_0x60.w = 0.0f;
	msgHitParam.field_0x20.x = this->rotationQuat.x;
	msgHitParam.field_0x20.y = 0.3f;
	msgHitParam.field_0x20.z = this->rotationQuat.z;
	msgHitParam.field_0x20.w = 0.0f;
	edF32Vector4NormalizeHard(&msgHitParam.field_0x20, &msgHitParam.field_0x20);
	msgHitParam.field_0x10 = 0.0f;
	msgHitParam.field_0x30 = 12.0f;
	msgHitParam.damage = 0.0f;
	msgHitParam.field_0x70 = 2.5f;
	DoMessage(this->field_0x354, MESSAGE_KICKED, &msgHitParam);

	return;
}

void CActorFighter::_StateFighterFlipOnMeInit()
{
	this->pAnimationController->RegisterBone(0x4146416f);
	(this->pCollisionData)->actorFieldA = this->pAdversary;

	return;
}


void CActorFighter::_StateFighterFlipOnMe()
{
	SetLookingAtOff();

	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(FIGHTER_FLIP_OFF_ME_B, -1);
	}

	return;
}

void CActorFighter::_StateFighterFlipOffMe()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;

	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(6, this->standAnim);
	}

	return;
}

void CActorFighter::_StateFighterRide()
{
	float fVar1;

	if (((this->fightFlags & FIGHT_FLAG_ACTION_LOCKED) != 0) && (this->field_0x474 < this->timeInAir)) {
		this->fightFlags &= ~FIGHT_FLAG_ACTION_LOCKED;
		this->field_0x474 = 0.0f;
	}

	this->adversaryAngleDiff = GetAngleYFromVector(&this->field_0x354->rotationQuat);

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	return;
}


void CActorFighter::_StateFighterRideBlow()
{
	bool bVar3;

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		this->fightFlags = this->fightFlags | 1;
		this->field_0x474 = 0.05f;
	}

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	return;
}

void CActorFighter::_StateFighterFlipOffMeTerm(int newState)
{
	if ((newState != 0x15) && (newState != FIGHTER_FLIP_OFF_ME_B)) {
		SetLookingAtOn();

		this->field_0x474 = 0.5f;
		this->pAnimationController->UnRegisterBone(0x4146416f);
		(this->pCollisionData)->actorFieldA = (CActor*)0x0;
	}

	return;
}

void CActorFighter::_StateFighterHoldStandInit()
{
	this->scalarDynForward.Reset();
	this->scalarDynLateral.Reset();
	this->scalarDynJump.Reset();

	if (this->field_0x474 == 0.0f) {
		this->fightFlags = this->fightFlags & 0xfffffffe;
	}
	else {
		this->fightFlags = this->fightFlags | 1;
	}

	this->dynamic.speed = 0.0f;
	SetAdversary((CActorFighter*)0x0);

	return;
}

void CActorFighter::_StateFighterHoldStand()
{
	edF32VECTOR4* v2;
	CBehaviourFighterSlave* pBehaviourFighterSlave;
	float fVar2;
	float fVar3;
	float puVar4;
	float puVar5;
	float fVar6;
	edF32VECTOR4 local_20;
	s_fighter_action_param local_10;
	s_fighter_action local_4;
	CActorFighter* pFighter;

	if (this->field_0x474 <= this->timeInAir) {
		this->fightFlags = this->fightFlags & 0xfffffffe;
		this->field_0x474 = 0.0f;
		this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
	}

	if ((this->fightFlags & 0x400) == 0) {
		edF32Vector4ScaleHard(this->field_0x50c, &local_20, &this->rotationQuat);
		edF32Vector4AddHard(&local_20, &local_20, &this->currentLocation);
		v2 = GetAdversaryHeldPos();
		edF32Vector4SubHard(&local_20, &local_20, v2);
		edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &local_20, &local_20);
	}
	else {
		local_20 = gF32Vector4Zero;
	}

	local_4.actionByte = local_4.actionByte & 0xf0;
	local_10.field_0x0 = &local_20;
	local_4.moveByte = 0;
	pBehaviourFighterSlave = static_cast<CBehaviourFighterSlave*>(this->field_0x354->GetBehaviour(this->field_0x354->curBehaviourId));
	pBehaviourFighterSlave->ManageByMaster(&local_4, &local_10);
	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	fVar2 = this->field_0x354->currentLocation.y - this->currentLocation.y;
	this->field_0x354->field_0xa50 = fVar2;
	this->field_0xa50 = fVar2;

	fVar2 = (this->field_0x4e0).x;
	if (fabsf(fVar2) <= 0.1f) {
		(this->field_0x4e0).x = 0.0f;
	}
	else {
		if (0.0f <= fVar2) {
			puVar4 = 1.0f;
		}
		else {
			puVar4 = -1.0f;
		}
		(this->field_0x4e0).x = (this->field_0x4e0).x - puVar4 * 0.1f;
	}

	fVar2 = (this->field_0x4e0).z;
	if (fabsf(fVar2) <= 0.1f) {
		(this->field_0x4e0).z = 0.0f;
	}
	else {
		if (0.0f <= fVar2) {
			puVar5 = 1.0f;
		}
		else {
			puVar5 = -1.0f;
		}
		(this->field_0x4e0).z = (this->field_0x4e0).z - static_cast<float>(puVar5) * 0.1f;
	}

	(this->field_0x4e0).y = 1.0f - (fabsf((this->field_0x4e0).x) + fabsf((this->field_0x4e0).z));
	fVar2 = (this->field_0x4e0).y;
	if (1.0f < fVar2) {
		(this->field_0x4e0).y = 1.0f;
	}
	else {
		if (fVar2 < 0.0f) {
			(this->field_0x4e0).y = 0.0f;
		}
	}

	pFighter = this->field_0x354;
	pFighter->field_0x4e0 = this->field_0x4e0;

	pBehaviourFighterSlave->ManageExitByMaster();

	return;
}

void CActorFighter::_StateFighterHoldRunInit()
{
	this->scalarDynJump.Reset();
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

	return;
}

void CActorFighter::_StateFighterHoldRun()
{
	int iVar1;
	StateConfig* pStateConfig;
	CBehaviourFighterSlave* pBehaviourFighterSlave;
	float fVar4;
	float fVar5;
	float fVar6;
	float puVar10;
	float puVar9;
	float puVar8;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32MATRIX4 eStack80;
	s_fighter_action_param actionParam;
	s_fighter_action action;
	CActorFighter* pFighter;

	pFighter = this->field_0x354;
	local_60 = pFighter->currentLocation;
	edF32Matrix4RotateYHard(this->field_0xa5c * GetTimer()->cutsceneDeltaTime, &eStack80, &gF32Matrix4Unit);
	edF32Matrix4MulF32Vector4Hard(&local_70, &eStack80, &this->rotationQuat);
	edF32Vector4SubHard(&eStack128, &local_70, &this->rotationQuat);

	pFighter = this->field_0x354;
	fVar4 = pFighter->currentLocation.x - this->currentLocation.x;
	fVar5 = pFighter->currentLocation.y - this->currentLocation.y;
	fVar6 = pFighter->currentLocation.z - this->currentLocation.z;
	edF32Vector4ScaleHard(sqrtf(fVar4 * fVar4 + fVar5 * fVar5 + fVar6 * fVar6) / GetTimer()->cutsceneDeltaTime, &eStack128, &eStack128);
	this->scalarDynForward.Integrate(GetTimer()->cutsceneDeltaTime);

	edF32Vector4ScaleHard((this->scalarDynForward).field_0x20, &local_70, &local_70);
	edF32Vector4AddHard(&local_70, &local_70, &eStack128);
	iVar1 = this->currentAnimType;
	pStateConfig = GetStateCfg(this->actorState);
	if (iVar1 == pStateConfig->animId) {
		action.moveByte = 3;
	}
	else {
		if (0.0f <= this->field_0xa5c) {
			action.moveByte = 2;
		}
		else {
			action.moveByte = 1;
		}
	}

	actionParam.field_0x0 = &local_70;
	action.moveByte = action.moveByte | 0x10;
	action.actionByte = action.actionByte & 0xf0;

	pBehaviourFighterSlave = static_cast<CBehaviourFighterSlave*>(this->field_0x354->GetBehaviour(this->field_0x354->curBehaviourId));
	pBehaviourFighterSlave->ManageByMaster(&action, &actionParam);
	edF32Vector4SubHard(&local_a0, &this->field_0x354->currentLocation, &local_60);
	edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &local_a0, &local_a0);
	edF32Vector4SubHard(&eStack144, &local_a0, &local_70);
	eStack144.y = 0.0f;
	edF32Vector4SubHard(&local_a0, &this->field_0x354->currentLocation, &this->currentLocation);
	fVar5 = edF32Vector4NormalizeHard(&local_a0, &local_a0);
	fVar4 = this->field_0x50c;
	local_b0.x = local_a0.x;
	local_b0.z = local_a0.z;
	local_b0.w = local_a0.w;
	local_b0.y = 0.0f;

	fVar6 = edF32Vector4NormalizeHard(&local_b0, &local_b0);
	if (fVar6 != 0.0f) {
		this->rotationQuat = local_b0;
	}

	local_c0.y = 0.0f;
	local_c0.w = 0.0f;
	local_c0.z = -local_a0.x;
	local_c0.x = local_a0.z;
	fVar6 = edF32Vector4DotProductHard(&eStack144, &local_c0);
	edF32Vector4ScaleHard(fVar6, &eStack144, &local_c0);
	edF32Vector4ScaleHard((fVar5 - fVar4) / GetTimer()->cutsceneDeltaTime, &local_70, &local_a0);
	edF32Vector4AddHard(&local_70, &local_70, &eStack144);
	fVar4 = edF32Vector4SafeNormalize0Hard(&local_70, &local_70);
	this->dynamic.speed = fVar4;
	this->dynamic.rotationQuat = local_70;
	(this->pCollisionData)->actorFieldA = this->field_0x354;

	ManageDyn(4.0f, 0x1006023b, (CActorsTable*)0x0);

	(this->pCollisionData)->actorFieldA = (CActor*)0x0;
	fVar4 = this->field_0x354->currentLocation.y - this->currentLocation.y;
	this->field_0x354->field_0xa50 = fVar4;
	this->field_0xa50 = fVar4;
	(this->field_0x4d0).x = 0.0f;
	(this->field_0x4d0).z = 0.0f;
	if (0.1f < this->dynamic.speed) {
		(this->field_0x4d0).z = 1.0f;
	}
	else {
		if (0.1f < fabsf(this->field_0xa5c)) {
			if (0.0f <= this->field_0xa5c) {
				puVar10 = 1.0f;
			}
			else {
				puVar10 = -1.0f;
			}
			(this->field_0x4d0).x = puVar10;
		}
	}

	fVar4 = (this->field_0x4d0).x - (this->field_0x4e0).x;
	fVar5 = (this->field_0x4d0).z - (this->field_0x4e0).z;
	if (fabsf(fVar4) <= 0.1f) {
		(this->field_0x4e0).x = (this->field_0x4d0).x;
	}
	else {
		if (0.0f <= fVar4) {
			puVar9 = 1.0f;
		}
		else {
			puVar9 = -1.0f;
		}
		(this->field_0x4e0).x = (this->field_0x4e0).x + puVar9 * 0.1f;
	}

	if (fabsf(fVar5) <= 0.1f) {
		(this->field_0x4e0).z = (this->field_0x4d0).z;
	}
	else {
		if (0.0f <= fVar5) {
			puVar8 = 1.0f;
		}
		else {
			puVar8 = -1.0f;
		}
		(this->field_0x4e0).z = (this->field_0x4e0).z + puVar8 * 0.1f;
	}

	(this->field_0x4e0).y = 1.0f - (fabsf((this->field_0x4e0).x) + fabsf((this->field_0x4e0).z));
	fVar4 = (this->field_0x4e0).y;
	if (1.0f < fVar4) {
		(this->field_0x4e0).y = 1.0f;
	}
	else {
		if (fVar4 < 0.0f) {
			(this->field_0x4e0).y = 0.0f;
		}
	}

	pFighter = this->field_0x354;
	pFighter->field_0x4e0 = this->field_0x4e0;

	pBehaviourFighterSlave->ManageExitByMaster();

	return;
}

void CActorFighter::_StateFighterHoldRotateInit()
{
	this->scalarDynForward.Reset();
	this->scalarDynJump.Reset();
	this->dynamic.speed = 0.0f;
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

	return;
}

void CActorFighter::_StateFighterHoldRotate()
{
	CBehaviourFighterSlave* pBehaviourFighterSlave;
	float fVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 local_100;
	edF32MATRIX4 eStack240;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 eStack128;
	edF32MATRIX4 eStack112;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	s_fighter_action_param actionParam;
	s_fighter_action action;
	CActorFighter* pFighter;

	pFighter = this->field_0x354;
	local_30 = pFighter->currentLocation;

	if ((this->fightFlags & 0x400) == 0) {
		edF32Matrix4RotateYHard(this->field_0xa5c * GetTimer()->cutsceneDeltaTime, &eStack112, &gF32Matrix4Unit);
		edF32Matrix4MulF32Vector4Hard(&local_20, &eStack112, &this->rotationQuat);
		edF32Vector4SubHard(&local_20, &local_20, &this->rotationQuat);
		fVar2 = this->field_0x354->currentLocation.x - this->currentLocation.x;
		fVar3 = this->field_0x354->currentLocation.z - this->currentLocation.z;
		edF32Vector4ScaleHard(sqrtf(fVar2 * fVar2 + 0.0f + fVar3 * fVar3) / GetTimer()->cutsceneDeltaTime, &local_20, &local_20);
	}
	else {
		local_20.y = 0.0f;
		local_20.x = fabsf(this->field_0xa5c);
		local_20.z = 0.0f;
		local_20.w = 0.0f;
	}

	if (this->field_0xa5c == 0.0f) {
		action.moveByte = 0;
	}
	else {
		if (0.0f < this->field_0xa5c) {
			action.moveByte = 0x12;
		}
		else {
			action.moveByte = 0x11;
		}
	}

	action.actionByte = action.actionByte & 0xf0;
	actionParam.field_0x0 = &local_20;
	pBehaviourFighterSlave = static_cast<CBehaviourFighterSlave*>(this->field_0x354->GetBehaviour(this->field_0x354->curBehaviourId));
	pBehaviourFighterSlave->ManageByMaster(&action, &actionParam);
	if ((this->fightFlags & 0x400) == 0) {
		edF32Vector4SubHard(&local_90, &this->field_0x354->currentLocation, &local_30);
		edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &local_90, &local_90);
		edF32Vector4SubHard(&eStack128, &local_90, &local_20);
		eStack128.y = 0.0f;
		edF32Vector4SubHard(&local_90, &this->field_0x354->currentLocation, &this->currentLocation);
		fVar3 = edF32Vector4NormalizeHard(&local_90, &local_90);
		fVar2 = this->field_0x50c;

		local_a0.x = local_90.x;
		local_a0.z = local_90.z;
		local_a0.w = local_90.w;
		local_a0.y = 0.0f;

		fVar4 = edF32Vector4NormalizeHard(&local_a0, &local_a0);
		if (fVar4 != 0.0f) {
			this->rotationQuat = local_a0;
		}
		local_b0.y = 0.0f;
		local_b0.w = 0.0f;
		local_b0.z = -local_90.x;
		local_b0.x = local_90.z;

		fVar4 = edF32Vector4DotProductHard(&eStack128, &local_b0);
		edF32Vector4ScaleHard(fVar4, &eStack128, &local_b0);
		edF32Vector4ScaleHard((fVar3 - fVar2) / GetTimer()->cutsceneDeltaTime, &local_20, &local_90);
		edF32Vector4AddHard(&local_20, &local_20, &eStack128);
		fVar2 = edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
		this->dynamic.speed = fVar2;
		this->dynamic.rotationQuat = local_20;
	}
	else {
		edF32Matrix4RotateYHard(this->field_0xa5c * GetTimer()->cutsceneDeltaTime, &eStack240, &gF32Matrix4Unit);
		edF32Matrix4MulF32Vector4Hard(&local_100, &eStack240, &this->rotationQuat);
		this->rotationQuat = local_100;
		this->dynamic.speed = 0.0f;
	}

	(this->pCollisionData)->actorFieldA = this->field_0x354;

	ManageDyn(4.0f, 0x1006023b, (CActorsTable*)0x0);
	(this->pCollisionData)->actorFieldA = (CActor*)0x0;
	fVar2 = this->field_0x354->currentLocation.y - this->currentLocation.y;
	this->field_0x354->field_0xa50 = fVar2;
	this->field_0xa50 = fVar2;
	pBehaviourFighterSlave->ManageExitByMaster();

	return;
}

void CActorFighter::_StateFighterHoldPushPrepareInit()
{
	this->scalarDynForward.Reset();
	this->scalarDynLateral.Reset();
	this->scalarDynJump.Reset();
	this->dynamic.speed = 0.0f;
	this->field_0x354->SetState(0x47, -1);

	return;
}

void CActorFighter::_StateFighterHoldPushPrepareTerm()
{
	_msg_hit_param msgHitParam;

	msgHitParam.projectileType = 8;
	msgHitParam.field_0x20 = this->rotationQuat;
	msgHitParam.field_0x30 = this->field_0x514;
	msgHitParam.flags = 0;
	DoMessage(this->field_0x354, MESSAGE_KICKED, &msgHitParam);
	this->pAdversary = this->field_0x354;
	this->field_0x354 = (CActorFighter*)0x0;

	return;
}

void CActorFighter::_StateFighterHoldThrowInit()
{
	StateConfig* pSVar1;
	int iVar2;
	float time;

	pSVar1 = GetStateCfg(0x36);
	iVar2 = GetIdMacroAnim(pSVar1->animId);
	if (iVar2 < 0) {
		time = 0.0f;
	}
	else {
		time = this->pAnimationController->GetAnimLength(iVar2, 1);
	}

	this->scalarDynForward.BuildFromSpeedDistTime(8.7f, 0.87f, 1.570796f, time);
	this->scalarDynLateral.Reset();
	this->scalarDynJump.Reset();
	this->dynamic.speed = 0.0f;
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

	return;
}

void CActorFighter::_StateFighterHoldThrowTerm()
{
	(this->scalarDynForward).field_0x24 = 0.0f;
	(this->scalarDynForward).field_0x20 = 0.0f;
	(this->scalarDynForward).field_0x1c = 0.0f;
	(this->scalarDynForward).field_0x18 = 0.0f;
	(this->scalarDynForward).field_0x14 = 0.0f;
	(this->scalarDynForward).field_0x10 = 0.0f;
	(this->scalarDynForward).field_0xc = 0.0f;
	(this->scalarDynForward).duration = 0.0f;
	(this->scalarDynForward).field_0x4 = 0.0f;
	(this->scalarDynForward).flags = 1;

	return;
}

void CActorFighter::_StateFighterHoldFollowStd(int nextState)
{
	CBehaviourFighterSlave* pBehaviourFighterSlave;
	s_fighter_action fighterAction;

	fighterAction.all = 0x0;
	pBehaviourFighterSlave = static_cast<CBehaviourFighterSlave*>(this->field_0x354->GetBehaviour(this->field_0x354->curBehaviourId));
	pBehaviourFighterSlave->ManageByMaster(&fighterAction, (s_fighter_action_param*)0x0);
	(this->pCollisionData)->actorFieldA = this->field_0x354;

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	(this->pCollisionData)->actorFieldA = (CActor*)0x0;

	pBehaviourFighterSlave->ManageExitByMaster();
	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(nextState, -1);
	}

	return;
}

void CActorFighter::StateFighterHoldStd(int nextState, int nextAnim)
{
	_ManageFighterDyn(0, 0x1002023b, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(nextState, nextAnim);
	}

	return;
}

void CActorFighter::_StateFighterPrepareFightAction(int nextState)
{
	_ManageFighterDyn((this->pBlow->blowStageBegin).flags, 0x40121, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(nextState, (this->pBlow->blowStageExecute).animId);
	}

	return;
}

void CActorFighter::_StateFighterReturnFromFightAction()
{
	CBehaviourFighter* pCVar4;

	_ManageFighterDyn((this->pBlow->blowStageEnd).flags, 0x40323, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		pCVar4 = static_cast<CBehaviourFighter*>(GetBehaviour(this->curBehaviourId));

		this->scalarDynForward.Reset();
		this->scalarDynLateral.Reset();
		this->scalarDynJump.Reset();

		pCVar4->SetInitialState();
	}

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
	this->fightFlags = this->fightFlags & ~FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK;

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
	this->scalarDynForward.BuildFromDistTime(dist, time);
	this->field_0x44c = this->field_0x44c & 0xf0 | 3;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffffcf;
	this->field_0x4f0 = this->currentLocation.y;
	this->fightFlags = this->fightFlags & ~FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK;

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
			SetState(FIGHTER_JUMP_FALL_TO_FLIP, -1);
		}
	}

	return;
}

void CActorFighter::_StateFighterExecuteGrab()
{
	CActorFighter* pNewAdversary;
	edF32VECTOR4* peVar4;
	bool bVar5;
	StateConfig* pSVar6;
	int iVar7;
	edF32VECTOR4* peVar8;
	uint uVar9;
	edF32VECTOR4* peVar10;
	float fVar11;
	edF32VECTOR4 local_110;
	edF32MATRIX4 auStack256;
	edF32VECTOR4 eStack192;
	_msg_params_0x65 _Stack176;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR3 eStack40;
	_ray_info_out local_18;
	_msg_params_0x65* local_c;
	float local_8;
	float local_4;

	_ManageFighterDyn(this->pExecutingGrab->field_0x80.flags, 0x40323, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		edF32Vector4AddHard(&local_40, &this->currentLocation, &(this->fighterAnatomyZones).field_0x10);
		local_40.w = 1.0f;
		edF32Vector4ScaleHard(((this->pCollisionData)->pObbPrim->scale).z - 0.1f, &eStack80, &this->rotationQuat);
		edF32Vector4AddHard(&local_40, &local_40, &eStack80);

		CCollisionRay CStack112 = CCollisionRay(this->pExecutingGrab->field_0x34 + 0.1f, &local_40, &this->rotationQuat);
		fVar11 = CStack112.IntersectActors((CActor*)0x0, this, 0x7fffffff, (edF32VECTOR4*)0x0, &local_18);
		if ((fVar11 != 1e+30f) && (bVar5 = (local_18.pActor_0x0)->IsKindOfObject(8), bVar5 != false)) {
			SetAdversary(static_cast<CActorFighter*>(local_18.pActor_0x0));
			pNewAdversary = this->pAdversary;

			if ((pNewAdversary->GetStateFlags(pNewAdversary->actorState) & 0x2000000) == 0) {
				FUN_00312130(&eStack192, &local_4, &local_8, &_Stack176);
				local_c = &_Stack176;
				iVar7 = DoMessage(this->pAdversary, (ACTOR_MESSAGE)0x65, local_c);
				if (iVar7 != 0) {
					this->fightFlags = this->fightFlags | 0x40;
					this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(this->pExecutingGrab->field_0xc0.field_0x4, 0);
					local_110 = {};
					this->field_0x36c = this->field_0x36c & 0xfffffffe;

					if ((this->flags & 0x1000) == 0) {
						GetAnglesFromVector(&eStack40, &this->rotationQuat);
						edF32Matrix4FromEulerSoft(&auStack256, &eStack40, "XYZ");
					}
					else {
						edF32Matrix4FromEulerSoft(&auStack256, &this->rotationEuler.xyz, "XYZ");
					}

					local_110.z = edF32Vector4DotProductHard(&eStack192, &auStack256.rowZ);
					local_110.x = edF32Vector4DotProductHard(&eStack192, &auStack256.rowX);
					_SetRelativeSpeedOnGround(local_8, &local_110);
					this->field_0x47c = local_4;
					SetState(0x6d, this->pExecutingGrab->field_0xc0.animId);

					return;
				}
			}
		}

		SetState(0x6c, this->pExecutingGrab->field_0xa0.animId);
	}

	return;
}

void CActorFighter::FUN_00312370()
{
	CActorFighter* pActor;
	uint uVar1;
	bool bVar2;
	int iVar3;
	s_fighter_grab* psVar4;
	CBehaviourFighter* pCVar5;
	float fVar6;
	_msg_hit_param local_110;
	_msg_hit_param local_90;
	CAnimation* pAnim;

	(this->pCollisionData)->actorFieldA = (CActor*)0x0;
	if (this->nextActionType == NEXT_ACTION_TYPE_GRAB) {
		iVar3 = (this->pNextAction.pGrab->field_0x80).animId;
	}
	else {
		if (this->nextActionType == NEXT_ACTION_TYPE_BLOW) {
			iVar3 = this->pNextAction.pBlow->blowStageBegin.animId;
		}
		else {
			iVar3 = (this->pExecutingGrab->field_0x100).animId;
		}
	}

	pAnim = this->pAnimationController;
	iVar3 = GetIdMacroAnim(iVar3);
	if (iVar3 < 0) {
		fVar6 = 0.0f;
	}
	else {
		fVar6 = pAnim->GetAnimLength(iVar3, 1);
	}

	psVar4 = this->pExecutingGrab;

	RestoreCollisionSphere(fVar6 / (psVar4->field_0x100).field_0x4);
	this->nextActionType = 0;
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
	this->field_0x36c = this->field_0x36c | 1;
	this->field_0x47c = 23.56194f;
	pActor = this->pAdversary;
	if (pActor->actorState == 0x71) {
		uVar1 = this->pExecutingGrab->field_0x38;
		if (uVar1 == 3) {
			bVar2 = pActor->IsKindOfObject(0x10);
			if (bVar2 == false) {
				this->pAdversary->LifeAnnihilate();
				this->pAdversary->SetBehaviour(FIGHTER_BEHAVIOUR_PROJECTED, 99, -1);
			}
		}
		else {
			if (uVar1 == 2) {
				local_110.projectileType = 7;
				local_110.flags = 0x85;
				local_110.field_0x30 = this->pExecutingGrab->field_0x60;
				local_110.field_0x70 = this->pExecutingGrab->field_0x64;
				local_110.field_0x50 = 1;
				local_110.field_0x10 = this->pExecutingGrab->field_0x10;
				local_110.damage = this->pExecutingGrab->field_0x3c * this->hitMultiplier;
				edF32Matrix4MulF32Vector4Hard(&local_110.field_0x20, (edF32MATRIX4*)this->pMeshTransform, (edF32VECTOR4*)&this->pExecutingGrab->field_0x40);
				edF32Matrix4MulF32Vector4Hard(&local_110.field_0x60, (edF32MATRIX4*)this->pMeshTransform, (edF32VECTOR4*)&this->pExecutingGrab->field_0x50);
				DoMessage(this->pAdversary, MESSAGE_KICKED, &local_110);
			}
			else {
				if (uVar1 == 1) {
					local_90.projectileType = 7;
					local_90.flags = 0x80;
					local_90.field_0x30 = this->pExecutingGrab->field_0x60;
					local_90.field_0x50 = 10;
					local_90.field_0x52 = (ushort)this->pExecutingGrab->field_0x68;
					local_90.field_0x10 = this->pExecutingGrab->field_0x10;
					local_90.damage = this->pExecutingGrab->field_0x3c * this->hitMultiplier;
					edF32Matrix4MulF32Vector4Hard(&local_90.field_0x20, (edF32MATRIX4*)this->pMeshTransform, (edF32VECTOR4*)&this->pExecutingGrab->field_0x40);
					DoMessage(this->pAdversary, MESSAGE_KICKED, &local_90);
				}
				else {
					if (uVar1 == 0) {
						pCVar5 = static_cast<CBehaviourFighter*>(pActor->GetBehaviour(pActor->curBehaviourId));
						this->pAdversary->LifeDecrease(this->pExecutingGrab->field_0x3c);
						pCVar5->SetInitialState();
					}
				}
			}
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

	pBlow->field_0xd0 = (FighterOnHitFunc)0x0;

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
	
	pGrab->field_0x20.x = pByteCode->GetF32();
	pGrab->field_0x20.y = pByteCode->GetF32();
	pGrab->field_0x20.z = pByteCode->GetF32();
	pGrab->field_0x20.w = 0.0f;
	
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

	pGrab->field_0x80.animId = pByteCode->GetS32();	
	pGrab->field_0x80.field_0x4 = pByteCode->GetF32();	
	pGrab->field_0x80.field_0x8 = pByteCode->GetF32();	
	pGrab->field_0x80.field_0x10.x = pByteCode->GetF32();
	pGrab->field_0x80.field_0x10.y = pByteCode->GetF32();
	pGrab->field_0x80.field_0x10.z = pByteCode->GetF32();
	pGrab->field_0x80.field_0x10.w = 0.0f;	
	pGrab->field_0x80.flags = pByteCode->GetU32();

	pGrab->field_0xa0.animId = pByteCode->GetS32();
	pGrab->field_0xa0.field_0x4 = pByteCode->GetF32();
	pGrab->field_0xa0.field_0x8 = pByteCode->GetF32();
	pGrab->field_0xa0.field_0x10.x = pByteCode->GetF32();
	pGrab->field_0xa0.field_0x10.y = pByteCode->GetF32();
	pGrab->field_0xa0.field_0x10.z = pByteCode->GetF32();
	pGrab->field_0xa0.field_0x10.w = 0.0f;
	pGrab->field_0xa0.flags = pByteCode->GetU32();

	pGrab->field_0xc0.animId = pByteCode->GetS32();
	pGrab->field_0xc0.field_0x4 = pByteCode->GetF32();
	pGrab->field_0xc0.field_0x8 = pByteCode->GetF32();
	pGrab->field_0xc0.field_0x10.x = pByteCode->GetF32();
	pGrab->field_0xc0.field_0x10.y = pByteCode->GetF32();
	pGrab->field_0xc0.field_0x10.z = pByteCode->GetF32();
	pGrab->field_0xc0.field_0x10.w = 0.0f;
	pGrab->field_0xc0.flags = pByteCode->GetU32();

	pGrab->field_0xe0.animId = pByteCode->GetS32();
	pGrab->field_0xe0.field_0x4 = pByteCode->GetF32();
	pGrab->field_0xe0.field_0x8 = pByteCode->GetF32();
	pGrab->field_0xe0.field_0x10.x = pByteCode->GetF32();
	pGrab->field_0xe0.field_0x10.y = pByteCode->GetF32();
	pGrab->field_0xe0.field_0x10.z = pByteCode->GetF32();
	pGrab->field_0xe0.field_0x10.w = 0.0f;
	pGrab->field_0xe0.flags = pByteCode->GetU32();

	pGrab->field_0x100.animId = pByteCode->GetS32();
	pGrab->field_0x100.field_0x4 = pByteCode->GetF32();
	pGrab->field_0x100.field_0x8 = pByteCode->GetF32();
	pGrab->field_0x100.field_0x10.x = pByteCode->GetF32();
	pGrab->field_0x100.field_0x10.y = pByteCode->GetF32();
	pGrab->field_0x100.field_0x10.z = pByteCode->GetF32();
	pGrab->field_0x100.field_0x10.w = 0.0f;
	pGrab->field_0x100.flags = pByteCode->GetU32();

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
		this->fightFlags = this->fightFlags | FIGHT_FLAG_EXTRA_HIT_TRACE_BONE_REGISTERED;
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
	edF32VECTOR4 eStack16;

	uVar2 = 0;
	nbCollisions = 0;
	pCollision = this->aFightCollisions;

	// Prev - Current
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

	// Current - Current + 1
	if ((this->pBlow->field_0x4.field_0x0byte & 0x10U) != 0) {
		uVar2 = 0;
		pCollisionDesc = aCollisionDescs + nbCollisions;
		pCollision = this->aFightCollisions;
		while ((uVar2 < this->nbFightCollisions - 1 && (nbCollisions < param_3))) {
			bVar1 = _SV_HIT_FightCollisionCheckIntersect(pCollisionDesc, &pCollision->curBonePosition, &pCollision[1].curBonePosition, kindMask);
			if (bVar1 != false) {
				pCollisionDesc = pCollisionDesc + 1;
				nbCollisions = nbCollisions + 1;
			}

			pCollision = pCollision + 1;
			uVar2 = uVar2 + 1;
		}
	}

	// Current - Prev + 1
	if ((this->pBlow->field_0x4.field_0x0byte & 0x20U) != 0) {
		uVar2 = 0;
		pCollisionDesc = aCollisionDescs + nbCollisions;
		pCollision = this->aFightCollisions;
		while ((uVar2 < this->nbFightCollisions - 1 && (nbCollisions < param_3))) {
			bVar1 = _SV_HIT_FightCollisionCheckIntersect(pCollisionDesc, &pCollision->curBonePosition, &pCollision[1].prevBonePosition, kindMask);
			if (bVar1 != false) {
				pCollisionDesc = pCollisionDesc + 1;
				nbCollisions = nbCollisions + 1;
			}

			pCollision = pCollision + 1;
			uVar2 = uVar2 + 1;
		}

		// Prev - Current + 1
		uVar2 = 0;
		pCollisionDesc = aCollisionDescs + nbCollisions;
		pCollision = this->aFightCollisions;
		while ((uVar2 < this->nbFightCollisions - 1 && (nbCollisions < param_3))) {
			bVar1 = _SV_HIT_FightCollisionCheckIntersect(pCollisionDesc, &pCollision->prevBonePosition, &pCollision[1].curBonePosition, kindMask);
			if (bVar1 != false) {
				pCollisionDesc = pCollisionDesc + 1;
				nbCollisions = nbCollisions + 1;
			}
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
	if ((this->fightFlags & FIGHT_FLAG_EXTRA_HIT_TRACE_BONE_REGISTERED) != 0) {
		this->pAnimationController->UnRegisterBone(0x53538f8b);
		this->fightFlags = this->fightFlags & ~FIGHT_FLAG_EXTRA_HIT_TRACE_BONE_REGISTERED;
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
				this->fightFlags = this->fightFlags | FIGHT_FLAG_ATTACK_CONNECTED;
			}
		}
		else {
			this->fightFlags = this->fightFlags | FIGHT_FLAG_ATTACK_CONNECTED;
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
				local_e0[0] = pReceiver->pCollisionData->pObbPrim->position;

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
	edF32Matrix4MulF32Vector4Hard(v0, &this->pCollisionData->pObbPrim->localToWorld, v0);

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

s_fighter_grab* CActorFighter::FindGrabByName(char* szName)
{
	s_fighter_grab* pGrab;

	const uint hash = CreateHashFromName(szName);
	pGrab = _FindByHashcode<s_fighter_grab>(this->aGrabs, this->nbGrabs, hash);

	return pGrab;
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

s_fighter_grab_react* CActorFighter::FindGrabReactByHash(uint hash)
{
	return _FindByHashcode<s_fighter_grab_react>(this->aGrabReacts, this->nbGrabReacts, hash);
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
	SubStructBlow* pCVar3;
	byte alpha;
	SubStructBlow* puVar4;
	uint uVar5;
	SubStructBlow* pCVar6;
	uint* piVar7;
	uint uVar8;
	float fVar9;
	edF32MATRIX4 eStack176;
	edF32VECTOR4 eStack112;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 eStack32[2];

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
		pCVar6 = this->field_0x610;
		do {
			s_fighter_blow_sub_obj* pBlowSubObj = pCVar6->pBlowSubObj;
			pBlowSubObj->boneRefA.pActor->SV_GetBoneWorldPosition(pBlowSubObj->boneRefA.boneId, &eStack32[0]);
			pBlowSubObj->boneRefB.pActor->SV_GetBoneWorldPosition(pBlowSubObj->boneRefB.boneId, &eStack32[1]);
			pCVar6->pPlayerSubStruct->UpdateTrailFromEndpoints(0.0f, eStack32, 0);

			if ((pCVar6->pPlayerSubStruct->flags & 2) == 0) {
				pBlowSubObj->boneRefA.pActor->pAnimationController->UnRegisterBone(pBlowSubObj->boneRefA.boneId);
				pBlowSubObj->boneRefB.pActor->pAnimationController->UnRegisterBone(pBlowSubObj->boneRefB.boneId);
				uVar5 = uVar8 + 1;
				if (uVar5 < this->field_0x630) {
					puVar4 = this->field_0x610 + uVar8;
					do {
						iVar2 = uVar5 - 1;
						uVar5 = uVar5 + 1;
						pCVar3 = this->field_0x610 + iVar2;
						*pCVar3 = *puVar4;
						puVar4 = puVar4 + 1;
					} while (uVar5 < this->field_0x630);
				}

				this->field_0x630 = this->field_0x630 + -1;
			}
			else {
				lVar1 = Func_0x1b4(inState);
				if ((lVar1 == 0) && ((pCVar6->pPlayerSubStruct->flags & 0x10) == 0)) {
					pCVar6->pPlayerSubStruct->FUN_004012a0(0.25f);
				}
			}

			uVar8 = uVar8 + 1;
			pCVar6 = pCVar6 + 1;
		} while (uVar8 < this->field_0x630);
	}

	iVar2 = this->actorState;
	if ((iVar2 == FIGHTER_HIT_STEP_BACK) || (iVar2 == 0x18)) {
		fVar9 = GetAngleYFromVector(&this->dynamic.velocityDirectionEuler);
		fVar9 = edF32Between_0_2Pi(fVar9 + 3.141593f);
		uVar8 = 0;

		piVar7 = gBoneIds_004343a0;
		do {
			SV_GetBoneWorldPosition(*piVar7, &eStack112);
			edF32Matrix4RotateYHard(fVar9, &eStack96, &gF32Matrix4Unit);
			edF32Matrix4TranslateHard(&eStack96, &eStack96, &eStack112);

			SV_FX_001a0600(&this->field_0x550[uVar8], &eStack96, &this->field_0x550[uVar8]);
			uVar8 = uVar8 + 1;
			piVar7 = piVar7 + 1;
		} while (uVar8 < 2);
	}

	return;
}

void CActorFighter::FUN_0031aad0(float animLength, s_fighter_blow* pBlow)
{
	s_fighter_blow_sub_obj* piVar1;
	bool bVar2;
	s_fighter_blow_sub_obj* piVar3;
	SubStructBlow* pCVar4;
	uint uVar5;
	int iVar6;
	uint uVar7;

	if ((this->field_0x630 != 0) && (uVar7 = 0, pBlow->nbSubObjs != 0)) {
		iVar6 = 0;
		do {
			uVar5 = 0;
			bVar2 = false;
			pCVar4 = this->field_0x610;
			while ((uVar5 < this->field_0x630 && (!bVar2))) {
				piVar1 = pCVar4->pBlowSubObj;
				piVar3 = pBlow->field_0x48 + uVar7;
				if ((piVar1->boneRefA.pActor == piVar3->boneRefA.pActor) && (piVar1->boneRefA.boneId == piVar3->boneRefA.boneId) &&
					(piVar1->boneRefB.pActor == piVar3->boneRefB.pActor) && (piVar1->boneRefB.boneId == piVar3->boneRefB.boneId) &&
					(piVar1->field_0x14uint == piVar3->field_0x14uint) && (piVar1->materialId == piVar3->materialId)) {
					bVar2 = true;
				}
				else {
					pCVar4 = pCVar4 + 1;
					uVar5 = uVar5 + 1;
				}
			}

			if (bVar2) {
				pCVar4->pPlayerSubStruct->FUN_004012a0(animLength);
			}

			uVar7 = uVar7 + 1;
		} while (uVar7 < pBlow->nbSubObjs);
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
	s_fighter_blow_sub_obj* piVar2;
	//CActorFighterVTable* pCVar3;
	bool bVar4;
	bool bVar5;
	int iVar6;
	PlayerSubStruct_64* pPVar5;
	//CActorFighterVTable** ppCVar8;
	uint uVar9;
	SubStructBlow* pCVar10;
	s_fighter_blow_sub_obj* piVar11;
	uint uVar12;
	int iVar13;
	int iVar14;

	iVar13 = 0;
	uVar12 = 0;
	if (pBlow->nbSubObjs != 0) {
		iVar14 = 0;
		do {
			uVar1 = this->field_0x630;
			uVar9 = 0;
			bVar5 = false;
			pCVar10 = this->field_0x610;
			while ((uVar9 < uVar1 && (!bVar5))) {
				piVar2 = pCVar10->pBlowSubObj;
				piVar11 = pBlow->field_0x48 + uVar12;
				if ((piVar2->boneRefA.pActor != piVar11->boneRefA.pActor) ||
					((((piVar2->boneRefA.boneId != piVar11->boneRefA.boneId || (piVar2->boneRefB.pActor != piVar11->boneRefB.pActor)) || (piVar2->boneRefB.boneId != piVar11->boneRefB.boneId)) ||
						((piVar2->field_0x14uint != piVar11->field_0x14uint || (bVar4 = true, piVar2->materialId != piVar11->materialId)))))) {
					bVar4 = false;
				}

				if (bVar4) {
					bVar5 = true;
				}
				else {
					pCVar10 = pCVar10 + 1;
					uVar9 = uVar9 + 1;
				}
			}

			if (bVar5) {
				s_fighter_blow_sub_obj* pBlowSubObj = pBlow->field_0x48 + uVar12;
				this->field_0x610[uVar9].pPlayerSubStruct->FUN_00401230(param_1, pBlowSubObj->field_0x1c, pBlowSubObj->field_0x20);
				iVar13 = iVar13 + 1;
			}
			else {
				if (uVar1 < 4) {
					uVar9 = 0;
					bVar5 = false;
					pPVar5 = PlayerSubStruct_64_ARRAY_0048f900;
					while ((uVar9 < 4 && (!bVar5))) {
						if ((pPVar5->flags & 2) == 0) {
							bVar5 = true;
						}
						else {
							pPVar5 = pPVar5 + 1;
							uVar9 = uVar9 + 1;
						}
					}

					if (bVar5) {
						SubStructBlow* pSubStructBlow = this->field_0x610 + uVar1;
						pSubStructBlow->pBlowSubObj = pBlow->field_0x48 + uVar12;
						pSubStructBlow->pPlayerSubStruct = PlayerSubStruct_64_ARRAY_0048f900 + uVar9;
						pSubStructBlow->pPlayerSubStruct->FUN_00401460(pSubStructBlow->pBlowSubObj->field_0x1c, pSubStructBlow->pBlowSubObj->field_0x20);
						pSubStructBlow->pPlayerSubStruct->FUN_004012f0(param_1, pSubStructBlow->pBlowSubObj->field_0x18, pSubStructBlow->pBlowSubObj->materialId, pSubStructBlow->pBlowSubObj->field_0x14uint);
						pSubStructBlow->pBlowSubObj->boneRefA.pActor->pAnimationController->RegisterBone(pSubStructBlow->pBlowSubObj->boneRefA.boneId);
						pSubStructBlow->pBlowSubObj->boneRefB.pActor->pAnimationController->RegisterBone(pSubStructBlow->pBlowSubObj->boneRefB.boneId);
						iVar13 = iVar13 + 1;
						this->field_0x630 = this->field_0x630 + 1;
					}
				}
			}

			uVar12 = uVar12 + 1;
			iVar14 = iVar14 + 0x24;
		} while (uVar12 < pBlow->nbSubObjs);
	}

	return iVar13 != 0;
}

int CActorFighter::GetProjectedBehaviour()
{
	int iVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	iVar2 = -1;
	if (((this->pCollisionData)->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
		fVar3 = this->field_0x478 + GetTimer()->cutsceneDeltaTime;
		this->field_0x478 = fVar3;
		if (0.15f < fVar3) {
			iVar2 = FIGHTER_BEHAVIOUR_PROJECTED;
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

void CActorFighter::FUN_00312130(edF32VECTOR4* param_2, float* param_3, float* param_4, _msg_params_0x65* param_5)
{
	int iVar1;
	float fVar2;
	float puVar3;
	float puVar5;
	float fVar3;
	float puVar4;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_10;

	edF32Vector4SubHard(&local_10, &this->pAdversary->currentLocation, &this->currentLocation);

	local_10.y = 0.0f;
	fVar2 = edF32Vector4NormalizeHard(&local_10, &local_10);
	fVar2 = (fVar2 - this->pExecutingGrab->field_0x30) / 2.0f;
	puVar3 = edF32Vector4DotProductHard(&local_10, &this->rotationQuat);
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
	if (0.0f <= local_10.z * this->rotationQuat.x - local_10.x * this->rotationQuat.z) {
		puVar4 = 1.0f;
	}
	else {
		puVar4 = -1.0f;
	}

	fVar3 = fVar3 * puVar4;
	fVar5 = (this->pExecutingGrab->field_0xc0).field_0x4;
	iVar1 = GetIdMacroAnim((this->pExecutingGrab->field_0xc0).animId);
	if (iVar1 < 0) {
		fVar4 = 0.0f;
	}
	else {
		fVar4 = this->pAnimationController->GetAnimLength(iVar1, 1);
	}

	fVar4 = fVar4 / fVar5;
	*param_4 = fVar4;
	if (fVar4 <= 0.0f) {
		*param_4 = 1.0f;
	}

	edF32Vector4ScaleHard(fVar2 / *param_4, param_2, &local_10);

	*param_3 = fVar3 / *param_4;
	if (param_5 != (_msg_params_0x65*)0x0) {
		edF32Vector4ScaleHard(-fVar2, &param_5->field_0x0, &local_10);
		edF32Matrix4MulF32Vector4Hard(&param_5->field_0x10, &this->pMeshTransform->base.transformA, &this->pExecutingGrab->field_0x20);
		param_5->field_0x20 = *param_4;
		param_5->field_0x28 = fVar5;
		param_5->field_0x2c = (this->pExecutingGrab->field_0xe0).field_0x4;
		param_5->field_0x24 = this->pExecutingGrab->hash.hash;
		param_5->field_0x30 = &this->pExecutingGrab->field_0x14;
	}

	return;
}

void CActorFighter::FUN_00311f80(_msg_params_0x65* pParams, CActorFighter* param_3)
{
	s_fighter_grab_react* pGrabReact;
	float puVar2;
	float puVar4;
	float fVar1;
	float puVar3;
	edF32VECTOR4 local_10;

	this->pAdversary = param_3;
	puVar2 = edF32Vector4DotProductHard(&this->rotationQuat, &pParams->field_0x10);
	if (1.0f < puVar2) {
		puVar4 = 1.0f;
	}
	else {
		puVar4 = -1.0f;
		if (-1.0f <= puVar2) {
			puVar4 = puVar2;
		}
	}

	fVar1 = acosf(puVar4);
	this->field_0x7b4 = fVar1;
	if (0.0f <= (pParams->field_0x10).x * this->rotationQuat.z - (pParams->field_0x10).z * this->rotationQuat.x) {
		puVar3 = 1.0f;
	}
	else {
		puVar3 = -1.0f;
	}

	this->field_0x7b4 = this->field_0x7b4 * puVar3;
	this->field_0x7b4 = this->field_0x7b4 / pParams->field_0x20;
	edF32Vector4ScaleHard(1.0f / pParams->field_0x20, &local_10, &pParams->field_0x0);
	fVar1 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
	this->dynamic.speed = fVar1;
	this->dynamic.rotationQuat = local_10;
	this->field_0x854 = pParams->field_0x30;
	pGrabReact = FindGrabReactByHash(pParams->field_0x24);
	this->field_0x84c = pGrabReact;
	this->field_0x85c = pParams->field_0x2c;
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(pParams->field_0x28, 0);

	if (this->field_0x84c == (s_fighter_grab_react*)0x0) {
		SetState(0x70, -1);
	}
	else {
		SetState(0x70, this->field_0x84c->field_0x4);
	}

	return;
}

void CActorFighter::UpdateScale_0030ac50(edF32VECTOR3* pNewScale)
{
	edF32VECTOR4* v0;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 local_10;

	SetScaleVector(pNewScale->x, pNewScale->y, pNewScale->z);

	local_10 = (this->fighterAnatomyZones).field_0x0;
	(this->fighterAnatomyZones).field_0x0.xyz = (this->fighterAnatomyZones).field_0x10.xyz * *pNewScale;
	(this->fighterAnatomyZones).field_0x0.w = 1.0f;
	edF32Matrix4FromEulerSoft(&eStack80, &this->rotationEuler.xyz, "XYZ");
	edF32Matrix4MulF32Vector4Hard(&this->fighterAnatomyZones.field_0x0, &eStack80, &this->fighterAnatomyZones.field_0x0);
	edF32Vector4SubHard(&local_10, &local_10, &this->fighterAnatomyZones.field_0x0);
	v0 = &this->currentLocation;
	edF32Vector4AddHard(v0, v0, &local_10);

	return;
}

void CActorFighter::PlayOrientedFx(edF32VECTOR4* pPosition, edF32VECTOR4* param_3, uint hitZone, CFxHandle* pOutHandle)
{
	bool bVar1;
	float fVar2;
	float puVar4;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 rotation;
	edF32MATRIX4 eStack80;
	CFxHandle handle;
	CNewFx* pNewFx;

	if (hitZone != 0xffffffff) {
		handle.id = 0;
		handle.pFx = (CNewFx*)0x0;

		if (pOutHandle == (CFxHandle*)0x0) {
			pOutHandle = &handle;
		}

		CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(pOutHandle, hitZone, FX_MATERIAL_SELECTOR_NONE);

		if (pOutHandle->IsValid()) {
			rotation = gF32Vector4Zero;

			fVar2 = edF32Vector4NormalizeHard(&local_80, param_3);
			if (fVar2 == 0.0f) {
				local_80 = gF32Vector4UnitX;
			}

			if (fabsf(local_80.x) == 1.0f) {
				if (local_80.x < 0.0f) {
					rotation.y = 3.141593f;
				}
			}
			else {
				local_70.x = 0.0f;
				local_70.w = 0.0f;
				local_70.y = -local_80.z;
				local_70.z = local_80.y;

				edF32Vector4NormalizeHard(&local_70, &local_70);
				if (1.0f < local_80.x) {
					puVar4 = 1.0f;
				}
				else {
					puVar4 = -1.0f;
					if (-1.0f <= local_80.x) {
						puVar4 = local_80.x;
					}
				}

				fVar2 = acosf(puVar4);
				edF32Matrix4FromAngAxisSoft(-fVar2, &eStack80, &local_70);
				edF32Matrix4ToEulerSoft(&eStack80, &rotation.xyz, "XYZ");
			}

			pOutHandle->SetPosition(&rotation);
			pOutHandle->SetPosition(pPosition);
			pOutHandle->Start();
		}
	}

	return;
}

void CActorFighter::PlayImpactFx(edF32VECTOR4* pPosition, edF32VECTOR4* param_3, int param_4, bool param_5)
{
	uint hitZone;

	if (param_5 == false) {
		hitZone = _SV_HIT_GetHitZoneFromImpact(&this->fighterAnatomyZones, pPosition, &this->currentLocation);

		if (hitZone == 0) {
			if (param_4 == 0) {
				hitZone = this->field_0x524;
			}
			else {
				hitZone = this->field_0x51c;
			}
		}
		else {
			if (param_4 == 0) {
				hitZone = this->field_0x528;
			}
			else {
				hitZone = this->field_0x520;
			}
		}
	}
	else {
		if (param_4 == 0) {
			hitZone = this->field_0x534;
		}
		else {
			hitZone = this->field_0x530;
		}
	}

	PlayOrientedFx(pPosition, param_3, hitZone, (CFxHandle*)0x0);

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

#define FIGHTER_STICK_FLAG_FORWARD (1 << 0)
#define FIGHTER_STICK_FLAG_RIGHT   (1 << 1)
#define FIGHTER_STICK_FLAG_BACK    (1 << 2)
#define FIGHTER_STICK_FLAG_LEFT    (1 << 3)

uint EvaluateStickDirections(edF32VECTOR4* pDir)
{
	bool bVar1;
	uint result;
	float deltaAngleY;
	edF32VECTOR4 normalizedDir;

	// Detects the direction of the stick (either forward or backward) then applies the left and right flags based on the angle of the stick. 
	// The stick is considered forward if it's within 60 degrees of the forward direction, and backward if it's within 60 degrees of the backward direction. 
	// The right flag is applied if the stick is between 30 and 150 degrees to the right, and the left flag is applied if it's between 30 and 150 degrees to the left.

	result = 0;

	if (0.35f <= edF32Vector4SafeNormalize0Hard(&normalizedDir, pDir)) {
		deltaAngleY = edF32Between_Pi(GetAngleYFromVector(&normalizedDir));
		bVar1 = false;
		if ((0.0f <= fabs(deltaAngleY)) && (bVar1 = false, fabs(deltaAngleY) <= 1.047198f)) {
			bVar1 = true;
		}

		if (bVar1) {
			result = FIGHTER_STICK_FLAG_FORWARD;
		}
		else {
			bVar1 = false;
			if ((2.094395f <= fabs(deltaAngleY)) && (bVar1 = false, fabs(deltaAngleY) <= 3.141593f)) {
				bVar1 = true;
			}

			if (bVar1) {
				result = FIGHTER_STICK_FLAG_BACK;
			}
		}

		bVar1 = false;
		if ((0.5235988f <= deltaAngleY) && (bVar1 = false, deltaAngleY <= 2.617994f)) {
			bVar1 = true;
		}

		if (bVar1) {
			result = result | FIGHTER_STICK_FLAG_RIGHT;
		}
		else {
			bVar1 = false;
			if ((-2.617994f <= deltaAngleY) && (bVar1 = false, deltaAngleY <= -0.5235988f)) {
				bVar1 = true;
			}

			if (bVar1) {
				result = result | FIGHTER_STICK_FLAG_LEFT;
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
		fVar4 = pGrab->field_0x34 - ((this->pCollisionData)->pObbPrim->scale).z;
		pGrab->field_0x34 = fVar4;
		pGrab->field_0x34 = fVar4 - 0.4f;
	}

	pGrab->field_0x10 = pGrab->field_0x10 * (fVar4 / _GetFighterAnimationLength(pGrab->field_0x100.animId));

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
			pCombo->actionHash.pData = STORE_POINTER(pBlow);

			if (pCombo->actionHash.pData == 0x0) {
				s_fighter_grab* pGrab = _FindByHashcode<s_fighter_grab>(this->aGrabs, this->nbGrabs, hash);
				pCombo->actionHash.pData = STORE_POINTER(pGrab);
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
					pBranch->pData = STORE_POINTER(pCombo);
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

		s_fighter_move* pAction = LOAD_POINTER_CAST(s_fighter_move*, pCurCombo->actionHash.pData);

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
		s_fighter_combo* pCurCombo = LOAD_POINTER_CAST(s_fighter_combo*, pCurBranchHash->pData);
		FIGHTER_LOG(LogLevel::Info, "\nCActorFighter::_FindComboBestNext Checking {}", gDebugActionHashToFullName[pCurCombo->hash.hash]);

		s_fighter_move* pAction = LOAD_POINTER_CAST(s_fighter_move*, pCurCombo->actionHash.pData);

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

#define FIGHTER_STATEFLAG_ALLOW_MOVE_DIR_LO      0x200000
#define FIGHTER_STATEFLAG_ALLOW_MOVE_MODE_HI     0x400000
#define FIGHTER_STATEFLAG_ALLOW_ACTION_KIND_LO   0x800000
#define FIGHTER_STATEFLAG_ALLOW_ACTION_TRIGGER_HI 0x1000000

bool CActorFighter::_ValidateCommand(s_fighter_action* pActionA, s_fighter_action* pActionB)
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;

	pActionB->all = pActionA->all & this->validCommandMask.all;

	if ((this->actorState == FIGHTER_BLOW_BEGIN) && (this->pBlow->field_0x50 * this->pBlow->field_0x8 <= this->timeInAir)) {
		pActionB->actionByte = pActionB->actionByte & 0xcf;
	}

	uVar3 = GetStateFlags(this->actorState);
	if ((uVar3 & FIGHTER_STATEFLAG_ALLOW_MOVE_DIR_LO) == 0) {
		pActionB->moveByte = pActionB->moveByte & 0xf0;
	}
	else {
		this->field_0x44c = this->field_0x44c & 0xf0 | pActionB->moveByte & 0xf;
	}

	if ((uVar3 & FIGHTER_STATEFLAG_ALLOW_MOVE_MODE_HI) == 0) {
		pActionB->moveByte = pActionB->moveByte & 0xf;
	}
	else {
		this->field_0x44c = this->field_0x44c & 0xf | (byte)((uint)(((ulong)pActionB->moveByte << 0x38) >> 0x3c) << 4);
	}

	if ((uVar3 & FIGHTER_STATEFLAG_ALLOW_ACTION_KIND_LO) == 0) {
		pActionB->actionByte = pActionB->actionByte & 0xf0;
	}
	else {
		this->field_0x44d = this->field_0x44d & 0xf0 | pActionB->actionByte & 0xf;
	}

	if ((uVar3 & FIGHTER_STATEFLAG_ALLOW_ACTION_TRIGGER_HI) == 0) {
		pActionB->actionByte = pActionB->actionByte & 0xcf;
	}
	else {
		this->field_0x44d = this->field_0x44d & 0xcf | (byte)((uint)(((ulong)pActionB->actionByte << 0x3a) >> 0x3e) << 4);
	}

	return pActionB->all != 0x0;
}

void CActorFighter::_SetRelativeSpeedOnGround(float speed, edF32VECTOR4* pDirection)
{
	this->field_0x44c = this->field_0x44c & 0xf0;
	if (pDirection->z == 0.0f) {
		this->scalarDynForward.Reset();
	}
	else {
		if (0.0f < pDirection->z) {
			this->field_0x44c = this->field_0x44c & 0xf0 | (this->field_0x44c & 0xf) + 3 & 0xf;
			this->scalarDynForward.BuildFromSpeedTime(pDirection->z, pDirection->z, speed);
		}
		else {
			this->field_0x44c = this->field_0x44c & 0xf0 | (this->field_0x44c & 0xf) + 6 & 0xf;
			this->scalarDynForward.BuildFromSpeedTime(-pDirection->z, -pDirection->z, speed);
		}
	}

	if (pDirection->x == 0.0f) {
		this->scalarDynLateral.Reset();
	}
	else {
		if (0.0 < pDirection->x) {
			this->field_0x44c = this->field_0x44c & 0xf0 | (this->field_0x44c & 0xf) + 2 & 0xf;
			this->scalarDynLateral.BuildFromSpeedTime(pDirection->x, pDirection->x, speed);
		}
		else {
			this->field_0x44c = this->field_0x44c & 0xf0 | (this->field_0x44c & 0xf) + 1 & 0xf;
			this->scalarDynLateral.BuildFromSpeedTime(-pDirection->x, -pDirection->x, speed);
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
			pFighterAction->moveByte = pFighterAction->moveByte & 0xf0 | (pFighterAction->moveByte & 0xf) + 2 & 0xf;
		}
		else {
			pFighterAction->moveByte = pFighterAction->moveByte & 0xf0 | (pFighterAction->moveByte & 0xf) + 1 & 0xf;
		}
	}
	if (0.5f <= fabs(param_4->z)) {
		if (0.0f < param_4->z) {
			pFighterAction->moveByte = pFighterAction->moveByte & 0xf0 | (pFighterAction->moveByte & 0xf) + 3 & 0xf;
		}
		else {
			pFighterAction->moveByte = pFighterAction->moveByte & 0xf0 | (pFighterAction->moveByte & 0xf) + 6 & 0xf;
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
	uint l2Pressed;
	uint l2Released;
	s_fighter_combo* psVar17;
	uint crossPressed;
	float r2Click;
	float l2Click;
	uint r2Released;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32MATRIX4 eStack112;
	edF32VECTOR4 local_30;
	edF32VECTOR4 padStickLeftVector;
	s_fighter_action_param fighterActionParam;
	s_fighter_action local_4;

	if (this->pInputAnalyser == (CInputAnalyser*)0x0) {
		return;
	}

	if ((pPlayerInput == (CPlayerInput*)0x0) || (param_3 == 0)) {
		r2Released = 0;
		padStickLeftVector.x = 0.0f;
		r2Click = 0.0f;
		crossPressed = 0;
		l2Released = 0;
		padStickLeftVector.z = 0.0f;
		l2Pressed = 0;
		l2Click = 0.0f;
	}
	else {
		padStickLeftVector.z = pPlayerInput->aAnalogSticks[PAD_STICK_LEFT].y;
		padStickLeftVector.x = pPlayerInput->aAnalogSticks[PAD_STICK_LEFT].x;
		l2Click = pPlayerInput->aButtons[INPUT_BUTTON_INDEX_L2].clickValue;
		r2Click = pPlayerInput->aButtons[INPUT_BUTTON_INDEX_R2].clickValue;
		l2Released = pPlayerInput->releasedBitfield & PAD_BITMASK_L2;
		r2Released = pPlayerInput->releasedBitfield & PAD_BITMASK_R2;
		l2Pressed = pPlayerInput->pressedBitfield & PAD_BITMASK_L2;
		crossPressed = pPlayerInput->pressedBitfield & PAD_BITMASK_CROSS;
	}

	fighterActionParam.pData = (void*)0x0;
	local_30 = gF32Vector4Zero;

	memset(&fighterActionParam, 0, sizeof(s_fighter_action));

	local_4.all = 0;
	iVar9 = 2;
	if ((padStickLeftVector.z != 0.0f) || (fVar3 = local_30.x, padStickLeftVector.y = local_30.y, fVar4 = local_30.z, padStickLeftVector.w = local_30.w, padStickLeftVector.x != 0.0f)) {
		padStickLeftVector.y = 0.0f;
		padStickLeftVector.w = 0.0f;
		edF32Matrix4GetTransposeHard(&eStack112, &(CScene::ptable.g_CameraManager_0045167c)->transMatrix_0x390);
		edF32Matrix4MulF32Vector4Hard(&padStickLeftVector, &eStack112, &padStickLeftVector);

		local_30.z = padStickLeftVector.z * -1.0f;
		local_30.x = padStickLeftVector.x;
		local_30.y = padStickLeftVector.y;
		local_30.w = padStickLeftVector.w;

		padStickLeftVector.z = local_30.z;
		edF32Matrix4SetIdentityHard(&eStack112);
		edF32Matrix4RotateYHard(this->adversaryAngleDiff, &eStack112, &eStack112);
		edF32Matrix4TransposeHard(&eStack112);
		edF32Matrix4MulF32Vector4Hard(&padStickLeftVector, &eStack112, &padStickLeftVector);
		const uint stickDirBitfield = EvaluateStickDirections(&padStickLeftVector);
		if ((stickDirBitfield & FIGHTER_STICK_FLAG_RIGHT) == 0) {
			if ((stickDirBitfield & FIGHTER_STICK_FLAG_LEFT) != 0) {
				local_4.all = local_4.all & 0xffffff00 | (uint)(local_4.moveByte & 0xf0 | (local_4.moveByte & 0xf) + 1 & 0xf);
			}
		}
		else {
			local_4.all = local_4.all & 0xffffff00 | (uint)(local_4.moveByte & 0xf0 | (local_4.moveByte & 0xf) + 2 & 0xf);
		}

		if ((stickDirBitfield & FIGHTER_STICK_FLAG_FORWARD) == 0) {
			if ((stickDirBitfield & FIGHTER_STICK_FLAG_BACK) != 0) {
				local_4.all = local_4.all & 0xffffff00 | (uint)(local_4.moveByte & 0xf0 | (local_4.moveByte & 0xf) + 6 & 0xf);
			}
		}
		else {
			local_4.all = local_4.all & 0xffffff00 | (uint)(local_4.moveByte & 0xf0 | (local_4.moveByte & 0xf) + 3 & 0xf);
		}

		fVar3 = padStickLeftVector.x;
		fVar4 = padStickLeftVector.z;
		if ((local_4.all & 0xf) != 0) {
			if (((l2Click == 0.0f) || ((this->pInputAnalyser->flags & 1) == 0)) || (Func_0x1ac() == 0)) {
				local_4.all = local_4.all & 0xffffff0f | 0x10;
				fVar3 = padStickLeftVector.x;
				fVar4 = padStickLeftVector.z;
			}
			else {
				local_4.all = local_4.all & 0xffffff0f | 0x20;
				this->pInputAnalyser->flags = this->pInputAnalyser->flags & 0xfffffffe;
				fVar3 = padStickLeftVector.x;
				fVar4 = padStickLeftVector.z;
			}
		}
	}

	padStickLeftVector.z = fVar4;
	padStickLeftVector.x = fVar3;
	if (l2Released != 0) {
		this->pInputAnalyser->flags = this->pInputAnalyser->flags | 1;
	}

	if (crossPressed != 0) {
		local_4.all = local_4.all & 0xffffff0f | 0x40;
	}

	if (Func_0x1a4() == 0) goto LAB_0030a390;

	if (Func_0x1a0() == 0) {
		SetVectorFromAngleY(this->adversaryAngleDiff, &eStack144);
		iVar9 = this->pInputAnalyser->Cumulate(pPlayerInput, &eStack144, &padStickLeftVector);
		if (iVar9 != 0) {
			uVar12 = (((ulong)this->pInputAnalyser->patternA.field_0x2ushort) << 0x34) >> 0x38;
			if ((uVar12 & 3) == 0) {
				if ((uVar12 & 4) != 0) {
					fighterActionParam.pData = (void*)0x0;
					local_4.all = local_4.all & 0xfffff0ff | 0x200;
				}
			}
			else {
				fighterActionParam.pData = (void*)0x0;
				local_4.all = local_4.all & 0xfffff0ff | 0x100;
			}
		}

		goto LAB_0030a390;
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
		iVar7 = this->pInputAnalyser->Cumulate(pPlayerInput, &eStack128, &padStickLeftVector);
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
			l2Released = psVar8->nbBranches;
			this->pInputAnalyser->pComboB = (s_fighter_combo*)0x0;
			uVar12 = CInputAnalyser::FUN_003381b0(this->pInputAnalyser, pPlayerInput, &eStack128, &padStickLeftVector);
			if ((uVar12 & 0xff) == 0) {
				while ((l2Released != 0 && (this->pInputAnalyser->pComboB == (s_fighter_combo*)0x0))) {
					if (((*(s_fighter_combo**)psVar17)->field_0x4 & 0x100U) == 0) {
						this->pInputAnalyser->pComboB = *(s_fighter_combo**)psVar17;
					}
					else {
						psVar17 = (s_fighter_combo*)((int)psVar17 + 4);
						l2Released = l2Released - 1;
					}
				}
				if ((this->pFighterCombo->field_0x4 & 0x200U) == 0) {
					iVar9 = 1;
					local_4 = local_4 & 0xfffff0ff | 0x800;
				}
			}
			else {
				while ((l2Released != 0 && (this->pInputAnalyser->pComboB == (s_fighter_combo*)0x0))) {
					if (((*(s_fighter_combo**)psVar17)->field_0x4 & 0x100U) == 0) {
						psVar17 = (s_fighter_combo*)((int)psVar17 + 4);
						l2Released = l2Released - 1;
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
						l2Click = _GetFighterAnimationLength(this->currentAnimType);

						if ((l2Click * this->pFighterCombo->field_0x8 <= this->timeInAir) ||
							(pTVar10 = GetTimer(), l2Click - this->timeInAir <= pTVar10->cutsceneDeltaTime))
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
			s_fighter_blow* pCurrentBlow = LOAD_POINTER_CAST(s_fighter_blow*, psVar8->actionHash.pData);
			s_fighter_blow* pBaseCatch = FindBlowByName("BASE_CATCH");
			if (pCurrentBlow == pBaseCatch) {
				local_4.actionByte = local_4.actionByte & 0xf0 | local_4.actionByte & 0xf | 2, local_4.moveByte;
				local_4.all = local_4.all & 0xffff0000 | (uint)local_4.commandLo;
			}
			else {
				if ((psVar8->field_0x4.field_0x0ushort & 0x400U) == 0) {
					local_4.actionByte = local_4.actionByte & 0xf0 | local_4.actionByte & 0xf | 1, local_4.moveByte;
					local_4.all = local_4.all & 0xffff0000 | (uint)local_4.commandLo;
				}
				else {
					local_4.actionByte = local_4.actionByte & 0xf0 | local_4.actionByte & 0xf | 4, local_4.moveByte;
					local_4.all = local_4.all & 0xffff0000 | (uint)local_4.commandLo;
				}
			}

			psVar8 = this->pInputAnalyser->pComboB;
			fighterActionParam.pData = LOAD_POINTER(psVar8->actionHash.pData);
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
	if (r2Released != 0) {
		this->pInputAnalyser->flags = this->pInputAnalyser->flags | 2;
		this->pInputAnalyser->flags = this->pInputAnalyser->flags | 4;
	}

	if (r2Click != 0.0f) {
		if ((((((ulong)this->validCommandMask.flags[1] << 0x3a) >> 0x3e & 2) != 0) && ((this->pInputAnalyser->flags & 4) != 0)) &&
			(((local_4.all & 0xf) != 0 && (((l2Released = local_4.all & 0xf, l2Released != 2 && (l2Released != 1)) && (5 < l2Released)))))) {
			local_4.all = local_4.all & 0xffffcfff | 0x2000;
			this->pInputAnalyser->flags = this->pInputAnalyser->flags & 0xfffffffb;
		}

		if (((((ulong)local_4.actionByte << 0x3a) >> 0x3e == 0) && ((((ulong)this->validCommandMask.flags[1] << 0x3a) >> 0x3e & 1) != 0)
			) && ((this->pInputAnalyser->flags & 2) != 0)) {
			local_4.all = local_4.all & 0xffffcfff | 0x1000;
		}
	}

	if ((((local_4.all & 0xf00) != 0) && (psVar8 = this->pInputAnalyser->pComboA, psVar8 != (s_fighter_combo*)0x0)) &&
		((((ulong)(psVar8->pattern).field_0x3byte << 0x38) >> 0x3c & 1) != 0)) {
		local_4.all = local_4.all & 0xffffcf0f;
		this->pInputAnalyser->flags = this->pInputAnalyser->flags & 0xfffffffd;
		this->pInputAnalyser->flags = this->pInputAnalyser->flags & 0xfffffffb;
	}

	if (((ulong)local_4.actionByte << 0x3a) >> 0x3e != 0) {
		uVar12 = ((ulong)local_4.moveByte << 0x38) >> 0x3c;
		if (((uVar12 != 0) && (uVar12 != 1)) &&
			(bVar5 = FUN_0031b5d0(this->actorState), bVar5 != false)) {
			local_4.all = local_4.all & 0xffffcfff;
		}
	}

	if ((((ulong)local_4.moveByte << 0x38) >> 0x3c == 0) && (l2Pressed != 0)) {
		local_4.all = local_4.all & 0xffffff0f | 0x80;
	}

	if ((((ulong)local_4.moveByte << 0x38) >> 0x3c != 2) && ((local_4.moveByte & 0xf) != 0)) {
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
		if ((pFighter->pCollisionData->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
			pFighter->field_0x4fc = pFighter->dynamic.linearAcceleration * fabs(pFighter->dynamic.velocityDirectionEuler.y) / 8.0f;
			if (1.0f < pFighter->field_0x4fc) {
				pFighter->field_0x4fc = 1.0;
			}

			if (pFighter->currentLocation.y < pFighter->field_0x4f0) {
				pFighter->fightFlags = pFighter->fightFlags | FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK;
			}
		}
		else {
			pFighter->SetState(0xc, -1);
		}
		break;
	case FIGHTER_JUMP_FALL_TO_FLIP:
		this->pOwner->_StateFighterJumpFallToFlip();
		break;
	case FIGHTER_JUMP_FLIP_IMPULSE:
		this->pOwner->_StateFighterFlipImpulse();
		break;
	case FIGHTER_JUMP_FLIP_PREPARE_FALL:
		this->pOwner->_StateFighterFlipPrepareFall();
		break;
	case FIGHTER_JUMP_FLIP_FALL:
		this->pOwner->_StateFighterFlipFall();
		break;
	case FIGHTER_DEFAULT_STATE_IDLE:
		if (pFighter->field_0x474 <= pFighter->timeInAir) {
			pFighter->AcquireAdversary();
			uVar14 = 0xc;
			if ((pFighter->fightFlags & ~FIGHT_FLAG_ACTION_LOCKED) != 0) {
				pFighter->SetLookingAtOn();
				pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
				pFighter->fightFlags = pFighter->fightFlags & ~FIGHT_FLAG_ACTION_LOCKED;
			}
		}

		pFighter->_ManageFighterDyn(uVar14, 0x100a023b, 0);
		break;
	case FIGHTER_DEFAULT_STATE_RUN:
		this->pOwner->_StateFighterRun((CActorsTable*)0x0);
		break;
	case FIGHTER_JUMP_FLIP_PREPARE_JUMP:
		this->pOwner->_StateFighterFlipPrepareJump();
		break;
	case FIGHTER_JUMP_FLIP_JUMP:
		this->pOwner->_StateFighterFlipJump();
		break;
	case FIGHTER_FLIP_ON_ME:
		this->pOwner->_StateFighterFlipOnMe();
		break;
	case FIGHTER_FLIP_OFF_ME_A:
	case FIGHTER_FLIP_OFF_ME_B:
		this->pOwner->_StateFighterFlipOffMe();
		break;
	case FIGHTER_STATE_19:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		if (((pFighter->pCollisionData)->flags_0x4 & 2) == 0) {
			pFighter->field_0x4fc = (pFighter->dynamic.linearAcceleration * fabsf(pFighter->dynamic.velocityDirectionEuler.y)) / 8.0f;
			if (1.0f < pFighter->field_0x4fc) {
				pFighter->field_0x4fc = 1.0f;
			}

			if (pFighter->currentLocation.y < pFighter->field_0x4f0) {
				pFighter->fightFlags = pFighter->fightFlags | 2;
			}
		}
		else {
			pFighter->SetState(0x1a, -1);
		}
		break;
	case FIGHTER_RIDE:
		this->pOwner->_StateFighterRide();
		break;
	case FIGHTER_STATE_RIDE_BLOW:
		this->pOwner->_StateFighterRideBlow();
		break;
	case FIGHTER_HOLD_STAND:
		this->pOwner->_StateFighterHoldStand();
		break;
	case FIGHTER_HOLD_RUN:
		this->pOwner->_StateFighterHoldRun();
		break;
	case FIGHTER_HOLD_ROTATE:
		this->pOwner->_StateFighterHoldRotate();
		break;
	case FIGHTER_HOLD_PUSH_PREPARE:
		this->pOwner->_StateFighterHoldFollowStd(0x35);
		break;
	case FIGHTER_HOLD_THROW_PREPARE:
		this->pOwner->StateFighterHoldStd(FIGHTER_DEFAULT_STATE_IDLE, iVar9);
		break;
	case FIGHTER_HOLD_THROW:
		this->pOwner->StateFighterHoldThrow();
		break;
	case FIGHTER_HIT_STEP_BACK:
		this->pOwner->_StateFighterHitStepBack(FIGHTER_DEFAULT_STATE_IDLE, iVar9, 1);
		break;
	case FIGHTER_HIT_PUSHED:
		this->pOwner->_StateFighterHitPushed();
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
			pFighter->scalarDynForward.Reset();
			pFighter->scalarDynLateral.Reset();
			pFighter->scalarDynJump.Reset();

			CBehaviourFighter* pBehaviourFighter = reinterpret_cast<CBehaviourFighter*>(pFighter->GetBehaviour(pFighter->curBehaviourId));
			pBehaviourFighter->SetInitialState();
		}
	}
	break;
	case FIGHTER_GRAB_BEGIN:
	{
		pFighter->_ManageFighterDyn((pFighter->pBlow->blowStageBegin).flags, 0x40121, (CActorsTable*)0x0);
		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(FIGHTER_EXECUTE_GRAB, (pFighter->pBlow->blowStageExecute).animId);
		}
	}
	break;
	case FIGHTER_EXECUTE_GRAB:
	{
		pFighter->_ManageFighterDyn((pFighter->pBlow->blowStageExecute).flags, 0x40323, (CActorsTable*)0x0);
		CActorFighter* pCaughtAdversary = pFighter->_Std_GetCaughtAdversary();
		if (pCaughtAdversary != (CActorFighter*)0x0) {
			int msgResult = pFighter->DoMessage(pCaughtAdversary, MESSAGE_CAUGHT, 0);
			if (msgResult != 0) {
				pFighter->pAdversary = pCaughtAdversary;
				pFighter->SetState(FIGHTER_HOLD_STAND, -1);
			}
		}

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			FighterOnHitFunc pcVar6 = pFighter->pBlow->field_0xd0;
			if (pcVar6 != (FighterOnHitFunc)0x0) {
				pcVar6(pFighter, 1);
			}

			pFighter->field_0x6cc = pFighter->field_0x6c8;
			pFighter->SetState(FIGHTER_GRAB_END, (pFighter->pBlow->blowStageEnd).animId);
		}
	}
	break;
	case FIGHTER_GRAB_END:
	{
		pFighter->_ManageFighterDyn((pFighter->pBlow->blowStageEnd).flags, 0x40323, (CActorsTable*)0x0);

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			CBehaviourFighter* pBehaviourFighter = static_cast<CBehaviourFighter*>(pFighter->GetBehaviour(pFighter->curBehaviourId));
			pFighter->scalarDynForward.Reset();
			pFighter->scalarDynLateral.Reset();
			pFighter->scalarDynJump.Reset();
			pBehaviourFighter->SetInitialState();
		}
	}
	break;
	case 0x6b:
		pFighter->_StateFighterExecuteGrab();
		break;
	case 0x6d:
		pFighter->_ManageFighterDyn(0xd, 0x1002023b, (CActorsTable*)0x0);

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x6e, (pFighter->pExecutingGrab->field_0xe0).animId);
		}
		break;
	case 0x6e:
		pFighter->_ManageFighterDyn(pFighter->pExecutingGrab->field_0xe0.flags, 0x40323, (CActorsTable*)0x0);

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			if (pFighter->nextActionType == NEXT_ACTION_TYPE_GRAB) {
				pFighter->pExecutingGrab = pFighter->pNextAction.pGrab;
				pFighter->field_0x474 = pFighter->pExecutingGrab->field_0xc;
				pFighter->SetState(0x6b, (pFighter->pExecutingGrab->field_0x80).animId);
			}
			else {
				if (pFighter->nextActionType == NEXT_ACTION_TYPE_BLOW) {
					pFighter->nextActionType = NEXT_ACTION_TYPE_NONE;
					pFighter->pBlow = pFighter->pNextAction.pBlow;
					pFighter->field_0x474 = pFighter->pBlow->field_0xc;
					pFighter->SetState(0x65, (pFighter->pBlow->blowStageBegin).animId);
				}
				else {
					pFighter->SetState(0x6f, (pFighter->pExecutingGrab->field_0x100).animId);
				}
			}
		}
	case 0x6f:
		pFighter->_ManageFighterDyn((pFighter->pExecutingGrab->field_0xa0).flags, 0x40323, (CActorsTable*)0x0);
		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			CBehaviourFighter* pBehaviourFighter = static_cast<CBehaviourFighter*>(pFighter->GetBehaviour(pFighter->curBehaviourId));
			pFighter->scalarDynForward.Reset();
			pFighter->scalarDynLateral.Reset();
			pFighter->scalarDynJump.Reset();
			pBehaviourFighter->SetInitialState();
		}
		break;
	case 0x70:
		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x71, pFighter->field_0x84c->field_0x8);
		}

		pTVar11 = GetTimer();

		pFighter->rotationEuler.y = pFighter->rotationEuler.y + pFighter->field_0x7b4 * pTVar11->cutsceneDeltaTime;
		pFighter->ManageDyn(4.0, 0x1002023b, (CActorsTable*)0x0);
		break;
	case 0x71:
	{
		local_110.nbEntries = 0;
		pFighter->ManageDyn(4.0f, 0, &local_110);

		bVar1 = (pFighter->pCollisionData)->flags_0x4;
		if (((bVar1 & 4) == 0) && ((bVar1 & 1) == 0)) {
			pFighter->field_0x6a0 = pFighter->currentLocation;
		}
		else {
			edF32Vector4SubHard(&pFighter->field_0x6a0, &pFighter->currentLocation, &pFighter->field_0x6a0);
			pTVar11 = GetTimer();
			fVar16 = pTVar11->cutsceneDeltaTime;
			float fVar14 = edF32Vector4NormalizeHard(&pFighter->field_0x6a0, &pFighter->field_0x6a0);
			fVar14 = fVar14 / fVar16;
			pFighter->field_0x6b0 = fVar14;
			if (fVar14 != 0.0) {
				pFighter->field_0x684 = 1;
				pFighter->field_0x8e4 = 0;
				pFighter->SetBehaviour(4, -1, -1);
				pFighter->_InterpretCollisions(0);
			}
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

	this->pOwner->nextActionType = NEXT_ACTION_TYPE_NONE;
	this->pOwner->pNextAction.pBlow = (s_fighter_blow*)0x0;
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
	this->pOwner->fightFlags = this->pOwner->fightFlags & ~FIGHT_FLAG_ACTION_LOCKED;

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

edF32VECTOR4 edF32VECTOR4_004257c0 = { 0.1f, 0.1f, 0.1f, 0.0f };
edF32VECTOR4 edF32VECTOR4_004257d0 = { 0.0f, 0.1f, 0.0f, 1.0f };

void CBehaviourFighter::InitState(int newState)
{
	CActorFighter* pFighter;
	int iVar2;
	s_fighter_blow_bone_ref* pBlowBoneRef;
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

			pBlowBoneRef = pFighter->field_0x854;
			if ((pBlowBoneRef != (s_fighter_blow_bone_ref*)0x0) && (pBlowBoneRef->pActor != (CActor*)0x0)) {
				pBlowBoneRef->pActor->pAnimationController->RegisterBone(pBlowBoneRef->boneId);
			}
		}
	}
	else {
		if (uVar10 == FIGHTER_EXECUTE_FLAGS_HOLD) {
			if ((this->pOwner->GetStateFlags(this->pOwner->prevActorState) & 0xff800) != FIGHTER_EXECUTE_FLAGS_HOLD) {
				this->pOwner->_BeginFighterHold();
			}
		}
		else {
			if (uVar10 == FIGHTER_EXECUTE_FLAGS_RIDE) {
				if ((this->pOwner->GetStateFlags(this->pOwner->prevActorState) & 0xff800) != FIGHTER_EXECUTE_FLAGS_RIDE) {
					this->pOwner->_BeginFighterRide();
				}
			}
			else {
				if (uVar10 == FIGHTER_EXECUTE_FLAGS_FLIP) {
					if ((this->pOwner->GetStateFlags(this->pOwner->prevActorState) & 0xff800) != FIGHTER_EXECUTE_FLAGS_FLIP) {
						this->pOwner->_BeginFighterFlip();
					}
				}
			}
		}
	}

	switch (newState) {
	case FIGHTER_DEFAULT_STATE_IDLE:
		pFighter = this->pOwner;
		pFighter->actorsExcludeTable.nbEntries = 0;
		pFighter->fightFlags = pFighter->fightFlags | (FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK | FIGHT_FLAG_ACTION_LOCKED);
		pFighter->field_0x44cuint = 0;
		pFighter->dynamic.speed = 0.0f;
		break;
	case FIGHTER_DEFAULT_STATE_RUN:
		pFighter = this->pOwner;

		bVar13 = pFighter->field_0x44c & 0xf;
		if (((bVar13 == 2) || (bVar13 == 1)) || (bVar13 < 6)) {
			pFighter->fightFlags = pFighter->fightFlags & ~FIGHT_FLAG_MOVE_BLEND_MIRRORED;
		}
		else {
			pFighter->fightFlags = pFighter->fightFlags | FIGHT_FLAG_MOVE_BLEND_MIRRORED;
		}

		pFighter->field_0x4e0 = gF32Vector4Zero;
		break;
	case 0x7:
		IMPLEMENTATION_GUARD(CActorFighter::FUN_00319460(this->pOwner);)
		break;
	case 0xa:
		this->pOwner->_StateFighter_0xaInit();
		break;
	case 0xb:
		this->pOwner->_StateFighter_0xbInit(1.6f);
		break;
	case 0xc:
		pCVar4 = this->pOwner->pCollisionData;
		pCVar4->flags_0x0 = pCVar4->flags_0x0 | 0x30;
		break;
	case FIGHTER_JUMP_FLIP_PREPARE_FALL:
		this->pOwner->_StateFighterFlipPrepareFallInit();
		break;
	case FIGHTER_JUMP_FLIP_FALL:
		this->pOwner->_StateFighterFlipFallInit();
		break;
	case FIGHTER_JUMP_FLIP_JUMP:
		this->pOwner->_StateFighterFlipJumpInit();
		break;
	case FIGHTER_FLIP_ON_ME:
		this->pOwner->_StateFighterFlipOnMeInit();
		break;
	case 0x17:
		IMPLEMENTATION_GUARD();
		break;
	case 0x18:
		IMPLEMENTATION_GUARD();
		break;
	case 0x1a:
		IMPLEMENTATION_GUARD();
		break;
	case 0x1b:
		IMPLEMENTATION_GUARD();
		break;
	case 0x1c:
		IMPLEMENTATION_GUARD();
		break;
	case FIGHTER_STATE_RIDE_EXIT:
		IMPLEMENTATION_GUARD(CActorFighter::_StateFighterRideExitInit(this->pOwner));
		break;
	case FIGHTER_HOLD_STAND:
		this->pOwner->_StateFighterHoldStandInit();
		break;
	case FIGHTER_HOLD_RUN:
		this->pOwner->_StateFighterHoldRunInit();
		break;
	case FIGHTER_HOLD_ROTATE:
		this->pOwner->_StateFighterHoldRotateInit();
		break;
	case FIGHTER_HOLD_PUSH_PREPARE:
		this->pOwner->_StateFighterHoldPushPrepareInit();
		break;
	case FIGHTER_HOLD_THROW:
		this->pOwner->_StateFighterHoldThrowInit();
		break;
	case FIGHTER_STATE_HOLD_ESCAPE:
		IMPLEMENTATION_GUARD(CActorFighter::_StateFighterHoldEscapeInit(this->pOwner));
		break;
	case FIGHTER_STATE_HOLD_HIT:
		IMPLEMENTATION_GUARD(CActorFighter::_StateFighterHoldHitInit(this->pOwner));
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
	case FIGHTER_HIT_PUSHED:
		this->pOwner->_StateFighterHitPushedInit();
		break;
	case FIGHTER_HIT_STEP_BACK_B:
		IMPLEMENTATION_GUARD(CActorFighter::_StateFighterHitStepBackInit(this->pOwner, 0x5e, 0));
		break;
	case 0x53:
		IMPLEMENTATION_GUARD();
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

		pFighter->fightFlags = pFighter->fightFlags & ~FIGHT_FLAG_ATTACK_CONNECTED;

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
	case 0x6b:
		pFighter = this->pOwner;
		pFighter->fightFlags = pFighter->fightFlags & 0xffffffbf;
		pFighter->_StateFighterFightActionDynInit(&pFighter->pExecutingGrab->field_0x80);
		break;
	case 0x6c:
		this->pOwner->_StateFighterFightActionDynInit(&this->pOwner->pExecutingGrab->field_0xa0);
		break;
	case 0x6d:
		this->pOwner->pCollisionData->actorFieldA = this->pOwner->pAdversary;
		break;
	case 0x6e:
		pFighter = this->pOwner;
		local_60 = edF32VECTOR4_004257c0;
		local_70 = edF32VECTOR4_004257d0;
		pFighter->ChangeCollisionSphere(0.0f, &local_60, &local_70);
		(pFighter->pCollisionData)->actorFieldA = pFighter->pAdversary;
		pFighter->_StateFighterFightActionDynInit(&pFighter->pExecutingGrab->field_0xe0);
		break;
	case 0x6f:
		this->pOwner->_StateFighterFightActionDynInit(&this->pOwner->pExecutingGrab->field_0x100);
		break;
	case 0x71:
	{
		pFighter = this->pOwner;
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(pFighter->field_0x85c, 0);

		s_fighter_blow_bone_ref* psVar3 = pFighter->field_0x854;
		if ((psVar3 != (s_fighter_blow_bone_ref*)0x0) && (psVar3->pActor != (CActor*)0x0)) {
			pFighter->LinkToActor(psVar3->pActor, psVar3->boneId, 1);
		}

		pFighter->field_0x6a0 = pFighter->currentLocation;
	}
		break;
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
	case 0x7:
		IMPLEMENTATION_GUARD();
		break;
	case 0xa:
	case 0xb:
		pFighter = this->pOwner;
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		break;
	case 0xd:
	case FIGHTER_JUMP_FALL_TO_FLIP:
		pFighter = this->pOwner;
		if ((pFighter->GetStateFlags(newState) & 0x100) != 0) {
			pFighter->fightFlags = pFighter->fightFlags | FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK;
		}
		break;
	case FIGHTER_FLIP_OFF_ME_A:
	case FIGHTER_FLIP_OFF_ME_B:
		this->pOwner->_StateFighterFlipOffMeTerm(newState);
		break;
	case 0x18:
		IMPLEMENTATION_GUARD();
		break;
	case FIGHTER_STATE_19:
	{
		pFighter = this->pOwner;
		if ((pFighter->GetStateFlags(newState) & 0x100) != 0) {
			pFighter->fightFlags = pFighter->fightFlags | 2;
		}
	}
	case 0x1a:
		IMPLEMENTATION_GUARD();
		break;
	case 0x23:
		IMPLEMENTATION_GUARD();
		break;
	case FIGHTER_HOLD_PUSH_PREPARE:
		this->pOwner->_StateFighterHoldPushPrepareTerm();
		break;
	case FIGHTER_HOLD_THROW:
		this->pOwner->_StateFighterHoldThrowTerm();
		break;
	case 0x38:
		IMPLEMENTATION_GUARD();
		break;
	case 0x53:
		IMPLEMENTATION_GUARD();
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
			pFighter->scalarDynForward.Reset();
			pFighter->scalarDynLateral.Reset();
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
			FighterOnHitFunc pcVar1 = pFighter->pBlow->field_0xd0;
			if (pcVar1 != (FighterOnHitFunc)0x0) {
				(*pcVar1)(pFighter, 0);
			}
		}
		else {
			FighterOnHitFunc pcVar1 = pFighter->pBlow->field_0xd0;
			if (pcVar1 != (FighterOnHitFunc)0x0) {
				(*pcVar1)(pFighter, 3);
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
	case FIGHTER_BLOW_END:
	case 0x6a:
	{
		pFighter = this->pOwner;
		FighterOnHitFunc pcVar1 = pFighter->pBlow->field_0xd0;
		if (pcVar1 != (FighterOnHitFunc)0x0) {
			bVar5 = true;
			bool bVar4 = false;
			if ((newState - 0x49U < 0x20) && ((1 << (newState - 0x49U & 0x1f) & 0xf000003fU) != 0)) {
				bVar4 = true;
			}

			if ((!bVar4) && (6 < newState - 0x69U)) {
				bVar5 = false;
			}

			if (bVar5) {
				(*pcVar1)(pFighter, 2);
			}
			else {
				(*pcVar1)(pFighter, 3);
			}
		}

		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		pFighter->field_0x36c = pFighter->field_0x36c | 1;
		pFighter->field_0x47c = 23.56194f;
	}
		break;
	case 0x6b:
		pFighter = this->pOwner;
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		pFighter->field_0x36c = pFighter->field_0x36c | 1;
		pFighter->field_0x47c = 23.56194f;
	break;
	case 0x6c:
		IMPLEMENTATION_GUARD();
		break;
	case 0x6d:
		pFighter = this->pOwner;
		pFighter->field_0x47c = 23.56194f;
		pFighter->scalarDynForward.Reset();
		pFighter->scalarDynLateral.Reset();
		pFighter->scalarDynJump.Reset();
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		pFighter->pCollisionData->actorFieldA = (CActor*)0x0;
		break;
	case 0x6e:
		this->pOwner->FUN_00312370();
		break;
	case 0x6f:
		pFighter = this->pOwner;
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		pFighter->field_0x36c = pFighter->field_0x36c | 1;
		pFighter->field_0x47c = 23.56194f;
		break;
	case 0x70:
		pFighter = this->pOwner;
		pFighter->dynamic.speed = 0.0f;
		pFighter->field_0x7b4 = 0.0f;
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		break;
	case 0x71:
		pFighter = this->pOwner;
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

		if ((pFighter->field_0x854 != (s_fighter_blow_bone_ref*)0x0) && (pFighter->field_0x854->pActor != (CActor*)0x0)) {
			pFighter->UnlinkFromActor();
		}

		break;
	}

	uVar13 = this->pOwner->GetStateFlags(oldState) & 0xff800;
	if (uVar13 == 0x80000) {
		if ((this->pOwner->GetStateFlags(newState) & 0xff800) != 0x80000) {
			pFighter = this->pOwner;
			if (newState != -1) {
				pFighter->Compute2DOrientationFromAngles();
				pFighter->flags = pFighter->flags & 0xffffefff;
			}

			pCVar2 = pFighter->pCollisionData;
			pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfffff7ff;
			(pFighter->pCollisionData)->actorFieldA = (CActor*)0x0;
			s_fighter_blow_bone_ref* psVar3 = pFighter->field_0x854;
			if ((psVar3 != (s_fighter_blow_bone_ref*)0x0) && (psVar3->pActor != (CActor*)0x0)) {
				psVar3->pActor->pAnimationController->UnRegisterBone(psVar3->boneId);
			}
		}
	}
	else {
		if (uVar13 == FIGHTER_EXECUTE_FLAGS_HOLD) {
			if ((this->pOwner->GetStateFlags(newState) & 0xff800) != FIGHTER_EXECUTE_FLAGS_HOLD) {
				this->pOwner->_EndFighterHold();
			}
		}
		else {
			if (uVar13 == FIGHTER_EXECUTE_FLAGS_RIDE) {
				if ((this->pOwner->GetStateFlags(newState) & 0xff800) != FIGHTER_EXECUTE_FLAGS_RIDE) {
					this->pOwner->_EndFighterRide(newState);
				}
			}
			else {
				if (uVar13 == FIGHTER_EXECUTE_FLAGS_FLIP) {
					if (newState == -1) {
						if ((this->pOwner->GetStateFlags(newState) & 0xff800) != FIGHTER_EXECUTE_FLAGS_FLIP) {
							this->pOwner->_EndFighterFlip();
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
		_msg_params_get_position* pPositionParams = reinterpret_cast<_msg_params_get_position*>(pMsgParam);
		if (pPositionParams->field_0x0 == 5) {
			if ((this->pOwner->GetStateFlags(this->pOwner->actorState) & 0xff800) == FIGHTER_EXECUTE_FLAGS_RIDE) {
				edF32Vector4ScaleHard(2.0f, &pPositionParams->vectorFieldB, &(this->pOwner->field_0x354->fighterAnatomyZones).field_0x10);
				pPositionParams->vectorFieldB.w = 0.0f;
				return 1;
			}
		}
	}
	else {
		if (msg == MESSAGE_REQUEST_CAMERA_TARGET) {
			if ((this->pOwner->GetStateFlags(this->pOwner->actorState) & 0xff800) == FIGHTER_EXECUTE_FLAGS_RIDE) {
				edF32VECTOR4* pVectorParam = reinterpret_cast<edF32VECTOR4*>(pMsgParam);
				*pVectorParam = this->pOwner->field_0x354->currentLocation;
				return 1;
			}
		}
		else {
			if (msg == MESSAGE_CAUGHT) {
				pCVar4 = this->pOwner->GetBehaviour(6);
				if (pCVar4 != (CBehaviour*)0x0) {
					return 1;
				}
			}
			else {
				if (msg == 0x65) {
					this->pOwner->FUN_00311f80(reinterpret_cast<_msg_params_0x65*>(pMsgParam), static_cast<CActorFighter*>(pSender));
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

				if (msg == MESSAGE_KICKED) {
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
	if ((pFighter->fightFlags & FIGHT_FLAG_ACTION_LOCKED) == 0) {
		pFighter->_ValidateCommand(param_2, &local_4);

		uVar4 = pFighter->GetStateFlags(pFighter->actorState) & 0xff800;
		if (uVar4 == FIGHTER_EXECUTE_FLAGS_HOLD) {
			pFighter->_Execute_Hold(&local_4, param_3);
		}
		else {
			if (uVar4 == FIGHTER_EXECUTE_FLAGS_RIDE) {
					pFighter->_Execute_Ride(&local_4, param_3);
			}
			else {
				if (uVar4 == FIGHTER_EXECUTE_FLAGS_FLIP) {
					pFighter->_Execute_Flip(&local_4, param_3);
				}
				else {
					if (uVar4 == FIGHTER_EXECUTE_FLAGS_STD) {
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

	if (flags == FIGHTER_EXECUTE_FLAGS_RIDE) {
		this->pOwner->_Ride_GetPossibleHit(bPlayImpact);
	}
	else {
		if (flags == FIGHTER_EXECUTE_FLAGS_HOLD) {
			this->pOwner->_Hold_GetPossibleHit(bPlayImpact);
		}
		else {
			if ((flags == 0x80000) || (flags == FIGHTER_EXECUTE_FLAGS_STD)) {
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
		if (flags == FIGHTER_EXECUTE_FLAGS_HOLD) {
			this->pOwner->_Hold_GetPossibleExit();
		}
		else {
			if (flags == FIGHTER_EXECUTE_FLAGS_RIDE) {
				this->pOwner->_Ride_GetPossibleExit();
			}
			else {
				if ((flags == 0x80000) || (flags == FIGHTER_EXECUTE_FLAGS_STD)) {
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
		if (((ulong)(pAction->moveByte & this->pOwner->field_0x44c) << 0x38) >> 0x3c == 0) {
			bVar1 = this->pOwner->field_0x44d;
			if (((pAction->actionByte & 0xf & bVar1 & 0xf) != 0) ||
				(bVar3 = false, ((ulong)(pAction->actionByte & bVar1) << 0x3a) >> 0x3e != 0)) goto LAB_00313210;
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

	dirFlags = EvaluateStickDirections(&local_10);
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

		if (pPlayerInput->aButtons[INPUT_BUTTON_INDEX_R2].clickValue == 0.0f) {
			bVar1 = this->patternB.field_0x3byte;
			this->patternB.field_0x3byte = bVar1 & 0xf | (byte)(((uint)(((ulong)bVar1 << 0x38) >> 0x3c) & 0xe) << 4);
		}
		else {
			bVar1 = this->patternB.field_0x3byte;
			this->patternB.field_0x3byte = bVar1 & 0xf | (byte)(((uint)(((ulong)bVar1 << 0x38) >> 0x3c) | 1) << 4);
		}

		if (pPlayerInput->aButtons[INPUT_BUTTON_INDEX_L2].clickValue == 0.0f) {
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

		if ((pPlayerInput->pressedBitfield & PAD_BITMASK_TRIANGLE) != 0) {
			uVar2 = this->patternB.field_0x2ushort;
			this->patternB.field_0x2ushort = uVar2 & 0xf00f | (ushort)(((uint)(((ulong)uVar2 << 0x34) >> 0x38) | 2) << 4);
		}

		if ((pPlayerInput->pressedBitfield & PAD_BITMASK_CIRCLE) != 0) {
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

	this->instanceIndex = 0;
	this->field_0x61 = 0;
	this->instanceIndex = 0;
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
		peVar1->x = peVar1->x * 0.8333333f;
		peVar1->y = peVar1->y * 0.8333333f;
	}

	this->instanceIndex = 0;
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
	this->field_0x70 = 0.0f;
	this->field_0x6c = 0;
	this->field_0x68 = 0;

	if (this->instanceIndex == 0) {
		if (HasMesh() != false) {
			Term((ed_3D_Scene*)0x0);
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

	if (this->instanceIndex == 1) {
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

void StaticMeshComponentAdvanced::SetMatrix(edF32MATRIX4* pMatrix)
{
	ed_3d_hierarchy_node* peVar1;

	peVar1 = this->pMeshTransformData;
	if (peVar1 != (ed_3d_hierarchy_node*)0x0) {
		if (pMatrix == (edF32MATRIX4*)0x0) {
			pMatrix = &gF32Matrix4Unit;
		}

		peVar1->base.transformA = *pMatrix;
	}

	return;
}

void StaticMeshComponentAdvanced::Func_0x28(float param_1, float param_2)
{
	edNODE* pNode;
	edF32VECTOR4* peVar1;

	if (((HasMesh() == false) && (this->field_0x64 != 2)) && (this->field_0x64 != 1)) {
		peVar1 = GetTextureAnimSpeedNormalExtruder();
		if (param_2 == 0.0f) {
			if (peVar1 != (edF32VECTOR4*)0x0) {
				peVar1->z = 0.0f;
				peVar1->w = 0.0f;
			}

			this->field_0x64 = 2;
		}
		else {
			if (peVar1 != (edF32VECTOR4*)0x0) {
				this->field_0x74 = peVar1->x;
				this->field_0x78 = peVar1->y;
				peVar1->x = 0.0f;
				peVar1->y = 0.0f;
				peVar1->z = 0.0f;
				peVar1->w = 0.0f;
			}

			this->field_0x64 = 1;
		}

		this->field_0x6c = param_1;
		this->field_0x70 = param_2;
		this->field_0x68 = 0.0f;

		if (this->instanceIndex == 0) {
			Init((ed_3D_Scene*)0x0, (ed_g3d_manager*)0x0, (ed_3d_hierarchy_setup*)0x0, (char*)0x0);
		}
		else {
			SetHidden((ed_3D_Scene*)0x0);
		}

		pNode = this->pMeshTransformParent;
		if (pNode != (edNODE*)0x0) {
			ed3DHierarchyNodeSetAlpha(pNode, 0);
		}
	}

	return;
}

void StaticMeshComponentAdvanced::Func_0x2c(float param_1)
{
	edNODE* pNode;
	bool bVar1;
	byte alpha;
	float fVar2;

	if (HasMesh() == true) {
		if (param_1 == 0.0f) {
			if (this->instanceIndex == 0) {
				if (HasMesh() != false) {
					Term((ed_3D_Scene*)0x0);
				}
			}
			else {
				SetVisible((ed_3D_Scene*)0x0);
			}
		}
		else {
			if (this->field_0x64 != 3) {
				if (this->field_0x64 == 2) {
					this->field_0x68 = param_1 * (1.0f - this->field_0x68 / this->field_0x6c);
				}
				else {
					this->field_0x68 = 0.0f;
				}

				this->field_0x64 = 3;
				this->field_0x6c = param_1;
				pNode = this->pMeshTransformParent;
				fVar2 = (1.0f - this->field_0x68 / this->field_0x6c) * 128.0f;
				if (pNode != (edNODE*)0x0) {
					if (fVar2 < 2.147484e+09f) {
						alpha = (byte)(int)fVar2;
					}
					else {
						alpha = (byte)(int)(fVar2 - 2.147484e+09f);
					}

					ed3DHierarchyNodeSetAlpha(pNode, alpha);
				}
			}
		}
	}
	return;
}

void StaticMeshComponentAdvanced::Func_0x30(edF32MATRIX4* pMatrix, float param_3)
{
	int iVar1;
	edNODE* peVar2;
	bool bVar3;
	edF32VECTOR4* peVar5;
	byte bVar6;
	float fVar7;

	if (HasMesh() != false) {
		this->field_0x68 = this->field_0x68 + GetTimer()->cutsceneDeltaTime;
		iVar1 = this->field_0x64;

		if (iVar1 == 3) {
			if (this->field_0x6c <= this->field_0x68) {
				this->field_0x64 = 0;
				fVar7 = 0.0f;

				if (this->instanceIndex == 0) {
					if (HasMesh() != false) {
						Term((ed_3D_Scene*)0x0);
					}
				}
				else {
					SetVisible((ed_3D_Scene*)0x0);
				}
			}
			else {
				fVar7 = 1.0f - this->field_0x68 / this->field_0x6c;
			}

			if ((HasMesh() != false) && (peVar2 = this->pMeshTransformParent, peVar2 != (edNODE*)0x0))
			{
				fVar7 = fVar7 * 128.0f;
				if (fVar7 < 2.147484e+09f) {
					bVar6 = (byte)(int)fVar7;
				}
				else {
					bVar6 = (byte)(int)(fVar7 - 2.147484e+09f);
				}

				ed3DHierarchyNodeSetAlpha(peVar2, bVar6);
			}
		}
		else {
			if (iVar1 == 2) {
				fVar7 = 1.0f;

				if (this->field_0x6c <= this->field_0x68) {
					this->field_0x64 = 0;
				}
				else {
					fVar7 = this->field_0x68 / this->field_0x6c;
				}

				peVar2 = this->pMeshTransformParent;
				if (peVar2 != (edNODE*)0x0) {
					fVar7 = fVar7 * 128.0f;
					if (fVar7 < 2.147484e+09f) {
						bVar6 = (byte)(int)fVar7;
					}
					else {
						bVar6 = (byte)(int)(fVar7 - 2.147484e+09f);
					}

					ed3DHierarchyNodeSetAlpha(peVar2, bVar6);
				}
			}
			else {
				if ((iVar1 == 1) && (this->field_0x70 <= this->field_0x68)) {
					peVar5 = GetTextureAnimSpeedNormalExtruder();
					if (peVar5 != (edF32VECTOR4*)0x0) {
						peVar5->x = this->field_0x74;
						peVar5->y = this->field_0x78;
					}

					this->field_0x64 = 2;
					this->field_0x68 = 0.0f;
				}
			}
		}

		if (pMatrix != (edF32MATRIX4*)0x0) {
			SetMatrix(pMatrix);
		}
	}

	return;
}

bool StaticMeshComponentAdvanced::IsValid()
{
	bool bValid;

	bValid = this->textureIndex != -1;

	if (bValid) {
		bValid = this->meshIndex != -1;
	}

	if (bValid == false) {
		bValid = this->pMeshTransformParent != (edNODE*)0x0;
	}

	return bValid;
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
	if (this->instanceIndex == 0) {
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
				
				ed_hash_code* pData = LOAD_POINTER_CAST(ed_hash_code*, peVar2->pData);

				ed_Chunck* pChunk = LOAD_POINTER_CAST(ed_Chunck*, pData->pData);

				peVar1 = reinterpret_cast<ed_g2d_material*>(pChunk + 1);
			}
		}
	}

	if (peVar1 == (ed_g2d_material*)0x0) {
		peVar4 = (edF32VECTOR4*)0x0;
	}
	else {
		peVar3 = ed3DG2DGetTextureFromMaterial(peVar1, 0);
		peVar4 = LOAD_POINTER_CAST(edF32VECTOR4*, peVar3->pAnimSpeedNormalExtruder);
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

PlayerSubStruct_64::PlayerSubStruct_64()
{
	this->field_0x4 = 0xffffffff;
	this->pSamples[0] = (edF32VECTOR4*)0x0;
	this->pSplineCoefficients[0] = (edF32VECTOR4*)0x0;

	this->pSamples[1] = (edF32VECTOR4*)0x0;
	this->pSplineCoefficients[1] = (edF32VECTOR4*)0x0;

	this->field_0x14 = 0;
	this->nbSamples = 0;
	this->flags = 0;

	return;
}

void PlayerSubStruct_64::Init(uint param_2)
{
	uint uVar3;

	this->nbSamples = param_2;

	// Clamp nbSamples to [2, 8]
	if (this->nbSamples < 9) {
		if (this->nbSamples < 2) {
			this->nbSamples = 2;
		}
	}
	else {
		this->nbSamples = 8;
	}

	uVar3 = 0;
	do {
		this->pSamples[uVar3] = new edF32VECTOR4[this->nbSamples];
		this->pSplineCoefficients[uVar3] = new edF32VECTOR4[(this->nbSamples - 1) * 3];
		uVar3 = uVar3 + 1;
	} while (uVar3 < 2);

	this->flags = this->flags | 1;

	return;
}

void PlayerSubStruct_64::Term()
{
	uint uVar2;

	if ((this->flags & 1) != 0) {
		uVar2 = 0;
		do {
			if (this->pSamples[uVar2] != (edF32VECTOR4*)0x0) {
				delete(this->pSamples[uVar2]);
				this->pSamples[uVar2] = (edF32VECTOR4*)0x0;
			}

			if (this->pSplineCoefficients[uVar2] != (edF32VECTOR4*)0x0) {
				delete(this->pSplineCoefficients[uVar2]);
				this->pSplineCoefficients[uVar2] = (edF32VECTOR4*)0x0;
			}

			uVar2 = uVar2 + 1;
		} while (uVar2 < 2);

		this->flags = this->flags & 0xfffffffe;
	}

	return;
}

void PlayerSubStruct_64::FUN_00401460(float param_1, int param_3)
{
	this->field_0x24 = param_1;
	this->field_0x28 = param_3;
	this->field_0x20 = 0.0f;

	return;
}

void PlayerSubStruct_64::FUN_004012f0(float param_1, uint param_3, int materialId, uint param_5)
{
	C3DFileManager* pFileManager;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	this->flags = this->flags & 0xffffff8f;
	this->flags = this->flags | 10;
	this->flags = this->flags | param_3 & 0x60;
	this->field_0x14 = 0;
	this->field_0x20 = 0.0f;
	this->field_0x30 = param_1;
	this->field_0x34 = 0;
	this->field_0x3c = 1.0f / this->field_0x30;
	this->field_0x38 = 0.0f;
	this->field_0x44 = param_5;

	this->pMaterial = pFileManager->GetMaterialFromId(materialId, 0);
	if (this->pMaterial == (edDList_material*)0x0) {
		this->pMaterial = pFileManager->GetMaterialFromId(CScene::_pinstance->defaultTextureIndex_0x28, 0);
	}

	return;
}

void PlayerSubStruct_64::FUN_00401230(float param_1, float param_2, int param_4)
{
	if ((this->flags & 2) != 0) {
		this->field_0x24 = param_2;
		this->field_0x28 = param_4;
		this->field_0x30 = param_1;
		this->field_0x34 = 0;
		this->field_0x3c = (1.0f - this->field_0x38) / this->field_0x30;
		this->flags = this->flags & 0xffffffef;
		this->flags = this->flags | 8;
	}

	return;
}

void PlayerSubStruct_64::FUN_004012a0(float param_1)
{
	this->flags = this->flags & 0xfffffff7;
	this->flags = this->flags | 0x10;
	this->field_0x30 = param_1;
	this->field_0x34 = 0.0f;
	this->field_0x3c = -this->field_0x38 / this->field_0x30;
	return;
}

void PlayerSubStruct_64::UpdateTrailFromEndpoints(float param_1, edF32VECTOR4* pEndPoints, int bForce)
{
	bool bVar1;
	uint uVar3;
	edF32VECTOR4* peVar4;
	edF32VECTOR4** pPVar5;
	uint uVar6;
	int iVar7;
	uint uVar8;
	edF32VECTOR4* peVar9;
	float fVar10;
	float fVar11;
	float fVar12;

	if ((this->flags & 2) != 0) {
		iVar7 = 0;
		if (this->field_0x24 == 0.0f) {
			this->field_0x20 = param_1;
			iVar7 = bForce;
		}
		else {
			if (this->field_0x24 < GetTimer()->scaledTotalTime - this->field_0x20) {
				this->field_0x20 = GetTimer()->scaledTotalTime;
				iVar7 = 1;
			}
		}

		if (iVar7 == 1) {
			uVar8 = 0;
			pPVar5 = this->pSamples;
			peVar4 = pEndPoints;
			if (this->field_0x14 == 0) {
				do {
					uVar3 = this->nbSamples;
					peVar9 = *pPVar5;
					uVar6 = 0;
					if (uVar3 != 0) {
						do {
							uVar6 = uVar6 + 1;
							*peVar9 = *peVar4;
							uVar3 = this->nbSamples;
							peVar9 = peVar9 + 1;
						} while (uVar6 < uVar3);
					}
					uVar8 = uVar8 + 1;
					pPVar5 = pPVar5 + 1;
					peVar4 = peVar4 + 1;
				} while (uVar8 < 2);

				this->field_0x14 = uVar3;
				this->flags = this->flags | 4;
			}
			else {
				uVar8 = 0;
				bVar1 = false;
				while ((uVar8 < 2 && (!bVar1))) {
					peVar9 = *pPVar5;
					if ((peVar9->x != peVar4->x) || ((peVar9->y != peVar4->y || (peVar9->z != peVar4->z)))) {
						bVar1 = true;
					}

					pPVar5 = pPVar5 + 1;
					peVar4 = peVar4 + 1;
					uVar8 = uVar8 + 1;
				}

				uVar8 = 0;
				pPVar5 = this->pSamples;
				if (bVar1) {
					do {
						iVar7 = this->nbSamples - 2;
						if (-1 < iVar7) {
							peVar4 = *pPVar5 + iVar7;
							do {
								iVar7 = iVar7 + -1;
								peVar4[1] = *peVar4;
								peVar4 = peVar4 + -1;
							} while (-1 < iVar7);
						}

						uVar8 = uVar8 + 1;
						pPVar5 = pPVar5 + 1;
					} while (uVar8 < 2);
				}
			}
		}

		peVar4 = this->pSamples[0];
		*peVar4 = pEndPoints[0];
		peVar4 = this->pSamples[1];
		*peVar4 = pEndPoints[1];

		BuildSplineCoefficients();

		fVar10 = this->field_0x34 + GetTimer()->cutsceneDeltaTime;
		this->field_0x34 = fVar10;
		if (this->field_0x30 <= fVar10) {
			if ((this->flags & 0x10) != 0) {
				this->flags = this->flags & 0xfffffff9;
			}
			this->flags = this->flags & 0xffffffe7;
		}
		else {
			fVar10 = this->field_0x38 + this->field_0x3c * GetTimer()->cutsceneDeltaTime;

			this->field_0x38 = fVar10;
			if (1.0f < fVar10) {
				this->field_0x38 = 1.0f;
			}
			else {
				if (fVar10 < 0.0f) {
					this->field_0x38 = 0.0f;
				}
			}
		}
	}

	return;
}

float FLOAT_ARRAY_0049cb68[10];
edF32VECTOR4 edF32VECTOR4_ARRAY_0049cb90[8];
edF32VECTOR4 edF32VECTOR4_ARRAY_0049cc10[32];

void PlayerSubStruct_64::BuildSplineCoefficients()
{
	int iVar1;
	float* pfVar2;
	edF32VECTOR4* peVar3;
	uint uVar4;
	uint uVar5;
	edF32VECTOR4* peVar7;
	edF32VECTOR4* v0;
	edF32VECTOR4* v2;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4* local_10;

	uVar4 = 1;
	FLOAT_ARRAY_0049cb68[2] = 0.5f;
	iVar1 = this->field_0x14;
	if (1 < iVar1 - 1U) {
		pfVar2 = FLOAT_ARRAY_0049cb68 + 3;
		do {
			uVar4 = uVar4 + 1;
			*pfVar2 = 1.0f / (4.0f - pfVar2[-1]);
			iVar1 = this->field_0x14;
			pfVar2 = pfVar2 + 1;
		} while (uVar4 < iVar1 - 1U);
	}

	uVar4 = 0;
	FLOAT_ARRAY_0049cb68[iVar1 + 1] = 1.0f / (2.0f - FLOAT_ARRAY_0049cb68[iVar1]);

	do {
		v2 = this->pSamples[uVar4];
		local_10 = this->pSplineCoefficients[uVar4];
		edF32Vector4SubHard(edF32VECTOR4_ARRAY_0049cb90, v2 + 1, v2);
		edF32Vector4ScaleHard(FLOAT_ARRAY_0049cb68[2] * 3.0f, edF32VECTOR4_ARRAY_0049cb90, edF32VECTOR4_ARRAY_0049cb90);
		iVar1 = this->field_0x14;
		uVar5 = 1;
		if (1 < iVar1 - 1U) {
			peVar7 = edF32VECTOR4_ARRAY_0049cb90 + 1;
			pfVar2 = FLOAT_ARRAY_0049cb68 + 3;
			peVar3 = peVar7;
			do {
				edF32Vector4SubHard(peVar7, v2 + uVar5 + 1, v2 + (uVar5 - 1));
				edF32Vector4ScaleHard(3.0f, peVar7, peVar3);
				edF32Vector4SubHard(peVar7, peVar3, edF32VECTOR4_ARRAY_0049cb90 + uVar5 - 1);
				edF32Vector4ScaleHard(*pfVar2, peVar7, peVar3);
				iVar1 = this->field_0x14;
				uVar5 = uVar5 + 1;
				peVar7 = peVar7 + 1;
				peVar3 = peVar3 + 1;
				pfVar2 = pfVar2 + 1;
			} while (uVar5 < iVar1 - 1U);
		}

		edF32Vector4SubHard(edF32VECTOR4_ARRAY_0049cb90 + iVar1 + -1, v2 + iVar1 + -1, v2 + iVar1 + -2);
		edF32Vector4ScaleHard(3.0f, edF32VECTOR4_ARRAY_0049cb90 + this->field_0x14 + -1, edF32VECTOR4_ARRAY_0049cb90 + this->field_0x14 + -1);
		edF32Vector4SubHard(edF32VECTOR4_ARRAY_0049cb90 + this->field_0x14 + -1, edF32VECTOR4_ARRAY_0049cb90 + this->field_0x14 + -1, edF32VECTOR4_ARRAY_0049cb90 + this->field_0x14 + -2);
		edF32Vector4ScaleHard(FLOAT_ARRAY_0049cb68[this->field_0x14 + 1], edF32VECTOR4_ARRAY_0049cb90 + this->field_0x14 + -1, edF32VECTOR4_ARRAY_0049cb90 + this->field_0x14 + -1);
		iVar1 = this->field_0x14 + -1;

		edF32VECTOR4_ARRAY_0049cc10[iVar1] = edF32VECTOR4_ARRAY_0049cb90[iVar1];
		iVar1 = this->field_0x14 + -2;
		if (-1 < iVar1) {
			v0 = edF32VECTOR4_ARRAY_0049cc10 + iVar1;
			peVar7 = edF32VECTOR4_ARRAY_0049cb90 + iVar1;
			pfVar2 = FLOAT_ARRAY_0049cb68 + this->field_0x14;
			peVar3 = v0;
			do {
				edF32Vector4ScaleHard(*pfVar2, v0, edF32VECTOR4_ARRAY_0049cc10 + 1 + iVar1);
				edF32Vector4SubHard(v0, peVar7, peVar3);
				iVar1 = iVar1 + -1;
				pfVar2 = pfVar2 + -1;
				v0 = v0 + -1;
				peVar3 = peVar3 + -1;
				peVar7 = peVar7 + -1;
			} while (-1 < iVar1);
		}

		uVar5 = 0;
		if (this->field_0x14 != 1) {
			peVar3 = edF32VECTOR4_ARRAY_0049cc10;
			do {
				fVar8 = v2[1].x;
				uVar5 = uVar5 + 1;
				fVar11 = v2->x;
				fVar9 = peVar3->x;
				fVar10 = peVar3[1].x;
				local_10->x = fVar11;
				local_10->y = fVar9;
				local_10->z = ((fVar8 - fVar11) * 3.0f - fVar9 * 2.0f) - fVar10;
				local_10->w = fVar10 + fVar9 + (fVar11 - fVar8) * 2.0f;
				fVar8 = v2[1].y;
				fVar11 = v2->y;
				fVar9 = peVar3->y;
				fVar10 = peVar3[1].y;
				local_10[1].x = fVar11;
				local_10[1].y = fVar9;
				local_10[1].z = ((fVar8 - fVar11) * 3.0f - fVar9 * 2.0f) - fVar10;
				local_10[1].w = fVar10 + fVar9 + (fVar11 - fVar8) * 2.0f;
				fVar8 = v2[1].z;
				fVar11 = v2->z;
				fVar9 = peVar3->z;
				fVar10 = peVar3[1].z;
				v2 = v2 + 1;
				local_10[2].x = fVar11;
				local_10[2].y = fVar9;
				local_10[2].z = ((fVar8 - fVar11) * 3.0f - fVar9 * 2.0f) - fVar10;
				local_10[2].w = fVar10 + fVar9 + (fVar11 - fVar8) * 2.0f;
				local_10 = local_10 + 3;
				peVar3 = peVar3 + 1;
			} while (uVar5 < this->field_0x14 - 1);
		}

		uVar4 = uVar4 + 1;
	} while (uVar4 < 2);

	return;
}

void PlayerSubStruct_64::Draw()
{
	bool bVar1;
	uint nbVertex;
	byte bVar2;
	edF32VECTOR4* peVar3;
	float* pfVar4;
	uint uVar5;
	uint uVar6;
	edF32VECTOR4* peVar7;
	byte bVar8;
	float fVar9;
	float fVar10;

	if (((this->flags & 2) != 0) && ((this->flags & 4) != 0)) {
		peVar7 = edF32VECTOR4_ARRAY_0049cc10;
		nbVertex = GetNbVertex(edF32VECTOR4_ARRAY_0049cc10);
		bVar1 = GameDList_BeginCurrent();
		if (bVar1 != false) {
			edDListLoadIdentity();
			edDListUseMaterial(this->pMaterial);
			edDListBlendSet(1);
			edDListBegin(0.0f, 0.0f, 0.0f, 4, nbVertex);
			uVar6 = 0;
			if ((this->flags & 0x20) == 0) {
				fVar9 = this->field_0x38 * static_cast<float>(this->field_0x44.a);
				if (fVar9 < 2.147484e+09f) {
					bVar8 = (byte)static_cast<int>(fVar9);
					bVar2 = (this->field_0x44).r;
				}
				else {
					bVar8 = (byte)static_cast<int>(fVar9 - 2.147484e+09f);
					bVar2 = (this->field_0x44).r;
				}

				edDListColor4u8(bVar2, (this->field_0x44).g, (this->field_0x44).b, bVar8);
			}

			if ((this->flags & 0x40) == 0) {
				edDListTexCoo2f(0.0f, 0.0f);
			}

			while (uVar6 < nbVertex) {
				uVar5 = 0;
				pfVar4 = &edF32VECTOR4_ARRAY_0049cb90[nbVertex + 6].w;
				peVar3 = peVar7;
				do {
					fVar9 = *pfVar4;
					if ((this->flags & 0x20) != 0) {
						fVar10 = peVar3->w / fVar9;
						fVar10 = (1.0f - fVar10 * fVar10) * this->field_0x38 * static_cast<float>(this->field_0x44.a);
						if (fVar10 < 2.147484e+09f) {
							bVar8 = (byte)static_cast<int>(fVar10);
							bVar2 = (this->field_0x44).r;
						}
						else {
							bVar8 = (byte)static_cast<int>(fVar10 - 2.147484e+09f);
							bVar2 = (this->field_0x44).r;
						}

						edDListColor4u8(bVar2, (this->field_0x44).g, (this->field_0x44).b, bVar8);
					}

					if ((this->flags & 0x40) != 0) {
						if (static_cast<int>(uVar5) < 0) {
							fVar10 = static_cast<float>(uVar5 >> 1 | uVar5 & 1);
							fVar10 = fVar10 + fVar10;
						}
						else {
							fVar10 = static_cast<float>(uVar5);
						}

						edDListTexCoo2f(fVar10, peVar3->w / fVar9);
					}

					edDListVertex4f(peVar3->x, peVar3->y, peVar3->z, 0.0f);

					uVar5 = uVar5 + 1;
					uVar6 = uVar6 + 1;
					pfVar4 = pfVar4 + 4;
					peVar3 = peVar3 + 1;
				} while (uVar5 < 2);
				peVar7 = peVar7 + 2;
			}

			edDListEnd();
			GameDList_EndCurrent();
		}
	}

	return;
}

int PlayerSubStruct_64::GetNbVertex(edF32VECTOR4* pData)
{
	uint uVar1;
	bool bVar2;
	edF32VECTOR4* peVar3;
	uint uVar5;
	edF32VECTOR4* peVar6;
	uint uVar7;
	edF32VECTOR4* v2;
	edF32VECTOR4* v1;
	int iVar8;
	uint uVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	edF32VECTOR4* local_30;
	edF32VECTOR4** local_20;
	edF32VECTOR4 eStack16;

	uVar9 = 0;
	iVar8 = 0;
	local_30 = pData;
	local_20 = this->pSplineCoefficients;
	do {
		fVar14 = 1.0f;
		fVar12 = 0.0f;
		v2 = (edF32VECTOR4*)0x0;
		uVar7 = 0;
		bVar2 = false;
		peVar6 = *local_20;
		fVar13 = 0.0f;
		v1 = local_30;
		while ((uVar7 < this->field_0x14 - 1 && (!bVar2))) {
			uVar5 = 1;
			peVar3 = v1;
			if (uVar7 == 0) {
				uVar5 = 0;
			}
			else {
				if (uVar7 == this->field_0x14 - 2) {
					if (this->field_0x24 == 0.0f) {
						fVar14 = 1.0f - this->field_0x20;
					}
					else {
						fVar14 = 1.0f - (GetTimer()->scaledTotalTime - this->field_0x20) / this->field_0x24;
					}
				}
			}
			while ((v1 = peVar3, uVar1 = this->field_0x28, uVar5 <= uVar1 && (!bVar2))) {
				if ((int)uVar5 < 0) {
					fVar10 = (float)(uVar5 >> 1 | uVar5 & 1);
					fVar10 = fVar10 + fVar10;
				}
				else {
					fVar10 = (float)uVar5;
				}
				if ((int)uVar1 < 0) {
					fVar11 = (float)(uVar1 >> 1 | uVar1 & 1);
					fVar11 = fVar11 + fVar11;
				}
				else {
					fVar11 = (float)uVar1;
				}
				fVar10 = fVar10 / fVar11;
				if (fVar14 < fVar10) {
					bVar2 = true;
					fVar10 = fVar14;
				}

				v1->x = peVar6->x + fVar10 * (peVar6->y + fVar10 * (peVar6->z + peVar6->w * fVar10));
				v1->y = peVar6[1].x + fVar10 * (peVar6[1].y + fVar10 * (peVar6[1].z + peVar6[1].w * fVar10));
				v1->z = peVar6[2].x + fVar10 * (peVar6[2].y + fVar10 * (peVar6[2].z + peVar6[2].w * fVar10));
				v1->w = 0.0f;
				if (v2 != (edF32VECTOR4*)0x0) {
					edF32Vector4SubHard(&eStack16, v1, v2);
					fVar13 = edF32Vector4GetDistHard(&eStack16);
				}

				fVar12 = fVar12 + fVar13;
				uVar5 = uVar5 + 1;
				iVar8 = iVar8 + 1;
				v1->w = fVar12;
				peVar3 = v1 + 2;
				v2 = v1;
			}

			peVar6 = peVar6 + 3;
			uVar7 = uVar7 + 1;
		}

		uVar9 = uVar9 + 1;
		local_20 = local_20 + 1;
		local_30 = local_30 + 1;
	} while (uVar9 < 2);

	return iVar8;
}

void pCallbackSlaveKick(CActorFighter* pFighter, int param_2)

{
	CBehaviourFighter* pBehaviourFighter;
	s_fighter_action_param local_10;
	s_fighter_action local_4;

	if ((param_2 == 1) && ((pFighter->fightFlags & 0x40) != 0)) {
		IMPLEMENTATION_GUARD(
		local_4 = (s_fighter_action)0x100;
		local_10.field_0x4 = CActorFighter::FindBlowByName(pFighter, s_SLAVE_KICK_SALTO_004342f0);
		local_10.field_0x0 = (edF32VECTOR4*)0x0;
		if (local_10.field_0x4 != (s_fighter_blow*)0x0) {
			pBehaviourFighter = static_cast<CBehaviourFighter*>(pFighter)->GetBehaviour(pFighter->curBehaviourId);
			Conditional_Execute(&local_4, &local_10);
		})
	}

	return;
}

void CBehaviourFighterSlave::Init(CActor* pOwner)
{
	s_fighter_blow* pSlaveKick;

	pSlaveKick = static_cast<CActorFighter*>(pOwner)->FindBlowByName("SLAVE_KICK");
	if (pSlaveKick != (s_fighter_blow*)0x0) {
		pSlaveKick->field_0xd0 = pCallbackSlaveKick;
	}

	return;
}

void CBehaviourFighterSlave::Term()
{
	return;
}

void CBehaviourFighterSlave::Manage()
{
	return;
}

void CBehaviourFighterSlave::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CActorFighter* pFighter;
	CInputAnalyser* this_00;
	float fVar2;
	float fVar3;
	CLifeInterface* this_01;
	float fVar4;
	s_fighter_slave_traj_in local_40;

	this->pOwner = static_cast<CActorFighter*>(pOwner);
	pFighter = this->pOwner;
	pFighter->field_0x354 = pFighter->pAdversary;
	(this->pOwner)->pAdversary = (CActorFighter*)0x0;
	pFighter = this->pOwner;
	(pFighter->pCollisionData)->actorFieldA = pFighter->field_0x354;
	(this->pOwner)->field_0x474 = 0.0f;
	fVar3 = gF32Vector4Zero.w;
	fVar2 = gF32Vector4Zero.z;
	fVar4 = gF32Vector4Zero.y;
	pFighter = this->pOwner;
	(pFighter->field_0x4e0).x = gF32Vector4Zero.x;
	(pFighter->field_0x4e0).y = fVar4;
	(pFighter->field_0x4e0).z = fVar2;
	(pFighter->field_0x4e0).w = fVar3;
	this->pOwner->field_0xa78 = 3;
	this->pOwner->scalarDynForward.Reset();
	this->pOwner->scalarDynLateral.Reset();
	this->pOwner->scalarDynJump.Reset();
	pFighter = this->pOwner;
	pFighter->fightFlags = pFighter->fightFlags & 0xfffff8ff;
	(this->pOwner)->nextActionType = NEXT_ACTION_TYPE_NONE;
	(this->pOwner)->pNextAction.pBlow = (s_fighter_blow*)0x0;
	this->pOwner->SetFighterCombo((s_fighter_combo*)0x0);

	this_00 = (this->pOwner)->pInputAnalyser;
	if (this_00 != (CInputAnalyser*)0x0) {
		*this_00 = CInputAnalyser();
	}

	if (newState == -1) {
		pFighter = this->pOwner;
		pFighter->SetState(0x3b, -1);
	}
	else {
		pFighter = this->pOwner;
		pFighter->SetState(newState, newAnimationType);
	}

	this->behaviourId = this->pOwner->prevBehaviourId;
	pFighter = this->pOwner;
	pFighter->fightFlags = pFighter->fightFlags | 0x8000;
	pFighter->fightFlags = pFighter->fightFlags & 0xfffeffff;
	this->pOwner->SetLookingAtOff();

	fVar4 = edF32Vector4GetDistHard(&((this->pOwner)->fighterAnatomyZones).field_0x10);
	local_40.field_0x0 = -3.590392f;
	local_40.field_0x4 = 3.590392f;
	local_40.field_0x8 = 9.189158f;
	local_40.field_0xc = 11.30973f;
	local_40.field_0x10 = 0.0f;
	local_40.field_0x14 = 1.795196f;
	local_40.field_0x18 = fVar4 + 0.65f;
	local_40.field_0x1c = fVar4 + 0.25f;
	local_40.field_0x20 = 0.0f;
	local_40.field_0x24 = 0.3f;
	local_40.field_0x28 = 1.570796f;
	local_40.field_0x2c = 1.570796f;
	local_40.field_0x34 = 0.5235988f;
	local_40.field_0x30 = 0.0f;
	this->pOwner->field_0x920.SetConfig(&local_40);

	local_40.field_0x0 = -3.590392f;
	local_40.field_0x4 = 3.590392f;
	local_40.field_0x8 = 11.30973f;
	local_40.field_0xc = 14.13717f;
	local_40.field_0x20 = 0.3f;
	local_40.field_0x24 = 0.69999999f;
	local_40.field_0x28 = 1.570796f;
	local_40.field_0x2c = 1.308997f;
	this->pOwner->field_0x980.SetConfig(&local_40);

	this->pOwner->slaveGroundSampler.field_0x4 = 0x18;
	this->pOwner->slaveGroundSampler.field_0x8 = 4.0f;

	(this->pOwner)->field_0xa5c = 0.0f;
	(this->pOwner)->field_0xa58 = 0.0f;
	(this->pOwner)->field_0xa54 = 0;
	(this->pOwner)->field_0xa60 = 1.0f;

	pFighter = this->pOwner;
	this_01 = pFighter->GetLifeInterfaceOther();
	this_01->SetPriority(4);

	return;
}

void CBehaviourFighterSlave::End(int newBehaviourId)
{
	CActorFighter* pFighter;
	bool bVar2;
	CLifeInterface* pCVar3;
	float fVar4;

	pFighter = this->pOwner;
	bVar2 = pFighter->IsFightRelated(newBehaviourId);
	if (bVar2 == false) {
		pFighter = this->pOwner;
		pCVar3 = pFighter->GetLifeInterface();
		fVar4 = pCVar3->GetValue();
		if (fVar4 <= 0.0f) {
			pFighter = this->pOwner;
			pFighter->NotifyMaster(0, 0);
		}
		else {
			pFighter = this->pOwner;
			pFighter->NotifyMaster(0xb, 0);
		}
	}

	this->pOwner->pCollisionData->actorFieldA = (CActor*)0x0;
	pFighter = this->pOwner;
	pFighter->fightFlags = pFighter->fightFlags & 0xffff7fff;
	pFighter->fightFlags = pFighter->fightFlags | 0x10000;
	pFighter = this->pOwner;
	pFighter->SetAdversary(pFighter->field_0x354);
	(this->pOwner)->field_0x354 = (CActorFighter*)0x0;
	pFighter = this->pOwner;
	pFighter->flags = pFighter->flags & 0xffdfffff;
	this->pOwner->ComputeAltitude();

	return;
}

void CBehaviourFighterSlave::InitState(int newState)
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;
	CCollision* pCol;
	CActorFighter* pFighter;

	pFighter = this->pOwner;
	if (newState == -1) {
		uVar3 = 0;
	}
	else {
		pSVar2 = pFighter->GetStateCfg(newState);
		uVar3 = pSVar2->flags_0x4;
	}
	if ((uVar3 & 0xff800) == 0x40000) {
		pFighter = this->pOwner;

		if ((pFighter->GetStateFlags(pFighter->prevActorState) & 0xff800) != 0x40000) {
			pFighter = this->pOwner;
			pFighter->NotifyMaster(10, 0);
			pFighter->LinkToActor(pFighter->field_0x354, 0x6b6e694c, 1);
		}
	}
	else {
		if ((uVar3 & 0xff800) == 0x20000) {
			pFighter = this->pOwner;

			if ((pFighter->GetStateFlags(pFighter->prevActorState) & 0xff800) != 0x20000) {
				pFighter = this->pOwner;
				pFighter->field_0x354->pAnimationController->RegisterBone(0x6b6e694c);
				pFighter->SetLocalBoundingSphere(pFighter->field_0x3f8 * 1.5f, &(pFighter->subObjA)->boundingSphere);
			}
		}
	}

	pFighter = this->pOwner;
	iVar1 = pFighter->actorState;
	if ((iVar1 == 0x4e) || (iVar1 == 0x4b)) {
		IMPLEMENTATION_GUARD(
		pFighter->_StateFighterReturnFromFightActionInit();)
	}
	else {
		if ((iVar1 == 0x4d) || (iVar1 == 0x4a)) {
			IMPLEMENTATION_GUARD(
			pFighter->_StateFighterExecuteFightActionInit();)
		}
		else {
			if ((iVar1 == 0x4c) || (iVar1 == 0x49)) {
				IMPLEMENTATION_GUARD(
				pFighter->_StateFighterPrepareFightActionInit();)
			}
			else {
				if ((iVar1 == 0x43) || (iVar1 == 0x48)) {
					pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(5.0f, 0);
				}
				else {
					if (iVar1 == 0x47) {
						pFighter->scalarDynForward.Reset();
						pFighter->scalarDynLateral.Reset();
						pFighter->scalarDynJump.Reset();
						pFighter->dynamic.speed = 0.0f;
					}
					else {
						if (iVar1 == 0x44) {
							pFighter->dynamic.speed = 0.0f;
							pFighter->field_0xa78 = 0;
						}
						else {
							if (iVar1 == 0x3e) {
								pFighter->dynamic.speed = 0.0f;
								pFighter->fightFlags = pFighter->fightFlags & 0xfffff7ff;
								pFighter->field_0xa78 = 2;
								pFighter->NotifyMaster(7, 0);
							}
							else {
								if (iVar1 == 0x3d) {
									if ((pFighter->flags & 0x1000) == 0) {
										GetAnglesFromVector((edF32VECTOR3*)&pFighter->rotationEuler, &pFighter->rotationQuat);
										pFighter->flags = pFighter->flags | 0x1000;
									}
									pCol = pFighter->pCollisionData;
									pCol->flags_0x0 = pCol->flags_0x0 | 0x800;
									pCol = pFighter->pCollisionData;
									pCol->flags_0x0 = pCol->flags_0x0 & 0xffffefd7;
									(pFighter->field_0x4e0).z = 0.0f;
									(pFighter->field_0x4e0).y = 0.0f;
									(pFighter->field_0x4e0).x = 0.0f;
									(pFighter->field_0x4e0).w = 1.0f;
								}
								else {
									if (iVar1 == 0x3b) {
										if (pFighter->field_0x474 == 0.0f) {
											pFighter->fightFlags = pFighter->fightFlags & 0xfffffffe;
										}
										else {
											pFighter->fightFlags = pFighter->fightFlags | 1;
										}

										pFighter->field_0x44cuint = 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

void CBehaviourFighterSlave::TermState(int oldState, int newState)
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;
	float fVar5;
	float fVar6;
	CAnimation* pAnim;
	CCollision* pCol;
	CActorFighter* pFighter;

	pFighter = this->pOwner;
	iVar1 = pFighter->actorState;
	if ((iVar1 == 0x4e) || (iVar1 == 0x4b)) {
		IMPLEMENTATION_GUARD(
		pFighter->_StateFighterReturnFromFightActionTerm(newState);)
	}
	else {
		if ((iVar1 == 0x4d) || (iVar1 == 0x4a)) {
			IMPLEMENTATION_GUARD(
			pFighter->_StateFighterExecuteFightActionTerm(newState);)
		}
		else {
			if ((iVar1 == 0x4c) || (iVar1 == 0x49)) {
				IMPLEMENTATION_GUARD(
				pFighter->_StateFighterPrepareFightActionTerm(newState);)
			}
			else {
				if ((iVar1 == 0x43) || (iVar1 == 0x48)) {
					pFighter->field_0x474 = 0.3f;
					if (newState == -1) {
						pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
					}
					else {
						pAnim = pFighter->pAnimationController;
						pSVar2 = pFighter->GetStateCfg(newState);
						iVar1 = pFighter->GetIdMacroAnim(pSVar2->animId);
						if (iVar1 < 0) {
							fVar5 = 0.0f;
						}
						else {
							fVar5 = pAnim->GetAnimLength(iVar1, 1);
							fVar6 = pAnim->GetAnimLength(iVar1, 2);
							fVar5 = fVar5 - fVar6;
						}

						pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar5 / pFighter->field_0x474, 0);
					}
				}
				else {
					if (iVar1 == 0x45) {
						pFighter->NotifyMaster(6, 0);
					}
					else {
						if (iVar1 == 0x3d) {
							pCol = pFighter->pCollisionData;
							pCol->flags_0x0 = pCol->flags_0x0 & 0xfffff7ff;
							pCol = pFighter->pCollisionData;
							pCol->flags_0x0 = pCol->flags_0x0 | 0x1028;
						}
						else {
							if (iVar1 == 0x3b) {
								pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
								pFighter->fightFlags = pFighter->fightFlags & 0xfffffffe;
							}
						}
					}
				}
			}
		}
	}

	pFighter = this->pOwner;

	if ((this->pOwner->GetStateFlags(oldState) & 0xff800) == 0x40000) {
		pFighter = this->pOwner;

		if ((this->pOwner->GetStateFlags(newState) & 0xff800) != 0x40000) {
			this->pOwner->UnlinkFromActor();
		}
	}
	else {
		if ((uVar3 & 0xff800) == 0x20000) {
			pFighter = this->pOwner;

			if ((this->pOwner->GetStateFlags(newState) & 0xff800) != 0x20000) {
				pFighter = this->pOwner;
				pFighter->field_0x354->pAnimationController->UnRegisterBone(0x6b6e694c);
				pFighter->SetLocalBoundingSphere(pFighter->field_0x3f8, &(pFighter->subObjA)->boundingSphere);
			}
		}
	}

	uVar3 = this->pOwner->FUN_0031b4d0(oldState);
	if (((uVar3 & 2) != 0) && (uVar3 = this->pOwner->FUN_0031b4d0(newState), (uVar3 & 2) == 0)) {
		(this->pOwner)->field_0xa78 = 3;
	}

	return;
}

int CBehaviourFighterSlave::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	uint uVar2;
	StateConfig* pSVar3;
	int iVar4;
	long lVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_90;
	s_fighter_anatomy_zones sStack128;
	CActor* local_50[4];
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	int local_20;
	undefined4 local_8;
	CActor** local_4;
	CCollision* pCol;
	CActorFighter* pFighter;

	bVar1 = true;
	if ((msg == 0x16) || (msg == 0x17)) {
		local_8 = 0;
		pFighter = this->pOwner;
		pFighter->NotifyMaster(2, 0);
		return 0;
	}

	if (msg != MESSAGE_KICKED) {
		return 0;
	}

	_msg_hit_param* pHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);

	iVar6 = pHitParam->projectileType;
	if (iVar6 != 7) {
		if (iVar6 != 9) {
			if (iVar6 != 8) {
				return 1;
			}

			pFighter = this->pOwner;
			if (static_cast<CActorFighter*>(pSender) != pFighter->field_0x354) {
				return 1;
			}

			pFighter->field_0x6a0 = pHitParam->field_0x20;
			(this->pOwner)->field_0x6b0 = pHitParam->field_0x30;
			pFighter = this->pOwner;
			pFighter->SetBehaviour(FIGHTER_BEHAVIOUR_DEFAULT, 0x50, -1);

			return 1;
		}

		bVar1 = false;
	}

	pFighter = this->pOwner;
	if (static_cast<CActorFighter*>(pSender) == pFighter->field_0x354) {
		pFighter->hitFlags = pHitParam->flags;
		(this->pOwner)->hitDamage = pHitParam->damage * 2.0f;

		pFighter = this->pOwner;
		pFighter->field_0x690 = pHitParam->field_0x40;

		pFighter = this->pOwner;
		pFighter->field_0x6a0 = pHitParam->field_0x20;

		(this->pOwner)->field_0x6b0 = pHitParam->field_0x30;
		(this->pOwner)->field_0x684 = pHitParam->field_0x50;
		(this->pOwner)->field_0x686 = pHitParam->field_0x52;

		pFighter = this->pOwner;
		if ((pFighter->field_0x684 & 1U) != 0) {
			pFighter->field_0x7a0 = pHitParam->field_0x60;
			(this->pOwner)->field_0x7b4 = pHitParam->field_0x70;
		}

		pFighter = this->pOwner;
		bVar1 = pFighter->actorsExcludeTable.IsInList(pFighter->field_0x354);
		if (bVar1 == false) {
			pFighter = this->pOwner;
			pFighter->actorsExcludeTable.Add(1.0f, pFighter->field_0x354);
		}

		pFighter = this->pOwner;
		pFighter->SetBehaviour(FIGHTER_BEHAVIOUR_PROJECTED, -1, -1);
		return 1;
	}

	if (pFighter->field_0xa78 != 3) {
		edF32Vector4ScaleHard(1.5f, (edF32VECTOR4*)&sStack128, (edF32VECTOR4*)&pFighter->fighterAnatomyZones);
		sStack128.field_0x20 = 1.0f;
		sStack128.field_0x24 = 0.999f;
		pCol = this->pOwner->pCollisionData;
		local_90.x = (pCol->transformMatrix).da;
		local_90.y = (pCol->transformMatrix).db;
		local_90.z = (pCol->transformMatrix).dc;
		local_90.w = (pCol->transformMatrix).dd;
		uVar2 = this->pOwner->_SV_HIT_GetHitZoneFromImpact(&sStack128, &pHitParam->field_0x40, &local_90);
		pFighter = this->pOwner;
		if (pFighter->field_0xa78 == (uVar2 & 0xffff)) {
			pSender->DoMessage(pFighter->field_0x354, MESSAGE_KICKED, pHitParam);
			return 1;
		}
	}

	pFighter->LifeDecrease(pHitParam->damage);

	local_40 = pHitParam->field_0x20;
	local_30 = pHitParam->field_0x40;

	pFighter = this->pOwner;
	iVar6 = pFighter->actorState;
	local_50[0] = pSender;
	if (iVar6 == -1) {
		uVar2 = 0;
	}
	else {
		pSVar3 = pFighter->GetStateCfg(iVar6);
		uVar2 = pSVar3->flags_0x4;
	}

	iVar6 = 0x43;
	if ((uVar2 & 0xff800) == 0x20000) {
		uVar2 = 1;
	}
	else {
		pFighter = this->pOwner;
		iVar6 = 0x48;
		uVar2 = pFighter->_SV_HIT_GetHitZoneFromImpact(&pFighter->fighterAnatomyZones, &local_30, &pFighter->currentLocation);
	}
	pFighter = this->pOwner;
	iVar4 = pFighter->_SV_ANM_GetMultiWaysAnim3D(pFighter->field_0x6d0 + uVar2 + 6, &local_40, (edF32VECTOR4*)0x0);
	pFighter = this->pOwner;
	local_20 = pFighter->_SV_ANM_GetMultiWaysAnim3D(pFighter->field_0x6d0 + uVar2 + 3, &local_40, (edF32VECTOR4*)0x0);
	if (bVar1) {
		this->pOwner->PlayImpactFx(&local_30, &local_40, static_cast<uint>((pHitParam->flags & 1) != 0), false);
	}

	local_4 = local_50;
	pFighter = this->pOwner;
	if (pFighter->NotifyMaster(1, local_4) != 0) {
		pFighter = this->pOwner;
		pFighter->SetState(iVar6, iVar4);
	}

	return 1;
}

bool CBehaviourFighterSlave::Execute(s_fighter_action* pAction, s_fighter_action_param* pParam)
{
	uint uVar1;
	s_fighter_blow* psVar2;
	s_fighter_blow* psVar3;
	ulong uVar4;
	bool bVar5;
	s_fighter_action local_4;
	CActorFighter* pFighter;

	pFighter = this->pOwner;
	uVar1 = pFighter->fightFlags;
	bVar5 = false;
	if (((uVar1 & 0x400) == 0) && ((uVar1 & 1) == 0)) {
		pFighter->_ValidateCommand(pAction, &local_4);
		uVar4 = (static_cast<ulong>(local_4.moveByte) << 0x38) >> 0x3c;
		if (uVar4 == 4) {
			pFighter = this->pOwner;
			pFighter->NotifyMaster(7, (void*)0x0);
			pFighter = this->pOwner;
			pFighter->SetState(0x3e, -1);
			bVar5 = true;
			(this->pOwner)->field_0x474 = 0.0f;
		}
		else {
			if ((uVar4 == 8) || (uVar4 == 2)) {
				pFighter = this->pOwner;
				pFighter->NotifyMaster(5, (void*)0x0);
				pFighter = this->pOwner;
				pFighter->SetState(0x44, -1);
				bVar5 = true;
				(this->pOwner)->field_0x474 = 0.0f;
			}
		}

		if (((local_4.actionByte & 0xf) == 2) || ((local_4.actionByte & 0xf) == 1)) {
			pFighter = this->pOwner;
			if (pFighter->actorState == 0x4a) {
				psVar3 = static_cast<s_fighter_blow*>(pParam->pData);
				psVar2 = pFighter->FindBlowByName("SLAVE_KICK_SALTO");
				if (psVar3 == psVar2) {
					(this->pOwner)->pNextAction.pBlow = psVar3;
					(this->pOwner)->nextActionType = NEXT_ACTION_TYPE_BLOW;
				}
			}
			else {
				psVar3 = pFighter->FindBlowByName("SLAVE_KICK");
				if (psVar3 != (s_fighter_blow*)0x0) {
					(this->pOwner)->pBlow = psVar3;
					pFighter = this->pOwner;
					pFighter->field_0x474 = pFighter->pBlow->field_0xc;
					pFighter = this->pOwner;
					pFighter->NotifyMaster(9, (void*)0x0);
					pFighter = this->pOwner;
					IMPLEMENTATION_GUARD(
					pFighter->SetState(0x49, (int)*static_cast<undefined**>((int)(((CBehaviourStand*)&pFighter->pBlow)->pVTable + 1) + 0x24));)
					bVar5 = true;
				}
			}
		}
	}

	return bVar5;
}

void CActorFighter::_StateFighterMasterSaidDownRotate()
{
	bool bVar1;
	s_fighter_slave_traj_out* peVar2;
	int iVar4;
	CLifeInterface* pLife;
	ushort uVar5;
	ushort uVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float puVar14;
	float puVar11;
	float puVar12;
	float fVar10;
	float puVar13;
	float fVar11;
	edF32VECTOR4 local_290;
	CActorsTable local_280;
	edF32VECTOR4 eStack368;
	edF32MATRIX4 eStack352;
	edF32MATRIX4 auStack288;
	edF32VECTOR4 local_e0;
	edF32MATRIX4 auStack208;
	edF32VECTOR4 eStack144;
	S_FIGHTER_SLAVE_GROUND_SAMPLE local_80;
	s_fighter_slave_traj_out local_60;
	s_fighter_slave_traj_in local_40;
	CActorFighter* pFighter;

	local_280.nbEntries = 0;

	fVar11 = fabsf(this->field_0xa5c);
	if (fVar11 < 11.30973f) {
		fVar7 = this->field_0xa58 + this->field_0xa5c * GetTimer()->cutsceneDeltaTime;
		this->field_0xa58 = fVar7;
		fVar7 = edF32Between_2Pi(fVar7);
		this->field_0xa58 = fVar7;
		if (fVar7 * this->field_0xa5c < 0.0f) {
			if (0.0f < this->field_0xa60) {
				this->field_0xa60 = -1.0f;
				memcpy(&local_40, &this->field_0x920.config, sizeof(s_fighter_slave_traj_in));
				local_40.field_0x0 = local_40.field_0x0 / 2.0f;
				local_40.field_0x4 = local_40.field_0x4 / 2.0f;
				this->field_0x920.SetConfigAdjust(&local_40, &this->field_0xa58, (float*)0x0);
			}
		}
		else {
			if (this->field_0xa60 < 0.0f) {
				this->field_0xa60 = 1.0f;
				memcpy(&local_40, &this->field_0x920.config, sizeof(s_fighter_slave_traj_in));
				local_40.field_0x0 = local_40.field_0x0 * 2.0f;
				local_40.field_0x4 = local_40.field_0x4 * 2.0f;
				this->field_0x920.SetConfigAdjust(&local_40, &this->field_0xa58, (float*)0x0);
			}
		}

		fVar7 = (this->field_0x920).config.field_0x4;
		if (fVar7 < this->field_0xa58) {
			this->field_0xa58 = fVar7;
		}
		else {
			fVar7 = (this->field_0x920).config.field_0x0;
			if (this->field_0xa58 < fVar7) {
				this->field_0xa58 = fVar7;
			}
		}

		peVar2 = this->field_0x920.Compute(-this->field_0xa58, fVar11);
		memcpy(&local_60, peVar2, 0x20);
	}
	else {
		puVar13 = 0.0f;
		if (this->field_0xa58 != 0.0f) {
			if (0.0f < this->field_0xa58) {
				puVar13 = 1.0f;
			}
			else {
				puVar13 = -1.0f;
			}
		}

		iVar4 = 1;
		if (this->field_0xa58 <= 0.0f) {
			iVar4 = -1;
		}

		if (puVar13 == static_cast<float>(iVar4)) {
			this->field_0xa58 = this->field_0xa58 + ((-puVar13 * 0.5235988f) / 0.25f) * GetTimer()->cutsceneDeltaTime;
		}
		else {
			this->field_0xa58 = 0.0f;
		}

		peVar2 = this->field_0x920.Compute(-this->field_0xa58, fVar11);
		memcpy(&local_60, peVar2, 0x20);
	}

	edF32Vector4AddHard(&local_e0, &this->currentLocation, &this->fighterAnatomyZones.field_0x0);
	local_e0.w = 1.0f;
	edF32Vector4SubHard(&eStack368, &local_e0, &this->field_0x354->currentLocation);
	fVar7 = edF32Vector4SafeNormalize0Hard(&eStack368, &eStack368);
	fVar8 = GetAngleYFromVector(&eStack368);
	fVar8 = edF32Between_0_2Pi(fVar8);
	this->slaveGroundSampler.GetGroundDesc(fVar8, fVar7 * 0.7692308f, &local_80, &this->field_0x354->currentLocation);
	if (local_60.field_0x0.y < local_80.field_0x10) {
		bVar1 = true;
		this->fightFlags = this->fightFlags & 0xfffffeff;
		local_60.field_0x0.y = local_80.field_0x10;
	}
	else {
		if ((local_60.field_0x0.y != 0.0f) || (0.0f <= local_80.field_0x10)) {
			bVar1 = false;
			this->fightFlags = this->fightFlags & 0xfffffeff;
			local_80.field_0x0 = gF32Vector4UnitY;
		}
		else {
			if (fabsf(eStack368.y) + ((this->pCollisionData)->pObbPrim->scale).z / local_80.field_0x0.y < fabsf(local_80.field_0x10)) {
				bVar1 = true;
				if ((this->fightFlags & 0x100) == 0) {
					this->dynamicExt.instanceIndex.y = 0.0f;
					this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x8000;
					this->fightFlags = this->fightFlags | 0x100;
				}
			}
			else {
				bVar1 = true;
				this->fightFlags = this->fightFlags & 0xfffffeff;
				local_60.field_0x0.y = local_80.field_0x10;
			}
		}
	}

	fVar7 = GetAngleYFromVector(&this->field_0x354->rotationQuat);
	edF32Matrix4RotateYHard(fVar7, &auStack288, &gF32Matrix4Unit);
	fVar7 = local_60.field_0x0.y;
	fVar10 = this->field_0x354->currentLocation.y;
	local_60.field_0x0.y = 0.0f;
	fVar9 = ((this->pCollisionData)->pObbPrim->scale).z / local_80.field_0x0.y;
	if ((this->fightFlags & 0x100) == 0) {
		this->dynamicExt.instanceIndex.y = (((fVar7 + fVar10) - local_e0.y) + fVar9) / GetTimer()->cutsceneDeltaTime;
		this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x8000;
	}

	edF32Matrix4MulF32Vector4Hard(&local_60.field_0x0, &auStack288, &local_60.field_0x0);
	edF32Vector4AddHard(&local_60.field_0x0, &local_60.field_0x0, &this->field_0x354->currentLocation);
	edF32Vector4SubHard(&local_60.field_0x0, &local_60.field_0x0, &local_e0);
	local_60.field_0x0.y = 0.0f;
	fVar7 = edF32Vector4SafeNormalize0Hard(&local_60.field_0x0, &local_60.field_0x0);
	this->dynamic.speed = fVar7 / GetTimer()->cutsceneDeltaTime;
	this->dynamic.rotationQuat = local_60.field_0x0;
	edF32Matrix4RotateYHard(fVar8, &auStack288, &gF32Matrix4Unit);
	fVar8 = GetTimer()->cutsceneDeltaTime * 6.283185f;
	fVar7 = local_60.field_0x10 - this->field_0xa54;
	if (fVar8 < fabsf(fVar7)) {
		if (0.0f <= fVar7) {
			puVar14 = 1.0f;
		}
		else {
			puVar14 = -1.0f;
		}

		this->field_0xa54 = this->field_0xa54 + fVar8 * puVar14;
	}
	else {
		this->field_0xa54 = local_60.field_0x10;
	}

	edF32Matrix4FromAngAxisSoft(-this->field_0xa54, &eStack352, &auStack288.rowX);
	edF32Matrix4MulF32Matrix4Hard(&auStack208, &auStack288, &eStack352);
	edF32Matrix4FromAngAxisSoft(-local_60.field_0x14, &eStack352, &auStack288.rowY);
	edF32Matrix4MulF32Matrix4Hard(&auStack208, &auStack208, &eStack352);

	if (bVar1) {
		edF32Vector4GetNegHard(&local_80.field_0x0, &local_80.field_0x0);
		edF32Vector4CrossProductHard(&eStack144, &auStack208.rowZ, &local_80.field_0x0);
		edF32Vector4SafeNormalize0Hard(&eStack144, &eStack144);
		puVar11 = edF32Vector4DotProductHard(&auStack208.rowZ, &local_80.field_0x0);
		if (1.0f < puVar11) {
			puVar12 = 1.0f;
		}
		else {
			puVar12 = -1.0f;
			if (-1.0f <= puVar11) {
				puVar12 = puVar11;
			}
		}

		fVar7 = acosf(puVar12);
		fVar7 = edF32Between_2Pi(fVar7);
		edF32Matrix4FromAngAxisSoft(-fVar7, &eStack352, &eStack144);
		edF32Matrix4MulF32Matrix4Hard(&auStack208, &auStack208, &eStack352);
	}

	_SV_DYN_SetRotationAroundMassCenter(&auStack208);
	ManageDyn(4.0f, 0x40020, &local_280);

	if (this->dynamic.speed != 0.0f) {
		if (fVar11 < 11.30973f) {
			local_290 = this->dynamic.rotationQuat;
			uVar6 = 2;
			uVar5 = 3;
		}
		else {
			local_290.x = this->dynamic.rotationQuat.x;
			local_290.y = this->dynamic.rotationQuat.y + 0.5f;
			local_290.z = this->dynamic.rotationQuat.z;
			local_290.w = 0.0f;
			edF32Vector4NormalizeHard(&local_290, &local_290);
			edF32Vector4ScaleHard(fVar11, &local_290, &local_290);
			uVar6 = 0xe;
			uVar5 = 1;
		}

		iVar4 = _SV_HIT_ProcessActorsCollisions(0.0f, &this->field_0x8f0, (edF32VECTOR4*)0x0, &local_290, &local_280, &this->actorsExcludeTable, true, uVar6, uVar5);
		if (iVar4 != 0) {
			pLife = GetLifeInterface();
			fVar11 = pLife->GetValueMax();
			LifeDecrease(fVar11 / 4.0f);
		}
	}

	static float FLOAT_00448b20 = 0.05f;

	if (this->field_0xa5c == 0.0f) {
		fVar11 = (this->field_0x4e0).w;
		if (1.0f - fVar11 <= FLOAT_00448b20) {
			(this->field_0x4e0).w = 1.0f;
		}
		else {
			(this->field_0x4e0).w = fVar11 + FLOAT_00448b20;
		}
	}
	else {
		fVar11 = (this->field_0x4e0).w;
		if (fVar11 <= FLOAT_00448b20) {
			(this->field_0x4e0).w = 0.0f;
		}
		else {
			(this->field_0x4e0).w = fVar11 - FLOAT_00448b20;
		}
	}

	(this->field_0x4e0).x = fabsf(this->field_0xa5c) / 14.13717f;
	fVar11 = (this->field_0x4e0).x;
	if (1.0f < fVar11) {
		(this->field_0x4e0).x = 1.0f;
	}
	else {
		if (fVar11 < 0.0f) {
			(this->field_0x4e0).x = 0.0f;
		}
	}

	fVar11 = (this->field_0x4e0).x;
	if (0.5f <= fVar11) {
		fVar11 = (fVar11 - 0.5f) * 2.0f;
		(this->field_0x4e0).z = fVar11;
		(this->field_0x4e0).y = 1.0f - fVar11;
		(this->field_0x4e0).x = 0.0f;
	}
	else {
		(this->field_0x4e0).y = fVar11 * 2.0f;
		(this->field_0x4e0).x = 1.0f - fVar11 * 2.0f;
		(this->field_0x4e0).z = 0.0f;
	}

	(this->field_0x4e0).x = (this->field_0x4e0).x * (1.0f - (this->field_0x4e0).w);
	(this->field_0x4e0).y = (this->field_0x4e0).y * (1.0f - (this->field_0x4e0).w);
	(this->field_0x4e0).z = (this->field_0x4e0).z * (1.0f - (this->field_0x4e0).w);
	pFighter = this->field_0x354;
	pFighter->field_0x4e0 = this->field_0x4e0;

	return;
}

void CBehaviourFighterSlave::ManageByMaster(s_fighter_action* pAction, s_fighter_action_param* pParam)
{
	CActorFighter* pCVar1;
	ed_3d_hierarchy_node* peVar2;
	bool bVar5;
	byte cVar6;
	edF32MATRIX4* peVar7;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 local_1a0;
	edF32MATRIX4 auStack400;
	edF32MATRIX4 auStack336;
	edF32MATRIX4 auStack272;
	edF32MATRIX4 auStack208;
	edF32MATRIX4 auStack144;
	edF32MATRIX4 auStack80;
	CActorFighter* pFighter;

	cVar6 = this->pOwner->UpdateFightCommand();
	if (cVar6 == 0) {
		ExecuteByMaster(pAction, pParam);
	}

	pFighter = this->pOwner;
	switch (pFighter->actorState) {
	case 0x3b:
		if ((pFighter->fightFlags & 1) == 0) {
			fVar9 = edFIntervalUnitSrcLERP(fabsf((pFighter->field_0x4e0).x), 1.0f, (pFighter->field_0x4e0).w);
			pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar9, 0);
		}
		else {
			if (pFighter->field_0x474 <= pFighter->timeInAir) {
				pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
				pFighter->fightFlags = pFighter->fightFlags & 0xfffffffe;
			}
		}

		pCVar1 = pFighter->field_0x354;
		pFighter->rotationQuat = pCVar1->rotationQuat;
		pFighter->ManageDyn(4.0f, 0x4022b, (CActorsTable*)0x0);
		break;
	case 0x3c:
		pCVar1 = pFighter->field_0x354;
		pFighter->rotationQuat = pCVar1->rotationQuat;
		pFighter->ManageDyn(4.0f, 0x4022b, (CActorsTable*)0x0);

		if (11.30973f <= fabsf(pFighter->field_0x354->field_0xa5c)) {
			pFighter->NotifyMaster(3, (void*)0x0);
			pFighter->fightFlags = pFighter->fightFlags | 0x400;
		}
		fVar9 = edFIntervalUnitSrcLERP(fabsf((pFighter->field_0x4e0).x), 1.0f, 1.25f);
		(pFighter->field_0x4e0).w = fVar9;
		break;
	case 0x3d:
		pFighter->_StateFighterMasterSaidDownRotate();
		break;
	case 0x3e:
		if ((pFighter->fightFlags & 0x800) == 0) {
			pFighter->fightFlags = pFighter->fightFlags | 0x800;
			pFighter->UpdatePosition(&pFighter->currentLocation, true);
		}
		else {
			peVar2 = pFighter->field_0x354->pMeshTransform;
			peVar7 = pFighter->field_0x354->pAnimationController->GetCurBoneMatrix(0x6b6e694c);
			edF32Matrix4MulF32Matrix4Hard(&auStack80, peVar7, &peVar2->base.transformA);
			pFighter->UpdatePosition(&auStack80.rowT, false);
		}

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x3f, -1);
		}
		break;
	case 0x3f:
		if ((pFighter->fightFlags & 0x800) == 0) {
			pFighter->fightFlags = pFighter->fightFlags | 0x800;
			pFighter->UpdatePosition(&pFighter->currentLocation, true);
		}
		else {
			peVar2 = pFighter->field_0x354->pMeshTransform;
			peVar7 = pFighter->field_0x354->pAnimationController->GetCurBoneMatrix(0x6b6e694c);
			edF32Matrix4MulF32Matrix4Hard(&auStack144, peVar7, &peVar2->base.transformA);
			pFighter->UpdatePosition(&auStack144.rowT, false);
		}

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x40, -1);
		}
		break;
	case 0x40:
		if ((pFighter->fightFlags & 0x800) == 0) {
			pFighter->fightFlags = pFighter->fightFlags | 0x800;
			pFighter->UpdatePosition(&pFighter->currentLocation, true);
		}
		else {
			peVar2 = pFighter->field_0x354->pMeshTransform;
			peVar7 = pFighter->field_0x354->pAnimationController->GetCurBoneMatrix(0x6b6e694c);
			edF32Matrix4MulF32Matrix4Hard(&auStack208, peVar7, &peVar2->base.transformA);
			pFighter->UpdatePosition(&auStack208.rowT, false);
		}
		if (pFighter->field_0xa70 < pFighter->timeInAir) {
			pFighter->SetState(0x41, -1);
			pFighter->NotifyMaster(8, (void*)0x0);
		}
		break;
	case 0x41:
		if ((pFighter->fightFlags & 0x800) == 0) {
			pFighter->fightFlags = pFighter->fightFlags | 0x800;
			pFighter->UpdatePosition(&pFighter->currentLocation, true);
		}
		else {
			peVar2 = pFighter->field_0x354->pMeshTransform;
			peVar7 = pFighter->field_0x354->pAnimationController->GetCurBoneMatrix(0x6b6e694c);
			edF32Matrix4MulF32Matrix4Hard(&auStack272, peVar7, &peVar2->base.transformA);
			pFighter->UpdatePosition(&auStack272.rowT, false);
		}

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x42, -1);
		}
		break;
	case 0x42:
		if ((pFighter->fightFlags & 0x800) == 0) {
			pFighter->fightFlags = pFighter->fightFlags | 0x800;
			pFighter->UpdatePosition(&pFighter->currentLocation, true);
		}
		else {
			peVar2 = pFighter->field_0x354->pMeshTransform;
			peVar7 = pFighter->field_0x354->pAnimationController->GetCurBoneMatrix(0x6b6e694c);
			edF32Matrix4MulF32Matrix4Hard(&auStack336, peVar7, &peVar2->base.transformA);
			pFighter->UpdatePosition(&auStack336.rowT, false);
		}

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x3b, -1);
		}
		break;
	case 0x43:
		if ((pFighter->fightFlags & 0x800) == 0) {
			pFighter->fightFlags = pFighter->fightFlags | 0x800;
			pFighter->UpdatePosition(&pFighter->currentLocation, true);
		}
		else {
			peVar2 = pFighter->field_0x354->pMeshTransform;
			peVar7 = pFighter->field_0x354->pAnimationController->GetCurBoneMatrix(0x6b6e694c);
			edF32Matrix4MulF32Matrix4Hard(&auStack400, peVar7, &peVar2->base.transformA);
			pFighter->UpdatePosition(&auStack400.rowT, false);
		}

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x3b, -1);
		}
		break;
	case 0x44:
		pFighter->ManageDyn(4.0f, 0, (CActorsTable*)0x0);

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x45, -1);
		}
		break;
	case 0x45:
		pFighter->ManageDyn(4.0f, 0, (CActorsTable*)0x0);
		if (pFighter->field_0xa74 < pFighter->timeInAir) {
			pFighter->SetState(0x46, -1);
		}
		break;
	case 0x46:
		pFighter->ManageDyn(4.0f, 0, (CActorsTable*)0x0);

		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x3b, -1);
		}
		break;
	case 0x47:
		pCVar1 = pFighter->field_0x354;
		pFighter->rotationQuat = pCVar1->rotationQuat;
		pFighter->ManageDyn(4.0f, 0x4022b, (CActorsTable*)0x0);
		break;
	case 0x48:
		pCVar1 = pFighter->field_0x354;
		pFighter->rotationQuat = pCVar1->rotationQuat;
		pFighter->ManageDyn(4.0f, 0x4022b, (CActorsTable*)0x0);
		break;
	case 0x49:
		pFighter->_StateFighterPrepareFightAction(0x4a);
		break;
	case 0x4a:
		pFighter->_StateFighterExecuteBlow(0x4b, 0x4c, 0);
		break;
	case 0x4b:
		pFighter->_StateFighterReturnFromFightAction();
		break;
	case 0x4c:
		pFighter->_StateFighterPrepareFightAction(0x4d);
		break;
	case 0x4d:
		pFighter->_StateFighterExecuteBlow(0x4e, -1, 0);
		break;
	case 0x4e:
		if (pFighter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			edF32Vector4SubHard(&local_1a0, &pFighter->currentLocation, &pFighter->field_0x740);
			edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &local_1a0, &local_1a0);
			pFighter->dynamicExt.instanceIndex.x = local_1a0.x;
			pFighter->dynamicExt.instanceIndex.y = local_1a0.y;
			pFighter->dynamicExt.instanceIndex.z = local_1a0.z;
			pFighter->dynamic.field_0x4c = pFighter->dynamic.field_0x4c | 0x1c000;
			pFighter->NotifyMaster(0xb, (void*)0x0);
			pFighter->SetBehaviour(3, -1, -1);
		}

		pFighter->field_0x740 = pFighter->currentLocation;
		pFighter->_StateFighterReturnFromFightAction();
	}

	return;
}

void CBehaviourFighterSlave::ManageExitByMaster()
{
	bool bVar1;
	CLifeInterface* pCVar2;
	float fVar3;
	edF32VECTOR4* local_4;
	CCollision* pCol;
	CActorFighter* pFighter;

	pFighter = this->pOwner;
	if (pFighter->field_0x354 != (CActorFighter*)0x0) {
		pCol = pFighter->pCollisionData;
		bVar1 = (pCol->flags_0x4 & 1) != 0;
		if (bVar1) {
			local_4 = &pCol->aCollisionContact->location;
		}

		pCVar2 = pFighter->GetLifeInterface();
		fVar3 = pCVar2->GetValue();
		if (fVar3 <= 0.0f) {
			local_4 = &(this->pOwner)->field_0x8f0;
		}

		if (fVar3 <= 0.0f || bVar1) {
			pFighter = this->pOwner;
			pFighter->NotifyMaster(4, local_4);
		}
	}

	return;
}

bool CBehaviourFighterSlave::ExecuteByMaster(s_fighter_action* pAction, s_fighter_action_param* pParam)
{
	byte bVar1;
	ulong uVar2;
	bool bSuccess;
	float fVar3;
	edF32VECTOR4 local_10;
	CActorFighter* pFighter;

	bSuccess = true;
	local_10 = gF32Vector4UnitZ;
	if (pParam == (s_fighter_action_param*)0x0) {
		bSuccess = false;
	}
	else {
		pFighter = this->pOwner;
		if ((pFighter->fightFlags & 0x400) == 0) {
			local_10.x = pParam->field_0x0->x;
			local_10.y = 0.0f;
			local_10.z = pParam->field_0x0->z;
			local_10.w = 0.0f;

			fVar3 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
			this->pOwner->dynamic.speed = fVar3;
			pFighter = this->pOwner;
			pFighter->dynamic.rotationQuat = local_10;
			uVar2 = (static_cast<ulong>(pAction->moveByte) << 0x38) >> 0x3c;
			if (uVar2 == 1) {
				pFighter = this->pOwner;
				pFighter->SetState(0x3c, -1);
			}
			else {
				if (uVar2 != 2) {
					if ((uVar2 == 4) || (uVar2 == 8)) {
						bSuccess = false;
					}
					else {
						pFighter = this->pOwner;
						pFighter->SetState(0x3b, -1);
					}
				}
			}

			if ((pAction->actionByte & 0xf) != 0) {
				bSuccess = false;
			}
		}
		else {
			pFighter->field_0x36c = pFighter->field_0x36c | 1;
			bVar1 = pAction->moveByte & 0xf;
			if (bVar1 == 1) {
				(this->pOwner)->field_0xa5c = -pParam->field_0x0->x;
			}
			else {
				if (bVar1 == 2) {
					(this->pOwner)->field_0xa5c = pParam->field_0x0->x;
				}
				else {
					if ((pAction->moveByte & 0xf) == 0) {
						(this->pOwner)->field_0xa5c = 0.0f;
					}
				}
			}

			pFighter = this->pOwner;
			pFighter->SetState(0x3d, -1);
		}
	}

	return bSuccess;
}

void CSlaveTrajectory::SetConfig(s_fighter_slave_traj_in* pConfig)
{
	memcpy(&this->config, pConfig, sizeof(s_fighter_slave_traj_in));

	return;
}

void CSlaveTrajectory::SetConfigAdjust(s_fighter_slave_traj_in* param_2, float* param_3, float* param_4)
{
	float fVar1;

	if (param_3 != (float*)0x0) {
		fVar1 = edFIntervalLERP(*param_3, (this->config).field_0x0, (this->config).field_0x4, param_2->field_0x0, param_2->field_0x4);
		*param_3 = fVar1;
	}

	if (param_4 != (float*)0x0) {
		fVar1 = edFIntervalLERP(*param_4, (this->config).field_0x8, (this->config).field_0xc, param_2->field_0x8, param_2->field_0xc);
		*param_4 = fVar1;
	}

	memcpy(&this->config, param_2, sizeof(s_fighter_slave_traj_in));

	return;
}

s_fighter_slave_traj_out* CSlaveTrajectory::Compute(float param_1, float param_2)
{
	float fVar1;
	float fVar2;
	float t;
	float puVar3;
	float puVar4;

	fVar1 = edFIntervalDotDstLERP(param_1, (this->config).field_0x0, (this->config).field_0x4);

	if (fVar1 < 0.0f) {
		puVar3 = -1.0f;
		fVar1 = -fVar1;
	}
	else {
		puVar3 = 1.0f;
	}

	fVar2 = edFIntervalUnitSrcLERP(fVar1, (this->config).field_0x10, (this->config).field_0x14);
	t = edFIntervalUnitSrcLERP(fVar1, (this->config).field_0x18, (this->config).field_0x1c);
	(this->output).field_0x0.x = puVar3 * cosf(fVar2 - 1.570796f);
	(this->output).field_0x0.y = 0.0f;
	(this->output).field_0x0.z = cosf(fVar2);
	(this->output).field_0x0.w = 0.0f;
	edF32Vector4ScaleHard(t, &this->output.field_0x0, &this->output.field_0x0);
	fVar1 = edFIntervalUnitSrcLERP(fVar1 * fVar1, (this->config).field_0x30, (this->config).field_0x34);
	(this->output).field_0x14 = puVar3 * fVar1;

	fVar1 = edFIntervalUnitDstLERP(param_2, (this->config).field_0x8, (this->config).field_0xc);
	if (fVar1 < 0.0f) {
		puVar4 = -1.0f;
	}
	else {
		puVar4 = 1.0f;
	}

	fVar2 = edFIntervalUnitSrcLERP(fabsf(fVar1), (this->config).field_0x20, (this->config).field_0x24);
	(this->output).field_0x0.y = puVar4 * fVar2;
	fVar1 = edFIntervalUnitSrcLERP(fabsf(fVar1), (this->config).field_0x28, (this->config).field_0x2c);
	(this->output).field_0x10 = puVar4 * fVar1;

	return &this->output;
}

edF32VECTOR4 edF32VECTOR4_004257b0 = { 0.0f, -1.0f, 0.0f, 0.0f };

void CSlaveGroundSampler::GetGroundDesc(float param_1, float param_2, S_FIGHTER_SLAVE_GROUND_SAMPLE* param_4, edF32VECTOR4* param_5)
{
	uint uVar1;
	float fVar2;
	uint uVar3;
	float fVar4;
	float fVar5;
	float puVar6;
	float puVar7;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	uVar3 = this->field_0x4;
	if (static_cast<int>(uVar3) < 0) {
		fVar2 = static_cast<float>(uVar3 >> 1 | uVar3 & 1);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = static_cast<float>(uVar3);
	}

	fVar2 = 6.283185f / fVar2;

	uVar3 = static_cast<uint>(param_1 / fVar2);
	if (static_cast<int>(uVar3) < 0) {
		fVar4 = static_cast<float>(uVar3 >> 1 | uVar3 & 1);
		fVar4 = fVar4 + fVar4;
	}
	else {
		fVar4 = static_cast<float>(uVar3);
	}

	if (param_1 / fVar2 < fVar4) {
		uVar3 = uVar3 - 1;
	}

	if (static_cast<int>(uVar3) < 0) {
		fVar4 = static_cast<float>(uVar3 >> 1 | uVar3 & 1);
		fVar4 = fVar4 + fVar4;
	}
	else {
		fVar4 = static_cast<float>(uVar3);
	}

	uVar1 = uVar3 + 1;
	fVar4 = fVar4 * fVar2;
	if (static_cast<int>(uVar1) < 0) {
		fVar5 = static_cast<float>(uVar1 >> 1 | uVar1 & 1);
		fVar5 = fVar5 + fVar5;
	}
	else {
		fVar5 = static_cast<float>(uVar1);
	}

	fVar5 = fVar5 * fVar2;
	if (this->field_0x0 != uVar3) {
		local_60 = edF32VECTOR4_004257b0;
		this->field_0x0 = uVar3;
		local_50.x = param_2 * cosf(fVar4 - 1.570796f);
		local_50.y = 0.0f;
		local_50.w = 0.0f;
		local_50.z = param_2 * cosf(fVar4);
		edF32Vector4AddHard(&local_50, &local_50, param_5);
		local_50.y = local_50.y + this->field_0x8 / 2.0f;
		CCollisionRay CStack128 = CCollisionRay(this->field_0x8, &local_50, &local_60);
		fVar2 = CStack128.Intersect(3, (CActor*)0x0, (CActor*)0x0, 0x40000008, &this->field_0x10, (_ray_info_out*)0x0);
		this->field_0x20 = fVar2;
		if (fVar2 == 1e+30f) {
			this->field_0x20 = -this->field_0x8 / 2.0f;
			this->field_0x10 = gF32Vector4UnitY;
		}
		else {
			this->field_0x20 = (local_50.y - fVar2) - param_5->y;
		}

		local_50.y = 0.0f;
		local_50.x = param_2 * cosf(fVar5 - 1.570796f);
		local_50.z = param_2 * cosf(fVar5);
		local_50.w = 0.0f;

		edF32Vector4AddHard(&local_50, &local_50, param_5);
		local_50.y = local_50.y + this->field_0x8 / 2.0f;
		CStack128.ChangeOrigin(&local_50);
		fVar2 = CStack128.Intersect(3, (CActor*)0x0, (CActor*)0x0, 0x40000008, &this->field_0x30, (_ray_info_out*)0x0);
		this->field_0x40 = fVar2;
		if (fVar2 == 1e+30f) {
			this->field_0x40 = this->field_0x20;
			this->field_0x30 = this->field_0x10;
		}
		else {
			this->field_0x40 = (local_50.y - fVar2) - param_5->y;
		}

		edF32Vector4CrossProductHard(&this->field_0x50, &this->field_0x10, &this->field_0x30);
		edF32Vector4SafeNormalize0Hard(&this->field_0x50, &this->field_0x50);
		puVar6 = edF32Vector4DotProductHard(&this->field_0x10, &this->field_0x30);
		if (1.0f < puVar6) {
			puVar7 = 1.0f;
		}
		else {
			puVar7 = -1.0f;
			if (-1.0f <= puVar6) {
				puVar7 = puVar6;
			}
		}

		fVar2 = acosf(puVar7);
		fVar2 = edF32Between_0_2Pi(fVar2);
		this->field_0x60 = fVar2;
	}

	fVar2 = edFIntervalUnitDstLERP(param_1, fVar4, fVar5);
	fVar4 = this->field_0x20;
	param_4->field_0x10 = fVar4 + fVar2 * (this->field_0x40 - fVar4);
	edF32Matrix4FromAngAxisSoft(-this->field_0x60 * fVar2, &eStack64, &this->field_0x50);
	edF32Matrix4MulF32Vector4Hard(&param_4->field_0x0, &eStack64, &this->field_0x10);

	return;
}
