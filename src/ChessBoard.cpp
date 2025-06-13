#include "ChessBoard.h"
#include "MathOps.h"
#include "edMem.h"
#include "ActorFighter.h"
#include "EventManager.h"

CChessBoardPawnsRefTable CChessBoard::_pawnsWork;

s_chess_board_coord _forceNoMove = { 0xffffffff, 0xffffffff };
s_chess_board_pattern _patternWork;

float CoordinateToFloat(uint coordinate)
{
	if ((int)coordinate < 0) {
		float fVar8 = (float)(coordinate >> 1 | coordinate & 1);
		return fVar8 + fVar8;
	}
	else {
		return (float)coordinate;
	}
}

uint FloatToCoordinate(float coordinate)
{
	if (coordinate < 2.147484e+09f) {
		return (int)coordinate;
	}
	else {
		return (int)(coordinate - 2.147484e+09f) | 0x80000000;
	}
}

bool CChessBoardPawnsRefTable::IsInList(CActor* pActor)
{
	int iVar1 = this->nbEntries;
	int iVar6 = 0;
	s_chess_board_pawn** pCVar5 = this->aEntries;
	if (0 < iVar1) {
		do {
			if ((*pCVar5)->pFighter == pActor) return true;

			iVar6 = iVar6 + 1;
			pCVar5 = pCVar5 + 1;
		} while (iVar6 < iVar1);
	}

	return false;
}

CChessBoard::CChessBoard()
{
	this->pawnsTable.aPawns[0].bActive = 0;
	this->pawnsTable.aPawns[1].bActive = 0;
	this->pawnsTable.aPawns[2].bActive = 0;
	this->pawnsTable.aPawns[3].bActive = 0;
	this->pawnsTable.aPawns[4].bActive = 0;
	this->pawnsTable.aPawns[5].bActive = 0;

	this->pawnsTable.nbPawns = 0;

	this->aSubObjs[0] = (int*)0x0;
	this->aSubObjs[1] = (int*)0x0;
	this->nbColumns = 0;
	this->nbRows = 0;

	return;
}

void CChessBoard::Init(float param_1, float param_2, uint param_4, uint param_5)
{
	uint uVar2;
	uint uVar4;
	uint uVar5;
	float fVar6;
	
	this->field_0x210 = param_1;
	this->field_0x214 = param_2;
	
	fVar6 = CoordinateToFloat(param_4);
	this->field_0x21c = this->field_0x214 / fVar6;

	fVar6 = CoordinateToFloat(param_5);
	this->field_0x218 = 6.283185f / fVar6;

	this->nbRows = param_5;

	this->nbColumns = FloatToCoordinate(this->field_0x214 / this->field_0x21c);

	uVar4 = 0;
	do {
		if (this->aSubObjs[uVar4] != (int*)0x0) {
			edMemFree(this->aSubObjs[uVar4]);
		}

		if (this->nbColumns != 0) {
			this->aSubObjs[uVar4] = reinterpret_cast<int*>(edMemAlloc(TO_HEAP(H_MAIN), this->nbColumns * sizeof(int)));
		}

		uVar4 = uVar4 + 1;
	} while (uVar4 < 2);

	this->writeLayerMask = 0xffffffff;
	this->readLayerMask = 0xffffffff;

	uVar5 = FloatToCoordinate(this->field_0x210 / this->field_0x21c + 0.5f);
	uVar4 = this->nbColumns;

	uVar2 = 0;
	if (uVar4 != 0) {
		do {
			uVar4 = 0;
			if (uVar2 < uVar5) {
				uVar4 = 0;
				do {
					if ((this->writeLayerMask & 1 << (uVar4 & 0x1f)) != 0) {
						this->aSubObjs[uVar4][uVar2] = 0;
					}

					uVar4 = uVar4 + 1;
				} while (uVar4 < 2);
			}
			else {
				do {
					if ((this->writeLayerMask & 1 << (uVar4 & 0x1f)) != 0) {
						this->aSubObjs[uVar4][uVar2] = -1;
					}

					uVar4 = uVar4 + 1;
				} while (uVar4 < 2);
			}

			uVar2 = uVar2 + 1;
		} while (uVar2 < this->nbColumns);
	}

	return;
}

void CChessBoard::GetPawnsWithSameRingPos(CChessBoardPawnsRefTable* pTable, int ringPos)
{
	s_chess_board_pawn* pCurPawn;
	s_chess_board_pawn* pPawnIt;
	int iVar3;

	iVar3 = 0;
	pTable->nbEntries = 0;
	pPawnIt = this->pawnsTable.aPawns;

	do {
		pCurPawn = (s_chess_board_pawn*)0x0;
		if (pPawnIt->bActive == 1) {
			pCurPawn = pPawnIt;
		}

		if (((pCurPawn != (s_chess_board_pawn*)0x0) && (pCurPawn->bActive == 1)) && (pCurPawn->coordinates.field_0x0 == ringPos)) {
			pTable->Add(pCurPawn);
		}

		iVar3 = iVar3 + 1;
		pPawnIt = pPawnIt + 1;
	} while (iVar3 < 6);

	return;
}

CActor* CChessBoard::GetPawnActor(int index)
{
	int iVar1;
	s_chess_board_pawn* pPawn;

	iVar1 = this->pawnsTable.aPawns[index].bActive;
	pPawn = (s_chess_board_pawn*)0x0;
	if (iVar1 == 1) {
		pPawn = this->pawnsTable.aPawns + index;
	}

	if (pPawn != (s_chess_board_pawn*)0x0) {
		pPawn = (s_chess_board_pawn*)0x0;

		if (iVar1 == 1) {
			pPawn = this->pawnsTable.aPawns + index;
		}

		if (pPawn->bActive != 0) {
			if (iVar1 != 1) {
				return (CActor*)0x0;
			}

			return pPawn->pFighter;
		}
	}

	return (CActor*)0x0;
}

