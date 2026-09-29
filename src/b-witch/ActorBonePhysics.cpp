#include "ActorWolfen.h"
#include "ActorBrazul.h"
#include "ActorPunchingBall.h"
#include "ActorBrazulBoneData.h"
#include "MathOps.h"
#include "TimeController.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace
{
	edF32MATRIX4 gWolfenCollisionFrame;
	edF32MATRIX4 gWolfenCollisionInverse;
	bool gWolfenCollisionFrameValid = false;
	void SetLink(ActorBonePhysicsLink* pLink, uint pointA, uint pointB, float minDistance, float maxDistance)
	{
		pLink->pointA = pointA;
		pLink->pointB = pointB;
		pLink->minDistance = minDistance;
		pLink->maxDistance = maxDistance;
	}

	void ZeroVector(edF32VECTOR4* pVector)
	{
		*pVector = gF32Vector4Zero;
	}

	void AddVector(edF32VECTOR4* pResult, const edF32VECTOR4* pA, const edF32VECTOR4* pB)
	{
		pResult->x = pA->x + pB->x;
		pResult->y = pA->y + pB->y;
		pResult->z = pA->z + pB->z;
		pResult->w = pA->w + pB->w;
	}

	void ScaleVector(edF32VECTOR4* pResult, const edF32VECTOR4* pVector, float scale)
	{
		pResult->x = pVector->x * scale;
		pResult->y = pVector->y * scale;
		pResult->z = pVector->z * scale;
		pResult->w = pVector->w * scale;
	}

	void UpdateInverseMatrix(CActorBonePhysics* pPhysics)
	{
		if (pPhysics->pEnemy_0x60 != nullptr && pPhysics->pEnemy_0x60->pMeshTransform != nullptr)
			edF32Matrix4GetInverseSoft(&pPhysics->matrix_0x20,
				&pPhysics->pEnemy_0x60->pMeshTransform->base.transformA);
	}

	void GetFighterAttachmentPosition(CActorFighter* pFighter, edF32VECTOR4* pPosition)
	{
		if (pFighter->actorState != -1 &&
			(pFighter->GetStateFlags(pFighter->actorState) & 0xff800) == 0x8000 &&
			pFighter->field_0x518 != 0 && pFighter->pAnimationController != nullptr &&
			pFighter->pMeshTransform != nullptr) {
			edF32MATRIX4* pBoneMatrix = pFighter->pAnimationController->GetCurBoneMatrix(pFighter->field_0x518);
			if (pBoneMatrix != nullptr) {
				edF32MATRIX4 attachmentMatrix;
				edF32Matrix4MulF32Matrix4Hard(&attachmentMatrix, pBoneMatrix,
					&pFighter->pMeshTransform->base.transformA);
				*pPosition = attachmentMatrix.rowT;
				return;
			}
		}
		*pPosition = pFighter->currentLocation;
	}

	void ApplyBindPose(int index, edF32MATRIX4* pScaleMatrix)
	{
		edANM_SKELETON* pSkeleton = TheAnimStage.anmSkeleton.pTag;
		if (pSkeleton == nullptr) return;
		uint boneCount = pSkeleton->boneCount;
		uint offsetB = (boneCount * 0xc + 0x13) & 0xfffffff0;
		if ((pSkeleton->flags & 2) == 0) {
			uint offsetA = ((boneCount * 3) * 4 + index * 0x10) * 4;
			edF32MATRIX4* pBindMatrix =
				reinterpret_cast<edF32MATRIX4*>(reinterpret_cast<char*>(pSkeleton) + offsetA + offsetB);
			edF32Matrix4MulF32Matrix4Hard(pScaleMatrix, pBindMatrix, pScaleMatrix);
		}
		else {
			uint offsetA = ((boneCount * 3) * 4 + index * 4) * 4;
			edF32VECTOR4* pBindPosition =
				reinterpret_cast<edF32VECTOR4*>(reinterpret_cast<char*>(pSkeleton) + offsetA + offsetB);
			edF32Matrix4MulF32Vector4Hard(&pScaleMatrix->rowT, pScaleMatrix, pBindPosition);
		}
	}
}

// 003fdc00
void CActorBonePhysics::SetObjCounts(int countA, int countB)
{
	count_0x8 = countA;
	count_0xc = countB == 0 ? countA - 1 : countB;
	field_0x0 = new StaticEnemy90*[count_0x8]();
	field_0x4 = new ActorBonePhysicsLink*[count_0xc]();
	Func_0x1c();
}

// 003fdb e0
void CActorBonePhysics::SetupObjects(CActor* pOwner)
{
	pEnemy_0x60 = pOwner;
	Func_0x18();
}

// 003fdb80
void CActorBonePhysics::Term()
{
	Func_0x20();
	delete[] field_0x0;
	field_0x0 = nullptr;
	delete[] field_0x4;
	field_0x4 = nullptr;
	count_0x8 = 0;
	count_0xc = 0;
}

// 003fda80
void CActorBonePhysics::Func_0x18()
{
	edF32VECTOR4* pLocation = pEnemy_0x60 == nullptr ? nullptr : &pEnemy_0x60->currentLocation;
	for (uint i = 0; i < count_0x8; i++) {
		StaticEnemy90* pPoint = field_0x0[i];
		pPoint->boneHash = 0;
		pPoint->flags = 0;
		pPoint->inverseMass = 0.0f;
		pPoint->position = pLocation == nullptr ? gF32Vector4Zero : *pLocation;
		pPoint->previousPosition = pPoint->position;
		ZeroVector(&pPoint->field_0x30);
		ZeroVector(&pPoint->rotation);
		pPoint->scale = {1.0f, 1.0f, 1.0f, 1.0f};
		ZeroVector(&pPoint->acceleration);
		ZeroVector(&pPoint->gravity);
	}
	for (uint i = 0; i < count_0xc; i++) {
		field_0x4[i]->pointA = 0;
		field_0x4[i]->pointB = 0;
		field_0x4[i]->minDistance = 0.0f;
		field_0x4[i]->maxDistance = 0.0f;
		field_0x4[i]->field_0x18 = 0;
	}
	edF32Matrix4CopyHard(&matrix_0x20, &gF32Matrix4Unit);
	field_0x10 = 1.0f;
	field_0x14 = 0.0f;
}

