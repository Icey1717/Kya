#ifndef PATH_FINDER_CLIENT_H
#define PATH_FINDER_CLIENT_H

#include "Types.h"
#include "PathDynamic.h"

class CActorMovable;

class CPathFinderClient
{
public:
	CPathFinderClient();

	void Init();
	void ChangePathfindingId(CActorMovable* pActor, int newId, edF32VECTOR4* pLocation);
	void CleanPathDynamic();
	bool FindPathDynamic(CActorMovable* pActor, edF32VECTOR4* pGoalPosition);

	bool IsValidPosition(edF32VECTOR4* pPosition);

	bool HasPathTo(CActorMovable* pActor, edF32VECTOR4* v0);

	int CheckAndUpdatePathDynamic(CActor* pActor, edF32VECTOR4* param_3);
	void ComputeSubTargetPathDynamic(CActor* pActor, edF32VECTOR4* param_3);

	int id;
	CPathDynamic* pPathDynamic;
	int naviMeshIndex;

	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
};

#endif // PATH_FINDER_CLIENT_H