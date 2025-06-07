#include "ActorWolfen.h"
#include "MemoryStream.h"
#include "ActorHero.h"
#include "ActorCommander.h"
#include "ActorWeapon.h"
#include "CinematicManager.h"
#include "WayPoint.h"
#include "MathOps.h"
#include "EventManager.h"
#include "ActorProjectile.h"
#include "FrontEndDisp.h"
#include "InputManager.h"
#include <string>

struct WolfenAnimMatrixData
{
	void Init()
	{
		edF32MATRIX3* peVar1;

		if ((this->aMatrices == (edF32MATRIX3*)0x0) && (this->nbBones != 0)) {
			this->aMatrices = new edF32MATRIX3[this->nbBones];
			this->pWolfen = (CActorWolfen*)0x0;
		}

		return;
	}

	edF32MATRIX3* aMatrices;
	int nbBones;
	CActorWolfen* pWolfen;
} gWolfenAnimMatrixData;

WolfenConfig CActorWolfen::_gStateCfg_WLF[68]
{
	WolfenConfig(0x0, 0x104, 0x5),
	WolfenConfig(0x6, 0x4, 0x1),
	WolfenConfig(0xA3, 0x904, 0xD),
	WolfenConfig(0x0, 0x104, 0x1),
	WolfenConfig(0x8D, 0x104, 0x45),
	WolfenConfig(0x6, 0x4, 0x1),
	WolfenConfig(0x6, 0x4, 0x1),
	WolfenConfig(0xFFFFFFFF, 0x1, 0x0),
	WolfenConfig(0xFFFFFFFF, 0x1, 0x0),
	WolfenConfig(0x87, 0x1, 0x0),
	WolfenConfig(0x87, 0x1, 0x0),
	WolfenConfig(0x87, 0x1, 0x0),
	WolfenConfig(0x87, 0x1, 0x0),
	WolfenConfig(0x87, 0x1, 0x0),
	WolfenConfig(0x0, 0x1, 0x0),
	WolfenConfig(0x7, 0x0, 0x0),
	WolfenConfig(0x9C, 0x4, 0x0),
	WolfenConfig(0x9C, 0x4, 0x1),
	WolfenConfig(0x9C, 0x4, 0x1),
	WolfenConfig(0x9C, 0x4, 0x5),
	WolfenConfig(0x9D, 0x4, 0x1),
	WolfenConfig(0x9E, 0x4, 0x5),
	WolfenConfig(0x9F, 0x4, 0x1),
	WolfenConfig(0xA0, 0x4, 0x1),
	WolfenConfig(0x82, 0x4, 0x0),
	WolfenConfig(0x83, 0x4, 0x0),
	WolfenConfig(0x7, 0x4, 0x1),
	WolfenConfig(0x0, 0x4, 0x1),
	WolfenConfig(0x0, 0x4, 0x1),
	WolfenConfig(0x0, 0x4, 0x1),
	WolfenConfig(0x0, 0x4, 0x1),
	WolfenConfig(0xA9, 0x2, 0x51),
	WolfenConfig(0xAA, 0x4, 0x51),
	WolfenConfig(0xAA, 0x4, 0x55),
	WolfenConfig(0xAA, 0x2, 0x15),
	WolfenConfig(0xA8, 0x0, 0x55),
	WolfenConfig(0xAD, 0x0, 0x1),
	WolfenConfig(0xAE, 0x0, 0x1),
	WolfenConfig(0x88, 0x0, 0x0),
	WolfenConfig(0x89, 0x0, 0x4),
	WolfenConfig(0x8B, 0x2, 0x2),
	WolfenConfig(0x8C, 0x2, 0x3),
	WolfenConfig(0x84, 0x0, 0x23),
	WolfenConfig(0x8E, 0x2, 0x6),
	WolfenConfig(0x8D, 0x0, 0x2),
	WolfenConfig(0x8F, 0x0, 0x6),
	WolfenConfig(0x91, 0x0, 0x2),
	WolfenConfig(0x92, 0x4, 0x0),
	WolfenConfig(0x93, 0x4, 0x0),
	WolfenConfig(0x6, 0x4, 0x0),
	WolfenConfig(0x0, 0x4, 0x4),
	WolfenConfig(0x94, 0x4, 0x0),
	WolfenConfig(0xA4, 0x4, 0x1),
	WolfenConfig(0xA5, 0x4, 0x1),
	WolfenConfig(0xA6, 0x4, 0x0),
	WolfenConfig(0x6, 0x4, 0x0),
	WolfenConfig(0x6, 0x4, 0x1),
	WolfenConfig(0x84, 0x4, 0x21),
	WolfenConfig(0x94, 0x4, 0x4),
	WolfenConfig(0xFFFFFFFF, 0x900, 0x0),
	WolfenConfig(0xA7, 0x2000101, 0x22),
	WolfenConfig(0xFFFFFFFF, 0x2000101, 0x22),
	WolfenConfig(0xFFFFFFFF, 0x0, 0x0),
	WolfenConfig(0xFFFFFFFF, 0x0, 0x0),
	WolfenConfig(0xFFFFFFFF, 0x0, 0x0),
	WolfenConfig(0xFFFFFFFF, 0x0, 0x0),
	WolfenConfig(0xFFFFFFFF, 0x0, 0x0),
	WolfenConfig(0xFFFFFFFF, 0x0, 0x0),
};

bool CActorWolfen::IsKindOfObject(ulong kind)
{
	return (kind & 0x1f) != 0;
}

WolfenCollisionSphere CActorWolfen::_pDefCollisions[4] =
{
	{ { 0.4f, 0.8f, 0.4f, 0.0f }, { 0.0f, 0.8f, 0.0f, 1.0f } },
	{ { 0.5f, 1.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ { 0.6f, 1.0f, 0.6f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ { 0.6f, 1.0f, 0.6f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
};

void CActorWolfen::Create(ByteCode* pByteCode)
{
	CCollision* pCVar1;
	edColPRIM_OBJECT* peVar2;
	uint uVar3;
	CVision* pPerception;
	EnemyComponent80* pEVar4;
	edF32VECTOR4* peVar5;
	int iVar6;
	float fVar7;

	this->field_0xb80 = 0;
	CActorFighter::Create(pByteCode);
	this->field_0xb74 = pByteCode->GetU32();

	assert(this->field_0xb74 < 4); // Need to ensure that there is valid table entries below for higher values.
	ChangeCollisionSphere(0.0f, &_pDefCollisions[this->field_0xb74].field_0x0, &_pDefCollisions[this->field_0xb74].field_0x10);
	StoreCollisionSphere();

	GetVision()->Create(this, pByteCode);

	//uVar3 = this->field_0xb74;
	//if ((uVar3 == 1) || (uVar3 == 0)) {
	//	if (g_EnemyComponent80_0049c6d0.field_0x0 == (StaticEnemy90*)0x0 || g_EnemyComponent80_0049c6d0.field_0x4 == 0) {
	//		(*(code*)(g_EnemyComponent80_0049c6d0.pVTable)->setObjCounts)(0x49c6d0, 4, 0);
	//		INT_004497e8 = 0;
	//	}
	//	this->pEnemyComponent80_0xd34 = &g_EnemyComponent80_0049c6d0;
	//	INT_004497e8 = INT_004497e8 + 1;
	//}
	//else {
	//	if (uVar3 == 3) {
	//		pEVar4 = (EnemyComponent80*)operator.new(0x80);
	//		if (pEVar4 != (EnemyComponent80*)0x0) {
	//			EnemyComponent80::Constructor_003fdca0(pEVar4);
	//			pEVar4->pVTable = &EnemyComponent80::VTable_004460b0;
	//		}
	//		this->pEnemyComponent80_0xd34 = pEVar4;
	//		(*(code*)this->pEnemyComponent80_0xd34->pVTable->setObjCounts)(this->pEnemyComponent80_0xd34, 0x1a, 0x1d);
	//		(*(code*)this->pEnemyComponent80_0xd34->pVTable->setupObjects)(this->pEnemyComponent80_0xd34, this);
	//	}
	//	else {
	//		this->pEnemyComponent80_0xd34 = (EnemyComponent80*)0x0;
	//	}
	//}

	this->field_0xb8c = pByteCode->GetS32();

	this->hearingDetectionProps.Create(pByteCode);
	this->visionDetectionProps.Create(pByteCode);

	this->field_0xcf4 = pByteCode->GetF32();

	this->walkSpeed = pByteCode->GetF32();
	this->walkAcceleration = pByteCode->GetF32();
	this->walkRotSpeed = pByteCode->GetF32();

	this->runSpeedScale = pByteCode->GetF32();
	fVar7 = pByteCode->GetF32();
	this->defaultRunSpeed = fVar7;

	this->runSpeed = fVar7;
	this->runAcceleration = pByteCode->GetF32();
	this->rotRunSpeed = pByteCode->GetF32();

	uVar3 = pByteCode->GetU32();
	if ((int)uVar3 < 0) {
		fVar7 = (float)(uVar3 >> 1 | uVar3 & 1);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)uVar3;
	}
	this->field_0xb84 = fVar7;

	this->field_0xd30 = pByteCode->GetU32();

	uVar3 = this->nbComboRoots;
	if (uVar3 == 0) {
		this->aComboMatchValues = (edF32VECTOR4*)0x0;
	}
	else {
		this->nbComboMatchValues = uVar3;

		if (this->nbComboMatchValues == 0) {
			this->field_0xbd0 = (ComboData*)0x0;
		}
		else {
			this->field_0xbd0 = new ComboData[this->nbComboMatchValues];
		}

		this->aComboMatchValues = new edF32VECTOR4[this->nbComboRoots];
	}

	this->field_0xb90 = pByteCode->GetF32();
	this->field_0xb94 = pByteCode->GetF32();
	this->field_0xb98 = pByteCode->GetF32();
	this->field_0xd1c = 10.0f;
	this->field_0xd20 = 20.0f;

	Create_FightParam(pByteCode);

	this->pTargetActor_0xc80 = (CActorFighter*)0x0;
	this->pCommander = (CActorCommander*)0x0;
	this->combatFlags_0xb78 = 0;
	this->combatMode_0xb7c = ECM_None;

	pCVar1 = this->pCollisionData;
	if (pCVar1 == (CCollision*)0x0) {
		this->field_0xcf0 = 1.5f;
	}
	else {
		peVar2 = pCVar1->pObbPrim;
		this->field_0xcf0 = (peVar2->field_0xb0).y + (peVar2->field_0x90).y * 0.6f;
	}

	return;
}

void CActorWolfen::Init()
{
	int* piVar1;
	CShadow* pCVar2;
	s_fighter_combo* pCombos;
	CLifeInterface* pCVar4;
	int iVar5;
	uint uVar6;
	int iVar7;
	int iVar8;
	uint uVar9;
	edF32VECTOR4* peVar10;
	byte* pcVar11;
	int iVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	float fVar21;
	float fVar22;
	float fVar23;
	float fVar24;
	float fVar25;
	float fVar26;
	float uVar27;

	CActorFighter::Init();
	this->combatFlags_0xb78 = 0;

	pCVar2 = this->pShadow;
	if (pCVar2 != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		(pCVar2->base).field_0x48 = (float)&DAT_3f4ccccd;
		(this->pShadow)->field_0x50 = (float)&DAT_3f4ccccd;)
	}

	if (gWolfenAnimMatrixData.aMatrices == (edF32MATRIX3*)0x0) {
		gWolfenAnimMatrixData.Init();
	}

	uVar6 = this->nbComboRoots;
	uVar9 = 0;
	fVar17 = 3.402823e+38f;
	fVar13 = -3.402823e+38f;
	fVar14 = fVar17;
	fVar24 = fVar17;
	fVar15 = fVar17;
	fVar18 = fVar17;
	fVar22 = fVar13;
	fVar25 = fVar13;
	if (uVar6 != 0) {
		fVar19 = fVar13;
		fVar16 = fVar13;
		fVar20 = fVar13;
		fVar26 = fVar13;
		do {
			fVar23 = 0.0f;
			fVar21 = 0.0f;
			s_fighter_combo* pCombo = this->aCombos + uVar9;
			s_fighter_blow* pCurrentBlow = LOAD_SECTION_CAST(s_fighter_blow*, pCombo->actionHash.pData);

			fVar25 = pCurrentBlow->field_0x50;
			fVar22 = pCurrentBlow->field_0x4c;

			do {
				pCurrentBlow = LOAD_SECTION_CAST(s_fighter_blow*, pCombo->actionHash.pData);
				fVar23 = fVar23 + pCurrentBlow->damage;
				if (pCombo->nbBranches == 0) {
					pCombo = (s_fighter_combo*)0x0;
					fVar25 = fVar25 + pCurrentBlow->field_0x54;
				}
				else {
					pCombo = LOAD_SECTION_CAST(s_fighter_combo*, pCombo->aBranches[0].pData);
				}

				fVar21 = fVar21 + 1.0f;
			} while (pCombo != (s_fighter_combo*)0x0);

			this->aComboMatchValues[uVar9].x = fVar25;
			this->aComboMatchValues[uVar9].y = fVar22;
			this->aComboMatchValues[uVar9].z = fVar23;
			this->aComboMatchValues[uVar9].w = fVar21;

			fVar13 = fVar22;
			if (fVar22 <= fVar19) {
				fVar13 = fVar19;
			}

			if (fVar18 <= fVar22) {
				fVar22 = fVar18;
			}

			fVar18 = fVar22;
			fVar22 = fVar25;
			if (fVar25 <= fVar20) {
				fVar22 = fVar20;
			}

			if (fVar15 <= fVar25) {
				fVar25 = fVar15;
			}

			fVar15 = fVar25;
			fVar25 = fVar23;
			if (fVar23 <= fVar26) {
				fVar25 = fVar26;
			}

			if (fVar14 <= fVar23) {
				fVar23 = fVar14;
			}

			fVar14 = fVar23;
			fVar20 = fVar21;
			if (fVar21 <= fVar16) {
				fVar20 = fVar16;
			}

			if (fVar21 < fVar24) {
				uVar6 = this->nbComboRoots;
				fVar24 = fVar21;
			}
			else {
				uVar6 = this->nbComboRoots;
			}

			uVar9 = uVar9 + 1;
			fVar19 = fVar13;
			fVar16 = fVar20;
			fVar20 = fVar22;
			fVar26 = fVar25;
		} while (uVar9 < uVar6);
	}
	uVar9 = 0;
	fVar19 = fVar17;
	if (uVar6 != 0) {
		fVar16 = fVar17;
		fVar20 = fVar17;

		do {
			pCombos = this->aCombos;

			fVar26 = 1.0f;
			if (fVar13 - fVar18 != 0.0f) {
				fVar26 = (this->aComboMatchValues[uVar9].y - fVar18) / (fVar13 - fVar18);
			}

			fVar23 = 1.0f;
			if (fVar22 - fVar15 != 0.0f) {
				fVar23 = 1.0f - (this->aComboMatchValues[uVar9].x - fVar15) / (fVar22 - fVar15);
			}

			uVar27 = 1.0f;
			fVar21 = 1.0f;
			if (fVar25 - fVar14 == 0.0f) {
			}
			else {
				fVar21 = (this->aComboMatchValues[uVar9].z - fVar14) / (fVar25 - fVar14);
			}

			edF32VECTOR4* pVec = this->aComboMatchValues + uVar9;

			if (fVar24 != pVec->z) {
				uVar27 = 0.0f;
			}

			fVar17 = fVar16;
			fVar19 = fVar20;
			if (0.5f <= fVar26) {
				fVar17 = pVec->y;
				if (fVar16 <= pVec->y) {
					fVar17 = fVar16;
				}
			}
			else {
				fVar19 = pVec->y;
				if (fVar20 <= fVar19) {
					fVar19 = fVar20;
				}
			}
			pVec->y = fVar26;

			this->aComboMatchValues[uVar9].x = fVar23;
			this->aComboMatchValues[uVar9].y = fVar21;
			this->aComboMatchValues[uVar9].w = uVar27;

			field_0xbd0[uVar9].pCombo = &pCombos[uVar9];
			this->field_0xbd8 = 0;
			field_0xbd0[uVar9].field_0x4 = fVar23 + 0.3f;
			this->field_0xbd8 = 0;
			pcVar11 = &field_0xbd0[uVar9].field_0x0;
			if (*pcVar11 != 1) {
				*pcVar11 = 1;
				this->field_0xbd8 = 0;
			}

			uVar9 = uVar9 + 1;
			fVar16 = fVar17;
			fVar20 = fVar19;
		} while (uVar9 < this->nbComboRoots);
	}

	this->field_0xbe4 = 2;

	uVar6 = 0;
	if (this->nbComboMatchValues != 0) {
		iVar5 = 0;
		do {
			this->field_0xbd0[uVar6].field_0x0 = 1;
			uVar6 = uVar6 + 1;
		} while (uVar6 < this->nbComboMatchValues);
	}

	this->field_0xbe0 = 0;
	this->field_0xbd8 = 0;

	if (fVar13 - fVar18 == 0.0f) {
		this->field_0xbf0 = fVar13;
		this->field_0xbec = fVar13;
	}
	else {
		this->field_0xbec = fVar19;
		this->field_0xbf0 = fVar17 * 0.9f;
	}

	fVar14 = this->field_0xbec;
	fVar24 = ((this->pCollisionData)->pObbPrim->field_0x90).z + (CActorHero::_gThis->pCollisionData->pObbPrim->field_0x90).z + 0.1f;
	if (fVar24 < fVar14) {
		this->field_0xbec = fVar14;
	}
	else {
		this->field_0xbec = fVar24;
		fVar14 = fVar24;
	}

	if (this->field_0xbf0 < fVar14) {
		this->field_0xbf0 = fVar14;
	}

	fVar14 = edFIntervalUnitSrcLERP(this->field_0xa80, fVar24, this->field_0xbec);
	this->field_0xbec = fVar14;
	fVar14 = edFIntervalUnitSrcLERP(this->field_0xa80, 0.7f, 0.0f);
	this->field_0xbf0 = this->field_0xbf0 - fVar14;
	this->field_0xb80 = 1;
	this->field_0xb88 = 0;
	pCVar4 = GetLifeInterfaceOther();
	//pCVar4->field_0x10 = this;
	ClearLocalData();
	return;
}

void CActorWolfen::Term()
{
	uint uVar1;
	EnemyComponent80* pAlloc;
	CActorWeapon* pCVar2;
	CActor* pCVar3;

	if (this->aComboMatchValues != (edF32VECTOR4*)0x0) {
		delete[] this->aComboMatchValues;
		this->aComboMatchValues = (edF32VECTOR4*)0x0;
	}

	//if (gWolfenAnimMatrixData.aMatrices != (edF32MATRIX3*)0x0) {
	//	FreeFunc_001fba40((int*)&gWolfenAnimMatrixData);
	//}

	//uVar1 = this->field_0xb74;
	//if (uVar1 == 3) {
	//	if (this->pEnemyComponent80_0xd34 != (EnemyComponent80*)0x0) {
	//		(*(code*)this->pEnemyComponent80_0xd34->pVTable->field_0x10)();
	//		pAlloc = this->pEnemyComponent80_0xd34;
	//		if (pAlloc != (EnemyComponent80*)0x0) {
	//			pAlloc->pVTable = &EnemyComponent80::VTable_004460b0;
	//			if (pAlloc != (EnemyComponent80*)0x0) {
	//				pAlloc->pVTable = &EnemyComponent80VTable_00447ec0;
	//			}
	//			Free(pAlloc);
	//		}
	//		this->pEnemyComponent80_0xd34 = (EnemyComponent80*)0x0;
	//	}
	//}
	//else {
	//	if (((uVar1 == 1) || (uVar1 == 0)) && (this->pEnemyComponent80_0xd34 != (EnemyComponent80*)0x0)) {
	//		this->pEnemyComponent80_0xd34 = (EnemyComponent80*)0x0;
	//		INT_004497e8 = INT_004497e8 + -1;
	//		if (INT_004497e8 < 1) {
	//			(*(code*)(g_EnemyComponent80_0049c6d0.pVTable)->field_0x10)();
	//		}
	//	}
	//}

	if (((this->flags & 0x2000000) == 0) && (GetWeapon() != (CActorWeapon*)0x0)) {
		pCVar3 = GetWeapon()->GetLinkFather();
		if (pCVar3 == this) {
			GetWeapon()->UnlinkWeapon();
		}
	}

	CActorFighter::Term();

	if (this->field_0xb64 != (WolfenComboData*)0x0) {
		delete this->field_0xb64;
		this->field_0xb64 = (WolfenComboData*)0x0;
	}

	//if (this->pWolfenKnowledge != (CActorWolfenKnowledge*)0x0) {
	//	CActorWolfenKnowledge::Term(this->pWolfenKnowledge);
	//	Free(this->pWolfenKnowledge);
	//	this->pWolfenKnowledge = (CActorWolfenKnowledge*)0x0;
	//}

	return;
}

void CActorWolfen::Manage()
{
	bool bVar1;
	CEventManager* pCVar2;
	CActorFighter* pCVar3;
	ed_zone_3d* peVar4;
	int iVar5;
	CLifeInterface* pCVar6;
	float fVar7;

	CActorFighter::Manage();

	ManageKnowledge();

	if (this->field_0xd30 != -1) {
		iVar5 = 2;
		pCVar3 = this->pCommander->GetIntruder();
		pCVar2 = CScene::ptable.g_EventManager_006f5080;
		if (pCVar3 != (CActorFighter*)0x0) {
			peVar4 = (ed_zone_3d*)0x0;
			if (this->field_0xd30 != 0xffffffff) {
				peVar4 = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->field_0xd30);
			}

			iVar5 = edEventComputeZoneAgainstVertex(pCVar2->activeChunkId, peVar4, &pCVar3->currentLocation, 0);
		}

		if (iVar5 == 2) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xffffffdf;
			if ((~this->combatFlags_0xb78 & 0x30) == 0x30) {
				this->pTargetActor_0xc80 = (CActorFighter*)0x0;
			}
		}
		else {
			if (iVar5 == 1) {
				if ((~this->combatFlags_0xb78 & 0x30) == 0x30) {
					this->pTargetActor_0xc80 = pCVar3;
				}
				this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x20;
			}
		}

		pCVar2 = CScene::ptable.g_EventManager_006f5080;
		peVar4 = (ed_zone_3d*)0x0;
		if (this->field_0xd30 != 0xffffffff) {
			peVar4 = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->field_0xd30);
		}

		iVar5 = edEventComputeZoneAgainstVertex(pCVar2->activeChunkId, peVar4, &this->currentLocation, 0);
		if (iVar5 == 2) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffffbff;
		}
		else {
			if (iVar5 == 1) {
				this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x400;
			}
		}
	}

	if ((0 < (int)this->combatMode_0xb7c) && (pCVar3 = this->pTargetActor_0xc80, pCVar3 != (CActorHero*)0x0)) {
		fVar7 = pCVar3->GetLifeInterface()->GetValue();
		if ((fVar7 <= 0.0f) && (this->combatMode_0xb7c != ECM_None)) {
			this->combatMode_0xb7c = ECM_None;
		}
	}

	bVar1 = false;
	if ((this->field_0xb80 != 2) &&
		(((~this->combatFlags_0xb78 & 0x600) != 0x600 ||
			(bVar1 = true, 50.0f <= this->field_0x7c8 - this->field_0x7d0)))) {
		bVar1 = false;
	}

	if (bVar1) {
		if ((this->combatFlags_0xb78 & 0x10000) == 0) {
			this->flags = this->flags | 2;
			this->flags = this->flags & 0xfffffffe;
			this->flags = this->flags | 0x80;
			this->flags = this->flags & 0xffffffdf;
			EvaluateDisplayState();
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x10000;
			this->pCommander->RemoveTracked();
		}
	}
	else {
		if ((this->combatFlags_0xb78 & 0x10000) != 0) {
			this->pCommander->AddTracked();
			this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffeffff;
			if ((this->flags & 2) != 0) {
				this->flags = this->flags & 0xfffffffc;
			}
			if ((this->flags & 0x80) != 0) {
				this->flags = this->flags & 0xffffff5f;
				EvaluateDisplayState();
			}
		}
	}

	return;
}

void CActorWolfen::Draw()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CActorWolfen::Reset()
{
	this->field_0xb80 = 1;
	this->field_0xb88 = 0;

	this->flags = this->flags & 0xffffff5f;
	EvaluateDisplayState();
	this->flags = this->flags & 0xfffffffc;

	ClearLocalData();
	CActorFighter::Reset();

	return;
}

void CActorWolfen::CheckpointReset()
{
	if (this->field_0xb80 == 2) {
		CActorAutonomous::CheckpointReset();
		ClearLocalData();
	}
	else {
		CActor::PreReset();
		Reset();
	}

	return;
}

void CActorWolfen::SectorChange(int oldSectorId, int newSectorId)
{
	if ((this->combatFlags_0xb78 & 0x10000) != 0) {
		IMPLEMENTATION_GUARD(
		FUN_00171a80(this->pCommander);
		this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffeffff;
		if ((this->flags & 2) != 0) {
			this->flags = this->flags & 0xfffffffc;
		}
		if ((this->flags & 0x80) != 0) {
			this->flags = this->flags & 0xffffff5f;
			CActor::EvaluateDisplayState((CActor*)this);
		}
		(*(code*)(this->pVTable)->CheckpointReset)(this);)
	}

	CActor::SectorChange(oldSectorId, newSectorId);

	return;
}

CBehaviour* CActorWolfen::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	switch (behaviourType) {
	case FIGHTER_BEHAVIOUR_DEFAULT:
		pBehaviour = &this->behaviourFighterWolfen;
		break;
	case FIGHTER_BEHAVIOUR_PROJECTED:
		pBehaviour = &this->behaviourWolfenFighterProjected;
		break;
	case FIGHTER_BEHAVIOUR_RIDDEN:
		pBehaviour = &this->behaviourWolfenFighterRidden;
		break;
	case FIGHTER_BEHAVIOUR_SLAVE:
		pBehaviour = &this->behaviourWolfenFighterSlave;
		break;
		IMPLEMENTATION_GUARD();
		break;
	case WOLFEN_BEHAVIOUR_WATCH_DOG:
		pBehaviour = new CBehaviourWatchDog;
		break;
	case WOLFEN_BEHAVIOUR_GUARD_AREA:
		pBehaviour = new CBehaviourGuardArea;
		break;
	case WOLFEN_BEHAVIOUR_SLEEP:
		pBehaviour = new CBehaviourSleep;
		break;
	case WOLFEN_BEHAVIOUR_ESCAPE:
		pBehaviour = new CBehaviourEscape;
		break;
	case WOLFEN_BEHAVIOUR_EXORCISM:
		pBehaviour = &behaviourExorcism;
		break;
	case WOLFEN_BEHAVIOUR_TRACK:
		pBehaviour = &behaviourTrack;
		break;
	case WOLFEN_BEHAVIOUR_TRACK_WEAPON:
		pBehaviour = new CBehaviourTrackWeapon;
		break;
	case WOLFEN_BEHAVIOUR_TRACK_WEAPON_STAND:
		pBehaviour = new CBehaviourTrackWeaponStand;
		break;
	case WOLFEN_BEHAVIOUR_TRACK_WEAPON_SNIPE:
		pBehaviour = new CBehaviourTrackWeaponSnipe;
		break;
	case WOLFEN_BEHAVIOUR_SNIPE:
		pBehaviour = new CBehaviourSnipe;
		break;
	case WOLFEN_BEHAVIOUR_LOST:
		pBehaviour = &behaviourLost;
		break;
	case WOLFEN_BEHAVIOUR_WOLFEN_UNKNOWN:
		pBehaviour = new CBehaviourWolfenUnknown;
		break;
	case WOLFEN_BEHAVIOUR_DCA:
		pBehaviour = new CBehaviourDCA;
		break;
	case WOLFEN_BEHAVIOUR_AVOID:
		pBehaviour = new CBehaviourAvoid;
		break;
	case WOLFEN_BEHAVIOUR_TRACK_STAND:
		pBehaviour = new CBehaviourTrackStand;
		break;
	default:
		assert(behaviourType < 7);
		pBehaviour = CActorFighter::BuildBehaviour(behaviourType);
		break;
	}
	return pBehaviour;
}

StateConfig* CActorWolfen::GetStateCfg(int state)
{
	StateConfig* pWVar1;

	if (state < 0x72) {
		pWVar1 = CActorFighter::GetStateCfg(state);
	}
	else {
		assert((state - 0x72) < 68);
		pWVar1 = _gStateCfg_WLF + state + -0x72;
	}

	return pWVar1;
}

uint _gBehaviourFlags_WLF[27] =
{
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x1,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
};

uint CActorWolfen::GetBehaviourFlags(int state)
{
	uint behaviourFlags;

	if (state < 7) {
		behaviourFlags = CActorFighter::GetBehaviourFlags(state);
	}
	else {
		behaviourFlags = _gBehaviourFlags_WLF[state];
	}

	return behaviourFlags;
}

void CActorWolfen::UpdatePostAnimEffects()
{
	uint uVar1;
	EnemyComponent80* this_00;

	uVar1 = this->field_0xb74;
	if (uVar1 == 3) {
		if (this->pEnemyComponent80_0xd34 != (EnemyComponent80*)0x0) {
			IMPLEMENTATION_GUARD(
			this->pEnemyComponent80_0xd34->FUN_003c1400();)
		}
	}
	else {
		if ((((uVar1 == 1) || (uVar1 == 0)) && (this_00 = this->pEnemyComponent80_0xd34, this_00 != (EnemyComponent80*)0x0)) && (this_00->pEnemy_0x60 == this)) {
			IMPLEMENTATION_GUARD(
			this_00->FUN_003c2b20();)
		}
	}

	return;
}

void CActorWolfen::SetState(int newState, int animType)
{
	int iVar1;
	ulong uVar2;

	if (newState == WOLFEN_STATE_TRACK_CHASE) {
		animType = 7;

		if (this->combatMode_0xb7c == ECM_InCombat) {
			animType = 6;
		}
	}
	else {
		if (newState == WOLFEN_STATE_COME_BACK) {
			animType = 6;
			if ((this->combatFlags_0xb78 & 0x100000) == 0) {
				animType = 7;
			}
		}
		else {
			if (((newState == 0x99) && (iVar1 = GetIdMacroAnim(0x8a), iVar1 != -1)) && (CScene::Rand() < 0x4000)) {
				animType = 0x8a;
			}
		}
	}

	CActorMovable::SetState(newState, animType);

	return;
}

void CActorWolfen::ChangeManageState(int state)
{
	int iVar1;
	CBehaviour* pCVar2;
	CActorWeapon* pCVar3;
	CActor* pCVar4;

	CActorAutonomous::ChangeManageState(state);

	iVar1 = this->curBehaviourId;
	if (iVar1 == 5) {
		IMPLEMENTATION_GUARD(
		pCVar2 = CActor::GetBehaviour((CActor*)this, this->curBehaviourId);
		(*(code*)pCVar2->pVTable[1].field_0x0.field_0x4)(pCVar2, state);)
	}
	else {
		if (iVar1 == 4) {
			IMPLEMENTATION_GUARD(
			pCVar2 = CActor::GetBehaviour((CActor*)this, this->curBehaviourId);
			(*(code*)pCVar2->pVTable[1].field_0x0.field_0x4)(pCVar2, state);)
		}
		else {
			if (iVar1 == 3) {
				CBehaviourFighterWolfen* pWolfenBehaviour = static_cast<CBehaviourFighterWolfen*>(GetBehaviour(this->curBehaviourId));
				pWolfenBehaviour->ManageCombatMusic(state);
			}
			else {
				if (iVar1 == 0xe) {
					IMPLEMENTATION_GUARD(
					pCVar2 = CActor::GetBehaviour((CActor*)this, this->curBehaviourId);
					(*(code*)pCVar2->pVTable->SaveContext)(pCVar2, state);)
				}
				else {
					if (this->actorState == WOLFEN_STATE_TRACK_CHASE) {
						if (state == 0) {
							IMPLEMENTATION_GUARD_AUDIO(
							FUN_001844c0(CScene::ptable.g_AudioManager_00451698);)
						}
						else {
							IMPLEMENTATION_GUARD_AUDIO(
							GlobalSound::Func_001844f0(CScene::ptable.g_AudioManager_00451698);)
						}
					}
				}
			}
		}
	}

	if (GetWeapon() != (CActorWeapon*)0x0) {
		pCVar3 = GetWeapon();
		pCVar4 = pCVar3->GetLinkFather();
		if (pCVar4 == this) {
			if (state == 0) {
				pCVar3 = GetWeapon();
				pCVar3->flags = pCVar3->flags & 0xfffffffc;
			}
			else {
				pCVar3 = GetWeapon();
				pCVar3->flags = pCVar3->flags | 2;
				pCVar3->flags = pCVar3->flags & 0xfffffffe;
			}
		}
	}

	this->lifeInterface.SetPriority(0);

	return;
}

void CActorWolfen::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	if (newAnim == 0xa2) {
		AnimEvaluate_0xa2(layerId);
	}
	else {
		if (((newAnim == 0xaa) || (newAnim == 0xa9)) || (newAnim == 0xa8)) {
			AnimEvaluate_0017c930(layerId, pAnimator);
		}
		else {
			CActorFighter::AnimEvaluate(layerId, pAnimator, newAnim);
		}
	}
	return;
}

void CActorWolfen::CinematicMode_Enter(bool bSetState)
{
	CActorWeapon* pCVar1;
	CAnimation* pAnimation;

	CActor::CinematicMode_Enter(bSetState);

	if (this->field_0xb74 == 3) {
		pAnimation = this->pAnimationController;
		if (pAnimation != (CAnimation*)0x0) {
			pAnimation->RemoveDisabledBone(0xae8f8ef8);
		}

		pCVar1 = GetWeapon();
		if (pCVar1 != (CActorWeapon*)0x0) {
			pCVar1 = GetWeapon();
			pCVar1->flags = pCVar1->flags & 0xffffff7f;
			pCVar1->flags = pCVar1->flags | 0x20;
			pCVar1->EvaluateDisplayState();
		}
	}

	return;
}

void CActorWolfen::CinematicMode_Leave(int behaviourId)
{
	CActorWeapon* pCVar1;
	CAnimation* pAnimation;

	CActor::CinematicMode_Leave(behaviourId);

	if (behaviourId == 0xe) {
		PlayAnim(_SV_ANM_GetTwoSidedAnim(0x75, this->field_0x7dc));
	}

	if (this->field_0xb74 == 3) {
		pAnimation = this->pAnimationController;
		if (pAnimation != (CAnimation*)0x0) {
			pAnimation->AddDisabledBone(0xae8f8ef8);
		}

		pCVar1 = CActorFighter::GetWeapon();
		if (pCVar1 != (CActorWeapon*)0x0) {
			pCVar1 = GetWeapon();
			pCVar1->flags = pCVar1->flags & 0xffffff5f;
			pCVar1->EvaluateDisplayState();
		}
	}

	return;
}

bool CActorWolfen::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	int iVar1;
	EnemyComponent80* pEVar2;
	CBehaviour* pCVar3;

	CActorFighter::CarriedByActor(pActor, m0);

	iVar1 = this->curBehaviourId;
	if (iVar1 == 0xe) {
		IMPLEMENTATION_GUARD(
		pCVar3 = GetBehaviour(this->curBehaviourId);
		FUN_001ee4e0((int)pCVar3, (long)(int)pActor, m0);)
	}
	else {
		if (iVar1 == 3) {
			IMPLEMENTATION_GUARD(
			pCVar3 = CActor::GetBehaviour((CActor*)this, this->curBehaviourId);
			(*(code*)pCVar3->pVTable[1].field_0x0.Create)(pCVar3, (long)(int)pActor, m0);)
		}
	}

	iVar1 = this->field_0xb74;
	if (iVar1 == 3) {
		IMPLEMENTATION_GUARD(
		if (this->pEnemyComponent80_0xd34 != (EnemyComponent80*)0x0) {
			FUN_003c1350((int*)this->pEnemyComponent80_0xd34, m0);
		})
	}
	else {
		if ((((iVar1 == 1) || (iVar1 == 0)) && (pEVar2 = this->pEnemyComponent80_0xd34, pEVar2 != (EnemyComponent80*)0x0)) && (pEVar2->pEnemy_0x60 == this)) {
			IMPLEMENTATION_GUARD(
			FUN_003c2980((int*)pEVar2, m0);)
		}
	}

	return true;
}

