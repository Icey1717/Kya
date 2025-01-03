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

void CWayPointManager::Level_ClearAll()
{
	this->aWaypoints = (CWayPoint*)0x0;
	this->waypointCount = 0;

	return;
}

template<>
void S_STREAM_REF<CWayPoint>::Init()
{
	CWayPoint* pZone;

	if (this->index == -1) {
		pZone = (CWayPoint*)0x0;
	}
	else {
		pZone = (CScene::ptable.g_WayPointManager_0045169c)->aWaypoints + this->index;
	}

	this->pObj = STORE_SECTION(pZone);
	return;
}