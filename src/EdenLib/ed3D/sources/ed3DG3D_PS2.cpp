#include "ed3D/ed3DG3D_PS2.h"
#include "ed3D.h"

ed_g3d_Anim_def* ed3DG2DAnimTexGet(ed_g2d_texture* pTexture)
{
	ed_Chunck* pAnimChunck = LOAD_SECTION_CAST(ed_Chunck*, pTexture->pAnimChunck);
	ed_g2d_Anim_def* p2dAnimDef = reinterpret_cast<ed_g2d_Anim_def*>(pAnimChunck + 1);
	return LOAD_SECTION_CAST(ed_g3d_Anim_def*, p2dAnimDef->p3dAnimDef);
}

int ed3DG3DNbMaxBaseRGBA(ed_3d_strip* pStrip)
{
	int* pRGBA = LOAD_SECTION_CAST(int*, pStrip->pColorBuf);
	return pRGBA[2];
}

ed_g3d_Anim_def* ed3DG3DAnimGet(ed_3d_strip* pStrip)
{
	int* pData = LOAD_SECTION_CAST(int*, pStrip->pColorBuf);
	return LOAD_SECTION_CAST(ed_g3d_Anim_def*, pData[1]);
}