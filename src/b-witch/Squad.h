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
	void EnableFightAction(int index);
	void DisableFightAction();

	int field_0x0;
	int field_0x4;
	int pawnId;
	int field_0x14;
	int enemyIndex;
	CActorFighter* pEnemyActor;
};

struct SquadSemaphore
{
	CTeamElt* pTeamElt;
	CTeamElt* field_0x4;
	float field_0x8;
	uint flags;
	float duration;
	float field_0x14;
};

struct SquadSemaphoreManager
{
	void ManageSemaphore();
	uint GetToken(CTeamElt* pTeamElt, int param_3);
	uint ForceGetToken(CTeamElt* pTeamElt, int param_3);

	int field_0x0;
	float field_0x4;
	int nbSemaphores;
	uint field_0xc;

	SquadSemaphore aSemaphores[5];
};

typedef CFixedTable<CTeamElt*, 5> CSquadTeam;

class CSquad
{
public:
	void Create(ByteCode* pByteCode);
	void Term();

	void Clear();

	int NbElt();

	void ClearSquadDeadAndNonFightingElt();

	void RemoveFighter(CTeamElt* pTeamElt);
	bool RemoveElt(CTeamElt* pTeamElt);

	void ComputeBoundingSphere(ed_Bound_Sphere* pBoundSphere);
	void InitSemaphores(CSquadConfig* pConfig);
	void SynchronizePattern(CChessBoardPawnsRefTable* pTable);
	bool AddFighter(CTeamElt* pTeamElt);
	void ManageSemaphore(int index);
	void SynchronizePawns();

	void ForceReleaseSemaphore(int index, CTeamElt* pTeamElt, bool param_4);
	bool QuerySemaphoreCold(int index, CTeamElt* pTeamElt);
	bool QuerySemaphoreWarm(int index, CTeamElt* pTeamElt);

	CSquadTeam eltTable;
	CChessBoard chessboard;

	SquadSemaphoreManager aSemaphores[2];
};

#endif // SQUAD_H