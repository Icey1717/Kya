#include "edPacket.h"
#include <stdio.h>
#include <string.h>

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif

#if defined(PLATFORM_PS2)
#define trap(...) __asm__ ( "break" )
#else
#define trap(...)
#endif

void edPacketInit(edPacket* pPacket, uint packetSize, ulonglong* pBuffer)
{
	if ((packetSize & 0xf) != 0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0xb1, " size must be qword \n");
		if ((packetSize & 0xf) != 0) {
			trap(0);
		}
	}
	if (pBuffer == (ulonglong*)0x0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0xb2, " edPacketInit invalid ptr \n");
		if (true) {
			trap(0);
		}
	}
	pPacket->pBuffer = pBuffer;
	pPacket->totalSize = (int)pPacket->pBuffer + packetSize;
	if (pPacket == (edPacket*)0x0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x81, "");
		if (true) {
			trap(0);
		}
	}
	pPacket->seekPos = pPacket->pBuffer;
	pPacket->field_0xc = (short*)0x0;
	pPacket->field_0x10 = 0;
	return;
}

void edPacketFunc_0026a2b0(edPacket* pPacket, uint param_2)
{
	undefined4* puVar1;
	undefined4 in_zero_lo;
	undefined4 in_zero_hi;
	undefined4 in_zero_udw;
	undefined4 in_register_0000000c;

	if (pPacket == (edPacket*)0x0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x1a0, "");
		if (true) {
			trap(0);
		}
	}
	edPacketFunc_00269aa0(pPacket);
	edPacketFunc_00269bb0(pPacket);
	if ((ulonglong*)(pPacket->totalSize - 0x10) < pPacket->seekPos) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x65, " packet s buffer full \n");
		if ((ulonglong*)(pPacket->totalSize - 0x10) < pPacket->seekPos) {
			trap(0);
		}
	}
	pPacket->field_0xc = (short*)pPacket->seekPos;
	puVar1 = (undefined4*)pPacket->field_0xc;
	*puVar1 = 0x0;
	puVar1[1] = 0x0;
	puVar1[2] = 0x0;
	puVar1[3] = 0x0;
	*(undefined4*)(pPacket->field_0xc + 6) = 0x99999999;
	*(undefined4*)(pPacket->field_0xc + 4) = 0x99999999;
	*(byte*)((int)pPacket->field_0xc + 3) = *(byte*)((int)pPacket->field_0xc + 3) & 0x8f | 0x10;
	*(byte*)((int)pPacket->field_0xc + 3) = *(byte*)((int)pPacket->field_0xc + 3) & 0x7f | (byte)((param_2 & 1) << 7);
	pPacket->seekPos = pPacket->seekPos + 2;
	return;
}

void edPacketFunc_00269aa0(edPacket* pPacket)
{
	uint len;
	ulonglong uStack64[10] = { 0 };

	len = (uint)pPacket->seekPos & 0xf;
	if (len != 0) {
		len = 0x10 - len;

		if ((ulonglong*)(pPacket->totalSize - len) < pPacket->seekPos) {
			//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
			PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x21, "");
			if ((ulonglong*)(pPacket->totalSize - len) < pPacket->seekPos) {
				trap(0);
			}
		}
		memcpy(pPacket->seekPos, &uStack64, (long)(int)len);
		pPacket->seekPos = (ulonglong*)((int)pPacket->seekPos + (len & 0xfffffffc));
	}
	return;
}

