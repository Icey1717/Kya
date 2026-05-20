#include "ActorBrazul.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "ActorManager.h"
#include "CollisionRay.h"
#include "WayPoint.h"

void CActorBrazul::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int materialId;
	uint uVar2;
	uint uVar3;
	astruct_19* pCVar4;
	float fVar5;

	CActorWolfen::Create(pByteCode);

	this->field_0xfc0 = pByteCode->GetF32();
	this->field_0xfc4 = pByteCode->GetF32();

	this->field_0x1050.Create(pByteCode);

	this->field_0x1060 = pByteCode->GetU32();
	pByteCode->GetU32();
	materialId = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(materialId);
	uVar1 = pByteCode->GetU32();
	uVar2 = pByteCode->GetU32();
	uVar3 = 0;
	pCVar4 = this->field_0x1070;
	do {
		pCVar4->Create(materialId, uVar1, uVar2);
		uVar3 = uVar3 + 1;
		pCVar4 = pCVar4 + 1;
	} while (uVar3 < 8);

	this->field_0x3000.Create(pByteCode);
	this->field_0x300c.Create(pByteCode);
	this->field_0x30a0.Create(pByteCode);

	uVar1 = pByteCode->GetU32();
	uVar2 = 0;
	CFxHandleExt* pHandle = this->field_0x30ac;
	do {
		pHandle->Init(uVar1);
		uVar2 = uVar2 + 1;
		pHandle = pHandle + 1;
	} while (uVar2 < 6);

	this->field_0x3100.Create(pByteCode);
	this->field_0x31e0.Create(pByteCode);

	this->field_0x3260.Create(pByteCode);
	this->field_0x326c.Create(pByteCode);
	this->field_0x3020.SetObjCounts(0x10, 0x15);

	return;
}

void DirectRayOnHit(CActorFighter* pFighter, int param_2)
{
	IMPLEMENTATION_GUARD();
}

void TornadoRayOnHit(CActorFighter* pFighter, int param_2)
{
	IMPLEMENTATION_GUARD();
}

void CActorBrazul::Init()
{
	CAnimation* pCVar1;
	CBehaviour* pCVar2;
	s_fighter_blow* psVar3;
	int iVar4;
	astruct_19* pCVar5;
	uint uVar6;
	float fVar7;

	CActorWolfen::Init();

	if (this->field_0x1060 != 4) {
		this->field_0x1050.Init();

		if ((this->field_0x1050).pPathFollow == (CPathFollow*)0x0) {
			this->field_0x1060 = 4;
		}
		else {
			pCVar2 = GetBehaviour(0x1a);
			if ((pCVar2 == (CBehaviour*)0x0) && (pCVar2 = GetBehaviour(0x1b), pCVar2 == (CBehaviour*)0x0)) {
				this->field_0x1060 = 4;
			}
		}
	}

	uVar6 = 0;
	this->flags = this->flags | 0x400;
	pCVar5 = this->field_0x1070;
	do {
		pCVar5->Init(this);
		uVar6 = uVar6 + 1;
		pCVar5 = pCVar5 + 1;
	} while (uVar6 < 8);

	this->field_0x3020.SetupObjects(this);

	psVar3 = FindBlowByName("DIRECT_RAY");
	if (psVar3 != (s_fighter_blow*)0x0) {
		psVar3->field_0xd0 = DirectRayOnHit;
	}

	psVar3 = FindBlowByName("TORNADO_RAY");
	if (psVar3 != (s_fighter_blow*)0x0) {
		psVar3->field_0xd0 = TornadoRayOnHit;
	}

	this->field_0x30a0.Stop();

	uVar6 = 0;
	CFxHandleExt* pHandleExt = this->field_0x30ac;
	do {
		pHandleExt->Stop();
		uVar6 = uVar6 + 1;
		pHandleExt = pHandleExt + 1;
	} while (uVar6 < 6);

	pCVar1 = this->pAnimationController;
	iVar4 = GetIdMacroAnim(0xeb);
	if (iVar4 < 0) {
		this->field_0x2ff0 = 0.0f;
	}
	else {
		fVar7 = pCVar1->GetAnimLength(iVar4, 1);
		this->field_0x2ff0 = fVar7;
	}

	pCVar1 = this->pAnimationController;
	iVar4 = GetIdMacroAnim(0xed);
	if (iVar4 < 0) {
		this->field_0x2ff4 = 0.0f;
	}
	else {
		fVar7 = pCVar1->GetAnimLength(iVar4, 1);
		this->field_0x2ff4 = fVar7;
	}

	this->field_0x2ff8 = 0;
	this->field_0x2ffc = 0;

	return;
}

uint UINT_ARRAY_00437ca0[28] = {
	0xD3DBDAB7,     0x65A1A5A9,     0x65A2A5A9,     0x65A3A5A9,
	0xA9D802E7,     0xA9D803E7,     0xA9D804E7,     0xA9D805E7,
	0xA9D806E7,     0xA9D807E7,     0xA9D802E7,     0xA9D803E7,
	0xA9D804E7,     0xA9D805E7,     0xA9D806E7,     0xA9D807E7,
	0x5A5958,     0x0,     0x0,     0x0,
	0xA9D802E7, 0xA9D803E7, 0xA9D804E7, 0xA9D805E7, 0xA9D806E7, 0xA9D807E7, 0x0, 0x0
};

