#include "ActorRope.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "CollisionManager.h"
#include "FileManager3D.h"
#include "PathManager.h"

void CActorRope::Create(ByteCode* pByteCode)
{
	S_NTF_TARGET_STREAM_REF* pSVar1;
	S_STREAM_EVENT_CAMERA* pSVar2;
	C3DFileManager* pFileManager;
	CPathManager* pPathManager;
	int iVar4;
	CPathFollow* pCVar5;
	_rgba _Var6;
	CSound* pCVar7;
	CActor* pCVar8;
	uint uVar9;
	CActorSound* pCVar10;
	DisplayList* pDVar11;
	edDList_material* pMaterialInfo;
	float fVar12;

	CActor::Create(pByteCode);

	pPathManager = CScene::ptable.g_PathManager_004516a0;
	pFileManager = CScene::ptable.g_C3DFileManager_00451664;

	iVar4 = pByteCode->GetS32();
	if (iVar4 == -1) {
		pCVar5 = (CPathFollow*)0x0;
	}
	else {
		pCVar5 = pPathManager->aPathFollow + iVar4;
	}
	this->pPathFollow = pCVar5;

	this->pDisplayList = (DisplayList*)0x0;

	this->ropeScale = pByteCode->GetF32();
	this->field_0x17c = pByteCode->GetF32();

	this->ropeMaterialId = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(this->ropeMaterialId);

	this->ropeColor.rgba = pByteCode->GetU32();

	(this->soundStreamRef).index = pByteCode->GetS32();
	this->field_0x194 = pByteCode->GetS32();

	(this->actorStreamRefA).index = pByteCode->GetS32();
	(this->actorStreamRefB).index = pByteCode->GetS32();

	this->targetSwitch.Create(pByteCode);

	this->ropeFlags = pByteCode->GetU32();
	this->field_0x188 = 0;
	if ((this->soundStreamRef).index == -1) {
		this->pActorSound = (CActorSound*)0x0;
	}
	else {
		this->pActorSound = CreateActorSound(1);
	}

	SV_ComputeDiffMatrixFromInit(&this->field_0x1a0);

	if (this->pPathFollow != (CPathFollow*)0x0) {
		if ((this->ropeFlags & 2) == 0) {
			BuildCollisionPrims();
		}

		PrecomputeDrawParams();

		pDVar11 = edDListNew(TO_HEAP(H_MAIN), 5, 3, this->nbSegments * 4 + 0x10, 0, 0, (DisplayList*)0x0);
		this->pDisplayList = pDVar11;
		if (this->pDisplayList != (DisplayList*)0x0) {
			edDListSetSceneUsed(this->pDisplayList, CScene::_scene_handleA);
			edDListSetCurrent(this->pDisplayList);
			edDListLoadIdentity();
			edDListUseMaterial(pFileManager->GetMaterialFromId(this->ropeMaterialId, 0));
			DrawTightened();
		}
	}

	return;
}

void CActorRope::Init()
{
	CActor::Init();

	this->soundStreamRef.Init();
	this->actorStreamRefA.Init();
	this->actorStreamRefB.Init();

	this->targetSwitch.Init();

	if (this->pPathFollow != (CPathFollow*)0x0) {
		TieActors(1);
	}

	return;
}

void CActorRope::Term()
{
	return;
}

void CActorRope::Draw()
{
	float fVar1;
	StateConfig* pSVar2;
	uint uVar3;
	int iVar4;
	edF32MATRIX4* peVar5;
	edF32MATRIX4* peVar6;
	edF32MATRIX4 eStack128[2];

	if (this->pPathFollow != (CPathFollow*)0x0) {
		uVar3 = GetStateFlags(this->actorState) & 0x300;

		if (uVar3 != 0) {
			if (this->pTiedActor == (CActor*)0x0) {
				if (this->pDisplayList != (DisplayList*)0x0) {
					edDlistAddtoView(this->pDisplayList);
				}
			}
			else {
				peVar6 = &this->field_0x1a0;
				if (this->pDisplayList != (DisplayList*)0x0) {
					eStack128[0] = this->field_0x1a0;
					eStack128[1] = eStack128[0];
					edDListChangeMatrix(this->pDisplayList, 2, eStack128);
					edDlistAddtoView(this->pDisplayList);
				}
			}
		}
	}

	return;
}

