#include "ScenaricCondition.h"
#include "MemoryStream.h"
#include "LevelScheduleManager.h"

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

struct S_STREAM_SIMPLE_COND {
	int field_0x0;
	SCENARIC_VARIABLE variable;
	int field_0x8;
	int field_0xc;
};

ulong ScenaricCondition::IsVerified(S_STREAM_SIMPLE_COND* pCond, ulong param_2)
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
