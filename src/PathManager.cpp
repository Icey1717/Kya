#include "PathManager.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include <math.h>

CPathManager::CPathManager()
{
	this->aPathFollow = (CPathFollow*)0x0;
	this->pathFollowCount = 0;
	this->field_0xc = (int*)0x0;
	return;
}


void CPathManager::Level_AddAll(ByteCode* pMemoryStream)
{
	uint count;
	int iVar1;
	int* piVar2;
	CPathFollow* pAVar3;
	undefined8 uVar4;
	int iVar5;

	pMemoryStream->GetChunk();
	iVar1 = pMemoryStream->GetS32();
	this->pathFollowCount = iVar1;
	count = this->pathFollowCount;
	if (count != 0) {
		pAVar3 = new CPathFollow[count];
		this->aPathFollow = pAVar3;

		for (int i = 0; i < this->pathFollowCount; i++) {
			this->aPathFollow[i].Create(pMemoryStream);
		}
	}
	//piVar2 = (int*)operator.new(0x434);
	//if ((long)(int)piVar2 != 0) {
	//	uVar4 = FUN_001bf5f0((long)(int)piVar2);
	//	piVar2 = (int*)uVar4;
	//}
	//this->field_0xc = piVar2;
	return;
}


