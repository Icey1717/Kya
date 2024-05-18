#include "Texture.h"
#include "ed3D.h"
#include "Log.h"
#include "renderer.h"
#include "port.h"

#define TEXTURE_LOG(level, format, ...) MY_LOG_CATEGORY("Texture", level, format, ##__VA_ARGS__)

namespace Renderer
{
	namespace Kya
	{
		TextureLibrary gTextureLibrary;

		void ProcessCommandList(CombinedImageData& imageData, const G2D::CommandList& commandList)
		{
			// The first sets of data will be MIP levels for the texture, with the last one the palette data.

			int currentMipLevel = 0;

			edpkt_data* pPkt = commandList.pList;

			ImageData* pCurrentImage = nullptr;

			for (int i = 0; i < commandList.size; i++) {
				if (pPkt->asU32[2] == SCE_GIF_PACKED_AD) {
					if (pCurrentImage) {
						assert(pCurrentImage->pImage);
						assert(pCurrentImage->readWidth != 0);
						assert(pCurrentImage->readHeight != 0);
					}

					const bool bNextCommandIsTexFlush = pPkt[1].asU32[2] == SCE_GS_TEXFLUSH;

					if (currentMipLevel < imageData.bitmaps.size()) {
						pCurrentImage = &imageData.bitmaps[currentMipLevel];
						currentMipLevel++;
					}
					else if (!bNextCommandIsTexFlush) {
						// Some of the textures have 2 mips in the bitmap, but actually have 3, so can't assert here.
						pCurrentImage = &imageData.palette;
					}
				}

				if (pPkt->asU32[2] == SCE_GS_TEXFLUSH) {
					assert(i == commandList.size - 1);
				}

				if ((pPkt->asU32[0] >> 28) == 0x03) {
					assert(pCurrentImage);
					pCurrentImage->pImage = LOAD_SECTION(pPkt->asU32[1]);
				}

				if (pPkt->cmdB == SCE_GS_TRXREG) {
					assert(pCurrentImage);
					pCurrentImage->readWidth = pPkt->asU32[0];
					pCurrentImage->readHeight = pPkt->asU32[1];
				}

				pPkt++;
			}
		}

		CombinedImageData CreateFromBitmapAndPalette(G2D::Bitmap& bitmap, G2D::Bitmap& palette)
		{
			CombinedImageData combinedImageData;
			const ed_g2d_bitmap* pBitmap = bitmap.GetBitmap();

			combinedImageData.bitmaps.resize(pBitmap->maxMipLevel);

			for (auto& mip : combinedImageData.bitmaps) {
				mip.canvasWidth = pBitmap->width;
				mip.canvasHeight = pBitmap->height;
				mip.bpp = pBitmap->psm;
				mip.maxMipLevel = pBitmap->maxMipLevel;
			}

			const ed_g2d_bitmap* pPalette = palette.GetBitmap();

			if (pPalette) {
				combinedImageData.palette.canvasWidth = pPalette->width;
				combinedImageData.palette.canvasHeight = pPalette->height;
				combinedImageData.palette.bpp = pPalette->psm;
				combinedImageData.palette.maxMipLevel = pPalette->maxMipLevel;
				assert(combinedImageData.palette.maxMipLevel == 1);
			}

			G2D::CommandList commandList = pPalette ? palette.GetUploadCommandsDefault() : bitmap.GetUploadCommandsDefault();

			ProcessCommandList(combinedImageData, commandList);

			return combinedImageData;
		}
	}
}

Renderer::Kya::G2D::G2D(ed_g2d_manager* pManager, std::string name)
	: pManager(pManager)
	, name(name)
{
	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::G2D Beginning processing of texture: {}", name.c_str());

	const int nbMaterials = ed3DG2DGetG2DNbMaterials(pManager->pMATA_HASH);
	materials.reserve(nbMaterials);

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::G2D Nb Materials: {}", nbMaterials);

	const ed_hash_code* pHashCodes = reinterpret_cast<ed_hash_code*>(pManager->pMATA_HASH + 1);

	for (int i = 0; i < nbMaterials; ++i) {
		const ed_hash_code* pHashCode = pHashCodes + i;

		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::G2D Processing material: {}", pHashCode->hash.ToString());

		ed_g2d_material* pMaterial = ed3DG2DGetG2DMaterialFromIndex(pManager, i);

		if (pMaterial) {
			ProcessMaterial(pMaterial);
		}
	}
}

void Renderer::Kya::G2D::ProcessMaterial(ed_g2d_material* pMaterial)
{
	assert(pMaterial);

	Material& material = materials.emplace_back();
	material.pMaterial = pMaterial;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessMaterial Nb layers: {} flags: 0x{:x}", pMaterial->nbLayers, pMaterial->flags);

	material.renderCommands.pList = LOAD_SECTION_CAST(edpkt_data*, pMaterial->pCommandBufferTexture);
	material.renderCommands.size = pMaterial->commandBufferTextureSize;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessMaterial Render command size: {} (0x{:x})", material.renderCommands.size, material.renderCommands.size);

	for (int i = 0; i < pMaterial->nbLayers; ++i) {
		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessMaterial Processing layer: {}", i);

		ed_Chunck* pLAY = LOAD_SECTION_CAST(ed_Chunck*, pMaterial->aLayers[i]);
		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessMaterial Layer chunk header: {}", pLAY->GetHeaderString());

		ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
		material.ProcessLayer(pLayer);
	}
}

