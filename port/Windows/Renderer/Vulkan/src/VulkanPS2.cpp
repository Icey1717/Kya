#include "VulkanPS2.h"
#include "VulkanReflect.h"
#include "renderer.h"
#include "VulkanRenderer.h"
#include <stdexcept>
#include <array>
#include <sstream>
#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include <unordered_map>
#include "hash.h"
#include "FrameBuffer.h"
#include "VulkanImage.h"
#include "GSState.h"
#include "Pipeline.h"
#include "GSVector.h"
#include "UniformBuffer.h"
#include "TextureCache.h"
#include "log.h"
#include "pcsx2/Selectors.h"

namespace PS2_Internal {
	bool bUseComplexBlending = true;

	PS2::GSState state;

	PS2::GSState& GetGSState() {
		return state;
	}

	Renderer::GSVertex MakeVertex(uint16_t x, uint16_t y, uint32_t z) {
		const auto& state = GetGSState();
		return Renderer::GSVertex{ { state.S, state.T }, {state.R, state.G, state.B, state.A }, state.Q, { x, y }, z, 0, 0 };
	}

	PS2::FrameVertexBuffers<Renderer::GSVertex, uint16_t> gVertexBuffers;
}

PS2::GSState& PS2::GetGSState() {
	return PS2_Internal::GetGSState();
}

HardwareState hwState;

HardwareState& GetHardwareState() {
	return hwState;
}

namespace Renderer {
#define ASSERT(...)
#define RESTRICT

	bool& GetUseComplexBlending() {
		return PS2_Internal::bUseComplexBlending;
	}

	void ResetRenderer() {
		PS2::GetTextureCache().GetEntries().clear();
		PS2::GetPipelines().clear();
	}

	TextureData gImageData;

	void ResetVertIndexBuffers()
	{
		PS2_Internal::gVertexBuffers.GetDrawBufferData().Reset();
	}

	enum GS_PRIM_CLASS
	{
		GS_POINT_CLASS = 0,
		GS_LINE_CLASS = 1,
		GS_TRIANGLE_CLASS = 2,
		GS_SPRITE_CLASS = 3,
		GS_INVALID_CLASS = 7,
	};

	uint32_t Skip = 0;

	void LogTex(const char* prefix, GIFReg::GSTex tex)
	{
		//// Create a formatted log message string
		//std::ostringstream oss;
		//oss << "tbp: " << tex.TBP0 << ", tbw: " << tex.TBW << ", psm: " << tex.PSM
		//	<< ", tw: " << tex.TW << ", th: " << tex.TH << ", tcc: " << tex.TCC
		//	<< ", tfx: " << tex.TFX << ", cbp: " << tex.CBP << " (0x" << std::hex << tex.CBP << "), cpsm: " << tex.CPSM
		//	<< ", csm: " << tex.CSM << ", csa: " << tex.CSA << ", cld: " << tex.CLD;
		//
		//// Log the formatted message
		//Log::GetInstance().AddLog(LogLevel::Verbose, "RendererPS2", "%s - %s", prefix, oss.str().c_str());
		//Log::GetInstance().AddLog(LogLevel::Verbose, "RendererPS2", "%s - %llx", prefix, tex.CMD);
	}

	void SetVertexSkip(uint32_t inSkip)
	{
		Skip = inSkip;
	}

	void SetXY(uint32_t x, uint32_t y)
	{
		PS2::GetGSState().SetXYOffset(x, y);
	}

	void SetST(float s, float t)
	{
		PS2::GetGSState().SetST(s, t);
	}

	void SetRGBAQ(uint32_t R, uint32_t G, uint32_t B, uint32_t A, float Q) {
		PS2::GetGSState().SetRGBAQ(R, G, B, A, Q);
	}

	void SetFrame(int fbp, int fbw, int psm, int fbmask) {
		PS2::GetGSState().SetFrame(fbp, fbw, psm, fbmask);
	}

	void SetAlpha(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint8_t fix)
	{
		PS2::GetGSState().SetAlpha(a, b, c, d, fix);
	}

	void SetTest(uint32_t ate, uint32_t atst, uint32_t aref, uint32_t afail, uint32_t date, uint32_t datm, uint32_t zte, uint32_t ztst) {
		GIFReg::GSTest NewTest = { ate, atst, aref, afail, date, datm, zte, ztst };

		if (PS2::GetGSState().TEST != NewTest) {
			Draw();
		}

		PS2::GetGSState().TEST = NewTest;
	}

	void SetPrim(GIFReg::GSPrimPacked prim, PS2::DrawBufferData<Renderer::GSVertex, uint16_t>* pDrawBuffer /*= nullptr*/) {
		if (!pDrawBuffer) {
			ResetVertIndexBuffers();
		}
		else {
			pDrawBuffer->Reset();
		}

		PS2::GetGSState().PRIM = { prim.PRIM, prim.IIP, prim.TME, prim.FGE, prim.ABE, prim.AA1, prim.FST, prim.CTXT, prim.FIX };
	}

	void SetPrim(uint32_t prim, uint32_t iip, uint32_t tme, uint32_t fge, uint32_t abe, uint32_t aa1, uint32_t fst, uint32_t ctxt, uint32_t fix) {
		ResetVertIndexBuffers();
		PS2::GetGSState().PRIM = { prim, iip, tme, fge, abe, aa1, fst, ctxt, fix };
	}

	void SetTEX(GIFReg::GSTex tex)
	{
		LogTex("SetTEX", tex);
		PS2::GetGSState().TEX = tex;
		PS2::GetGSState().bTexSet = true;
	}

	void SetTest(GIFReg::GSTest test)
	{
		PS2::GetGSState().TEST = test;
	}

	void SetClamp(GIFReg::GSClamp clamp)
	{
		PS2::GetGSState().CLAMP = clamp;
	}

	void SetColClamp(GIFReg::GSColClamp colClamp)
	{
		PS2::GetGSState().COLCLAMP = colClamp;
	}

	PS2::DrawBufferData<Renderer::GSVertex, uint16_t>& GetDefaultDrawBuffer()
	{
		return PS2_Internal::gVertexBuffers.GetDrawBufferData();
	}

	void KickVertex(uint16_t x, uint16_t y, uint32_t z)
	{
		auto vtx = PS2_Internal::MakeVertex(x, y, z);
		KickVertex<GSVertex, uint16_t>(vtx, PS2::GetGSState().PRIM, Skip, GetDefaultDrawBuffer());
	}

	template<>
	void UpdateXyTail<GSVertexUnprocessed, uint16_t>(const GSVertexUnprocessed& vtx, PS2::DrawBufferData<GSVertexUnprocessed, uint16_t>& drawBuffer, const size_t& xy_tail) {
	}

	template<>
	void UpdateXyTail<GSVertex, uint16_t>(const GSVertex& vtx, PS2::DrawBufferData<GSVertex, uint16_t>& drawBuffer, const size_t& xy_tail) {
		const uint32_t x = vtx.XY[0];
		const uint32_t y = vtx.XY[1];
		const uint32_t z = vtx.Z;

		uint32_t OFX = PS2::GetGSState().XY.X;
		uint32_t OFY = PS2::GetGSState().XY.Y;

		GSVector4i m_ofxy = GSVector4i(
			0x8000,
			0x8000,
			(int)OFX - 15,
			(int)OFY - 15);

		GSVector4i v1;
		v1.x = x | ((uint32_t)y << 0x10);

		GSVector4i xy = v1.xxxx().u16to32().sub32(m_ofxy);
		GSVector4i::storel(&drawBuffer.vertex.xy[xy_tail & 3], xy.upl64(xy.sra32(4).zwzw()).ps32());
	}

	template<>
	void TraceUpdateSkip<GSVertexUnprocessed, uint16_t>(uint32_t& skip, PS2::DrawBufferData<GSVertexUnprocessed, uint16_t>& drawBuffer, const GS_PRIM& prim, const size_t& xy_tail, const size_t& m)
	{

	}

	template<>
	void TraceUpdateSkip<GSVertex, uint16_t>(uint32_t& skip, PS2::DrawBufferData<GSVertex, uint16_t>& drawBuffer, const GS_PRIM& prim, const size_t& xy_tail, const size_t& m)
	{
		uint32_t OFX = PS2::GetGSState().XY.X;
		uint32_t OFY = PS2::GetGSState().XY.Y;

		GSVector4i v0, v1, v2, v3, pmin, pmax;

		v0 = GSVector4i::loadl(&drawBuffer.vertex.xy[(xy_tail + 1) & 3]); // T-3
		v1 = GSVector4i::loadl(&drawBuffer.vertex.xy[(xy_tail + 2) & 3]); // T-2
		v2 = GSVector4i::loadl(&drawBuffer.vertex.xy[(xy_tail + 3) & 3]); // T-1
		v3 = GSVector4i::loadl(&drawBuffer.vertex.xy[(xy_tail - m) & 3]); // H

		GSVector4 cross;

		switch (prim)
		{
		case GS_POINTLIST:
			pmin = v2;
			pmax = v2;
			break;
		case GS_LINELIST:
		case GS_LINESTRIP:
		case GS_SPRITE:
			pmin = v2.min_i16(v1);
			pmax = v2.max_i16(v1);
			break;
		case GS_TRIANGLELIST:
		case GS_TRIANGLESTRIP:
			pmin = v2.min_i16(v1.min_i16(v0));
			pmax = v2.max_i16(v1.max_i16(v0));
			break;
		case GS_TRIANGLEFAN:
			pmin = v2.min_i16(v1.min_i16(v3));
			pmax = v2.max_i16(v1.max_i16(v3));
			break;
		default:
			break;
		}

		bool m_nativeres = true;
		GSVector4i m_scissor;

		m_scissor.u16[0] = (uint16_t)((hwState.scissor.offset.x << 4) + OFX - 0x8000);
		m_scissor.u16[1] = (uint16_t)((hwState.scissor.offset.y << 4) + OFY - 0x8000);
		m_scissor.u16[2] = (uint16_t)((hwState.scissor.extent.width << 4) + OFX - 0x8000);
		m_scissor.u16[3] = (uint16_t)((hwState.scissor.extent.height << 4) + OFY - 0x8000);

		GSVector4i test = pmax.lt16(m_scissor) | pmin.gt16(m_scissor.zwzwl());

		switch (prim)
		{
		case GS_TRIANGLELIST:
		case GS_TRIANGLESTRIP:
		case GS_TRIANGLEFAN:
		case GS_SPRITE:
			// FIXME: GREG I don't understand the purpose of the m_nativeres check
			// It impacts badly the number of draw call in the HW renderer.
			test |= m_nativeres ? pmin.eq16(pmax).zwzwl() : pmin.eq16(pmax);
			break;
		default:
			break;
		}

		switch (prim)
		{
		case GS_TRIANGLELIST:
		case GS_TRIANGLESTRIP:
			// TODO: any way to do a 16-bit integer cross product?
			// cross product is zero most of the time because either of the vertices are the same
			/*
			cross = GSVector4(v2.xyxyl().i16to32().sub32(v0.upl32(v1).i16to32())); // x20, y20, x21, y21
			cross = cross * cross.wzwz(); // x20 * y21, y20 * x21
			test |= GSVector4i::cast(cross == cross.yxwz());
			*/
			test = (test | v0 == v1) | (v1 == v2 | v0 == v2);
			break;
		case GS_TRIANGLEFAN:
			/*
			cross = GSVector4(v2.xyxyl().i16to32().sub32(v3.upl32(v1).i16to32())); // x23, y23, x21, y21
			cross = cross * cross.wzwz(); // x23 * y21, y23 * x21
			test |= GSVector4i::cast(cross == cross.yxwz());
			*/
			test = (test | v3 == v1) | (v1 == v2 | v3 == v2);
			break;
		default:
			break;
		}

		skip |= test.mask() & 15;
	}

