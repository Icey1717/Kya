#include "ActorHero.h"
#include "TimeController.h"
#include "WayPoint.h"
#include "LevelScheduleManager.h"
#include "EventManager.h"

CActorHero* CActorHero::_gThis = (CActorHero*)0x0;


ulong gBoomyHashCodes[4] = {
	0x0,
	CHAR_TO_UINT64("BOOMY_P0"),
	CHAR_TO_UINT64("BOOMY_P1"),
	CHAR_TO_UINT64("BOOMY_P2"),
};

AnimResultHero CActorHero::_gStateCfg_HRO[HERO_STATE_COUNT] = {
AnimResultHero(
	0x0,
	0x0,
	0x0
),
AnimResultHero(
	0x0,
	0x100,
	0xC0401F
),
AnimResultHero(
	0x83,
	0x100,
	0xC0401F
),
AnimResultHero(
	0x84,
	0x100,
	0xC0401F
),
AnimResultHero(
	0xFE,
	0x102,
	0xC4401F
),
AnimResultHero(
	0xFE,
	0x100,
	0xC4401F
),
AnimResultHero(
	0xC7,
	0x100,
	0x4017
),
AnimResultHero(
	0xCB,
	0x4000000,
	0x200601B
),
AnimResultHero(
	0xCC,
	0x4000102,
	0x80401F
),
AnimResultHero(
	0xC7,
	0x100,
	0x401F
),
AnimResultHero(
	0xC8,
	0x4000000,
	0x200601B
),
AnimResultHero(
	0xCA,
	0x4000102,
	0x80401F
),
AnimResultHero(
	0xC9,
	0x4000000,
	0x84601F
),
AnimResultHero(
	0xC9,
	0x4000000,
	0x84601F
),
AnimResultHero(
	0xC9,
	0x4000000,
	0x80601F
),
AnimResultHero(
	0xBE,
	0x0,
	0x80000
),
AnimResultHero(
	0xBF,
	0x0,
	0x80000
),
AnimResultHero(
	0xC0,
	0x100,
	0xC04027
),
AnimResultHero(
	0xC0,
	0x100,
	0xC04027
),
AnimResultHero(
	0xC1,
	0x100,
	0xC04037
),
AnimResultHero(
	0xC2,
	0x100,
	0x844037
),
AnimResultHero(
	0xC2,
	0x0,
	0x804026
),
AnimResultHero(
	0xC3,
	0x100,
	0x20
),
AnimResultHero(
	0xC4,
	0x100,
	0x20
),
AnimResultHero(
	0xC5,
	0x100,
	0x30
),
AnimResultHero(
	0xC3,
	0x100,
	0x30
),
AnimResultHero(
	0xC6,
	0x100,
	0x4022
),
AnimResultHero(
	0xC5,
	0x0,
	0x20
),
AnimResultHero(
	0xA7,
	0x100,
	0x80000
),
AnimResultHero(
	0xA7,
	0x100,
	0x80000
),
AnimResultHero(
	0xA9,
	0x100,
	0x80000
),
AnimResultHero(
	0xAA,
	0x0,
	0x180000
),
AnimResultHero(
	0xAA,
	0x0,
	0x180800
),
AnimResultHero(
	0x107,
	0x0,
	0x81800
),
AnimResultHero(
	0xDA,
	0x100,
	0x80800
),
AnimResultHero(
	0xDA,
	0x100,
	0x88000
),
AnimResultHero(
	0xAB,
	0x100,
	0x0
),
AnimResultHero(
	0xAC,
	0x101,
	0x0
),
AnimResultHero(
	0xAD,
	0x101,
	0x0
),
AnimResultHero(
	0xAE,
	0x1,
	0x100800
),
AnimResultHero(
	0xAF,
	0x1,
	0x1800
),
AnimResultHero(
	0xDB,
	0x1,
	0x800
),
AnimResultHero(
	0xDB,
	0x101,
	0x8000
),
AnimResultHero(
	0xB0,
	0x101,
	0x0
),
AnimResultHero(
	0xB1,
	0x101,
	0x0
),
AnimResultHero(
	0xB2,
	0x1,
	0x0
),
AnimResultHero(
	0xB3,
	0x1,
	0x0
),
AnimResultHero(
	0xB4,
	0x1,
	0x0
),
AnimResultHero(
	0xB5,
	0x1,
	0x0
),
AnimResultHero(
	0xB5,
	0x1,
	0x0
),
AnimResultHero(
	0x1A2,
	0x1,
	0x20
),
AnimResultHero(
	0x117,
	0x100,
	0x1040000
),
AnimResultHero(
	0x118,
	0x102,
	0x1040000
),
AnimResultHero(
	0x119,
	0x100,
	0x1000002
),
AnimResultHero(
	0x105,
	0x0,
	0x3
),
AnimResultHero(
	0x9B,
	0x0,
	0x80
),
AnimResultHero(
	0x9C,
	0x0,
	0x80
),
AnimResultHero(
	0x9B,
	0x0,
	0x80
),
AnimResultHero(
	0x9C,
	0x0,
	0x80
),
AnimResultHero(
	0x99,
	0x0,
	0x80
),
AnimResultHero(
	0x99,
	0x0,
	0x80
),
AnimResultHero(
	0x9A,
	0x0,
	0x80
),
AnimResultHero(
	0x9A,
	0x0,
	0x80
),
AnimResultHero(
	0x9D,
	0x0,
	0x80
),
AnimResultHero(
	0x9E,
	0x0,
	0x80
),
AnimResultHero(
	0x97,
	0x0,
	0x80
),
AnimResultHero(
	0x98,
	0x0,
	0x80
),
AnimResultHero(
	0x9F,
	0x0,
	0x80
),
AnimResultHero(
	0xA0,
	0x0,
	0x80
),
AnimResultHero(
	0xA1,
	0x0,
	0x80
),
AnimResultHero(
	0xA2,
	0x0,
	0x80
),
AnimResultHero(
	0xA3,
	0x4000000,
	0x2007
),
AnimResultHero(
	0xA4,
	0x4000000,
	0x2007
),
AnimResultHero(
	0xA5,
	0x0,
	0x80
),
AnimResultHero(
	0xB7,
	0x0,
	0x41
),
AnimResultHero(
	0xB8,
	0x0,
	0x41
),
AnimResultHero(
	0xBB,
	0x0,
	0x41
),
AnimResultHero(
	0xBB,
	0x0,
	0x41
),
AnimResultHero(
	0xB9,
	0x0,
	0x41
),
AnimResultHero(
	0xBA,
	0x0,
	0x41
),
AnimResultHero(
	0xB9,
	0x0,
	0x41
),
AnimResultHero(
	0xB9,
	0x0,
	0x41
),
AnimResultHero(
	0xBA,
	0x0,
	0x41
),
AnimResultHero(
	0xBA,
	0x0,
	0x41
),
AnimResultHero(
	0xBC,
	0x0,
	0x40
),
AnimResultHero(
	0xC8,
	0x4000000,
	0x59
),
AnimResultHero(
	0xB6,
	0x0,
	0x40
),
AnimResultHero(
	0xB7,
	0x0,
	0x40
),
AnimResultHero(
	0xBD,
	0x0,
	0x40
),
AnimResultHero(
	0xF8,
	0x0,
	0x1
),
AnimResultHero(
	0xF8,
	0x0,
	0x401
),
AnimResultHero(
	0xF9,
	0x0,
	0x401
),
AnimResultHero(
	0xFA,
	0x0,
	0x401
),
AnimResultHero(
	0xFB,
	0x0,
	0x0
),
AnimResultHero(
	0xFC,
	0x4000000,
	0x18
),
AnimResultHero(
	0x88,
	0x0,
	0x0
),
AnimResultHero(
	0x85,
	0x100,
	0x10000
),
AnimResultHero(
	0x86,
	0x100,
	0x10000
),
AnimResultHero(
	0x87,
	0x100,
	0x14002
),
AnimResultHero(
	0x82,
	0x100,
	0x10002
),
AnimResultHero(
	0x0,
	0x100,
	0x14002
),
AnimResultHero(
	0x82,
	0x0,
	0x0
),
AnimResultHero(
	0x83,
	0x0,
	0x0
),
AnimResultHero(
	0x84,
	0x0,
	0x0
),
AnimResultHero(
	0x88,
	0x100,
	0x0
),
AnimResultHero(
	0x114,
	0x100,
	0x0
),
AnimResultHero(
	0xFFFFFFFF,
	0x100,
	0x2
),
AnimResultHero(
	0xFFFFFFFF,
	0x100,
	0x2
),
AnimResultHero(
	0xFFFFFFFF,
	0x100,
	0x2
),
AnimResultHero(
	0x0,
	0x0,
	0x0
),
AnimResultHero(
	0xFD,
	0x102,
	0x400006
),
AnimResultHero(
	0xCD,
	0x100,
	0x80401F
),
AnimResultHero(
	0xCD,
	0x100,
	0x80401F
),
AnimResultHero(
	0xCE,
	0x100,
	0x80401F
),
AnimResultHero(
	0xCF,
	0x100,
	0x80411F
),
AnimResultHero(
	0xD0,
	0x100,
	0x104
),
AnimResultHero(
	0x77,
	0x100,
	0x4
),
AnimResultHero(
	0xD1,
	0x100,
	0x804017
),
AnimResultHero(
	0xD2,
	0x100,
	0xC102
),
AnimResultHero(
	0xD3,
	0x4000000,
	0xE002
),
AnimResultHero(
	0xD7,
	0x4000000,
	0xA002
),
AnimResultHero(
	0xD8,
	0x0,
	0xAA000
),
AnimResultHero(
	0xD9,
	0x0,
	0xAA000
),
AnimResultHero(
	0xD5,
	0x100,
	0xC002
),
AnimResultHero(
	0xD6,
	0x100,
	0xC002
),
AnimResultHero(
	0x104,
	0x100,
	0xC002
),
AnimResultHero(
	0x102,
	0x4000000,
	0x100001
),
AnimResultHero(
	0xFF,
	0x0,
	0x100800
),
AnimResultHero(
	0x102,
	0x0,
	0x100800
),
AnimResultHero(
	0xFF,
	0x0,
	0x100800
),
AnimResultHero(
	0x102,
	0x0,
	0x100000
),
AnimResultHero(
	0x105,
	0x0,
	0x807
),
AnimResultHero(
	0x104,
	0x100,
	0x807
),
AnimResultHero(
	0x106,
	0x0,
	0x1800
),
AnimResultHero(
	0x107,
	0x0,
	0x1800
),
AnimResultHero(
	0x108,
	0x0,
	0x1800
),
AnimResultHero(
	0x109,
	0x0,
	0x1800
),
AnimResultHero(
	0x10A,
	0x0,
	0x1800
),
AnimResultHero(
	0x10B,
	0x0,
	0x1800
),
AnimResultHero(
	0xBC,
	0x0,
	0x1800
),
AnimResultHero(
	0x105,
	0x0,
	0x1800
),
AnimResultHero(
	0x10C,
	0x0,
	0x1800
),
AnimResultHero(
	0x10D,
	0x0,
	0x1802
),
AnimResultHero(
	0x105,
	0x0,
	0x1802
),
AnimResultHero(
	0xD1,
	0x100,
	0x0
),
AnimResultHero(
	0x10E,
	0x100,
	0x0
),
AnimResultHero(
	0x10F,
	0x100,
	0x0
),
AnimResultHero(
	0x110,
	0x100,
	0x0
),
AnimResultHero(
	0xF7,
	0x110,
	0x0
),
AnimResultHero(
	0x111,
	0x100,
	0x4
),
AnimResultHero(
	0x111,
	0x100,
	0x4
),
AnimResultHero(
	0x112,
	0x100,
	0x0
),
AnimResultHero(
	0x113,
	0x100,
	0x0
),
AnimResultHero(
	0x115,
	0x2,
	0x4002
),
AnimResultHero(
	0xC8,
	0x4000000,
	0x200400F
),
AnimResultHero(
	0x102,
	0x2,
	0x4000
),
AnimResultHero(
	0x102,
	0x2,
	0x4000
),
AnimResultHero(
	0x11A,
	0x0,
	0x200
),
AnimResultHero(
	0x11A,
	0x0,
	0x200
),
AnimResultHero(
	0x11B,
	0x0,
	0x200
),
AnimResultHero(
	0x11A,
	0x0,
	0x200
),
AnimResultHero(
	0x11B,
	0x0,
	0x200
),
AnimResultHero(
	0xBB,
	0x0,
	0x40
),
AnimResultHero(
	0x11C,
	0x0,
	0x200
),
AnimResultHero(
	0xC8,
	0x4000000,
	0x6007
),
AnimResultHero(
	0x0,
	0x2000700,
	0x200000
),
AnimResultHero(
	0x0,
	0x2000700,
	0x200001
),
AnimResultHero(
	0x0,
	0x700,
	0x0
),
AnimResultHero(
	0x0,
	0x100,
	0x4002
),
AnimResultHero(
	0xC1,
	0x100,
	0x4022
),
AnimResultHero(
	0x18E,
	0x4000000,
	0x6000
),
AnimResultHero(
	0xC8,
	0x4000000,
	0x200601B
),
AnimResultHero(
	0x190,
	0x110,
	0x4000
),
AnimResultHero(
	0x18F,
	0x110,
	0x4000
),
AnimResultHero(
	0x191,
	0x112,
	0x0
),
AnimResultHero(
	0x18F,
	0x112,
	0x0
),
AnimResultHero(
	0x191,
	0x112,
	0x0
),
AnimResultHero(
	0x18F,
	0x112,
	0x0
),
AnimResultHero(
	0x192,
	0x110,
	0x0
),
AnimResultHero(
	0x193,
	0x10,
	0x0
),
AnimResultHero(
	0x195,
	0x12,
	0x0
),
AnimResultHero(
	0x196,
	0x12,
	0x0
),
AnimResultHero(
	0x194,
	0x10,
	0x0
),
AnimResultHero(
	0x194,
	0x10,
	0x0
),
AnimResultHero(
	0x197,
	0x10,
	0x0
),
AnimResultHero(
	0x198,
	0x10,
	0x0
),
AnimResultHero(
	0x199,
	0x101,
	0x0
),
AnimResultHero(
	0x19A,
	0x101,
	0x0
),
AnimResultHero(
	0x19B,
	0x1,
	0x0
),
AnimResultHero(
	0x19D,
	0x110,
	0x0
),
AnimResultHero(
	0x19C,
	0x10,
	0x0
),
AnimResultHero(
	0x19E,
	0x110,
	0x404003
),
AnimResultHero(
	0x19F,
	0x110,
	0x404003
),
AnimResultHero(
	0x1A0,
	0x10,
	0x4003
)
};

