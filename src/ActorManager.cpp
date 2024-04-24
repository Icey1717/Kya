#include "ActorManager.h"
#include "MemoryStream.h"
#include "CameraViewManager.h"
#include "ActorFactory.h"
#include "SectorManager.h"

#include <math.h>
#include "MathOps.h"

void CActorManager::Level_Init()
{
	int iVar1;
	int iVar2;
	int iVar3;

	PrecomputeSectorsBoundindBoxes();

	this->cluster.Init(this->nbActors, this->aSectorBoundingBoxes, 0);

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

		ACTOR_LOG(LogLevel::Info, "id: {0} type: 0x{1:x} ({1})", actorCount, pActor->typeID);

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
	uint actorFlags;
	edF32VECTOR4 cameraLocation;
	int activeActorLen;
	bool bShouldBeActive;
	uint counter;
	CActor** ppActiveActors;
	CActor** ppSectorActorsEnd;
	CActor** ppActiveActorsEnd;
	CActor** ppSectorActors;
	CActor* pActor;
	float eleX;
	float eleY;
	float eleZ;
	
	// Clusterizers
	iVar1 = this->aClassInfo[0x18].totalCount;
	for (counter = 0; (int)counter < iVar1; counter = counter + 1) {
		this->aClassInfo[0x18].aActors[counter].Manage();
	}

	cameraLocation.xyz = CCameraManager::_gThis->transformationMatrix.rowT.xyz;

	this->nbActiveActors = 0;

	ACTOR_LOG(LogLevel::Verbose, "CActorManager::Level_Manage nbActors: {} camera location: {}", this->nbActors, cameraLocation.ToString());

	ppSectorActors = this->aSectorActors;
	ppActiveActors = this->aActiveActors;
	ppSectorActorsEnd = ppSectorActors + this->nbSectorActors;

	for (; ppSectorActors < ppSectorActorsEnd; ppSectorActors = ppSectorActors + 1) {
		pActor = *ppSectorActors;

		if ((pActor->flags & 0x800000) == 0) {

			eleX = pActor->sphereCentre.x - cameraLocation.x;
			eleY = pActor->sphereCentre.y - cameraLocation.y;
			eleZ = pActor->sphereCentre.z - cameraLocation.z;
			pActor->distanceToCamera = sqrtf(eleX * eleX + eleY * eleY + eleZ * eleZ) - pActor->sphereCentre.w;

			actorFlags = pActor->flags;

			ACTOR_LOG(LogLevel::Verbose, "CActorManager::Level_Manage sector actor: {} distance: {} flags: 0x{:x}", pActor->name, pActor->distanceToCamera, actorFlags);

			if ((actorFlags & 8) == 0) {
				bShouldBeActive = pActor->distanceToCamera <= (pActor->subObjA)->visibilityDistance;
				ACTOR_LOG(LogLevel::Verbose, "CActorManager::Level_Manage distance check sector actor: {} {} <= {} result: {}", 
					pActor->name, pActor->distanceToCamera, (pActor->subObjA)->visibilityDistance, bShouldBeActive);
			}
			else {
				if ((actorFlags & 0x10) == 0) {
					bShouldBeActive = pActor->state_0x10;
					ACTOR_LOG(LogLevel::Verbose, "CActorManager::Level_Manage state check sector actor: {} state: {} result: {}", 
						pActor->name, pActor->state_0x10, bShouldBeActive);
				}
				else {
					bShouldBeActive = false;

					ACTOR_LOG(LogLevel::Verbose, "CActorManager::Level_Manage state and distance check sector actor: {} state: {} distance: {} <= {} result: {}", 
						pActor->name, pActor->state_0x10, pActor->distanceToCamera, (pActor->subObjA)->visibilityDistance, bShouldBeActive);

					if ((pActor->state_0x10 != 0) && (pActor->distanceToCamera <= (pActor->subObjA)->visibilityDistance))
					{
						bShouldBeActive = true;
					}
				}
			}

			ACTOR_LOG(LogLevel::Verbose, "CActorManager::Level_Manage sector actor: {} shouldBeActive: {}", pActor->name, bShouldBeActive);

			actorFlags = pActor->flags;
			if (((bShouldBeActive | actorFlags & 2) == 0) || ((actorFlags & 0x2000001) != 0)) {
				if ((pActor->flags & 4) != 0) {
					pActor->FUN_00101110((CActor*)0x0);
					pActor->ChangeManageState(0);
				}

				pActor->UpdateVisibility();
			}
			else {
				if ((actorFlags & 4) == 0) {
					pActor->ChangeManageState(1);
				}

				pActor->FUN_00101110((CActor*)0x0);
				*ppActiveActors = pActor;
				ppActiveActors = ppActiveActors + 1;

				ACTOR_LOG(LogLevel::Verbose, "CActorManager::Level_Manage active actor: {}", pActor->name);
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
				bShouldDraw = pActor->distanceToCamera <= (pActor->subObjA)->field_0x1c;
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
		this->cluster.SetWorldBox(this->aSectorBoundingBoxes, 0);
	}
	else {
		this->cluster.SetWorldBox(this->aSectorBoundingBoxes + newSectorId,
			*(uint*)((CScene::ptable.g_SectorManager_00451670)->subObjArray[newSectorId].pFileData + 8) & 1);
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
				pActor->distanceToCamera = sqrtf(sphereCentre.x * sphereCentre.x + sphereCentre.y * sphereCentre.y + sphereCentre.z * sphereCentre.z) - pActor->sphereCentre.w;
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

void CActorManager::PrecomputeSectorsBoundindBoxes()
{
	int iVar1;
	ed_Bound_Box* peVar2;
	edF32VECTOR4* peVar3;
	ed_Bound_Box* puVar4;
	int iVar4;
	int iVar5;
	edF32VECTOR4* peVar6;
	int iVar7;
	float fVar8;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CActor* pActor;

	iVar1 = (CScene::ptable.g_SectorManager_00451670)->count_0x368;
	peVar2 = new ed_Bound_Box[iVar1 + 1];
	this->aSectorBoundingBoxes = peVar2;
	puVar4 = this->aSectorBoundingBoxes;
	iVar4 = 0;

	if (-1 < iVar1) {
		if (8 < iVar1 + 1) {
			do {
				(puVar4->tl).z = 1e+20f;
				(puVar4->tl).y = 1e+20f;
				iVar4 = iVar4 + 8;
				(puVar4->tl).x = 1e+20f;
				(puVar4->tl).w = 1.0f;
				(puVar4->br).z = -1e+20f;
				(puVar4->br).y = -1e+20f;
				(puVar4->br).x = -1e+20f;
				(puVar4->br).w = 1.0f;
				puVar4[1].tl.z = 1e+20f;
				puVar4[1].tl.y = 1e+20f;
				puVar4[1].tl.x = 1e+20f;
				puVar4[1].tl.w = 1.0f;
				puVar4[1].br.z = -1e+20f;
				puVar4[1].br.y = -1e+20f;
				puVar4[1].br.x = -1e+20f;
				puVar4[1].br.w = 1.0f;
				puVar4[2].tl.z = 1e+20f;
				puVar4[2].tl.y = 1e+20f;
				puVar4[2].tl.x = 1e+20f;
				puVar4[2].tl.w = 1.0f;
				puVar4[2].br.z = -1e+20f;
				puVar4[2].br.y = -1e+20f;
				puVar4[2].br.x = -1e+20f;
				puVar4[2].br.w = 1.0f;
				puVar4[3].tl.z = 1e+20f;
				puVar4[3].tl.y = 1e+20f;
				puVar4[3].tl.x = 1e+20f;
				puVar4[3].tl.w = 1.0f;
				puVar4[3].br.z = -1e+20f;
				puVar4[3].br.y = -1e+20f;
				puVar4[3].br.x = -1e+20f;
				puVar4[3].br.w = 1.0f;
				puVar4[4].tl.z = 1e+20f;
				puVar4[4].tl.y = 1e+20f;
				puVar4[4].tl.x = 1e+20f;
				puVar4[4].tl.w = 1.0f;
				puVar4[4].br.z = -1e+20f;
				puVar4[4].br.y = -1e+20f;
				puVar4[4].br.x = -1e+20f;
				puVar4[4].br.w = 1.0f;
				puVar4[5].tl.z = 1e+20f;
				puVar4[5].tl.y = 1e+20f;
				puVar4[5].tl.x = 1e+20f;
				puVar4[5].tl.w = 1.0f;
				puVar4[5].br.z = -1e+20f;
				puVar4[5].br.y = -1e+20f;
				puVar4[5].br.x = -1e+20f;
				puVar4[5].br.w = 1.0f;
				puVar4[6].tl.z = 1e+20f;
				puVar4[6].tl.y = 1e+20f;
				puVar4[6].tl.x = 1e+20f;
				puVar4[6].tl.w = 1.0f;
				puVar4[6].br.z = -1e+20f;
				puVar4[6].br.y = -1e+20f;
				puVar4[6].br.x = -1e+20f;
				puVar4[6].br.w = 1.0f;
				puVar4[7].tl.z = 1e+20f;
				puVar4[7].tl.y = 1e+20f;
				puVar4[7].tl.x = 1e+20f;
				puVar4[7].tl.w = 1.0f;
				puVar4[7].br.z = -1e+20f;
				puVar4[7].br.y = -1e+20f;
				puVar4[7].br.x = -1e+20f;
				puVar4[7].br.w = 1.0f;
				puVar4 = puVar4 + 8;
			} while (iVar4 <= iVar1 + -8);
		}

		for (; iVar4 <= iVar1; iVar4 = iVar4 + 1) {
			(puVar4->tl).z = 1e+20f;
			(puVar4->tl).y = 1e+20f;
			(puVar4->tl).x = 1e+20f;
			(puVar4->tl).w = 1.0f;
			(puVar4->br).z = -1e+20f;
			(puVar4->br).y = -1e+20f;
			(puVar4->br).x = -1e+20f;
			(puVar4->br).w = 1.0f;
			puVar4 = puVar4 + 1;
		}
	}

	local_20.z = 1e+20f;
	local_20.y = 1e+20f;
	local_20.x = 1e+20f;
	local_20.w = 1.0f;
	local_10.z = -1e+20f;
	local_10.y = -1e+20f;
	local_10.x = -1e+20f;
	local_10.w = 1.0f;

	iVar4 = 0;
	if (0 < this->nbActors) {
		do {
			pActor = this->aActors[iVar4];

			peVar6 = &pActor->sphereCentre;
			iVar7 = pActor->objectId;

			if (iVar7 == -1) {
				peVar2 = this->aSectorBoundingBoxes;
			}
			else {
				peVar2 = this->aSectorBoundingBoxes + iVar7;
			}

			iVar7 = 0;
			peVar3 = &local_20;
			do {
				fVar8 = peVar6->x;
				if (((peVar2->br).x < fVar8) && ((peVar2->br).x = fVar8, peVar3[1].x < fVar8)) {
					peVar3[1].x = fVar8;
				}
				if ((fVar8 < (peVar2->tl).x) && ((peVar2->tl).x = fVar8, fVar8 < peVar3->x)) {
					peVar3->x = fVar8;
				}
				iVar7 = iVar7 + 1;
				peVar6 = (edF32VECTOR4*)&peVar6->y;
				peVar2 = (ed_Bound_Box*)&(peVar2->tl).y;
				peVar3 = (edF32VECTOR4*)&peVar3->y;
			} while (iVar7 < 3);

			iVar4 = iVar4 + 1;
		} while (iVar4 < this->nbActors);
	}

	static edF32VECTOR4 edF32VECTOR4_0040e1a0 = { 20.0f, 20.0f, 20.0f, 0.0f };

	local_30 = edF32VECTOR4_0040e1a0;

	edF32Vector4SubHard(&eStack64, &local_10, &local_20);
	edF32Vector4AddHard(&local_10, &local_20, &local_10);
	edF32Vector4ScaleHard(0.5, &local_10, &local_10);
	edF32Vector4AddHard(&local_20, &eStack64, &local_30);

	peVar2 = this->aSectorBoundingBoxes;
	iVar4 = 0;
	if (-1 < iVar1) {
		do {
			if ((peVar2->br).x < (peVar2->tl).x) {
				peVar2->tl = local_20;

				peVar2->br = local_10;
			}
			else {
				edF32Vector4SubHard(&eStack64, &peVar2->br, &peVar2->tl);
				edF32Vector4AddHard(&peVar2->br, &peVar2->tl, &peVar2->br);
				edF32Vector4ScaleHard(0.5f, &peVar2->br, &peVar2->br);
				edF32Vector4AddHard(&peVar2->tl, &eStack64, &local_30);
			}
			iVar4 = iVar4 + 1;
			peVar2 = peVar2 + 1;
		} while (iVar4 <= iVar1);
	}
	peVar2 = this->aSectorBoundingBoxes;

	peVar2->tl = local_20;
	peVar2->br = local_10;
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

	for (pCVar2 = this->pActorArray_0x8; pCVar2 != (CActor*)0x0; pCVar2 = *(CActor**)&pCVar2->distanceToCamera) {
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

void CActorManager::GetActorsByClassID(int classId, CActorsTable* pOutList)
{
	int iVar1;
	CActor** pList;
	int curIndex;
	int totalCount;
	CClassInfo* pClassInfo;
	CActor** ppCVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	int iVar9;
	int iVar10;

	totalCount = this->aClassInfo[classId].totalCount;
	pClassInfo = &this->aClassInfo[classId];

	if (0x40 < totalCount) {
		totalCount = 0x40;
	}

	curIndex = 0;
	if (0 < totalCount) {
		pList = pOutList->aEntries;
		if (8 < totalCount) {
			do {
				IMPLEMENTATION_GUARD(); // CHECK CORRECT
				pList[curIndex + 0] = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pClassInfo->aActors) + (curIndex + 0) * pClassInfo->size);
				pList[curIndex + 1] = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pClassInfo->aActors) + (curIndex + 1) * pClassInfo->size);
				pList[curIndex + 2] = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pClassInfo->aActors) + (curIndex + 2) * pClassInfo->size);
				pList[curIndex + 3] = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pClassInfo->aActors) + (curIndex + 3) * pClassInfo->size);
				pList[curIndex + 4] = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pClassInfo->aActors) + (curIndex + 4) * pClassInfo->size);
				pList[curIndex + 5] = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pClassInfo->aActors) + (curIndex + 5) * pClassInfo->size);
				pList[curIndex + 6] = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pClassInfo->aActors) + (curIndex + 6) * pClassInfo->size);
				pList[curIndex + 7] = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pClassInfo->aActors) + (curIndex + 7) * pClassInfo->size);

				curIndex = curIndex + 8;
			} while (curIndex < totalCount + -8);
		}

		if (curIndex < totalCount) {
			ppCVar3 = pOutList->aEntries + curIndex + -1;
			do {
				ppCVar3[1] = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pClassInfo->aActors) + (curIndex + 0) * pClassInfo->size);
				curIndex = curIndex + 1;
				ppCVar3 = ppCVar3 + 1;
			} while (curIndex < totalCount);
		}
	}

	pOutList->entryCount = curIndex;
	return;
}

