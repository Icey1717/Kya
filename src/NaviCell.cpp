#include "NaviCell.h"

void CBasicNaviCell::Create(ByteCode* pByteCode)
{
	this->nbSubDataB = pByteCode->GetU16();
	this->nbSubDataA = pByteCode->GetU16();

	return;
}