bool CActorWolfen::IsMakingNoise()
{
	int iVar1;
	bool bVar2;
	StateConfig* pSVar2;

	iVar1 = this->actorState;
	if (iVar1 == 6) {
		bVar2 = true;
	}
	else {
		bVar2 = (GetStateFlags(iVar1) & 2);
	}

	return bVar2;
}

CVision* CActorWolfen::GetVision()
{
	return &vision;
}

int CActorWolfen::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	//CBehaviourMovingPlatformVTable* pCVar1;
	edColPRIM_OBJECT* peVar2;
	CActorFighter* pCVar3;
	CActorCommander* pCVar4;
	CEventManager* pCVar5;
	bool bVar6;
	char cVar7;
	CBehaviour* pCVar8;
	CLifeInterface* pCVar9;
	CActorHero* pCVar10;
	ed_zone_3d* peVar11;
	uint uVar12;
	CVision* pCVar13;
	CActor* pCVar14;
	CPathFinderClient* pPathFindingClient;
	int iVar15;
	long lVar16;
	undefined8 uVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	edF32VECTOR4 local_130;
	edF32VECTOR4 local_120;
	edF32VECTOR4 local_110;
	float local_100;
	float fStack252;
	float fStack248;
	float fStack244;
	edF32VECTOR4 local_f0;
	float local_e0;
	float fStack220;
	float local_d8;
	float fStack212;
	float local_d0;
	float local_cc;
	float local_c8;
	float local_c4;
	edF32VECTOR4 local_c0;
	float local_b0;
	float local_ac;
	float local_a8;
	float local_a4;
	edF32VECTOR4 local_a0;
	undefined4 auStack144[8];
	edF32VECTOR4 aeStack112[6];
	undefined4* local_4;

	if (msg == 0x25) {
		IMPLEMENTATION_GUARD(
		FUN_001fbaf0(this, 0);)
	LAB_0017c6e8:
		iVar15 = CActorFighter::InterpretMessage(pSender, msg, pMsgParam);
		return iVar15;
	}

	if (msg == 0x26) {
		IMPLEMENTATION_GUARD(
		FUN_001fbaf0(this, 1);
		goto LAB_0017c6e8;)
	}

	if (msg != 0x1a) {
		if (msg == 0x3f) {
			return 2;
		}

		if (msg == 99) {
			IMPLEMENTATION_GUARD(
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x8000;)
			goto LAB_0017c6e8;
		}

		if (msg == 0x3a) {
			bVar6 = false;
			if (pSender->IsMakingNoise() != 0) {
				
				local_b0 = this->currentLocation.x;
				local_a8 = this->currentLocation.z;
				local_a4 = this->currentLocation.w;
				local_ac = this->currentLocation.y + this->field_0xcf0;
				pSender->SV_GetActorColCenter(&local_c0);
				local_e0 = local_b0 - local_c0.x;
				fStack220 = local_ac - local_c0.y;
				local_d8 = local_a8 - local_c0.z;
				fStack212 = local_a4 - local_c0.w;
				if ((local_e0 * local_e0 + local_d8 * local_d8 < this->hearingDetectionProps.rangeSquared) &&
					(fabs(local_ac - local_c0.y) < this->hearingDetectionProps.maxHeightDifference)) {
					bVar6 = true;
				}
			}

			if (!bVar6) {
				iVar15 = this->actorState;
				uVar12 = 0;
	
				if ((iVar15 != -1) && (uVar12 = 0, 0x71 < iVar15)) {
					uVar12 = _gStateCfg_WLF[iVar15 + -0x72].field_0x8;
				}

				if ((uVar12 & 0x20) == 0) {
					GetVision()->location = this->currentLocation;
					GetVision()->rotationQuat = this->rotationQuat;
				}

				pCVar14 = GetVision()->ScanForTarget(pSender, 1);

				if (pCVar14 == (CActor*)0x0) {
					if (this->visionDetectionProps.field_0x0 <= sqrtf(edF32Vector4DotProductHard_I(&pSender->currentLocation, &this->currentLocation))) {
						if (!IsAlive(pSender)) {
							return 1;
						}

						EnterCombatState(pSender);

						UpdateCombatMode();

						return 1;
					}
				}
			}

		
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x100;

			if (pSender->typeID != 9) {
				if (pSender->typeID != 0x1c) {
					return 1;
				}
				this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x80;
				return 1;
			}

			if (!IsAlive(static_cast<CActor*>(pMsgParam))) {
				return 1;
			}

			EnterCombatState(static_cast<CActor*>(pMsgParam));

			UpdateCombatMode();

			return 1;
		}

		if (msg == MESSAGE_GET_RUN_SPEED) {
			float* pRunSpeedParam = reinterpret_cast<float*>(pMsgParam);
			*pRunSpeedParam = this->defaultRunSpeed;
			return 1;
		}

		if ((msg == 0x66) || (msg == 0x27)) {
			IMPLEMENTATION_GUARD(
			if ((this->field_0xb74 != 0) && (this->field_0xb74 != 1)) {
				return 0;
			}
			iVar15 = CActorFighter::InterpretMessage((CActorFighter*)this, pSender, msg, pMsgParam);
			return iVar15;)
		}

		if (msg == 0x86) {
			IMPLEMENTATION_GUARD(
			if ((this->field_0xb80 != 2) && (this->field_0xb80 != 0)) {
				(*(code*)(this->pVTable)->LifeAnnihilate)();
				SetBehaviour(0xe, 0x7f, -1);
			}
			return 1;)
		}

		if (msg != 2) {
			if (msg == 1) {
				IMPLEMENTATION_GUARD(
				pCVar9 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
				fVar18 = (*pCVar9->pVtable->GetValue)((CInterface*)pCVar9);
				if (0.0 < fVar18) {
					return 1;
				})
			}
			else {
				if (msg == 0x16) {
					IMPLEMENTATION_GUARD(
					this->field_0x6a0.x = g_xVector.x;
					this->field_0x6a0.y = g_xVector.y;
					this->field_0x6a0.z = g_xVector.z;
					this->field_0x6a0.w = g_xVector.w;
					this->field_0x6b0 = 0.001;
					SetBehaviour(4, -1, -1);
					return 1;)
				}
				if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
					GetPositionMsgParams* pGetPosMsgParam = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
					/* WARNING: Load size is inaccurate */
					if ((pGetPosMsgParam->field_0x0 == 1) || (pGetPosMsgParam->field_0x0 == 0)) {
						peVar2 = (this->pCollisionData)->pObbPrim;
						pGetPosMsgParam->vectorFieldB.x = 0.0f;
						pGetPosMsgParam->vectorFieldB.y = (peVar2->field_0x90).y + (peVar2->field_0xb0).y;
						pGetPosMsgParam->vectorFieldB.z = 0.0f;
						pGetPosMsgParam->vectorFieldB.w = 0.0f;
						return 1;
					}
				}
				else {
					if (msg == 0x36) {
						IMPLEMENTATION_GUARD(
						if (pMsgParam == (void*)0x0) {
							this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xffffdfff;
						}
						else {
							this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x2000;
						}
						return 1;)
					}

					if (msg == 3) {
						IMPLEMENTATION_GUARD(
						if (pSender->typeID == 0x33) {
							pCVar8 = CActor::GetBehaviour((CActor*)this, 0x18);
							if ((pCVar8 != (CBehaviour*)0x0) && (this->curBehaviourId != 4)) {
								pCVar8 = CActor::GetBehaviour((CActor*)this, 0x18);
								FUN_001ec780((int)pCVar8, auStack144, (int)pSender);
								local_4 = auStack144;
								CActor::DoMessage(pSender, (CActor*)this, 2, (uint)local_4);
								CActorFighter::PlayImpactFx
								((CActorFighter*)this,
									(long)(int)&(this->pCollisionData)->field_0x90, aeStack112, 1,
									'\0');
							}
						}
						else {
							(*(code*)(this->pVTable)->LifeAnnihilate)();
							pCVar1 = (CBehaviourMovingPlatformVTable*)this->curBehaviourId;
							pCVar8 = CActor::GetBehaviour((CActor*)this, 0xe);
							pCVar8[2].pVTable = pCVar1;
							SetBehaviour(4, 0x5a, -1);
						})
					}
				}
			}

			goto LAB_0017c6e8;
		}

		IMPLEMENTATION_GUARD(
		/* WARNING: Load size is inaccurate */
		if ((*pMsgParam != 8) && (*pMsgParam != 7)) goto LAB_0017ba38;
		this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x100;
		bVar6 = (*pSender->pVTable->IsKindOfObject)(pSender, 8);
		if (bVar6 == false) {
			return 1;
		}
		if (this->field_0x350 == (CActor*)0x0) {
			(*(code*)this->pVTable[1].actorBase)();
		}

		if (IsAlive(pSender)) {
			EnterCombatState(pSender);
			
			UpdateCombatMode();
		}
		
		iVar15 = this->actorState;
		if ((iVar15 == 0xa1) || (iVar15 == 0xa2)) {
			edF32Vector4GetNegHard(&local_a0, &this->rotationQuat);
			this->rotationQuat.x = local_a0.x;
			this->rotationQuat.y = local_a0.y;
			this->rotationQuat.z = local_a0.z;
			this->rotationQuat.w = local_a0.w;
		}
		iVar15 = CActorFighter::InterpretMessage((CActorFighter*)this, pSender, 2, pMsgParam);
		return iVar15;)
	}

	InitPathfindingClientMsgParams* pMsgParams = reinterpret_cast<InitPathfindingClientMsgParams*>(pMsgParam);
	/* WARNING: Load size is inaccurate */
	switch (pMsgParams->msgId) {
	case 0:
		IMPLEMENTATION_GUARD(
		SetBehaviour(*(int*)((int)pMsgParam + 4), -1, -1);)
		break;
	case 1:
		pCVar3 = pMsgParams->pActor;
		this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x10;
		this->pTargetActor_0xc80 = pCVar3;
		break;
	case 2:
		this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xffffffef;
		if ((this->combatFlags_0xb78 & 0x20) == 0) {
			this->pTargetActor_0xc80 = (CActorHero*)0x0;
		}
		break;
	case 3:
		pCVar3 = pMsgParams->pActor;
		if (this->field_0xd30 == -1) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x20;
			this->pTargetActor_0xc80 = pCVar3;
		}
		break;
	case 4:
		if ((this->field_0xd30 == -1) &&
			(this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xffffffdf, (this->combatFlags_0xb78 & 0x10) == 0)) {
			this->pTargetActor_0xc80 = (CActorHero*)0x0;
		}
		break;
	case 5:
		pCVar3 = pMsgParams->pActor;

		if (!IsAlive(pCVar3)) {
			return 1;
		}

		EnterCombatState(pCVar3);

		UpdateCombatMode();
		break;
	case 6:
		if (this->field_0xd30 == -1) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x400;
		}
		break;
	case 7:
		if (this->field_0xd30 == -1) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffffbff;
		}
		break;
	case 8:
		this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x200;
		break;
	case 9:
		this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffffdff;
		break;
	case 10:
	{
		pPathFindingClient = GetPathfinderClient();
		pPathFindingClient->Init();
		pPathFindingClient->ChangePathfindingId(pMsgParams->pActor, pMsgParams->newId, &this->currentLocation);
	}
		break;
	case 0xf:
		if (this->actorState == 6) {
			SetState(0xad, -1);
		}
	}

	return 1;
}

float CActorWolfen::GetWalkSpeed()
{
	return this->walkSpeed;
}

float CActorWolfen::GetWalkRotSpeed()
{
	return this->walkRotSpeed;
}

float CActorWolfen::GetWalkAcceleration()
{
	return this->walkAcceleration;
}

float CActorWolfen::GetRunSpeed()
{
	return this->runSpeed;
}

float CActorWolfen::GetRunRotSpeed()
{
	return this->rotRunSpeed;
}

float CActorWolfen::GetRunAcceleration()
{
	return this->runAcceleration;
}

CPathFinderClient* CActorWolfen::GetPathfinderClient()
{
	return &pathFinderClient;
}

CPathFinderClient* CActorWolfen::GetPathfinderClientAlt()
{
	return &pathFinderClient;
}

bool CActorWolfen::Func_0x19c()
{
	bool uVar1;

	uVar1 = false;
	if (this->pInputAnalyser != (CInputAnalyser*)0x0) {
		uVar1 = CActorFighter::Func_0x19c();
	}

	return uVar1;
}

bool CActorWolfen::Func_0x1ac()
{
	IMPLEMENTATION_GUARD();
}

void CActorWolfen::_Std_OnFightActionSuccess()
{
	_msg_fight_action_success_params params;

	params.field_0x0 = 1;
	params.pAdversary = this->pAdversary;
	DoMessage(this->pCommander, MESSAGE_FIGHT_ACTION_SUCCESS, &params);

	return;
}

void CActorWolfen::Func_0x204(CActorFighter* pOther)
{
	CActorFighter* pCVar1;
	int iVar2;
	float fVar3;
	edF32VECTOR4 eStack32;
	float adversaryRunSpeed;

	SetAdversary(pOther);

	this->runSpeed = 0.0f;
	pCVar1 = this->pAdversary;
	if (pCVar1 == (CActorFighter*)0x0) {
		this->adversaryDistance = this->field_0x358;
		this->adversaryAngleDiff = GetAngleYFromVector(&this->rotationQuat);
		SetRunSpeed(this->defaultRunSpeed);
	}
	else {
		edF32Vector4SubHard(&eStack32, &pCVar1->currentLocation, &this->currentLocation);
		this->adversaryAngleDiff = GetAngleYFromVector(&eStack32);
		this->adversaryDistance = edF32Vector4GetDistHard(&eStack32);
		if (DoMessage(this->pAdversary, MESSAGE_GET_RUN_SPEED, &adversaryRunSpeed) != 0) {
			SetRunSpeed(adversaryRunSpeed * this->runSpeedScale);
		}
	}
	return;
}

bool CActorWolfen::AcquireAdversary(CActorFighter* pTarget)
{
	float fVar1;
	bool bVar2;
	CBehaviour* pCVar3;
	bool bSuccess;
	float fVar6;

	bSuccess = false;
	if (((pTarget == CActorHero::_gThis) && (this->pTargetActor_0xc80 == (CActorFighter*)0x0)) && ((~this->combatFlags_0xb78 & 0x30) == 0x30)) {
		bSuccess = true;
	}
	else {
		if ((this->pTargetActor_0xc80 == pTarget) && ((int)this->combatMode_0xb7c < 1)) {
			bSuccess = true;
		}
		else {
			if ((this->combatFlags_0xb78 & 0x200) == 0) {
				bSuccess = true;
			}
			else {
				fVar6 = pTarget->currentLocation.x - this->currentLocation.x;
				fVar1 = pTarget->currentLocation.z - this->currentLocation.z;
				bVar2 = this->field_0xb94 <= sqrtf(fVar6 * fVar6 + 0.0f + fVar1 * fVar1);
				if (!bVar2) {
					bVar2 = this->field_0xb98 < fabs((this->distanceToGround + this->currentLocation.y) - (pTarget->distanceToGround + pTarget->currentLocation.y));
				}
				if (bVar2) {
					bSuccess = true;
				}
				else {
					bVar2 = false;
					if (((pTarget != (CActorFighter*)0x0) && (pTarget->typeID == ACTOR_HERO_PRIVATE)) && (pTarget->curBehaviourId == 8)) {
						bVar2 = true;
					}

					if ((bVar2) && (pCVar3 = GetBehaviour(0x18), pCVar3 != (CBehaviour*)0x0)) {
						bSuccess = true;
					}
					else {
						if (((this->combatFlags_0xb78 & 7) == 0) && (bVar2 = SV_AUT_CanMoveTo(&pTarget->currentLocation), bVar2 == false)) {
							bSuccess = true;
						}
						else {
							if (pTarget->GetLifeInterface()->GetValue() <= 0.0f) {
								bSuccess = true;
							}
						}
					}
				}
			}
		}
	}

	return bSuccess;
}

void CActorWolfen::SetRunSpeed(float param_1)
{
	this->runSpeed = param_1;

	return;
}

bool CActorWolfen::IsCurrentPositionValid()
{
	bool bValid;

	if ((this->combatFlags_0xb78 & 0x400) == 0) {
	LAB_00173a88:
		bValid = false;
	}
	else {
		if (GetPathfinderClientAlt()->id != -1) {
			bValid = GetPathfinderClientAlt()->IsValidPosition(&this->currentLocation);
			if (bValid == false) goto LAB_00173a88;
		}

		bValid = true;
	}

	return bValid;
}

void CActorWolfen::SetCombatMode(EEnemyCombatMode newCombatMode)
{
	if (this->combatMode_0xb7c != newCombatMode) {
		this->combatMode_0xb7c = newCombatMode;
	}

	return;
}

uint CActorWolfen::GetStateWolfenFlags(int state)
{
	uint uVar1;

	uVar1 = 0;
	if (state != -1) {
		if (state < 0x72) {
			uVar1 = 0;
		}
		else {
			uVar1 = _gStateCfg_WLF[state + -0x72].field_0x8;
		}
	}

	return uVar1;
}

