#include "VulkanPS2.h"
#include "renderer.h"
#include "VulkanRenderer.h"
#include <sstream>
#include "GSState.h"
#include "Objects/Pipeline.h"
#include "GSVector.h"
#include "Objects/UniformBuffer.h"
#include "log.h"
#include "Native/NativeRenderer.h"

namespace PS2
{
	namespace Hardware {
		FrameVertexBuffers<Renderer::GSVertexUnprocessed, uint16_t> gVertexBuffers;

		void Setup() {
			gVertexBuffers.Init(Renderer::VertexIndexBufferSizeGPU, Renderer::VertexIndexBufferSizeGPU);
		}
	}
}

PS2::DrawBufferData<Renderer::GSVertexUnprocessed, uint16_t>& Renderer::GetHardwareDrawBuffer()
{
	return PS2::Hardware::gVertexBuffers.GetDrawBufferData();
}

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

	UniformBuffer<PS2::VSConstantBuffer> gVertexConstBuffer;
	UniformBuffer<PS2::PSConstantBuffer> gPixelConstBuffer;
}

UniformBuffer<PS2::VSConstantBuffer>& PS2::GetVertexUniformBuffer() {
	return PS2_Internal::gVertexConstBuffer;
}

UniformBuffer<PS2::PSConstantBuffer>& PS2::GetPixelUniformBuffer() {
	return PS2_Internal::gPixelConstBuffer;
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

	bool& GetUseComplexBlending() {
		return PS2_Internal::bUseComplexBlending;
	}

	void ResetRenderer() {
		PS2::GetPipelines().clear();
	}

	void ResetVertIndexBuffers()
	{
		PS2_Internal::gVertexBuffers.GetDrawBufferData().Reset();
		PS2::Hardware::gVertexBuffers.GetDrawBufferData().Reset();
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

	static std::string TexToString(const GIFReg::GSTex tex)
	{
#ifdef USE_STRING_STREAMS
		// Create a formatted log message string
		std::ostringstream oss;
		oss << "tbp: " << tex.TBP0 << ", tbw: " << tex.TBW << ", psm: " << tex.PSM
			<< ", tw: " << tex.TW << ", th: " << tex.TH << ", tcc: " << tex.TCC
			<< ", tfx: " << tex.TFX << ", cbp: " << tex.CBP << " (0x" << std::hex << tex.CBP << "), cpsm: " << tex.CPSM
			<< ", csm: " << tex.CSM << ", csa: " << tex.CSA << ", cld: " << tex.CLD;

		return oss.str();
#else
		// Todo
		return std::string();
#endif
	}

	void LogTex(const char* prefix, const GIFReg::GSTex tex)
	{		
		// Log the formatted message
		VULKAN_LOG(LogLevel::Verbose, "{} - {}", prefix, TexToString(tex));
		VULKAN_LOG(LogLevel::Verbose, "{} - 0x{:x}", prefix, tex.CMD);
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

	void SetZbuf(uint32_t zmask)
	{
		PS2::GetGSState().ZBUF.ZMSK = zmask;
	}

	void SetPrim(GIFReg::GSPrim prim, PS2::DrawBufferData<Renderer::GSVertex, uint16_t>* pDrawBuffer /*= nullptr*/) {
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

		auto& state = PS2::GetGSState();

		state.TEX = tex;
		state.bTexSet = true;

		switch (tex.CLD) {
		case 0:
		case 1:
			// Nothing.
			break;
		case 2:
			state.CachedCBP[0] = tex.CBP;
			break;
		case 3:
			state.CachedCBP[1] = tex.CBP;
			break;
		}
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
		return;
		auto vtx = PS2_Internal::MakeVertex(x, y, z);
		KickVertex<GSVertex, uint16_t>(vtx, PS2::GetGSState().PRIM, Skip, GetDefaultDrawBuffer());
	}

	template<>
	void UpdateXyTail<GSVertexUnprocessed, uint16_t>(const GSVertexUnprocessed& vtx, PS2::DrawBufferData<GSVertexUnprocessed, uint16_t>& drawBuffer, const size_t& xy_tail) {
	}

	template<>
	void UpdateXyTail<GSVertexUnprocessedNormal, uint16_t>(const GSVertexUnprocessedNormal& vtx, PS2::DrawBufferData<GSVertexUnprocessedNormal, uint16_t>& drawBuffer, const size_t& xy_tail) {
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
	void UpdateXyTail<DisplayListVertex, uint16_t>(const DisplayListVertex& vtx, PS2::DrawBufferData<DisplayListVertex, uint16_t>& drawBuffer, const size_t& xy_tail) {
		drawBuffer.vertex.fxyz[xy_tail & 3][0] = vtx.XYZ[0];
		drawBuffer.vertex.fxyz[xy_tail & 3][1] = vtx.XYZ[1];
		drawBuffer.vertex.fxyz[xy_tail & 3][2] = vtx.XYZ[2];
	}

	template<>
	void TraceUpdateSkip<GSVertexUnprocessedNormal, uint16_t>(uint32_t& skip, PS2::DrawBufferData<GSVertexUnprocessedNormal, uint16_t>& drawBuffer, const GS_PRIM& prim, const size_t& xy_tail, const size_t& m)
	{

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

		m_scissor.U16[0] = (uint16_t)((hwState.scissor.offset.x << 4) + OFX - 0x8000);
		m_scissor.U16[1] = (uint16_t)((hwState.scissor.offset.y << 4) + OFY - 0x8000);
		m_scissor.U16[2] = (uint16_t)((hwState.scissor.extent.width << 4) + OFX - 0x8000);
		m_scissor.U16[3] = (uint16_t)((hwState.scissor.extent.height << 4) + OFY - 0x8000);

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

	template<>
	void TraceUpdateSkip<DisplayListVertex, uint16_t>(uint32_t& skip, PS2::DrawBufferData<DisplayListVertex, uint16_t>& drawBuffer, const GS_PRIM& prim, const size_t& xy_tail, const size_t& m)
	{
		auto v0 = drawBuffer.vertex.fxyz[(xy_tail + 1) & 3]; // T-3
		auto v1 = drawBuffer.vertex.fxyz[(xy_tail + 2) & 3]; // T-2
		auto v2 = drawBuffer.vertex.fxyz[(xy_tail + 3) & 3]; // T-1
		auto v3 = drawBuffer.vertex.fxyz[(xy_tail - m) & 3]; // H

		auto verticesEqual = [](const float* a, const float* b) {
			return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
			};

		switch (prim)
		{
		case GS_TRIANGLELIST:
		case GS_TRIANGLESTRIP:
			if (verticesEqual(v0, v1) || verticesEqual(v1, v2) || verticesEqual(v0, v2)) {
				skip = 0;
			}
			break;
		case GS_TRIANGLEFAN:
			/*
			cross = GSVector4(v2.xyxyl().i16to32().sub32(v3.upl32(v1).i16to32())); // x23, y23, x21, y21
			cross = cross * cross.wzwz(); // x23 * y21, y23 * x21
			test |= GSVector4i::cast(cross == cross.yxwz());
			*/
			//test = (test | v3 == v1) | (v1 == v2 | v3 == v2);
			break;
		default:
			break;
		}
	}

	void SetScissor(int x, int y, uint32_t width, uint32_t height) {
		assert(x >= 0 && y >= 0);
		hwState.scissor.offset = { x, y };
		hwState.scissor.extent = { width, height };
	}
}

namespace PS2
{
	PSSamplerSelector EmulateTextureSampler(int width, int height)
	{
		return EmulateTextureSampler(width, height, PS2_Internal::state.CLAMP, PS2_Internal::state.TEX, PS2_Internal::state.PRIM);
	}

	PSSamplerSelector EmulateTextureSampler(int width, int height, const GIFReg::GSClamp& CLAMP, const GIFReg::GSTex& TEX, const GIFReg::GSPrim& PRIM)
	{
		auto& vs_cb = PS2_Internal::gVertexConstBuffer.GetBufferData();
		auto& ps_cb = PS2_Internal::gPixelConstBuffer.GetBufferData();

		const uint8_t wms = CLAMP.WMS;
		const uint8_t wmt = CLAMP.WMT;
		const bool bilinear = false;

		const int tw = 1 << TEX.TW;
		const int th = 1 << TEX.TH;
		const GSVector4 wh(tw, th, width, height);

		const GSVector4 textureScale = GSVector4(0.0625f) / wh.xyxy();
		vs_cb.Texture_Scale_Offset.x = textureScale.x;
		vs_cb.Texture_Scale_Offset.y = textureScale.y;

		ps_cb.WH = wh;
		ps_cb.HalfTexel = GSVector4(-0.5f, 0.5f).xxyy() / wh.zwzw();

		PSSamplerSelector selector{};
		selector.tau = (wms != CLAMP_CLAMP);
		selector.tav = (wmt != CLAMP_CLAMP);
		selector.ltf = bilinear;

		return selector;
	}
}

void Renderer::BindTexture(SimpleTexture* pNewTexture)
{
	assert(pNewTexture);
	Native::BindTexture(pNewTexture);
}

const Renderer::InUseTextureList& Renderer::GetInUseTextures()
{
	static const InUseTextureList empty;
	return empty;
}

void Renderer::Draw() {
}

void Renderer::Draw(PS2::DrawBufferBase& drawBuffer) {
	Draw(drawBuffer, nullptr, PS2::GetGSState(), true);
}

void Renderer::Draw(PS2::DrawBufferBase& drawBuffer, SimpleTexture* pBoundTexture, PS2::GSState& state, bool bHardware) 
{
	(void)drawBuffer;
	(void)pBoundTexture;
	(void)state;
	(void)bHardware;
}

void PS2::Setup()
{
	CreateDefaultRenderPass();

	PS2_Internal::gVertexBuffers.Init(Renderer::VertexIndexBufferSizeGPU, Renderer::VertexIndexBufferSizeGPU);
	PS2::Hardware::gVertexBuffers.Init(Renderer::VertexIndexBufferSizeGPU, Renderer::VertexIndexBufferSizeGPU);

	PS2_Internal::gVertexConstBuffer.Init();
	PS2_Internal::gPixelConstBuffer.Init();
}

void PS2::BeginFrame()
{
	PS2_Internal::gVertexBuffers.Reset();
	PS2::Hardware::gVertexBuffers.Reset();
}

void PS2::Cleanup()
{
	PS2_Internal::gVertexBuffers.DestroyResources();
	PS2::Hardware::gVertexBuffers.DestroyResources();
	PS2_Internal::gVertexConstBuffer.DestroyResources();
	PS2_Internal::gPixelConstBuffer.DestroyResources();

	for (auto& pipeline : PS2::GetPipelines()) {
		pipeline.second.Destroy();
	}
	PS2::GetPipelines().clear();
	
	//vkDestroyRenderPass(GetDevice(), PS2_Internal::renderPassPs2, nullptr);
}
