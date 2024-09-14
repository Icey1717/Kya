#ifndef _PATH_MANAGER_H
#define _PATH_MANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "MemoryStream.h"
#include "PathFollow.h"

struct CPathManager : public CObjectManager{
	CPathManager();
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);
	CPathFollow* aPathFollow;
	undefined4 pathFollowCount;
	int* field_0xc;
};

#endif // _PATH_MANAGER_H
