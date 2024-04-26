#include "edCBank.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "edCFiler.h"

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif
#include "edSystem.h"
#include "edCFiler_BNK.h"


int g_FileSystemSysHandlerID_00469bc4 = 0x10;
int g_FileSystemHandlers_00469bc8 = 6;

edSysHandlersNodeTable* PTR_g_SysHandlersNodeTable_00469b80 = &g_SysHandlersNodeTable_00489170;
