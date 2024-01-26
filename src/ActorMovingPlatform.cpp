#include "ActorMovingPlatform.h"
#include "MemoryStream.h"

void CActorMovingPlatform::Create(ByteCode* pByteCode)
{
	char* pcVar1;
	uint* puVar2;
	int iVar3;
	uint uVar4;
	int* piVar5;
	S_STREAM_MPF_NO_FRICTION_ZONE* pSVar6;
	int iVar7;
	CBehaviour* pCVar8;
	ulong uVar9;
	int iVar10;
	int iVar11;
	float fVar12;

	CActorMovable::Create(pByteCode);
	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 8;
	}
	//this->field_0x244 = piVar5;
	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 4;
	}
	//this->field_0x248 = piVar5;
	uVar4 = pByteCode->GetS32();
	this->noFrictionZoneCount = uVar4;
	this->aNoFrictionZones = (S_STREAM_MPF_NO_FRICTION_ZONE*)0x0;
	uVar4 = this->noFrictionZoneCount;
	if (uVar4 != 0) {
		this->aNoFrictionZones = new S_STREAM_MPF_NO_FRICTION_ZONE[uVar4];
		pSVar6 = this->aNoFrictionZones;
		iVar11 = 0;
		if (0 < (int)this->noFrictionZoneCount) {
			do {
				iVar7 = pByteCode->GetS32();
				pSVar6->field_0x0 = iVar7;
				piVar5 = (int*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos = (char*)(piVar5 + 1);
				if (*piVar5 != 0) {
					pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 4;
				}
				pSVar6->field_0x4 = (char*)piVar5;
				iVar11 = iVar11 + 1;
				pSVar6 = pSVar6 + 1;
			} while (iVar11 < (int)this->noFrictionZoneCount);
		}
	}
	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 0x48;
	}
	//this->field_0x254 = piVar5;
	pcVar1 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar1 + 0x4c;
	//this->field_0x1d0 = pcVar1;
	//this->field_0x1d8 = 0;
	//this->field_0x1ec = 0;
	//this->field_0x1f0 = 0;
	//this->field_0x1f4 = 0;
	//this->field_0x1f8 = 0;
	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 0x10;
	}
	//this->field_0x1dc = piVar5;
	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 0x10;
	}
#if 0
	this->field_0x1e0 = piVar5;
	pcVar1 = this->field_0x1d0;
	fVar12 = *(float*)(pcVar1 + 0x14);
	*(undefined4*)(pcVar1 + 0x14) = *(undefined4*)(pcVar1 + 0x18);
	*(float*)(this->field_0x1d0 + 0x18) = -fVar12;
	puVar2 = *(uint**)&this->field_0x2c;
	if (puVar2 != (uint*)0x0) {
		*puVar2 = *puVar2 & 0xffffffc4;
		*puVar2 = *puVar2 | 0x1800480;
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x40) != 0) {
			*puVar2 = *puVar2 | 0x100;
		}
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x400000) != 0) {
			*puVar2 = *puVar2 | 0x200;
		}
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x4000) != 0) {
			*puVar2 = *puVar2 & 0xffefffff;
		}
		uVar9 = 0;
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x80) != 0) {
			uVar9 = 0x400;
		}
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x100) != 0) {
			uVar9 = uVar9 | 0x800;
		}
		if (uVar9 != 0) {
			CCollision::PatchObbTreeFlagsRecurse((CollisionDataSubObj_176*)puVar2[4], uVar9, 0xffffffffffffffff, 0);
		}
	}
	if (this->field_0x254 == (int*)0x0) {
		iVar11 = 0;
	}
	else {
		iVar11 = *this->field_0x254;
	}
	iVar7 = 0;
	if (0 < iVar11) {
		iVar10 = 0;
		do {
			iVar3 = *(int*)((int)this->field_0x254 + iVar10 + 4);
			if (iVar3 == 0xe) {
				this->field_0x1d8 = this->field_0x1d8 | 8;
			}
			else {
				if ((iVar3 == 0xc) || (iVar3 == 0xb)) {
					this->field_0x1d8 = this->field_0x1d8 | 4;
				}
			}
			iVar7 = iVar7 + 1;
			iVar10 = iVar10 + 0x48;
		} while (iVar7 < iVar11);
	}
	if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x1000) != 0) {
		this->field_0x1d8 = this->field_0x1d8 | 4;
	}
	if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x400) != 0) {
		this->field_0x1d8 = this->field_0x1d8 | 8;
	}
	uVar9 = 0;
	iVar7 = 0;
	if (0 < iVar11) {
		piVar5 = this->field_0x254;
		do {
			if (piVar5[0x12] != -1) {
				uVar9 = SEXT48((int)uVar9 + 1);
			}
			iVar7 = iVar7 + 1;
			piVar5 = piVar5 + 0x12;
		} while (iVar7 < iVar11);
	}
	if (3 < uVar9) {
		uVar9 = 3;
	}
	piVar5 = (int*)this->field_0x1d0;
	iVar11 = *piVar5;
	if (iVar11 == -1) {
		iVar11 = piVar5[1];
	}
	if (iVar11 == -1) {
		iVar11 = piVar5[2];
		if (iVar11 == -1) {
			iVar11 = piVar5[3];
		}
		if (iVar11 == -1) goto LAB_0015d240;
	}
	this->field_0x1d8 = this->field_0x1d8 | 1;
