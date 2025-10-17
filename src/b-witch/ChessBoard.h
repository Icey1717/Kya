#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "Types.h"

class CActorFighter;
class CPathFinderClient;

struct s_chess_board_coord
{
	int field_0x0;
	int field_0x4;
};

struct s_chess_board_pattern
{
	s_chess_board_coord aCells[6];
	uint nbCells;
};

struct s_chess_board_pawn
{
	int bActive;
	CActorFighter* pFighter;
	int field_0x8;

	edF32VECTOR4 field_0x10;

	s_chess_board_coord coordinates;
	s_chess_board_coord field_0x28;
	s_chess_board_coord field_0x30;
	s_chess_board_coord field_0x38;
	s_chess_board_coord field_0x40;

	uint field_0x48;
};

class CChessBoardPawnsRefTable : public CFixedTable<s_chess_board_pawn*, 6>
{
public:
	bool IsInList(CActor* pActor);
};

struct CChessBoardSubObj
{
	void* field_0x0[2];

	uint field_0x8;
	uint field_0xc;
};

struct CChessBoardPawnsTable
{
	int Add(CActor* pActor, int param_3);

	s_chess_board_pawn aPawns[6];
	int nbPawns;
};

class CChessBoard
{
public:
	static CChessBoardPawnsRefTable _pawnsWork;

	CChessBoard();
	void Init(float param_1, float param_2, uint param_4, uint param_5);

	void GetPawnsWithSameRingPos(CChessBoardPawnsRefTable* pTable, int ringPos);
	CActor* GetPawnActor(int index);
	edF32VECTOR4* GetPawnPosition(int index);
	void RemovePawn(int index);
	int AddPawn(CActor* pActor, int param_3);
	int GetPawnFromActor(CActor* pActor);

	int IsPawnUsed(int index);
	bool PawnShallMove(int index);

	void ValidateAll();

	void InvalidateSection(float param_1, edF32VECTOR4* param_3);
	void InvalidateByRayTrace();
	void InvalidateByZone(int zoneId);
	void InvalidateByPath(CPathFinderClient* pPathFinderClient);

	s_chess_board_coord* _FindNearestAvailableCell(s_chess_board_coord* param_2, s_chess_board_coord* param_3);
	float _PermutPawns(float param_1, CChessBoardPawnsRefTable* pTable, s_chess_board_pattern* pPattern, float* param_5, uint param_6);
	void Organise(CChessBoardPawnsRefTable* pTable);
	void UpdatePawns();
	bool MoveTo(s_chess_board_pawn* pPawn);

	int GetPawnsInSection(float param_1, edF32VECTOR4* param_3);

	uint ReadLayer(uint index);
	void WriteLayer(uint index, uint value);

	int* aSubObjs[2];

	uint nbColumns;
	uint nbRows;

	uint readLayerMask;
	uint writeLayerMask;

	CChessBoardPawnsTable pawnsTable;

	float field_0x210;
	float field_0x214;
	float field_0x218;
	float field_0x21c;

	int field_0x220;

	edF32VECTOR4 field_0x230;
	edF32VECTOR4 field_0x240;
};

#endif // CHESSBOARD_H