#ifndef PATH_FINDER_CLIENT_H
#define PATH_FINDER_CLIENT_H

#include "Types.h"
#include "PathDynamic.h"

class CActorMovable;

class CPathFinderClient
{
public:
	void Init();
	void ChangePathfindingId(CActorMovable* pActor, int newId, edF32VECTOR4* pLocation);
	void CleanPathDynamic();

	bool IsValidPosition(edF32VECTOR4* pPosition);

	int id;
	CPathDynamic* pPathDynamic;
	undefined4 field_0x8;

	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
};

#endif // PATH_FINDER_CLIENT_H