uint* UINT_ARRAY_00437cf0 = UINT_ARRAY_00437ca0 + 20;

void CActorBrazul::Term()
{
	int iVar2;
	bool bVar3;
	uint* puVar4;
	CFxHandleExt* pCVar5;
	uint uVar6;

	CActorWolfen::Term();

	this->field_0x3020.Term();

	uVar6 = 0;
	puVar4 = UINT_ARRAY_00437cf0;
	pCVar5 = this->field_0x30ac;
	do {
		if (pCVar5->IsValid()) {
			this->pAnimationController->UnRegisterBone(*puVar4);
			pCVar5->Stop();
		}
		uVar6 = uVar6 + 1;
		pCVar5 = pCVar5 + 1;
		puVar4 = puVar4 + 1;
	} while (uVar6 < 6);

	return;
}

edF32VECTOR4 edF32VECTOR4_00427990 = { 1.0f, 1.0f, 1.0f, 1.0f };
edF32VECTOR4 edF32VECTOR4_004279a0 = { 0.0f, 1.3f, 0.0f, 0.0f };

void CActorBrazul::Manage()
{
	CNewFx* pCVar1;
	int iVar2;
	bool bVar3;
	CLifeInterface* uVar5;
	CBehaviour* pCVar5;
	edF32VECTOR4* peVar6;
	astruct_19* pCVar7;
	uint uVar8;
	uint* puVar9;
	float fVar10;
	edF32VECTOR4 local_d0;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 eStack64;

	local_c0 = edF32VECTOR4_00427990;
	local_d0 = edF32VECTOR4_004279a0;

	CActorWolfen::Manage();

	this->field_0xfcc = this->field_0xfcc + GetTimer()->cutsceneDeltaTime;

	if (this->curBehaviourId != 3) goto LAB_003e8da8;

	if (this->field_0xfcc < this->field_0xfc0) {
		if (this->field_0xfc4 == 0.0f) goto LAB_003e8da8;

		fVar10 = GetLifeInterface()->GetValue();

		if (this->field_0xfc4 < fVar10) goto LAB_003e8da8;
	}

	pCVar5 = GetBehaviour(0x1a);
	if (pCVar5 == (CBehaviour*)0x0) {
		pCVar5 = GetBehaviour(0x1b);
		if (pCVar5 != (CBehaviour*)0x0) {
			this->field_0xfc8 = 0x1b;
		}
	}
	else {
		this->field_0xfc8 = 0x1a;
	}

LAB_003e8da8:
	uVar8 = 0;
	pCVar7 = this->field_0x1070;
	do {
		pCVar7->Manage(&local_d0);
		uVar8 = uVar8 + 1;
		pCVar7 = pCVar7 + 1;
	} while (uVar8 < 8);

	FUN_003e60c0();
	FUN_003e59e0();

	pCVar1 = (this->field_0x30a0).pFx;
	if (((pCVar1 == (CNewFx*)0x0) || (iVar2 = (this->field_0x30a0).id, iVar2 == 0)) || (iVar2 != pCVar1->id)) {
		bVar3 = false;
	}
	else {
		bVar3 = true;
	}

	if (!bVar3) {
		this->field_0x30a0.InitSpatialized(this, 0x554f43);
	}

	uVar8 = 0;
	puVar9 = UINT_ARRAY_00437cf0;
	CFxHandleExt* pHandleExt = this->field_0x30ac;
	do {
		if (pHandleExt->IsValid()) {
			SV_GetBoneWorldPosition(*puVar9, &local_90);
			pHandleExt->SetPosition(&local_90);
		}
		else {
			this->pAnimationController->RegisterBone(*puVar9);
			pHandleExt->InitPositionRotation((edF32VECTOR4*)0x0, (edF32VECTOR4*)0x0);
		}

		uVar8 = uVar8 + 1;
		pHandleExt = pHandleExt + 1;
		puVar9 = puVar9 + 1;
	} while (uVar8 < 6);

	uVar8 = 0;
	pHandleExt = this->field_0x30ac;
	do {
		if (uVar8 < 5) {
			pCVar1 = pHandleExt[1].pFx;
			if (((pCVar1 == (CNewFx*)0x0) || (iVar2 = pHandleExt[1].id, iVar2 == 0)) || (iVar2 != pCVar1->id)) {
				peVar6 = (edF32VECTOR4*)0x0;
			}
			else {
				peVar6 = &pCVar1->position;
			}

			local_b0 = *peVar6;
		}
		else {
			pCVar1 = this->field_0x30ac[0].pFx;
			if (((pCVar1 == (CNewFx*)0x0) || (iVar2 = this->field_0x30ac[0].id, iVar2 == 0)) || (iVar2 != pCVar1->id)) {
				peVar6 = (edF32VECTOR4*)0x0;
			}
			else {
				peVar6 = &pCVar1->position;
			}

			local_b0 = *peVar6;
		}

		pCVar1 = pHandleExt[0].pFx;
		if (((pCVar1 == (CNewFx*)0x0) || (iVar2 = pHandleExt[0].id, iVar2 == 0)) || (peVar6 = &pCVar1->position, iVar2 != pCVar1->id)) {
			peVar6 = (edF32VECTOR4*)0x0;
		}

		edF32Vector4SubHard(&local_a0, peVar6, &local_b0);
		edF32Vector4AddHard(&local_90, &local_a0, &local_b0);
		local_a0.y = 0.0f;
		local_c0.x = edF32Vector4SafeNormalize0Hard(&local_b0, &local_a0);
		edF32Matrix4ScaleHard(&eStack128, &gF32Matrix4Unit, &local_c0);
		local_a0.x = -local_b0.z;
		local_a0.z = local_b0.x;
		local_b0.x = 0.0f;
		local_b0.y = GetAngleYFromVector(&local_a0);
		local_b0.z = 0.0f;
		local_b0.w = 0.0f;
		edF32Matrix4FromEulerSoft(&eStack64, &local_b0.xyz, "XYZ");
		edF32Matrix4TranslateHard(&eStack64, &eStack64, &local_90);
		edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack128, &eStack64);
		SV_FX_001a0600(pHandleExt, &eStack64, pHandleExt);
		uVar8 = uVar8 + 1;
		pHandleExt = pHandleExt + 1;
	} while (uVar8 < 6);

	return;
}

