#include "ActorShootService.h"
#include "Actor.h"
#include "ActorFactory.h"
#include "ActorManager.h"
#include "DlistManager.h"
#include "FileManager3D.h"
#include "MathOps.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "edDlist.h"

// 0x0039f0e0
static bool CriterionConicalWave(CActor* pActor, void* pParams)
{
	CConicalWaveShoot* pWave = static_cast<CConicalWaveShoot*>(pParams);
	edF32VECTOR4 actorPosition;
	edF32VECTOR4 wavePosition;
	edF32VECTOR4 delta;
	edF32VECTOR4 direction;
	edF32VECTOR4* pBottom;
	float distance;
	float bottomHeight;
	float actorHeight;
	float waveHeight;
	float dotProduct;
	float angle;

	actorPosition = pActor->currentLocation;
	actorPosition.y = 0.0f;
	wavePosition = pWave->field_0x30;
	wavePosition.y = 0.0f;
	edF32Vector4SubHard(&delta, &actorPosition, &wavePosition);
	distance = edF32Vector4GetDistHard(&delta);
	pBottom = pActor->GetBottomPosition();
	bottomHeight = pBottom->y;
	actorHeight = pActor->currentLocation.y;
	waveHeight = pWave->field_0x30.y + pWave->field_0x8;

	if (pActor != pWave->pOwner) {
		edF32Vector4SubHard(&direction, &pActor->currentLocation, &pWave->pOwner->currentLocation);
		direction.y = 0.0f;
		edF32Vector4NormalizeHard(&direction, &direction);
		dotProduct = edF32Vector4DotProductHard(&direction, &pWave->field_0x40);
		angle = 1.0f;
		if (fabs(dotProduct) <= 1.0f) {
			angle = fabs(dotProduct);
		}
		angle = edF32ACosHard(angle);

		if (((CActorFactory::gClassProperties[pActor->typeID].flags & 0x10) != 0) &&
			(pActor->currentLocation.y < waveHeight) &&
			(pWave->field_0x30.y < (bottomHeight - actorHeight) + pActor->currentLocation.y) &&
			(distance < pWave->field_0x20) && (pWave->field_0x20 - pWave->field_0x4 < distance) &&
			(fabs(angle * 57.29578f) < pWave->field_0xc / 2.0f)) {
			return !pWave->actorsTable.IsInList(pActor);
		}
	}

	return false;
}

CConicalWaveShoot::CConicalWaveShoot()
{
	return;
}

CConicalWaveShoot::~CConicalWaveShoot()
{
	return;
}

void CConicalWaveShoot::Create(ByteCode* pByteCode)
{
	this->field_0x0 = pByteCode->GetF32();
	this->field_0x4 = pByteCode->GetF32();
	this->field_0x8 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetF32();
	this->field_0x14 = pByteCode->GetF32();
	this->field_0x18 = pByteCode->GetF32();

	return;
}

void CConicalWaveShoot::Init(CActor* pOwner)
{
	this->pOwner = pOwner;
	Reset();

	return;
}

void CConicalWaveShoot::Reset()
{
	this->field_0x20 = 0.0f;
	this->actorsTable.nbEntries = 0;
	this->field_0x30 = gF32Vector4Zero;
	this->field_0x40 = gF32Vector4Zero;
	this->field_0x24 = 0;

	return;
}

void CConicalWaveShoot::Fire(edF32VECTOR4* pPosition, edF32VECTOR4* pDirection)
{
	Reset();

	this->pOwner->flags = this->pOwner->flags | 0x80;
	this->pOwner->flags = this->pOwner->flags & 0xffffffdf;
	this->pOwner->EvaluateDisplayState();
	this->pOwner->flags = this->pOwner->flags | 0x400;
	this->field_0x30 = *pPosition;
	this->field_0x40 = *pDirection;
	this->field_0x24 = 1;

	return;
}

