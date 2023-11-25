#include "WayPoint.h"
#include "MemoryStream.h"

CWayPointManager::CWayPointManager()
{
	this->field_0x4 = (ManagerC_24*)0x0;
	this->count_0x8 = 0;
	return;
}

void CWayPointManager::Level_AddAll(ByteCode* pMemoryStream)
{
	ManagerC_24* pMVar1;
	int iVar2;

	pMemoryStream->GetChunk();
	iVar2 = pMemoryStream->GetS32();
	this->count_0x8 = iVar2;
	if (this->count_0x8 == 0) {
		this->field_0x4 = (ManagerC_24*)0x0;
	}
	else {
		pMVar1 = (ManagerC_24*)pMemoryStream->currentSeekPos;
		pMemoryStream->currentSeekPos = (char*)(pMVar1 + this->count_0x8);
		this->field_0x4 = pMVar1;
	}
	return;
}

