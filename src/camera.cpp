#include "camera.h"

#include "MathOps.h"
#include "MemoryStream.h"

PACK(
	struct S_STREAM_SIMPLE_ACT_COND {
	int field_0x0;
	int field_0x4;
	int field_0x8;
	float field_0xc;

	bool IsVerified(bool bDefault);
});

bool S_STREAM_SIMPLE_ACT_COND::IsVerified(bool bDefault)
{
	int iVar1;
	//ActorState AVar2;
	bool bVar3;
	bool bVar4;
	//AnimResult* pAVar4;
	uint uVar5;
	CActor* pAVar6;

	iVar1 = this->field_0x4;
	bVar4 = bDefault;
	if (iVar1 != -1) {
		pAVar6 = (CActor*)0x0;
		if (iVar1 != -1) {
			IMPLEMENTATION_GUARD(
				pAVar6 = (*(CScene::ptable.g_ActorManager_004516a4)->aActors)[iVar1];)
		}
		switch (this->field_0x8) {
		case 0:
			IMPLEMENTATION_GUARD(
				AVar2 = (pAVar6->data).actorState;
			if (AVar2 == AS_None) {
				bVar4 = false;
			}
			else {
				pAVar4 = (*pAVar6->pVTable->getAnimForState)(pAVar6, AVar2);
				bVar4 = (bool)((byte)pAVar4->flags_0x4 & 1);
			})
				break;
		case 1:
			IMPLEMENTATION_GUARD(
				AVar2 = (pAVar6->data).actorState;
			if (AVar2 == AS_None) {
				uVar5 = 0;
			}
			else {
				pAVar4 = (*pAVar6->pVTable->getAnimForState)(pAVar6, AVar2);
				uVar5 = pAVar4->flags_0x4 & 1;
			}
			bVar4 = uVar5 == 0;)
				break;
		case 2:
			IMPLEMENTATION_GUARD(
				bVar3 = (*pAVar6->pVTable->checkFuncA)(pAVar6, 2);
			if (bVar3 == false) {
				bVar4 = true;
			}
			else {
				bVar4 = this->field_0xc < pAVar6[1].data.anotherVector.y;
			})
				break;
		case 3:
			IMPLEMENTATION_GUARD(
				bVar3 = (*pAVar6->pVTable->checkFuncA)(pAVar6, 2);
			if (bVar3 == false) {
				bVar4 = true;
			}
			else {
				bVar4 = pAVar6[1].data.anotherVector.y < this->field_0xc;
			})
				break;
		case 4:
			IMPLEMENTATION_GUARD(
				bVar4 = ((pAVar6->data).flags & 0x2000001) == 0;)
				break;
		case 5:
			IMPLEMENTATION_GUARD(
				bVar4 = (bool)((byte)(pAVar6->data).flags & 1);)
				break;
		default:
			bVar4 = true;
		}
		iVar1 = this->field_0x0;
		if (iVar1 == 2) {
			bVar4 = (bool)(bVar4 ^ bDefault);
		}
		else {
			if (iVar1 == 1) {
				bVar4 = (bool)(bVar4 | bDefault);
			}
			else {
				if (iVar1 == 0) {
					bVar4 = (bool)(bVar4 & bDefault);
				}
			}
		}
	}
	return bVar4;
}

bool SpecificCondition::IsVerified()
{
	int* piVar1;
	bool bDefault;
	S_STREAM_SIMPLE_ACT_COND* pSimpleActCond;
	int iVar2;

	piVar1 = this->pData;
	if (piVar1 == (int*)0x0) {
		bDefault = true;
	}
	else {
		bDefault = true;
		pSimpleActCond = (S_STREAM_SIMPLE_ACT_COND*)(piVar1 + 1);
		for (iVar2 = *piVar1; 0 < iVar2; iVar2 = iVar2 + -1) {
			bDefault = pSimpleActCond->IsVerified(bDefault);
			pSimpleActCond = pSimpleActCond + 1;
		}
	}
	return bDefault;
}


