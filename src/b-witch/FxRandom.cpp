#include "FxRandom.h"
#include "MemoryStream.h"
#include "MathOps.h"

CFxRandomScenaricData::CFxRandomScenaricData()
{
	this->nbData = 0;

	return;
}

void CFxRandomScenaricData::Init()
{
	return;
}

void CFxRandomScenaricData::Create(ByteCode* pByteCode)
{
	this->nbData = pByteCode->GetU32();
	this->field_0x44 = 0.0f;

	if (this->nbData != 0) {
		for (uint i = 0; i < this->nbData; i++) {
			this->aSubObjs[i].field_0x0 = pByteCode->GetU32();
			this->aSubObjs[i].field_0x4 = pByteCode->GetF32();
		}
	}

	return;
}

void CFxRandomScenaricData::Term()
{
	return;
}

bool CFxRandomScenaricData::IsLooped()
{
	bool bVar1;
	CFxRandomScenaricDataSubObj* pCVar2;
	uint uVar3;
	CFxManager* pFxManager;

	pFxManager = CScene::ptable.g_EffectsManager_004516b8;
	if ((this->nbData != 0) && (uVar3 = 0, pCVar2 = this->aSubObjs, this->nbData != 0)) {
		do {
			bVar1 = pFxManager->IsLooped(pCVar2->field_0x0);
			if (bVar1 != false) {
				return true;
			}

			uVar3 = uVar3 + 1;
			pCVar2 = pCVar2 + 1;
		} while (uVar3 < this->nbData);
	}

	return false;
}

CRandomFx::CRandomFx()
	: CNewFx()
{
	this->randFxHandle.id = 0;
	this->randFxHandle.pFx = (CNewFx*)0x0;

	return;
}

void CRandomFx::Draw()
{
	return;
}

void CRandomFx::Kill()
{
	int iVar1;
	bool bVar2;
	CNewFx* pRandFx;

	pRandFx = (this->randFxHandle).pFx;

	if (((pRandFx == (CNewFx*)0x0) || (iVar1 = (this->randFxHandle).id, iVar1 == 0)) || (bVar2 = true, iVar1 != pRandFx->id)) {
		bVar2 = false;
	}

	if (bVar2) {
		if (((pRandFx != (CNewFx*)0x0) && (iVar1 = (this->randFxHandle).id, iVar1 != 0)) && (iVar1 == pRandFx->id)) {
			pRandFx->Kill();
		}

		(this->randFxHandle).pFx = (CNewFx*)0x0;
		(this->randFxHandle).id = 0;
	}

	CNewFx::Kill();

	return;
}

void CRandomFx::Start(float param_1, float param_2)
{
	CNewFx* pCVar1;
	CFxRandomScenaricData* pCVar2;
	bool bVar3;
	byte bVar4;
	int iVar5;
	CFxRandomScenaricData::CFxRandomScenaricDataSubObj* pCVar6;
	uint uVar7;
	uint uVar8;
	float fVar9;
	CFxManager* pFxManager;

	this->flags = this->flags | 3;
	this->flags = this->flags & 0xfffffff3;
	if (0.0f < param_1) {
		this->flags = this->flags | 0x40;
	}

	this->field_0xc = param_1;
	this->field_0x10 = param_2;


	if (this->randFxHandle.IsValid()) {
		this->randFxHandle.Kill();
		this->randFxHandle.Reset();
	}

	pFxManager = CScene::ptable.g_EffectsManager_004516b8;
	pCVar2 = this->pScenariacData;
	fVar9 = pCVar2->field_0x44;
	iVar5 = rand();
	uVar7 = pCVar2->nbData;
	uVar8 = 0;
	fVar9 = fVar9 * (static_cast<float>(iVar5) / 2.147484e+09f);
	pCVar6 = pCVar2->aSubObjs;
	if (uVar7 != 1) {
		do {
			fVar9 = fVar9 - pCVar6->field_0x4;
			if (fVar9 < 0.0f) {
				uVar7 = pCVar2->aSubObjs[uVar8].field_0x0;
				goto LAB_0036ae68;
			}
			uVar8 = uVar8 + 1;
			pCVar6 = pCVar6 + 1;
		} while (uVar8 < uVar7 - 1);
	}

	if (uVar7 == 0) {
		uVar7 = 0xffffffff;
	}
	else {
		uVar7 = pCVar2->aSubObjs[uVar7 - 1].field_0x0;
	}

LAB_0036ae68:
	if (-1 < static_cast<int>(uVar7)) {
		pFxManager->GetDynamicFx(&this->randFxHandle, uVar7, this->selector);
		pCVar1 = (this->randFxHandle).pFx;

		if (this->randFxHandle.IsValid()) {
			if (((pCVar1 != (CNewFx*)0x0) && (iVar5 = (this->randFxHandle).id, iVar5 != 0)) && (iVar5 == pCVar1->id)) {
				pCVar1->pSon = this;
				pCVar1->field_0x8 = 0;
			}

			this->field_0x88 = this->randFxHandle.GetType();
		}
	}

	if (this->randFxHandle.IsValid()) {
		if (this->spatializeActor != (CActor*)0x0) {
			this->randFxHandle.SpatializeOnActor(this->spatializeFlags, (CActor*)this->spatializeActor, this->spatializeBone);
		}

		this->randFxHandle.Start();
		this->flags = this->flags & 0xfffffffd;
	}
	else {
		Kill();
	}

	return;
}