CCluster::CCluster()
{
	this->ppNodes = (CClusterNode**)0x0;
	this->aClusterNodes = (CClusterNode*)0x0;
	this->nbClusterNodes = 0;

	IMPLEMENTATION_GUARD_LOG(
	this->field_0x30 = 0;
	this->field_0x32 = 0;)
	this->field_0x34 = 0;
	this->pClusterNode = (CClusterNode*)0x0;
	return;
}

void CCluster::Init(int actorCount, ed_Bound_Box* pBoundBox, int param_4)
{
	uint count;
	CClusterNode* pCurClusterNode;
	int iVar3;
	int iVar4;

	this->ppNodes = new CClusterNode*[0x2000];
	iVar4 = 0;
	do {
		this->ppNodes[iVar4] = (CClusterNode*)0x0;
		iVar4 += 1;
	} while (iVar4 < 0x2000);

	this->field_0x34 = 0;

	this->nbClusterNodes = actorCount;
	count = this->nbClusterNodes;
	if (count == 0) {
		this->aClusterNodes = (CClusterNode*)0x0;
	}
	else {
		pCurClusterNode = new CClusterNode[count];
		this->aClusterNodes = pCurClusterNode;
		for (iVar3 = this->nbClusterNodes; 0 < iVar3; iVar3 = iVar3 + -1) {
			pCurClusterNode->pPrev = pCurClusterNode + -1;
			pCurClusterNode->pNext = pCurClusterNode + 1;
			pCurClusterNode = pCurClusterNode + 1;
		}
		this->aClusterNodes->pPrev = (CClusterNode*)0x0;
		this->aClusterNodes[this->nbClusterNodes + -1].pNext = (CClusterNode*)0x0;
	}

	this->pClusterNode = this->aClusterNodes;

	SetWorldBox(pBoundBox, param_4);
	return;
}

