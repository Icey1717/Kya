#include "Squad.h"
#include "MathOps.h"
#include "MemoryStream.h"
#include "ActorWolfen.h"
#include "ActorHero.h"

void CSquadConfig::Create(ByteCode* pByteCode)
{
	uint uVar2;

	uVar2 = 0;
	do {
		this->field_0x0[uVar2] = pByteCode->GetU32();
		uVar2 = uVar2 + 1;
	} while (uVar2 < 2);

	return;
}

CTeamElt::CTeamElt()
{
	this->field_0x0 = 2;
	this->field_0x4 = 2;
	this->field_0x14 = -1;

	return;
}

void CSquad::Create(ByteCode* pByteCode)
{
	uint uVar1;
	uint uVar2;
	float fVar3;
	float fVar4;

	fVar3 = pByteCode->GetF32();
	uVar1 = pByteCode->GetU32();
	uVar2 = pByteCode->GetU32();
	if ((int)uVar1 < 0) {
		fVar4 = (float)(uVar1 >> 1 | uVar1 & 1);
		fVar4 = fVar4 + fVar4;
	}
	else {
		fVar4 = (float)uVar1;
	}

	this->chessboard.Init(fVar3, fVar3 * fVar4, uVar1, uVar2);

	this->aSemaphores[0].field_0x0 = 0;
	this->aSemaphores[0].field_0x4 = 0.6f;
	this->aSemaphores[0].field_0xc = 0;
	this->aSemaphores[0].nbSemaphores = 0;
	memset(this->aSemaphores[0].aSemaphores, 0, sizeof(this->aSemaphores[0].aSemaphores));

	this->aSemaphores[1].field_0x0 = 1;
	this->aSemaphores[1].field_0x4 = 0.0f;
	this->aSemaphores[1].field_0xc = 0;
	this->aSemaphores[1].nbSemaphores = 0;
	memset(this->aSemaphores[1].aSemaphores, 0, sizeof(this->aSemaphores[1].aSemaphores));

	return;
}

void CSquad::Term()
{
	IMPLEMENTATION_GUARD();
}

void CSquad::Clear()
{
	int iVar1;
	uint uVar2;
	SquadSemaphore* pSemaphore;
	uint uVar4;
	uint uVar5;

	iVar1 = (this->eltTable).nbEntries;
	while (iVar1 != 0) {
		CActorWolfen* pWolfen = static_cast<CActorWolfen*>((this->eltTable).aEntries[iVar1 - 1]->pEnemyActor);
		pWolfen->TermFightAction();
		this->eltTable.PopCurrent();
		iVar1 = (this->eltTable).nbEntries;
	}

	SquadSemaphoreManager* pSemaphoreManager = this->aSemaphores;

	uVar5 = 0;
	do {
		uVar4 = 0;
		pSemaphore = pSemaphoreManager->aSemaphores;

		if (pSemaphoreManager->nbSemaphores != 0) {
			do {
				uVar2 = pSemaphore->flags & 1;
				if (((uVar2 != 0) && (uVar4 != 0xffffffff)) && (uVar2 != 0)) {
					CActorWolfen* pWolfen = pSemaphore->pWolfen;
					if (pWolfen != (CActorWolfen*)0x0) {
						IMPLEMENTATION_GUARD(
							pWolfen->DisableFightAction();)
					}

					pWolfen = pSemaphore->pWolfen;
					if (pWolfen != (CActorWolfen*)0x0) {
						pSemaphore->field_0x4 = pWolfen;
					}

					if ((pSemaphore->flags & 5) != 0) {
						pSemaphoreManager->field_0xc = pSemaphoreManager->field_0xc + 1;
					}

					pSemaphore->flags = 0;
					pSemaphore->duration = 0.0f;
					pSemaphore->pWolfen = (CActorWolfen*)0x0;
					pSemaphore->field_0x8 = 0;
				}

				CActorWolfen* pWolfen = pSemaphore->pWolfen;
				if (pWolfen != (CActorWolfen*)0x0) {
					pSemaphore->field_0x4 = pWolfen;
				}

				if ((pSemaphore->flags & 5) != 0) {
					pSemaphoreManager->field_0xc = pSemaphoreManager->field_0xc + 1;
				}

				pSemaphore->flags = 0;
				pSemaphore->duration = 0.0f;
				uVar4 = uVar4 + 1;
				pSemaphore->pWolfen = (CActorWolfen*)0x0;
				pSemaphore->field_0x8 = 0;
				pSemaphore = pSemaphore + 1;
			} while (uVar4 < pSemaphoreManager->nbSemaphores);
		}

		uVar5 = uVar5 + 1;
		pSemaphoreManager = pSemaphoreManager + 1;
	} while (uVar5 < 2);

	return;
}

