#ifndef ED_SYS_DATA_TRANSFER_H
#define ED_SYS_DATA_TRANSFER_H

#include "Types.h"

typedef void(edSysTransferFunc)(void*);
typedef int _EDSYS_RPC_CB_IDENT;

enum EdSysTransferFlags
{
	NO_FLAGS_SET = 0,
	ONE_BLOCK_ONLY = 1,
	KEEP_IN_IOP = 2,
	HIGH_MEM = 4
};

void _edSysTransferInit(void);
void _edSysWaitUntilTransferFinished(uint param_1);
void _edSysWaitUntilAllTransfersFinished(void);
uint _edSysTransferData(void* pSource, uint size, uint alignment, EdSysTransferFlags setupFlags, _EDSYS_RPC_CB_IDENT setupCallbackIdent, _EDSYS_RPC_CB_IDENT transferCallbackIdent, _EDSYS_RPC_CB_IDENT endCallbackIdent,
	void* pCallbackParams, uint returnNbBytes, uint userData, edSysTransferFunc* pCallback);

extern int _edSoundLastTransferIndex;
extern int _edSysTransferIndex;
extern int _edSysCompletedTransferIndex;

#endif // ED_SYS_DATA_TRANSFER_H