// 003fd8c0
void CActorBonePhysics::Func_0x14(uint flags)
{
	if (pEnemy_0x60 == nullptr || pEnemy_0x60->pAnimationController == nullptr ||
		(pEnemy_0x60->flags & 0x4800) == 0 || TheAnimStage.pFrameMatrixData == nullptr) {
		return;
	}
	CAnimation* pAnimation = pEnemy_0x60->pAnimationController;
	for (uint i = 0; i < count_0x8; i++) {
		StaticEnemy90* pPoint = field_0x0[i];
		int index = pAnimation->GetBoneIndex(pPoint->boneHash);
		if (index < 0) continue;
		edF32MATRIX4 boneMatrix;
		pAnimation->anmSkeleton.UnskinNMatrices(&boneMatrix, TheAnimStage.pFrameMatrixData, index, 1);
		edF32MATRIX4 normalizedMatrix = boneMatrix;
		edF32VECTOR4 boneScale = {
			edF32Vector4NormalizeHard(&normalizedMatrix.v0, &normalizedMatrix.v0),
			edF32Vector4NormalizeHard(&normalizedMatrix.v1, &normalizedMatrix.v1),
			edF32Vector4NormalizeHard(&normalizedMatrix.v2, &normalizedMatrix.v2), 1.0f
		};
		if (flags & 1) {
			edF32Matrix4MulF32Vector4Hard(&pPoint->position, &pEnemy_0x60->pMeshTransform->base.transformA,
				(edF32VECTOR4*)&boneMatrix.da);
		}
		if (flags & 2) {
			edQuatFromMatrix4(&pPoint->rotation, &normalizedMatrix);
		}
		if (flags & 4) {
			pPoint->scale = boneScale;
		}
		pPoint->previousPosition = pPoint->position;
	}
}

// 003fd180
void CActorBonePhysics::Func_0x1c()
{
	for (uint i = 0; i < count_0x8; i++) {
		field_0x0[i] = allocateA(i);
	}
	for (uint i = 0; i < count_0xc; i++) {
		field_0x4[i] = allocateB(i);
	}
}

// 003fd090
void CActorBonePhysics::Func_0x20()
{
	for (uint i = 0; i < count_0x8; i++) {
		delete field_0x0[i];
		field_0x0[i] = nullptr;
	}
	for (uint i = 0; i < count_0xc; i++) {
		delete field_0x4[i];
		field_0x4[i] = nullptr;
	}
}

// 0035b070 / 0035b0a0 / 0035b0d0
StaticEnemy90* CActorBonePhysics::allocateA(uint)
{
	return new StaticEnemy90{};
}

ActorBonePhysicsLink* CActorBonePhysics::allocateB(uint)
{
	return new ActorBonePhysicsLink{};
}

void* CActorBonePhysics::Func_0x2c()
{
	return ::operator new(0x20);
}

void CActorBonePhysics::Func_0x30() {}
void CActorBonePhysics::Func_0x38(int) {}
void CActorBonePhysics::Func_0x3c(int, edF32VECTOR4*, edF32VECTOR4*) {}

void CActorBonePhysics::TransformPoints(edF32MATRIX4* pTransform)
{
	for (uint i = 0; i < count_0x8; i++) {
		StaticEnemy90* pPoint = field_0x0[i];
		edF32Matrix4MulF32Vector4Hard(&pPoint->position, pTransform, &pPoint->position);
		edF32Matrix4MulF32Vector4Hard(&pPoint->previousPosition, pTransform, &pPoint->previousPosition);
	}
}

// 003fced0
void CActorBonePhysics::Func_0x34(int index)
{
	ActorBonePhysicsLink* pLink = field_0x4[index];
	StaticEnemy90* pA = field_0x0[pLink->pointA];
	StaticEnemy90* pB = field_0x0[pLink->pointB];
	edF32VECTOR4 direction;
	edF32Vector4SubHard(&direction, &pB->position, &pA->position);
	float distance = edF32Vector4SafeNormalize0Hard(&direction, &direction);
	float correction = 0.0f;
	if (distance < pLink->minDistance) {
		correction = distance - pLink->minDistance;
	}
	else if (distance > pLink->maxDistance) {
		correction = distance - pLink->maxDistance;
	}
	if (correction != 0.0f) {
		float totalWeight = pA->inverseMass + pB->inverseMass;
		if (totalWeight != 0.0f) {
			edF32VECTOR4 displacement;
			ScaleVector(&displacement, &direction, correction * pA->inverseMass / totalWeight);
			AddVector(&pA->position, &pA->position, &displacement);
			ScaleVector(&displacement, &direction, correction * pB->inverseMass / totalWeight);
			pB->position.x -= displacement.x;
			pB->position.y -= displacement.y;
			pB->position.z -= displacement.z;
		}
	}
}

// 003fd5e0
void CActorBonePhysics::Simulate(uint iterations)
{
	float dt = GetTimer()->cutsceneDeltaTime;
	for (uint i = 0; i < count_0x8; i++) {
		StaticEnemy90* pPoint = field_0x0[i];
		edF32VECTOR4 velocity;
		edF32VECTOR4 force;
		AddVector(&force, &pPoint->acceleration, &pPoint->gravity);
		ScaleVector(&force, &force, dt * dt);
		edF32Vector4SubHard(&velocity, &pPoint->position, &pPoint->previousPosition);
		ScaleVector(&velocity, &velocity, field_0x10);
		pPoint->previousPosition = pPoint->position;
		AddVector(&force, &force, &velocity);
		float length = std::sqrt(force.x * force.x + force.y * force.y + force.z * force.z);
		if (length > 1.0f) {
			ScaleVector(&force, &force, 1.0f / length);
		}
		AddVector(&pPoint->position, &pPoint->position, &force);
		ZeroVector(&pPoint->acceleration);
	}
	Func_0x30();
	for (uint iteration = 0; iteration < iterations; iteration++) {
		for (uint i = 0; i < count_0x8; i++) Func_0x38(i);
		for (uint i = 0; i < count_0xc; i++) Func_0x34(i);
	}
}

