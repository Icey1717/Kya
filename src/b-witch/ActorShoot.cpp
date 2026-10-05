#include "ActorShoot.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "ActorHero.h"
#include "EventManager.h"

CActorShoot::~CActorShoot()
{
	this->addOnGenerator.Term();

	return;
}

void CActorShoot::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	float fVar3;

	CActorAutonomous::Create(pByteCode);

	uVar1 = pByteCode->GetU32();
	this->field_0x350 = uVar1;
	uVar1 = pByteCode->GetU32();
	this->field_0x354 = uVar1;
	iVar2 = pByteCode->GetS32();
	this->staticMeshComponent.textureIndex = iVar2;
	iVar2 = pByteCode->GetS32();
	this->staticMeshComponent.meshIndex = iVar2;
	this->staticMeshComponent.Reset();
	fVar3 = pByteCode->GetF32();
	this->field_0x3c0 = fVar3;
	fVar3 = pByteCode->GetF32();
	this->field_0x3c4 = fVar3;
	fVar3 = pByteCode->GetF32();
	this->field_0x3c8 = fVar3;
	fVar3 = pByteCode->GetF32();
	this->field_0x3cc = fVar3;
	uVar1 = pByteCode->GetU32();
	this->field_0x3d0 = uVar1;
	fVar3 = pByteCode->GetF32();
	this->field_0x3d4 = fVar3;
	fVar3 = pByteCode->GetF32();
	this->field_0x3d8 = fVar3;
	fVar3 = pByteCode->GetF32();
	this->field_0x3dc = fVar3;
	fVar3 = pByteCode->GetF32();
	this->field_0x3e0 = fVar3;
	uVar1 = pByteCode->GetU32();
	this->field_0x3e8 = uVar1;
	uVar1 = pByteCode->GetU32();
	this->field_0x3e4 = uVar1;
	uVar1 = pByteCode->GetU32();
	this->field_0x3ec = uVar1;
	uVar1 = pByteCode->GetU32();
	this->field_0x3f0 = uVar1;

	this->addOnGenerator.Create(this, pByteCode);

	return;
}

void CActorShoot::Init()
{
	KyaUpdateObjA* pKVar1;

	CActorAutonomous::Init();
	ClearLocalData();
	memset(&this->altHierarchySetup, 0, 0x20);
	pKVar1 = this->subObjA;
	this->cachedBoundingSphere = pKVar1->boundingSphere;
	(this->altHierarchySetup).pBoundingSphere = &this->cachedBoundingSphere;
	this->cachedClipping = *this->hierarchySetup.clipping_0x0;
	(this->altHierarchySetup).clipping_0x0 = &this->cachedClipping;
	this->addOnGenerator.Init(0);

	return;
}

void CActorShoot::ComputeLighting()
{
	CScene::ptable.g_LightManager_004516b0->ComputeLighting(this->lightingFloat_0xe0, this, this->lightingFlags, &this->lightingConfig);

	return;
}

void CActorShoot::Reset()
{
	CActorAutonomous::Reset();
	ClearLocalData();

	return;
}

struct S_SAVE_CLASS_SHOOT
{
	uint field_0x0;
};

void CActorShoot::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_SHOOT* pSaveData = reinterpret_cast<S_SAVE_CLASS_SHOOT*>(pData);

	if (mode == 1) {
		pSaveData->field_0x0 = 0.0f < GetLifeInterface()->GetValue();
	}

	return;
}

void CActorShoot::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_SHOOT* pSaveData = reinterpret_cast<S_SAVE_CLASS_SHOOT*>(pData);

	if ((mode == 1) && (pSaveData->field_0x0 == 0)) {
		LifeAnnihilate();

		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;

		EvaluateDisplayState();

		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;

		SetBehaviour(SHOOT_BEHAVIOUR_FIRE_WAVE, 0xf, -1);
	}

	return;
}

CBehaviour* CActorShoot::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == SHOOT_BEHAVIOUR_FIRE_WAVE) {
		pBehaviour = &this->behaviourShootFireWave;
	}
	else {
		if (behaviourType == SHOOT_BEHAVIOUR_FIRE) {
			pBehaviour = &this->behaviourShootFire;
		}
		else {
			pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}

StateConfig CActorShoot::_gStateCfg_SHT[10] = {
	{ 0x00, 0x104 },
	{ 0x0C, 0x144 },
	{ 0x0D, 0x944 },
	{ 0x06, 0x9C4 },
	{ 0x0E, 0x9C4 },
	{ 0x0F, 0x9C4 },
	{ 0x10, 0x9C4 },
	{ 0x06, 0x104 },
	{ 0x12, 0x104 },
	{ 0x14, 0x101 },
};

StateConfig* CActorShoot::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 6) < 10);
		pStateConfig = _gStateCfg_SHT + state + -6;
	}

	return pStateConfig;
}

