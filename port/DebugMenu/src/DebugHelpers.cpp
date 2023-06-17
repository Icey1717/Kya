#include "DebugHelpers.h"

#include "pointer_conv.h"
#include "ed3D.h"
#include "edDlist.h"
#include "DebugRenderer.h"

DebugHelpers::DebugMaterial::DebugMaterial(edDList_material* pInMaterial, bool bCreateTexID)
	: pMaterial(pInMaterial)
	, texture(LoadTextureData())
	, texID(bCreateTexID ? DebugMenu::AddTexture(texture) : nullptr)
{

}
Renderer::TextureData DebugHelpers::DebugMaterial::LoadTextureData()
{
	ed_g2d_bitmap* pPaletteBitmap;
	edpkt_data* pRVar4;
	ed_g2d_bitmap* pTextureBitmap;
	bool bHasPalette;

	if (pMaterial != (edDList_material*)0x0) {
		if ((pMaterial->mode & 4U) == 0) {
			if (pMaterial->pMAT != (ed_g2d_material*)0x0) {
				bHasPalette = false;
				pTextureBitmap = (ed_g2d_bitmap*)0x0;
				pPaletteBitmap = edDListGetG2DBitmap(pMaterial->pMAT, 0, &bHasPalette, &pTextureBitmap);

				if (pPaletteBitmap != (ed_g2d_bitmap*)0x0 && pTextureBitmap != (ed_g2d_bitmap*)0x0) {
					pRVar4 = (edpkt_data*)(((char*)LOAD_SECTION(pPaletteBitmap->pPSX2)) + 0x40);

					return MakeTextureDataFromPacket(pRVar4, pTextureBitmap, pPaletteBitmap);
				}
			}
		}
	}

	return Renderer::TextureData();
}
