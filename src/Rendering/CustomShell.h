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

struct DMA_Register_Struct {
	uint* CHCR;
	uint MADR;
	uint QWC;
};

void edDmaLoadFromFastRam_nowait(Vector* param_1, uint param_2, uint param_3);
bool edDmaLoadFromFastRam(uint addr, uint qwc, uint param_3);
void edDmaFlushCache(void);
int edDmaSyncPath(void);
int edDmaSync(int channel);
void edDmaSend_nowait(int channel, ulonglong* pBuffer);
void edDmaSend(int channel, ulonglong* pBuffer);
void edDmaSendN_nowait(int channel, void* memory, uint qwc);
void edDmaSendN(int channel, void* memory, u_int qwc);
bool edDmaWaitDma(uint madr);

extern DMA_Register_Struct edDmaChannelList[10];

#endif // _CUSTOM_SHELL_H
