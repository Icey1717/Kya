#ifndef _PATH_MANAGER_H
#define _PATH_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct CPathManager : public Manager{
	CPathManager();
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);
	struct Actor_3C* field_0x4;
	undefined4 count_0x8;
	int* field_0xc;
};

#endif // _PATH_MANAGER_H
