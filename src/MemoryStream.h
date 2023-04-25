#ifndef _MEMORYSTREAM_H
#define _MEMORYSTREAM_H

#include "Types.h"

struct MemoryStream {
public:
	void SeekForward_00189a50(int param_2);
	undefined4 Func_00189a90();

	char* ReadString_00189ab0();
	ulong ReadLongFunc_00189b00();
	float ReadFloat_00189b30();
	uint ReadUint_00189b50();
	int ReadInt_00189b70();
	byte ReadByte_00189bb0();
	char* GetSeekPos();
	void SetSeekPos(char* newPos);
	char* Setup_00189c00(char* fileBuffer);
	MemoryStream();
	char* data;
	char* streamEnd;
	int size;
	char* currentSeekPos;
};
#endif // _MEMORYSTREAM_H
