#ifndef FX_TAIL_H
#define FX_TAIL_H

#include "Types.h"

class CFxTail : public CObject
{
public:
	CFxTail() : pData_0x18((void*)0x0) {}

	virtual bool IsKindOfObject(ulong kind);
	virtual bool InitDlistPatchable(int);

	void Create(float param_1, int count, int param_4, int materialId);
	void Init(float param_1, int id);

	void Reset();

	void ChangeOrder(char* pNewOrder);

	bool SetPatchActive(int bActive);

	bool Manage(edF32VECTOR4* param_2, edF32VECTOR4* param_3, int param_4);

	int dlistPatchId;

	uint flags;

	int particleID_0x14;

	void* pData_0x18;

	_rgba field_0x24;
	_rgba field_0x28;

	float field_0x30;
	int count_0x34;

	uint field_0xb0;
	float field_0xb8;
	float field_0xbc;

	float field_0xc0;
	float field_0xc4;

	char szOrder[4];
};

#endif // FX_TAIL_H