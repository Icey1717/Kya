#include "Sprite.h"
#include "ed3D.h"
#include "renderer.h"
#include <memory>
#include <array>

#define SPRITE_LOG_TRACE(level, format, ...)
#define SPRITE_LOG(level, format, ...) MY_LOG_CATEGORY("Sprite", level, format, ##__VA_ARGS__)

namespace Renderer::Kya::Sprite
{
	class SpritePool
	{
	public:
		SpritePool()
		{
			for (auto& sprite : sprites)
			{
				GIFReg::GSPrim prim;
				prim.ABE = 1;
				sprite = std::make_unique<SimpleMesh>("Sprite", prim);
				sprite->GetVertexBufferData().Init(0x100, 0x1000);
			}
		}

		SimpleMesh* GetSimpleMesh()
		{
			if (spriteIndex >= sprites.size()) {
				spriteIndex = 0; // Reset index if we exceed the size
			}

			return sprites[spriteIndex++].get();
		}

		void Reset()
		{
			spriteIndex = 0;
		}

	private:
		std::array<std::unique_ptr<SimpleMesh>, 0x1000> sprites;
		int32_t spriteIndex = 0;
	} gSpritePool;

	constexpr uint32_t gGifTagCopyCode = 0x6c018000;

	static Gif_Tag ExtractGifTagFromVifList(ed_3d_sprite* pSprite, int index = 0)
	{
		// Pull the prim reg out from the gif packet, not a big fan of this.
		char* const pVifList = reinterpret_cast<char*>(pSprite) + pSprite->offsetA;
		edpkt_data* pPkt = reinterpret_cast<edpkt_data*>(pVifList);

		while (index > 0) {
			if (pPkt->asU32[0] == gVifEndCode) {
				index--;
			}

			pPkt++;
		}

		if (pPkt[1].asU32[3] != gGifTagCopyCode) {
			pPkt = reinterpret_cast<edpkt_data*>(pVifList);
		}

		assert(pPkt[1].asU32[3] == gGifTagCopyCode);

		u8* const pGifPkt = LOAD_SECTION_CAST(u8*, pPkt[1].asU32[1]);
		Gif_Tag gifTag;
		gifTag.setTag(pGifPkt, true);
		return gifTag;
	}

	void ProcessVertices(ed_3d_sprite* pSprite, SimpleMesh* pMesh)
	{
		const Gif_Tag firstGifTag = ExtractGifTagFromVifList(pSprite);

		auto& vertexBufferData = pMesh->GetVertexBufferData();

		// Assume that the first gif tag has the largest vtx count.
		int totalVtxCount = 0;

		for (int j = 0; j < pSprite->nbBatches; j++) {
			Gif_Tag gifTag = ExtractGifTagFromVifList(pSprite, j);
			totalVtxCount += gifTag.nLoop;
		}

		assert(totalVtxCount > 0);

		union VertexColor
		{
			uint32_t rgba;

			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
				uint8_t a;
			};
		};

		static_assert(sizeof(VertexColor) == 4);

		union TextureData
		{
			uint32_t st;

			struct
			{
				int16_t s;
				int16_t t;
			};
		};

		static_assert(sizeof(TextureData) == 4);

		union WidthHeightData
		{
			uint32_t wh;

			struct
			{
				int16_t w;
				int16_t h;
			};
		};

		// --- Order of data after the sprite class.
		// Vertex
		// STQ (UV)
		// Color
		// Width Height

		VertexColor* pRgba = LOAD_SECTION_CAST(VertexColor*, pSprite->pColorBuf);
		TextureData* pStq = LOAD_SECTION_CAST(TextureData*, pSprite->pSTBuf);
		edF32VECTOR4* pVectorVertex = LOAD_SECTION_CAST(edF32VECTOR4*, pSprite->pVertexBuf);
		GSVertexUnprocessed::Vertex* pVertex = LOAD_SECTION_CAST(GSVertexUnprocessed::Vertex*, pSprite->pVertexBuf);
		WidthHeightData* pWh = LOAD_SECTION_CAST(WidthHeightData*, pSprite->pWHBuf);

		// This increases by 2 every loop because we start the next vtx at the end of the previous vtx.
		int vtxOffset = 0;
		int meshOffset = 0;

