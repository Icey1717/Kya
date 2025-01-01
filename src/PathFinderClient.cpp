#include "PathFinderClient.h"
#include "PathManager.h"
#include "PathFinder.h"
#include "ActorMovable.h"

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
	this->naviMeshIndex = 0;
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
			this->naviMeshIndex = 1;

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

bool CPathFinderClient::HasPathTo(CActorMovable* pActor, edF32VECTOR4* v0)
{
	CBasicPathFinder* pBasicPathFinder;
	CPathDynamic* pPathDynamic;
	edF32VECTOR4* pStart;
	bool bHasPathTo;

	bHasPathTo = false;
	pStart = (edF32VECTOR4*)0x0;
	pBasicPathFinder = (CScene::ptable.g_PathManager_004516a0)->pBasicPathFinder;

	if (pBasicPathFinder->IsValidPosition(&pActor->currentLocation, this->id) == 0) {
		if (this->naviMeshIndex != 0) {
			pStart = &this->field_0x10;
		}
	}
	else {
		pStart = &pActor->currentLocation;
	}

	if ((pStart != (edF32VECTOR4*)0x0) && (pPathDynamic = pBasicPathFinder->NewPathDynamic(pStart, v0), pPathDynamic != (CPathDynamic*)0x0)) {
		pBasicPathFinder->FindPath(pPathDynamic, this->id);
		if ((pPathDynamic->GetStatus() == 0) || (bHasPathTo = false, pPathDynamic->GetStatus() == 1)) {
			bHasPathTo = true;
		}

		pBasicPathFinder->DeletePathDynamic(pPathDynamic);
	}

	return bHasPathTo;
}