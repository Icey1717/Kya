#include "SwitchBehaviour.h"

#include "MemoryStream.h"
#include "edMem.h"
#include "Actor.h"
#include "EventManager.h"
#include "ActorWolfen.h"

CSwitchBehaviour::CSwitchBehaviour()
{
	this->nbTotalSwitches = 0;
	this->apSwitches = (CCoupleConditionExecution**)0x0;

	return;
}

void CSwitchBehaviour::Create(ByteCode* pByteCode)
{
	int iVar1;
	CCondition* pCVar2;
	CExecute* pCVar3;
	TCouple_Condition_Execution<CondSeeActorInArea, CExecuteSwitchBehaviour>* pvVar6;
	TCouple_Condition_Execution<CondActorInArea, CExecuteSwitchBehaviour>* pvVar5;
	int* piVar5;
	TCouple_Condition_Execution<CondSeeActor, CExecuteSwitchBehaviour>* pvVar4;
	void* pvVar7;
	undefined8 uVar8;
	CCoupleConditionExecution*** ppCVar9;
	int iVar11;
	CCoupleConditionExecution** local_10[4];
	TCouple_Condition_Execution<CondMessage, CExecuteSwitchBehaviour>* piVar2;
	TCouple_Condition_Execution<CondSeeActorInArea, CExecuteSwitchBehaviour>* piVar4;

	local_10[3] = (CCoupleConditionExecution**)0x0;
	local_10[2] = (CCoupleConditionExecution**)0x0;
	ppCVar9 = local_10;
	local_10[1] = (CCoupleConditionExecution**)0x0;
	local_10[0] = (CCoupleConditionExecution**)0x0;
	pByteCode->GetS32();
	this->nbCondMsgSwitches = pByteCode->GetS32();
	if (0 < this->nbCondMsgSwitches) {
		edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
		local_10[0] = new CCoupleConditionExecution* [this->nbCondMsgSwitches];
		edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
		iVar1 = 0;
		if (0 < this->nbCondMsgSwitches) {
			do {
				local_10[0][iVar1] = new TCouple_Condition_Execution<CondMessage, CExecuteSwitchBehaviour>();
				local_10[0][iVar1]->GetCondition()->Create(pByteCode);
				local_10[0][iVar1]->GetExecution()->Create(pByteCode);
				iVar1 = iVar1 + 1;
			} while (iVar1 < this->nbCondMsgSwitches);
		}
	}

	pByteCode->GetS32();
	this->nbCondSeeInAreaSwitches = pByteCode->GetS32();
	if (0 < this->nbCondSeeInAreaSwitches) {
		edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
		local_10[1] = new CCoupleConditionExecution* [this->nbCondSeeInAreaSwitches];
		edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
		iVar1 = 0;
		if (0 < this->nbCondSeeInAreaSwitches) {
			do {
				local_10[1][iVar1] = new TCouple_Condition_Execution<CondSeeActorInArea, CExecuteSwitchBehaviour>();
				local_10[1][iVar1]->GetCondition()->Create(pByteCode);
				local_10[1][iVar1]->GetExecution()->Create(pByteCode);
				iVar1 = iVar1 + 1;
			} while (iVar1 < this->nbCondSeeInAreaSwitches);
		}
	}

	pByteCode->GetS32();
	this->nbCondActorInAreaSwitches = pByteCode->GetS32();
	if (0 < this->nbCondActorInAreaSwitches) {
		edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
		local_10[2] = new CCoupleConditionExecution* [this->nbCondActorInAreaSwitches];
		edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
		iVar1 = 0;
		if (0 < this->nbCondActorInAreaSwitches) {
			do {
				local_10[2][iVar1] = new TCouple_Condition_Execution<CondActorInArea, CExecuteSwitchBehaviour>();
				local_10[2][iVar1]->GetCondition()->Create(pByteCode);
				local_10[2][iVar1]->GetExecution()->Create(pByteCode);
				iVar1 = iVar1 + 1;
			} while (iVar1 < this->nbCondActorInAreaSwitches);
		}
	}

	pByteCode->GetS32();
	this->nbCondSeeActorSwitches = pByteCode->GetS32();
	if (0 < this->nbCondSeeActorSwitches) {
		edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
		local_10[3] = new CCoupleConditionExecution* [this->nbCondSeeActorSwitches];
		edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
		if (0 < this->nbCondSeeActorSwitches) {
			iVar1 = 0;
			do {
				local_10[3][iVar1] = new TCouple_Condition_Execution<CondSeeActor, CExecuteSwitchBehaviour>();
				local_10[3][iVar1]->GetCondition()->Create(pByteCode);
				local_10[3][iVar1]->GetExecution()->Create(pByteCode);
				iVar1 = iVar1 + 1;
			} while (iVar1 < this->nbCondSeeActorSwitches);
		}
	}

	this->nbTotalSwitches = this->nbCondMsgSwitches + this->nbCondSeeInAreaSwitches + this->nbCondSeeActorSwitches + this->nbCondActorInAreaSwitches;
	if (0 < this->nbTotalSwitches) {
		this->apSwitches = new CCoupleConditionExecution* [this->nbTotalSwitches];
		memcpy(this->apSwitches, local_10[0], this->nbCondMsgSwitches * sizeof(CCoupleConditionExecution*));
		memcpy(this->apSwitches + this->nbCondMsgSwitches, local_10[1], this->nbCondSeeInAreaSwitches * sizeof(CCoupleConditionExecution*));
		memcpy(this->apSwitches + this->nbCondMsgSwitches + this->nbCondSeeInAreaSwitches, local_10[2], this->nbCondActorInAreaSwitches * sizeof(CCoupleConditionExecution*));
		memcpy(this->apSwitches + this->nbCondMsgSwitches + this->nbCondSeeInAreaSwitches + this->nbCondActorInAreaSwitches, local_10[3], this->nbCondSeeActorSwitches * sizeof(CCoupleConditionExecution*));
	}

	iVar1 = 0;
	do {
		if (local_10[iVar1] != (CCoupleConditionExecution**)0x0) {
			delete[] local_10[iVar1];
		}

		iVar1 = iVar1 + 1;
	} while (iVar1 < 4);

	return;
}

