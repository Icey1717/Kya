#ifndef _SCENARICCONDITION_H
#define _SCENARICCONDITION_H

#include "Types.h"

typedef int SCENARIC_VARIABLE;

// PS2 condition codes: CScenaricCondition::IsVerified (0x001175e0).
#define SCENARIC_COMBINE_AND 0
#define SCENARIC_COMBINE_OR 1
#define SCENARIC_COMBINE_XOR 2
#define SCENARIC_COMBINE_COUNT 3

#define SCENARIC_COMPARE_ALWAYS 0
#define SCENARIC_COMPARE_LESS 1
#define SCENARIC_COMPARE_LESS_EQUAL 2
#define SCENARIC_COMPARE_EQUAL 3
#define SCENARIC_COMPARE_GREATER_EQUAL 4
#define SCENARIC_COMPARE_GREATER 5
#define SCENARIC_COMPARE_NOT_EQUAL 6
#define SCENARIC_COMPARE_COUNT 7

// PS2 operation codes: S_STREAM_SIMPLE_OPERATION::Perform (0x00117510).
#define SCENARIC_OP_ASSIGN 0
#define SCENARIC_OP_ADD 1
#define SCENARIC_OP_SUBTRACT 2
#define SCENARIC_OP_MULTIPLY 3
#define SCENARIC_OP_DIVIDE 4

struct S_STREAM_SIMPLE_COND {
	// PS2 IsVerified (0x001175e0): combine the comparison with the accumulated result.
	// SCENARIC_COMBINE_*; unknown codes replace the accumulated result.
	// AND requires both true, OR either true, XOR exactly one true.
	int field_0x0;
	SCENARIC_VARIABLE variable;
	// SCENARIC_COMPARE_*: compare the current scenario value to field_0xc;
	// unknown codes also yield true.
	int field_0x8;
	// Data-defined integer threshold/state; 0/1/2 do not have universal story meanings.
	int field_0xc;
};

static_assert(sizeof(S_STREAM_SIMPLE_COND) == 0x10);

struct CScenaricCondition
{
	CScenaricCondition() : pData((int*)0x0) {}

	int* pData;

	void Create(int* param_2);
	void Create(struct ByteCode* pByteCode);
	ulong IsVerified();
	static ulong IsVerified(S_STREAM_SIMPLE_COND* pCond, int param_2);
	int GetNumSimpleConds();
	int GetDataSize();
	int* GetBeginPtr();
	int* GetEndPtr();
	void Invalidate();
};

struct CND_OP_HEADER {
	int cndOpCount;
};

static_assert(sizeof(CND_OP_HEADER) == 0x4);

struct COND_HEADER {
	int condCount;
	S_STREAM_SIMPLE_COND aConds[];
};

static_assert(sizeof(COND_HEADER) == 0x4);

struct S_STREAM_SIMPLE_OPERATION {
	int scenVarId;
	// SCENARIC_OP_*; division uses signed integers (zero divisor traps).
	int opType;
	int value;

	void Perform();
};

static_assert(sizeof(S_STREAM_SIMPLE_OPERATION) == 0xc);

struct OP_HEADER {
	int opCount;
	S_STREAM_SIMPLE_OPERATION aOps[];
};

static_assert(sizeof(OP_HEADER) == 0x4);

struct ConditionedOperationArray {
	ConditionedOperationArray() : pHeader((CND_OP_HEADER*)0x0) {}

	CND_OP_HEADER* pHeader;
	void Create(struct ByteCode* pByteCode);
	void Perform();
	void Invalidate();
};

#endif // _SCENARICCONDITION_H