void CActorWolfen::AnimEvaluate_0xa2(uint layerId)
{
	edANM_HDR* peVar1;
	int index;
	edAnmLayer* peVar2;
	CAnimation* pAnim;

	pAnim = this->pAnimationController;
	index = pAnim->PhysicalLayerFromLayerId(layerId);
	peVar2 = (pAnim->anmBinMetaAnimator).aAnimData + index;

	peVar2->field_0x4 = this->field_0xd24; // This can be read before it is initialized... Might be worth fixing

	if (0.0f < this->field_0xd24) {
		pAnim->anmBinMetaAnimator.SetLayerTimeWarper(this->field_0xd24, index);

		peVar1 = (peVar2->currentAnimDesc).state.pAnimKeyTableEntry;
		if ((peVar1->keyIndex_0x8.asKey == 2) && (peVar1->field_0x4.asKey == 1)) {
			char* pBase = (char*)peVar1;
			AnimKeySomething* pValue = (AnimKeySomething*)(pBase + (peVar2->currentAnimDesc).state.pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

			pValue->field_0xc = 1.0f - this->field_0xd28;
			pValue->field_0x10 = this->field_0xd28;
		}
	}

	return;
}

void CActorWolfen::AnimEvaluate_0017c930(uint layerId, edAnmMacroAnimator* pAnimator)
{
	edANM_HDR* peVar1;
	CBehaviour* pCVar2;
	uint uVar3;
	int iVar4;
	float fVar5;
	uint uVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	peVar1 = pAnimator->pAnimKeyTableEntry;
	if (1 < peVar1->keyIndex_0x8.asKey) {
		CActor::SV_GetActorColCenter(&eStack32);
		iVar4 = this->curBehaviourId;
		if ((((iVar4 == WOLFEN_BEHAVIOUR_TRACK_WEAPON_STAND) || (iVar4 == WOLFEN_BEHAVIOUR_TRACK_WEAPON)) || (iVar4 == WOLFEN_BEHAVIOUR_TRACK_WEAPON_SNIPE)) || (iVar4 == WOLFEN_BEHAVIOUR_SNIPE)) {
			CBehaviourWolfen* pWolfenBehaviour = static_cast<CBehaviourWolfen*>(GetBehaviour(iVar4));
			edF32Vector4SubHard(&local_10, &pWolfenBehaviour->rotationEuler, &eStack32);
		}
		else {
			if (this->pTargetActor_0xc80 == (CActorFighter*)0x0) {
				local_10 = this->rotationQuat;
			}
			else {
				this->pTargetActor_0xc80->SV_GetActorColCenter(&eStack48);
				edF32Vector4SubHard(&local_10, &eStack48, &eStack32);
			}
		}

		edF32Vector4NormalizeHard(&local_10, &local_10);
		fVar7 = edF32Vector4DotProductHard(&local_10, &g_xVector);
		uVar3 = peVar1->keyIndex_0x8.asKey - 1;
		fVar7 = 0.5f - fVar7 * 0.5f;
		if ((int)uVar3 < 0) {
			fVar8 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar8 = fVar8 + fVar8;
		}
		else {
			fVar8 = (float)uVar3;
		}

		fVar8 = 1.0f / fVar8;
		fVar5 = fVar7 / fVar8;
		if (fVar5 < 2.147484e+09f) {
			uVar6 = (uint)fVar5;
		}
		else {
			uVar6 = (int)(fVar5 - 2.147484e+09f) | 0x80000000;
		}

		if (peVar1->keyIndex_0x8.asKey <= uVar6) {
			uVar6 = uVar3;
		}

		if ((int)uVar6 < 0) {
			fVar5 = (float)(uVar6 >> 1 | uVar6 & 1);
			fVar5 = fVar5 + fVar5;
		}
		else {
			fVar5 = (float)uVar6;
		}

		uVar3 = uVar6 + 1;
		if ((int)uVar3 < 0) {
			fVar9 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar9 = fVar9 + fVar9;
		}
		else {
			fVar9 = (float)uVar3;
		}

		char* pBase = (char*)peVar1;
		AnimKeySomething* pValue = (AnimKeySomething*)(pBase + peVar1->keyIndex_0x8.asKey * 4);

		fVar7 = edFIntervalLERP(fVar7, fVar5 * fVar8, fVar9 * fVar8, 0.0f, 1.0f);
		uVar3 = 0;
		if (peVar1->keyIndex_0x8.asKey != 0) {
			do {
				pValue->field_0xc_array[uVar3];
				uVar3 = uVar3 + 1;
			} while (uVar3 < peVar1->keyIndex_0x8.asKey);
		}

		pValue->field_0xc_array[uVar6] = fVar7;
		pValue->field_0xc_array[uVar6] = (1.0f - fVar7);
	}

	return;
}

void CActorWolfen::Create_FightParam(ByteCode* pByteCode)
{
	uint uVar1;
	uint uVar2;
	void* pvVar3;
	uint uVar4;
	int uVar5;
	CActorWolfenKnowledge* pKnowledge;
	undefined8 uVar6;
	int iVar7;
	char* pcVar8;
	WolfenComboData* puVar9;
	uint uVar10;
	int iVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	uint local_2c0;
	uint local_2b0;
	uint local_290;
	WolfenComboData local_280[40];

	pByteCode->GetU32();
	this->aCapabilities[0].semaphoreId = 0;
	this->aCapabilities[0].field_0x4 = pByteCode->GetU32();
	this->aCapabilities[0].field_0x8 = pByteCode->GetU32();
	this->aCapabilities[0].field_0xc = pByteCode->GetU32();
	this->aCapabilities[0].field_0x10 = pByteCode->GetU32();
	this->aCapabilities[0].field_0x14 = pByteCode->GetU32();

	this->aCapabilities[2].semaphoreId = 1;
	this->aCapabilities[2].field_0x4 = pByteCode->GetU32();
	this->aCapabilities[2].field_0x8 = pByteCode->GetU32();
	this->aCapabilities[2].field_0xc = pByteCode->GetU32();
	this->aCapabilities[2].field_0x10 = pByteCode->GetU32();
	this->aCapabilities[2].field_0x14 = pByteCode->GetU32();
	
	memcpy(this->aCapabilities + 1, this->aCapabilities, sizeof(WFIGS_Capability));

	this->field_0xb30 = 3;
	this->activeCapabilityIndex = 3;
	this->field_0xa80 = pByteCode->GetF32();
	this->field_0xa84 = pByteCode->GetF32();
	this->field_0xa88 = pByteCode->GetF32();
	this->field_0xa8c = pByteCode->GetF32();

	local_280[2].field_0x0 = 0x20000;
	local_280[2].field_0x4 = 2;
	local_280[3].field_0x4 = 3;
	local_280[0].field_0x0 = 1;
	local_280[4].field_0x4 = 4;
	local_280[0].field_0x8 = 1;
	local_280[5].field_0x4 = 5;
	local_280[1].field_0x8 = -1;
	local_280[6].field_0x0 = 0x40000;
	local_280[1].field_0x4 = 1;
	local_280[6].field_0x4 = 6;
	local_280[2].field_0x8 = -1;
	local_280[7].field_0x0 = 0x4000;
	local_280[3].field_0x0 = 0x80000;
	local_280[7].field_0x4 = 7;
	local_280[3].field_0x8 = -1;
	local_280[8].field_0x0 = 0x400;
	local_280[4].field_0x0 = 0x80000;
	local_280[9].field_0x0 = 0x10000;
	local_280[5].field_0x0 = 0x80000;
	local_280[9].field_0x4 = 9;
	local_280[4].field_0x8 = -1;
	local_280[5].field_0x8 = -1;
	local_280[6].field_0x8 = -1;
	local_280[7].field_0x8 = 8;
	local_280[8].field_0x4 = 8;
	local_280[7].field_0xc = 1;
	local_280[8].field_0xc = 1;
	local_280[8].field_0x8 = -1;
	local_280[9].field_0x8 = -1;
	local_280[0].field_0x4 = 0;
	local_280[0].field_0xc = 0;
	local_280[1].field_0x0 = 0;
	local_280[1].field_0xc = 0;
	local_280[2].field_0xc = 0;
	local_280[3].field_0xc = 0;
	local_280[4].field_0xc = 0;
	local_280[5].field_0xc = 0;
	local_280[6].field_0xc = 0;
	local_280[9].field_0xc = 0;

	uint nbComboItems = 10;
	this->aCapabilities[0].Create(pByteCode, local_280 + nbComboItems, this->field_0xa8c, &nbComboItems);
	this->aCapabilities[1].Create(pByteCode, local_280 + nbComboItems, this->field_0xa8c, &nbComboItems);
	this->aCapabilities[2].Create(pByteCode, local_280 + nbComboItems, this->field_0xa8c, &nbComboItems);

	uVar4 = pByteCode->GetU32();
	uVar2 = nbComboItems;
	if (uVar4 == 0) {
		uVar2 = 0;
	}

	uVar10 = 0;
	if (uVar4 != 0) {
		puVar9 = local_280 + nbComboItems;
		do {
			puVar9->field_0x0 = pByteCode->GetU32();
			if (uVar10 < uVar4 - 1) {
				uVar5 = nbComboItems + 1;
			}
			else {
				uVar5 = -1;
			}

			puVar9->field_0x8 = uVar5;
			uVar10 = uVar10 + 1;
			puVar9->field_0x4 = nbComboItems;
			puVar9 = puVar9 + 1;
			nbComboItems = nbComboItems + 1;
		} while (uVar10 < uVar4);
	}

	this->field_0xb34 = uVar4;
	this->field_0xb38 = uVar2;

	uVar4 = pByteCode->GetU32();
	uVar2 = nbComboItems;
	if (uVar4 == 0) {
		uVar2 = 0;
	}

	uVar10 = 0;
	if (uVar4 != 0) {
		puVar9 = local_280 + nbComboItems;
		do {
			puVar9->field_0x0 = pByteCode->GetU32();
			if (uVar10 < uVar4 - 1) {
				uVar5 = nbComboItems + 1;
			}
			else {
				uVar5 = -1;
			}

			puVar9->field_0x8 = uVar5;
			uVar10 = uVar10 + 1;
			puVar9->field_0x4 = nbComboItems;
			puVar9 = puVar9 + 1;
			nbComboItems = nbComboItems + 1;
		} while (uVar10 < uVar4);
	}

	this->field_0xb3c = uVar4;
	this->field_0xb40 = uVar2;

	pByteCode->GetU32();
	this->field_0xb44 = pByteCode->GetU32();
	this->field_0xb48 = pByteCode->GetF32();
	this->field_0xb4c = pByteCode->GetF32();
	this->field_0xb50 = pByteCode->GetF32();
	this->field_0xb54 = pByteCode->GetF32();
	this->field_0xb58 = pByteCode->GetF32();
	this->field_0xb5c = pByteCode->GetF32();
	this->field_0xb60 = nbComboItems;
	
	assert(nbComboItems < 40);
	this->field_0xb64 = new WolfenComboData[nbComboItems];
	memcpy(this->field_0xb64, local_280, nbComboItems * sizeof(WolfenComboData));

	uVar1 = pByteCode->GetU32();
	uVar2 = pByteCode->GetU32();
	uVar4 = pByteCode->GetU32();
	uVar10 = pByteCode->GetU32();
	if (uVar2 == 0) {
		this->pWolfenKnowledge = (CActorWolfenKnowledge*)0x0;
	}
	else {
		this->pWolfenKnowledge = new CActorWolfenKnowledge;
		this->pWolfenKnowledge->Init(uVar1, uVar4, uVar10, uVar2, 0x80);
	}

	return;
}

void CActorWolfen::ManageKnowledge()
{
	s_fighter_combo* pFighterCombo;
	float fVar1;
	float fVar2;
	bool bVar3;
	CActorFighter* pAdv;
	CActorWolfenKnowledge* pKnowledge;

	if (((this->pWolfenKnowledge != (CActorWolfenKnowledge*)0x0) &&
		(pAdv = this->pAdversary, pAdv != (CActorFighter*)0x0)) && ((CActorWolfen*)pAdv->pAdversary == this)) {
		IMPLEMENTATION_GUARD(
		if (((pAdv->pFighterCombo != (s_fighter_combo*)0x0) &&
			(bVar3 = CActorFighter::FUN_0031b790(pAdv, (pAdv->characterBase).base.base.actorState), bVar3 != false)) &&
			(pFighterCombo = pAdv->pFighterCombo,
				fVar1 = (pAdv->characterBase).base.base.currentLocation.x -
				this->currentLocation.x,
				fVar2 = (pAdv->characterBase).base.base.currentLocation.z -
				this->currentLocation.z,
				fVar1 * fVar1 + fVar2 * fVar2 <=
				*(float*)(pFighterCombo->actionHash + 0x4c) * *(float*)(pFighterCombo->actionHash + 0x4c) * 1.25)) {
			pKnowledge = this->pWolfenKnowledge;
			if (pKnowledge->field_0x1c == 0) {
				CActorWolfenKnowledge::BeginMemory(pKnowledge, pFighterCombo);
			}
			else {
				CActorWolfenKnowledge::NextStage(pKnowledge, pFighterCombo);
			}

			if (this->pWolfenKnowledge->field_0x2c == 1) {
				this->field_0xb70 = 0;
			}
		}

		if ((this->pWolfenKnowledge->field_0x1c != 0) &&
			(bVar3 = CActorFighter::FUN_0031b790(pAdv, (pAdv->characterBase).base.base.actorState), bVar3 == false)) {
			CActorWolfenKnowledge::EndMemory(this->pWolfenKnowledge);
		})
	}

	return;
}

void CActorWolfen::BehaviourStand_Manage(CBehaviourWolfen* pBehaviour)
{
	CActorWolfen* pCVar1;
	CActorFighter* pCVar2;
	bool bVar3;
	uint uVar4;
	CLifeInterface* pCVar5;
	int iVar6;
	int iVar7;
	float fVar8;

	iVar6 = this->actorState;
	if (iVar6 == 0x72) {
	LAB_001f46f0:
		StateStandGuard();
	}
	else {
		if (iVar6 == 0x9c) {
			SetState(0x72, -1);
			goto LAB_001f46f0;
		}
	}

	pCVar1 = pBehaviour->pOwner;
	if (((pCVar1->combatFlags_0xb78 & 0x800) != 0) &&
		(uVar4 = pCVar1->GetStateWolfenFlags(pCVar1->actorState), (uVar4 & 2) == 0)) {
		pBehaviour->pOwner->combatFlags_0xb78 = pBehaviour->pOwner->combatFlags_0xb78 | 0x1000;
	}

	uVar4 = pBehaviour->pOwner->GetStateWolfenFlags(pBehaviour->pOwner->actorState);
	if ((uVar4 & 1) == 0) goto LAB_001f47e8;

	if ((pBehaviour->pOwner->combatFlags_0xb78 & 0x10) == 0) {
	LAB_001f47b8:
		pBehaviour->pOwner->combatFlags_0xb78 = pBehaviour->pOwner->combatFlags_0xb78 & 0xfffffff8;
	}
	else {
		pCVar2 = pBehaviour->pOwner->pTargetActor_0xc80;
		fVar8 = pCVar2->GetLifeInterface()->GetValue();
		if (fVar8 <= 0.0f) goto LAB_001f47b8;

		pBehaviour->CheckDetection();
	}

	pBehaviour->pOwner->UpdateInRange_001744a0((pBehaviour->pOwner->combatFlags_0xb78 & 7) != 0);

LAB_001f47e8:
	if (((~pBehaviour->pOwner->combatFlags_0xb78 & 0x1800) == 0x1800) &&
		(iVar6 = pBehaviour->switchBehaviour.Test(pBehaviour->pOwner), iVar6 != 0)) {
		pBehaviour->pOwner->combatFlags_0xb78 = pBehaviour->pOwner->combatFlags_0xb78 | 0x800;
	}

	pCVar1 = pBehaviour->pOwner;
	iVar6 = -1;
	if ((pCVar1->combatFlags_0xb78 & 0x20000) == 0) {
		if (((pCVar1->pCollisionData)->flags_0x4 & 2) != 0) {
			pCVar1->combatFlags_0xb78 = pCVar1->combatFlags_0xb78 | 0x20000;
		}
	}
	else {
		if ((pCVar1->fightFlags & 2) != 0) {
			iVar6 = pCVar1->FUN_0030a6a0();
		}
	}

	if (iVar6 == -1) {
		pCVar1 = pBehaviour->pOwner;
		iVar6 = -1;
		if (((((~pCVar1->combatFlags_0xb78 & 0x30) == 0x30) &&
			(pCVar1->curBehaviourId != (pCVar1->subObjA)->defaultBehaviourId)) &&
			(pCVar2 = pCVar1->pAdversary, iVar6 = -1, pCVar2 != (CActorFighter*)0x0)) &&
			(bVar3 = pCVar2->IsKindOfObject(0x10), bVar3 == false))
		{
			pBehaviour->pOwner->combatFlags_0xb78 = pBehaviour->pOwner->combatFlags_0xb78 & 0xffffe7ff;
			pBehaviour->pOwner->SetCombatMode(ECM_None);
			iVar6 = pBehaviour->pOwner->subObjA->defaultBehaviourId;
		}

		iVar7 = -1;
		if (iVar6 == -1) {
			pCVar1 = pBehaviour->pOwner;
			if ((pCVar1->combatFlags_0xb78 & 0x1000) != 0) {
				pCVar1->combatFlags_0xb78 = pCVar1->combatFlags_0xb78 & 0xffffe7ff;
				iVar7 = pBehaviour->switchBehaviour.Execute(pBehaviour->pOwner);
			}

			if (iVar7 != -1) {
				SetBehaviour(iVar7, -1, -1);
			}
		}
		else {
			SetBehaviour(iVar6, -1, -1);
		}
	}
	else {
		SetBehaviour(iVar6, -1, -1);
	}

	return;
}

void CActorWolfen::BehaviourWatchDog_Manage(CBehaviourWatchDog* pBehaviour)
{
	CActorWolfen* pCVar1;
	CActorFighter* pCVar2;
	//CBehaviourMovingPlatformVTable* pCVar3;
	bool bVar4;
	uint uVar5;
	CLifeInterface* pCVar6;
	undefined4 uVar7;
	CBehaviour* pCVar8;
	S_TARGET_STREAM_REF* pSVar9;
	long lVar10;
	int iVar11;
	int iVar12;
	float fVar13;

	iVar12 = this->actorState;
	if (iVar12 == 0xa5) {
		IMPLEMENTATION_GUARD(
		StateWolfenBombShoot();)
	}
	else {
		if (iVar12 == 0xa4) {
			IMPLEMENTATION_GUARD(
			StateWolfenBombOrientTo(pBehaviour);)
		}
		else {
			if (iVar12 == 0xa3) {
				IMPLEMENTATION_GUARD(
				StateWolfenBombWalkTo(pBehaviour);)
			}
			else {
				if (iVar12 == 0xa2) {
					IMPLEMENTATION_GUARD(
					StateWolfenBombStand();)
				}
				else {
					if (iVar12 == 0xa1) {
						IMPLEMENTATION_GUARD(
						StateWolfenBombFlip();)
					}
					else {
						if (iVar12 == 0xa0) {
							IMPLEMENTATION_GUARD(
							StateWolfenInsultEnd(pBehaviour);)
						}
						else {
							if (iVar12 == 0x9f) {
								IMPLEMENTATION_GUARD(
								StateWolfenInsultReceive(pBehaviour);)
							}
							else {
								if (iVar12 == 0x9e) {
									IMPLEMENTATION_GUARD(
									StateWolfenInsultStand();)
								}
								else {
									if (iVar12 == 0x9d) {
										IMPLEMENTATION_GUARD(
										StateWolfenInsult(pBehaviour);)
									}
									else {
										if (iVar12 == 0x9b) {
											StateWolfen_00179db0(pBehaviour);
										}
										else {
											if (iVar12 == WOLFEN_STATE_SURPRISE) {
												StateWolfenSurprise(pBehaviour);
											}
											else {
												if (iVar12 == 0x98) {
													IMPLEMENTATION_GUARD(
													StateWolfenBoomyHit();)
												}
												else {
													if (iVar12 == WOLFEN_STATE_LOCATE) {
														StateWolfenLocate(pBehaviour);
													}
													else {
														if (iVar12 == 0x74) {
															IMPLEMENTATION_GUARD(
															ActorFunc_0035eac0();)
														}
														else {
															if (iVar12 == WOLFEN_STATE_COME_BACK) {
																StateWolfenComeBack(pBehaviour);
															}
															else {
																if (iVar12 == WOLFEN_STATE_WATCH_DOG_GUARD) {
																	StateWatchDogGuard(pBehaviour);
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	pCVar1 = pBehaviour->pOwner;
	if (((pCVar1->combatFlags_0xb78 & 0x800) != 0) &&
		(uVar5 = pCVar1->GetStateWolfenFlags(pCVar1->actorState), (uVar5 & 2) == 0)) {
		pBehaviour->pOwner->combatFlags_0xb78 = pBehaviour->pOwner->combatFlags_0xb78 | 0x1000;
	}

	uVar5 = pBehaviour->pOwner->GetStateWolfenFlags(pBehaviour->pOwner->actorState);

	if ((uVar5 & 1) == 0) goto LAB_001f42d8;

	if ((pBehaviour->pOwner->combatFlags_0xb78 & 0x10) == 0) {
	LAB_001f42a8:
		pBehaviour->pOwner->combatFlags_0xb78 = pBehaviour->pOwner->combatFlags_0xb78 & 0xfffffff8;
	}
	else {
		pCVar2 = pBehaviour->pOwner->pTargetActor_0xc80;
		fVar13 = pCVar2->GetLifeInterface()->GetValue();
		if (fVar13 <= 0.0f) goto LAB_001f42a8;

		pBehaviour->CheckDetection();
	}

	pBehaviour->pOwner->UpdateInRange_001744a0((pBehaviour->pOwner->combatFlags_0xb78 & 7) != 0);

LAB_001f42d8:
	bVar4 = (this->combatFlags_0xb78 & 4) != 0;
	if (bVar4 != pBehaviour->bool_0x68) {
		if ((bVar4) && (pBehaviour->bool_0x68 == false)) {
			for (int i = 0; i < pBehaviour->pTargetStreamRef->entryCount; i++) {
				pBehaviour->pTargetStreamRef->aEntries[i].SwitchOn(pBehaviour->pOwner);
			}

			pBehaviour->pCameraStreamEvent->SwitchOn(pCVar1);
		}

		if ((!bVar4) && (pBehaviour->bool_0x68 == true)) {
			for (int i = 0; i < pBehaviour->pTargetStreamRef->entryCount; i++) {
				pBehaviour->pTargetStreamRef->aEntries[i].SwitchOff(pBehaviour->pOwner);
			}
		}
	}

	pBehaviour->bool_0x68 = bVar4;
	if (((~pBehaviour->pOwner->combatFlags_0xb78 & 0x1800) == 0x1800) &&
		(iVar12 = pBehaviour->switchBehaviour.Test(pBehaviour->pOwner), iVar12 != 0)) {
		pBehaviour->pOwner->combatFlags_0xb78 = pBehaviour->pOwner->combatFlags_0xb78 | 0x800;
	}

	pCVar1 = pBehaviour->pOwner;
	iVar12 = -1;

	if ((pCVar1->combatFlags_0xb78 & 0x20000) == 0) {
		if (((pCVar1->pCollisionData)->flags_0x4 & 2) != 0) {
			pCVar1->combatFlags_0xb78 = pCVar1->combatFlags_0xb78 | 0x20000;
		}
	}
	else {
		if ((pCVar1->fightFlags & 2) != 0) {
			iVar12 = pCVar1->FUN_0030a6a0();
		}
	}

	if (iVar12 == -1) {
		pCVar1 = pBehaviour->pOwner;
		iVar12 = -1;
		if (((((~pCVar1->combatFlags_0xb78 & 0x30) == 0x30) && (pCVar1->curBehaviourId != (pCVar1->subObjA)->defaultBehaviourId)) &&
			(pCVar2 = pCVar1->pAdversary, iVar12 = -1, pCVar2 != (CActorFighter*)0x0)) && (bVar4 = pCVar2->IsKindOfObject(0x10), bVar4 == false)) {
			pBehaviour->pOwner->combatFlags_0xb78 = pBehaviour->pOwner->combatFlags_0xb78 & 0xffffe7ff;
			pBehaviour->pOwner->SetCombatMode(ECM_None);
			iVar12 = pBehaviour->pOwner->subObjA->defaultBehaviourId;
		}

		iVar11 = -1;
		if (iVar12 == -1) {
			pCVar1 = pBehaviour->pOwner;
			if ((pCVar1->combatFlags_0xb78 & 0x1000) != 0) {
				pCVar1->combatFlags_0xb78 = pCVar1->combatFlags_0xb78 & 0xffffe7ff;
				iVar11 = pBehaviour->switchBehaviour.Execute(pBehaviour->pOwner);
			}
			if (iVar11 == -1) {
				iVar12 = this->actorState;
				if (iVar12 == 0xb0) {
					SetBehaviour(3, -1, -1);
				}
				else {
					if (iVar12 == 0xb1) {
						SetBehaviour(pBehaviour->GetTrackBehaviour(), -1, -1);
					}
					else {
						if (iVar12 == 0xb4) {
							SetBehaviour(0x18, -1, -1);
						}
						else {
							if (iVar12 == 0xb5) {
								IMPLEMENTATION_GUARD(
								pCVar3 = (CBehaviourMovingPlatformVTable*)this->curBehaviourId;
								pCVar8 = CActor::GetBehaviour((CActor*)this, 0xe);
								pCVar8[2].pVTable = pCVar3;
								SetBehaviour(4, 0x5a, -1);)
							}
						}
					}
				}
			}
			else {
				SetBehaviour(iVar11, -1, -1);
			}
		}
		else {
			SetBehaviour(iVar12, -1, -1);
		}
	}
	else {
		SetBehaviour(iVar12, -1, -1);
	}

	return;
}

void CActorWolfen::BehaviourTrackWeapon_Manage(CBehaviourTrackWeapon* pBehaviour)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	CActorFighter* pCVar3;
	//CBehaviourMovingPlatformVTable* pCVar4;
	bool bVar5;
	int iVar6;
	CLifeInterface* pCVar7;
	CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* pCVar8;
	CBehaviour* pCVar9;
	float fVar10;

	iVar6 = this->actorState;

	if (iVar6 == 0x97) {
		this->dynamic.speed = 0.0f;
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		pCVar1 = this->pAnimationController;
		peVar2 = (pCVar1->anmBinMetaAnimator).aAnimData;
		bVar5 = false;

		if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
			if (peVar2->animPlayState == 0) {
				bVar5 = false;
			}
			else {
				bVar5 = (peVar2->field_0xcc & 2) != 0;
			}
		}

		if (bVar5) {
			SetState(this->field_0xcf8, -1);
		}
		else {
			iVar6 = pBehaviour->GetState_001f0930();
			if (iVar6 != -1) {
				SetState(iVar6, -1);
			}
		}
	}
	else {
		if (iVar6 == 0x96) {
			this->dynamic.speed = 0.0f;
			ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
			pCVar1 = this->pAnimationController;
			peVar2 = (pCVar1->anmBinMetaAnimator).aAnimData;
			bVar5 = false;
			if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
				if (peVar2->animPlayState == 0) {
					bVar5 = false;
				}
				else {
					bVar5 = (peVar2->field_0xcc & 2) != 0;
				}
			}
			if (bVar5) {
				SetState(this->field_0xcf8, -1);
			}
			else {
				iVar6 = pBehaviour->GetState_001f0930();
				if (iVar6 != -1) {
					SetState(iVar6, -1);
				}
			}
		}
		else {
			if (iVar6 == WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION) {
				StateTrackCheckPosition(pBehaviour);
			}
			else {
				if (iVar6 == 0xaa) {
					IMPLEMENTATION_GUARD(
					FUN_0035d950(this, (CBehaviourWolfen*)pBehaviour);)
				}
				else {
					if (iVar6 == 0xa9) {
						IMPLEMENTATION_GUARD(
						FUN_0035dbb0(this, (CBehaviourWolfen*)pBehaviour);)
					}
					else {
						if (iVar6 == WOLFEN_STATE_RELOAD) {
							StateTrackWeaponReload(pBehaviour);
						}
						else {
							if (iVar6 == WOLFEN_STATE_FIRE) {
								StateTrackWeaponStandFire(pBehaviour);
							}
							else {
								if (iVar6 == WOLFEN_STATE_AIM) {
									StateTrackWeaponAim(pBehaviour);
								}
								else {
									if (iVar6 == 0x78) {
										IMPLEMENTATION_GUARD(
										FUN_0035e560(this, (CBehaviourWolfen*)pBehaviour);)
									}
									else {
										if (iVar6 == WOLFEN_STATE_TRACK_DEFEND) {
											StateTrackWeaponDefend(pBehaviour);
										}
										else {
											if (iVar6 == 0x74) {
												IMPLEMENTATION_GUARD(
												ActorFunc_0035eac0(this);)
											}
											else {
												if (iVar6 == WOLFEN_STATE_TRACK_CHASE) {
													StateTrackWeaponChase(pBehaviour);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	pBehaviour->CheckDetection_Intruder();
	iVar6 = pBehaviour->FUN_001f0ab0();
	if (iVar6 == -1) {
		pCVar3 = this->pTargetActor_0xc80;
		if (pCVar3 != (CActorFighter*)0x0) {
			fVar10 = GetLifeInterface()->GetValue();
			if (fVar10 <= 0.0f) {
				SetBehaviour((this->subObjA)->defaultBehaviourId, -1, -1);
				return;
			}
		}
		iVar6 = this->actorState;
		if (iVar6 == 0xb3) {
			pCVar8 = pBehaviour->GetNotificationTargetArray();
			SetBehaviour(pCVar8->field_0x34, -1, -1);
		}
		else {
			if ((iVar6 == 0x9a) || (iVar6 == 0x9b)) {
				pCVar8 = pBehaviour->GetNotificationTargetArray();
				SetBehaviour(pCVar8->field_0x34, this->actorState, -1);
			}
			else {
				if (iVar6 == 0xb4) {
					SetBehaviour(0x18, -1, -1);
				}
				else {
					if (iVar6 == 0xb5) {
						IMPLEMENTATION_GUARD(
							pCVar4 = (CBehaviourMovingPlatformVTable*)this->curBehaviourId;
						pCVar9 = CActor::GetBehaviour((CActor*)this, 0xe);
						pCVar9[2].pVTable = pCVar4;
						SetBehaviour(4, 0x5a, -1););
					}
					else {
						if (iVar6 == 0xb0) {
							SetBehaviour(3, -1, -1);
						}
					}
				}
			}
		}
	}
	else {
		SetBehaviour(iVar6, -1, -1);
	}

	return;
}

void CActorWolfen::BehaviourTrackWeaponStand_Manage(CBehaviourTrackWeaponStand* pBehaviour)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	CActorFighter* pCVar3;
	//CBehaviourMovingPlatformVTable* pCVar4;
	bool bVar5;
	int iVar6;
	CLifeInterface* pCVar7;
	CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* pCVar8;
	CBehaviour* pCVar9;
	float fVar10;

	iVar6 = this->actorState;

	if (iVar6 == WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION) {
		StateTrackWeaponCheckPosition(pBehaviour);
	}
	else {
		if (iVar6 == 0x97) {
			this->dynamic.speed = 0.0f;
			ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
			pCVar1 = this->pAnimationController;
			peVar2 = (pCVar1->anmBinMetaAnimator).aAnimData;
			bVar5 = false;

			if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
				if (peVar2->animPlayState == 0) {
					bVar5 = false;
				}
				else {
					bVar5 = (peVar2->field_0xcc & 2) != 0;
				}
			}

			if (bVar5) {
				SetState(this->field_0xcf8, -1);
			}
			else {
				iVar6 = pBehaviour->GetState_001f0930();
				if (iVar6 != -1) {
					SetState(iVar6, -1);
				}
			}
		}
		else {
			if (iVar6 == 0x96) {
				this->dynamic.speed = 0.0f;
				ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
				pCVar1 = this->pAnimationController;
				peVar2 = (pCVar1->anmBinMetaAnimator).aAnimData;
				bVar5 = false;
				if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
					if (peVar2->animPlayState == 0) {
						bVar5 = false;
					}
					else {
						bVar5 = (peVar2->field_0xcc & 2) != 0;
					}
				}
				if (bVar5) {
					SetState(this->field_0xcf8, -1);
				}
				else {
					iVar6 = pBehaviour->GetState_001f0930();
					if (iVar6 != -1) {
						SetState(iVar6, -1);
					}
				}
			}
			else {
				if (iVar6 == WOLFEN_STATE_RELOAD) {
					StateTrackWeaponReload(pBehaviour);
				}
				else {
					if (iVar6 == WOLFEN_STATE_FIRE) {
						StateTrackWeaponStandFire(pBehaviour);
					}
					else {
						if (iVar6 == WOLFEN_STATE_AIM) {
							StateTrackStandAim(pBehaviour);
						}
						else {
							if (iVar6 == 0x78) {
								IMPLEMENTATION_GUARD(
									FUN_0035e560(this, (CBehaviourWolfen*)pBehaviour);)
							}
							else {
								if (iVar6 == WOLFEN_STATE_TRACK_DEFEND) {
									StateTrackWeaponStandDefend(pBehaviour);
								}
							}
						}
					}
				}
			}
		}
	}

	pBehaviour->CheckDetection_Intruder();
	iVar6 = pBehaviour->FUN_001f0ab0();
	if (iVar6 == -1) {
		pCVar3 = this->pTargetActor_0xc80;
		if (pCVar3 != (CActorFighter*)0x0) {
			fVar10 = GetLifeInterface()->GetValue();
			if (fVar10 <= 0.0f) {
				SetBehaviour((this->subObjA)->defaultBehaviourId, -1, -1);
				return;
			}
		}
		iVar6 = this->actorState;
		if (iVar6 == 0xb3) {
			pCVar8 = pBehaviour->GetNotificationTargetArray();
			SetBehaviour(pCVar8->field_0x34, -1, -1);
		}
		else {
			if ((iVar6 == 0x9a) || (iVar6 == 0x9b)) {
				pCVar8 = pBehaviour->GetNotificationTargetArray();
				SetBehaviour(pCVar8->field_0x34, this->actorState, -1);
			}
			else {
				if (iVar6 == 0xb4) {
					SetBehaviour(0x18, -1, -1);
				}
				else {
					if (iVar6 == 0xb5) {
						IMPLEMENTATION_GUARD(
							pCVar4 = (CBehaviourMovingPlatformVTable*)this->curBehaviourId;
						pCVar9 = CActor::GetBehaviour((CActor*)this, 0xe);
						pCVar9[2].pVTable = pCVar4;
						SetBehaviour(4, 0x5a, -1););
					}
					else {
						if (iVar6 == 0xb0) {
							SetBehaviour(3, -1, -1);
						}
					}
				}
			}
		}
	}
	else {
		SetBehaviour(iVar6, -1, -1);
	}

	return;
}

void CActorWolfen::BehaviourTrack_Manage(CBehaviourTrack* pBehaviour)
{
	int iVar1;
	float fVar4;
	CActorFighter* pTargetFighter;

	iVar1 = this->actorState;
	if (iVar1 == WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION) {
		StateTrackCheckPosition(pBehaviour);
	}
	else {
		if (iVar1 == 0xaa) {
			IMPLEMENTATION_GUARD(
			FUN_0035d950(this, (CBehaviourWolfen*)pBehaviour);)
		}
		else {
			if (iVar1 == 0xa9) {
				IMPLEMENTATION_GUARD(
				FUN_0035dbb0(this, (CBehaviourWolfen*)pBehaviour);)
			}
			else {
				if (iVar1 == 0x99) {
					IMPLEMENTATION_GUARD(
					FUN_0035e120(this);)
				}
				else {
					if (iVar1 == 0x78) {
						IMPLEMENTATION_GUARD(
						FUN_0035e560(this, (CBehaviourWolfen*)pBehaviour);)
					}
					else {
						if (iVar1 == WOLFEN_STATE_TRACK_DEFEND) {
							StateTrackDefend(pBehaviour);
						}
						else {
							if (iVar1 == 0x74) {
								IMPLEMENTATION_GUARD(
								FUN_0035eac0(this);)
							}
							else {
								if (iVar1 == WOLFEN_STATE_TRACK_CHASE) {
									StateTrackChase(pBehaviour);
								}
							}
						}
					}
				}
			}
		}
	}

	pBehaviour->CheckDetection_Intruder();

	iVar1 = pBehaviour->FUN_001f0ab0();
	if (iVar1 == -1) {
		pTargetFighter = this->pTargetActor_0xc80;
		if (pTargetFighter != (CActorFighter*)0x0) {
			if (pTargetFighter->GetLifeInterface()->GetValue() <= 0.0f) {
				SetBehaviour((this->subObjA)->defaultBehaviourId, -1, -1);
				return;
			}
		}

		if (this->actorState == 0xb3) {
			SetBehaviour(this->subObjA->defaultBehaviourId, -1, -1);
		}
		else {
			if ((this->actorState == 0x9a) || (this->actorState == 0x9b)) {
				SetBehaviour(pBehaviour->GetNotificationTargetArray()->field_0x34, this->actorState, -1);
			}
			else {
				if (this->actorState == 0xb4) {
					SetBehaviour(0x18, -1, -1);
				}
			}
		}
	}
	else {
		SetBehaviour(iVar1, -1, -1);
	}

	return;
}

void CActorWolfen::BehaviourFighterStd_Exit(CBehaviourFighterWolfen* pBehaviour)
{
	int iVar1;
	CActorHero* pCVar2;
	bool bVar3;
	bool bVar4;
	CActorFighter* pActor;
	StateConfig* pSVar5;
	CLifeInterface* pCVar6;
	long lVar7;
	uint uVar8;
	float fVar9;

	pActor = this->pCommander->GetIntruder();
	if ((((pActor != this->pAdversary) && ((this->combatFlags_0xb78 & 0x10) != 0)) &&
		((bVar4 = SV_WLF_IsIntruderMakingNoise(pActor), bVar4 != false ||
			((bVar4 = SV_WLF_IsIntruderInVitalSphere(pActor), bVar4 != false ||
				(bVar4 = SV_WLF_IsIntruderInVision(pActor), bVar4 != false)))))) &&
		(this->combatMode_0xb7c == ECM_None)) {
		SetCombatMode( ECM_InCombat);
	}

	uVar8 = GetStateFlags(this->actorState);

	bVar4 = (uVar8 & 0x100000) != 0;

	bVar3 = bVar4 && iVar1 != 0x50;
	if (bVar4 && iVar1 != 0x50) {
		bVar3 = iVar1 != 0x55;
	}

	if (bVar3) {
		bVar3 = iVar1 != 0x56;
	}

	if (bVar3) {
		pCVar2 = static_cast<CActorHero*>(this->pAdversary);
		if (pCVar2 == CActorHero::_gThis) {
			lVar7 = this->AcquireAdversary(pCVar2);
			if (lVar7 == 0) {
				bVar4 = this->pCommander->CanContinueToFight(this);
				if (bVar4 == false) {
					SetBehaviour(pBehaviour->behaviourId, -1, -1);
				}
			}
			else {
				SetBehaviour(pBehaviour->behaviourId, -1, -1);
			}
		}
		else {
			if (((int)this->combatMode_0xb7c < 1) && (pCVar2 != (CActorHero*)0x0)) {
				fVar9 = pCVar2->GetLifeInterface()->GetValue();
				if (0.0f < fVar9) {
					return;
				}
			}

			SetBehaviour(pBehaviour->behaviourId, -1, -1);
		}
	}

	return;
}

int CActorWolfen::_waitStandAnimArray[8] =
{
	0x96,
	0x97,
	0x98,
	0x99,
	0x9a,
	0x9b,
	-1
};

int CActorWolfen::_waitDefendAnimArray[4] =
{
	0x8e,
	0x8f,
	0x90,
	-1
};

void CActorWolfen::WaitingAnimation_Defend()
{
	int inAnimType;
	int iVar1;
	int iVar2;

	inAnimType = _waitDefendAnimArray[CScene::Rand() % 3];
	iVar1 = GetIdMacroAnim(inAnimType);
	if (iVar1 != -1) {
		iVar1 = rand();
		iVar2 = rand();
		PlayWaitingAnimation(((float)iVar2 / 2.147484e+09f) * 3.0f + 2.0f, ((float)iVar1 / 2.147484e+09f) * 0.7f + 0.8f, inAnimType, -1, 1);
	}
	return;
}

void CActorWolfen::WaitingAnimation_Guard()
{
	int newState = _waitStandAnimArray[(CScene::Rand() % 7)];
	int iVar1 = GetIdMacroAnim(newState);
	if (iVar1 != -1) {
		iVar1 = rand();
		int iVar2 = rand();
		PlayWaitingAnimation(((float)iVar2 / 2.147484e+09f) * 10.0f + 5.0f, ((float)iVar1 / 2.147484e+09f) * 0.7f + 0.8f, newState, -1, 1);
	}
}

void CActorWolfen::StateStandGuard()
{
	int inAnimType;
	int iVar1;
	int iVar2;

	this->dynamic.speed = 0.0f;
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
	inAnimType = _waitStandAnimArray[CScene::Rand() % 7];
	iVar1 = GetIdMacroAnim(inAnimType);
	if (iVar1 != -1) {
		iVar1 = rand();
		iVar2 = rand();
		PlayWaitingAnimation(((float)iVar2 / 2.147484e+09f) * 10.0f + 5.0f, ((float)iVar1 / 2.147484e+09f) * 0.7f + 0.8f, inAnimType, -1, 1);
	}

	return;
}

void CActorWolfen::StateTrackWeaponStandDefend(CBehaviourTrackWeaponStand* pBehaviour)
{
	CActorCommander* pCommander;
	float fVar2;
	float fVar3;
	bool bVar4;
	int iVar7;
	float fVar8;
	edF32VECTOR4 targetDirection;

	fVar8 = GetWalkRotSpeed();
	edF32Vector4SubHard(&targetDirection, &this->pCommander->targetPosition, &this->currentLocation);
	edF32Vector4NormalizeHard(&targetDirection, &targetDirection);
	bVar4 = SV_WLF_UpdateOrientation2D(fVar8, &targetDirection, 0);
	this->dynamic.speed = 0.0f;
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	if (0 < this->pCommander->field_0x194) {
		WaitingAnimation_Defend();
		pBehaviour->field_0xec = Timer::GetTimer()->scaledTotalTime;
	}

	if ((bVar4 != true) || (Timer::GetTimer()->scaledTotalTime - (float)pBehaviour->field_0xec <= 4.0f)) {
		pCommander = this->pCommander;
		if ((pCommander->field_0x194 < 1) ||
			(fVar8 = (pCommander->targetPosition).x - this->currentLocation.x,
				fVar2 = (pCommander->targetPosition).y - this->currentLocation.y,
				fVar3 = (pCommander->targetPosition).z - this->currentLocation.z,
				pBehaviour->field_0x90 <= sqrtf(fVar8 * fVar8 + fVar2 * fVar2 + fVar3 * fVar3))) {
			iVar7 = pBehaviour->GetState_001f0930();
			if (iVar7 != -1) {
				SetState(iVar7, -1);
			}
		}
		else {
			SetState(pBehaviour->Func_0x74(), -1);
		}
	}
	else {
		SetState(pBehaviour->Func_0x78(), -1);
	}

	return;
}

void CActorWolfen::StateTrackWeaponStandFire(CBehaviourTrackWeaponStand* pBehaviour)
{
	uint uVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	CActorWeapon* pCVar5;
	CActor* pCVar6;
	Timer* pTVar7;
	int iVar8;
	CActor* pOtherActor;
	long nextAnim;
	long nextState;
	float fVar11;
	float fVar12;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	nextState = -1;

	pOtherActor = this->pTargetActor_0xc80;
	if (pOtherActor != (CActor*)0x0) {
		fVar11 = GetRunRotSpeed();
		edF32Vector4SubHard(&eStack16, &pOtherActor->currentLocation, &this->currentLocation);
		edF32Vector4NormalizeHard(&eStack16, &eStack16);
		SV_WLF_UpdateOrientation2D(fVar11, &eStack16, 0);
		uVar1 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x0;

		pCVar5 = GetWeapon();
		SV_GetActorHitPos(pOtherActor, &local_20);

		if ((uVar1 & 1) != 0) {
			edF32Vector4SubHard(&eStack48, &local_20, &pCVar5->currentLocation);
			fVar11 = edF32Vector4GetDistHard(&eStack48);
			fVar12 = pCVar5->FUN_002d5710();
			pCVar6 = pOtherActor->GetCollidingActor();
			do {
				if (pCVar6 == pOtherActor) {
					pOtherActor = (CActor*)0x0;
				}
				else {
					bVar4 = pOtherActor->IsKindOfObject(2);
					if (bVar4 != false) {
						CActorMovable* pMovable = static_cast<CActorMovable*>(pOtherActor);
						edF32Vector4ScaleHard((fVar11 / fVar12) * pMovable->dynamic.linearAcceleration, &eStack96, &pMovable->dynamic.velocityDirectionEuler);
						edF32Vector4AddHard(&local_20, &local_20, &eStack96);
					}

					pOtherActor = pOtherActor->pTiedActor;
				}
			} while (pOtherActor != (CActor*)0x0);
		}

		fVar11 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x8;
		local_50 = local_20;

		if ((pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x0 & 2) != 0) {
			edF32Vector4SubHard(&eStack64, &local_50, &pBehaviour->field_0x80);
			fVar12 = edF32Vector4GetDistHard(&eStack64);
			pTVar7 = Timer::GetTimer();
			fVar11 = fVar11 * pTVar7->cutsceneDeltaTime;
			if (fVar11 <= fVar12) {
				fVar12 = fVar11;
			}

			fVar11 = edF32Vector4SafeNormalize0Hard(&eStack64, &eStack64);
			if (fVar11 != 0.0f) {
				edF32Vector4ScaleHard(fVar12, &eStack64, &eStack64);
				edF32Vector4AddHard(&local_50, &pBehaviour->field_0x80, &eStack64);
			}
		}

		pBehaviour->field_0x80 = local_50;
	}

	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	iVar8 = GetWeapon()->GetBurstState();
	if (iVar8 == 3) {
		nextState = 0x91;
		nextAnim = -1;
	}
	else {
		if (iVar8 == 1) {
			nextState = WOLFEN_STATE_AIM;
			nextAnim = 0xa8;
		}
		else {
			nextAnim = nextState;
			if (pBehaviour->field_0x94 == 2) {
				pCVar2 = this->pAnimationController;
				peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
				bVar4 = false;
				if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
					if (peVar3->animPlayState == 0) {
						bVar4 = false;
					}
					else {
						bVar4 = (peVar3->field_0xcc & 2) != 0;
					}
				}

				if (bVar4) {
					PlayAnim(0xaa);
					nextAnim = -1;
				}
			}
		}
	}
	if (nextState == -1) {
		iVar8 = pBehaviour->GetState_001f08a0();
		if (iVar8 != -1) {
			SetState(iVar8, -1);
		}
	}
	else {
		SetState(nextState, nextAnim);
	}

	return;
}

void CActorWolfen::StateTrackStandAim(CBehaviourTrackWeaponStand* pBehaviour)
{
	CActorCommander* pCVar1;
	uint uVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	float fVar5;
	bool bVar6;
	bool bVar7;
	bool bVar8;
	int iVar9;
	CActorWeapon* pWeapon;
	CActor* pCVar10;
	Timer* pTVar11;
	ulong uVar12;
	edF32VECTOR4* v2;
	CActor* pOtherActor;
	float fVar13;
	float fVar14;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	GetVision();
	pOtherActor = this->pTargetActor_0xc80;
	bVar7 = false;
	fVar13 = GetRunRotSpeed();
	edF32Vector4SubHard(&eStack32, &this->pCommander->targetPosition, &this->currentLocation);
	edF32Vector4NormalizeHard(&eStack32, &eStack32);
	bVar6 = SV_WLF_UpdateOrientation2D(fVar13, &eStack32, 0);

	if ((bVar6 == true) && (this->pCommander->field_0x194 < 1)) {
		SetState(pBehaviour->GetStateWolfenWeapon(), -1);
	}
	else {
		if (pOtherActor != (CActorFighter*)0x0) {
			int iVar8 = pBehaviour->GetState_001f0930();
			if (iVar8 == -1) {
				bVar6 = CanSwitchToFight_Area(pOtherActor);
				if (bVar6 == false) {
					bVar7 = true;
				}
				else {
					bVar6 = this->pCommander->BeginFightIntruder(this, pOtherActor);
					if (bVar6 != false) {
						Func_0x204(static_cast<CActorFighter*>(pOtherActor));
						bVar6 = SetBehaviour(3, -1, -1);
						if (bVar6 == false) {
							this->pCommander->EndFightIntruder(this);
						}
					}
				}
			}
			else {
				SetState(iVar8, -1);
			}
		}
	}

	if ((((bVar7) && ((this->combatFlags_0xb78 & 4) != 0)) && (pOtherActor != (CActorFighter*)0x0)) &&
		(pCVar1 = this->pCommander,
			fVar13 = (pCVar1->targetPosition).x - this->currentLocation.x,
			fVar14 = (pCVar1->targetPosition).y - this->currentLocation.y,
			fVar5 = (pCVar1->targetPosition).z - this->currentLocation.z,
			sqrtf(fVar13 * fVar13 + fVar14 * fVar14 + fVar5 * fVar5) <= pBehaviour->field_0x90)) {
		uVar2 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x0;
		pWeapon = GetWeapon();
		SV_GetActorHitPos(pOtherActor, &local_40);
		if ((uVar2 & 1) != 0) {
			edF32Vector4SubHard(&eStack80, &local_40, &pWeapon->currentLocation);
			fVar13 = edF32Vector4GetDistHard(&eStack80);
			fVar14 = pWeapon->FUN_002d5710();
			pCVar10 = pOtherActor->GetCollidingActor();

			do {
				if (pCVar10 == pOtherActor) {
					pOtherActor = (CActorFighter*)0x0;
				}
				else {
					bVar8 = pOtherActor->IsKindOfObject(2);
					if (bVar8 != false) {
						CActorMovable* pMovable = static_cast<CActorMovable*>(pOtherActor);
						edF32Vector4ScaleHard((fVar13 / fVar14) * pMovable->dynamic.linearAcceleration, &eStack128, &pMovable->dynamic.velocityDirectionEuler);
						edF32Vector4AddHard(&local_40, &local_40, &eStack128);
					}

					pOtherActor = pOtherActor->pTiedActor;
				}
			} while (pOtherActor != (CActorFighter*)0x0);
		}

		local_70 = local_40;

		fVar13 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x8;
		if ((pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x0 & 2) != 0) {
			local_10 = local_70;

			edF32Vector4SubHard(&eStack96, &local_70, &pBehaviour->field_0x80);
			fVar14 = edF32Vector4GetDistHard(&eStack96);
			fVar13 = fVar13 * Timer::GetTimer()->cutsceneDeltaTime;
			if (fVar13 <= fVar14) {
				fVar14 = fVar13;
			}

			fVar13 = edF32Vector4SafeNormalize0Hard(&eStack96, &eStack96);
			if (fVar13 != 0.0f) {
				edF32Vector4ScaleHard(fVar14, &eStack96, &eStack96);
				edF32Vector4AddHard(&local_70, &pBehaviour->field_0x80, &eStack96);
			}
		}

		local_10 = local_70;
		pBehaviour->field_0x80 = local_70;

		fVar14 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x4;
		GetWeapon();
		fVar13 = fVar14 * 0.5f;
		local_10.x = (local_10.x - fVar13) + (fVar14 * (float)CScene::Rand()) / 32767.0f;
		local_10.y = (local_10.y - fVar13) + (fVar14 * (float)CScene::Rand()) / 32767.0f;
		local_10.w = 1.0f;
		local_10.z = (local_10.z - fVar13) + (fVar14 * (float)CScene::Rand()) / 32767.0f;

		iVar9 = GetWeapon()->GetBurstState();
		if (iVar9 == 1) {
			iVar9 = GetWeapon()->Action(&local_10, this);
			if (iVar9 != 0) {
				SetState(WOLFEN_STATE_FIRE, -1);
			}
		}
	}

	pCVar3 = this->pAnimationController;
	peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
	if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
		bVar8 = false;
		if (peVar4->animPlayState != 0) {
			bVar8 = (peVar4->field_0xcc & 2) != 0;
		}
	}
	else {
		bVar8 = false;
	}

	if (bVar8) {
		PlayAnim(0xaa);
	}

	this->dynamic.speed = 0.0f;
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	return;
}

void CActorWolfen::StateTrackWeaponReload(CBehaviourTrackWeaponStand* pBehaviour)
{
	uint uVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	CActorWeapon* pCVar5;
	CActor* pCVar6;
	Timer* pTVar7;
	int iVar8;
	CActor* pOtherActor;
	long nextState;
	float fVar11;
	float fVar12;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	pOtherActor = this->pTargetActor_0xc80;

	nextState = -1;

	if (pOtherActor != (CActor*)0x0) {
		fVar11 = GetRunRotSpeed();
		SV_UpdateOrientationToPosition2D(fVar11, &pOtherActor->currentLocation);
		uVar1 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x0;

		pCVar5 = GetWeapon();
		SV_GetActorHitPos(pOtherActor, &local_20);

		if ((uVar1 & 1) != 0) {
			edF32Vector4SubHard(&eStack48, &local_20, &pCVar5->currentLocation);
			fVar11 = edF32Vector4GetDistHard(&eStack48);
			fVar12 = pCVar5->FUN_002d5710();
			pCVar6 = pOtherActor->GetCollidingActor();
			do {
				if (pCVar6 == pOtherActor) {
					pOtherActor = (CActor*)0x0;
				}
				else {
					bVar4 = pOtherActor->IsKindOfObject(2);
					if (bVar4 != false) {
						CActorMovable* pMovable = static_cast<CActorMovable*>(pOtherActor);
						edF32Vector4ScaleHard((fVar11 / fVar12) * pMovable->dynamic.linearAcceleration, &eStack96, &pMovable->dynamic.velocityDirectionEuler);
						edF32Vector4AddHard(&local_20, &local_20, &eStack96);
					}

					pOtherActor = pOtherActor->pTiedActor;
				}
			} while (pOtherActor != (CActor*)0x0);
		}

		fVar11 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x8;
		local_50 = local_20;

		if ((pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x0 & 2) != 0) {
			edF32Vector4SubHard(&eStack64, &local_50, &pBehaviour->field_0x80);
			fVar12 = edF32Vector4GetDistHard(&eStack64);
			pTVar7 = Timer::GetTimer();
			fVar11 = fVar11 * pTVar7->cutsceneDeltaTime;
			if (fVar11 <= fVar12) {
				fVar12 = fVar11;
			}

			fVar11 = edF32Vector4SafeNormalize0Hard(&eStack64, &eStack64);
			if (fVar11 != 0.0f) {
				edF32Vector4ScaleHard(fVar12, &eStack64, &eStack64);
				edF32Vector4AddHard(&local_50, &pBehaviour->field_0x80, &eStack64);
			}
		}

		pBehaviour->field_0x80 = local_50;
	}

	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	iVar8 = GetWeapon()->GetBurstState();
	if (iVar8 == 1) {
		nextState = WOLFEN_STATE_AIM;

		if (this->pCommander->field_0x194 < 1) {
			if (this->curBehaviourId == 0x11) {
				nextState = pBehaviour->Func_0x70();
			}
			else {
				nextState = pBehaviour->GetStateWolfenWeapon();
			}
		}
	}
	else {
		if (pBehaviour->field_0x94 == 2) {
			pCVar2 = this->pAnimationController;
			peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
			bVar4 = false;
			if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
				if (peVar3->animPlayState == 0) {
					bVar4 = false;
				}
				else {
					bVar4 = (peVar3->field_0xcc & 2) != 0;
				}
			}

			if (bVar4) {
				PlayAnim(0xaa);
			}
		}
	}

	if (nextState == -1) {
		iVar8 = pBehaviour->GetState_001f08a0();
		if (iVar8 != -1) {
			SetState(iVar8, -1);
		}
	}
	else {
		SetState(nextState, -1);
	}

	return;
}

void CActorWolfen::StateWolfenComeBack(CBehaviourWolfen* pBehaviour)
{
	char cVar1;
	bool bVar2;
	edF32VECTOR4* pComeBackPosition;
	int nextState;
	undefined4 uVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	CActorsTable actorsTable;
	CActorMovParamsIn actorMovParamsIn;
	CActorMovParamsOut actorMovParamsOut;

	actorMovParamsOut.flags = 0;
	actorMovParamsIn.flags = 0;
	actorMovParamsIn.pRotation = (edF32VECTOR4*)0x0;
	actorMovParamsIn.speed = 0.0f;

	pComeBackPosition = pBehaviour->GetComeBackPosition();

	if (sqrtf(edF32Vector4DotProductHard_I(pComeBackPosition, &this->currentLocation)) < 0.5f) goto LAB_0017ae10;

	actorMovParamsIn.flags = actorMovParamsIn.flags | 0x150;
	if (this->currentAnimType == 7) {
		actorMovParamsIn.rotSpeed = GetWalkRotSpeed();
		actorMovParamsIn.flags = actorMovParamsIn.flags | 2;
		actorMovParamsIn.acceleration = GetWalkAcceleration();
		actorMovParamsIn.speed = GetWalkSpeed();
	}
	else {
		actorMovParamsIn.rotSpeed = GetRunRotSpeed();
		actorMovParamsIn.flags = actorMovParamsIn.flags | 2;
		actorMovParamsIn.acceleration = GetRunAcceleration();
		actorMovParamsIn.speed = GetRunSpeed();
	}

	actorMovParamsIn.flags = actorMovParamsIn.flags | 0x400;

	pComeBackPosition = pBehaviour->GetComeBackPosition();
	if ((this->combatFlags_0xb78 & 0x400) == 0) {
	LAB_0017ad88:
		bVar2 = false;
	}
	else {
		if (GetPathfinderClientAlt()->id != -1) {
			if (GetPathfinderClientAlt()->IsValidPosition(&this->currentLocation) == false) goto LAB_0017ad88;
		}

		bVar2 = true;
	}

	if (bVar2) {
		if ((this->combatFlags_0xb78 & 0x80000) == 0) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x80000;
		}

		this->pathOriginPosition = this->currentLocation;
	}

	SV_AUT_MoveTo(&actorMovParamsOut, &actorMovParamsIn, pComeBackPosition);

	if ((actorMovParamsOut.flags & 2) != 0) {
		this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x40000;
	}
LAB_0017ae10:
	actorsTable.nbEntries = 0;
	ManageDyn(4.0f, 0x1002023b, &actorsTable);

	nextState = SV_WLF_CheckBoxOnWay(&actorsTable);

	if (nextState == -1) {
		nextState = pBehaviour->TestState_001f0a90();

		if (nextState == -1) {
			nextState = pBehaviour->TestState_001f0a70();

			if (nextState == -1) {
				nextState = pBehaviour->TestState_001f0a30();

				if (nextState == -1) {
					pComeBackPosition = pBehaviour->GetComeBackPosition();

					if (0.5f <= sqrtf(edF32Vector4DotProductHard_I(pComeBackPosition, &this->currentLocation))) {
						if ((this->pTargetActor_0xc80 != (CActorFighter*)0x0) &&
							(nextState = pBehaviour->TestState_001f09b0(), nextState != -1)) {
							SetState(nextState, -1);
						}
					}
					else {
						SetState(pBehaviour->GetStateWolfenGuard(), -1);
					}
				}
				else {
					SetState(nextState, -1);
				}
			}
			else {
				SetState(nextState, -1);
			}
		}
		else {
			SetState(nextState, -1);
		}
	}
	else {
		SetState(nextState, -1);
	}

	return;
}

void CActorWolfen::StateWolfen_00179db0(CBehaviourWolfen* pBehaviour)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	int iVar4;

	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	iVar4 = pBehaviour->GetState_001f0930();
	if (iVar4 == -1) {
		if ((int)this->combatMode_0xb7c < 2) {
			pCVar1 = this->pAnimationController;
			peVar2 = (pCVar1->anmBinMetaAnimator).aAnimData;
			bVar3 = false;
			if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
				if (peVar2->animPlayState == 0) {
					bVar3 = false;
				}
				else {
					bVar3 = (peVar2->field_0xcc & 2) != 0;
				}
			}

			if (bVar3) {
				SetState(WOLFEN_STATE_LOCATE, -1);
			}
		}
		else {
			SetState(0x9a, -1);
		}
	}
	else {
		SetState(iVar4, -1);
	}

	return;
}

void CActorWolfen::StateWolfenSurprise(CBehaviourWolfen* pBehaviour)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	int iVar4;
	StateConfig* pSVar5;
	int iVar6;
	edF32VECTOR4* v1;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack80;
	undefined4 local_40;
	undefined4 local_38;
	undefined4 local_34;
	undefined4 local_c;

	local_c = 0;
	local_40 = 0;
	local_38 = 0;
	local_34 = 0;

	v1 = (edF32VECTOR4*)0x0;

	if ((int)this->combatMode_0xb7c < 1) {
		if (((this->combatFlags_0xb78 & 0x80) == 0) || (this->pTrackedProjectile == (CActorProjectile*)0x0)) {
			if (this->field_0xd04 != (CActor*)0x0) {
				v1 = &this->field_0xd04->currentLocation;
			}
		}
		else {
			v1 = &this->pTrackedProjectile->currentLocation;
		}
	}
	else {
		if (this->pTargetActor_0xc80 != (CActorFighter*)0x0) {
			v1 = &this->pTargetActor_0xc80->currentLocation;
		}
	}

	if ((v1 != (edF32VECTOR4*)0x0) && (this->actorState == 0x9a)) {
		edF32Vector4SubHard(&eStack80, v1, &this->currentLocation);
		edF32Vector4NormalizeHard(&eStack80, &eStack80);
		fVar7 = GetAngleYFromVector(&eStack80);
		fVar7 = edF32Between_Pi(fVar7 - this->field_0xcfc);
		pCVar1 = this->pAnimationController;
		iVar4 = GetIdMacroAnim(this->currentAnimType);
		if (iVar4 < 0) {
			fVar8 = 0.0f;
		}
		else {
			fVar8 = pCVar1->GetAnimLength(iVar4, 1);
		}

		if (fVar7 <= 0.0f) {
			fVar7 = -fVar7;
		}

		SV_UpdateOrientationToPosition2D(fVar7 / fVar8, v1);
	}

	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	iVar4 = pBehaviour->GetState_001f0930();
	if (iVar4 == -1) {
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
			if ((int)this->combatMode_0xb7c < 1) {
				if (((this->combatFlags_0xb78 & 0x80) == 0) || (v1 == (edF32VECTOR4*)0x0)) {
					if (this->field_0xd04 != (CActor*)0x0) {
						iVar4 = 0x9d;
					}
				}
				else {
					if (this->pTrackedProjectile != (CActorProjectile*)0x0) {
						this->pTrackedProjectile->GetTimeToExplode();

						pCVar1 = this->pAnimationController;
						iVar6 = GetIdMacroAnim(GetStateCfg(0xa5)->animId);
						if (-1 < iVar6) {
							pCVar1->GetAnimLength(iVar6, 0);
						}
						
						GetRunSpeed();

						if ((pBehaviour->flags_0x4 & 0x20) == 0) {
							iVar4 = 0xa1;
						}
						else {
							if (this->pTrackedProjectile->field_0x40c == (CActor*)0x0) {
								iVar4 = 0xa3;
								this->pTrackedProjectile->field_0x40c = this;
							}
						}
					}
				}
			}
			else {
				if (1 < (int)this->combatMode_0xb7c) {
					SV_AUT_WarnActors(this->field_0xcf4, 0.0f, this->pTargetActor_0xc80);
				}

				iVar4 = pBehaviour->GetStateWolfenTrack();
			}

			if (iVar4 == -1) {
				SetState(WOLFEN_STATE_LOCATE, -1);
			}
			else {
				SetState(iVar4, -1);
			}

			this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xffffff7f;
		}
	}
	else {
		SetState(iVar4, -1);
	}

	return;
}

void CActorWolfen::StateWolfenLocate(CBehaviourWolfen* pBehaviour)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	float fVar4;
	float fVar5;
	CActorWolfen* pScannedWolfen;
	int iVar7;
	int* piVar9;
	Timer* pTVar10;
	int lVar11;
	long lVar12;
	int iVar13;
	float fVar14;
	CActorsTable local_220;
	CActorsTable local_110;

	this->dynamic.speed = 0.0;
	iVar7 = -1;
	
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	InternState_WolfenLocate();

	if (((this->combatFlags_0xb78 & 4) == 0) || ((int)this->combatMode_0xb7c < 1)) {
		if ((this->combatFlags_0xb78 & 0x80) != 0) {
			if ((this->field_0xb74 == 1) || (this->field_0xb74 == 0)) {
				local_110.nbEntries = 0;
				GetVision()->ScanFromClassID(PROJECTILE, &local_110, 1);
				iVar13 = 0;
				if (local_110.nbEntries != 0) {
					for (; iVar13 < local_110.nbEntries; iVar13 = iVar13 + 1) {
						if (local_110.aEntries[iVar13]->actorState == 0xc) {
							this->pTrackedProjectile = static_cast<CActorProjectile*>(local_110.aEntries[iVar13]);
							iVar7 = 0x9a;
							break;
						}
					}
				}
			}
			else {
				this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xffffff7f;
			}
		}
	}
	else {
		iVar7 = pBehaviour->GetState_001f0b30();
	}

	if ((this->combatFlags_0xb78 & 0x40) != 0) {
		local_220.nbEntries = 0;
		GetVision()->ScanFromClassID(WOLFEN, &local_220, 1);
		for (iVar13 = 0; pScannedWolfen = static_cast<CActorWolfen*>(local_220.aEntries[0]), iVar13 < local_220.nbEntries; iVar13 = iVar13 + 1) {
			fVar14 = pScannedWolfen->GetLifeInterface()->GetValue();
			if ((0.0f < fVar14) &&
				(fVar14 = pScannedWolfen->currentLocation.x - this->currentLocation.x,
					fVar4 = pScannedWolfen->currentLocation.y - this->currentLocation.y,
					fVar5 = pScannedWolfen->currentLocation.z - this->currentLocation.z,
					sqrtf(fVar14 * fVar14 + fVar4 * fVar4 + fVar5 * fVar5) <= 5.0f)) {
				pTVar10 = Timer::GetTimer();
				if ((this->field_0xd04 == (CActor*)0x0) || (15.0f < pTVar10->scaledTotalTime - this->field_0xd08)) {
					pTVar10 = Timer::GetTimer();
					this->field_0xd08 = pTVar10->scaledTotalTime;
				}

				this->field_0xd04 = pScannedWolfen;
			}
		}
	}

	if (iVar7 == -1) {
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
			if ((int)this->combatMode_0xb7c < 1) {
				lVar11 = pBehaviour->GetStateWolfenComeBack();
			}
			else {
				lVar11 = pBehaviour->GetStateWolfenTrack();
			}

			if ((((this->combatFlags_0xb78 & 0x40) != 0) && ((pBehaviour->flags_0x4 & 0x10) != 0)) && (this->field_0xd04 != (CActor*)0x0)
				) {
				pTVar10 = Timer::GetTimer();
				if (pTVar10->scaledTotalTime - this->field_0xd08 < this->timeInAir) {
					lVar11 = 0x9a;
				}
				else {
					IMPLEMENTATION_GUARD(
					lVar12 = (*(code*)this->pVTable[1].field_0x4)(this, this->field_0xd04);
					if (lVar12 == 0) {
						(*(code*)this->pVTable[1].actorBase)(this, this->field_0xd04);
						(*(code*)(this->pVTable)->SetFightBehaviour)(this);
						lVar11 = -1;
					}
					this->field_0xd04 = (CActor*)0x0;)
				}
			}

			this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffffe3f;

			if (lVar11 != -1) {
				SetState((int)lVar11, -1);
			}
		}
		else {
			iVar7 = pBehaviour->GetState_001f0930();
			if (iVar7 != -1) {
				SetState(iVar7, -1);
			}
		}
	}
	else {
		this->field_0xd08 = 0.0f;
		this->field_0xd04 = (CActor*)0x0;
		this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffffebf;
		SetState(iVar7, -1);
	}

	return;
}

void CActorWolfen::StateWatchDogGuard(CBehaviourWatchDog* pBehaviour)
{
	edF32VECTOR4* rotEuler;
	int iVar1;
	int iVar2;
	int newState;
	ulong uVar3;
	float fVar4;
	edF32VECTOR4 eStack16;

	if ((pBehaviour->flags_0x4 & 8) != 0) {
		rotEuler = pBehaviour->GetComeBackAngles();
		SetVectorFromAngles(&eStack16, &rotEuler->xyz);
		SV_WLF_UpdateOrientation2D(GetWalkRotSpeed(), &eStack16, 0);
	}

	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	WaitingAnimation_Guard();

	newState = pBehaviour->TestState_001f09b0();
	if (newState == -1) {
		/* Not in combat. */
		newState = pBehaviour->TestState_001f0a90();
		if (newState == -1) {
			newState = pBehaviour->TestState_001f0a70();
			if (newState == -1) {
				newState = pBehaviour->TestState_001f0a30();
				if (newState == -1) {
					newState = pBehaviour->TestState_001f0a00();
					if (newState != -1) {
						SetState(newState, -1);
					}
				}
				else {
					SetState(newState, -1);
				}
			}
			else {
				SetState(newState, -1);
			}
		}
		else {
			SetState(newState, -1);
		}
	}
	else {
		SetState(newState, -1);
	}

	return;
}

void CActorWolfen::StateTrackWeaponAim(CBehaviourTrackWeapon* pBehaviour)
{
	CActorCommander* pCVar1;
	uint uVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	float fVar5;
	bool bVar6;
	bool bVar7;
	bool bVar8;
	int iVar9;
	CActorWeapon* pWeapon;
	CActor* pCVar10;
	Timer* pTVar11;
	ulong uVar12;
	edF32VECTOR4* v2;
	CActor* pOtherActor;
	float fVar13;
	float fVar14;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	GetVision();
	pOtherActor = this->pTargetActor_0xc80;
	bVar8 = false;
	fVar13 = GetRunRotSpeed();
	edF32Vector4SubHard(&eStack32, &this->pCommander->targetPosition, &this->currentLocation);
	edF32Vector4NormalizeHard(&eStack32, &eStack32);
	bVar6 = SV_WLF_UpdateOrientation2D(fVar13, &eStack32, 0);
	pCVar1 = this->pCommander;
	if (((pCVar1->field_0x194 < 1) ||
		(fVar13 = (pCVar1->targetPosition).x - this->currentLocation.x,
			fVar14 = (pCVar1->targetPosition).y - this->currentLocation.y,
			fVar5 = (pCVar1->targetPosition).z - this->currentLocation.z,
			sqrt(fVar13 * fVar13 + fVar14 * fVar14 + fVar5 * fVar5) <= pBehaviour->field_0x90)) ||
		(bVar7 = SV_WLF_CanMoveTo(&this->pCommander->targetGroundPosition), bVar7 == false)) {
		if ((this->combatFlags_0xb78 & 4) == 0) {
			SetState(pBehaviour->Func_0x70(), -1);
		}
		else {
			if (((bVar6 != true) || (0 < this->pCommander->field_0x194)) ||
				(this->timeInAir <= 1.0f)) {
				if (pOtherActor != (CActorFighter*)0x0) {
					iVar9 = pBehaviour->GetState_001f0930();
					if (iVar9 == -1) {
						iVar9 = pBehaviour->GetState_001f08a0();
						if (iVar9 == -1) {
							bVar8 = true;
						}
						else {
							SetState(iVar9, -1);
						}
					}
					else {
						SetState(iVar9, -1);
					}
				}
			}
			else {
				SetState(pBehaviour->GetStateWolfenWeapon(), -1);
			}
		}
	}
	else {
		SetState(pBehaviour->Func_0x70(), -1);
	}

	if ((((this->combatFlags_0xb78 & 4) != 0) && (pOtherActor != (CActorFighter*)0x0)) && (bVar8)) {
		uVar2 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x0;
		pWeapon = GetWeapon();
		SV_GetActorHitPos(pOtherActor, &local_40);
		if ((uVar2 & 1) != 0) {
			edF32Vector4SubHard(&eStack80, &local_40, &pWeapon->currentLocation);
			fVar13 = edF32Vector4GetDistHard(&eStack80);
			fVar14 = pWeapon->FUN_002d5710();
			pCVar10 = pOtherActor->GetCollidingActor();

			do {
				if (pCVar10 == pOtherActor) {
					pOtherActor = (CActorFighter*)0x0;
				}
				else {
					bVar8 = pOtherActor->IsKindOfObject(2);
					if (bVar8 != false) {
						CActorMovable* pMovable = static_cast<CActorMovable*>(pOtherActor);
						edF32Vector4ScaleHard((fVar13 / fVar14) * pMovable->dynamic.linearAcceleration, &eStack128, &pMovable->dynamic.velocityDirectionEuler);
						edF32Vector4AddHard(&local_40, &local_40, &eStack128);
					}

					pOtherActor = pOtherActor->pTiedActor;
				}
			} while (pOtherActor != (CActorFighter*)0x0);
		}

		local_70 = local_40;

		fVar13 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x8;
		if ((pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x0 & 2) != 0) {
			local_10 = local_70;

			edF32Vector4SubHard(&eStack96, &local_70, &pBehaviour->field_0x80);
			fVar14 = edF32Vector4GetDistHard(&eStack96);
			fVar13 = fVar13 * Timer::GetTimer()->cutsceneDeltaTime;
			if (fVar13 <= fVar14) {
				fVar14 = fVar13;
			}

			fVar13 = edF32Vector4SafeNormalize0Hard(&eStack96, &eStack96);
			if (fVar13 != 0.0f) {
				edF32Vector4ScaleHard(fVar14, &eStack96, &eStack96);
				edF32Vector4AddHard(&local_70, &pBehaviour->field_0x80, &eStack96);
			}
		}

		local_10 = local_70;
		pBehaviour->field_0x80 = local_70;

		fVar14 = pBehaviour->aSubObjs[pBehaviour->field_0xe8].field_0x4.field_0x4;
		GetWeapon();
		fVar13 = fVar14 * 0.5f;
		local_10.x = (local_10.x - fVar13) + (fVar14 * (float)CScene::Rand()) / 32767.0f;
		local_10.y = (local_10.y - fVar13) + (fVar14 * (float)CScene::Rand()) / 32767.0f;
		local_10.w = 1.0f;
		local_10.z = (local_10.z - fVar13) + (fVar14 * (float)CScene::Rand()) / 32767.0f;

		iVar9 = GetWeapon()->GetBurstState();
		if (iVar9 == 3) {
			SetState(WOLFEN_STATE_RELOAD, -1);
		}
		else {
			if (iVar9 == 1) {
				iVar9 = GetWeapon()->Action(&local_10, this);
				if (iVar9 != 0) {
					SetState(WOLFEN_STATE_FIRE, -1);
				}
			}
		}
	}

	pCVar3 = this->pAnimationController;
	peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
	if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
		bVar8 = false;
		if (peVar4->animPlayState != 0) {
			bVar8 = (peVar4->field_0xcc & 2) != 0;
		}
	}
	else {
		bVar8 = false;
	}

	if (bVar8) {
		PlayAnim(0xaa);
	}

	this->dynamic.speed = 0.0f;
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	return;
}

void CActorWolfen::StateTrackWeaponChase(CBehaviourTrackWeapon* pBehaviour)
{
	float fVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* pCVar5;
	undefined4 uVar6;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;
	CActorsTable actorsTable;
	CActorCommander* pCommander;

	actorsTable.nbEntries = 0;
	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.speed = 0.0;
	movParamsIn.flags = 0x150;
	iVar4 = -1;
	if (this->currentAnimType == 7) {
		movParamsIn.rotSpeed = GetWalkRotSpeed();
		movParamsIn.flags = movParamsIn.flags | 2;
		movParamsIn.acceleration = GetWalkAcceleration();
		movParamsIn.speed = GetWalkSpeed();
	}
	else {
		movParamsIn.rotSpeed = GetRunRotSpeed();
		movParamsIn.flags = movParamsIn.flags | 2;
		movParamsIn.acceleration = GetRunAcceleration();
		movParamsIn.speed = GetRunSpeed();
	}

	movParamsIn.flags = movParamsIn.flags | 0x400;
	SV_WLF_MoveTo(&movParamsOut, &movParamsIn, &this->pCommander->targetGroundPosition);

	if ((movParamsOut.flags & 2) != 0) {
		iVar4 = pBehaviour->GetStateWolfenWeapon();
	}

	ManageDyn(4.0f, 0x1002023b, &actorsTable);

	if (iVar4 == -1) {
		iVar4 = SV_WLF_CheckBoxOnWay(&actorsTable);
	}

	if (iVar4 == -1) {
		if ((this->combatMode_0xb7c == ECM_InCombat) &&
			(pCVar5 = pBehaviour->GetNotificationTargetArray(), (int)pCVar5->combatMode < 2)) {
			SetState(pBehaviour->GetState_001f0b30(), -1);
		}
		else {
			if ((~this->combatFlags_0xb78 & 0x420) == 0x420) {
				SetState(pBehaviour->GetStateWolfenWeapon(), -1);
			}
			else {
				iVar4 = pBehaviour->GetState_001f08a0();
				if (iVar4 == -1) {
					if (((this->combatFlags_0xb78 & 4) == 0) ||
						(pCommander = this->pCommander,
							fVar1 = (pCommander->targetPosition).x - this->currentLocation.x,
							fVar2 = (pCommander->targetPosition).y - this->currentLocation.y,
							fVar3 = (pCommander->targetPosition).z - this->currentLocation.z,
							pBehaviour->field_0xf0 < sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3))) {
						pCommander = this->pCommander;
	
							fVar1 = (pCommander->targetGroundPosition).x - this->currentLocation.x;
						fVar2 = (pCommander->targetGroundPosition).y - this->currentLocation.y;
						fVar3 = (pCommander->targetGroundPosition).z - this->currentLocation.z;
						if (0.5f <= sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3)) {
							iVar4 = FUN_0035f1e0(&actorsTable, &this->pCommander->targetGroundPosition);
							if (iVar4 == 0) {
								iVar4 = pBehaviour->GetState_001f0930();
								if (iVar4 != -1) {
									SetState(iVar4, -1);
								}
							}
							else {
								IMPLEMENTATION_GUARD(
								SetState(pBehaviour->field_0x78(), -1);)
							}
						}
						else {
							if ((int)this->combatMode_0xb7c < 1) {
								IMPLEMENTATION_GUARD(
								SetState(pBehaviour->field_0x78(), -1);)
							}
							else {
								SetState(pBehaviour->GetStateWolfenWeapon(), -1);
							}
						}
					}
					else {
						pCVar5 = pBehaviour->GetNotificationTargetArray();
						if ((int)pCVar5->combatMode < 2) {
							SetState(pBehaviour->GetState_001f0b30(), -1);
						}
						else {
							SetState(pBehaviour->Func_0x74(), -1);
						}
					}
				}
				else {
					SetState(iVar4, -1);
				}
			}
		}
	}
	else {
		SetState(iVar4, -1);
	}

	return;
}

void CActorWolfen::StateTrackWeaponCheckPosition(CBehaviourTrackWeaponStand* pBehaviour)
{
	CActorCommander* pCVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	bool bVar7;
	undefined4 uVar8;
	int iVar9;
	CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* this_00;

	this->dynamic.speed = 0.0f;
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
	InternState_WolfenLocate();

	pCVar1 = this->pCommander;
	if (pCVar1->field_0x194 < 1) {
		pCVar2 = this->pAnimationController;
		peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
		if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
			bVar7 = false;
			if (peVar3->animPlayState != 0) {
				bVar7 = (peVar3->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar7 = false;
		}
		if (bVar7) {
			bVar7 = pBehaviour->GetNotificationTargetArray()->FUN_003c38c0(this);
			if (bVar7 == false) {
				SetState(0xb3, -1);
			}
			else {
				SetState(pBehaviour->GetStateWolfenWeapon(), -1);
			}
		}
		else {
			iVar9 = pBehaviour->GetState_001f0b30();
			if (iVar9 != -1) {
				SetState(iVar9, -1);
			}
		}
	}
	else {
		if (((this->combatFlags_0xb78 & 4) == 0) ||
			(fVar4 = (pCVar1->targetPosition).x - this->currentLocation.x,
				fVar5 = (pCVar1->targetPosition).y - this->currentLocation.y,
				fVar6 = (pCVar1->targetPosition).z - this->currentLocation.z,
				pBehaviour->field_0x90 < sqrtf(fVar4 * fVar4 + fVar5 * fVar5 + fVar6 * fVar6))) {
			SetState(pBehaviour->GetState_001f0b30(), -1);
		}
		else {
			SetState(pBehaviour->Func_0x74(), -1);
		}
	}

	return;
}

void CActorWolfen::StateTrackWeaponDefend(CBehaviourTrackWeapon* pBehaviour)
{
	CActorCommander* pCVar1;
	float fVar2;
	float fVar3;
	bool bVar4;
	int iVar5;
	undefined4 uVar6;
	float fVar7;
	edF32VECTOR4 eStack16;

	fVar7 = GetWalkRotSpeed();
	edF32Vector4SubHard(&eStack16, &this->pCommander->targetPosition, &this->currentLocation);
	edF32Vector4NormalizeHard(&eStack16, &eStack16);
	bVar4 = SV_WLF_UpdateOrientation2D(fVar7, &eStack16, 0);
	this->dynamic.speed = 0.0f;
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	if (0 < this->pCommander->field_0x194) {
		WaitingAnimation_Defend();
	}

	if ((((this->combatFlags_0xb78 & 0x420) == 0) || (pCVar1 = this->pCommander,
			fVar7 = (pCVar1->targetGroundPosition).x - this->currentLocation.x,
			fVar2 = (pCVar1->targetGroundPosition).y - this->currentLocation.y,
			fVar3 = (pCVar1->targetGroundPosition).z - this->currentLocation.z,
			sqrtf(fVar7 * fVar7 + fVar2 * fVar2 + fVar3 * fVar3) < 0.5f)) ||
		(iVar5 = SV_WLF_CanMoveTo(&this->pCommander->targetGroundPosition), iVar5 == 0)) {
		if (((bVar4 != true) || (0 < this->pCommander->field_0x194)) || (this->timeInAir <= 1.0f)) {
			if (((this->pCommander->field_0x194 < 1) || ((this->combatFlags_0xb78 & 4) == 0)) ||
				(pCVar1 = this->pCommander,
					fVar7 = (pCVar1->targetPosition).x - this->currentLocation.x,
					fVar2 = (pCVar1->targetPosition).y - this->currentLocation.y,
					fVar3 = (pCVar1->targetPosition).z - this->currentLocation.z,
					pBehaviour->field_0x90 <= sqrtf(fVar7 * fVar7 + fVar2 * fVar2 + fVar3 * fVar3))) {
				iVar5 = pBehaviour->GetState_001f0930();
				if (iVar5 != -1) {
					SetState(iVar5, -1);
				}
			}
			else {
				SetState(pBehaviour->Func_0x74(), -1);
			}
		}
		else {
			SetState(pBehaviour->Func_0x78(), -1);
		}
	}
	else {
		SetState(pBehaviour->Func_0x70(), -1);
	}

	return;
}

void CActorWolfen::StateTrackCheckPosition(CBehaviourWolfen* pBehaviour)
{
	CAnimation* pAnim;
	edAnmLayer* pAnmLayer;
	bool bVar7;
	int newState;

	this->dynamic.speed = 0.0f;
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
	InternState_WolfenLocate();

	if (this->pCommander->field_0x194 < 1) {
		pAnim = this->pAnimationController;
		pAnmLayer = (pAnim->anmBinMetaAnimator).aAnimData;

		bVar7 = false;
		if ((pAnmLayer->currentAnimDesc).animType == pAnim->currentAnimType_0x30) {
			if (pAnmLayer->animPlayState == 0) {
				bVar7 = false;
			}
			else {
				bVar7 = (pAnmLayer->field_0xcc & 2) != 0;
			}
		}

		if (bVar7) {
			if (pBehaviour->GetNotificationTargetArray()->FUN_003c38c0(this) == false) {
				SetState(0xb3, -1);
			}
			else {
				SetState(0xa9, -1);
			}
		}
		else {
			newState = pBehaviour->GetState_001f0930();
			if (newState != -1) {
				SetState(newState, -1);
			}
		}
	}
	else {
		SetState(GetState_00174190(), -1);
	}

	return;
}

void CActorWolfen::StateTrackChase(CBehaviourTrack* pBehaviour)
{
	CActorFighter* pTarget;
	float fVar1;
	float fVar2;
	bool bVar3;
	int nextState;
	int iVar5;
	CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* pCVar5;
	undefined4 uVar6;
	float fVar7;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;
	CActorsTable actorsTable;
	CActor* pActor;
	CActorCommander* pCommander;

	nextState = -1;
	pTarget = this->pTargetActor_0xc80;
	actorsTable.nbEntries = 0;

	if (CheckLost() != false) {
		this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x40000;
	}

	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.speed = 0.0f;
	movParamsIn.flags = 0x150;

	if (this->currentAnimType == 7) {
		movParamsIn.rotSpeed = GetWalkRotSpeed();
		movParamsIn.flags = movParamsIn.flags | 2;
		movParamsIn.acceleration = GetWalkAcceleration();
		movParamsIn.speed = GetWalkSpeed();
	}
	else {
		movParamsIn.rotSpeed = GetRunRotSpeed();
		movParamsIn.flags = movParamsIn.flags | 2;
		movParamsIn.acceleration = GetRunAcceleration();
		movParamsIn.speed = GetRunSpeed();
	}

	movParamsIn.flags = movParamsIn.flags | 0x400;

	SV_WLF_MoveTo(&movParamsOut, &movParamsIn, &this->pCommander->targetGroundPosition);

	if ((movParamsOut.flags & 2) != 0) {
		nextState = pBehaviour->GetStateWolfenWeapon();
	}

	ManageDyn(4.0f, 0x1002023b, &actorsTable);

	if (nextState == -1) {
		nextState = SV_WLF_CheckBoxOnWay(&actorsTable);
	}

	if (nextState == -1) {
		if ((this->combatMode_0xb7c == ECM_InCombat) && (pCVar5 = pBehaviour->GetNotificationTargetArray(), (int)pCVar5->combatMode < 2)) {
			SetState(pBehaviour->GetState_001f0b30(), -1);
		}
		else {
			if ((pTarget == (CActorFighter*)0x0) || (bVar3 = CanSwitchToFight_Area(pTarget), bVar3 == false))
			{
				if ((~this->combatFlags_0xb78 & 0x420) == 0x420) {
					SetState(pBehaviour->GetStateWolfenWeapon(), -1);
				}
				else {
					pCommander = this->pCommander;
					fVar1 = (pCommander->targetGroundPosition).x - this->currentLocation.x;
					fVar2 = (pCommander->targetGroundPosition).y - this->currentLocation.y;
					fVar7 = (pCommander->targetGroundPosition).z - this->currentLocation.z;
					if (0.5f <= sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar7 * fVar7)) {
						pCommander = this->pCommander;
						for (iVar5 = 0; bVar3 = false, iVar5 < actorsTable.nbEntries; iVar5 = iVar5 + 1) {
							pActor = actorsTable.aEntries[iVar5];
							if ((pActor->typeID == 0x1b) || (pActor->typeID == 0x1c)) {
								fVar1 = (pCommander->targetGroundPosition).x - (pActor->currentLocation).x;
								fVar2 = (pCommander->targetGroundPosition).z - (pActor->currentLocation).z;
								fVar7 = pActor->GetPosition_00117db0();
								bVar3 = true;
								if (sqrtf(fVar1 * fVar1 + 0.0f + fVar2 * fVar2) < fVar7) break;
							}
						}

						if (bVar3) {
							SetState(pBehaviour->Func_0x70(), -1);
						}
						else {
							nextState = pBehaviour->GetState_001f0930();
							if (nextState != -1) {
								SetState(nextState, -1);
							}
						}
					}
					else {
						if ((int)this->combatMode_0xb7c < 1) {
							SetState(pBehaviour->Func_0x70(), -1);
						}
						else {
							SetState(pBehaviour->GetStateWolfenWeapon(), -1);
						}
					}
				}
			}
			else {
				if (this->pCommander->BeginFightIntruder(this, pTarget) == false) {
					SetState(0x99, -1);
				}
				else {
					Func_0x204(pTarget);
					if (SetBehaviour(3, -1, -1) == false) {
						SetState(0x99, -1);
						this->pCommander->EndFightIntruder(this);
					}
				}
			}
		}
	}
	else {
		SetState(nextState, -1);
	}

	return;
}

void CActorWolfen::StateTrackDefend(CBehaviourTrack* pBehaviour)
{
	CActorFighter* pTarget;
	CActorCommander* pCVar1;
	float fVar2;
	float fVar3;
	bool bUpdateOrientationSuccess;
	bool bCanMoveTo;
	undefined4 uVar6;
	int iVar7;
	float fVar8;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, &this->pCommander->targetPosition, &this->currentLocation);
	edF32Vector4NormalizeHard(&eStack16, &eStack16);
	bUpdateOrientationSuccess = SV_WLF_UpdateOrientation2D(GetWalkRotSpeed(), &eStack16, 0);

	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	if (0 < this->pCommander->field_0x194) {
		WaitingAnimation_Defend();
	}

	pTarget = this->pTargetActor_0xc80;

	if ((((this->combatFlags_0xb78 & 0x420) == 0) ||
		(pCVar1 = this->pCommander,
			fVar8 = (pCVar1->targetGroundPosition).x - this->currentLocation.x,
			fVar2 = (pCVar1->targetGroundPosition).y - this->currentLocation.y,
			fVar3 = (pCVar1->targetGroundPosition).z - this->currentLocation.z,
			sqrtf(fVar8 * fVar8 + fVar2 * fVar2 + fVar3 * fVar3) < 0.5f)) ||
		(bCanMoveTo = SV_WLF_CanMoveTo(&this->pCommander->targetGroundPosition), bCanMoveTo == false)) {
		if (((bUpdateOrientationSuccess != true) || (0 < this->pCommander->field_0x194)) || (this->timeInAir <= 1.0f)) {
			if ((pTarget == (CActorFighter*)0x0) || (CanSwitchToFight_Area(pTarget) == false))
			{
				iVar7 = pBehaviour->GetState_001f0930();
				if (iVar7 != -1) {
					SetState(iVar7, -1);
				}
			}
			else {
				if (this->pCommander->BeginFightIntruder(this, pTarget) == false) {
					SetState(0x99, -1);
				}
				else {
					Func_0x204(pTarget);
					if (SetBehaviour(3, -1, -1) == false) {
						this->pCommander->EndFightIntruder(this);
					}
				}
			}
		}
		else {
			SetState(pBehaviour->Func_0x70(), -1);
		}
	}
	else {
		// CAN path to our target ground position, so chase to that position.
		SetState(WOLFEN_STATE_TRACK_CHASE, -1);
	}

	return;
}

void CActorWolfen::StateTrackWeaponDefend(CBehaviourTrackWeaponStand* pBehaviour)
{
	CActorCommander* pCVar1;
	float fVar2;
	float fVar3;
	bool bVar4;
	Timer* pTVar5;
	undefined4 uVar6;
	int iVar7;
	float fVar8;
	edF32VECTOR4 eStack16;

	fVar8 = GetWalkRotSpeed();
	edF32Vector4SubHard(&eStack16, &this->pCommander->targetPosition, &this->currentLocation);
	edF32Vector4NormalizeHard(&eStack16, &eStack16);
	bVar4 = SV_WLF_UpdateOrientation2D(fVar8, &eStack16, 0);
	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	if (0 < this->pCommander->field_0x194) {
		WaitingAnimation_Defend();
		pTVar5 = Timer::GetTimer();
		pBehaviour->field_0xec = pTVar5->scaledTotalTime;
	}

	if ((bVar4 != true) || (pTVar5 = Timer::GetTimer(), pTVar5->scaledTotalTime - (float)pBehaviour->field_0xec <= 4.0f)) {
		pCVar1 = this->pCommander;
		if ((pCVar1->field_0x194 < 1) ||
			(fVar8 = (pCVar1->targetPosition).x - this->currentLocation.x,
				fVar2 = (pCVar1->targetPosition).y - this->currentLocation.y,
				fVar3 = (pCVar1->targetPosition).z - this->currentLocation.z,
				pBehaviour->field_0x90 <= sqrtf(fVar8 * fVar8 + fVar2 * fVar2 + fVar3 * fVar3))) {
			iVar7 = pBehaviour->GetState_001f0930();
			if (iVar7 != -1) {
				SetState(iVar7, -1);
			}
		}
		else {
			SetState(pBehaviour->Func_0x74(), -1);
		}
	}
	else {
		SetState(pBehaviour->Func_0x78(), -1);
	}

	return;
}

int CActorWolfen::SV_WLF_CheckBoxOnWay(CActorsTable* pTable)
{
	int nextState;
	CActor* pBox;

	nextState = -1;

	if (((this->field_0xb74 != 5) && (this->pBoxInWay == (CActor*)0x0)) && ((pTable->IsInList(BOX) != false || (pTable->IsInList(BASIC_BOX) != false)))) {
		pBox = this->pBoxInWay;

		CActor** pEntry = pTable->aEntries;

		while (pBox == (CActor*)0x0) {
			if (((*pEntry)->typeID == BOX) || ((*pEntry)->typeID == BASIC_BOX)) {
				this->pBoxInWay = *pEntry;
			}

			pEntry = pEntry + 1;
			pBox = this->pBoxInWay;
		}

		nextState = 0x74;
	}

	return nextState;
}

void CActorWolfen::ClearLocalData()
{
	CinNamedObject30* pCVar1;
	CActorWeapon* pCVar2;
	CActorWolfen* pCVar3;
	int iVar4;
	uint uVar5;
	float fVar6;
	float fVar7;
	CAnimation* pAnim;

	if (((this->flags & 0x2000000) == 0) && (GetWeapon() != (CActorWeapon*)0x0)) {
		if (GetWeapon()->GetLinkFather() == this) {
			GetWeapon()->UnlinkWeapon();
		}
		if (this->field_0xb80 != 2) {
			GetWeapon()->LinkWeapon(this, 0xcc414f1b);
		}
	}

	Func_0x204((CActorFighter*)0x0);

	this->combatFlags_0xb78 = 0;
	this->combatMode_0xb7c = ECM_None;

	uVar5 = 0;
	if (this->nbComboMatchValues != 0) {
		do {
			this->field_0xbd0[uVar5].field_0x0 = 1;
			uVar5 = uVar5 + 1;
		} while (uVar5 < this->nbComboMatchValues);
	}

	this->field_0xbe0 = 0;
	this->field_0xbd8 = 0;
	this->field_0xd04 = (CActor*)0x0;
	//this->field_0xd0c = 0;
	this->field_0xd08 = 0;
	this->pTrackedProjectile = (CActorProjectile*)0x0;
	this->pBoxInWay = (CActor*)0x0;
	this->field_0xd24 = 0.0f;
	this->field_0xd28 = 1.0f;
	this->pTargetActor_0xc80 = (CActorFighter*)0x0;
	LifeRestore();
	UpdatePosition(&this->baseLocation, true);

	pCVar1 = this->pCinData;
	this->rotationEuler.xyz = pCVar1->rotationEuler;

	TermFightAction();

	if (this->pWolfenKnowledge != (CActorWolfenKnowledge*)0x0) {
		//this->pWolfenKnowledge->Reset();
		//this->field_0xb6c = 0xffffffff;
		//this->field_0xb70 = 1;
	}

	if (this->field_0xb74 == 3) {
		pAnim = this->pAnimationController;
		if (pAnim != (CAnimation*)0x0) {
			pAnim->AddDisabledBone(0xae8f8ef8);
		}

		if (GetWeapon() != (CActorWeapon*)0x0) {
			GetWeapon()->flags = GetWeapon()->flags & 0xffffff5f;
			GetWeapon()->EvaluateDisplayState();
		}
	}

	return;
}

int CActorWolfen::CheckDetectArea(edF32VECTOR4* pPosition)
{
	return this->pCommander->CheckDetectArea(pPosition);
}

void CActorWolfen::SV_WLF_MoveTo(CActorMovParamsOut* pMovParamsOut, CActorMovParamsIn* pMovParamsIn, edF32VECTOR4* pPosition)
{
	bool bValidPosition;

	if ((this->combatFlags_0xb78 & 0x400) == 0) {
	LAB_001739a0:
		bValidPosition = false;
	}
	else {
		if (GetPathfinderClientAlt()->id != -1) {
			bValidPosition = GetPathfinderClientAlt()->IsValidPosition(&this->currentLocation);
			if (bValidPosition == false) goto LAB_001739a0;
		}

		bValidPosition = true;
	}

	if (bValidPosition) {
		if ((this->combatFlags_0xb78 & 0x80000) == 0) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 0x80000;
		}

		this->pathOriginPosition = this->currentLocation;
	}

	SV_AUT_MoveTo(pMovParamsOut, pMovParamsIn, pPosition);

	return;
}

bool CActorWolfen::SV_WLF_CanMoveTo(edF32VECTOR4* pPosition)
{
	uint zoneId;
	bool bCanMoveTo;
	ed_zone_3d* pZone;
	int iVar2;

	zoneId = this->field_0xd30;
	bCanMoveTo = false;

	if (zoneId == 0xffffffff) {
		iVar2 = this->pCommander->CheckGuardArea(pPosition);
	}
	else {
		pZone = (ed_zone_3d*)0x0;
		if (zoneId != 0xffffffff) {
			pZone = edEventGetChunkZone(CScene::ptable.g_EventManager_006f5080->activeChunkId, zoneId);
		}

		iVar2 = edEventComputeZoneAgainstVertex(CScene::ptable.g_EventManager_006f5080->activeChunkId, pZone, pPosition, 0);
	}

	if ((iVar2 == 1) && (iVar2 = this->pCommander->CheckDetectArea(pPosition), iVar2 == 1)) {
		bCanMoveTo = SV_AUT_CanMoveTo(pPosition);
	}

	return bCanMoveTo;
}

bool CActorWolfen::SV_WLF_UpdateOrientation2D(float param_1, edF32VECTOR4* v0, int rotationType)
{
	edAnmLayer* peVar1;
	bool bVar2;
	int AVar3;
	int iVar4;
	Timer* pTVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CAnimation* pAnim;

	pAnim = this->pAnimationController;
	AVar3 = GetIdMacroAnim(0xa2);
	iVar4 = pAnim->PhysicalLayerFromLayerId(8);
	peVar1 = (pAnim->anmBinMetaAnimator).aAnimData;
	bVar2 = pAnim->IsLayerActive(8);
	if (((bVar2 != false) && (AVar3 != -1)) && (peVar1[iVar4].currentAnimDesc.animType == AVar3)) {
		local_10.x = this->rotationQuat.x;
		local_10.y = 0.0f;
		local_10.z = this->rotationQuat.z;
		local_10.w = 0.0f;

		edF32Vector4NormalizeHard(&local_10, &local_10);

		local_20.x = v0->x;
		local_20.y = 0.0f;
		local_20.z = v0->z;
		local_20.w = 0.0f;

		edF32Vector4NormalizeHard(&local_20, &local_20);
		fVar6 = edF32Vector4DotProductHard(&local_10, &local_20);
		fVar8 = 1.0f;
		if (fVar6 <= 1.0f) {
			fVar8 = fVar6;
		}
		fVar6 = edF32ACosHard(fVar8);
		pTVar5 = Timer::GetTimer();
		fVar8 = pTVar5->cutsceneDeltaTime;
		fVar7 = GetWalkRotSpeed();
		fVar6 = edFIntervalLERP(fabs(fVar6), 0.0f, fVar8 * fVar7 * 0.5f, 0.0f, 1.0f);
		fVar7 = this->field_0xd24;
		if (fVar6 < fVar7) {
			if (fVar7 < fVar6 + 0.2f) {
				this->field_0xd24 = fVar6;
			}
			else {
				this->field_0xd24 = fVar7 - 0.2f;
			}
		}
		else {
			if (fVar7 < fVar6) {
				if (fVar6 - 0.2f < fVar7) {
					this->field_0xd24 = fVar6;
				}
				else {
					this->field_0xd24 = fVar7 + 0.2f;
				}
			}
		}
		if (0.0f < local_10.x * local_20.z - local_20.x * local_10.z) {
			fVar8 = this->field_0xd28 + fVar8;
			this->field_0xd28 = fVar8;
			if (1.0f < fVar8) {
				this->field_0xd28 = 1.0f;
			}
		}
		else {
			fVar8 = this->field_0xd28 - fVar8;
			this->field_0xd28 = fVar8;
			if (fVar8 < 0.0f) {
				this->field_0xd28 = 0.0f;
			}
		}
	}

	return SV_UpdateOrientation2D(param_1, v0, rotationType);
}

bool CActorWolfen::SV_WLF_IsIntruderMakingNoise(CActor* pActor)
{
	bool bMakingNoise;
	int iVar2;
	long lVar3;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	bMakingNoise = false;
	if (((pActor != (CActor*)0x0) && (iVar2 = this->pCommander->CheckZone_00170f90(&pActor->currentLocation), iVar2 == 2)) && (pActor->IsMakingNoise() != false)) {
		local_10.x = this->currentLocation.x;
		local_10.z = this->currentLocation.z;
		local_10.w = this->currentLocation.w;
		local_10.y = this->currentLocation.y + this->field_0xcf0;

		pActor->SV_GetActorColCenter(&local_20);

		if (((local_10.x - local_20.x) * (local_10.x - local_20.x) + (local_10.z - local_20.z) * (local_10.z - local_20.z) <
			(this->hearingDetectionProps).rangeSquared) && (fabs(local_10.y - local_20.y) < (this->hearingDetectionProps).maxHeightDifference)) {
			bMakingNoise = true;
		}
	}

	if (bMakingNoise) {
		if ((this->combatFlags_0xb78 & 1) == 0) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 1;
		}
	}
	else {
		if ((this->combatFlags_0xb78 & 1) != 0) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffffffe;
		}
	}

	return bMakingNoise;
}

bool CActorWolfen::SV_WLF_IsIntruderInVitalSphere(CActor* pActor)
{
	float fVar1;
	float fVar2;
	float fVar3;
	bool bInVitalSphere;

	bInVitalSphere = false;
	if (pActor != (CActor*)0x0) {
		fVar1 = (pActor->currentLocation).x - this->currentLocation.x;
		fVar2 = (pActor->currentLocation).y - this->currentLocation.y;
		fVar3 = (pActor->currentLocation).z - this->currentLocation.z;

		bInVitalSphere = sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) < (this->visionDetectionProps).field_0x0;
	}

	if (bInVitalSphere) {
		if ((this->combatFlags_0xb78 & 2) == 0) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 2;
		}
	}
	else {
		if ((this->combatFlags_0xb78 & 2) != 0) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffffffd;
		}
	}

	return bInVitalSphere;
}

bool CActorWolfen::SV_WLF_IsIntruderInVision(CActor* pActor)
{
	int iVar1;
	uint uVar2;
	bool bInVision;

	bInVision = false;
	if (pActor != (CActor*)0x0) {
		iVar1 = this->actorState;
		uVar2 = 0;
		if ((iVar1 != -1) && (uVar2 = 0, 0x71 < iVar1)) {
			uVar2 = _gStateCfg_WLF[iVar1 + -0x72].field_0x8;
		}

		if ((uVar2 & 0x20) == 0) {
			GetVision()->location.x = this->currentLocation.x;
			GetVision()->location.y = this->currentLocation.y + this->field_0xcf0;
			GetVision()->location.z = this->currentLocation.z;
			GetVision()->location.w = this->currentLocation.w;

			GetVision()->rotationQuat = this->rotationQuat;
		}

		bInVision = GetVision()->ScanForTarget(pActor, 1) != (CActor*)0x0;
	}

	if (bInVision) {
		if ((this->combatFlags_0xb78 & 4) == 0) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 | 4;
		}
	}
	else {
		if ((this->combatFlags_0xb78 & 4) != 0) {
			this->combatFlags_0xb78 = this->combatFlags_0xb78 & 0xfffffffb;
		}
	}

	return bInVision;
}

bool CActorWolfen::FUN_001738e0(CActor* pActor)
{
	bool bVar1;

	bVar1 = false;
	if (((pActor != (CActor*)0x0) && (pActor->typeID == ACTOR_HERO_PRIVATE)) && (pActor->curBehaviourId == 8)) {
		bVar1 = true;
	}

	return bVar1;
}

bool CActorWolfen::FUN_0035f1e0(CActorsTable* pTable, edF32VECTOR4* pPosition)
{
	CActor* pActor;
	float fVar1;
	float fVar2;
	int curEntryIndex;
	float fVar4;

	curEntryIndex = 0;
	while (true) {
		if (pTable->nbEntries <= curEntryIndex) {
			return false;
		}

		pActor = pTable->aEntries[curEntryIndex];
		if (((pActor->typeID == DCA) || (pActor->typeID == PROJECTILE)) && (fVar1 = pPosition->x - (pActor->currentLocation).x, fVar2 = pPosition->z - (pActor->currentLocation).z,
				fVar4 = pActor->GetPosition_00117db0(), sqrtf(fVar1 * fVar1 + 0.0f + fVar2 * fVar2) < fVar4)) break;

		curEntryIndex = curEntryIndex + 1;
	}
	return true;
}

int CActorWolfen::GetState_00174190()
{
	int newState;

	newState = 0x9b;
	if (1 < (int)this->combatMode_0xb7c) {
		newState = WOLFEN_STATE_SURPRISE;
	}

	return newState;
}

void CActorWolfen::EnterCombatState(CActor* pSender)
{
	this->pTargetActor_0xc80 = static_cast<CActorFighter*>(pSender);

	if ((this->pTargetActor_0xc80 == this->pCommander->GetIntruder()) && (this->pCommander != (CActorCommander*)0x0)) {
		edF32VECTOR4 targetPosition;
		this->pTargetActor_0xc80->SV_GetActorColCenter(&targetPosition);
		this->pCommander->targetPosition = targetPosition;

		if (this->pCommander->bInCombat_0x1b0 == 0) {
			edF32VECTOR4 targetGroundPosition;
			this->pTargetActor_0xc80->SV_GetGroundPosition(&targetGroundPosition);
			int iVar15 = this->pCommander->CheckDetectArea(&targetGroundPosition);

			CEventManager* pCVar5 = CScene::ptable.g_EventManager_006f5080;
			if (iVar15 != 1) {
				uint uVar12 = this->field_0xd30;
				if (uVar12 == 0xffffffff) {
					iVar15 = this->pCommander->CheckGuardArea(&targetGroundPosition);
				}
				else {
					ed_zone_3d* pZone = (ed_zone_3d*)0x0;
					if (uVar12 != 0xffffffff) {
						pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, uVar12);
					}

					iVar15 = edEventComputeZoneAgainstVertex(pCVar5->activeChunkId, pZone, &targetGroundPosition, 0);
				}

				if (iVar15 != 1) return;
			}

			if (GetPathfinderClientAlt()->id != -1) {
				if (GetPathfinderClientAlt()->IsValidPosition(&targetGroundPosition) == false) return;
			}

			this->pCommander->targetGroundPosition = targetGroundPosition;
			this->pCommander->bInCombat_0x1b0 = 1;
		}
	}

	return;
}

void CActorWolfen::UpdateInRange_001744a0(bool bFlag)
{
	if (bFlag == false) {
		return;
	}

	EnterCombatState(this->pTargetActor_0xc80);

	if ((this->combatFlags_0xb78 & 4) == 0) {
		if ((this->combatMode_0xb7c == ECM_None) && (this->combatMode_0xb7c != ECM_Alerted)) {
			this->combatMode_0xb7c = ECM_Alerted;
		}
	}
	else {
		if (this->combatMode_0xb7c != ECM_InCombat) {
			this->combatMode_0xb7c = ECM_InCombat;
		}
	}

	return;
}


bool CActorWolfen::IsAlive(CActor* pActor)
{
	int iVar15 = -1;

	if ((pActor != (CActor*)0x0) && (pActor->typeID == ACTOR_HERO_PRIVATE)) {
		CActorAutonomous* pAutonomous = static_cast<CActorAutonomous*>(pActor);
		if (0.0f < pAutonomous->GetLifeInterface()->GetValue()) {
			iVar15 = 0;
		}
	}

	if (iVar15 < 0) {
		return false;
	}

	return true;
}

bool CActorWolfen::CanSwitchToFight_Area(CActor* pTarget)
{
	float fVar1;
	float fVar2;
	bool bVar3;
	bool bVar4;
	int iVar5;
	CBehaviour* pCVar6;
	long lVar7;
	bool bCanSwitchToFight;

	bCanSwitchToFight = false;
	if ((this->combatFlags_0xb78 & 4) != 0) {
		bVar4 = false;
		bVar3 = pTarget->IsKindOfObject(OBJ_TYPE_FIGHTER);
		if (((bVar3 != false) &&
			(fVar1 = (pTarget->currentLocation).x - this->currentLocation.x,
				fVar2 = (pTarget->currentLocation).z - this->currentLocation.z,
				sqrtf(fVar1 * fVar1 + 0.0f + fVar2 * fVar2) < this->field_0xb90)) &&
			(fabs((this->distanceToGround + this->currentLocation.y) - (pTarget->distanceToGround + (pTarget->currentLocation).y)) < this->field_0xb98 - 0.2f)) {
			bVar4 = true;
		}

		if (bVar4) {
			CActorFighter* pFighter = static_cast<CActorFighter*>(pTarget);
			bVar4 = false;
			if (((this->combatFlags_0xb78 & 0x30) != 0) &&
				(iVar5 = SV_AUT_CanMoveTo(&pFighter->currentLocation), iVar5 != 0)) {
				bVar4 = true;
			}

			if ((bVar4) && ((bVar4 = pFighter->IsKindOfObject(OBJ_TYPE_AUTONOMOUS), bVar4 == false || (AcquireAdversary(pFighter) == 0)))) {
				bVar4 = false;

				if ((pFighter != (CActor*)0x0) && ((pFighter->typeID == ACTOR_HERO_PRIVATE && (pFighter->curBehaviourId == 8)))) {
					bVar4 = true;
				}

				if ((!bVar4) || (pCVar6 = GetBehaviour(0x18), pCVar6 == (CBehaviour*)0x0)) {
					bCanSwitchToFight = true;
				}				
			}
		}
	}

	return bCanSwitchToFight;
}

void CActorWolfen::InternState_WolfenLocate()
{
	CAnimation* pAnimationController;
	float fVar2;
	float fVar3;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.x = this->currentLocation.x;
	local_10.z = this->currentLocation.z;
	local_10.w = this->currentLocation.w;
	local_10.y = this->currentLocation.y + this->field_0xcf0;

	pAnimationController = this->pAnimationController;

	if (((pAnimationController->anmBinMetaAnimator).aAnimData)->animPlayState == 1) {
		fVar2 = pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0, 0);
		fVar3 = fVar2;
		pAnimationController->anmBinMetaAnimator.GetAnimType_00242330(0);

		IMPLEMENTATION_GUARD_LOG(); // IS this correct?
		float cosValue = sinf(fabs(((fVar3 / fVar2) * 6.283185)));
		fVar3 = edF32Between_0_2Pi(-cosValue * (3.141593 - GetVision()->field_0x30 * 2.0f * 57.29578f * 0.01745329f * 0.5f) + this->rotationEuler.y);
		SetVectorFromAngleY(fVar3, &local_20);
		GetVision()->location = local_10;
		GetVision()->rotationQuat = local_20;
	}
	else {
		GetVision()->location = local_10;
		GetVision()->rotationQuat = this->rotationQuat;
	}

	return;
}