void CSwitchBehaviour::Init(CActor* pOwner)
{
	for (int i = 0; i < this->nbTotalSwitches; i++) {
		this->apSwitches[i]->GetCondition()->Init(pOwner);
		this->apSwitches[i]->GetExecution()->Init(pOwner);
	}

	return;
}

void CSwitchBehaviour::Begin(CActor* pOwner)
{
	for (int i = 0; i < this->nbTotalSwitches; i++) {
		this->apSwitches[i]->flags = this->apSwitches[i]->flags | 1;
		this->apSwitches[i]->GetCondition()->Begin(pOwner);
	}

	return;
}

int CSwitchBehaviour::InterpretMessage(CActor* pOwner, CActor* pSender, int msg, void* pMsgParam)
{
	int result = 0;

	for (int i = 0; i < this->nbTotalSwitches; i++) {
		result = this->apSwitches[i]->GetCondition()->InterpretMessage(pOwner, pSender, msg, pMsgParam);
	}

	return result;
}

int CSwitchBehaviour::Test(CActor* pOwner)
{
	bool bVar1;
	CCondition* pCVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	CCoupleConditionExecution* piVar1;

	for (int i = 0; i < this->nbTotalSwitches; i++) {
		CCoupleConditionExecution* pSwitch = this->apSwitches[i];

		bVar1 = false;

		if ((pSwitch->flags & 1) != 0) {
			pSwitch->GetCondition()->Manage(pOwner);
			if (pSwitch->GetCondition()->field_0x4 != 0) {
				bVar1 = true;
			}

			if (bVar1) {
				iVar5 = 1;
			}
		}
	}

	return iVar5;
}

int CSwitchBehaviour::Execute(CActor* pOwner)
{
	CCondition* pCondition;
	int i;
	int result;
	CCoupleConditionExecution* pCouple;

	result = -1;
	for (i = 0; (result == -1 && (i < this->nbTotalSwitches)); i = i + 1) {
		pCouple = this->apSwitches[i];
		result = -1;

		if (((pCouple->flags & 1) != 0) && (pCondition = pCouple->GetCondition(), pCondition->field_0x4 != 0)) {
			result = pCouple->GetExecution()->Execute(pOwner);
		}
	}

	return result;
}

