#ifndef CAMERA_PAN_MASTER_HEADER_H
#define CAMERA_PAN_MASTER_HEADER_H

#include "Types.h"
#include "edMem.h"

struct MeshTransformSpecial {
	ushort field_0x0;
	ushort field_0x2;
	MeshTransformSpecial* pNext_0x4;
	MeshTransformSpecial* field_0x8;
	struct DisplayListInternal* pRenderInput;
};

struct MeshTransformParent {
	short bLoaded;
	undefined field_0x2;
	undefined field_0x3;
	MeshTransformParent* pNext;
	MeshTransformParent* pLink_0x8;
	struct MeshTransformData* pMeshTransformData;
};

struct MeshTransformParentHeader {
	char name[4];
	MeshTransformSpecial* field_0x4;
	int totalCount;
	int usedCount;
};

union CameraPanHeaderUnion {
	short field0;
	char* field1;
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

void Link_00290c10(CameraPanMasterHeader* param_1);
CameraPanMasterHeader* AllocateCameraAndMesh_00290a10(int cameraPanCount, int meshHeaderCount, void* heapID);
bool Setup_00290bf0(CameraPanMasterHeader* pCameraPanStatic, MeshTransformParentHeader* param_2, int param_3, undefined* param_4);
void* Setup_00290b70(MeshTransformParentHeader* pAllocatedBuffer, int count);

#endif //CAMERA_PAN_MASTER_HEADER_H