edF32VECTOR4* CChessBoard::GetPawnPosition(int index)
{
	edF32VECTOR4* peVar1;

	if (this->pawnsTable.aPawns[index].bActive == 0x1) {
		peVar1 = &this->pawnsTable.aPawns[index].field_0x10;
	}
	else {
		peVar1 = (edF32VECTOR4*)0x10;
	}

	return peVar1;
}

void CChessBoard::RemovePawn(int index)
{
	if (this->pawnsTable.aPawns[index].bActive == 1) {
		this->pawnsTable.aPawns[index].bActive = 0;
		this->pawnsTable.nbPawns = this->pawnsTable.nbPawns - 1;
	}

	return;
}

int CChessBoard::AddPawn(CActor* pActor, int param_3)
{
	uint uVar1;
	int pawnId;
	s_chess_board_pawn* pPawnSafe;
	s_chess_board_pawn* pPawn;
	float fVar3;
	float fVar4;
	edF32VECTOR4 eStack80;
	edF32MATRIX4 auStack64;

	pawnId = this->pawnsTable.Add(pActor, param_3);
	if (pawnId != -1) {
		pPawn = this->pawnsTable.aPawns + pawnId;
		pPawnSafe = (s_chess_board_pawn*)0x0;
		if (pPawn->bActive == 0x1) {
			pPawnSafe = pPawn;
		}

		edF32Vector4SubHard(&auStack64.rowT, &gF32Vector4Zero, &this->field_0x230);
		edF32Matrix4TranslateHard(&auStack64, &gF32Matrix4Unit, &auStack64.rowT);
		fVar3 = GetAngleYFromVector(&this->field_0x240);
		edF32Matrix4RotateYHard(-fVar3, &auStack64, &auStack64);
		edF32Matrix4MulF32Vector4Hard(&eStack80, &auStack64, &pActor->currentLocation);
		eStack80.y = 0.0f;
		eStack80.w = 0.0f;
		fVar3 = GetAngleYFromVector(&eStack80);
		fVar3 = edF32Between_0_2Pi(fVar3);

		fVar4 = CoordinateToFloat(this->nbRows);

		fVar4 = (fVar3 / 6.283185f) * fVar4;

		pPawnSafe->coordinates.field_0x4 = FloatToCoordinate(fVar4);

		fVar3 = edF32Vector4GetDistHard(&eStack80);

		fVar4 = CoordinateToFloat(this->nbColumns);

		pPawnSafe->coordinates.field_0x0 = CoordinateToFloat(fVar4 * (fVar3 / this->field_0x214));

		pPawnSafe = (s_chess_board_pawn*)0x0;
		if (pPawn->bActive == 1) {
			pPawnSafe = pPawn;
		}

		if (this->nbColumns <= pPawnSafe->coordinates.field_0x0) {
			pPawnSafe = pPawn;

			if (pPawn->bActive != 1) {
				pPawnSafe = (s_chess_board_pawn*)0x0;
			}

			pPawnSafe->coordinates.field_0x0 = this->nbColumns - 1;
		}

		if (pPawn->bActive != 1) {
			pPawn = (s_chess_board_pawn*)0x0;
		}

		pPawn->field_0x30 = _forceNoMove;
	}
	return pawnId;
}

int CChessBoard::GetPawnFromActor(CActor* pActor)
{
	int iVar1;
	int iVar2;
	s_chess_board_pawn* pCVar3;
	s_chess_board_pawn* pCVar4;

	if (((this->pawnsTable).nbPawns != 0) && (iVar2 = 0, pActor != (CActor*)0x0)) {
		pCVar4 = this->pawnsTable.aPawns;
		do {
			iVar1 = pCVar4->bActive;
			pCVar3 = (s_chess_board_pawn*)0x0;
			if (iVar1 == 1) {
				pCVar3 = pCVar4;
			}

			if (pCVar3 != (s_chess_board_pawn*)0x0) {
				pCVar3 = (s_chess_board_pawn*)0x0;
				if (iVar1 == 1) {
					pCVar3 = pCVar4;
				}

				if (pCVar3->bActive != 0) {
					pCVar3 = (s_chess_board_pawn*)0x0;
					if (iVar1 == 1) {
						pCVar3 = pCVar4;
					}

					if (pCVar3->pFighter == pActor) {
						return iVar2;
					}
				}
			}

			iVar2 = iVar2 + 1;
			pCVar4 = pCVar4 + 1;
		} while (iVar2 < 6);
	}
	return -1;
}

int CChessBoard::IsPawnUsed(int index)
{
	int result;
	s_chess_board_pawn* pPawnA;
	s_chess_board_pawn* pPawnB;

	pPawnA = this->pawnsTable.aPawns + index;
	pPawnB = (s_chess_board_pawn*)0x0;
	if (pPawnA->bActive == 0x1) {
		pPawnB = pPawnA;
	}
	if (pPawnB == (s_chess_board_pawn*)0x0) {
		result = 0x0;
	}
	else {
		result = 0;
		if (pPawnB->bActive == 0x1) {
			result = pPawnB->bActive;
		}
	}

	return result;
}

