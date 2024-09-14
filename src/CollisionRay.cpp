#include "CollisionRay.h"
#include "MathOps.h"
#include "Actor.h"
#include "ActorFactory.h"
#include "CollisionManager.h"
#include "ActorManager.h"


CCollisionRay::CCollisionRay(float f0, float f1, float f2, edF32VECTOR4* pLocation, edF32VECTOR4* pDirection)
{
	edF32MATRIX4 localMatrix;

	COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::CCollisionRay f0: {} f1: {} f2: {} pLocation: {} pDirection: {}", 
		f0, f1, f2, pLocation->ToString(), pDirection->ToString());

	edF32Matrix4RotateXHard(f0, &localMatrix, &gF32Matrix4Unit);
	edF32Matrix4RotateYHard(f1, &localMatrix, &localMatrix);
	edF32Matrix4MulF32Vector4Hard(pDirection, &localMatrix, &gF32Vector4UnitZ);

	this->pLocation = pLocation;
	this->pLeadVector = pDirection;
	this->lengthA = f2;
	this->lengthB.x = f2;
	return;
}

CCollisionRay::CCollisionRay(float f0, edF32VECTOR4* pLocation, edF32VECTOR4* pDirection)
{
	COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::CCollisionRay f0: {} pLocation: {} pDirection: {}", f0, pLocation->ToString(), pDirection->ToString());

	this->pLocation = pLocation;
	this->pLeadVector = pDirection;
	this->lengthA = f0;
	this->lengthB.x = f0;
}

float CCollisionRay::Intersect(uint type, CActor* pActor, CActor* pOther, uint flags, edF32VECTOR4* pOutVector, _ray_info_out* pResultData)
{
	float fVar1;
	float fVar2;
	edF32VECTOR4 outVector;
	_ray_info_out outResult;

	fVar1 = 1e+30f;

	COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::Intersect type: {} actor: {} other: {} flags: 0x{:x}", 
		type, pActor ? pActor->name : "None", pOther ? pOther->name : "None", flags);

	if ((type & 2) != 0) {
		COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::Intersect IntersectActors");
		fVar1 = IntersectActors(pActor, pOther, flags, pOutVector, pResultData);
	}

	if ((type & 1) != 0) {
		COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::Intersect IntersectScenery");

		if (pOutVector == (edF32VECTOR4*)0x0) {
			fVar2 = IntersectScenery((edF32VECTOR4*)0x0, &outResult);
		}
		else {
			fVar2 = IntersectScenery(&outVector, &outResult);
		}

		if (fVar2 < fVar1) {
			if (pOutVector != (edF32VECTOR4*)0x0) {
				*pOutVector = outVector;
			}

			fVar1 = fVar2;

			if (pResultData != (_ray_info_out*)0x0) {
				pResultData->pActor_0x0 = outResult.pActor_0x0;
				pResultData->pVector_0x4 = outResult.pVector_0x4;
				pResultData->type_0x8 = outResult.type_0x8;
			}
		}
	}

	return fVar1;
}

struct ClusterCallbackParams_0x10 {
	CFixedTable<CActor*, 64>* pTable;
	CActor* pActor;
	CActor* pOtherActor;
	uint flags;
};

void gClusterCallback_GetActorsRay(CActor* pActor, void* pInParams)
{
	ClusterCallbackParams_0x10* pParams = reinterpret_cast<ClusterCallbackParams_0x10*>(pInParams);

	COLLISION_LOG(LogLevel::Verbose, "gClusterCallback_GetActorsRay actor: {} other: {} flags: {}", 
		pActor ? pActor->name : "None", pParams->pOtherActor ? pParams->pOtherActor->name : "None", pParams->flags);

	if ((((pActor->pCollisionData != (CCollision*)0x0) && (pParams->pActor != pActor)) &&
		(pParams->pOtherActor != pActor)) &&
		(((pParams->flags & 0x3fffffff & CActorFactory::gClassProperties[pActor->typeID].field_0x4) != 0 &&
			((pActor->pCollisionData->flags_0x0 & 0x80000) != 0)))) {
		pParams->pTable->Add(pActor);
	}
}