void CActorBonePhysics::ApplyToAnimation()
{
	if (pEnemy_0x60 == nullptr || pEnemy_0x60->pAnimationController == nullptr ||
		(pEnemy_0x60->flags & 0x4800) == 0 || TheAnimStage.pFrameMatrixData == nullptr ||
		field_0x14 == 0.0f) return;
	CAnimation* pAnimation = pEnemy_0x60->pAnimationController;
	for (uint i = 0; i < count_0x8; i++) {
		StaticEnemy90* pPoint = field_0x0[i];
		if ((pPoint->flags & 1) != 0 || pPoint->inverseMass == 0.0f) continue;
		int index = pAnimation->GetBoneIndex(pPoint->boneHash);
		if (index < 0) continue;
		edF32MATRIX4* pFrame = TheAnimStage.pFrameMatrixData + index;
		edF32VECTOR4 rotation = pPoint->rotation;
		edF32VECTOR4 scale = pPoint->scale;
		Func_0x3c(i, &rotation, &scale);
		edF32VECTOR4 localPosition;
		edF32Matrix4MulF32Vector4Hard(&localPosition, &matrix_0x20, &pPoint->position);
		AddVector(&localPosition, &localPosition, &pPoint->field_0x30);
		if (field_0x14 != 1.0f) {
			edF32MATRIX4 animatedMatrix;
			pAnimation->anmSkeleton.UnskinNMatrices(&animatedMatrix, pFrame, index, 1);
			edF32MATRIX4 normalizedMatrix = animatedMatrix;
			edF32VECTOR4 animatedScale = {
				edF32Vector4NormalizeHard(&normalizedMatrix.v0, &normalizedMatrix.v0),
				edF32Vector4NormalizeHard(&normalizedMatrix.v1, &normalizedMatrix.v1),
				edF32Vector4NormalizeHard(&normalizedMatrix.v2, &normalizedMatrix.v2), 1.0f
			};
			edF32VECTOR4 animatedRotation;
			edQuatFromMatrix4(&animatedRotation, &normalizedMatrix);
			edQuatShortestSLERPHard(field_0x14, &rotation, &animatedRotation, &rotation);
			edF32VECTOR4 animatedPosition = {animatedMatrix.da, animatedMatrix.db,
				animatedMatrix.dc, animatedMatrix.dd};
			edF32Vector4LERPHard(field_0x14, &localPosition, &animatedPosition, &localPosition);
			edF32Vector4LERPHard(field_0x14, &scale, &animatedScale, &scale);
		}
		edF32MATRIX4 scaleMatrix;
		edF32Matrix4ScaleHard(&scaleMatrix, &gF32Matrix4Unit, &scale);
		ApplyBindPose(index, &scaleMatrix);
		edF32MATRIX4 result;
		edQuatToMatrix4Hard(&rotation, &result);
		edF32Matrix4TranslateHard(&result, &result, &localPosition);
		edF32Matrix4MulF32Matrix4Hard(pFrame, &scaleMatrix, &result);
	}
}

// 003c2910 / 003c28a0
void CActorBonePhysics::FUN_003c2910()
{
	for (int i = 0; i < 3; i++) SetLink(field_0x4[i], field_0x4[i]->pointA, field_0x4[i]->pointB, 0.1f, 0.2f);
	field_0x74 = 90.0f;
}

void CActorBonePhysics::FUN_003c28a0()
{
	static const float maximums[3] = {0.25f, 0.35f, 0.25f};
	for (int i = 0; i < 3; i++) SetLink(field_0x4[i], field_0x4[i]->pointA, field_0x4[i]->pointB, 0.1f, maximums[i]);
	field_0x74 = 10.0f;
}

// 003c2aa0 / 003c2a30
void CActorBonePhysics::FUN_003c2aa0()
{
	field_0x70 = 2;
	if (pEnemy_0x60 != nullptr && pEnemy_0x60->pAnimationController != nullptr)
		pEnemy_0x60->pAnimationController->RegisterBone(0x43a18690);
	field_0x14 = 1.0f;
}

void CActorBonePhysics::FUN_003c2a30()
{
	field_0x70 = 0;
	if (pEnemy_0x60 != nullptr && pEnemy_0x60->pAnimationController != nullptr)
		pEnemy_0x60->pAnimationController->UnRegisterBone(0x43a18690);
	field_0x14 = 0.0f;
}