bool CChessBoard::PawnShallMove(int index)
{
	uint uVar1;
	s_chess_board_pawn* pPawn;
	uint piVar3;
	float fVar4;
	float fVar5;
	uint piVar6;
	edF32VECTOR4 eStack80;
	edF32MATRIX4 auStack64;

	pPawn = this->pawnsTable.aPawns + index;
	if (this->pawnsTable.aPawns[index].bActive != 0x1) {
		pPawn = (s_chess_board_pawn*)0x0;
	}

	if (pPawn != (s_chess_board_pawn*)0x0) {
		edF32Vector4SubHard(&auStack64.rowT, &gF32Vector4Zero, &this->field_0x230);
		edF32Matrix4TranslateHard(&auStack64, &gF32Matrix4Unit, &auStack64.rowT);
		fVar4 = GetAngleYFromVector(&this->field_0x240);
		edF32Matrix4RotateYHard(-fVar4, &auStack64, &auStack64);
		edF32Matrix4MulF32Vector4Hard(&eStack80, &auStack64, &pPawn->pFighter->currentLocation);

		eStack80.y = 0.0f;
		eStack80.w = 0.0f;

		fVar4 = GetAngleYFromVector(&eStack80);
		fVar4 = edF32Between_0_2Pi(fVar4);

		fVar5 = CoordinateToFloat(this->nbRows);
		fVar5 = (fVar4 / 6.283185f) * fVar5;
		piVar3 = FloatToCoordinate(fVar5);

		fVar4 = edF32Vector4GetDistHard(&eStack80);

		fVar5 = CoordinateToFloat(this->nbColumns);
		fVar5 = fVar5 * (fVar4 / this->field_0x214);
		piVar6 = FloatToCoordinate(fVar5);

		if ((pPawn->coordinates.field_0x0 != piVar6) || (pPawn->coordinates.field_0x4 != piVar3)) {
			return true;
		}
	}

	return false;
}

void CChessBoard::ValidateAll()
{
	uint uVar1;
	CChessBoard* pCVar2;
	int iVar3;
	uint uVar4;
	float fVar5;
	uint uVar6;

	uVar6 = FloatToCoordinate(this->field_0x210 / this->field_0x21c + 0.5f);
	uVar4 = this->nbColumns;

	uVar1 = 0;
	if (uVar4 != 0) {
		iVar3 = 0;
		do {
			uVar4 = 0;
			if (uVar1 < uVar6) {
				uVar4 = 0;
				do {
					if ((this->writeLayerMask & 1 << (uVar4 & 0x1f)) != 0) {
						this->aSubObjs[uVar4][uVar1] = 0;
					}
					uVar4 = uVar4 + 1;
				} while (uVar4 < 2);
			}
			else {
				do {
					if ((this->writeLayerMask & 1 << (uVar4 & 0x1f)) != 0) {
						this->aSubObjs[uVar4][uVar1] = -1;
					}
					uVar4 = uVar4 + 1;
				} while (uVar4 < 2);
			}

			uVar1 = uVar1 + 1;
		} while (uVar1 < this->nbColumns);
	}
	return;
}

void CChessBoard::InvalidateSection(float param_1, edF32VECTOR4* param_3)
{
	uint uVar1;
	uint uVar2;
	uint uVar3;
	uint uVar4;
	uint unaff_s0_lo;
	uint uVar5;
	uint uVar6;
	uint uVar7;
	float puVar8;
	float puVar9;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 local_10;

	local_10.x = (this->field_0x240).z;
	local_10.y = 0.0f;
	local_10.z = -(this->field_0x240).x;
	local_10.w = 0.0f;
	puVar8 = edF32Vector4DotProductHard(param_3, &this->field_0x240);
	if (1.0f < puVar8) {
		puVar9 = 1.0f;
	}
	else {
		puVar9 = -1.0f;
		if (-1.0f <= puVar8) {
			puVar9 = puVar8;
		}
	}

	fVar8 = acosf(puVar9);
	fVar9 = edF32Vector4DotProductHard(param_3, &local_10);
	if (fVar9 < 0.0f) {
		fVar8 = 6.283185f - fVar8;
	}

	fVar9 = CoordinateToFloat(this->nbRows);

	fVar9 = (param_1 * fVar9) / 6.283185f + 0.5f;
	uVar7 = FloatToCoordinate(fVar9);

	fVar9 = 3.402823e+38f;
	for (uVar6 = 0; uVar6 < this->nbRows; uVar6 = uVar6 + 1) {
		fVar11 = CoordinateToFloat(uVar6);
		fVar12 = CoordinateToFloat(uVar7);
		fVar10 = CoordinateToFloat(this->nbRows);

		fVar11 = edF32Between_0_2Pi(((fVar11 + fVar12 / 2.0f) * 6.283185f) / fVar10);
		fVar11 = fabs(fVar11 - fVar8);
		uVar5 = uVar6;
		if (fVar9 <= fVar11) {
			fVar11 = fVar9;
			uVar5 = unaff_s0_lo;
		}

		fVar9 = fVar11;
		unaff_s0_lo = uVar5;
	}

	for (uVar6 = 0; uVar6 < uVar7; uVar6 = uVar6 + 1) {
		uVar5 = this->nbRows;
		if (uVar5 == 0) {
			trap(7);
		}

		for (uVar2 = 0; uVar2 < this->nbColumns; uVar2 = uVar2 + 1) {
			uVar1 = 0xffffffff;
			for (uVar3 = 0; uVar4 = 0xffffffff, uVar3 < 2; uVar3 = uVar3 + 1) {
				if ((this->readLayerMask & 1 << (uVar3 & 0x1f)) != 0) {
					uVar1 = uVar1 & this->aSubObjs[uVar3][uVar2];
				}
			}

			for (uVar3 = 0; uVar3 < 2; uVar3 = uVar3 + 1) {
				if ((this->readLayerMask & 1 << (uVar3 & 0x1f)) != 0) {
					uVar4 = uVar4 & this->aSubObjs[uVar3][uVar2];
				}
			}

			for (uVar3 = 0; uVar3 < 2; uVar3 = uVar3 + 1) {
				if ((this->writeLayerMask & 1 << (uVar3 & 0x1f)) != 0) {
					this->aSubObjs[uVar3][uVar2] = uVar1 ^ 1 << ((int)(unaff_s0_lo + uVar6) % (int)uVar5 & 0x1fU) & uVar4;
				}
			}
		}
	}

	return;
}

void CChessBoard::InvalidateByRayTrace()
{
	IMPLEMENTATION_GUARD();
}

uint CChessBoard::ReadLayer(uint index)
{
	uint result = 0xffffffff;
	uint currentIndex = 0;
	int** pDataIt = this->aSubObjs;
	do {
		if ((this->readLayerMask & 1 << (currentIndex & 0x1f)) != 0) {
			result = result & (*pDataIt)[index];
		}

		currentIndex = currentIndex + 1;
		pDataIt = pDataIt + 1;
	} while (currentIndex < 2);

	return result;
}