	//void KickVertex(const GSVertex& vtx, GIFReg::GSPrim primReg, uint32_t skip, PS2::BufferData<GSVertex, uint16_t>& drawBuffer)
	//{
	//	GS_PRIM prim = (GS_PRIM)primReg.PRIM;
	//	assert(drawBuffer.vertex.tail < drawBuffer.vertex.maxcount + 3);
	//
	//	size_t head = drawBuffer.vertex.head;
	//	size_t tail = drawBuffer.vertex.tail;
	//	size_t next = drawBuffer.vertex.next;
	//	size_t xy_tail = drawBuffer.vertex.xy_tail;
	//
	//	drawBuffer.vertex.buff[drawBuffer.vertex.tail] = vtx;
	//
	//	UpdateXyTail(vtx, xy_tail, drawBuffer);
	//
	//	drawBuffer.vertex.tail = ++tail;
	//	drawBuffer.vertex.xy_tail = ++xy_tail;
	//
	//	size_t n = 0;
	//
	//	switch (prim)
	//	{
	//	case GS_POINTLIST: n = 1; break;
	//	case GS_LINELIST: n = 2; break;
	//	case GS_LINESTRIP: n = 2; break;
	//	case GS_TRIANGLELIST: n = 3; break;
	//	case GS_TRIANGLESTRIP: n = 3; break;
	//	case GS_TRIANGLEFAN: n = 3; break;
	//	case GS_SPRITE: n = 2; break;
	//	case GS_INVALID: n = 1; break;
	//	}
	//
	//	size_t m = tail - head;
	//
	//	if (m < n)
	//	{
	//		return;
	//	}
	//
	//	if (skip == 0 && (prim != GS_TRIANGLEFAN || m <= 4)) // m_vertex.xy only knows about the last 4 vertices, head could be far behind for fan
	//	{
	//		TraceUpdateSkip(skip, drawBuffer, prim, xy_tail, m);
	//	}
	//
	//	if (skip != 0)
	//	{
	//		switch (prim)
	//		{
	//		case GS_POINTLIST:
	//		case GS_LINELIST:
	//		case GS_TRIANGLELIST:
	//		case GS_SPRITE:
	//		case GS_INVALID:
	//			drawBuffer.vertex.tail = head; // no need to check or grow the buffer length
	//			break;
	//		case GS_LINESTRIP:
	//		case GS_TRIANGLESTRIP:
	//			drawBuffer.vertex.head = head + 1;
	//			// fall through
	//		case GS_TRIANGLEFAN:
	//			if (tail >= drawBuffer.vertex.maxcount) assert(false); // in case too many vertices were skipped
	//			break;
	//		default:
	//			__assume(0);
	//		}
	//
	//		return;
	//	}
	//
	//	if (tail >= drawBuffer.vertex.maxcount) assert(false);
	//
	//	uint16_t* RESTRICT buff = &drawBuffer.index.buff[drawBuffer.index.tail];
	//
	//	switch (prim)
	//	{
	//	case GS_POINTLIST:
	//		buff[0] = head + 0;
	//		drawBuffer.vertex.head = head + 1;
	//		drawBuffer.vertex.next = head + 1;
	//		drawBuffer.index.tail += 1;
	//		break;
	//	case GS_LINELIST:
	//		buff[0] = head + 0;
	//		buff[1] = head + 1;
	//		drawBuffer.vertex.head = head + 2;
	//		drawBuffer.vertex.next = head + 2;
	//		drawBuffer.index.tail += 2;
	//		break;
	//	case GS_LINESTRIP:
	//		if (next < head)
	//		{
	//			drawBuffer.vertex.buff[next + 0] = drawBuffer.vertex.buff[head + 0];
	//			drawBuffer.vertex.buff[next + 1] = drawBuffer.vertex.buff[head + 1];
	//			head = next;
	//			drawBuffer.vertex.tail = next + 2;
	//		}
	//		buff[0] = head + 0;
	//		buff[1] = head + 1;
	//		drawBuffer.vertex.head = head + 1;
	//		drawBuffer.vertex.next = head + 2;
	//		drawBuffer.index.tail += 2;
	//		break;
	//	case GS_TRIANGLELIST:
	//		buff[0] = head + 0;
	//		buff[1] = head + 1;
	//		buff[2] = head + 2;
	//		drawBuffer.vertex.head = head + 3;
	//		drawBuffer.vertex.next = head + 3;
	//		drawBuffer.index.tail += 3;
	//		break;
	//	case GS_TRIANGLESTRIP:
	//		if (next < head)
	//		{
	//			drawBuffer.vertex.buff[next + 0] = drawBuffer.vertex.buff[head + 0];
	//			drawBuffer.vertex.buff[next + 1] = drawBuffer.vertex.buff[head + 1];
	//			drawBuffer.vertex.buff[next + 2] = drawBuffer.vertex.buff[head + 2];
	//			head = next;
	//			drawBuffer.vertex.tail = next + 3;
	//		}
	//		buff[0] = head + 0;
	//		buff[1] = head + 1;
	//		buff[2] = head + 2;
	//		drawBuffer.vertex.head = head + 1;
	//		drawBuffer.vertex.next = head + 3;
	//		drawBuffer.index.tail += 3;
	//		break;
	//	case GS_TRIANGLEFAN:
	//		// TODO: remove gaps, next == head && head < tail - 3 || next > head && next < tail - 2 (very rare)
	//		buff[0] = head + 0;
	//		buff[1] = tail - 2;
	//		buff[2] = tail - 1;
	//		drawBuffer.vertex.next = tail;
	//		drawBuffer.index.tail += 3;
	//		break;
	//	case GS_SPRITE:
	//		buff[0] = head + 0;
	//		buff[1] = head + 1;
	//		drawBuffer.vertex.head = head + 2;
	//		drawBuffer.vertex.next = head + 2;
	//		drawBuffer.index.tail += 2;
	//		break;
	//	case GS_INVALID:
	//		drawBuffer.vertex.tail = head;
	//		break;
	//	default:
	//		__assume(0);
	//	}
	//}

	void SetScissor(int x, int y, uint32_t width, uint32_t height) {
		assert(x >= 0 && y >= 0);
		hwState.scissor.offset = { x, y };
		hwState.scissor.extent = { width, height };
	}
}

struct GSSelector
{
	uint32_t iip;
	uint32_t prim;
	uint32_t point;
	uint32_t line;

	GSSelector()
		: iip(0)
		, prim(0)
		, point(0)
		, line(0)
	{}

	void ResetStates() {
		*this = GSSelector();
	}
};

static class GSUtilMaps
{
public:
	uint8_t PrimClassField[8];
	uint8_t VertexCountField[8];
	uint8_t ClassVertexCountField[4];
	uint32_t CompatibleBitsField[64][2];
	uint32_t SharedBitsField[64][2];

	// Defer init to avoid AVX2 illegal instructions
	GSUtilMaps()
	{
		PrimClassField[Renderer::GS_POINTLIST] = Renderer::GS_POINT_CLASS;
		PrimClassField[Renderer::GS_LINELIST] = Renderer::GS_LINE_CLASS;
		PrimClassField[Renderer::GS_LINESTRIP] = Renderer::GS_LINE_CLASS;
		PrimClassField[Renderer::GS_TRIANGLELIST] = Renderer::GS_TRIANGLE_CLASS;
		PrimClassField[Renderer::GS_TRIANGLESTRIP] = Renderer::GS_TRIANGLE_CLASS;
		PrimClassField[Renderer::GS_TRIANGLEFAN] = Renderer::GS_TRIANGLE_CLASS;
		PrimClassField[Renderer::GS_SPRITE] = Renderer::GS_SPRITE_CLASS;
		PrimClassField[Renderer::GS_INVALID] = Renderer::GS_INVALID_CLASS;

		VertexCountField[Renderer::GS_POINTLIST] = 1;
		VertexCountField[Renderer::GS_LINELIST] = 2;
		VertexCountField[Renderer::GS_LINESTRIP] = 2;
		VertexCountField[Renderer::GS_TRIANGLELIST] = 3;
		VertexCountField[Renderer::GS_TRIANGLESTRIP] = 3;
		VertexCountField[Renderer::GS_TRIANGLEFAN] = 3;
		VertexCountField[Renderer::GS_SPRITE] = 2;
		VertexCountField[Renderer::GS_INVALID] = 1;

		ClassVertexCountField[Renderer::GS_POINT_CLASS] = 1;
		ClassVertexCountField[Renderer::GS_LINE_CLASS] = 2;
		ClassVertexCountField[Renderer::GS_TRIANGLE_CLASS] = 3;
		ClassVertexCountField[Renderer::GS_SPRITE_CLASS] = 2;

		//memset(CompatibleBitsField, 0, sizeof(CompatibleBitsField));
		//
		//for (int i = 0; i < 64; i++)
		//{
		//	CompatibleBitsField[i][i >> 5] |= 1 << (i & 0x1f);
		//}
		//
		//CompatibleBitsField[PSM_PSMCT32][PSM_PSMCT24 >> 5] |= 1 << (PSM_PSMCT24 & 0x1f);
		//CompatibleBitsField[PSM_PSMCT24][PSM_PSMCT32 >> 5] |= 1 << (PSM_PSMCT32 & 0x1f);
		//CompatibleBitsField[PSM_PSMCT16][PSM_PSMCT16S >> 5] |= 1 << (PSM_PSMCT16S & 0x1f);
		//CompatibleBitsField[PSM_PSMCT16S][PSM_PSMCT16 >> 5] |= 1 << (PSM_PSMCT16 & 0x1f);
		//CompatibleBitsField[PSM_PSMZ32][PSM_PSMZ24 >> 5] |= 1 << (PSM_PSMZ24 & 0x1f);
		//CompatibleBitsField[PSM_PSMZ24][PSM_PSMZ32 >> 5] |= 1 << (PSM_PSMZ32 & 0x1f);
		//CompatibleBitsField[PSM_PSMZ16][PSM_PSMZ16S >> 5] |= 1 << (PSM_PSMZ16S & 0x1f);
		//CompatibleBitsField[PSM_PSMZ16S][PSM_PSMZ16 >> 5] |= 1 << (PSM_PSMZ16 & 0x1f);
		//
		//memset(SharedBitsField, 0, sizeof(SharedBitsField));
		//
		//SharedBitsField[PSM_PSMCT24][PSM_PSMT8H >> 5] |= 1 << (PSM_PSMT8H & 0x1f);
		//SharedBitsField[PSM_PSMCT24][PSM_PSMT4HL >> 5] |= 1 << (PSM_PSMT4HL & 0x1f);
		//SharedBitsField[PSM_PSMCT24][PSM_PSMT4HH >> 5] |= 1 << (PSM_PSMT4HH & 0x1f);
		//SharedBitsField[PSM_PSMZ24][PSM_PSMT8H >> 5] |= 1 << (PSM_PSMT8H & 0x1f);
		//SharedBitsField[PSM_PSMZ24][PSM_PSMT4HL >> 5] |= 1 << (PSM_PSMT4HL & 0x1f);
		//SharedBitsField[PSM_PSMZ24][PSM_PSMT4HH >> 5] |= 1 << (PSM_PSMT4HH & 0x1f);
		//SharedBitsField[PSM_PSMT8H][PSM_PSMCT24 >> 5] |= 1 << (PSM_PSMCT24 & 0x1f);
		//SharedBitsField[PSM_PSMT8H][PSM_PSMZ24 >> 5] |= 1 << (PSM_PSMZ24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HL][PSM_PSMCT24 >> 5] |= 1 << (PSM_PSMCT24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HL][PSM_PSMZ24 >> 5] |= 1 << (PSM_PSMZ24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HL][PSM_PSMT4HH >> 5] |= 1 << (PSM_PSMT4HH & 0x1f);
		//SharedBitsField[PSM_PSMT4HH][PSM_PSMCT24 >> 5] |= 1 << (PSM_PSMCT24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HH][PSM_PSMZ24 >> 5] |= 1 << (PSM_PSMZ24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HH][PSM_PSMT4HL >> 5] |= 1 << (PSM_PSMT4HL & 0x1f);
	}

} s_maps;

Renderer::GS_PRIM_CLASS GetPrimClass(uint32_t prim) {
	return (Renderer::GS_PRIM_CLASS)s_maps.PrimClassField[prim];
}

GSSelector g_GSSelector;

namespace PS2
{
	GSHWDrawConfig m_conf = {};

