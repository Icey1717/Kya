#include "ActorCheckpointManager.h"
#include "MemoryStream.h"

void CActorCheckpointManager::Create(ByteCode* pByteCode)
{
	int iVar1;
	int* piVar2;
	S_CHECKPOINT* pSVar3;
	EventChunk_24* pEVar4;
	CWayPoint* pMVar5;
	int iVar6;
	uint uVar7;
	ActorAndWaypoint* pAVar8;
	int iVar9;
	int iVar10;

	CActor::Create(pByteCode);
	this->aCheckpoints = (S_CHECKPOINT*)0x0;
	//this->field_0x168 = -1;
	iVar1 = pByteCode->GetS32();
	this->checkpointCount = iVar1;
	uVar7 = this->checkpointCount;
	if (uVar7 != 0) {
		this->aCheckpoints = new S_CHECKPOINT[uVar7];
		pSVar3 = this->aCheckpoints;
		iVar1 = 0;
		if (0 < this->checkpointCount) {
			do {
				pEVar4 = (EventChunk_24*)pByteCode->GetS32();
				//pSVar3->pEventChunk24_0x0 = pEVar4;
				pMVar5 = (CWayPoint*)pByteCode->GetS32();
				//pSVar3->pManagerC24_0x4 = pMVar5;
				iVar6 = pByteCode->GetS32();
				//pSVar3->field_0x8 = iVar6;
				pMVar5 = (CWayPoint*)pByteCode->GetS32();
				//pSVar3->pManagerC24_0xc = pMVar5;
				uVar7 = pByteCode->GetU32();
				//pSVar3->flags_0x10 = uVar7;
				iVar6 = pByteCode->GetS32();
				pSVar3->actorWaypointsCount = iVar6;
				uVar7 = pSVar3->actorWaypointsCount;
				if (uVar7 == 0) {
					pSVar3->aActorWaypoints = (ActorAndWaypoint*)0x0;
				}
				else {
					pSVar3->aActorWaypoints = new ActorAndWaypoint[uVar7];
					iVar6 = 0;
					if (0 < pSVar3->actorWaypointsCount) {
						iVar10 = 0;
						do {
							pAVar8 = pSVar3->aActorWaypoints;
							iVar9 = pByteCode->GetS32();
							//*(int*)((int)&pAVar8->pActor + iVar10) = iVar9;
							pAVar8 = pSVar3->aActorWaypoints;
							iVar9 = pByteCode->GetS32();
							//*(int*)((int)&pAVar8->pWaypoint + iVar10) = iVar9;
							iVar6 = iVar6 + 1;
							iVar10 = iVar10 + 8;
						} while (iVar6 < pSVar3->actorWaypointsCount);
					}
				}
				iVar1 = iVar1 + 1;
				pSVar3 = pSVar3 + 1;
			} while (iVar1 < this->checkpointCount);
		}
	}
	return;
}
