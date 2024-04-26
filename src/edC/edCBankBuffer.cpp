#include "edCBankBuffer.h"
#include "edCBank.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#else
#include "port.h"
#endif
#include "../edStr.h"

#define IO_LOG(level, format, ...) MY_LOG_CATEGORY("IO", level, format, ##__VA_ARGS__)
#define IO_LOG_DISABLED(level, format, ...) 



