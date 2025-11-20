#include "PoolAllocators.h"
#include "DlistManager.h"
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
PoolAllocator* g_edDLIST_MATERIAL_allocator;

S_GLOBAL_DLIST_PATCH* NewPool_S_EYES_BRIGHT_SHADOW(int count)
{
	int iVar1;
	bool bVar2;
	void** ppvVar3;
	PoolAllocator* pPVar4;
	PoolAllocator* pPVar5;
	void* pvVar6;
	S_GLOBAL_DLIST_PATCH* pSVar7;
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
				pvVar6 = new S_GLOBAL_DLIST_PATCH[pPVar5->size];
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
				pvVar6 = new S_GLOBAL_DLIST_PATCH[pPVar5->size];
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
		pSVar7 = (S_GLOBAL_DLIST_PATCH*)0x0;
	}
	else {
		ppvVar3 = &g_S_EYES_BRIGHT_SHADOW_allocator->pValue;
		g_S_EYES_BRIGHT_SHADOW_allocator->free = iVar8;
		pSVar7 = (S_GLOBAL_DLIST_PATCH*)(*ppvVar3) + iVar1;
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
				pPVar5->pValue = new edF32MATRIX4[pPVar5->size];
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
		else {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_edF32MATRIX4_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = count;
				pPVar5->pValue = new edF32MATRIX4[pPVar5->size];
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
				pPVar5->pValue = new CBehaviourCinematic[pPVar5->size];
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
		else {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_CBehaviourCinematic_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = count;
				pPVar5->pValue = new CBehaviourCinematic[pPVar5->size];
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
	int freeCount;
	bool bCanCreate;
	PoolAllocator* pOldAllocator;
	PoolAllocator* pAllocator;
	void* pBuff;
	float* pNewPool;
	int newFreeCount;

	bCanCreate = true;
	if ((g_U32_allocator != (PoolAllocator*)0x0) &&
		(g_U32_allocator->free + count <= g_U32_allocator->size)) {
		bCanCreate = false;
	}

	pAllocator = g_U32_allocator;

	if (bCanCreate) {
		if (count < 0x21) {
			pAllocator = new PoolAllocator;
			pOldAllocator = g_U32_allocator;
			if (pAllocator != (PoolAllocator*)0x0) {
				pAllocator->size = 0x20;
				pAllocator->pValue = new float[pAllocator->size];
				pAllocator->pAllocator = pOldAllocator;
				pAllocator->free = 0;
			}
		}
		else {
			pAllocator = new PoolAllocator;
			pOldAllocator = g_U32_allocator;
			if (pAllocator != (PoolAllocator*)0x0) {
				pAllocator->size = count;
				pAllocator->pValue = new float[pAllocator->size];
				pAllocator->pAllocator = pOldAllocator;
				pAllocator->free = 0;
			}
		}
	}

	g_U32_allocator = pAllocator;
	freeCount = g_U32_allocator->free;
	newFreeCount = freeCount + count;

	if (g_U32_allocator->size < newFreeCount) {
		pNewPool = (float*)0x0;
	}
	else {
		g_U32_allocator->free = newFreeCount;
		pNewPool = reinterpret_cast<float*>(reinterpret_cast<char*>(g_U32_allocator->pValue) + freeCount * sizeof(float));
	}

	return pNewPool;
}

ushort* NewPool_edU16(int count)
{
	int freeCount;
	bool bCanCreate;
	PoolAllocator* pOldAllocator;
	PoolAllocator* pAllocator;
	void* pBuff;
	ushort* pNewPool;
	int newFreeCount;

	bCanCreate = true;
	if ((g_U32_allocator != (PoolAllocator*)0x0) &&
		(g_U32_allocator->free + count <= g_U32_allocator->size)) {
		bCanCreate = false;
	}

	pAllocator = g_U32_allocator;

	if (bCanCreate) {
		if (count < 0x21) {
			pAllocator = new PoolAllocator;
			pOldAllocator = g_U32_allocator;
			if (pAllocator != (PoolAllocator*)0x0) {
				pAllocator->size = 0x20;
				pAllocator->pValue = new ushort[pAllocator->size];
				pAllocator->pAllocator = pOldAllocator;
				pAllocator->free = 0;
			}
		}
		else {
			pAllocator = new PoolAllocator;
			pOldAllocator = g_U32_allocator;
			if (pAllocator != (PoolAllocator*)0x0) {
				pAllocator->size = count;
				pAllocator->pValue = new ushort[pAllocator->size];
				pAllocator->pAllocator = pOldAllocator;
				pAllocator->free = 0;
			}
		}
	}

	g_U32_allocator = pAllocator;
	freeCount = g_U32_allocator->free;
	newFreeCount = freeCount + count;

	if (g_U32_allocator->size < newFreeCount) {
		pNewPool = (ushort*)0x0;
	}
	else {
		g_U32_allocator->free = newFreeCount;
		pNewPool = reinterpret_cast<ushort*>(reinterpret_cast<char*>(g_U32_allocator->pValue) + freeCount * sizeof(ushort));
	}

	return pNewPool;
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
				pPVar5->pValue = new void* [pPVar5->size];
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
		else {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_U32_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = count;
				pPVar5->pValue = new void* [pPVar5->size];
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

edDList_material* NewPool_edDLIST_MATERIAL(int count)
{
	int iVar1;
	bool bVar2;
	void** ppvVar3;
	PoolAllocator* pPVar4;
	PoolAllocator* pPVar5;
	void* pvVar6;
	edDList_material* peVar7;
	int iVar8;
	bVar2 = true;
	if ((g_edDLIST_MATERIAL_allocator != (PoolAllocator*)0x0) &&
		(g_edDLIST_MATERIAL_allocator->free + count <= g_edDLIST_MATERIAL_allocator->size)) {
		bVar2 = false;
	}

	pPVar5 = g_edDLIST_MATERIAL_allocator;
	if (bVar2) {
		if (count < 0x21) {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_edDLIST_MATERIAL_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = 0x20;
				pvVar6 = new edDList_material[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
		else {
			pPVar5 = new PoolAllocator;
			pPVar4 = g_edDLIST_MATERIAL_allocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				pPVar5->size = count;
				pvVar6 = new edDList_material[pPVar5->size];
				pPVar5->pValue = pvVar6;
				pPVar5->pAllocator = pPVar4;
				pPVar5->free = 0;
			}
		}
	}

	g_edDLIST_MATERIAL_allocator = pPVar5;
	iVar1 = g_edDLIST_MATERIAL_allocator->free;
	iVar8 = iVar1 + count;

	if (g_edDLIST_MATERIAL_allocator->size < iVar8) {
		peVar7 = (edDList_material*)0x0;
	}
	else {
		ppvVar3 = &g_edDLIST_MATERIAL_allocator->pValue;
		g_edDLIST_MATERIAL_allocator->free = iVar8;
		peVar7 = (edDList_material*)((char*)*ppvVar3 + iVar1 * sizeof(edDList_material));
	}

	return peVar7;
}

void FreeAllAllocators()
{
	if (g_U32_allocator != (PoolAllocator*)0x0) {
		bool bVar1 = g_U32_allocator != (PoolAllocator*)0x0;
		PoolAllocator* pPVar5 = g_U32_allocator;
		while (g_U32_allocator = pPVar5, bVar1) {
			PoolAllocator* pPVar2 = pPVar5->pAllocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				if (pPVar5->pValue != (void*)0x0) {
					float* pOriginalPtr = (float*)pPVar5->pValue;
					delete[] pOriginalPtr;
				}

				delete pPVar5;
			}

			bVar1 = pPVar2 != (PoolAllocator*)0x0;
			pPVar5 = pPVar2;
		}
	}

	if (g_CBehaviourCinematic_allocator != (PoolAllocator*)0x0) {
		bool bVar1 = g_CBehaviourCinematic_allocator != (PoolAllocator*)0x0;
		PoolAllocator* pPVar5 = g_CBehaviourCinematic_allocator;
		while (g_CBehaviourCinematic_allocator = pPVar5, bVar1) {
			PoolAllocator* pPVar2 = pPVar5->pAllocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				if (pPVar5->pValue != (void*)0x0) {
					CBehaviourCinematic* pOriginalPtr = (CBehaviourCinematic*)pPVar5->pValue;
					delete[] pOriginalPtr;
				}

				delete pPVar5;
			}

			bVar1 = pPVar2 != (PoolAllocator*)0x0;
			pPVar5 = pPVar2;
		}
	}

	if (g_edF32MATRIX4_allocator != (PoolAllocator*)0x0) {
		bool bVar1 = g_edF32MATRIX4_allocator != (PoolAllocator*)0x0;
		PoolAllocator* pPVar5 = g_edF32MATRIX4_allocator;
		while (g_edF32MATRIX4_allocator = pPVar5, bVar1) {
			PoolAllocator* pPVar2 = pPVar5->pAllocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				if (pPVar5->pValue != (void*)0x0) {
					edF32MATRIX4* pOriginalPtr = (edF32MATRIX4*)pPVar5->pValue;
					delete[] pOriginalPtr;
				}

				delete pPVar5;
			}

			bVar1 = pPVar2 != (PoolAllocator*)0x0;
			pPVar5 = pPVar2;
		}
	}

	if (g_S_EYES_BRIGHT_SHADOW_allocator != (PoolAllocator*)0x0) {
		bool bVar1 = g_S_EYES_BRIGHT_SHADOW_allocator != (PoolAllocator*)0x0;
		PoolAllocator* pPVar5 = g_S_EYES_BRIGHT_SHADOW_allocator;
		while (g_S_EYES_BRIGHT_SHADOW_allocator = pPVar5, bVar1) {
			PoolAllocator* pPVar2 = pPVar5->pAllocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				if (pPVar5->pValue != (void*)0x0) {
					S_GLOBAL_DLIST_PATCH* pOriginalPtr = (S_GLOBAL_DLIST_PATCH*)pPVar5->pValue;
					delete[] pOriginalPtr;
				}

				delete pPVar5;
			}

			bVar1 = pPVar2 != (PoolAllocator*)0x0;
			pPVar5 = pPVar2;
		}
	}

	if (g_edDLIST_MATERIAL_allocator != (PoolAllocator*)0x0) {
		bool bVar1 = g_edDLIST_MATERIAL_allocator != (PoolAllocator*)0x0;
		PoolAllocator* pPVar5 = g_edDLIST_MATERIAL_allocator;
		while (g_edDLIST_MATERIAL_allocator = pPVar5, bVar1) {
			PoolAllocator* pPVar2 = pPVar5->pAllocator;
			if (pPVar5 != (PoolAllocator*)0x0) {
				if (pPVar5->pValue != (void*)0x0) {
					edDList_material* pOriginalPtr = (edDList_material*)pPVar5->pValue;
					delete[] pOriginalPtr;
				}

				delete pPVar5;
			}
			bVar1 = pPVar2 != (PoolAllocator*)0x0;
			pPVar5 = pPVar2;
		}
	}
}
