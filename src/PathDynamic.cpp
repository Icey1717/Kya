#include "PathDynamic.h"
#include "MathOps.h"

CPathDynamic::CPathDynamic()
{
	this->pPathNode = (CPathNode*)0x0;
	this->field_0x54 = 0;
	this->pNext = (CPathDynamic*)0x0;
	this->status = 5;
	this->field_0x30 = this->field_0x30 & 0xfe;
}

void CPathDynamic::Init(edF32VECTOR4* pStart, edF32VECTOR4* pDestination)
{
	this->destinationPosition = *pDestination;
	this->startPosition = *pStart;

	SetStatus(2);

	return;
}

void CPathDynamic::SetStatus(int newStatus)
{
	this->status = newStatus;
	return;
}

void CPathDynamic::Clear()
{
	this->pPathNode = (CPathNode*)0x0;
	SetStatus(5);

	this->startPosition = gF32Vector4Zero;
	this->destinationPosition = gF32Vector4Zero;
	this->field_0x54 = 0;
	this->field_0x30 = this->field_0x30 & 0xfe;
	return;
}