// 003c1400: variant 3 Wolfen starts the simulation on its first update.
int CActorBonePhysics::UpdateFullBodyPostAnimEffects()
{
	if (field_0x70 == 0) {
		field_0x70 = 1;
		Func_0x14(3);
		for (uint i = 0; i < count_0x8; i++) {
			if (field_0x0[i]->inverseMass != 0.0f)
				field_0x0[i]->gravity = {0.0f, -20.0f, 0.0f, 0.0f};
		}
		field_0x14 = 1.0f;
		field_0x10 = 0.8f;
		return 0;
	}
	if (pEnemy_0x60 == nullptr || pEnemy_0x60->pAnimationController == nullptr ||
		pEnemy_0x60->pMeshTransform == nullptr || TheAnimStage.pFrameMatrixData == nullptr)
		return 0;
	UpdateInverseMatrix(this);
	CAnimation* pAnimation = pEnemy_0x60->pAnimationController;
	int index = pAnimation->GetBoneIndex(0x463c1970);
	if (index >= 0) {
		edF32MATRIX4 boneMatrix;
		pAnimation->anmSkeleton.UnskinNMatrices(&boneMatrix, TheAnimStage.pFrameMatrixData, index, 1);
		edF32VECTOR4 boneOrigin = {boneMatrix.da, boneMatrix.db, boneMatrix.dc, boneMatrix.dd};
		edF32Matrix4MulF32Vector4Hard(&field_0x0[0]->position,
			&pEnemy_0x60->pMeshTransform->base.transformA, &boneOrigin);
		static const edF32VECTOR4 offsets[4] = {
			{0.1f, 0.0f, 0.0f, 0.0f}, {0.2f, -0.05f, 0.0f, 0.0f},
			{-0.1f, 0.0f, 0.0f, 0.0f}, {-0.2f, -0.05f, 0.0f, 0.0f}
		};
		for (int i = 0; i < 4; i++) {
			edF32VECTOR4 offset = offsets[i];
			edF32VECTOR4 localPosition;
			edF32Matrix4MulF32Vector4Hard(&localPosition, &boneMatrix, &offset);
			AddVector(&localPosition, &localPosition, &boneOrigin);
			edF32Matrix4MulF32Vector4Hard(&field_0x0[i + 1]->position,
				&pEnemy_0x60->pMeshTransform->base.transformA, &localPosition);
		}
	}
	index = pAnimation->GetBoneIndex(0x55949591);
	if (index >= 0) {
		edF32MATRIX4 collisionBone;
		pAnimation->anmSkeleton.UnskinNMatrices(&collisionBone, TheAnimStage.pFrameMatrixData, index, 1);
		edF32Matrix4MulF32Matrix4Hard(&collisionBone, &collisionBone,
			&pEnemy_0x60->pMeshTransform->base.transformA);
		edF32VECTOR4 collisionScale = {0.5f, 0.4f, 0.35f, 1.0f};
		edF32VECTOR4 collisionOffset = {0.0f, -0.1f, 0.1f, 0.0f};
		edF32Matrix4ScaleHard(&gWolfenCollisionFrame, &gF32Matrix4Unit, &collisionScale);
		edF32Matrix4TranslateHard(&gWolfenCollisionFrame, &gWolfenCollisionFrame, &collisionOffset);
		edF32Matrix4MulF32Matrix4Hard(&gWolfenCollisionFrame, &gWolfenCollisionFrame, &collisionBone);
		edF32Matrix4GetInverseSoft(&gWolfenCollisionInverse, &gWolfenCollisionFrame);
		gWolfenCollisionFrameValid = true;
		edF32VECTOR4 axis = collisionBone.v2;
		if (edF32Vector4NormalizeHard(&axis, &axis) == 0.0f) axis = gF32Vector4UnitZ;
		float angle = GetAngleYFromVector(&axis);
		edF32MATRIX4 rotationMatrix;
		edF32Matrix4RotateYHard(angle, &rotationMatrix, &gF32Matrix4Unit);
		static const float angles[5] = {0.0f, -0.08f, -0.12f, 0.08f, 0.12f};
		for (int i = 0; i < 5; i++) {
			edF32VECTOR4 localAxis;
			SetVectorFromAngleY(angles[i], &localAxis);
			edF32Matrix4MulF32Vector4Hard(&field_0x4[i]->axis, &rotationMatrix, &localAxis);
		}
	}
	Simulate(1);
	ApplyToAnimation();
	return 1;
}

// 003c2b20 / 0035b260: shared Wolfen and punching ball update.
int CActorBonePhysics::UpdateLinkedPostAnimEffects()
{
	if (field_0x70 == 0) return 0;
	if (field_0x70 == 2) {
		Func_0x14(1);
		if (count_0x8 > 1)
			field_0x0[1]->gravity = {0.0f, -10.0f, 0.0f, 0.0f};
		field_0x70 = 1;
		return 1;
	}
	UpdateInverseMatrix(this);
	if (pEnemy_0x60 != nullptr && pEnemy_0x60->pAnimationController != nullptr &&
		pEnemy_0x60->pMeshTransform != nullptr && count_0x8 > 0) {
		edF32MATRIX4* pBoneMatrix = pEnemy_0x60->pAnimationController->GetCurBoneMatrix(0x43a18690);
		if (pBoneMatrix != nullptr) {
			edF32Matrix4MulF32Vector4Hard(&field_0x0[0]->position,
				&pEnemy_0x60->pMeshTransform->base.transformA, (edF32VECTOR4*)&pBoneMatrix->da);
		}
		CActorFighter* pFighter = static_cast<CActorFighter*>(pEnemy_0x60);
		if (pFighter->field_0x354 != nullptr && count_0x8 > 2) {
			edF32VECTOR4 attachmentPosition;
			GetFighterAttachmentPosition(pFighter->field_0x354, &attachmentPosition);
			uint endPoint = count_0x8 - 1;
			edF32Matrix4MulF32Vector4Hard(&field_0x0[endPoint]->position,
				&matrix_0x20, &attachmentPosition);
			AddVector(&field_0x0[endPoint]->position,
				&field_0x0[endPoint]->position, &field_0x80);
			edF32Matrix4MulF32Vector4Hard(&field_0x0[endPoint]->position,
				&pEnemy_0x60->pMeshTransform->base.transformA,
				&field_0x0[endPoint]->position);
		}
	}
	Simulate(count_0x8 == 4 ? 2 : 1);
	ApplyToAnimation();
	return 1;
}

// 0035b3d0, vtable 00442fb0
void CPunchingBallBonePhysics::SetupObjects(CActor* pOwner)
{
	CActorBonePhysics::SetupObjects(pOwner);
	field_0x0[0]->boneHash = 0;
	field_0x0[1]->boneHash = 0x7596d4d6;
	field_0x0[2]->boneHash = 0;
	field_0x0[1]->inverseMass = 1.0f;
	field_0x0[2]->inverseMass = 0.001f;
	SetLink(field_0x4[0], 0, 1, 0.2f, 0.4f);
	SetLink(field_0x4[1], 1, 2, 0.1f, 0.35f);
	field_0x10 = 0.8f;
	field_0x80 = {0.04f, -0.1f, -0.05f, 0.0f};
	field_0x70 = 0;
}