void Renderer::Kya::G2D::Material::ProcessLayer(ed_g2d_layer* pLayer)
{
	Layer& layer = layers.emplace_back();
	layer.pLayer = pLayer;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Material::ProcessLayer Layer flags 0x0: 0x{:x} flags 0x4: 0x{:x} field 0x1b: {} bHasTexture: {} paletteId: {}", 
		pLayer->flags_0x0, pLayer->flags_0x4, pLayer->field_0x1b, pLayer->bHasTexture, pLayer->paletteId);

	if (pLayer->bHasTexture) {
		ed_Chunck* pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Material::ProcessLayer Texture chunk header: {}", pTEX->GetHeaderString());

		ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
		layer.ProcessTexture(pTexture);
	}
}

Renderer::SimpleTexture* Renderer::Kya::G2D::Material::FindRenderTextureFromBitmap(ed_g2d_bitmap* pBitmap) const
{
	for (auto& layer : layers) {
		for (auto& texture : layer.textures) {
			if (texture.bitmap.GetBitmap() == pBitmap || texture.palette.GetBitmap() == pBitmap) {
				return texture.pSimpleTexture;
			}
		}
	}

	return nullptr;
}

bool Renderer::Kya::G2D::Material::GetInUse() const
{
	auto& inUseTextures = Renderer::GetInUseTextures();

	for (auto& layer : layers) {
		for (auto& texture : layer.textures) {
			if (texture.pSimpleTexture) {
				for (auto& inUseTexture : inUseTextures) {
					if (inUseTexture == texture.pSimpleTexture) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

void Renderer::Kya::G2D::Layer::ProcessTexture(ed_g2d_texture* pTexture)
{
	G2D::Texture& texture = textures.emplace_back();
	texture.pTexture = pTexture;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Layer::ProcessTexture Texture hash: {} field 0x14: {} bHasPalette: {}",
		pTexture->hashCode.hash.ToString(), pTexture->field_0x14, pTexture->bHasPalette);

	ed_hash_code* pBitmapHashCode = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);
	if (pBitmapHashCode != (ed_hash_code*)0x0) {
		ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pBitmapHashCode->pData);

		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Layer::ProcessTexture Bitmap chunk header: {}", pT2D->GetHeaderString());
		ed_g2d_bitmap* pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
		texture.bitmap.SetBitmap(pBitmap);		
	}

	if (pTexture->bHasPalette) {
		ed_hash_code* pPaletteHashCodes = reinterpret_cast<ed_hash_code*>(pTexture + 1);
		ed_hash_code* pPaletteHashCode = LOAD_SECTION_CAST(ed_hash_code*, pPaletteHashCodes[pLayer->paletteId].pData);
		ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pPaletteHashCode->pData);

		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Layer::ProcessTexture Palette chunk header: {}", pT2D->GetHeaderString());
		ed_g2d_bitmap* pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
		texture.palette.SetBitmap(pBitmap);
	}

	const CombinedImageData combinedImageData = CreateFromBitmapAndPalette(texture.bitmap, texture.palette);

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Layer::ProcessTexture bitmap mips: {}", combinedImageData.bitmaps.size());

	// Try create a simple texture
	texture.pSimpleTexture = new SimpleTexture(combinedImageData);
}

void Renderer::Kya::G2D::Bitmap::SetBitmap(ed_g2d_bitmap* pBitmap)
{
	this->pBitmap = pBitmap;
	UpdateCommands();
}

void Renderer::Kya::G2D::Bitmap::UpdateCommands()
{
	if (pBitmap->pPSX2) {
		edPSX2Header* pHeader = LOAD_SECTION_CAST(edPSX2Header*, pBitmap->pPSX2);

		for (int i = 0; i < 2; i++) {
			uploadCommands[i].pList = LOAD_SECTION_CAST(edpkt_data*, pHeader[i].pPkt);
			uploadCommands[i].size = pHeader[i].size;
			TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Bitmap::UpdateCommands Upload command size: {} (0x{:x})", uploadCommands[i].size, uploadCommands[i].size);
		}
	}
}

void Renderer::Kya::TextureLibrary::Init()
{
	ed3DGetTextureLoadedDelegate() += Renderer::Kya::TextureLibrary::AddTexture;
}

const Renderer::Kya::G2D::Material* Renderer::Kya::TextureLibrary::FindMaterial(const ed_g2d_material* pMaterial) const
{
	for (auto& texture : gTextures) {
		for (auto& material : texture.GetMaterials()) {
			if (material.pMaterial == pMaterial) {
				return &material;
			}
		}
	}
}

void Renderer::Kya::TextureLibrary::BindFromDmaMaterial(const ed_dma_material* pMaterial) const
{
	const G2D::Material* pRendererMaterial = FindMaterial(pMaterial->pMaterial);
	assert(pRendererMaterial);
	Renderer::SimpleTexture* pRendererTexture = pRendererMaterial->FindRenderTextureFromBitmap(pMaterial->pBitmap);
	assert(pRendererTexture);
	Renderer::BindTexture(pRendererTexture);
}

void Renderer::Kya::TextureLibrary::AddTexture(ed_g2d_manager* pManager, std::string name)
{
	gTextureLibrary.gTextures.emplace_back(pManager, name);
}

const Renderer::Kya::TextureLibrary& Renderer::Kya::GetTextureLibrary()
{
	return gTextureLibrary;
}
