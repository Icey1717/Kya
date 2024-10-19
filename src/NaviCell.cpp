#include "NaviCell.h"

void CBasicNaviCell::Create(ByteCode* pByteCode)
{
	this->field_0x4 = pByteCode->GetU16();
	this->field_0xc = pByteCode->GetU16();

	return;
}

