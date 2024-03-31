#include "ScenaricCondition.h"
#include "MemoryStream.h"
#include "LevelScheduleManager.h"
#include <assert.h>

void ScenaricCondition::Create(char* param_2)
{
	this->field_0x0 = param_2;
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
	this->field_0x0 = (char*)piVar1;
	if (*(int*)this->field_0x0 != 0) {
		*(int*)((ulong)this->field_0x0 + 4) = 0;
	}
	return;
}

ulong ScenaricCondition::IsVerified(S_STREAM_SIMPLE_COND* pCond, int param_2)
{
	int iVar1;
	int iVar2;
	ulong uVar3;

	iVar2 = LevelScheduleManager::ScenVar_Get(pCond->variable);
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
	pCond = (S_STREAM_SIMPLE_COND*)((ulong)this->field_0x0 + 4);
	for (iVar2 = *(int*)this->field_0x0; 0 < iVar2; iVar2 = iVar2 + -1) {
		uVar1 = IsVerified(pCond, uVar1);
		pCond = pCond + 1;
	}
	return uVar1;
}

int ScenaricCondition::GetNumSimpleConds()
{
	int iVar1;

	if ((int*)this->field_0x0 == (int*)0x0) {
		iVar1 = 0;
	}
	else {
		iVar1 = *(int*)this->field_0x0;
	}
	return iVar1;
}

int ScenaricCondition::GetDataSize()
{
	return *(int*)this->field_0x0 * 0x10 + 4;
}

uint* ScenaricCondition::GetEndPtr()
{
	return (uint*)((int*)((ulong)this->field_0x0 + *(int*)this->field_0x0 * 4 * 4) + 1);
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

void S_STREAM_SIMPLE_OPERATION::Perform()
{
	int scenVarValue;
	int type;

	scenVarValue = LevelScheduleManager::ScenVar_Get(this->scenVarId);
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
	LevelScheduleManager::ScenVar_Set(this->scenVarId, scenVarValue);
	LevelScheduleManager::OnSceneVarSet();
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

				S_STREAM_SIMPLE_OPERATION* pOp = (S_STREAM_SIMPLE_OPERATION*)(pOpHeader + 1);

				COND_HEADER* pCondHeader = (COND_HEADER*)(pOp + pOpHeader->opCount);
				int condCount = pCondHeader->condCount;
				pCond = (S_STREAM_SIMPLE_COND*)(pCondHeader + 1);

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
				pOpHeader = (OP_HEADER*)(pOp + pOpHeader->opCount);
			} while (uVar3 < this->pHeader->cndOpCount);
		}
	}
	return;
}