void CChessBoard::WriteLayer(uint index, uint value)
{
	uint currentIndex = 0;
	int** pDataIt = this->aSubObjs;
	do {
		if ((this->writeLayerMask & 1 << (currentIndex & 0x1f)) != 0) {
			(*pDataIt)[index] = value;
		}

		currentIndex = currentIndex + 1;
		pDataIt = pDataIt + 1;
	} while (currentIndex < 2);
}

void CChessBoard::InvalidateByZone(int zoneId)
{
	CEventManager* pEventManager;
	ed_zone_3d* pZone;
	uint uVar2;
	int iVar3;
	int** pCVar4;
	uint uVar5;
	uint uVar6;
	uint uVar7;
	uint uVar8;
	uint uVar9;
	uint curColmunIndex;
	uint curRowIndex;
	float fVar12;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pZone = (ed_zone_3d*)0x0;
	if (zoneId != -1) {
		pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, zoneId);
	}

	curRowIndex = 0;
	if (this->nbRows != 0) {
		do {
			if ((int)curRowIndex < 0) {
				fVar12 = (float)(curRowIndex >> 1 | curRowIndex & 1);
				fVar12 = fVar12 + fVar12;
			}
			else {
				fVar12 = (float)curRowIndex;
			}

			SetVectorFromAngleY(fVar12 * this->field_0x218, &eStack16);

			curColmunIndex = 0;
			if (this->nbColumns != 0) {
				uVar9 = 1 << (curRowIndex & 0x1f);
				do {
					fVar12 = CoordinateToFloat(curColmunIndex + 1);

					edF32Vector4ScaleHard(fVar12 * this->field_0x21c, &eStack32, &eStack16);
					edF32Vector4AddHard(&eStack32, &eStack32, &this->field_0x230);

					iVar3 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &eStack32, 0);
					if (iVar3 == 2) {
						uVar2 = ReadLayer(curColmunIndex);
						uVar7 = ReadLayer(curColmunIndex);

						WriteLayer(curColmunIndex, uVar2 ^ uVar9 & uVar7);

						if (curRowIndex == 0) {
							uVar2 = this->nbRows;
						}
						else {
							uVar2 = curRowIndex;
						}
						uVar2 = 1 << (uVar2 - 1 & 0x1f);

						uVar7 = ReadLayer(curColmunIndex);
						uVar8 = ReadLayer(curColmunIndex);

						WriteLayer(curColmunIndex, uVar7 ^ uVar2 & uVar8);

						iVar3 = curColmunIndex + 1;
						if (curColmunIndex < this->nbColumns - 1) {
							uVar5 = ReadLayer(iVar3);
							uVar6 = ReadLayer(iVar3);

							WriteLayer(iVar3, uVar5 ^ uVar9 & uVar6);
							
							uVar2 = ReadLayer(iVar3);
							uVar7 = ReadLayer(iVar3);

							WriteLayer(iVar3, uVar2 ^ uVar9 & uVar7);
						}
					}

					curColmunIndex = curColmunIndex + 1;
				} while (curColmunIndex < this->nbColumns);
			}

			curRowIndex = curRowIndex + 1;
		} while (curRowIndex < this->nbRows);
	}

	return;
}

void CChessBoard::InvalidateByPath(CPathFinderClient* pPathFinderClient)
{
	bool bVar1;
	uint uVar2;
	int iVar3;
	uint uVar5;
	uint uVar6;
	uint uVar7;
	uint uVar8;
	uint uVar9;
	uint uVar10;
	uint uVar11;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	uVar11 = 0;
	if (this->nbRows != 0) {
		do {
			SetVectorFromAngleY(CoordinateToFloat(uVar11) * this->field_0x218, &eStack16);

			uVar10 = 0;
			if (this->nbColumns != 0) {
				uVar9 = 1 << (uVar11 & 0x1f);
				do {
					uVar2 = uVar10 + 1;

					edF32Vector4ScaleHard(CoordinateToFloat(uVar2) * this->field_0x21c, &eStack32, &eStack16);
					edF32Vector4AddHard(&eStack32, &eStack32, &this->field_0x230);

					bVar1 = pPathFinderClient->IsValidPosition(&eStack32);
					if (bVar1 == false) {
						uVar2 = ReadLayer(uVar10);
						uVar7 = ReadLayer(uVar10);
						WriteLayer(uVar10, uVar2 ^ uVar9 & uVar7);

						if (uVar11 == 0) {
							uVar2 = this->nbRows;
						}
						else {
							uVar2 = uVar11;
						}

						uVar2 = 1 << (uVar2 - 1 & 0x1f);

						uVar7 = ReadLayer(uVar10);
						uVar8 = ReadLayer(uVar10);
						WriteLayer(uVar10, uVar7 ^ uVar2 & uVar8);
						
						iVar3 = uVar10 + 1;
						if (uVar10 < this->nbColumns - 1) {
							uVar5 = ReadLayer(iVar3);
							uVar6 = ReadLayer(iVar3);
							WriteLayer(iVar3, uVar5 ^ uVar2 & uVar6);
							
							uVar2 = ReadLayer(iVar3);
							uVar7 = ReadLayer(iVar3);
							WriteLayer(iVar3, uVar2 ^ uVar9 & uVar7);
						}
					}

					uVar10 = uVar10 + 1;
				} while (uVar10 < this->nbColumns);
			}

			uVar11 = uVar11 + 1;
		} while (uVar11 < this->nbRows);
	}

	return;
}