void CActorShoot::ChangeManageState(int state)
{
	bool bVar1;

	CActor::ChangeManageState(state);

	if ((state == 0) && (bVar1 = this->staticMeshComponent.HasMesh(), bVar1 != false)) {
		this->staticMeshComponent.Term(CScene::_scene_handleA);
	}

	return;
}

int CActorShoot::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CLifeInterface* pCVar2;
	int iVar3;
	edF32VECTOR4* v0;
	float fVar4;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	if (msg == 3) {
		pCVar2 = GetLifeInterface();
		fVar4 = pCVar2->GetValue();
		if (0.0f < fVar4) {
			if ((pSender->typeID != JAMGUT) && (pSender->typeID != ACTOR_HERO_PRIVATE)) {
				SetState(0xf, -1);

				return 1;
			}

			if ((this->field_0x3f0 & 4) == 0) {
				SetState(0xf, -1);

				return 1;
			}
		}
	}
	else {
		if (msg != 2) {
			iVar3 = CActor::InterpretMessage(pSender, msg, pMsgParam);
			return iVar3;
		}

		_msg_hit_param* pHitParam = (_msg_hit_param*)pMsgParam;

		if ((this->field_0x43c != 0) && (pHitParam->projectileType == 4)) {
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;
			this->dynamicExt.field_0x6c = 0.0f;

			eStack16.y = 1.0f;
			eStack16.x = 0.0f;
			eStack16.z = 0.0f;
			eStack16.w = 0.0f;

			edF32Vector4ScaleHard(300.0f, &eStack16, &eStack16);
			edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack32, &eStack16);
			v0 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(v0, v0, &eStack32);
			fVar4 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar4;

			LifeDecrease(pHitParam->damage);

			pCVar2 = GetLifeInterface();
			fVar4 = pCVar2->GetValue();
			if (fVar4 <= 0.0f) {
				SetState(0xf, -1);
			}
			else {
				if (this->field_0x438 == (CActorHero*)0x0) {
					SetState(8, 0x13);
					this->field_0x438 = CActorHero::_gThis;
				}
				else {
					if (this->currentAnimType == 0x13) {
						RestartCurAnim();
					}
					else {
						PlayAnim(0x13);
					}
				}
			}

			return 1;
		}
	}

	return 0;
}

void CActorShoot::ClearLocalData()
{
	float fVar1;
	float fVar2;
	bool bVar3;
	int iVar4;
	edF32MATRIX4* peVar5;
	edF32MATRIX4* peVar6;
	float fVar7;
	CAnimation* pAnim;

	bVar3 = this->staticMeshComponent.HasMesh();
	if (bVar3 != false) {
		this->staticMeshComponent.Term((ed_3D_Scene*)0x0);
	}
	this->staticMeshComponent.Reset();

	this->field_0x438 = (CActorHero*)0x0;
	this->field_0x43c = true;

	this->field_0x3f8 = this->field_0x3cc;
	this->field_0x3fc = this->field_0x3d4;
	this->field_0x3f4 = 0.0f;
	this->field_0x400 = 0;
	this->field_0x440 = 0;

	this->lightAmbient = gF32Vector4Zero;
	this->lightDirection = gF32Matrix4Unit;
	this->lightColor = gF32Matrix4Unit;
	
	(this->lightingConfig).pLightAmbient = &this->lightAmbient;
	(this->lightingConfig).pLightDirections = &this->lightDirection;
	(this->lightingConfig).pLightColorMatrix = &this->lightColor;

	this->field_0x43f = 0;
	this->field_0x43e = false;

	pAnim = this->pAnimationController;
	iVar4 = GetIdMacroAnim(0x11);
	if (iVar4 < 0) {
		fVar7 = 0.0f;
	}
	else {
		fVar7 = pAnim->GetAnimLength(iVar4, 0);
	}

	if (this->field_0x3cc < fVar7 + 0.15f) {
		this->field_0x3cc = fVar7 + 0.15f;
	}

	return;
}

