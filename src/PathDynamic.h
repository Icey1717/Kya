#ifndef PATH_DYNAMIC_H
#define PATH_DYNAMIC_H

#include "Types.h"
#include "Path.h"

class CPathDynamic : CPath
{
public:
	CPathDynamic();

	virtual void Init(edF32VECTOR4* pStart, edF32VECTOR4* pDestination);
	virtual edF32VECTOR4* GetStart();
	virtual void NextWayPoint();
	virtual bool AtGoal();
	virtual int GetStatus();
	virtual void SetStatus(int newStatus);
	virtual edF32VECTOR4* GetGoalB();
	virtual void Reset();
	virtual void Clear();

	CPathNode* GetLastData();

	int status;
	int naviMeshIndex;

	CPathDynamic* pNext;

	CPathNode* pPathNode;

	edF32VECTOR4 destinationPosition;
	edF32VECTOR4 startPosition;

	union {
		byte field_0x30;
		uint field_0x30_uint;
	};

	edF32VECTOR4 field_0x40;

	CPathNode* field_0x54;
};

#endif // PATH_DYNAMIC_H