void CRandomFx::Stop(float param_1)
{
	int iVar1;
	bool bVar2;
	CNewFx* pRandFx;

	CNewFx::Stop(param_1);

	pRandFx = (this->randFxHandle).pFx;
	if (((pRandFx == (CNewFx*)0x0) || (iVar1 = (this->randFxHandle).id, iVar1 == 0)) || (bVar2 = true, iVar1 != pRandFx->id)) {
		bVar2 = false;
	}

	if (((bVar2) && (pRandFx != (CNewFx*)0x0)) && ((iVar1 = (this->randFxHandle).id, iVar1 != 0 && (iVar1 == pRandFx->id)))) {
		pRandFx->Stop(param_1);
	}

	return;
}

void CRandomFx::Func_0x28()
{
	IMPLEMENTATION_GUARD(
	return this->field_0x88;)
}

int CRandomFx::GetType()
{
	return FX_TYPE_RANDOM;
}

void CRandomFx::NotifySonIsDead(CNewFx* pSon, int param_2)
{
	CNewFx::NotifySonIsDead(pSon, param_2);

	(this->randFxHandle).id = 0;
	(this->randFxHandle).pFx = (CNewFx*)0x0;

	Kill();

	return;
}

void CRandomFx::SpatializeOnActor(uint flags, CActor* pActor, uint boneId)
{
	CNewFx::SpatializeOnActor(flags, pActor, boneId);

	this->spatializeFlags = flags | 0x40;
	this->spatializeActor = pActor;
	this->spatializeBone = boneId;

	return;
}

void CRandomFx::UpdateSpatializeActor(uint newFlags, edF32VECTOR4 *pNewPosition)
{
	CNewFx::UpdateSpatializeActor(newFlags, pNewPosition);

	this->spatializeFlags = newFlags | 0x40;
	this->spatializeActor = pNewPosition;

	return;
}

void CRandomFx::Manage()
{
	uint uVar1;
	int iVar2;
	bool bVar3;
	CNewFx* pFx;

	CNewFx::Manage();

	uVar1 = this->flags;

	if ((uVar1 & 0x20) == 0) {
		bVar3 = (uVar1 & 1) != 0;
		if (bVar3) {
			bVar3 = (uVar1 & 4) == 0;
		}
		if (bVar3) {
			pFx = (this->randFxHandle).pFx;
			if (((pFx == (CNewFx*)0x0) || (iVar2 = (this->randFxHandle).id, iVar2 == 0)) || (bVar3 = true, iVar2 != pFx->id)) {
				bVar3 = false;
			}

			if (bVar3) {
				if (((pFx != (CNewFx*)0x0) && (iVar2 = (this->randFxHandle).id, iVar2 != 0)) && (iVar2 == pFx->id)) {
					pFx->position = this->position;
				}

				pFx = (this->randFxHandle).pFx;
				if (((pFx != (CNewFx*)0x0) && (iVar2 = (this->randFxHandle).id, iVar2 != 0)) && (iVar2 == pFx->id)) {
					pFx->rotationEuler = this->rotationEuler;
				}

				pFx = (this->randFxHandle).pFx;
				if (((pFx != (CNewFx*)0x0) && (iVar2 = (this->randFxHandle).id, iVar2 != 0)) &&
					(iVar2 == pFx->id)) {
					pFx->scale = this->scale;
				}
			}
			else {
				Kill();
			}
		}
	}
	else {
		Kill();
	}

	return;
}

void* CFxRandomManager::InstanciateFx(uint scenaricDataIndex, FX_MATERIAL_SELECTOR selector)
{
	CFxRandomScenaricData* pCVar1;
	CRandomFx* pNewRandomFx;

	pNewRandomFx = _InstanciateFx();

	if ((pNewRandomFx != (CRandomFx*)0x0) && (scenaricDataIndex != 0xffffffff)) {
		pCVar1 = this->aScenaricData;
		pNewRandomFx->position = gF32Vertex4Zero;
		pNewRandomFx->rotationEuler = gF32Vector4Zero;
		pNewRandomFx->scale.z = 1.0f;
		pNewRandomFx->scale.y = 1.0f;
		pNewRandomFx->scale.x = 1.0f;
		pNewRandomFx->scale.w = 0.0f;
		pNewRandomFx->pSon = (CNewFx*)0x0;
		pNewRandomFx->pScenariacData = pCVar1 + scenaricDataIndex;
		pNewRandomFx->selector = selector;
		pNewRandomFx->randFxHandle.id = 0;
		pNewRandomFx->randFxHandle.pFx = (CNewFx*)0x0;
		pNewRandomFx->field_0x88 = 0;
		pNewRandomFx->spatializeActor = (CActor*)0x0;
		pNewRandomFx->spatializeFlags = 0;
		pNewRandomFx->spatializeBone = 0;
	}

	return pNewRandomFx;
}

bool CFxRandomManager::IsFxLooped(uint index)
{
	return this->aScenaricData[index].IsLooped();
}