void CActorWolfen::TermFightAction()
{
	CActorCommander* this_00;
	undefined4* puVar1;
	long lVar2;

	if (this->activeCapabilityIndex != 3) {
		IMPLEMENTATION_GUARD(
		this_00 = this->pCommander;
		lVar2 = CActorCommander::FUN_00171440(this_00, (int)this);
		if (lVar2 != 0) {
			puVar1 = (undefined4*)0x0;
			if (this->activeCapabilityIndex != 3) {
				puVar1 = (undefined4*)(&this->field_0xa90 + this->activeCapabilityIndex * 0x34);
			}
			CActorCommander::ReleaseSemaphore(this_00, (undefined4*)*puVar1, (int)this);
		})
	}
	return;
}

bool CActorWolfen::CheckLost()
{
	return false;
}

void CActorWolfen::CheckValidPatterns(CRndChooser<CFightIA::WFIGS_Chain>* pRndChooser)
{
	bool bVar1;
	s_fighter_blow* psVar2;
	CActorWeapon* pCVar3;
	byte* pbVar4;
	uint uVar5;
	int iVar6;
	uint local_30;
	uint currentIndex;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	currentIndex = 0;
	if (pRndChooser->nbItems != 0) {
		do {
			iVar6 = pRndChooser->pItems[currentIndex].rndData.commandId;
			while (iVar6 != -1) {
				bVar1 = false;
				if (iVar6 == -1) {
					uVar5 = 0;
				}
				else {
					uVar5 = this->field_0xb64[iVar6].field_0x0;
				}

				if ((uVar5 & 0x1e0) != 0) {
					local_30 = 0;
					if (this->nbComboMatchValues != 0) {
						do {
							GetComboMatchValues(local_30, &local_4, &local_8, &local_c, &local_10);
							if ((((((uVar5 & 0x20) == 0) || (0.5 <= local_8)) && (((uVar5 & 0x40) == 0 || (0.5 <= local_4)))) &&
								(((uVar5 & 0x80) == 0 || (0.5 <= local_c)))) && (((uVar5 & 0x100) == 0 || (0.5 <= local_10)))) {
								bVar1 = true;
								break;
							}
							local_30 = local_30 + 1;
						} while (local_30 < (uint)this->nbComboMatchValues);
					}
				}

				if (((uVar5 & 0x200) != 0) &&
					(psVar2 = FindBlowByName("BASE_CATCH"),
						psVar2 != (s_fighter_blow*)0x0)) {
					bVar1 = true;
				}

				if (((uVar5 & 0x10) != 0) &&
					(pCVar3 = GetWeapon(), pCVar3 != (CActorWeapon*)0x0)) {
					bVar1 = true;
				}

				if (!bVar1) {
					pbVar4 = &pRndChooser->pItems[currentIndex].field_0x0;
					if (*pbVar4 != 0) {
						*pbVar4 = 0;
						pRndChooser->field_0x8 = 0;
					}
					break;
				}

				pbVar4 = &pRndChooser->pItems[currentIndex].field_0x0;
				if (*pbVar4 != 1) {
					*pbVar4 = 1;
					pRndChooser->field_0x8 = 0;
				}

				if (iVar6 == -1) {
					iVar6 = -1;
				}
				else {
					iVar6 = this->field_0xb64[iVar6].field_0x8;
				}
			}

			currentIndex = currentIndex + 1;
		} while (currentIndex < pRndChooser->nbItems);
	}

	return;
}