int CSquad::NbElt()
{
	return this->eltTable.nbEntries;
}

void CSquad::ClearSquadDeadAndNonFightingElt()
{
	CTeamElt* pEntry;
	CActorWolfen* pWolfen;
	CTeamElt** pEntries;
	int curIndex;

	curIndex = 0;
	pEntries = this->eltTable.aEntries;
	if (0 < (this->eltTable).nbEntries) {
		do {
			pEntry = *pEntries;
			if (pEntry != (CTeamElt*)0x0) {
				pWolfen = static_cast<CActorWolfen*>(pEntry->pEnemyActor);

				if ((pWolfen->GetLifeInterface()->GetValue() <= 0.0f) || (pWolfen->IsFightRelated(pWolfen->curBehaviourId) == false) || ((pWolfen->flags & 4) == 0)) {
					pWolfen->TermFightAction();
					this->eltTable.Pop(curIndex);
				}
			}

			curIndex = curIndex + 1;
			pEntries = pEntries + 1;
		} while (curIndex < (this->eltTable).nbEntries);
	}

	return;
}

void CSquad::RemoveFighter(CTeamElt* pTeamElt)
{
	int iVar1;
	CTeamElt** pCVar2;
	int iVar3;

	iVar1 = (this->eltTable).nbEntries;
	iVar3 = 0;
	pCVar2 = this->eltTable.aEntries;
	if (0 < iVar1) {
		do {
			if (*pCVar2 == pTeamElt) {
				CActorWolfen* pWolfen = static_cast<CActorWolfen*>(pTeamElt->pEnemyActor);
				pWolfen->TermFightAction();
				this->eltTable.Pop(iVar3);
				SynchronizePawns();

				return;
			}

			iVar3 = iVar3 + 1;
			pCVar2 = pCVar2 + 1;
		} while (iVar3 < iVar1);
	}

	return;
}

