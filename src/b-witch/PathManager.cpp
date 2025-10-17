#include "PathManager.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include <math.h>

CPathManager::CPathManager()
{
	this->aPathFollow = (CPathFollow*)0x0;
	this->pathFollowCount = 0;
	this->pBasicPathFinder = (CBasicPathFinder*)0x0;
	return;
}


void CPathManager::Level_Init()
{
	if (this->pBasicPathFinder != (CBasicPathFinder*)0x0) {
		this->pBasicPathFinder->Init();
	}
}

void CPathManager::Level_Term()
{
	return;
}

void CPathManager::Level_AddAll(ByteCode* pByteCode)
{
	uint count;
	int iVar1;
	int* piVar2;
	CPathFollow* pAVar3;
	undefined8 uVar4;
	int iVar5;

	pByteCode->GetChunk();
	iVar1 = pByteCode->GetS32();
	this->pathFollowCount = iVar1;
	count = this->pathFollowCount;
	if (count != 0) {
		pAVar3 = new CPathFollow[count];
		this->aPathFollow = pAVar3;

		for (int i = 0; i < this->pathFollowCount; i++) {
			this->aPathFollow[i].Create(pByteCode);
		}
	}

	this->pBasicPathFinder = new CBasicPathFinder();

	return;
}

void CPathManager::Level_ClearAll()
{
	CBasicPathFinder* pCVar1;

	if (this->aPathFollow != (CPathFollow*)0x0) {
		delete[] this->aPathFollow;
	}

	if ((this->pBasicPathFinder != (CBasicPathFinder*)0x0) && (this->pBasicPathFinder != (CBasicPathFinder*)0x0)) {
		delete this->pBasicPathFinder;
	}

	this->aPathFollow = (CPathFollow*)0x0;
	this->pathFollowCount = 0;
	this->pBasicPathFinder = (CBasicPathFinder*)0x0;

	return;
}

void CPathManager::Level_Manage()
{
	return;
}

void CPathManager::Level_ManagePaused()
{
	return;
}

void CPathManager::Level_Draw()
{
	return;
}
