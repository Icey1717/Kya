#include "ActorShockerCmd.h"
#include "MemoryStream.h"

void CActorShockerCmd::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	int* piVar4;
	CSequenceParam* pCVar5;
	int iVar6;
	float* pfVar7;
	CSequenceParam* piVar3;
	float fVar10;

	CActor::Create(pByteCode);
	uVar1 = pByteCode->GetU32();
	//this->field_0x168 = uVar1;
	iVar2 = pByteCode->GetS32();
	this->sequenceParamsCount = iVar2;
	uVar1 = this->sequenceParamsCount;
	if (uVar1 != 0) {
		this->aSequenceParams = new CSequenceParam[uVar1];
		iVar2 = 0;
		if (0 < this->sequenceParamsCount) {
			do {
				piVar3 = &this->aSequenceParams[iVar2];
				iVar6 = pByteCode->GetS32();
				piVar3->field_0x0 = iVar6;
				iVar6 = pByteCode->GetS32();
				piVar3->count_0x4 = iVar6;
				uVar1 = piVar3->count_0x4;
				if (uVar1 != 0) {
					piVar3->aFloats = new float[uVar1];
					iVar6 = 0;
					if (0 < piVar3->count_0x4) {
						do {
							piVar3->aFloats[iVar6] = pByteCode->GetF32();
							iVar6 = iVar6 + 1;
						} while (iVar6 < piVar3->count_0x4);
					}
				}
				iVar2 = iVar2 + 1;
			} while (iVar2 < this->sequenceParamsCount);
		}
	}
	return;
}