void CCluster::Term()
{
	if (this->ppNodes != (CClusterNode**)0x0) {
		delete[] this->ppNodes;
	}

	if (this->aClusterNodes != (CClusterNode*)0x0) {
		delete[] this->aClusterNodes;
	}

	this->ppNodes = (CClusterNode**)0x0;
	this->aClusterNodes = (CClusterNode*)0x0;
	this->nbClusterNodes = 0;
	this->field_0x30 = 0;
	this->field_0x32 = 0;
	this->field_0x34 = (CClusterNode*)0x0;
	this->pClusterNode = (CClusterNode*)0x0;

	return;
}

void CCluster::SetWorldBox(ed_Bound_Box* pBoundBox, int param_3)
{
	edF32VECTOR4 eStack48;
	edF32VECTOR4 bottomRight;
	edF32VECTOR4 topLeft;

	bottomRight = pBoundBox->br;
	topLeft = pBoundBox->tl;

	if (bottomRight.x < 320.0f) {
		bottomRight.x = 320.0f;
	}
	if (bottomRight.z < 320.0f) {
		bottomRight.z = 320.0f;
	}
	if (param_3 == 0) {
		topLeft.y = topLeft.y - bottomRight.y;
		bottomRight.y = 80000.0f;
		topLeft.y = topLeft.y + 5000.0f;
	}
	else {
		if (bottomRight.y < 80.0f) {
			bottomRight.y = 80.0f;
		}
	}
	edF32Vector4ScaleHard(0.5f, &eStack48, &bottomRight);
	edF32Vector4SubHard(&this->vectorA, &topLeft, &eStack48);
	(this->worldBox).x = 32.0f / bottomRight.x;
	(this->worldBox).y = 8.0f / bottomRight.y;
	(this->worldBox).z = 32.0f / bottomRight.z;
	(this->worldBox).w = 1.0f;
	return;
}

