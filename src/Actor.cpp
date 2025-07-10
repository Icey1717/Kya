#include "Actor.h"
#include "LargeObject.h"
#include "SectorManager.h"
#include "CinematicManager.h"
#include "ActorFactory.h"
#include "MathOps.h"
#include "CameraViewManager.h"
#include <string.h>
#include <math.h>
#include "TimeController.h"
#include "AnmManager.h"
#include "port/pointer_conv.h"
#include "MemoryStream.h"
#include "PoolAllocators.h"
#include "Actor_Cinematic.h"
#include "LightManager.h"
#include "CollisionManager.h"
#include "ActorManager.h"
#include "InputManager.h"
#include "PathManager.h"
#include "CollisionRay.h"
#include "ActorAutonomous.h"
#include "Vision.h"

#ifdef PLATFORM_WIN
#include "displaylist.h"
#endif
#include "ActorMoney.h"
#include "ActorHero.h"

CPathFollowReader::CPathFollowReader()
{
	this->splinePointIndex = 0;
	this->field_0xc = 1;
}

void CPathFollowReader::Create(ByteCode* pByteCode)
{
	int iVar1;

	iVar1 = pByteCode->GetS32();
	this->index = iVar1;
	return;
}

void CPathFollowReader::Init()
{
	CPathFollow* pCVar1;

	if (this->index == -1) {
		pCVar1 = (CPathFollow*)0x0;
	}
	else {
		pCVar1 = (CScene::ptable.g_PathManager_004516a0)->aPathFollow + this->index;
	}

	this->pPathFollow = pCVar1;

	return;
}

void CPathFollowReader::Reset()
{
	this->splinePointIndex = 0;
	this->field_0xc = 1;
	return;
}

void CPathFollowReader::NextWayPoint()
{
	bool bVar1;
	CPathFollow* pFollow;
	uint uVar3;
	bool bVar4;
	int iVar5;
	ulong uVar6;
	int iVar7;
	ulong* puVar8;

	pFollow = this->pPathFollow;

	iVar5 = this->field_0xc;
	iVar7 = this->splinePointIndex;

	uVar3 = pFollow->type;
	if (uVar3 != 0) {
		bVar4 = false;
		goto LAB_001c2930;
	}

	bVar4 = true;
	if (pFollow->mode == 1) {
		bVar1 = iVar7 == 0;
		iVar7 = iVar5;
		if (bVar1) {
		joined_r0x001c2918:
			if (iVar7 == 0) goto LAB_001c2930;
		}
	}
	else {
		if (pFollow->mode == 0) {
			if (iVar5 == 0) goto joined_r0x001c2918;
			if (iVar7 + 1 == pFollow->splinePointCount) goto LAB_001c2930;
		}
	}
	bVar4 = false;
LAB_001c2930:
	if (!bVar4) {
		iVar7 = pFollow->splinePointCount;
		if (iVar7 == 1) {
			this->splinePointIndex = 0;
		}
		else {
			if ((uVar3 == 2) || (uVar3 == 1)) {
				if (iVar5 == 0) {
					if (((pFollow->mode == 0) && (this->splinePointIndex == pFollow->field_0x14)) &&
						(pFollow->field_0x14 < this->field_0x8)) {
						this->splinePointIndex = iVar7;
					}
				}
				else {
					if ((pFollow->mode == 0) && (this->splinePointIndex + 1 == iVar7)) {
						this->splinePointIndex = pFollow->field_0x14 + -1;
					}
				}
			}

			uVar3 = this->pPathFollow->mode;

			if (uVar3 == 2) {
				IMPLEMENTATION_GUARD(
				puVar8 = &CScene::_pinstance->field_0x38;
				do {
					uVar6 = *puVar8 * 0x343fd + 0x269ec3;
					*puVar8 = uVar6;
					iVar7 = this->pPathFollow->splinePointCount + -1;
					iVar5 = (int)((uint)(uVar6 >> 0x10) & 0x7fff) % iVar7;
					if (iVar7 == 0) {
						trap(7);
					}
				} while (iVar5 == this->splinePointIndex);
				this->splinePointIndex = iVar5;)
			}
			else {
				if (uVar3 == 1) {
					if (this->field_0xc == 0) {
						iVar5 = this->splinePointIndex + -1;
						this->splinePointIndex = iVar5;
						if (iVar5 < 0) {
							this->splinePointIndex = this->splinePointIndex + 2;
							this->field_0xc = 1;
						}
					}
					else {
						iVar5 = this->pPathFollow->splinePointCount;
						iVar7 = this->splinePointIndex + 1;
						this->splinePointIndex = iVar7;
						if (iVar5 <= iVar7) {
							this->splinePointIndex = this->splinePointIndex + -2;
							this->field_0xc = 0;
						}
					}
				}
				else {
					if (uVar3 == 0) {
						if (this->field_0xc == 0) {
							this->splinePointIndex = this->splinePointIndex + -1;
						}
						else {
							this->splinePointIndex = this->splinePointIndex + 1;
						}
					}
				}
			}

			iVar5 = GetPrevPlace(this->splinePointIndex, this->field_0xc);
			this->field_0x8 = iVar5;
		}
	}
	return;
}

