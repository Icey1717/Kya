#include "Vision.h"
#include "MathOps.h"
#include "MemoryStream.h"
#include "Actor.h"
#include "ActorFactory.h"
#include "CollisionRay.h"
#include "ActorManager.h"

CVision::CVision()
	: pOwner((CActor*)0x0)
{

}

void CVision::Create(CActor* pOwner, ByteCode* pByteCode)
{
	uint uVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	fVar2 = pByteCode->GetF32();
	fVar3 = pByteCode->GetF32();
	fVar4 = pByteCode->GetF32();
	fVar5 = pByteCode->GetF32();
	uVar1 = pByteCode->GetU32();
	this->visionRange_0x34 = fVar2;
	this->field_0x38 = fVar4 * 0.5f;
	this->field_0x3c = fVar5 * 0.5f;
	fVar2 = fVar3 * 0.5f * 0.01745329f;
	this->flags = 0;
	this->field_0x30 = fVar2;
	this->field_0x44 = cosf(fVar2); // edFCosinus[(int)(fabs(fVar2 * 1303.797) + 0.5) & 0x1fff];
	this->field_0x3c = this->field_0x3c;
	fVar2 = 1.570796f - this->field_0x30;
	this->field_0x40 = this->field_0x3c * sinf(fVar2) / cosf(fVar2); // (edFCosinus[(int)(fabs((fVar2 - 1.570796) * 1303.797) + 0.5) & 0x1fff] / edFCosinus[(int)(fabs(fVar2 * 1303.797) + 0.5) & 0x1fff]);
	this->pActor_0x48 = (CActor*)0x0;
	//this->field_0x4c = 0;
	//this->field_0x54 = 0;
	//this->field_0x50 = 0;
	this->field_0x58 = 0.0f;
	this->flags = uVar1;
	this->pOwner = pOwner;
	return;
}


void CVision::Reset()
{
	//this->pActor_0x48 = (CActor*)0x0;
	//this->field_0x4c = 0;
	//this->field_0x54 = 0;
	//this->field_0x50 = 0;
	//this->field_0x58 = 0.0;
	return;
}

CActor* CVision::ScanForTarget(CActor* pTarget, int mode)
{
	uint uVar1;
	long lVar2;
	long lVar3;
	edF32VECTOR4 eStack16;

	if (pTarget == (CActor*)0x0) {
		pTarget = (CActor*)0x0;
	}
	else {
		lVar2 = pTarget->GetNumVisualDetectionPoints();
		if (((this->flags & 8) == 0) || (lVar3 = pTarget->IsLockable(), lVar3 != 0)) {
			if (((this->flags & 0x10) == 0) || (CActorFactory::gClassProperties[pTarget->typeID].field_0x8 != 0)) {
				if (lVar2 == 1) {
					pTarget->GetVisualDetectionPoint(&eStack16, 0);
					uVar1 = _PointIsDetected(&eStack16, pTarget);
				}
				else {
					IMPLEMENTATION_GUARD(
						uVar1 = _ScanForTargetMultiPoint(this, pTarget, mode);)
				}
				if (uVar1 == 0) {
					pTarget = (CActor*)0x0;
				}
			}
			else {
				pTarget = (CActor*)0x0;
			}
		}
		else {
			pTarget = (CActor*)0x0;
		}
	}

	return pTarget;
}

struct ScanParams
{
	int classId;
	CActor* pOwner;
	CVision* pVision;
	CActorsTable* pTable;
	int mode;
};

void gClusterCallback_AllClassActorsInVision(CActor* pActor, void* pParams)
{
	CActor* pCVar1;

	ScanParams* pScanParams = reinterpret_cast<ScanParams*>(pParams);

	if (((pActor != pScanParams->pOwner) && (pScanParams->classId == pActor->typeID)) &&
		(pCVar1 = pScanParams->pVision->ScanForTarget(pActor, pScanParams->mode), pCVar1 != (CActor*)0x0)) {
		pScanParams->pTable->Add(pActor);
	}

	return;
}

void CVision::ScanFromClassID(int classId, CActorsTable* pTable, int mode)
{
	ScanParams scanParams;
	edF32VECTOR4 scanSphere;

	scanSphere.x = (this->location).x;
	scanSphere.y = (this->location).y;
	scanSphere.z = (this->location).z;
	scanSphere.w = this->field_0x40 + this->visionRange_0x34;

	scanParams.pOwner = this->pOwner;
	scanParams.classId = classId;
	scanParams.pVision = this;
	scanParams.pTable = pTable;
	scanParams.mode = mode;

	(CScene::ptable.g_ActorManager_004516a4)->cluster.ApplyCallbackToActorsIntersectingSphere(&scanSphere, gClusterCallback_AllClassActorsInVision, &scanParams);
	return;
}

