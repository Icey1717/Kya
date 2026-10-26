#include "ScenaricCondition.h"
#include "MemoryStream.h"
#include "LevelScheduler.h"
#include <assert.h>

void CScenaricCondition::Create(int* param_2)
{
	// PS2 0x001174b0: borrow the existing condition data without normalizing its first combiner.
	this->pData = param_2;
	return;
}

void CScenaricCondition::Create(ByteCode* pByteCode)
{
	int* piVar1;

	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x10;
	}
	this->pData = piVar1;

	if (*this->pData != 0) {
		// PS2 0x001174c0: make the first test AND the initial true accumulator.
		this->pData[1] = SCENARIC_COMBINE_AND;
	}

	return;
}

ulong CScenaricCondition::IsVerified(S_STREAM_SIMPLE_COND* pCond, int param_2)
{
	int iVar1;
	int iVar2;
	ulong uVar3;

	iVar2 = CLevelScheduler::ScenVar_Get(pCond->variable);
	// PS2 0x001175e0 (comparison jump table 0x004288d0): signed comparisons;
	// the variable is a global scenario-table index, not an actor-local value.
	iVar1 = pCond->field_0xc;
	switch (pCond->field_0x8) {
	case SCENARIC_COMPARE_ALWAYS:
		uVar3 = 1;
		break;
	case SCENARIC_COMPARE_LESS:
		uVar3 = (ulong)(iVar2 < iVar1);
		break;
	case SCENARIC_COMPARE_LESS_EQUAL:
		uVar3 = (ulong)(iVar1 < iVar2) ^ 1;
		break;
	case SCENARIC_COMPARE_EQUAL:
		uVar3 = (ulong)(iVar2 == iVar1);
		break;
	case SCENARIC_COMPARE_GREATER_EQUAL:
		uVar3 = (ulong)(iVar2 < iVar1) ^ 1;
		break;
	case SCENARIC_COMPARE_GREATER:
		uVar3 = (ulong)(iVar1 < iVar2);
		break;
	case SCENARIC_COMPARE_NOT_EQUAL:
		uVar3 = (ulong)(iVar2 != iVar1);
		break;
	default:
		uVar3 = 1;
	}
	iVar1 = pCond->field_0x0;
	if (iVar1 == SCENARIC_COMBINE_XOR) {
		uVar3 = uVar3 ^ param_2;
	}
	else {
		if (iVar1 == SCENARIC_COMBINE_OR) {
			uVar3 = uVar3 | param_2;
		}
		else {
			if (iVar1 == SCENARIC_COMBINE_AND) {
				uVar3 = uVar3 & param_2;
			}
		}
	}
	return uVar3;
}

ulong CScenaricCondition::IsVerified()
{
	ulong uVar1;
	S_STREAM_SIMPLE_COND* pCond;
	int iVar2;

	// PS2 0x00117440: fold in stream order from true, without short-circuiting
	// or AND/OR precedence. XOR toggles the accumulated truth value on a true test.
	// An empty list passes; pData must still point to a count word.
	uVar1 = 1;
	pCond = reinterpret_cast<S_STREAM_SIMPLE_COND*>(this->pData + 1);
	for (iVar2 = *this->pData; 0 < iVar2; iVar2 = iVar2 + -1) {
		uVar1 = IsVerified(pCond, uVar1);
		pCond = pCond + 1;
	}
	return uVar1;
}

int CScenaricCondition::GetNumSimpleConds()
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

int CScenaricCondition::GetDataSize()
{
	return *this->pData * 0x10 + 4;
}

int* CScenaricCondition::GetBeginPtr()
{
	return this->pData;
}

int* CScenaricCondition::GetEndPtr()
{
	return this->pData + *this->pData * 4 + 1;
}

void CScenaricCondition::Invalidate()
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
	// PS2 0x00117330: each block stores operations before its conditions.
	// Unlike CScenaricCondition::Create(ByteCode*), this preserves every combiner.
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

#ifdef PLATFORM_WIN
			// Verify
			for (int i = 0; i < pOpHeader->opCount; i++) {
				auto* pOp = pOpHeader->aOps + i;
				if (pOp->opType == SCENARIC_OP_DIVIDE) {
					assert(pOp->value != 0);
				}
			}
#endif

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
	if (type == SCENARIC_OP_DIVIDE) {
		scenVarValue = scenVarValue / this->value;
		if (this->value == 0) {
			trap(7);
		}
	}
	else {
		if (type == SCENARIC_OP_MULTIPLY) {
			scenVarValue = scenVarValue * this->value;
		}
		else {
			if (type == SCENARIC_OP_SUBTRACT) {
				scenVarValue = scenVarValue - this->value;
			}
			else {
				if (type == SCENARIC_OP_ADD) {
					scenVarValue = scenVarValue + this->value;
				}
				else {
					if (type == SCENARIC_OP_ASSIGN) {
						scenVarValue = this->value;
					}
				}
			}
		}
	}
	CLevelScheduler::ScenVar_Set(this->scenVarId, scenVarValue);
	// PS2 0x00117510 -> 0x002db5b0: notify after each write, even if unchanged;
	// the hero/Boomy receive ability updates, rather than one notification per array.
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
	// PS2 0x00117210: run every passing block, in order, not just the first match.
	// Each block folds its conditions from true (empty = unconditional), then runs
	// its operations in order. Later blocks see earlier writes to scenario variables.
	// A leading OR yields true; a leading XOR negates its comparison.
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
					uVar2 = CScenaricCondition::IsVerified(pCond, uVar2);
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
				pOpHeader = (OP_HEADER*)(pCondHeader->aConds + pCondHeader->condCount);
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