void CActorShoot::BehaviourShootFire_Manage(CBehaviourShootFire* pBehaviour)
{
	ed_3d_hierarchy_node* peVar2;
	bool bVar5;
	uint uVar7;
	edF32VECTOR4* v1;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	ComputeInvincibility();
	pBehaviour->fireshot.ManageShots();
	ComputeTimeAndParamToShoot();
	uVar7 = GetStateFlags(this->actorState) & 0x800;
	if ((uVar7 != 0) && (this->field_0x438 != (CActorHero*)0x0)) {
		SetLookingAtOn(0.0f);
		SetLookingAtRotationHeight(3.1415927f, &this->field_0x438->currentLocation);
	}
	switch(this->actorState) {
	case 6:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		uVar7 = CheckArea();
		if (uVar7 == 1) {
			bVar5 = (this->staticMeshComponent).textureIndex != -1;
			if (bVar5) {
				bVar5 = (this->staticMeshComponent).meshIndex != -1;
			}
			if (bVar5) {
				SetState(7, -1);
			}
			else {
				SetState(8, -1);
			}
		}
		break;
	case 7:
		if (this->field_0x438 != (CActorHero*)0x0) {
			SV_UpdateOrientationToPosition2D(0.7853982f, &this->field_0x438->currentLocation);
		}
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		fVar8 = this->timeInAir;
		eStack32 = gF32Vector4UnitY;
		v1 = &this->currentLocation;
		bVar5 = this->staticMeshComponent.HasMesh();
		if ((bVar5 != false) && (v1 != (edF32VECTOR4*)0x0)) {
			fVar9 = 0.4f;
			if ((fVar8 <= 0.4f) && (fVar9 = fVar8, fVar8 < 0.0f)) {
				fVar9 = 0.0f;
			}
			edFIntervalLERP(fVar9, 0.0f, 0.4f, 0.0f, 0.32f);
			// The original uses the clamped time and discards the LERP result.
			edF32Vector4ScaleHard(-(0.32f - fVar9), &eStack32, &eStack32);
			edF32Vector4AddHard(&eStack16, v1, &eStack32);
			peVar2 = (this->staticMeshComponent).pMeshTransformData;
			if (peVar2 != (ed_3d_hierarchy_node*)0x0) {
				peVar2->base.transformA.rowT = eStack16;
			}
		}
		if (0.4f < this->timeInAir) {
			SetState(8, -1);
		}
		break;
	case 8:
		if (this->field_0x438 != (CActorHero*)0x0) {
			SV_UpdateOrientationToPosition2D(0.7853982f, &this->field_0x438->currentLocation);
		}
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		uVar7 = CheckArea();
		if (uVar7 == 2) {
			if ((this->field_0x3f0 & 1) == 0) {
				if (this->currentAnimType == 0x13) {
					SetState(6, 0x13);
				}
				else {
					SetState(6, -1);
				}
			}
			else {
				SetState(0xe, -1);
			}
		}
		else {
			bVar5 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
			if (bVar5 && (this->currentAnimType == 0xd)) {
				SetState(9, -1);
			}
		}
		break;
	case 9:
		StateShootChase();
		break;
	case 0xc:
		StateShootFire(pBehaviour);
		break;
	case 0xd:
		StateShootComeBack();
		break;
	case 0xe:
		bVar5 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
		if (bVar5) {
			SetState(6, -1);
		}
		break;
	case 0xf:
		bVar5 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
		if (bVar5) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
		}
	}
	if (this->currentAnimType == 0x13) {
		bVar5 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
		if (bVar5) {
			SetState(this->actorState, -1);
		}
	}
	return;
}

void CActorShoot::BehaviourFireWave_Manage(CBehaviourShootFireWave* pBehaviour)
{
	ed_3d_hierarchy_node* peVar1;
	bool bVar4;
	uint uVar5;
	edF32VECTOR4* v1;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	ComputeInvincibility();
	pBehaviour->conicalWaveShoot.Manage();
	ComputeTimeAndParamToShoot();
	switch(this->actorState) {
	case 6:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		uVar5 = CheckArea();
		if (uVar5 == 1) {
			bVar4 = (this->staticMeshComponent).textureIndex != -1;
			if (bVar4) {
				bVar4 = (this->staticMeshComponent).meshIndex != -1;
			}
			if (bVar4) {
				SetState(7, -1);
			}
			else {
				SetState(8, -1);
			}
		}
		break;
	case 7:
		if (this->field_0x438 != (CActorHero*)0x0) {
			SV_UpdateOrientationToPosition2D(0.7853982f, &this->field_0x438->currentLocation);
		}
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		fVar6 = this->timeInAir;
		local_20 = gF32Vector4UnitY;
		v1 = &this->currentLocation;
		bVar4 = this->staticMeshComponent.HasMesh();
		if ((bVar4 != false) && (v1 != (edF32VECTOR4*)0x0)) {
			fVar7 = 0.4f;
			if ((fVar6 <= 0.4f) && (fVar7 = fVar6, fVar6 < 0.0f)) {
				fVar7 = 0.0f;
			}
			edFIntervalLERP(fVar7, 0.0f, 0.4f, 0.0f, 0.32f);
			// The original uses the clamped time and discards the LERP result.
			edF32Vector4ScaleHard(-(0.32f - fVar7), &local_20, &local_20);
			edF32Vector4AddHard(&local_10, v1, &local_20);
			peVar1 = (this->staticMeshComponent).pMeshTransformData;
			if (peVar1 != (ed_3d_hierarchy_node*)0x0) {
				peVar1->base.transformA.rowT = local_10;
			}
		}
		if (0.4f < this->timeInAir) {
			SetState(8, -1);
		}
		break;
	case 8:
		if (this->field_0x438 != (CActorHero*)0x0) {
			SV_UpdateOrientationToPosition2D(0.7853982f, &this->field_0x438->currentLocation);
		}
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		uVar5 = CheckArea();
		if (uVar5 == 2) {
			if ((this->field_0x3f0 & 1) == 0) {
				if (this->currentAnimType == 0x13) {
					SetState(6, 0x13);
				}
				else {
					SetState(6, -1);
				}
			}
			else {
				SetState(0xe, -1);
			}
		}
		else {
			bVar4 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
			if (bVar4 && (this->currentAnimType == 0xd)) {
				SetState(9, -1);
			}
		}
		break;
	case 9:
		StateShootChase();
		break;
	case 0xc:
		StateShootFireWave(pBehaviour);
		break;
	case 0xd:
		StateShootComeBack();
		break;
	case 0xf:
		bVar4 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
		if (bVar4) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
		}
	}
	if (this->currentAnimType == 0x13) {
		bVar4 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
		if (bVar4) {
			SetState(this->actorState, -1);
		}
	}
	return;
}