uint CCluster::GetMapCoords(edS32VECTOR3* pOutCoords, edF32VECTOR4* pLocation)
{
	uint uVar1;
	int iVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	uVar1 = 0;
	fVar3 = pLocation->y;
	fVar4 = pLocation->z;
	fVar5 = (this->vectorA).y;
	fVar7 = (this->vectorA).z;
	fVar6 = (this->worldBox).y;
	fVar8 = (this->worldBox).z;
	iVar2 = (int)((this->worldBox).x * (pLocation->x - (this->vectorA).x));
	if (iVar2 < 0) {
		uVar1 = 1;
		iVar2 = 0;
	}
	else {
		if (0x1f < iVar2) {
			uVar1 = 2;
			iVar2 = 0x1f;
		}
	}
	pOutCoords->x = iVar2;
	iVar2 = (int)(fVar6 * (fVar3 - fVar5));
	if (iVar2 < 0) {
		uVar1 = uVar1 | 4;
		iVar2 = 0;
	}
	else {
		if (7 < iVar2) {
			uVar1 = uVar1 | 8;
			iVar2 = 7;
		}
	}
	pOutCoords->y = iVar2;
	iVar2 = (int)(fVar8 * (fVar4 - fVar7));
	if (iVar2 < 0) {
		uVar1 = uVar1 | 0x10;
		iVar2 = 0;
	}
	else {
		if (0x1f < iVar2) {
			uVar1 = uVar1 | 0x20;
			iVar2 = 0x1f;
		}
	}
	pOutCoords->z = iVar2;
	return uVar1;
}