float CCollisionRay::IntersectActors(CActor* pActor, CActor* pOtherActor, uint flags, edF32VECTOR4* pOutVector, _ray_info_out* pOutResult)
{
	edF32VECTOR4* pLoc;
	edF32VECTOR4* pDir;
	float y;
	float z;
	float x;
	ClusterCallbackParams_0x10 callbackParams;
	S_BOUNDING_BOX boundingBox;
	edF32VECTOR4 local_120;
	CFixedTable<CActor*, 64> actorIntersectingBoxTable;

	actorIntersectingBoxTable.entryCount = 0;

	pLoc = this->pLocation;
	pDir = this->pLeadVector;

	COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::IntersectActors location: {} direction: {} length: {}", pLoc->ToString(), pDir->ToString(), z);

	x = pDir->x * this->lengthA + pLoc->x;
	y = pDir->y * this->lengthA + pLoc->y;
	z = pDir->z * this->lengthA + pLoc->z;

	boundingBox.minPoint.x = std::min(x, pLoc->x);
	boundingBox.maxPoint.x = std::max(x, pLoc->x);

	boundingBox.minPoint.y = std::min(y, pLoc->y);
	boundingBox.maxPoint.y = std::max(y, pLoc->y);

	boundingBox.minPoint.z = std::min(z, pLoc->z);
	boundingBox.maxPoint.z = std::max(z, pLoc->z);

	boundingBox.minPoint.w = 1.0f;
	boundingBox.maxPoint.w = 1.0f;

	assert(boundingBox.minPoint.x < 4294967296.0f);

	COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::IntersectActors bounding box: min: {} max: {}", boundingBox.minPoint.ToString(), boundingBox.maxPoint.ToString());

	callbackParams.pTable = &actorIntersectingBoxTable;
	callbackParams.pActor = pActor;
	callbackParams.pOtherActor = pOtherActor;
	callbackParams.flags = flags;

	(CScene::ptable.g_ActorManager_004516a4)->cluster.ApplyCallbackToActorsIntersectingBox(&boundingBox, gClusterCallback_GetActorsRay, &callbackParams);

	COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::IntersectActors intersecting actors: {}", actorIntersectingBoxTable.entryCount);

	const float distance = IntersectActorsTable(&actorIntersectingBoxTable, pOutVector, pOutResult);

	COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::IntersectActors intersecting distance: {}", distance);

	return distance;
}

float CCollisionRay::IntersectActorsTable(CActorsTable* pTable, edF32VECTOR4* v0, _ray_info_out* pOutResult)
{
	CActor* pActor;
	int counter;
	uint outHitType;
	CActor* pOutActor;
	void* pOutHitPrim;
	float obbIntersectDistance;
	float distance;
	edF32VECTOR4* pLocalVector;
	edF32VECTOR4 localVector;
	void* pHitPrim;
	uint hitType;
	uint local_4;
	int total;

	COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::IntersectActorsTable count: {}", pTable->entryCount);

	distance = 1e+30f;

	pOutActor = (CActor*)0x0;
	outHitType = 0;
	local_4 = 0;
	this->lengthA = this->lengthB.x;
	total = pTable->entryCount;
	counter = 0;

	if (0 < total) {
		do {
			pActor = pTable->aEntries[counter];

			COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::IntersectActorsTable actor: {}", pActor->name);

			obbIntersectDistance = edObbIntersectObbTreeRayPrim(&pHitPrim, &hitType, pActor->pCollisionData->pObbTree, this);

			COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::IntersectActorsTable obbIntersectDistance: {}", obbIntersectDistance);

			if ((0.0f <= obbIntersectDistance) && (obbIntersectDistance < distance)) {
				this->lengthA = obbIntersectDistance;
				distance = obbIntersectDistance;
				pOutActor = pActor;
				outHitType = hitType;
				pOutHitPrim = pHitPrim;
			}

			counter = counter + 1;
		} while (counter < total);
	}

	if (distance != 1e+30f) {
		pLocalVector = v0;

		if (v0 == (edF32VECTOR4*)0x0) {
			pLocalVector = &localVector;
		}

		ComputeIntersectionNormalAndProps(distance, pOutHitPrim, outHitType, pLocalVector, &local_4);
	}

	if (pOutResult != (_ray_info_out*)0x0) {
		pOutResult->pActor_0x0 = pOutActor;
		pOutResult->pVector_0x4 = local_4;
		pOutResult->type_0x8 = outHitType;
	}

	COLLISION_LOG(LogLevel::Verbose, "CCollisionRay::IntersectActorsTable distance: {}", distance);

	return distance;
}

void ComputeNormal(edF32VECTOR4* param_1, edF32VECTOR4* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar3 = param_3->x - param_2->x;
	fVar5 = param_3->y - param_2->y;
	fVar1 = param_3->z - param_2->z;
	fVar4 = param_4->x - param_2->x;
	fVar6 = param_4->y - param_2->y;
	fVar2 = param_4->z - param_2->z;
	param_1->x = fVar5 * fVar2 - fVar6 * fVar1;
	param_1->y = fVar1 * fVar4 - fVar2 * fVar3;
	param_1->z = fVar3 * fVar6 - fVar4 * fVar5;
	param_1->w = in_vf0x;

	fVar3 = param_1->x;
	fVar4 = param_1->y;
	fVar5 = param_1->z;
	fVar6 = 1.0f / (sqrtf(fVar3 * fVar3 + fVar4 * fVar4 + fVar5 * fVar5) + 0.0f);
	param_1->x = fVar3 * fVar6;
	param_1->y = fVar4 * fVar6;
	param_1->z = fVar5 * fVar6;
	param_1->w = 0.0f;
	return;
}

