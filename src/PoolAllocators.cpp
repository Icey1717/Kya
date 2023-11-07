#include "PoolAllocators.h"
#include "Rendering/DisplayList.h"

struct PoolAllocator {
	void* pValue;
	PoolAllocator* pAllocator;
	int size;
	int free;
};

PoolAllocator* g_S_EYES_BRIGHT_SHADOW_allocator;

S_EYES_BRIGHT_SHADOW* NewPool_S_EYES_BRIGHT_SHADOW(int count)
{
	int iVar1;
	bool bVar2;
	void** ppvVar3;
	PoolAllocator* pPVar4;
	PoolAllocator* pPVar5;
	void* pvVar6;
	S_EYES_BRIGHT_SHADOW* pSVar7;
	int iVar8;

	bVar2 = true;
	if ((g_S_EYES_BRIGHT_SHADOW_allocator != (PoolAllocator*)0x0) &&
		(g_S_EYES_BRIGHT_SHADOW_allocator->free + count <= g_S_EYES_BRIGHT_SHADOW_allocator->size)) {
		bVar2 = false;
	}
	pPVar5 = g_S_EYES_BRIGHT_SHADOW_allocator;
	if (bVar2) {
		if (count < 0x81) {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_S_EYES_BRIGHT_SHADOW_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = 0x80;
				pvVar6 = new S_EYES_BRIGHT_SHADOW[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
		else {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_S_EYES_BRIGHT_SHADOW_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = count;
				pvVar6 = new S_EYES_BRIGHT_SHADOW[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
	}
	g_S_EYES_BRIGHT_SHADOW_allocator = pPVar5;
	iVar1 = g_S_EYES_BRIGHT_SHADOW_allocator->free;
	iVar8 = iVar1 + count;
	if (g_S_EYES_BRIGHT_SHADOW_allocator->size < iVar8) {
		pSVar7 = (S_EYES_BRIGHT_SHADOW*)0x0;
	}
	else {
		ppvVar3 = &g_S_EYES_BRIGHT_SHADOW_allocator->pValue;
		g_S_EYES_BRIGHT_SHADOW_allocator->free = iVar8;
		pSVar7 = (S_EYES_BRIGHT_SHADOW*)(*ppvVar3) + iVar1;
	}
	return pSVar7;
}