int CChessBoardPawnsTable::Add(CActor* pActor, int param_3)
{
	int index;
	s_chess_board_pawn* psVar1;
	int iVar2;

	index = -1;

	if (this->nbPawns < 6) {
		iVar2 = 0;
		do {
			if (this->aPawns[iVar2].pFighter == pActor) {
				index = iVar2;
				iVar2 = 6;
			}
			else {
				if ((index == -1) && (this->aPawns[iVar2].bActive == 0)) {
					index = iVar2;
				}
			}

			iVar2 = iVar2 + 1;
		} while (iVar2 < 6);

		if (index != -1) {
			psVar1 = this->aPawns + index;
			psVar1->bActive = 1;
			psVar1->pFighter = reinterpret_cast<CActorFighter*>(pActor);
			psVar1->field_0x8 = param_3;
			this->nbPawns = this->nbPawns + 1;
		}
	}

	return index;
}


s_chess_board_coord* CChessBoard::_FindNearestAvailableCell(s_chess_board_coord* param_2, s_chess_board_coord* param_3)
{
	uint uVar1;
	uint uVar2;
	bool bVar3;
	uint in_a3_lo;
	uint in_t0_lo;
	int iVar4;
	int iVar5;
	uint uVar6;
	uint uVar7;
	CChessBoard* pCVar8;
	uint uVar9;
	int iVar10;
	int iVar11;
	uint uVar12;

	iVar4 = 1;
	bVar3 = false;
LAB_003531d0:
	if (!bVar3) {
		uVar1 = this->nbRows;
		uVar2 = this->nbColumns;

		uVar12 = uVar1;
		if (uVar1 < uVar2) {
			uVar12 = uVar2;
		}

		if (iVar4 < (int)uVar12) {
			iVar5 = -iVar4;
			iVar10 = 0;
			iVar11 = iVar5;
		LAB_00352ef8:
			do {
				if ((0 < iVar11) || (bVar3)) goto LAB_00352f08;

				uVar12 = (int)(param_2->field_0x4 + iVar10) % (int)uVar1;
				uVar6 = param_2->field_0x0 + iVar11;

				if (uVar1 == 0) {
					trap(7);
				}

				if ((int)uVar1 <= (int)uVar12) {
					uVar12 = uVar12 - uVar1;
				}

				if ((int)uVar12 < 0) {
					uVar12 = uVar12 + uVar1;
				}

				if ((-1 < (int)uVar6) && ((int)uVar6 < (int)uVar2)) {
					uVar7 = ReadLayer(uVar6);
					if ((uVar7 >> (uVar12 & 0x1f) & 1) != 0) {
						bVar3 = true;
						in_t0_lo = uVar6;
						in_a3_lo = uVar12;
						goto LAB_00352ef8;
					}
				}

				iVar11 = iVar11 + 1;
				iVar10 = iVar10 + 1;
			} while (true);
		}
	}

	if (bVar3) {
		param_3->field_0x0 = in_t0_lo;
		param_3->field_0x4 = in_a3_lo;
	}
	else {
		param_3 = (s_chess_board_coord*)0x0;
	}

	return param_3;

LAB_00352f08:
	iVar11 = 0;
LAB_00352fd8:
	do {
		if ((0 < iVar5) || (bVar3)) break;

		uVar12 = (int)(param_2->field_0x4 + iVar11) % (int)uVar1;
		uVar6 = param_2->field_0x0 + iVar5;
		if (uVar1 == 0) {
			trap(7);
		}

		if ((int)uVar1 <= (int)uVar12) {
			uVar12 = uVar12 - uVar1;
		}

		if ((int)uVar12 < 0) {
			uVar12 = uVar12 + uVar1;
		}

		if ((-1 < (int)uVar6) && ((int)uVar6 < (int)uVar2)) {
			uVar7 = ReadLayer(uVar6);
			if ((uVar7 >> (uVar12 & 0x1f) & 1) != 0) {
				bVar3 = true;
				in_t0_lo = uVar6;
				in_a3_lo = uVar12;
				goto LAB_00352fd8;
			}
		}
		iVar5 = iVar5 + 1;
		iVar11 = iVar11 + -1;
	} while (true);
	iVar11 = 1;
	iVar5 = iVar4 + -1;
LAB_003530c0:
	do {
		if ((iVar4 < iVar11) || (bVar3)) break;
		uVar12 = (int)(param_2->field_0x4 + iVar5) % (int)uVar1;
		uVar6 = param_2->field_0x0 + iVar11;

		if (uVar1 == 0) {
			trap(7);
		}

		if ((int)uVar1 <= (int)uVar12) {
			uVar12 = uVar12 - uVar1;
		}

		if ((int)uVar12 < 0) {
			uVar12 = uVar12 + uVar1;
		}

		if ((-1 < (int)uVar6) && ((int)uVar6 < (int)uVar2)) {
			uVar7 = ReadLayer(uVar6);
			if ((uVar7 >> (uVar12 & 0x1f) & 1) != 0) {
				bVar3 = true;
				in_t0_lo = uVar6;
				in_a3_lo = uVar12;
				goto LAB_003530c0;
			}
		}
		iVar11 = iVar11 + 1;
		iVar5 = iVar5 + -1;
	} while (true);
	iVar11 = 1;
	iVar5 = 1 - iVar4;
LAB_003531b0:
	do {
		if ((iVar4 < iVar11) || (bVar3)) break;
		uVar12 = (int)(param_2->field_0x4 + iVar5) % (int)uVar1;
		uVar6 = param_2->field_0x0 + iVar11;
		if (uVar1 == 0) {
			trap(7);
		}
		if ((int)uVar1 <= (int)uVar12) {
			uVar12 = uVar12 - uVar1;
		}
		if ((int)uVar12 < 0) {
			uVar12 = uVar12 + uVar1;
		}
		if ((-1 < (int)uVar6) && ((int)uVar6 < (int)uVar2)) {
			uVar7 = ReadLayer(uVar6);
			if ((uVar7 >> (uVar12 & 0x1f) & 1) != 0) {
				bVar3 = true;
				in_t0_lo = uVar6;
				in_a3_lo = uVar12;
				goto LAB_003531b0;
			}
		}

		iVar11 = iVar11 + 1;
		iVar5 = iVar5 + 1;
	} while (true);

	iVar4 = iVar4 + 1;
	goto LAB_003531d0;
}

