#include "ActorFogManager.h"
#include "MemoryStream.h"
#include "ActorHero.h"
#include "CameraViewManager.h"
#include "EventManager.h"
#include "MathOps.h"

void CActorFogManager::Create(ByteCode* pByteCode)
{
	uint fogCount;
	int curIndex;
	CFogHierarchy* pHier;
	float fVar4;

	CActor::Create(pByteCode);

	this->fogHierarchiesArray.nbFogHierarchies = pByteCode->GetS32();
	fogCount = this->fogHierarchiesArray.nbFogHierarchies;

	if (fogCount != 0) {
		this->fogHierarchiesArray.aFogHierarchies = new CFogHierarchy[this->fogHierarchiesArray.nbFogHierarchies];
		curIndex = 0;
		if (0 < this->fogHierarchiesArray.nbFogHierarchies) {
			do {
				pHier = this->fogHierarchiesArray.aFogHierarchies + curIndex;
				pHier->Create(pByteCode);

				pHier->field_0x20 = pByteCode->GetF32();
				pHier->field_0x24 = pByteCode->GetF32();

				curIndex = curIndex + 1;
			} while (curIndex < this->fogHierarchiesArray.nbFogHierarchies);
		}
	}

	return;
}

void CActorFogManager::Init()
{
	int curIndex;

	CActor::Init();

	this->flags = this->flags | 2;
	this->flags = this->flags & 0xfffffffe;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	EvaluateDisplayState();

	curIndex = 0;
	if (0 < this->fogHierarchiesArray.nbFogHierarchies) {
		do {
			this->fogHierarchiesArray.aFogHierarchies[curIndex].Init(this->sectorId);
			curIndex = curIndex + 1;
		} while (curIndex < this->fogHierarchiesArray.nbFogHierarchies);
	}

	return;
}

void CActorFogManager::Term()
{
	int curIndex;

	curIndex = 0;
	if (0 < this->fogHierarchiesArray.nbFogHierarchies) {
		do {
			this->fogHierarchiesArray.aFogHierarchies[curIndex].Term(this->sectorId);
			curIndex = curIndex + 1;
		} while (curIndex < this->fogHierarchiesArray.nbFogHierarchies);
	}

	CActor::Term();

	return;
}

void CActorFogManager::Reset()
{
	CActor::Reset();

	this->flags = this->flags | 2;
	this->flags = this->flags & 0xfffffffe;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	EvaluateDisplayState();

	return;
}

CBehaviour* CActorFogManager::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourFogManagerZones;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

void CBehaviourFogManagerZones::Create(ByteCode* pByteCode)
{
	CStreamFogZoneInternal* pFogZoneArray;

	pFogZoneArray = reinterpret_cast<CStreamFogZoneInternal*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pFogZoneArray->aFogZones);
	if (pFogZoneArray->nbFogZones != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pFogZoneArray->nbFogZones * sizeof(CStreamFogZone);
	}

	this->streamFogZoneArray.pInternal = pFogZoneArray;

	return;
}

void CBehaviourFogManagerZones::Init(CActor* pOwner)
{
	CStreamFogZone* pFogZone;
	int curIndex;

	this->pOwner = static_cast<CActorFogManager*>(pOwner);
	pFogZone = this->streamFogZoneArray.pInternal->aFogZones;
	for (curIndex = this->streamFogZoneArray.pInternal->nbFogZones; curIndex != 0; curIndex = curIndex + -1) {
		pFogZone->zoneRef.Init();
		pFogZone = pFogZone + 1;
	}

	return;
}

void CBehaviourFogManagerZones::Term()
{
	return;
}

void CBehaviourFogManagerZones::Manage()
{
	this->streamFogZoneArray.Manage();
	this->pOwner->fogHierarchiesArray.Manage();

	return;
}

void CBehaviourFogManagerZones::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CStreamFogZone* pFogZone;
	int curIndex;

	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	pFogZone = this->streamFogZoneArray.pInternal->aFogZones;
	for (curIndex = this->streamFogZoneArray.pInternal->nbFogZones; curIndex != 0; curIndex = curIndex + -1) {
		if ((pFogZone->flags & 2) != 0) {
			pFogZone->flags = pFogZone->flags & 0xfffffffd;
			CScene::_pinstance->PopFogAndClippingSettings(&pFogZone->fogDef);
		}

		pFogZone = pFogZone + 1;
	}

	return;
}

void CBehaviourFogManagerZones::End(int newBehaviourId)
{
	CStreamFogZone* pFogZone;
	int curIndex;

	pFogZone = this->streamFogZoneArray.pInternal->aFogZones;
	for (curIndex = this->streamFogZoneArray.pInternal->nbFogZones; curIndex != 0; curIndex = curIndex + -1) {
		if ((pFogZone->flags & 2) != 0) {
			pFogZone->flags = pFogZone->flags & 0xfffffffd;
			CScene::_pinstance->PopFogAndClippingSettings(&pFogZone->fogDef);
		}

		pFogZone = pFogZone + 1;
	}

	return;
}