void CPunchingBallBonePhysics::Func_0x3c(int index, edF32VECTOR4* rotation, edF32VECTOR4* direction)
{
	if (index != 1 || rotation == nullptr || direction == nullptr) return;
	ActorBonePhysicsLink* pLink = field_0x4[index];
	edF32VECTOR4 axis;
	edF32Vector4SubHard(&axis, &field_0x0[pLink->pointB]->position,
		&field_0x0[pLink->pointA]->position);
	float length = edF32Vector4SafeNormalize0Hard(&axis, &axis);
	if (length != 0.0f) {
		*direction = axis;
		field_0x0[pLink->pointB]->scale = {1.0f, 1.0f, length / 0.3f, 1.0f};
	}
}

// 003c2cc0, vtable 00446100
void CWolfenSharedBonePhysics::SetupObjects(CActor* pOwner)
{
	CActorBonePhysics::SetupObjects(pOwner);
	field_0x0[0]->boneHash = 0;
	field_0x0[1]->boneHash = 0x7596d4d6;
	field_0x0[2]->boneHash = 0x7597d4d6;
	field_0x0[3]->boneHash = 0x7598d4d6;
	field_0x0[1]->inverseMass = 1.0f;
	field_0x0[2]->inverseMass = 1.0f;
	field_0x0[3]->inverseMass = 0.001f;
	field_0x0[2]->scale = {1.0f, 0.5f, 1.0f, 1.0f};
	field_0x0[3]->scale = {1.0f, 0.5f, 1.0f, 1.0f};
	for (uint i = 0; i < 3; i++) SetLink(field_0x4[i], i, i + 1, 0.1f, 0.2f);
	field_0x74 = 90.0f;
	field_0x80 = {0.04f, -0.02f, 0.1f, 0.0f};
	field_0x70 = 0;
}

// 003c26e0
void CWolfenSharedBonePhysics::Func_0x30()
{
	for (uint i = 0; i < count_0xc; i++) {
		ActorBonePhysicsLink* pLink = field_0x4[i];
		StaticEnemy90* pA = field_0x0[pLink->pointA];
		StaticEnemy90* pB = field_0x0[pLink->pointB];
		edF32VECTOR4 direction;
		edF32Vector4SubHard(&direction, &pB->position, &pA->position);
		float distance = edF32Vector4SafeNormalize0Hard(&direction, &direction);
		if (distance > pLink->minDistance) {
			float length = std::min(distance, pLink->maxDistance);
			float force = length * field_0x74 * length;
			float totalWeight = pA->inverseMass + pB->inverseMass;
			if (totalWeight != 0.0f) {
				edF32VECTOR4 impulse;
				ScaleVector(&impulse, &direction, force * pA->inverseMass / totalWeight);
				AddVector(&pA->acceleration, &pA->acceleration, &impulse);
				ScaleVector(&impulse, &direction, force * pB->inverseMass / totalWeight);
				pB->acceleration.x -= impulse.x;
				pB->acceleration.y -= impulse.y;
				pB->acceleration.z -= impulse.z;
			}
		}
	}
}

// 003c1810, vtable 004460b0
void CWolfenFullBodyPhysics::SetupObjects(CActor* pOwner)
{
	CActorBonePhysics::SetupObjects(pOwner);
	static const uint boneHashes[26] = {
		0x463c1970, 0x463c1970, 0x463c1970, 0x463c1970, 0x463c1970,
		0xcae60310, 0xcae70310, 0xcae80310, 0xcae90310, 0,
		0xc290efdb, 0xc290f0db, 0xc290f1db, 0, 0xc28cefdb,
		0xc28cf0db, 0xc28cf1db, 0xd18defe7, 0xd18df0e7, 0xd18df1e7,
		0, 0xd189efe7, 0xd189f0e7, 0xd189f1e7, 0, 0
	};
	static const float weights[26] = {
		0, 0, 0, 0, 0, 0.001f, 1.0f, 1.25f, 1.5f, 1.75f,
		0.001f, 1.0f, 1.25f, 1.5f, 0.001f, 1.0f, 1.25f,
		0.001f, 1.0f, 1.25f, 1.5f, 1.75f, 0.001f, 1.0f, 1.25f, 1.5f
	};
	for (uint i = 0; i < 26; i++) {
		field_0x0[i]->boneHash = boneHashes[i];
		field_0x0[i]->inverseMass = weights[i];
	}
	static const uint links[29][2] = {
		{0, 5}, {1, 10}, {2, 14}, {3, 18}, {4, 22},
		{5, 6}, {6, 7}, {7, 8}, {8, 9},
		{10, 11}, {11, 12}, {12, 13},
		{14, 15}, {15, 16}, {16, 17},
		{18, 19}, {19, 20}, {20, 21},
		{22, 23}, {23, 24}, {24, 25},
		{6, 11}, {11, 15}, {6, 19}, {19, 23},
		{7, 12}, {12, 16}, {7, 20}, {20, 24}
	};
	for (uint i = 0; i < 29; i++) {
		float minDistance = i < 5 ? 0.25f : i < 21 ? 0.2f : 0.1f;
		float maxDistance = i < 5 ? 0.25f :
			(i == 5 || i == 9 || i == 12 || i == 15 || i == 18) ? 0.3f :
			i < 21 ? 0.4f : 0.12f;
		SetLink(field_0x4[i], links[i][0], links[i][1], minDistance, maxDistance);
		if (i < 5) {
			static const float angles[5] = {0.0f, -0.08f, -0.12f, 0.08f, 0.12f};
			field_0x4[i]->constraintType = 2;
			SetVectorFromAngleY(angles[i], &field_0x4[i]->axis);
			field_0x4[i]->field_0x30.y = 0.85f;
		}
		else if (i < 21) {
			field_0x4[i]->constraintType = (i == 5 || i == 9 || i == 12 || i == 15 || i == 18) ? 2 : 1;
			field_0x4[i]->field_0x18 = 1;
			static const uint parents[16] = {
				0, 5, 6, 7, 1, 9, 10, 2, 12, 13, 3, 15, 16, 4, 18, 19
			};
			field_0x4[i]->parentLink = parents[i - 5];
			field_0x4[i]->field_0x30.y = 1.0f;
		}
	}
	field_0x70 = 0;
}