CCamera::CCamera(ByteCode* pMemoryStream)
	: CObject()
{
	int* piVar2;
	int iVar3;
	uint uVar4;
	float fVar6;
	edF32VECTOR4 local_10;

	this->specCondition.pData = (int*)0x0;
	this->pNextCameraView_0xa4 = (CCamera*)0x0;
	this->field_0xa0 = 0;
	SetOtherTarget(0);
	fVar6 = pMemoryStream->GetF32();
	this->fov = fVar6;
	local_10.x = pMemoryStream->GetF32();
	local_10.y = pMemoryStream->GetF32();
	local_10.z = pMemoryStream->GetF32();
	local_10.w = 1.0;
	(this->transformationMatrix).da = local_10.x;
	(this->transformationMatrix).db = local_10.y;
	(this->transformationMatrix).dc = local_10.z;
	(this->transformationMatrix).dd = 1.0f;
	local_10.x = pMemoryStream->GetF32();
	local_10.y = pMemoryStream->GetF32();
	local_10.z = pMemoryStream->GetF32();
	local_10.w = 1.0;
	(this->lookAt).x = local_10.x;
	(this->lookAt).y = local_10.y;
	(this->lookAt).z = local_10.z;
	(this->lookAt).w = 1.0f;
	edF32Vector4SubHard(&local_10, &this->lookAt, (edF32VECTOR4*)&(this->transformationMatrix).da);
	edF32Vector4NormalizeHard(&local_10, &local_10);
	(this->transformationMatrix).ca = local_10.x;
	(this->transformationMatrix).cb = local_10.y;
	(this->transformationMatrix).cc = local_10.z;
	(this->transformationMatrix).cd = local_10.w;
	local_10.x = pMemoryStream->GetF32();
	local_10.y = pMemoryStream->GetF32();
	local_10.z = pMemoryStream->GetF32();
	local_10.w = 0.0;
	(this->transformationMatrix).ba = local_10.x;
	(this->transformationMatrix).bb = local_10.y;
	(this->transformationMatrix).bc = local_10.z;
	(this->transformationMatrix).bd = 0.0;
	edF32Vector4CrossProductHard(&local_10, (edF32VECTOR4*)&(this->transformationMatrix).ba, (edF32VECTOR4*)&(this->transformationMatrix).ca);
	local_10.w = 0.0;
	(this->transformationMatrix).aa = local_10.x;
	(this->transformationMatrix).ab = local_10.y;
	(this->transformationMatrix).ac = local_10.z;
	(this->transformationMatrix).ad = 0.0f;
	iVar3 = pMemoryStream->GetS32();
	this->objectId = iVar3;
	iVar3 = pMemoryStream->GetS32();
	this->field_0x8 = iVar3;
	uVar4 = pMemoryStream->GetU32();
	this->flags_0xc = uVar4 | 0x200000;
	fVar6 = pMemoryStream->GetF32();
	this->field_0x50.x = fVar6;
	fVar6 = pMemoryStream->GetF32();
	this->field_0x50.y = fVar6;
	fVar6 = pMemoryStream->GetF32();
	this->field_0x50.z = fVar6;
	this->field_0x50.w = 0.0f;
	if ((this->flags_0xc & 2) == 0) {
		this->field_0x8c = 0.0f;
		this->switchMode = SWITCH_MODE_F;
		this->field_0x98 = 0.8f;
		this->field_0x94 = SWITCH_MODE_A;
		this->field_0x9c = 0.8f;
	}
	else {
		this->field_0x80 = pMemoryStream->GetS32();
		int* pStream = (int*)pMemoryStream->currentSeekPos;

		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + sizeof(int);

		if (*pStream != 0) {
			pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + (*pStream * 4);
		}

		this->field_0x84 = (ActorAndWaypoint*)pStream;

		piVar2 = (int*)pMemoryStream->currentSeekPos;
		pMemoryStream->currentSeekPos = (char*)(piVar2 + 1);
		if (*piVar2 != 0) {
			pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar2 * 4;
		}
		this->field_0x88 = piVar2;

		fVar6 = pMemoryStream->GetF32();
		this->field_0x8c = fVar6;
		iVar3 = pMemoryStream->GetS32();
		this->switchMode = (SWITCH_MODE)iVar3;
		fVar6 = pMemoryStream->GetF32();
		this->field_0x98 = fVar6;
		iVar3 = pMemoryStream->GetS32();
		this->field_0x94 = (SWITCH_MODE)iVar3;
		fVar6 = pMemoryStream->GetF32();
		this->field_0x9c = fVar6;
		if (this->field_0x80 == -1) {
			this->flags_0xc = this->flags_0xc & 0xfffffffd;
		}
	}
	piVar2 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar2 * 0x10;
	}
	(this->specCondition).pData = (int*)piVar2;
	return;
}