float CChessBoard::_PermutPawns(float param_1, CChessBoardPawnsRefTable* pTable, s_chess_board_pattern* pPattern, float* param_5, uint param_6)
{
	uint uVar1;
	uint uVar2;
	s_chess_board_pawn* psVar3;
	bool bVar4;
	uint* puVar5;
	int iVar6;
	uint* puVar7;
	uint uVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;

	fVar13 = *param_5;
	for (uVar8 = param_6; uVar8 < _patternWork.nbCells; uVar8 = uVar8 + 1) {
		if (param_6 != uVar8) {
			uVar1 = _patternWork.aCells[param_6].field_0x0;
			puVar7 = &_patternWork.aCells[param_6].field_0x4;
			puVar5 = &_patternWork.aCells[uVar8].field_0x4;
			uVar2 = *puVar7;
			_patternWork.aCells[param_6].field_0x0 = _patternWork.aCells[uVar8].field_0x0;
			*puVar7 = *puVar5;
			_patternWork.aCells[uVar8].field_0x0 = uVar1;
			*puVar5 = uVar2;
		}

		psVar3 = pTable->aEntries[param_6];
		bVar4 = false;
		if ((_patternWork.aCells[param_6].field_0x0 == _forceNoMove.field_0x0) &&
			(_patternWork.aCells[param_6].field_0x4 == _forceNoMove.field_0x4)) {
			bVar4 = true;
		}

		if (bVar4) {
			fVar13 = 0.0f;
		}
		else {
			fVar9 = CoordinateToFloat(_patternWork.aCells[param_6].field_0x0);
			fVar11 = CoordinateToFloat((psVar3->coordinates).field_0x0);
			fVar10 = CoordinateToFloat(_patternWork.aCells[param_6].field_0x4);
			fVar12 = CoordinateToFloat((psVar3->coordinates).field_0x4);

			fVar10 = fVar10 - fVar12;
			if (0.0f < fVar9 - fVar11) {
				fVar13 = (fVar9 - fVar11) + 0.0f;
			}

			iVar6 = 1;
			if (fVar10 <= 0.0f) {
				iVar6 = -1;
			}

			fVar11 = CoordinateToFloat(this->nbRows);

			fVar11 = fabs(fVar10 - fVar11 * (float)iVar6);
			fVar13 = fVar13 + (float)((int)fVar9 * (uint)(fVar9 < fVar11) | (int)fVar11 * (uint)(fVar9 >= fVar11));
			fVar13 = fVar13 * fVar13;
		}

		fVar13 = param_1 + fVar13;
		if (param_6 + 1 < _patternWork.nbCells) {
			if (fVar13 < *param_5) {
				fVar13 = _PermutPawns(fVar13, pTable, pPattern, param_5, param_6 + 1);
			}
		}
		else {
			if (fVar13 < *param_5) {
				memcpy(pPattern, &_patternWork, sizeof(s_chess_board_pattern));
				*param_5 = fVar13;
			}
		}

		if (param_6 != uVar8) {
			uVar1 = _patternWork.aCells[param_6].field_0x0;
			puVar7 = &_patternWork.aCells[param_6].field_0x4;
			puVar5 = &_patternWork.aCells[uVar8].field_0x4;
			uVar2 = *puVar7;
			_patternWork.aCells[param_6].field_0x0 = _patternWork.aCells[uVar8].field_0x0;
			*puVar7 = *puVar5;
			_patternWork.aCells[uVar8].field_0x0 = uVar1;
			*puVar5 = uVar2;
		}
	}

	return fVar13;
}