void CActorWolfen::GetComboMatchValues(int index, float* param_3, float* param_4, float* param_5, float* param_6)
{
	*param_3 = this->aComboMatchValues[index].x;
	*param_4 = this->aComboMatchValues[index].y;
	*param_5 = this->aComboMatchValues[index].z;
	*param_6 = this->aComboMatchValues[index].w;

	return;
}

bool CActorWolfen::CanPerformWeaponCommand()
{
	CActorWeapon* pWeapon;
	bool bVar2;

	pWeapon = static_cast<CActorWeapon*>(this->pWeaponActor.Get());

	bVar2 = false;
	if ((pWeapon != (CActorWeapon*)0x0) && (pWeapon->field_0x1d0 == 2)) {
		bVar2 = true;
	}

	return bVar2;
}

bool CActorWolfen::RequestFightAction(int index)
{
	uint uVar1;
	uint uVar2;
	WFIGS_Capability* pCabability;
	CBehaviourFighterWolfen* pCVar5;
	int iVar7;
	int pCVar9;
	int pCVar10;
	CActorCommander* pCommander;

	pCabability = this->aCapabilities + index;

	uVar1 = pCabability->field_0xc;
	uVar2 = pCabability->field_0x8;
	iVar7 = ((uVar1 - uVar2) + 1) * (CScene::Rand());
	if (iVar7 < 0) {
		iVar7 = iVar7 + 0x7fff;
	}

	pCVar9 = uVar2 + (iVar7 >> 0xf);
	uVar1 = pCabability->field_0x14;
	uVar2 = pCabability->field_0x10;

	iVar7 = ((uVar1 - uVar2) + 1) * (CScene::Rand());
	if (iVar7 < 0) {
		iVar7 = iVar7 + 0x7fff;
	}

	pCVar10 = uVar2 + (iVar7 >> 0xf);
	if ((pCVar9 != 0x0) && (pCVar10 != 0x0)) {
		if ((this->activeCapabilityIndex != 3) && (this->activeCapabilityIndex != 3)) {
			pCommander = this->pCommander;
			if (pCommander->IsValidEnemy(this) != false) {
				pCommander->ReleaseSemaphore(GetActiveCapability()->semaphoreId, this);
			}
		}

		if (this->pCommander->QuerySemaphoreCold(pCabability->semaphoreId, this) != 0) {
			pCVar5 = static_cast<CBehaviourFighterWolfen*>(GetBehaviour(3));
			pCVar5->field_0x24 = pCVar9;
			pCVar5->field_0x2c = pCVar10;
			pCVar5->field_0x30 = 0;
			pCVar5->field_0x28 = 0;
			this->activeCapabilityIndex = 3;
			this->field_0xb30 = index;
			return true;
		}
	}

	return false;
}

float CActorWolfen::SemaphoreEval()
{
	CBehaviourFighterWolfen* pCVar1;
	CBehaviourFighterWolfen::Rule* pCVar2;
	int iVar3;
	float fVar4;
	float fVar5;

	pCVar1 = static_cast<CBehaviourFighterWolfen*>(GetBehaviour(3));
	if (pCVar1 == (CBehaviourFighterWolfen*)0x0) {
		fVar5 = 0.0f;
	}
	else {
		fVar5 = 0.0f;
		iVar3 = 0;
		pCVar2 = pCVar1->aRules;
		if (pCVar1->pOwner->curBehaviourId == 3) {
			do {
				fVar4 = pCVar2->pFunc(this);
				iVar3 = iVar3 + 1;
				fVar5 = fVar5 + pCVar2->value * fVar4;
				pCVar2 = pCVar2 + 1;
			} while (iVar3 < 3);
		}
	}

	return fVar5;
}

bool CActorWolfen::SemaphoreKeepIt()
{
	CActorWolfen* pCVar1;
	bool bVar2;
	CBehaviourFighterWolfen* pFighterBehaviour;
	ulong uVar3;
	uint uVar4;
	WFIGS_Capability* pWVar5;

	uVar3 = this->pCommander->CanReleaseSemaphore(this);
	if ((uVar3 == 1) && (pFighterBehaviour = static_cast<CBehaviourFighterWolfen*>(GetBehaviour(3)), pFighterBehaviour != (CBehaviourFighterWolfen*)0x0)) {
		uVar4 = 0;
		if (pFighterBehaviour->currentCommandId != -1) {
			pCVar1 = static_cast<CActorWolfen*>(pFighterBehaviour->pOwner);
			uVar4 = pCVar1->field_0xb64[pFighterBehaviour->currentCommandId].field_0xc;
		}

		if ((uVar4 & 1) == 0) {
			pCVar1 = static_cast<CActorWolfen*>(pFighterBehaviour->pOwner);
			pWVar5 = (WFIGS_Capability*)0x0;
			if (pCVar1->activeCapabilityIndex != 3) {
				pWVar5 = pCVar1->aCapabilities + pCVar1->activeCapabilityIndex;
			}

			if (pWVar5 == (WFIGS_Capability*)0x0) {
				bVar2 = false;
			}
			else {
				pWVar5 = (WFIGS_Capability*)0x0;
				if (pCVar1->activeCapabilityIndex != 3) {
					pWVar5 = pCVar1->aCapabilities + pCVar1->activeCapabilityIndex;
				}

				bVar2 = (pWVar5->field_0x4 & 1) != 0;
			}

			if ((!bVar2) && (((uint)pFighterBehaviour->field_0x24 <= (uint)pFighterBehaviour->field_0x28 || ((uint)pFighterBehaviour->field_0x2c <= (uint)pFighterBehaviour->field_0x30)))) {
				return false;
			}
		}
	}

	return true;
}



void CActorWolfen::EnableFightAction()
{
	if (this->curBehaviourId == 3) {
		this->activeCapabilityIndex = this->field_0xb30;
		this->field_0xb30 = 3;
	}
	else {
		this->pCommander->ReleaseSemaphore(this->aCapabilities[this->field_0xb30].semaphoreId, this);
		this->field_0xb30 = 3;
		this->activeCapabilityIndex = 3;
	}

	if (gWolfenAnimMatrixData.aMatrices != (edF32MATRIX3*)0x0) {
		if (gWolfenAnimMatrixData.pWolfen != (CActorWolfen*)0x0) {
			gWolfenAnimMatrixData.pWolfen->pAnimationController->pAnimMatrix = (edF32MATRIX3*)0x0;
		}

		gWolfenAnimMatrixData.pWolfen = this;
		this->pAnimationController->SetBoneMatrixData(gWolfenAnimMatrixData.aMatrices, gWolfenAnimMatrixData.nbBones);
	}

	return;
}

void CActorWolfen::DisableFightAction()
{
	CBehaviourFighterWolfen* pCVar3;

	this->field_0xb30 = 3;
	this->activeCapabilityIndex = 3;

	if (this->curBehaviourId == 3) {
		pCVar3 = static_cast<CBehaviourFighterWolfen*>(GetBehaviour(3));
		pCVar3->ValidateCommand();
		pCVar3->currentCommandId = -1;
	}

	if ((gWolfenAnimMatrixData.aMatrices != (edF32MATRIX3*)0x0) && (gWolfenAnimMatrixData.pWolfen == this)) {
		gWolfenAnimMatrixData.pWolfen->pAnimationController->pAnimMatrix = (edF32MATRIX3*)0x0;
		gWolfenAnimMatrixData.pWolfen = (CActorWolfen*)0x0;
	}

	return;
}

void CActorWolfen::UpdateCombatMode()
{
	if ((this->combatFlags_0xb78 & 4) == 0) {
		if ((this->combatMode_0xb7c == ECM_None) && (this->combatMode_0xb7c != ECM_Alerted)) {
			this->combatMode_0xb7c = ECM_Alerted;
		}
	}
	else {
		if (this->combatMode_0xb7c != ECM_InCombat) {
			this->combatMode_0xb7c = ECM_InCombat;
		}
	}

	if ((this->combatMode_0xb7c < 2) && (this->combatMode_0xb7c != ECM_InCombat)) {
		this->combatMode_0xb7c = ECM_InCombat;
	}
}

WFIGS_Capability* CActorWolfen::GetActiveCapability()
{
	if (this->activeCapabilityIndex == 3) {
		return (WFIGS_Capability*)0x0;
	}
	else {
		return this->aCapabilities + this->activeCapabilityIndex;
	}
}

void CBehaviourWatchDog::Create(ByteCode* pByteCode)
{
	S_TARGET_STREAM_REF* piVar1;
	S_STREAM_EVENT_CAMERA* pSVar2;
	uint uVar3;
	CWayPoint* pCVar4;
	int iVar5;

	this->flags_0x4 = pByteCode->GetU32();
	this->field_0x80.index = pByteCode->GetS32();
	this->trackBehaviourId = pByteCode->GetS32();

	this->switchBehaviour.Create(pByteCode);

	this->pTargetStreamRef.Create(pByteCode);
	this->pCameraStreamEvent.Create(pByteCode);

	return;
}

void CBehaviourWatchDog::Init(CActor* pOwner)
{
	CWayPoint* pCVar1;
	S_TARGET_STREAM_REF* pSVar2;
	int iVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR3 local_10;

	this->baseLocation = pOwner->baseLocation;
	this->rotationEuler.xyz = pOwner->pCinData->rotationEuler;

	this->pOwner = reinterpret_cast<CActorWolfen*>(pOwner);

	this->field_0x80.Init();

	pCVar1 = this->field_0x80.Get();
	if (pCVar1 == (CWayPoint*)0x0) {
		this->baseLocation = pOwner->baseLocation;
		this->rotationEuler.xyz = pOwner->pCinData->rotationEuler;
	}
	else {
		this->baseLocation.xyz = pCVar1->location;
		(this->baseLocation).w = 1.0f;

		local_10 = pCVar1->rotation;
		this->pOwner->SV_BuildAngleWithOnlyY(&this->rotationEuler.xyz, &local_10);
		this->rotationEuler.w = 0.0f;
	}

	this->switchBehaviour.Init(pOwner);

	this->pTargetStreamRef.Init();

	this->pCameraStreamEvent->Init();
	return;
}

void CBehaviourWatchDog::Manage()
{
	this->pCameraStreamEvent->Manage(this->pOwner);
	this->pOwner->BehaviourWatchDog_Manage(this);

	return;
}

void CBehaviourWatchDog::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorWolfen* pCVar1;
	undefined4 uVar2;
	S_TARGET_STREAM_REF* pSVar3;
	int iVar4;
	int iVar5;

	if (newState == -1) {
		this->pOwner->SetState(this->GetStateWolfenComeBack(), -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->switchBehaviour.Begin(pOwner);

	this->bool_0x68 = false;

	for (int i = 0; i < this->pTargetStreamRef->entryCount; i++) {
		this->pTargetStreamRef->aEntries[i].Reset();
	}

	this->pCameraStreamEvent->Reset(pOwner);

	return;
}

void CBehaviourWatchDog::End(int newBehaviourId)
{
	CActorWolfen* pActor;
	int iVar3;

	this->pOwner->SV_AUT_PathfindingEnd();

	if ((this->bool_0x68 != false) && (this->bool_0x68 == true)) {
		pActor = this->pOwner;
		iVar3 = 0;
		if (0 < this->pTargetStreamRef->entryCount) {
			do {
				this->pTargetStreamRef->aEntries[iVar3].SwitchOff(this->pOwner);
				iVar3 = iVar3 + 1;
			} while (iVar3 < this->pTargetStreamRef->entryCount);
		}
	}
	this->bool_0x68 = false;
	return;
}

int CBehaviourWatchDog::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;

	iVar1 = this->switchBehaviour.InterpretMessage(this->pOwner, pSender, msg, pMsgParam);
	if (iVar1 == 0) {
		iVar1 = CBehaviourWolfen::InterpretMessage(pSender, msg, pMsgParam);
	}
	return iVar1;
}

int CBehaviourWatchDog::GetTrackBehaviour()
{
	this->pOwner->GetBehaviour(this->trackBehaviourId);
	return this->trackBehaviourId;
}

void CBehaviourWolfen::Create(ByteCode* pByteCode)
{
	this->flags_0x4 = pByteCode->GetU32();

	return;
}

void CBehaviourWolfen::Init(CActor* pOwner)
{

	this->baseLocation = pOwner->baseLocation;
	this->rotationEuler.xyz = pOwner->pCinData->rotationEuler;

	this->pOwner = reinterpret_cast<CActorWolfen*>(pOwner);

	return;
}

void CBehaviourWolfen::End(int newBehaviourId)
{	
	this->pOwner->SV_AUT_PathfindingEnd();

	return;
}

