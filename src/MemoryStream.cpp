#include "MemoryStream.h"

#include <assert.h>

ulong ByteCode::BuildU64(int param_1, int param_2)
{
	return (ulong)param_1 & 0xffffffffU | (ulong)param_2 << 0x20;
}

void ByteCode::Align(int align)
{
	this->currentSeekPos = this->currentSeekPos + align + -1;
	if (align == 0) {
		IMPLEMENTATION_GUARD();
	}
	this->currentSeekPos = this->currentSeekPos + -((ulong)this->currentSeekPos % align);
	return;
}

undefined4 ByteCode::GetChunk()
{
	undefined4 uVar1;

	uVar1 = *(undefined4*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	this->currentSeekPos = this->currentSeekPos + 4;
	return uVar1;
}

char* ByteCode::GetString()
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

ulong ByteCode::GetU64()
{
	undefined4 uVar1;
	undefined4 uVar2;

	uVar1 = *(undefined4*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	uVar2 = *(undefined4*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	return (ulong)uVar2 << 32 | (ulong)uVar1;
}

float ByteCode::GetF32()
{
	float fVar1;

	fVar1 = *(float*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + 4;
	return fVar1;
}

uint ByteCode::GetU32()
{
	uint uVar1;

	uVar1 = *(uint*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + sizeof(uint);
	return uVar1;
}

int ByteCode::GetS32()
{
	int iVar1;

	iVar1 = *(int*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + sizeof(int);
	return iVar1;
}

ushort ByteCode::GetU16()
{
	ushort uVar1;

	uVar1 = *(ushort*)this->currentSeekPos;
	this->currentSeekPos = this->currentSeekPos + sizeof(ushort);
	return uVar1;
}

byte ByteCode::GetU8()
{
	byte* pbVar1;

	pbVar1 = (byte*)this->currentSeekPos;
	this->currentSeekPos = (char*)(pbVar1 + 1);
	return *pbVar1;
}

char* ByteCode::GetPosition()
{
	return this->currentSeekPos;
}

void ByteCode::SetPosition(char* newPos)
{
	this->currentSeekPos = newPos;
	return;
}

void ByteCode::Term(void)
{
	return;
}

char* ByteCode::Init(char* fileBuffer)
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

ByteCode::ByteCode()
{
	data = (char*)0x0;
	size = 0;
	currentSeekPos = (char*)0x0;
	return;
}
