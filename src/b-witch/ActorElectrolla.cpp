#include "ActorElectrolla.h"
#include "ActorHero.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "ActorMovable.h"
#include "AnmManager.h"
#include "CameraManager.h"
#include "DlistManager.h"
#include "FileManager3D.h"
#include "TimeController.h"
#include "edDlist.h"

static float scanDelay = 2.0f;
static float scanUp = 2.0f;
static float scanDown = 0.5f;

StateConfig CActorElectrolla::_gStateCfg_ELE[8] =
{
	StateConfig(6, 4),
	StateConfig(8, 4),
	StateConfig(7, 4),
	StateConfig(9, 0x100),
	StateConfig(9, 0x100),
	StateConfig(10, 0x100),
	StateConfig(8, 0x100),
	StateConfig(0, 0)
};

CSharedLights<CLightOmni, 3> CActorElectrolla::_gELE_Lights;

void Criterion_Near(CActor* pActor, void* pParams);

// 0x00131c20: scanning only reacts to moving actors.
static void CriterionElectrollaMoving(CActor* pActor, void* pParams)
{
	_criterion_near_params* pNearParams = reinterpret_cast<_criterion_near_params*>(pParams);
	if ((pNearParams->pActor != pActor) && pActor->IsKindOfObject(OBJ_TYPE_MOVABLE) &&
		(pActor->typeID != 5) && (static_cast<CActorMovable*>(pActor)->dynamic.speed > 0.05f)) {
		edF32VECTOR4 delta;
		edF32Vector4SubHard(&delta, &pActor->currentLocation, &pNearParams->pActor->currentLocation);

		float distance = edF32Vector4DotProductHard(&delta, &delta);
		if (distance <= pNearParams->field_0x10) {
			if (distance < pNearParams->nearestDistance) {
				pNearParams->nearestDistance = distance;
			}

			if (pNearParams->pTable->nbEntries < 64) {
				pNearParams->aDistances[pNearParams->pTable->nbEntries] = sqrtf(distance);
				pNearParams->pTable->Add(pActor);
			}
		}
	}
}

// 0x00131d10: once alerted, stationary movable actors also count.
static void CriterionElectrollaMovable(CActor* pActor, void* pParams)
{
	_criterion_near_params* pNearParams = reinterpret_cast<_criterion_near_params*>(pParams);

	if ((pNearParams->pActor != pActor) && pActor->IsKindOfObject(OBJ_TYPE_MOVABLE) && (pActor->typeID != 5)) {
		Criterion_Near(pActor, pParams);
	}
}

CActorElectrolla::CActorElectrolla()
{
	this->field_0x170 = (CFxSparkNoAlloc<3, 12>*)0x0;

	return;
}

CActorElectrolla::~CActorElectrolla()
{
	delete[] this->field_0x170;
}

void CActorElectrolla::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);
	this->field_0x174 = pByteCode->GetS32();
	this->field_0x178 = pByteCode->GetF32();
	this->field_0x17c = pByteCode->GetF32();
	this->field_0x180 = pByteCode->GetF32();
	this->field_0x194 = pByteCode->GetF32();
	this->field_0x198 = pByteCode->GetF32();
	this->field_0x19c = pByteCode->GetF32();
	this->materialId = pByteCode->GetS32();
	int sparkMaterialId = pByteCode->GetS32();
	this->field_0x1c4.Create(pByteCode);
	SV_InstallMaterialId(this->materialId);
	SV_InstallMaterialId(sparkMaterialId);

	this->field_0x170 = new CFxSparkNoAlloc<3, 12>[this->field_0x174];
	for (int i = 0; i < this->field_0x174; i++) {
		CFxSparkNoAlloc<3, 12>* pSpark = this->field_0x170 + i;
		pSpark->Create(3, 12, pSpark->aVectorData, pSpark->aFloatData, sparkMaterialId);
		pSpark->field_0xe4 = pSpark->aUnknown;
		pSpark->SetParameters(0.5f, 0.025f, 20.0f, 0.25f, 8.0f, 3);
	}
}

