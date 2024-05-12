#include "Texture.h"
#include "ed3D.h"
#include "Log.h"

#define TEXTURE_LOG(level, format, ...) MY_LOG_CATEGORY("Texture", level, format, ##__VA_ARGS__)

namespace Renderer
{
	namespace Kya
	{
		TextureLibrary gTextureLibrary;
	}
}

Renderer::Kya::Texture::Texture(ed_g2d_manager* pManager, std::string name)
	: pManager(pManager)
	, name(name)
{
	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Texture Beginning processing of texture: {}", name.c_str());

	const int nbMaterials = ed3DG2DGetG2DNbMaterials(pManager->pMATA_HASH);
	materials.reserve(nbMaterials);

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Texture Nb Materials: {}", nbMaterials);

	const ed_hash_code* pHashCodes = reinterpret_cast<ed_hash_code*>(pManager->pMATA_HASH + 1);

	for (int i = 0; i < nbMaterials; ++i) {
		const ed_hash_code* pHashCode = pHashCodes + i;

		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Texture Processing material: {}", pHashCode->hash.ToString());

		ed_g2d_material* pMaterial = ed3DG2DGetG2DMaterialFromIndex(pManager, i);

		if (pMaterial) {
			ProcessMaterial(pMaterial);
		}
	}
}

void Renderer::Kya::Texture::ProcessMaterial(ed_g2d_material* pMaterial)
{
	assert(pMaterial);

	Material& material = materials.emplace_back();
	material.pMaterial = pMaterial;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::ProcessMaterial Nb layers: {} flags: 0x{:x}", pMaterial->nbLayers, pMaterial->flags);

	material.renderCommands.pList = LOAD_SECTION_CAST(edpkt_data*, pMaterial->pCommandBufferTexture);
	material.renderCommands.size = pMaterial->commandBufferTextureSize;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::ProcessMaterial Render command size: {} (0x{:x})", material.renderCommands.size, material.renderCommands.size);

	for (int i = 0; i < pMaterial->nbLayers; ++i) {
		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::ProcessMaterial Processing layer: {}", i);

		ed_Chunck* pLAY = LOAD_SECTION_CAST(ed_Chunck*, pMaterial->aLayers[i]);
		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::ProcessMaterial Layer chunk header: {}", pLAY->GetHeaderString());

		ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
		material.ProcessLayer(pLayer);
	}
}

void Renderer::Kya::Texture::Material::ProcessLayer(ed_g2d_layer* pLayer)
{
	Layer& layer = layers.emplace_back();
	layer.pLayer = pLayer;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Material::ProcessLayer Layer flags 0x0: 0x{:x} flags 0x4: 0x{:x} field 0x1b: {} bHasTexture: {} paletteId: {}", 
		pLayer->flags_0x0, pLayer->flags_0x4, pLayer->field_0x1b, pLayer->bHasTexture, pLayer->paletteId);

	if (pLayer->bHasTexture) {
		ed_Chunck* pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Material::ProcessLayer Texture chunk header: {}", pTEX->GetHeaderString());

		ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
		layer.ProcessTexture(pTexture);
	}
}

void Renderer::Kya::Texture::Material::Layer::ProcessTexture(ed_g2d_texture* pTexture)
{
	Texture& texture = textures.emplace_back();
	texture.pTexture = pTexture;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Material::Layer::ProcessTexture Texture hash: {} field 0x14: {} bHasPalette: {}",
		pTexture->hashCode.hash.ToString(), pTexture->field_0x14, pTexture->bHasPalette);

	ed_hash_code* pBitmapHashCode = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);
	if (pBitmapHashCode != (ed_hash_code*)0x0) {
		ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pBitmapHashCode->pData);

		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Material::Layer::ProcessTexture Bitmap chunk header: {}", pT2D->GetHeaderString());
		ed_g2d_bitmap* pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
		texture.bitmap.SetBitmap(pBitmap);		
	}

	if (pTexture->bHasPalette) {
		ed_hash_code* pPaletteHashCodes = reinterpret_cast<ed_hash_code*>(pTexture + 1);
		ed_hash_code* pPaletteHashCode = LOAD_SECTION_CAST(ed_hash_code*, pPaletteHashCodes[pLayer->paletteId].pData);
		ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pPaletteHashCode->pData);

		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Material::Layer::ProcessTexture Palette chunk header: {}", pT2D->GetHeaderString());
		ed_g2d_bitmap* pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
		texture.palette.SetBitmap(pBitmap);
	}
}

void Renderer::Kya::Texture::Material::Layer::Texture::Bitmap::SetBitmap(ed_g2d_bitmap* pBitmap)
{
	this->pBitmap = pBitmap;
	UpdateCommands();
}

void Renderer::Kya::Texture::Material::Layer::Texture::Bitmap::UpdateCommands()
{
	if (pBitmap->pPSX2) {
		edPSX2Header* pHeader = LOAD_SECTION_CAST(edPSX2Header*, pBitmap->pPSX2);

		for (int i = 0; i < 2; i++) {
			uploadCommands[i].pList = LOAD_SECTION_CAST(edpkt_data*, pHeader[i].pPkt);
			uploadCommands[i].size = pHeader[i].size;
			TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Material::Layer::Texture::Bitmap::UpdateCommands Upload command size: {} (0x{:x})", uploadCommands[i].size, uploadCommands[i].size);
		}
	}
}

void Renderer::Kya::TextureLibrary::Init()
{
	ed3DGetTextureLoadedDelegate() += Renderer::Kya::TextureLibrary::AddTexture;
}

void Renderer::Kya::TextureLibrary::AddTexture(ed_g2d_manager* pManager, std::string name)
{
	gTextureLibrary.gTextures.emplace_back(pManager, name);
}

const Renderer::Kya::TextureLibrary& Renderer::Kya::GetTextureLibrary()
{
	return gTextureLibrary;
}