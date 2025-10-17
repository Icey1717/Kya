#ifndef WAYPOINT_H
#define WAYPOINT_H

#include "Types.h"
#include "LargeObject.h"

PACK(
class CWayPoint {
public:
	edF32VECTOR3 location;
	edF32VECTOR3 rotation;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
});

static_assert(sizeof(CWayPoint) == 0x24, "CWaypoint size mismatch");

struct CWayPointManager : public CObjectManager
{
	CWayPointManager();

	// CObjectManager
	virtual void Level_Init() {}
	virtual void Level_Term() {}
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);
	virtual void Level_ClearAll();

	virtual void Level_Manage() {}
	virtual void Level_ManagePaused() {}
	virtual void Level_Draw() {}

#ifdef PLATFORM_WIN
	virtual char* ProfileGetName() { return "Waypoint"; }
#endif

	CWayPoint* aWaypoints;
	int waypointCount;
};

#endif // WAYPOINT_H