bool CPathFollowReader::AtGoal(int param_2, int param_3)
{
	CPathFollow* pCVar1;

	pCVar1 = this->pPathFollow;

	if (pCVar1->type == 0) {
		if (pCVar1->mode == 1) {
			if ((param_2 == 0) && (param_3 == 0)) {
				return true;
			}
		}
		else {
			if (pCVar1->mode == 0) {
				if (param_3 == 0) {
					if (param_2 == 0) {
						return true;
					}
				}
				else {
					if (param_2 + 1 == pCVar1->splinePointCount) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

int CPathFollowReader::GetPrevPlace(int param_2, int param_3)
{
	CPathFollow* pCVar1;
	uint uVar2;
	int iVar3;
	int iVar4;
	float fVar5;

	if (param_3 == 0) {
		iVar4 = 1;
	}
	else {
		iVar4 = -1;
	}

	pCVar1 = this->pPathFollow;
	uVar2 = pCVar1->mode;

	if (((uVar2 == 0) && (pCVar1->type == 0)) &&
		(((param_3 == 0 && (pCVar1->splinePointCount + -1 <= param_2)) || ((param_3 != 0 && (param_2 < 1)))))) {
		iVar4 = -1;
	}
	else {
		if (((param_2 == pCVar1->field_0x14) && ((param_3 != 0 && (this->splinePointIndex < this->field_0x8)))) ||
			((param_2 == pCVar1->splinePointCount + -1 && (param_3 == 0)))) {
			if (uVar2 == 1) {
				if ((param_3 != 0) && (pCVar1->type != 1)) {
					return -1;
				}
				iVar4 = -iVar4;
			}

			if ((uVar2 == 0) && (pCVar1->type == 1)) {
				iVar3 = pCVar1->field_0x14;
				fVar5 = fmodf((float)((param_2 + pCVar1->splinePointCount + -1) - iVar3), (float)(((pCVar1->splinePointCount + -1) - iVar3) * 2));
				param_2 = ((int)fVar5 - iVar4) + iVar3;
			}
		}

		iVar4 = param_2 + iVar4;
	}
	return iVar4;
}

float CPathFollowReader::GetDelay()
{
	float delay;
	float* aDelays;

	aDelays = this->pPathFollow->aDelays;
	if (aDelays == (float*)0x0) {
		delay = 0.0f;
	}
	else {
		delay = aDelays[this->splinePointIndex];
	}

	return delay;
}

edF32VECTOR4* CPathFollowReader::GetWayPoint(int index)
{
	edF32VECTOR4* pWaypoint;

	pWaypoint = this->pPathFollow->aSplinePoints;

	if (pWaypoint == (edF32VECTOR4*)0x0) {
		pWaypoint = &gF32Vertex4Zero;
	}
	else {
		pWaypoint = pWaypoint + index;
	}

	return pWaypoint;
}

edF32VECTOR4* CPathFollowReader::GetWayPoint()
{
	edF32VECTOR4* pWaypoint;

	pWaypoint = this->pPathFollow->aSplinePoints;

	if (pWaypoint == (edF32VECTOR4*)0x0) {
		pWaypoint = &gF32Vertex4Zero;
	}
	else {
		pWaypoint = pWaypoint + this->splinePointIndex;
	}

	return pWaypoint;
}

edF32VECTOR4* CPathFollowReader::GetWayPointAngles()
{
	edF32VECTOR4* pWayPointAngles;

	pWayPointAngles = this->pPathFollow->aSplineRotationsEuler;

	if (pWayPointAngles == (edF32VECTOR4*)0x0) {
		pWayPointAngles = &gF32Vector4Zero;
	}
	else {
		pWayPointAngles = pWayPointAngles + this->splinePointIndex;
	}

	return pWayPointAngles;
}

CActor::CActor()
	: CObject()
{
	float fVar1;
	float fVar2;
	this->objectId = -1;
	//this->field_0x138 = -1.0f;
	//this->field_0x140 = 0;
	//this->field_0x144 = 0;
	//this->aActorSounds = (CActorSound*)0x0;
	//this->field_0x14c = 0;
	this->actorManagerIndex = -1;
	this->objectId = -1;
	this->pCinData = (CinNamedObject30*)0x0;
	this->subObjA = (KyaUpdateObjA*)0x0;
	this->flags = 0;
	this->currentLocation.x = 0.0f;
	this->currentLocation.y = 0.0f;
	this->currentLocation.z = 0.0f;
	this->currentLocation.w = 1.0f;
	this->scale.x = 1.0;
	this->scale.y = 1.0;
	this->scale.z = 1.0;
	this->scale.w = 1.0;
	this->rotationEuler.x = 0.0f;
	this->rotationEuler.y = 0.0f;
	this->rotationEuler.z = 0.0f;
	this->rotationEuler.w = 0.0f;
	this->rotationQuat.x = 0.0f;
	this->rotationQuat.y = 0.0f;
	this->rotationQuat.z = 0.0f;
	this->rotationQuat.w = 0.0f;
	this->pMeshNode = (edNODE*)0x0;
	this->p3DHierNode = (ed_3d_hierarchy_node*)0x0;
	this->pHier = (ed_g3d_hierarchy*)0x0;
	this->pMeshTransform = (ed_3d_hierarchy_node*)0x0;
	memset(&this->hierarchySetup, 0, sizeof(ed_3d_hierarchy_setup));

	this->lodBiases[0] = 1e+10f;
	this->lodBiases[1] = 1e+10f;
	this->lodBiases[2] = 1e+10f;
	this->lodBiases[3] = 1e+10f;

	this->sphereCentre.x = 0.0f;
	this->sphereCentre.y = 0.0f;
	this->sphereCentre.z = 0.0f;
	this->sphereCentre.w = 0.0f;
	this->pClusterNode = (CClusterNode*)0x0;
	this->distanceToCamera = 0.0;
	this->pAnimationController = (CAnimation*)0x0;
	this->pCollisionData = (CCollision*)0x0;
	this->pShadow = (CShadow*)0x0;
	this->pTiedActor = (CActor*)0x0;
	this->aComponents = (int*)0x0;
	this->curBehaviourId = -1;
	this->prevBehaviourId = -1;
	this->actorState = AS_None;
	this->prevActorState = AS_None;
	this->timeInAir = 0.0f;
	this->idleTimer = 0.0f;
	this->field_0x110 = (undefined*)0x0;
	this->currentAnimType = -1;
	this->prevAnimType = -1;
	this->lightingFlags = 0;
	this->lightingFloat_0xe0 = 1.0f;
	this->field_0x11 = 0;
	this->state_0x10 = 0;
	this->distanceToGround = -1.0f;
	this->field_0xf0 = 3.0f;
	this->field_0xf4 = 0xffff;
	this->previousLocation = gF32Vector3Zero;
	this->vector_0x12c = gF32Vector3Zero;

	//this->field_0x13c = 0;
	//this->field_0x138 = 0.0;
	this->pMBNK = (void*)0x0;

	return;
}

void CActor::PreInit()
{
	int iVar3;
	int uVar4;
	int uVar5;
	int componentCount;
	int outIntB;
	int outIntA;
	BehaviourEntry* pCVar2;
	CBehaviour* pComponent;
	CShadow* pGVar1;
	
	if (this->pCollisionData != (CCollision*)0x0) {
		pCollisionData->Init();
	}
	pGVar1 = this->pShadow;
	if (pGVar1 != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		CShadow::Init(pGVar1, this->field_0x0);)
	}
	
	this->flags = this->flags & 0xfffffffc;
	this->flags = this->flags & 0xffffff5f;
	
	EvaluateDisplayState();
	
	this->flags = this->flags & 0xfffff7ff;
	this->flags = this->flags & 0xffbfffff;

	if ((((this->actorFieldS & 1) != 0) && (this != (CActor*)0x0)) && ((this->flags & 0x2000000) == 0)) {
		ReceiveMessage(this, (ACTOR_MESSAGE)0x5d, 0);
	}

	BehaviourList<1>* pComponentList = (BehaviourList<1>*)this->aComponents;
	uVar5 = 0;
	uVar4 = 0;
	BehaviourEntry* pEntry = pComponentList->aComponents;

	for (componentCount = pComponentList->count; componentCount != 0; componentCount = componentCount + -1) {
		pComponent = pEntry->GetBehaviour();


		if (pComponent != (CBehaviour*)0x0) {
			pComponent->Init(this);
			pComponent->GetDlistPatchableNbVertexAndSprites(&outIntA, &outIntB);
		}

		if (uVar5 < outIntA) {
			uVar5 = outIntA;
		}

		if (uVar4 < outIntB) {
			uVar4 = outIntB;
		}

		pEntry = pEntry + 1;
	}

	this->dlistPatchId = -1;
	
	if (uVar5 + uVar4 != 0) {
		IMPLEMENTATION_GUARD_LOG(
		iVar3 = GameDListPatch_Register(this, uVar5, uVar4);)
		this->dlistPatchId = iVar3;
	}
	
	return;
}

void CActor::EvaluateManageState()
{
	int iVar1;
	ulong uVar2;

	iVar1 = this->objectId;
	if ((iVar1 == ((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID) || (iVar1 == -1)) {
		uVar2 = this->flags;
		if ((uVar2 & 0x2000001) == 0) {
			if ((uVar2 & 8) == 0) {
				uVar2 = uVar2 & 2 | (ulong)(this->distanceToCamera <= (this->subObjA)->visibilityDistance);
			}
			else {
				uVar2 = uVar2 & 2 | this->state_0x10;
			}
			goto LAB_001034b0;
		}
	}
	uVar2 = 0;
LAB_001034b0:
	if ((this->flags & 4) == 0) {
		if (uVar2 != 0) {
			ChangeManageState(1);
		}
	}
	else {
		if (uVar2 == 0) {
			ChangeManageState(0);
		}
	}
	return;
}

void CActor::EvaluateDisplayState()
{
	int iVar1;
	uint uVar2;

	iVar1 = this->objectId;
	if ((iVar1 == ((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID) || (iVar1 == -1)) {
		uVar2 = this->flags;
		if ((uVar2 & 0x2000060) == 0) {
			if ((uVar2 & 0x200) == 0) {
				uVar2 = 1;
			}
			else {
				uVar2 = uVar2 & 0x80 | (uint)(0 < this->state_0x10);
			}
			goto LAB_001033c0;
		}
	}
	uVar2 = 0;
LAB_001033c0:
	if ((this->flags & 0x100) == 0) {
		if (uVar2 != 0) {
			this->ChangeDisplayState(1);
		}
	}
	else {
		if (uVar2 == 0) {
			this->ChangeDisplayState(0);
		}
	}
	return;
}

void CActor::SetScale(float x, float y, float z)
{
	this->scale.x = x;
	this->scale.y = y;
	this->scale.z = z;
	this->scale.w = 1.0f;

	if (((x == 1.0f) && (y == 1.0f)) && (z == 1.0f)) {
		this->flags = this->flags & 0xfbffffff;
	}
	else {
		this->flags = this->flags | 0x4000000;
	}
	return;
}

void CActor::SnapOrientation(float x, float y, float z)
{
	edF32VECTOR3* peVar1;
	float* pfVar2;
	float fVar3;
	float fVar4;

	if ((x != 0.0f) && (peVar1 = &this->pCinData->rotationEuler, peVar1 != (edF32VECTOR3*)0x0)) {
		fVar4 = peVar1->x;
		if (fVar4 < 0.0f) {
			fVar4 = -fVar4;
		}
		fVar4 = fVar4 + x * 0.5f;
		fVar3 = fmodf(fVar4, x);
		if (peVar1->x < 0.0f) {
			peVar1->x = -(fVar4 - fVar3);
		}
		else {
			peVar1->x = fVar4 - fVar3;
		}
	}

	if ((y != 0.0f) && (pfVar2 = &(this->pCinData->rotationEuler).y, pfVar2 != (float*)0x0)) {
		fVar4 = *pfVar2;
		if (fVar4 < 0.0f) {
			fVar4 = -fVar4;
		}
		fVar4 = fVar4 + y * 0.5f;
		fVar3 = fmodf(fVar4, y);
		if (*pfVar2 < 0.0f) {
			*pfVar2 = -(fVar4 - fVar3);
		}
		else {
			*pfVar2 = fVar4 - fVar3;
		}
	}

	if ((z != 0.0f) && (pfVar2 = &(this->pCinData->rotationEuler).z, pfVar2 != (float*)0x0)) {
		fVar4 = *pfVar2;
		if (fVar4 < 0.0f) {
			fVar4 = -fVar4;
		}
		fVar4 = fVar4 + z * 0.5f;
		fVar3 = fmodf(fVar4, z);
		if (*pfVar2 < 0.0f) {
			*pfVar2 = -(fVar4 - fVar3);
		}
		else {
			*pfVar2 = fVar4 - fVar3;
		}
	}

	RestoreInitData();
	return;
}

bool CActor::IsKindOfObject(ulong kind)
{
	return (kind & 1) != 0;
}

void CActor::FUN_00115ea0(uint param_2)
{
	edNODE* pNode;

	if ((this->pHier != (ed_g3d_hierarchy*)0x0) && (pNode = this->pMeshNode, pNode != (edNODE*)0x0)) {
		ed3DSetMeshTransformFlag_002abd80(pNode, 0xffff);
		if ((CActorFactory::gClassProperties[this->typeID].flags & 0x1000) != 0) {
			ed3DSetMeshTransformFlag_002abff0(this->pMeshNode, (ushort)param_2 & 0xff);
		}
		if ((CActorFactory::gClassProperties[this->typeID].flags & 0x2000) != 0) {
			ed3DSetMeshTransformFlag_002abff0(this->pMeshNode, (ushort)param_2);
		}
	}
	return;
}

bool CActor::Can_0x9c()
{
	bool bVar1;
	uint stateFlags;
	StateConfig* pAVar3;

	stateFlags = GetStateFlags(this->actorState);

	bVar1 = (stateFlags & 1) == 0;

	if (bVar1) {
		bVar1 = (this->flags & 4) != 0;
	}

	if (bVar1) {
		bVar1 = (this->actorFieldS & 0x60) != 0;
	}

	if (bVar1) {
		if (this->actorState == -1) {
			stateFlags = 0;
		}
		else {
			pAVar3 = GetStateCfg(this->actorState);
			stateFlags = pAVar3->flags_0x4 & 0x40;
		}
		bVar1 = stateFlags != 0;
	}

	return bVar1;
}

void CActor::Create(ByteCode* pByteCode)
{
	CinNamedObject30* pCVar1;
	KyaUpdateObjA* pKVar2;
	CCollision* pCVar3;
	char* newPos;
	char* pcVar4;
	char* inString;
	char* newPos_00;
	ulong lVar5;
	ulong uVar6;
	int iVar7;
	int iVar8;
	int* piVar9;
	MeshTextureHash* pMVar9;
	float fVar11;
	float fVar12;
	float fVar13;
	MeshTextureHash local_110[16];

	char* name = pByteCode->GetString();

	ACTOR_LOG(LogLevel::Info, "CActor::Create {}", name);

	memcpy(this->name, name, 64);

	if (strcmp(name, "TELEPORTER_LVL_DOOR_L1") == 0) {
		memcpy(this->name, name, 64);
	}

	pByteCode->Align(4);
	pCVar1 = (CinNamedObject30*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pCVar1 + 1);
	this->pCinData = pCVar1;
	piVar9 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar9 + 1);
	if (*piVar9 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar9 * 8;
	}
	this->field_0x110 = (undefined*)piVar9;
	if (*(int*)this->field_0x110 < 1) {
		this->field_0x110 = (undefined*)0x0;
	}
	newPos = pByteCode->GetPosition();
	pByteCode->Align(4);
	piVar9 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar9 + 1);
	if (*piVar9 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar9 * 8;
	}
	iVar7 = *piVar9;
	piVar9 = piVar9 + 1;
	if (0 < iVar7) {
		for (; 0 < iVar7; iVar7 = iVar7 + -1) {
			pcVar4 = pByteCode->GetPosition();
			pByteCode->SetPosition(pcVar4 + piVar9[1]);
			piVar9 = piVar9 + 2;
		}
	}
	iVar7 = pByteCode->GetS32();
	iVar8 = 0;
	if (0 < iVar7) {
		pMVar9 = local_110;
		do {
			pcVar4 = pByteCode->GetString();
			inString = pByteCode->GetString();
			lVar5 = ed3DComputeHashCode(pcVar4);
			pMVar9->meshHash = lVar5;
			lVar5 = ed3DComputeHashCode(inString);
			pMVar9->textureHash = lVar5;
			iVar8 = iVar8 + 1;
			pMVar9 = pMVar9 + 1;
		} while (iVar8 < iVar7);
	}
	pKVar2 = (KyaUpdateObjA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pKVar2 + 1);
	this->subObjA = pKVar2;
	pKVar2 = this->subObjA;
	if (pKVar2->defaultBehaviourId == 1) {
		pKVar2->defaultBehaviourId = -1;
	}
	this->actorFieldS = (this->subObjA)->actorFieldS;
	this->objectId = (this->subObjA)->field_0x24;
	if (this->objectId == 0) {
		this->objectId = -1;
	}
	SetupModel(iVar7, local_110);
	SetupDefaultPosition();
	this->hierarchySetup.clipping_0x0 = &(this->subObjA)->floatFieldB;
	this->hierarchySetup.pBoundingSphere = &(this->subObjA)->boundingSphere;

	this->lodBiases[0] = (this->subObjA)->lodBiases[0];
	this->lodBiases[1] = (this->subObjA)->lodBiases[1];
	this->lodBiases[2] = 10000.0;
	this->lodBiases[3] = 1e+10;
	float lodBias = this->lodBiases[0];
	this->lodBiases[0] = lodBias * lodBias;
	lodBias = this->lodBiases[1];
	this->lodBiases[1] = lodBias * lodBias;
	lodBias = this->lodBiases[2];
	this->lodBiases[2] = lodBias * lodBias;
	lodBias = this->lodBiases[3];
	this->lodBiases[3] = lodBias * lodBias;

	this->lightingFlags = 1;
	if (((this->subObjA)->flags_0x48 & 1) != 0) {
		this->lightingFlags = this->lightingFlags | 2;
	}
	if (((this->subObjA)->flags_0x48 & 2) == 0) {
		this->lightingFlags = this->lightingFlags | 4;
	}
	this->lightingFloat_0xe0 = (this->subObjA)->lightingFloat_0x4c;
	if ((CActorFactory::gClassProperties[this->typeID].flags & 0x20) != 0) {
		this->actorFieldS = this->actorFieldS | 0x10;
	}
	if (CActorFactory::gClassProperties[this->typeID].field_0x14 == 0) {
		this->actorFieldS = this->actorFieldS & 0xffffffef;
	}
	if ((this->actorFieldS & 0x80) != 0) {
		FUN_00115ea0(0);
	}
	ScenaricCondition local_4;
	local_4.Create(pByteCode);
	uVar6 = local_4.IsVerified();
	if (uVar6 == 0) {
		this->flags = this->flags | 0x2000000;
	}
	newPos_00 = pByteCode->GetPosition();
	pByteCode->SetPosition(newPos);
	LoadBehaviours(pByteCode);
	pByteCode->SetPosition(newPos_00);
	if (((this->actorFieldS & 4) != 0) && (pCVar3 = this->pCollisionData, pCVar3 != (CCollision*)0x0)) {
		pCVar3->flags_0x0 = pCVar3->flags_0x0 | 0x20000;
	}
	pCVar1 = this->pCinData;
	this->scale.xyz = pCVar1->scale;
	this->scale.w = 1.0f;
	if (((this->scale.x == 1.0f) && (this->scale.y == 1.0f)) && (this->scale.z == 1.0f)) {
		this->flags = this->flags & 0xfbffffff;
	}
	else {
		this->flags = this->flags | 0x4000000;
	}
	pCVar1 = this->pCinData;
	this->rotationEuler.xyz = pCVar1->rotationEuler;
	this->field_0x58 = 0;
	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
	UpdatePosition(&this->baseLocation, true);
	this->flags = this->flags | 0x80000;
	this->distanceToGround = -1.0f;
	this->flags = this->flags & 0xffdfffff;
	this->field_0xf4 = 0xffff;
	this->field_0xf0 = 3.0f;
	return;
}

void CActor::Init()
{
	uint* puVar1;

	///* While this pointer is invalid */
	//for (puVar1 = (uint*)this->field_0x144; puVar1 != (uint*)0x0; puVar1 = (uint*)puVar1[0xe]) {
	//	CActorSound::Init(puVar1);
	//}

	this->SetBehaviour((this->subObjA)->defaultBehaviourId, -1, -1);
	return;
}

void CActor::Manage()
{
	uint uVar1;
	uint* puVar2;
	bool bVar3;
	CBehaviour* pBehaviour;
	Timer* pTimeController;
	ulong uVar4;
	float fVar5;

	ACTOR_LOG(LogLevel::Info, "CActor::Manage {}", this->name);

	pBehaviour = GetBehaviour(this->curBehaviourId);
	if (pBehaviour != (CBehaviour*)0x0) {
		if ((GameFlags & GAME_STATE_PAUSED | this->flags & 0x400000) == 0) {
			pBehaviour->Manage();
		}
		else {
			pBehaviour->ManageFrozen();
		}
	}

	uVar1 = this->flags;
	fVar5 = (this->subObjA)->floatFieldB;
	if (((uVar1 & 0x100) == 0) || (fVar5 < this->distanceToCamera)) {
		if ((uVar1 & 0x4000) != 0) {
			ChangeVisibleState(0);
		}
	}
	else {
		bVar3 = CCameraManager::_gThis->IsSphereVisible(fVar5, &this->sphereCentre);
		if (bVar3 == false) {
			if ((this->flags & 0x4000) != 0) {
				ChangeVisibleState(0);
			}
		}
		else {
			if ((this->flags & 0x4000) == 0) {
				ChangeVisibleState(1);
			}
		}
	}

	if (this->pAnimationController != (CAnimation*)0x0) {
		uVar4 = this->currentAnimType == -1;
		if (this->actorState == 3) {
			fVar5 = 0.0f;
			uVar4 = 0;
		}
		else {
			if ((this->flags & 0x400000) == 0) {
				pTimeController = GetTimer();
				fVar5 = pTimeController->cutsceneDeltaTime;
			}
			else {
				fVar5 = 0.0f;
			}
		}
		this->pAnimationController->Manage(fVar5, this, this->flags & 0x4800, uVar4);
	}

	ComputeAltitude();

	IMPLEMENTATION_GUARD_AUDIO(
	//puVar2 = (uint*)this->field_0x144;
	//if (puVar2 != (uint*)0x0) {
	//	(*(code*)this->pVTable->field_0x5c)(this);
	//	for (; puVar2 != (uint*)0x0; puVar2 = (uint*)puVar2[0xe]) {
	//		CActorSound::Manage(puVar2, this);
	//	}
	//}
	)

	pTimeController = Timer::GetTimer();
	this->timeInAir = this->timeInAir + pTimeController->cutsceneDeltaTime;

	pTimeController = Timer::GetTimer();
	this->idleTimer = this->idleTimer + pTimeController->cutsceneDeltaTime;
	return;
}

void CActor::ChangeManageState(int state)
{
	EActorState actorState;
	ed_3d_hierarchy_node* pHierNode;
	CAnimation* pAnimation;
	CClusterNode* pClusterNode;
	StateConfig* pAVar4;
	uint uVar5;
	//CActorSound* pActorSound;

	if (state == 0) {
		this->flags = this->flags & 0xfffffffb;

		pClusterNode = this->pClusterNode;
		if (pClusterNode != (CClusterNode*)0x0) {
			(CScene::ptable.g_ActorManager_004516a4)->cluster.DeleteNode(pClusterNode);
			this->pClusterNode = (CClusterNode*)0x0;
		}

		IMPLEMENTATION_GUARD_AUDIO(
		for (pActorSound = (CActorSound*)this->field_0x144; pActorSound != (CActorSound*)0x0;
			pActorSound = *(CActorSound**)&pActorSound[1].field_0x4) {
			CActorSound::DisableSounds(pActorSound);
		})

		pHierNode = this->p3DHierNode;
		if (pHierNode != (ed_3d_hierarchy_node*)0x0) {
			(pHierNode->base).pAnimMatrix = (edF32MATRIX4*)0x0;
			(pHierNode->base).pShadowAnimMatrix = (edF32MATRIX4*)0x0;
		}

		actorState = this->actorState;
		if (actorState == AS_None) {
			uVar5 = 0;
		}
		else {
			IMPLEMENTATION_GUARD_AUDIO(
			pAVar4 = GetStateCfg(actorState);
			uVar5 = pAVar4->flags_0x4 & 0x80;)
		}

		if (uVar5 != 0) {
			IMPLEMENTATION_GUARD_AUDIO(
			StateTransitionSoundFunc_00184470((int)CScene::ptable.g_AudioManager_00451698);)
		}
	}
	else {
		this->flags = this->flags | 4;

		if ((this->pClusterNode == (CClusterNode*)0x0) && (this->typeID != 1)) {
			pClusterNode = (CScene::ptable.g_ActorManager_004516a4)->cluster.NewNode(this);
			this->pClusterNode = pClusterNode;
		}

		actorState = this->actorState;
		if (actorState == AS_None) {
			uVar5 = 0;
		}
		else {
			pAVar4 = GetStateCfg(actorState);
			uVar5 = pAVar4->flags_0x4 & 0x80;
		}

		if (uVar5 != 0) {
			IMPLEMENTATION_GUARD_LOG(
			StateTransitionSoundFunc_001844a0((int)CScene::ptable.g_AudioManager_00451698);)
		}
	}

	pAnimation = this->pAnimationController;
	if (pAnimation != (CAnimation*)0x0) {
		pAnimation->StopEventTrack(state);
	}

	return;
}

void CActor::ChangeDisplayState(int state)
{
	CShadow* pCVar1;
	uint ret;
	float BfloatB;

	if (state == 0) {
		pCVar1 = this->pShadow;
		if (pCVar1 != (CShadow*)0x0) {
			IMPLEMENTATION_GUARD(
			(**(code**)(*(int*)pCVar1 + 0x14))();)
		}
		this->flags = this->flags & 0xfffffeff;
	}
	else {
		pCVar1 = this->pShadow;
		if (pCVar1 != (CShadow*)0x0) {
			IMPLEMENTATION_GUARD(
			(**(code**)(*(int*)pCVar1 + 0x14))();)
		}
		this->flags = this->flags | 0x100;
	}

	ret = this->flags;
	BfloatB = (this->subObjA)->floatFieldB;
	if (((ret & 0x100) == 0) || (BfloatB < this->distanceToCamera)) {
		if ((ret & 0x4000) != 0) {
			this->ChangeVisibleState(0);
		}
	}
	else {
		bool bSphereVisible = CCameraManager::_gThis->IsSphereVisible(BfloatB, &this->sphereCentre);
		if (bSphereVisible == 0) {
			if ((this->flags & 0x4000) != 0) {
				this->ChangeVisibleState(0);
			}
		}
		else {
			if ((this->flags & 0x4000) == 0) {
				this->ChangeVisibleState(1);
			}
		}
	}

	return;
}

void CActor::SetState(int newState, int animType)
{
	EActorState curActorState;
	CAnimation* pAnimationController;
	CAudioManager* pGVar2;
	CBehaviour* pBehaviour;
	StateConfig* pNewStateCfg;
	uint curStateFlags;
	uint newStateFlags;

	ACTOR_LOG(LogLevel::Info, "CActor::SetState {} state: 0x{:x} anim: 0x{:x} (cur bhvr: 0x{:x})", this->name, newState, animType, this->curBehaviourId);

	pBehaviour = GetBehaviour(this->curBehaviourId);

	if ((animType == -1) && (newState != AS_None)) {
		pNewStateCfg = GetStateCfg(newState);
		animType = pNewStateCfg->animId;
	}

	pGVar2 = CScene::ptable.g_AudioManager_00451698;

	curActorState = this->actorState;
	if (curActorState == newState) {
		/* State is the same as our current state */
		if (this->currentAnimType != animType) {
			PlayAnim(animType);
		}
	}
	else {
		/* New state */
		curStateFlags = GetStateFlags(curActorState);
		newStateFlags = GetStateFlags(newState);

		if (((curStateFlags & 0x80) == 0) || ((newStateFlags & 0x80) != 0)) {
			if (((curStateFlags & 0x80) == 0) && ((newStateFlags & 0x80) != 0)) {
				IMPLEMENTATION_GUARD_LOG(
				StateTransitionSoundFunc_001844a0((int)pGVar2);)
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			StateTransitionSoundFunc_00184470((int)pGVar2);)
		}

		if ((pBehaviour != (CBehaviour*)0x0) && (curActorState = this->actorState, curActorState != AS_None)) {
			/* End old state? */
			pBehaviour->TermState(curActorState, newState);
		}

		this->prevActorState = this->actorState;
		this->actorState = (EActorState)newState;

		if ((this->numIdleLoops != 0) && (pAnimationController = this->pAnimationController, pAnimationController != (CAnimation*)0x0)) {
			pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		}

		if (this->actorState == AS_None) {
			PlayAnim(animType);
		}
		else {
			PlayAnim(animType);

			if (pBehaviour != (CBehaviour*)0x0) {
				pBehaviour->InitState(this->actorState);
			}
		}

		this->timeInAir = 0.0f;
		this->idleTimer = 0.0f;
		this->numIdleLoops = 0;
	}

	return;
}

bool CActor::SetBehaviour(int behaviourId, int newState, int animationType)
{
	uint uVar1;
	bool bVar2;
	CBehaviour* pComponent;
	CBehaviour* pSVar3;
	int iVar4;

	ACTOR_LOG(LogLevel::Info, "CActor::SetBehaviour {} bhvr: {} state: {} anim: {} (cur bhvr: {})", this->name, behaviourId, newState, animationType, this->curBehaviourId);

	uVar1 = this->flags;
	if (((uVar1 & 0x800000) == 0) && (((uVar1 & 0x2000000) == 0 || (behaviourId == 1)))) {
		if (this->curBehaviourId == behaviourId) {
			if (newState != -1) {
				this->SetState(newState, animationType);
			}

			bVar2 = false;
		}
		else {
			pComponent = (CBehaviour*)0x0;

			if ((behaviourId == -1) || (pComponent = this->GetBehaviour(behaviourId), pComponent != (CBehaviour*)0x0)) {
				pSVar3 = this->GetBehaviour(this->curBehaviourId);

				if (pSVar3 != (CBehaviour*)0x0) {
					if (pComponent == (CBehaviour*)0x0) {
						this->SetState(-1, -1);
					}
					else {
						this->SetState(-1, this->currentAnimType);
					}

					pSVar3->End(behaviourId);
				}

				this->prevBehaviourId = this->curBehaviourId;
				this->curBehaviourId = behaviourId;

				if (pComponent != (CBehaviour*)0x0) {
					pComponent->Begin(this, newState, animationType);

					if (this->dlistPatchId != -1) {
						IMPLEMENTATION_GUARD(
						iVar4 = GetManagerObject(MO_17);
						ActorFunc_002d79e0(iVar4, this->dlistPatchId, 0, (Actor*)this));
					}
				}
				bVar2 = true;
			}
			else {
				// Missing behaviour!
				// assert(false);
				ACTOR_LOG(LogLevel::Info, "CActor::SetBehaviour Missing Behaviour {} bhvr: {} state: {} anim: {} (cur bhvr: {})", this->name, behaviourId, newState, animationType, this->curBehaviourId);
				bVar2 = false;
			}
		}
	}
	else {
		bVar2 = false;
	}

	return bVar2;
}

void CActor::CinematicMode_Enter(bool bSetState)
{
	CCollision* pCVar1;
	CCinematic* pCinematic;
	CCineActorConfig* pActorConfig;

	DoMessage(this, (ACTOR_MESSAGE)0x3e, 0x0);

	if (bSetState == false) {
		this->SetBehaviour(1, -1, -1);
	}
	else {
		this->SetBehaviour(1, 1, -1);
	}

	this->flags = this->flags | 0x800000;

	FUN_00101110((CActor*)0x0);

	if ((this->flags & 0x2000060) != 0) {
		this->flags = this->flags | 0x8000000;
		this->flags = this->flags & 0xffffff5f;
		this->EvaluateDisplayState();
	}
	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
	pActorConfig = pCinematic->GetActorConfig(this);
	if (pActorConfig != (CCineActorConfig*)0x0) {
		if ((pActorConfig->flags & 0x20) == 0) {
			this->CinematicMode_InterpreteCinMessage(0.0f, 1.0f, 4, 0);
		}
		else {
			this->CinematicMode_InterpreteCinMessage(0.0f, 1.0f, 3, 0);
		}

		if (((pActorConfig->flags & 0x80) != 0) && (pCVar1 = this->pCollisionData, pCVar1 != (CCollision*)0x0)) {
			pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xfff7efff;
		}

		if (((pActorConfig->flags & 0x100) != 0) && (this->pTiedActor != (CActor*)0x0)) {
			IMPLEMENTATION_GUARD(
			(*(code*)this->pVTable->field_0xd0)(this, 0, 0, 1, 0);)
		}
	}
	return;
}

void CActor::CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition)
{
	UpdatePosition(pPosition, 1);
}

void CActor::CinematicMode_SetAnimation(edCinActorInterface::ANIM_PARAMStag* const pTag, int param_3)
{
	char cVar1;
	char cVar2;
	edANM_HDR* pDstHdr;
	edANM_HDR* pSrcHdr;
	edAnmLayer* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float local_30[4];
	float local_20[4];
	float local_10[4];
	CAnimation* pAnimation;

	pAnimation = this->pAnimationController;
	if (pAnimation != (CAnimation*)0x0) {
		pAnimation->anmBinMetaAnimator.SetLayerTimeWarper(0.0f, 0);
		pDstHdr = (edANM_HDR*)LOAD_SECTION(pTag->dstAnim.pHdr);
		pSrcHdr = (edANM_HDR*)LOAD_SECTION(pTag->srcAnim.pHdr);
		if ((pDstHdr == (edANM_HDR*)0x0) || (param_3 != 0)) {
			peVar3 = (pAnimation->anmBinMetaAnimator).aAnimData;
			cVar1 = pTag->srcAnim.field_0x8;
			fVar4 = pTag->srcAnim.field_0x4;
			peVar3->animPlayState = STATE_ANIM_NONE;
			peVar3->SetRawAnim(pSrcHdr, (uint)(cVar1 != '\0'), 0xfffffffe);
			edAnmStage::ComputeAnimParams(fVar4, (peVar3->currentAnimDesc).state.keyStartTime_0x14, 0.0f, local_10, false, (uint)(((peVar3->currentAnimDesc).state.currentAnimDataFlags & 1) != 0));
			(peVar3->currentAnimDesc).state.time_0x10 = local_10[0];
			(peVar3->currentAnimDesc).state.time_0xc = local_10[1];
		}
		else {
			peVar3 = (pAnimation->anmBinMetaAnimator).aAnimData;
			cVar1 = pTag->dstAnim.field_0x8;
			fVar5 = pTag->field_0x18;
			cVar2 = pTag->srcAnim.field_0x8;
			fVar4 = pTag->srcAnim.field_0x4;
			fVar6 = pTag->dstAnim.field_0x4;
			peVar3->animPlayState = STATE_ANIM_NONE;
			IMPLEMENTATION_GUARD(
			peVar3->SetRawAnim(pDstHdr, (uint)(cVar1 != '\0'), (int)&pDstHdr->flags + 1);
			peVar3->SetRawAnim(pSrcHdr, (uint)(cVar2 != '\0'), (int)&pSrcHdr->flags + 2);)
			peVar3->field_0xbc = 1.0f;
			peVar3->MorphingStartDT();
			(peVar3->currentAnimDesc).morphDuration = 1.0f;
			(peVar3->nextAnimDesc).morphDuration = fVar5;
			edAnmStage::ComputeAnimParams(fVar6, (peVar3->currentAnimDesc).state.keyStartTime_0x14, 0.0f, local_20, false, (uint)(((peVar3->currentAnimDesc).state.currentAnimDataFlags & 1) != 0));
			(peVar3->currentAnimDesc).state.time_0x10 = local_20[0];
			(peVar3->currentAnimDesc).state.time_0xc = local_20[1];
			edAnmStage::ComputeAnimParams(fVar4, (peVar3->nextAnimDesc).state.keyStartTime_0x14, 0.0f, local_30, false, (uint)(((peVar3->nextAnimDesc).state.currentAnimDataFlags & 1) != 0));
			(peVar3->nextAnimDesc).state.time_0x10 = local_30[0];
			(peVar3->nextAnimDesc).state.time_0xc = local_30[1];
		}
	}
	return;
}

uint CActor::IsLookingAt()
{
	return this->flags & 0x2000;
}

void CActor::SetLookingAtOn()
{
	this->flags = this->flags | 0x2000;

	return;
}

void CActor::SetLookingAtOff()
{
	this->flags = this->flags & 0xffffdfff;

	return;
}

void CActor::UpdateLookingAt()
{
	return;
}

void CActor::UpdateAnimEffects()
{
	CBehaviour* pBehaviour;
	Timer* pTVar2;

	if (((this->flags & 0x800000) != 0) && (pBehaviour = GetBehaviour(1), pBehaviour != (CBehaviour*)0x0)) {
		CBehaviourCinematic* pCinematicBehaviour = reinterpret_cast<CBehaviourCinematic*>(pBehaviour);
		pCinematicBehaviour->field_0x144.DoAnimation(Timer::GetTimer()->cutsceneDeltaTime, 1.0f, pCinematicBehaviour->pOwner);
	}
	return;
}

void CActor::UpdatePostAnimEffects()
{
	return;
}

void CActor::Destroy()
{
	int iVar2;
	BehaviourEntry* piVar3;

	if (this->typeID != -1) {
		BehaviourList<1>* pComponentList = (BehaviourList<1>*)this->aComponents;

		if ((pComponentList != (BehaviourList<1>*)0x0) && (this->typeID != 1)) {
			piVar3 = pComponentList->aComponents;
			for (iVar2 = pComponentList->count; 0 < iVar2; iVar2 = iVar2 + -1) {
				if (piVar3->pBehaviour != 0) {
					TermBehaviour(piVar3->id, piVar3->GetBehaviour());
				}

				piVar3 = piVar3 + 1;
			}
		}

		if (this->pMBNK != (void*)0x0) {
			edMemFree(this->pMBNK);
		}
	}

	return;
}

int CActor::ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam)
{
	int bHandled;
	CBehaviour* pBehaviour;

	pBehaviour = GetBehaviour(this->curBehaviourId);
	if (pBehaviour == (CBehaviour*)0x0) {
		bHandled = 0;
	}
	else {
		bHandled = pBehaviour->InterpretMessage(pSender, msg, pMsgParam);
	}

	if (bHandled == 0) {
		bHandled = InterpretMessage(pSender, msg, pMsgParam);
	}

	return bHandled;
}

int CActor::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	CCollision* pCVar2;
	CActor* pCVar3;
	bool bVar4;
	long lVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float local_18;
	float local_c;
	float local_8;

	fVar8 = gF32Vector4Zero.w;
	fVar7 = gF32Vector4Zero.z;
	fVar6 = gF32Vector4Zero.y;
	if (((msg == 0x78) || (msg == 0x77)) || (msg == 0x76)) {
		IMPLEMENTATION_GUARD(
		if (msg == 0x78) {
			local_18 = 0.3f;
		}
		else {
			if (msg == 0x77) {
				local_18 = 0.2f;
			}
			else {
				if (msg == 0x76) {
					local_18 = 0.1f;
				}
			}
		}
		lVar5 = (*(code*)this->pVTable->GetInputManager)(this, 1, 0);
		if (lVar5 != 0) {
			CPlayerInput::FUN_001b66f0(1.0f, 0.0f, local_18, 0.0f, (float*)((int)lVar5 + 0x1c), 0);
		}
		bVar4 = true;)
	}
	else {
		if (((((msg == 0x75) || (msg == 0x74)) || ((msg == 0x73 || ((msg == 0x72 || (msg == 0x71)))))) || (msg == 0x70)) ||
			(((msg == 0x6f || (msg == 0x6e)) || (msg == 0x6d)))) {
			IMPLEMENTATION_GUARD(
			switch (msg) {
			case 0x6d:
			case 0x6e:
			case 0x6f:
				local_c = 0.4f;
				if (msg == 0x6f) {
					local_8 = 0.6f;
				}
				else {
					if (msg == 0x6e) {
						local_8 = 0.4f;
					}
					else {
						if (msg == 0x6d) {
							local_8 = 0.2f;
						}
					}
				}
				break;
			case 0x70:
			case 0x71:
			case 0x72:
				local_c = 0.6;
				if (msg == 0x72) {
					local_8 = 0.5;
				}
				else {
					if (msg == 0x71) {
						local_8 = 0.3;
					}
					else {
						if (msg == 0x70) {
							local_8 = 0.15;
						}
					}
				}
				break;
			case 0x73:
			case 0x74:
			case 0x75:
				local_c = 1.0;
				if (msg == 0x75) {
					local_8 = 0.3;
				}
				else {
					if (msg == 0x74) {
						local_8 = 0.2;
					}
					else {
						if (msg == 0x73) {
							local_8 = 0.1;
						}
					}
				}
			}
			lVar5 = (*(code*)this->pVTable->GetInputManager)(this, 1, 0);
			if (lVar5 != 0) {
				CPlayerInput::FUN_001b66f0(local_c, 0.0, local_8, 0.0, (float*)((int)lVar5 + 0x40), 0);
			}
			bVar4 = true;)
		}
		else {
			if (msg == 0x6c) {
				IMPLEMENTATION_GUARD(
				lVar5 = (*(code*)this->pVTable->GetInputManager)(this, 1, 0);
				if (lVar5 != 0) {
					/* WARNING: Load size is inaccurate */
					if (*pMsgParam == 0) {
						CPlayerInput::FUN_001b66f0(*(float*)((int)pMsgParam + 4), 0.0, *(float*)((int)pMsgParam + 8), 0.0,
							(float*)((int)lVar5 + 0x1c), 0);
					}
					else {
						CPlayerInput::FUN_001b66f0(*(float*)((int)pMsgParam + 4), 0.0, *(float*)((int)pMsgParam + 8), 0.0,
							(float*)((int)lVar5 + 0x40), 0);
					}
				}
				bVar4 = true;)
			}
			else {
				if (msg == 0x5c) {
					this->flags = this->flags & 0xffffff5f;
					EvaluateDisplayState();
					this->flags = this->flags & 0xfffffffc;

					pCVar2 = this->pCollisionData;
					if (pCVar2 != (CCollision*)0x0) {
						pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x81000;
					}

					bVar4 = true;
				}
				else {
					if (msg == 0x5d) {
						pCVar2 = this->pCollisionData;
						if (pCVar2 != (CCollision*)0x0) {
							pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfff7efff;
						}
						this->flags = this->flags & 0xffffff7f;
						this->flags = this->flags | 0x20;
						EvaluateDisplayState();
						bVar4 = true;
						this->flags = this->flags & 0xfffffffd;
						this->flags = this->flags | 1;
					}
					else {
						if (msg == 0x32) {
							pCVar2 = this->pCollisionData;
							if ((pCVar2 != (CCollision*)0x0) && (pMsgParam != (void*)0x0)) {
								IMPLEMENTATION_GUARD(
								pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x81000;)
							}
							this->flags = this->flags | 0x80;
							this->flags = this->flags & 0xffffffdf;
							EvaluateDisplayState();
							bVar4 = true;
						}
						else {
							if (msg == MESSAGE_TRAP_CAUGHT) {
								pCVar2 = this->pCollisionData;
								if ((pCVar2 != (CCollision*)0x0) && (pMsgParam != (void*)0x0)) {
									pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfff7efff;
								}

								this->flags = this->flags & 0xffffff7f;
								this->flags = this->flags | 0x20;
								EvaluateDisplayState();
								bVar4 = true;
							}
							else {
								if (msg == 0x3c) {
									pCVar3 = this->pTiedActor;
									PreReset();
									Reset();
									if (((pCVar3 != (CActor*)0x0) && (pCVar3 != (CActor*)0x0)) &&
										((pCVar3->flags & 0x2000000) == 0)) {
										pCVar3->ReceiveMessage(this, (ACTOR_MESSAGE)0x3d, 0);
									}
									bVar4 = true;
								}
								else {
									if (msg == 0x38) {
										if ((this->flags & 0x400000) == 0) {
											this->flags = this->flags | 0x400000;
										}
										else {
											this->flags = this->flags & 0xffbfffff;
										}
										bVar4 = true;
									}
									else {
										if (msg == 0x37) {
											bVar4 = true;
											this->flags = this->flags & 0xffbfffff;
										}
										else {
											if (msg == 0x36) {
												bVar4 = true;
												this->flags = this->flags | 0x400000;
											}
											else {
												if (msg == MESSAGE_REQUEST_CAMERA_TARGET) {
													bVar4 = true;
													edF32VECTOR4* pResolvedMsg = (edF32VECTOR4*)pMsgParam;
													*pResolvedMsg = this->currentLocation;
												}
												else {
													if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
														GetPositionMsgParams* pResolvedMsg = static_cast<GetPositionMsgParams*>(pMsgParam);
														iVar1 = pResolvedMsg->field_0x0;
														if (iVar1 == 5) {
															if (((this->pMeshTransform != (ed_3d_hierarchy_node*)0x0) &&
																(pCVar2 = this->pCollisionData, pCVar2 != (CCollision*)0x0)) &&
																(pCVar2->pObbPrim != 0)) {
																(pResolvedMsg->vectorFieldB).x = 0.0f;
																(pResolvedMsg->vectorFieldB).y = (this->pCollisionData->pObbPrim->field_0xb0).y;
																(pResolvedMsg->vectorFieldB).z = 0.0f;
																(pResolvedMsg->vectorFieldB).w = 0.0f;
																edF32Matrix4MulF32Vector4Hard (&pResolvedMsg->vectorFieldB, &this->pMeshTransform->base.transformA, &pResolvedMsg->vectorFieldB);
																return true;
															}
														}
														else {
															if ((iVar1 == 1) || (iVar1 == 0)) {
																pResolvedMsg->vectorFieldB = gF32Vector4Zero;

																edColPRIM_OBJECT* pObj;
																pCVar2 = this->pCollisionData;
																if ((pCVar2 != (CCollision*)0x0) && (pObj = pCVar2->pObbPrim, pObj != 0)) {
																	edF32Matrix4MulF32Vector4Hard(&pResolvedMsg->vectorFieldB, &this->pMeshTransform->base.transformA, &pObj->field_0xb0);
																	edF32Vector4SubHard(&pResolvedMsg->vectorFieldB, &pResolvedMsg->vectorFieldB, &this->currentLocation);
																	return true;
																}

																return true;
															}
														}
													}
													bVar4 = false;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return bVar4;
}

void CActor::ChangeVisibleState(int bVisible)
{
	edNODE* peVar1;
	CShadow* pCVar2;
	bool bVar3;
	ed_3D_Scene* peVar4;

	if (bVisible == 0) {
		peVar1 = this->pMeshNode;
		if (peVar1 != (edNODE*)0x0) {
			peVar4 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOn(peVar4, peVar1);
		}
		pCVar2 = this->pShadow;
		if (pCVar2 != (CShadow*)0x0) {
			IMPLEMENTATION_GUARD(
			(**(code**)(*(int*)pCVar2 + 0x14))(pCVar2, 0);)
		}
		this->flags = this->flags & 0xffffbfff;
	}
	else {
		peVar1 = this->pMeshNode;
		if (peVar1 != (edNODE*)0x0) {
			peVar4 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOff(peVar4, peVar1);
		}
		pCVar2 = this->pShadow;
		if (pCVar2 != (CShadow*)0x0) {
			bVar3 = (this->flags & 0x4400) != 0;
			if (bVar3) {
				bVar3 = this->distanceToCamera <= (this->subObjA)->field_0x1c;
			}
			if (bVar3 != false) {
				bVar3 = (this->flags & 0x100) != 0;
			}
			IMPLEMENTATION_GUARD(
			(**(code**)(*(int*)pCVar2 + 0x14))(pCVar2, bVar3);)
		}
		this->flags = this->flags | 0x4000;
	}
	return;
}

CBehaviour* CActor::GetBehaviour(int behaviourId)
{
	BehaviourList<1>* pComponentList = (BehaviourList<1>*)this->aComponents;
	CBehaviour* pOutBehaviour = (CBehaviour*)0x0;

	for (int i = 0; i < pComponentList->count; i++) {
		if (pComponentList->aComponents[i].id == behaviourId) {
			pOutBehaviour = pComponentList->aComponents[i].GetBehaviour();
		}
	}

	return pOutBehaviour;
}

void CActor::SetupClippingInfo()
{
	this->hierarchySetup.clipping_0x0 = (float*)0x0;
	this->hierarchySetup.pBoundingSphere = &this->subObjA->boundingSphere;
}

void CActor::GetVisualDetectionPoint(edF32VECTOR4* pOutPoint, int index)
{
	edColPRIM_OBJECT* peVar1;
	GetPositionMsgParams local_40;
	float fVar3;

	if (IsLockable() == 0) {
		if ((this->pCollisionData == (CCollision*)0x0) ||
			(peVar1 = this->pCollisionData->pObbPrim, peVar1 == (edColPRIM_OBJECT*)0x0)) {
			fVar3 = 0.0f;
		}
		else {
			fVar3 = (peVar1->field_0xb0).y;
		}

		pOutPoint->x = (this->currentLocation).x;
		pOutPoint->y = fVar3 + (this->currentLocation).y;
		pOutPoint->z = (this->currentLocation).z;
		pOutPoint->w = 1.0f;
	}
	else {
		local_40.field_0x0 = 0;

		local_40.vectorFieldB.x = 0.0f;
		local_40.vectorFieldB.y = 0.0f;
		local_40.vectorFieldB.z = 0.0f;
		local_40.vectorFieldB.w = 0.0f;

		local_40.vectorFieldA = this->currentLocation;

		if ((this != (CActor*)0x0) && ((this->flags & 0x2000000) == 0)) {
			ReceiveMessage(this, MESSAGE_GET_VISUAL_DETECTION_POINT, &local_40);
		}

		*pOutPoint = this->currentLocation + local_40.vectorFieldB;
	}

	return;
}

int CActor::GetNumVisualDetectionPoints()
{
	return 1;
}

bool CActor::InitDlistPatchable(int param_2)
{
	if (GetBehaviour(this->curBehaviourId)->InitDlistPatchable(param_2) == false) {

		edDListLoadIdentity();
		edDListUseMaterial((edDList_material*)0x0);
		edDListBegin(0.0f, 0.0f, 0.0f, 8, 4);
		edDListColor4u8(0, 0, 0, 0);
		edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
		edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
		edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
		edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
		edDListEnd();
	}

	return true;
}

void CActor::SV_GetActorHitPos(CActor* pOtherActor, edF32VECTOR4* v0)
{
	int iVar1;
	GetPositionMsgParams local_40;

	local_40.field_0x0 = 1;

	local_40.vectorFieldB.x = 0.0f;
	local_40.vectorFieldB.y = 1.2f;
	local_40.vectorFieldB.z = 0.0f;
	local_40.vectorFieldB.w = 0.0f;

	local_40.vectorFieldA.x = v0->x;
	local_40.vectorFieldA.y = v0->y;
	local_40.vectorFieldA.z = v0->z;
	local_40.vectorFieldA.w = v0->w;

	iVar1 = DoMessage(pOtherActor, (ACTOR_MESSAGE)7, &local_40);
	if (iVar1 != 0) {
		edF32Vector4AddHard(v0, &pOtherActor->currentLocation, &local_40.vectorFieldB);
	}

	return;
}

float CActor::SV_GetDirectionalAlignmentToTarget(edF32VECTOR4* v0)
{
	float result;
	edF32VECTOR4 directionVector;

	edF32Vector4SubHard(&directionVector, v0, &this->currentLocation);
	edF32Vector4NormalizeHard(&directionVector, &directionVector);
	result = edF32Vector4DotProductHard(&this->rotationQuat, &directionVector);

	return result;
}

void CActor::SV_BuildAngleWithOnlyY(edF32VECTOR3* v0, edF32VECTOR3* v1)
{
	float fVar1;
	edF32MATRIX4 m0;

	edF32Matrix4FromEulerSoft(&m0, v1, "XYZ");
	v0->x = 0.0f;
	fVar1 = GetAngleYFromVector(&m0.rowZ);
	v0->y = fVar1;
	v0->z = 0.0f;
	return;
}

bool CActor::SV_UpdateOrientationToPosition2D(float speed, edF32VECTOR4* pOrientation)
{
	bool uVar1;
	float fVar1;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, pOrientation, &this->currentLocation);

	fVar1 = edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
	uVar1 = true;

	if (0.0f < fVar1) {
		uVar1 = SV_UpdateOrientation2D(speed, &eStack16, 0);
	}

	return uVar1;
}

bool CActor::SV_IsOrientation2DInRange(float param_1, edF32VECTOR4* param_3)
{
	bool bVar1;
	float fVar3;
	float puVar4;
	float puVar5;
	float fVar4;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if (param_1 == 0.0f) {
		bVar1 = false;
	}
	else {
		local_10.x = (this->rotationQuat).x;
		local_10.y = 0.0f;
		local_10.z = (this->rotationQuat).z;
		local_10.w = 0.0f;

		edF32Vector4NormalizeHard(&local_10, &local_10);

		local_20.x = param_3->x;
		local_20.y = 0.0f;
		local_20.z = param_3->z;
		local_20.w = 0.0f;

		edF32Vector4NormalizeHard(&local_20, &local_20);

		fVar3 = GetTimer()->cutsceneDeltaTime;
		puVar4 = edF32Vector4DotProductHard(&local_10, &local_20);
		if (1.0f < puVar4) {
			puVar5 = 1.0f;
		}
		else {
			puVar5 = -1.0f;
			if (-1.0f <= puVar4) {
				puVar5 = puVar4;
			}
		}

		fVar4 = acosf(puVar5);
		bVar1 = fVar4 <= param_1 * fVar3;
	}

	return bVar1;
}



void CActor::SV_RestoreOrgModel(CActorAlternateModel* pActorAlternateModel)
{
	int inAnimType;
	int textureIndex;
	int meshIndex;
	KyaUpdateObjA* pKVar1;
	ed_g3d_manager* pMeshInfo;
	ed_g2d_manager* peVar2;
	ed_g2d_manager* peVar3;
	ed_g2d_manager* pTextureInfo;
	float fVar4;
	float fVar5;
	C3DFileManager* p3DManager;

	if (this->pMeshNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pMeshNode);
		if (this->pMBNK != (void*)0x0) {
			edMemFree(this->pMBNK);
			this->pMBNK = (void*)0x0;
		}
	}

	inAnimType = this->currentAnimType;
	PlayAnim(-1);
	p3DManager = CScene::ptable.g_C3DFileManager_00451664;
	pMeshInfo = (ed_g3d_manager*)0x0;
	textureIndex = this->pCinData->textureIndex;
	meshIndex = this->pCinData->meshIndex;
	pTextureInfo = (ed_g2d_manager*)0x0;
	if (textureIndex == -1) {
		textureIndex = CScene::_pinstance->defaultTextureIndex_0x2c;
	}
	if ((meshIndex != -1) && (textureIndex != -1)) {
		pMeshInfo = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(meshIndex, textureIndex);
		peVar2 = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(textureIndex);
		peVar3 = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMeshMaterial(meshIndex);
		if (peVar3 != peVar2) {
			pTextureInfo = peVar2;
		}
	}

	SV_SetModel(pMeshInfo, 0, (MeshTextureHash*)0x0, pTextureInfo);

	pKVar1 = this->subObjA;
	pKVar1->boundingSphere = pActorAlternateModel->cachedBoundingSphere;
	if (this->pMeshTransform == (ed_3d_hierarchy_node*)0x0) {
		this->pMeshTransform = pActorAlternateModel->pHierarchy;
	}

	SetupClippingInfo();
	PlayAnim(inAnimType);
	UpdatePosition(&this->currentLocation, false);

	if (this->pAnimationController != (CAnimation*)0x0) {
		this->pAnimationController->Manage(0.0f, this, this->flags & 0x4800, 0);
	}

	return;
}

void CActor::SV_SwitchToModel(CActorAlternateModel* pAlternateModel, ed_g3d_manager* p3dManager, edF32VECTOR4* pBoundingSphere)
{
	KyaUpdateObjA* pKVar1;
	int inAnimType;
	float fVar2;
	float fVar3;

	pKVar1 = this->subObjA;
	pAlternateModel->cachedBoundingSphere = pKVar1->boundingSphere;
	pAlternateModel->pHierarchy = this->pMeshTransform;

	if (this->pMeshNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pMeshNode);
		if (this->pMBNK != (void*)0x0) {
			edMemFree(this->pMBNK);
			this->pMBNK = (void*)0x0;
		}
	}

	inAnimType = this->currentAnimType;
	PlayAnim(-1);

	if (pBoundingSphere != (edF32VECTOR4*)0x0) {
		pKVar1 = this->subObjA;
		pKVar1->boundingSphere = *pBoundingSphere;
	}

	SV_SetModel(p3dManager, 0, (MeshTextureHash*)0x0, (ed_g2d_manager*)0x0);
	SetupClippingInfo();
	PlayAnim(inAnimType);
	UpdatePosition(&this->currentLocation, false);

	if (this->pAnimationController != (CAnimation*)0x0) {
		this->pAnimationController->Manage(0.0f, this, this->flags & 0x4800, 0);
	}

	return;
}

void CActor::SV_SwitchToModel(CActorAlternateModel* pAlternateModel, int meshIndex, int materialIndex, edF32VECTOR4* pBoundingSphere)
{
	KyaUpdateObjA* pKVar1;
	int inAnimType;
	float fVar2;
	float fVar3;

	ed_g3d_manager* pMeshInfo;
	ed_g2d_manager* peVar2;
	ed_g2d_manager* peVar3;
	ed_g2d_manager* pTextureInfo;
	C3DFileManager* pFileManager;

	pKVar1 = this->subObjA;
	pAlternateModel->cachedBoundingSphere = pKVar1->boundingSphere;
	pAlternateModel->pHierarchy = this->pMeshTransform;

	if (this->pMeshNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pMeshNode);
		if (this->pMBNK != (void*)0x0) {
			edMemFree(this->pMBNK);
			this->pMBNK = (void*)0x0;
		}
	}

	inAnimType = this->currentAnimType;
	PlayAnim(-1);

	if (pBoundingSphere != (edF32VECTOR4*)0x0) {
		pKVar1 = this->subObjA;
		pKVar1->boundingSphere = *pBoundingSphere;
	}

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	pMeshInfo = (ed_g3d_manager*)0x0;
	pTextureInfo = (ed_g2d_manager*)0x0;
	if (materialIndex == -1) {
		materialIndex = CScene::_pinstance->defaultTextureIndex_0x2c;
	}
	if ((meshIndex != -1) && (materialIndex != -1)) {
		pMeshInfo = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(meshIndex, materialIndex);
		peVar2 = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(materialIndex);
		peVar3 = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMeshMaterial(meshIndex);
		if (peVar3 != peVar2) {
			pTextureInfo = peVar2;
		}
	}

	SV_SetModel(pMeshInfo, 0, (MeshTextureHash*)0x0, pTextureInfo);
	SetupClippingInfo();
	PlayAnim(inAnimType);
	UpdatePosition(&this->currentLocation, false);

	if (this->pAnimationController != (CAnimation*)0x0) {
		this->pAnimationController->Manage(0.0f, this, this->flags & 0x4800, 0);
	}

	return;
}

int CActor::SV_InstallMaterialId(int materialId)
{
	int index;

	if (materialId == -1) {
		index = 0;
	}
	else {
		index = CScene::ptable.g_C3DFileManager_00451664->InstanciateG2D(materialId);
	}
	return index;
}

void CActor::SV_InstanciateMaterialBank()
{
	int size;
	void* pvVar1;

	size = ed3DHierarchyBankMatGetSize(&this->p3DHierNode->base);
	pvVar1 = edMemAlloc(TO_HEAP(H_MAIN), size);
	pvVar1 = ed3DHierarchyBankMatInstanciate(&this->p3DHierNode->base, pvVar1);
	this->pMBNK = pvVar1;
	return;
}

bool CActor::IsLockable()
{
	bool bVar1;
	StateConfig* pSVar2;
	uint stateFlags;
	bool bLockable;

	stateFlags = GetStateFlags(this->actorState);

	bVar1 = this->pCollisionData != (CCollision*)0x0;
	bLockable = false;

	if (bVar1) {
		bVar1 = (stateFlags & 1) == 0;
	}

	if (bVar1) {
		bVar1 = (stateFlags & 4) != 0;
	}

	if ((bVar1) && ((CActorFactory::gClassProperties[this->typeID].field_0x4 & 0x20) != 0)) {
		bLockable = true;
	}

	return bLockable;
}

void CActor::SetupDefaultPosition()
{
	edF32VECTOR3* v0;
	float fVar1;
	float fVar2;
	CinNamedObject30* namedObj30;

	namedObj30 = this->pCinData;
	this->baseLocation.xyz = namedObj30->position;
	this->baseLocation.w = 1.0f;
	namedObj30 = this->pCinData;
	if (fabs(namedObj30->scale.x - 1.0f) <= 0.0001f) {
		namedObj30->scale.x = 1.0f;
	}
	if (fabs(namedObj30->scale.y - 1.0f) <= 0.0001f) {
		namedObj30->scale.y = 1.0f;
	}
	if (fabs(namedObj30->scale.z - 1.0f) <= 0.0001f) {
		namedObj30->scale.z = 1.0f;
	}
	if ((CActorFactory::gClassProperties[this->typeID].flags & 2) != 0) {
		v0 = &(this->pCinData)->rotationEuler;
		SV_BuildAngleWithOnlyY(v0, v0);
	}
	if ((CActorFactory::gClassProperties[this->typeID].flags & 0x800) == 0) {
		this->flags = this->flags & 0xffffefff;
	}
	else {
		this->flags = this->flags | 0x1000;
	}
	return;
}

void CActor::RestoreInitData()
{
	CCollision* pCVar1;
	CinNamedObject30* pCVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	if (((this->actorFieldS & 4) != 0) && (pCVar1 = this->pCollisionData, pCVar1 != (CCollision*)0x0)) {
		IMPLEMENTATION_GUARD(
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x20000;)
	}
	pCVar2 = this->pCinData;
	this->scale.xyz = pCVar2->scale;
	this->scale.w = 1.0f;
	if (((this->scale.x == 1.0f) && (this->scale.y == 1.0f)) && (this->scale.z == 1.0f)) {
		this->flags = this->flags & 0xfbffffff;
	}
	else {
		this->flags = this->flags | 0x4000000;
	}
	pCVar2 = this->pCinData;
	this->rotationEuler.xyz = pCVar2->rotationEuler;
	this->field_0x58 = 0;
	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
	UpdatePosition(&this->baseLocation, true);
	this->flags = this->flags | 0x80000;
	this->distanceToGround = -1.0f;
	this->flags = this->flags & 0xffdfffff;
	this->field_0xf4 = 0xffff;
	return;
}

void CActor::UpdatePosition(edF32VECTOR4* v0, bool bUpdateCollision)
{
	ed_3d_hierarchy_node* pHier;
	KyaUpdateObjA* pKVar1;
	CCollision* pCVar2;
	CClusterNode* pSubObj;
	int iVar3;
	edF32MATRIX4* pIdentityMatrix;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32MATRIX4 eStack224;
	edF32VECTOR4 local_a0;
	edF32MATRIX4 eStack144;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	this->currentLocation = *v0;
	pHier = this->pMeshTransform;

	if ((this->flags & 0x4000000) == 0) {
		if ((this->flags & 0x1000) == 0) {
			edF32Matrix4BuildFromVectorUnitSoft(&pHier->base.transformA, &this->rotationQuat);
			fVar5 = this->rotationEuler.z;
			if (0.0001f <= fabs(fVar5)) {
				edF32Matrix4RotateZHard(fVar5, &eStack224, &gF32Matrix4Unit);
				edF32Matrix4MulF32Matrix4Hard(&pHier->base.transformA, &eStack224, &pHier->base.transformA);
			}
		}
		else {
			edF32Matrix4FromEulerSoft(&pHier->base.transformA, &this->rotationEuler.xyz, "XYZ");
		}

		(pHier->base).transformA.rowT = this->currentLocation;

		pKVar1 = this->subObjA;
		local_50.xyz = pKVar1->boundingSphere.xyz;
		local_50.w = 1.0;
		edF32Matrix4MulF32Vector4Hard(&local_50, &pHier->base.transformA, &local_50);


		this->sphereCentre.xyz = local_50.xyz;
		this->sphereCentre.w = (pKVar1->boundingSphere).w;
		if ((bUpdateCollision != false) && (pCVar2 = this->pCollisionData, pCVar2 != (CCollision*)0x0)) {
			pCVar2->UpdateMatrix(&pHier->base.transformA);
		}
	}
	else {
		if ((this->flags & 0x1000) == 0) {
			edF32Matrix4BuildFromVectorUnitSoft(&eStack64, &this->rotationQuat);
			fVar5 = this->rotationEuler.z;
			if (0.0001f <= fabs(fVar5)) {
				edF32Matrix4RotateZHard(fVar5, &eStack144, &gF32Matrix4Unit);
				edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack144, &eStack64);
			}
		}
		else {
			edF32Matrix4FromEulerSoft(&eStack64, &this->rotationEuler.xyz, "XYZ");
		}
		eStack64.rowT = this->currentLocation;

		(pHier->base).transformA = gF32Matrix4Unit;

		
		(pHier->base).transformA.rowX.x = this->scale.x;
		(pHier->base).transformA.rowY.y = this->scale.y;
		(pHier->base).transformA.rowZ.z = this->scale.z;
		edF32Matrix4MulF32Matrix4Hard(&pHier->base.transformA, &pHier->base.transformA, &eStack64);
		pKVar1 = this->subObjA;
		local_a0.xyz = (pKVar1->boundingSphere).xyz;
		local_a0.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&local_a0, &pHier->base.transformA, &local_a0);
		this->sphereCentre.xyz = local_a0.xyz;
		this->sphereCentre.w = (pKVar1->boundingSphere).w * std::max(this->scale.z, std::max(this->scale.y, this->scale.x));

		if (bUpdateCollision != false) {
			pCVar2 = this->pCollisionData;
			if (pCVar2 != (CCollision*)0x0) {
				pCVar2->UpdateMatrix(&eStack64);
			}
		}
	}
	pSubObj = this->pClusterNode;
	if (pSubObj != (CClusterNode*)0x0) {
		pSubObj->Update(&(CScene::ptable.g_ActorManager_004516a4)->cluster);
	}
	return;
}

void CActor::UpdatePosition(edF32MATRIX4* pPosition, int bUpdateCollision)
{
	ed_3d_hierarchy_node* pHier;
	KyaUpdateObjA* pKVar1;
	CCollision* pCollisionData;
	CClusterNode* pSubObj;
	int iVar2;
	edF32MATRIX4* peVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	ACTOR_LOG(LogLevel::Verbose, "CActor::UpdatePosition {} {}", this->name, pPosition->ToString());

	this->currentLocation = pPosition->rowT;

	pHier = this->pMeshTransform;
	if ((this->flags & 0x4000000) == 0) {
		(pHier->base).transformA = *pPosition;
		
		pKVar1 = this->subObjA;
		local_10.xyz = (pKVar1->boundingSphere).xyz;
		local_10.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&local_10, &pHier->base.transformA, &local_10);
		this->sphereCentre.xyz = local_10.xyz;
		this->sphereCentre.w = (pKVar1->boundingSphere).w;
	}
	else {
		(pHier->base).transformA = gF32Matrix4Unit;


		(pHier->base).transformA.rowX.x = this->scale.x;
		(pHier->base).transformA.rowY.y = this->scale.y;
		(pHier->base).transformA.rowZ.z = this->scale.z;

		edF32Matrix4MulF32Matrix4Hard(&pHier->base.transformA, &pHier->base.transformA, pPosition);
		pKVar1 = this->subObjA;
		local_20.xyz = (pKVar1->boundingSphere).xyz;
		local_20.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&local_20, &pHier->base.transformA, &local_20);
		this->sphereCentre.xyz = local_20.xyz;
		this->sphereCentre.w = (pKVar1->boundingSphere).w * std::max(this->scale.z, std::max(this->scale.y, this->scale.x));
	}

	if ((bUpdateCollision != 0) && (pCollisionData = this->pCollisionData, pCollisionData != (CCollision*)0x0))
	{
		pCollisionData->UpdateMatrix(pPosition);
	}

	pSubObj = this->pClusterNode;
	if (pSubObj != (CClusterNode*)0x0) {
		pSubObj->Update(&(CScene::ptable.g_ActorManager_004516a4)->cluster);
	}

	return;
}

void CActor::PlayAnim(int inAnimType)
{
	int animType;
	CAnimation* pAnimationController;

	ACTOR_LOG(LogLevel::Info, "CActor::PlayAnim {} 0x{:x}", this->name, inAnimType);

	if (inAnimType != this->currentAnimType) {
		/* Continue playing existing animation */
		pAnimationController = this->pAnimationController;
		if ((this->field_0x110 == (undefined*)0x0) || (pAnimationController == (CAnimation*)0x0)) {
			this->currentAnimType = -1;
		}
		else {
			/* Play new animation */
			animType = -1;

			if (inAnimType != -1) {
				/* Remove for T-Pose */
				animType = GetIdMacroAnim(inAnimType);
			}

			if (animType == -1) {
				this->currentAnimType = -1;
				pAnimationController->Reset(this);
			}
			else {
				this->prevAnimType = this->currentAnimType;
				this->currentAnimType = inAnimType;
				pAnimationController->PlayAnim(this, animType, inAnimType);
			}
		}
	}

	return;
}

void CActor::LoadBehaviours(ByteCode* pByteCode)
{
	int iVar1;
	char* pcVar2;
	char* pcVar3;
	char* pcVar4;
	int componentCount;
	BehaviourEntry* pComponentStrm;

	pByteCode->Align(4);

#ifdef PLATFORM_WIN
	static_assert(sizeof(BehaviourEntry) == 8, "BehaviourEntry must be 8 bytes to match serialized data!");
	static_assert(offsetof(BehaviourList<1>, aComponents) == 4, "Components array must be 4 bytes ahead of count!");
#endif

	BehaviourList<1>* pComponentList = (BehaviourList<1>*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)&pComponentList->aComponents;
	if (pComponentList->count != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (pComponentList->count * sizeof(BehaviourEntry));
	}

	ACTOR_LOG(LogLevel::Info, "CActor::LoadBehaviours {} total: {}", this->name, pComponentList->count);

	this->aComponents = pComponentList;
	componentCount = pComponentList->count;
	if (0 < componentCount) {
		if (((this->flags & 0x2000000) == 0) || ((CActorFactory::gClassProperties[this->typeID].flags & 0x40000) == 0)) {
			BehaviourEntry* pEntry = pComponentList->aComponents;;

			for (; 0 < componentCount; componentCount = componentCount + -1) {
				const int componentStrmLen = pEntry->GetSize();

				ACTOR_LOG(LogLevel::Info, "CActor::LoadBehaviours {} {} id: {} length: {}", this->name, componentCount - 1, pEntry->id, componentStrmLen);

				CBehaviour* pNewBehaviour = BuildBehaviour(pEntry->id);
				pEntry->SetBehaviour(pNewBehaviour);

				pcVar2 = pByteCode->GetPosition();

				if (pEntry->GetBehaviour() != (CBehaviour*)0x0) {
					pEntry->GetBehaviour()->Create(pByteCode);
				}

				pcVar3 = pByteCode->GetPosition();

				int processedLen = pcVar3 - pcVar2;

				if (processedLen != componentStrmLen) {
					pByteCode->SetPosition(pcVar2 + componentStrmLen);
				}

				pEntry = pEntry + 1;
			}
		}
		else {
			for (; 0 < componentCount; componentCount = componentCount + -1) {
				BehaviourEntry* pEntry = &pComponentList->aComponents[componentCount - 1];
				pcVar4 = pByteCode->GetPosition();
				pByteCode->SetPosition(pcVar4 + pEntry->GetSize());
			}
			pComponentList->count = 0;
			(this->subObjA)->defaultBehaviourId = -1;
		}
	}
	return;
}

void CActor::CheckpointReset()
{
	bool bIsMoveable;
	//CActorSound* pCVar2;
	float fVar3;
	float fVar4;
	CAnimation* pCVar5;

	this->vector_0x120 = this->currentLocation.xyz;

	bIsMoveable = IsKindOfObject(2);

	if (bIsMoveable == false) {
		this->vector_0x12c = gF32Vector3Zero;
	}
	else {
		this->vector_0x12c = static_cast<CActorMovable*>(this)->dynamic.velocityDirectionEuler.xyz;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	for (pCVar2 = this->aActorSounds; pCVar2 != (CActorSound*)0x0; pCVar2 = (CActorSound*)pCVar2[1].field_0x0) {
		pCVar2->Reset();
	})

	return;
}

void CActor::Term()
{
	CActorSound* pCVar1;
	int componentCount;
	int* piVar3;

	SetBehaviour(-1, -1, -1);

	BehaviourList<1>* pComponentList = (BehaviourList<1>*)this->aComponents;
	BehaviourEntry* pEntry = pComponentList->aComponents;

	for (componentCount = pComponentList->count; componentCount != 0; componentCount = componentCount + -1) {
		CBehaviour* pBehaviour = pEntry->GetBehaviour();
		if (pBehaviour == NULL) { pEntry = pEntry + 1; continue; } // HACK FOR BEHAVIOUR WHILE IMPLEMENTING
		pBehaviour->Term();
		pEntry = pEntry + 1;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	for (pCVar1 = this->aActorSounds; pCVar1 != (CActorSound*)0x0; pCVar1 = (CActorSound*)pCVar1[1].field_0x0) {
		CActorSound::Term((int)pCVar1);
	}

	pCVar1 = this->aActorSounds;
	while (pCVar1 != (CActorSound*)0x0) {
		CSimpleLinkedList<CActorSound>::RemoveHead((int*)&this->aActorSounds);
		pCVar1 = this->aActorSounds;
	})

	if (this->pClusterNode != (CClusterNode*)0x0) {
		(CScene::ptable.g_ActorManager_004516a4)->cluster.DeleteNode(this->pClusterNode);
		this->pClusterNode = (CClusterNode*)0x0;
	}

	if (this->pMeshNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pMeshNode);
		this->pMeshNode = (edNODE*)0x0;
	}

	return;
}

void CActor::SetupModel(int count, MeshTextureHash* aHashes)
{
	CinNamedObject30* pCVar1;
	int index;
	ed_3d_hierarchy_node* peVar2;
	CCollision* pCollisionData;

	pCVar1 = this->pCinData;
	SV_SetModel(pCVar1->meshIndex, pCVar1->textureIndex, count, aHashes);
	if (this->pMeshTransform == (ed_3d_hierarchy_node*)0x0) {
		peVar2 = (ed_3d_hierarchy_node*)NewPool_edF32MATRIX4(1);
		this->pMeshTransform = peVar2;
	}
	index = (this->pCinData)->collisionDataIndex;
	if (index != -1) {
		pCollisionData = CScene::ptable.g_CollisionManager_00451690->NewCCollision();
		pCollisionData->Create(this, index);
		this->pCollisionData = pCollisionData;
		if ((CActorFactory::gClassProperties[this->typeID].field_0x4 & 0x200) != 0) {
			CCollision::PatchObbTreeFlagsRecurse(pCollisionData->pObbTree, 0x200, 0xffffffffffffffff, 0);
		}
		if ((CActorFactory::gClassProperties[this->typeID].field_0x4 & 0x8000) != 0) {
			pCollisionData->flags_0x0 = pCollisionData->flags_0x0 | 0x800;
		}
		if ((CActorFactory::gClassProperties[this->typeID].field_0x4 & 0x10000) != 0) {
			pCollisionData->flags_0x0 = pCollisionData->flags_0x0 & 0xfffbffff;
		}
	}
	return;
}

bool CActor::CanPassThrough()
{
	bool bVar1;
	bool bCanPassThrough;
	uint stateFlags;

	stateFlags = GetStateFlags(this->actorState);
	bCanPassThrough = true;
	bVar1 = true;

	if (((CActorFactory::gClassProperties[this->typeID].field_0x4 & 0x2000) == 0) && ((stateFlags & 8) == 0)) {
		bVar1 = false;
	}

	if ((!bVar1) && ((stateFlags & 1) == 0)) {
		bCanPassThrough = false;
	}

	return bCanPassThrough;
}

bool CActor::IsProjectionAim()
{
	bool bIsProjectionAim;

	bIsProjectionAim = this->pCollisionData != (CCollision*)0x0;
	if (bIsProjectionAim) {
		bIsProjectionAim = (GetStateFlags(this->actorState) & 1) == 0;
	}

	if (bIsProjectionAim) {
		bIsProjectionAim = (CActorFactory::gClassProperties[this->typeID].flags & 0x8000) != 0;
	}

	return bIsProjectionAim;
}

void CActor::SV_SetModel(int meshIndex, int textureIndex, int count, MeshTextureHash* aHashes)
{
	ed_g3d_manager* pMeshInfo;
	ed_g2d_manager* pTVar1;
	ed_g2d_manager* pTVar2;
	ed_g2d_manager* pTextureInfo;

	pMeshInfo = (ed_g3d_manager*)0x0;
	pTextureInfo = (ed_g2d_manager*)0x0;

	if (textureIndex == -1) {
		textureIndex = CScene::_pinstance->defaultTextureIndex_0x2c;
	}

	if ((meshIndex != -1) && (textureIndex != -1)) {
		pMeshInfo = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(meshIndex, textureIndex);
		pTVar1 = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(textureIndex);
		pTVar2 = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMeshMaterial(meshIndex);
		if (pTVar2 != pTVar1) {
			pTextureInfo = pTVar1;
		}
	}

	SV_SetModel(pMeshInfo, count, aHashes, pTextureInfo);

	return;
}

void CActor::SV_SetModel(ed_g3d_manager* pMeshInfo, int count, MeshTextureHash* aHashes, ed_g2d_manager* pTextureInfo)
{
	ed_3d_hierarchy_node* pHier;
	ed_g2d_manager* pTextureInfo_00;
	ed_g3d_hierarchy* peVar1;
	edNODE* peVar2;
	int iVar3;
	void* pvVar4;
	ed_hash_code* pHashCode;
	ed_3D_Scene* peVar5;
	void* uVar6;
	ed_hash_code* lVar7;
	ed_hash_code* peVar8;
	int iVar9;
	ulong uVar10;
	int iVar11;

	if (pMeshInfo == (ed_g3d_manager*)0x0) {
		this->pHier = (ed_g3d_hierarchy*)0x0;
		this->pMeshNode = (edNODE*)0x0;
		this->p3DHierNode = (ed_3d_hierarchy_node*)0x0;
		this->pMBNK = 0;
		this->pMeshTransform = (ed_3d_hierarchy_node*)0x0;
	}
	else {
		pTextureInfo_00 = CScene::ptable.g_C3DFileManager_00451664->LoadDefaultTexture_001a65d0();

		if (pTextureInfo_00 != (ed_g2d_manager*)0x0) {
			ed3DLinkG2DToG3D(pMeshInfo, pTextureInfo_00);
		}

		peVar1 = ed3DG3DHierarchyGetFromIndex(pMeshInfo, 0);
		this->pHier = peVar1;
		peVar1 = this->pHier;
		if (peVar1 != (ed_g3d_hierarchy*)0x0) {
			if ((CActorFactory::gClassProperties[this->typeID].flags & 0x1000) != 0) {
				if (1 < peVar1->lodCount) {
					peVar1->flags_0x9e = peVar1->flags_0x9e | 0x100;
				}

				ed3DG3DHierarchySetStripShadowCastFlag(this->pHier, 0xffff);
			}

			if ((CActorFactory::gClassProperties[this->typeID].flags & 0x2000) != 0) {
				ed3DG3DHierarchySetStripShadowReceiveFlag(this->pHier, 0xffff);
			}
		}

		peVar2 = ed3DHierarchyAddToScene(CScene::_scene_handleA, pMeshInfo, (char*)0x0);
		this->pMeshNode = peVar2;
		this->p3DHierNode = (ed_3d_hierarchy_node*)(this->pMeshNode)->pData;
		this->pMeshTransform = this->p3DHierNode;
		peVar2 = this->pMeshNode;
		if (peVar2 != (edNODE*)0x0) {
			ed3DSetMeshTransformFlag_002abd80(peVar2, 0xffff);
			if ((CActorFactory::gClassProperties[this->typeID].flags & 0x1000) != 0) {
				ed3DSetMeshTransformFlag_002abff0(this->pMeshNode, 1);
			}
			if ((CActorFactory::gClassProperties[this->typeID].flags & 0x2000) != 0) {
				ed3DSetMeshTransformFlag_002abff0(this->pMeshNode, 0xffff);
			}
		}
		if ((((CActorFactory::gClassProperties[this->typeID].flags & 0x4000) == 0) &&
			(pTextureInfo == (ed_g2d_manager*)0x0)) && (count == 0)) {
			this->pMBNK = 0;
		}
		else {
			iVar3 = ed3DHierarchyBankMatGetSize((ed_3d_hierarchy*)this->p3DHierNode);
			pvVar4 = edMemAlloc(TO_HEAP(H_MAIN), iVar3);
			this->pMBNK = ed3DHierarchyBankMatInstanciate((ed_3d_hierarchy*)this->p3DHierNode, pvVar4);
		}

		if (pTextureInfo != (ed_g2d_manager*)0x0) {
			ed3DHierarchyBankMatLinkG2D(this->p3DHierNode, pTextureInfo);
			ed3DHierarchyBankMatLinkG2D(this->p3DHierNode, pTextureInfo_00);
		}

		if (((count != 0) && (pHier = this->p3DHierNode, pHier != (ed_3d_hierarchy_node*)0x0)) &&
			(pTextureInfo_00 != (ed_g2d_manager*)0x0)) {
			pHashCode = ed3DHierarchyGetMaterialBank(&pHier->base);
			iVar3 = ed3DG2DGetG2DNbMaterials(pHashCode);
			iVar11 = 0;
			if (0 < count) {
				do {
					uVar10 = aHashes->meshHash;
					lVar7 = ed3DG2DGetMaterial(pTextureInfo_00, aHashes->textureHash);
					if ((lVar7 != (ed_hash_code*)0x0) && (iVar9 = 0, peVar8 = pHashCode, 0 < iVar3)) {
						do {
							if (uVar10 == peVar8->hash.number) {
								pHashCode[iVar9].pData = STORE_SECTION(lVar7);
								break;
							}
							iVar9 = iVar9 + 1;
							peVar8 = peVar8 + 1;
						} while (iVar9 < iVar3);
					}
					iVar11 = iVar11 + 1;
					aHashes = aHashes + 1;
				} while (iVar11 < count);
			}
		}

		memset(&this->hierarchySetup, 0, sizeof(ed_3d_hierarchy_setup));

		if (1 < ((this->p3DHierNode)->base).lodCount) {
			this->hierarchySetup.pLodBiases = this->lodBiases;
		}

		ed3DHierarchySetSetup(&this->p3DHierNode->base, &this->hierarchySetup);

		((this->p3DHierNode)->base).pAnimMatrix = (edF32MATRIX4*)0x0;
		((this->p3DHierNode)->base).pShadowAnimMatrix = (edF32MATRIX4*)0x0;

		ed3DHierarchyNodeGetSkeletonChunck(this->pMeshNode, false);

		if ((this->flags & 0x4000) == 0) {
			peVar5 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOn(peVar5, this->pMeshNode);
		}
		else {
			peVar5 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOff(peVar5, this->pMeshNode);
		}
	}
	return;
}

void CActor::SaveContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

void CActor::LoadContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviour::GetDlistPatchableNbVertexAndSprites(int* nbVertex, int* nbSprites)
{
	*nbVertex = 0;
	*nbSprites = 0;
	return;
}

int CBehaviour::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CActor::Draw()
{
	CShadow* pShadow;
	CBehaviour* pBehaviour;

	pShadow = this->pShadow;
	if (pShadow != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		pShadow->Draw(););
	}
	pBehaviour = GetBehaviour(this->curBehaviourId);
	if (pBehaviour != (CBehaviour*)0x0) {
		pBehaviour->Draw();
	}
	return;
}

void CActor::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	if (this->curBehaviourId == 1) {
		IMPLEMENTATION_GUARD_LOG(
		AnimEvaluateLipsync(layerId, pAnimator);)
	}
	return;
}

void CActor::SetupLodInfo()
{
	float fVar1;

	this->lodBiases[0] = (this->subObjA)->lodBiases[0];
	this->lodBiases[1] = (this->subObjA)->lodBiases[1];
	this->lodBiases[2] = 10000.0;
	this->lodBiases[3] = 1e+10;
	fVar1 = this->lodBiases[0];
	this->lodBiases[0] = fVar1 * fVar1;
	fVar1 = this->lodBiases[1];
	this->lodBiases[1] = fVar1 * fVar1;
	fVar1 = this->lodBiases[2];
	this->lodBiases[2] = fVar1 * fVar1;
	fVar1 = this->lodBiases[3];
	this->lodBiases[3] = fVar1 * fVar1;
	return;
}

float CActor::GetPosition_00117db0()
{
	edColPRIM_OBJECT* peVar1;
	float fVar2;
	float fVar3;

	fVar3 = 0.5f;
	if ((this->pCollisionData != (CCollision*)0x0) && (peVar1 = this->pCollisionData->pObbPrim, fVar3 = 0.5f, peVar1 != (edColPRIM_OBJECT*)0x0)) {
		fVar2 = (peVar1->field_0x90).z;
		fVar3 = (peVar1->field_0x90).x;
		if (fVar3 <= fVar2) {
			fVar3 = fVar2;
		}
	}

	return fVar3;
}


void CActor::GetPosition_00101130(edF32VECTOR4* pOutPosition)
{
	CActor* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	bool bVar5;
	edF32VECTOR4 newPosition;

	*pOutPosition = gF32Vector4Zero;

	for (pCVar1 = this->pTiedActor; pCVar1 != (CActor*)0x0; pCVar1 = pCVar1->pTiedActor) {
		bVar5 = pCVar1->IsKindOfObject(OBJ_TYPE_MOVABLE);
		if (bVar5 == false) {
			newPosition = gF32Vector4Zero;
		}
		else {
			CActorMovable* pMovable = static_cast<CActorMovable*>(pCVar1);
			edF32Vector4ScaleHard((pMovable->dynamic).linearAcceleration, &newPosition, &(pMovable->dynamic).velocityDirectionEuler);
		}

		edF32Vector4AddHard(pOutPosition, pOutPosition, &newPosition);
	}

	return;
}



CActorSound* CActor::CreateActorSound(int soundType)
{
	IMPLEMENTATION_GUARD_AUDIO();
	return NULL;
}

void CActor::Compute2DOrientationFromAngles()
{
	edF32VECTOR3 newRotationEuler;

	SV_BuildAngleWithOnlyY(&newRotationEuler, &this->rotationEuler.xyz);
	(this->rotationEuler).xyz = newRotationEuler;
	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);

	return;
}

CActor* CActor::GetLinkFather()
{
	_linked_actor* pLinkedActorEntry;
	CActor* pLinkFather;

	pLinkedActorEntry = CScene::ptable.g_ActorManager_004516a4->FindLinkedActor(this);

	if (pLinkedActorEntry == (_linked_actor*)0x0) {
		pLinkFather = (CActor*)0x0;
	}
	else {
		pLinkFather = pLinkedActorEntry->pLinkedActor;
	}

	return pLinkFather;
}


void CActor::UpdateBoundingSphere(CActInstance* pInstances, int nbInstances)
{
	int iVar1;
	int instanceClassSize;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if (1 < nbInstances) {
		instanceClassSize = sizeof(CBnsInstance);
		if (this->typeID != BONUS) {
			instanceClassSize = sizeof(CMnyInstance);
		}

		local_10.y = 1e+08f;
		iVar1 = 0;
		local_20.z = -1e+08f;
		fVar3 = local_20.z;
		local_20.x = local_20.z;
		fVar4 = local_20.z;
		local_20.y = local_20.z;
		fVar5 = local_20.z;
		fVar6 = local_10.y;
		local_10.z = local_10.y;
		fVar7 = local_10.y;
		local_10.x = local_10.y;
		fVar8 = local_10.y;
		if (0 < nbInstances) {
			do {
				local_30 = pInstances->currentPosition;

				local_10.x = local_30.x;
				if (fVar8 <= local_30.x) {
					local_10.x = fVar8;
				}
				local_20.x = local_30.x;
				if (local_30.x <= fVar4) {
					local_20.x = fVar4;
				}
				local_10.y = local_30.y;
				if (fVar6 <= local_30.y) {
					local_10.y = fVar6;
				}
				local_20.y = local_30.y;
				if (local_30.y <= fVar5) {
					local_20.y = fVar5;
				}
				local_10.z = local_30.z;
				if (fVar7 <= local_30.z) {
					local_10.z = fVar7;
				}
				local_20.z = local_30.z;
				if (local_30.z <= fVar3) {
					local_20.z = fVar3;
				}

				iVar1 = iVar1 + 1;
				pInstances = reinterpret_cast<CActInstance*>(reinterpret_cast<char*>(pInstances) + instanceClassSize);
				fVar3 = local_20.z;
				fVar4 = local_20.x;
				fVar5 = local_20.y;
				fVar6 = local_10.y;
				fVar7 = local_10.z;
				fVar8 = local_10.x;
			} while (iVar1 < nbInstances);
		}

		local_20.x = local_20.x - local_10.x;
		local_20.w = 0.0f;
		local_20.y = local_20.y - local_10.y;
		local_20.z = local_20.z - local_10.z;
		local_10.w = 1.0f;

		edF32Vector4ScaleHard(0.5f, &eStack64, &local_20);
		edF32Vector4AddHard(&local_10, &local_10, &eStack64);
		fVar3 = edF32Vector4GetDistHard(&local_20);
		edF32Vector4SubHard(&local_10, &local_10, &this->currentLocation);
		SetLocalBoundingSphere(fVar3 * 0.5f + 10.0f, &local_10);
	}

	return;
}



StateConfig CActor::gStateCfg_ACT[5] =
{
	StateConfig(0, 4),
	StateConfig(1, 0),
	StateConfig(0, 0),
	StateConfig(0, 0),
	StateConfig(5, 0)
};

uint CActor::_gBehaviourFlags_ACT[2] =
{
	0, 0
};

StateConfig* CActor::GetStateCfg(int state)
{
	assert(state < 5);
	return gStateCfg_ACT + state;
}

uint CActor::GetBehaviourFlags(int state)
{
	uint uVar1;

	assert(state < 2);

	if (state == -1) {
		uVar1 = 0;
	}
	else {
		uVar1 = _gBehaviourFlags_ACT[state];
	}
	return uVar1;
}

void CAddOnGenerator::Create(CActor* pActor, ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	int iVar3;
	int* piVar4;
	void* pvVar5;
	int iVar6;
	int iVar7;

	//iVar6 = (pActor->data).objectId;
	uVar1 = pByteCode->GetU32();
	//this->field_0x0 = (ushort)uVar1 & 3;
	uVar1 = pByteCode->GetU32();
	//this->field_0x2 = (ushort)uVar1;
	iVar2 = pByteCode->GetS32();
	//this->field_0x4 = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->currentOrbs_0x8 = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->field_0xc = (float)iVar2 * 0.01;
	this->subObj.Create(pByteCode);
	//this->field_0x28 = 0;
	//this->maxOrbs_0x2c = 0;
	//this->field_0x30 = 0;
	//if (_gAddOn_sectors == (void*)0x0) {
	//	iVar7 = LevelScheduleManager::gThis->aLevelInfo[LevelScheduleManager::gThis->currentLevelID].sectorCount_0x14 + 1;
	//	_gAddOn_sectors = operator.new.array((long)(iVar7 * 0x10));
	//	iVar2 = 0;
	//	if (0 < iVar7) {
	//		iVar3 = 0;
	//		do {
	//			iVar2 = iVar2 + 1;
	//			*(undefined4*)((int)_gAddOn_sectors + iVar3) = 0;
	//			*(undefined4*)((int)_gAddOn_sectors + iVar3 + 4) = 0;
	//			*(undefined4*)((int)_gAddOn_sectors + iVar3 + 8) = 0;
	//			*(undefined4*)((int)_gAddOn_sectors + iVar3 + 0xc) = 0;
	//			iVar3 = iVar3 + 0x10;
	//		} while (iVar2 < iVar7);
	//	}
	//}
	//_gAddOn_NbTotalAddOn = _gAddOn_NbTotalAddOn + 1;
	//(&_gAddOn_NbTotalMoneyInLevel)[(uint)this->field_0x2 * 4] =
	//	(&_gAddOn_NbTotalMoneyInLevel)[(uint)this->field_0x2 * 4] + this->field_0x4;
	//_gAddOn_NbTotalBonusInLevel = _gAddOn_NbTotalBonusInLevel + this->currentOrbs_0x8;
	//if (iVar6 == -1) {
	//	iVar6 = 0;
	//}
	//piVar4 = (int*)((int)_gAddOn_sectors + (uint)this->field_0x2 * 4 + iVar6 * 0x10);
	//*piVar4 = *piVar4 + this->field_0x4;
	//pvVar5 = (void*)(iVar6 * 0x10 + (int)_gAddOn_sectors);
	//*(int*)((int)pvVar5 + 0xc) = *(int*)((int)pvVar5 + 0xc) + this->currentOrbs_0x8;
	return;
}

void CActor::RestartCurAnim()
{
	if ((this->pAnimationController != (CAnimation*)0x0) && (this->currentAnimType != -1)) {
		this->pAnimationController->anmBinMetaAnimator.SetLayerAnimTime(0.0f, 0, 1);
	}

	return;
}

void CActor::SetupLighting()
{
	this->lightingFlags = 1;
	if (((this->subObjA)->flags_0x48 & 1) != 0) {
		this->lightingFlags = this->lightingFlags | 2;
	}
	if (((this->subObjA)->flags_0x48 & 2) == 0) {
		this->lightingFlags = this->lightingFlags | 4;
	}
	this->lightingFloat_0xe0 = (this->subObjA)->lightingFloat_0x4c;

	// #HACK - Usually cleared by memzero on whole memory?
	this->actorFieldS = 0;

	if ((CActorFactory::gClassProperties[this->typeID].flags & 0x20) != 0) {
		this->actorFieldS = this->actorFieldS | 0x10;
	}
	if (CActorFactory::gClassProperties[this->typeID].field_0x14 == 0) {
		this->actorFieldS = this->actorFieldS & 0xffffffef;
	}
	return;
}

int CActor::DoMessage(CActor* pReceiver, ACTOR_MESSAGE type, MSG_PARAM flags)
{
	int uVar1;

	if ((pReceiver == (CActor*)0x0) || ((pReceiver->flags & 0x2000000) != 0)) {
		uVar1 = false;
	}
	else {
		uVar1 = pReceiver->ReceiveMessage(this, type, flags);
	}

	return uVar1;
}

void CAddOnGenerator_SubObj::Create(ByteCode* pByteCode)
{
	uint uVar1;
	float fVar2;

	uVar1 = pByteCode->GetU32();
	if (uVar1 == 1) {
		fVar2 = pByteCode->GetF32();
		this->field_0x0 = fVar2 * 0.01745329f;
		fVar2 = pByteCode->GetF32();
		this->field_0x4 = fVar2 * 0.01745329f;
		this->field_0x8 = 0.0;
		fVar2 = pByteCode->GetF32();
		this->field_0xc = fVar2 * 0.01745329f;
		fVar2 = pByteCode->GetF32();
		this->field_0x10 = fVar2;
		fVar2 = pByteCode->GetF32();
		this->field_0x14 = fVar2;
	}
	else {
		this->field_0x0 = 0.7853982f;
		this->field_0x4 = 0.3926991f;
		this->field_0x8 = 0.0f;
		this->field_0xc = 3.141593f;
		this->field_0x10 = 3.0f;
		this->field_0x14 = 2.5f;
	}
	return;
}

void CActor::SV_LookTo(CActorParamsOut* pActorParamsOut, CActorParamsIn* pActorParamsIn)
{
	edF32VECTOR4* peVar1;
	byte bVar2;
	bool bVar3;
	Timer* pTVar4;
	float fVar5;
	float puVar6;
	float puVar7;
	float fVar8;
	edF32VECTOR4 eStack112;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if ((pActorParamsIn->flags & 2) == 0) {
		this->rotationQuat = *pActorParamsIn->pRotation;
	}
	else {
		if ((pActorParamsIn->flags & 8) == 0) {
			peVar1 = pActorParamsIn->pRotation;
			local_10.z = peVar1->z;
			local_10.w = peVar1->w;
			local_10.x = peVar1->x;
			local_10.y = 0.0f;

			fVar5 = edF32Vector4GetDistHard(&local_10);
			if (0.0f < fVar5) {
				bVar3 = SV_UpdateOrientation2D(pActorParamsIn->rotSpeed, &local_10, 0);
				pActorParamsOut->flags = pActorParamsOut->flags | (int)bVar3;
			}
		}
		else {
			fVar5 = edF32Vector4GetDistHard((edF32VECTOR4*)pActorParamsOut);
			if (0.0f < fVar5) {
				peVar1 = pActorParamsIn->pRotation;
				fVar5 = pActorParamsIn->rotSpeed;
				local_20 = this->rotationQuat;

				puVar6 = edF32Vector4DotProductHard(&local_20, peVar1);
				if (1.0f < puVar6) {
					puVar7 = 1.0f;
				}
				else {
					puVar7 = -1.0f;
					if (-1.0f <= puVar6) {
						puVar7 = puVar6;
					}
				}

				fVar8 = acosf(puVar7);

				pTVar4 = GetTimer();
				fVar5 = fVar5 * pTVar4->cutsceneDeltaTime;
				if ((fVar5 <= 0.0f) || (fVar8 <= fVar5)) {
					bVar2 = 1;
					local_20 = *peVar1;
				}
				else {
					edF32Vector4CrossProductHard(&eStack112, &local_20, peVar1);
					edF32Matrix4BuildFromVectorAndAngle(fVar5, &eStack96, &eStack112);
					edF32Matrix4MulF32Vector4Hard(&local_20, &eStack96, &local_20);
					edF32Vector4NormalizeHard(&local_20, &local_20);
					bVar2 = 0;
				}

				this->rotationQuat = local_20;

				GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
				pActorParamsOut->flags = pActorParamsOut->flags | (uint)bVar2;
			}
		}
	}

	return;
}

float CActor::SV_AttractActorInAConeAboveMe(CActor* pActor, CActorConeInfluence* pActorConeInfluence)
{
	edF32VECTOR4* peVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	peVar1 = GetBottomPosition();
	edF32Vector4AddHard(&eStack16, peVar1, &pActorConeInfluence->field_0x20);
	peVar1 = pActor->GetTopPosition();
	edF32Vector4SubHard(&local_20, peVar1, &eStack16);

	if ((local_20.y < -0.01f) || (pActorConeInfluence->field_0x8 < local_20.y)) {
		fVar5 = pActorConeInfluence->field_0x8 + 0.001f;
	}
	else {
		fVar3 = edFIntervalLERP(local_20.y, 0.0f, pActorConeInfluence->field_0x8, pActorConeInfluence->field_0x0, pActorConeInfluence->field_0x4);
		fVar5 = local_20.y;
		if (fVar3 * fVar3 < local_20.z * local_20.z + local_20.x * local_20.x) {
			fVar5 = pActorConeInfluence->field_0x8 + 0.001f;
		}
		else {
			CActorAutonomous* pActorAutonomous = (CActorAutonomous*)pActor;
			if ((pActorConeInfluence->field_0x14 == 2) || ((pActorConeInfluence->field_0x14 == 1 &&
					(pActorAutonomous->dynamic.linearAcceleration * pActorAutonomous->dynamic.velocityDirectionEuler.y < 1.0f)))) {
				local_20.y = 0.0f;

				fVar3 = edF32Vector4DotProductHard(&pActorAutonomous->dynamic.horizontalVelocityDirectionEuler, &local_20);
				if (fVar3 < 0.0f) {
					fVar4 = edFIntervalLERP(fVar5, 0.0f, pActorConeInfluence->field_0x8, pActorConeInfluence->field_0xc, pActorConeInfluence->field_0x10);
					pTVar2 = GetTimer();
					fVar3 = pTVar2->cutsceneDeltaTime;
					pTVar2 = GetTimer();
					edF32Vector4ScaleHard(-(((fVar4 * 0.01f) / fVar3) / pTVar2->cutsceneDeltaTime), &local_20, &local_20);
					peVar1 = pActorAutonomous->dynamicExt.aImpulseVelocities;
					edF32Vector4AddHard(peVar1, peVar1, &local_20);
					fVar3 = edF32Vector4GetDistHard(pActorAutonomous->dynamicExt.aImpulseVelocities);
					pActorAutonomous->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar3;
				}
			}
		}
	}

	return fVar5;
}

void CActor::PreReset()
{
	float fVar1;
	float fVar2;
	float fVar3;
	CAnimation* pAnimation;
	CinNamedObject30* pCinData;
	CCollision* pCollision;

	if ((this->flags & 4) != 0) {
		ChangeManageState(0);
	}

	if ((this->flags & 0x100) != 0) {
		ChangeDisplayState(0);
	}

	SetBehaviour(-1, -1, -1);

	if (this->pTiedActor != (CActor*)0x0) {
		TieToActor(0, 0, 1, 0);
	}

	pCollision = this->pCollisionData;
	if (pCollision != (CCollision*)0x0) {
		pCollision->Reset();
	}

	pAnimation = this->pAnimationController;
	if (pAnimation != (CAnimation*)0x0) {
		pAnimation->Reset(this);
	}

	if (((this->actorFieldS & 4) != 0) &&
		(pCollision = this->pCollisionData, pCollision != (CCollision*)0x0)) {
		pCollision->flags_0x0 = pCollision->flags_0x0 | 0x20000;
	}

	pCinData = this->pCinData;
	this->scale.xyz = pCinData->scale;
	this->scale.w = 1.0f;

	if (((this->scale.x == 1.0f) && (this->scale.y == 1.0f)) && (this->scale.z == 1.0f)) {
		this->flags = this->flags & 0xfbffffff;
	}
	else {
		this->flags = this->flags | 0x4000000;
	}

	pCinData = this->pCinData;
	this->rotationEuler.xyz = pCinData->rotationEuler;
	this->field_0x58 = 0;

	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
	UpdatePosition(&this->baseLocation, true);

	this->flags = this->flags | 0x80000;
	this->distanceToGround = -1.0f;
	this->flags = this->flags & 0xffdfffff;
	this->field_0xf4 = 0xffff;
	this->flags = this->flags & 0xfffffffc;
	this->flags = this->flags & 0xffffff5f;

	EvaluateDisplayState();

	this->flags = this->flags & 0xfffff7ff;
	this->flags = this->flags & 0xffbfffff;

	if ((((this->actorFieldS & 1) != 0) && (this != (CActor*)0x0)) && ((this->flags & 0x2000000) == 0)) {
		ReceiveMessage(this, (ACTOR_MESSAGE)0x5d, 0);
	}

	return;
}

edF32VECTOR4* CActor::GetBottomPosition()
{
	edF32VECTOR4* pBottomPosition;

	if (this->pCollisionData == (CCollision*)0x0) {
		pBottomPosition = &this->currentLocation;
	}
	else {
		pBottomPosition = &this->pCollisionData->lowestVertex;
	}

	return pBottomPosition;
}

edF32VECTOR4* CActor::GetTopPosition()
{
	edF32VECTOR4* pTopPosition;

	if (this->pCollisionData == (CCollision*)0x0) {
		pTopPosition = &this->currentLocation;
	}
	else {
		pTopPosition = &this->pCollisionData->highestVertex;
	}

	return pTopPosition;
}

void CActor::CinematicMode_Leave(int behaviourId)
{
	CCollision* pCVar1;
	ed_3d_hierarchy_node* peVar2;
	uint uVar3;
	CCinematic* pCinematic;
	CCineActorConfig* pCVar4;

	ACTOR_LOG(LogLevel::Info, "CActor::CinematicMode_Leave {}", this->name);

	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
	pCVar4 = pCinematic->GetActorConfig(this);
	if (((pCVar4 != (CCineActorConfig*)0x0) && ((pCVar4->flags & 0x80) != 0)) &&
		(pCVar1 = this->pCollisionData, pCVar1 != (CCollision*)0x0)) {
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x81000;
	}

	peVar2 = this->p3DHierNode;
	if (peVar2 != (ed_3d_hierarchy_node*)0x0) {
		ed3DUnLockLOD(peVar2);
	}

	uVar3 = this->flags;
	if ((uVar3 & 0x8000000) != 0) {
		this->flags = uVar3 & 0xf7ffffff;
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;
		EvaluateDisplayState();
	}

	this->flags = this->flags & 0xff7fffff;
	this->flags = this->flags & 0xffffffbf;
	EvaluateDisplayState();
	SetBehaviour(behaviourId, -1, -1);

	return;
}

bool CActor::CinematicMode_InterpreteCinMessage(float, float, int param_2, int param_3)
{
	bool bSuccess;
	CBehaviourCinematic* pBehaviourCinematic;

	bSuccess = false;
	pBehaviourCinematic = (CBehaviourCinematic*)GetBehaviour(1);
	if (pBehaviourCinematic != (CBehaviourCinematic*)0x0) {
		bSuccess = pBehaviourCinematic->CinematicMode_InterpreteCinMessage(param_2, param_3);
	}
	return bSuccess;
}

void CActor::SkipToNextActor(ByteCode* pByteCode) 
{
	ACTOR_LOG(LogLevel::Info, "CActor::Create SKIPPED");
	CActor::Create(pByteCode);

	SkipToNextActorNoBase(pByteCode);
}

void CActor::SkipToNextActorNoBase(ByteCode* pByteCode)
{
	ACTOR_LOG(LogLevel::Info, "CActor::Create NAME: {}", this->name);

	char* pCurrent = pByteCode->currentSeekPos;

	while (true) {
		if (strncmp(pCurrent, "TSNI", 4) == 0) {
			break;
		}
		pCurrent++;
	}

	pByteCode->currentSeekPos = pCurrent;
}

void CActor::ComputeWorldBoundingSphere(edF32VECTOR4* v0, edF32MATRIX4* m0)
{
	KyaUpdateObjA* pKVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 local_10;

	pKVar1 = this->subObjA;
	local_10.xyz = (pKVar1->boundingSphere).xyz;

	local_10.w = 1.0f;

	edF32Matrix4MulF32Vector4Hard(&local_10, m0, &local_10);

	v0->xyz = local_10.xyz;
	v0->w = (pKVar1->boundingSphere).w * std::max(this->scale.z, std::max(this->scale.y, this->scale.x));
	return;
}

bool CActor::SV_IsWorldBoundingSphereIntersectingSphere(edF32VECTOR4* param_2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	fVar5 = this->sphereCentre.w;
	fVar1 = this->sphereCentre.x - param_2->x;
	fVar2 = this->sphereCentre.y - param_2->y;
	fVar3 = this->sphereCentre.z - param_2->z;
	fVar4 = param_2->w;
	return fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3 <= fVar5 * (fVar4 * 2.0f + fVar5) + fVar4 * fVar4;
}

bool CActor::SV_IsWorldBoundingSphereIntersectingBox(S_BOUNDING_BOX* pBoundingBox)
{
	undefined* puVar1;
	bool bIsIntersecting;
	undefined* puVar3;
	undefined* puVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 local_10;

	fVar6 = (this->sphereCentre).w;	
	fVar5 = (this->sphereCentre).x;

	if ((((pBoundingBox->maxPoint.x + fVar6 < fVar5) || (fVar5 < pBoundingBox->minPoint.x - fVar6)) ||
		(fVar5 = (this->sphereCentre).y, pBoundingBox->maxPoint.y + fVar6 < fVar5)) || (((fVar5 < pBoundingBox->minPoint.y - fVar6 ||
			(fVar5 = (this->sphereCentre).z, pBoundingBox->maxPoint.z + fVar6 < fVar5)) || (fVar5 < pBoundingBox->minPoint.z - fVar6)))) {
		bIsIntersecting = false;
	}
	else {
		bIsIntersecting = true;
	}

	COLLISION_LOG(LogLevel::VeryVerbose, "CActor::SV_IsWorldBoundingSphereIntersectingBox {} {}", this->name, bIsIntersecting);

	return bIsIntersecting;
}

void CActor::TermBehaviour(int behaviourId, CBehaviour* pBehaviour)
{
	// Basically only delete the behaviour if its not a class member...
	if ((((reinterpret_cast<char*>(pBehaviour) < reinterpret_cast<char*>(this)) ||
		((reinterpret_cast<char*>(this) + (CScene::ptable.g_ActorManager_004516a4)->aClassInfo[this->typeID].size) < reinterpret_cast<char*>(pBehaviour))) &&
		(behaviourId != 1)) && (pBehaviour != (CBehaviour*)0x0)) {
		delete pBehaviour;
	}

	return;
}

CBehaviour* CActor::BuildBehaviour(int behaviourType)
{
	CBehaviour* pCVar1;

	if (behaviourType == 1) {
		pCVar1 = (CBehaviour*)NewPool_CBehaviourCinematic(1);
	}
	else {
		if (behaviourType == 0) {
			pCVar1 = &this->standBehaviour;
		}
		else {
			pCVar1 = (CBehaviour*)0x0;
		}
	}
	return pCVar1;
}

void CBehaviourStand::Init(CActor* pOwner)
{
	this->pOwner = pOwner;
}

void CBehaviourStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}
	return;
}