void CChessBoard::Organise(CChessBoardPawnsRefTable* pTable)
{
	s_chess_board_pawn* psVar1;
	bool bVar2;
	s_chess_board_coord* psVar3;
	int iVar4;
	ulong uVar5;
	CChessBoardPawnsTable* pCVar6;
	uint uVar7;
	ulong uVar8;
	ulong in_a3;
	ulong in_t0;
	uint uVar9;
	uint uVar10;
	ulong uVar11;
	int iVar14;
	s_chess_board_pawn** ppsVar15;
	ulong uVar16;
	CChessBoardPawnsRefTable localTable;
	s_chess_board_pattern pattern;
	s_chess_board_coord local_10;
	float local_4;

	if (pTable == (CChessBoardPawnsRefTable*)0x0) {
		pTable = &_pawnsWork;
		_pawnsWork.nbEntries = 0;
	}
	else {
		pTable->nbEntries = 0;
	}

	pattern.nbCells = 0;
	iVar14 = 0;

	localTable.nbEntries = 0;

	s_chess_board_pawn* pPawnIt = this->pawnsTable.aPawns;
	do {
		s_chess_board_pawn* pPawn = (s_chess_board_pawn*)0x0;
		if (pPawnIt->bActive == 1) {
			pPawn = pPawnIt;
		}

		if ((pPawn != (s_chess_board_pawn*)0x0) && (pPawn->bActive == 1)) {
			localTable.Add(pPawn);
		}

		iVar14 = iVar14 + 1;
		pPawnIt = pPawnIt + 1;
	} while (iVar14 < 6);

	iVar14 = 0;
	if (0 < localTable.nbEntries) {
		s_chess_board_pawn** ppPawn = localTable.aEntries;
		do {
			uVar9 = pattern.nbCells;
			s_chess_board_pawn* pPawn = *ppPawn;
			if (((pPawn->coordinates).field_0x0 < this->nbColumns) && ((pPawn->coordinates).field_0x4 < this->nbRows)) {
				uVar7 = ReadLayer((pPawn->coordinates).field_0x0);

				if (((uVar7 >> ((pPawn->coordinates).field_0x4 & 0x1f) & 1) == 0) || (this->field_0x220 == 1))
					goto LAB_00354438;

				uVar8 = ReadLayer((pPawn->coordinates).field_0x0);
				uVar7 = 0;

				pattern.nbCells = pattern.nbCells + 1;
				pattern.aCells[uVar9] = pPawn->coordinates;
				
				uVar11 = ReadLayer((pPawn->coordinates).field_0x0);

				WriteLayer((pPawn->coordinates).field_0x0, uVar8 ^ 1 << ((pPawn->coordinates).field_0x4 & 0x1f) & uVar11);
			}
			else {
			LAB_00354438:
				pTable->Add(pPawn);
			}

			iVar14 = iVar14 + 1;
			ppPawn = ppPawn + 1;
		} while (iVar14 < localTable.nbEntries);
	}

	iVar14 = pTable->nbEntries + -1;
	if (-1 < iVar14) {
		ppsVar15 = pTable->aEntries + pTable->nbEntries + -2;
		do {
			uVar9 = pattern.nbCells;
			psVar3 = _FindNearestAvailableCell(&ppsVar15[1]->coordinates, &local_10);
			if (psVar3 == (s_chess_board_coord*)0x0) {
				pattern.nbCells = pattern.nbCells + 1;
				pattern.aCells[uVar9] = _forceNoMove;
			}
			else {
				uVar8 = ReadLayer(local_10.field_0x0);
				pattern.nbCells = pattern.nbCells + 1;
				pattern.aCells[uVar9] = local_10;
				
				uVar11 = ReadLayer(local_10.field_0x0);

				WriteLayer(local_10.field_0x0, uVar8 ^ 1 << (local_10.field_0x4 & 0x1f) & uVar11);

				pTable->Pop(iVar14);
			}

			iVar14 = iVar14 + -1;
			ppsVar15 = ppsVar15 + -1;
		} while (-1 < iVar14);
	}

	if ((pTable->nbEntries == 0) && (uVar9 = 0, pattern.nbCells != 0)) {
		psVar3 = pattern.aCells;
		do {
			bVar2 = psVar3->field_0x0 == _forceNoMove.field_0x0;
			if (bVar2) {
				bVar2 = psVar3->field_0x4 == _forceNoMove.field_0x4;
			}

			if (!bVar2) {
				while (psVar3->field_0x0 != 0) {
					uVar10 = ReadLayer(psVar3->field_0x0 - 1);

					if ((uVar10 >> (psVar3->field_0x4 & 0x1f) & 1) == 0) break;

					psVar3->field_0x0 = psVar3->field_0x0 - 1;
				}
			}
			uVar9 = uVar9 + 1;
			psVar3 = psVar3 + 1;
		} while (uVar9 < pattern.nbCells);
	}

	local_4 = 3.402823e+38f;
	memcpy(&_patternWork, &pattern, sizeof(s_chess_board_pattern));
	_PermutPawns(0.0f, &localTable, &pattern, &local_4, 0);

	uVar9 = 0;
	if (pattern.nbCells != 0) {
		psVar3 = pattern.aCells;
		do {
			bVar2 = _forceNoMove.field_0x0 == psVar3->field_0x0;
			if (bVar2) {
				bVar2 = _forceNoMove.field_0x4 == psVar3->field_0x4;
			}

			if (!bVar2) {
				psVar1 = localTable.aEntries[uVar9];

				(psVar1->coordinates).field_0x0 = psVar3->field_0x0;
				(psVar1->coordinates).field_0x4 = psVar3->field_0x4;
			}

			uVar9 = uVar9 + 1;
			psVar3 = psVar3 + 1;
		} while (uVar9 < pattern.nbCells);
	}

	this->field_0x220 = 0;

	return;
}

void CChessBoard::UpdatePawns()
{
	int iVar1;
	uint uVar2;
	s_chess_board_pawn* pPawnA;
	s_chess_board_pawn* pPawnB;
	s_chess_board_pawn* pPawnItA;
	s_chess_board_pawn* pPawnItB;
	edF32VECTOR4* v0;
	int iVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	if ((this->pawnsTable).nbPawns != 0) {
		iVar7 = 0;
		pPawnItA = this->pawnsTable.aPawns;
		pPawnItB = this->pawnsTable.aPawns;
		do {
			iVar1 = pPawnItB->bActive;
			pPawnA = (s_chess_board_pawn*)0x0;
			if (iVar1 == 1) {
				pPawnA = pPawnItA;
			}

			if (pPawnA != (s_chess_board_pawn*)0x0) {
				pPawnA = (s_chess_board_pawn*)0x0;

				if (iVar1 == 1) {
					pPawnA = pPawnItA;
				}

				MoveTo(pPawnA);

				iVar1 = pPawnItB->bActive;
				pPawnA = (s_chess_board_pawn*)0x0;
				if (iVar1 == 1) {
					pPawnA = pPawnItA;
				}

				pPawnB = pPawnItA;
				if (iVar1 != 1) {
					pPawnB = (s_chess_board_pawn*)0x0;
				}

				v0 = &pPawnB->field_0x10;

				fVar8 = CoordinateToFloat(pPawnA->field_0x28.field_0x4);
				fVar9 = CoordinateToFloat(this->nbRows);

				SetVectorFromAngleY(((fVar8 + 0.5f) / fVar9) * 6.283185f, v0);

				fVar8 = CoordinateToFloat(pPawnA->field_0x28.field_0x0);

				edF32Vector4ScaleHard((fVar8 + 0.5f) * this->field_0x21c, v0, v0);

				local_50.xyz = (this->field_0x230).xyz;
				local_50.w = 0.0f;

				fVar8 = GetAngleYFromVector(&this->field_0x240);
				edF32Matrix4RotateYHard(fVar8, &eStack64, &gF32Matrix4Unit);
				edF32Matrix4TranslateHard(&eStack64, &eStack64, &local_50);
				pPawnB->field_0x10.w = 1.0f;
				edF32Matrix4MulF32Vector4Hard(v0, &eStack64, v0);
			}

			iVar7 = iVar7 + 1;
			pPawnItB = pPawnItB + 1;
			pPawnItA = pPawnItA + 1;
		} while (iVar7 < 6);
	}

	return;
}

