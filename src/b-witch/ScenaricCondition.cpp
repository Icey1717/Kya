#include "ScenaricCondition.h"
#include "MemoryStream.h"
#include "LevelScheduler.h"
#include <assert.h>

void ScenaricCondition::Create(int* param_2)
{
	this->pData = param_2;
	return;
}

void ScenaricCondition::Create(ByteCode* pByteCode)
{
	int* piVar1;

	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x10;
	}
	this->pData = piVar1;

	if (*this->pData != 0) {
		this->pData[1] = 0;
	}

	return;
}

ulong ScenaricCondition::IsVerified(S_STREAM_SIMPLE_COND* pCond, int param_2)
{
	int iVar1;
	int iVar2;
	ulong uVar3;

	iVar2 = CLevelScheduler::ScenVar_Get(pCond->variable);
	iVar1 = pCond->field_0xc;
	switch (pCond->field_0x8) {
	case 0:
		uVar3 = 1;
		break;
	case 1:
		uVar3 = (ulong)(iVar2 < iVar1);
		break;
	case 2:
		uVar3 = (ulong)(iVar1 < iVar2) ^ 1;
		break;
	case 3:
		uVar3 = (ulong)(iVar2 == iVar1);
		break;
	case 4:
		uVar3 = (ulong)(iVar2 < iVar1) ^ 1;
		break;
	case 5:
		uVar3 = (ulong)(iVar1 < iVar2);
		break;
	case 6:
		uVar3 = (ulong)(iVar2 != iVar1);
		break;
	default:
		uVar3 = 1;
	}
	iVar1 = pCond->field_0x0;
	if (iVar1 == 2) {
		uVar3 = uVar3 ^ param_2;
	}
	else {
		if (iVar1 == 1) {
			uVar3 = uVar3 | param_2;
		}
		else {
			if (iVar1 == 0) {
				uVar3 = uVar3 & param_2;
			}
		}
	}
	return uVar3;
}

ulong ScenaricCondition::IsVerified()
{
	ulong uVar1;
	S_STREAM_SIMPLE_COND* pCond;
	int iVar2;

	uVar1 = 1;
	pCond = reinterpret_cast<S_STREAM_SIMPLE_COND*>(this->pData + 1);
	for (iVar2 = *this->pData; 0 < iVar2; iVar2 = iVar2 + -1) {
		uVar1 = IsVerified(pCond, uVar1);
		pCond = pCond + 1;
	}
	return uVar1;
}

int ScenaricCondition::GetNumSimpleConds()
{
	int iVar1;

	if (this->pData == (int*)0x0) {
		iVar1 = 0;
	}
	else {
		iVar1 = *this->pData;
	}

	return iVar1;
}

int ScenaricCondition::GetDataSize()
{
	return *this->pData * 0x10 + 4;
}

int* ScenaricCondition::GetBeginPtr()
{
	return this->pData;
}

int* ScenaricCondition::GetEndPtr()
{
	return this->pData + *this->pData * 4 + 1;
}

void ScenaricCondition::Invalidate()
{
	pData = (int*)0x0;
	return;
}

void ConditionedOperationArray::Create(ByteCode* pByteCode)
{
	CND_OP_HEADER* puVar1;
	int iVar3;
	uint uVar4;

	puVar1 = (CND_OP_HEADER*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(puVar1 + 1);
	this->pHeader = puVar1;

	uVar4 = 0;
	if (this->pHeader->cndOpCount != 0) {
		do {
			OP_HEADER* pOpHeader = (OP_HEADER*)pByteCode->currentSeekPos;
			pByteCode->currentSeekPos = (char*)(pOpHeader + 1);
			iVar3 = pOpHeader->opCount;
			if (iVar3 != 0) {
				pByteCode->currentSeekPos = pByteCode->currentSeekPos + iVar3 * sizeof(S_STREAM_SIMPLE_OPERATION);
			}

			COND_HEADER* pCondHeader = (COND_HEADER*)pByteCode->currentSeekPos;
			pByteCode->currentSeekPos = (char*)(pCondHeader + 1);
			iVar3 = pCondHeader->condCount;
			if (iVar3 != 0) {
				pByteCode->currentSeekPos = pByteCode->currentSeekPos + iVar3 * sizeof(S_STREAM_SIMPLE_COND);
			}

			uVar4 = uVar4 + 1;
		} while (uVar4 < this->pHeader->cndOpCount);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorServices.cpp
void S_STREAM_SIMPLE_OPERATION::Perform()
{
	int scenVarValue;
	int type;

	scenVarValue = CLevelScheduler::ScenVar_Get(this->scenVarId);
	type = this->opType;
	if (type == 4) {
		scenVarValue = scenVarValue / this->value;
		if (this->value == 0) {
			trap(7);
		}
	}
	else {
		if (type == 3) {
			scenVarValue = scenVarValue * this->value;
		}
		else {
			if (type == 2) {
				scenVarValue = scenVarValue - this->value;
			}
			else {
				if (type == 1) {
					scenVarValue = scenVarValue + this->value;
				}
				else {
					if (type == 0) {
						scenVarValue = this->value;
					}
				}
			}
		}
	}
	CLevelScheduler::ScenVar_Set(this->scenVarId, scenVarValue);
	CLevelScheduler::OnSceneVarSet();
	return;
}

void ConditionedOperationArray::Perform()
{
	int iVar1;
	COND_HEADER* pOVar2;
	ulong uVar2;
	uint uVar3;
	S_STREAM_SIMPLE_COND* pCond;
	COND_HEADER* pCVar5;
	int iVar6;
	CND_OP_HEADER* pCndOpHeader;

	pCndOpHeader = this->pHeader;
	if (pCndOpHeader != (CND_OP_HEADER*)0x0) {
		uVar3 = 0;

		OP_HEADER* pOpHeader = (OP_HEADER*)(pCndOpHeader + 1);
		if (pCndOpHeader->cndOpCount != 0) {
			do {
				uVar2 = 1;

				S_STREAM_SIMPLE_OPERATION* pOp = pOpHeader->aOps;

				COND_HEADER* pCondHeader = (COND_HEADER*)(pOp + pOpHeader->opCount);
				int condCount = pCondHeader->condCount;
				pCond = pCondHeader->aConds;

				for (; 0 < condCount; condCount = condCount + -1) {
					uVar2 = ScenaricCondition::IsVerified(pCond, uVar2);
					pCond = pCond + 1;
				}

				int currentOpIndex;

				if ((uVar2 != 0) && (currentOpIndex = 0, 0 < pOpHeader->opCount)) {
					do {
						pOp->Perform();
						currentOpIndex = currentOpIndex + 1;
						pOp += 1;
					} while (currentOpIndex < pOpHeader->opCount);
				}

				uVar3 = uVar3 + 1;
				pOpHeader = (OP_HEADER*)(pOpHeader->aOps + pOpHeader->opCount);
			} while (uVar3 < this->pHeader->cndOpCount);
		}
	}
	return;
}

void ConditionedOperationArray::Invalidate()
{
	this->pHeader = (CND_OP_HEADER*)0x0;
	return;
}