void CCluster::DeleteNode(CClusterNode* pNode)
{
	CClusterNode* pCVar1;
	CClusterNode** ppCVar2;

	ppCVar2 = &this->field_0x34;
	if (pNode->nodeIndex == -1) {
		pCVar1 = pNode->pNext;
	}
	else {
		ppCVar2 = this->ppNodes + pNode->nodeIndex;
		pCVar1 = pNode->pNext;
	}
	if (pCVar1 != (CClusterNode*)0x0) {
		pCVar1->pPrev = pNode->pPrev;
	}
	if (pNode->pPrev == (CClusterNode*)0x0) {
		*ppCVar2 = pNode->pNext;
	}
	else {
		pNode->pPrev->pNext = pNode->pNext;
	}
	pNode->nodeIndex = -2;
	pNode->pNext = (CClusterNode*)0x0;
	pNode->pPrev = (CClusterNode*)0x0;
	pNode->nodeIndex = -2;
	pNode->pNext = this->pClusterNode;
	if (this->pClusterNode != (CClusterNode*)0x0) {
		this->pClusterNode->pPrev = pNode;
	}
	pNode->pPrev = (CClusterNode*)0x0;
	this->pClusterNode = pNode;
	return;
}

CClusterNode* CCluster::NewNode(CActor* pActor)
{
	CClusterNode* pCVar1;
	ulong uVar2;
	CClusterNode** ppCVar3;
	int iVar4;
	edF32VECTOR4 local_20;
	edS32VECTOR3 local_10;

	pCVar1 = this->pClusterNode;
	if (pCVar1 != (CClusterNode*)0x0) {
		if (pCVar1->pNext != (CClusterNode*)0x0) {
			pCVar1->pNext->pPrev = pCVar1->pPrev;
		}
		if (pCVar1->pPrev == (CClusterNode*)0x0) {
			this->pClusterNode = pCVar1->pNext;
		}
		else {
			pCVar1->pPrev->pNext = pCVar1->pNext;
		}
		pCVar1->pActor = pActor;
		if (8.0f < pActor->sphereCentre.w) {
			iVar4 = -1;
		}
		else {
			local_20.xyz = pActor->sphereCentre.xyz;
			local_20.w = 1.0f;
			uVar2 = GetMapCoords(&local_10, &local_20);
			iVar4 = -1;
			if (uVar2 == 0) {
				iVar4 = local_10.x + local_10.y * 0x400 + local_10.z * 0x20;
			}
		}
		ppCVar3 = &this->field_0x34;
		if (iVar4 == -1) {
			pCVar1->nodeIndex = -1;
		}
		else {
			ppCVar3 = this->ppNodes + iVar4;
			pCVar1->nodeIndex = iVar4;
		}
		pCVar1->pNext = *ppCVar3;
		if (*ppCVar3 != (CClusterNode*)0x0) {
			(*ppCVar3)->pPrev = pCVar1;
		}
		pCVar1->pPrev = (CClusterNode*)0x0;
		*ppCVar3 = pCVar1;
	}
	return pCVar1;
}