	void EmulateAtst(const int pass, GSTexImage& tex, const GSState& state)
	{
		PSConstantBuffer& ps_cb = tex.constantBuffer.GetPixelConstantBufferData();

		static const uint32_t inverted_atst[] = { ATST_ALWAYS, ATST_NEVER, ATST_GEQUAL, ATST_GREATER, ATST_NOTEQUAL, ATST_LESS, ATST_LEQUAL, ATST_EQUAL };
		int atst = (pass == 2) ? inverted_atst[state.TEST.ATST] : state.TEST.ATST;

		if (!state.TEST.ATE) return;

		switch (atst)
		{
		case ATST_LESS:
			ps_cb.FogColor_AREF.a = (float)state.TEST.AREF - 0.1f;
			m_conf.ps.atst = 1;
			break;
		case ATST_LEQUAL:
			ps_cb.FogColor_AREF.a = (float)state.TEST.AREF - 0.1f + 1.0f;
			m_conf.ps.atst = 1;
			break;
		case ATST_GEQUAL:
			// Maybe a -1 trick multiplication factor could be used to merge with ATST_LEQUAL case
			ps_cb.FogColor_AREF.a = (float)state.TEST.AREF - 0.1f;
			m_conf.ps.atst = 2;
			break;
		case ATST_GREATER:
			// Maybe a -1 trick multiplication factor could be used to merge with ATST_LESS case
			ps_cb.FogColor_AREF.a = (float)state.TEST.AREF - 0.1f + 1.0f;
			m_conf.ps.atst = 2;
			break;
		case ATST_EQUAL:
			ps_cb.FogColor_AREF.a = (float)state.TEST.AREF;
			m_conf.ps.atst = 3;
			break;
		case ATST_NOTEQUAL:
			ps_cb.FogColor_AREF.a = (float)state.TEST.AREF;
			m_conf.ps.atst = 4;
			break;

		case ATST_NEVER: // Draw won't be done so no need to implement it in shader
		case ATST_ALWAYS:
		default:
			m_conf.ps.atst = 0;
			break;
		}
	}

	enum GS_TFX
	{
		TFX_MODULATE = 0,
		TFX_DECAL = 1,
		TFX_HIGHLIGHT = 2,
		TFX_HIGHLIGHT2 = 3,
		TFX_NONE = 4,
	};

	void EmulateTextureSampler(PS2::GSTexImage& tex, const GSState& state)
	{
		auto& vs_cb = tex.constantBuffer.GetVertexConstantBufferData();
		auto& ps_cb = tex.constantBuffer.GetPixelConstantBufferData();

		// Warning fetch the texture PSM format rather than the context format. The latter could have been corrected in the texture cache for depth.
		//const GSLocalMemory::psm_t &psm = GSLocalMemory::m_psm[state.TEX.PSM];
		//const GSLocalMemory::psm_t& psm = GSLocalMemory::m_psm[tex->m_TEX0.PSM];
		//const GSLocalMemory::psm_t& cpsm = psm.pal > 0 ? GSLocalMemory::m_psm[state.TEX.CPSM] : psm;

		const uint8_t wms = state.CLAMP.WMS;
		const uint8_t wmt = state.CLAMP.WMT;
		bool complex_wms_wmt = !!((wms | wmt) & 2);

		bool bilinear = false; // m_vt.IsLinear();
		bool shader_emulated_sampler = false; // tex->m_palette || cpsm.fmt != 0 || complex_wms_wmt || psm.depth;

		// 1 and 0 are equivalent
		m_conf.ps.wms = (wms & 2) ? wms : 0;
		m_conf.ps.wmt = (wmt & 2) ? wmt : 0;

		//int w = 0; //tex->m_texture->GetWidth();
		//int h = 0; //tex->m_texture->GetHeight();

		int tw = (int)(1 << state.TEX.TW);
		int th = (int)(1 << state.TEX.TH);

		GSVector4 WH(tw, th, tex.width, tex.height);

		// Depth + bilinear filtering isn't done yet (And I'm not sure we need it anyway but a game will prove me wrong)
		// So of course, GTA set the linear mode, but sampling is done at texel center so it is equivalent to nearest sampling
		//ASSERT(!(psm.depth && m_vt.IsLinear()));

		// Performance note:
		// 1/ Don't set 0 as it is the default value
		// 2/ Only keep aem when it is useful (avoid useless shader permutation)
		if (m_conf.ps.shuffle)
		{
			// Force a 32 bits access (normally shuffle is done on 16 bits)
			// m_conf.ps.fmt = 0; // removed as an optimization
			//m_conf.ps.aem = m_env.TEXA.AEM;
			//ASSERT(tex->m_target);
			//
			//// Require a float conversion if the texure is a depth otherwise uses Integral scaling
			//if (psm.depth)
			//{
			//	m_conf.ps.depth_fmt = (tex->m_texture->GetType() != GSTexture::DepthStencil) ? 3 : 1;
			//}
			//
			//// Shuffle is a 16 bits format, so aem is always required
			//GSVector4 ta(m_env.TEXA & GSVector4i::x000000ff());
			//ps_cb.MinF_TA = (GSVector4(ps_cb.MskFix) + 0.5f).xyxy(ta) / WH.xyxy(GSVector4(255, 255));
			//
			//bilinear &= m_vt.IsLinear();
			//
			//GSVector4 half_offset = RealignTargetTextureCoordinate(tex);
			//vs_cb.Texture_Scale_Offset.z = half_offset.x;
			//vs_cb.Texture_Scale_Offset.w = half_offset.y;

		}
		//else if (tex->m_target)
		//{
		//	// Use an old target. AEM and index aren't resolved it must be done
		//	// on the GPU
		//
		//	// Select the 32/24/16 bits color (AEM)
		//	m_conf.ps.fmt = cpsm.fmt;
		//	m_conf.ps.aem = m_env.TEXA.AEM;
		//
		//	// Don't upload AEM if format is 32 bits
		//	if (cpsm.fmt)
		//	{
		//		GSVector4 ta(m_env.TEXA & GSVector4i::x000000ff());
		//		ps_cb.MinF_TA = (GSVector4(ps_cb.MskFix) + 0.5f).xyxy(ta) / WH.xyxy(GSVector4(255, 255));
		//	}
		//
		//	// Select the index format
		//	if (tex->m_palette)
		//	{
		//		// FIXME Potentially improve fmt field in GSLocalMemory
		//		if (state.TEX.PSM == PSM_PSMT4HL)
		//			m_conf.ps.fmt |= 1 << 2;
		//		else if (state.TEX.PSM == PSM_PSMT4HH)
		//			m_conf.ps.fmt |= 2 << 2;
		//		else
		//			m_conf.ps.fmt |= 3 << 2;
		//
		//		// Alpha channel of the RT is reinterpreted as an index. Star
		//		// Ocean 3 uses it to emulate a stencil buffer.  It is a very
		//		// bad idea to force bilinear filtering on it.
		//		bilinear &= m_vt.IsLinear();
		//	}
		//
		//	// Depth format
		//	if (tex->m_texture->GetType() == GSTexture::DepthStencil)
		//	{
		//		// Require a float conversion if the texure is a depth format
		//		m_conf.ps.depth_fmt = (psm.bpp == 16) ? 2 : 1;
		//
		//		// Don't force interpolation on depth format
		//		bilinear &= m_vt.IsLinear();
		//	}
		//	else if (psm.depth)
		//	{
		//		// Use Integral scaling
		//		m_conf.ps.depth_fmt = 3;
		//
		//		// Don't force interpolation on depth format
		//		bilinear &= m_vt.IsLinear();
		//	}
		//
		//	GSVector4 half_offset = RealignTargetTextureCoordinate(tex);
		//	vs_cb.Texture_Scale_Offset.z = half_offset.x;
		//	vs_cb.Texture_Scale_Offset.w = half_offset.y;
		//}
		//else if (tex->m_palette)
		//{
		//	// Use a standard 8 bits texture. AEM is already done on the CLUT
		//	// Therefore you only need to set the index
		//	// m_conf.ps.aem     = 0; // removed as an optimization
		//
		//	// Note 4 bits indexes are converted to 8 bits
		//	m_conf.ps.fmt = 3 << 2;
		//
		//}
		else
		{
			// Standard texture. Both index and AEM expansion were already done by the CPU.
			// m_conf.ps.fmt = 0; // removed as an optimization
			// m_conf.ps.aem = 0; // removed as an optimization
		}

		//if (state.TEX.TFX == TFX_MODULATE) // && m_vt.m_eq.rgba == 0xFFFF && m_vt.m_min.c.eq(GSVector4i(128)))
		//{
		//	// Micro optimization that reduces GPU load (removes 5 instructions on the FS program)
		//	m_conf.ps.tfx = TFX_DECAL;
		//}
		//else
		{
			m_conf.ps.tfx = state.TEX.TFX;
		}

		m_conf.ps.tcc = state.TEX.TCC;

		m_conf.ps.ltf = bilinear && shader_emulated_sampler;

		m_conf.ps.point_sampler = !bilinear || shader_emulated_sampler;

		GSVector4 TextureScale = GSVector4(0.0625f) / WH.xyxy();
		vs_cb.Texture_Scale_Offset.x = TextureScale.x;
		vs_cb.Texture_Scale_Offset.y = TextureScale.y;

		if (state.PRIM.FST)
		{
			//Maybe better?
			//vs_cb.TextureScale = GSVector4(1.0f / 16) * GSVector4(tex->m_texture->GetScale()).xyxy() / WH.zwzw();
			m_conf.ps.fst = 1;
		}

		ps_cb.WH = WH;
		ps_cb.HalfTexel = GSVector4(-0.5f, 0.5f).xxyy() / WH.zwzw();
		//if (complex_wms_wmt)
		//{
		//	ps_cb.MskFix = GSVector4i(state.CLAMP.MINU, state.CLAMP.MINV, state.CLAMP.MAXU, state.CLAMP.MAXV);
		//	ps_cb.MinMax = GSVector4(ps_cb.MskFix) / WH.xyxy();
		//}

		// TC Offset Hack
		//m_conf.ps.tcoffsethack = m_userhacks_tcoffset;
		//ps_cb.TC_OffsetHack = GSVector4(m_userhacks_tcoffset_x, m_userhacks_tcoffset_y).xyxy() / WH.xyxy();

		// Must be done after all coordinates math
		//if (m_context->HasFixedTEX0() && !PRIM->FST)
		//{
		//	m_conf.ps.invalid_tex0 = 1;
		//	// Use invalid size to denormalize ST coordinate
		//	ps_cb.WH.x = (float)(1 << m_context->stack.TEX0.TW);
		//	ps_cb.WH.y = (float)(1 << m_context->stack.TEX0.TH);
		//
		//	// We can't handle m_target with invalid_tex0 atm due to upscaling
		//	ASSERT(!tex->m_target);
		//}

		PSSamplerSelector ps_ssel{};

		// Only enable clamping in CLAMP mode. REGION_CLAMP will be done manually in the shader
		ps_ssel.tau = (wms != CLAMP_CLAMP);
		ps_ssel.tav = (wmt != CLAMP_CLAMP);
		ps_ssel.ltf = bilinear && !shader_emulated_sampler;

		tex.UpdateSampler(ps_ssel);
	}

	enum HWBlendFlags
	{
		// A couple of flag to determine the blending behavior
		BLEND_A_MAX = 0x100, // Impossible blending uses coeff bigger than 1
		BLEND_C_CLR = 0x200, // Clear color blending (use directly the destination color as blending factor)
		BLEND_NO_BAR = 0x400, // Doesn't require sampling of the RT as a texture
		BLEND_ACCU = 0x800, // Allow to use a mix of SW and HW blending to keep the best of the 2 worlds
	};

	enum BlendFactor : u8
	{
		// HW blend factors
		SRC_COLOR, INV_SRC_COLOR, DST_COLOR, INV_DST_COLOR,
		SRC1_COLOR, INV_SRC1_COLOR, SRC_ALPHA, INV_SRC_ALPHA,
		DST_ALPHA, INV_DST_ALPHA, SRC1_ALPHA, INV_SRC1_ALPHA,
		CONST_COLOR, INV_CONST_COLOR, CONST_ONE, CONST_ZERO,
	};

	enum BlendOp : u8
	{
		// HW blend operations
		OP_ADD, OP_SUBTRACT, OP_REV_SUBTRACT
	};

	// Determines the HW blend function for DX11/OGL
	struct HWBlend { uint16_t flags, op, src, dst; };

