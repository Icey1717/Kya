#ifndef PROFILE_H
#define PROFILE_H

#include "Types.h"

#define IMPLEMENTATION_GUARD_PROFILE()

uint edProfileNew(uint, uint, uint, uint);
uint edProfileNew(uint, uint, uint, uint, char*);

void edProfileBegin(uint profileId);
void edProfileEnd(uint profileId);

void edProfileDel(uint profileId);

#endif