void CActorShoot::ComputeInvincibility()
{
	switch(this->actorState) {
	case 6:
		this->field_0x43c = true;
		break;
	case 7:
	case 8:
		if ((this->field_0x3ec & 1) == 0) {
			this->field_0x43c = true;
		}
		else {
			this->field_0x43c = false;
		}
		break;
	case 9:
		if ((this->field_0x3ec & 4) == 0) {
			this->field_0x43c = true;
		}
		else {
			this->field_0x43c = false;
		}
		break;
	default:
		this->field_0x43c = true;
		break;
	case 0xc:
		if ((this->field_0x3ec & 8) == 0) {
			this->field_0x43c = true;
		}
		else {
			this->field_0x43c = false;
		}
		break;
	case 0xd:
		if ((this->field_0x3ec & 0x20) == 0) {
			this->field_0x43c = true;
		}
		else {
			this->field_0x43c = false;
		}
		break;
	case 0xe:
		this->field_0x43c = false;
		break;
	case 0xf:
		this->field_0x43c = true;
	}
	return;
}

void CActorShoot::ComputeTimeAndParamToShoot()
{
	Timer* pTVar1;
	bool bVar2;
	bool bVar3;

	bVar3 = false;
	bVar2 = false;
	pTVar1 = GetTimer();
	this->field_0x3fc = this->field_0x3fc + pTVar1->cutsceneDeltaTime;
	pTVar1 = GetTimer();
	this->field_0x3f8 = this->field_0x3f8 + pTVar1->cutsceneDeltaTime;
	if (this->field_0x3d4 <= this->field_0x3fc) {
		this->field_0x3fc = this->field_0x3d4;
		bVar3 = false;
		if (this->field_0x3cc <= this->field_0x3f8) {
			if ((this->actorState == 0xc) &&
				(this->currentAnimType == 0x11)) {
				pTVar1 = GetTimer();
				this->field_0x3f4 = this->field_0x3f4 + pTVar1->cutsceneDeltaTime;
			}
			bVar2 = this->field_0x3c8 < this->field_0x3f4;
			bVar3 = true;
		}
		if (this->field_0x400 == this->field_0x3d0) {
			this->field_0x400 = 0;
			this->field_0x3fc = 0.0f;
		}
	}
	this->field_0x43e = bVar3;
	this->field_0x43d = bVar2;
	return;
}

uint CActorShoot::CheckArea()
{
	CEventManager* pEventManager;
	CActorHero* pHero;
	ed_zone_3d* pOuterZone;
	ed_zone_3d* pInnerZone;
	int innerResult;
	int outerResult;
	uint result;
	float x;
	float y;
	float z;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pOuterZone = (ed_zone_3d*)0x0;
	pInnerZone = (ed_zone_3d*)0x0;
	if (this->field_0x3e4 != 0xffffffff) {
		pOuterZone = edEventGetChunkZone(pEventManager->activeChunkId, this->field_0x3e4);
	}

	if (((this->field_0x3f0 & 2) == 0) && (this->field_0x3e8 != 0xffffffff)) {
		pInnerZone = edEventGetChunkZone(pEventManager->activeChunkId, this->field_0x3e8);
	}

	pHero = CActorHero::_gThis;
	if ((this->field_0x3f0 & 2) == 0) {
		innerResult = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pInnerZone, &pHero->currentLocation, 0);
	}
	else {
		x = pHero->currentLocation.x - this->currentLocation.x;
		y = pHero->currentLocation.y - this->currentLocation.y;
		z = pHero->currentLocation.z - this->currentLocation.z;
		innerResult = 1;
		if (this->field_0x3e0 <= sqrtf(x * x + y * y + z * z)) {
			innerResult = 2;
		}
	}

	outerResult = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pOuterZone, &pHero->currentLocation, 0);
	if ((outerResult == 1) || (innerResult == 1)) {
		result = 1;
		if (innerResult == 1) {
			this->field_0x438 = pHero;
		}
		else {
			result = 0xffffffff;
		}
	}
	else {
		this->field_0x438 = (CActorHero*)0x0;
		result = 2;
	}

	return result;
}