void CActorBrazul::Draw()
{
	uint uVar1;

	CActorWolfen::Draw();

	uVar1 = 0;
	do {
		this->field_0x1070[uVar1].Draw();
		uVar1 = uVar1 + 1;
	} while (uVar1 < 8);

	return;
}

void CActorBrazul::Reset()
{
	CBehaviour* pCVar2;
	CBehaviourBrazulA* pCVar1;
	astruct_19* pCVar3;
	uint uVar4;

	CActorWolfen::Reset();
	uVar4 = 0;
	pCVar3 = this->field_0x1070;
	do {
		pCVar3->Reset(0.0f);
		uVar4 = uVar4 + 1;
		pCVar3 = pCVar3 + 1;
	} while (uVar4 < 8);

	FUN_003e62d0(0.0f);
	FUN_003e5f40(0.0f);

	pCVar2 = GetBehaviour(0x1a);
	if (pCVar2 != (CBehaviour*)0x0) {
		pCVar1 = static_cast<CBehaviourBrazulA*>(GetBehaviour(0x1a));
		IMPLEMENTATION_GUARD(
		*(undefined4*)&pCVar1->field_0x14 = 0;
		*(undefined4*)&pCVar1->field_0x10 = 0;)
	}

	return;
}

CBehaviour* CActorBrazul::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 0x1b) {
		pBehaviour = new CBehaviourBrazulB;
	}
	else {
		if (behaviourType == 0x1a) {
			pBehaviour = new CBehaviourBrazulA;
		}
		else {
			if (behaviourType == 3) {
				pBehaviour = new CBehaviourFighterBrazul;
			}
			else {
				pBehaviour = CActorWolfen::BuildBehaviour(behaviourType);
			}
		}
	}

	return pBehaviour;
}

StateConfig CActorBrazul::gStateCfg_BRZ[12] = {
	StateConfig(0xE6, 0x08100000),
	StateConfig(0xE7, 0x02000000),
	StateConfig(0xE8, 0x02000000),
	StateConfig(0xE9, 0x02000000),
	StateConfig(0xEA, 0x02000100),
	StateConfig(0xEB, 0x08000000),
	StateConfig(0xEC, 0x08000000),
	StateConfig(0xED, 0x00000000),
	StateConfig(0xEE, 0x02000000),
	StateConfig(0x00, 0x02000100),
	StateConfig(0xEF, 0x00000000),
	StateConfig(0xF0, 0x00000000),
};

StateConfig* CActorBrazul::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 0xb6) {
		pStateConfig = CActorWolfen::GetStateCfg(state);
	}
	else {
		assert((state - 0xb6) < 12);
		pStateConfig = gStateCfg_BRZ + state + -0xb6;
	}

	return pStateConfig;
}

uint CActorBrazul::GetBehaviourFlags(int state)
{
	uint uVar1;

	if (state < 0x1a) {
		uVar1 = CActorWolfen::GetBehaviourFlags(state);
	}
	else {
		assert(state < 28);
		uVar1 = UINT_ARRAY_00437ca0[state];
	}

	return uVar1;
}

void CActorBrazul::UpdatePostAnimEffects()
{
	int iVar1;

	iVar1 = this->field_0x3020.UpdatePostAnimEffects();
	if (iVar1 == 0) {
		CActorWolfen::UpdatePostAnimEffects();
	}

	return;
}

bool CActorBrazul::SetBehaviour(int behaviourId, int newState, int animationType)
{
	bool bVar1;
	bool uVar2;

	bVar1 = IsFightRelated(behaviourId);
	if ((bVar1 == false) || (uVar2 = IsFightRelated(this->curBehaviourId), uVar2 == false)) {
		this->field_0xfc8 = -1;
		this->field_0xfcc = 0.0f;
	}

	return CActorFighter::SetBehaviour(behaviourId, newState, animationType);
}

struct BrazulStateStruct
{
	int field_0x0;
	byte field_0x4;
	byte field_0x5;
	byte field_0x6;
	byte field_0x7;
	float field_0x8;
};