		for (int batchIndex = 0; batchIndex < pSprite->nbBatches; batchIndex++) {
			SPRITE_LOG(LogLevel::Info, "Renderer::Kya::Sprite::ProcessVertices Starting section: {} / {}", batchIndex, pSprite->nbBatches);
			Gif_Tag gifTag = ExtractGifTagFromVifList(pSprite, batchIndex);

			int vtxCount = gifTag.nLoop;

			// Sanity check that our gif tag matches the calculated batches.
			if (pSprite->nbBatches > 1) {
				if (batchIndex < pSprite->nbBatches - 1) {
					//assert(gifTag.nLoop == 0x48); // Full batch. 72 vertices per batch, 18 quads.
					vtxCount = 0x48;
				}
				else {
					//assert(gifTag.nLoop == pSprite->nbRemainderVertices); // Last batch, should match the remainder vertices.
					vtxCount = pSprite->nbRemainderVertices;
				}
			}
			else {
				//assert(gifTag.nLoop == pSprite->nbRemainderVertices); // Single batch, should match the remainder vertices.
				vtxCount = pSprite->nbRemainderVertices;
			}

			SPRITE_LOG(LogLevel::Info, "Renderer::Kya::Sprite::ProcessVertices NLOOP: {}", gifTag.nLoop);

			// Expand the vertex. Four vertices per quad.
			for (int i = 0; i < vtxCount; i += 4) {
				const int index = i + meshOffset;

				assert(pVectorVertex[index + 0].wi != gGifTagCopyCode);
				assert(pVectorVertex[index + 1].wi != gGifTagCopyCode);
				assert(pVectorVertex[index + 2].wi != gGifTagCopyCode);
				assert(pVectorVertex[index + 3].wi != gGifTagCopyCode);

				WidthHeightData whi = pWh[index / 4];
				edF32VECTOR4 vtx = pVectorVertex[index];

				edF32VECTOR2 whf;
				whf.x = int12_to_float(whi.w) * 2.0f;
				whf.y = int12_to_float(whi.h) * 2.0f;

				const edF32VECTOR4 normalizedX = gCamNormal_X * whf.x;
				const edF32VECTOR4 normalizedY = gCamNormal_Y * whf.y;

				const edF32VECTOR4 tl = vtx - normalizedX + normalizedY;
				const edF32VECTOR4 bl = vtx - normalizedX - normalizedY;
				const edF32VECTOR4 tr = vtx + normalizedX + normalizedY;
				const edF32VECTOR4 br = vtx + normalizedX - normalizedY;

				// Store the vertices in the vertex buffer data.
				pVectorVertex[index + 0] = tl;
				pVectorVertex[index + 1] = bl;
				pVectorVertex[index + 2] = tr;
				pVectorVertex[index + 3] = br;

				// Emulated unpacking, vu1 would usually fill these values in within the data row for W and H.
				pVectorVertex[index + 0].wi = g_stSpriteWidthHeightHeader[1].asU32[3];
				pVectorVertex[index + 1].wi = g_stSpriteWidthHeightHeader[1].asU32[3];
			}

			for (int i = 0; i < vtxCount; i++) {
				const int index = i + meshOffset;

				Renderer::GSVertexUnprocessedNormal vtx;
				vtx.RGBA[0] = pRgba[index].r;
				vtx.RGBA[1] = pRgba[index].g;
				vtx.RGBA[2] = pRgba[index].b;
				vtx.RGBA[3] = pRgba[index].a;

				vtx.STQ.ST[0] = pStq[index].s;
				vtx.STQ.ST[1] = pStq[index].t;
				vtx.STQ.Q = 1.0f;

				memset(&vtx.normal, 0, sizeof(vtx.normal));

				vtx.XYZFlags = pVertex[index];

				const uint primReg = firstGifTag.tag.PRIM;
				const GIFReg::GSPrim primPacked = *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);

				const uint skip = vtx.XYZFlags.flags & 0x8000;

				SPRITE_LOG_TRACE(LogLevel::Info, "Renderer::Kya::Sprite::ProcessVertices Processing vertex: {}, primPacked: 0x{:x}, nloop: 0x{:x}, skip: 0x{:x}", i, primReg, gifTag.nLoop, skip);

				SPRITE_LOG_TRACE(LogLevel::Info, "Renderer::Kya::Sprite::ProcessVertices Processing vertex: {}, (S: {} T: {} Q: {}) (R: {} G: {} B: {} A: {}) (X: {} Y: {} Z: {} Skip: {})\n",
					i, vtx.STQ.ST[0], vtx.STQ.ST[1], vtx.STQ.Q, vtx.RGBA[0], vtx.RGBA[1], vtx.RGBA[2], vtx.RGBA[3], vtx.XYZFlags.fXYZ[0], vtx.XYZFlags.fXYZ[1], vtx.XYZFlags.fXYZ[2], vtx.XYZFlags.flags);

				Renderer::KickVertex(vtx, primPacked, skip, vertexBufferData);

				SPRITE_LOG_TRACE(LogLevel::Info, "Renderer::Kya::Sprite::ProcessVertices Kick complete vtx tail: 0x{:x} index tail: 0x{:x}",
					vertexBufferData.GetVertexTail(), vertexBufferData.GetIndexTail());
			}

			meshOffset += vtxCount;
		}
	}
}

void Renderer::Kya::Sprite::RenderNode(const edNODE* pNode)
{
	short sVar1;
	ushort uVar2;
	ushort uVar3;
	ed_3d_sprite* pSprite;
	uint uVar8;

	pSprite = reinterpret_cast<ed_3d_sprite*>(pNode->pData);

	sVar1 = pNode->header.typeField.flags;
	uVar2 = pSprite->nbBatches;

	if (uVar2 != 0) {
		uVar3 = pSprite->pRenderFrame30;
		if (((uVar3 & 0x80) == 0) || ((uVar3 & 0x200) == 0)) {
			SPRITE_LOG(LogLevel::Info, "Renderer::Kya::Sprite::RenderNode Rendering sprite with count: 0x{:x}", uVar2);

			if ((uVar3 & 0x200) == 0) {
				for (uVar8 = 0; uVar8 < uVar2; uVar8 = uVar8 + 1) {
					auto* pSimpleMesh = gSpritePool.GetSimpleMesh();
					pSimpleMesh->GetVertexBufferData().ResetAfterDraw();
					ProcessVertices(pSprite, pSimpleMesh);

					Renderer::RenderMesh(pSimpleMesh, pNode->header.typeField.flags);
				}
			}
			else {
				// Flare FX
			}
		}
		else {
			// Scale flare
		}
	}
}
