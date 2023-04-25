#include "CustomShell.h"

#if defined(PLATFORM_PS2)
#include <libdma.h>
#include <eeregs.h>
#include <devvu0.h>
#include <devvu1.h>
#include <libgraph.h>
#else
#include "Port.h"
#endif

#define CHCR_STR 0x100

DMA_Register_Struct DMA_Registers[10] =
{
	/* DMAC */
	{
		(uint*)D0_CHCR,
		0x1,
		0x1
	},

	{
		(uint*)D1_CHCR,
		0x2,
		0x41
	},

	{
		(uint*)D2_CHCR,
		0x4,
		0x80
	},

	{
		(uint*)D3_CHCR,
		0x8,
		0x1
	},

	{
		(uint*)D4_CHCR,
		0x10,
		0x0
	},

	{
		(uint*)D5_CHCR,
		0x20,
		0x0
	},

	{
		(uint*)D6_CHCR,
		0x40,
		0x0
	},

	{
		(uint*)D7_CHCR,
		0x80,
		0x0
	},

	{
		(uint*)D8_CHCR,
		0x100,
		0x0
	},

	{
		(uint*)D9_CHCR,
		0x200,
		0x0
	},
};

void Reset_00257f90(void)
{
	PrintString("#\n");
	*DMA_Registers[1].CHCR = DMA_Registers[1].QWC;
	SYNC(0);
	*DMA_Registers[2].CHCR = DMA_Registers[2].QWC;
	SYNC(0);
	*DMA_Registers[0].CHCR = DMA_Registers[0].QWC;
	SYNC(0);

#ifdef PLATFORM_PS2
	sceDmaReset(1);
	DPUT_VIF0_FBRST(9);
	SYNC(0);
	sceDevVu0Reset();
	DPUT_VIF1_FBRST(9);
	SYNC(0);
	sceDevVu1Reset();
	DPUT_GIF_CTRL(1); // RESET
	SYNC(0);
	sceGsResetPath();
#endif
	return;
}

void FUN_00258050(Vector* param_1, uint param_2, uint param_3)
{
	uint* puVar1;
	uint uVar2;

	uVar2 = DMA_Registers[8].MADR;
	puVar1 = DMA_Registers[8].CHCR;
#ifdef PLATFORM_PS2
	DMA_Registers[8].CHCR[4] = param_3;
	SYNC(0);
	puVar1[0x20] = (uint)param_1;
	SYNC(0);
	puVar1[8] = param_2 >> 4;
	SYNC(0);
	DPUT_D_STAT(uVar2);
	SYNC(0);
	*puVar1 = DMA_Registers[8].QWC | 0x100;
	SYNC(0);
#endif
	return;
}

bool FUN_002580b0(uint addr, uint qwc, uint param_3)
{
	uint* puVar1;
	uint madr;
	bool bVar2;

	madr = DMA_Registers[8].MADR;
	puVar1 = DMA_Registers[8].CHCR;
#ifdef PLATFORM_PS2
	DPUT_D_PCR(DMA_Registers[8].MADR);
	SYNC(0);
	DPUT_D_STAT(DMA_Registers[8].MADR);
	SYNC(0);
	DMA_Registers[8].CHCR[4] = param_3;
	SYNC(0);
	puVar1[0x20] = addr;
	SYNC(0);
	puVar1[8] = qwc >> 4;
	SYNC(0);
	*puVar1 = DMA_Registers[8].QWC | 0x100;
#endif
	bVar2 = MADR_Func_002586c0(madr);
	return bVar2;
}

void WaitDMA(void)
{
#ifdef PLATFORM_PS2
	FlushCache(0);
#endif
	return;
}

unsigned int GetCOUNT(void)
{
#ifdef PLATFORM_PS2
	unsigned int i;

	__asm__ volatile (" mfc0 %0, $09" : "=r"(i));

	return i;
#else
	return 0;
#endif
}

int UINT_00449180 = 0;
int UINT_00449184 = 0;

int thunk_FUN_00291be0(void)
{
	int iVar1;

	iVar1 = 0;
	if (GetCOUNT() < UINT_00449184) {
		iVar1 = -0x28000;
	}
	UINT_00449180 = (GetCOUNT() + iVar1) - UINT_00449184;
	UINT_00449184 = GetCOUNT();
	return UINT_00449180;
}