struct ClusterCriterionCallbackParams
{
	CActorsTable* pTable;
	CritenionFunc* pFunc;
	void* pData;
};

void gClusterCallback_GetActorsWithCriterion(CActor* pActor, void* pData)
{
	bool bResult;

	ClusterCriterionCallbackParams* pParams = (ClusterCriterionCallbackParams*)pData;

	bResult = pParams->pFunc(pActor, pParams->pData);

	if (bResult != false) {
		pParams->pTable->Add(pActor);
	}

	return;
}

bool CCluster::GetActorsIntersectingSphereWithCriterion(CActorsTable* pTable, edF32VECTOR4* pLocation, CritenionFunc* pFunc, void* pData)
{
	ClusterCriterionCallbackParams local_10;

	local_10.pTable = pTable;
	local_10.pFunc = pFunc;
	local_10.pData = pData;

	ApplyCallbackToActorsIntersectingSphere(pLocation, gClusterCallback_GetActorsWithCriterion, &local_10);

	return pTable->entryCount != 0;
}

void CCluster::ApplyCallbackToActorsIntersectingSphere(edF32VECTOR4* pSphere, ColCallbackFuncPtr* pFunc, void* pParams)
{
	CClusterNode* pCVar1;
	CActor* this_00;
	int iVar2;
	CClusterNode** ppCVar3;
	int iVar4;
	CClusterNode** ppCVar5;
	bool bVar6;
	uint uVar7;
	uint uVar8;
	int iVar9;
	CClusterNode** local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edS32VECTOR3 local_20;
	edS32VECTOR3 local_10;

	if (this->ppNodes != (CClusterNode**)0x0) {
		for (pCVar1 = this->field_0x34; pCVar1 != (CClusterNode*)0x0; pCVar1 = pCVar1->pNext) {
			this_00 = pCVar1->pActor;
			bVar6 = this_00->SV_IsWorldBoundingSphereIntersectingSphere(pSphere);
			if (bVar6 != false) {
				pFunc(this_00, pParams);
			}
		}
		local_30.z = pSphere->w + 8.0f;

		local_40.xyz = pSphere->xyz - local_30.z;
		local_40.w = 1.0f;

		local_30.xyz = pSphere->xyz + local_30.z;
		local_30.w = 1.0f;

		uVar7 = GetMapCoords(&local_10, &local_40);
		uVar8 = GetMapCoords(&local_20, &local_30);
		if ((uVar7 & uVar8) == 0) {
			local_20.x = local_20.x - local_10.x;
			iVar9 = local_20.y - local_10.y;
			local_20.z = local_20.z - local_10.z;
			local_10.x = local_10.x + local_10.y * 0x400 + local_10.z * 0x20;
			local_20.y = iVar9;
			if (local_10.x == -1) {
				local_50 = &this->field_0x34;
			}
			else {
				local_50 = this->ppNodes + local_10.x;
			}
			for (; iVar2 = local_20.z, ppCVar3 = local_50, -1 < iVar9; iVar9 = iVar9 + -1) {
				for (; iVar4 = local_20.x, ppCVar5 = ppCVar3, -1 < iVar2; iVar2 = iVar2 + -1) {
					for (; -1 < iVar4; iVar4 = iVar4 + -1) {
						for (pCVar1 = *ppCVar5; pCVar1 != (CClusterNode*)0x0; pCVar1 = pCVar1->pNext) {
							bVar6 = pCVar1->pActor->SV_IsWorldBoundingSphereIntersectingSphere(pSphere);
							if (bVar6 != false) {
								pFunc(pCVar1->pActor, pParams);
							}
						}
						ppCVar5 = ppCVar5 + 1;
					}
					ppCVar3 = ppCVar3 + 0x20;
				}
				local_50 = local_50 + 0x400;
			}
		}
	}
	return;
}