void CActorRope::Reset()
{
	S_NTF_TARGET_STREAM_REF* pSVar1;
	int iVar2;
	int iVar3;
	CCollision* pCollision;

	CActor::Reset();
	pCollision = this->pCollisionData;
	if (pCollision != (CCollision*)0x0) {
		pCollision->flags_0x0 = pCollision->flags_0x0 | 0x1000;
	}

	if (this->pPathFollow != (CPathFollow*)0x0) {
		TieActors(1);
	}

	this->targetSwitch.Reset(this);

	return;
}

void CActorRope::SaveContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

void CActorRope::LoadContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorRope::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == ROPE_BEHAVIOUR_STAND) {
		pBehaviour = &this->behaviourRopeStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorRope::_gStateCfg_ROP[2] =
{
	StateConfig(0x0, 0x104),
	StateConfig(0x0, 0x200)
};

CActorRope::~CActorRope()
{
	if (this->pDisplayList != (DisplayList*)0x0) {
		edDListDelete(this->pDisplayList);
	}

	return;
}

StateConfig* CActorRope::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 6);
		pStateConfig = _gStateCfg_ROP + state + -5;
	}

	return pStateConfig;
}

bool CActorRope::IsLockable()
{
	bool bLockable;

	bLockable = (this->ropeFlags & 2) == 0;

	if (bLockable) {
		bLockable = CActor::IsLockable();
		bLockable = bLockable != false;
	}

	return bLockable;
}

int CActorRope::GetNumVisualDetectionPoints()
{
	return this->pPathFollow->splinePointCount + -1;
}

void CActorRope::GetVisualDetectionPoint(edF32VECTOR4* pOutPoint, int index)
{
	edF32VECTOR4* pCurPoint;
	edF32VECTOR4* pSplinePoints;

	pSplinePoints = this->pPathFollow->aSplinePoints;
	if (pSplinePoints == (edF32VECTOR4*)0x0) {
		pCurPoint = &gF32Vertex4Zero;
	}
	else {
		pCurPoint = pSplinePoints + index;
	}

	if (pSplinePoints == (edF32VECTOR4*)0x0) {
		pSplinePoints = &gF32Vertex4Zero;
	}
	else {
		pSplinePoints = pSplinePoints + index + 1;
	}

	edF32Vector4AddHard(pOutPoint, pCurPoint, pSplinePoints);
	edF32Vector4ScaleHard(0.5f, pOutPoint, pOutPoint);

	if (this->pTiedActor != (CActor*)0x0) {
		edF32Matrix4MulF32Vector4Hard(pOutPoint, &this->field_0x1a0, pOutPoint);
	}

	return;
}

struct Dummy
{
	edF32VECTOR4 a;
	edF32VECTOR4 b;
	edF32VECTOR4 c;
};