int CActorShoot::UpdateOrientationAndLookingAt(CActor* pLookAtActor)
{
	int animType;
	float dotProduct;
	float angle;
	edF32VECTOR4 direction;
	edF32VECTOR4 targetDirection;
	edF32VECTOR4 currentDirection;

	animType = -1;
	if (pLookAtActor != (CActor*)0x0) {
		edF32Vector4SubHard(&direction, &pLookAtActor->currentLocation, &this->currentLocation);
		edF32Vector4NormalizeHard(&direction, &direction);
		currentDirection.x = this->rotationQuat.x;
		currentDirection.y = 0.0f;
		currentDirection.z = this->rotationQuat.z;
		currentDirection.w = 0.0f;
		edF32Vector4NormalizeHard(&currentDirection, &currentDirection);
		targetDirection.x = direction.x;
		targetDirection.y = 0.0f;
		targetDirection.z = direction.z;
		targetDirection.w = 0.0f;
		edF32Vector4NormalizeHard(&targetDirection, &targetDirection);
		GetTimer();
		dotProduct = edF32Vector4DotProductHard(&currentDirection, &targetDirection);
		angle = -1.0f;
		if (1.0f < dotProduct) {
			angle = 1.0f;
		}
		else {
			if (-1.0f <= dotProduct) {
				angle = dotProduct;
			}
		}
		angle = acosf(angle);
		if (currentDirection.x * targetDirection.z - targetDirection.x * currentDirection.z < 0.0f) {
			if ((0.7853982f < angle) || (this->field_0x43f != 0)) {
				this->field_0x43f = 1;
				animType = 0xf;
			}
		}
		else {
			if ((0.7853982f < angle) || (this->field_0x43f != 0)) {
				this->field_0x43f = 1;
				animType = 0xe;
			}
		}
	}

	return animType;
}

void CActorShoot::StateShootChase()
{
	uint areaResult;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;

	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.rotSpeed = this->field_0x3c4;
	movParamsIn.speed = this->field_0x3c0;
	movParamsIn.acceleration = 10.0f;
	movParamsIn.flags = 0x452;
	SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &this->field_0x438->currentLocation);
	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	if (movParamsOut.moveVelocity <= this->field_0x3d8) {
		this->dynamic.speed = 0.0f;
		SetState(0xc, -1);
	}
	else {
		areaResult = CheckArea();
		if (areaResult == 2) {
			SetState(0xd, -1);
		}
	}

	return;
}

void CActorShoot::StateShootFire(CBehaviourShootFire* pBehaviour)
{
	bool bOrientationReached;
	int animType;
	uint areaResult;
	float x;
	float z;
	edF32VECTOR4 shotPosition;
	edF32VECTOR4 shotDirection;
	edF32VECTOR4 shotTarget;

	animType = UpdateOrientationAndLookingAt(this->field_0x438);
	bOrientationReached = false;
	if (((this->field_0x43f != 0) && (this->field_0x438 != (CActorHero*)0x0)) &&
		(this->currentAnimType != 0x11)) {
		bOrientationReached = SV_UpdateOrientationToPosition2D(this->field_0x3c4, &this->field_0x438->currentLocation);
	}
	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	if (bOrientationReached == true) {
		this->field_0x43f = 0;
	}
	areaResult = CheckArea();
	if (areaResult == 2) {
		SetState(0xd, -1);
	}
	else {
		x = this->field_0x438->currentLocation.x - this->currentLocation.x;
		z = this->field_0x438->currentLocation.z - this->currentLocation.z;
		if (this->field_0x3dc < sqrtf(x * x + 0.0f + z * z)) {
			SetState(9, -1);
		}
		else {
			if ((this->field_0x43f == 0) && (this->field_0x43d != false)) {
				SV_GetBoneWorldPosition(pBehaviour->field_0x2b0, &shotPosition);
				edF32Vector4SubHard(&shotDirection, &this->field_0x438->currentLocation, &shotPosition);
				shotDirection.y = 0.0f;
				edF32Vector4NormalizeHard(&shotDirection, &shotDirection);
				edF32Vector4ScaleHard(pBehaviour->field_0x2b4, &shotDirection, &shotDirection);
				edF32Vector4AddHard(&shotTarget, &shotPosition, &shotDirection);
				pBehaviour->fireshot.FireNewShotStraight(&shotPosition, &shotTarget, this);
				this->field_0x3f4 = 0.0f;
				this->field_0x3f8 = 0.0f;
				this->field_0x400 = this->field_0x400 + 1;
			}
			if (this->currentAnimType == 0x11) {
				if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
					if (this->field_0x43f == 0) {
						if (this->field_0x43e == false) {
							PlayAnim(0x10);
						}
						else {
							RestartCurAnim();
							this->field_0x43e = false;
						}
					}
					else {
						PlayAnim(animType);
					}
				}
			}
			else {
				if (this->field_0x43f == 0) {
					if (this->field_0x43e == false) {
						PlayAnim(0x10);
					}
					else {
						PlayAnim(0x11);
						this->field_0x43e = false;
					}
				}
				else {
					PlayAnim(animType);
				}
			}
		}
	}

	return;
}

