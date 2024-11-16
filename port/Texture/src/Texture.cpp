#include "Texture.h"
#include "ed3D.h"
#include "Log.h"
#include "renderer.h"
#include "port.h"

#define TEXTURE_LOG(level, format, ...) MY_LOG_CATEGORY("TextureLibrary", level, format, ##__VA_ARGS__)

namespace Renderer
{
	namespace Kya
	{
		TextureLibrary gTextureLibrary;

		using TextureCache = std::unordered_map<const ed_g2d_material* , const Renderer::Kya::G2D::Material*>;
		static TextureCache gTextureCache;

		void ProcessRenderCommandList(CombinedImageData& imageData, const G2D::CommandList& commandList)
		{
			bool bSet = false;
			edpkt_data* pPkt = commandList.pList;

			for (int i = 0; i < commandList.size; i++) {
				switch (pPkt->asU32[2]) {
				case SCE_GS_TEX0_1:
				{
					assert(!bSet);
					imageData.registers.tex.CMD = pPkt->cmdA;
					bSet = true;

					const GIFReg::GSTex tex = *reinterpret_cast<GIFReg::GSTex*>(&pPkt->cmdA);
					TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessRenderCommandList SCE_GS_TEX0_1: CBP: 0x{:x} CLD: {} CPSM: {} CSA: {} CSM: {}",
						tex.CBP, tex.CLD, tex.CPSM, tex.CSA, tex.CSM);

					TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessRenderCommandList SCE_GS_TEX0_1: PSM: {} TBP0: 0x{:x} TBW: {} TCC: {} TFX: {} TW: {} TH: {}",
						tex.PSM, tex.TBP0, tex.TBW, tex.TCC, tex.TFX, tex.TW, tex.TH);
				}
				break;
				case SCE_GS_CLAMP_1:
				{
					imageData.registers.clamp.CMD = pPkt->cmdA;

					const GIFReg::GSClamp clamp = *reinterpret_cast<GIFReg::GSClamp*>(&pPkt->cmdA);
					TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessRenderCommandList SCE_GS_CLAMP_1: WMS: {} WMT: {} MINU: {} MAXU: {} MAXV: {} MINV: {}",
						clamp.WMS, clamp.WMT, clamp.MINU, clamp.MAXU, clamp.MAXV, clamp.MINV);
				}
				break;
				case SCE_GS_TEST_1:
				{
					imageData.registers.test.CMD = pPkt->cmdA;

					const GIFReg::GSTest test = *reinterpret_cast<GIFReg::GSTest*>(&pPkt->cmdA);
					TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessRenderCommandList SCE_GS_TEST_1: ATE: {} ATST: {} AREF: {} AFAIL: {} DATE: {} DATM: {} ZTE: {} ZTST: {}",
						test.ATE, test.ATST, test.AREF, test.AFAIL, test.DATE, test.DATM, test.ZTE, test.ZTST);

				}
				break;
				case SCE_GS_ALPHA_1:
				{
					imageData.registers.alpha.CMD = pPkt->cmdA;

					const GIFReg::GSAlpha alpha = *reinterpret_cast<GIFReg::GSAlpha*>(&pPkt->cmdA);
					TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessRenderCommandList SCE_GS_ALPHA_1: A: {} B: {} C: {} D: {} FIX: {}",
						alpha.A, alpha.B, alpha.C, alpha.D, alpha.FIX);

					switch (alpha.A) {
					case 0:
						TEXTURE_LOG(LogLevel::Info, "A: Cs");
						break;
					case 1:
						TEXTURE_LOG(LogLevel::Info, "A: Cd");
						break;
					case 2:
						TEXTURE_LOG(LogLevel::Info, "A: 0");
						break;
					}

					switch (alpha.B) {
					case 0:
						TEXTURE_LOG(LogLevel::Info, "B: Cs");
						break;
					case 1:
						TEXTURE_LOG(LogLevel::Info, "B: Cd");
						break;
					case 2:
						TEXTURE_LOG(LogLevel::Info, "B: 0");
						break;
					}

