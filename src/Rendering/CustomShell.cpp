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

DMA_Register_Struct edDmaChannelList[10] =
{
	/* DMAC */
	{
		(DMA_Reg_Ptr*)D0_CHCR,
		0x1,
		0x1
	},

	{
		(DMA_Reg_Ptr*)D1_CHCR,
		0x2,
		0x41
	},

	{
		(DMA_Reg_Ptr*)D2_CHCR,
		0x4,
		0x80
	},

	{
		(DMA_Reg_Ptr*)D3_CHCR,
		0x8,
		0x1
	},

	{
		(DMA_Reg_Ptr*)D4_CHCR,
		0x10,
		0x0
	},

	{
		(DMA_Reg_Ptr*)D5_CHCR,
		0x20,
		0x0
	},

	{
		(DMA_Reg_Ptr*)D6_CHCR,
		0x40,
		0x0
	},

	{
		(DMA_Reg_Ptr*)D7_CHCR,
		0x80,
		0x0
	},

	{
		(DMA_Reg_Ptr*)D8_CHCR,
		0x100,
		0x0
	},

	{
		(DMA_Reg_Ptr*)D9_CHCR,
		0x200,
		0x0
	},
};

void edDmaResetAll(void)
{
	edDebugPrintf("#\n");
	(edDmaChannelList[1].pReg)->CHCR = edDmaChannelList[1].QWC;
	SYNC(0);
	(edDmaChannelList[2].pReg)->CHCR = edDmaChannelList[2].QWC;
	SYNC(0);
	(edDmaChannelList[0].pReg)->CHCR = edDmaChannelList[0].QWC;
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

void edDmaLoadFromFastRam_nowait(void* pSrc, uint qwc, void* pDest)
{
	DMA_Reg_Ptr* pReg;
	uint uVar2;

	uVar2 = edDmaChannelList[8].MADR;
	pReg = edDmaChannelList[8].pReg;
#ifdef PLATFORM_PS2
	edDmaChannelList[8].pReg->MADR = pDest;
	SYNC(0);
	pReg->SADR = (uint)pSrc;
	SYNC(0);
	pReg->QWC = qwc >> 4;
	SYNC(0);
	DPUT_D_STAT(uVar2);
	SYNC(0);
	pReg->CHCR = edDmaChannelList[8].QWC | 0x100;
	SYNC(0);
#else
	memcpy(pDest, pSrc, qwc);
#endif
	return;
}

bool edDmaLoadFromFastRam(uint addr, uint qwc, uint param_3)
{
	DMA_Reg_Ptr* pReg;
	uint madr;
	bool bVar2;

	madr = edDmaChannelList[8].MADR;
	pReg = edDmaChannelList[8].pReg;
#ifdef PLATFORM_PS2
	DPUT_D_PCR(edDmaChannelList[8].MADR);
	SYNC(0);
	DPUT_D_STAT(edDmaChannelList[8].MADR);
	SYNC(0);
	edDmaChannelList[8].pReg->MADR = param_3;
	SYNC(0);
	pReg->SADR = addr;
	SYNC(0);
	pReg->QWC = qwc >> 4;
	SYNC(0);
	pReg->CHCR = edDmaChannelList[8].QWC | 0x100;
#endif
	bVar2 = edDmaWaitDma(madr);
	return bVar2;
}

void edDmaFlushCache(void)
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

int edTimerTimeGetCycleElapsedU32(void)
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


int edDmaSyncPath(void)
{
	char cVar1;
	uint uVar2;
	int iVar3;
	long lVar4;
	uint uVar5;

	//MY_LOG("WaitForDraw_00258230\n");

#ifdef PLATFORM_PS2
	if ((edDmaChannelList[2].pReg->CHCR & CHCR_STR) != 0) {
		edTimerTimeGetCycleElapsedU32();
		DPUT_D_PCR(edDmaChannelList[2].MADR);
		edDmaWaitDma(edDmaChannelList[2].MADR);
	}
	if ((edDmaChannelList[1].pReg->CHCR & CHCR_STR) != 0) {
		edTimerTimeGetCycleElapsedU32();
		DPUT_D_PCR(edDmaChannelList[1].MADR);
		edDmaWaitDma(edDmaChannelList[1].MADR);
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
		edDmaResetAll();
		iVar3 = (int)lVar4;
	}
	else {
		edTimerTimeGetCycleElapsedU32();
		uVar5 = 0;
		while (uVar2 = DGET_VIF0_STAT(), (uVar2 & 3) != 0) { // Wait for VIF to become idle
			iVar3 = edTimerTimeGetCycleElapsedU32();
			uVar5 = uVar5 + iVar3;
			if (0x6a0914 < uVar5) {
				edDmaResetAll();
			}
		}
		iVar3 = edTimerTimeGetCycleElapsedU32();
		lVar4 = (long)iVar3;
		uVar5 = 0;
		while (true) {
			iVar3 = (int)lVar4;
			uVar2 = DGET_GIF_STAT();
			if ((uVar2 & 0x600) == 0) break;
			iVar3 = edTimerTimeGetCycleElapsedU32();
			lVar4 = (long)iVar3;
			uVar5 = uVar5 + iVar3;
			if (0x6a0914 < uVar5) {
				edDmaResetAll();
			}
		}
	}
	return iVar3;
#else
	return 0;
#endif
}

int edDmaSync(int channel)
{
#ifdef PLATFORM_PS2
	if ((edDmaChannelList[channel].pReg->CHCR & 0x100) != 0) {
		edTimerTimeGetCycleElapsedU32();
		DPUT_D_PCR(edDmaChannelList[channel].MADR);
		edDmaWaitDma(edDmaChannelList[channel].MADR);
	}
#endif
	return 1;
}

void edDmaSend_nowait(int channel, ulonglong* pBuffer)
{
#ifdef PLATFORM_PS2
	DMA_Reg_Ptr* pReg;
	uint uVar2;
	uint uVar3;

	pReg = edDmaChannelList[channel].pReg;
	uVar2 = edDmaChannelList[channel].MADR;
	uVar3 = (uint)pBuffer & 0xfffffff;
	if (((uint)pBuffer & 0xf0000000) == 0x70000000) {
		uVar3 = uVar3 | 0x80000000;
	}
	pReg->TADR = uVar3;
	SYNC(0);
	pReg->QWC = 0;
	SYNC(0);
	DPUT_D_STAT(uVar2);
	SYNC(0);
	pReg->CHCR = edDmaChannelList[channel].QWC | 0x104;
	SYNC(0);
#endif
	return;
}

void edDmaSend(int channel, uint addr)
{
#ifdef PLATFORM_PS2
	uint madr;
	DMA_Reg_Ptr* pReg;
	uint uVar2;

	madr = edDmaChannelList[channel].MADR;
	pReg = edDmaChannelList[channel].pReg;
	uVar2 = addr & 0xfffffff;
	if ((addr & 0xf0000000) == 0x70000000) {
		uVar2 = uVar2 | 0x80000000;
	}
	pReg->TADR = uVar2;
	SYNC(0);
	pReg->QWC = 0;
	SYNC(0);
	DPUT_D_PCR(madr);
	SYNC(0);
	DPUT_D_STAT(madr);
	SYNC(0);
	pReg->CHCR = edDmaChannelList[channel].QWC | 0x104;
	edDmaWaitDma(madr);
#endif
	return;
}

void edDmaSendN_nowait(int channel, void* memory, uint qwc)
{
#ifdef PLATFORM_PS2
	DMA_Reg_Ptr* pReg;
	uint uVar2;
	uint madr;

	pReg = edDmaChannelList[channel].pReg;
	madr = edDmaChannelList[channel].MADR;
	uVar2 = (uint)memory & 0xfffffff;
	if (((uint)memory & 0xf0000000) == 0x70000000) {
		uVar2 = uVar2 | 0x80000000;
	}
	pReg->MADR = uVar2;
	SYNC(0);
	pReg->QWC = qwc >> 4;
	SYNC(0);
	DPUT_D_PCR(madr);
	SYNC(0);
	DPUT_D_STAT(madr);
	SYNC(0);
	pReg->CHCR = edDmaChannelList[channel].QWC | 0x100;
	SYNC(0);
#endif
	return;
}

void edDmaSendN(int channel, void* memory, u_int qwc)
{
#ifdef PLATFORM_PS2
	uint madr;
	DMA_Reg_Ptr* pReg;
	uint uVar2;

	madr = edDmaChannelList[channel].MADR;
	pReg = edDmaChannelList[channel].pReg;
	uVar2 = (uint)memory & 0xfffffff;
	if (((uint)memory & 0xf0000000) == 0x70000000) {
		uVar2 = uVar2 | 0x80000000;
	}
	pReg->MADR = uVar2;
	SYNC(0);
	pReg->QWC = qwc >> 4;
	SYNC(0);
	DPUT_D_PCR(madr);
	SYNC(0);
	DPUT_D_STAT(madr);
	SYNC(0);
	pReg->CHCR = edDmaChannelList[channel].QWC | 0x100;
	edDmaWaitDma(madr);
#endif
	return;
}

bool edDmaWaitDma(uint madr)
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
		edDmaResetAll();
	}
#endif
	return 0 < iVar3;
}