#include "WayPoint.h"
#include "MemoryStream.h"

CWayPointManager::CWayPointManager()
{
	this->aWaypoints = (CWayPoint*)0x0;
	this->waypointCount = 0;
	return;
}

void CWayPointManager::Level_AddAll(ByteCode* pMemoryStream)
{
	CWayPoint* pMVar1;
	int iVar2;

	pMemoryStream->GetChunk();
	iVar2 = pMemoryStream->GetS32();
	this->waypointCount = iVar2;
	if (this->waypointCount == 0) {
		this->aWaypoints = (CWayPoint*)0x0;
	}
	else {
		pMVar1 = (CWayPoint*)pMemoryStream->currentSeekPos;
		pMemoryStream->currentSeekPos = (char*)(pMVar1 + this->waypointCount);
		this->aWaypoints = pMVar1;
	}
	return;
}