BrazulStateStruct BrazulStateStruct_ARRAY_00426d20[34] = {
	{ 0x0C, 0, 2, 2, 0, 0.5f },
	{ 0x15, 1, 1, 1, 0, 0.0f },
	{ 0x21, 1, 1, 0, 0, 0.0f },
	{ 0x22, 1, 1, 0, 0, 0.0f },
	{ 0x23, 1, 1, 0, 0, 0.0f },
	{ 0x24, 1, 1, 0, 0, 0.0f },
	{ 0x25, 1, 1, 0, 0, 0.0f },
	{ 0x26, 1, 1, 0, 0, 0.0f },
	{ 0x60, 1, 1, 0, 0, 0.0f },
	{ 0x61, 1, 1, 0, 0, 0.0f },
	{ 0x62, 1, 1, 0, 0, 0.0f },
	{ 0x63, 1, 1, 0, 0, 0.0f },
	{ 0x64, 1, 1, 0, 0, 0.0f },
	{ 0x65, 1, 1, 0, 0, 0.0f },
	{ 0x66, 1, 1, 0, 0, 0.0f },
	{ 0x67, 1, 1, 0, 0, 0.0f },
	{ 0x68, 1, 1, 0, 0, 0.0f },
	{ 0x69, 1, 1, 0, 0, 0.0f },
	{ 0x6A, 1, 3, 1, 0, 0.1f },
	{ 0x6B, 1, 3, 1, 0, 0.1f },
	{ 0x6C, 1, 3, 1, 0, 0.1f },
	{ 0x6D, 1, 3, 1, 0, 0.1f },
	{ 0x6E, 1, 3, 1, 0, 0.1f },
	{ 0x6F, 1, 3, 1, 0, 0.1f },
	{ 0x70, 1, 3, 1, 0, 0.1f },
	{ 0xE2, 0, 2, 2, 0, 0.5f },
	{ 0xE3, 0, 2, 2, 0, 0.5f },
	{ 0xE4, 0, 2, 2, 0, 0.5f },
	{ 0xE5, 0, 2, 2, 0, 0.5f },
	{ 0xE6, 1, 1, 1, 0, 0.0f },
	{ 0xE9, 0, 1, 2, 0, 0.0f },
	{ 0xEB, 0, 1, 2, 0, 0.0f },
	{ 0xEF, 1, 0, 0, 0, 0.0f },
	{ 0xF0, 1, 0, 1, 0, 0.0f },
};

void CActorBrazul::SetState(int newState, int animType)
{
	bool bVar1;
	StateConfig* piVar2;
	BrazulStateStruct* pBVar2;
	BrazulStateStruct* pBVar3;
	uint uVar5;
	int iVar6;

	if (((newState != this->actorState) || ((animType != -1 && (animType != this->currentAnimType)))) && (newState != -1)) {
		iVar6 = animType;
		if (animType == -1) {
			piVar2 = GetStateCfg(newState);
			iVar6 = piVar2->animId;
		}

		uVar5 = 0;
		bVar1 = false;
		pBVar2 = BrazulStateStruct_ARRAY_00426d20;
		pBVar3 = (BrazulStateStruct*)0x0;
		while ((!bVar1 && (uVar5 < 0x22))) {
			if (iVar6 == pBVar2->field_0x0) {
				bVar1 = true;
				pBVar3 = pBVar2;
			}
			else {
				pBVar2 = pBVar2 + 1;
				uVar5 = uVar5 + 1;
			}
		}

		//this->field_0x3020.FUN_003e4460(puVar4, (int)&DAT_00426ec0, (int)&DAT_004276c0);)
	}

	CActorWolfen::SetState(newState, animType);

	return;
}

void CActorBrazul::ChangeManageState(int state)
{
	uint uVar1;

	CActorWolfen::ChangeManageState(state);

	if (state == 0) {
		this->field_0x30a0.Stop();
		uVar1 = 0;
		do {
			this->field_0x30ac[uVar1].Stop();
			uVar1 = uVar1 + 1;
		} while (uVar1 < 6);
	}

	return;
}

bool CActorBrazul::Can_0x9c()
{
	bool bVar1;

	bVar1 = (this->actorFieldS & 0x60) != 0;
	if (bVar1) {
		bVar1 = this->curBehaviourId == 0x1a;
	}

	return bVar1;
}

