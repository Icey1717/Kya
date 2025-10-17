#include "Path.h"

CPathNode::CPathNode()
{
	this->field_0x0 = 0;
	this->field_0x4 = -1;
	this->field_0x8 = (CPathNode*)0x0;
	this->field_0xc = 0.0f;
	this->field_0x10 = 0.0f;
	this->pPrev = (CPathNode*)0x0;
	this->pNext = (CPathNode*)0x0;

	return;
}

void CPathNode::Reset()
{
	this->field_0x0 = 0;
	this->field_0x4 = -1;
	this->field_0x8 = (CPathNode*)0x0;
	this->field_0xc = 0.0f;
	this->field_0x10 = 0.0f;
	this->pPrev = (CPathNode*)0x0;
	this->pNext = (CPathNode*)0x0;

	return;
}
