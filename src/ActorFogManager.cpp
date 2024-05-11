#include "ActorFogManager.h"
#include "MemoryStream.h"

void CActorFogManager::Create(ByteCode* pByteCode)
{
	uint count;
	int iVar1;
	int* piVar2;
	int iVar3;
	CFogHierarchy* pHier;
	float fVar4;

	CActor::Create(pByteCode);
	iVar1 = pByteCode->GetS32();
	this->field_0x16c = iVar1;
	count = this->field_0x16c;
	if (count != 0) {
		this->field_0x170 = new CFogHierarchy[this->field_0x16c];
		iVar1 = 0;
		if (0 < this->field_0x16c) {
			iVar3 = 0;
			do {
				pHier = this->field_0x170 + iVar1;
				pHier->Create(pByteCode);
				fVar4 = pByteCode->GetF32();
				pHier->field_0x20 = fVar4;
				fVar4 = pByteCode->GetF32();
				pHier->field_0x24 = fVar4;
				iVar1 = iVar1 + 1;
				iVar3 = iVar3 + 0x28;
			} while (iVar1 < this->field_0x16c);
		}
	}
	return;
}
