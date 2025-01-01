#ifndef PATH_H
#define PATH_H

#include "Types.h"

class CPath {
};

class CPathNode
{
public:
	CPathNode();

	void Reset();

	undefined4 field_0x0;
	short field_0x4;
	undefined field_0x6;
	undefined field_0x7;
	CPathNode* field_0x8;
	float field_0xc;
	float field_0x10;
	CPathNode* pPrev;
	CPathNode* pNext;
	short field_0x1c;
	undefined field_0x1e;
	undefined field_0x1f;
};

#endif // !PATH_H
