#pragma once
#include <stdint.h>
#include "delegate.h"

// Forward decs for RenderDelegate
typedef struct VkFramebuffer_T* VkFramebuffer;
struct VkExtent2D;

namespace Renderer
{
	/*
		float2 st : TEXCOORD0;
		uint4 c : COLOR0;
		float q : TEXCOORD1;
		uint2 p : POSITION0;
		uint z : POSITION1;
		uint2 uv : TEXCOORD2;
		float4 f : COLOR1;
	*/

	struct alignas(32) GSVertex
	{
		float ST[2]; // S:0, T:4
		uint32_t RGBA[4]; // RGBA:8, Q:12
		float Q;
		uint32_t XY[2]; // XY:16, Z:20
		uint32_t Z;
		uint32_t UV[2]; // UV:24
		float FOG[4]; // FOG:28
	};

	struct ImageData {
		void* pImage = nullptr;
		uint32_t canvasWidth = 0;
		uint32_t canvasHeight = 0;
		uint32_t readWidth = 0;
		uint32_t readHeight = 0;
		uint32_t bpp = 0;
	};

	struct TextureData {
		ImageData image;
		ImageData palette;
	};

	struct PrimPacked
	{
		uint32_t PRIM : 3;
		uint32_t IIP : 1;
		uint32_t TME : 1;
		uint32_t FGE : 1;
		uint32_t ABE : 1;
		uint32_t AA1 : 1;
		uint32_t FST : 1;
		uint32_t CTXT : 1;
		uint32_t FIX : 1;
		uint32_t _PAD1 : 21;
		uint32_t _PAD2 : 32;
	};

	union GSTex
	{
		struct
		{
			uint32_t TBP0 : 14;
			uint32_t TBW : 6;
			uint32_t PSM : 6;
			uint32_t TW : 4;
			uint32_t _PAD1 : 2;
			uint32_t _PAD2 : 2;
			uint32_t TCC : 1;
			uint32_t TFX : 2;
			uint32_t CBP : 14;
			uint32_t CPSM : 4;
			uint32_t CSM : 1;
			uint32_t CSA : 5;
			uint32_t CLD : 3;
		};

		struct
		{
			uint64_t _PAD3 : 30;
			uint64_t TH : 4;
			uint64_t _PAD4 : 30;
		};

		struct
		{
			uint64_t CMD;
		};

		bool operator==(const GSTex& other) const {
			return TBP0 == other.TBP0 &&
				TBW == other.TBW &&
				PSM == other.PSM &&
				TW == other.TW &&
				TH == other.TH &&
				TCC == other.TCC &&
				TFX == other.TFX &&
				CBP == other.CBP &&
				CPSM == other.CPSM &&
				CSM == other.CSM &&
				CSA == other.CSA &&
				CLD == other.CLD;
		}
	};

	void Setup();
	void RenderImage(char* imageData, int width, int height);
	void WaitUntilReady();
	void Present();
	void SetScissor(int x, int y, uint32_t width, uint32_t height);
	void Draw();

	void SetVertexSkip(uint32_t inSkip);

	void SetXY(uint32_t x, uint32_t y);

	void SetST(float s, float t);
	void SetRGBAQ(uint32_t R, uint32_t G, uint32_t B, uint32_t A, float Q);
	void SetFrame(int fbp, int fbw, int psm, int fbmask);
	void SetTest(uint32_t ate, uint32_t atst, uint32_t aref, uint32_t afail, uint32_t date, uint32_t datm, uint32_t zte, uint32_t ztst);
	void SetPrim(PrimPacked prim);
	void SetPrim(uint32_t prim, uint32_t iip, uint32_t tme, uint32_t fge, uint32_t abe, uint32_t aa1, uint32_t fst, uint32_t ctxt, uint32_t fix);
	void KickVertex(uint16_t x, uint16_t y, uint32_t z);

	void SetTEX(Renderer::GSTex tex);

	void SetImagePointer(TextureData inImage);

	using RenderDelegate = Multidelegate<const VkFramebuffer&, const VkExtent2D&>;

	RenderDelegate& GetRenderDelegate();

	void SetHeadless(bool bValue);
}