edF32VECTOR4 edF32VECTOR4_00426880 = { 8.0f, 8.0f, 8.0f, 0.0f };

void CCluster::ApplyCallbackToActorsIntersectingBox(S_BOUNDING_BOX* pBoundingBox, ColCallbackFuncPtr* pFunc, void* pParams)
{
	bool bVar1;
	uint uVar2;
	uint uVar3;
	int iVar4;
	int iVar5;
	CClusterNode* pCVar6;
	CClusterNode** ppCVar7;
	CClusterNode** ppCVar8;
	CClusterNode** ppCVar9;
	int iVar10;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edS32VECTOR3 local_20;
	edS32VECTOR3 local_10;

	if (this->ppNodes != (CClusterNode**)0x0) {
		for (pCVar6 = this->field_0x34; pCVar6 != (CClusterNode*)0x0; pCVar6 = pCVar6->pNext) {
			bVar1 = pCVar6->pActor->SV_IsWorldBoundingSphereIntersectingBox(pBoundingBox);

			if (bVar1 != false) {
				pFunc(pCVar6->pActor, pParams);
			}
		}

		local_40 = pBoundingBox->minPoint - edF32VECTOR4_00426880;
		local_30 = pBoundingBox->maxPoint + edF32VECTOR4_00426880;

		uVar2 = GetMapCoords(&local_10, &local_40);
		uVar3 = GetMapCoords(&local_20, &local_30);

		if ((uVar2 & uVar3) == 0) {
			iVar4 = local_20.y - local_10.y;
			local_20.z = local_20.z - local_10.z;
			local_20.x = local_20.x - local_10.x;
			local_10.x = local_10.x + local_10.y * 0x400 + local_10.z * 0x20;
			local_20.y = iVar4;

			if (local_10.x == -1) {
				ppCVar8 = &this->field_0x34;
			}
			else {
				ppCVar8 = this->ppNodes + local_10.x;
			}

			for (; ppCVar7 = ppCVar8, iVar5 = local_20.z, -1 < iVar4; iVar4 = iVar4 + -1) {
				for (; ppCVar9 = ppCVar7, iVar10 = local_20.x, -1 < iVar5; iVar5 = iVar5 + -1) {
					for (; -1 < iVar10; iVar10 = iVar10 + -1) {
						for (pCVar6 = *ppCVar9; pCVar6 != (CClusterNode*)0x0; pCVar6 = pCVar6->pNext) {
							bVar1 = pCVar6->pActor->SV_IsWorldBoundingSphereIntersectingBox(pBoundingBox);

							if (bVar1 != false) {
								pFunc(pCVar6->pActor, pParams);
							}
						}

						ppCVar9 = ppCVar9 + 1;
					}

					ppCVar7 = ppCVar7 + 0x20;
				}

				ppCVar8 = ppCVar8 + 0x400;
			}
		}
	}

	return;
}