void CActorRope::BuildCollisionPrims()
{
	Dummy* pAlloc;
	CCollision* pCollision;
	edF32VECTOR4* peVar1;
	int curIndex;
	edF32VECTOR4* pitchAngles;
	edF32VECTOR4* peVar4;
	edF32VECTOR4* v0;
	int nbSplinePoints;
	edF32VECTOR4* peVar6;
	float fVar7;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32MATRIX4 eStack64;

	edF32Matrix4FromEulerSoft(&eStack64, &(this->pCinData)->rotationEuler, "XYZ");
	eStack64.rowT = this->baseLocation;
	edF32Matrix4GetInverseOrthoHard(&eStack64, &eStack64);
	nbSplinePoints = this->pPathFollow->splinePointCount + -1;
	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);

	pAlloc = new Dummy[nbSplinePoints];

	peVar6 = reinterpret_cast<edF32VECTOR4*>(pAlloc);
	edF32VECTOR4* peVar7 = reinterpret_cast<edF32VECTOR4*>(reinterpret_cast<char*>(peVar6) + sizeof(edF32VECTOR4) * nbSplinePoints);
	edF32VECTOR4* peVar8 = reinterpret_cast<edF32VECTOR4*>(reinterpret_cast<char*>(peVar7) + sizeof(edF32VECTOR4) * nbSplinePoints);

	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);

	curIndex = 0;
	if (0 < nbSplinePoints) {
		v0 = peVar6;
		peVar4 = peVar7;
		pitchAngles = peVar8;

		do {
			peVar1 = this->pPathFollow->aSplinePoints;
			if (peVar1 == (edF32VECTOR4*)0x0) {
				peVar1 = &gF32Vertex4Zero;
			}
			else {
				peVar1 = peVar1 + curIndex;
			}

			edF32Matrix4MulF32Vector4Hard(&eStack96, &eStack64, peVar1);

			peVar1 = this->pPathFollow->aSplinePoints;
			if (peVar1 == (edF32VECTOR4*)0x0) {
				peVar1 = &gF32Vertex4Zero;
			}
			else {
				peVar1 = peVar1 + curIndex + 1;
			}

			edF32Matrix4MulF32Vector4Hard(&eStack112, &eStack64, peVar1);
			edF32Vector4AddHard(v0, &eStack96, &eStack112);
			edF32Vector4ScaleHard(0.5f, v0, v0);
			v0->w = 1.0f;
			edF32Vector4SubHard(&eStack80, &eStack112, &eStack96);
			peVar4->x = 0.75f;
			peVar4->y = 0.75f;
			fVar7 = edF32Vector4GetDistHard(&eStack80);
			peVar4->z = fVar7 * 0.5f + 0.75f;
			peVar4->w = 1.0f;
			GetAnglesFromVector(&pitchAngles->xyz, &eStack80);
			curIndex = curIndex + 1;
			pitchAngles->w = 0.0f;
			v0 = v0 + 1;
			peVar4 = peVar4 + 1;
			pitchAngles = pitchAngles + 1;
		} while (curIndex < nbSplinePoints);
	}

	pCollision = CScene::ptable.g_CollisionManager_00451690->NewCCollision();
	pCollision->Create(this, nbSplinePoints, 1, peVar7, peVar6, peVar8);
	this->pCollisionData = pCollision;
	pCollision->flags_0x0 = pCollision->flags_0x0 | 0x2800;
	CCollision::PatchObbTreeFlagsRecurse(pCollision->pObbTree, 4, -1, 0);
	UpdatePosition( &this->currentLocation, true);

	delete(pAlloc);

	return;
}