void CBehaviourWolfen::InitState(int newState)
{
	CAnimation* pCVar1;
	bool bVar2;
	StateConfig* pSVar3;
	CActorWolfen* pCVar4;
	uint uVar5;
	int iVar6;
	int mode;
	ulong uVar7;
	float fVar8;

	if (newState == 0x74) {
		pSVar3 = this->pOwner->GetStateCfg(0x74);
		iVar6 = this->pOwner->GetIdMacroAnim(pSVar3->animId);
		if (iVar6 < 0) {
			fVar8 = 0.0f;
			pCVar4 = this->pOwner;
		}
		else {
			fVar8 = this->pOwner->pAnimationController->GetAnimLength(iVar6, 0);
		}

		this->pOwner->field_0xd18 = fVar8;
	}
	else {
		if (newState != WOLFEN_STATE_LOCATE) {
			if (newState != WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION) {
				if (newState == 0xb3) {
					this->pOwner->SetCombatMode(ECM_None);
				}
				else {
					if (newState == WOLFEN_STATE_TRACK_CHASE) {
						IMPLEMENTATION_GUARD_AUDIO(
						GlobalSound::Func_001844f0(CScene::ptable.g_AudioManager_00451698);)
					}
					else {
						if ((newState != WOLFEN_STATE_COME_BACK) && (newState == WOLFEN_STATE_SURPRISE)) {
							this->pOwner->field_0xcfc = this->pOwner->rotationEuler.y;
							pCVar4 = this->pOwner;
							pCVar1 = pCVar4->pAnimationController;
							iVar6 = pCVar4->GetIdMacroAnim(pCVar4->currentAnimType);
							if (iVar6 < 0) {
								fVar8 = 0.0f;
							}
							else {
								fVar8 = pCVar1->GetAnimLength(iVar6, 1);
							}

							this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar8 / 0.3f, 0);
						}
					}
				}

				goto LAB_001f1738;
			}

			this->pOwner->SetCombatMode(ECM_None);
		}

		fVar8 = ((float)CScene::Rand() * 0.4f) / 32767.0f + 0.8f;

		if ((this->pOwner->combatFlags_0xb78 & 0x100000) == 0) {
			fVar8 = fVar8 * 0.7f;
		}

		this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar8, 0);
	}

LAB_001f1738:
	uVar5 = this->pOwner->GetStateWolfenFlags(newState);
	if ((uVar5 & 4) != 0) {
		pCVar1 = this->pOwner->pAnimationController;
		iVar6 = pCVar1->PhysicalLayerFromLayerId(8);
		mode = this->pOwner->GetIdMacroAnim(0xa2);
		bVar2 = pCVar1->IsLayerActive(8);
		if ((bVar2 != false) && (mode != -1)) {
			pCVar1->anmBinMetaAnimator.SetLayerBlendingOp(iVar6, 3);
			pCVar1->anmBinMetaAnimator.SetAnimOnLayer(mode, iVar6, 0xa2);
			pCVar4 = this->pOwner;
			pCVar4->field_0xd24 = 0.0f;
			pCVar4->field_0xd28 = 1.0f;
		}
	}
	return;
}

void CBehaviourWolfen::TermState(int oldState, int newState)
{
	edANM_HDR* peVar1;
	CActorWolfen* pCVar2;
	bool bVar3;
	uint uVar4;
	int iVar5;
	CActorProjectile* pProjectile;
	int AVar6;
	edAnmLayer* peVar7;
	CAnimation* pAnim;

	uVar4 = this->pOwner->GetStateWolfenFlags(oldState);
	if ((uVar4 & 4) != 0) {
		pAnim = this->pOwner->pAnimationController;
		iVar5 = pAnim->PhysicalLayerFromLayerId(8);
		AVar6 = this->pOwner->GetIdMacroAnim(0xa2);
		peVar7 = (pAnim->anmBinMetaAnimator).aAnimData + iVar5;
		bVar3 = pAnim->IsLayerActive(8);
		if (((bVar3 != false) && (AVar6 != -1)) && ((peVar7->currentAnimDesc).animType == AVar6)) {
			peVar1 = (peVar7->currentAnimDesc).state.pAnimKeyTableEntry;
			if ((peVar1->field_0x4.asKey == 1) && (peVar1->keyIndex_0x8.asKey == 2)) {
				AnimKeySomething* peVar8 = reinterpret_cast<AnimKeySomething*>(peVar1);
				peVar8->field_0x14 = 0.5f;
				peVar8->field_0x1c = 0.5f;
			}

			pAnim->anmBinMetaAnimator.SetAnimOnLayer(-1, iVar5, 0xffffffff);
		}
	}

	if (oldState == 0xa5) {
		pCVar2 = this->pOwner;
		pProjectile = pCVar2->pTrackedProjectile;
		if (pCVar2 == (CActorWolfen*)0x0) {
			bVar3 = pProjectile->field_0x40c != (CActor*)0x0;
		}
		else {
			bVar3 = pCVar2 == pProjectile->field_0x40c;
		}
		if (bVar3) {
			pProjectile->field_0x40c = (CActor*)0x0;
		}

		this->pOwner->pTrackedProjectile = (CActorProjectile*)0x0;
	}
	else {
		if (oldState == 0xa4) {
			if (newState != 0xa5) {
				this->pOwner->pTrackedProjectile->field_0x40c = (CActor*)0x0;
				this->pOwner->pTrackedProjectile = (CActorProjectile*)0x0;
			}
		}
		else {
			if (oldState == 0xa3) {
				if ((newState != 0xa4) && (newState != 0xa5)) {
					this->pOwner->pTrackedProjectile->field_0x40c = (CActor*)0x0;
					this->pOwner->pTrackedProjectile = (CActorProjectile*)0x0;
				}
			}
			else {
				if ((oldState == WOLFEN_STATE_LOCATE) || (oldState == WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION)) {
					this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
				}
				else {
					if (oldState == WOLFEN_STATE_COME_BACK) {
						this->pOwner->combatFlags_0xb78 = this->pOwner->combatFlags_0xb78 & 0xffefffff;
					}
					else {
						if (oldState == WOLFEN_STATE_SURPRISE) {
			
							this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
							if ((newState != 0xa1) && (newState != 0xa3)) {
								pCVar2 = this->pOwner;
								pProjectile = pCVar2->pTrackedProjectile;
								if (pProjectile != (CActorProjectile*)0x0) {
									if (pCVar2 == (CActorWolfen*)0x0) {
										bVar3 = pProjectile->field_0x40c != (CActor*)0x0;
									}
									else {
										bVar3 = pCVar2 == pProjectile->field_0x40c;
									}
									if (bVar3) {
										pProjectile->field_0x40c = (CActor*)0x0;
									}
								}

								this->pOwner->pTrackedProjectile = (CActorProjectile*)0x0;
							}
						}
						else {
							if (oldState == WOLFEN_STATE_TRACK_CHASE) {
								IMPLEMENTATION_GUARD_AUDIO(
								FUN_001844c0(CScene::ptable.g_AudioManager_00451698);)
								this->pOwner->SV_AUT_PathfindingEnd();
							}
						}
					}
				}
			}
		}
	}

	return;
}

int CBehaviourWolfen::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorWolfen* pCVar1;
	bool bVar2;
	CLifeInterface* pCVar3;
	int iVar4;
	long lVar5;
	ulong uVar6;
	float fVar7;
	edF32VECTOR4 local_b0;
	float local_a0;
	float local_9c;
	float local_98;
	float local_94;
	undefined4 local_90[2];
	undefined4 local_88;
	undefined4 local_84;
	undefined4 local_80;
	edF32VECTOR4 eStack112;
	undefined4 local_60;
	undefined4 local_50;
	undefined4 local_4c;
	undefined4 local_48;
	undefined4 local_44;
	undefined2 local_40;
	float local_30;
	float local_2c;
	float local_28;
	float local_24;
	undefined4 local_20;
	undefined4* local_4;

	if (msg == 0xb) {
		IMPLEMENTATION_GUARD(
		pCVar1 = this->pOwner;
		bVar2 = false;
		lVar5 = (*(code*)pSender->pVTable->IsMakingNoise)((long)(int)pSender);
		if (lVar5 != 0) {
			local_a0 = pCVar1->currentLocation.x;
			local_98 = pCVar1->currentLocation.z;
			local_94 = pCVar1->currentLocation.w;
			local_9c = pCVar1->currentLocation.y + pCVar1->field_0xcf0;
			CActor::SV_GetActorColCenter(pSender, &local_b0);
			if (((local_a0 - local_b0.x) * (local_a0 - local_b0.x) + (local_98 - local_b0.z) * (local_98 - local_b0.z) <
				*(float*)&pCVar1->field_0xc84) && (fabs(local_9c - local_b0.y) < *(float*)&pCVar1->field_0xc88)) {
				bVar2 = true;
			}
		}
		if ((!bVar2) && (uVar6 = FUN_00173550((long)(int)this->pOwner, (long)(int)pSender), uVar6 == 0)) {
			return 0;
		}
		this->pOwner->combatFlags_0xb78 = this->pOwner->combatFlags_0xb78 | 0x4000;
		*(CActor**)&this->pOwner->field_0xd0c = pSender;)
		return 1;
	}

	if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
		if (this->pOwner->GetLifeInterface()->GetValue() <= 0.0f) {
			return 0;
		}

		GetPositionMsgParams* pMsgParamPos = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);

		/* WARNING: Load size is inaccurate */
		if ((pMsgParamPos->field_0x0 != 1) && (pMsgParamPos->field_0x0 != 0)) {
			return 0;
		}

		pMsgParamPos->vectorFieldB.x = 0.0f;
		pMsgParamPos->vectorFieldB.y = 1.5f;
		pMsgParamPos->vectorFieldB.z = 0.0f;
		pMsgParamPos->vectorFieldB.w = 1.0f;

		return 1;
	}

	if (msg != 2) {
		return 0;
	}

	/* WARNING: Load size is inaccurate */
	int* pMsgParamInt = (int*)pMsgParam;
	iVar4 = *pMsgParamInt;
	if (iVar4 == 4) {
		IMPLEMENTATION_GUARD(
		this->pOwner->combatFlags_0xb78 = this->pOwner->combatFlags_0xb78 | 0x40;)
		return 1;
	}
	if (((iVar4 != 7) && (iVar4 != 8)) && (iVar4 != 10)) {
		IMPLEMENTATION_GUARD(
		pCVar3 = (*(this->pOwner->pVTable)->GetLifeInterface)((CActor*)this->pOwner);
		fVar7 = (*pCVar3->pVtable->GetValue)((CInterface*)pCVar3);
		if (0.0 < fVar7) {
			/* WARNING: Load size is inaccurate */
			if ((pSender->typeID != 0x20) && (*pMsgParam != 4)) {
				(*(this->pOwner->pVTable)->LifeDecrease)
					(*(float*)((int)pMsgParam + 0xc), (CActorAutonomous*)this->pOwner);
			}
			pCVar3 = (*(this->pOwner->pVTable)->GetLifeInterface)((CActor*)this->pOwner);
			fVar7 = (*pCVar3->pVtable->GetValue)();
			if (fVar7 <= 0.0) {
				local_90[0] = 7;
				local_88 = 0;
				local_84 = 0;
				local_80 = 0;
				local_20 = 0;
				local_30 = g_xVector.x;
				local_2c = g_xVector.y;
				local_28 = g_xVector.z;
				local_24 = g_xVector.w;
				local_40 = 1;
				local_50 = *(undefined4*)((int)pMsgParam + 0x40);
				local_4c = *(undefined4*)((int)pMsgParam + 0x44);
				local_48 = *(undefined4*)((int)pMsgParam + 0x48);
				local_44 = *(undefined4*)((int)pMsgParam + 0x4c);
				local_60 = 0x41800000;
				edF32Vector4SubHard(&eStack112, &this->pOwner->currentLocation,
					&pSender->currentLocation);
				edF32Vector4SafeNormalize1Hard(&eStack112, &eStack112);
				local_4 = local_90;
				CActor::DoMessage((CActor*)this->pOwner, (CActor*)this->pOwner, 2, (uint)local_4);
			}
			iVar4 = 1;
			goto LAB_001f0ee8;
		})
	}

	iVar4 = 0;
LAB_001f0ee8:
	this->pOwner->combatFlags_0xb78 = this->pOwner->combatFlags_0xb78 | 0x100;

	return iVar4;
}

edF32VECTOR4* CBehaviourWolfen::GetComeBackPosition()
{
	edF32MATRIX4 eStack64;
	CActor* pActor;

	pActor = this->pOwner->pTiedActor;
	if (pActor == (CActor*)0x0) {
		this->comeBackPosition = this->baseLocation;
	}
	else {
		pActor->SV_ComputeDiffMatrixFromInit(&eStack64);
		edF32Matrix4MulF32Vector4Hard(&this->comeBackPosition, &eStack64, &this->baseLocation);
	}

	return &this->comeBackPosition;
}

edF32VECTOR4* CBehaviourWolfen::GetComeBackAngles()
{
	float fVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 eStack80;
	edF32MATRIX4 eStack64;
	CActor* pActor;

	pActor = this->pOwner->pTiedActor;
	if (pActor == (CActor*)0x0) {
		this->comeBackAngles = this->rotationEuler;
	}
	else {
		pActor->SV_ComputeDiffMatrixFromInit(&eStack64);
		SetVectorFromAngles(&eStack80, &this->rotationEuler.xyz);
		edF32Matrix4MulF32Vector4Hard(&eStack80, &eStack64, &eStack80);
		GetAnglesFromVector(&this->comeBackAngles.xyz, &eStack80);
		(this->comeBackAngles).w = 0.0f;
	}

	return &this->comeBackAngles;
}

int CBehaviourWolfen::GetTrackBehaviour()
{
	return -1;
}

CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* CBehaviourWolfen::GetNotificationTargetArray()
{
	return 0;
}

int CBehaviourWolfen::GetStateWolfenComeBack()
{
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard_I(&eStack16, GetComeBackPosition(), &this->pOwner->currentLocation);
	float magnitude = edF32Vector4DotProductHard_I(&eStack16, &eStack16);

	int iVar6;

	if (0.5f <= sqrtf(magnitude)) {
		iVar6 = WOLFEN_STATE_COME_BACK;
	}
	else {
		iVar6 = GetStateWolfenGuard();
	}

	return iVar6;
}

int CBehaviourWolfen::GetStateWolfenTrack()
{
	int stateTrack;

	stateTrack = -1;

	if (GetTrackBehaviour() != this->pOwner->curBehaviourId) {
		stateTrack = 0xb1;
	}

	return stateTrack;
}

int CBehaviourWolfen::GetStateWolfenGuard()
{
	return WOLFEN_STATE_WATCH_DOG_GUARD;
}

int CBehaviourWolfen::GetStateWolfenWeapon(void)
{
	return WOLFEN_STATE_TRACK_DEFEND;
}

void CBehaviourWolfen::CheckDetection()
{
	CActorWolfen* pWolfen;
	CActorFighter* pTarget;

	pWolfen = this->pOwner;
	pTarget = pWolfen->pTargetActor_0xc80;

	if ((this->flags_0x4 & 1) == 0) {
		if ((pWolfen->combatFlags_0xb78 & 1) != 0) {
			pWolfen->combatFlags_0xb78 = pWolfen->combatFlags_0xb78 & 0xfffffffe;
		}
	}
	else {
		pWolfen->SV_WLF_IsIntruderMakingNoise(pTarget);
	}

	if ((this->flags_0x4 & 2) == 0) {
		pWolfen = this->pOwner;
		if ((pWolfen->combatFlags_0xb78 & 2) != 0) {
			pWolfen->combatFlags_0xb78 = pWolfen->combatFlags_0xb78 & 0xfffffffd;
		}
	}
	else {
		pWolfen->SV_WLF_IsIntruderInVitalSphere(pTarget);
	}

	if ((this->flags_0x4 & 4) == 0) {
		pWolfen = this->pOwner;
		if ((pWolfen->combatFlags_0xb78 & 4) != 0) {
			pWolfen->combatFlags_0xb78 = pWolfen->combatFlags_0xb78 & 0xfffffffb;
		}
	}
	else {
		pWolfen->SV_WLF_IsIntruderInVision(pTarget);
	}

	return;
}

void CBehaviourWolfen::CheckDetection_Intruder()
{
	uint uVar2;
	float fVar4;

	uVar2 = this->pOwner->GetStateWolfenFlags(this->pOwner->actorState);
	if ((uVar2 & 1) == 0) {
		return;
	}

	if ((this->pOwner->combatFlags_0xb78 & 0x10) != 0) {
		fVar4 = this->pOwner->pTargetActor_0xc80->GetLifeInterface()->GetValue();
		if (0.0f < fVar4) {
			CheckDetection();
			goto LAB_001f0688;
		}
	}

	this->pOwner->combatFlags_0xb78 = this->pOwner->combatFlags_0xb78 & 0xfffffff8;

LAB_001f0688:
	this->pOwner->UpdateInRange_001744a0((this->pOwner->combatFlags_0xb78 & 7) != 0);
	return;
}

int CBehaviourWolfen::FUN_001f0ab0()
{
	int iVar1;
	CActorWolfen* pWolfen;

	pWolfen = this->pOwner;
	iVar1 = -1;
	if ((pWolfen->combatFlags_0xb78 & 0x20000) == 0) {
		if ((pWolfen->pCollisionData->flags_0x4 & 2) != 0) {
			pWolfen->combatFlags_0xb78 = pWolfen->combatFlags_0xb78 | 0x20000;
		}
	}
	else {
		if ((pWolfen->fightFlags & 2) != 0) {
			iVar1 = pWolfen->FUN_0030a6a0();
		}
	}

	return iVar1;
}

int CBehaviourWolfen::TestState_001f09b0()
{
	CActorWolfen* pEnemy;
	int iVar1;

	pEnemy = this->pOwner;
	iVar1 = -1;
	if (((pEnemy->combatFlags_0xb78 & 0x400) != 0) && (0 < (int)pEnemy->combatMode_0xb7c)) {
		iVar1 = pEnemy->GetState_00174190();
	}

	return iVar1;
}

int CBehaviourWolfen::TestState_001f0a00()
{
	CActorWolfen* pCVar1;
	int iVar2;

	pCVar1 = this->pOwner;
	iVar2 = -1;
	if ((pCVar1->combatFlags_0xb78 & 0x4000) != 0) {
		iVar2 = 0x9f;
		pCVar1->combatFlags_0xb78 = pCVar1->combatFlags_0xb78 & 0xffffbfff;
	}

	return iVar2;
}

int CBehaviourWolfen::TestState_001f0a30()
{
	CActorWolfen* pCVar1;
	int iVar2;

	pCVar1 = this->pOwner;
	iVar2 = -1;
	if ((pCVar1->combatFlags_0xb78 & 0x8000) != 0) {
		iVar2 = 0xb5;
		pCVar1->combatFlags_0xb78 = pCVar1->combatFlags_0xb78 & 0xffff7fff;
	}

	return iVar2;
}

int CBehaviourWolfen::TestState_001f0a70()
{
	int iVar1;

	iVar1 = -1;
	if ((this->pOwner->combatFlags_0xb78 & 0x100) != 0) {
		iVar1 = WOLFEN_STATE_LOCATE;
	}

	return iVar1;
}

int CBehaviourWolfen::TestState_001f0a90()
{
	int iVar1;

	iVar1 = -1;
	if ((this->pOwner->combatFlags_0xb78 & 0x40) != 0) {
		iVar1 = 0x98;
	}

	return iVar1;
}

int CBehaviourWolfen::GetState_001f0930()
{
	bool bVar1;
	CBehaviour* pBehaviour;
	int newState;

	newState = -1;

	if ((((this->pOwner->pTargetActor_0xc80 != (CActorFighter*)0x0) && (0 < (int)this->pOwner->combatMode_0xb7c)) &&
		(bVar1 = this->pOwner->FUN_001738e0(this->pOwner->pTargetActor_0xc80), bVar1 != false)) &&
		(pBehaviour = this->pOwner->GetBehaviour(WOLFEN_BEHAVIOUR_DCA), pBehaviour != (CBehaviour*)0x0)) {
		newState = 0xb4;
	}

	return newState;
}

int CBehaviourWolfen::GetState_001f0b30()
{
	int newState;

	newState = 0x9b;
	if (1 < (int)this->pOwner->combatMode_0xb7c) {
		newState = 0x9a;
	}

	return newState;
}

int CBehaviourWolfen::GetState_001f08a0()
{
	CActorFighter* pIntruder;
	bool bVar1;
	int iVar2;
	int newState;

	pIntruder = this->pOwner->pTargetActor_0xc80;
	newState = -1;

	if (((pIntruder != (CActorFighter*)0x0) && (bVar1 = this->pOwner->CanSwitchToFight_Area(pIntruder), bVar1 != false)) &&
		(iVar2 = this->pOwner->pCommander->BeginFightIntruder(this->pOwner, pIntruder), iVar2 != 0)) {
		IMPLEMENTATION_GUARD(
		(*(code*)this->pOwner->pVTable[1].actorBase)(this->pOwner, pIntruder);)
		newState = 0xb0;
	}

	return newState;
}

void CBehaviourTrack::Create(ByteCode* pByteCode)
{
	CBehaviourWolfen::Create(pByteCode);

	GetNotificationTargetArray()->field_0x0 = pByteCode->GetF32();
}

void CBehaviourTrack::Manage()
{
	this->pOwner->BehaviourTrack_Manage(this);

	return;
}

void CBehaviourTrack::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	EEnemyCombatMode EVar1;
	CActorWolfen* pCVar2;
	bool bVar3;
	CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* pCVar4;
	int iVar5;
	undefined4 uVar6;
	long lVar7;

	if (this->pOwner->IsFightRelated(this->pOwner->prevBehaviourId) == 0) {
		GetNotificationTargetArray()->field_0x34 = this->pOwner->prevBehaviourId;
	}

	if (newState == -1) {
		this->pOwner->SetState(0x73, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	GetNotificationTargetArray()->FUN_003c3a30();
	GetNotificationTargetArray()->combatMode = this->pOwner->combatMode_0xb7c;

	if ((int)this->pOwner->combatMode_0xb7c < 2) {
		this->pOwner->combatFlags_0xb78 = this->pOwner->combatFlags_0xb78 & 0xffefffff;
	}
	else {
		this->pOwner->combatFlags_0xb78 = this->pOwner->combatFlags_0xb78 | 0x100000;
	}

	if (newState == -1) {
		if ((this->pOwner->IsFightRelated(this->pOwner->prevBehaviourId) != false) && ((this->pOwner->combatFlags_0xb78 & 0x80000) != 0)) {
			if (this->pOwner->IsCurrentPositionValid() == false) {
				this->pOwner->SetState(0x78, -1);
			}
			else {
				this->pOwner->SetState(GetStateWolfenWeapon(), -1);
			}
		}
	}

	return;
}

void CBehaviourTrack::End(int newBehaviourId)
{
	CBehaviourWolfen::End(newBehaviourId);

	this->pOwner->combatFlags_0xb78 = this->pOwner->combatFlags_0xb78 & 0xfffffebf;

	return;
}

int CBehaviourTrack::Func_0x70()
{
	return GetNotificationTargetArray()->GetState_003c37c0(this->pOwner);
}

void HearingDetectionProps::Create(ByteCode* pByteCode)
{
	const float range = pByteCode->GetF32();
	this->rangeSquared = range * range;
	this->maxHeightDifference = pByteCode->GetF32();

	return;
}

void VisionDetectionProps::Create(ByteCode* pByteCode)
{
	this->field_0x0 = pByteCode->GetF32();
	return;
}

void CBehaviourWolfenWeapon::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner->GetWeapon();
	if (newState == -1) {
		this->pOwner->SetState(0x77, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->flags = this->pOwner->flags | 0x400;
	
	return;
}

void CBehaviourWolfenWeapon::End(int newBehaviourId)
{
	this->pOwner->flags = this->pOwner->flags & 0xfffffbff;

	CBehaviourWolfen::End(newBehaviourId);

	return;
}

void CBehaviourTrackWeapon::Create(ByteCode* pByteCode)
{
	CBehaviourTrackWeaponStand::Create(pByteCode);

	this->field_0xf0 = pByteCode->GetF32();
	GetNotificationTargetArray()->field_0x0 = pByteCode->GetF32();
}

void CBehaviourTrackWeapon::Init(CActor* pOwner)
{
	CBehaviourTrackWeaponStand::Init(pOwner);

	return;
}

void CBehaviourTrackWeapon::Manage()
{
	int iVar1;
	int iVar3;

	this->pOwner->BehaviourTrackWeapon_Manage(this);
	iVar1 = FUN_002faf40();
	if (iVar1 != -1) {
		if (this->field_0xe8 != iVar1) {
			this->field_0xe8 = iVar1;
		}

		iVar1 = this->aSubObjs[this->field_0xe8].field_0x10;
		iVar3 = this->pOwner->GetWeapon()->FUN_002d57c0();
		if (iVar1 != iVar3) {
			this->pOwner->GetWeapon()->FUN_002d57e0(iVar1);
		}
	}

	return;
}

void CBehaviourTrackWeapon::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorWolfen* pWolfen;
	bool bVar2;
	int iVar3;

	if (newState == -1) {
		pWolfen = this->pOwner;
		bVar2 = pWolfen->IsFightRelated(pWolfen->prevBehaviourId);
		if ((bVar2 == false) || (pWolfen = this->pOwner, (pWolfen->combatFlags_0xb78 & 0x80000) == 0)) {
			newState = Func_0x70();
			newAnimationType = 6;
			if ((this->pOwner)->combatMode_0xb7c != ECM_InCombat) {
				newAnimationType = 7;
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			iVar3 = pWolfen->FUN_00173a20();
			if (iVar3 == 0) {
				newState = 0x78;
			}
			else {
				newState = Func_0x70();
			})
		}
	}

	CBehaviourTrackWeaponStand::Begin(pOwner, newState, newAnimationType);

	return;
}

void CBehaviourTrackWeapon::TermState(int oldState, int newState)
{
	if (oldState == WOLFEN_STATE_TRACK_CHASE) {
		this->pOwner->SV_AUT_PathfindingEnd();
	}

	if ((oldState == WOLFEN_STATE_RELOAD) || (oldState == WOLFEN_STATE_FIRE)) {
		this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
	}

	CBehaviourWolfen::TermState(oldState, newState);

	return;
}

int CBehaviourTrackWeapon::Func_0x70()
{
	if (this->pOwner->GetWeapon()->FUN_002d58a0() != 0) {
		if (this->pOwner->GetWeapon()->FUN_002d5830() == 0) {
			this->pOwner->field_0xcf8 = WOLFEN_STATE_TRACK_CHASE;
			return 0x97;
		}
	};

	return WOLFEN_STATE_TRACK_CHASE;
}

void CBehaviourTrackWeapon::Func_0x80(int* param_2, int* param_3, CActor* pTarget)
{
	CActorWolfen* pCVar1;
	float fVar2;
	float fVar3;

	pCVar1 = this->pOwner;
	fVar3 = (pTarget->currentLocation).x - pCVar1->currentLocation.x;
	fVar2 = (pTarget->currentLocation).z - pCVar1->currentLocation.z;
	if (this->field_0xf0 < sqrtf(fVar3 * fVar3 + 0.0f + fVar2 * fVar2)) {
		*param_3 = *param_3 + 1;
	}
	else {
		fVar3 = pTarget->SV_GetDirectionalAlignmentToTarget(&this->pOwner->currentLocation);
		if (0.001f < fVar3) {
			*param_2 = *param_2 + 1;
		}
		else {
			if (fVar3 < 0.001f) {
				*param_3 = *param_3 + 1;
			}
		}
	}

	return;
}

void CBehaviourTrackWeaponStand::Create(ByteCode* pByteCode)
{
	uint count;
	int iVar1;
	int* pBase;
	TrackSubObj* pTVar2;
	int iVar3;
	float fVar5;

	CBehaviourWolfen::Create(pByteCode);

	this->field_0x90 = pByteCode->GetF32();
	this->field_0x94 = pByteCode->GetS32();
	this->field_0x98 = pByteCode->GetS32();
	this->nbSubObjs = pByteCode->GetS32();

	this->aSubObjs = new TrackSubObj[this->nbSubObjs];

	iVar1 = 0;
	if (0 < this->nbSubObjs) {
		do {
			this->aSubObjs[iVar1].field_0x0 = pByteCode->GetS32();
			this->aSubObjs[iVar1].field_0x4.Create(pByteCode);
			this->aSubObjs[iVar1].field_0x10 = pByteCode->GetS32();
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->nbSubObjs);
	}

	return;
}

void CBehaviourTrackWeaponStand::Init(CActor* pOwner)
{
	CBehaviourWolfen::Init(pOwner);

	if (this->pOwner->field_0xb90 <= this->field_0x90) {
		this->pOwner->GetVision();
	}

	return;
}

void CBehaviourTrackWeaponStand::Term()
{
	TrackSubObj* pTVar1;

	pTVar1 = this->aSubObjs;
	if (pTVar1 != (TrackSubObj*)0x0) {
		if (pTVar1 != (TrackSubObj*)0x0) {
			IMPLEMENTATION_GUARD_LOG(); // Diff from expected code.
			delete this->aSubObjs;
		}

		this->aSubObjs = (TrackSubObj*)0x0;
	}

	return;
}

void CBehaviourTrackWeaponStand::Manage()
{
	int iVar1;
	int iVar3;

	this->pOwner->BehaviourTrackWeaponStand_Manage(this);
	iVar1 = FUN_002faf40();
	if (iVar1 != -1) {
		if (this->field_0xe8 != iVar1) {
			this->field_0xe8 = iVar1;
		}

		iVar1 = this->aSubObjs[this->field_0xe8].field_0x10;
		iVar3 = this->pOwner->GetWeapon()->FUN_002d57c0();
		if (iVar1 != iVar3) {
			this->pOwner->GetWeapon()->FUN_002d57e0(iVar1);
		}
	}

	return;
}

void CBehaviourTrackWeaponStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorWolfen* pCVar1;
	uint uVar2;
	EEnemyCombatMode EVar3;
	bool bVar4;
	CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* paVar5;
	int iVar6;
	CActorWeapon* pActor;
	CActor* pCVar7;
	CActor* pOtherActor;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	this->pOwner->GetWeapon();

	if (newState == -1) {
		pCVar1 = this->pOwner;
		pCVar1->SetState(0x77, -1);
	}
	else {
		pCVar1 = this->pOwner;
		pCVar1->SetState(newState, newAnimationType);
	}
	pCVar1 = this->pOwner;
	pCVar1->flags = pCVar1->flags | 0x400;

	this->field_0xe8 = 0;
	this->field_0xec = 0;

	pCVar1 = this->pOwner;
	bVar4 = pCVar1->IsFightRelated(pCVar1->prevBehaviourId);
	if (bVar4 == false) {
		GetNotificationTargetArray()->field_0x34 = this->pOwner->prevBehaviourId;
	}

	if (newState == -1) {
		pCVar1 = this->pOwner;
		bVar4 = pCVar1->IsFightRelated(pCVar1->prevBehaviourId);
		if ((bVar4 == false) || (pCVar1 = this->pOwner, (pCVar1->combatFlags_0xb78 & 0x80000) == 0)) {
			pCVar1 = this->pOwner;
			pCVar1->SetState(GetStateWolfenWeapon(), -1);
		}
		else {
			IMPLEMENTATION_GUARD(
			iVar6 = FUN_00173a20((long)(int)pCVar1);
			if (iVar6 == 0) {
				pCVar1 = this->pOwner;
				pCVar1->SetState(0x78, -1);
			}
			else {
				pCVar1 = this->pOwner;
				iVar6 = (*(code*)(this->pVTable)->field_0x6c)(this);
				pCVar1->SetState(iVar6, -1);
			})
		}
	}
	else {
		pCVar1 = this->pOwner;
		pCVar1->SetState(newState, newAnimationType);
	}

	pCVar1 = this->pOwner;
	pOtherActor = pCVar1->pTargetActor_0xc80;
	if (pOtherActor == (CActor*)0x0) {
		edF32Vector4ScaleHard(3.0f, &local_10, &pCVar1->rotationQuat);
		edF32Vector4AddHard(&local_10, &local_10, &this->pOwner->currentLocation);
		this->field_0x80 = local_10;
	}
	else {
		uVar2 = this->aSubObjs[this->field_0xe8].field_0x4.field_0x0;
		pActor = pCVar1->GetWeapon();
		this->pOwner->SV_GetActorHitPos(pOtherActor, &local_20);
		if ((uVar2 & 1) != 0) {
			edF32Vector4SubHard(&eStack48, &local_20, &pActor->currentLocation);
			fVar8 = edF32Vector4GetDistHard(&eStack48);
			fVar9 = pActor->FUN_002d5710();
			pCVar7 = pOtherActor->GetCollidingActor();
			do {
				if (pCVar7 == pOtherActor) {
					pOtherActor = (CActor*)0x0;
				}
				else {
					bVar4 = pOtherActor->IsKindOfObject(2);
					if (bVar4 != false) {
						CActorMovable* pMovable = static_cast<CActorMovable*>(pOtherActor);
						edF32Vector4ScaleHard((fVar8 / fVar9) * pMovable->dynamic.linearAcceleration, &eStack64, &pMovable->dynamic.velocityDirectionEuler);
						edF32Vector4AddHard(&local_20, &local_20, &eStack64);
					}

					pOtherActor = pOtherActor->pTiedActor;
				}
			} while (pOtherActor != (CActor*)0x0);
		}

		local_10 = local_20;
		this->field_0x80 = local_10;
	}

	GetNotificationTargetArray()->FUN_003c3a30();
	GetNotificationTargetArray()->combatMode = (this->pOwner)->combatMode_0xb7c;

	pCVar1 = this->pOwner;
	if ((int)pCVar1->combatMode_0xb7c < 2) {
		pCVar1->combatFlags_0xb78 = pCVar1->combatFlags_0xb78 & 0xffefffff;
	}
	else {
		pCVar1->combatFlags_0xb78 = pCVar1->combatFlags_0xb78 | 0x100000;
	}

	return;
}

void CBehaviourTrackWeaponStand::End(int newBehaviourId)
{
	this->pOwner->flags = this->pOwner->flags & 0xfffffbff;

	CBehaviourWolfen::End(newBehaviourId);

	if ((this->pOwner->combatFlags_0xb78 & 0x140) != 0) {
		this->pOwner->combatFlags_0xb78 = this->pOwner->combatFlags_0xb78 & 0xfffffebf;
	}

	return;
}

void CBehaviourTrackWeaponStand::InitState(int newState)
{
	CActorWeapon* pWeapon;
	StateConfig* pConfig;
	int iVar1;
	float fVar2;
	float fVar3;
	CAnimation* pAnim;
	CActorWolfen* pWolfen;

	if (newState == 0x97) {
		IMPLEMENTATION_GUARD(
		pWeapon = CActorFighter::GetWeapon((CActorFighter*)this->pOwner);
		FUN_002d5860((int)pWeapon, 1);)
	}
	else {
		if (newState == 0x96) {
			IMPLEMENTATION_GUARD(
			pWeapon = CActorFighter::GetWeapon((CActorFighter*)this->pOwner);
			FUN_002d5860((int)pWeapon, 0);)
		}
		else {
			if (newState == WOLFEN_STATE_AIM) {
				this->pOwner->SV_AUT_WarnActors(this->pOwner->field_0xcf4, 0.0f, (CActor*)0x0);
			}
			else {
				if (newState == WOLFEN_STATE_RELOAD) {
					if (this->field_0x98 == 1) {
						fVar2 = this->pOwner->GetWeapon()->GetLaunchSpeed(3);
						pWolfen = this->pOwner;
						pAnim = pWolfen->pAnimationController;
						pConfig = pWolfen->GetStateCfg(WOLFEN_STATE_RELOAD);
						iVar1 = pWolfen->GetIdMacroAnim(pConfig->animId);
						if (iVar1 < 0) {
							fVar3 = 0.0f;
						}
						else {
							fVar3 = pAnim->GetAnimLength(iVar1, 0);
						}

						this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar3 / fVar2, 0);
					}
				}
				else {
					if (newState == WOLFEN_STATE_FIRE) {
						if (this->field_0x94 == 1) {
							fVar2 = this->pOwner->GetWeapon()->GetLaunchSpeed(2);
							pWolfen = this->pOwner;
							pAnim = pWolfen->pAnimationController;
							pConfig = pWolfen->GetStateCfg(WOLFEN_STATE_FIRE);
							iVar1 = pWolfen->GetIdMacroAnim(pConfig->animId);
							if (iVar1 < 0) {
								fVar3 = 0.0f;
							}
							else {
								fVar3 = pAnim->GetAnimLength(iVar1, 0);
							}

							this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar3 / fVar2, 0);
						}
					}
					else {
						if (newState == WOLFEN_STATE_TRACK_DEFEND) {
							this->field_0xec = Timer::GetTimer()->scaledTotalTime;
						}
					}
				}
			}
		}
	}

	CBehaviourWolfen::InitState(newState);

	return;
}

