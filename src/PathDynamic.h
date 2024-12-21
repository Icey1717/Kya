#ifndef PATH_DYNAMIC_H
#define PATH_DYNAMIC_H

#include "Types.h"
#include "Path.h"

class CPathDynamic : CPath
{
public:
	CPathDynamic();

	virtual void Init(edF32VECTOR4* pStart, edF32VECTOR4* pDestination);
	virtual void SetStatus(int newStatus);
	virtual void Clear();

	int status;

	CPathDynamic* pNext;

	CPathNode* pPathNode;

	edF32VECTOR4 destinationPosition;
	edF32VECTOR4 startPosition;

	byte field_0x30;

	undefined4 field_0x54;
};

#endif // PATH_DYNAMIC_H