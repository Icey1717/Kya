#ifndef SQUAD_H
#define SQUAD_H

#include "Types.h"
#include "ChessBoard.h"

class CActorWolfen;
class CActorFighter;

struct ByteCode;

class CSquadConfig
{
public:
	void Create(ByteCode* pByteCode);

	uint field_0x0[2];
};

class CTeamElt
{
public:
	CTeamElt();
	int field_0x0;
	int field_0x4;
	int pawnId;
	int field_0x14;
	int enemyIndex;
	CActorFighter* pEnemyActor;
};

struct SquadSemaphore
{
	CActorWolfen* pWolfen;
	CActorWolfen* field_0x4;
	undefined4 field_0x8;
	uint flags;
	float duration;
};

struct SquadSemaphoreManager
{
	void ManageSemaphore();

	undefined4 field_0x0;
	float field_0x4;
	int nbSemaphores;
	uint field_0xc;

	SquadSemaphore aSemaphores[5];
};

class CSquad
{
public:
	void Create(ByteCode* pByteCode);
	void Term();

	void Clear();

	int NbElt();

	void ClearSquadDeadAndNonFightingElt();

	void RemoveFighter(CTeamElt* pTeamElt);

	void ComputeBoundingSphere(ed_Bound_Sphere* pBoundSphere);
	void InitSemaphores(CSquadConfig* pConfig);
	void SynchronizePattern(CChessBoardPawnsRefTable* pTable);
	bool AddFighter(CTeamElt* pTeamElt);
	void ManageSemaphore(int index);
	void SynchronizePawns();

	CFixedTable<CTeamElt*, 5> eltTable;
	CChessBoard chessboard;

	SquadSemaphoreManager aSemaphores[2];
};

#endif // SQUAD_H