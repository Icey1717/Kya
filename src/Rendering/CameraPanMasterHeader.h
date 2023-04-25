#ifndef CAMERA_PAN_MASTER_HEADER_H
#define CAMERA_PAN_MASTER_HEADER_H

#include "Types.h"
#include "edMem.h"

union MeshTransformSpecialUnion {
	byte byteFlags[4];
	short field_0x0[2];
	int count;
	void* randoPtr;
};

struct MeshTransformSpecial {
	MeshTransformSpecialUnion specUnion;
	MeshTransformSpecial* pNext_0x4;
	MeshTransformSpecial* pPrev_0x8;
	struct DisplayListInternal* pRenderInput;
};

struct MeshTransformParent {
	short bLoaded;
	undefined field_0x2;
	undefined field_0x3;
	MeshTransformParent* pNext;
	MeshTransformParent* pLink_0x8;
	struct DisplayListInternalMesh* pMeshTransformData;
};

struct MeshTransformParentHeader {
	char name[4];
	MeshTransformSpecial* field_0x4;
	int totalCount;
	int usedCount;
};

typedef int(CameraPanFunc)(DisplayListInternal*, DisplayListInternal*);

union CameraPanHeaderUnion {
	short field0;
	CameraPanFunc* field1;
};

struct CameraPanMasterHeader {
	CameraPanHeaderUnion headerUnion;
	MeshTransformSpecial* pLoadedData;
	struct MeshTransformParent* pCameraPanStatic_0x8;
	MeshTransformParentHeader* pCameraPanHeader_0xc;
	undefined4 field_0x10;
	int count_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

void Link_00290c10(CameraPanMasterHeader* pCameraPanMasterHeader);
CameraPanMasterHeader* AllocateCameraAndMesh_00290a10(int cameraPanCount, int meshHeaderCount, void* heapID);
bool Setup_00290bf0(CameraPanMasterHeader* pCameraPanStatic, MeshTransformParentHeader* param_2, int param_3, CameraPanFunc* param_4);
void* Setup_00290b70(MeshTransformParentHeader* pAllocatedBuffer, int count);
bool FUN_002909f0(CameraPanMasterHeader* param_1, int param_2, CameraPanFunc* param_3);

#endif //CAMERA_PAN_MASTER_HEADER_H