void CActorRope::PrecomputeDrawParams()
{
	int iVar1;
	edF32VECTOR4* v1;
	edF32VECTOR4* peVar2;
	int iVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	iVar4 = 0;
	local_20.x = 0.0f;
	local_20.y = 0.0f;
	local_20.z = 0.0f;
	local_20.w = 0.0f;

	iVar1 = this->pPathFollow->splinePointCount;
	if (0 < iVar1) {
		do {
			peVar2 = this->pPathFollow->aSplinePoints;
			if (peVar2 == (edF32VECTOR4*)0x0) {
				peVar2 = &gF32Vertex4Zero;
			}
			else {
				peVar2 = peVar2 + iVar4;
			}

			edF32Vector4AddHard(&local_20, &local_20, peVar2);
			iVar4 = iVar4 + 1;
		} while (iVar4 < iVar1);
	}

	edF32Vector4ScaleHard(1.0f / (float)iVar1, &local_20, &local_20);

	fVar9 = 0.0f;
	iVar4 = 0;

	if (0 < iVar1) {
		fVar8 = fVar9;

		do {
			peVar2 = this->pPathFollow->aSplinePoints;
			if (peVar2 == (edF32VECTOR4*)0x0) {
				peVar2 = &gF32Vertex4Zero;
			}
			else {
				peVar2 = peVar2 + iVar4;
			}

			edF32Vector4SubHard(&eStack16, &local_20, peVar2);

			fVar9 = edF32Vector4GetDistHard(&eStack16);
			if (fVar9 <= fVar8) {
				fVar9 = fVar8;
			}

			iVar4 = iVar4 + 1;
			fVar8 = fVar9;
		} while (iVar4 < iVar1);
	}

	edF32Matrix4FromEulerSoft(&eStack96, &(this->pCinData)->rotationEuler, "XYZ");
	eStack96.rowT = this->baseLocation;

	edF32Matrix4GetInverseOrthoHard(&eStack96, &eStack96);
	edF32Matrix4MulF32Vector4Hard(&local_20, &eStack96, &local_20);
	SetLocalBoundingSphere(fVar9 + 1.299038f, &local_20);

	fVar9 = -8.0f;
	this->nbSegments = 0;
	iVar4 = 0;
	if (0 < iVar1 + -1) {
		do {
			this->nbSegments = this->nbSegments + 2;

			peVar2 = this->pPathFollow->aSplinePoints;
			if (peVar2 == (edF32VECTOR4*)0x0) {
				v1 = &gF32Vertex4Zero;
				peVar2 = &gF32Vertex4Zero;
			}
			else {
				v1 = peVar2 + iVar4 + 1;
				peVar2 = peVar2 + iVar4;
			}

			edF32Vector4SubHard(&eStack16, v1, peVar2);

			fVar6 = edF32Vector4GetDistHard(&eStack16);
			fVar8 = this->field_0x17c;
			fVar7 = fVar6 / fVar8;
			fVar5 = fVar9 + fVar7;

			while (7.9f <= fVar5) {
				fVar9 = (7.9f - fVar9) * fVar8;
				this->nbSegments = this->nbSegments + 2;
				fVar8 = this->field_0x17c;
				fVar6 = fVar6 - fVar9;
				fVar9 = -8.0f;
				fVar7 = fVar6 / fVar8;
				fVar5 = fVar7 + -8.0f;
			}

			iVar4 = iVar4 + 1;
			fVar9 = fVar9 + fVar7;
		} while (iVar4 < iVar1 + -1);
	}

	return;
}

struct edF32VECTOR4Pair
{
	edF32VECTOR4 a;
	edF32VECTOR4 b;
};