bool CActor::SV_PatchMaterial(ulong originalHash, ulong newHash, ed_g2d_manager* pMaterial)
{
	int index;
	bool bVar1;
	ed_hash_code* pHashCode;
	ed_hash_code* peVar2;
	int iVar3;
	ed_g2d_manager* pTextureInfo;
	int iVar4;
	C3DFileManager* pFileManager;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	if (pMaterial == (ed_g2d_manager*)0x0) {
		pMaterial = pFileManager->LoadDefaultTexture_001a65d0();
	}

	pHashCode = ed3DHierarchyGetMaterialBank((ed_3d_hierarchy*)this->p3DHierNode);
	peVar2 = ed3DG2DGetMaterial(pMaterial, newHash);
	iVar3 = ed3DG2DGetG2DNbMaterials(pHashCode);
	iVar4 = 0;
	if ((peVar2 == (ed_hash_code*)0x0) && (index = (this->pCinData)->textureIndex, index != -1)) {
		pTextureInfo = pFileManager->GetActorsCommonMaterial(index);
		peVar2 = ed3DG2DGetMaterial(pTextureInfo, newHash);
	}

	for (; (pHashCode->hash.number != originalHash && (iVar4 < iVar3)); iVar4 = iVar4 + 1) {
		pHashCode = pHashCode + 1;
	}

	if ((iVar4 == iVar3) || (peVar2 == (ed_hash_code*)0x0)) {
		bVar1 = false;
	}
	else {
		pHashCode->pData = STORE_SECTION(peVar2);
		bVar1 = true;
	}

	return bVar1;
}