void CActorBrazul::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	edANM_HDR* peVar1;

	if (newAnim == 0xec) {
		IMPLEMENTATION_GUARD(
		peVar1 = pAnimator->pAnimKeyTableEntry;
		(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = this->field_0x2ffc;
		(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = (int)(1.0 - (float)this->field_0x2ffc);)
	}
	else {
		CActorWolfen::AnimEvaluate(layerId, pAnimator, newAnim);
	}

	return;
}

int CActorBrazul::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	uint uVar1;
	float fVar2;
	CLifeInterface* pCVar3;
	edF32VECTOR4* peVar4;
	int result;
	ulong uVar5;
	int pvVar6;
	int index;
	int iVar6;
	float fVar7;
	float fVar8;

	if (msg == 0x3b) {
		pCVar3 = GetLifeInterface();
		fVar7 = pCVar3->GetValue();
		if (fVar7 <= 0.0f) {
			result = 0;
		}
		else {
			uVar1 = this->field_0x1060;
			iVar6 = -1;
			result = iVar6;
			if ((uVar1 != 4) && (result = (int)pMsgParam, uVar1 != 3)) {
				if (uVar1 == 2) {
					fVar7 = -3.402823e+38f;
					for (index = 0; result = iVar6, index < ((this->field_0x1050).pPathFollow)->splinePointCount; index = index + 1) {
						peVar4 = this->field_0x1050.GetWayPoint(index);
						fVar8 = peVar4->x - this->currentLocation.x;
						fVar2 = peVar4->z - this->currentLocation.z;
						fVar8 = sqrtf(fVar8 * fVar8 + 0.0f + fVar2 * fVar2);
						result = index;
						if (fVar8 <= fVar7) {
							fVar8 = fVar7;
							result = iVar6;
						}
						iVar6 = result;
						fVar7 = fVar8;
					}
				}
				else {
					if (uVar1 == 1) {
						fVar7 = 3.402823e+38f;
						for (pvVar6 = 0; result = iVar6, pvVar6 < ((this->field_0x1050).pPathFollow)->splinePointCount; pvVar6 = pvVar6 + 1) {
							peVar4 = this->field_0x1050.GetWayPoint(pvVar6);
							fVar8 = peVar4->x - this->currentLocation.x;
							fVar2 = peVar4->z - this->currentLocation.z;
							fVar8 = sqrtf(fVar8 * fVar8 + 0.0f + fVar2 * fVar2);
							result = pvVar6;
							if (fVar7 <= fVar8) {
								fVar8 = fVar7;
								result = iVar6;
							}
							iVar6 = result;
							fVar7 = fVar8;
						}
					}
					else {
						result = iVar6;
						if (uVar1 == 0) {
							iVar6 = ((this->field_0x1050).pPathFollow)->splinePointCount;
							uVar5 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
							CScene::_pinstance->field_0x38 = uVar5;
							iVar6 = iVar6 * (static_cast<uint>(uVar5 >> 0x10) & 0x7fff);
							result = iVar6 >> 0xf;
							if (iVar6 < 0) {
								result = iVar6 + 0x7fff >> 0xf;
							}
						}
					}
				}
			}

			if (result != -1) {
				peVar4 = this->field_0x1050.GetWayPoint(result);
				this->field_0x1064 = peVar4;
				SetBehaviour(FIGHTER_BEHAVIOUR_DEFAULT, 0xb7, 0xffffffff);
			}

			result = 1;
		}
	}
	else {
		result = CActorWolfen::InterpretMessage(pSender, msg, pMsgParam);
	}

	return result;
}

void CActorBrazul::FUN_003e60c0()
{
	CActorManager* pCVar1;
	bool bVar2;
	Timer* pTVar3;
	CActor* pHitActor;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_1a0;
	edF32MATRIX4 eStack400;
	CFixedTable<CActor, 64> local_150;
	CActorBrazul* local_40[4];
	float local_30;
	float local_2c;
	float local_28;
	float local_24;
	float local_20;
	float local_1c;
	undefined4 local_18;
	edF32VECTOR4 local_10;

	bVar2 = this->field_0x3100.HasMesh();
	if (bVar2 != false) {
		IMPLEMENTATION_GUARD();
	}
}

void CActorBrazul::FUN_003e59e0()
{
	CActorFighter* pCVar1;
	CNewFx* pCVar2;
	bool bVar3;
	int iVar4;
	Timer* pTVar5;
	edF32VECTOR4* v2;
	float puVar6;
	float fVar6;
	float fVar7;
	CCollisionRay CStack208;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32MATRIX4 eStack144;
	undefined auStack80[52];
	float local_1c;
	float local_18;
	float local_14;
	_ray_info_out local_10;

	bVar3 = this->field_0x31e0.HasMesh();
	if (bVar3 != false) {
		IMPLEMENTATION_GUARD();
	}

	return;
}

void CActorBrazul::FUN_003e62d0(float param_1)
{
	this->field_0x3100.Func_0x2c(param_1);

	return;
}

void CActorBrazul::FUN_003e5f40(float param_1)
{
	this->field_0x31e0.Func_0x2c(param_1);
	this->field_0x3260.Reset();
	this->field_0x326c.Reset();

	return;
}

void CActorBrazul::FUN_003e87e0(int nextState, int param_3)
{
	CActorFighter* pCVar1;
	bool bVar2;
	bool bVar3;
	uint dynFlags;
	float fVar6;
	float fVar7;

	dynFlags = 0x40029;
	bVar2 = false;
	if (param_3 == 1) {
		dynFlags = 0x4002b;
		if (((this->pCollisionData)->flags_0x4 & 2) != 0) {
			bVar2 = true;
		}
	}
	else {
		if ((this->field_0xfd0).field_0x40 <= this->timeInAir) {
			bVar2 = true;
		}
	}

	pCVar1 = this->pTargetActor_0xc80;
	if (pCVar1 != (CActorFighter*)0x0) {
		SV_UpdateOrientationToPosition2D(6.283185f, &pCVar1->currentLocation);
	}

	this->field_0xfd0.Integrate(GetTimer()->cutsceneDeltaTime);
	bVar3 = this->field_0xfd0.IsFinished();
	if (bVar3 == false) {
		fVar6 = (this->field_0xfd0).field_0x60.y;
		fVar7 = (this->field_0xfd0).field_0x60.z;
		dynFlags = dynFlags | 0x1c000;
		this->dynamicExt.instanceIndex.x = (this->field_0xfd0).field_0x60.x;
		this->dynamicExt.instanceIndex.y = fVar6;
		this->dynamicExt.instanceIndex.z = fVar7;
		this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x1c000;
	}
	else {
		this->dynamic.speed = 0.0f;
	}

	ManageDyn(4.0f, dynFlags, (CActorsTable*)0x0);

	if (bVar2) {
		SetState(nextState, 0xffffffff);
	}

	return;
}

