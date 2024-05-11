#pragma once
#include <stdint.h>
#include <assert.h>
#include "delegate.h"
#include "GIFReg.h"
#include "GSState.h"
#include "GSVector.h"

#include "../../../include/profiling.h"

// Forward decs for RenderDelegate
typedef struct VkFramebuffer_T* VkFramebuffer;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
struct VkExtent2D;

namespace PS2 {
	struct DrawBufferBase {
		virtual size_t GetIndexTail() const = 0;
		virtual void ResetAfterDraw() = 0;
	};

	template<typename VertexType, typename IndexType>
	struct DrawBufferData : public DrawBufferBase {

		void Init(const int vertexCount, const int indexCount) {
			vertex.buff = (VertexType*)_aligned_malloc(sizeof(VertexType) * vertexCount, 32);
			index.buff = (IndexType*)_aligned_malloc(sizeof(IndexType) * indexCount, 32);
			vertex.maxcount = vertexCount;
			index.maxcount = indexCount;
		}

		~DrawBufferData() {
			_aligned_free(vertex.buff);
			_aligned_free(index.buff);
		}

		struct Index
		{
			IndexType* buff{};
			size_t tail{};
			size_t maxcount{};
		};

		struct Vertex
		{
			VertexType* buff{};
			size_t head{}, tail{}, next{}, maxcount{}; // head: first vertex, tail: last vertex + 1, next: last indexed + 1
			size_t xy_tail{};
			uint64_t xy[4]{};
		};

		void Reset() {
			assert(index.tail == 0 || index.buff[index.tail - 1] + 1 == vertex.next);

			if (index.tail == 0)
			{
				vertex.next = 0;
			}

			vertex.head = vertex.tail = vertex.next; // remove unused vertices from the end of the vertex buffer
		}

		virtual size_t GetIndexTail() const {
			return index.tail;
		}

		virtual void ResetAfterDraw() {
			index.tail = 0;
			vertex.head = 0;
			vertex.tail = 0;
			vertex.next = 0;
			vertex.xy_tail = 0;
		}

		Index index;
		Vertex vertex;
	};
}

namespace Renderer
{
	enum GS_PRIM
	{
		GS_POINTLIST = 0,
		GS_LINELIST = 1,
		GS_LINESTRIP = 2,
		GS_TRIANGLELIST = 3,
		GS_TRIANGLESTRIP = 4,
		GS_TRIANGLEFAN = 5,
		GS_SPRITE = 6,
		GS_INVALID = 7,
	};

	/*
		float2 st : TEXCOORD0;
		uint4 c : COLOR0;
		float q : TEXCOORD1;
		uint2 p : POSITION0;
		uint z : POSITION1;
		uint2 uv : TEXCOORD2;
		float4 f : COLOR1;
	*/

	struct alignas(32) GSVertexUnprocessed
	{
		struct {
			int32_t ST[2];
			float Q;
			float _pad;
		} STQ;
		uint32_t RGBA[4];

		struct {
			float XYZ[3];
			uint32_t Skip;
		} XYZSkip;
	};

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
		uint32_t maxMipLevel = 0;