void CActor::ComputeLighting()
{
	ACTOR_LOG(LogLevel::VeryVerbose, "CActor::ComputeLighting {} flags: {:x}", this->name, this->lightingFlags);
	CScene::ptable.g_LightManager_004516b0->ComputeLighting(this->lightingFloat_0xe0, this, this->lightingFlags, (ed_3D_Light_Config*)0x0);
	return;
}

void CActor::SectorChange(int oldSectorId, int newSectorId)
{
	CBehaviour* pBehvaiour;

	pBehvaiour = GetBehaviour(this->curBehaviourId);
	if (pBehvaiour != (CBehaviour*)0x0) {
		pBehvaiour->SectorChange(oldSectorId, newSectorId);
	}
	return;
}

void CActor::Reset()
{
	bool bVar1;
	//CActorSound* pActorSound;
	float fVar2;
	float fVar3;
	CAnimation* pAnimation;

	this->vector_0x120 = this->currentLocation.xyz;
	bVar1 = IsKindOfObject(2);
	if (bVar1 == false) {
		this->vector_0x12c = gF32Vector3Zero;
	}
	else {
		CActorMovable* pMovable = static_cast<CActorMovable*>(this);
		pMovable->vector_0x12c = pMovable->dynamic.velocityDirectionEuler.xyz * pMovable->dynamic.linearAcceleration;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	for (pActorSound = this->aActorSounds; pActorSound != (CActorSound*)0x0;
		pActorSound = *(CActorSound**)&pActorSound[1].field_0x4) {
		CActorSound::Reset(pActorSound);
	})
	SetBehaviour((this->subObjA)->defaultBehaviourId, -1, -1);
	return;
}