uint CActorHero::_gStateCfg_ELE[HERO_BHVR_COUNT] = {
	0, 0, 0, 0, 0, 0, 
	0, 0x768, 0x3e0, 0x0,
	0, 0, 0, 0
};

StateConfig* CActorHero::GetStateCfg(int state)
{
	AnimResultHero* pHeroStateCfg;

	if (state < 0x72) {
		pHeroStateCfg = (AnimResultHero*)CActorFighter::GetStateCfg(state);
	}
	else {
		assert(state - 0x72 < HERO_STATE_COUNT);
		pHeroStateCfg = _gStateCfg_HRO + state - 0x72;
	}
	return (StateConfig*)pHeroStateCfg;
}

uint CActorHero::GetBehaviourFlags(int state)
{
	uint bhvrFlags;

	if (state < 7) {
		bhvrFlags = CActorFighter::GetBehaviourFlags(state);
	}
	else {
		bhvrFlags = _gStateCfg_ELE[state];
	}
	return bhvrFlags;
}

edF32VECTOR4* CActorHero::GetAdversaryPos()
{
	return (edF32VECTOR4*)0x0;
}

uint CActorHero::GetStateHeroFlags(int state)
{
	uint uVar1;

	uVar1 = 0;
	if (state != AS_None) {
		if (state < 0x72) {
			uVar1 = 0;
		}
		else {
			assert(state - 0x72 < HERO_STATE_COUNT);
			uVar1 = _gStateCfg_HRO[state - 0x72].heroFlags;
		}
	}
	return uVar1;
}

