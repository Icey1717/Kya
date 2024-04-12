#include "PoolAllocators.h"
#include "Rendering/DisplayList.h"
#include "Actor_Cinematic.h"

struct PoolAllocator {
	void* pValue;
	PoolAllocator* pAllocator;
	int size;
	int free;
};

PoolAllocator* g_S_EYES_BRIGHT_SHADOW_allocator;
PoolAllocator* g_edF32MATRIX4_allocator;
PoolAllocator* g_CBehaviourCinematic_allocator;
PoolAllocator* g_U32_allocator;

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

edF32MATRIX4* NewPool_edF32MATRIX4(int count)
{
	int iVar1;
	bool bVar2;
	void** ppvVar3;
	PoolAllocator* pPVar4;
	PoolAllocator* pPVar5;
	void* pvVar6;
	edF32MATRIX4* peVar7;
	int iVar8;

	bVar2 = true;
	if ((g_edF32MATRIX4_allocator != (PoolAllocator*)0x0) &&
		(g_edF32MATRIX4_allocator->free + count <= g_edF32MATRIX4_allocator->size)) {
		bVar2 = false;
	}

	pPVar5 = g_edF32MATRIX4_allocator;

	if (bVar2) {
		if (count < 0x21) {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_edF32MATRIX4_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = 0x20;
				pvVar6 = new edF32MATRIX4[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
		else {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_edF32MATRIX4_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = count;
				pvVar6 = new edF32MATRIX4[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
	}

	g_edF32MATRIX4_allocator = pPVar5;
	iVar1 = g_edF32MATRIX4_allocator->free;
	iVar8 = iVar1 + count;

	if (g_edF32MATRIX4_allocator->size < iVar8) {
		peVar7 = (edF32MATRIX4*)0x0;
	}
	else {
		ppvVar3 = &g_edF32MATRIX4_allocator->pValue;
		g_edF32MATRIX4_allocator->free = iVar8;
		peVar7 = (edF32MATRIX4*)((char*)*ppvVar3 + iVar1 * sizeof(edF32MATRIX4));
	}

	return peVar7;
}

CBehaviourCinematic* NewPool_CBehaviourCinematic(int count)
{
	int iVar1;
	bool bVar2;
	void** ppvVar3;
	PoolAllocator* pPVar4;
	PoolAllocator* pPVar5;
	void* pvVar6;
	CBehaviourCinematic* peVar7;
	int iVar8;

	bVar2 = true;
	if ((g_CBehaviourCinematic_allocator != (PoolAllocator*)0x0) &&
		(g_CBehaviourCinematic_allocator->free + count <= g_CBehaviourCinematic_allocator->size)) {
		bVar2 = false;
	}

	pPVar5 = g_CBehaviourCinematic_allocator;

	if (bVar2) {
		if (count < 0x21) {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_CBehaviourCinematic_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = 0x20;
				pvVar6 = new CBehaviourCinematic[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
		else {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_CBehaviourCinematic_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = count;
				pvVar6 = new CBehaviourCinematic[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
	}

	g_CBehaviourCinematic_allocator = pPVar5;
	iVar1 = g_CBehaviourCinematic_allocator->free;
	iVar8 = iVar1 + count;

	if (g_CBehaviourCinematic_allocator->size < iVar8) {
		peVar7 = (CBehaviourCinematic*)0x0;
	}
	else {
		ppvVar3 = &g_CBehaviourCinematic_allocator->pValue;
		g_CBehaviourCinematic_allocator->free = iVar8;
		peVar7 = (CBehaviourCinematic*)((char*)*ppvVar3 + iVar1 * sizeof(CBehaviourCinematic));
	}

	return peVar7;
}

float* NewPool_edF32(int count)
{
	int iVar1;
	bool bVar2;
	void** ppvVar3;
	PoolAllocator* pPVar4;
	PoolAllocator* pPVar5;
	void* pvVar6;
	float* peVar7;
	int iVar8;

	bVar2 = true;
	if ((g_U32_allocator != (PoolAllocator*)0x0) &&
		(g_U32_allocator->free + count <= g_U32_allocator->size)) {
		bVar2 = false;
	}

	pPVar5 = g_U32_allocator;

	if (bVar2) {
		if (count < 0x21) {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_U32_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = 0x20;
				pvVar6 = new float[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
		else {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_U32_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = count;
				pvVar6 = new float[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
	}

	g_U32_allocator = pPVar5;
	iVar1 = g_U32_allocator->free;
	iVar8 = iVar1 + count;

	if (g_U32_allocator->size < iVar8) {
		peVar7 = (float*)0x0;
	}
	else {
		ppvVar3 = &g_U32_allocator->pValue;
		g_U32_allocator->free = iVar8;
		peVar7 = (float*)((char*)*ppvVar3 + iVar1 * sizeof(float));
	}

	return peVar7;
}

void** NewPool_Pointer(int count)
{
	int iVar1;
	bool bVar2;
	void** ppvVar3;
	PoolAllocator* pPVar4;
	PoolAllocator* pPVar5;
	void* pvVar6;
	void** peVar7;
	int iVar8;

	bVar2 = true;
	if ((g_U32_allocator != (PoolAllocator*)0x0) &&
		(g_U32_allocator->free + count <= g_U32_allocator->size)) {
		bVar2 = false;
	}

	pPVar5 = g_U32_allocator;

	if (bVar2) {
		if (count < 0x21) {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_U32_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = 0x20;
				pvVar6 = new void*[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
		else {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_U32_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = count;
				pvVar6 = new void* [pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
	}

	g_U32_allocator = pPVar5;
	iVar1 = g_U32_allocator->free;
	iVar8 = iVar1 + count;

	if (g_U32_allocator->size < iVar8) {
		peVar7 = (void**)0x0;
	}
	else {
		ppvVar3 = &g_U32_allocator->pValue;
		g_U32_allocator->free = iVar8;
		peVar7 = (void**)((char*)*ppvVar3 + iVar1 * sizeof(void*));
	}

	return peVar7;
}