int CActor::GetIdMacroAnim(int inAnimType)
{
	int iVar1;
	int ret;
	int* piVar2;
	int iVar3;
	int iVar4;
	int iVar5;

	piVar2 = (int*)this->field_0x110;
	if ((piVar2 == (int*)0x0) || (iVar4 = *piVar2, iVar4 == 0)) {
		ret = -1;
	}
	else {
		iVar3 = iVar4 >> 1;
		piVar2 = piVar2 + 1;
		iVar5 = 0;
		if (iVar3 != 0) {
			do {
				iVar1 = iVar3;
				if (piVar2[iVar3 * 2] <= inAnimType) {
					iVar1 = iVar4;
					iVar5 = iVar3;
				}
				iVar4 = iVar1;
				iVar3 = iVar5 + iVar4 >> 1;
			} while (iVar3 != iVar5);
		}
		ret = -1;
		if (inAnimType == piVar2[iVar5 * 2]) {
			ret = (piVar2 + iVar5 * 2)[1];
		}
	}
	return ret;
}


void CActor::SV_GetActorTargetPos(CActor* pOtherActor, edF32VECTOR4* pTargetPos)
{
	int iVar1;
	GetPositionMsgParams local_50;
	edF32VECTOR4 eStack32;

	local_50.field_0x0 = 0;
	local_50.vectorFieldA = *pTargetPos;

	iVar1 = DoMessage(pOtherActor, MESSAGE_GET_VISUAL_DETECTION_POINT, &local_50);
	if (iVar1 != 0) {
		iVar1 = DoMessage(pOtherActor, (ACTOR_MESSAGE)0x49, &eStack32);
		if (iVar1 != 0) {
			edF32Vector4AddHard(pTargetPos, &eStack32, &local_50.vectorFieldB);
		}
	}

	return;
}