int CActorHero::GetLastCheckpointSector()
{
	return this->lastCheckPointSector;
}

bool CActorHero::CanActivateCheckpoint(uint flags)
{
	int iVar1;
	bool bVar2;
	CLifeInterface* pCVar3;
	uint uVar4;
	StateConfig* pSVar5;
	float fVar6;

	pCVar3 = GetLifeInterface();
	fVar6 = pCVar3->GetValue();

	bVar2 = fVar6 - this->field_0x2e4 <= 0.0f;
	if (!bVar2) {
		iVar1 = this->actorState;
		if (iVar1 == -1) {
			uVar4 = 0;
		}
		else {
			pSVar5 = GetStateCfg(iVar1);
			uVar4 = pSVar5->flags_0x4 & 1;
		}
		bVar2 = uVar4 != 0;
	}

	if (!bVar2) {
		bVar2 = this->field_0x1420 != 0;
		if ((!bVar2) && (bVar2 = true, this->field_0x1558 <= 0.0f)) {
			bVar2 = false;
		}
		if (!bVar2) {
			if ((flags & 0x40000000) == 0) {
				iVar1 = this->actorState;
				bVar2 = true;
				if ((iVar1 != 0xa8) && (iVar1 != 0x129)) {
					bVar2 = false;
				}
				if (bVar2) {
					return true;
				}
			}

			if ((flags & 1) != 0) {
				iVar1 = this->actorState;
				if (iVar1 == -1) {
					uVar4 = 0;
				}
				else {
					pSVar5 = GetStateCfg(iVar1);
					uVar4 = pSVar5->flags_0x4 & 0x100;
				}
				if (uVar4 != 0) {
					return uVar4 != 0;
				}
				iVar1 = this->actorState;
				if (iVar1 == -1) {
					uVar4 = 0;
				}
				else {
					if (iVar1 < 0x72) {
						uVar4 = 0;
					}
					else {
						uVar4 = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
					}
				}

				return (uVar4 & 0x100000) != 0;
			}

			if ((flags & 2) != 0) {
				return false;
			}

			if ((flags & 0x40000000) != 0) {
				bVar2 = this->curBehaviourId == 8;
				if (!bVar2) {
					return bVar2;
				}

				iVar1 = this->actorState;
				if (iVar1 == -1) {
					uVar4 = 0;
				}
				else {
					pSVar5 = GetStateCfg(iVar1);
					uVar4 = pSVar5->flags_0x4 & 0x100;
				}

				return uVar4 != 0;
			}

			iVar1 = this->actorState;
			if (iVar1 == -1) {
				return false;
			}

			return GetStateCfg(iVar1)->flags_0x4 & 0x100;
		}
	}

	return false;
}

