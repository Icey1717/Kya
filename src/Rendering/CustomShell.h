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

void WaitDMA(void);
int WaitForDraw_00258230(void);
int shellDmaSync(int channel);
void shellDmaStartChainB(int channel, ulonglong* pBuffer);
void shellDmaStartChain(int channel, ulonglong* pBuffer);
void shellDmaStartB(int channel, void* memory, uint qwc);
void shellDmaStart(int channel, void* memory, u_int qwc);
bool MADR_Func_002586c0(uint madr);

#endif // _CUSTOM_SHELL_H
