#ifndef _MEMORYSTREAM_H
#define _MEMORYSTREAM_H

#include "Types.h"

struct ByteCode {
public:
	static ulong BuildU64(uint param_1, uint param_2);
	void Align(int param_2);
	undefined4 GetChunk();

	char* GetString();
	ulong GetU64();
	float GetF32();
	uint GetU32();
	int GetS32();
	ushort GetU16();
	byte GetU8();
	char* GetPosition();
	void SetPosition(char* newPos);
	void Term(void);
	char* Init(char* fileBuffer);
	ByteCode();

	char* data;
	char* streamEnd;
	int size;
	char* currentSeekPos;
};

#endif // _MEMORYSTREAM_H
