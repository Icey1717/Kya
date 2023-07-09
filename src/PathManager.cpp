#include "PathManager.h"
#include "MemoryStream.h"

CPathManager::CPathManager()
{
	this->field_0x4 = (Actor_3C*)0x0;
	this->count_0x8 = 0;
	this->field_0xc = (int*)0x0;
	return;
}


void CPathManager::Level_AddAll(ByteCode* pMemoryStream)
{
	uint count;
	int iVar1;
	int* piVar2;
	Actor_3C* pAVar3;
	undefined8 uVar4;
	int iVar5;

	pMemoryStream->GetChunk();
	iVar1 = pMemoryStream->GetS32();
	this->count_0x8 = iVar1;
	count = this->count_0x8;
	if (count != 0) {
		IMPLEMENTATION_GUARD(
		piVar2 = (int*)operator.new.array((long)(int)(count * 0x3c + 0x10));
		pAVar3 = (Actor_3C*)__construct_new_array(piVar2, FUN_001c37c0, FUN_001bf800, 0x3c, count);
		this->field_0x4 = pAVar3;
		iVar1 = 0;
		if (0 < (int)this->count_0x8) {
			iVar5 = 0;
			do {
				piVar2 = (int*)((int)&this->field_0x4->field_0x0 + iVar5);
				(**(code**)(*piVar2 + 0x28))(piVar2, pMemoryStream);
				iVar1 = iVar1 + 1;
				iVar5 = iVar5 + 0x3c;
			} while (iVar1 < (int)this->count_0x8);
		})
	}
	//piVar2 = (int*)operator.new(0x434);
	//if ((long)(int)piVar2 != 0) {
	//	uVar4 = FUN_001bf5f0((long)(int)piVar2);
	//	piVar2 = (int*)uVar4;
	//}
	//this->field_0xc = piVar2;
	return;
}
