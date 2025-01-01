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
	virtual int GetStatus();
	virtual void SetStatus(int newStatus);
	virtual edF32VECTOR4* GetGoalB();
	virtual void Reset();
	virtual void Clear();

	int status;
	int naviMeshIndex;

	CPathDynamic* pNext;

	CPathNode* pPathNode;

	edF32VECTOR4 destinationPosition;
	edF32VECTOR4 startPosition;

	byte field_0x30;

	CPathNode* field_0x54;
};

#endif // PATH_DYNAMIC_H