void CActorRope::DrawTightened()
{
	edF32VECTOR4* pAlloc;
	edF32VECTOR4* v1;
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR4* peVar4;
	int iVar5;
	int iVar6;
	float fVar8;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	iVar5 = this->pPathFollow->splinePointCount + -1;
	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	pAlloc = (edF32VECTOR4*)new edF32VECTOR4Pair[iVar5];
	peVar4 = pAlloc + iVar5;
	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
	iVar6 = 0;
	if (0 < iVar5) {
		peVar3 = pAlloc;
		peVar2 = peVar4;
		do {
			peVar1 = this->pPathFollow->aSplinePoints;
			if (peVar1 == (edF32VECTOR4*)0x0) {
				v1 = &gF32Vertex4Zero;
				peVar1 = &gF32Vertex4Zero;
			}
			else {
				v1 = peVar1 + iVar6 + 1;
				peVar1 = peVar1 + iVar6;
			}

			edF32Vector4SubHard(&local_10, v1, peVar1);
			edF32Vector4NormalizeHard(&eStack32, &local_10);
			local_10.y = 0.0;
			edF32Vector4SafeNormalize1Hard(&local_10, &local_10);
			peVar3->x = local_10.z;
			peVar3->y = 0.0;
			peVar3->z = -local_10.x;
			peVar3->w = 0.0;
			edF32Vector4CrossProductHard(peVar2, peVar3, &eStack32);
			edF32Vector4ScaleHard(this->ropeScale, peVar2, peVar2);
			edF32Vector4ScaleHard(this->ropeScale, peVar3, peVar3);
			iVar6 = iVar6 + 1;
			peVar3 = peVar3 + 1;
			peVar2 = peVar2 + 1;
		} while (iVar6 < iVar5);
	}

	edDListBegin(this->sphereCentre.x, this->sphereCentre.y, this->sphereCentre.z, 4, this->nbSegments << 1);
	edDListColor4u8((this->ropeColor).r, (this->ropeColor).g, (this->ropeColor).b, (this->ropeColor).a);
	fVar8 = -8.0f;
	iVar6 = 0;
	if (0 < iVar5) {
		peVar3 = pAlloc;
		do {
			peVar2 = this->pPathFollow->aSplinePoints;
			if (peVar2 == (edF32VECTOR4*)0x0) {
				peVar1 = &gF32Vertex4Zero;
				peVar2 = &gF32Vertex4Zero;
			}
			else {
				peVar1 = peVar3 + iVar6;
				peVar2 = peVar2 + iVar6 + 1;
			}
			fVar8 = DrawRopeSegment(fVar8, peVar1, peVar2, peVar3);
			iVar6 = iVar6 + 1;
			peVar3 = peVar3 + 1;
		} while (iVar6 < iVar5);
	}
	edDListEnd();

	edDListBegin(this->sphereCentre.x, this->sphereCentre.y, this->sphereCentre.z, 4, this->nbSegments << 1);
	edDListColor4u8((this->ropeColor).r, (this->ropeColor).g, (this->ropeColor).b, (this->ropeColor).a);
	fVar8 = -8.0f;
	iVar6 = 0;
	if (0 < iVar5) {
		do {
			peVar3 = this->pPathFollow->aSplinePoints;
			if (peVar3 == (edF32VECTOR4*)0x0) {
				peVar2 = &gF32Vertex4Zero;
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar2 = peVar3 + iVar6;
				peVar3 = peVar3 + iVar6 + 1;
			}
			fVar8 = DrawRopeSegment(fVar8, peVar2, peVar3, peVar4);
			iVar6 = iVar6 + 1;
			peVar4 = peVar4 + 1;
		} while (iVar6 < iVar5);
	}
	edDListEnd();

	delete(pAlloc);

	return;
}

float CActorRope::DrawRopeSegment(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, param_4, param_3);
	fVar2 = edF32Vector4GetDistHard(&eStack16);
	edDListTexCoo2f(param_1, 0.0f);
	edF32Vector4SubHard(&local_30, param_3, param_5);
	edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
	edDListTexCoo2f(param_1, 1.0f);
	edF32Vector4AddHard(&local_30, param_3, param_5);
	edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);

	fVar4 = fVar2 / this->field_0x17c;
	if (7.9f <= param_1 + fVar4) {
		edF32Vector4NormalizeHard(&eStack32, &eStack16);
		local_40 = *param_3;

		fVar3 = this->field_0x17c;
		fVar4 = fVar2 / fVar3;
		fVar1 = param_1 + fVar4;
		while (7.9f <= fVar1) {
			fVar4 = (7.9f - param_1) * fVar3;
			edF32Vector4ScaleHard(fVar4, &local_30, &eStack32);
			edF32Vector4AddHard(&local_40, &local_40, &local_30);
			edDListTexCoo2f(7.9f, 0.0f);
			edF32Vector4SubHard(&local_30, &local_40, param_5);
			edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
			edDListTexCoo2f(7.9f, 1.0f);
			edF32Vector4AddHard(&local_30, &local_40, param_5);
			edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
			edDListTexCoo2f(-8.0f, 0.0f);
			edF32Vector4SubHard(&local_30, &local_40, param_5);
			edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
			edDListTexCoo2f(-8.0f, 1.0f);
			edF32Vector4AddHard(&local_30, &local_40, param_5);
			edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
			fVar3 = this->field_0x17c;
			fVar2 = fVar2 - fVar4;
			param_1 = -8.0f;
			fVar4 = fVar2 / fVar3;
			fVar1 = fVar4 + -8.0f;
		}
	}

	fVar4 = param_1 + fVar4;

	edDListTexCoo2f(fVar4, 0.0f);
	edF32Vector4SubHard(&local_30, param_4, param_5);
	edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
	edDListTexCoo2f(fVar4, 1.0f);
	edF32Vector4AddHard(&local_30, param_4, param_5);
	edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);

	return fVar4;
}