void CCollisionRay::ComputeIntersectionNormalAndProps(float distance, void* pColObj, int colType, edF32VECTOR4* pOutNormal, uint* pOutProps)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if ((((colType == 0xe) || (colType == 0xd)) || (colType == 10)) || (colType == 0xb)) {
		edColPRIM_OBJECT* pPrim = reinterpret_cast<edColPRIM_OBJECT*>(pColObj);

		*pOutProps = pPrim->flags_0x80;

		peVar1 = this->pLeadVector;
		peVar2 = this->pLocation;

		local_10 = *peVar1 * distance + *peVar2;

		edF32Matrix4MulF32Vector4Hard(&local_20, &pPrim->worldTransform, &local_10);
		if (colType == 10) {
			if (fabs(local_20.x) <= fabs(local_20.y)) {
				local_20.x = 0.0f;
				if (fabs(local_20.y) <= fabs(local_20.z)) {
					local_20.y = 0.0f;
				}
				else {
					local_20.z = 0.0f;
				}
			}
			else {
				local_20.y = 0.0f;
				if (fabs(local_20.x) < fabs(local_20.z)) {
					local_20.x = 0.0f;
				}
				else {
					local_20.z = 0.0f;
				}
			}
		}

		edColGetNormalInWorldFromLocal(pOutNormal, &pPrim->worldTransform, &local_20);

	}
	else {
		if (colType == 8) {
			edF32QUAD4* pQuad = reinterpret_cast<edF32QUAD4*>(pColObj);
			*pOutProps = pQuad->flags;

			peVar1 = this->pLeadVector;
			peVar2 = this->pLocation;
			local_10 = *peVar1 * distance + *peVar2;

			ComputeNormal(&local_30, LOAD_SECTION_CAST(edF32VECTOR4*, pQuad->p1), LOAD_SECTION_CAST(edF32VECTOR4*, pQuad->p2), LOAD_SECTION_CAST(edF32VECTOR4*, pQuad->p3));
			ComputeNormal(&local_40, LOAD_SECTION_CAST(edF32VECTOR4*, pQuad->p1), LOAD_SECTION_CAST(edF32VECTOR4*, pQuad->p3), LOAD_SECTION_CAST(edF32VECTOR4*, pQuad->p4));

			fVar3 = edDistPointToPlane(LOAD_SECTION_CAST(edF32VECTOR4*, pQuad->p1), &local_30, &local_10);
			fVar4 = edDistPointToPlane(LOAD_SECTION_CAST(edF32VECTOR4*, pQuad->p1), &local_40, &local_10);

			if (fVar3 <= fVar4) {
				*pOutNormal = local_30;
			}
			else {
				*pOutNormal = local_40;
			}
		}
		else {
			if (colType == 4) {
				edF32TRIANGLE4* pTriangle = reinterpret_cast<edF32TRIANGLE4*>(pColObj);
				*pOutProps = pTriangle->flags;
				ComputeNormal(pOutNormal, LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p1), LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p2), LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p3));
			}
			else {
				*pOutProps = 0;
				pOutNormal->x = 0.0f;
				pOutNormal->y = -1.0f;
				pOutNormal->z = 0.0f;
				pOutNormal->w = 0.0f;
			}
		}
	}
	return;
}

float CCollisionRay::IntersectScenery(edF32VECTOR4* pOutVector, _ray_info_out* pOutResult)
{
	int iVar1;
	CCollisionManager* pCVar2;
	void* unaff_s0_lo;
	uint finalType;
	edObbTREE_DYN** ppeVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 eStack32;
	void* local_c;
	uint outType;
	uint local_4;

	pCVar2 = CScene::ptable.g_CollisionManager_00451690;
	fVar6 = 1e+30f;
	iVar4 = 0;
	finalType = 0;
	local_4 = 0;
	this->lengthA = this->lengthB.x;
	iVar1 = pCVar2->staticCollisionCount;
	ppeVar3 = pCVar2->aStaticCollisionRefs;
	if (0 < iVar1) {
		do {
			fVar5 = edObbIntersectObbTreeRayPrim(&local_c, &outType, *ppeVar3, this);
			if ((0.0f <= fVar5) && (fVar5 < fVar6)) {
				this->lengthA = fVar5;
				fVar6 = fVar5;
				finalType = outType;
				unaff_s0_lo = local_c;
			}
			iVar4 = iVar4 + 1;
			ppeVar3 = ppeVar3 + 1;
		} while (iVar4 < iVar1);
	}
	if (fVar6 != 1e+30f) {
		if (pOutVector == (edF32VECTOR4*)0x0) {
			pOutVector = &eStack32;
		}
		ComputeIntersectionNormalAndProps(fVar6, unaff_s0_lo, finalType, pOutVector, &local_4);
	}
	if (pOutResult != (_ray_info_out*)0x0) {
		pOutResult->pActor_0x0 = (CActor*)0x0;
		pOutResult->pVector_0x4 = local_4;
		pOutResult->type_0x8 = finalType;
	}
	return fVar6;
}

void CCollisionRay::ChangeLeadVector(edF32VECTOR4* pNewLeadVector)
{
	this->pLeadVector = pNewLeadVector;
}
