#ifndef PATH_DYNAMIC_H
#define PATH_DYNAMIC_H

#include "Types.h"
#include "Path.h"

class CPathDynamic : CPath
{
public:
	CPathDynamic();

	virtual void SetStatus(int newStatus);
	virtual void Clear();

	int status;

	CPathDynamic* pNext;

	CPathNode* pPathNode;

	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;

	byte field_0x30;

	undefined4 field_0x54;
};

#endif // PATH_DYNAMIC_H