// 003c1310
ActorBonePhysicsLink* CWolfenFullBodyPhysics::allocateB(uint)
{
	return new ActorBonePhysicsLink{};
}

void CWolfenFullBodyPhysics::Func_0x34(int index)
{
	CActorBonePhysics::Func_0x34(index);
	ActorBonePhysicsLink* pLink = field_0x4[index];
	if (pLink->constraintType == 0 || pLink->constraintType > 4) return;
	StaticEnemy90* pA = field_0x0[pLink->pointA];
	StaticEnemy90* pB = field_0x0[pLink->pointB];
	edF32VECTOR4 direction;
	edF32Vector4SubHard(&direction, &pA->position, &pB->position);
	float length = edF32Vector4SafeNormalize0Hard(&direction, &direction);
	if (length == 0.0f) return;
	edF32VECTOR4 axis = pLink->axis;
	if (pLink->field_0x18 == 1 && pLink->parentLink < count_0xc) {
		ActorBonePhysicsLink* pParent = field_0x4[pLink->parentLink];
		edF32Vector4SubHard(&axis, &field_0x0[pParent->pointA]->position,
			&field_0x0[pParent->pointB]->position);
		edF32Vector4SafeNormalize0Hard(&axis, &axis);
	}
	else if (pEnemy_0x60 != nullptr && pEnemy_0x60->pMeshTransform != nullptr) {
		edF32Matrix4MulF32Vector4Hard(&axis, &pEnemy_0x60->pMeshTransform->base.transformA, &axis);
	}
	float cosine = std::clamp(edF32Vector4DotProductHard(&axis, &direction), -1.0f, 1.0f);
	float angle = std::acos(cosine);
	float maxAngle = pLink->field_0x30.y;
	if (maxAngle <= 0.0f || angle <= maxAngle) return;
	edF32VECTOR4 normal;
	edF32Vector4CrossProductHard(&normal, &axis, &direction);
	if (edF32Vector4SafeNormalize0Hard(&normal, &normal) == 0.0f) return;
	edF32MATRIX4 rotation;
	edF32Matrix4FromAngAxisSoft(maxAngle - angle, &rotation, &normal);
	edF32Matrix4MulF32Vector4Hard(&direction, &rotation, &direction);
	ScaleVector(&direction, &direction, length);
	pB->position.x = pA->position.x - direction.x;
	pB->position.y = pA->position.y - direction.y;
	pB->position.z = pA->position.z - direction.z;
}

// 003c1180: keep mobile points outside the reference body's unit sphere.
void CWolfenFullBodyPhysics::Func_0x38(int index)
{
	StaticEnemy90* pPoint = field_0x0[index];
	pPoint->flags &= ~2u;
	if (pPoint->inverseMass == 0.0f || pEnemy_0x60 == nullptr ||
		pEnemy_0x60->pMeshTransform == nullptr) return;
	edF32VECTOR4 localPosition;
	edF32Matrix4MulF32Vector4Hard(&localPosition,
		gWolfenCollisionFrameValid ? &gWolfenCollisionInverse : &matrix_0x20, &pPoint->position);
	localPosition.w = 0.0f;
	float length = edF32Vector4SafeNormalize0Hard(&localPosition, &localPosition);
	if (length < 1.0f) {
		if (length == 0.0f) localPosition = gF32Vector4UnitZ;
		localPosition.z = -std::abs(localPosition.z);
		localPosition.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&pPoint->position,
			gWolfenCollisionFrameValid ? &gWolfenCollisionFrame :
			&pEnemy_0x60->pMeshTransform->base.transformA, &localPosition);
		pPoint->flags |= 2;
	}
}

// 003c0b60: orientation of the link starting at the requested point.
void CWolfenFullBodyPhysics::Func_0x3c(int index, edF32VECTOR4* rotation, edF32VECTOR4*)
{
	if (rotation == nullptr) return;
	for (uint i = 0; i < count_0xc; i++) {
		ActorBonePhysicsLink* pLink = field_0x4[i];
		if (pLink->pointA != (uint)index) continue;
		edF32VECTOR4 direction;
		edF32Vector4SubHard(&direction, &field_0x0[pLink->pointA]->position,
			&field_0x0[pLink->pointB]->position);
		edF32Matrix4MulF32Vector4Hard(&direction, &matrix_0x20, &direction);
		edF32Vector4SafeNormalize0Hard(&direction, &direction);
		edF32VECTOR4 axis;
		edF32Vector4CrossProductHard(&axis, &direction, &gF32Vector4UnitZ);
		edF32Vector4SafeNormalize0Hard(&axis, &axis);
		float cosine = std::clamp(edF32Vector4DotProductHard(&direction, &gF32Vector4UnitZ), -1.0f, 1.0f);
		edQuatFromAngAxis(std::acos(cosine), rotation, &axis);
		return;
	}
}

// 003e4d50, vtable 004471f0
extern uint UINT_ARRAY_00437ca0[16];

