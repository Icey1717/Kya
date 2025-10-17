#include "PathDynamic.h"
#include "MathOps.h"

CPathDynamic::CPathDynamic()
{
	this->pPathNode = (CPathNode*)0x0;
	this->field_0x54 = (CPathNode*)0x0;
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

edF32VECTOR4* CPathDynamic::GetStart()
{
	return &this->startPosition;
}

void CPathDynamic::NextWayPoint()
{
	this->field_0x54 = this->field_0x54->pNext;

	return;
}

bool CPathDynamic::AtGoal()
{
	return this->field_0x54->pNext == (CPathNode*)0x0;
}

int CPathDynamic::GetStatus()
{
	return this->status;
}

void CPathDynamic::SetStatus(int newStatus)
{
	this->status = newStatus;
	return;
}

edF32VECTOR4* CPathDynamic::GetGoalB()
{
	return &this->destinationPosition;
}

void CPathDynamic::Reset()
{
	this->field_0x54 = this->pPathNode;
	return;
}

void CPathDynamic::Clear()
{
	this->pPathNode = (CPathNode*)0x0;

	SetStatus(5);

	this->startPosition = gF32Vector4Zero;
	this->destinationPosition = gF32Vector4Zero;
	this->field_0x54 = (CPathNode*)0x0;
	this->field_0x30 = this->field_0x30 & 0xfe;
	return;
}

CPathNode* CPathDynamic::GetLastData()
{
	CPathNode* pCVar1;
	CPathNode* pCVar2;

	pCVar1 = this->pPathNode;
	do {
		pCVar2 = pCVar1;
		pCVar1 = pCVar2->pNext;
	} while (pCVar1 != (CPathNode*)0x0);

	return pCVar2;
}