void CActorHero::ActivateCheckpoint(_evt_checkpoint_param* pEventCheckpointParam)
{
	CWayPoint* pCVar1;
	//CBehaviourVtable* pCVar2;
	CBehaviour* pCVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	float fVar7;
	float fVar8;

	this->field_0xe50.xyz = pEventCheckpointParam->pWayPointA->location;
	(this->field_0xe50).w = 1.0f;

	this->field_0xe60 = pEventCheckpointParam->pWayPointA->rotation;

	this->levelDataField1C_0xe6c = 0;
	iVar6 = pEventCheckpointParam->sectorId;
	if ((iVar6 != -1) && (0 < iVar6)) {
		this->lastCheckPointSector = iVar6;
	}

	if (((pEventCheckpointParam->flags & 8U) != 0) && (this == _gThis)) {
		this->field_0xe80.xyz = pEventCheckpointParam->pWayPointA->location;
		(this->field_0xe80).w = 1.0f;

		this->field_0xe90 = pEventCheckpointParam->pWayPointA->rotation;

		this->levelDataField1C_0xe9c = 0;
		iVar6 = pEventCheckpointParam->sectorId;
		if ((iVar6 != -1) && (0 < iVar6)) {
			this->field_0xea0 = iVar6;
		}
	}

	if (((pEventCheckpointParam->flags & 0x40000000U) != 0) && (this->curBehaviourId == 8)) {
		IMPLEMENTATION_GUARD(
		pCVar3 = CActor::GetBehaviour(this, (this->character).characterBase.base.base.curBehaviourId);
		pCVar2 = pCVar3[0x23].pVTable;
		if (pCVar2 != (CBehaviourVtable*)0x0) {
			(**(code**)(pCVar2->_vt + 0xfc))(pCVar2, pEventCheckpointParam->pWayPointB, pEventCheckpointParam->flags & 8);
		})
	}

	iVar6 = 0;
	do {
		iVar5 = 0;
		do {
			if (1 < this->field_0xd34[iVar6][iVar5]) {
				this->field_0xd34[iVar6][iVar5] = 1;
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < 0x10);

		iVar6 = iVar6 + 1;
	} while (iVar6 < 0x10);

	return;
}

HeroActionStateCfg CActorHero::_gActionCfg_HRO[16] = {
	{ 0x0 },
	{ 0x11 },
	{ 0x11 },
	{ 0x11 },
	{ 0x0 },
	{ 0x0 },
	{ 0x1 },
	{ 0x0 },
	{ 0x1 },
	{ 0x11 },
	{ 0x11 },
	{ 0x0 },
	{ 0x0 },
	{ 0x0 },
	{ 0x11 },
	{ 0x11 },
};

HeroActionStateCfg* CActorHero::GetActionCfg(int index)
{
	return _gActionCfg_HRO + index;
}

uint CActorHero::TestState_IsInHit(uint inFlags)
{
	EActorState currentState;

	if (inFlags == 0xffffffff) {
		currentState = this->actorState;
		inFlags = 0;

		if ((currentState != AS_None) && (inFlags = 0, 0x71 < (int)currentState)) {
			assert(currentState - 0x72 < HERO_STATE_COUNT);

			inFlags = _gStateCfg_HRO[currentState - 0x72].heroFlags;
		}
	}

	return inFlags & 0x80000;
}

uint CActorHero::TestState_CanTrampo(uint inFlags)
{
	int iVar1;
	StateConfig* pSVar2;

	if (inFlags == 0xffffffff) {
		inFlags = GetStateFlags(this->actorState);
	}

	return inFlags & 0x4000000;
}

uint CActorHero::TestState_IsOnAToboggan(uint inFlags)
{
	EActorState currentState;

	if (inFlags == 0xffffffff) {
		currentState = this->actorState;
		inFlags = 0;

		if ((currentState != AS_None) && (inFlags = 0, 0x71 < (int)currentState)) {
			assert(currentState - 0x72 < HERO_STATE_COUNT);

			inFlags = _gStateCfg_HRO[currentState - 0x72].heroFlags;
		}
	}

	return inFlags & 0x8000;
}

uint CActorHero::TestState_IsGrippedOrClimbing(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0xc0;
}

bool CActorHero::TestState_IsInCheatMode()
{
	int iVar1;
	bool ret;

	iVar1 = this->actorState;
	ret = true;

	if ((iVar1 != 0xa8) && (iVar1 != 0x129)) {
		ret = false;
	}

	return ret;
}

uint CActorHero::TestState_IsInTheWind(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x800;
}

uint CActorHero::TestState_BounceWalls(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x400;
}

uint CActorHero::TestState_AllowAction(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 1;
}

uint CActorHero::TestState_IsSliding(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x100;
}

uint CActorHero::TestState_IsFlying(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}
	return inFlags & 0x100000;
}