					switch (alpha.C) {
					case 0:
						TEXTURE_LOG(LogLevel::Info, "C: As");
						break;
					case 1:
						TEXTURE_LOG(LogLevel::Info, "C: Ad");
						break;
					case 2:
						TEXTURE_LOG(LogLevel::Info, "C: FIX");
						break;
					}

					switch (alpha.D) {
					case 0:
						TEXTURE_LOG(LogLevel::Info, "D: Cs");
						break;
					case 1:
						TEXTURE_LOG(LogLevel::Info, "D: Cd");
						break;
					case 2:
						TEXTURE_LOG(LogLevel::Info, "D: 0");
						break;
					}
				}
				break;
				case SCE_GS_COLCLAMP:
				{
					imageData.registers.colClamp.CMD = pPkt->cmdA;

					const GIFReg::GSColClamp colClamp = *reinterpret_cast<GIFReg::GSColClamp*>(&pPkt->cmdA);
					TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessRenderCommandList SCE_GS_COLCLAMP: CLAMP: {}", colClamp.CLAMP);
				}
				break;
				}

				pPkt++;
			}

			assert(bSet);
		}

		void ProcessUploadCommandList(CombinedImageData& imageData, const G2D::CommandList& commandList)
		{
			// The first sets of data will be MIP levels for the texture, with the last one the palette data.

			int currentMipLevel = 0;

			edpkt_data* pPkt = commandList.pList;

			ImageData* pCurrentImage = nullptr;

			for (int i = 0; i < commandList.size; i++) {
				if (pPkt->asU32[2] == SCE_GIF_PACKED_AD) {
					if (pCurrentImage) {
						assert(pCurrentImage->pImage);
						assert(pCurrentImage->trxReg.RRW != 0);
						assert(pCurrentImage->trxReg.RRH != 0);
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
					pCurrentImage->trxReg.CMD = pPkt->cmdA;
				}

				if (pPkt->cmdB == SCE_GS_TRXPOS) {
					assert(pCurrentImage);
					pCurrentImage->trxPos.CMD = pPkt->cmdA;
				}

				if (pPkt->cmdB == SCE_GS_BITBLTBUF) {
					assert(pCurrentImage);
					pCurrentImage->bitBltBuf.CMD = pPkt->cmdA;
				}

				pPkt++;
			}
		}

		CombinedImageData CreateFromBitmapAndPalette(G2D::Bitmap& bitmap, G2D::Bitmap& palette)
		{
			CombinedImageData combinedImageData{};
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
			ProcessUploadCommandList(combinedImageData, commandList);

			return combinedImageData;
		}
	}
}

Renderer::Kya::G2D::G2D(ed_g2d_manager* pManager, std::string name)
	: pManager(pManager)
	, name(name)
{
	TEXTURE_LOG(LogLevel::Info, "\n\nRenderer::Kya::G2D::G2D Beginning processing of texture: {}", name.c_str());

	const int nbMaterials = ed3DG2DGetG2DNbMaterials(pManager->pMATA_HASH);
	materials.reserve(nbMaterials);

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::G2D Nb Materials: {}", nbMaterials);

	const ed_hash_code* pHashCodes = reinterpret_cast<ed_hash_code*>(pManager->pMATA_HASH + 1);

	for (int i = 0; i < nbMaterials; ++i) {
		const ed_hash_code* pHashCode = pHashCodes + i;

		TEXTURE_LOG(LogLevel::Info, "\nRenderer::Kya::G2D::G2D Processing material {}/{} hash: {}", i, nbMaterials, pHashCode->hash.ToString());

		ed_g2d_material* pMaterial = ed3DG2DGetG2DMaterialFromIndex(pManager, i);

		if (pMaterial) {
			ProcessMaterial(pMaterial, i);
		}
	}
}