int WaitForDraw_00258230(void)
{
	char cVar1;
	uint uVar2;
	int iVar3;
	long lVar4;
	uint uVar5;

	//MY_LOG("WaitForDraw_00258230\n");

#ifdef PLATFORM_PS2
	if ((*DMA_Registers[2].CHCR & CHCR_STR) != 0) {
		thunk_FUN_00291be0();
		DPUT_D_PCR(DMA_Registers[2].MADR);
		MADR_Func_002586c0(DMA_Registers[2].MADR);
	}
	if ((*DMA_Registers[1].CHCR & CHCR_STR) != 0) {
		thunk_FUN_00291be0();
		DPUT_D_PCR(DMA_Registers[1].MADR);
		MADR_Func_002586c0(DMA_Registers[1].MADR);
	}

	lVar4 = 0x35048a;

	asm("
	.set noreorder
		loop2 :
			blez %0,dmaquit2
			nop
			addiu %0,%0,-1
			blez %0,dmaquit2
			nop
			addiu %0,%0,-1
			blez %0,dmaquit2
			nop
			bc2f dmaquit2
			addiu %0,%0,-1
			addiu %0,%0,-1
			j loop2
			nop
		dmaquit2:			
	.set reorder
	":"=r"(lVar4):"r"(lVar4));		// Sync bc2f

	if (lVar4 < 1) {
		Reset_00257f90();
		iVar3 = (int)lVar4;
	}
	else {
		thunk_FUN_00291be0();
		uVar5 = 0;
		while (uVar2 = DGET_VIF0_STAT(), (uVar2 & 3) != 0) { // Wait for VIF to become idle
			iVar3 = thunk_FUN_00291be0();
			uVar5 = uVar5 + iVar3;
			if (0x6a0914 < uVar5) {
				Reset_00257f90();
			}
		}
		iVar3 = thunk_FUN_00291be0();
		lVar4 = (long)iVar3;
		uVar5 = 0;
		while (true) {
			iVar3 = (int)lVar4;
			uVar2 = DGET_GIF_STAT();
			if ((uVar2 & 0x600) == 0) break;
			iVar3 = thunk_FUN_00291be0();
			lVar4 = (long)iVar3;
			uVar5 = uVar5 + iVar3;
			if (0x6a0914 < uVar5) {
				Reset_00257f90();
			}
		}
	}
	return iVar3;
#else
	return 0;
#endif
}

int shellDmaSync(int channel)
{
#ifdef PLATFORM_PS2
	if ((*DMA_Registers[channel].CHCR & 0x100) != 0) {
		thunk_FUN_00291be0();
		DPUT_D_PCR(DMA_Registers[channel].MADR);
		MADR_Func_002586c0(DMA_Registers[channel].MADR);
	}
#endif
	return 1;
}

void shellDmaStartChainB(int channel, ulonglong* pBuffer)
{
#ifdef PLATFORM_PS2
	uint* puVar1;
	uint uVar2;
	uint uVar3;

	puVar1 = DMA_Registers[channel].CHCR;
	uVar2 = DMA_Registers[channel].MADR;
	uVar3 = (uint)pBuffer & 0xfffffff;
	if (((uint)pBuffer & 0xf0000000) == 0x70000000) {
		uVar3 = uVar3 | 0x80000000;
	}
	puVar1[0xc] = uVar3;
	SYNC(0);
	puVar1[8] = 0;
	SYNC(0);
	DPUT_D_STAT(uVar2);
	SYNC(0);
	*puVar1 = DMA_Registers[channel].QWC | 0x104;
	SYNC(0);
#endif
	return;
}

void shellDmaStartChain(int channel, ulonglong* pBuffer)
{
#ifdef PLATFORM_PS2
	uint madr;
	uint* puVar1;
	uint uVar2;

	madr = DMA_Registers[channel].MADR;
	puVar1 = DMA_Registers[channel].CHCR;
	uVar2 = (uint)pBuffer & 0xfffffff;
	if (((uint)pBuffer & 0xf0000000) == 0x70000000) {
		uVar2 = uVar2 | 0x80000000;
	}
	puVar1[0xc] = uVar2;
	SYNC(0);
	puVar1[8] = 0;
	SYNC(0);
	DPUT_D_PCR(madr);
	SYNC(0);
	DPUT_D_STAT(madr);
	SYNC(0);
	*puVar1 = DMA_Registers[channel].QWC | 0x104;
	MADR_Func_002586c0(madr);
#endif
	return;
}

void shellDmaStartB(int channel, void* memory, uint qwc)
{
#ifdef PLATFORM_PS2
	uint* puVar1;
	uint uVar2;
	uint madr;

	puVar1 = DMA_Registers[channel].CHCR;
	madr = DMA_Registers[channel].MADR;
	uVar2 = (uint)memory & 0xfffffff;
	if (((uint)memory & 0xf0000000) == 0x70000000) {
		uVar2 = uVar2 | 0x80000000;
	}
	puVar1[4] = uVar2;
	SYNC(0);
	puVar1[8] = qwc >> 4;
	SYNC(0);
	DPUT_D_PCR(madr);
	SYNC(0);
	DPUT_D_STAT(madr);
	SYNC(0);
	*puVar1 = DMA_Registers[channel].QWC | 0x100;
	SYNC(0);
#endif
	return;
}

void shellDmaStart(int channel, void* memory, u_int qwc)
{
#ifdef PLATFORM_PS2
	uint madr;
	uint* puVar1;
	uint uVar2;

	madr = DMA_Registers[channel].MADR;
	puVar1 = DMA_Registers[channel].CHCR;
	uVar2 = (uint)memory & 0xfffffff;
	if (((uint)memory & 0xf0000000) == 0x70000000) {
		uVar2 = uVar2 | 0x80000000;
	}
	puVar1[4] = uVar2;
	SYNC(0);
	puVar1[8] = qwc >> 4;
	SYNC(0);
	DPUT_D_PCR(madr);
	SYNC(0);
	DPUT_D_STAT(madr);
	SYNC(0);
	*puVar1 = DMA_Registers[channel].QWC | 0x100;
	MADR_Func_002586c0(madr);
#endif
	return;
}

bool MADR_Func_002586c0(uint madr)
{
	char cVar1;
	int iVar2;
	int iVar3;

	iVar3 = 0x35048a;
	
#ifdef PLATFORM_PS2
	asm("
	.set noreorder
		loop :
			sync.l
			sync.p
			blez %0,dmaquit
			nop
			addiu %0,%0,-1
			blez %0,dmaquit
			nop
			addiu %0,%0,-1
			blez %0,dmaquit
			nop
			bc0f loop
			addiu %0,%0,-1
			addiu %0,%0,-1
		dmaquit:			
	.set reorder
	":"=r"(iVar3):"r"(iVar3));		// Sync bc0f

	if (0 < iVar3) {
		DPUT_D_STAT(madr);
		SYNC(0);
	}
	else {
		Reset_00257f90();
	}
#endif
	return 0 < iVar3;
}