#ifndef _EDDLIST_H
#define _EDDLIST_H

#include "Types.h"

struct DisplayList_0x10 {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	char* field_0x8;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
};

struct DisplayListInternal;
struct RenderCommand;
struct MaterialInfo;

namespace edDlist
{
	extern DisplayList_0x10* g_DisplayListPtr_0044965c;
	extern int g_DisplayListObjCount_004250e0;
	extern int MaterialDisplayListCount_00449648;
	extern int g_DisplayListGlobal_00449664;
	extern DisplayListInternal* g_DisplayListEnd_004495d8;
	extern MaterialInfo* g_pMaterialInfo_00449644;
	extern int g_DisplayListCommandCount_004495d4;
	extern RenderCommand* g_CachedRenderCommandPtr_00449650;

	void Init(void);

	void CopyMatrixToDisplayList_002d07d0(Matrix* m0);

	void LoadMaterialResource_002cb850(struct MaterialInfo* pMaterialInfo);
}
#endif //_EDDLIST_H