int CActorRope::GetNearestSegment(edF32VECTOR4* pPosition)
{
	float fVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	edF32VECTOR4* peVar5;
	edF32VECTOR4* peVar6;
	float fVar7;
	float fVar8;
	float minDistance;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	minDistance = 1e+30f;
	iVar3 = 0;
	for (iVar4 = 0; iVar4 < this->pPathFollow->splinePointCount + -1; iVar4 = iVar4 + 1) {
		peVar5 = this->pPathFollow->aSplinePoints;
		if (peVar5 == (edF32VECTOR4*)0x0) {
			peVar5 = &gF32Vertex4Zero;
		}
		else {
			peVar5 = peVar5 + iVar4;
		}

		peVar6 = this->pPathFollow->aSplinePoints;
		if (peVar6 == (edF32VECTOR4*)0x0) {
			peVar6 = &gF32Vertex4Zero;
		}
		else {
			peVar6 = peVar6 + iVar4 + 1;
		}

		edF32Vector4SubHard(&local_10, pPosition, peVar5);
		edF32Vector4SubHard(&local_20, peVar6, peVar5);

		fVar1 = local_20.x * local_20.x + local_20.y * local_20.y + local_20.z * local_20.z;
		fVar8 = local_10.x * local_20.x + local_10.y * local_20.y + local_10.z * local_20.z;
		fVar7 = minDistance;
		iVar2 = iVar3;

		if ((-1.0f <= fVar8) && (fVar8 <= fVar1 + 1.0f + sqrtf(fVar1))) {
			fVar8 = fVar8 / fVar1;
			local_30.x = (local_20.x * fVar8 + peVar5->x) - pPosition->x;
			local_30.y = (local_20.y * fVar8 + peVar5->y) - pPosition->y;
			local_30.z = (local_20.z * fVar8 + peVar5->z) - pPosition->z;
			local_30.w = (local_20.w * fVar8 + peVar5->w) - pPosition->w;

			fVar7 = edF32Vector4GetDistHard(&local_30);
			iVar2 = iVar4;
			if (minDistance <= fVar7) {
				fVar7 = minDistance;
				iVar2 = iVar3;
			}
		}

		minDistance = fVar7;
		iVar3 = iVar2;
	}

	return iVar3;
}