void CConicalWaveShoot::Manage()
{
	CActor* pActor;
	CActorsTable actors;
	edF32VECTOR4 sphere;
	_msg_hit_param hitParam;

	if (this->field_0x24 != 0) {
		sphere.w = this->field_0x20 + this->field_0x10 * Timer::GetTimer()->cutsceneDeltaTime;
		this->field_0x20 = sphere.w;
		sphere.xyz = this->field_0x30.xyz;
		CScene::ptable.g_ActorManager_004516a4->cluster.GetActorsIntersectingSphereWithCriterion(&actors, &sphere, CriterionConicalWave, this);

		for (int i = 0; i < actors.nbEntries; i++) {
			pActor = actors.aEntries[i];
			this->actorsTable.Add(pActor);
			hitParam.projectileType = 0;
			edF32Vector4SubHard(&hitParam.field_0x20, &pActor->currentLocation, &this->field_0x30);
			edF32Vector4NormalizeHard(&hitParam.field_0x20, &hitParam.field_0x20);
			hitParam.field_0x20.y = 0.5f;
			edF32Vector4NormalizeHard(&hitParam.field_0x20, &hitParam.field_0x20);
			hitParam.damage = this->field_0x14;
			hitParam.field_0x30 = this->field_0x18;
			this->pOwner->DoMessage(pActor, MESSAGE_KICKED, &hitParam);
		}

		if (this->field_0x0 <= this->field_0x20) {
			this->pOwner->flags = this->pOwner->flags & 0xfffffbff;
			this->pOwner->flags = this->pOwner->flags & 0xffffff5f;
			this->pOwner->EvaluateDisplayState();
			Reset();
		}
	}

	return;
}

void CConicalWaveShoot::Draw()
{
	edDList_material* pMaterial;
	float halfAngle;

	if ((this->field_0x24 != 0) && GameDList_BeginCurrent()) {
		pMaterial = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(CScene::_pinstance->defaultTextureIndex_0x28, 2);
		edDListLoadIdentity();
		edDListUseMaterial(pMaterial);
		for (int i = 0; i < 60; i++) {
			halfAngle = (this->field_0xc * 0.01745329f) / 2.0f;
			if (((float)i * 0.1047198f < halfAngle) || (6.283185f - halfAngle < (float)i * 0.1047198f)) {
				DrawWavePart(0.1047198f, i);
			}
		}

		GameDList_EndCurrent();
	}

	return;
}

// 0x0039f4d0
void CConicalWaveShoot::DrawWavePart(float angle, int part)
{
	float outerRadius;
	float innerRadius;
	float middleRadius;
	edF32MATRIX4 matrixA;
	edF32MATRIX4 matrixB;
	edF32VECTOR4 directionA;
	edF32VECTOR4 directionB;

	edDListBegin(0.0f, 0.0f, 0.0f, 4, 6);

	outerRadius = this->field_0x20;
	innerRadius = outerRadius - this->field_0x4;
	if (innerRadius < 0.0f) {
		innerRadius = 0.0f;
	}
	middleRadius = (innerRadius + outerRadius) / 2.0f;
	edF32Matrix4BuildFromVectorAndAngle(angle * (float)part, &matrixA, &gF32Vector4UnitY);
	edF32Matrix4MulF32Vector4Hard(&directionA, &matrixA, &this->field_0x40);
	edF32Matrix4BuildFromVectorAndAngle(angle * (float)(part + 1), &matrixB, &gF32Vector4UnitY);
	edF32Matrix4MulF32Vector4Hard(&directionB, &matrixB, &this->field_0x40);
	edDListColor4u8(0xff, 0, 0, 0x2d);
	edDListVertex4f(directionA.x * innerRadius + this->field_0x30.x, this->field_0x30.y, directionA.z * innerRadius + this->field_0x30.z, 0.0f);
	edDListVertex4f(directionB.x * innerRadius + this->field_0x30.x, this->field_0x30.y, directionB.z * innerRadius + this->field_0x30.z, 0.0f);
	edDListVertex4f(directionA.x * middleRadius + this->field_0x30.x, this->field_0x30.y + this->field_0x8, directionA.z * middleRadius + this->field_0x30.z, 0.0f);
	edDListVertex4f(directionB.x * middleRadius + this->field_0x30.x, this->field_0x30.y + this->field_0x8, directionB.z * middleRadius + this->field_0x30.z, 0.0f);
	edDListVertex4f(directionA.x * outerRadius + this->field_0x30.x, this->field_0x30.y, directionA.z * outerRadius + this->field_0x30.z, 0.0f);
	edDListVertex4f(directionB.x * outerRadius + this->field_0x30.x, this->field_0x30.y, directionB.z * outerRadius + this->field_0x30.z, 0.0f);
	edDListEnd();

	return;
}