void astruct_19::Create(int param_2, int param_3, int param_4)
{
	this->field_0x40.Create(3, 0xc, this->field_0x130, this->field_0x154, param_2);
	//(this->field_0x40).field_0xe4 = &this->field_0x190;
	this->field_0x3d0.Init(param_3);
	this->field_0x3dc.Init(param_4);
	//(this->field_0x40).field_0x90 = &DAT_00a6a500;
	//(this->field_0x40).field_0x94 = &DAT_00a6a500;

	return;
}

void astruct_19::Init(CActorBrazul* param_2)
{
	this->pOwner = param_2;
	this->field_0x40.Init(this->pOwner->sectorId);
	this->field_0x40.SetParameters(0.5f, 0.025f, 20.0f, 0.1f, 8.0f, 3);
	(this->field_0x40).vector_0x80.x = 0.0f;
	(this->field_0x40).vector_0x80.y = 0.3f;
	(this->field_0x40).vector_0x80.z = 0.0f;
	(this->field_0x40).vector_0x80.w = 0.0f;

	return;
}

void astruct_19::Manage(edF32VECTOR4* v0)
{
	int iVar1;
	CNewFx* pCVar2;
	bool bVar3;
	Timer* pTVar4;
	float fVar5;
	float t;
	edF32VECTOR4 eStack224;
	edF32VECTOR4 eStack208;
	_msg_hit_param local_c0;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	_msg_hit_param* local_4;

	iVar1 = this->field_0x4;
	if (iVar1 == 3) {
		pTVar4 = GetTimer();
		fVar5 = this->field_0x34 - pTVar4->cutsceneDeltaTime;
		this->field_0x34 = fVar5;
		if (fVar5 <= 0.0f) {
			this->field_0x40.Reset();
			this->field_0x3d0.Stop();
			this->field_0x3dc.Stop();
			this->field_0x4 = 0;
		}
		edF32Vector4AddHard(&eStack208, &this->pOwner->currentLocation, v0);
		this->field_0x40.Manage(&eStack208, &this->field_0x10);
	}
	else {
		if (iVar1 == 2) {
			edF32Vector4SubHard(&eStack224, &this->field_0x30->currentLocation, (edF32VECTOR4*)&this->field_0x10);
			eStack224.y = 0.0f;
			fVar5 = edF32Vector4GetDistHard(&eStack224);
			if (fVar5 <= this->field_0x3c) {
				local_c0.projectileType = 5;
				local_4 = &local_c0;
				local_c0.damage = 5.0f;
				this->pOwner->DoMessage(this->field_0x30, MESSAGE_KICKED, local_4);
			}

			edF32Vector4AddHard(&eStack64, &this->pOwner->currentLocation, v0);
			this->field_0x40.Manage(&eStack64, &this->field_0x10);
		}
		else {
			if (iVar1 == 1) {
				edF32Vector4SubHard(&eStack32, &this->field_0x30->currentLocation, &this->field_0x10);
				eStack32.y = 0.0f;
				fVar5 = edF32Vector4SafeNormalize0Hard(&eStack48, &eStack32);
				pTVar4 = GetTimer();
				t = this->field_0x34 * pTVar4->cutsceneDeltaTime;
				if (t < fVar5) {
					edF32Vector4ScaleHard(t, &eStack32, &eStack48);
				}
				edF32Vector4AddHard(&this->field_0x10, &this->field_0x10, &eStack32);

				if (this->field_0x3d0.IsValid()) {
					this->field_0x3d0.SetPosition(&this->field_0x10);
				}

				pTVar4 = GetTimer();
				fVar5 = this->field_0x38 - pTVar4->cutsceneDeltaTime;
				this->field_0x38 = fVar5;
				if (fVar5 <= 0.0f) {
					if (this->field_0x4 == 1) {
						this->field_0x3d0.Stop();
					}

					this->field_0x3dc.InitPositionRotation(&this->field_0x10, &gF32Vertex4Zero);
					this->field_0x4 = 2;
				}
			}
		}
	}

	return;
}

void astruct_19::Reset(float param_1)
{
	if ((this->field_0x4 != 0) && (this->field_0x4 != 3)) {
		this->field_0x4 = 3;
		this->field_0x38 = param_1;
		this->field_0x34 = param_1;
	}

	return;
}

