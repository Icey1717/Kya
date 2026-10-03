#include "CircularWaveShoot.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "ActorManager.h"
#include "ActorFactory.h"
#include "CameraManager.h"
#include "FileManager3D.h"
#include "DlistManager.h"
#include "edDlist.h"

// 0x003a0050: only hit actors inside the expanding ring, once per shot.
static bool CriterionCircularWave(CActor* pActor, void* pParams)
{
	CCircularWaveShoot* pWave = static_cast<CCircularWaveShoot*>(pParams);
	edF32VECTOR4 actorPosition = pActor->currentLocation;
	edF32VECTOR4 wavePosition = pWave->field_0x40;
	actorPosition.y = 0.0f;
	wavePosition.y = 0.0f;
	edF32VECTOR4 delta;
	edF32Vector4SubHard(&delta, &actorPosition, &wavePosition);
	float distance = edF32Vector4GetDistHard(&delta);
	edF32VECTOR4* pBottom = pActor->GetBottomPosition();
	float height = pActor->currentLocation.y;

	if (((CActorFactory::gClassProperties[pActor->typeID].flags & 0x10) == 0) ||
		(pWave->field_0x40.y + pWave->field_0x10 <= height) ||
		((pBottom->y - height) + height <= pWave->field_0x40.y) ||
		(pWave->field_0x2c <= distance) || (distance <= pWave->field_0x2c - pWave->field_0xc) ||
		(pActor == pWave->pOwner)) {
		return false;
	}
	return !pWave->actorsTable.IsInList(pActor);
}

CCircularWaveShoot::CCircularWaveShoot()
{
	return;
}

bool CCircularWaveShoot::InitDlistPatchable(int patchId)
{
	return false;
}

void CCircularWaveShoot::Create(ByteCode* pByteCode)
{
	this->field_0x8 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetF32();
	this->field_0x14 = pByteCode->GetF32();
	this->field_0x18 = pByteCode->GetF32();
	this->field_0x1c = pByteCode->GetF32();
	this->field_0x20 = pByteCode->GetS32();
	this->field_0x24 = pByteCode->GetS32();

	return;
}

void CCircularWaveShoot::Init(CActor* pOwner)
{
	this->pOwner = pOwner;
	CActor::SV_InstallMaterialId(this->field_0x20);

	for (int i = 0; i < 4; i++) {
		CFxSparkNoAlloc<2, 10>* pSpark = this->aFxSparks + i;
		pSpark->Create(2, 10, pSpark->aVectorData, pSpark->aFloatData, this->field_0x20);
		pSpark->field_0xe4 = pSpark->aUnknown;
		pSpark->SetParameters(0.5f, 0.5f, 10.0f, 0.05f, 16.0f, 1);
		pSpark->Init(pOwner->sectorId);
		pSpark->vector_0x80 = edF32VECTOR4{ -0.2f, 0.0f, 0.0f, 0.0f };
	}
	Reset();

	return;
}

void CCircularWaveShoot::Reset()
{
	this->field_0x2c = 0.0f;
	this->actorsTable.nbEntries = 0;
	this->field_0x40 = gF32Vector4Zero;
	this->field_0x30 = 0;
	for (int i = 0; i < 4; i++) {
		this->aFxSparks[i].Reset();
	}
	if (this->field_0xb14.IsValid()) {
		this->field_0xb14.Kill();
		this->field_0xb14.Reset();
	}

	return;
}

void CCircularWaveShoot::Fire(edF32VECTOR4* pPosition)
{
	Reset();
	this->pOwner->flags = this->pOwner->flags | 0x80;
	this->pOwner->flags = this->pOwner->flags & 0xffffffdf;
	this->pOwner->EvaluateDisplayState();
	this->pOwner->flags = this->pOwner->flags | 0x400;
	this->field_0x40 = *pPosition;
	this->field_0x30 = 1;

	return;
}

void CCircularWaveShoot::UpdateWaveLife()
{
	if (this->field_0x30 == 0) {
		if (this->field_0xb14.IsValid()) {
			this->field_0xb14.Kill();
			this->field_0xb14.Reset();
		}
	}
	else {
		float radius = this->field_0x2c + this->field_0x14 * Timer::GetTimer()->cutsceneDeltaTime;
		this->field_0x2c = (radius < 0.5f) ? 0.5f : radius;
		SparkPosOnCircle();

		if (this->field_0x24 != -1) {
			if (!this->field_0xb14.IsValid()) {
				CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0xb14, this->field_0x24, FX_MATERIAL_SELECTOR_NONE);
				this->field_0xb14.Start();
			}
			edF32VECTOR4 delta;
			edF32VECTOR4 position;
			edF32Vector4SubHard(&delta, &CScene::ptable.g_CameraManager_0045167c->pActiveCamera->transformationMatrix.rowT, &this->field_0x40);
			delta.y = 0.0f;
			edF32Vector4NormalizeHard(&delta, &delta);
			edF32Vector4ScaleHard(this->field_0x2c, &delta, &delta);
			edF32Vector4AddHard(&position, &this->field_0x40, &delta);
			this->field_0xb14.SetPosition(&position);
		}

		edF32VECTOR4 sphere = this->field_0x40;
		sphere.w = this->field_0x2c;
		CActorsTable actors;
		CScene::ptable.g_ActorManager_004516a4->cluster.GetActorsIntersectingSphereWithCriterion(&actors, &sphere, CriterionCircularWave, this);
		for (int i = 0; i < actors.nbEntries; i++) {
			CActor* pActor = actors.aEntries[i];
			this->actorsTable.Add(pActor);
			_msg_hit_param hitParam = {};
			hitParam.projectileType = 1;
			edF32Vector4SubHard(&hitParam.field_0x20, &this->field_0x40, &pActor->currentLocation);
			edF32Vector4NormalizeHard(&hitParam.field_0x20, &hitParam.field_0x20);
			hitParam.field_0x20.y = 0.0f;
			edF32Vector4NormalizeHard(&hitParam.field_0x20, &hitParam.field_0x20);
			hitParam.damage = this->field_0x18;
			hitParam.field_0x30 = this->field_0x1c;
			this->pOwner->DoMessage(pActor, MESSAGE_KICKED, &hitParam);
		}
		if (this->field_0x8 <= this->field_0x2c) {
			this->pOwner->flags = this->pOwner->flags & 0xfffffbff;
			this->pOwner->flags = this->pOwner->flags & 0xffffff5f;
			this->pOwner->EvaluateDisplayState();
			Reset();
		}
	}

	return;
}