void CStreamFogZoneArray::Manage()
{
	ed_zone_3d* pZone;
	bool bIsInsideZone;
	CActorHero* pCameraTargetHero;
	int zoneResult;
	CStreamFogZone* pFogZone;
	int remainingZones;
	CCameraManager* pCameraManager;
	CEventManager* pEventManager;
	CActorHero* pHero;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pCameraManager = CScene::ptable.g_CameraManager_0045167c;

	pHero = CActorHero::_gThis;
	pCameraTargetHero = static_cast<CActorHero*>((CScene::ptable.g_CameraManager_0045167c)->pActiveCamera->GetTarget());
	if ((pHero != (CActorHero*)0x0) && (pHero == pCameraTargetHero)) {
		pCameraTargetHero = (CActorHero*)0x0;
	}

	remainingZones = this->pInternal->nbFogZones;
	pFogZone = this->pInternal->aFogZones;

	if (remainingZones != 0) {
		do {
			pZone = (pFogZone->zoneRef).Get();
			bIsInsideZone = false;
			if (pZone != (ed_zone_3d*)0x0) {
				if ((pFogZone->flags & 1) != 0) {
					if (pHero != (CActorHero*)0x0) {
						zoneResult = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pHero->currentLocation, 0);
						bIsInsideZone = zoneResult != 2;
					}

					if (pCameraTargetHero != (CActorHero*)0x0) {
						zoneResult = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pCameraTargetHero->currentLocation, 0);
						bIsInsideZone = static_cast<bool>(bIsInsideZone | zoneResult != 2);
					}
				}

				if ((pFogZone->flags & 2) != 0) {
					zoneResult = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, (edF32VECTOR4*)&(pCameraManager->transformationMatrix).da, 0);
					bIsInsideZone = static_cast<bool>(bIsInsideZone | zoneResult != 2);
				}
			}

			if (bIsInsideZone) {
				if ((pFogZone->flags & 2) == 0) {
					CScene::_pinstance->PushFogAndClippingSettings(pFogZone->field_0x18, &pFogZone->fogDef);
					pFogZone->flags = pFogZone->flags | 2;
				}
			}
			else {
				if ((pFogZone->flags & 2) != 0) {
					pFogZone->flags = pFogZone->flags & 0xfffffffd;
					CScene::_pinstance->PopFogAndClippingSettings(&pFogZone->fogDef);
				}
			}

			remainingZones = remainingZones + -1;
			pFogZone = pFogZone + 1;
		} while (remainingZones != 0);
	}

	return;
}

CFogHierarchiesArray::CFogHierarchiesArray()
{
	this->nbFogHierarchies = 0;
	this->aFogHierarchies = (CFogHierarchy*)0x0;

	return;
}

void CFogHierarchiesArray::Manage()
{
	edNODE* pNode;
	ed_3d_hierarchy* pHier;
	edF32MATRIX4* peVar2;
	CCameraManager* pCameraManager;
	byte alpha;
	CFogHierarchy* pFogHier;
	int index;
	float minDistance;
	float distance;
	edF32VECTOR4 relativePosition;

	pCameraManager = CScene::ptable.g_CameraManager_0045167c;

	for (index = 0; index < this->nbFogHierarchies; index = index + 1) {
		pFogHier = this->aFogHierarchies + index;
		pNode = pFogHier->pNode;

		if (pNode != (edNODE*)0x0) {
			pHier = reinterpret_cast<ed_3d_hierarchy*>(pNode->pData);
			edF32VECTOR4* pLodSphere = ed3DGetHierarchyFirstLODSphere(pHier);
			if (pLodSphere == (edF32VECTOR4*)0x0) {
				relativePosition = (pHier->transformA).rowT;
			}
			else {
				relativePosition.x = pLodSphere->x;
				relativePosition.y = pLodSphere->y;
				relativePosition.z = pLodSphere->z;
				relativePosition.w = 1.0f;

				edF32Matrix4MulF32Vector4Hard(&relativePosition, &pHier->transformA, &relativePosition);
			}

			relativePosition = relativePosition - (CScene::ptable.g_CameraManager_0045167c)->transformationMatrix.rowT;

			distance = edF32Vector4GetDistHard(&relativePosition);
			alpha = 0x80;
			if (distance < pFogHier->field_0x24) {
				minDistance = pFogHier->field_0x20;
				alpha = 0;

				if (minDistance < distance) {
					distance = ((distance - minDistance) * 128.0f) / (pFogHier->field_0x24 - minDistance);
					if (distance < 2.147484e+09f) {
						alpha = (byte)static_cast<int>(distance);
					}
					else {
						alpha = (byte)static_cast<int>(distance - 2.147484e+09f);
					}
				}
			}

			pFogHier->SetAlpha(alpha);
		}
	}

	return;
}
