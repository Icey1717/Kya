#ifndef _PATH_MANAGER_H
#define _PATH_MANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "MemoryStream.h"

struct CPath {
	virtual void Create(ByteCode* pByteCode) = 0;
};

struct CPathFollow {
	char* field_0x0;
	uint field_0x4;
	uint field_0x8;
	uint field_0xc;
	int field_0x10;
	int field_0x14;
	uint field_0x18;
	char* field_0x1c;
	char* field_0x20;
	char* field_0x24;
	char* field_0x28;
	char* field_0x2c;
	char* field_0x30;
	char* field_0x34;
	char* field_0x38;

	virtual void Create(ByteCode* pByteCode);
};

struct CPathManager : public CObjectManager{
	CPathManager();
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);
	CPathFollow* aPathFollow;
	undefined4 pathFollowCount;
	int* field_0xc;
};

#endif // _PATH_MANAGER_H
