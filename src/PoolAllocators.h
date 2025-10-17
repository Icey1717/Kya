#ifndef _POOL_ALLOCATORS_H
#define _POOL_ALLOCATORS_H

#include "Types.h"

struct S_GLOBAL_DLIST_PATCH;
struct CBehaviourCinematic;
struct edDList_material;

S_GLOBAL_DLIST_PATCH* NewPool_S_EYES_BRIGHT_SHADOW(int count);
edF32MATRIX4* NewPool_edF32MATRIX4(int count);
CBehaviourCinematic* NewPool_CBehaviourCinematic(int count);
float* NewPool_edF32(int count);
ushort* NewPool_edU16(int count);
void** NewPool_Pointer(int count);
edDList_material* NewPool_edDLIST_MATERIAL(int count);

void FreeAllAllocators();

#endif // _POOL_ALLOCATORS_H