CCamera::CCamera()
	:CObject()
{
	(this->specCondition).pData = (int*)0x0;
	this->pNextCameraView_0xa4 = (CCamera*)0x0;
	edF32Matrix4SetIdentityHard(&this->transformationMatrix);
	this->flags_0xc = 0x200000;
	this->field_0x50.x = 0.0f;
	this->field_0x50.y = 0.0f;
	this->field_0x50.z = 0.0f;
	this->field_0x50.w = 0.0f;
	this->field_0xa0 = 0;
	this->objectId = -1;
	this->field_0x8 = -1;
	this->field_0x8c = 0.0f;

	SetOtherTarget(0x0);
	this->switchMode = SWITCH_MODE_A;
	this->field_0x98 = 0.8f;
	this->field_0x94 = SWITCH_MODE_A;
	this->field_0x9c = 0.8f;
	return;
}

bool CCamera::IsKindOfObject(ulong kind)
{
	return (kind & 0x20) != 0;
}

void CCamera::Init()
{
	CEventManager* pEVar1;
	//EventChunk_24* pEVar2;
	//CheckpointManagerSubObjB* pCVar3;
	//Actor* pAVar4;
	edF32VECTOR3 local_10;

	pEVar1 = CScene::ptable.g_EventManager_006f5080;
	edF32Matrix4ToEulerSoft(&this->transformationMatrix, &local_10, "ZXY");
	SetAngleAlpha(local_10.x);
	SetAngleBeta(local_10.y);
	SetAngleGamma(local_10.z);
	if ((this->flags_0xc & 2) != 0) {
		IMPLEMENTATION_GUARD(
			this->field_0x7c = 0;
		pEVar2 = (EventChunk_24*)0x0;
		if (this->field_0x80 != (EventChunk_24*)0xffffffff) {
			pEVar2 = edEventGetChunkZone(pEVar1->activeEventChunkID_0x8, (int)this->field_0x80);
		}

		this->field_0x80 = pEVar2;
		pCVar3 = this->field_0x84;
		for (pAVar4 = pCVar3->pActor_0x0; pAVar4 != (Actor*)0x0; pAVar4 = (Actor*)((int)&pAVar4[-1].data.field_0x158 + 3))
		{
			pCVar3 = (CheckpointManagerSubObjB*)&pCVar3->field_0x4;
			LinkCheckpointActor_00119a00(pCVar3);
		}
		pCVar3 = (CheckpointManagerSubObjB*)this->field_0x88;
		for (pAVar4 = pCVar3->pActor_0x0; pAVar4 != (Actor*)0x0; pAVar4 = (Actor*)((int)&pAVar4[-1].data.field_0x158 + 3))
		{
			pCVar3 = (CheckpointManagerSubObjB*)&pCVar3->field_0x4;
			LinkCheckpointActor_00119a00(pCVar3);
		})
	}
	return;
}

bool CCamera::Manage()
{
	CActor* pActorA;
	bool returnVal;
	CActor* pAVar1;
	void* actorAddrA;

	if ((this->flags_0xc & 0x800000) == 0) {
		this->flags_0xc = this->flags_0xc | 0x800000;
		actorAddrA = GetTarget();
		if (actorAddrA != 0) {
			pActorA = GetTarget();
			pAVar1 = pActorA->pTiedActor;
			if (pAVar1 == (CActor*)0x0) {
				pAVar1 = GetTarget();
				pAVar1 = pAVar1->GetCollidingActor();
			}
			SetOtherTarget(pAVar1);
		}
		returnVal = false;
	}
	else {
		returnVal = true;
	}
	return returnVal;
}