void CActorElectrolla::Init()
{
	CActor::Init();

	this->field_0x1c4.Init();
	this->field_0x188 = 0.0f;
	this->field_0x18c = 0.6f;

	for (int i = 0; i < this->field_0x174; i++) {
		CFxSparkNoAlloc<3, 12>* pSpark = this->field_0x170 + i;
		pSpark->Init(this->sectorId);
		pSpark->field_0x90 = 0x80f09614;
		pSpark->vector_0x80 = edF32VECTOR4{ 0.0f, 0.25f, 0.25f, 0.0f };
	}

	this->dlistPatchId = GameDListPatch_Register(this, 4, 0);
	_gELE_Lights.Init(2.0f, 4.0f, 0x101004, 0xa0a010, 0);
}

void CActorElectrolla::Term()
{
	CActor::Term();

	_gELE_Lights.Term();
}

void CActorElectrolla::Reset()
{
	CActor::Reset();

	this->field_0x1c4.Reset(this);
}

void CActorElectrolla::SaveContext(void* pData, uint mode, uint maxSize)
{
	this->field_0x1c4.SaveContext(reinterpret_cast<S_SAVE_CLASS_SWITCH_CAMERA*>(pData));

	return;
}

void CActorElectrolla::LoadContext(void* pData, uint mode, uint maxSize)
{
	if (mode == 0x20001) {
		this->field_0x1c4.LoadContext(reinterpret_cast<S_SAVE_CLASS_SWITCH_CAMERA*>(pData));
	}

	return;
}

CBehaviour* CActorElectrolla::BuildBehaviour(int behaviourType)
{
	if (behaviourType == ELECTROLLA_BEHAVIOUR_STAND) {
		return &this->behaviourStand;
	}

	return CActor::BuildBehaviour(behaviourType);
}

StateConfig* CActorElectrolla::GetStateCfg(int state)
{
	if (state < ELECTROLLA_STATE_STAND) {
		return CActor::GetStateCfg(state);
	}

	assert((state - ELECTROLLA_STATE_STAND) < 8);

	return _gStateCfg_ELE + state - ELECTROLLA_STATE_STAND;
}

void CActorElectrolla::ChangeManageState(int state)
{
	CActor::ChangeManageState(state);
	if (state == 0) {
		this->pAnimationController->UnRegisterBone(0x45477cb3);
	}
	else {
		this->pAnimationController->RegisterBone(0x45477cb3);
	}
}

void CActorElectrolla::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	if (this->currentAnimType == 8) {
		float* pAnimValues = pAnimator->pAnimKeyTableEntry->pData + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey;
		pAnimValues[1] = this->field_0x188;
		pAnimValues[0] = 1.0f - pAnimValues[1];
	}
	else {
		CActor::AnimEvaluate(layerId, pAnimator, newAnim);
	}
}

int CActorElectrolla::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	if (msg == MESSAGE_KICKED) {
		if ((GetStateFlags(this->actorState) & 0x100) == 0) {
			this->field_0x1a8.nbEntries = 0;
			SetState(ELECTROLLA_STATE_CHARGE, -1);
		}
		return 1;
	}

	if ((msg == 1) && ((GetStateFlags(this->actorState) & 0x100) == 0)) {
		return 1;
	}

	if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
		_msg_params_get_position* pParams = reinterpret_cast<_msg_params_get_position*>(pMsgParam);
		if ((pParams->field_0x0 == 0) || (pParams->field_0x0 == 1)) {
			edF32Vector4SubHard(&pParams->vectorFieldB, &this->field_0x160, &this->currentLocation);
			return 1;
		}
	}

	return CActor::InterpretMessage(pSender, msg, pMsgParam);
}

void CActorElectrolla::BehaviourElectrolla_InitState(int newState)
{
	this->field_0x1a0 = 0.0f;
}

