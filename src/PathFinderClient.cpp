#include "PathFinderClient.h"
#include "PathManager.h"
#include "PathFinder.h"

CPathFinderClient::CPathFinderClient()
{
	this->id = -1;
	this->pPathDynamic = (CPathDynamic*)0x0;

	return;
}

void CPathFinderClient::Init()
{
	this->id = -1;
	this->pPathDynamic = (CPathDynamic*)0x0;
	this->field_0x8 = 0;
	return;
}

void CPathFinderClient::ChangePathfindingId(CActorMovable* pActor, int newId, edF32VECTOR4* pLocation)
{
	bool cVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	if (newId != this->id) {
		if (this->pPathDynamic != (CPathDynamic*)0x0) {
			CScene::ptable.g_PathManager_004516a0->pBasicPathFinder->DeletePathDynamic(this->pPathDynamic);
			this->pPathDynamic = 0;
		}

		this->id = newId;
		cVar1 = CScene::ptable.g_PathManager_004516a0->pBasicPathFinder->IsValidPosition(pLocation, this->id);

		if (cVar1 != false) {
			this->field_0x8 = 1;

			this->field_0x10 = *pLocation;
		}
	}
	return;
}

void CPathFinderClient::CleanPathDynamic()
{
	if (this->pPathDynamic != (CPathDynamic*)0x0) {
		(CScene::ptable.g_PathManager_004516a0)->pBasicPathFinder->DeletePathDynamic(this->pPathDynamic);
		this->pPathDynamic = (CPathDynamic*)0x0;
	}

	return;
}

bool CPathFinderClient::IsValidPosition(edF32VECTOR4* pPosition)
{
	return CScene::ptable.g_PathManager_004516a0->pBasicPathFinder->IsValidPosition(pPosition, this->id);
}