void astruct_19::Draw()
{
	int iVar1;
	undefined* puVar2;
	long lVar3;
	float fVar4;
	int iVar5;

	IMPLEMENTATION_GUARD_FX(
	iVar1 = this->field_0x4;
	lVar3 = 0;
	iVar5 = 0x7f;
	if (iVar1 != 0) {
		if (iVar1 != 2) {
			if (iVar1 != 3) goto LAB_003e53e8;
			fVar4 = (this->field_0x34 / this->field_0x38) * 127.0f;
			iVar5 = static_cast<int>(float)(static_cast<int>(fVar4) * static_cast<uint>(0.0f < fVar4));
		}
		puVar2 = static_cast<undefined*>(iVar5 << 0x18 | 0xa6a500);
		lVar3 = 1;
		(this->field_0x40).field_0x90 = puVar2;
		(this->field_0x40).field_0x94 = puVar2;
	}
LAB_003e53e8:
	this->field_0x40.Draw(lVar3);)

	return;
}

void CBehaviourBrazulA::Create(ByteCode* pByteCode)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourBrazulA::Init(CActor * pOwner)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourBrazulA::Term()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourBrazulA::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourBrazulA::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourBrazulA::End(int newBehaviourId)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourBrazulA::InitState(int newState)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourBrazulA::TermState(int oldState, int newState)
{
	IMPLEMENTATION_GUARD();
}

int CBehaviourBrazulA::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
	return 0;
}

void CBehaviourBrazulB::Create(ByteCode* pByteCode)
{
	this->waypointRef.index = pByteCode->GetS32();
	this->ntfSwitch.Create(pByteCode);

	return;
}

void CBehaviourBrazulB::Init(CActor * pOwner)
{
	this->waypointRef.Init();
	this->ntfSwitch.Init();

	return;
}

void CBehaviourBrazulB::Term()
{
	return;
}

void CBehaviourBrazulB::Manage()
{
	int iVar1;
	CActorBrazul* pBrazul;

	this->ntfSwitch.pStreamEventCamera->Manage(this->pOwner);

	pBrazul = this->pOwner;
	iVar1 = pBrazul->actorState;
	if (iVar1 == 0xbf) {
		pBrazul->ManageDyn(4.0f, 0x80600, (CActorsTable*)0x0);
	}
	else {
		if (iVar1 == 0xba) {
			pBrazul->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

			if (pBrazul->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				pBrazul->SetState(0xbf, 0xffffffff);
			}
		}
		else {
			if (iVar1 == 0xb9) {
				pBrazul->FUN_003e87e0(0xba, 1);
			}
			else {
				if (iVar1 == 0xb8) {
					pBrazul->FUN_003e87e0(0xb9, 0);
				}
				else {
					if (iVar1 == 0xb7) {
						pBrazul->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

						if (pBrazul->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
							pBrazul->SetState(0xb8, 0xffffffff);
						}
					}
				}
			}
		}
	}

	return;
}

void CBehaviourBrazulB::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CCollision* pCol;

	this->pOwner = static_cast<CActorBrazul*>(pOwner);

	this->ntfSwitch.Reset(this->pOwner);

	if (newState == -1) {
		this->pOwner->SetState(0xb7, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->dynamic.speed = 0.0f;
	pCol = this->pOwner->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 & ~0x81000;

	return;
}

void CBehaviourBrazulB::End(int newBehaviourId)
{
	CCollision* pCol;

	pCol = this->pOwner->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 | 0x81000;

	return;
}

void CBehaviourBrazulB::InitState(int newState)
{
	CWayPoint* pCVar1;
	CActorBrazul* pCVar2;
	StateConfig* pSVar3;
	int iVar4;
	S_NTF_TARGET_STREAM_REF* pSVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	if (newState == 0xbf) {
		this->ntfSwitch.SwitchOn(this->pOwner);
	}
	else {
		if (newState == 0xb8) {
			pCVar1 = (this->waypointRef).Get();
			local_10.xyz = pCVar1->location;
			local_10.w = 1.0f;
			pCVar2 = this->pOwner;
			edF32Vector4SubHard(&eStack32, &local_10, &pCVar2->currentLocation);
			local_30 = CDynamicExt::gForceGravity;
			pCVar2->field_0xfd0.BuildFromAccelDistAmplitude(eStack32.y + 3.0f, &local_30, &eStack32, 1);
			pSVar3 = pCVar2->GetStateCfg(pCVar2->actorState);
			iVar4 = pCVar2->GetIdMacroAnim(pSVar3->animId);
			if (iVar4 < 0) {
				fVar8 = 0.0f;
				fVar7 = (pCVar2->field_0xfd0).field_0x40;
			}
			else {
				fVar8 = pCVar2->pAnimationController->GetAnimLength(iVar4, 1);
				fVar7 = (pCVar2->field_0xfd0).field_0x40;
			}

			pCVar2->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar7 / fVar8, 0);
		}
		else {
			if (newState == 0xb7) {
				this->pOwner->dynamic.speed = 0.0f;
			}
		}
	}

	return;
}

void CBehaviourBrazulB::TermState(int oldState, int newState)
{
	int iVar2;
	int iVar3;
	CActorBrazul* pBrazul;

	if (oldState == 0xbf) {
		this->ntfSwitch.pTargetStreamRef->SwitchOff(this->pOwner);
	}
	else {
		if ((oldState == 0xba) || (oldState == 0xb9)) {
			this->pOwner->dynamic.speed = 0.0f;
		}
	}

	return;
}