bool CActorElectrolla::InitDlistPatchable(int patchId)
{
	edDListLoadIdentity();
	edDListUseMaterial(CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->materialId, 0));
	edDListBegin(0.0f, 0.0f, 0.0f, DLIST_PRIM_TYPE_QUAD, 4);
	edDListColor4u8(0x80, 0x80, 0x80, 0);
	edDListTexCoo2f(0.0f, 0.0f);
	edDListVertex4f(-1.0f, -1.0f, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 0.0f);
	edDListVertex4f(1.0f, -1.0f, 0.0f, 0.0f);
	edDListTexCoo2f(0.0f, 1.0f);
	edDListVertex4f(-1.0f, 1.0f, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 1.0f);
	edDListVertex4f(1.0f, 1.0f, 0.0f, 0.0f);
	edDListEnd();

	return true;
}

void CActorElectrolla::Draw()
{
	CActor::Draw();
	if (this->field_0x190.a == 0) {
		if (GameDListPatch_BeginCurrent(this->dlistPatchId) != (CGlobalDListPatch*)0x0) {
			GameDListPatch_EndCurrent(0, 0);
		}
	}
	else {
		if (this->materialId == -1) {
			return;
		}

		CCameraManager* pCameraManager = CCameraManager::_gThis;
		edF32VECTOR4 direction;
		edF32MATRIX4 rotation;
		edF32MATRIX4 transform;
		edF32Vector4SubHard(&direction, &pCameraManager->transformationMatrix.rowT, &this->field_0x160);
		edF32Vector4NormalizeHard(&direction, &direction);
		float angle = GetAngleYFromVector(&direction);
		edF32Matrix4CopyHard(&transform, &pCameraManager->transMatrix_0x390);
		edF32Vector4ScaleHard(0.66f, &direction, &direction);
		edF32Vector4AddHard(&transform.rowT, &this->field_0x160, &direction);
		edF32Matrix4RotateZHard(angle, &rotation, &gF32Matrix4Unit);
		edF32Vector4ScaleHard(this->field_0x18c, &rotation.rowX, &rotation.rowX);
		edF32Vector4ScaleHard(this->field_0x18c, &rotation.rowY, &rotation.rowY);
		edF32Vector4ScaleHard(this->field_0x18c, &rotation.rowZ, &rotation.rowZ);
		edF32Matrix4MulF32Matrix4Hard(&transform, &rotation, &transform);

		CGlobalDListPatch* pPatch = GameDListPatch_BeginCurrent(this->dlistPatchId);
		if (pPatch != (CGlobalDListPatch*)0x0) {
			edF32Matrix4CopyHard(&pPatch->pCurrentPatch->pDisplayListCommand->matrix, &transform);
			_rgba color = this->field_0x190;
			color.a = static_cast<byte>(this->field_0x190.a * 0.5f + this->field_0x190.a * (static_cast<float>(rand()) / RAND_MAX) * 0.4f);

			for (int i = 0; i < 4; i++) {
				pPatch->pCurrentPatch->pRgba[i] = color;
			}

			GameDListPatch_EndCurrent(-1, 0);
		}
	}

	for (int i = 0; i < this->field_0x174; i++) {
		this->field_0x170[i].Draw(this->actorState == ELECTROLLA_STATE_DISCHARGE);
	}
}

