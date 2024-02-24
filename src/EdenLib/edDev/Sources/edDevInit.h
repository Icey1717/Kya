#ifndef ED_DEV_INIT_H
#define ED_DEV_INIT_H

#include "Types.h"

void edDevInit(uint flags);
int edDevInitPort(uint port, int slot, uint type);

#endif //ED_DEV_INIT_H