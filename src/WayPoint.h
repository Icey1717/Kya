#ifndef WAYPOINT_H
#define WAYPOINT_H

#include "Types.h"
#include "LargeObject.h"

PACK(
struct ManagerC_24 {
	byte a[0x24];
});

struct CWayPointManager : public CObjectManager {
	CWayPointManager();

	virtual void Level_Init() {}
	virtual void Level_Term() {}
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);


	struct ManagerFunctionData* pManagerFunctionData;
	ManagerC_24* field_0x4;
	int count_0x8;
};

#endif // WAYPOINT_H