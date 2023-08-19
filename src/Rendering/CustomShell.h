#ifndef _CUSTOM_SHELL_H
#define _CUSTOM_SHELL_H

#include "Types.h"

typedef enum
{
	SHELLDMA_CHANNEL_VIF0 = 0,
	SHELLDMA_CHANNEL_VIF1 = 1,
	SHELLDMA_CHANNEL_GIF = 2,
	SHELLDMA_CHANNEL_FROMIPU = 3,
	SHELLDMA_CHANNEL_TOIPU = 4,
	SHELLDMA_CHANNEL_FROMSPR = 8,
	SHELLDMA_CHANNEL_TOSPR = 9
}	EShellDmaChannel;

#ifdef PLATFORM_PS2
#define SYNC(...) __asm__ volatile(" sync.l ")
#else
#define SYNC(...)
#endif

/*
#define D0_CHCR         ((volatile u_int *)(0x10008000))
#define D0_MADR         ((volatile u_int *)(0x10008010))
#define D0_QWC          ((volatile u_int *)(0x10008020))
#define D0_TADR         ((volatile u_int *)(0x10008030))
#define D0_ASR0         ((volatile u_int *)(0x10008040))
#define D0_ASR1         ((volatile u_int *)(0x10008050))
*/

PACK(
	struct DMA_Reg_Ptr {
	uint CHCR;
	uint packA[3];
	uint MADR;
	uint packB[3];
	uint QWC;
	uint packC[3];
	uint TADR;
	uint packD[3];
	uint ASR0;
	uint packE[3];
	uint ASR1;
	uint packF[11];
	uint SADR;
});

struct DMA_Register_Struct {
	DMA_Reg_Ptr* pReg;
	uint MADR;
	uint QWC;
};

void edDmaLoadFromFastRam_nowait(void* pSrc, uint qwc, void* pDest);
bool edDmaLoadFromFastRam(void* pSrc, uint qwc, void* pDest);
void edDmaFlushCache(void);
int edDmaSyncPath(void);
int edDmaSync(int channel);
void edDmaSend_nowait(int channel, ulonglong* pBuffer);
void edDmaSend(int channel, uint addr);
void edDmaSendN_nowait(int channel, void* memory, uint qwc);
void edDmaSendN(int channel, void* memory, uint qwc);
bool edDmaWaitDma(uint madr);

extern DMA_Register_Struct edDmaChannelList[10];

#endif // _CUSTOM_SHELL_H