bool CVision::SV_PointIsInVision(edF32VECTOR4* v0)
{
	bool bVar1;
	float fVar2;
	float fVar3;
	float t;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	edF32Vector4ScaleHard(this->field_0x40, &local_10, &this->rotationQuat);
	edF32Vector4SubHard(&local_10, &this->location, &local_10);
	fVar3 = this->visionRange_0x34 + this->field_0x40;
	fVar3 = fVar3 * fVar3;
	if (fabs((this->rotationQuat).y) < 0.00125f) {
		edF32Vector4SubHard(&local_20, v0, &local_10);
		bVar1 = false;
		if (fabs(local_20.y) <= this->field_0x38) {
			fVar2 = local_20.z * local_20.z + local_20.x * local_20.x;
			bVar1 = false;
			if ((fVar2 <= fVar3) && (bVar1 = false, this->field_0x40 * this->field_0x40 <= fVar2)) {
				local_40.x = (this->rotationQuat).x;
				local_40.z = (this->rotationQuat).z;
				local_40.w = (this->rotationQuat).w;
				local_40.y = 0.0f;
				local_20.y = 0.0f;
				edF32Vector4NormalizeHard(&local_20, &local_20);
				edF32Vector4NormalizeHard(&local_40, &local_40);
				fVar3 = edF32Vector4DotProductHard(&local_20, &local_40);
				bVar1 = false;
				if (this->field_0x44 <= fVar3) {
					bVar1 = true;
				}
			}
		}
	}
	else {
		edF32Vector4CrossProductHard(&eStack96, &g_xVector, &this->rotationQuat);
		edF32Vector4CrossProductHard(&eStack96, &this->rotationQuat, &eStack96);
		edF32Vector4NormalizeHard(&eStack96, &eStack96);
		t = edDistPointToPlane(&local_10, &eStack96, v0);
		edF32Vector4ScaleHard(t, &local_50, &eStack96);
		edF32Vector4SubHard(&local_50, v0, &local_50);
		edF32Vector4SubHard(&local_30, &local_50, &local_10);
		fVar2 = local_30.z * local_30.z + local_30.x * local_30.x + local_30.y * local_30.y;
		bVar1 = false;
		if (((fVar2 <= fVar3) && (bVar1 = false, this->field_0x40 * this->field_0x40 <= fVar2)) &&
			(bVar1 = true, 1e-05f <= fabs((local_50.z - local_10.z) + (local_50.x - local_10.x) + (local_50.y - local_10.y))))
		{
			edF32Vector4NormalizeHard(&local_30, &local_30);
			fVar3 = edF32Vector4DotProductHard(&local_30, &this->rotationQuat);
			bVar1 = false;
			if ((this->field_0x44 <= fVar3) && (bVar1 = false, fabs(t) <= this->field_0x38)) {
				bVar1 = true;
			}
		}
	}

	return bVar1;
}

bool CVision::_PointIsDetected(edF32VECTOR4* v0, CActor* pTargetActor)
{
	bool bVar1;
	bool bVar2;
	bool bResult;
	uint type;
	float fVar3;
	edF32VECTOR4 vectorResult;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	_ray_info_out traceResult;

	local_20 = *v0;

	bResult = SV_PointIsInVision(&local_20);

	if (bResult != false) {
		bVar1 = (this->flags & 2) != 0;
		type = 0;

		if (bVar1) {
			type = 2;
		}

		bVar2 = (this->flags & 1) != 0;
		if (bVar2) {
			type = type | 1;
		}

		if (bVar2 || bVar1) {
			edF32Vector4SubHard(&eStack48, v0, &this->location);
			eStack48.w = 0.0f;
			fVar3 = edF32Vector4NormalizeHard(&eStack48, &eStack48);
			CCollisionRay traceParams = CCollisionRay(fVar3, &this->location, &eStack48);
			fVar3 = traceParams.Intersect(type, (CActor*)0x0, (CActor*)0x0, 0x40000002, &vectorResult, &traceResult);
			bResult = fVar3 == 1e+30f;
			if (((!bResult) && (traceResult.pActor_0x0 != (CActor*)0x0)) && (traceResult.pActor_0x0 == pTargetActor)) {
				bResult = true;
			}
		}
	}

	return bResult;
}

struct ScanActorsParams
{
	CVision* pVision;
	CActor* pOwner;
	CActorsTable* pTable;
	int mode;
};

void gClusterCallback_AllActorsInVision(CActor* pActor, void* pData)
{
	CActor* pCVar1;

	ScanActorsParams* pParams = reinterpret_cast<ScanActorsParams*>(pData);

	if ((pActor != pParams->pOwner) &&
		(pCVar1 = pParams->pVision->ScanForTarget(pActor, pParams->mode), pCVar1 != (CActor*)0x0)) {
		pParams->pTable->Add(pActor);
	}

	return;
}

void CVision::ScanAccurate(float maxDistance, CActorsTable* pTable, int param_4)
{
	CActor** pCVar1;
	int iVar2;
	float fVar3;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	ScanActorsParams local_20;
	_ray_info_out local_10;

	local_30.xyz = (this->location).xyz;
	local_30.w = this->field_0x40 + this->visionRange_0x34;

	local_20.pOwner = this->pOwner;
	local_20.pVision = this;
	local_20.pTable = pTable;
	local_20.mode = param_4;
	(CScene::ptable.g_ActorManager_004516a4)->cluster.ApplyCallbackToActorsIntersectingSphere(&local_30, gClusterCallback_AllActorsInVision, &local_20);

	CCollisionRay CStack112 =CCollisionRay(1.0f, &this->location, &gF32Vector4UnitZ);
	iVar2 = 0;
	pCVar1 = pTable->aEntries;
	if (0 < pTable->nbEntries) {
		do {
			(*pCVar1)->GetVisualDetectionPoint(&eStack80, 0);
			edF32Vector4SubHard(&eStack64, &eStack80, &this->location);
			eStack64.w = 0.0f;
			fVar3 = edF32Vector4NormalizeHard(&eStack64, &eStack64);
			CStack112.ChangeLeadVector(&eStack64);
			CStack112.ChangeMaxDistance(fVar3 + 1.0f);

			fVar3 = CStack112.Intersect(2, (CActor*)0x0, (CActor*)0x0, 0x40000020, (edF32VECTOR4*)0x0, &local_10);
			if ((local_10.pActor_0x0 != *pCVar1) || (maxDistance < fVar3)) {
				pTable->Pop(iVar2);
			}
			else {
				pCVar1 = pCVar1 + 1;
				iVar2 = iVar2 + 1;
			}
		} while (iVar2 < pTable->nbEntries);
	}

	return;
}