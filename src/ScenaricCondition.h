#ifndef _SCENARICCONDITION_H
#define _SCENARICCONDITION_H

#include "Types.h"

struct ScenaricCondition {
	char* field_0x0;
	void Create(char* param_2);
	void Create(struct ByteCode* pByteCode);
	ulong IsVerified();
	ulong IsVerified(struct S_STREAM_SIMPLE_COND* pCond, ulong param_2);
	int GetNumSimpleConds();
	int GetDataSize();
	uint* GetEndPtr();
};

#endif // _SCENARICCONDITION_H