uint CActorHero::TestState_NoMoreHit(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x20000;
}

uint CActorHero::TestState_IsCrouched(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x20;
}

uint CActorHero::TestState_AllowMagic(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 4;
}

uint CActorHero::TestState_IsOnCeiling(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x200;
}

uint CActorHero::TestState_IsGripped(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x40;
}

uint CActorHero::TestState_AllowAttack(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x2;
}

uint CActorHero::TestState_001328a0(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;
		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x1000000;
}

uint CActorHero::TestState_00132830(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;
		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x2000000;
}

uint CActorHero::TestState_00132b90(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x10000;
}

uint CActorHero::TestState_CheckFight(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x10;
}

uint CActorHero::TestState_AllowFight(uint inFlags)
{
	int iVar1;

	bool bVar2;
	uint uVar3;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	bVar2 = (inFlags & 8) != 0;
	uVar3 = (uint)bVar2;

	if (bVar2) {
		uVar3 = (this->flags & 0x800000) != 0 ^ 1;
	}

	return uVar3;
}

uint CActorHero::TestState_CanPlaySoccer(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x40000;
}

uint CActorHero::TestState_WindWall(uint inFlags)
{
	int iVar1;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	return inFlags & 0x1000;
}

uint CActorHero::TestState_AllowInternalView(uint inFlags)
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;
	uint uVar4;

	if (inFlags == 0xffffffff) {
		iVar1 = this->actorState;
		inFlags = 0;

		if ((iVar1 != -1) && (inFlags = 0, 0x71 < iVar1)) {
			inFlags = _gStateCfg_HRO[iVar1 + -0x72].heroFlags;
		}
	}

	iVar1 = this->actorState;
	uVar4 = 0;
	if (iVar1 == -1) {
		uVar3 = 0;
	}
	else {
		pSVar2 = GetStateCfg(iVar1);
		uVar4 = pSVar2->flags_0x4;
		uVar3 = uVar4 & 0x100;
	}

	uVar3 = (uint)(uVar3 != 0);
	if (uVar3 != 0) {
		uVar3 = (uVar4 & 1) != 0 ^ 1;
	}

	if (uVar3 != 0) {
		uVar3 = (uint)((inFlags & 0x400000) != 0);
	}

	return uVar3;
}