	const std::array<HWBlend, 3 * 3 * 3 * 3> m_blendMap =
	{ {
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0000: (Cs - Cs)*As + Cs ==> Cs
		{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 0001: (Cs - Cs)*As + Cd ==> Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 0002: (Cs - Cs)*As +  0 ==> 0
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0010: (Cs - Cs)*Ad + Cs ==> Cs
		{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 0011: (Cs - Cs)*Ad + Cd ==> Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 0012: (Cs - Cs)*Ad +  0 ==> 0
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0020: (Cs - Cs)*F  + Cs ==> Cs
		{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 0021: (Cs - Cs)*F  + Cd ==> Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 0022: (Cs - Cs)*F  +  0 ==> 0
		{ BLEND_A_MAX | BLEND_MIX2 , OP_SUBTRACT     , CONST_ONE       , SRC1_ALPHA}      , // 0100: (Cs - Cd)*As + Cs ==> Cs*(As + 1) - Cd*As
		{ BLEND_MIX1               , OP_ADD          , SRC1_ALPHA      , INV_SRC1_ALPHA}  , // 0101: (Cs - Cd)*As + Cd ==> Cs*As + Cd*(1 - As)
		{ BLEND_MIX1               , OP_SUBTRACT     , SRC1_ALPHA      , SRC1_ALPHA}      , // 0102: (Cs - Cd)*As +  0 ==> Cs*As - Cd*As
		{ BLEND_A_MAX              , OP_SUBTRACT     , CONST_ONE       , DST_ALPHA}       , // 0110: (Cs - Cd)*Ad + Cs ==> Cs*(Ad + 1) - Cd*Ad
		{ 0                        , OP_ADD          , DST_ALPHA       , INV_DST_ALPHA}   , // 0111: (Cs - Cd)*Ad + Cd ==> Cs*Ad + Cd*(1 - Ad)
		{ 0                        , OP_SUBTRACT     , DST_ALPHA       , DST_ALPHA}       , // 0112: (Cs - Cd)*Ad +  0 ==> Cs*Ad - Cd*Ad
		{ BLEND_A_MAX | BLEND_MIX2 , OP_SUBTRACT     , CONST_ONE       , CONST_COLOR}     , // 0120: (Cs - Cd)*F  + Cs ==> Cs*(F + 1) - Cd*F
		{ BLEND_MIX1               , OP_ADD          , CONST_COLOR     , INV_CONST_COLOR} , // 0121: (Cs - Cd)*F  + Cd ==> Cs*F + Cd*(1 - F)
		{ BLEND_MIX1               , OP_SUBTRACT     , CONST_COLOR     , CONST_COLOR}     , // 0122: (Cs - Cd)*F  +  0 ==> Cs*F - Cd*F
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0200: (Cs -  0)*As + Cs ==> Cs*(As + 1)
		{ BLEND_ACCU               , OP_ADD          , SRC1_ALPHA      , CONST_ONE}       , // 0201: (Cs -  0)*As + Cd ==> Cs*As + Cd
		{ BLEND_NO_REC             , OP_ADD          , SRC1_ALPHA      , CONST_ZERO}      , // 0202: (Cs -  0)*As +  0 ==> Cs*As
		{ BLEND_A_MAX              , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0210: (Cs -  0)*Ad + Cs ==> Cs*(Ad + 1)
		{ BLEND_HW_CLR3            , OP_ADD          , DST_ALPHA       , CONST_ONE}       , // 0211: (Cs -  0)*Ad + Cd ==> Cs*Ad + Cd
		{ BLEND_HW_CLR3            , OP_ADD          , DST_ALPHA       , CONST_ZERO}      , // 0212: (Cs -  0)*Ad +  0 ==> Cs*Ad
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0220: (Cs -  0)*F  + Cs ==> Cs*(F + 1)
		{ BLEND_ACCU               , OP_ADD          , CONST_COLOR     , CONST_ONE}       , // 0221: (Cs -  0)*F  + Cd ==> Cs*F + Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_COLOR     , CONST_ZERO}      , // 0222: (Cs -  0)*F  +  0 ==> Cs*F
		{ BLEND_MIX3               , OP_ADD          , INV_SRC1_ALPHA  , SRC1_ALPHA}      , // 1000: (Cd - Cs)*As + Cs ==> Cd*As + Cs*(1 - As)
		{ BLEND_A_MAX | BLEND_MIX1 , OP_REV_SUBTRACT , SRC1_ALPHA      , CONST_ONE}       , // 1001: (Cd - Cs)*As + Cd ==> Cd*(As + 1) - Cs*As
		{ BLEND_MIX1               , OP_REV_SUBTRACT , SRC1_ALPHA      , SRC1_ALPHA}      , // 1002: (Cd - Cs)*As +  0 ==> Cd*As - Cs*As
		{ 0                        , OP_ADD          , INV_DST_ALPHA   , DST_ALPHA}       , // 1010: (Cd - Cs)*Ad + Cs ==> Cd*Ad + Cs*(1 - Ad)
		{ BLEND_A_MAX              , OP_REV_SUBTRACT , DST_ALPHA       , CONST_ONE}       , // 1011: (Cd - Cs)*Ad + Cd ==> Cd*(Ad + 1) - Cs*Ad
		{ 0                        , OP_REV_SUBTRACT , DST_ALPHA       , DST_ALPHA}       , // 1012: (Cd - Cs)*Ad +  0 ==> Cd*Ad - Cs*Ad
		{ BLEND_MIX3               , OP_ADD          , INV_CONST_COLOR , CONST_COLOR}     , // 1020: (Cd - Cs)*F  + Cs ==> Cd*F + Cs*(1 - F)
		{ BLEND_A_MAX | BLEND_MIX1 , OP_REV_SUBTRACT , CONST_COLOR     , CONST_ONE}       , // 1021: (Cd - Cs)*F  + Cd ==> Cd*(F + 1) - Cs*F
		{ BLEND_MIX1               , OP_REV_SUBTRACT , CONST_COLOR     , CONST_COLOR}     , // 1022: (Cd - Cs)*F  +  0 ==> Cd*F - Cs*F
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 1100: (Cd - Cd)*As + Cs ==> Cs
		{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 1101: (Cd - Cd)*As + Cd ==> Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 1102: (Cd - Cd)*As +  0 ==> 0
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 1110: (Cd - Cd)*Ad + Cs ==> Cs
		{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 1111: (Cd - Cd)*Ad + Cd ==> Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 1112: (Cd - Cd)*Ad +  0 ==> 0
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 1120: (Cd - Cd)*F  + Cs ==> Cs
		{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 1121: (Cd - Cd)*F  + Cd ==> Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 1122: (Cd - Cd)*F  +  0 ==> 0
		{ 0                        , OP_ADD          , CONST_ONE       , SRC1_ALPHA}      , // 1200: (Cd -  0)*As + Cs ==> Cs + Cd*As
		{ BLEND_HW_CLR1            , OP_ADD          , DST_COLOR       , SRC1_ALPHA}      , // 1201: (Cd -  0)*As + Cd ==> Cd*(1 + As)
		{ BLEND_HW_CLR2            , OP_ADD          , DST_COLOR       , SRC1_ALPHA}      , // 1202: (Cd -  0)*As +  0 ==> Cd*As
		{ 0                        , OP_ADD          , CONST_ONE       , DST_ALPHA}       , // 1210: (Cd -  0)*Ad + Cs ==> Cs + Cd*Ad
		{ BLEND_HW_CLR1            , OP_ADD          , DST_COLOR       , DST_ALPHA}       , // 1211: (Cd -  0)*Ad + Cd ==> Cd*(1 + Ad)
		{ 0                        , OP_ADD          , CONST_ZERO      , DST_ALPHA}       , // 1212: (Cd -  0)*Ad +  0 ==> Cd*Ad
		{ 0                        , OP_ADD          , CONST_ONE       , CONST_COLOR}     , // 1220: (Cd -  0)*F  + Cs ==> Cs + Cd*F
		{ BLEND_HW_CLR1            , OP_ADD          , DST_COLOR       , CONST_COLOR}     , // 1221: (Cd -  0)*F  + Cd ==> Cd*(1 + F)
		{ BLEND_HW_CLR2            , OP_ADD          , DST_COLOR       , CONST_COLOR}     , // 1222: (Cd -  0)*F  +  0 ==> Cd*F
		{ BLEND_NO_REC             , OP_ADD          , INV_SRC1_ALPHA  , CONST_ZERO}      , // 2000: (0  - Cs)*As + Cs ==> Cs*(1 - As)
		{ BLEND_ACCU               , OP_REV_SUBTRACT , SRC1_ALPHA      , CONST_ONE}       , // 2001: (0  - Cs)*As + Cd ==> Cd - Cs*As
		{ BLEND_NO_REC             , OP_REV_SUBTRACT , SRC1_ALPHA      , CONST_ZERO}      , // 2002: (0  - Cs)*As +  0 ==> 0 - Cs*As
		{ 0                        , OP_ADD          , INV_DST_ALPHA   , CONST_ZERO}      , // 2010: (0  - Cs)*Ad + Cs ==> Cs*(1 - Ad)
		{ BLEND_HW_CLR3            , OP_REV_SUBTRACT , DST_ALPHA       , CONST_ONE}       , // 2011: (0  - Cs)*Ad + Cd ==> Cd - Cs*Ad
		{ 0                        , OP_REV_SUBTRACT , DST_ALPHA       , CONST_ZERO}      , // 2012: (0  - Cs)*Ad +  0 ==> 0 - Cs*Ad
		{ BLEND_NO_REC             , OP_ADD          , INV_CONST_COLOR , CONST_ZERO}      , // 2020: (0  - Cs)*F  + Cs ==> Cs*(1 - F)
		{ BLEND_ACCU               , OP_REV_SUBTRACT , CONST_COLOR     , CONST_ONE}       , // 2021: (0  - Cs)*F  + Cd ==> Cd - Cs*F
		{ BLEND_NO_REC             , OP_REV_SUBTRACT , CONST_COLOR     , CONST_ZERO}      , // 2022: (0  - Cs)*F  +  0 ==> 0 - Cs*F
		{ 0                        , OP_SUBTRACT     , CONST_ONE       , SRC1_ALPHA}      , // 2100: (0  - Cd)*As + Cs ==> Cs - Cd*As
		{ 0                        , OP_ADD          , CONST_ZERO      , INV_SRC1_ALPHA}  , // 2101: (0  - Cd)*As + Cd ==> Cd*(1 - As)
		{ 0                        , OP_SUBTRACT     , CONST_ZERO      , SRC1_ALPHA}      , // 2102: (0  - Cd)*As +  0 ==> 0 - Cd*As
		{ 0                        , OP_SUBTRACT     , CONST_ONE       , DST_ALPHA}       , // 2110: (0  - Cd)*Ad + Cs ==> Cs - Cd*Ad
		{ 0                        , OP_ADD          , CONST_ZERO      , INV_DST_ALPHA}   , // 2111: (0  - Cd)*Ad + Cd ==> Cd*(1 - Ad)
		{ 0                        , OP_SUBTRACT     , CONST_ZERO      , DST_ALPHA}       , // 2112: (0  - Cd)*Ad +  0 ==> 0 - Cd*Ad
		{ 0                        , OP_SUBTRACT     , CONST_ONE       , CONST_COLOR}     , // 2120: (0  - Cd)*F  + Cs ==> Cs - Cd*F
		{ 0                        , OP_ADD          , CONST_ZERO      , INV_CONST_COLOR} , // 2121: (0  - Cd)*F  + Cd ==> Cd*(1 - F)
		{ 0                        , OP_SUBTRACT     , CONST_ZERO      , CONST_COLOR}     , // 2122: (0  - Cd)*F  +  0 ==> 0 - Cd*F
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 2200: (0  -  0)*As + Cs ==> Cs
		{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 2201: (0  -  0)*As + Cd ==> Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 2202: (0  -  0)*As +  0 ==> 0
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 2210: (0  -  0)*Ad + Cs ==> Cs
		{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 2211: (0  -  0)*Ad + Cd ==> Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 2212: (0  -  0)*Ad +  0 ==> 0
		{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 2220: (0  -  0)*F  + Cs ==> Cs
		{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 2221: (0  -  0)*F  + Cd ==> Cd
		{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 2222: (0  -  0)*F  +  0 ==> 0
	} };

	const std::array<u8, 16> m_replaceDualSrcBlendMap =
	{ {
		SRC_COLOR,        // SRC_COLOR
		INV_SRC_COLOR,    // INV_SRC_COLOR
		DST_COLOR,        // DST_COLOR
		INV_DST_COLOR,    // INV_DST_COLOR
		SRC_COLOR,        // SRC1_COLOR
		INV_SRC_COLOR,    // INV_SRC1_COLOR
		SRC_ALPHA,        // SRC_ALPHA
		INV_SRC_ALPHA,    // INV_SRC_ALPHA
		DST_ALPHA,        // DST_ALPHA
		INV_DST_ALPHA,    // INV_DST_ALPHA
		SRC_ALPHA,        // SRC1_ALPHA
		INV_SRC_ALPHA,    // INV_SRC1_ALPHA
		CONST_COLOR,      // CONST_COLOR
		INV_CONST_COLOR,  // INV_CONST_COLOR
		CONST_ONE,        // CONST_ONE
		CONST_ZERO        // CONST_ZERO
	} };

	static HWBlend GetBlend(u32 index, bool replace_dual_src)
	{
		HWBlend ret = m_blendMap[index];
		if (replace_dual_src)
		{
			ret.src = m_replaceDualSrcBlendMap[ret.src];
			ret.dst = m_replaceDualSrcBlendMap[ret.dst];
		}
		return ret;
	}

	uint16_t GetBlendFlags(size_t index) { return m_blendMap[index].flags; }

	bool m_sw_blending = false;

	bool IsCoverageAlpha(Renderer::GS_PRIM_CLASS primclass, const GSState& state)
	{
		auto PRIM = state.PRIM;
		return !PRIM.ABE && PRIM.AA1 && (primclass == Renderer::GS_LINE_CLASS || primclass == Renderer::GS_TRIANGLE_CLASS);
	}

	enum class AccBlendLevel : u8
	{
		Minimum,
		Basic,
		Medium,
		High,
		Full,
		Maximum,
	};

	AccBlendLevel AccurateBlendingUnit = AccBlendLevel::Basic;

	__fi static constexpr bool IsDualSourceBlendFactor(u8 factor)
	{
		return (factor == SRC1_ALPHA || factor == INV_SRC1_ALPHA || factor == SRC1_COLOR
			/* || factor == INV_SRC1_COLOR*/); // not used
	}

	__fi static bool IsDualSourceBlend(u32 index)
	{
		return (IsDualSourceBlendFactor(m_blendMap[index].src) ||
			IsDualSourceBlendFactor(m_blendMap[index].dst));
	}

	enum PRIM_OVERLAP
	{
		PRIM_OVERLAP_UNKNOW,
		PRIM_OVERLAP_YES,
		PRIM_OVERLAP_NO
	};

	PRIM_OVERLAP m_prim_overlap = PRIM_OVERLAP_UNKNOW;

	// clang-format off
	struct FeatureSupport
	{
		bool broken_point_sampler : 1; ///< Issue with AMD cards, see tfx shader for details
		bool vs_expand : 1; ///< Supports expanding points/lines/sprites in the vertex shader
		bool primitive_id : 1; ///< Supports primitive ID for use with prim tracking destination alpha algorithm
		bool texture_barrier : 1; ///< Supports sampling rt and hopefully texture barrier
		bool provoking_vertex_last : 1; ///< Supports using the last vertex in a primitive as the value for flat shading.
		bool point_expand : 1; ///< Supports point expansion in hardware.
		bool line_expand : 1; ///< Supports line expansion in hardware.
		bool prefer_new_textures : 1; ///< Allocate textures up to the pool size before reusing them, to avoid render pass restarts.
		bool dxt_textures : 1; ///< Supports DXTn texture compression, i.e. S3TC and BC1-3.
		bool bptc_textures : 1; ///< Supports BC6/7 texture compression.
		bool framebuffer_fetch : 1; ///< Can sample from the framebuffer without texture barriers.
		bool dual_source_blend : 1; ///< Can use alpha output as a blend factor.
		bool clip_control : 1; ///< Can use 0..1 depth range instead of -1..1.
		bool stencil_buffer : 1; ///< Supports stencil buffer, and can use for DATE.
		bool cas_sharpening : 1; ///< Supports sufficient functionality for contrast adaptive sharpening.
		bool test_and_sample_depth : 1; ///< Supports concurrently binding the depth-stencil buffer for sampling and depth testing.
		FeatureSupport()
		{
			memset(this, 0, sizeof(*this));
		}
	};

#define GL_DBG(...)
#define GL_INS(...)
#define GL_PERF(...)

	void EmulateBlending_Simple(Renderer::GS_PRIM_CLASS primclass, GSTexImage& tex, const GSState& state)
	{
		PSConstantBuffer& ps_cb = tex.constantBuffer.GetPixelConstantBufferData();

		// Partial port of OGL SW blending. Currently only works for accumulation blend.
		auto ALPHA = state.ALPHA;
		auto PRIM = state.PRIM;
		auto PABE = state.PABE;
		const GIFReg::GSColClamp& COLCLAMP = state.COLCLAMP;
		bool sw_blending = false;

		// No blending so early exit
		if (!(PRIM.ABE || (PRIM.AA1 && primclass == Renderer::GS_LINE_CLASS)))
		{
			m_conf.blend = {};
			return;
		}



		//m_om_bsel.abe = 1;
		//m_om_bsel.a = ALPHA.A;
		//m_om_bsel.b = ALPHA.B;
		//m_om_bsel.c = ALPHA.C;
		//m_om_bsel.d = ALPHA.D;

		//uint8 blend_index = uint8(((ALPHA.A * 3 + ALPHA.B) * 3 + ALPHA.C) * 3 + ALPHA.D);
		//int blend_flag = m_dev->GetBlendFlags(blend_index);

		uint8_t blend_index = static_cast<uint8_t>(((ALPHA.A * 3 + ALPHA.B) * 3 + ALPHA.C) * 3 + ALPHA.D);
		const HWBlend blend_preliminary = GetBlend(blend_index, true);
		const int blend_flag = blend_preliminary.flags;

		m_conf.blend = { true, (u8)blend_preliminary.src, (u8)blend_preliminary.dst, (u8)blend_preliminary.op, m_conf.ps.blend_c == 2, ALPHA.FIX };

		if (PABE.PABE)
		{
			if (ALPHA.A == 0 && ALPHA.B == 1 && ALPHA.C == 0 && ALPHA.D == 1)
			{
				// this works because with PABE alpha blending is on when alpha >= 0x80, but since the pixel shader
				// cannot output anything over 0x80 (== 1.0) blending with 0x80 or turning it off gives the same result

				m_conf.blend.enable = 0;
			}
			else
			{
				//Breath of Fire Dragon Quarter triggers this in battles. Graphics are fine though.
				//ASSERT(0);
			}
		}

		// SW free blend.
		bool free_blend = !!(blend_flag & (BLEND_NO_BAR | BLEND_ACCU));

		// Do the multiplication in shader for blending accumulation: Cs*As + Cd or Cs*Af + Cd
		bool accumulation_blend = !!(blend_flag & BLEND_ACCU);

		switch (AccurateBlendingUnit)
		{
		case AccBlendLevel::High:
		case AccBlendLevel::Medium:
		case AccBlendLevel::Basic:
			sw_blending |= free_blend;
			// fall through
		default: break;
		}

		if (COLCLAMP.CLAMP == 0)
		{
			if (accumulation_blend)
			{
				// fprintf(stderr, "%d: COLCLIP HDR mode with accumulation blend\n", s_n);
				sw_blending = true;
				m_conf.ps.hdr = 1;
			}
			else if (sw_blending)
			{
				// So far only BLEND_NO_BAR should hit this path, it's faster than standard HDR algo.
				// Note: Isolate the code to BLEND_NO_BAR if other blending conditions are added.
				// fprintf(stderr, "%d: COLCLIP SW ENABLED (blending is %d/%d/%d/%d)\n", s_n, ALPHA.A, ALPHA.B, ALPHA.C, ALPHA.D);
				m_conf.ps.colclip = 1;
			}
			else
			{
				// fprintf(stderr, "%d: COLCLIP HDR mode\n", s_n);
				m_conf.ps.hdr = 1;
			}
		}

		/*fprintf(stderr, "%d: BLEND_INFO: %d/%d/%d/%d. Clamp:%d. Prim:%d number %d (sw %d)\n",
			s_n, ALPHA.A, ALPHA.B, ALPHA.C, ALPHA.D, m_env.COLCLAMP.CLAMP, m_vt.m_primclass, m_vertex.next, sw_blending);*/

		if (sw_blending)
		{
			m_conf.ps.blend_a = ALPHA.A;
			m_conf.ps.blend_b = ALPHA.B;
			m_conf.ps.blend_c = ALPHA.C;
			m_conf.ps.blend_d = ALPHA.D;

			if (accumulation_blend)
			{
				m_conf.blend = { true, CONST_ONE, CONST_ONE, (u8)blend_preliminary.op, false, 0 };

				if (ALPHA.A == 2) {
					// The blend unit does a reverse subtraction so it means
					// the shader must output a positive value.
					// Replace 0 - Cs by Cs - 0
					m_conf.ps.blend_a = ALPHA.B;
					m_conf.ps.blend_b = 2;
				}
				// Remove the addition/substraction from the SW blending
				m_conf.ps.blend_d = 2;
			}
			else
			{
				// Disable HW blending
				// Only BLEND_NO_BAR should hit this code path for now.
				m_conf.blend.enable = 0;
			}

			// Require the fix alpha vlaue
			if (ALPHA.C == 2)
				ps_cb.Af.x = (float)ALPHA.FIX / 128.0f;
		}
		else
		{
			m_conf.ps.clr1 = !!(blend_flag & BLEND_C_CLR);
			// FIXME: When doing HW blending with a 24 bit frambuffer and ALPHA.C == 1 (Ad) it should be handled
			// as if Ad = 1.0f. As with OGL side it is probably best to set m_om_bsel.c = 1 (Af) and use
			// AFIX = 0x80 (Af = 1.0f).
		}

		// HW blend can handle Cd output.
		bool color_dest_blend = !!(blend_flag & BLEND_CD);

		if (color_dest_blend) {
			// Output is Cd, set rgb write to 0.
			m_conf.colormask.wrgba &= 0x8;
		}
	}

	void EmulateBlending_Complex(int rt_alpha_min, int rt_alpha_max, bool& DATE_PRIMID, bool& DATE_BARRIER, bool& blending_alpha_pass, Renderer::GS_PRIM_CLASS primclass, const GSState& state)
	{
		auto PRIM = state.PRIM;
		auto PABE = state.PABE;
		auto FRAME = state.FRAME;

		{
			// AA1: Don't enable blending on AA1, not yet implemented on hardware mode,
			// it requires coverage sample so it's safer to turn it off instead.
			const bool AA1 = PRIM.AA1 && (primclass == Renderer::GS_LINE_CLASS || primclass == Renderer::GS_TRIANGLE_CLASS);
			// PABE: Check condition early as an optimization.
			const bool bPABE = PRIM.ABE && PABE.PABE && (GetAlphaMinMax().max < 128);
			// FBMASK: Color is not written, no need to do blending.
			const uint32_t temp_fbmask = m_conf.ps.dfmt == 2 ? 0x00F8F8F8 : 0x00FFFFFF;
			const bool FBMASK = (FRAME.FBMSK & temp_fbmask) == temp_fbmask;

			// No blending or coverage anti-aliasing so early exit
			if (FBMASK || bPABE || !(PRIM.ABE || AA1))
			{
				m_conf.blend = {};
				m_conf.ps.no_color1 = true;
				return;
			}
		}

		// Compute the blending equation to detect special case
		FeatureSupport features;

		features.texture_barrier = true;
		features.dual_source_blend = false;

		const GIFReg::GSAlpha& ALPHA = state.ALPHA;
		const GIFReg::GSColClamp& COLCLAMP = state.COLCLAMP;
		// AFIX: Afix factor.
		uint8_t AFIX = ALPHA.FIX;

		// Set blending to shader bits
		m_conf.ps.blend_a = ALPHA.A;
		m_conf.ps.blend_b = ALPHA.B;
		m_conf.ps.blend_c = ALPHA.C;
		m_conf.ps.blend_d = ALPHA.D;

#ifdef ENABLE_OGL_DEBUG
		static constexpr const char* col[3] = { "Cs", "Cd", "0" };
		static constexpr const char* alpha[3] = { "As", "Ad", "Af" };
		GL_INS("EmulateBlending(): (%s - %s) * %s + %s", col[ALPHA.A], col[ALPHA.B], alpha[ALPHA.C], col[ALPHA.D]);
		GL_INS("Draw AlphaMinMax: %d-%d, RT AlphaMinMax: %d-%d", GetAlphaMinMax().min, GetAlphaMinMax().max, rt_alpha_min, rt_alpha_max);
#endif

		bool blend_ad_improved = false;
		const bool alpha_mask = (FRAME.FBMSK & 0xFF000000) == 0xFF000000;

		// When AA1 is enabled and Alpha Blending is disabled, alpha blending done with coverage instead of alpha.
		// We use a COV value of 128 (full coverage) in triangles (except the edge geometry, which we can't do easily).
		if (IsCoverageAlpha(primclass, state))
		{
			m_conf.ps.fixed_one_a = 1;
			m_conf.ps.blend_c = 0;
		}
		else if (m_conf.ps.blend_c == 1)
		{
			// When both rt alpha min and max are equal replace Ad with Af, easier to manage.
			if (rt_alpha_min == rt_alpha_max)
			{
				AFIX = rt_alpha_min;
				m_conf.ps.blend_c = 2;
			}
			// 24 bits doesn't have an alpha channel so use 128 (1.0f) fix factor as equivalent.
			else if (m_conf.ps.dfmt == 1)
			{
				AFIX = 128;
				m_conf.ps.blend_c = 2;
			}
			// Check whenever we can use rt alpha min as the new alpha value, will be more accurate.
			else if (!alpha_mask && (rt_alpha_min >= (rt_alpha_max / 2)))
			{
				AFIX = rt_alpha_min;
				m_conf.ps.blend_c = 2;
				blend_ad_improved = true;
			}
		}

		// Get alpha value
		const bool alpha_c0_zero = (m_conf.ps.blend_c == 0 && GetAlphaMinMax().max == 0);
		const bool alpha_c0_one = (m_conf.ps.blend_c == 0 && (GetAlphaMinMax().min == 128) && (GetAlphaMinMax().max == 128));
		const bool alpha_c0_high_min_one = (m_conf.ps.blend_c == 0 && GetAlphaMinMax().min > 128);
		const bool alpha_c0_high_max_one = (m_conf.ps.blend_c == 0 && GetAlphaMinMax().max > 128);
		const bool alpha_c2_zero = (m_conf.ps.blend_c == 2 && AFIX == 0u);
		const bool alpha_c2_one = (m_conf.ps.blend_c == 2 && AFIX == 128u);
		const bool alpha_c2_high_one = (m_conf.ps.blend_c == 2 && AFIX > 128u);
		const bool alpha_one = alpha_c0_one || alpha_c2_one;

		// Optimize blending equations, must be done before index calculation
		if ((m_conf.ps.blend_a == m_conf.ps.blend_b) || ((m_conf.ps.blend_b == m_conf.ps.blend_d) && alpha_one))
		{
			// Condition 1:
			// A == B
			// (A - B) * C, result will be 0.0f so set A B to Cs, C to As
			// Condition 2:
			// B == D
			// Swap D with A
			// A == B
			// (A - B) * C, result will be 0.0f so set A B to Cs, C to As
			if (m_conf.ps.blend_a != m_conf.ps.blend_b)
				m_conf.ps.blend_d = m_conf.ps.blend_a;
			m_conf.ps.blend_a = 0;
			m_conf.ps.blend_b = 0;
			m_conf.ps.blend_c = 0;
		}
		else if (alpha_c0_zero || alpha_c2_zero)
		{
			// C == 0.0f
			// (A - B) * C, result will be 0.0f so set A B to Cs
			m_conf.ps.blend_a = 0;
			m_conf.ps.blend_b = 0;
		}
		else if (COLCLAMP.CLAMP && m_conf.ps.blend_a == 2
			&& (m_conf.ps.blend_d == 2 || (m_conf.ps.blend_b == m_conf.ps.blend_d && (alpha_c0_high_min_one || alpha_c2_high_one))))
		{
			// CLAMP 1, negative result will be clamped to 0.
			// Condition 1:
			// (0  - Cs)*Alpha +  0, (0  - Cd)*Alpha +  0
			// Condition 2:
			// Alpha is either As or F higher than 1.0f
			// (0  - Cd)*Alpha  + Cd, (0  - Cs)*F  + Cs
			// Results will be 0.0f, make sure D is set to 2.
			m_conf.ps.blend_a = 0;
			m_conf.ps.blend_b = 0;
			m_conf.ps.blend_c = 0;
			m_conf.ps.blend_d = 2;
		}

		// Ad cases, alpha write is masked, one barrier is enough, for d3d11 read the fb
		// Replace Ad with As, blend flags will be used from As since we are chaging the blend_index value.
		// Must be done before index calculation, after blending equation optimizations
		const bool blend_ad = m_conf.ps.blend_c == 1;
		bool blend_ad_alpha_masked = blend_ad && alpha_mask;
		if (((AccurateBlendingUnit >= AccBlendLevel::Basic) || (COLCLAMP.CLAMP == 0))
			&& features.texture_barrier && blend_ad_alpha_masked)
			m_conf.ps.blend_c = 0;
		else if (((AccurateBlendingUnit >= AccBlendLevel::Medium)
			// Detect barrier aka fbmask on d3d11.
			|| m_conf.require_one_barrier)
			&& blend_ad_alpha_masked)
			m_conf.ps.blend_c = 0;
		else
			blend_ad_alpha_masked = false;

		uint8_t blend_index = static_cast<uint8_t>(((m_conf.ps.blend_a * 3 + m_conf.ps.blend_b) * 3 + m_conf.ps.blend_c) * 3 + m_conf.ps.blend_d);
		const HWBlend blend_preliminary = GetBlend(blend_index, false);
		const int blend_flag = blend_preliminary.flags;

		// Re set alpha, it was modified, must be done after index calculation
		if (blend_ad_alpha_masked)
			m_conf.ps.blend_c = ALPHA.C;

		// HW blend can handle Cd output.
		bool color_dest_blend = !!(blend_flag & BLEND_CD);

		// Do the multiplication in shader for blending accumulation: Cs*As + Cd or Cs*Af + Cd
		bool accumulation_blend = !!(blend_flag & BLEND_ACCU);
		// If alpha == 1.0, almost everything is an accumulation blend!
		// Ones that use (1 + Alpha) can't guarante the mixed sw+hw blending this enables will give an identical result to sw due to clamping
		// But enable for everything else that involves dst color
		if (alpha_one && (m_conf.ps.blend_a != m_conf.ps.blend_d) && blend_preliminary.dst != CONST_ZERO)
			accumulation_blend = true;

		// Blending doesn't require barrier, or sampling of the rt
		const bool blend_non_recursive = !!(blend_flag & BLEND_NO_REC);

		// BLEND MIX selection, use a mix of hw/sw blending
		const bool blend_mix1 = !!(blend_flag & BLEND_MIX1) &&
			(features.dual_source_blend || !(m_conf.ps.blend_b == m_conf.ps.blend_d && (alpha_c0_high_min_one || alpha_c2_high_one)));
		const bool blend_mix2 = !!(blend_flag & BLEND_MIX2);
		const bool blend_mix3 = !!(blend_flag & BLEND_MIX3);
		bool blend_mix = (blend_mix1 || blend_mix2 || blend_mix3) && COLCLAMP.CLAMP;

		const bool one_barrier = m_conf.require_one_barrier || blend_ad_alpha_masked;

		// Blend can be done on hw. As and F cases should be accurate.
		// BLEND_HW_CLR1 with Ad, BLEND_HW_CLR3  Cs > 0.5f will require sw blend.
		// BLEND_HW_CLR1 with As/F and BLEND_HW_CLR2 can be done in hw.
		const bool clr_blend = !!(blend_flag & (BLEND_HW_CLR1 | BLEND_HW_CLR2 | BLEND_HW_CLR3));
		bool clr_blend1_2 = (blend_flag & (BLEND_HW_CLR1 | BLEND_HW_CLR2)) && (m_conf.ps.blend_c != 1) && !blend_ad_improved // Make sure it isn't an Ad case
			&& !PABE.PABE // No PABE as it will require sw blending.
			&& (COLCLAMP.CLAMP) // Let's add a colclamp check too, hw blend will clamp to 0-1.
			&& !(one_barrier || m_conf.require_full_barrier); // Also don't run if there are barriers present.

		// Warning no break on purpose
		// Note: the [[fallthrough]] attribute tell compilers not to complain about not having breaks.
		bool sw_blending = false;
		if (features.texture_barrier)
		{
			// Condition 1: Require full sw blend for full barrier.
			// Condition 2: One barrier is already enabled, prims don't overlap so let's use sw blend instead.
			const bool prefer_sw_blend = m_conf.require_full_barrier || (one_barrier && m_prim_overlap == PRIM_OVERLAP_NO);
			const bool no_prim_overlap = (m_prim_overlap == PRIM_OVERLAP_NO);
			const bool free_blend = blend_non_recursive // Free sw blending, doesn't require barriers or reading fb
				|| accumulation_blend; // Mix of hw/sw blending
			const bool blend_requires_barrier = (blend_flag & BLEND_A_MAX) // Impossible blending
				|| (m_conf.require_full_barrier) // Another effect (for example fbmask) already requires a full barrier
				// Blend can be done in a single draw, and we already need a barrier
				// On fbfetch, one barrier is like full barrier
				|| (one_barrier && (no_prim_overlap || features.framebuffer_fetch))
				|| ((alpha_c2_high_one || alpha_c0_high_max_one) && no_prim_overlap)
				// Ad blends are completely wrong without sw blend (Ad is 0.5 not 1 for 128). We can spare a barrier for it.
				|| ((blend_ad || blend_ad_improved) && no_prim_overlap);

			switch (AccurateBlendingUnit)
			{
			case AccBlendLevel::Maximum:
				clr_blend1_2 = false;
				sw_blending |= true;
				[[fallthrough]];
			case AccBlendLevel::Full:
				sw_blending |= m_conf.ps.blend_a != m_conf.ps.blend_b && alpha_c0_high_max_one;
				[[fallthrough]];
			case AccBlendLevel::High:
				sw_blending |= m_conf.ps.blend_c == 1 || (m_conf.ps.blend_a != m_conf.ps.blend_b && alpha_c2_high_one);
				[[fallthrough]];
			case AccBlendLevel::Medium:
				// Initial idea was to enable accurate blending for sprite rendering to handle
				// correctly post-processing effect. Some games (ZoE) use tons of sprites as particles.
				// In order to keep it fast, let's limit it to smaller draw call.
				assert(false);
				//sw_blending |= primclass == Renderer::GS_SPRITE_CLASS && m_drawlist.size() < 100;
				[[fallthrough]];
			case AccBlendLevel::Basic:
				// SW FBMASK, needs sw blend, avoid hitting any hw blend pre enabled (accumulation, blend mix, blend cd),
				// fixes shadows in Superman shadows of Apokolips.
				// DATE_BARRIER already does full barrier so also makes more sense to do full sw blend.
				color_dest_blend &= !prefer_sw_blend;
				// If prims don't overlap prefer full sw blend on blend_ad_alpha_masked cases.
				accumulation_blend &= !(prefer_sw_blend || (blend_ad_alpha_masked && m_prim_overlap == PRIM_OVERLAP_NO));
				// Enable sw blending for barriers.
				sw_blending |= blend_requires_barrier;
				// Try to do hw blend for clr2 case.
				sw_blending &= !clr_blend1_2;
				// blend_ad_improved should only run if no other barrier blend is enabled, otherwise restore bit values.
				if (blend_ad_improved && (sw_blending || prefer_sw_blend))
				{
					AFIX = 0;
					m_conf.ps.blend_c = 1;
				}
				// Enable sw blending for free blending, should be done after blend_ad_improved check.
				sw_blending |= free_blend;
				// Do not run BLEND MIX if sw blending is already present, it's less accurate.
				blend_mix &= !sw_blending;
				sw_blending |= blend_mix;
				// Disable dithering on blend mix.
				m_conf.ps.dither &= !blend_mix;
				[[fallthrough]];
			case AccBlendLevel::Minimum:
				break;
			}
		}
		else
		{
			// FBMASK or channel shuffle already reads the fb so it is safe to enable sw blend when there is no overlap.
			const bool fbmask_no_overlap = m_conf.require_one_barrier && (m_prim_overlap == PRIM_OVERLAP_NO);

			switch (AccurateBlendingUnit)
			{
			case AccBlendLevel::Maximum:
				if (m_prim_overlap == PRIM_OVERLAP_NO)
				{
					clr_blend1_2 = false;
					sw_blending |= true;
				}
				[[fallthrough]];
			case AccBlendLevel::Full:
				sw_blending |= ((m_conf.ps.blend_c == 1 || (blend_mix && (alpha_c2_high_one || alpha_c0_high_max_one))) && (m_prim_overlap == PRIM_OVERLAP_NO));
				[[fallthrough]];
			case AccBlendLevel::High:
				sw_blending |= (!(clr_blend || blend_mix) && (m_prim_overlap == PRIM_OVERLAP_NO));
				[[fallthrough]];
			case AccBlendLevel::Medium:
				// If prims don't overlap prefer full sw blend on blend_ad_alpha_masked cases.
				if (blend_ad_alpha_masked && m_prim_overlap == PRIM_OVERLAP_NO)
				{
					accumulation_blend = false;
					sw_blending |= true;
				}
				[[fallthrough]];
			case AccBlendLevel::Basic:
				// Disable accumulation blend when there is fbmask with no overlap, will be faster.
				color_dest_blend &= !fbmask_no_overlap;
				accumulation_blend &= !fbmask_no_overlap;
				// Blending requires reading the framebuffer when there's no overlap.
				sw_blending |= fbmask_no_overlap;
				// Try to do hw blend for clr2 case.
				sw_blending &= !clr_blend1_2;
				// blend_ad_improved should only run if no other barrier blend is enabled, otherwise restore bit values.
				if (blend_ad_improved && (sw_blending || fbmask_no_overlap))
				{
					AFIX = 0;
					m_conf.ps.blend_c = 1;
				}
				// Enable sw blending for free blending, should be done after blend_ad_improved check.
				sw_blending |= accumulation_blend || blend_non_recursive;
				// Do not run BLEND MIX if sw blending is already present, it's less accurate.
				blend_mix &= !sw_blending;
				sw_blending |= blend_mix;
				// Disable dithering on blend mix.
				m_conf.ps.dither &= !blend_mix;
				[[fallthrough]];
			case AccBlendLevel::Minimum:
				break;
			}
		}

		bool replace_dual_src = false;
		if (!features.dual_source_blend && IsDualSourceBlend(blend_index))
		{
			// if we don't have an alpha channel, we don't need a second pass, just output the alpha blend
			// in the single colour's alpha chnanel, and blend with it
			if (!m_conf.colormask.wa)
			{
				GL_INS("Outputting alpha blend in col0 because of no alpha write");
				m_conf.ps.no_ablend = true;
				replace_dual_src = true;
			}
			else if (features.framebuffer_fetch || m_conf.require_one_barrier || m_conf.require_full_barrier)
			{
				// prefer single pass sw blend (if barrier) or framebuffer fetch over dual pass alpha when supported
				sw_blending = true;
				color_dest_blend = false;
				accumulation_blend &= !features.framebuffer_fetch;
				blend_mix = false;
			}
			else
			{
				// split the draw into two
				blending_alpha_pass = true;
				replace_dual_src = true;
			}
		}
		else if (features.framebuffer_fetch)
		{
			// If we have fbfetch, use software blending when we need the fb value for anything else.
			// This saves outputting the second color when it's not needed.
			if (one_barrier || m_conf.require_full_barrier)
			{
				sw_blending = true;
				color_dest_blend = false;
				accumulation_blend = false;
				blend_mix = false;
			}
		}

		// Color clip
		if (COLCLAMP.CLAMP == 0)
		{
			bool free_colclip = false;
			if (features.framebuffer_fetch)
				free_colclip = true;
			else if (features.texture_barrier)
				free_colclip = m_prim_overlap == PRIM_OVERLAP_NO || blend_non_recursive;
			else
				free_colclip = blend_non_recursive;

			GL_DBG("COLCLIP Info (Blending: %u/%u/%u/%u, OVERLAP: %d)", m_conf.ps.blend_a, m_conf.ps.blend_b, m_conf.ps.blend_c, m_conf.ps.blend_d, m_prim_overlap);
			if (color_dest_blend)
			{
				// No overflow, disable colclip.
				GL_INS("COLCLIP mode DISABLED");
			}
			else if (free_colclip)
			{
				// The fastest algo that requires a single pass
				GL_INS("COLCLIP Free mode ENABLED");
				m_conf.ps.colclip = 1;
				sw_blending = true;
				// Disable the HDR algo
				accumulation_blend = false;
				blend_mix = false;
			}
			else if (accumulation_blend)
			{
				// A fast algo that requires 2 passes
				GL_INS("COLCLIP Fast HDR mode ENABLED");
				m_conf.ps.hdr = 1;
				sw_blending = true; // Enable sw blending for the HDR algo
			}
			else if (sw_blending)
			{
				// A slow algo that could requires several passes (barely used)
				GL_INS("COLCLIP SW mode ENABLED");
				m_conf.ps.colclip = 1;
			}
			else
			{
				GL_INS("COLCLIP HDR mode ENABLED");
				m_conf.ps.hdr = 1;
			}
		}

		// Per pixel alpha blending
		if (PABE.PABE)
		{
			// Breath of Fire Dragon Quarter, Strawberry Shortcake, Super Robot Wars, Cartoon Network Racing.

			if (sw_blending)
			{
				GL_INS("PABE mode ENABLED");
				if (features.texture_barrier)
				{
					// Disable hw/sw blend and do pure sw blend with reading the framebuffer.
					color_dest_blend = false;
					accumulation_blend = false;
					blend_mix = false;
					m_conf.ps.pabe = 1;

					// HDR mode should be disabled when doing sw blend, swap with sw colclip.
					if (m_conf.ps.hdr)
					{
						m_conf.ps.hdr = 0;
						m_conf.ps.colclip = 1;
					}
				}
				else
				{
					m_conf.ps.pabe = !(accumulation_blend || blend_mix);
				}
			}
			else if (m_conf.ps.blend_a == 0 && m_conf.ps.blend_b == 1 && m_conf.ps.blend_c == 0 && m_conf.ps.blend_d == 1)
			{
				// this works because with PABE alpha blending is on when alpha >= 0x80, but since the pixel shader
				// cannot output anything over 0x80 (== 1.0) blending with 0x80 or turning it off gives the same result
				blend_index = 0;
			}
		}

		// For stat to optimize accurate option
#if 0
		GL_INS("BLEND_INFO: %u/%u/%u/%u. Clamp:%u. Prim:%d number %u (drawlist %zu) (sw %d)",
			m_conf.ps.blend_a, m_conf.ps.blend_b, m_conf.ps.blend_c, m_conf.ps.blend_d,
			m_env.COLCLAMP.CLAMP, m_vt.m_primclass, m_vertex.next, m_drawlist.size(), sw_blending);
#endif
		if (color_dest_blend)
		{
			// Blend output will be Cd, disable hw/sw blending.
			m_conf.blend = {};
			m_conf.ps.no_color1 = true;
			m_conf.ps.blend_a = m_conf.ps.blend_b = m_conf.ps.blend_c = m_conf.ps.blend_d = 0;
			sw_blending = false; // DATE_PRIMID

			// Output is Cd, set rgb write to 0.
			m_conf.colormask.wrgba &= 0x8;
		}
		else if (sw_blending)
		{
			// Require the fix alpha vlaue
			if (m_conf.ps.blend_c == 2)
			{
				assert(false);
				//m_conf.cb_ps.TA_MaxDepth_Af.a = static_cast<float>(AFIX) / 128.0f;
			}

			const HWBlend blend = GetBlend(blend_index, replace_dual_src);
			if (accumulation_blend)
			{
				// Keep HW blending to do the addition/subtraction
				m_conf.blend = { true, CONST_ONE, CONST_ONE, (u8)blend.op, false, 0 };
				blending_alpha_pass = false;

				// Remove Cd from sw blend, it's handled in hw
				if (m_conf.ps.blend_a == 1)
					m_conf.ps.blend_a = 2;
				if (m_conf.ps.blend_b == 1)
					m_conf.ps.blend_b = 2;
				if (m_conf.ps.blend_d == 1)
					m_conf.ps.blend_d = 2;

				if (m_conf.ps.blend_a == 2)
				{
					// Accumulation blend is only available in (Cs - 0)*Something + Cd, or with alpha == 1
					ASSERT(m_conf.ps.blend_d == 2 || alpha_one);
					// A bit of normalization
					m_conf.ps.blend_a = m_conf.ps.blend_d;
					m_conf.ps.blend_d = 2;
				}

				if (blend.op == OP_REV_SUBTRACT)
				{
					ASSERT(m_conf.ps.blend_a == 2);
					if (m_conf.ps.hdr)
					{
						// HDR uses unorm, which is always positive
						// Have the shader do the inversion, then clip to remove the negative
						m_conf.blend.op = OP_ADD;
					}
					else
					{
						// The blend unit does a reverse subtraction so it means
						// the shader must output a positive value.
						// Replace 0 - Cs by Cs - 0
						m_conf.ps.blend_a = m_conf.ps.blend_b;
						m_conf.ps.blend_b = 2;
					}
				}

				// Dual source output not needed (accumulation blend replaces it with ONE).
				m_conf.ps.no_color1 = true;

				// Only Ad case will require one barrier
				// No need to set a_masked bit for blend_ad_alpha_masked case
				m_conf.require_one_barrier |= blend_ad_alpha_masked;
			}
			else if (blend_mix)
			{
				// For mixed blend, the source blend is done in the shader (so we use CONST_ONE as a factor).
				m_conf.blend = { true, CONST_ONE, (u8)blend.dst, (u8)blend.op, m_conf.ps.blend_c == 2, AFIX };
				m_conf.ps.blend_mix = (blend.op == OP_REV_SUBTRACT) ? 2 : 1;

				// Elide DSB colour output if not used by dest.
				m_conf.ps.no_color1 |= !IsDualSourceBlendFactor(blend.dst);

				if (blend_mix1)
				{
					if (m_conf.ps.blend_b == m_conf.ps.blend_d && (alpha_c0_high_min_one || alpha_c2_high_one))
					{
						// Replace Cs*As + Cd*(1 - As) with Cs*As - Cd*(As - 1).
						// Replace Cs*F + Cd*(1 - F) with Cs*F - Cd*(F - 1).
						// As - 1 or F - 1 subtraction is only done for the dual source output (hw blending part) since we are changing the equation.
						// Af will be replaced with As in shader and send it to dual source output.
						m_conf.blend = { true, CONST_ONE, SRC1_COLOR, OP_SUBTRACT, false, 0 };
						// blend hw 1 will disable alpha clamp, we can reuse the old bits.
						m_conf.ps.blend_hw = 1;
						// DSB output will always be used.
						m_conf.ps.no_color1 = false;
					}
					else if (m_conf.ps.blend_a == m_conf.ps.blend_d)
					{
						// Compensate slightly for Cd*(As + 1) - Cs*As.
						// Try to compensate a bit with subtracting 1 (0.00392) * (Alpha + 1) from Cs.
						m_conf.ps.blend_hw = 2;
					}

					m_conf.ps.blend_a = 0;
					m_conf.ps.blend_b = 2;
					m_conf.ps.blend_d = 2;
				}
				else if (blend_mix2)
				{
					// Allow to compensate when Cs*(Alpha + 1) overflows, to compensate we change
					// the alpha output value for Cd*Alpha.
					m_conf.blend = { true, CONST_ONE, SRC1_COLOR, (u8)blend.op, false, 0 };
					m_conf.ps.blend_hw = 3;
					m_conf.ps.no_color1 = false;

					m_conf.ps.blend_a = 0;
					m_conf.ps.blend_b = 2;
					m_conf.ps.blend_d = 0;
				}
				else if (blend_mix3)
				{
					m_conf.ps.blend_a = 2;
					m_conf.ps.blend_b = 0;
					m_conf.ps.blend_d = 0;
				}

				// Only Ad case will require one barrier
				if (blend_ad_alpha_masked)
				{
					// Swap Ad with As for hw blend
					m_conf.ps.a_masked = 1;
					m_conf.require_one_barrier |= true;
				}
			}
			else
			{
				// Disable HW blending
				m_conf.blend = {};
				m_conf.ps.no_color1 = true;
				replace_dual_src = false;
				blending_alpha_pass = false;

				// No need to set a_masked bit for blend_ad_alpha_masked case
				const bool blend_non_recursive_one_barrier = blend_non_recursive && blend_ad_alpha_masked;
				if (blend_non_recursive_one_barrier)
					m_conf.require_one_barrier |= true;
				else if (features.texture_barrier)
					m_conf.require_full_barrier |= !blend_non_recursive;
				else
					m_conf.require_one_barrier |= !blend_non_recursive;
			}
		}
		else
		{
			// No sw blending
			m_conf.ps.blend_a = 0;
			m_conf.ps.blend_b = 0;
			m_conf.ps.blend_d = 0;

			// Care for hw blend value, 6 is for hw/sw, sw blending used.
			if (blend_flag & BLEND_HW_CLR1)
			{
				m_conf.ps.blend_hw = 1;
			}
			else if (blend_flag & BLEND_HW_CLR2)
			{
				if (m_conf.ps.blend_c == 2)
				{
					assert(false);
					//m_conf.cb_ps.TA_MaxDepth_Af.a = static_cast<float>(AFIX) / 128.0f;
				}

				m_conf.ps.blend_hw = 2;
			}
			else if (blend_flag & BLEND_HW_CLR3)
			{
				m_conf.ps.blend_hw = 3;
			}

			if (blend_ad_alpha_masked)
			{
				m_conf.ps.a_masked = 1;
				m_conf.require_one_barrier |= true;
			}

			const HWBlend blend(GetBlend(blend_index, replace_dual_src));
			m_conf.blend = { true, (u8)blend.src, (u8)blend.dst, (u8)blend.op, m_conf.ps.blend_c == 2, AFIX };

			// Remove second color output when unused. Works around bugs in some drivers (e.g. Intel).
			m_conf.ps.no_color1 |= !IsDualSourceBlendFactor(m_conf.blend.src_factor) &&
				!IsDualSourceBlendFactor(m_conf.blend.dst_factor);
		}

		// Notify the shader that it needs to invert rounding
		if (m_conf.blend.op == OP_REV_SUBTRACT)
			m_conf.ps.round_inv = 1;

		// DATE_PRIMID interact very badly with sw blending. DATE_PRIMID uses the primitiveID to find the primitive
		// that write the bad alpha value. Sw blending will force the draw to run primitive by primitive
		// (therefore primitiveID will be constant to 1).
		// Switch DATE_PRIMID with DATE_BARRIER in such cases to ensure accuracy.
		// No mix of COLCLIP + sw blend + DATE_PRIMID, neither sw fbmask + DATE_PRIMID.
		// Note: Do the swap in the end, saves the expensive draw splitting/barriers when mixed software blending is used.
		if (sw_blending && DATE_PRIMID && m_conf.require_full_barrier)
		{
			GL_PERF("DATE: Swap DATE_PRIMID with DATE_BARRIER");
			m_conf.require_full_barrier = true;
			DATE_PRIMID = false;
			DATE_BARRIER = true;
		}
	}

	void UpdateHWPipelineSelector(GSHWDrawConfig& config, PipelineSelector& pipe)
	{
		pipe.vs.key = config.vs.key;
		pipe.ps.key_hi = config.ps.key_hi;
		pipe.ps.key_lo = config.ps.key_lo;
		pipe.dss.key = config.depth.key;
		pipe.bs.key = config.blend.key;
		pipe.bs.constant = 0; // don't dupe states with different alpha values
		pipe.cms.key = config.colormask.key;
		pipe.topology = static_cast<u32>(config.topology);
		//pipe.rt = config.rt != nullptr;
		//pipe.ds = config.ds != nullptr;
		//pipe.line_width = config.line_expand;
		//pipe.feedback_loop_flags =
		//	(m_features.texture_barrier &&
		//		(config.ps.IsFeedbackLoop() || config.require_one_barrier || config.require_full_barrier)) ?
		//	FeedbackLoopFlag_ReadAndWriteRT :
		//	FeedbackLoopFlag_None;
		//pipe.feedback_loop_flags |=
		//	(config.tex && config.tex == config.ds) ? FeedbackLoopFlag_ReadDS : FeedbackLoopFlag_None;

		// enable point size in the vertex shader if we're rendering points regardless of upscaling.
		pipe.vs.point_size |= (config.topology == GSHWDrawConfig::Topology::Point);
	}

	PipelineSelector m_pipelineSelector;
}

namespace PipelineDebug
{
	const char* gTopologyNames[4] = {
		"VK_PRIMITIVE_TOPOLOGY_POINT_LIST",
		"VK_PRIMITIVE_TOPOLOGY_LINE_LIST",
		"",
		"VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST",
	};
}

void Renderer::SetImagePointer(Renderer::TextureData inImage) {
	gImageData = inImage;
}

Renderer::TextureData& Renderer::GetImagePointer() {
	return gImageData;
}

void Renderer::Draw() {
	Draw(GetDefaultDrawBuffer(), gImageData, PS2::GetGSState());
}

void Renderer::Draw(PS2::DrawBufferData<Renderer::GSVertex, uint16_t>& drawBuffer, TextureData& textureData, PS2::GSState& state) {
	if (drawBuffer.index.tail == 0) {
		return;
	}

	Log::GetInstance().AddLog(LogLevel::Verbose, "RendererPS2", "Draw: {0}(0x{0:x})", drawBuffer.index.tail);

	g_GSSelector.ResetStates();
	PS2::m_conf.ps = GSHWDrawConfig::PSSelector();

	const bool unscale_pt_ln = false;

	VkPrimitiveTopology topology;

	switch (GetPrimClass(state.PRIM.PRIM))
	{
	case GS_POINT_CLASS:
		if (unscale_pt_ln)
		{
			g_GSSelector.point = 1;
			//gs_cb.PointSize = GSVector2(16.0f * sx, 16.0f * sy);
		}

		topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case GS_LINE_CLASS:
		if (unscale_pt_ln)
		{
			g_GSSelector.line = 1;
			//gs_cb.PointSize = GSVector2(16.0f * sx, 16.0f * sy);
		}

		topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		break;
	case GS_SPRITE_CLASS:
		topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	case GS_TRIANGLE_CLASS:

		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		break;
	default:
		__assume(0);
	}

	g_GSSelector.iip = state.PRIM.IIP;
	auto primclass = GetPrimClass(state.PRIM.PRIM);
	g_GSSelector.prim = primclass;

	LogTex("Lookup", state.TEX);
	PS2::GSTexEntry& tex = PS2::GetTextureCache().Lookup(state.TEX, textureData);

	// Blend
	int blend_alpha_min = 0, blend_alpha_max = 255;
	bool DATE_PRIMID = false;
	bool DATE_BARRIER = false;

	bool blending_alpha_pass = false;
	if ((!state.IsOpaque() || state.ALPHA.IsBlack()) /* && rt */ && (PS2::m_conf.colormask.wrgba & 0x7))
	{
		if (PS2_Internal::bUseComplexBlending) {
			PS2::EmulateBlending_Complex(blend_alpha_min, blend_alpha_max, DATE_PRIMID, DATE_BARRIER, blending_alpha_pass, primclass, state);
		}
		else {
			PS2::EmulateBlending_Simple(primclass, tex.value.image, state);
		}
	}
	else
	{
		PS2::m_conf.blend = {}; // No blending please
		PS2::m_conf.ps.no_color1 = true;
	}

	PS2::EmulateAtst(1, tex.value.image, state);

	if (state.bTexSet) {
		PS2::EmulateTextureSampler(tex.value.image, state);
	}
	else {
		tex.value.image.UpdateSampler();
		PS2::m_conf.ps.tfx = 4;
	}

	if (state.PRIM.FGE)
	{
		PS2::m_conf.ps.fog = 1;

		//GSVector4 fc = GSVector4::rgba32(m_env.FOGCOL.u32[0]);
#if _M_SSE >= 0x401
		// Blend AREF to avoid to load a random value for alpha (dirty cache)
		ps_cb.FogColor_AREF = fc.blend32<8>(ps_cb.FogColor_AREF);
#else
		//ps_cb.FogColor_AREF = fc;
#endif
	}

	PS2::m_conf.vs.tme = state.PRIM.TME;
	PS2::m_conf.vs.fst = state.PRIM.FST;

	const std::string GS_CONFIG_NAME = 
		  std::string("-DGS_PRIM=") + std::to_string(g_GSSelector.prim) + " "
		+ std::string("-DGS_POINT=") + std::to_string(g_GSSelector.point) + " "
		+ std::string("-DGS_LINE=") + std::to_string(g_GSSelector.line) + " "
		+ std::string("-DGS_IIP=") + std::to_string(g_GSSelector.iip);

	const std::string PS_CONFIG_NAME = 
		  std::string("-DPS_ATST=") + std::to_string(PS2::m_conf.ps.atst) + " "
		+ std::string("-DPS_FOG=") + std::to_string(PS2::m_conf.ps.fog) + " "
		+ std::string("-DPS_COLCLIP=") + std::to_string(PS2::m_conf.ps.colclip) + " "
		+ std::string("-DPS_BLEND_A=") + std::to_string(PS2::m_conf.ps.blend_a) + " "
		+ std::string("-DPS_BLEND_B=") + std::to_string(PS2::m_conf.ps.blend_b) + " "
		+ std::string("-DPS_BLEND_C=") + std::to_string(PS2::m_conf.ps.blend_c) + " "
		+ std::string("-DPS_BLEND_D=") + std::to_string(PS2::m_conf.ps.blend_d) + " "
		+ std::string("-DPS_TFX=") + std::to_string(PS2::m_conf.ps.tfx);

	const std::string VS_CONFIG_NAME = 
		  std::string("-DVS_TME=") + std::to_string(PS2::m_conf.vs.tme) + " "
		+ std::string("-DVS_FST=") + std::to_string(PS2::m_conf.vs.fst);

	//Log::GetInstance().AddLog(LogLevel::Verbose, "RendererPS2", "DATE: %d DATM: %d", state.TEST.DATE, state.TEST.DATM);

	const PipelineDebug::ConfigData debugData = { PipelineDebug::gTopologyNames[(int)topology]};

	PS2::UpdateHWPipelineSelector(PS2::m_conf, PS2::m_pipelineSelector);

	const PS2::PipelineKey key = { {GS_CONFIG_NAME, PS_CONFIG_NAME, VS_CONFIG_NAME}, topology, PS2::m_pipelineSelector, debugData };
	
	auto pipeline = PS2::GetPipeline(key);

	{
		if (!hwState.bActivePass || hwState.FBP != state.FRAME.FBP) {

			if (hwState.bActivePass) {
				vkCmdEndRenderPass(GetCurrentCommandBuffer());
			}

			PS2::FrameBuffer& frameBuffer = PS2::FrameBuffer::Get(state.FRAME.FBP);

			hwState.FBP = state.FRAME.FBP;
			hwState.bActivePass = true;

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = frameBuffer.renderPass;
			renderPassInfo.framebuffer = frameBuffer.framebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { (unsigned int)GetRTSize().x, (unsigned int)GetRTSize().y };

			VkClearValue clearColors[] = { {{0.0f, 0.0f, 0.0f, 1.0f}}, {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = clearColors;

			vkCmdBeginRenderPass(GetCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			float m_hack_topleft_offset = (false) ? -0.01f : 0.0f;
			VkViewport viewport{};
			viewport.x = m_hack_topleft_offset;
			viewport.y = m_hack_topleft_offset;
			viewport.width = (float)GetRTSize().x;
			viewport.height = (float)GetRTSize().y;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(GetCurrentCommandBuffer(), 0, 1, &viewport);
		}

		{
			float sx = 2.0f * GetRTScale().x / (GetRTSize().x << 4);
			float sy = 2.0f * GetRTScale().y / (GetRTSize().y << 4);
			float ox = state.XY.X;
			float oy = state.XY.Y;
			float ox2 = -1.0f / GetRTSize().x;
			float oy2 = -1.0f / GetRTSize().y;

			tex.value.image.constantBuffer.GetVertexConstantBufferData().VertexScale = GSVector4(sx, -sy, ldexpf(1, -32), 0.0f);
			tex.value.image.constantBuffer.GetVertexConstantBufferData().VertexOffset = GSVector4(ox * sx + ox2 + 1, -(oy * sy + oy2 + 1), 0.0f, -1.0f);

			tex.value.image.constantBuffer.UpdateUniformBuffers();
		}

		vkCmdSetScissor(GetCurrentCommandBuffer(), 0, 1, &hwState.scissor);

		vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

		PS2_Internal::gVertexBuffers.BindData(GetCurrentCommandBuffer());

		vkCmdBindDescriptorSets(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &tex.value.image.GetDescriptorSets(pipeline).GetSet(GetCurrentFrame()), 0, nullptr);

		vkCmdDrawIndexed(GetCurrentCommandBuffer(), static_cast<uint32_t>(drawBuffer.index.tail), 1, 0, 0, 0);

	}

	drawBuffer.ResetAfterDraw();
	state.bTexSet = false;
}

void PS2::Setup()
{
	CreateDefaultRenderPass();

	PS2_Internal::gVertexBuffers.Init(Renderer::VertexIndexBufferSizeGPU, Renderer::VertexIndexBufferSizeGPU);
}

void PS2::BeginFrame()
{
	PS2_Internal::gVertexBuffers.Reset();
}

void PS2::Cleanup()
{
	for (auto& pipeline : PS2::GetPipelines()) {
		vkDestroyPipeline(GetDevice(), pipeline.second.pipeline, nullptr);
		vkDestroyPipelineLayout(GetDevice(), pipeline.second.layout, nullptr);
	}
	
	//vkDestroyRenderPass(GetDevice(), PS2_Internal::renderPassPs2, nullptr);
}