void CActor::UpdateVisibility()
{
	uint uVar1;
	bool bVar2;
	float other;

	uVar1 = this->flags;
	other = (this->subObjA)->floatFieldB;
	if (((uVar1 & 0x100) == 0) || (other < this->distanceToCamera)) {
		if ((uVar1 & 0x4000) != 0) {
			ChangeVisibleState(0);
		}
	}
	else {
		bVar2 = CCameraManager::_gThis->IsSphereVisible(other, &this->sphereCentre);
		if (bVar2 == false) {
			if ((this->flags & 0x4000) != 0) {
				ChangeVisibleState(0);
			}
		}
		else {
			if ((this->flags & 0x4000) == 0) {
				ChangeVisibleState(1);
			}
		}
	}
	return;
}

void CActor::FUN_00101110(CActor* pOtherActor)
{
	CCollision* pCVar1;

	pCVar1 = this->pCollisionData;
	if (pCVar1 != (CCollision*)0x0) {
		pCVar1->actorField = pOtherActor;
	}
	return;
}


void CActor::SetScaleVector(float x, float y, float z)
{
	this->scale.x = x;
	this->scale.y = y;
	this->scale.z = z;
	this->scale.w = 1.0f;
	if (((x == 1.0f) && (y == 1.0f)) && (z == 1.0f)) {
		this->flags = this->flags & 0xfbffffff;
	}
	else {
		this->flags = this->flags | 0x4000000;
	}
	return;
}

void CActor::ComputeAltitude()
{
	uint actorFlags;
	CCollision* pCVar2;
	bool bUpdateAltitude;
	float fVar4;
	//CCollisionRay CStack96;
	edF32VECTOR4 rayLocation;
	edF32VECTOR4 rayDirection;
	edF32VECTOR4 local_20;
	_ray_info_out local_10;
	CShadow* pShadow;

	actorFlags = this->flags;
	bUpdateAltitude = false;

	if (((actorFlags & 0x100000) != 0) && ((actorFlags & 0x200000) == 0)) {
		bUpdateAltitude = true;
	}

	ACTOR_LOG(LogLevel::VeryVerbose, "CActor::ComputeAltitude {} flags: 0x{:x} bUpdateAltitude: {} before distance: {}", this->name, actorFlags, bUpdateAltitude, this->distanceToGround);

	if ((bUpdateAltitude) || (this->distanceToGround == -1.0f)) {
		static edF32VECTOR4 edF32VECTOR4_0040e180 = { 0.0f, 1.0f, 0.0f, 0.0f };
		static edF32VECTOR4 edF32VECTOR4_0040e190 = { 0.0f, -1.0f, 0.0f, 0.0f };

		local_20 = edF32VECTOR4_0040e180;
		rayDirection = edF32VECTOR4_0040e190;

		pCVar2 = this->pCollisionData;
		if ((pCVar2 == (CCollision*)0x0) || ((pCVar2->flags_0x0 & 0x40000) == 0)) {
			rayLocation = this->currentLocation;
		}
		else {
			rayLocation = pCVar2->highestVertex;
		}

		rayLocation.y = rayLocation.y + 0.3f;
		CCollisionRay CStack96 = CCollisionRay(this->field_0xf0, &rayLocation, &rayDirection);
		fVar4 = CStack96.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, this, (CActor*)0x0, 0x40000048, &local_20, &local_10);
		this->distanceToGround = fVar4;

		if (fVar4 == 1e+30f) {
			this->distanceToGround = this->field_0xf0;
			this->flags = this->flags & 0xfeffffff;
			this->field_0xf4 = 0xffff;
		}
		else {
			this->field_0xf4 = local_10.type_0x4[1];
			this->flags = this->flags | 0x1000000;

			if (local_10.pActor_0x0 != (CActor*)0x0) {
				this->flags = this->flags & 0xfeffffff;
			}
		}

		ACTOR_LOG(LogLevel::VeryVerbose, "CActor::ComputeAltitude {} after distance: {}", this->name, this->distanceToGround);

		pShadow = this->pShadow;
		if (pShadow != (CShadow*)0x0) {
			IMPLEMENTATION_GUARD(
			rayLocation.y = rayLocation.y - this->distanceToGround;
			(pShadow->base).field_0x10.x = rayLocation.x;
			(pShadow->base).field_0x10.y = rayLocation.y;
			(pShadow->base).field_0x10.z = rayLocation.z;
			(pShadow->base).field_0x10.w = rayLocation.w;
			(pShadow->base).field_0x4c = this->rotationEuler.y;
			CShadowShared::SetIntensity(1.0 - this->distanceToGround / this->field_0xec, pShadow);
			(pShadow->base).field_0x20.x = local_20.x;
			(pShadow->base).field_0x20.y = local_20.y;
			(pShadow->base).field_0x20.z = local_20.z;
			(pShadow->base).field_0x20.w = local_20.w;)
		}

		fVar4 = this->distanceToGround;
		if (fVar4 < this->field_0xf0) {
			this->distanceToGround = fVar4 - 0.3f;
			ACTOR_LOG(LogLevel::VeryVerbose, "CActor::ComputeAltitude {} adjust after distance: {}", this->name, this->distanceToGround);
		}
	}

	this->flags = this->flags & 0xffdfffff;
	return;
}

void CActor::ComputeRotTransMatrix(edF32MATRIX4* pOutMatrix)
{
	edF32MATRIX4 eStack64;

	if ((this->flags & 0x1000) == 0) {
		edF32Matrix4BuildFromVectorUnitSoft(pOutMatrix, &this->rotationQuat);

		if (0.0001f <= fabs((this->rotationEuler).z)) {
			edF32Matrix4RotateZHard((this->rotationEuler).z, &eStack64, &gF32Matrix4Unit);
			edF32Matrix4MulF32Matrix4Hard(pOutMatrix, &eStack64, pOutMatrix);
		}
	}
	else {
		edF32Matrix4FromEulerSoft(pOutMatrix, &this->rotationEuler.xyz, "XYZ");
	}

	pOutMatrix->rowT = this->currentLocation;

	return;
}

void CActor::ComputeLocalMatrix(edF32MATRIX4* m0, edF32MATRIX4* m1)
{
	*m0 = gF32Matrix4Unit;

	m0->aa = (this->scale).x;
	m0->bb = (this->scale).y;
	m0->cc = (this->scale).z;

	edF32Matrix4MulF32Matrix4Hard(m0, m0, m1);

	return;
}

void CActor::ResetActorSound()
{
	IMPLEMENTATION_GUARD_AUDIO();
}

bool CActor::IsMakingNoise()
{
	return GetStateFlags(this->actorState) & 2;
}

void CActor::FillThisFrameExpectedDifferentialMatrix(edF32MATRIX4* pMatrix)
{
	edF32MATRIX4* peVar3;

	if (this->pTiedActor == (CActor*)0x0) {
		*pMatrix = gF32Matrix4Unit;
	}
	else {
		this->pTiedActor->FillThisFrameExpectedDifferentialMatrix(pMatrix);
	}
	return;
}

int CActor::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return *param_5 == 0;
}

void CActor::TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* param_5)
{
	CActor* pCurrentTiedActor;
	CCollision* pCVar2;
	CCollision* pCVar3;
	bool bVar4;
	uint uVar5;
	Timer* pTVar6;

	ACTOR_LOG(LogLevel::Info, "CActor::TieToActor {} -> {}", this->name, pTieActor ? pTieActor->name : "None");

	pCurrentTiedActor = this->pTiedActor;
	if (pCurrentTiedActor == pTieActor) {
		if (param_4 == 1) {
			this->flags = this->flags | 0x10000;
		}
		if (param_4 != -1) {
			this->flags = this->flags & 0xfffdffff;
		}
	}
	else {
		if (((pCurrentTiedActor == (CActor*)0x0) || ((this->flags & 0x10000) == 0)) || (param_4 == 1)) {
			pCVar2 = this->pCollisionData;
			if ((((pCurrentTiedActor != (CActor*)0x0) && ((pCurrentTiedActor->flags & 4) != 0)) &&
				((param_4 != 1 && (pCVar2 != (CCollision*)0x0)))) &&
				((pTieActor == (CActor*)0x0 || (pCurrentTiedActor != pTieActor->pTiedActor)))) {
				pCVar3 = pCurrentTiedActor->pCollisionData;
				uVar5 = pCVar3->flags_0x0;
				if (((uVar5 & 0x80) != 0) &&
					(((uVar5 & 0x200000) == 0 &&
						(uVar5 = CCollision::IsVertexAboveAndAgainstObbTree(&pCVar2->highestVertex, pCVar3->pObbTree), uVar5 != 0))))
				{
					this->flags = this->flags & 0xfffeffff;
					return;
				}
			}

			pCurrentTiedActor = this->pTiedActor;

			if ((pCurrentTiedActor != (CActor*)0x0) && (((pTieActor != (CActor*)0x0 || ((this->flags & 0x20000) == 0)) || (param_4 == -1)))) {
				pCurrentTiedActor->pCollisionData->UnregisterTiedActor(this);
				this->pTiedActor = (CActor*)0x0;
				this->flags = this->flags & 0xfffeffff;
				this->flags = this->flags & 0xfffdffff;
				bVar4 = this->IsKindOfObject(OBJ_TYPE_AUTONOMOUS);
				if (bVar4 != false) {
					pTVar6 = GetTimer();
					CActorAutonomous* pAutonomous = reinterpret_cast<CActorAutonomous*>(this);
					(pAutonomous->dynamicExt).scaledTotalTime = pTVar6->scaledTotalTime;
				}
			}

			if (pTieActor != (CActor*)0x0) {
	
				pTieActor->pCollisionData->RegisterTiedActor(pTieActor, this, carryMethod);
				this->pTiedActor = pTieActor;
				this->flags = this->flags | 0x40000;
				if (param_4 == 1) {
					this->flags = this->flags | 0x10000;
					this->flags = this->flags & 0xfffdffff;
				}
				else {
					this->flags = this->flags & 0xfffeffff;
					if (param_4 == -1) {
						this->flags = this->flags | 0x20000;
					}
				}
			}
		}
	}
	return;
}

int CActor::ReceiveEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* pEventData)
{
	bool bVar1;
	int eventType;
	CBehaviour* pCVar3;
	CWayPoint* pCVar4;
	long lVar6;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	undefined4 local_a0[2];
	undefined4 local_98;
	int local_94;
	edF32VECTOR4 local_80;
	float local_70;
	undefined4* local_14;
	int local_10;
	int local_8;
	int local_4;

	ACTOR_LOG(LogLevel::Info, "CActor::ReceiveEvent {} {}", this->name, param_4);

	eventType = pEventData[0];

	ACTOR_LOG(LogLevel::Info, "CActor::ReceiveEvent {} type: {} param: {}", this->name, eventType, pEventData[1]);

	if (eventType == 0x16) {
		switch (pEventData[1]) {
		case 0:
			local_a0[0] = 0;
			local_98 = 0;
			break;
		case 1:
		case 2:
		case 5:
			IMPLEMENTATION_GUARD(
			if (pEventData[1] == 1) {
				local_a0[0] = 1;
			}
			else {
				if ((this->pCollisionData != (CCollision*)0x0) && ((this->pCollisionData->flags_0x4 & 2) == 0)) {
					return 1;
				}
				local_a0[0] = 2;
			}
			local_98 = 1;
			eventType = piVar5[3];
			if (eventType == -1) {
				local_80.x = (this->rotationQuat).x;
				local_80.y = (this->rotationQuat).y;
				local_80.z = (this->rotationQuat).z;
				local_80.w = (this->rotationQuat).w;
			}
			else {
				pCVar4 = (CWayPoint*)0x0;
				local_b0.x = fRam00000000;
				if (eventType != -1) {
					pCVar4 = (CScene::ptable.g_CWayPointManager_0045169c)->aWaypoints + eventType;
					local_b0.x = (pCVar4->field_0x0).x;
				}
				local_b0.y = (pCVar4->field_0x0).y;
				local_b0.z = (pCVar4->field_0x0).z;
				local_b0.w = 1.0;
				edF32Vector4SubHard(&local_80, &this->currentLocation, &local_b0);
				local_80.y = 0.0;
				edF32Vector4SafeNormalize1Hard(&local_80, &local_80);
			}
			local_70 = (float)piVar5[4];
			if (local_70 == 0.0) {
				local_70 = 200.0;
			})
			break;
		case 3:
			IMPLEMENTATION_GUARD(
			local_a0[0] = 10;
			local_98 = 1;
			eventType = piVar5[3];
			if (eventType == -1) {
				edF32Vector4ScaleHard(-1.0f, &local_80, &this->rotationQuat);
			}
			else {
				pCVar4 = (CWayPoint*)0x0;
				local_c0.x = fRam00000000;
				if (eventType != -1) {
					pCVar4 = (CScene::ptable.g_CWayPointManager_0045169c)->aWaypoints + eventType;
					local_c0.x = (pCVar4->field_0x0).x;
				}
				local_c0.y = (pCVar4->field_0x0).y;
				local_c0.z = (pCVar4->field_0x0).z;
				local_c0.w = 1.0;
				edF32Vector4SubHard(&local_80, &local_c0, &this->currentLocation);
				local_80.y = 0.0;
				edF32Vector4SafeNormalize1Hard(&local_80, &local_80);
			}
			local_70 = (float)piVar5[4];
			if (local_70 == 0.0) {
				local_70 = 700.0;
			}
			local_80.y = 0.707;
			edF32Vector4NormalizeHard(&local_80, &local_80);)
			break;
		case 4:
			local_98 = 0;
			local_a0[0] = 6;
		}
		local_14 = local_a0;
		local_94 = pEventData[2];
		if ((this != (CActor*)0x0) && ((this->flags & 0x2000000) == 0)) {
			eventType = ReceiveMessage(this, (ACTOR_MESSAGE)2, (MSG_PARAM)local_14);
			return eventType;
		}
		return 0;
	}

	if (eventType == 0x15) {
		if (pEventData[1] != 0x7f) {
			local_10 = pEventData[2];
			if ((this != (CActor*)0x0) && ((this->flags & 0x2000000) == 0)) {
				eventType = ReceiveMessage(this, (ACTOR_MESSAGE)pEventData[1], (MSG_PARAM)local_10);
				return eventType;
			}
			return 0;
		}
	}
	else {
		if (eventType == 0x14) {
			if ((this != (CActor*)0x0) && ((this->flags & 0x2000000) == 0)) {
				eventType = ReceiveMessage(this, (ACTOR_MESSAGE)0xe, (MSG_PARAM)pEventData[1]);
				return eventType;
			}
			return 0;
		}

		if (eventType == 0x13) {
			if ((this != (CActor*)0x0) && ((this->flags & 0x2000000) == 0)) {
				local_8 = pEventData[1];
				eventType = ReceiveMessage(this, (ACTOR_MESSAGE)0x10, (MSG_PARAM)pEventData[1]);
				return eventType;
			}
			return 0;
		}

		if (eventType == 0x12) {
			if ((this != (CActor*)0x0) && ((this->flags & 0x2000000) == 0)) {
				local_4 = pEventData[1];
				eventType = ReceiveMessage(this, (ACTOR_MESSAGE)0xf, (MSG_PARAM)pEventData[1]);
				return eventType;
			}
			return 0;
		}
	}

	lVar6 = 0;
	pCVar3 = GetBehaviour(this->curBehaviourId);

	if (pCVar3 != (CBehaviour*)0x0) {
		lVar6 = pCVar3->InterpretEvent(pEventMessage, param_3, param_4, pEventData);
	}

	if (lVar6 == 0) {
		lVar6 = this->InterpretEvent(pEventMessage, param_3, param_4, pEventData);
	}

	return (int)lVar6;
}

void CActor::SetLocalBoundingSphere(float radius, edF32VECTOR4* pLocation)
{
	KyaUpdateObjA* pKVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 local_10;

	pKVar1 = this->subObjA;
	(pKVar1->boundingSphere).xyz = pLocation->xyz;
	(this->subObjA->boundingSphere).w = radius;

	pKVar1 = this->subObjA;
	local_10.xyz = (pKVar1->boundingSphere).xyz;
	local_10.w = 1.0f;
	edF32Matrix4MulF32Vector4Hard(&local_10, &this->pMeshTransform->base.transformA, &local_10);
	(this->sphereCentre).xyz = local_10.xyz;
	(this->sphereCentre).w = (pKVar1->boundingSphere).w * std::max(this->scale.z, std::max(this->scale.y, this->scale.x));
	return;
}

void CActor::CinematicMode_InterpolateTo(CCineActorConfig* pConfig, edF32MATRIX4* param_3, edF32MATRIX4* param_4)
{
	IMPLEMENTATION_GUARD();
}

bool CActor::ColWithAToboggan()
{
	CCollision* pColData;
	uint uVar2;
	bool bColWithAToboggan;

	pColData = this->pCollisionData;
	bColWithAToboggan = false;
	if ((pColData != (CCollision*)0x0) && (bColWithAToboggan = (pColData->flags_0x4 & 2) != 0, bColWithAToboggan)) {
		uVar2 = pColData->aCollisionContact[1].materialFlags & 0xf;
		if (uVar2 == 0) {
			uVar2 = CScene::_pinstance->defaultMaterialIndex;
		}
		bColWithAToboggan = uVar2 == 5;
	}
	return bColWithAToboggan;
}

bool CActor::ColWithCactus()
{
	CCollision* pCVar1;
	bool bVar2;
	uint uVar3;

	pCVar1 = this->pCollisionData;
	bVar2 = false;
	if ((pCVar1 != (CCollision*)0x0) && (bVar2 = (pCVar1->flags_0x4 & 7) != 0, bVar2)) {
		uVar3 = pCVar1->aCollisionContact[1].materialFlags & 0xf;
		if (uVar3 == 0) {
			uVar3 = CScene::_pinstance->defaultMaterialIndex;
		}
		bVar2 = uVar3 == 7;
		if (!bVar2) {
			uVar3 = pCVar1->aCollisionContact[0].materialFlags & 0xf;
			if (uVar3 == 0) {
				uVar3 = CScene::_pinstance->defaultMaterialIndex;
			}
			bVar2 = uVar3 == 7;
		}
		if (!bVar2) {
			uVar3 = pCVar1->aCollisionContact[2].materialFlags & 0xf;
			if (uVar3 == 0) {
				uVar3 = CScene::_pinstance->defaultMaterialIndex;
			}
			bVar2 = uVar3 == 7;
		}
	}
	return bVar2;
}

bool CActor::ColWithLava()
{
	CCollision* pCVar1;
	bool bVar2;
	uint uVar3;

	pCVar1 = this->pCollisionData;
	bVar2 = false;
	if ((pCVar1 != (CCollision*)0x0) && (bVar2 = (pCVar1->flags_0x4 & 7) != 0, bVar2)) {
		uVar3 = pCVar1->aCollisionContact[1].materialFlags & 0xf;
		if (uVar3 == 0) {
			uVar3 = CScene::_pinstance->defaultMaterialIndex;
		}
		bVar2 = uVar3 == 3;
		if (!bVar2) {
			uVar3 = pCVar1->aCollisionContact[0].materialFlags & 0xf;
			if (uVar3 == 0) {
				uVar3 = CScene::_pinstance->defaultMaterialIndex;
			}
			bVar2 = uVar3 == 3;
		}
		if (!bVar2) {
			uVar3 = pCVar1->aCollisionContact[2].materialFlags & 0xf;
			if (uVar3 == 0) {
				uVar3 = CScene::_pinstance->defaultMaterialIndex;
			}
			bVar2 = uVar3 == 3;
		}
	}
	return bVar2;
}

bool CActor::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	edF32Matrix4MulF32Vector4Hard(&local_10, m0, &this->rotationQuat);
	local_10.y = 0.0f;
	edF32Vector4SafeNormalize1Hard(&local_10, &local_10);
	this->rotationQuat = local_10;
	GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
	edF32Matrix4MulF32Vector4Hard(&eStack32, m0, &this->currentLocation);
	UpdatePosition(&eStack32, true);

	this->flags = this->flags & 0xffdfffff;
	return true;
}

CPlayerInput* CActor::GetInputManager(int, int)
{
	return &gPlayerInput;
}