void CBrazulBonePhysics::SetupObjects(CActor* pOwner)
{
	CActorBonePhysics::SetupObjects(pOwner);
	for (uint i = 0; i < 16; i++) {
		field_0x0[i]->boneHash = UINT_ARRAY_00437ca0[i];
		field_0x0[i]->flags |= 2;
		if (i != 0) field_0x0[i]->inverseMass = 1.0f;
		if (i >= 4 && i < 10) field_0x0[i]->flags |= 1;
	}
	for (uint i = 0; i < 3; i++) {
		ActorBonePhysicsLink* pLink = field_0x4[i];
		SetLink(pLink, i, i + 1, 0.1f, 0.2f);
		pLink->constraintType = i == 0 ? 1 : 0;
		pLink->parentLink = i == 0 ? 0xffff : i - 1;
		pLink->axis = gF32Vector4UnitZ;
		pLink->field_0x40 = 0.0f;
		pLink->field_0x44 = 6.2831855f;
		pLink->field_0x48 = -1.57f;
		pLink->field_0x4c = i == 0 ? -1.45f : i == 1 ? -1.35f : -0.9f;
	}
	for (uint i = 3; i < 9; i++) {
		ActorBonePhysicsLink* pLink = field_0x4[i];
		SetLink(pLink, 3, i + 1, 0.2f, 0.3f);
		pLink->constraintType = 2;
		pLink->parentLink = 2;
		pLink->field_0x40 = 3.1415927f;
		pLink->field_0x44 = 3.1415927f;
		pLink->field_0x48 = -0.5f;
		pLink->field_0x4c = -0.5f;
	}
	for (uint i = 9; i < 15; i++) {
		ActorBonePhysicsLink* pLink = field_0x4[i];
		SetLink(pLink, i - 5, i + 1, 0.1f, 0.4f);
		pLink->constraintType = 2;
		pLink->parentLink = i - 6;
		pLink->field_0x40 = 2.3561945f;
		pLink->field_0x44 = 3.9269907f;
		pLink->field_0x48 = -1.5707964f;
		pLink->field_0x4c = 1.5707964f;
	}
	for (uint i = 0; i < 6; i++) {
		ActorBonePhysicsLink* pLink = field_0x4[i + 15];
		SetLink(pLink, i + 10, i == 5 ? 10 : i + 11, 0.2f, 0.65f);
		pLink->parentLink = 0xffff;
	}
	field_0x10 = 0.82f;
	field_0x70 = 0;
	pCurrentState = nullptr;
	if (pOwner != nullptr && pOwner->pAnimationController != nullptr)
		pOwner->pAnimationController->RegisterBone(0xd3dbdab7);
}

// 003e4d10
void CBrazulBonePhysics::Term()
{
	CActorBonePhysics::Term();
	if (pEnemy_0x60 != nullptr && pEnemy_0x60->pAnimationController != nullptr)
		pEnemy_0x60->pAnimationController->UnRegisterBone(0xd3dbdab7);
}

// 003e4460
void CBrazulBonePhysics::ChangeState(const BrazulStateStruct* pState)
{
	if (pCurrentState == pState || pEnemy_0x60 == nullptr) return;
	static const BrazulStateStruct defaultState = {-1, 0, 0, 2, 0, 0.0f};
	const BrazulStateStruct* pNewState = pState == nullptr ? &defaultState : pState;
	if (pCurrentState == nullptr || pCurrentState->field_0x6 != pNewState->field_0x6) {
		if (pNewState->field_0x6 != 2) {
			uint group = pNewState->field_0x6;
			for (uint i = 0; i < count_0x8; i++) {
				const uint32_t* pData = gBrazulPointStates + group * 256 + i * 16;
				StaticEnemy90* pPoint = field_0x0[i];
				std::memcpy(&pPoint->rotation, pData, 16);
				std::memcpy(&pPoint->field_0x30, pData + 8, 16);
				std::memcpy(&pPoint->scale, pData + 12, 16);
				edF32VECTOR4 localPosition;
				std::memcpy(&localPosition, pData + 4, 16);
				if (pEnemy_0x60->pMeshTransform != nullptr)
					edF32Matrix4MulF32Vector4Hard(&pPoint->position,
						&pEnemy_0x60->pMeshTransform->base.transformA, &localPosition);
				else
					pPoint->position = localPosition;
				pPoint->previousPosition = pPoint->position;
			}
			for (uint i = 0; i < count_0xc; i++) {
				const uint32_t* pData = gBrazulLinkStates + group * 84 + i * 4;
				std::memcpy(&field_0x4[i]->field_0x30, pData, 16);
			}
		}
		Func_0x14(1);
		for (uint i = 1; i < count_0x8; i++)
			field_0x0[i]->gravity = {0.0f, -20.0f, 0.0f, 0.0f};
	}
	float target = pNewState->field_0x4 == 1 ? 1.0f : 0.0f;
	float duration = 0.0f;
	if (pNewState->field_0x5 == 3) {
		duration = pNewState->field_0x8;
	}
	else if ((pNewState->field_0x5 == 1 || pNewState->field_0x5 == 2) &&
		pEnemy_0x60->pAnimationController != nullptr) {
		int animationId = pEnemy_0x60->GetIdMacroAnim(pNewState->field_0x0);
		if (animationId >= 0) {
			float length = pEnemy_0x60->pAnimationController->GetAnimLength(animationId, 1);
			if (pNewState->field_0x5 == 1)
				duration = length - pEnemy_0x60->pAnimationController->GetAnimLength(animationId, 2);
			else
				duration = length * pNewState->field_0x8;
		}
	}
	if (duration == 0.0f) {
		field_0x78 = 0.0f;
		field_0x14 = target;
	}
	else {
		field_0x78 = (target - field_0x14) / duration;
	}
	pCurrentState = pState;
}

ActorBonePhysicsLink* CBrazulBonePhysics::allocateB(uint)
{
	return new ActorBonePhysicsLink{};
}

void CBrazulBonePhysics::Func_0x18()
{
	CActorBonePhysics::Func_0x18();
	field_0x70 = 0;
	field_0x74 = 0.0f;
}