bool CCamera::AlertCamera(int param_2, int param_3, CCamera* param_4)
{
	int* piVar1;
	int iVar2;
	uint* puVar3;
	int iVar4;
	int iVar5;
	float fVar6;

	if (param_2 == 1) {
		fVar6 = this->field_0x8c;
		if ((this->flags_0xc & 2) != 0) {
			iVar5 = 0;
			iVar4 = 0;
			IMPLEMENTATION_GUARD(
				while (true) {
					piVar1 = (int*)this->field_0x88;
					iVar2 = 0;
					if (piVar1 != (int*)0x0) {
						iVar2 = *piVar1;
					}
					if (iVar2 <= iVar5) break;
					CActor::DoMessage
					((Actor*)this->field_0x84->field_0x4, *(Actor**)((int)piVar1 + iVar4 + 4), 0x2a,
						(ActorCompareStruct*)(uint)(0.0 < fVar6));
					iVar4 = iVar4 + 4;
					iVar5 = iVar5 + 1;
				})
		}
	}
	else {
		if (param_2 == 2) {
			fVar6 = this->field_0x8c;
			if ((this->flags_0xc & 2) != 0) {
				IMPLEMENTATION_GUARD(
					iVar5 = 0;
				iVar4 = 0;
				while (true) {
					piVar1 = (int*)this->field_0x88;
					iVar2 = 0;
					if (piVar1 != (int*)0x0) {
						iVar2 = *piVar1;
					}
					if (iVar2 <= iVar5) break;
					CActor::DoMessage
					((Actor*)this->field_0x84->field_0x4, *(Actor**)((int)piVar1 + iVar4 + 4), 0x29,
						(ActorCompareStruct*)(uint)(0.0 < fVar6));
					iVar4 = iVar4 + 4;
					iVar5 = iVar5 + 1;
				})
			}
		}
		else {
			if (param_2 == 0) {
				puVar3 = &this->flags_0xc;
				if (param_3 == 0) {
					*puVar3 = *puVar3 & 0xffefffff;
				}
				else {
					*puVar3 = *puVar3 | 0x100000;
				}
			}
		}
	}

	return false;
}

void CCamera::SetOtherTarget(CActor* pNewTarget)
{
	this->pOtherTarget = pNewTarget;
	return;
}

float CCamera::GetDistance()
{
	float fVar1;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, &this->lookAt, &(this->transformationMatrix).rowT);
	fVar1 = edF32Vector4GetDistHard(&eStack16);
	return fVar1;
}

float CCamera::GetAngleAlpha()
{
	return GetAngleXFromVector(&(this->transformationMatrix).rowZ);
}

float CCamera::GetAngleBeta()
{
	return GetAngleYFromVector(&(this->transformationMatrix).rowZ);
}

float CCamera::GetAngleGamma()
{
	return 0.0f;
}

ECameraType CCamera::GetMode()
{
	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found CT_Frontend: 0x{:x}", (int)CT_Frontend);
	return CT_Frontend;
}


void CCameraExt::Init()
{
	CEventManager* pEVar1;
	//EventChunk_24* pEVar2;
	//CheckpointManagerSubObjB* pCVar3;
	//Actor* pAVar4;
	edF32VECTOR3 local_10;

	pEVar1 = CScene::ptable.g_EventManager_006f5080;

	edF32Matrix4ToEulerSoft(&this->transformationMatrix, &local_10, "ZXY");

	SetAngleAlpha(local_10.x);
	SetAngleBeta(local_10.y);
	SetAngleGamma(local_10.z);

	if ((this->flags_0xc & 2) != 0) {
		IMPLEMENTATION_GUARD(
			this->field_0x7c = 0;
		pEVar2 = (EventChunk_24*)0x0;
		if (this->field_0x80 != (EventChunk_24*)0xffffffff) {
			pEVar2 = edEventGetChunkZone(pEVar1->activeEventChunkID_0x8, (int)this->field_0x80);
		}
		this->field_0x80 = pEVar2;
		pCVar3 = this->field_0x84;
		for (pAVar4 = pCVar3->pActor_0x0; pAVar4 != (Actor*)0x0; pAVar4 = (Actor*)((int)&pAVar4[-1].data.field_0x158 + 3))
		{
			pCVar3 = (CheckpointManagerSubObjB*)&pCVar3->field_0x4;
			LinkCheckpointActor_00119a00(pCVar3);
		}
		pCVar3 = (CheckpointManagerSubObjB*)this->field_0x88;
		for (pAVar4 = pCVar3->pActor_0x0; pAVar4 != (Actor*)0x0; pAVar4 = (Actor*)((int)&pAVar4[-1].data.field_0x158 + 3))
		{
			pCVar3 = (CheckpointManagerSubObjB*)&pCVar3->field_0x4;
			LinkCheckpointActor_00119a00(pCVar3);
		})
	}
	return;
}