void CBehaviourTrackWeaponStand::TermState(int oldState, int newState)
{
	if ((oldState == WOLFEN_STATE_RELOAD) || (oldState == WOLFEN_STATE_FIRE)) {
		this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
	}

	CBehaviourWolfen::TermState(oldState, newState);
	return;
}

CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* CBehaviourTrackWeaponStand::GetNotificationTargetArray()
{
	return &this->notificationTargetArray;
}

int CBehaviourTrackWeaponStand::GetStateWolfenWeapon()
{
	if (this->pOwner->GetWeapon()->FUN_002d58a0() != 0) {
		if (this->pOwner->GetWeapon()->FUN_002d5830() == 0) {
			(this->pOwner)->field_0xcf8 = WOLFEN_STATE_TRACK_DEFEND;
			return 0x97;
		}
	}

	return WOLFEN_STATE_TRACK_DEFEND;
}

int CBehaviourTrackWeaponStand::Func_0x70()
{
	return -1;
}

int CBehaviourTrackWeaponStand::Func_0x74()
{
	if (this->pOwner->GetWeapon()->FUN_002d58a0() != 0) {
		if (this->pOwner->GetWeapon()->FUN_002d5830() == 0) {
			(this->pOwner)->field_0xcf8 = WOLFEN_STATE_AIM;
			return 0x96;
		}
	}

	return WOLFEN_STATE_AIM;
}

int CBehaviourTrackWeaponStand::Func_0x78()
{
	return notificationTargetArray.GetState_003c37c0(this->pOwner);
}

void CBehaviourTrackWeaponStand::Func_0x80(int* param_2, int* param_3, CActor* pTarget)
{
	float fVar1;

	fVar1 = pTarget->SV_GetDirectionalAlignmentToTarget(&this->pOwner->currentLocation);

	if (0.001f < fVar1) {
		*param_2 = *param_2 + 1;
	}
	else {
		if (fVar1 < 0.001f) {
			*param_3 = *param_3 + 1;
		}
	}

	return;
}

int CBehaviourTrackWeaponStand::FUN_002faf40()
{
	int iVar1;
	TrackSubObj* pTVar2;
	int iVar3;
	int iVar4;
	int local_8;
	int local_4;

	local_4 = 0;
	local_8 = 0;
	if (this->pOwner->pTargetActor_0xc80 != (CActorFighter*)0x0) {
		Func_0x80(&local_4, &local_8, this->pOwner->pTargetActor_0xc80);
	}
	iVar1 = -1;
	iVar4 = 0;
	iVar3 = 0;
	if (0 < this->nbSubObjs) {
		pTVar2 = this->aSubObjs;
		do {
			if ((pTVar2->field_0x0 == 0) && (iVar4 < local_4)) {
				iVar1 = iVar3;
				iVar4 = local_4;
			}
			if ((pTVar2->field_0x0 == 1) && (iVar4 < local_8)) {
				iVar1 = iVar3;
				iVar4 = local_8;
			}
			iVar3 = iVar3 + 1;
			pTVar2 = pTVar2 + 1;
		} while (iVar3 < this->nbSubObjs);
	}
	return iVar1;
}

astruct_16::astruct_16()
{
	this->field_0x0 = 0;
	this->field_0x4 = 1.0;
	this->field_0x8 = 5.0;

	return;
}

void astruct_16::Create(ByteCode* pByteCode)
{
	this->field_0x0 = pByteCode->GetU32();
	this->field_0x4 = pByteCode->GetF32();
	this->field_0x8 = pByteCode->GetF32();

	return;
}

CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* CBehaviourTrackStand::GetNotificationTargetArray()
{
	return &this->notificationTargetArray;
}

void CBehaviourLost::Create(ByteCode* pByteCode)
{
	CBehaviourWolfen::Create(pByteCode);

	this->switchBehaviour.Create(pByteCode);

	return;
}

void CBehaviourLost::Init(CActor* pOwner)
{
	CBehaviourWolfen::Init(pOwner);

	this->switchBehaviour.Init(pOwner);

	return;
}

void CBehaviourLost::Manage()
{
	this->pOwner->BehaviourStand_Manage(this);
}

void CBehaviourLost::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (newState == -1) {
		this->pOwner->SetState(0x72, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->switchBehaviour.Begin(pOwner);

	return;
}

int CBehaviourLost::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1 = this->switchBehaviour.InterpretMessage(this->pOwner, pSender, msg, pMsgParam);

	if (iVar1 == 0) {
		iVar1 = CBehaviourWolfen::InterpretMessage(pSender, msg, pMsgParam);
	}

	return iVar1;
}

void CBehaviourFighterWolfen::Create(ByteCode* pByteCode)
{
	return;
}

float _Rule_Life(CActorWolfen* pWolfen)
{
	return pWolfen->GetLifeInterface()->GetValue() / pWolfen->GetLifeInterface()->GetValueMax();
}

float _Rule_Unknown(CActorWolfen* pWolfen)
{
	return 0.0f;
}

float _Rule_Agressivity(CActorWolfen* pWolfen)
{
	CActorFighter* pCVar1;
	float fVar2;

	pCVar1 = pWolfen->pAdversary;
	if ((pCVar1 == (CActorFighter*)0x0) || (pCVar1->pAdversary != pWolfen)) {
		fVar2 = 0.0f;
	}
	else {
		fVar2 = 1.0f;
	}

	return fVar2;
}

void CBehaviourFighterWolfen::Init(CActor* pOwner)
{
	CActorWolfen* pWolfen;
	int iVar3;
	uint uVar5;
	float fVar6;
	float fVar7;

	CBehaviourFighter::Init(pOwner);

	if ((gWolfenAnimMatrixData.aMatrices == (edF32MATRIX3*)0x0) && (uVar5 = this->pOwner->pAnimationController->anmSkeleton.pTag->boneCount,
		gWolfenAnimMatrixData.nbBones < uVar5)) {
		gWolfenAnimMatrixData.nbBones = uVar5;
	}

	this->aRules[0].pFunc = _Rule_Life;
	this->aRules[0].value = 1.0f;
	this->aRules[1].pFunc = _Rule_Unknown;
	this->aRules[1].value = 1.0f;
	this->aRules[2].pFunc = _Rule_Agressivity;
	this->aRules[2].value = 1.0f;

	pWolfen = static_cast<CActorWolfen*>(this->pOwner);
	fVar6 = pWolfen->field_0xb48;
	fVar7 = pWolfen->field_0xb4c;

	if (pWolfen->field_0xb44 == 1) {
		this->field_0x38 = GetTimer()->scaledTotalTime;
		this->field_0x34 = fVar6 + ((fVar7 - fVar6) * (float)CScene::Rand()) / 32767.0f;
	}
	else {
		if (pWolfen->field_0xb44 == 0) {
			this->field_0x38 = 0.0f;
			iVar3 = (((int)fVar7 - (int)fVar6) + 1) * CScene::Rand();
			if (iVar3 < 0) {
				iVar3 = iVar3 + 0x7fff;
			}
			this->field_0x34 = (float)((int)fVar6 + (iVar3 >> 0xf));
		}
	}

	return;
}

CActorWolfen* gWolfenPtr;

void CBehaviourFighterWolfen::Manage()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	StateConfig* pSVar4;
	CActorFighter* pCVar5;
	CLifeInterface* pCVar6;
	int iVar7;
	Timer* pTVar8;
	CPlayerInput* pPlayerInput;
	uint uVar9;
	undefined4 newPriority;
	float fVar10;
	float fVar11;
	float fVar12;
	CActorWolfen* pWolfen;

	pWolfen = static_cast<CActorWolfen*>(this->pOwner);

	if (pWolfen == gWolfenPtr) {
		if (pWolfen == gWolfenPtr) {
			pPlayerInput = GetPlayerInput(1);
			pWolfen->UpdateFightCommandInternal(pPlayerInput, 1);
		}

		CBehaviourFighter::Manage();
		goto LAB_001f96e0;
	}

	uVar9 = pWolfen->GetStateFlags(pWolfen->actorState);
	uVar9 = uVar9 & 0xff800;
	if (uVar9 == 0x8000) {
		pWolfen = static_cast<CActorWolfen*>(this->pOwner);
		CBehaviourFighter::Manage();
		pCVar6 = pWolfen->GetLifeInterfaceOther();
		pCVar6->SetPriority(4);
		if (pWolfen->actorState == 0x38) {
			this->adversaryBlowDuration = 0.0f;
		}
		else {
			pTVar8 = GetTimer();
			this->adversaryBlowDuration = this->adversaryBlowDuration + pTVar8->cutsceneDeltaTime;
		}
	}
	else {
		if ((((uVar9 == 0x80000) || (uVar9 == 0x4000)) || (uVar9 == 0x2000)) || ((uVar9 == 0x1000 || (uVar9 == 0x800)))) {
			pWolfen = static_cast<CActorWolfen*>(this->pOwner);
			if (this->currentCommandId == -1) {
				PickCommand();
			}

			UpdateFightContext(&this->fightContext);
			ManageWFigState(this->currentCommandId);

			pCVar5 = (CActorFighter*)pWolfen->pCommander->GetIntruder();
			if ((pWolfen->pTargetActor_0xc80 == pCVar5) && ((pWolfen->combatFlags_0xb78 & 0x10) != 0)) {
				pCVar6 = pCVar5->GetLifeInterface();
				fVar10 = pCVar6->GetValue();
				if (fVar10 <= 0.0) goto LAB_001f9458;
				pWolfen->SV_WLF_IsIntruderMakingNoise(pCVar5);
				pWolfen->SV_WLF_IsIntruderInVitalSphere(pCVar5);
				pWolfen->SV_WLF_IsIntruderInVision(pCVar5);
			}
			else {
			LAB_001f9458:
				pCVar6 = pCVar5->GetLifeInterface();
				fVar10 = pCVar6->GetValue();
				if (fVar10 <= 0.0f) {
					pWolfen->SetCombatMode(ECM_None);
				}

				pWolfen->combatFlags_0xb78 = pWolfen->combatFlags_0xb78 & 0xfffffff8;
			}

			pWolfen->UpdateInRange_001744a0((pWolfen->combatFlags_0xb78 & 7) != 0);

			iVar7 = pWolfen->actorState;
			if (iVar7 == 0x74) {
				IMPLEMENTATION_GUARD(
				CActorWolfen::FUN_0035eac0(pWolfen);)
			}
			else {
				if (iVar7 == 0xad) {
					if (pWolfen->field_0xd00 < pWolfen->timeInAir) {
						iVar7 = rand();
						pWolfen->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(((float)iVar7 / 2.147484e+09f) * 0.8000001f + 1.6f, 0);
						pWolfen->SetLookingAtOff();
						pWolfen->PlayAnim(0xe5);
					}

					if (pWolfen->currentAnimType == 0xe5) {
						pCVar1 = pWolfen->pAnimationController;
						peVar2 = (pCVar1->anmBinMetaAnimator).aAnimData;

						bVar3 = false;
						if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
							if (peVar2->animPlayState == 0) {
								bVar3 = false;
							}
							else {
								bVar3 = (peVar2->field_0xcc & 2) != 0;
							}
						}

						if (bVar3) {
							pWolfen->SetState(6, -1);
						}
					}
				}
				else {
					CBehaviourFighter::Manage();
				}
			}

			pCVar5 = pWolfen->pAdversary;
			if (pCVar5 != (CActorFighter*)0x0) {
				newPriority = 4;
				if ((CActorWolfen*)pCVar5->pAdversary == pWolfen) {
					newPriority = 5;
				}

				pCVar6 = pWolfen->GetLifeInterfaceOther();
				pCVar6->SetPriority(newPriority);
			}
		}
	}

	ManageExit();

LAB_001f96e0:
	pWolfen = static_cast<CActorWolfen*>(this->pOwner);
	this->field_0x58 = pWolfen->fighterAnatomyZones.field_0x0.y + pWolfen->currentLocation.y;

	iVar7 = -1;
	if (((pWolfen->pCollisionData)->flags_0x4 & 2) != 0) {
		this->field_0x5c = this->field_0x58;
	}

	if (((pWolfen->combatFlags_0xb78 & 0x20000) != 0) &&
		(pWolfen->field_0x7cc < this->field_0x5c - this->field_0x58)) {
		iVar7 = 4;
		pWolfen->field_0x6a0 = pWolfen->dynamic.velocityDirectionEuler;

		pWolfen->field_0x6b0 = pWolfen->dynamic.linearAcceleration;
		pWolfen->field_0x684 = 1;
		pWolfen->field_0x7a0 = gF32Vector4UnitY;
		pWolfen->field_0x7b4 = 0.0f;
	}

	if (iVar7 != -1) {
		this->pOwner->SetBehaviour(iVar7, -1, -1);
		this->pOwner->field_0x7c8 = this->field_0x5c;
		this->pOwner->field_0x7d0 = this->field_0x58;
	}

	return;
}

void CBehaviourFighterWolfen::Draw()
{
	return;
}

void CBehaviourFighterWolfen::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorWolfen* pCVar1;
	CActorFighter* pCVar2;
	float fVar3;
	float fVar4;
	bool bVar6;
	int iVar8;
	uint uVar9;
	float fVar10;

	fVar10 = this->pOwner->GetLifeInterfaceOther()->GetValue();
	if (0.0f < fVar10) {
		CScene::ptable.g_FrontendManager_00451680->DeclareInterface(FRONTEND_INTERFACE_ENEMY_LIST, this->pOwner->GetLifeInterfaceOther());
	}

	CBehaviourFighter::Begin(pOwner, newState, newAnimationType);

	CActorWolfen* pWolfen = static_cast<CActorWolfen*>(this->pOwner);

	pWolfen->CheckValidPatterns(&pWolfen->aCapabilities[0].rndChooser);
	pWolfen->CheckValidPatterns(&pWolfen->aCapabilities[1].rndChooser);
	pWolfen->CheckValidPatterns(&pWolfen->aCapabilities[2].rndChooser);

	this->field_0x64 = 0.0f;

	pWolfen->aCapabilities[0].Begin();
	pWolfen->aCapabilities[1].Begin();
	pWolfen->aCapabilities[2].Begin();
	
	this->currentCommandId = -1;

	ValidateCommand();

	this->currentCommandId = -1;
	this->field_0x70 = 1;
	this->field_0x71 = 0;

	this->field_0x74.all = 0;
	this->pActiveBlow = (s_fighter_blow*)0x0;

	this->field_0x80 = gF32Vector4Zero;

	this->field_0x90.field_0x4 = 0;
	this->field_0x90.field_0x0 = (edF32VECTOR4*)0x0;

	FlushInput();

	memset(&this->fightContext, 0, sizeof(CFightContext));
	(this->fightContext).state_0x14 = -1;

	UpdateFightContext(&this->fightContext);
	TreatContext(&this->fightContext);

	this->field_0x68 = 0;
	this->field_0x3c = 0;

	return;
}

void CBehaviourFighterWolfen::End(int newBehaviourId)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighterWolfen::InitState(int newState)
{
	CActorWolfen* pWolfen;
	uint stateFlags;

	stateFlags = this->pOwner->GetStateFlags(newState);

	if ((stateFlags & 0xff800) == 0x8000) {
		pWolfen = static_cast<CActorWolfen*>(this->pOwner);
		stateFlags = pWolfen->GetStateFlags(pWolfen->prevActorState);

		if ((stateFlags & 0xff800) != 0x8000) {
			this->adversaryBlowDuration = 0.0f;
		}
	}

	if (newState == 0xad) {
		pWolfen = static_cast<CActorWolfen*>(this->pOwner);
		pWolfen->PlayAnim(pWolfen->standAnim);
		pWolfen->field_0xd00 = (((float)rand() / 2.147484e+09f) * 0.6f + 0.0f);
		pWolfen->fightFlags = pWolfen->fightFlags | 1;
	}
	else {
		if (newState == 6) {
			pWolfen = static_cast<CActorWolfen*>(this->pOwner);
			this->pOwner->SV_AUT_WarnActors(pWolfen->field_0xcf4, 0.0f, (CActor*)0x0);
			CBehaviourFighter::InitState(6);
		}
		else {
			if (newState == 0x74) {
				pWolfen = static_cast<CActorWolfen*>(this->pOwner);
				pWolfen->field_0xd18 = pWolfen->_GetFighterAnimationLength(pWolfen->GetStateCfg(0x74)->animId);
			}
			else {
				CBehaviourFighter::InitState(newState);
			}
		}
	}

	return;
}

void CBehaviourFighterWolfen::TermState(int oldState, int newState)
{
	CActorWolfen* pCVar1;

	if (oldState == 0xad) {
		pCVar1 = static_cast<CActorWolfen*>(this->pOwner);
		pCVar1->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		pCVar1->fightFlags = pCVar1->fightFlags & 0xfffffffe;
		pCVar1->SetLookingAtOn();
	}
	else {
		CBehaviourFighter::TermState(oldState, newState);
	}

	return;
}

int CBehaviourFighterWolfen::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorWolfen* pCVar1;
	CActorFighter* pCVar2;
	bool bVar3;
	WFIGS_Capability* pWVar4;
	uint uVar5;
	int iVar6;
	long lVar7;
	CActorCommander* pCommander;

	if (msg == 0x66) {
		IMPLEMENTATION_GUARD(
		uVar5 = this->pOwner->field_0xb74;
		if ((uVar5 == 0) || (uVar5 == 1)) {
			iVar6 = CBehaviourFighter::InterpretMessage
			((CBehaviourFighter*)this, pSender, 0x66, (GetPositionMsgParams*)pMsgParam);
		}
		else {
			iVar6 = 0;
		})
	}
	else {
		if (msg == 0x27) {
			IMPLEMENTATION_GUARD(
			uVar5 = this->pOwner->field_0xb74;
			if ((uVar5 == 0) || (uVar5 == 1)) {
				iVar6 = 1;
			}
			else {
				lVar7 = (*(code*)(this->pOwner->pVTable)->Func_0x1ac)();
				if (lVar7 != 0) {
					ValidateCommand();
					InitCommand(this, this->pOwner->field_0xb64[6].field_0x0, 6);
					this->field_0x68 = 1;
					this->currentCommandId = 6;
				}
				iVar6 = 0;
			})
		}
		else {
			if (msg == 2) {
				IMPLEMENTATION_GUARD(
				if ((CActorFighter*)pSender == this->pOwner->pAdversary) {
					(this->fightContext).field_0x0 = (this->fightContext).field_0x0 & 0xfd | 2;
					(this->fightContext).field_0x0 = (this->fightContext).field_0x0 & 0xfe;
					bVar3 = CActorFighter::FUN_0031b5d0
					((CActorFighter*)this->pOwner, this->pOwner->actorState);
					if (bVar3 == false) {
						if (((uint)pMsgParam->z & 8) == 0) {
							*(int*)&this->field_0x30 = *(int*)&this->field_0x30 + 1;
						}
						else {
							pCVar1 = this->pOwner;
							if (pCVar1->activeCapabilityIndex != 3) {
								pCommander = pCVar1->pCommander;
								lVar7 = CActorCommander::FUN_00171440(pCommander, (int)pCVar1);
								if (lVar7 != 0) {
									pWVar4 = (WFIGS_Capability*)0x0;
									if (pCVar1->activeCapabilityIndex != 3) {
										pWVar4 = pCVar1->aCapabilities + pCVar1->activeCapabilityIndex;
									}
									CActorCommander::ReleaseSemaphore(pCommander, pWVar4->semaphoreId, (int)pCVar1);
								}
							}
						}

						ValidateCommand();
						this->currentCommandId = -1;
					}

					if (this->pOwner->field_0xb44 == 0) {
						this->field_0x38 = 0;
					}
				})

				iVar6 = CBehaviourFighter::InterpretMessage(pSender, 2, (GetPositionMsgParams*)pMsgParam);
			}
			else {
				iVar6 = CBehaviourFighter::InterpretMessage(pSender, msg, pMsgParam);
			}
		}
	}
	return iVar6;
}

void CBehaviourFighterWolfen::ManageExit()
{
	StateConfig* pSVar1;
	int iVar2;
	uint uVar3;
	CActorWolfen* pWolfen;

	pWolfen = static_cast<CActorWolfen*>(this->pOwner);
	uVar3 = pWolfen->GetStateFlags(pWolfen->actorState);

	uVar3 = uVar3 & 0xff800;
	if (uVar3 == 0x8000) {
		iVar2 = pWolfen->pCommander->squad.NbElt();
		if ((iVar2 < 2) || (6.0f < this->adversaryBlowDuration)) {
			(this->field_0x74).field_0x1 = (this->field_0x74).field_0x1 & 0xf0 | 1;
			this->pActiveCombo = (s_fighter_combo*)0x0;
			this->pActiveBlow = (s_fighter_blow*)0x0;
			this->field_0x70 = 1;
			FlushInput();
		}
	}
	else {
		if ((((uVar3 == 0x80000) || (uVar3 == 0x4000)) || (uVar3 == 0x2000)) || ((uVar3 == 0x1000 || (uVar3 == 0x800)))) {
			pWolfen->BehaviourFighterStd_Exit(this);
		}
	}

	return;
}

void CBehaviourFighterWolfen::ManageCombatMusic(int state)
{
	IMPLEMENTATION_GUARD_AUDIO(
	if (state == 0) {
		CAudioManager::StopCombatMusic(CScene::ptable.g_AudioManager_00451698);
	}
	else {
		CAudioManager::PlayCombatMusic(CScene::ptable.g_AudioManager_00451698);
	})
	return;
}

void CBehaviourFighterWolfen::Func_0x64()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourFighterWolfen::SetPositionToHold(float param_1, edF32VECTOR4* pPosition)
{
	this->holdPosition = *pPosition;

	this->field_0x50 = param_1;
	this->field_0x3c = 1;

	return;
}

void CBehaviourFighterWolfen::InputPunch(uint cmd)
{
	byte bVar1;
	s_fighter_combo* pCombo;

	pCombo = PickCombo_Attack(&this->fightContext, (cmd & 0x40) != 0, (cmd & 0x20) != 0, (cmd & 0x80) != 0, (cmd & 0x100) != 0);
	if (pCombo == (s_fighter_combo*)0x0) {
		this->pActiveBlow = (s_fighter_blow*)0x0;
		this->field_0x70 = 0;
	}
	else {
		this->field_0x70 = 1;
		this->pActiveCombo = pCombo;

		if ((pCombo->field_0x4.field_0x0ushort & 0x400U) == 0) {
			bVar1 = (this->field_0x74).field_0x1;
			(this->field_0x74).field_0x1 = bVar1 & 0xf0 | bVar1 & 0xf | 1;
			this->pActiveBlow = LOAD_SECTION_CAST(s_fighter_blow*, pCombo->actionHash.pData);
		}
		else {
			bVar1 = (this->field_0x74).field_0x1;
			(this->field_0x74).field_0x1 = bVar1 & 0xf0 | bVar1 & 0xf | 4;
			this->pActiveBlow = LOAD_SECTION_CAST(s_fighter_blow*, pCombo->actionHash.pData);
		}
	}

	return;
}

void CBehaviourFighterWolfen::FlushInput()
{
	float fVar1;
	float fVar2;
	float fVar3;
	byte bVar4;

	bVar4 = 0;
	if (this->field_0x70 == 1) {
		if (((this->field_0x74).field_0x1 & 0xf) == 0) {
			if (((this->field_0x74).field_0x0 & 0xf) == 0xc) {
				this->field_0x90.field_0x0 = &this->field_0x80;
				this->field_0x90.field_0x4 = 0;
				bVar4 = Conditional_Execute(&this->field_0x74, &this->field_0x90);
			}
			else {
				bVar4 = Conditional_Execute(&this->field_0x74, (s_fighter_action_param*)0x0);
			}
		}
		else {
			this->field_0x90.field_0x0 = (edF32VECTOR4*)0x0;
			this->field_0x90.pData = this->pActiveBlow;
			bVar4 = CBehaviourFighter::Conditional_Execute(&this->field_0x74, &this->field_0x90);
		}
	}

	this->field_0x70 = 1;
	this->field_0x71 = 0;

	this->field_0x74.all = 0x0;
	this->pActiveBlow = (s_fighter_blow*)0x0;
	this->field_0x80 = gF32Vector4Zero;
	(this->field_0x90).field_0x4 = 0;
	(this->field_0x90).field_0x0 = (edF32VECTOR4*)0x0;
	this->field_0x70 = 0;
	this->field_0x71 = bVar4;

	return;
}

void CBehaviourFighterWolfen::UpdateFightContext(CFightContext* pFightContext)
{
	byte bVar1;
	byte bVar2;
	byte bVar3;
	int iVar4;
	CActorWolfen* pWolfen;
	int iVar6;
	int iVar7;
	bool bVar8;
	edF32VECTOR4* v2;
	long lVar10;
	ulong uVar11;
	float fVar12;
	float fVar13;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;
	CActorFighter* pFighter;

	pFighter = this->pOwner->pAdversary;
	if (pFighter != (CActorFighter*)0x0) {
		iVar4 = pFighter->actorState;
		pFightContext->state_0x14 = pFightContext->state_0x10;
		pFightContext->state_0x10 = iVar4;

		pWolfen = static_cast<CActorWolfen*>(this->pOwner);
		iVar4 = pFightContext->field_0x8;

		v2 = pWolfen->GetAdversaryPos();
		edF32Vector4SubHard(&eStack16, &pWolfen->currentLocation, v2);
		eStack16.y = 0.0f;
		fVar12 = edF32Vector4SafeNormalize1Hard(&eStack16, &eStack16);
		if (0.001f < fabs(pFightContext->field_0xc - fVar12)) {
			if (pFightContext->field_0xc < fVar12) {
				pFightContext->field_0x8 = 2;
			}
			else {
				pFightContext->field_0x8 = 0;
			}
		}
		else {
			pFightContext->field_0x8 = 1;
		}

		pFightContext->field_0xc = fVar12;
		iVar6 = pFightContext->field_0x4;
		if (pFightContext->field_0xc < pWolfen->field_0xbec) {
			pFightContext->field_0x4 = 0;
		}
		else {
			if (fabs(pFightContext->field_0xc - pWolfen->field_0xbf0) < 0.1f) {
				pFightContext->field_0x4 = 2;
			}
			else {
				pFightContext->field_0x4 = 1;
			}
		}

		pWolfen = static_cast<CActorWolfen*>(this->pOwner);
		if ((pWolfen->flags & 0x1000) == 0) {
			local_20.x = pWolfen->rotationQuat.x;
			local_20.z = pWolfen->rotationQuat.z;
			local_20.w = pWolfen->rotationQuat.w;
			local_20.y = 0.0f;
			edF32Vector4NormalizeHard(&local_20, &local_20);
		}
		else {
			SetVectorFromAngleY(pWolfen->rotationEuler.y, &local_20);
		}
		if ((pFighter->flags & 0x1000) == 0) {
			local_30.x = pFighter->rotationQuat.x;
			local_30.z = pFighter->rotationQuat.z;
			local_30.w = pFighter->rotationQuat.w;
			local_30.y = 0.0f;
			edF32Vector4NormalizeHard(&local_30, &local_30);
		}
		else {
			SetVectorFromAngleY(pFighter->rotationEuler.y, &local_30);
		}

		bVar1 = pFightContext->field_0x0;
		pFightContext->field_0x0 = bVar1 & 0xf7;

		fVar13 = edF32Vector4DotProductHard(&local_20, &eStack16);
		if (((((fVar13 <= 0.0f) && (fVar13 = edF32Vector4DotProductHard(&local_20, &local_30), 0.707f < fVar13)) &&
			(lVar10 = pFighter->Func_0x190(this->pOwner), lVar10 != 0)) &&
			(((pFighter->field_0x44d & 0xf) != 0 && (pFighter->pFighterCombo != (s_fighter_combo*)0x0)))) &&
			((pFighter->pFighterCombo->field_0x4.field_0x0ushort & 0x400U) == 0)) {
			pFightContext->field_0x0 = pFightContext->field_0x0 & 0xf7 | 8;
		}

		bVar2 = pFightContext->field_0x0;
		pFightContext->field_0x0 = bVar2 & 0xef;

		iVar7 = pWolfen->field_0xb44;
		if (iVar7 == 0) {
			bVar8 = pFighter->FUN_0031b5d0(pFighter->actorState);
			if ((bVar8 != false) && ((uint)this->field_0x34 <= (uint)this->field_0x38)) {
				pFightContext->field_0x0 = pFightContext->field_0x0 & 0xef | 0x10;
			}
		}
		else {
			if (iVar7 == 1) {
				bVar8 = pFighter->FUN_0031b5d0(pFighter->actorState);
				if (bVar8 == false) {
					this->field_0x38 = (int)GetTimer()->scaledTotalTime;
				}
				else {
					if (this->field_0x34 <= GetTimer()->scaledTotalTime - (float)this->field_0x38) {
						pFightContext->field_0x0 = pFightContext->field_0x0 & 0xef | 0x10;
					}
				}
			}
		}

		if (((long)((ulong)pFightContext->field_0x0 << 0x3b) < 0) &&
			((fVar13 = edF32Vector4DotProductHard(&local_20, &local_30), 0.0f < fVar13 || ((this->fightContext).field_0x4 != 0)))) {
			pFightContext->field_0x0 = pFightContext->field_0x0 & 0xef;
		}

		bVar3 = pFightContext->field_0x0;
		bVar8 = pFighter->FUN_0031b790(pFighter->actorState);
		if (((bVar8 != true) || (pFighter->pBlow == (s_fighter_blow*)0x0)) || (pFighter->pBlow->field_0x4c < fVar12 - 0.5))
		{
			pFightContext->field_0x0 = pFightContext->field_0x0 & 0xfd;
		}
		else {
			pFightContext->field_0x0 = pFightContext->field_0x0 & 0xfd | 2;
		}

		uVar11 = (ulong)pFightContext->field_0x0;
		if (((uVar11 & 1) == 1) &&
			((((((ulong)bVar3 << 0x3e) >> 0x3f != (uVar11 << 0x3e) >> 0x3f || (iVar4 != pFightContext->field_0x8)) ||
				((iVar6 != pFightContext->field_0x4 ||
					((((ulong)bVar1 << 0x3c) >> 0x3f != (uVar11 << 0x3c) >> 0x3f ||
						(((ulong)bVar2 << 0x3b) >> 0x3f != (uVar11 << 0x3b) >> 0x3f)))))) ||
				(pFightContext->state_0x14 != pFightContext->state_0x10)))) {
			pFightContext->field_0x0 = pFightContext->field_0x0 & 0xfe;
		}
	}

	return;
}

void CBehaviourFighterWolfen::ManageWFigState(uint commandId)
{
	CActorWolfen* pWolfen;
	int iVar2;
	bool bTreatSuccess;
	bool bVar3;
	uint uVar4;
	int iVar5;
	CActorFighter* pFighter;

	bTreatSuccess = false;
	if (commandId == 0xffffffff) {
		uVar4 = 0;
	}
	else {
		pWolfen = static_cast<CActorWolfen*>(this->pOwner);
		uVar4 = pWolfen->field_0xb64[commandId].field_0x0;
	}

	if ((uVar4 & 0x3f9ef) != 0) {
		bTreatSuccess = TreatContext(&this->fightContext);
	}

	if (bTreatSuccess == false) {
		if (this->field_0x68 == 0) {
			InitCommand(uVar4);
			this->field_0x68 = 1;
		}
		else {
			ExecuteCommand(uVar4, commandId);
			uVar4 = IsCommandFinished(uVar4);
			if (uVar4 != 0) {
				iVar5 = -1;
				pWolfen = static_cast<CActorWolfen*>(this->pOwner);
				if (commandId != 0xffffffff) {
					iVar5 = pWolfen->field_0xb64[commandId].field_0x8;
				}

				ValidateCommand();

				if (iVar5 != -1) {
					if (iVar5 == -1) {
						uVar4 = 0;
					}
					else {
						uVar4 = pWolfen->field_0xb64[iVar5].field_0x0;
					}

					InitCommand(uVar4);
					this->field_0x68 = 1;
				}

				this->currentCommandId = iVar5;
			}
		}
	}

	return;
}


bool CBehaviourFighterWolfen::TreatContext(CFightContext* pFightContext)
{
	CActorWolfen* pCVar1;
	CActorFighter* pCVar2;
	bool bVar3;
	bool uVar4;
	bool bVar4;
	int iVar5;
	byte* pbVar6;
	uint uVar7;
	int local_4;

	uVar4 = false;
	local_4 = 0;
	if ((pFightContext->field_0x0 & 1) == 0) {
		if ((long)((ulong)pFightContext->field_0x0 << 0x3e) < 0) {
			IMPLEMENTATION_GUARD(
			uVar4 = TreatContext_Attacked(pFightContext, &local_4);)
		}

		if ((long)((ulong)pFightContext->field_0x0 << 0x3d) < 0) {
			pCVar1 = static_cast<CActorWolfen*>(this->pOwner);
			uVar4 = false;
			if (pCVar1->pAdversary != (CActorFighter*)0x0) {
				bVar4 = pCVar1->aCapabilities[1].Get();

				if (bVar4) {
					if (pCVar1->activeCapabilityIndex == 0) {
						pCVar1->activeCapabilityIndex = 1;
						ValidateCommand();
						this->currentCommandId = -1;
					}
					else {
						IMPLEMENTATION_GUARD(
						pCVar1->FUN_001fbff0(1, '\0');)
						ValidateCommand();
						this->currentCommandId = -1;
					}

					uVar4 = true;
				}
			}

			pFightContext->field_0x0 = pFightContext->field_0x0 & 0xfb;
		}

		if ((uVar4 == false) && ((long)((ulong)pFightContext->field_0x0 << 0x3c) < 0)) {
			pCVar1 = static_cast<CActorWolfen*>(this->pOwner);
			uVar4 = false;
			if (pCVar1->pAdversary != (CActorFighter*)0x0) {
				bVar4 = pCVar1->aCapabilities[2].Get();

				IMPLEMENTATION_GUARD(
				if ((bVar4) && (iVar5 = pCVar1->FUN_001fc300(2), iVar5 != 0)) {
					ValidateCommand();
					this->currentCommandId = -1;
					uVar4 = true;
				})
			}
		}

		if ((uVar4 == false) && ((long)((ulong)pFightContext->field_0x0 << 0x3b) < 0)) {
			IMPLEMENTATION_GUARD(
			uVar4 = FUN_001f6110(this, pFightContext, &local_4);)
		}

		if (local_4 == 0) {
			pFightContext->field_0x0 = pFightContext->field_0x0 & 0xfe | 1;
		}
	}

	return uVar4;
}