void CSquad::ComputeBoundingSphere(ed_Bound_Sphere* pBoundSphere)
{
	CTeamElt* pCVar1;
	s_chess_board_pawn* puVar2;
	int iVar4;
	s_chess_board_pawn** pCVar5;
	int iVar6;
	s_chess_board_pawn* psVar7;
	float z1;
	float z2;
	float y1;
	float y2;
	float x1;
	float x2;
	edF32VECTOR4 eStack736;
	edF32VECTOR4 local_2d0;
	edF32VECTOR4 local_2c0;
	edF32VECTOR4 local_2b0;
	s_chess_board_pawn asStack672[8];
	CChessBoardPawnsRefTable pawnTable;

	local_2b0.z = 1e+08f;
	local_2c0.z = -1e+08f;
	pawnTable.nbEntries = 0;
	local_2b0.y = local_2b0.z;
	local_2b0.x = local_2b0.z;
	this->chessboard.GetPawnsWithSameRingPos(&pawnTable, 1);
	iVar6 = 1;
	asStack672[0].pFighter = CActorHero::_gThis;

	iVar4 = 0;
	if (0 < (this->eltTable).nbEntries) {
		puVar2 = asStack672 + 1;
		CTeamElt** pCVar3 = this->eltTable.aEntries;
		do {
			pCVar1 = *pCVar3;
			if (pCVar1->field_0x14 != -1) {
				iVar6 = iVar6 + 1;
				puVar2->pFighter = pCVar1->pEnemyActor;
				puVar2 = puVar2 + 1;
			}

			iVar4 = iVar4 + 1;
			pCVar3 = pCVar3 + 1;
		} while (iVar4 < (this->eltTable).nbEntries);
	}

	iVar4 = 0;
	if (0 < iVar6) {
		psVar7 = asStack672;
		do {
			pawnTable.Add(psVar7);
			iVar4 = iVar4 + 1;
			psVar7 = psVar7 + 1;
		} while (iVar4 < iVar6);
	}

	iVar4 = 0;
	local_2c0.y = local_2c0.z;
	local_2c0.x = local_2c0.z;

	if (0 < pawnTable.nbEntries) {
		pCVar5 = pawnTable.aEntries;
		z1 = local_2b0.z;
		z2 = local_2c0.z;
		y1 = local_2b0.y;
		y2 = local_2c0.z;
		x1 = local_2b0.x;
		x2 = local_2c0.z;
		do {
			CActorFighter* pFighter = (*pCVar5)->pFighter;
			local_2d0 = pFighter->currentLocation;

			local_2b0.x = local_2d0.x;
			if (x1 <= local_2d0.x) {
				local_2b0.x = x1;
			}
			local_2c0.x = local_2d0.x;
			if (local_2d0.x <= x2) {
				local_2c0.x = x2;
			}

			local_2b0.y = local_2d0.y;
			if (y1 <= local_2d0.y) {
				local_2b0.y = y1;
			}
			local_2c0.y = local_2d0.y;
			if (local_2d0.y <= y2) {
				local_2c0.y = y2;
			}

			local_2b0.z = local_2d0.z;
			if (z1 <= local_2d0.z) {
				local_2b0.z = z1;
			}
			local_2c0.z = local_2d0.z;
			if (local_2d0.z <= z2) {
				local_2c0.z = z2;
			}

			iVar4 = iVar4 + 1;
			pCVar5 = pCVar5 + 1;
			z1 = local_2b0.z;
			z2 = local_2c0.z;

			y1 = local_2b0.y;
			y2 = local_2c0.y;

			x1 = local_2b0.x;
			x2 = local_2c0.x;
		} while (iVar4 < pawnTable.nbEntries);
	}

	local_2c0.x = local_2c0.x - local_2b0.x;
	local_2c0.w = 0.0f;
	local_2c0.y = local_2c0.y - local_2b0.y;
	local_2c0.z = local_2c0.z - local_2b0.z;

	local_2b0.w = 1.0f;
	edF32Vector4ScaleHard(0.5f, &eStack736, &local_2c0);
	edF32Vector4AddHard(&local_2b0, &local_2b0, &eStack736);

	pBoundSphere->xyz = local_2b0.xyz;

	pBoundSphere->radius = edF32Vector4GetDistHard(&local_2c0) * 0.5f;

	return;
}

void CSquad::InitSemaphores(CSquadConfig* pConfig)
{
	uint uVar1;
	CActorWolfen* pCVar2;
	uint uVar3;
	uint uVar5;
	uint uVar6;

	uVar6 = 0;
	do {
		uVar1 = pConfig->field_0x0[uVar6];
		uVar5 = 0;
		if (this->aSemaphores[uVar6].nbSemaphores != 0) {
			do {
				uVar3 = this->aSemaphores[uVar6].aSemaphores[uVar5].flags & 1;
				if (((uVar3 != 0) && (uVar5 != 0xffffffff)) && (uVar3 != 0)) {
					pCVar2 = this->aSemaphores[uVar6].aSemaphores[uVar5].pWolfen;
					if (pCVar2 != (CActorWolfen*)0x0) {
						IMPLEMENTATION_GUARD(
						CActorWolfen::DisableFightAction(pCVar2);)
					}

					pCVar2 = this->aSemaphores[uVar6].aSemaphores[uVar5].pWolfen;
					if (pCVar2 != (CActorWolfen*)0x0) {
						this->aSemaphores[uVar6].aSemaphores[uVar5].field_0x4 = pCVar2;
					}

					if ((this->aSemaphores[uVar6].aSemaphores[uVar5].flags & 5) != 0) {
						this->aSemaphores[uVar6].field_0xc = this->aSemaphores[uVar6].field_0xc + 1;
					}

					this->aSemaphores[uVar6].aSemaphores[uVar5].flags = 0;
					this->aSemaphores[uVar6].aSemaphores[uVar5].duration = 0.0f;
					this->aSemaphores[uVar6].aSemaphores[uVar5].pWolfen = (CActorWolfen*)0x0;
					this->aSemaphores[uVar6].aSemaphores[uVar5].field_0x8 = 0;
				}

				pCVar2 = this->aSemaphores[uVar6].aSemaphores[uVar5].pWolfen;
				if (pCVar2 != (CActorWolfen*)0x0) {
					this->aSemaphores[uVar6].aSemaphores[uVar5].field_0x4 = pCVar2;
				}

				if ((this->aSemaphores[uVar6].aSemaphores[uVar5].flags & 5) != 0) {
					this->aSemaphores[uVar6].field_0xc = this->aSemaphores[uVar6].field_0xc + 1;
				}

				this->aSemaphores[uVar6].aSemaphores[uVar5].flags = 0;
				this->aSemaphores[uVar6].aSemaphores[uVar5].duration = 0.0f;
				this->aSemaphores[uVar6].aSemaphores[uVar5].pWolfen = (CActorWolfen*)0x0;
				this->aSemaphores[uVar6].aSemaphores[uVar5].field_0x8 = 0;
				uVar5 = uVar5 + 1;
			} while (uVar5 < this->aSemaphores[uVar6].nbSemaphores);
		}

		this->aSemaphores[uVar6].field_0xc = uVar1;
		this->aSemaphores[uVar6].nbSemaphores = uVar1;
		uVar6 = uVar6 + 1;
	} while (uVar6 < 2);

	return;
}