CActor* CCameraExt::GetTarget()
{
	return this->pActorView;
}

void CCameraExt::SetTarget(CActor* pActor)
{
	this->pActorView = pActor;
}

float CCameraExt::GetDistance()
{
	return this->distance;
}

void CCameraExt::SetDistance(float distance)
{
	this->distance = distance;
	return;
}

float CCameraExt::GetAngleAlpha()
{
	return this->angles.alpha;
}

float CCameraExt::GetAngleBeta()
{
	return this->angles.beta;
}

float CCameraExt::GetAngleGamma()
{
	return this->angles.gamma;
}

void CCameraExt::SetAngleAlpha(float angle)
{
	(this->angles).alpha = angle;
	return;
}

void CCameraExt::SetAngleBeta(float angle)
{
	(this->angles).beta = angle;
	return;
}

void CCameraExt::SetAngleGamma(float angle)
{
	(this->angles).gamma = angle;
	return;
}

edF32VECTOR3* CCameraExt::GetAngles()
{
	return &this->angles;
}

CCameraExt::CCameraExt(ByteCode* pMemoryStream)
	: CCamera(pMemoryStream)
{
	this->pActorView = (CActor*)0x0;
	this->distance = 0.0f;
	this->angles = gF32Vector3Zero;
	return;
}

CCameraExt::CCameraExt()
	: CCamera()
{
	this->pActorView = (CActor*)0x0;
	this->distance = 0.0f;
	this->angles = gF32Vector3Zero;
	return;
}


void CCameraExt::ComputeTargetOffset(edF32VECTOR4* v0)
{
	undefined8 uVar1;
	char cVar2;
	CActor* pReceiver;
	CActor* this_00;
	CActor* pOutActor;
	edF32VECTOR4 local_50;
	ActorMessage_7 auStack64;

	cVar2 = false;
	if ((this->flags_0xc & 1) != 0) {
		auStack64.field_0x0 = 5;
		auStack64.field_0x20 = gF32Vector4Zero;
		pReceiver = GetTarget();
		this_00 = GetTarget();
		cVar2 = this_00->DoMessage(pReceiver, (ACTOR_MESSAGE)7, (MSG_PARAM)&auStack64);
		if (cVar2 != false) {
			*v0 = auStack64.field_0x20;
		}
	}

	if (cVar2 == false) {
		local_50 = this->field_0x50;

		if ((this->flags_0xc & 0x80) == 0) {
			*v0 = local_50;
		}
		else {
			pOutActor = GetTarget();
			edF32Matrix4MulF32Vector4Hard(v0, &pOutActor->pMeshTransform->base.transformA, &local_50);
		}
	}
	return;
}

void CCameraExt::ComputeTargetPosition(edF32VECTOR4* param_2)
{
	bool bVar1;
	CActor* this_00;
	CActor* pReceiver;
	CActor* pOutActor;
	edF32VECTOR4 local_20;

	this_00 = GetTarget();
	pReceiver = GetTarget();
	bVar1 = this_00->DoMessage(pReceiver, (ACTOR_MESSAGE)0x49, (MSG_PARAM)&local_20);
	if (bVar1 == false) {
		pOutActor = GetTarget();
		*param_2 = pOutActor->currentLocation;
	}
	else {
		*param_2 = local_20;
	}
	return;
}

bool CCameraExt::IsKindOfObject(ulong kind)
{
	return (kind & 0x60) != 0;
}
