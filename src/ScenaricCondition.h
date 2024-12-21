#ifndef _SCENARICCONDITION_H
#define _SCENARICCONDITION_H

#include "Types.h"

typedef int SCENARIC_VARIABLE;

struct S_STREAM_SIMPLE_COND {
	int field_0x0;
	SCENARIC_VARIABLE variable;
	int field_0x8;
	int field_0xc;
};

static_assert(sizeof(S_STREAM_SIMPLE_COND) == 0x10);

struct ScenaricCondition {
	ScenaricCondition() : pData((int*)0x0) {}

	int* pData;

	void Create(int* param_2);
	void Create(struct ByteCode* pByteCode);
	ulong IsVerified();
	static ulong IsVerified(S_STREAM_SIMPLE_COND* pCond, int param_2);
	int GetNumSimpleConds();
	int GetDataSize();
	int* GetBeginPtr();
	int* GetEndPtr();
};

struct CND_OP_HEADER {
	int cndOpCount;
};

static_assert(sizeof(CND_OP_HEADER) == 0x4);

struct COND_HEADER {
	int condCount;
};

static_assert(sizeof(COND_HEADER) == 0x4);

struct OP_HEADER {
	int opCount;
};

static_assert(sizeof(OP_HEADER) == 0x4);

struct S_STREAM_SIMPLE_OPERATION {
	int scenVarId;
	int opType;
	int value;

	void Perform();
};

static_assert(sizeof(S_STREAM_SIMPLE_OPERATION) == 0xc);

struct ConditionedOperationArray {
	ConditionedOperationArray() : pHeader((CND_OP_HEADER*)0x0) {}

	CND_OP_HEADER* pHeader;
	void Create(struct ByteCode* pByteCode);
	void Perform();
};

#endif // _SCENARICCONDITION_H
