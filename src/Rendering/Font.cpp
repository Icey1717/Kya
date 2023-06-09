#include "Font.h"

#include <assert.h>

#ifndef PLATFORM_PS2
#include <corecrt_malloc.h>
#include <corecrt_math.h>
#include "renderer.h"
#else
#include <math.h>
#endif
#include "edText.h"
#include "edDlist.h"
#include "port/pointer_conv.h"
#include "edMem.h"
#include "MathOps.h"

char* s_0123456789abcdef_00432380 = "0123456789abcdef";
char* s_0123456789ABCDEF_004323a0 = "0123456789ABCDEF";

