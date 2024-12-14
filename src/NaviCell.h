#ifndef NAVI_CELL_H
#define NAVI_CELL_H

#include "Types.h"
#include "MemoryStream.h"

class CBasicNaviCell
{
public:
	void Create(ByteCode* pByteCode);

	uint nbSubDataA;
	ushort* pDataA;

	uint nbSubDataB;
	ushort* pDataB;

	edF32VECTOR2 field_0x14;

	edF32VECTOR4 field_0x20;
};

#endif // NAVI_CELL_H