void CSquad::SynchronizePattern(CChessBoardPawnsRefTable* pTable)
{
	bool bVar1;
	int iVar2;
	CActor* pActor;
	CBehaviour* pCVar3;
	edF32VECTOR4* puVar4;
	long lVar5;
	int currentPawnIndex;
	CChessBoardPawnsRefTable local_20;

	if ((this->eltTable).nbEntries != 0) {
		local_20.nbEntries = 0;
		if (pTable == (CChessBoardPawnsRefTable*)0x0) {
			pTable = &local_20;
		}

		(this->chessboard).readLayerMask = 3;
		(this->chessboard).writeLayerMask = 1;

		this->chessboard.Organise(pTable);

		(this->chessboard).writeLayerMask = 2;
		(this->chessboard).readLayerMask = 2;

		this->chessboard.UpdatePawns();

		currentPawnIndex = 0;
		do {
			iVar2 = this->chessboard.IsPawnUsed(currentPawnIndex);
			if (iVar2 != 0) {
				iVar2 = this->chessboard.PawnShallMove(currentPawnIndex);
				if (iVar2 != 0) {
					pActor = this->chessboard.GetPawnActor(currentPawnIndex);
					lVar5 = pTable->IsInList(pActor);
					if (((lVar5 == 0) && (pActor->curBehaviourId == 3)) && (pActor->IsKindOfObject(0x10) != false)) {
						pCVar3 = pActor->GetBehaviour(pActor->curBehaviourId);
						puVar4 = this->chessboard.GetPawnPosition(currentPawnIndex);
						//pCVar3->SetPositionToHold(0.05f, puVar4);
					}
				}
			}

			currentPawnIndex = currentPawnIndex + 1;
		} while (currentPawnIndex < 6);
	}

	return;
}


bool CSquad::AddFighter(CTeamElt* pTeamElt)
{
	int iVar1;
	bool bSuccess;
	CTeamElt** pEntry;
	int iVar3;

	iVar3 = 0;
	bSuccess = false;

	iVar1 = (this->eltTable).nbEntries;
	pEntry = this->eltTable.aEntries;

	while ((iVar3 < iVar1 && (!bSuccess))) {
		if ((*pEntry) == pTeamElt) {
			bSuccess = true;
		}
		else {
			pEntry = pEntry + 1;
			iVar3 = iVar3 + 1;
		}
	}

	if (bSuccess) {
		bSuccess = true;
	}
	else {
		if (iVar1 < 5) {
			this->eltTable.Add(pTeamElt);
			bSuccess = true;
		}
		else {
			bSuccess = false;
		}
	}

	return bSuccess;
}

void CSquad::ManageSemaphore(int index)
{
	this->aSemaphores[index].ManageSemaphore();

	return;
}