int CBehaviourBrazulB::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	CActorBrazul* pBrazul;

	if (msg == 0xf) {
		pBrazul = this->pOwner;
		pBrazul->SetBehaviour((pBrazul->subObjA)->defaultBehaviourId, 0xffffffff, 0xffffffff);
	}

	return msg == 0xf;
}

void CBehaviourFighterBrazul::Term()
{
	CActorBrazul* pBrazul;

	pBrazul = static_cast<CActorBrazul*>(this->pOwner);
	CBehaviourFighter::Term();

	if (pBrazul != (CActorBrazul*)0x0) {
		pBrazul->FUN_003e62d0(0.0f);
		pBrazul->FUN_003e5f40(0.0f);
	}

	return;
}

void CBehaviourFighterBrazul::Manage()
{
	CActorBrazul* pBrazul;
	int iVar1;

	pBrazul = (CActorBrazul*)this->pOwner;

	iVar1 = pBrazul->actorState;
	if (iVar1 == 0xba) {
		pBrazul->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (pBrazul->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pBrazul->SetState(6, 0xffffffff);
		}
	}
	else {
		if (iVar1 == 0xb9) {
			pBrazul->FUN_003e87e0(0xba, 1);
		}
		else {
			if (iVar1 == 0xb8) {
				pBrazul->FUN_003e87e0(0xb9, 0);
			}
			else {
				if (iVar1 == 0xb7) {
					pBrazul->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

					if (pBrazul->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
						pBrazul->SetState(0xb8, 0xffffffff);
					}
				}
				else {
					CBehaviourFighterWolfen::Manage();
				}
			}
		}
	}

	return;
}

void CBehaviourFighterBrazul::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CBehaviourFighterWolfen::Begin(pOwner, newState, newAnimationType);
	this->behaviourId = this->pOwner->subObjA->defaultBehaviourId;

	return;
}

void CBehaviourFighterBrazul::End(int newBehaviourId)
{
	bool uVar1;
	CActorBrazul* pBrazul;

	pBrazul = (CActorBrazul*)this->pOwner;

	CBehaviourFighterWolfen::End(newBehaviourId);

	if ((pBrazul != (CActorBrazul*)0x0) && (uVar1 = pBrazul->IsFightRelated(newBehaviourId), uVar1 == false)) {
		pBrazul->FUN_003e62d0(0.0f);
		pBrazul->FUN_003e5f40(0.0f);
	}

	return;
}

void CBehaviourFighterBrazul::InitState(int newState)
{
	StateConfig* piVar1;
	int iVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;
	CActorBrazul* pBrazul;

	pBrazul = (CActorBrazul*)this->pOwner;
	if (newState == 0xb8) {
		edF32Vector4SubHard(&eStack16, pBrazul->field_0x1064, &pBrazul->currentLocation);
		local_20 = CDynamicExt::gForceGravity;
		pBrazul->field_0xfd0.BuildFromAccelDistAmplitude(eStack16.y + 3.0f, &local_20, &eStack16, 1);
		piVar1 = pBrazul->GetStateCfg(pBrazul->actorState);
		iVar1 = pBrazul->GetIdMacroAnim(piVar1->animId);
		if (iVar1 < 0) {
			fVar3 = 0.0f;
			fVar2 = (pBrazul->field_0xfd0).field_0x40;
		}
		else {
			fVar3 = pBrazul->pAnimationController->GetAnimLength(iVar1, 1);
			fVar2 = (pBrazul->field_0xfd0).field_0x40;
		}

		pBrazul->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar2 / fVar3, 0);
	}
	else {
		if (newState == 0xb7) {
			pBrazul->dynamic.speed = 0.0f;
		}
		else {
			CBehaviourFighterWolfen::InitState(newState);
		}
	}

	return;
}

void CBehaviourFighterBrazul::TermState(int oldState, int newState)
{
	if ((oldState == 0xba) || (oldState == 0xb9)) {
		this->pOwner->dynamic.speed = 0.0f;
	}
	else {
		CBehaviourFighterWolfen::TermState(oldState, newState);
	}

	return;
}

void CBehaviourFighterBrazul::_ManageHit(bool bPlayImpact)
{
	this->pOwner->hitFlags = this->pOwner->hitFlags & 0xfffffff7;
	CBehaviourFighter::_ManageHit(bPlayImpact);

	return;
}

void CBehaviourFighterBrazul::ManageExit()
{
	CActorBrazul* pBrazul;
	int iVar2;
	int peVar3;
	StateConfig* pSVar4;
	uint uVar5;

	CBehaviourFighterWolfen::ManageExit();
	pBrazul = (CActorBrazul*)this->pOwner;

	iVar2 = pBrazul->actorState;
	if (iVar2 == -1) {
		uVar5 = 0;
	}
	else {
		pSVar4 = pBrazul->GetStateCfg(iVar2);
		uVar5 = pSVar4->flags_0x4;
	}

	if ((uVar5 & 0x100000) != 0) {
		pBrazul = (CActorBrazul*)this->pOwner;
		if (((pBrazul->curBehaviourId == FIGHTER_BEHAVIOUR_DEFAULT) && (peVar3 = pBrazul->field_0xfc8, peVar3 != -1)) && (peVar3 != FIGHTER_BEHAVIOUR_DEFAULT)) {
			pBrazul->SetBehaviour(peVar3, -1, -1);
		}
	}

	return;
}