void edPacketFunc_00269bb0(edPacket* pPacket)
{
	short* psVar1;
	edPacket* piVar3;
	ulonglong* puVar2;
	uint uVar3;

	if (pPacket == (edPacket*)0x0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x37, "");
		if (true) {
			trap(0);
		}
	}
	psVar1 = pPacket->field_0xc;
	if (psVar1 != (short*)0x0) {
		if (*psVar1 == 0) {
			puVar2 = (ulonglong*)((int)pPacket->seekPos - (int)psVar1);
			uVar3 = (uint)puVar2 & 0xf;
			if (uVar3 != 0) {
				//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
				PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x41, " size must be qword \n");
				if (uVar3 != 0) {
					trap(0);
				}
			}
			*pPacket->field_0xc = (short)((uint)puVar2 >> 4) + -1;
		}
		if (*(int*)(pPacket->field_0xc + 4) == -0x66666667) {
			*(undefined4*)(pPacket->field_0xc + 4) = 0;
		}
		if (*(int*)(pPacket->field_0xc + 6) == -0x66666667) {
			*(undefined4*)(pPacket->field_0xc + 6) = 0;
		}
	}
	return;
}

void edPacketWriteLong_0026a000(edPacket* pPacket, ulonglong value)
{
	if (pPacket == (edPacket*)0x0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x153, "");
		if (true) {
			trap(0);
		}
	}
	if ((ulonglong*)(pPacket->totalSize - 8) < pPacket->seekPos) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x154, " packet s buffer full \n");
		if ((ulonglong*)(pPacket->totalSize - 8) < pPacket->seekPos) {
			trap(0);
		}
	}
	*pPacket->seekPos = value;
	pPacket->seekPos = pPacket->seekPos + 1;
	return;
}

void edPacketFunc_0026a440(edPacket* pPacket, long param_2, uint param_3, uint param_4)
{
	undefined4* puVar1;
	undefined4 in_zero_lo;
	undefined4 in_zero_hi;
	undefined4 in_zero_udw;
	undefined4 in_register_0000000c;
	uint uVar2;

	if (pPacket == (edPacket*)0x0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x1b4, "");
		if (true) {
			trap(0);
		}
	}
	if (param_2 == 0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x1b5, "");
		if (true) {
			trap(0);
		}
	}
	uVar2 = param_3 & 0xf;
	if (uVar2 != 0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x1b6, "");
		if (uVar2 != 0) {
			trap(0);
		}
	}
	if (uVar2 == 0) {
		edPacketFunc_00269aa0(pPacket);
		edPacketFunc_00269bb0(pPacket);
		if ((ulonglong*)(pPacket->totalSize - 0x10) < pPacket->seekPos) {
			//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
			PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x65, " packet s buffer full \n");
			if ((ulonglong*)(pPacket->totalSize - 0x10) < pPacket->seekPos) {
				trap(0);
			}
		}
		pPacket->field_0xc = (short*)pPacket->seekPos;
		puVar1 = (undefined4*)pPacket->field_0xc;
		*puVar1 = 0x0;
		puVar1[1] = 0x0;
		puVar1[2] = 0x0;
		puVar1[3] = 0x0;
		*(undefined4*)(pPacket->field_0xc + 6) = 0x99999999;
		*(undefined4*)(pPacket->field_0xc + 4) = 0x99999999;
		*(byte*)((int)pPacket->field_0xc + 3) = *(byte*)((int)pPacket->field_0xc + 3) & 0x8f | 0x30;
		*(byte*)((int)pPacket->field_0xc + 3) = *(byte*)((int)pPacket->field_0xc + 3) & 0x7f | (byte)((param_4 & 1) << 7);
		pPacket->seekPos = pPacket->seekPos + 2;
		*(uint*)(pPacket->field_0xc + 2) = *(uint*)(pPacket->field_0xc + 2) & 0x80000000 | (uint)((ulong)(param_2 << 0x21) >> 0x21);
		*pPacket->field_0xc = (short)(param_3 >> 4);
	}
	return;
}

