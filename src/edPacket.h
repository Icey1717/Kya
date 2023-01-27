#ifndef _EDPACKET_H
#define _EDPACKET_H

#include "Types.h"

struct edPacket
{
	ulonglong* seekPos;
	ulonglong* pBuffer;
	uint totalSize;
	short* field_0xc;
	int field_0x10;
	int size_0x14;
};

void edPacketInit(edPacket* pedPacket, uint packetSize, ulonglong* pBuffer);
void edPacketFunc_0026a2b0(edPacket* pedPacket, uint param_2);
void edPacketFunc_00269aa0(edPacket* pedPacket);
void edPacketFunc_00269bb0(edPacket* pedPacket);
void edPacketWriteLong_0026a000(edPacket* pedPacket, ulonglong value);
void edPacketFunc_0026a440(edPacket* pedPacket, long param_2, uint param_3, uint param_4);
void edPacketFunc_0026a6d0(edPacket* pedPacket, uint param_2);
void edPacketClose_00269e70(edPacket* pedPacket);

#endif //_EDPACKET_H
