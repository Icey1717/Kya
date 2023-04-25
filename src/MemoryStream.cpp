#include "MemoryStream.h"

#include <assert.h>

void MemoryStream::SeekForward_00189a50(int param_2)
{
	this->currentSeekPos = this->currentSeekPos + param_2 + -1;
	if (param_2 == 0) {
		assert(false);
	}
	this->currentSeekPos = this->currentSeekPos + -((ulong)this->currentSeekPos % param_2);
	return;
}

undefined4 MemoryStream::Func_00189a90()
{
	undefined4 uVar1;

	uVar1 = *(undefined4*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	this->currentSeekPos = this->currentSeekPos + 4;
	return uVar1;
}

char* MemoryStream::ReadString_00189ab0()
{
	char* pcVar1;
	char* pcVar2;

	pcVar1 = this->currentSeekPos;
	do {
		pcVar2 = this->currentSeekPos;
		this->currentSeekPos = pcVar2 + 1;
	} while (*pcVar2 != '\0');
	this->currentSeekPos = this->currentSeekPos + 3;
	this->currentSeekPos = this->currentSeekPos + -((ulong)this->currentSeekPos & 3);
	return pcVar1;
}

ulong MemoryStream::ReadLongFunc_00189b00()
{
	undefined4 uVar1;
	undefined4 uVar2;

	uVar1 = *(undefined4*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	uVar2 = *(undefined4*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	return (ulong)uVar2 << 32 | (ulong)uVar1;
}

float MemoryStream::ReadFloat_00189b30()
{
	float fVar1;

	fVar1 = *(float*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	return fVar1;
}

uint MemoryStream::ReadUint_00189b50()
{
	uint uVar1;

	uVar1 = *(uint*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	return uVar1;
}

int MemoryStream::ReadInt_00189b70()
{
	int iVar1;

	iVar1 = *(int*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	return iVar1;
}

// GAP

byte MemoryStream::ReadByte_00189bb0()
{
	byte* pbVar1;

	pbVar1 = (byte*)this->currentSeekPos;
	this->currentSeekPos = (char*)(pbVar1 + 1);
	return *pbVar1;
}

char* MemoryStream::GetSeekPos()
{
	return this->currentSeekPos;
}

void MemoryStream::SetSeekPos(char* newPos)
{
	this->currentSeekPos = newPos;
	return;
}

char* MemoryStream::Setup_00189c00(char* fileBuffer)
{
	char* pcVar1;

	this->data = fileBuffer;
	this->size = *(int*)(this->data + 4);
	pcVar1 = this->data + 8;
	this->streamEnd = this->data + this->size;
	this->data = pcVar1;
	this->currentSeekPos = pcVar1;
	return this->currentSeekPos;
}

MemoryStream::MemoryStream()
{
	data = (char*)0x0;
	size = 0;
	currentSeekPos = (char*)0x0;
	return;
}