void CCircularWaveShoot::SparkPosOnCircle()
{
	float radius = this->field_0x2c - this->field_0xc / 2.0f;
	if (radius < 0.5f) {
		radius = 0.5f;
	}
	edF32VECTOR4 center = this->field_0x40;
	center.y = center.y + this->field_0x10 / 2.0f;

	for (int i = 0; i < 4; i++) {
		edF32VECTOR4 directionA;
		edF32VECTOR4 directionB;
		edF32VECTOR4 directionMid;
		edF32VECTOR4 offset;
		edF32VECTOR4 midPosition;
		edF32VECTOR4 positionA;
		edF32VECTOR4 positionB;
		edF32VECTOR4 delta;
		edF32VECTOR4 sparkStart;
		edF32VECTOR4 sparkEnd;
		edF32MATRIX4 matrix;
		SetVectorFromAngleY((float)i * 1.570796f, &directionA);
		SetVectorFromAngleY((float)(i + 1) * 1.570796f, &directionB);
		SetVectorFromAngleY(((float)i * 2.0f + 1.0f) * 1.570796f / 2.0f, &directionMid);
		edF32Vector4ScaleHard(radius, &offset, &directionMid);
		edF32Vector4AddHard(&midPosition, &center, &offset);
		edF32Vector4ScaleHard(radius, &offset, &directionA);
		edF32Vector4AddHard(&positionA, &center, &offset);
		edF32Vector4ScaleHard(radius, &offset, &directionB);
		edF32Vector4AddHard(&positionB, &center, &offset);
		edF32Vector4SubHard(&delta, &positionB, &positionA);
		float length = edF32Vector4NormalizeHard(&delta, &delta);
		edF32Vector4ScaleHard(this->field_0x8 / 2.0f, &delta, &delta);
		edF32Vector4AddHard(&sparkEnd, &midPosition, &delta);
		edF32Vector4SubHard(&sparkStart, &midPosition, &delta);
		this->aFxSparks[i].Manage(&sparkStart, &sparkEnd);
		edF32Matrix4MulF32Hard(length / this->field_0x8, &matrix, &this->aFxSparks[i].field_0x20);
		matrix.rowT = positionA;
		this->aFxSparks[i].field_0x20 = matrix;
	}

	return;
}

void CCircularWaveShoot::Draw()
{
	for (int i = 0; i < 4; i++) {
		this->aFxSparks[i].Draw(this->field_0x30 != 0);
	}
	if ((this->field_0x30 != 0) && GameDList_BeginCurrent()) {
		edDList_material* pMaterial = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(CScene::_pinstance->defaultTextureIndex_0x28, 2);
		edDListLoadIdentity();
		edDListUseMaterial(pMaterial);
		for (int i = 0; i < 60; i++) {
			DrawWavePart(0.1047198f, i);
		}
		GameDList_EndCurrent();
	}

	return;
}

void CCircularWaveShoot::DrawWavePart(float angle, int part)
{
	edDListBegin(0.0f, 0.0f, 0.0f, 4, 6);
	float outerRadius = this->field_0x2c;
	float innerRadius = outerRadius - this->field_0xc;
	if (innerRadius < 0.0f) {
		innerRadius = 0.0f;
	}
	float middleRadius = (innerRadius + outerRadius) / 2.0f;
	edF32VECTOR4 directionA;
	edF32VECTOR4 directionB;
	SetVectorFromAngleY(angle * (float)part, &directionA);
	SetVectorFromAngleY(angle * (float)(part + 1), &directionB);
	edDListColor4u8(0, 0, 0x3c, 0x5a);
	edDListVertex4f(directionA.x * innerRadius + this->field_0x40.x, this->field_0x40.y, directionA.z * innerRadius + this->field_0x40.z, 0.0f);
	edDListVertex4f(directionB.x * innerRadius + this->field_0x40.x, this->field_0x40.y, directionB.z * innerRadius + this->field_0x40.z, 0.0f);
	edDListColor4u8(0, 0, 0x7d, 0x5a);
	edDListVertex4f(directionA.x * middleRadius + this->field_0x40.x, this->field_0x40.y + this->field_0x10, directionA.z * middleRadius + this->field_0x40.z, 0.0f);
	edDListVertex4f(directionB.x * middleRadius + this->field_0x40.x, this->field_0x40.y + this->field_0x10, directionB.z * middleRadius + this->field_0x40.z, 0.0f);
	edDListColor4u8(0, 0, 0x3c, 0x5a);
	edDListVertex4f(directionA.x * outerRadius + this->field_0x40.x, this->field_0x40.y, directionA.z * outerRadius + this->field_0x40.z, 0.0f);
	edDListVertex4f(directionB.x * outerRadius + this->field_0x40.x, this->field_0x40.y, directionB.z * outerRadius + this->field_0x40.z, 0.0f);
	edDListEnd();

	return;
}