CClusterNode::CClusterNode()
{
	this->pNext = (CClusterNode*)0x0;
	this->pPrev = (CClusterNode*)0x0;
	this->nodeIndex = -2;
	this->pActor = (CActor*)0x0;
}

void CClusterNode::Insert(CCluster* pCluster, CActor* pActor)
{
	uint uVar1;
	CClusterNode** ppCVar2;
	int iVar3;
	edF32VECTOR4 local_20;
	edS32VECTOR3 local_10;

	this->pActor = pActor;

	if (8.0f < (pActor->sphereCentre).w) {
		iVar3 = -1;
	}
	else {
		local_20.xyz = (pActor->sphereCentre).xyz;
		local_20.w = 1.0f;

		uVar1 = pCluster->GetMapCoords(&local_10, &local_20);

		iVar3 = -1;
		if (uVar1 == 0) {
			iVar3 = local_10.x + local_10.y * 0x400 + local_10.z * 0x20;
		}
	}

	ppCVar2 = &pCluster->field_0x34;
	if (iVar3 == -1) {
		this->nodeIndex = -1;
	}
	else {
		ppCVar2 = pCluster->ppNodes + iVar3;
		this->nodeIndex = iVar3;
	}

	this->pNext = *ppCVar2;
	if (*ppCVar2 != (CClusterNode*)0x0) {
		(*ppCVar2)->pPrev = this;
	}

	this->pPrev = (CClusterNode*)0x0;
	*ppCVar2 = this;
	return;
}

void CClusterNode::Update(CCluster* pCluster)
{
	int iVar1;
	uint uVar2;
	CClusterNode* pCVar3;
	CClusterNode** ppCVar4;
	int iVar5;
	edF32VECTOR4 location;
	edS32VECTOR3 mapCoords;
	CActor* pCurActor;

	pCurActor = this->pActor;
	if (8.0f < pCurActor->sphereCentre.w) {
		iVar5 = -1;
	}
	else {
		location.xyz = pCurActor->sphereCentre.xyz;
		location.w = 1.0f;
		uVar2 = pCluster->GetMapCoords(&mapCoords, &location);
		iVar5 = -1;
		if (uVar2 == 0) {
			iVar5 = mapCoords.x + mapCoords.y * 0x400 + mapCoords.z * 0x20;
		}
	}
	iVar1 = this->nodeIndex;
	if (iVar1 != iVar5) {
		ppCVar4 = &pCluster->field_0x34;
		if (iVar1 == -1) {
			pCVar3 = this->pNext;
		}
		else {
			ppCVar4 = pCluster->ppNodes + iVar1;
			pCVar3 = this->pNext;
		}
		if (pCVar3 != (CClusterNode*)0x0) {
			pCVar3->pPrev = this->pPrev;
		}
		if (this->pPrev == (CClusterNode*)0x0) {
			*ppCVar4 = this->pNext;
		}
		else {
			this->pPrev->pNext = this->pNext;
		}
		ppCVar4 = &pCluster->field_0x34;
		if (iVar5 == -1) {
			this->nodeIndex = -1;
		}
		else {
			ppCVar4 = pCluster->ppNodes + iVar5;
			this->nodeIndex = iVar5;
		}
		this->pNext = *ppCVar4;
		if (*ppCVar4 != (CClusterNode*)0x0) {
			(*ppCVar4)->pPrev = this;
		}
		this->pPrev = (CClusterNode*)0x0;
		*ppCVar4 = this;
	}
	return;
}
