#ifndef _COLLISION_MANAGER_H
#define _COLLISION_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct CCollisionManager : public CObjectManager {
	void Level_Create(ByteCode* pMemoryStream);

	int field_0x4;
};

#endif // _COLLISION_MANAGER_H
