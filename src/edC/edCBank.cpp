#include "edCBank.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "edCFiler.h"

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif
#include "EdenLib/edSys/sources/EdHandlers.h"
#include "edCFiler_BNK.h"


int g_FileSystemSysHandlerID_00469bc4 = 0x10;
int g_FileSystemHandlers_00469bc8 = 6;

edCSysHandlerPool* PTR_g_SysHandlersNodeTable_00469b80 = &edSysHandlerMainPool;