void CBrazulBonePhysics::Func_0x14(uint)
{
	UpdateInverseMatrix(this);
	for (uint i = 0; i < count_0xc; i++) {
		ActorBonePhysicsLink* pLink = field_0x4[i];
		if ((pLink->constraintType & 2) == 0) {
			pLink->axis = gF32Vector4UnitZ;
			continue;
		}
		uint parent = pLink->parentLink;
		while (parent < count_0xc && (field_0x0[field_0x4[parent]->pointA]->flags & 1) != 0)
			parent = field_0x4[parent]->parentLink;
		if (parent >= count_0xc) continue;
		ActorBonePhysicsLink* pParent = field_0x4[parent];
		edF32Vector4SubHard(&pLink->axis, &field_0x0[pLink->pointB]->position,
			&field_0x0[pParent->pointA]->position);
		edF32Matrix4MulF32Vector4Hard(&pLink->axis, &matrix_0x20, &pLink->axis);
		pLink->axis.y = 0.0f;
		if (edF32Vector4SafeNormalize0Hard(&pLink->axis, &pLink->axis) == 0.0f)
			pLink->axis = gF32Vector4UnitZ;
	}
}

void CBrazulBonePhysics::Func_0x34(int index)
{
	CActorBonePhysics::Func_0x34(index);
	ActorBonePhysicsLink* pLink = field_0x4[index];
	if (pLink->parentLink == 0xffff && (pLink->constraintType & 1) == 0) return;
	StaticEnemy90* pA = field_0x0[pLink->pointA];
	StaticEnemy90* pB = field_0x0[pLink->pointB];
	edF32VECTOR4 localDirection;
	edF32Vector4SubHard(&localDirection, &pB->position, &pA->position);
	float length = edF32Vector4SafeNormalize0Hard(&localDirection, &localDirection);
	if (length == 0.0f) return;
	edF32Matrix4MulF32Vector4Hard(&localDirection, &matrix_0x20, &localDirection);
	float yaw = edF32Between_0_2Pi(std::atan2(localDirection.x, localDirection.z));
	float pitch = std::asin(std::clamp(-localDirection.y, -1.0f, 1.0f));
	float clampedYaw = std::clamp(yaw, pLink->field_0x40, pLink->field_0x44);
	float clampedPitch = std::clamp(pitch, pLink->field_0x48, pLink->field_0x4c);
	if (clampedYaw == yaw && clampedPitch == pitch) return;
	edF32VECTOR4 corrected = {
		std::sin(clampedYaw) * std::cos(clampedPitch) * length,
		-std::sin(clampedPitch) * length,
		std::cos(clampedYaw) * std::cos(clampedPitch) * length, 0.0f
	};
	if (pEnemy_0x60 != nullptr && pEnemy_0x60->pMeshTransform != nullptr)
		edF32Matrix4MulF32Vector4Hard(&corrected,
			&pEnemy_0x60->pMeshTransform->base.transformA, &corrected);
	pB->position.x = pA->position.x + corrected.x;
	pB->position.y = pA->position.y + corrected.y;
	pB->position.z = pA->position.z + corrected.z;
}

// 003e3ad0
void CBrazulBonePhysics::Func_0x3c(int index, edF32VECTOR4* rotation, edF32VECTOR4* direction)
{
	if (index == 0 || rotation == nullptr || direction == nullptr ||
		(uint)(index - 1) >= count_0xc) return;
	ActorBonePhysicsLink* pLink = field_0x4[index - 1];
	edF32VECTOR4 linkDirection;
	edF32Vector4SubHard(&linkDirection, &field_0x0[pLink->pointA]->position,
		&field_0x0[pLink->pointB]->position);
	edF32Matrix4MulF32Vector4Hard(&linkDirection, &matrix_0x20, &linkDirection);
	edF32Vector4SafeNormalize0Hard(&linkDirection, &linkDirection);
	edF32VECTOR4 axis;
	edF32Vector4CrossProductHard(&axis, &linkDirection, &pLink->field_0x30);
	float sine = std::clamp(edF32Vector4SafeNormalize0Hard(&axis, &axis), -1.0f, 1.0f);
	float angle = std::asin(sine);
	edF32VECTOR4 adjustment;
	edQuatFromAngAxis(angle, &adjustment, &axis);
	edQuatMul(rotation, rotation, &adjustment);
	edF32MATRIX4 rotationMatrix;
	edF32Matrix4FromAngAxisSoft(angle, &rotationMatrix, &axis);
	edF32Matrix4MulF32Vector4Hard(direction, &rotationMatrix, direction);
}

int CBrazulBonePhysics::UpdatePostAnimEffects()
{
	if (pEnemy_0x60 == nullptr || pEnemy_0x60->pCollisionData == nullptr) return 0;
	field_0x14 += field_0x78 * GetTimer()->cutsceneDeltaTime;
	if (field_0x14 < 0.0f || field_0x14 > 1.0f) {
		field_0x14 = std::clamp(field_0x14, 0.0f, 1.0f);
		field_0x78 = 0;
	}
	UpdateInverseMatrix(this);
	if (pEnemy_0x60->pAnimationController != nullptr && pEnemy_0x60->pMeshTransform != nullptr) {
		edF32MATRIX4* pBoneMatrix = pEnemy_0x60->pAnimationController->GetCurBoneMatrix(0xd3dbdab7);
		if (pBoneMatrix != nullptr)
			edF32Matrix4MulF32Vector4Hard(&field_0x0[0]->position,
				&pEnemy_0x60->pMeshTransform->base.transformA, (edF32VECTOR4*)&pBoneMatrix->da);
	}
	float height = pEnemy_0x60->pCollisionData->highestVertex.y;
	if ((pEnemy_0x60->pCollisionData->flags_0x4 & 2) == 0) {
		if (height < field_0x74) field_0x74 = height - 1.0f;
	}
	else {
		field_0x74 = height + 0.2f;
	}
	Simulate(1);
	ApplyToAnimation();
	return 1;
}