void CActorShoot::StateShootFireWave(CBehaviourShootFireWave* pBehaviour)
{
	uint areaResult;
	float x;
	float y;
	float z;
	edF32VECTOR4 direction;

	if (this->field_0x438 != (CActorHero*)0x0) {
		SV_UpdateOrientationToPosition2D(this->field_0x3c4, &this->field_0x438->currentLocation);
	}
	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	if ((this->field_0x43d != false) && (pBehaviour->conicalWaveShoot.field_0x24 == 0)) {
		direction = this->rotationQuat;
		direction.y = 0.0f;
		edF32Vector4NormalizeHard(&direction, &direction);
		pBehaviour->conicalWaveShoot.Fire(&this->currentLocation, &direction);
		this->field_0x3f8 = 0.0f;
		this->field_0x400 = this->field_0x400 + 1;
	}
	areaResult = CheckArea();
	if (areaResult == 2) {
		SetState(0xd, -1);
	}
	else {
		x = this->field_0x438->currentLocation.x - this->currentLocation.x;
		y = this->field_0x438->currentLocation.y - this->currentLocation.y;
		z = this->field_0x438->currentLocation.z - this->currentLocation.z;
		if (this->field_0x3dc < sqrtf(x * x + y * y + z * z)) {
			SetState(9, -1);
		}
	}

	return;
}

void CActorShoot::StateShootComeBack()
{
	uint areaResult;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;

	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.rotSpeed = this->field_0x3c4;
	movParamsIn.speed = this->field_0x3c0;
	movParamsIn.acceleration = 10.0f;
	movParamsIn.flags = 0x452;
	SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &this->baseLocation);
	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	if (movParamsOut.moveVelocity < 1.0f) {
		this->dynamic.speed = 0.0f;
		if ((this->field_0x3f0 & 1) == 0) {
			if (this->currentAnimType == 0x13) {
				SetState(6, 0x13);
			}
			else {
				SetState(6, -1);
			}
		}
		else {
			SetState(0xe, -1);
		}
	}
	areaResult = CheckArea();
	if (areaResult == 1) {
		SetState(9, -1);
	}

	return;
}

void CBehaviourShoot::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourShoot::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorShoot*>(pOwner);

	return;
}

int CBehaviourShoot::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourShoot::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourShootFire::Create(ByteCode* pByteCode)
{
	this->field_0x2b0 = pByteCode->GetU32();
	this->field_0x8 = pByteCode->GetF32();
	this->field_0x2b4 = pByteCode->GetF32();

	this->fireshot.Create(pByteCode);

	return;
}

void CBehaviourShootFire::Init(CActor * pOwner)
{
	this->fireshot.Init();

	return;
}

void CBehaviourShootFire::Manage()
{
	this->pOwner->BehaviourShootFire_Manage(this);

	return;
}

void CBehaviourShootFire::Draw()
{
	return;
}

