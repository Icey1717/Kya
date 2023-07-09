#ifndef WAYPOINT_H
#define WAYPOINT_H

#include "Types.h"
#include "LargeObject.h"

struct CWayPointManager : public Manager {
	CWayPointManager();

	virtual void Level_Init() {}
	virtual void Level_Term() {}
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);


	struct ManagerFunctionData* pManagerFunctionData;
	struct ManagerC_24* field_0x4;
	int count_0x8;
};

#endif // WAYPOINT_H