void Renderer::Kya::G2D::ProcessMaterial(ed_g2d_material* pMaterial, const int materialIndex)
{
	assert(pMaterial);

	Material& material = materials.emplace_back();
	material.pMaterial = pMaterial;
	material.pParent = this;

	gTextureCache[pMaterial] = &material;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessMaterial Nb layers: {} flags: 0x{:x}", pMaterial->nbLayers, pMaterial->flags);

	material.renderCommands.pList = LOAD_SECTION_CAST(edpkt_data*, pMaterial->pCommandBufferTexture);
	material.renderCommands.size = pMaterial->commandBufferTextureSize;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessMaterial Render command size: {} (0x{:x})", material.renderCommands.size, material.renderCommands.size);

	material.layers.reserve(pMaterial->nbLayers);

	for (int i = 0; i < pMaterial->nbLayers; ++i) {
		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessMaterial Processing layer: {}", i);

		ed_Chunck* pLAY = LOAD_SECTION_CAST(ed_Chunck*, pMaterial->aLayers[i]);
		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::ProcessMaterial Layer chunk header: {}", pLAY->GetHeaderString());

		ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
		material.ProcessLayer(pLayer, materialIndex);
	}
}

void Renderer::Kya::G2D::Material::ProcessLayer(ed_g2d_layer* pLayer, const int materialIndex)
{
	const int layerIndex = layers.size();

	Layer& layer = layers.emplace_back();
	layer.pLayer = pLayer;
	layer.pParent = this;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Material::ProcessLayer Layer flags 0x0: 0x{:x} flags 0x4: 0x{:x} field 0x1b: {} bHasTexture: {} paletteId: {}", 
		pLayer->flags_0x0, pLayer->flags_0x4, pLayer->field_0x1b, pLayer->bHasTexture, pLayer->paletteId);

	if (pLayer->bHasTexture) {
		ed_Chunck* pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
		TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Material::ProcessLayer Texture chunk header: {}", pTEX->GetHeaderString());

		ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
		layer.ProcessTexture(pTexture, materialIndex, layerIndex);
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

void Renderer::Kya::G2D::Layer::ProcessTexture(ed_g2d_texture* pTexture, const int materialIndex, const int layerIndex)
{
	G2D::Texture& texture = textures.emplace_back();
	texture.pTexture = pTexture;
	texture.pParent = this;

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Layer::ProcessTexture Texture hash: {} field 0x14: {} bHasPalette: {}",
		pTexture->hashCode.hash.ToString(), pTexture->pAnimSpeedNormalExtruder, pTexture->bHasPalette);

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

	CombinedImageData combinedImageData = CreateFromBitmapAndPalette(texture.bitmap, texture.palette);

	ProcessRenderCommandList(combinedImageData, pParent->renderCommands);

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Layer::ProcessTexture bitmap mips: {}", combinedImageData.bitmaps.size());

	// strip everything before the last forward slash 
	const std::string textureName = pParent->pParent->name.substr(pParent->pParent->name.find_last_of('\\') + 1);

	texture.pSimpleTexture = new SimpleTexture(textureName, { layerIndex, materialIndex, pParent->layers.capacity(), pParent->pParent->materials.capacity() }, combinedImageData.registers);
	texture.pSimpleTexture->CreateRenderer(combinedImageData);

	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::G2D::Layer::ProcessTexture Texture created: {} {}/{} {}/{}", textureName, layerIndex, pParent->layers.capacity(), materialIndex, pParent->pParent->materials.capacity());
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
	constexpr bool bUseTextureCache = true;

	if (bUseTextureCache) {
		return gTextureCache[pMaterial];
	}

	for (auto& texture : gTextures) {
		for (auto& material : texture.GetMaterials()) {
			if (material.pMaterial == pMaterial) {
				return &material;
			}
		}
	}

	return nullptr;
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
