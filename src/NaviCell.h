#ifndef NAVI_CELL_H
#define NAVI_CELL_H

#include "Types.h"
#include "MemoryStream.h"

class CBasicNaviCell
{
public:
	void Create(ByteCode* pByteCode);

	uint field_0x4;
	ushort* field_0x8;
	uint field_0xc;
	ushort* field_0x10;

	edF32VECTOR2 field_0x14;

	edF32VECTOR4 field_0x20;
};

#endif // NAVI_CELL_H