void CBehaviourFighterWolfen::InitCommand(uint commandId)
{
	byte bVar1;
	bool bVar2;
	s_fighter_blow* psVar4;
	CPathFinderClient* pCVar5;
	int iVar6;
	long lVar7;
	ulong uVar8;
	CActorWolfen* pWolfen;
	float fVar10;
	float fVar11;
	float fVar12;
	float time;
	edF32VECTOR4 eStack16;

	time = GetTimer()->scaledTotalTime;

	if (commandId != 0x80000) {
		if (commandId != 0x40000) {
			if (commandId == 0x20000) {
				bVar1 = (this->field_0x74).field_0x1;
				(this->field_0x74).field_0x1 = bVar1 & 0xcf | (byte)(((uint)(((ulong)bVar1 << 0x3a) >> 0x3e) | 1) << 4);
				this->field_0x70 = 1;
				FlushInput();
			}
			else {
				if (commandId != 0x10000) {
					if (commandId == 0x4000) {
						this->adversaryBlowDuration = time + 2.0f;
					}
					else {
						if ((((commandId != 0x8000) && (commandId != 0x2000)) && (commandId != 0x1000)) && (commandId != 0x800)) {
							if (commandId == 0x400) {
								IMPLEMENTATION_GUARD(
								FUN_001fa910();)
								FlushInput();
							}
							else {
								if (commandId == 0x200) {
									bVar1 = (this->field_0x74).field_0x1;
									(this->field_0x74).field_0x1 = bVar1 & 0xf0 | bVar1 & 0xf | 2;
									psVar4 = this->pOwner->FindBlowByName("BASE_CATCH");
									if ((psVar4 == (s_fighter_blow*)0x0) || (psVar4->field_0x4c < (this->fightContext).field_0xc)) {
										this->pActiveCombo = (s_fighter_combo*)0x0;
										this->pActiveBlow = (s_fighter_blow*)0x0;
										this->field_0x70 = 0;
									}
									else {
										this->pActiveCombo = (s_fighter_combo*)0x0;
										this->pActiveBlow = psVar4;
										this->field_0x70 = 1;
									}

									FlushInput();
								}
								else {
									if (commandId == 0x10) {
										bVar1 = (this->field_0x74).field_0x1;
										(this->field_0x74).field_0x1 = bVar1 & 0xf0 | bVar1 & 0xf | 1;
										psVar4 = this->pOwner->FindBlowByName("CROSSBOW_FIRE");
										if (psVar4 == (s_fighter_blow*)0x0) {
											this->pActiveCombo = (s_fighter_combo*)0x0;
											this->pActiveBlow = (s_fighter_blow*)0x0;
											this->field_0x70 = 0;
										}
										else {
											this->pActiveCombo = (s_fighter_combo*)0x0;
											this->pActiveBlow = psVar4;
											this->field_0x70 = 1;
										}

										FlushInput();
									}
									else {
										if (((((commandId == 0x1e0) || (commandId == 0x1c0)) ||
											((commandId == 0x160 || ((commandId == 0xe0 || (commandId == 0x180)))))) ||
											(commandId == 0x120)) ||
											(((((commandId == 0xa0 || (commandId == 0x140)) || (commandId == 0xc0)) ||
												((commandId == 0x60 || (commandId == 0x100)))) ||
												((commandId == 0x80 || ((commandId == 0x40 || (commandId == 0x20)))))))) {
											InputPunch(commandId);
											FlushInput();
										}
										else {
											pWolfen = static_cast<CActorWolfen*>(this->pOwner);
											if (commandId == 8) {
												this->adversaryBlowDuration = time + pWolfen->field_0xb5c;
											}
											else {
												if (commandId == 4) {
													this->adversaryBlowDuration = time + pWolfen->field_0xb58;
												}
												else {
													if (commandId == 2) {
														this->adversaryBlowDuration = time + pWolfen->field_0xb54;
													}
													else {
														if (commandId == 1) {
															this->adversaryBlowDuration = time + pWolfen->field_0xb50;
														}
														else {
															if (commandId == 0) {
																FunReset();
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			goto LAB_001f8e50;
		}

		FunReset();

		(this->field_0x74).field_0x1 = (this->field_0x74).field_0x1 & 0xf0 | 8;
		this->field_0x70 = 1;

		FlushInput();

		FunReset();

		edF32Vector4ScaleHard(this->pOwner->field_0x4c4 * 1.1f, &eStack16, &this->pOwner->rotationQuat);
		edF32Vector4SubHard(&eStack16, &this->pOwner->currentLocation, &eStack16);

		if (this->pOwner->GetPathfinderClient()->id == -1) {
		LAB_001f8c78:
			pWolfen = static_cast<CActorWolfen*>(this->pOwner);
			lVar7 = pWolfen->CheckDetectArea(&eStack16);

			if (lVar7 == 1) goto LAB_001f8c90;

			if ((CScene::Rand() & 0x10000) == 0) {
				(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf0 | 2;
				(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf | 0x20;
				this->field_0x70 = 1;
			}
			else {
				(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf0 | 1;
				(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf | 0x20;
				this->field_0x70 = 1;
			}
		}
		else {
			bVar2 = this->pOwner->GetPathfinderClient()->IsValidPosition(&eStack16);

			if (bVar2 == false) goto LAB_001f8c78;

		LAB_001f8c90:
			(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf0 | 6;
			(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf | 0x20;
			this->field_0x70 = 1;
		}

		FlushInput();
	}

	if ((this->pOwner->pAdversary)->pBlow == (s_fighter_blow*)0x0) {
		this->adversaryBlowDuration = 0.0f;
	}
	else {
		pWolfen = static_cast<CActorWolfen*>(this->pOwner);
		fVar11 = this->pOwner->_GetFighterAnimationLength(0xd);
		fVar10 = pWolfen->field_0x474;
		fVar12 = (pWolfen->pAdversary)->pBlow->field_0x50 - fVar10;
		this->adversaryBlowDuration = time + fVar10;
		if (fVar11 * 0.8f < fVar12) {
			this->adversaryBlowDuration = this->adversaryBlowDuration + (fVar12 - fVar11 * 0.8f);
		}
	}

LAB_001f8e50:
	this->field_0x64 = time;
	return;
}

void CBehaviourFighterWolfen::ExecuteCommand(uint param_2, uint param_3)
{
	byte bVar1;
	int iVar2;
	CActorWolfen* pCVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4* pPosition;
	Timer* pTVar7;
	ulong uVar8;
	CActorMovParamsIn movParamsInA;
	CActorMovParamsOut movParamsOutA;
	float local_50;
	undefined4 local_4c;
	float fStack72;
	float fStack68;
	CActorMovParamsIn movParamsInB;
	CActorMovParamsOut movParamsOutB;

	if (param_2 == 0x1000) {
		(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf0 | 6;
		(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf | 0x10;
		this->field_0x70 = 1;
		FlushInput();
	}
	else {
		if (param_2 == 0x800) {
			(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf0 | 3;
			(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf | 0x10;
			this->field_0x70 = 1;
			FlushInput();
		}
		else {
			if (param_2 == 0x80000) {
				IMPLEMENTATION_GUARD(
				if (((((ulong)this->pOwner->field_0x44c << 0x38) >> 0x3c != 2) &&
					(this->pOwner->prevActorState != 8)) &&
					(pTVar7 = Timer::GetTimer(), this->adversaryBlowDuration <= pTVar7->scaledTotalTime)) {
					this->field_0x70 = 1;
					this->field_0x71 = 0;
					this->field_0x74 = (s_fighter_action)0x0;
					this->pActiveBlow = (s_fighter_blow*)0x0;
					fVar6 = gF32Vector4Zero.w;
					fVar5 = gF32Vector4Zero.z;
					fVar4 = gF32Vector4Zero.y;
					(this->field_0x80).x = gF32Vector4Zero.x;
					(this->field_0x80).y = fVar4;
					(this->field_0x80).z = fVar5;
					(this->field_0x80).w = fVar6;
					(this->field_0x90).field_0x4 = 0;
					(this->field_0x90).field_0x0 = (edF32VECTOR4*)0x0;
					if (param_3 == 5) {
						(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf0 | 1;
						(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf | 0x20;
						this->field_0x70 = 1;
					}
					else {
						if (param_3 == 4) {
							(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf0 | 2;
							(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf | 0x20;
							this->field_0x70 = 1;
						}
						else {
							uVar8 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
							CScene::_pinstance->field_0x38 = uVar8;
							if ((uVar8 & 0x10000) == 0) {
								(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf0 | 2;
								(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf | 0x20;
								this->field_0x70 = 1;
							}
							else {
								(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf0 | 1;
								(this->field_0x74).field_0x0 = (this->field_0x74).field_0x0 & 0xf | 0x20;
								this->field_0x70 = 1;
							}
						}
					}
					FlushInput();
				})
			}
			else {
				if (param_2 != 0x40000) {
					if (param_2 == 0x20000) {
						bVar1 = (this->field_0x74).field_0x1;
						(this->field_0x74).field_0x1 = bVar1 & 0xcf | (byte)(((uint)(((ulong)bVar1 << 0x3a) >> 0x3e) | 1) << 4);
						this->field_0x70 = 1;

						FlushInput();
					}
					else {
						if (param_2 == 0x10000) {
							IMPLEMENTATION_GUARD(
							pCVar3 = this->pOwner;
							local_50 = (this->holdPosition).x - (pCVar3->base).characterBase.base.base.currentLocation.x;
							fStack72 = (this->holdPosition).z - (pCVar3->base).characterBase.base.base.currentLocation.z;
							fStack68 = (this->holdPosition).w - (pCVar3->base).characterBase.base.base.currentLocation.w;
							local_4c = 0;
							if (((this->field_0x50 < SQRT(local_50 * local_50 + 0.0 + fStack72 * fStack72)) && (this->field_0x3c != 0)
								) && ((this->pOwner->fightFlags & 1) == 0)) {
								movParamsOutA.flags = 0;
								movParamsInA.pRotation = (edF32VECTOR4*)0x0;
								movParamsInA.speed = 0.0;
								movParamsInA.flags = 0x110;
								movParamsInA.rotSpeed =
									(float)(*(code*)(this->pOwner->pVTable)->GetRunRotSpeed)();
								movParamsInA.flags = movParamsInA.flags | 2;
								movParamsInA.acceleration =
									(float)(*(code*)(this->pOwner->pVTable)->GetRunAcceleration)();
								movParamsInA.speed =
									(float)(*(code*)(this->pOwner->pVTable)->GetRunSpeed)();
								movParamsInA.speed = movParamsInA.speed * 2.0;
								movParamsInA.flags = movParamsInA.flags | 0x400;
								CActorWolfen::SV_WLF_MoveTo(this->pOwner, &movParamsOutA, &movParamsInA, &this->holdPosition);
							})
						}
						else {
							if ((param_2 == 0x4000) || (param_2 == 0x8000)) {
								movParamsOutB.flags = 0;
								movParamsInB.pRotation = (edF32VECTOR4*)0x0;
								movParamsInB.speed = 0.0;
								movParamsInB.flags = 0x110;
								movParamsInB.rotSpeed = this->pOwner->GetRunRotSpeed();
								movParamsInB.flags = movParamsInB.flags | 2;
								movParamsInB.acceleration = this->pOwner->GetRunAcceleration();
								movParamsInB.speed = this->pOwner->GetRunSpeed();
								movParamsInB.flags = movParamsInB.flags | 0x400;
								pCVar3 = static_cast<CActorWolfen*>(this->pOwner);
								pPosition = pCVar3->GetAdversaryPos();
								pCVar3->SV_WLF_MoveTo(&movParamsOutB, &movParamsInB, pPosition);
							}
							else {
								if (param_2 == 0x2000) {
									IMPLEMENTATION_GUARD(
									uVar8 = FUN_001faae0((int)this);
									if ((uVar8 & 0xff) == 0) {
										(this->fightContext).field_0x4 = 2;
									})
								}
								else {
									if (((param_2 != 0x400) && (param_2 != 0x200)) &&
										(((((param_2 == 0x1e0 ||
											((((param_2 == 0x1c0 || (param_2 == 0x160)) || (param_2 == 0xe0)) ||
												((param_2 == 0x180 || (param_2 == 0x120)))))) || (param_2 == 0xa0)) ||
											(((param_2 == 0x140 || (param_2 == 0xc0)) ||
												((param_2 == 0x60 || (((param_2 == 0x100 || (param_2 == 0x80)) || (param_2 == 0x40)))))))) ||
											(param_2 == 0x20)))) {
										iVar2 = this->pOwner->actorState;
										if ((iVar2 == 0x65) || (iVar2 == 0x6d)) {
											this->field_0xb8 = 1;
										}

										iVar2 = this->pOwner->actorState;
										if (((iVar2 == 0x66) || (iVar2 == 0x6e)) && (this->field_0xb8 != 0)) {
											s_fighter_combo* pCombo = this->pActiveCombo;
											if ((iVar2 == 0) || (pCombo->nbBranches == 0)) {
												this->pActiveCombo = (s_fighter_combo*)0x0;
												this->pActiveBlow = (s_fighter_blow*)0x0;
												this->field_0x70 = 0;
											}
											else {
												this->pActiveCombo = LOAD_SECTION_CAST(s_fighter_combo*, pCombo->aBranches[0].pData);
												this->field_0x70 = 1;

												if ((this->pActiveCombo->field_0x4.field_0x0ushort & 0x400) == 0) {
													bVar1 = (this->field_0x74).field_0x1;
													(this->field_0x74).field_0x1 = bVar1 & 0xf0 | bVar1 & 0xf | 1;
													this->pActiveBlow = LOAD_SECTION_CAST(s_fighter_blow*, this->pActiveCombo->actionHash.pData);
												}
												else {
													bVar1 = (this->field_0x74).field_0x1;
													(this->field_0x74).field_0x1 = bVar1 & 0xf0 | bVar1 & 0xf | 4;
													this->pActiveBlow = LOAD_SECTION_CAST(s_fighter_blow*, this->pActiveCombo->actionHash.pData);
												}
											}

											FlushInput();
											this->field_0xb8 = 0;
										}
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

bool CBehaviourFighterWolfen::IsCommandFinished(uint param_2)
{
	CActorWolfen* pWolfen;
	int iVar2;
	CActorFighter* pCVar3;
	float fVar4;
	float fVar5;
	bool bVar6;
	bool bVar7;
	bool bVar8;
	StateConfig* pSVar9;
	uint uVar10;
	Timer* pTVar11;
	float fVar12;

	pWolfen = static_cast<CActorWolfen*>(this->pOwner);
	iVar2 = pWolfen->actorState;
	bVar7 = false;
	uVar10 = pWolfen->GetStateFlags(pWolfen->actorState);
	if ((uVar10 & 8) == 0) {
		if ((param_2 == 0x80000) || (param_2 == 0x40000)) {
			if (this->pOwner->prevActorState == 8) {
				bVar7 = true;
			}
		}
		else {
			if (param_2 == 0x20000) {
				pCVar3 = this->pOwner->pAdversary;
				if ((pCVar3 == (CActorFighter*)0x0) ||
					(bVar8 = pCVar3->FUN_0031b790(pCVar3->actorState), bVar8 == false)) {
					bVar7 = true;
				}
			}
			else {
				if (param_2 == 0x10000) {
					if (this->field_0x3c == 0) {
						bVar7 = true;
					}
					else {
						fVar4 = (this->holdPosition).x - this->pOwner->currentLocation.x;
						fVar5 = (this->holdPosition).z - this->pOwner->currentLocation.z;
						bVar7 = sqrtf(fVar4 * fVar4 + 0.0f + fVar5 * fVar5) < this->field_0x50;
					}
				}
				else {
					if (param_2 == 0x4000) {
						if (((ulong)this->pOwner->field_0x44c << 0x38) >> 0x3c != 1) {
							pTVar11 = Timer::GetTimer();
							this->adversaryBlowDuration = pTVar11->scaledTotalTime + 2.0f;
						}
						pTVar11 = Timer::GetTimer();
						if (pTVar11->scaledTotalTime < this->adversaryBlowDuration) {
							pWolfen = static_cast<CActorWolfen*>(this->pOwner);
							pCVar3 = pWolfen->pAdversary;
							if ((pCVar3 == (CActorFighter*)0x0) ||
								(fVar12 = ((pWolfen->pCollisionData)->pObbPrim->field_0x90).z +
									((pCVar3->pCollisionData)->pObbPrim->field_0x90).z + 0.2f,
									fVar4 = pCVar3->currentLocation.x -
									pWolfen->currentLocation.x,
									fVar5 = pCVar3->currentLocation.z -
									pWolfen->currentLocation.z, bVar7 = false,
									fVar4 * fVar4 + fVar5 * fVar5 <= fVar12 * fVar12)) {
								bVar7 = true;
							}
							if (!bVar7) {
								return false;
							}
						}
						bVar7 = true;
					}
					else {
						if (param_2 == 0x8000) {
							if ((this->fightContext).field_0x4 == 0) {
								bVar7 = true;
							}
						}
						else {
							if (param_2 == 0x2000) {
								if ((this->fightContext).field_0x4 == 2) {
									bVar7 = true;
								}
							}
							else {
								if ((param_2 == 0x1000) || (param_2 == 0x800)) {
									bVar7 = true;
								}
								else {
									if ((param_2 == 0x200) || (param_2 == 0x10)) {
										bVar6 = false;
										bVar8 = false;
										if ((((ulong)this->pOwner->field_0x44c << 0x38) >> 0x3c == 0) &&
											((this->pOwner->field_0x44d & 0xf) == 0)) {
											bVar8 = true;
										}
										if ((bVar8) && ((this->pOwner->fightFlags & 1) == 0)) {
											bVar6 = true;
										}
										if (bVar6) {
											bVar7 = true;
										}
									}
									else {
										if (((((((param_2 == 0x400) || (param_2 == 0x1e0)) || (param_2 == 0x1c0)) ||
											((param_2 == 0x160 || (param_2 == 0xe0)))) || (param_2 == 0x180)) ||
											((((param_2 == 0x120 || (param_2 == 0xa0)) ||
												((param_2 == 0x140 || (((param_2 == 0xc0 || (param_2 == 0x60)) || (param_2 == 0x100)))))) ||
												((param_2 == 0x80 || (param_2 == 0x40)))))) || (param_2 == 0x20)) {
											bVar8 = this->pOwner->FUN_0031b790(this->pOwner->actorState);
											if ((bVar8 == false) && ((this->pOwner->field_0x44d & 0xf) == 0)) {
												bVar7 = true;
											}
										}
										else {
											if ((((param_2 == 8) || (param_2 == 4)) || (param_2 == 2)) || (param_2 == 1)) {
												pTVar11 = Timer::GetTimer();
												if (this->adversaryBlowDuration <= pTVar11->scaledTotalTime) {
													bVar6 = false;
													bVar8 = false;
													if ((((ulong)this->pOwner->field_0x44c << 0x38) >> 0x3c == 0) &&
														((this->pOwner->field_0x44d & 0xf) == 0)) {
														bVar8 = true;
													}
													if ((bVar8) && ((this->pOwner->fightFlags & 1) == 0)) {
														bVar6 = true;
													}
													if (bVar6) {
														bVar7 = true;
													}
												}
											}
											else {
												if (param_2 == 0) {
													bVar6 = false;
													bVar8 = false;
													if ((((ulong)this->pOwner->field_0x44c << 0x38) >> 0x3c == 0) &&
														((this->pOwner->field_0x44d & 0xf) == 0)) {
														bVar8 = true;
													}
													if ((bVar8) && ((this->pOwner->fightFlags & 1) == 0)) {
														bVar6 = true;
													}
													if (bVar6) {
														bVar7 = true;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else {
		bVar7 = false;
	}
	return bVar7;
}



void CBehaviourFighterWolfen::ValidateCommand()
{
	CActorFighter* pCVar2;
	bool bVar4;

	int iVar5 = this->currentCommandId;
	if (iVar5 != -1) {
		if (iVar5 == -1) {
			iVar5 = 0;
		}
		else {
			CActorWolfen* pWolfen = static_cast<CActorWolfen*>(this->pOwner);
			iVar5 = pWolfen->field_0xb64[iVar5].field_0x0;
		}

		if (iVar5 == 0x80000) {
			if (this->pOwner->prevActorState == 8) {
				(this->fightContext).field_0x0 = (this->fightContext).field_0x0 & 0xfb | 4;
			}
		}
		else {
			if ((iVar5 != 0x40000) &&
				(((((((iVar5 == 0x400 || (iVar5 == 0x200)) || (iVar5 == 0x10)) || ((iVar5 == 0x1e0 || (iVar5 == 0x1c0)))) || (iVar5 == 0x160)) ||
					((((iVar5 == 0xe0 || (iVar5 == 0x180)) || ((iVar5 == 0x120 || (((iVar5 == 0xa0 || (iVar5 == 0x140)) || (iVar5 == 0xc0)))))) ||
						((iVar5 == 0x60 || (iVar5 == 0x100)))))) || ((iVar5 == 0x80 || ((iVar5 == 0x40 || (iVar5 == 0x20)))))))) {
				this->field_0x28 = this->field_0x28 + 1;

				if (((this->pOwner->fightFlags & 0x40) == 0) && ((pCVar2 = this->pOwner->pAdversary, pCVar2 != (CActorFighter*)0x0 &&
					(bVar4 = pCVar2->FUN_0031b5d0(pCVar2->actorState), bVar4 != false)))) {
					this->field_0x38 = this->field_0x38 + 1;
				}
			}
		}

		this->currentCommandId = -1;
	}

	return;
}

void CBehaviourFighterWolfen::PickCommand()
{
	CActorWolfen* pWolfen;
	int pCVar2;
	CActorFighter* pAdversary;
	float fVar4;
	float fVar5;
	bool bVar6;
	bool bVar7;
	WFIGS_Capability* pWVar8;
	uint commandId;
	RndData* pRndData;
	StateConfig* pSVar11;
	int iVar12;
	CActorWeapon* pWeapon;
	s_fighter_blow* psVar13;
	edF32VECTOR4* pfVar14;
	long lVar15;
	ulong uVar16;
	uint uVar17;
	CFightIA::WFIGS_Chain* pCVar18;
	float local_8;
	float local_4;

	pWolfen = static_cast<CActorWolfen*>(this->pOwner);
	commandId = 0;

	if (pWolfen->pTargetActor_0xc80 == pWolfen->pAdversary) {
		if (pWolfen->pCommander->CanContinueToFight(pWolfen) != false) {
			pWolfen = static_cast<CActorWolfen*>(this->pOwner);
			pCVar2 = pWolfen->activeCapabilityIndex;
			if (((pCVar2 == 0x2) || (pCVar2 == 0x1)) || (pCVar2 == 0x0)) {
				pAdversary = this->pOwner->pAdversary;

				commandId = pAdversary->GetStateFlags(pAdversary->actorState);

				if ((commandId & 0x2000000) == 0) {
					pWolfen = static_cast<CActorWolfen*>(this->pOwner);

					if (pWolfen->GetActiveCapability() == (WFIGS_Capability*)0x0) {
						bVar7 = false;
					}
					else {
						bVar7 = pWolfen->GetActiveCapability()->rndChooser.CanPickRndData();
					}

					if (bVar7) {
						pWolfen = static_cast<CActorWolfen*>(this->pOwner);
						
						if (pWolfen->GetActiveCapability() == (WFIGS_Capability*)0x0) {
							pRndData = (RndData*)0x0;
						}
						else {
							pRndData = pWolfen->GetActiveCapability()->rndChooser.PickRndData();
						}

						commandId = pRndData->commandId;
					}
					else
					{
						commandId = 0;
					}

					if (pWolfen->activeCapabilityIndex == 1) {
						pWolfen->activeCapabilityIndex = 0;
					}
				}
				else {
					commandId = 0xffffffff;
				}
			}
			else {
				commandId = pWolfen->GetStateFlags(pWolfen->actorState);

				if ((commandId & 0x800000) == 0) {
					commandId = 0;
				}
				else {
					commandId = 0xffffffff;

					if (pWolfen->RequestFightAction(0) == false) {
						commandId = pWolfen->field_0xb38;
					}
				}
			}
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		pWolfen->FUN_001fbff0(0, '\x01');)

		pWolfen = static_cast<CActorWolfen*>(this->pOwner);

		if (pWolfen->GetActiveCapability() == (WFIGS_Capability*)0x0) {
			bVar7 = false;
		}
		else {
			bVar7 = pWolfen->GetActiveCapability()->rndChooser.CanPickRndData();
		}

		if (bVar7) {
			pWolfen = static_cast<CActorWolfen*>(this->pOwner);

			if (pWolfen->GetActiveCapability() == (WFIGS_Capability*)0x0) {
				pRndData = (RndData*)0x0;
			}
			else {
				pRndData = pWolfen->GetActiveCapability()->rndChooser.PickRndData();
			}

			commandId = pRndData->commandId;
		}
		else {
			commandId = 0;
		}
	}

	bVar7 = pWolfen->CanPerformWeaponCommand();
	if (bVar7 != false) {
		pWeapon = this->pOwner->GetWeapon();
		iVar12 = pWeapon->GetBurstState();
		if ((((iVar12 == 1) && (psVar13 = this->pOwner->FindBlowByName("CROSSBOW_FIRE"), psVar13 != (s_fighter_blow*)0x0)) &&
			(pfVar14 = this->pOwner->GetAdversaryPos(),
				fVar4 = pfVar14->x - this->pOwner->currentLocation.x,
				fVar5 = pfVar14->z - this->pOwner->currentLocation.z,
				4.0f < sqrtf(fVar4 * fVar4 + 0.0f + fVar5 * fVar5))) &&
			(pfVar14 = this->pOwner->GetAdversaryPos(),
				fVar4 = pfVar14->x - this->pOwner->currentLocation.x,
				fVar5 = pfVar14->z - this->pOwner->currentLocation.z,
				sqrtf(fVar4 * fVar4 + 0.0f + fVar5 * fVar5) < 10.0f)) {
			commandId = pWolfen->field_0xb40;
		}
	}

	ValidateCommand();

	this->currentCommandId = -1;

	if (commandId != 0xffffffff) {
		if (commandId == 0xffffffff) {
			uVar17 = 0;
		}
		else {
			uVar17 = pWolfen->field_0xb64[commandId].field_0x0;
		}

		InitCommand(uVar17);
		this->field_0x68 = 1;
	}

	this->currentCommandId = commandId;
	return;
}


s_fighter_combo* CBehaviourFighterWolfen::PickCombo_Attack(CFightContext* pFightContext, bool param_3, bool param_4, bool param_5, bool param_6)
{
	CActorWolfen* pCVar1;
	uint uVar2;
	bool bVar3;
	bool bVar4;
	ulong uVar5;
	ComboData* pCVar6;
	int iVar7;
	uint uVar8;
	ComboData** ppCVar9;
	s_fighter_combo* psVar10;
	s_fighter_combo* psVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float local_14;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	psVar11 = (s_fighter_combo*)0x0;
	pCVar1 = static_cast<CActorWolfen*>(this->pOwner);
	ppCVar9 = &pCVar1->field_0xbd0;
	uVar8 = 0;
	if (pCVar1->nbComboMatchValues != 0) {
		fVar13 = -3.402823e+38f;
		psVar10 = psVar11;
		do {
			psVar11 = (*ppCVar9)[uVar8].pCombo;
			pCVar1->GetComboMatchValues(uVar8, &local_4, &local_8, &local_c, &local_10);
			fVar14 = 0.0f;
			bVar3 = true;
			fVar12 = 0.0f;

			if (param_3 != false) {
				local_4 = edFIntervalLERP(pCVar1->field_0xa84, 1.0f, 0.0f, local_4, 1.0f);
				if (0.5f <= local_4) {
					fVar14 = local_4 + 0.0f;
					fVar12 = fVar12 + 1.0f;
				}
				else {
					bVar3 = false;
				}
			}

			if (param_5 != false) {
				local_c = edFIntervalLERP(pCVar1->field_0xa84, 1.0f, 0.0f, local_c, 1.0f);
				if (0.5f <= local_c) {
					fVar14 = fVar14 + local_c;
					fVar12 = fVar12 + 1.0f;
				}
				else {
					bVar3 = false;
				}
			}

			if (param_4 != false) {
				local_8 = edFIntervalLERP(pCVar1->field_0xa84, 1.0f, 0.0f, local_8, 1.0f);
				if (0.5 <= local_8) {
					fVar14 = fVar14 + local_8;
					fVar12 = fVar12 + 1.0;
				}
				else {
					bVar3 = false;
				}
			}

			if (param_6 != false) {
				local_10 = edFIntervalLERP(pCVar1->field_0xa84, 1.0f, 0.0f, local_10, 1.0f);
				if (0.5f <= local_10) {
					fVar14 = fVar14 + local_10;
					fVar12 = fVar12 + 1.0f;
				}
				else {
					bVar3 = false;
				}
			}

			if (0.0f < fVar14) {
				fVar14 = fVar14 / fVar12;
			}
			else {
				bVar3 = false;
			}

			fVar12 = fVar14;
			if (fVar14 <= fVar13) {
				fVar12 = fVar13;
				psVar11 = psVar10;
			}

			if (bVar3) {
				(*ppCVar9)[uVar8].field_0x4 = fVar14;
				pCVar1->field_0xbd8 = 0;

				if ((*ppCVar9)[uVar8].field_0x0 != 1) {
					(*ppCVar9)[uVar8].field_0x0 = 1;
					pCVar1->field_0xbd8 = 0;
				}
			}
			else {
				if ((*ppCVar9)[uVar8].field_0x0 != 0) {
					(*ppCVar9)[uVar8].field_0x0 = 0;
					pCVar1->field_0xbd8 = 0;
				}
			}

			uVar8 = uVar8 + 1;
			fVar13 = fVar12;
			psVar10 = psVar11;
		} while (uVar8 < pCVar1->nbComboMatchValues);
	}

	bVar3 = false;
	uVar8 = 0;
	while (true) {
		bVar4 = false;
		if ((!bVar3) && (uVar8 < pCVar1->nbComboMatchValues)) {
			bVar4 = true;
		}

		if (!bVar4) break;

		bVar4 = false;
		if (((*ppCVar9)[uVar8].field_0x0 == 1) && (0.0f < (*ppCVar9)[uVar8].field_0x4)) {
			bVar4 = true;
		}

		if (bVar4) {
			bVar3 = true;
		}

		uVar8 = uVar8 + 1;
	}
	if (bVar3) {
		if (pCVar1->field_0xbd8 == 0) {
			local_14 = 0.0;
			uVar8 = 0;
			if (pCVar1->nbComboMatchValues != 0) {
				do {
					if ((*ppCVar9)[uVar8].field_0x0 != 0) {
						local_14 = local_14 + (*ppCVar9)[uVar8].field_0x4;
					}

					(*ppCVar9)[uVar8].field_0x8 = local_14;
					uVar8 = uVar8 + 1;
				} while (uVar8 < pCVar1->nbComboMatchValues);
			}

			uVar8 = 0;
			if (pCVar1->nbComboMatchValues != 0) {
				do {
					(*ppCVar9)[uVar8].field_0x8 = ((*ppCVar9)[uVar8].field_0x8 * 32767.0f) / local_14;
					uVar8 = uVar8 + 1;
				} while (uVar8 < pCVar1->nbComboMatchValues);
			}

			(*ppCVar9)[pCVar1->nbComboMatchValues - 1].field_0x8 = 0x7fff;
			pCVar1->field_0xbd8 = 1;
		}

		uVar2 = pCVar1->nbComboMatchValues;
		uVar8 = 0;
		if (uVar2 != 0) {
			pCVar6 = *ppCVar9;
			do {
				if ((int)CScene::Rand() <= (int)pCVar6->field_0x8) {
					if (uVar8 == pCVar1->field_0xbdc) {
						if (pCVar1->field_0xbe0 < pCVar1->field_0xbe4) {
							pCVar1->field_0xbe0 = pCVar1->field_0xbe0 + 1;
						}
						else {
							do {
								uVar8 = (int)(uVar8 + 1) % (int)uVar2;
								if (uVar2 == 0) {
									trap(7);
								}

								bVar3 = true;

								if (((*ppCVar9)[uVar8].field_0x4 != 0.0f) && ((*ppCVar9)[uVar8].field_0x0 != 0)) {
									bVar3 = false;
								}
							} while (bVar3);

							pCVar1->field_0xbdc = uVar8;
							pCVar1->field_0xbe0 = 0;
						}
					}
					else {
						pCVar1->field_0xbdc = uVar8;
						pCVar1->field_0xbe0 = 0;
					}
					break;
				}

				uVar8 = uVar8 + 1;
				pCVar6 = pCVar6 + 1;
			} while (uVar8 < pCVar1->nbComboMatchValues);
		}

		psVar11 = (*ppCVar9)[uVar8].pCombo;
	}

	return psVar11;
}



void CBehaviourFighterWolfen::FunReset()
{
	this->field_0x70 = 1;
	this->field_0x71 = 0;
	this->field_0x74.all = 0x0;
	this->pActiveBlow = (s_fighter_blow*)0x0;
	this->field_0x80 = gF32Vector4Zero;
	(this->field_0x90).field_0x4 = 0;
	(this->field_0x90).field_0x0 = (edF32VECTOR4*)0x0;

	(this->field_0x74).field_0x1 = (this->field_0x74).field_0x1 & 0xf0 | 8;
	this->field_0x70 = 1;

	return;
}

void WFIGS_Capability::Create(ByteCode* pByteCode, WolfenComboData* pComboData, float multiplier, uint* pOutCount)
{
	this->nbItems = pByteCode->GetU32();
	this->rndChooser.nbItems = this->nbItems;
	if (this->rndChooser.nbItems == 0) {
		this->rndChooser.pItems = (CFightIA::WFIGS_Chain*)0x0;
	}
	else {
		this->rndChooser.pItems = new CFightIA::WFIGS_Chain[this->rndChooser.nbItems];
	}

	uint curItemIndex = 0;
	if (this->nbItems != 0) {
		uint uVar2 = (*pOutCount);
		do {
			uint uVar4 = pByteCode->GetU32();
			uint uVar10 = 0;
			(*pOutCount) = uVar2;
			if (uVar4 != 0) {
				do {
					pComboData->field_0x0 = pByteCode->GetU32();
					int uVar5;
					if (uVar10 < uVar4 - 1) {
						uVar5 = (*pOutCount) + 1;
					}
					else {
						uVar5 = -1;
					}
					pComboData->field_0x8 = uVar5;
					uVar10 = uVar10 + 1;
					pComboData->field_0x4 = (*pOutCount);
					pComboData = pComboData + 1;
					(*pOutCount) = (*pOutCount) + 1;
				} while (uVar10 < uVar4);
			}

			float fVar13 = pByteCode->GetF32();
			float fVar14 = pByteCode->GetF32();
			float fVar12 = multiplier;

			CFightIA::WFIGS_Chain* pChain = this->rndChooser.pItems + curItemIndex;

			pChain->rndData.field_0x0 = uVar4;
			pChain->rndData.commandId = uVar2;

			this->rndChooser.field_0x8 = 0;
			pChain->field_0x4 = fVar14 + fVar12 * (fVar13 - fVar14);
			this->rndChooser.field_0x8 = 0;

			if (pChain->field_0x0 != 1) {
				pChain->field_0x0 = 1;
				this->rndChooser.field_0x8 = 0;
			}

			curItemIndex = curItemIndex + 1;
			uVar2 = (*pOutCount);
		} while (curItemIndex < this->nbItems);
	}
}

void WFIGS_Capability::Begin()
{
	uint curIndex = 0;
	if (this->rndChooser.nbItems != 0) {
		do {
			this->rndChooser.pItems[curIndex].field_0x0 = 1;
			curIndex = curIndex + 1;
		} while (curIndex < this->rndChooser.nbItems);
	}

	this->rndChooser.field_0x10 = 0;
	this->rndChooser.field_0x8 = 0;

	return;
}

bool WFIGS_Capability::Get()
{
	bool bResult = false;
	uint currentIndex = 0;

	while (true) {
		bool bVar3 = false;
		if ((!bResult) && (currentIndex < this->rndChooser.nbItems)) {
			bVar3 = true;
		}

		if (!bVar3) break;

		CFightIA::WFIGS_Chain* pChain = &this->rndChooser.pItems[currentIndex];
		bVar3 = false;

		if ((pChain->field_0x0 == 1) && (0.0f < pChain->field_0x4)) {
			bVar3 = true;
		}

		if (bVar3) {
			bResult = true;
		}

		currentIndex = currentIndex + 1;
	}

	return bResult;
}


void CActorWolfenKnowledge::Init(int memMode, uint param_3, uint param_4, uint nbObjs, uint param_6)
{
	int iVar1;
	int iVar3;
	uint uVar4;

	this->memMode = memMode;
	this->nbSubObjs = nbObjs;
	this->field_0xc = param_6;
	this->field_0x14 = param_3;
	this->field_0x18 = param_4 + param_3;
	iVar1 = this->field_0xc;
	iVar3 = this->nbSubObjs * sizeof(CActorWolfenKnowledge_0x14);
	this->aSubObjs = reinterpret_cast<CActorWolfenKnowledge_0x14*>(edMemAlloc(TO_HEAP(H_MAIN), iVar3));
	memset(this->aSubObjs, 0, iVar3);

	uVar4 = 0;
	if (this->nbSubObjs != 0) {
		do {
			this->aSubObjs[uVar4].field_0x4 = edMemAlloc(TO_HEAP(H_MAIN), iVar1 * sizeof(void*));
			uVar4 = uVar4 + 1;
		} while (uVar4 < this->nbSubObjs);
	}

	Reset();

	return;
}

void CActorWolfenKnowledge::Reset()
{
	uint uVar2;

	uVar2 = 0;
	if (this->nbSubObjs != 0) {
		do {
			memset(this->aSubObjs[uVar2].field_0x4, 0xff, this->field_0xc * sizeof(void*));
			this->aSubObjs[uVar2].field_0x8 = 0;
			this->aSubObjs[uVar2].field_0x0 = 0;
			this->aSubObjs[uVar2].field_0xc = 0;
			this->aSubObjs[uVar2].field_0x10 = 0;
			uVar2 = uVar2 + 1;
		} while (uVar2 < this->nbSubObjs);
	}

	this->field_0x4 = 0;
	this->field_0x20 = (s_fighter_combo*)0x0;
	this->field_0x24 = 0;
	this->field_0x28 = (s_fighter_combo*)0x0;
	this->field_0x2c = 0;
	this->field_0x1c = 0;

	return;
}