void CActorRope::TieActors(int bTied)
{
	S_NTF_TARGET_STREAM_REF* pSVar1;
	edF32VECTOR4* peVar2;
	int iVar3;
	int iVar4;
	edF32VECTOR4 eStack48;
	_msg_tied_params tiedParams;

	tiedParams.bTied = bTied != 0 ^ 1;
	this->field_0x188 = tiedParams.bTied;

	if (this->pTiedActor == (CActor*)0x0) {
		tiedParams.pPosition = this->pPathFollow->aSplinePoints;
		if (tiedParams.pPosition == (edF32VECTOR4*)0x0) {
			tiedParams.pPosition = &gF32Vertex4Zero;
		}
		DoMessage((this->actorStreamRefA).Get(), MESSAGE_TIED, &tiedParams);

		tiedParams.pPosition = this->pPathFollow->aSplinePoints;
		if (tiedParams.pPosition == (edF32VECTOR4*)0x0) {
			tiedParams.pPosition = &gF32Vertex4Zero;
		}
		else {
			tiedParams.pPosition = tiedParams.pPosition + this->pPathFollow->splinePointCount + -1;
		}
		DoMessage((this->actorStreamRefB).Get(), MESSAGE_TIED, &tiedParams);
	}
	else {
		tiedParams.pPosition = &eStack48;
		peVar2 = this->pPathFollow->aSplinePoints;
		if (peVar2 == (edF32VECTOR4*)0x0) {
			peVar2 = &gF32Vertex4Zero;
		}
		edF32Matrix4MulF32Vector4Hard(&eStack48, &this->field_0x1a0, peVar2);
		DoMessage((this->actorStreamRefA).Get(), MESSAGE_TIED, &tiedParams);

		peVar2 = this->pPathFollow->aSplinePoints;
		if (peVar2 == (edF32VECTOR4*)0x0) {
			peVar2 = &gF32Vertex4Zero;
		}
		else {
			peVar2 = peVar2 + this->pPathFollow->splinePointCount + -1;
		}

		edF32Matrix4MulF32Vector4Hard(&eStack48, &this->field_0x1a0, peVar2);
		DoMessage((this->actorStreamRefB).Get(), MESSAGE_TIED, &tiedParams);
	}

	if (bTied == 0) {
		this->targetSwitch.Switch(this);
		this->targetSwitch.PostSwitch(this);
	}

	return;
}

void CBehaviourRopeStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourRopeStand::Manage()
{
	int iVar1;
	edF32VECTOR4 local_10;
	CActorRope* pRope;

	pRope = this->pOwner;
	
	pRope->targetSwitch.pStreamEventCamera->Manage(pRope);

	iVar1 = pRope->actorState;
	if (iVar1 == ROPE_STAND_STATE_SNAPPED) {
		if ((((pRope->targetSwitch).pStreamEventCamera)->field_0x4 & 0x40000000U) == 0) {
			pRope->flags = pRope->flags & 0xfffffffd;
			pRope->flags = pRope->flags | 1;
			pRope->flags = pRope->flags & 0xffffff7f;
			pRope->flags = pRope->flags | 0x20;
			pRope->EvaluateDisplayState();
		}
	}
	else {
		if ((iVar1 == 5) && (pRope->pTiedActor != (CActor*)0x0)) {
			local_10 = pRope->baseLocation;

			pRope->SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
			pRope->SV_ComputeDiffMatrixFromInit(&pRope->field_0x1a0);

			if (pRope->pPathFollow != (CPathFollow*)0x0) {
				pRope->TieActors(1);
			}
		}
	}

	return;

}

void CBehaviourRopeStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorRope*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}
}

int CBehaviourRopeStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorRope* pRope;
	CSound* pCVar2;
	CCollision* pCVar3;
	int iVar4;
	StateConfig* pSVar5;
	uint uVar6;
	edF32VECTOR4* peVar7;
	edF32VECTOR4* peVar8;
	edF32VECTOR4 eStack80;
	edF32MATRIX4 eStack64;

	if ((((msg == 3) || (msg == 0xe)) || (msg == 0x10)) || (msg == 0xf)) {
		pRope = this->pOwner;
		uVar6 = pRope->GetStateFlags(pRope->actorState) & 0x100;

		if ((uVar6 != 0) && ((pRope->ropeFlags & 1) == 0)) {
			pCVar2 = (pRope->soundStreamRef).Get();
			if (pCVar2 != (CSound*)0x0) {
				pRope->pActorSound->SoundStart(pRope, 0, pCVar2, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
			}

			pCVar3 = pRope->pCollisionData;
			if (pCVar3 != (CCollision*)0x0) {
				pCVar3->flags_0x0 = pCVar3->flags_0x0 & 0xffffefff;
			}

			pRope->SetState(ROPE_STAND_STATE_SNAPPED, -1);

			if (pRope->pPathFollow != (CPathFollow*)0x0) {
				pRope->TieActors(0);
			}
		}

		iVar4 = 1;
	}
	else {
		if (msg == MESSAGE_KICKED) {
			_msg_hit_param* pHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
			if (pHitParam->projectileType == HIT_TYPE_BOOMY) {
				pRope = this->pOwner;
				if ((pRope->ropeFlags & 2) == 0) {
					uVar6 = pRope->GetStateFlags(pRope->actorState) & 0x100;

					if ((uVar6 != 0) && ((pRope->ropeFlags & 1) == 0)) {
						pCVar2 = (pRope->soundStreamRef).Get();
						if (pCVar2 != (CSound*)0x0) {
							pRope->pActorSound->SoundStart(pRope, 0, pCVar2, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
						}

						pCVar3 = pRope->pCollisionData;
						if (pCVar3 != (CCollision*)0x0) {
							pCVar3->flags_0x0 = pCVar3->flags_0x0 & 0xffffefff;
						}

						pRope->SetState(ROPE_STAND_STATE_SNAPPED, -1);

						if (pRope->pPathFollow != (CPathFollow*)0x0) {
							pRope->TieActors(0);
						}
					}
				}

				return 1;
			}
		}
		else {
			GetPositionMsgParams* pGetPosMsgParam = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
			if ((msg == MESSAGE_GET_VISUAL_DETECTION_POINT) && ((pGetPosMsgParam->field_0x0 == 1 || (pGetPosMsgParam->field_0x0 == 0)))) {
				pRope = this->pOwner;
				if (pRope->pTiedActor == (CActor*)0x0) {
					iVar4 = pRope->GetNearestSegment(&pGetPosMsgParam->vectorFieldA);
					peVar8 = this->pOwner->pPathFollow->aSplinePoints;
					if (peVar8 == (edF32VECTOR4*)0x0) {
						peVar7 = &gF32Vertex4Zero;
						peVar8 = &gF32Vertex4Zero;
					}
					else {
						peVar7 = peVar8 + iVar4;
						peVar8 = peVar8 + iVar4 + 1;
					}

					edF32Vector4AddHard(&pGetPosMsgParam->vectorFieldB, peVar7, peVar8);
					edF32Vector4ScaleHard(0.5f, &pGetPosMsgParam->vectorFieldB, &pGetPosMsgParam->vectorFieldB);
					edF32Vector4SubHard(&pGetPosMsgParam->vectorFieldB, &pGetPosMsgParam->vectorFieldB, &this->pOwner->currentLocation);
				}
				else {
					edF32Matrix4GetInverseOrthoHard(&eStack64, &pRope->field_0x1a0);
					edF32Matrix4MulF32Vector4Hard(&eStack80, &eStack64, &pGetPosMsgParam->vectorFieldA);
					iVar4 = this->pOwner->GetNearestSegment(&eStack80);
					peVar8 = this->pOwner->pPathFollow->aSplinePoints;
					if (peVar8 == (edF32VECTOR4*)0x0) {
						peVar7 = &gF32Vertex4Zero;
						peVar8 = &gF32Vertex4Zero;
					}
					else {
						peVar7 = peVar8 + iVar4;
						peVar8 = peVar8 + iVar4 + 1;
					}

					edF32Vector4AddHard(&pGetPosMsgParam->vectorFieldB, peVar7, peVar8);
					edF32Vector4ScaleHard(0.5f, &pGetPosMsgParam->vectorFieldB, &pGetPosMsgParam->vectorFieldB);
					edF32Matrix4MulF32Vector4Hard(&pGetPosMsgParam->vectorFieldB, &this->pOwner->field_0x1a0, &pGetPosMsgParam->vectorFieldA);
					edF32Vector4SubHard(&pGetPosMsgParam->vectorFieldB, &pGetPosMsgParam->vectorFieldB, &this->pOwner->currentLocation);
				}

				return 1;
			}
		}

		iVar4 = 0;
	}

	return iVar4;
}