bool CActorHero::FUN_0014cb60(edF32VECTOR4* v0)
{
	ed_zone_3d* pZone;
	int* piVar1;
	CEventManager* pCVar2;
	bool ret;
	uint uVar3;
	int iVar4;
	int iVar6;

	pCVar2 = CScene::ptable.g_EventManager_006f5080;

	if (2.24f <= CScene::_pinstance->field_0x1c) {
		iVar6 = 0;
		while (true) {
			S_ZONE_STREAM_REF* pZoneStreamRef = this->field_0xe48;
			iVar4 = 0;
			if (piVar1 != (int*)0x0) {
				iVar4 = pZoneStreamRef->entryCount;
			}

			if (iVar4 <= iVar6) goto LAB_0014cc18;

			pZone = pZoneStreamRef->aEntries[iVar6].Get();

			if ((pZone != (ed_zone_3d*)0x0) && (uVar3 = edEventComputeZoneAgainstVertex(pCVar2->activeChunkId, pZone, v0, 0), (uVar3 & 1) != 0)) break;

			iVar6 = iVar6 + 1;
		}
		ret = true;
	}
	else {
	LAB_0014cc18:
		ret = false;
	}

	return ret;
}

float CActorHero::GetMagicalForce()
{
	return this->magicInterface.GetValue();
}