void CActorElectrolla::BehaviourElectrolla_Manage()
{
	CActorsTable actors;
	edF32VECTOR4 target;
	_msg_hit_param hitParams;
	float distance;
	float scanTarget;
	int i;

	this->field_0x1c4.pStreamEventCamera->Manage(this);
	this->field_0x1a4 = 0;
	edF32VECTOR4 offset = { 0.0f, 0.15f, 0.0f, 0.0f };
	edF32MATRIX4* pBoneMatrix = this->pAnimationController->GetCurBoneMatrix(0x45477cb3);
	this->field_0x160 = pBoneMatrix->rowT;
	pBoneMatrix = this->pAnimationController->GetCurBoneMatrix(0x45477cb3);
	edF32Matrix4MulF32Vector4Hard(&offset, pBoneMatrix, &offset);
	edF32Vector4AddHard(&this->field_0x160, &this->field_0x160, &offset);
	edF32Matrix4MulF32Vector4Hard(&this->field_0x160, &this->pMeshTransform->base.transformA, &this->field_0x160);

	switch (this->actorState) {
	case ELECTROLLA_STATE_STAND:
		this->field_0x190 = _rgba(0x3060a080);
		this->field_0x188 = 0.0f;
		this->field_0x18c = 0.3f;
		GetActorsNearWithCriterion(this->field_0x180, this, &actors, CriterionElectrollaMoving);
		if (actors.nbEntries != 0) {
			SetState(ELECTROLLA_STATE_SCAN, -1);
		}
		break;
	case ELECTROLLA_STATE_SCAN:
		distance = GetActorsNearWithCriterion(this->field_0x180, this, &actors, CriterionElectrollaMoving);
		if (this->field_0x180 < distance) {
			scanTarget = this->field_0x184;
			if (scanDelay < this->field_0x1a0) {
				scanTarget = 0.0f;
			}
			this->field_0x1a0 = this->field_0x1a0 + GetTimer()->cutsceneDeltaTime;
		}
		else {
			this->field_0x1a0 = 0.0f;
			scanTarget = (this->field_0x180 - distance) / (this->field_0x180 - this->field_0x17c);
		}

		scanTarget = scanTarget - this->field_0x184;
		if (scanTarget < 0.0f) {
			this->field_0x184 = this->field_0x184 - scanDown * GetTimer()->cutsceneDeltaTime;
		}
		else {
			if (0.0f < scanTarget) {
				this->field_0x184 = this->field_0x184 + scanUp * GetTimer()->cutsceneDeltaTime;
			}
		}

		if (this->field_0x184 < 0.0f) {
			this->field_0x184 = 0.0f;
		}

		if (1.0f < this->field_0x184) {
			this->field_0x184 = 1.0f;
		}

		this->field_0x18c = this->field_0x184 * 0.6f * 0.5f + 0.3f;
		this->field_0x188 = this->field_0x184;
		this->field_0x190.LerpRGBA(this->field_0x184, 0x3060a080, 0x80408080);
		if (distance < this->field_0x17c) {
			this->field_0x184 = 0.0f;
			SetState(ELECTROLLA_STATE_ALERT, -1);
		}
		else {
			if ((this->field_0x180 < distance) && (this->field_0x184 == 0.0f)) {
				SetState(ELECTROLLA_STATE_STAND, -1);
			}
		}
		break;
	case ELECTROLLA_STATE_ALERT:
		distance = GetActorsNearWithCriterion(this->field_0x17c, this, &actors, CriterionElectrollaMovable);
		this->field_0x184 = this->field_0x184 + scanUp * GetTimer()->cutsceneDeltaTime;
		if (1.0f < this->field_0x184) {
			this->field_0x184 = 1.0f;
		}

		this->field_0x188 = 1.0f;
		this->field_0x18c = 0.6f;
		this->field_0x190 = _rgba(0x800404ff);
		if (distance < this->field_0x178) {
			this->field_0x1a8.nbEntries = 0;
			SetState(ELECTROLLA_STATE_CHARGE, -1);
		}
		else {
			if (actors.nbEntries == 0) {
				SetState(ELECTROLLA_STATE_SCAN, -1);
			}
		}
		break;
	case ELECTROLLA_STATE_CHARGE:
		this->field_0x188 = 1.0f;
		this->field_0x190 = _rgba(0x80f09614);
		this->field_0x18c = (static_cast<float>(rand()) / RAND_MAX) * 0.4f * 0.6f + 0.36f;
		if (this->field_0x194 <= this->field_0x1a0) {
			hitParams.projectileType = 5;
			hitParams.field_0x30 = 0.0f;
			hitParams.damage = 10.0f;

			GetActorsNearWithCriterion(this->field_0x180, this, &actors, Criterion_Near);

			while (actors.nbEntries != 0) {
				CActor* pActor = actors.PopCurrent();
				if (DoMessage(pActor, static_cast<ACTOR_MESSAGE>(1), &hitParams) != 0) {
					DoMessage(pActor, MESSAGE_KICKED, &hitParams);

					if (this->field_0x1a4 < this->field_0x174) {
						this->field_0x1a8.Add(pActor);
						if (pActor->typeID == 6) {
							pActor->SV_GetBoneWorldPosition(0x45544554, &target);
						}
						else {
							target = pActor->currentLocation;
							target.y = target.y + (static_cast<float>(rand()) / RAND_MAX) * 0.5f + 0.5f;
						}

						this->field_0x170[this->field_0x1a4].Reset();
						i = this->field_0x1a4;
						this->field_0x1a4 = i + 1;
						this->field_0x170[i].Manage(&this->field_0x160, &target);
					}
				}
			}

			while (this->field_0x1a4 < this->field_0x174) {
				target.x = (this->field_0x160.x + (static_cast<float>(rand()) / RAND_MAX) * 3.0f) - 1.5f;
				target.y = this->currentLocation.y;
				target.z = (this->field_0x160.z + (static_cast<float>(rand()) / RAND_MAX) * 3.0f) - 1.5f;
				target.w = 1.0f;
				this->field_0x170[this->field_0x1a4].Reset();
				i = this->field_0x1a4;
				this->field_0x1a4 = i + 1;
				this->field_0x170[i].Manage(&this->field_0x160, &target);
			}
			_gELE_Lights.Register(reinterpret_cast<CActInstance*>(this));
			SetState(ELECTROLLA_STATE_DISCHARGE, -1);
			this->field_0x1c4.SwitchOn(this);
		}

		this->field_0x1a0 = this->field_0x1a0 + GetTimer()->cutsceneDeltaTime;
		break;
	case ELECTROLLA_STATE_DISCHARGE:
		this->field_0x188 = 1.0f;
		this->field_0x190 = _rgba(0x80f09614);
		this->field_0x18c = (static_cast<float>(rand()) / RAND_MAX) * 0.4f * 0.6f + 0.36f;

		if (this->field_0x1a0 < this->field_0x198) {
			_gELE_Lights.Update(reinterpret_cast<CActInstance*>(this), &this->field_0x160);
			hitParams.projectileType = 0;
			hitParams.flags = 0;
			hitParams.damage = GetTimer()->cutsceneDeltaTime;
			hitParams.field_0x30 = 0.0f;
			GetActorsNearWithCriterion(this->field_0x180, this, &actors, Criterion_Near);
			while (actors.nbEntries != 0) {
				CActor* pActor = actors.PopCurrent();
				if (!this->field_0x1a8.IsInList(pActor)) {
					if ((this->field_0x1a8.nbEntries < this->field_0x174) && (DoMessage(pActor, static_cast<ACTOR_MESSAGE>(1), &hitParams) != 0)) {
						this->field_0x1a8.Add(pActor);
						this->field_0x1a4 = this->field_0x1a4 + 1;
					}
				}
				else {
					DoMessage(pActor, MESSAGE_KICKED, &hitParams);
					if (pActor->typeID == 6) {
						pActor->SV_GetBoneWorldPosition(0x45544554, &target);
					}
					else {
						target = pActor->currentLocation;
						target.y = target.y + (static_cast<float>(rand()) / RAND_MAX) * 0.5f + 0.5f;
					}

					i = this->field_0x1a4;
					if (i < this->field_0x174) {
						this->field_0x1a4 = i + 1;
						this->field_0x170[i].Manage(&this->field_0x160, &target);
					}
				}
			}

			for (i = 0; i < this->field_0x1a8.nbEntries; i++) {
				CActor* pActor = this->field_0x1a8.aEntries[i];
				edF32VECTOR4 delta;
				edF32Vector4SubHard(&delta, &pActor->currentLocation, &this->currentLocation);
				distance = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
				if (((pActor->GetStateFlags(pActor->actorState) & 1) != 0) || (this->field_0x180 * 2.0f * this->field_0x180 < distance)) {
					this->field_0x1a8.Remove(pActor);
				}
			}

			while (this->field_0x1a4 < this->field_0x174) {
				target.x = (this->field_0x160.x + (static_cast<float>(rand()) / RAND_MAX) * 3.0f) - 1.5f;
				target.y = this->currentLocation.y;
				target.z = (this->field_0x160.z + (static_cast<float>(rand()) / RAND_MAX) * 3.0f) - 1.5f;
				target.w = 1.0f;
				i = this->field_0x1a4;
				this->field_0x1a4 = i + 1;
				this->field_0x170[i].Manage(&this->field_0x160, &target);
			}
		}
		else {
			_gELE_Lights.Unregister(reinterpret_cast<CActInstance*>(this));
			SetState(ELECTROLLA_STATE_DISCHARGE_END, -1);
			this->field_0x1c4.pTargetStreamRef->SwitchOff(this);
			this->field_0x1a8.nbEntries = 0;
		}

		this->field_0x1a0 = this->field_0x1a0 + GetTimer()->cutsceneDeltaTime;
		SetLocalBoundingSphere(this->field_0x180 + 2.0f, &this->subObjA->boundingSphere);
		break;
	case ELECTROLLA_STATE_DISCHARGE_END:
		this->field_0x190 = _rgba(0);
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(ELECTROLLA_STATE_COOLDOWN, -1);
		}
		break;
	case ELECTROLLA_STATE_COOLDOWN:
		if (this->field_0x19c <= this->field_0x1a0) {
			SetLocalBoundingSphere(this->field_0x178, &this->subObjA->boundingSphere);
			this->field_0x1a0 = 0.0f;
			SetState(ELECTROLLA_STATE_STAND, -1);
		}
		this->field_0x1a0 = this->field_0x1a0 + GetTimer()->cutsceneDeltaTime;
		break;
	}

	return;
}