bool CActor::PlayWaitingAnimation(float param_1, float param_2, int specialAnimType, int regularAnimType, byte idleLoopsToPlay)
{
	edAnmLayer* peVar1;
	bool bVar2;
	StateConfig* pAVar3;
	CAnimation* pAnimation;

	pAnimation = this->pAnimationController;
	if ((pAnimation != (CAnimation*)0x0) && (specialAnimType != -1)) {
		if (this->numIdleLoops == 0) {
			if (param_1 < this->idleTimer) {
				/* Play special idle (or shift feet) */
				pAnimation->anmBinMetaAnimator.SetLayerTimeWarper(param_2, 0);
				this->numIdleLoops = idleLoopsToPlay;
				PlayAnim(specialAnimType);
			}
		}
		else {
			peVar1 = (pAnimation->anmBinMetaAnimator).aAnimData;
			bVar2 = false;
			if ((peVar1->currentAnimDesc).animType == ((edAnmMetaAnimator*)&pAnimation->currentAnimType_0x30)->layerCount) {
				if (peVar1->animPlayState == STATE_ANIM_NONE) {
					bVar2 = false;
				}
				else {
					bVar2 = (peVar1->field_0xcc & 2) != 0;
				}
			}
			if ((bVar2) && (this->numIdleLoops = this->numIdleLoops - 1, (char)this->numIdleLoops < '\x01')) {
				pAnimation->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
				if (regularAnimType == -1) {
					if (this->prevAnimType == -1) {
						if (this->actorState != -1) {
							pAVar3 = this->GetStateCfg(this->actorState);
							if (pAVar3->animId != -1) {
								/* Not hit when standing still */
								PlayAnim(pAVar3->animId);
							}
						}
					}
					else {
						/* Return to previous non special animation */
						PlayAnim(this->prevAnimType);
					}
				}
				else {
					/* Play new regular idle (feet have shifted from special anim) */
					PlayAnim(regularAnimType);
				}
				this->idleTimer = 0.0f;
				this->numIdleLoops = 0;
				return true;
			}
		}
	}
	return false;
}

bool CActor::SV_UpdateOrientation(float param_1, edF32VECTOR4* pOrientation)
{
	bool bSuccess;
	Timer* pTVar2;
	float puVar3;
	float puVar4;
	float fVar3;
	float t0;
	edF32VECTOR4 eStack96;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 localRotation;

	localRotation = this->rotationQuat;

	puVar3 = edF32Vector4DotProductHard(&localRotation, pOrientation);

	if (1.0f < puVar3) {
		puVar4 = 1.0f;
	}
	else {
		puVar4 = -1.0f;
		if (-1.0 <= puVar3) {
			puVar4 = puVar3;
		}
	}

	fVar3 = acosf(puVar4);
	pTVar2 = GetTimer();

	t0 = param_1 * pTVar2->cutsceneDeltaTime;

	if ((t0 <= 0.0f) || (fVar3 <= t0)) {
		bSuccess = true;
		localRotation = *pOrientation;
	}
	else {
		edF32Vector4CrossProductHard(&eStack96, &localRotation, pOrientation);
		edF32Matrix4BuildFromVectorAndAngle(t0, &eStack80, &eStack96);
		edF32Matrix4MulF32Vector4Hard(&localRotation, &eStack80, &localRotation);
		edF32Vector4NormalizeHard(&localRotation, &localRotation);
		bSuccess = false;
	}

	this->rotationQuat = localRotation;
	GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);

	return bSuccess;
}

bool CActor::SV_UpdateOrientation2D(float speed, edF32VECTOR4* pNewOrientation, int mode)
{
	bool bSuccess;
	Timer* pTVar1;
	float puVar3;
	float puVar2;
	float fVar3;
	float fVar4;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	bSuccess = false;
	if (speed == 0.0f) {
		bSuccess = false;
	}
	else {
		local_10.x = (this->rotationQuat).x;
		local_10.y = 0.0f;
		local_10.z = (this->rotationQuat).z;
		local_10.w = 0.0f;

		edF32Vector4NormalizeHard(&local_10, &local_10);

		local_20.x = pNewOrientation->x;
		local_20.y = 0.0f;
		local_20.z = pNewOrientation->z;
		local_20.w = 0.0f;

		edF32Vector4NormalizeHard(&local_20, &local_20);

		pTVar1 = GetTimer();
		fVar4 = speed * pTVar1->cutsceneDeltaTime;
		puVar3 = edF32Vector4DotProductHard(&local_10, &local_20);

		if (1.0f < puVar3) {
			puVar2 = 1.0f;
		}
		else {
			puVar2 = -1.0f;
			if (-1.0 <= puVar3) {
				puVar2 = puVar3;
			}
		}

		fVar3 = acosf(puVar2);
		if (fVar4 < fVar3) {
			if (mode == 0) {
				if (0.0f < local_10.x * local_20.z - local_20.x * local_10.z) {
					fVar4 = -fVar4;
				}
			}
			else {
				if (mode == 2) {
					fVar4 = -fVar4;
				}
			}

			edF32Matrix4RotateYHard(fVar4, &eStack96, &gF32Matrix4Unit);
			edF32Matrix4MulF32Vector4Hard(&local_10, &eStack96, &local_10);
			edF32Vector4NormalizeHard(&local_10, &local_10);

			this->rotationQuat = local_10;
		}
		else {
			bSuccess = true;
			this->rotationQuat = local_20;
		}

		fVar4 = GetAngleYFromVector(&this->rotationQuat);
		(this->rotationEuler).y = fVar4;

	}
	return bSuccess;
}

void CActor::SV_GetGroundPosition(edF32VECTOR4* v0)
{
	CCollision* pCol;

	pCol = this->pCollisionData;
	if (pCol == (CCollision*)0x0) {
		*v0 = this->currentLocation;
	}
	else {
		*v0 = pCol->highestVertex;
	}

	v0->y = v0->y - this->distanceToGround;

	return;
}

void CActor::SV_GetActorColCenter(edF32VECTOR4* pColCenter)
{
	if (this->pCollisionData == (CCollision*)0x0) {
		*pColCenter = this->currentLocation;
	}
	else {
		edF32Vector4AddHard(pColCenter, &this->currentLocation, &this->pCollisionData->pObbPrim->field_0xb0);
		pColCenter->w = 1.0f;
	}

	return;
}

float CActor::SV_GetCosAngle2D(edF32VECTOR4* pToLocation)
{
	float fVar1;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, pToLocation, &this->currentLocation);
	eStack16.y = 0.0f;
	edF32Vector4NormalizeHard(&eStack16, &eStack16);

	if ((this->rotationQuat).y == 0.0f) {
		fVar1 = edF32Vector4DotProductHard(&this->rotationQuat, &eStack16);
	}
	else {
		local_20.x = (this->rotationQuat).x;
		local_20.y = 0.0f;
		local_20.z = (this->rotationQuat).z;
		local_20.w = 0.0f;
		edF32Vector4NormalizeHard(&local_20, &local_20);

		fVar1 = edF32Vector4DotProductHard(&this->rotationQuat, &local_20);
	}

	return fVar1;
}

bool CActor::SV_Vector4SLERP(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	bool ret;
	Timer* pTVar1;
	float puVar2;
	float puVar5;
	float fVar2;
	float puVar3;
	float puVar4;
	float t0;
	edF32VECTOR4 eStack80;
	edF32MATRIX4 eStack64;

	puVar2 = edF32Vector4DotProductHard(param_3, param_4);

	if (1.0f < puVar2) {
		puVar5 = 1.0f;
	}
	else {
		puVar5 = -1.0f;
		if (-1.0f <= puVar2) {
			puVar5 = puVar2;
		}
	}

	fVar2 = acosf(puVar5);
	pTVar1 = GetTimer();

	t0 = param_1 * pTVar1->cutsceneDeltaTime;
	if ((t0 <= 0.0f) || (fVar2 <= t0)) {
		ret = true;
		*param_3 = *param_4;
	}
	else {
		edF32Vector4CrossProductHard(&eStack80, param_3, param_4);
		edF32Matrix4BuildFromVectorAndAngle(t0, &eStack64, &eStack80);
		edF32Matrix4MulF32Vector4Hard(param_3, &eStack64, param_3);
		edF32Vector4NormalizeHard(param_3, param_3);
		ret = false;
	}

	return ret;
}

void CActor::SV_GetBoneDefaultWorldPosition(uint boneId, edF32VECTOR4* pOutPosition)
{
	edF32MATRIX4 eStack16;
	this->pAnimationController->GetDefaultBoneMatrix(boneId, &eStack16);
	edF32Matrix4MulF32Vector4Hard(pOutPosition, &this->pMeshTransform->base.transformA, &eStack16.rowT);

	return;
}

void CActor::SV_GetBoneWorldPosition(int boneIndex, edF32VECTOR4* pOutPosition)
{
	edF32MATRIX4* peVar1;
	peVar1 = this->pAnimationController->GetCurBoneMatrix(boneIndex);
	*pOutPosition = peVar1->rowT;
	edF32Matrix4MulF32Vector4Hard(pOutPosition, &this->pMeshTransform->base.transformA, pOutPosition);
	return;
}

void CActor::SV_UpdatePosition_Rel(edF32VECTOR4* pPosition, int param_3, int param_4, CActorsTable* pActorsTable, edF32VECTOR4* param_6)
{
	edF32MATRIX4 local_40;

	ACTOR_LOG(LogLevel::VeryVerbose, "CActor::SV_UpdatePosition_Rel {} pPosition: {}", this->name, pPosition->ToString());

	if (this->pTiedActor == (CActor*)0x0) {
		if (param_3 != 0) {
			if (param_6 != (edF32VECTOR4*)0x0) {
				edF32Vector4AddHard(pPosition, pPosition, param_6);
			}
			if ((param_4 == 0) || (this->pCollisionData == (CCollision*)0x0)) {
				UpdatePosition(pPosition, true);
			}
			else {
				this->pCollisionData->CheckCollisions_MoveActor(this, pPosition, pActorsTable, 0, 1);
			}
		}
	}
	else {
		if ((param_3 != 0) || ((this->flags & 0x40000) != 0)) {
			edF32Matrix4FromEulerSoft(&local_40, &this->pCinData->rotationEuler, "XYZ");
			local_40.rowT = *pPosition;
			SV_UpdateMatrix_Rel(&local_40, param_3, param_4, pActorsTable, param_6);
		}
	}
	return;
}

void CActor::SV_ComputeDiffMatrixFromInit(edF32MATRIX4* m0)
{
	S_CARRY_ACTOR_ENTRY* peVar1;
	float fVar2;
	int iVar3;
	edF32MATRIX4* peVar4;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 eStack64;

	if ((this->pCollisionData == (CCollision*)0x0) ||
		(peVar1 = this->pCollisionData->pCarryActorEntry, peVar4 = &peVar1->m1, peVar1 == (S_CARRY_ACTOR_ENTRY*)0x0)) {
		edF32Matrix4FromEulerSoft(&eStack64, &this->pCinData->rotationEuler, "XYZ");
		eStack64.rowT = this->baseLocation;

		edF32Matrix4GetInverseOrthoHard(&eStack64, &eStack64);
		edF32Matrix4FromEulerSoft(&eStack128, &this->rotationEuler.xyz, "XYZ");
		eStack128.rowT = this->currentLocation;
		edF32Matrix4MulF32Matrix4Hard(m0, &eStack64, &eStack128);
	}
	else {
		*m0 = *peVar4;
	}

	return;
}


void CActor::SV_UpdatePercent(float target, float speed, float* pValue)
{
	Timer* pTimer;
	float fVar2;
	float fVar3;

	pTimer = GetTimer();
	fVar2 = powf(speed, pTimer->cutsceneDeltaTime * 50.0f);

	fVar3 = 1.0f;

	if ((fVar2 <= 1.0f) && (fVar3 = fVar2, fVar2 < 0.0f)) {
		fVar3 = 0.0f;
	}

	*pValue = target * (1.0f - fVar3) + *pValue * fVar3;
	return;
}

bool CActor::SV_UpdateValue(float target, float speed, float* pValue)
{
	bool bVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;

	if (*pValue == target) {
		bVar1 = false;
	}
	else {
		pTVar2 = GetTimer();
		fVar4 = *pValue;
		fVar3 = speed * pTVar2->cutsceneDeltaTime;
		if (fVar4 < target) {
			fVar4 = fVar4 + fVar3;
			*pValue = fVar4;
			if (target < fVar4) {
				*pValue = target;
			}
		}
		else {
			fVar4 = fVar4 - fVar3;
			*pValue = fVar4;
			if (fVar4 <= target) {
				*pValue = target;
			}
		}
		bVar1 = *pValue == target;
	}
	return bVar1;
}

void CActor::LinkToActor(CActor* pLinkedActor, uint key, int param_4)
{
	CActorManager* pActorManager;
	_linked_actor* pData;

	pActorManager = CScene::ptable.g_ActorManager_004516a4;
	CScene::ptable.g_ActorManager_004516a4->FindLinkedActor(this);
	pData = pActorManager->AddLinkedActor();

	if (pData != (_linked_actor*)0x0) {
		pData->pActor = this;
		pData->pLinkedActor = pLinkedActor;
		pData->key = key;
		pData->field_0xc = param_4;
		pLinkedActor->pAnimationController->RegisterBone(key);
	}

	return;
}

void CActor::UnlinkFromActor()
{
	CActorManager* pActorManager;
	_linked_actor* pActor;

	pActorManager = CScene::ptable.g_ActorManager_004516a4;
	pActor = CScene::ptable.g_ActorManager_004516a4->FindLinkedActor(this);

	if (pActor != (_linked_actor*)0x0) {
		if ((this->flags & 0x1000) == 0) {
			(this->rotationEuler).z = 0.0f;
		}

		pActor->pLinkedActor->pAnimationController->UnRegisterBone(pActor->key);
		pActorManager->RemoveLinkedActor(pActor);
	}
	return;
}

void CActor::SetAlpha(byte alpha)
{
	if (this->pMeshNode != (edNODE*)0x0) {
		ed3DHierarchyNodeSetAlpha(this->pMeshNode, alpha);
	}

	return;
}

void CActor::ToggleMeshAlpha()
{
	if (this->pMeshNode != (edNODE*)0x0) {
		ed3DHierarchyNodeSetAlpha(this->pMeshNode, 0x80);
	}
	return;
}

void CActor::SetBFCulling(byte bActive)
{
	if (this->pMeshNode != (edNODE*)0x0) {
		ed3DHierarchyNodeSetBFCulling(this->pMeshNode, bActive);
	}
	return;
}

void CActor::PauseChange(int bIsPaused)
{
	CActorSound* pCVar1;
	CBehaviour* pCVar2;

	IMPLEMENTATION_GUARD_AUDIO(
	if (bIsPaused == 0) {
		for (pCVar1 = this->aActorSounds; pCVar1 != (CActorSound*)0x0; pCVar1 = (CActorSound*)pCVar1[1].field_0x0) {
			CActorSound::ResumeSounds((int)pCVar1);
		}
	}
	else {
		for (pCVar1 = this->aActorSounds; pCVar1 != (CActorSound*)0x0; pCVar1 = (CActorSound*)pCVar1[1].field_0x0) {
			CActorSounds::PauseSounds((int)pCVar1);
		}
	})

	if (this->pAnimationController != (CAnimation*)0x0) {
		this->pAnimationController->PauseChange(bIsPaused);
	}

	pCVar2 = GetBehaviour(this->curBehaviourId);
	if (pCVar2 != (CBehaviour*)0x0) {
		pCVar2->PauseChange(bIsPaused);
	}

	return;
}

uint CActor::GetStateFlags(int state)
{
	if (state == -1) {
		return 0;
	}
	else {
		return this->GetStateCfg(state)->flags_0x4;
	}
}

void CActor::UpdateClusterNode()
{
	if (this->pClusterNode != (CClusterNode*)0x0) {
		this->pClusterNode->Update(&(CScene::ptable.g_ActorManager_004516a4)->cluster);
	}

	return;
}

void CActor::UpdateShadow(edF32VECTOR4* pLocation, int bInAir, ushort param_4)
{
	CCollision* pCVar1;
	edF32VECTOR4* peVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	CShadow* pShad;

	if (bInAir == 0) {
		this->flags = this->flags & 0xfeffffff;
	}
	else {
		this->flags = this->flags | 0x1000000;
	}

	this->field_0xf4 = param_4;
	/* Set our ground distance to zero */
	this->distanceToGround = 0.0f;

	pShad = this->pShadow;
	if (pShad != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		pCVar1 = this->pCollisionData;
		if ((pCVar1 == (CCollision*)0x0) || (peVar2 = &pCVar1->highestVertex, (pCVar1->flags_0x0 & 0x40000) == 0)) {
			peVar2 = &this->currentLocation;
		}
		fVar5 = peVar2->y;
		fVar3 = peVar2->z;
		fVar4 = peVar2->w;
		(pShad->base).field_0x10.x = peVar2->x;
		(pShad->base).field_0x10.y = fVar5;
		(pShad->base).field_0x10.z = fVar3;
		(pShad->base).field_0x10.w = fVar4;
		(pShad->base).field_0x4c = (this->rotationEuler).y;
		CShadowShared::SetIntensity(1.0, pShad);
		fVar5 = pLocation->y;
		fVar3 = pLocation->z;
		fVar4 = pLocation->w;
		(pShad->base).field_0x20.x = pLocation->x;
		(pShad->base).field_0x20.y = fVar5;
		(pShad->base).field_0x20.z = fVar3;
		(pShad->base).field_0x20.w = fVar4;)
	}

	this->flags = this->flags | 0x200000;
	return;
}

CActor* CActor::GetCollidingActor()
{
	CActor* pCVar1;

	if (this->pCollisionData == (CCollision*)0x0) {
		pCVar1 = (CActor*)0x0;
	}
	else {
		pCVar1 = this->pCollisionData->actorField;
	}
	return pCVar1;
}

void CActor::SV_UpdateMatrix_Rel(edF32MATRIX4* m0, int param_3, int bCheckCollisions, CActorsTable* pActorsTable, edF32VECTOR4* v0)
{
	edF32VECTOR3 local_10;

	ACTOR_LOG(LogLevel::Verbose, "CActor::SV_UpdateMatrix_Rel {} {}", this->name, m0->rowT.ToString());

	if (strcmp(this->name, "TELEPORTER_LVL_DOOR_L1") == 0) {
		ACTOR_LOG(LogLevel::Verbose, "CActor::SV_UpdateMatrix_Rel {} {}", this->name, m0->rowT.ToString());
	}

	if ((this->pTiedActor != (CActor*)0x0) && ((param_3 != 0 || ((this->flags & 0x40000) != 0)))) {
		this->flags = this->flags & 0xfffbffff;
		SV_InheritMatrixFromTiedToActor(m0);
		param_3 = 1;
	}

	if (param_3 != 0) {
		if (v0 != (edF32VECTOR4*)0x0) {
			edF32Vector4AddHard(&m0->rowT, &m0->rowT, v0);
		}

		edF32Matrix4ToEulerSoft(m0, &local_10, "XYZ");
		(this->rotationEuler).xyz = local_10;

		SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);

		if ((bCheckCollisions == 0) || (this->pCollisionData == (CCollision*)0x0)) {
			UpdatePosition(m0, 1);
		}
		else {
			this->pCollisionData->CheckCollisions_MoveActor(this, m0, pActorsTable, 0, 1);
		}
	}
	return;
}

void CActor::SV_InheritMatrixFromTiedToActor(edF32MATRIX4* m0)
{
	CActor* pCVar1;
	S_CARRY_ACTOR_ENTRY* pSVar2;
	uint uVar3;
	CActor* pCVar4;
	int iVar5;
	edF32MATRIX4* peVar6;
	edF32MATRIX4* peVar7;
	float fVar8;
	edF32MATRIX4 eStack224;
	edF32MATRIX4 eStack160;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 local_40;

	pCVar4 = this->pTiedActor->pCollisionData->FindTiedActor(this);
	pCVar1 = this->pTiedActor;

	if ((pCVar1->pCollisionData == (CCollision*)0x0) || (pSVar2 = pCVar1->pCollisionData->pCarryActorEntry, peVar7 = &pSVar2->m1, pSVar2 == (S_CARRY_ACTOR_ENTRY*)0x0)) {
		edF32Matrix4FromEulerSoft(&eStack160, &pCVar1->pCinData->rotationEuler, "XYZ");
		eStack160.rowT = pCVar1->baseLocation;

		edF32Matrix4GetInverseOrthoHard(&eStack160, &eStack160);
		edF32Matrix4FromEulerSoft(&eStack224, &pCVar1->rotationEuler.xyz, "XYZ");

		eStack224.rowT = pCVar1->currentLocation;

		edF32Matrix4MulF32Matrix4Hard(&local_40, &eStack160, &eStack224);
	}
	else {
		local_40 = *peVar7;
	}

	uVar3 = pCVar4->actorFieldS;
	if (uVar3 != 0) {
		if (uVar3 == 3) {
			local_60 = m0->rowT;
			edF32Matrix4MulF32Matrix4Hard(m0, m0, &local_40);
			m0->rowT = local_60;
			return;
		}

		if (uVar3 == 1) {
			edF32Matrix4MulF32Vector4Hard(&local_50, &local_40, &m0->rowT);
			fVar8 = GetAngleYFromVector(&local_40.rowZ);
			edF32Matrix4RotateYHard(fVar8, m0, m0);
			m0->rowT = local_50;
			return;
		}

		if (uVar3 == 2) {
			edF32Matrix4MulF32Vector4Hard(&m0->rowT, &local_40, &m0->rowT);
			return;
		}
	}

	edF32Matrix4MulF32Matrix4Hard(m0, m0, &local_40);
	return;
}

bool CActor::SV_AmICarrying(CActor* pOther)
{
	CActor* pCVar1;

	for (pCVar1 = pOther->pTiedActor; (pCVar1 != (CActor*)0x0 && (this != pCVar1)); pCVar1 = pCVar1->pTiedActor) {
	}
	return this == pCVar1;
}

