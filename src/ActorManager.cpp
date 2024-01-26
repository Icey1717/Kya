#include "ActorManager.h"
#include "MemoryStream.h"
#include "CameraViewManager.h"
#include "ActorFactory.h"
#include "SectorManager.h"

void CActorManager::Level_Init()
{
	int iVar1;
	int iVar2;
	int iVar3;

	IMPLEMENTATION_GUARD_LOG(
	CActorManager::PrecomputeSectorsBoundindBoxes(this);
	CCluster::Init(&this->cluster, this->nbActors, this->pActorManagerSectorArray, 0););

	for (int i = 0; i < this->nbActors; i++) {
		this->aActors[i]->PreInit();
	}

	for (int i = 0; i < this->nbActors; i++) {
		this->aActors[i]->Init();
	}

	if (((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID == -1) {
		this->Level_SectorChange(-1, -1);
	}
	return;
}

void CActorManager::Level_AddAll(ByteCode* pMemoryStream)
{
	int actorCount = pMemoryStream->GetS32();
	this->nbActors = actorCount;

	ACTOR_LOG(LogLevel::Info, "CActorManager::Level_AddAll count: {}", actorCount);

	if (this->nbActors < 1) {
		this->aActors = (CActor**)0x0;
		this->aActiveActors = (CActor**)0x0;
		this->aSectorActors = (CActor**)0x0;
	}
	else {
		this->aActors = new CActor*[this->nbActors * 3];
		this->aActiveActors = this->aActors + this->nbActors;
		this->aSectorActors = this->aActiveActors + this->nbActors;
	}

	// #Debug
	ACTOR_CLASS lastClass;

	for (; 0 < actorCount; actorCount = actorCount + -1) {

		pMemoryStream->GetChunk();
		int actorIndex = pMemoryStream->GetS32();
		uint actorType = pMemoryStream->GetU32();

		CClassInfo* pClassInfo = &this->aClassInfo[actorType];
		CActor* pActor = (CActor*)(((char*)pClassInfo->aActors) + (pClassInfo->allocatedCount * pClassInfo->size));

		pClassInfo->allocatedCount = pClassInfo->allocatedCount + 1;

		pActor->actorManagerIndex = actorIndex;
		pActor->typeID = (ACTOR_CLASS)actorType;

		ACTOR_LOG(LogLevel::Info, "{0} type: 0x{1:x} ({1})", actorCount, pActor->typeID);

		pActor->Create(pMemoryStream);

		lastClass = pActor->typeID;

		this->aActors[actorIndex] = pActor;
	}

	if (this->nbActors != 0) {
		edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
		int* aActorLayerCounts = new int[this->nbActors];
		edMemClearFlags(TO_HEAP(H_MAIN), 0x100);

		int animationCount = 0;
		int animLayerCount = 0;
		int shadowCount = 0;

		if (0 < this->nbActors) {

			for (int i = 0; i < this->nbActors; i++) {
				CActor* pActor = this->aActors[i];

				if (pActor->field_0x110 == (undefined*)0x0) {
					aActorLayerCounts[i] = 0;
				}
				else {
					animationCount = animationCount + 1;
					int iVar7 = 0;
					int iVar8 = 0;
					uint uVar10 = 1;
					do {
						if ((pActor->subObjA->animLayerCount & uVar10) != 0) {
							iVar7 = iVar7 + 1;
						}
						iVar8 = iVar8 + 1;
						uVar10 = uVar10 << 1;
					} while (iVar8 < 4);
					aActorLayerCounts[i] = iVar7;
					animLayerCount = animLayerCount + iVar7;
				}

				if (pActor->subObjA->field_0x40 != 0) {
					shadowCount = shadowCount + 1;
				}
			}
		}

		if (animationCount != 0) {
			this->initializedAnimationCount = 0;
			this->animationCount = animationCount;
			this->aAnimation = new CAnimation[this->animationCount];

			this->initalizedAnimLayerCount = 0;
			this->animLayerCount = animLayerCount;
			this->aAnimLayers = new edAnmLayer[animLayerCount];
		}

		if (shadowCount != 0) {
			this->initializedShadowCount = 0;
			this->shadowCount = shadowCount;
			this->aShadows = new CShadow[this->shadowCount];
		}

		if (0 < this->nbActors) {
			for (int i = 0; i < this->nbActors; i++) {
				CActor* pActor = this->aActors[i];
				const int layerCount = aActorLayerCounts[i];

				if (layerCount != 0) {
					pActor->pAnimationController = &this->aAnimation[this->initializedAnimationCount];

					pActor->pAnimationController->Create(pActor, pActor->subObjA->animLayerCount, this->aAnimLayers + this->initalizedAnimLayerCount, layerCount);

					this->initalizedAnimLayerCount += layerCount;
					this->initializedAnimationCount++;
				}

				if (pActor->subObjA->field_0x40 == 0) {
					IMPLEMENTATION_GUARD_LOG(
					pActor->SetupShadow((CShadow*)0x0);)
				}
				else {
					IMPLEMENTATION_GUARD_LOG(
					pActor->SetupShadow(&this->aShadows[this->initializedShadowCount]);)
					this->initializedShadowCount++;
				}
			}
		}
		delete[] aActorLayerCounts;
	}
	return;
}

void CActorManager::Level_Manage()
{
	int iVar1;
	uint uVar2;
	edF32VECTOR4 localVector;
	int activeActorLen;
	ulong uVar3;
	ulong uVar4;
	uint counter;
	CActor** ppActiveActors;
	CActor** ppSectorActors;
	CActor** ppActiveActorsEnd;
	CActor** ppActors;
	CActor* pActor;
	float eleX;
	float eleY;
	float eleZ;
	
	// Clusterizers
	iVar1 = this->aClassInfo[0x18].totalCount;
	for (counter = 0; (int)counter < iVar1; counter = counter + 1) {
		this->aClassInfo[0x18].aActors[counter].Manage();
	}

	localVector.xyz = CCameraManager::_gThis->transformationMatrix.rowT.xyz;

	this->nbActiveActors = 0;

	ppActors = this->aSectorActors;
	ppActiveActors = this->aActiveActors;
	ppSectorActors = ppActors + this->nbSectorActors;
	for (; ppActors < ppSectorActors; ppActors = ppActors + 1) {
		pActor = *ppActors;
		if ((pActor->flags & 0x800000) == 0) {

			eleX = pActor->sphereCentre.x - localVector.x;
			eleY = pActor->sphereCentre.y - localVector.y;
			eleZ = pActor->sphereCentre.z - localVector.z;
			pActor->adjustedMagnitude = sqrtf(eleX * eleX + eleY * eleY + eleZ * eleZ) - pActor->sphereCentre.w;
			uVar2 = pActor->flags;

			if ((uVar2 & 8) == 0) {
				uVar3 = (ulong)(pActor->adjustedMagnitude <= (pActor->subObjA)->floatField);
			}
			else {
				if ((uVar2 & 0x10) == 0) {
					IMPLEMENTATION_GUARD(
					uVar3 = SEXT18(pActor->field_0xc);)
				}
				else {
					IMPLEMENTATION_GUARD(
					uVar3 = 0;
					if ((pActor->field_0xc != '\0') &&
						(pActor->adjustedMagnitude <= (pActor->subObjA)->floatField))
					{
						uVar3 = 1;
					})
				}
			}
			uVar4 = pActor->flags;
			if (((uVar3 | uVar4 & 2) == 0) || ((uVar4 & 0x2000001) != 0)) {
				if ((pActor->flags & 4) != 0) {
					pActor->FUN_00101110((CActor*)0x0);
					pActor->ChangeManageState(0);
				}
				pActor->UpdateVisibility();
			}
			else {
				if ((uVar4 & 4) == 0) {
					pActor->ChangeManageState(1);
				}
				pActor->FUN_00101110((CActor*)0x0);
				*ppActiveActors = pActor;
				ppActiveActors = ppActiveActors + 1;
			}
		}
	}

	activeActorLen = (char*)ppActiveActors - (char*)this->aActiveActors;
	if (activeActorLen < 0) {
		IMPLEMENTATION_GUARD();
		activeActorLen = activeActorLen + 3;
	}

	this->nbActiveActors = activeActorLen / sizeof(CActor*);

	ppActiveActors = this->aActiveActors;
	ppActiveActorsEnd = ppActiveActors + this->nbActiveActors;
	for (; ppActiveActors < ppActiveActorsEnd; ppActiveActors = ppActiveActors + 1) {
		/* Update animation */
		(*ppActiveActors)->Manage();
	}

	UpdateLinkedActors();
	return;
}

void CActorManager::Level_Draw()
{
	CActor* pActor;
	bool bShouldDraw;
	CActor** pActorsEnd;
	CActor** aActors;

	if ((GameFlags & 0x200) == 0) {
		aActors = this->aActiveActors;
		pActorsEnd = aActors + this->nbActiveActors;
		for (; aActors < pActorsEnd; aActors = aActors + 1) {
			pActor = *aActors;
			bShouldDraw = (pActor->flags & 0x4400) != 0;
			if (bShouldDraw) {
				bShouldDraw = pActor->adjustedMagnitude <= (pActor->subObjA)->field_0x1c;
			}
			if (bShouldDraw) {
				pActor->Draw();
			}
		}
	}
	if ((GameFlags & 0x20) == 0) {
		aActors = this->aSectorActors;
		pActorsEnd = aActors + this->nbSectorActors;
		for (; aActors < pActorsEnd; aActors = aActors + 1) {
			pActor = *aActors;
			if (((pActor->flags & 0x4000) != 0) && (pActor->p3DHierNode != (ed_3d_hierarchy_node*)0x0)) {
				pActor->ComputeLighting();
			}
		}
	}
	return;
}

void CActorManager::Level_SectorChange(int oldSectorId, int newSectorId)
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	int iVar8;
	CActor** pActorItr;
	CActor* pActor;

	if (oldSectorId != -1) {
		pActorItr = this->aSectorActors;
		for (iVar8 = this->nbSectorActors; 0 < iVar8; iVar8 = iVar8 + -1) {
			pActor = *pActorItr;
			if (oldSectorId == pActor->objectId) {
				pActor->EvaluateManageState();
				pActor->EvaluateDisplayState();
			}
			pActorItr = pActorItr + 1;
		}
	}

	if (newSectorId == -1) {
		IMPLEMENTATION_GUARD(
		SetWorldBox((int)&this->kyaChild, (undefined8*)this->pActorManagerSectorArray, 0);)
	}
	else {
		IMPLEMENTATION_GUARD_LOG(
		SetWorldBox((int)&this->kyaChild, (undefined8*)(this->pActorManagerSectorArray + newSectorId),
			(long)*(int*)((CScene::ptable.g_SectorManager_00451670)->subObjArray[newSectorId].pFileData + 8) & 1);)
	}

	const edF32VECTOR3 sphereOffset = (CCameraManager::_gThis->transformationMatrix).rowT.xyz;

	this->nbSectorActors = 0;
	pActorItr = this->aActors;
	for (iVar8 = this->nbActors; 0 < iVar8; iVar8 = iVar8 + -1) {
		pActor = *pActorItr;
		iVar1 = pActor->objectId;
		if ((iVar1 == newSectorId) || (iVar1 == -1)) {
			if (iVar1 == newSectorId) {
				const edF32VECTOR3 sphereCentre = pActor->sphereCentre.xyz - sphereOffset;
				pActor->adjustedMagnitude = sqrtf(sphereCentre.x * sphereCentre.x + sphereCentre.y * sphereCentre.y + sphereCentre.z * sphereCentre.z) - pActor->sphereCentre.w;
			}
			pActor->EvaluateManageState();
			pActor->EvaluateDisplayState();
			this->aSectorActors[this->nbSectorActors] = pActor;
			this->nbSectorActors = this->nbSectorActors + 1;

			IMPLEMENTATION_GUARD_LOG(
			pActor->UpdateClusterNode();)
		}

		pActorItr = pActorItr + 1;
	}

	for (iVar8 = 0; iVar8 < this->nbActors; iVar8 = iVar8 + 1) {
		pActor = this->aActors[iVar8];
		iVar1 = pActor->objectId;
		if ((newSectorId == iVar1) || (oldSectorId == iVar1)) {
			pActor->SectorChange(oldSectorId, newSectorId);
		}
	}
	return;
}

void CActorManager::UpdateLinkedActors()
{
	int iVar1;
	edF32MATRIX4* m1;
	CActor* pCVar2;
	float fVar3;
	edF32VECTOR3 local_50;
	edF32MATRIX4 local_40;
	//CActorFighterVTable* pActor;

	for (pCVar2 = this->pActorArray_0x8; pCVar2 != (CActor*)0x0; pCVar2 = *(CActor**)&pCVar2->adjustedMagnitude) {
		IMPLEMENTATION_GUARD(
		pActor = pCVar2->pVTable;
		iVar1 = (pCVar2->data).objectId;
		if (((((ActorBaseData*)&pActor->actorBase)->flags & 4) != 0) && ((*(uint*)(iVar1 + 8) & 4) != 0)) {
			m1 = CAnimation::GetCurBoneMatrix(*(CAnimation**)(iVar1 + 0x28), (pCVar2->data).flags);
			edF32Matrix4MulF32Matrix4Hard(&local_40, m1, *(edF32MATRIX4**)(iVar1 + 0x94));
			fVar3 = 1.0 / (SQRT(local_40.aa * local_40.aa + local_40.ab * local_40.ab + local_40.ac * local_40.ac) + 0.0);
			local_40.aa = local_40.aa * fVar3;
			local_40.ab = local_40.ab * fVar3;
			local_40.ac = local_40.ac * fVar3;
			local_40.ad = 0.0;
			fVar3 = 1.0 / (SQRT(local_40.ba * local_40.ba + local_40.bb * local_40.bb + local_40.bc * local_40.bc) + 0.0);
			local_40.ba = local_40.ba * fVar3;
			local_40.bb = local_40.bb * fVar3;
			local_40.bc = local_40.bc * fVar3;
			local_40.bd = 0.0;
			fVar3 = 1.0 / (SQRT(local_40.ca * local_40.ca + local_40.cb * local_40.cb + local_40.cc * local_40.cc) + 0.0);
			local_40.ca = local_40.ca * fVar3;
			local_40.cb = local_40.cb * fVar3;
			local_40.cc = local_40.cc * fVar3;
			local_40.cd = 0.0;
			(((ActorBaseData*)&pActor->actorBase)->rotationQuat).x = local_40.ca;
			(((ActorBaseData*)&pActor->actorBase)->rotationQuat).y = local_40.cb;
			(((ActorBaseData*)&pActor->actorBase)->rotationQuat).z = local_40.cc;
			(((ActorBaseData*)&pActor->actorBase)->rotationQuat).w = 0.0;
			edF32Matrix4ToEulerSoft(&local_40, &local_50, s_XYZ_00427e10);
			(((ActorBaseData*)&pActor->actorBase)->rotationEuler).x = local_50.x;
			(((ActorBaseData*)&pActor->actorBase)->rotationEuler).y = local_50.y;
			(((ActorBaseData*)&pActor->actorBase)->rotationEuler).z = local_50.z;
			CActor::UpdatePosition((CActor*)pActor, &local_40, (pCVar2->data).actorFieldS);
		})
	}
	return;
}

void CActorManager::Level_LoadClassesInfo(struct ByteCode* pMemoryStream)
{
	int actorCount;
	int classId;
	CClassInfo* pClassInfo;
	int totalCount;
	CActor* pAVar4;

	actorCount = pMemoryStream->GetS32();
	if (0 < actorCount) {
		for (int i = 0; i < actorCount; i++) {
			classId = pMemoryStream->GetS32();
			pClassInfo = &this->aClassInfo[classId];
			totalCount = pMemoryStream->GetS32();
			pClassInfo->totalCount = totalCount;
			pClassInfo->allocatedCount = 0;
			if (pClassInfo->totalCount != 0) {
				pClassInfo->aActors = CActorFactory::Factory((ACTOR_CLASS)classId, pClassInfo->totalCount, &pClassInfo->size);
			}
		}
	}
	return;
}

CActor* CActorManager::GetActorByHashcode(int hashCode)
{
	CActor* pCVar1;
	int iVar1;

	for (int i = 0; i < this->nbActors; i++) {
		if (hashCode == this->aActors[i]->subObjA->hashCode) {
			return this->aActors[i];
		}
	}

	return (CActor*)0x0;
}