void CBehaviourElectrolla::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourElectrolla::Manage()
{
	this->pOwner->BehaviourElectrolla_Manage();

	return;
}

void CBehaviourElectrolla::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorElectrolla*>(pOwner);
	if (newState == -1) {
		this->pOwner->SetState(ELECTROLLA_STATE_STAND, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourElectrolla::InitState(int newState)
{
	this->pOwner->BehaviourElectrolla_InitState(newState);

	return;
}

float GetActorsNearWithCriterion(float radius, CActor* pActor, CActorsTable* pTable, ColCallbackFuncPtr* pFunc)
{
	CActorHero* pCVar1;
	bool bVar2;
	float* pfVar3;
	float* pfVar4;
	float* pfVar5;
	int iVar6;
	int iVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_30;
	_criterion_near_params local_20;

	pfVar3 = reinterpret_cast<float*>(gSP_Manager.GetFreeBuffer(0x100));
	pCVar1 = CActorHero::_gThis;
	local_20.field_0x10 = radius * radius;
	local_20.nearestDistance = 1e+20f;
	local_30.xyz = pActor->currentLocation.xyz;
	local_30.w = radius;
	local_20.pTable = pTable;
	local_20.aDistances = pfVar3;
	local_20.pActor = pActor;

	if ((pActor->actorFieldS & 8) == 0) {
		CScene::ptable.g_ActorManager_004516a4->cluster.ApplyCallbackToActorsIntersectingSphere(&local_30, pFunc, &local_20);
	}
	else {
		bVar2 = CActorHero::_gThis->SV_IsWorldBoundingSphereIntersectingSphere(&local_30);
		if (bVar2 != false) {
			pFunc(pCVar1, &local_20);
		}
	}

	iVar6 = 0;
	iVar7 = 0;
	fVar9 = sqrtf(local_20.nearestDistance);
	pfVar4 = pfVar3;
	pfVar5 = pfVar3;
	if (0 < pTable->nbEntries) {
		do {
			if (*pfVar4 < *pfVar5) {
				pTable->Swap(iVar7, iVar6);
				fVar8 = *pfVar4;
				iVar6 = iVar6 + 1;
				*pfVar4 = *pfVar5;
				*pfVar5 = fVar8;
				pfVar5 = pfVar5 + 1;
			}
			iVar7 = iVar7 + 1;
			pfVar4 = pfVar4 + 1;
		} while (iVar7 < pTable->nbEntries);
	}

	gSP_Manager.ReleaseBuffer(pfVar3);

	return fVar9;
}