bool CChessBoard::MoveTo(s_chess_board_pawn* pPawn)
{
	CActorFighter* pCVar1;
	uint uVar2;
	bool bVar3;
	int iVar4;
	uint uVar5;
	uint uVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack96;
	edF32MATRIX4 auStack80;
	s_chess_board_coord local_8;

	pCVar1 = pPawn->pFighter;
	edF32Vector4SubHard(&auStack80.rowT, &gF32Vector4Zero, &this->field_0x230);
	edF32Matrix4TranslateHard(&auStack80, &gF32Matrix4Unit, &auStack80.rowT);
	fVar7 = GetAngleYFromVector(&this->field_0x240);
	edF32Matrix4RotateYHard(-fVar7, &auStack80, &auStack80);
	edF32Matrix4MulF32Vector4Hard(&eStack96, &auStack80, &pCVar1->currentLocation);
	eStack96.y = 0.0f;
	eStack96.w = 0.0f;
	fVar7 = GetAngleYFromVector(&eStack96);
	fVar7 = edF32Between_0_2Pi(fVar7);
	fVar8 = CoordinateToFloat(this->nbRows);
	fVar8 = (fVar7 / 6.283185f) * fVar8;
	local_8.field_0x4 = FloatToCoordinate(fVar8);
	fVar7 = edF32Vector4GetDistHard(&eStack96);
	fVar8 = CoordinateToFloat(this->nbColumns);
	fVar8 = fVar8 * (fVar7 / this->field_0x214);
	local_8.field_0x0 = FloatToCoordinate(fVar8);

	if (((pPawn->field_0x30).field_0x0 != (pPawn->coordinates).field_0x0) ||
		((pPawn->field_0x30).field_0x4 != (pPawn->coordinates).field_0x4)) {
		(pPawn->field_0x30).field_0x0 = (pPawn->coordinates).field_0x0;
		(pPawn->field_0x30).field_0x4 = (pPawn->coordinates).field_0x4;
		pPawn->field_0x28 = local_8;

		if (this->nbColumns <= (pPawn->field_0x28).field_0x0) {
			(pPawn->field_0x28).field_0x0 = this->nbColumns - 1;
		}

		pPawn->field_0x38 = pPawn->field_0x28;

		pPawn->field_0x40.field_0x0 = (pPawn->coordinates).field_0x0 - (pPawn->field_0x28).field_0x0;
		pPawn->field_0x40.field_0x4 = (pPawn->coordinates).field_0x4 - (pPawn->field_0x28).field_0x4;

		if ((float)(this->nbRows >> 1) < fabs((float)pPawn->field_0x40.field_0x4)) {
			iVar4 = 1;

			if ((int)pPawn->field_0x40.field_0x4 < 1) {
				iVar4 = -1;
			}

			pPawn->field_0x40.field_0x4 = pPawn->field_0x40.field_0x4 - this->nbRows * iVar4;
		}

		pPawn->field_0x48 = 0;
	}

	if ((local_8.field_0x0 == (pPawn->coordinates).field_0x0) && (local_8.field_0x4 == (pPawn->coordinates).field_0x4)) {
		bVar3 = true;
		pPawn->field_0x28 = pPawn->coordinates;
	}
	else {
		if ((local_8.field_0x0 == (pPawn->field_0x28).field_0x0) && (local_8.field_0x4 == (pPawn->field_0x28).field_0x4)) {
			do {
				pPawn->field_0x48 = pPawn->field_0x48 + 1;
				uVar6 = pPawn->field_0x40.field_0x4;
				if ((int)uVar6 < 1) {
					uVar6 = -uVar6;
				}

				uVar5 = pPawn->field_0x40.field_0x0;
				if ((int)uVar5 < 1) {
					uVar5 = -uVar5;
				}

				if ((int)uVar6 < (int)uVar5) {
					uVar6 = pPawn->field_0x40.field_0x0;
					if ((int)uVar6 < 1) {
						uVar6 = -uVar6;
					}
				}
				else {
					uVar6 = pPawn->field_0x40.field_0x4;
					if ((int)uVar6 < 1) {
						uVar6 = -uVar6;
					}
				}

				fVar7 = CoordinateToFloat(pPawn->field_0x48);

				uVar5 = pPawn->field_0x38.field_0x4 + (int)((float)pPawn->field_0x40.field_0x4 * (fVar7 / (float)uVar6));
				if ((int)uVar5 < 0) {
					fVar8 = CoordinateToFloat(this->nbRows);
					uVar5 = (uint)((float)uVar5 + fVar8);
				}

				uVar2 = this->nbRows;
				if (uVar2 <= uVar5) {
					fVar8 = CoordinateToFloat(uVar2);
					uVar5 = (uint)((float)uVar5 - fVar8);
				}

				(pPawn->field_0x28).field_0x0 = pPawn->field_0x38.field_0x0 + (int)((float)pPawn->field_0x40.field_0x0 * (fVar7 / (float)uVar6));
				(pPawn->field_0x28).field_0x4 = uVar5;

				uVar5 = 0xffffffff;
				for (uVar6 = 0; uVar6 < 2; uVar6 = uVar6 + 1) {
					if ((this->readLayerMask & 1 << (uVar6 & 0x1f)) != 0) {
						uVar5 = uVar5 & this->aSubObjs[uVar6][(pPawn->field_0x28).field_0x0];
					}
				}
			} while (((uVar5 >> ((pPawn->field_0x28).field_0x4 & 0x1f) & 1) == 0) &&
				(((pPawn->field_0x28).field_0x0 != (pPawn->coordinates).field_0x0 ||
					((pPawn->field_0x28).field_0x4 != (pPawn->coordinates).field_0x4))));
		}

		bVar3 = false;
	}

	return bVar3;
}