void edPacketFunc_0026a6d0(edPacket* pPacket, uint param_2)
{
	undefined4* puVar1;
	undefined4 in_zero_lo;
	undefined4 in_zero_hi;
	undefined4 in_zero_udw;
	undefined4 in_register_0000000c;

	if (pPacket == (edPacket*)0x0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x233, "");
		if (true) {
			trap(0);
		}
	}
	edPacketFunc_00269aa0(pPacket);
	edPacketFunc_00269bb0(pPacket);
	if ((ulonglong*)(pPacket->totalSize - 0x10) < pPacket->seekPos) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x65, " packet s buffer full \n");
		if ((ulonglong*)(pPacket->totalSize - 0x10) < pPacket->seekPos) {
			trap(0);
		}
	}
	pPacket->field_0xc = (short*)pPacket->seekPos;
	puVar1 = (undefined4*)pPacket->field_0xc;
	*puVar1 = 0x0;
	puVar1[1] = 0x0;
	puVar1[2] = 0x0;
	puVar1[3] = 0x0;
	*(undefined4*)(pPacket->field_0xc + 6) = 0x99999999;
	*(undefined4*)(pPacket->field_0xc + 4) = 0x99999999;
	*(byte*)((int)pPacket->field_0xc + 3) = *(byte*)((int)pPacket->field_0xc + 3) & 0x8f | 0x70;
	*(byte*)((int)pPacket->field_0xc + 3) = *(byte*)((int)pPacket->field_0xc + 3) & 0x7f | (byte)((param_2 & 1) << 7);
	pPacket->seekPos = pPacket->seekPos + 2;
	return;
}


void edPacketClose_00269e70(edPacket* pPacket)
{
	uint uVar1;
	undefined4 local_40[32] = { 0 };
	undefined4 uStack60;
	undefined4 uStack56;
	undefined4 uStack52;
	undefined4 local_30;
	undefined4 uStack44;
	undefined4 uStack40;
	undefined4 uStack36;
	undefined4 local_20;
	undefined4 uStack28;
	undefined4 uStack24;
	undefined4 uStack20;
	ulonglong local_10;
	uint local_8;

	uVar1 = (uint)pPacket->seekPos & 0xf;
	if (uVar1 != 0) {
		uVar1 = 0x10 - uVar1;
		//local_40 = (undefined4)ULONGLONG_00482f70;
		//uStack60 = (undefined4)(ULONGLONG_00482f70 >> 0x20);
		//uStack56 = (undefined4)ULONGLONG_00482f78;
		//uStack52 = ULONGLONG_00482f78._4_4_;
		//local_30 = (undefined4)ULONGLONG_00482f80;
		//uStack44 = ULONGLONG_00482f80._4_4_;
		//uStack40 = (undefined4)ULONGLONG_00482f88;
		//uStack36 = ULONGLONG_00482f88._4_4_;
		//local_20 = (undefined4)ULONGLONG_00482f90;
		//uStack28 = ULONGLONG_00482f90._4_4_;
		//uStack24 = (undefined4)ULONGLONG_00482f98;
		//uStack20 = ULONGLONG_00482f98._4_4_;
		//local_10 = ULONGLONG_00482fa0;
		//local_8 = UINT_00482fa8;
		if ((ulonglong*)(pPacket->totalSize - uVar1) < pPacket->seekPos) {
			//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
			PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0x21, "");
			if ((ulonglong*)(pPacket->totalSize - uVar1) < pPacket->seekPos) {
				trap(0);
			}
		}
		memcpy(pPacket->seekPos, &local_40, (long)(int)uVar1);
		pPacket->seekPos = (ulonglong*)((int)pPacket->seekPos + (uVar1 & 0xfffffffc));
	}
	edPacketFunc_00269bb0(pPacket);
	pPacket->size_0x14 = (int)((int)pPacket->seekPos - (int)pPacket->pBuffer);
	if ((pPacket->size_0x14 & 0xfU) != 0) {
		//edSysHandlersCall(INT_00489154, edSysHandlersNodeParent_00489140.pTypeArray_0x4, INT_00489150, 2, 0);
		PrintString("\n%s(%i) : Error : %s\n", "edpacket.cpp", 0xde, "pb edPacketClose, unvalid size \n");
		if ((pPacket->size_0x14 & 0xfU) != 0) {
			trap(0);
		}
	}
	return;
}
