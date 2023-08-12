#include "DebugMaterial.h"

#include "pointer_conv.h"
#include "ed3D.h"
#include "edDlist.h"
#include "DebugRenderer.h"

DebugHelpers::DebugMaterial::DebugMaterial(edDList_material* pInMaterial, bool bCreateTexID)
	: key(pInMaterial)
	, texture(LoadTextureData())
	, texID(bCreateTexID ? DebugMenu::AddTexture(texture.image) : nullptr)
	, paletteTexID(bCreateTexID ? DebugMenu::AddTexture(texture.paletteImage) : nullptr)
{

}

DebugHelpers::DebugMaterial::DebugMaterial(const PS2::GSTexValue& inTexture, ImTextureID inTexID)
	: key(inTexture, inTexID)
	, texture(inTexture)
	, texID(inTexID)
{

}

Renderer::TextureData DebugHelpers::DebugMaterial::LoadTextureData()
{
	ed_g2d_bitmap* pPaletteBitmap;
	edpkt_data* pRVar4;
	ed_g2d_bitmap* pTextureBitmap;
	bool bHasPalette;

	if (key.pMaterial != (edDList_material*)0x0) {
		if ((key.pMaterial->mode & 4U) == 0) {
			if (key.pMaterial->pMAT != (ed_g2d_material*)0x0) {
				bHasPalette = false;
				pTextureBitmap = (ed_g2d_bitmap*)0x0;
				pPaletteBitmap = edDListGetG2DBitmap(key.pMaterial->pMAT, 0, &bHasPalette, &pTextureBitmap);

				if (pPaletteBitmap != (ed_g2d_bitmap*)0x0 && pTextureBitmap != (ed_g2d_bitmap*)0x0) {
					return MakeTextureDataFromPacket(pTextureBitmap, pPaletteBitmap);
				}
			}
		}
	}

	return Renderer::TextureData();
}