void CSquad::SynchronizePawns()
{
	CTeamElt* pCurTeamElt;
	bool bVar2;
	CActor* pCVar3;
	int iVar4;
	int iVar5;
	CTeamElt** pTeamEltIt;

	iVar5 = 0;
	do {
		pCVar3 = this->chessboard.GetPawnActor(iVar5);
		iVar4 = 0;
		bVar2 = false;
		pTeamEltIt = this->eltTable.aEntries;
		while ((iVar4 < (this->eltTable).nbEntries && (!bVar2))) {
			if ((*pTeamEltIt)->pEnemyActor == pCVar3) {
				bVar2 = true;
			}
			else {
				pTeamEltIt = pTeamEltIt + 1;
				iVar4 = iVar4 + 1;
			}
		}

		if (!bVar2) {
			this->chessboard.RemovePawn(iVar5);
		}

		iVar5 = iVar5 + 1;
	} while (iVar5 < 6);

	iVar5 = 0;
	pTeamEltIt = this->eltTable.aEntries;
	if (0 < (this->eltTable).nbEntries) {
		do {
			pCurTeamElt = *pTeamEltIt;
			iVar4 = this->chessboard.GetPawnFromActor(pCurTeamElt->pEnemyActor);
			if (pCurTeamElt->field_0x14 == -1) {
				if (iVar4 == -1) {
					pCurTeamElt->pawnId = this->chessboard.AddPawn(pCurTeamElt->pEnemyActor, 2);
				}
			}
			else {
				if (iVar4 != -1) {
					this->chessboard.RemovePawn(iVar4);
				}
			}

			iVar5 = iVar5 + 1;
			pTeamEltIt = pTeamEltIt + 1;
		} while (iVar5 < (this->eltTable).nbEntries);
	}
	return;
}

void SquadSemaphoreManager::ManageSemaphore()
{
	uint uVar1;
	CActorWolfen* pCVar2;
	int iVar3;
	SquadSemaphore* pSemaphore;
	uint uVar6;
	float fVar7;

	uVar6 = 0;
	pSemaphore = this->aSemaphores;
	if (this->nbSemaphores != 0) {
		do {
			uVar1 = pSemaphore->flags;
			if ((uVar1 & 1) == 0) {
				if ((uVar1 & 4) != 0) {
					fVar7 = pSemaphore->duration - GetTimer()->cutsceneDeltaTime;
					pSemaphore->duration = fVar7;
					if (fVar7 <= 0.0f) {
						pSemaphore->flags = pSemaphore->flags & 0xfffffffb;
						this->field_0xc = this->field_0xc + 1;
					}
				}
			}
			else {
				if ((uVar1 & 2) == 0) {
					IMPLEMENTATION_GUARD(
					iVar3 = CActorWolfen::SemaphoreKeepIt
					(*(CActorWolfen**)
						&((pSemaphore->pWolfen)->base).characterBase.base.base.state_0x10);
					if (iVar3 == 0) {
						CActorWolfen::DisableFightAction(pSemaphore->pWolfen);
						pCVar2 = pSemaphore->pWolfen;
						if (pCVar2 != (CActorWolfen*)0x0) {
							pSemaphore->field_0x4 = pCVar2;
						}
						if (this->field_0x4 == 0.0f) {
							if ((pSemaphore->flags & 5) != 0) {
								this->field_0xc = this->field_0xc + 1;
							}
							pSemaphore->flags = 0;
							pSemaphore->duration = 0.0f;
						}
						else {
							pSemaphore->flags = 4;
							pSemaphore->duration = this->field_0x4;
						}
						pSemaphore->pWolfen = (CActorWolfen*)0x0;
						pSemaphore->field_0x8 = 0;
					})
				}
				else {
					IMPLEMENTATION_GUARD(
					FUN_00172d40((int)pSemaphore->pWolfen, this->field_0x0);
					pSemaphore->flags = pSemaphore->flags & 0xfffffffd;)
				}
			}

			uVar6 = uVar6 + 1;
			pSemaphore = pSemaphore + 1;
		} while (uVar6 < this->nbSemaphores);
	}

	return;
}
