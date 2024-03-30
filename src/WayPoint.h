#ifndef WAYPOINT_H
#define WAYPOINT_H

#include "Types.h"
#include "LargeObject.h"

PACK(
class CWayPoint {
public:
	edF32VECTOR3 field_0x0;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
});

static_assert(sizeof(CWayPoint) == 0x24, "CWaypoint size mismatch");

struct CWayPointManager : public CObjectManager {
	CWayPointManager();

	virtual void Level_Init() {}
	virtual void Level_Term() {}
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);

	CWayPoint* aWaypoints;
	int waypointCount;
};

#endif // WAYPOINT_H