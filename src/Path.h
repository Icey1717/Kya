#ifndef PATH_H
#define PATH_H

#include "Types.h"

class CPath {
};

class CPathNode
{
public:
	CPathNode();

	undefined4 field_0x0;
	short field_0x4;
	undefined field_0x6;
	undefined field_0x7;
	undefined4 field_0x8;
	undefined4 field_0xc;
	undefined4 field_0x10;
	undefined4 field_0x14;
	CPathNode* pNext;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

#endif // !PATH_H