void CondMessage::Create(ByteCode* pByteCode)
{
	int iVar1;

	iVar1 = pByteCode->GetS32();
	if (iVar1 == 2) {
		this->field_0x8 = 0x10;
	}
	else {
		if (iVar1 == 1) {
			this->field_0x8 = 0xf;
		}
		else {
			if (iVar1 == 0) {
				this->field_0x8 = 0xe;
			}
		}
	}
	return;
}

void CondMessage::Init(CActor* pOwner)
{
	return;
}

int CondMessage::InterpretMessage(CActor* pOwner, CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;

	bVar1 = pMsgParam == (void*)pOwner->flags;
	if (bVar1) {
		IMPLEMENTATION_GUARD(
		*(undefined*)&pOwner->objectId = 1;)
	}

	return bVar1;
}

void CExecuteSwitchBehaviour::Create(ByteCode* pByteCode)
{
	this->behaviourId = pByteCode->GetS32();

	return;
}

void CExecuteSwitchBehaviour::Init(CActor* pOwner)
{
	// I think this is just to throw an error if there's no behaviour with the given ID
	pOwner->GetBehaviour(this->behaviourId);

	return;
}

int CExecuteSwitchBehaviour::Execute(CActor* pOwner)
{
	return this->behaviourId;
}

bool CheckZone(ed_zone_3d* pZone, uint flags, uint* pResult, edF32VECTOR4* pPosition)
{
	uint computeResult;
	bool bInZone;

	bInZone = false;

	computeResult = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone, pPosition, 0);
	if (computeResult == *pResult) {
		if (computeResult == 1) {
			if ((flags & 4) != 0) {
				bInZone = true;
			}
		}
		else {
			if ((flags & 8) != 0) {
				bInZone = true;
			}
		}
	}
	else {
		bInZone = false;
		if (computeResult == 1) {
			if ((flags & 1) != 0) {
				bInZone = true;
			}
		}
		else {
			if ((flags & 2) != 0) {
				bInZone = true;
			}
		}
		*pResult = computeResult;
	}

	return bInZone;
}

void CondActorInArea::Create(ByteCode* pByteCode)
{
	this->pActor.index = pByteCode->GetS32();
	this->pZone.index = pByteCode->GetS32();
	this->flags = pByteCode->GetU32();

	return;
}

void CondActorInArea::Init(CActor* pOwner)
{
	this->pActor.Init();
	this->pZone.Init();

	return;
}

void CondActorInArea::Begin(CActor* pOwner)
{
	this->field_0x4 = 0;
	this->zoneResult = 2;

	return;
}

void CondActorInArea::Manage(CActor* pOwner)
{
	if (CheckZone(this->pZone.Get(), this->flags, &this->zoneResult, &this->pActor.Get()->currentLocation) != false) {
		this->field_0x4 = 1;
	}

	return;
}

void CondSeeActorInArea::Create(ByteCode* pByteCode)
{
	this->pActor.index = pByteCode->GetS32();
	this->pZone.index = pByteCode->GetS32();
	this->flags = pByteCode->GetU32();
}

void CondSeeActorInArea::Init(CActor* pOwner)
{
	this->pActor.Init();
	this->pZone.Init();

	return;
}

void CondSeeActorInArea::Begin(CActor* pOwner)
{
	this->field_0x4 = 0;
	this->zoneResult = 2;

	return;
}

void CondSeeActorInArea::Manage(CActor* pOwner)
{
	CActor* pCVar1;
	bool bVar2;
	CVision* pVision;
	CActor* pCVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	CActorWolfen* pWolfen = static_cast<CActorWolfen*>(pOwner);

	pCVar1 = this->pActor.Get();
	if ((pCVar1 == pWolfen->pTargetActor_0xc80) && (CheckZone(this->pZone.Get(), this->flags, &this->zoneResult, &this->pActor.Get()->currentLocation) != false)) {
		pVision = pWolfen->GetVision();
		pVision->location = pWolfen->currentLocation;
		pVision->location.y = pWolfen->currentLocation.y + pWolfen->field_0xcf0;
		pVision->rotationQuat = pWolfen->rotationQuat;
		pCVar3 = pVision->ScanForTarget(this->pActor.Get(), 1);
		if (pCVar3 != (CActor*)0x0) {
			this->field_0x4 = 1;
		}
	}
	return;
}
