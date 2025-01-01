#include "NaviCell.h"

CBasicNaviCell::CBasicNaviCell()
{
	this->pPathNode = (CPathNode*)0x0;

	this->nbSubDataB = 0;
	this->pDataB = (ushort*)0x0;

	this->nbSubDataA = 0;
	this->pDataA = (ushort*)0x0;

	return;
}

void CBasicNaviCell::Create(ByteCode* pByteCode)
{
	this->nbSubDataB = pByteCode->GetU16();
	this->nbSubDataA = pByteCode->GetU16();

	return;
}