bool CMagicInterface::Activate(int bActive)
{
	bool bSuccess;

	bSuccess = false;

	if (bActive == 0) {
		this->bActive = 0;
		bSuccess = true;
	}
	else {
		if (0.0f < this->value) {
			bSuccess = true;
			this->bActive = 1;
		}
	}

	return bSuccess;
}

bool CMagicInterface::CanActivate()
{
	bool bSuccess;

	bSuccess = 0.0f < this->value;
	if (bSuccess) {
		bSuccess = this->bActive == 0;
	}

	return bSuccess;
}

bool CMagicInterface::IsActive()
{
	return this->bActive;
}

bool CMagicInterface::Manage()
{
	bool bSuccess;

	bSuccess = CScene::_pinstance->CheckFunc_001b9300();
	return bSuccess == false;
}

void CMagicInterface::Draw()
{
	Timer* pTimer;
	float fVar1;

	if (this->bActive != 0) {
		pTimer = Timer::GetTimer();
		fVar1 = this->value - pTimer->cutsceneDeltaTime;
		this->value = fVar1;
		if (fVar1 <= 0.0) {
			this->value = 0.0;
			this->bActive = 0;
		}
	}
	return;
}

void CMagicInterface::SetValue(float value)
{
	this->value = value;
	return;
}

float CMagicInterface::GetValue()
{
	return this->value;
}

float CMagicInterface::GetValueMax()
{
	return this->valueMax;
}

void CMagicInterface::SetValueMax(float max)
{
	this->valueMax = valueMax;
	return;
}

void CMagicInterface::SetHasMagicInteractionAround(int bHasMagicAround)
{
	this->bHasMagicAround = bHasMagicAround;
	return;
}

bool CMoneyInterface::Manage()
{
	bool bVar1;

	bVar1 = CScene::_pinstance->CheckFunc_001b9300();
	return bVar1 == false;
}

float CMoneyInterface::GetValue()
{
	return (float)CLevelScheduler::_gGameNfo.nbMoney;
}
