#ifndef ED3D_G3D_PS2_H
#define ED3D_G3D_PS2_H

#include "Types.h"

struct ed_g2d_texture;
struct ed_g3d_Anim_def;
struct ed_3d_strip;

struct ed_g2d_Anim_def
{
	strd_ptr(ed_g3d_Anim_def*) p3dAnimDef;
	uint field_0x4;
	uint field_0x8;
	uint field_0xc;
	uint field_0x10;
	ushort field_0x14;
	ushort field_0x16;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

static_assert(sizeof(ed_g2d_Anim_def) == 0x20);

ed_g3d_Anim_def* ed3DG2DAnimTexGet(ed_g2d_texture* pManager);
int ed3DG3DNbMaxBaseRGBA(ed_3d_strip* pStrip);
ed_g3d_Anim_def* ed3DG3DAnimGet(ed_3d_strip* pStrip);

#endif // ED3D_G3D_PS2_H