		void Log(const char* prefix) const;
	};

	using PaletteMap = std::unordered_map<int, ImageData>;

	struct TextureData {
		// ed_g2d_material that the texture is associated with.
		void* pMaterial;
		ImageData image;
		PaletteMap palettes;
	};

	void Setup();
	void RenderImage(char* imageData, int width, int height);
	void WaitUntilReady();
	void Present();
	void SetScissor(int x, int y, uint32_t width, uint32_t height);
	void Draw();
	void Draw(PS2::DrawBufferBase& drawBuffer);
	void Draw(PS2::DrawBufferBase& drawBuffer, TextureData& textureData, PS2::GSState& state, bool bHardware = false);

	PS2::DrawBufferData<GSVertex, uint16_t>& GetDefaultDrawBuffer();
	PS2::DrawBufferData<GSVertexUnprocessed, uint16_t>& GetHardwareDrawBuffer();

	void SetVertexSkip(uint32_t inSkip);

	void SetXY(uint32_t x, uint32_t y);

	void SetST(float s, float t);
	void SetRGBAQ(uint32_t R, uint32_t G, uint32_t B, uint32_t A, float Q);
	void SetFrame(int fbp, int fbw, int psm, int fbmask);
	void SetTest(uint32_t ate, uint32_t atst, uint32_t aref, uint32_t afail, uint32_t date, uint32_t datm, uint32_t zte, uint32_t ztst);

	void SetPrim(GIFReg::GSPrimPacked prim, PS2::DrawBufferData<GSVertex, uint16_t>* pDrawBuffer = nullptr);
	void SetPrim(uint32_t prim, uint32_t iip, uint32_t tme, uint32_t fge, uint32_t abe, uint32_t aa1, uint32_t fst, uint32_t ctxt, uint32_t fix);

	// Variation that uses GS State to fill in PRIM, STQ, RGBA
	void KickVertex(uint16_t x, uint16_t y, uint32_t z);

	// Variation that contains all data in vtxInfo.
	template<typename VertexType, typename IndexType>
	void UpdateXyTail(const VertexType& vtx, PS2::DrawBufferData<VertexType, IndexType>& drawBuffer, const size_t& xy_tail);

	template<typename VertexType, typename IndexType>
	void TraceUpdateSkip(uint32_t& skip, PS2::DrawBufferData<VertexType, IndexType>& drawBuffer, const GS_PRIM& prim, const size_t& xy_tail, const size_t& m);

	template<typename VertexType, typename IndexType>
	void KickVertex(VertexType& vtx, GIFReg::GSPrimPacked primReg, uint32_t skip, PS2::DrawBufferData<VertexType, IndexType>& drawBuffer)
	{
		GS_PRIM prim = (GS_PRIM)primReg.PRIM;
		assert(drawBuffer.vertex.tail < drawBuffer.vertex.maxcount + 3);

		size_t head = drawBuffer.vertex.head;
		size_t tail = drawBuffer.vertex.tail;
		size_t next = drawBuffer.vertex.next;
		size_t xy_tail = drawBuffer.vertex.xy_tail;

		drawBuffer.vertex.buff[drawBuffer.vertex.tail] = vtx;

		UpdateXyTail(vtx, drawBuffer, xy_tail);

		drawBuffer.vertex.tail = ++tail;
		drawBuffer.vertex.xy_tail = ++xy_tail;

		size_t n = 0;

		switch (prim)
		{
		case GS_POINTLIST: n = 1; break;
		case GS_LINELIST: n = 2; break;
		case GS_LINESTRIP: n = 2; break;
		case GS_TRIANGLELIST: n = 3; break;
		case GS_TRIANGLESTRIP: n = 3; break;
		case GS_TRIANGLEFAN: n = 3; break;
		case GS_SPRITE: n = 2; break;
		case GS_INVALID: n = 1; break;
		}

		size_t m = tail - head;

		if (m < n)
		{
			return;
		}

		if (skip == 0 && (prim != GS_TRIANGLEFAN || m <= 4)) // m_vertex.xy only knows about the last 4 vertices, head could be far behind for fan
		{
			TraceUpdateSkip(skip, drawBuffer, prim, xy_tail, m);
		}

		if (skip != 0)
		{
			switch (prim)
			{
			case GS_POINTLIST:
			case GS_LINELIST:
			case GS_TRIANGLELIST:
			case GS_SPRITE:
			case GS_INVALID:
				drawBuffer.vertex.tail = head; // no need to check or grow the buffer length
				break;
			case GS_LINESTRIP:
			case GS_TRIANGLESTRIP:
				drawBuffer.vertex.head = head + 1;
				// fall through
			case GS_TRIANGLEFAN:
				if (tail >= drawBuffer.vertex.maxcount) assert(false); // in case too many vertices were skipped
				break;
			default:
				__assume(0);
			}

			return;
		}

		if (tail >= drawBuffer.vertex.maxcount) assert(false);

		uint16_t* RESTRICT buff = &drawBuffer.index.buff[drawBuffer.index.tail];

		switch (prim)
		{
		case GS_POINTLIST:
			buff[0] = head + 0;
			drawBuffer.vertex.head = head + 1;
			drawBuffer.vertex.next = head + 1;
			drawBuffer.index.tail += 1;
			break;
		case GS_LINELIST:
			buff[0] = head + 0;
			buff[1] = head + 1;
			drawBuffer.vertex.head = head + 2;
			drawBuffer.vertex.next = head + 2;
			drawBuffer.index.tail += 2;
			break;
		case GS_LINESTRIP:
			if (next < head)
			{
				drawBuffer.vertex.buff[next + 0] = drawBuffer.vertex.buff[head + 0];
				drawBuffer.vertex.buff[next + 1] = drawBuffer.vertex.buff[head + 1];
				head = next;
				drawBuffer.vertex.tail = next + 2;
			}
			buff[0] = head + 0;
			buff[1] = head + 1;
			drawBuffer.vertex.head = head + 1;
			drawBuffer.vertex.next = head + 2;
			drawBuffer.index.tail += 2;
			break;
		case GS_TRIANGLELIST:
			buff[0] = head + 0;
			buff[1] = head + 1;
			buff[2] = head + 2;
			drawBuffer.vertex.head = head + 3;
			drawBuffer.vertex.next = head + 3;
			drawBuffer.index.tail += 3;
			break;
		case GS_TRIANGLESTRIP:
			if (next < head)
			{
				drawBuffer.vertex.buff[next + 0] = drawBuffer.vertex.buff[head + 0];
				drawBuffer.vertex.buff[next + 1] = drawBuffer.vertex.buff[head + 1];
				drawBuffer.vertex.buff[next + 2] = drawBuffer.vertex.buff[head + 2];
				head = next;
				drawBuffer.vertex.tail = next + 3;
			}
			buff[0] = head + 0;
			buff[1] = head + 1;
			buff[2] = head + 2;
			drawBuffer.vertex.head = head + 1;
			drawBuffer.vertex.next = head + 3;
			drawBuffer.index.tail += 3;
			break;
		case GS_TRIANGLEFAN:
			// TODO: remove gaps, next == head && head < tail - 3 || next > head && next < tail - 2 (very rare)
			buff[0] = head + 0;
			buff[1] = tail - 2;
			buff[2] = tail - 1;
			drawBuffer.vertex.next = tail;
			drawBuffer.index.tail += 3;
			break;
		case GS_SPRITE:
			buff[0] = head + 0;
			buff[1] = head + 1;
			drawBuffer.vertex.head = head + 2;
			drawBuffer.vertex.next = head + 2;
			drawBuffer.index.tail += 2;
			break;
		case GS_INVALID:
			drawBuffer.vertex.tail = head;
			break;
		default:
			__assume(0);
		}
	}

	void SetAlpha(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint8_t fix);

	void SetTEX(GIFReg::GSTex tex);
	void SetTest(GIFReg::GSTest test);
	void SetClamp(GIFReg::GSClamp clamp);
	void SetColClamp(GIFReg::GSColClamp colClamp);

	void SetTextureData(TextureData inTextureData);
	void SetWorldViewProjScreen(float* pWorld, float* pView, float* pProj, float* pScreen);
	TextureData& GetTextureData();

	using RenderDelegate = Multidelegate<const VkFramebuffer&, const VkExtent2D&>;

	RenderDelegate& GetRenderDelegate();

	void RemoveByMaterial(void* pMaterial);

	void SetHeadless(bool bValue);

	bool& GetUseComplexBlending();
	void ResetRenderer();

	namespace Debug {
		template<typename... Args>
		void BeginLabel(const char* format, Args... args) {
			char buffer[512];
			sprintf_s(buffer, 512, format, args...);
			BeginLabel(buffer);
		}

		void BeginLabel(const VkCommandBuffer& cmdBuffer, const char* szLabel);
		void EndLabel(const VkCommandBuffer& cmdBuffer);
		void BeginLabel(const char* szLabel);
		void EndLabel();
	}
}