int CActor::SV_UpdateMatrixOnTrajectory_Rel(float param_1, CPathFollowReaderAbsolute* pPathFollowReaderAbs, int param_4, int param_5, CActorsTable* pActorsTable, edF32MATRIX4* pMatrix, edF32VECTOR4* param_8, S_PATHREADER_POS_INFO* pPathReaderPosInfo)
{
	CCollision* pCVar1;
	uint* iVar2;
	uint uVar3;
	int iVar4;
	uint uVar5;
	edF32MATRIX4 auStack96;
	edF32VECTOR4 local_20;
	S_PATHREADER_POS_INFO SStack16;

	if (pPathFollowReaderAbs->pActor3C_0x0 == (CPathFollow*)0x0) {
		iVar4 = 2;
		if (pMatrix == (edF32MATRIX4*)0x0) {
			local_20 = this->baseLocation;
			SV_UpdatePosition_Rel(&local_20, 0, param_5, pActorsTable, param_8);
		}
		else {
			edF32Matrix4FromEulerSoft(&auStack96, &this->pCinData->rotationEuler, "XYZ");
			auStack96.rowT = this->baseLocation;
			edF32Matrix4MulF32Matrix4Hard(&auStack96, pMatrix, &auStack96);
			SV_UpdateMatrix_Rel(&auStack96, 1, param_5, pActorsTable, param_8);
		}

		if (pPathReaderPosInfo != (S_PATHREADER_POS_INFO*)0x0) {
			pPathReaderPosInfo->field_0x0 = 0;
			pPathReaderPosInfo->field_0x4 = 0;
			pPathReaderPosInfo->field_0x8 = 0.0f;
		}
	}
	else {
		if (pPathReaderPosInfo == (S_PATHREADER_POS_INFO*)0x0) {
			pPathReaderPosInfo = &SStack16;
		}

		if (param_4 == 0) {
			if (pMatrix == (edF32MATRIX4*)0x0) {
				iVar4 = pPathFollowReaderAbs->ComputePosition(param_1, &local_20, (edF32VECTOR4*)0x0, pPathReaderPosInfo);
				SV_UpdatePosition_Rel(&local_20, 1, param_5, pActorsTable, param_8);
			}
			else {
				edF32Matrix4FromEulerSoft(&auStack96, &this->pCinData->rotationEuler, "XYZ");
				iVar4 = pPathFollowReaderAbs->ComputePosition(param_1, &auStack96.rowT, (edF32VECTOR4*)0x0, pPathReaderPosInfo);
				edF32Matrix4MulF32Matrix4Hard(&auStack96, pMatrix, &auStack96);
				SV_UpdateMatrix_Rel(&auStack96, 1, param_5, pActorsTable, param_8);
			}
		}
		else {
			iVar4 = pPathFollowReaderAbs->ComputeMatrix(param_1, &auStack96, 0, pPathReaderPosInfo);

			if (pMatrix != (edF32MATRIX4*)0x0) {
				edF32Matrix4MulF32Matrix4Hard(&auStack96, pMatrix, &auStack96);
			}

			SV_UpdateMatrix_Rel(&auStack96, 1, param_5, pActorsTable, param_8);
		}

		pCVar1 = this->pCollisionData;

		if (pCVar1 != (CCollision*)0x0) {
	
			iVar2 = pPathFollowReaderAbs->pActor3C_0x0->field_0x30;
			if (iVar2 == (uint*)0x0) {
				uVar5 = 0;
			}
			else {
				uVar5 = iVar2[pPathReaderPosInfo->field_0x4];
			}

			uVar3 = pPathFollowReaderAbs->pActor3C_0x0->field_0x18;

			if ((uVar3 & 2) != 0) {
				if ((uVar5 & 2) == 0) {
					pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffdfffff;
				}
				else {
					pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x200000;
				}
			}

			if ((uVar3 & 4) != 0) {
				if ((uVar5 & 4) == 0) {
					pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x100000;
				}
				else {
					pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffefffff;
				}
			}
		}
	}

	return iVar4;
}

bool CActor::UpdateNormal(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	return SV_Vector4SLERP(param_1, param_3, param_4);
}

void CActor::SV_Blend3AnimationsWith2Ratios(float r1, float r2, edAnmMacroBlendN* param_3, uint param_4, uint param_5, uint param_6)
{
	float* piVar1;
	float fVar1;

	fVar1 = (1.0f - r2) - r1;
	piVar1 = param_3->pHdr->pData + param_3->pHdr->keyIndex_0x8.asKey;
	piVar1[param_4] = fVar1;
	if (fVar1 < 0.0f) {
		piVar1[param_4] = 0.0f;
	}
	piVar1[param_5] = r1;
	piVar1[param_6] = r2;
}

void CActor::SV_Blend4AnimationsWith2Ratios(float r1, float r2, edAnmMacroBlendN* param_3, uint param_4, uint param_5, uint param_6, uint param_7)
{
	float* piVar1;

	piVar1 = param_3->pHdr->pData + param_3->pHdr->keyIndex_0x8.asKey;
	piVar1[param_4] = ((1.0f - r1) * (1.0f - r2));
	piVar1[param_5] = (r1 * (1.0f - r2));
	piVar1[param_6] = ((1.0f - r1) * r2);
	piVar1[param_7] = (r1 * r2);
}

void CActor::SV_SetOrientationToPosition2D(edF32VECTOR4* pPosition)
{
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, pPosition, &this->currentLocation);
	eStack16.y = 0.0f;
	edF32Vector4NormalizeHard(&this->rotationQuat, &eStack16);
	return;
}

CAddOn::CAddOn()
{
	this->pOwner = (CActor*)0x0;
	this->pSubObj = 0;

	return;
}

void CAddOn::Init(CActor* pActor)
{
	this->pOwner = pActor;
	this->pSubObj = (CAddOnSubObj*)0x0;

	this->field_0xc = 0;
	this->field_0xd = 1;

	return;
}

void CAddOn::Reset()
{
	CAddOnSubObj* pCVar1;

	pCVar1 = this->pSubObj;

	if (pCVar1 != (CAddOnSubObj*)0x0) {
		pCVar1->pCinematic = (CCinematic*)0x0;
		pCVar1->lastPlayedCinematicId = -1;
		pCVar1->field_0x14 = 0.0f;
	}

	this->pSubObj = (CAddOnSubObj*)0x0;
	this->field_0xc = 0;
	this->field_0xd = 1;

	return;
}

void CAddOn::Manage()
{
	CAddOnSubObj* pSub;
	bool bVar2;
	CCinematic* pCinematic;

	if (GetCinematic() == (CCinematic*)0x0) {
		return;
	}

	if ((GameFlags & 0x4020) == 0) goto LAB_003e3558;

	pCinematic = GetCinematic();

	if (pCinematic == (CCinematic*)0x0) {
	LAB_003e34e0:
		bVar2 = false;
	}
	else {
		pCinematic = GetCinematic();

		if ((pCinematic->state == CS_Stopped) || (bVar2 = true, this->field_0xc == 0)) goto LAB_003e34e0;
	}

	if (bVar2) {
		GetCinematic()->FUN_001c92b0();

		bVar2 = GetCinematic()->Has_0x2d8();
		if (bVar2 != false) {
			GetCinematic()->Remove_0x2d8();
		}

		Reset();
	}
LAB_003e3558:
	if (this->field_0xc != 0) {
		if (GetCinematic()->state == CS_Stopped) {
			if ((GetCinematic()->flags_0x8 & 0x80) == 0) {
				this->field_0xc = 0;
				if (this->pSubObj != (CAddOnSubObj*)0x0) {
					this->pSubObj->SetCinematic((CCinematic*)0x0);
				}

				this->pSubObj = (CAddOnSubObj*)0x0;
			}
		}
	}

	return;
}

bool CAddOn::Func_0x34(uint param_2, CActor* pActor)
{
	return Func_0x24(param_2, pActor);
}

CCinematic* CAddOn::GetCinematic()
{
	CCinematic* pCinematic = (CCinematic*)0x0;
	if (this->pSubObj != (CAddOnSubObj*)0x0) {
		pCinematic = this->pSubObj->pCinematic;
	}

	return pCinematic;
}

void CAddOnSubObj::SetCinematic(CCinematic* pCinematic)
{
	bool bVar1;

	if ((this->pCinematic != (CCinematic*)0x0) && (pCinematic == (CCinematic*)0x0)) {
		bVar1 = this->pCinematic->Has_0x2d8();
		if (bVar1 != false) {
			this->pCinematic->Remove_0x2d8();
		}

		this->field_0x14 = 0.0f;
	}

	if ((this->pCinematic == (CCinematic*)0x0) && (pCinematic != (CCinematic*)0x0)) {
		bVar1 = pCinematic->Has_0x2d8();
		if (bVar1 == false) {
			pCinematic->Add_0x2d8();
		}

		this->field_0x14 = 0.0f;
	}

	if (((this->pCinematic != (CCinematic*)0x0) && (pCinematic != (CCinematic*)0x0)) && (this->pCinematic != pCinematic)) {
		bVar1 = this->pCinematic->Has_0x2d8();
		if (bVar1 != false) {
			this->pCinematic->Remove_0x2d8();
		}

		bVar1 = pCinematic->Has_0x2d8();
		if (bVar1 == false) {
			pCinematic->Add_0x2d8();
		}

		this->field_0x14 = 0.0f;
	}

	this->pCinematic = pCinematic;

	return;
}

int CAddOnSubObj::PickCinematic()
{
	bool bVar1;
	int iVar5;
	int nbValidCinematics;
	int cinematicIndex;

	cinematicIndex = 0;
	nbValidCinematics = 0;
	bVar1 = false;
	if (0 < this->nbCinematics) {
		do {
			if (g_CinematicManager_0048efc->GetCinematic(this->aCinematicIds[cinematicIndex]) != (CCinematic*)0x0) {
				if (g_CinematicManager_0048efc->GetCinematic(this->aCinematicIds[cinematicIndex])->CanBePlayed() != false) {
					nbValidCinematics = nbValidCinematics + 1;
				}
			}

			cinematicIndex = cinematicIndex + 1;
		} while (cinematicIndex < this->nbCinematics);
	}

	// If there are multiple valid cinematics, we check if the last played cinematic should be skipped.
	if (((1 < nbValidCinematics) && (this->lastPlayedCinematicId != -1)) && (g_CinematicManager_0048efc->GetCinematic(this->aCinematicIds[this->lastPlayedCinematicId]) != (CCinematic*)0x0)) {
		if (g_CinematicManager_0048efc->GetCinematic(this->aCinematicIds[lastPlayedCinematicId])->CanBePlayed() != false) {
			nbValidCinematics = nbValidCinematics + -1;
			bVar1 = true;
		}
	}

	if (0 < nbValidCinematics) {
		nbValidCinematics = nbValidCinematics * CScene::Rand();

		if (nbValidCinematics < 0) {
			nbValidCinematics = nbValidCinematics + 0x7fff;
		}

		iVar5 = 0;
		cinematicIndex = 0;
		if (0 < this->nbCinematics) {
			do {
				if (((!bVar1) || (cinematicIndex != this->lastPlayedCinematicId)) && (g_CinematicManager_0048efc->GetCinematic(this->aCinematicIds[cinematicIndex]) != (CCinematic*)0x0)) {
					if (g_CinematicManager_0048efc->GetCinematic(this->aCinematicIds[cinematicIndex])->CanBePlayed() != false) {
						iVar5 = iVar5 + 1;
					}
				}

				if (iVar5 + -1 == nbValidCinematics >> 0xf) {
					const int cinematicId = this->aCinematicIds[cinematicIndex];
					SetCinematic(g_CinematicManager_0048efc->GetCinematic(cinematicId));
					this->lastPlayedCinematicId = cinematicIndex;
					return cinematicId;
				}

				cinematicIndex = cinematicIndex + 1;
			} while (cinematicIndex < this->nbCinematics);
		}
	}

	SetCinematic((CCinematic*)0x0);

	this->lastPlayedCinematicId = -1;

	return -1;
}

bool CActorsTable::IsInList(CActor* pActor)
{
	CActor** piVar1;
	int iVar2;

	piVar1 = this->aEntries;
	iVar2 = 0;
	if (0 < this->nbEntries) {
		do {
			if (pActor == (*piVar1)) {
				return true;
			}

			iVar2 = iVar2 + 1;
			piVar1 = piVar1 + 1;
		} while (iVar2 < this->nbEntries);
	}

	return false;
}

// temlate specialization for is in list but with int
bool CActorsTable::IsInList(int typeId)
{
	CActor** piVar1;
	int iVar2;

	piVar1 = this->aEntries;
	iVar2 = 0;
	if (0 < this->nbEntries) {
		do {
			if (typeId == (*piVar1)->typeID) {
				return true;
			}

			iVar2 = iVar2 + 1;
			piVar1 = piVar1 + 1;
		} while (iVar2 < this->nbEntries);
	}

	return false;
}

CActor* CActorsTable::Remove(CActor* pActor)
{
	int totalEntries;
	bool bFound;
	CActor* pRemovedActor;
	CActor** ppCVar4;
	CActor** pCVar5;
	int actorIndex;

	totalEntries = this->nbEntries;
	actorIndex = 0;
	pRemovedActor = (CActor*)0x0;
	pCVar5 = this->aEntries;
	while (true) {
		bFound = false;
		if ((actorIndex < totalEntries) && (*pCVar5 != pActor)) {
			bFound = true;
		}

		if (!bFound) break;

		pCVar5 = pCVar5 + 1;
		actorIndex = actorIndex + 1;
	}

	if (actorIndex < totalEntries) {
		ppCVar4 = this->aEntries + actorIndex + -1;
		pRemovedActor = ppCVar4[1];
		if (actorIndex < totalEntries + -1) {
			do {
				actorIndex = actorIndex + 1;
				ppCVar4[1] = ppCVar4[2];
				ppCVar4 = ppCVar4 + 1;
			} while (actorIndex < this->nbEntries + -1);
		}

		this->nbEntries = this->nbEntries + -1;
	}

	return pRemovedActor;
}

void CActorsTable::SortByClassPriority()
{
	int iVar1;
	CActor** ppCVar2;
	CActor** pCVar3;
	int iVar4;
	int iVar5;

	iVar1 = this->nbEntries;
	iVar4 = 0;
	pCVar3 = this->aEntries;
	if (0 < iVar1 + -1) {
		do {
			iVar5 = iVar4 + 1;
			if (iVar5 < iVar1) {
				ppCVar2 = this->aEntries + iVar4;
				do {
					if (CActorFactory::gClassProperties[(*pCVar3)->typeID].classPriority <
						CActorFactory::gClassProperties[ppCVar2[1]->typeID].classPriority) {
						Swap(iVar4, iVar5);
					}

					iVar1 = this->nbEntries;
					iVar5 = iVar5 + 1;
					ppCVar2 = ppCVar2 + 1;
				} while (iVar5 < iVar1);
			}
			iVar4 = iVar4 + 1;
			pCVar3 = pCVar3 + 1;
		} while (iVar4 < iVar1 + -1);
	}

	return;
}

void CActorsTable::Swap(int a, int b)
{
	CActor* tmp;
	// Access the element at position 'a' and store it in tmp
	tmp = this->aEntries[a];
	// Swap the values of elements at positions 'a' and 'b'
	this->aEntries[a] = this->aEntries[b];
	this->aEntries[b] = tmp;
}

void CBehaviourInactive::Create(ByteCode* pByteCode)
{
	this->activateMsgId = pByteCode->GetS32();
	this->flags = pByteCode->GetU32();
	this->activeBehaviourId = pByteCode->GetS32();

	return;
}

void CBehaviourInactive::Init(CActor* pOwner)
{
	this->pOwner = pOwner;

	return;
}

void CBehaviourInactive::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActor* pCVar1;
	CinNamedObject30* pCVar2;
	float fVar3;
	float fVar4;
	CCollision* pCol;

	if ((this->flags & 1) != 0) {
		pCol = this->pOwner->pCollisionData;
		if (pCol != (CCollision*)0x0) {
			pCol->Reset();
		}

		this->pOwner->UpdatePosition(&this->pOwner->baseLocation, true);
	}

	if ((this->flags & 2) != 0) {
		pCVar1 = this->pOwner;
		pCVar2 = pCVar1->pCinData;
		pCVar1->rotationEuler.xyz = pCVar2->rotationEuler;
	}

	pCVar1 = this->pOwner;
	pCVar1->flags = pCVar1->flags & 0xfffffffd;
	pCVar1->flags = pCVar1->flags | 1;
	pCVar1 = this->pOwner;
	pCVar1->flags = pCVar1->flags & 0xffffff7f;
	pCVar1->flags = pCVar1->flags | 0x20;
	pCVar1->EvaluateDisplayState();

	return;
}

void CBehaviourInactive::End(int newBehaviourId)
{
	CActor* pOwn;

	this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
	pOwn = this->pOwner;
	pOwn->flags = pOwn->flags & 0xffffff5f;
	pOwn->EvaluateDisplayState();

	return;
}

int CBehaviourInactive::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;

	bVar1 = msg == this->activateMsgId;
	if (bVar1) {
		this->pOwner->SetBehaviour(this->activeBehaviourId, -1, -1);
	}

	return bVar1;
}

CActInstance::CActInstance()
{
	this->field_0x7c = -1.0f;
	this->field_0x84 = 0;
	this->field_0x88 = 0;

	return;
}

void CActInstance::SetState(int newState)
{
	this->state = newState;

	this->field_0x5c = 0.0f;

	if (this->state == 1) {
		this->flags = this->flags | 0x10;
	}

	if (this->state == 3) {
		this->field_0x54 = 0.0f;

		this->field_0x40 = this->currentPosition;
	}
	return;
}

void CActInstance::CheckpointReset()
{
	return;
}

void CActInstance::Init(CActor* pOwner, edF32VECTOR4* pPosition, edF32VECTOR4* pBoundSphere, int param_5)
{
	ed_3d_hierarchy* peVar1;
	ed_g3d_manager* pG3D;
	edNODE* peVar2;
	ed_3D_Scene* pScene;
	int textureIndex;
	float fVar3;
	float fVar4;
	float fVar5;

	this->pOwner = pOwner;

	this->basePosition = *pPosition;

	this->field_0x60 = param_5;
	this->field_0x5c = 0.0f;
	this->field_0x58 = 0.0f;

	this->flags = 1;

	if (pOwner->p3DHierNode != (ed_3d_hierarchy_node*)0x0) {
		textureIndex = pOwner->pCinData->textureIndex;
		if (textureIndex == -1) {
			textureIndex = CScene::_pinstance->field_0x28;
		}

		pG3D = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(pOwner->pCinData->meshIndex, textureIndex);

		this->flags = this->flags | 0x80;
		peVar2 = ed3DHierarchyAddToScene(CScene::_scene_handleA, pG3D, (char*)0x0);
		this->pNode = peVar2;
		this->pHierarchy = (ed_3d_hierarchy*)this->pNode->pData;
		ed3DHierarchySetSetup(this->pHierarchy, &pOwner->hierarchySetup);
		this->pHierarchy->pHierarchySetup->pBoundingSphere = pBoundSphere;
	}

	this->currentPosition = this->basePosition;

	if ((this->flags & 0x80) != 0) {
		this->pHierarchy->transformA.rowT = this->currentPosition;
	}

	this->field_0x64 = this->basePosition.xyz;
	this->field_0x70 = gF32Vector3Zero;

	this->flags = this->flags & 0xfffffffb;

	if ((this->flags & 0x80) != 0) {
		pScene = GetStaticMeshMasterA_001031b0();
		ed3DHierarchyNodeSetRenderOn(pScene, this->pNode);
	}

	return;
}

void CActInstance::SetVisible(int bVisible)
{
	ed_3D_Scene* peVar1;

	if (bVisible == 0) {
		this->flags = this->flags & 0xfffffffb;
	}
	else {
		this->flags = this->flags | 4;
	}

	if ((this->flags & 0x80) != 0) {
		if (bVisible == 1) {
			peVar1 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOff(peVar1, this->pNode);
		}
		else {
			peVar1 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOn(peVar1, this->pNode);
		}
	}

	return;
}

void CActInstance::Reset()
{
	this->currentPosition = this->basePosition;

	if ((this->flags & 0x80) != 0) {
		this->pHierarchy->transformA.rowT = this->currentPosition;
	}

	this->field_0x64 = this->basePosition.xyz;
	this->field_0x70 = gF32Vector3Zero;

	this->field_0x5c = 0.0f;
	this->field_0x58 = 0.0f;
	this->flags = this->flags | 1;
	
	SetState(1);

	return;
}

void CActInstance::SetAlive(int bAlive)
{
	ed_3D_Scene* pScene;

	if (bAlive == 0) {
		this->flags = this->flags & 0xfffffffe;
		this->flags = this->flags & 0xfffffffb;

		if ((this->flags & 0x80) != 0) {
			pScene = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOn(pScene, this->pNode);
		}
	}
	else {
		this->flags = this->flags | 1;
	}

	return;
}

float CActInstance::DistSquared(edF32VECTOR4* pPosition)
{
	float fVar1;
	float fVar2;
	float fVar3;

	fVar1 = (this->currentPosition).x - pPosition->x;
	fVar2 = (this->currentPosition).y - pPosition->y;
	fVar3 = (this->currentPosition).z - pPosition->z;
	return fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3;
}

void CActInstance::State_GotoKim()
{
	ed_3d_hierarchy* peVar1;
	CActorHero* pCVar2;
	edF32VECTOR4* peVar3;
	Timer* pTVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	pCVar2 = CActorHero::_gThis;
	peVar3 = CActorHero::_gThis->GetPosition_00369c80();

	local_10.x = peVar3->x;
	local_10.z = peVar3->z;
	local_10.w = peVar3->w;
	local_10.y = peVar3->y + pCVar2->subObjA->boundingSphere.w;

	edF32Vector4LERPHard(this->field_0x54, &local_20, &this->field_0x40, &local_10);
	local_20.w = 1.0f;

	(this->currentPosition).xyz = local_20.xyz;
	(this->currentPosition).w = 1.0f;

	if ((this->flags & 0x80) != 0) {
		(this->pHierarchy->transformA).rowT = this->currentPosition;
	}

	this->field_0x54 = this->field_0x54 + GetTimer()->cutsceneDeltaTime / 0.25f;
	edF32Vector4SubHard(&eStack48, &local_10, &this->currentPosition);
	fVar5 = edF32Vector4GetDistHard(&eStack48);
	if ((1.0f <= this->field_0x54) || (fVar5 <= 0.25f)) {
		SetState(4);
	}
	return;
}

void CActInstance::State_Wait()
{
	CActorHero* pCVar1;
	edF32VECTOR4* v1;
	int iVar2;
	long lVar3;
	float fVar4;
	edF32VECTOR4 eStack32;
	undefined4 local_4;

	pCVar1 = CActorHero::_gThis;
	v1 = CActorHero::_gThis->GetPosition_00369c80();
	edF32Vector4SubHard(&eStack32, v1, &this->currentPosition);
	eStack32.y = eStack32.y + pCVar1->subObjA->boundingSphere.w;
	fVar4 = edF32Vector4GetDistHard(&eStack32);
	this->field_0x58 = fVar4;
	lVar3 = 7;
	if (this->field_0x58 <= 0.85f) {
		lVar3 = 4;
	}
	else {
		if (this->field_0x58 <= 2.0f) {
			lVar3 = 3;
		}
	}

	if (lVar3 != 7) {
		if ((this->flags & 8) == 0) {
			SetState(lVar3);
		}
		else {
			local_4 = 0xffffffff;
			iVar2 = this->pOwner->DoMessage(CActorHero::_gThis, (ACTOR_MESSAGE)9, (void*)0xffffffff);
			if (iVar2 == 0) {
				IMPLEMENTATION_GUARD(
				CActorHero::_gThis->magicInterface.FUN_001dcda0(this->field_0x58);)
			}
			else {
				SetState(lVar3);
			}
		}
	}
	return;
}

void CActInstance::FUN_003982c0()
{
	bool bVar1;
	bool bVar2;
	ed_3D_Scene* peVar3;

	if (((this->flags & 1) == 0) && ((this->flags & 4) != 0)) {
		this->flags = this->flags & 0xfffffffb;
		if ((this->flags & 0x80) != 0) {
			peVar3 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOn(peVar3, this->pNode);
		}
	}
	else {
		bVar2 = CCameraManager::_gThis->InFrustum(0.35f, this->pOwner->subObjA->field_0x1c, &this->currentPosition);
		bVar1 = (this->flags & 4) != 0;

		if ((bVar1) || (bVar2 == false)) {
			if (((bVar1) && (bVar2 == false)) && (this->flags = this->flags & 0xfffffffb, (this->flags & 0x80) != 0)) {
				peVar3 = GetStaticMeshMasterA_001031b0();
				ed3DHierarchyNodeSetRenderOn(peVar3, this->pNode);
			}
		}
		else {
			this->flags = this->flags | 4;

			if ((this->flags & 0x80) != 0) {
				peVar3 = GetStaticMeshMasterA_001031b0();
				ed3DHierarchyNodeSetRenderOff(peVar3, this->pNode);
			}
		}
	}

	return;
}

void CActInstance::Term()
{
	if ((this->flags & 0x80) != 0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pNode);
	}
	this->pNode = (edNODE*)0x0;

	return;
}


