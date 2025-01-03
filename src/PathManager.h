#ifndef _PATH_MANAGER_H
#define _PATH_MANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "MemoryStream.h"
#include "PathFollow.h"
#include "PathFinder.h"

struct CPathManager : public CObjectManager{
	CPathManager();
	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);
	virtual void Level_ClearAll();
	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();
	CPathFollow* aPathFollow;
	undefined4 pathFollowCount;
	CBasicPathFinder* pBasicPathFinder;
};

#endif // _PATH_MANAGER_H