void CBehaviourShootFire::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorShoot* pShoot;

	CBehaviourShoot::Begin(pOwner, newState, newAnimationType);

	this->fireshot.Reset();

	if (newState == -1) {
		pShoot = this->pOwner;
		pShoot->SetState(6, -1);
	}
	else {
		pShoot = this->pOwner;
		pShoot->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourShootFire::InitState(int newState)
{
	int iVar1;
	ed_3d_hierarchy_node* peVar2;
	CCollision* pCVar3;
	bool bVar4;
	StateConfig* pSVar5;
	edF32VECTOR4* peVar6;
	uint uVar7;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CActorShoot* pShoot;

	pShoot = this->pOwner;

	if ((pShoot->GetStateFlags(newState) & 0x800) == 0) {
		iVar1 = pShoot->prevActorState;

		if ((pShoot->GetStateFlags(iVar1) & 0x800) != 0) {
			pShoot->pAnimationController->UnRegisterBone(pShoot->field_0x350);
			pShoot->pAnimationController->UnRegisterBone(pShoot->field_0x354);
			pShoot->SetLookingAtOff();
		}
	}

	if ((pShoot->GetStateFlags(newState) & 0x800) != 0) {
		iVar1 = pShoot->prevActorState;

		if ((pShoot->GetStateFlags(iVar1) & 0x800) == 0) {
			pShoot->pAnimationController->RegisterBone(pShoot->field_0x350);
			pShoot->pAnimationController->RegisterBone(pShoot->field_0x354);
			pShoot->SetLookingAtBones(pShoot->field_0x354, pShoot->field_0x350);
			pShoot->SetLookingAtBounds(-0.08726646f, 0.08726646f, -0.7853982f, 0.7853982f);
		}
	}

	if (newState != 8) {
		if (newState == 0xf) {
			pCVar3 = pShoot->pCollisionData;
			peVar6 = pShoot->GetBottomPosition();
			edF32Vector4AddHard(&eStack16, &pCVar3->highestVertex, peVar6);
			edF32Vector4ScaleHard(0.5f, &eStack16, &eStack16);
			pShoot->addOnGenerator.Generate(&eStack16);
		}
		else {
			if (newState == 0xc) {
				pShoot->pAnimationController->RegisterBone(this->field_0x2b0);

				pShoot->dynamicExt.normalizedTranslation.x = 0.0f;
				pShoot->dynamicExt.normalizedTranslation.y = 0.0f;
				pShoot->dynamicExt.normalizedTranslation.z = 0.0f;
				pShoot->dynamicExt.normalizedTranslation.w = 0.0f;
				pShoot->dynamicExt.field_0x6c = 0.0f;
				pShoot->dynamic.speed = 0.0f;
			}
			else {
				if (newState == 7) {
					if ((pShoot->field_0x3f0 & 1) != 0) {
						pShoot->flags = pShoot->flags & 0xffffff7f;
						pShoot->flags = pShoot->flags | 0x20;
						pShoot->EvaluateDisplayState();
					}

					bVar4 = (pShoot->staticMeshComponent).textureIndex != -1;
					if (bVar4) {
						bVar4 = (pShoot->staticMeshComponent).meshIndex != -1;
					}

					if (bVar4) {
						pShoot->staticMeshComponent.Init(CScene::_scene_handleA, (ed_g3d_manager*)0x0, &pShoot->altHierarchySetup, (char*)0x0);
						eStack48 = gF32Vector4UnitY;
						peVar6 = &pShoot->currentLocation;
						bVar4 = pShoot->staticMeshComponent.HasMesh();
						if ((bVar4 != false) && (peVar6 != (edF32VECTOR4*)0x0)) {
							edFIntervalLERP(0.0f, 0.0f, 0.4f, 0.0f, 0.32f);
							edF32Vector4ScaleHard(-0.32f, &eStack48, &eStack48);
							edF32Vector4AddHard(&eStack32, peVar6, &eStack48);
							peVar2 = (pShoot->staticMeshComponent).pMeshTransformData;
							if (peVar2 != (ed_3d_hierarchy_node*)0x0) {
								pShoot->staticMeshComponent.pMeshTransformData->base.transformA.rowT = eStack32;
							}
						}
					}
				}
				else {
					if ((newState == 6) && ((pShoot->field_0x3f0 & 1) != 0)) {
						pShoot->flags = pShoot->flags & 0xffffff7f;
						pShoot->flags = pShoot->flags | 0x20;
						pShoot->EvaluateDisplayState();
					}
				}
			}
		}
	}

	return;
}

void CBehaviourShootFire::TermState(int oldState, int newState)
{
	bool bVar1;
	CActorShoot* pShoot;

	pShoot = this->pOwner;
	if (oldState == 0xe) {
		bVar1 = pShoot->staticMeshComponent.HasMesh();
		if (bVar1 != false) {
			pShoot->staticMeshComponent.Term(CScene::_scene_handleA);
		}
	}
	else {
		if (oldState == 0xc) {
			pShoot->pAnimationController->UnRegisterBone(this->field_0x2b0);
			pShoot->SetLookingAtOff();
		}
		else {
			if (oldState == 7) {
				if ((pShoot->field_0x3f0 & 1) != 0) {
					pShoot->flags = pShoot->flags & 0xffffff5f;
					pShoot->EvaluateDisplayState();
				}
			}
			else {
				if ((oldState == 6) && ((pShoot->field_0x3f0 & 1) != 0)) {
					pShoot->flags = pShoot->flags & 0xffffff5f;
					pShoot->EvaluateDisplayState();
				}
			}
		}
	}

	return;
}

int CBehaviourShootFire::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CBehaviourShootFire::Reset()
{
	this->fireshot.Reset();

	return;
}

void CBehaviourShootFireWave::Create(ByteCode* pByteCode)
{
	this->conicalWaveShoot.Create(pByteCode);

	return;
}

void CBehaviourShootFireWave::Init(CActor* pOwner)
{
	this->conicalWaveShoot.Init(pOwner);

	return;
}

void CBehaviourShootFireWave::Manage()
{
	this->pOwner->BehaviourFireWave_Manage(this);

	return;
}

void CBehaviourShootFireWave::Draw()
{
	this->conicalWaveShoot.Draw();

	return;
}

void CBehaviourShootFireWave::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CActorShoot* pShoot;

	this->pOwner = (CActorShoot*)pOwner;

	AltReset();

	if (newState == -1) {
		pShoot = this->pOwner;
		pShoot->SetState(6, -1);
	}
	else {
		pShoot = this->pOwner;
		pShoot->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourShootFireWave::InitState(int newState)
{
	int iVar1;
	ed_3d_hierarchy_node* peVar2;
	CCollision* pCVar3;
	bool bVar4;
	StateConfig* pSVar5;
	edF32VECTOR4* peVar6;
	uint uVar7;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CActorShoot* pShoot;

	pShoot = this->pOwner;

	if ((pShoot->GetStateFlags(newState) & 0x800) == 0) {
		if ((pShoot->GetStateFlags(pShoot->prevActorState) & 0x800) != 0) {
			pShoot->pAnimationController->UnRegisterBone(pShoot->field_0x350);
			pShoot->pAnimationController->UnRegisterBone(pShoot->field_0x354);
			pShoot->SetLookingAtOff();
		}
	}

	if ((pShoot->GetStateFlags(newState) & 0x800) != 0) {
		if ((pShoot->GetStateFlags(pShoot->prevActorState) & 0x800) == 0) {
			pShoot->pAnimationController->RegisterBone(pShoot->field_0x350);
			pShoot->pAnimationController->RegisterBone(pShoot->field_0x354);
			pShoot->SetLookingAtBones(pShoot->field_0x354, pShoot->field_0x350);
			pShoot->SetLookingAtBounds(-0.08726646f, 0.08726646f, -0.7853982f, 0.7853982f);
		}
	}

	if (newState == 0xf) {
		pCVar3 = pShoot->pCollisionData;
		peVar6 = pShoot->GetBottomPosition();
		edF32Vector4AddHard(&eStack16, &pCVar3->highestVertex, peVar6);
		edF32Vector4ScaleHard(0.5f, &eStack16, &eStack16);
		pShoot->addOnGenerator.Generate(&eStack16);
	}
	else {
		if ((newState != 0xc) && (newState == 7)) {
			if ((pShoot->field_0x3f0 & 1) != 0) {
				pShoot->flags = pShoot->flags & 0xffffff7f;
				pShoot->flags = pShoot->flags | 0x20;
				pShoot->EvaluateDisplayState();
			}

			bVar4 = (pShoot->staticMeshComponent).textureIndex != -1;
			if (bVar4) {
				bVar4 = (pShoot->staticMeshComponent).meshIndex != -1;
			}

			if (bVar4) {
				pShoot->staticMeshComponent.Init(CScene::_scene_handleA, (ed_g3d_manager*)0x0, &pShoot->altHierarchySetup, (char*)0x0);
				eStack48 = gF32Vector4UnitY;
				peVar6 = &pShoot->currentLocation;
				bVar4 = ((StaticMeshComponentAdvanced*)&pShoot->staticMeshComponent)->HasMesh();
				if ((bVar4 != false) && (peVar6 != (edF32VECTOR4*)0x0)) {
					edFIntervalLERP(0.0f, 0.0f, 0.4f, 0.0f, 0.32f);
					edF32Vector4ScaleHard(-0.32f, &eStack48, &eStack48);
					edF32Vector4AddHard(&eStack32, peVar6, &eStack48);
					peVar2 = (pShoot->staticMeshComponent).pMeshTransformData;
					if (peVar2 != (ed_3d_hierarchy_node*)0x0) {
						peVar2->base.transformA.rowT = eStack32;
					}
				}
			}
		}
	}

	return;
}

void CBehaviourShootFireWave::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourShootFireWave::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CBehaviourShootFireWave::Reset()
{
	this->conicalWaveShoot.Reset();

	return;
}

void CBehaviourShootFireWave::AltReset()
{
	this->conicalWaveShoot.Reset();

	return;
}