LAB_0015d240:
	pCVar8 = CActor::GetBehaviour((CActor*)this, 7);
	if ((pCVar8 != (CBehaviour*)0x0) &&
		(pCVar8 = CActor::GetBehaviour((CActor*)this, 7), pCVar8[2].pVTable != (CBehaviourVtable*)0xffffffff)) {
		this->field_0x1d8 = this->field_0x1d8 | 1;
	}
	pCVar8 = CActor::GetBehaviour((CActor*)this, 3);
	if ((pCVar8 != (CBehaviour*)0x0) &&
		(pCVar8 = CActor::GetBehaviour((CActor*)this, 3), pCVar8[8].pVTable != (CBehaviourVtable*)0xffffffff)) {
		this->field_0x1d8 = this->field_0x1d8 | 2;
	}
	pCVar8 = CActor::GetBehaviour((CActor*)this, 8);
	if ((pCVar8 != (CBehaviour*)0x0) || (pCVar8 = CActor::GetBehaviour((CActor*)this, 9), pCVar8 != (CBehaviour*)0x0)) {
		pCVar8 = CActor::GetBehaviour((CActor*)this, 8);
		if (pCVar8 == (CBehaviour*)0x0) {
			pCVar8 = CActor::GetBehaviour((CActor*)this, 9);
		}
		if (pCVar8[5].pVTable != (CBehaviourVtable*)0xffffffff) {
			this->field_0x1d8 = this->field_0x1d8 | 2;
		}
	}
	if ((this->field_0x1d8 & 2) != 0) {
		uVar9 = SEXT48((int)uVar9 + 1);
	}
	if ((this->field_0x1d8 & 1) != 0) {
		uVar9 = SEXT48((int)uVar9 + 1);
	}
	if (uVar9 == 0) {
		this->field_0x240 = (int*)0x0;
	}
	else {
		piVar5 = CActor::CreateActorSound((CActor*)this, uVar9);
		this->field_0x240 = piVar5;
	}
	this->field_0x1e4 = 0;
	this->field_0x1e8 = 0;
#endif
	return;
}

template<typename PlatformBehaviourType>
struct PlatformHeader
{
	PlatformBehaviourType* aPlatforms;
	PlatformHeader* pPrev;
	int totalEntries;
	int inUseEntries;

	inline static PlatformBehaviourType* Get(PlatformHeader** pObj) {
		PlatformBehaviourType* pBehaviour;

		bool bRequiresNewHeader = true;

		// Create the header if we don't already have one.
		if (((*pObj) != (PlatformHeader*)0x0) &&
			((*pObj)->inUseEntries + 1 <= (*pObj)->totalEntries)) {
			bRequiresNewHeader = false;
		}

		PlatformHeader* pPrevHeader;
		PlatformHeader* pCurHeader = (*pObj);

		// If the header is new or needs extending, create some new behaviours.
		if ((bRequiresNewHeader) && (pCurHeader = new PlatformHeader, pPrevHeader = (*pObj), pCurHeader != (PlatformHeader*)0x0)) {
			pCurHeader->totalEntries = 0x20;
			pCurHeader->aPlatforms = new PlatformBehaviourType[pCurHeader->totalEntries];
			pCurHeader->pPrev = pPrevHeader;
			pCurHeader->inUseEntries = 0;
		}

		(*pObj) = pCurHeader;

		if ((*pObj)->totalEntries < (*pObj)->inUseEntries + 1) {
			pBehaviour = (PlatformBehaviourType*)0x0;
		}
		else {
			pBehaviour = &(*pObj)->aPlatforms[(*pObj)->inUseEntries];
			(*pObj)->inUseEntries++;
		}

		return pBehaviour;
	}
};

PlatformHeader<CBehaviourPlatformTrajectory>* gPlatform_00448e14;
PlatformHeader<CBehaviourPlatformSlab>* gPlatform_00448e18;
PlatformHeader<CBehaviourPlatformDestroyed>* gPlatform_00448e1c;
PlatformHeader<CBehaviourPlatformStand>* gPlatform_00448e20;
PlatformHeader<CBehaviourSelectorNew>* gPlatform_00448e24;
PlatformHeader<CBehaviourSelectorSlave>* gPlatform_00448e28;

CBehaviour* CActorMovingPlatform::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType < 0xb) {
		switch (behaviourType) {
		case 0x0:
		case 0x1:
			pBehaviour = CActor::BuildBehaviour(behaviourType);
		break;
		case 0x2:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformTrajectory>::Get(&gPlatform_00448e14);
		}
		break;
		case 0x3:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformSlab>::Get(&gPlatform_00448e18);
		}
		break;
		case 0x4:
		{
			pBehaviour = new CBehaviourTeleportRandom;
		}
		break;
		case 0x5:
		{
			pBehaviour = new CBehaviourWeighingMachineSlave;
		}
		break;
		case 0x6:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformDestroyed>::Get(&gPlatform_00448e1c);
		}
		break;
		case 0x7:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformStand>::Get(&gPlatform_00448e20);
		}
		break;
		case 0x8:
		{
			pBehaviour = PlatformHeader<CBehaviourSelectorSlave>::Get(&gPlatform_00448e28);
		}
		break;
		case 0x9:
		{
			pBehaviour = PlatformHeader<CBehaviourSelectorNew>::Get(&gPlatform_00448e24);
		}
		break;
		default:
			assert(false);
		}
		return pBehaviour;
	}
	pBehaviour = CActor::BuildBehaviour(behaviourType);
	return pBehaviour;
}
