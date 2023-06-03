#include "VulkanPS2.h"
#include "VulkanReflect.h"
#include "renderer.h"
#include "VulkanRenderer.h"
#include <stdexcept>
#include <array>
#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include <unordered_map>
#include "MD5.h"
#include "FrameBuffer.h"
#include "VulkanImage.h"
#include "GSState.h"
#include "Pipeline.h"
#include "GSVector.h"
#include "UniformBuffer.h"
#include "TextureCache.h"

namespace PS2_Internal {

	PS2::GSState state;

	PS2::GSState& GetGSState() {
		return state;
	}

	void createDescriptorSets() {
		for (auto& pipeline : PS2::GetPipelines()) {
			std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline.second.descriptorSetLayouts[0]);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = pipeline.second.descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
			allocInfo.pSetLayouts = layouts.data();

			pipeline.second.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(GetDevice(), &allocInfo, pipeline.second.descriptorSets.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				VkDescriptorBufferInfo vertexConstantBuffer{};
				vertexConstantBuffer.buffer = PS2::GetVertexConstantUniformBuffer(i);
				vertexConstantBuffer.offset = 0;
				vertexConstantBuffer.range = sizeof(PS2::VSConstantBuffer);

				std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = pipeline.second.descriptorSets[i];
				descriptorWrites[0].dstBinding = 5;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &vertexConstantBuffer;

				VkDescriptorBufferInfo pixelConstantBuffer{};
				pixelConstantBuffer.buffer = PS2::GetPixelConstantUniformBuffer(i);
				pixelConstantBuffer.offset = 0;
				pixelConstantBuffer.range = sizeof(PS2::PSConstantBuffer);

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = pipeline.second.descriptorSets[i];
				descriptorWrites[1].dstBinding = 6;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pBufferInfo = &pixelConstantBuffer;

				vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}
	}

	void createDescriptorPool() {
		for (auto& pipeline : PS2::GetPipelines()) {
			auto& descriptorSetLayoutBindings = pipeline.second.descriptorSetLayoutBindings[0];
			// Create descriptor pool based on the descriptor set count from the shader
			std::vector<VkDescriptorPoolSize> poolSizes;
			for (uint32_t i = 0; i < descriptorSetLayoutBindings.size(); ++i) {
				auto& descriptorSet = descriptorSetLayoutBindings[i];
				poolSizes.push_back({ descriptorSet.descriptorType, descriptorSet.descriptorCount * MAX_FRAMES_IN_FLIGHT });
			}

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

			if (vkCreateDescriptorPool(GetDevice(), &poolInfo, nullptr, &pipeline.second.descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}
	}

	Renderer::GSVertex MakeVertex(uint16_t x, uint16_t y, uint32_t z) {
		return Renderer::GSVertex{ { GetGSState().S, GetGSState().T }, {GetGSState().R, GetGSState().G, GetGSState().B, GetGSState().A }, GetGSState().Q, { x, y }, z, 0, 0 };
	}
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

	struct
	{
		Renderer::GSVertex* buff;
		size_t head, tail, next, maxcount; // head: first vertex, tail: last vertex + 1, next: last indexed + 1
		size_t xy_tail;
		uint64_t xy[4];
	} m_vertex;

	struct
	{
		uint16_t* buff;
		size_t tail;
	} m_index;

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

	enum GS_PRIM_CLASS
	{
		GS_POINT_CLASS = 0,
		GS_LINE_CLASS = 1,
		GS_TRIANGLE_CLASS = 2,
		GS_SPRITE_CLASS = 3,
		GS_INVALID_CLASS = 7,
	};

	uint32_t OFX = 0;
	uint32_t OFY = 0;

	uint32_t Skip = 0;

	void SetVertexSkip(uint32_t inSkip)
	{
		Skip = inSkip;
	}

	void SetXY(uint32_t x, uint32_t y)
	{
		OFX = x;
		OFY = y;
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

	void SetTest(uint32_t ate, uint32_t atst, uint32_t aref, uint32_t afail, uint32_t date, uint32_t datm, uint32_t zte, uint32_t ztst) {
		PS2::GSState::GSTest NewTest = { ate, atst, aref, afail, date, datm, zte, ztst };

		if (PS2::GetGSState().TEST != NewTest) {
			Draw();
		}

		PS2::GetGSState().TEST = NewTest;
	}

	void SetPrim(uint32_t prim, uint32_t iip, uint32_t tme, uint32_t fge, uint32_t abe, uint32_t aa1, uint32_t fst, uint32_t ctxt, uint32_t fix) {
		PS2::GetGSState().PRIM = { prim, iip, tme, fge, abe, aa1, fst, ctxt, fix };
	}

	void SetTEX(uint32_t tbp, uint32_t tbw, uint32_t psm, uint32_t tw, uint32_t th, uint32_t tcc, uint32_t tfx, uint32_t cbp, uint32_t cpsm, uint32_t csm, uint32_t csa, uint32_t cld)
	{
		PS2::GetGSState().TEX = { tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld };
	}

	void KickVertex(uint16_t x, uint16_t y, uint32_t z)
	{
		uint32_t skip = Skip;
		GS_PRIM prim = (GS_PRIM)PS2::GetGSState().PRIM.PRIM;
		ASSERT(m_vertex.tail < m_vertex.maxcount + 3);

		size_t head = m_vertex.head;
		size_t tail = m_vertex.tail;
		size_t next = m_vertex.next;
		size_t xy_tail = m_vertex.xy_tail;

		// callers should write XYZUVF to m_v.m[1] in one piece to have this load store-forwarded, either by the CPU or the compiler when this function is in lined
		m_vertex.buff[m_vertex.tail] = PS2_Internal::MakeVertex(x, y, z);

		m_vertex.tail = ++tail;
		m_vertex.xy_tail = ++xy_tail;

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
			GSVector4i v0, v1, v2, v3, pmin, pmax;
		
			v0 = GSVector4i::loadl(&m_vertex.xy[(xy_tail + 1) & 3]); // T-3
			v1 = GSVector4i::loadl(&m_vertex.xy[(xy_tail + 2) & 3]); // T-2
			v2 = GSVector4i::loadl(&m_vertex.xy[(xy_tail + 3) & 3]); // T-1
			v3 = GSVector4i::loadl(&m_vertex.xy[(xy_tail - m) & 3]); // H
		
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
			
			//skip |= test.mask() & 15;
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
				m_vertex.tail = head; // no need to check or grow the buffer length
				break;
			case GS_LINESTRIP:
			case GS_TRIANGLESTRIP:
				m_vertex.head = head + 1;
				// fall through
			case GS_TRIANGLEFAN:
				//if (tail >= m_vertex.maxcount) GrowVertexBuffer(); // in case too many vertices were skipped
				break;
			default:
				__assume(0);
			}
		
			return;
		}

		//if (tail >= m_vertex.maxcount) GrowVertexBuffer();

		uint16_t* RESTRICT buff = &m_index.buff[m_index.tail];

		switch (prim)
		{
		case GS_POINTLIST:
			buff[0] = head + 0;
			m_vertex.head = head + 1;
			m_vertex.next = head + 1;
			m_index.tail += 1;
			break;
		case GS_LINELIST:
			buff[0] = head + 0;
			buff[1] = head + 1;
			m_vertex.head = head + 2;
			m_vertex.next = head + 2;
			m_index.tail += 2;
			break;
		case GS_LINESTRIP:
			if (next < head)
			{
				m_vertex.buff[next + 0] = m_vertex.buff[head + 0];
				m_vertex.buff[next + 1] = m_vertex.buff[head + 1];
				head = next;
				m_vertex.tail = next + 2;
			}
			buff[0] = head + 0;
			buff[1] = head + 1;
			m_vertex.head = head + 1;
			m_vertex.next = head + 2;
			m_index.tail += 2;
			break;
		case GS_TRIANGLELIST:
			buff[0] = head + 0;
			buff[1] = head + 1;
			buff[2] = head + 2;
			m_vertex.head = head + 3;
			m_vertex.next = head + 3;
			m_index.tail += 3;
			break;
		case GS_TRIANGLESTRIP:
			if (next < head)
			{
				m_vertex.buff[next + 0] = m_vertex.buff[head + 0];
				m_vertex.buff[next + 1] = m_vertex.buff[head + 1];
				m_vertex.buff[next + 2] = m_vertex.buff[head + 2];
				head = next;
				m_vertex.tail = next + 3;
			}
			buff[0] = head + 0;
			buff[1] = head + 1;
			buff[2] = head + 2;
			m_vertex.head = head + 1;
			m_vertex.next = head + 3;
			m_index.tail += 3;
			break;
		case GS_TRIANGLEFAN:
			// TODO: remove gaps, next == head && head < tail - 3 || next > head && next < tail - 2 (very rare)
			buff[0] = head + 0;
			buff[1] = tail - 2;
			buff[2] = tail - 1;
			m_vertex.next = tail;
			m_index.tail += 3;
			break;
		case GS_SPRITE:
			buff[0] = head + 0;
			buff[1] = head + 1;
			m_vertex.head = head + 2;
			m_vertex.next = head + 2;
			m_index.tail += 2;
			break;
		case GS_INVALID:
			m_vertex.tail = head;
			break;
		default:
			__assume(0);
		}
	}

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

struct PSSelector
{
	union
	{
		struct
		{
			// *** Word 1
			// Format
			uint32_t fmt : 4;
			uint32_t dfmt : 2;
			uint32_t depth_fmt : 2;
			// Alpha extension/Correction
			uint32_t aem : 1;
			uint32_t fba : 1;
			// Fog
			uint32_t fog : 1;
			// Pixel test
			uint32_t atst : 3;
			// Color sampling
			uint32_t fst : 1;
			uint32_t tfx : 3;
			uint32_t tcc : 1;
			uint32_t wms : 2;
			uint32_t wmt : 2;
			uint32_t ltf : 1;
			// Shuffle and fbmask effect
			uint32_t shuffle : 1;
			uint32_t read_ba : 1;
			uint32_t fbmask : 1;

			// Blend and Colclip
			uint32_t blend_a : 2;
			uint32_t blend_b : 2;
			uint32_t blend_c : 2;
			uint32_t blend_d : 2;
			uint32_t clr1 : 1;
			uint32_t hdr : 1;
			uint32_t colclip : 1;

			// Others ways to fetch the texture
			uint32_t channel : 3;

			// Hack
			uint32_t tcoffsethack : 1;
			uint32_t urban_chaos_hle : 1;
			uint32_t tales_of_abyss_hle : 1;
			uint32_t point_sampler : 1;
			uint32_t invalid_tex0 : 1; // Lupin the 3rd

			uint32_t _free : 18;
		};

		uint64_t key;
	};

	operator uint64_t() { return key; }

	PSSelector() : key(0) {}
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

void Renderer::Draw() {
	if (Renderer::m_index.tail == 0) {
		return;
	}

	g_GSSelector.ResetStates();

	const PS2::GSState& state = PS2::GetGSState();

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
	g_GSSelector.prim = GetPrimClass(state.PRIM.PRIM);

	auto& frameBuffer = FrameBuffer::Get(state.FBP);


	const std::string CONFIG_NAME = "GS_PRIM_" + std::to_string(g_GSSelector.prim)
		+ "_POINT_" + std::to_string(g_GSSelector.point)
		+ "_LINE_" + std::to_string(g_GSSelector.line)
		+ "_IIP_" + std::to_string(g_GSSelector.iip);

	const std::string hash = GetMD5String(CONFIG_NAME);

	const PS2::PipelineKey key = { hash, topology };
	auto pipelineItr = PS2::GetPipelines().find(key);
	assert(pipelineItr != PS2::GetPipelines().end());
	auto pipeline = (*pipelineItr).second;

	PS2::GSTexEntry& tex = PS2::GetTextureCache().Lookup(state.TEX, pipeline.descriptorSetLayouts, pipeline.descriptorSetLayoutBindings);

	if (!hwState.bActivePass || hwState.FBP != state.FBP) {

		if (hwState.bActivePass) {
			vkCmdEndRenderPass(GetCurrentCommandBuffer());
		}

		hwState.FBP = state.FBP;
		hwState.bActivePass = true;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = frameBuffer.renderPass;
		renderPassInfo.framebuffer = frameBuffer.framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { (unsigned int)GetRTSize().x, (unsigned int)GetRTSize().y };

		VkClearValue clearColors[] = {{{0.0f, 0.0f, 0.0f, 1.0f}}, {{0.0f, 0.0f, 0.0f, 1.0f}} };
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

		float sx = 2.0f * GetRTScale().x / (GetRTSize().x << 4);
		float sy = 2.0f * GetRTScale().y / (GetRTSize().y << 4);
		float ox = Renderer::OFX;
		float oy = Renderer::OFY;
		float ox2 = -1.0f / GetRTSize().x;
		float oy2 = -1.0f / GetRTSize().y;

		PS2::GetVertexConstantBufferData().Texture_Scale_Offset = GSVector4(0.0f);
		PS2::GetVertexConstantBufferData().VertexScale = GSVector4(sx, -sy, ldexpf(1, -32), 0.0f);
		PS2::GetVertexConstantBufferData().VertexOffset = GSVector4(ox * sx + ox2 + 1, -(oy * sy + oy2 + 1), 0.0f, -1.0f);

		PS2::UpdateUniformBuffers();
	}

	vkCmdSetScissor(GetCurrentCommandBuffer(), 0, 1, &hwState.scissor);

	vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

	const VkDeviceSize vertexOffset = PS2::MapVertices(Renderer::m_vertex.buff, Renderer::m_vertex.tail);
	const VkDeviceSize indexOffset = PS2::MapIndices(Renderer::m_index.buff, Renderer::m_index.tail);

	VkBuffer vertexBuffers[] = { PS2::GetVertexBuffer() };
	VkDeviceSize offsets[] = { vertexOffset };
	vkCmdBindVertexBuffers(GetCurrentCommandBuffer(), 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(GetCurrentCommandBuffer(), PS2::GetIndexBuffer(), indexOffset, VK_INDEX_TYPE_UINT16);

	vkCmdBindDescriptorSets(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &tex.value.descriptorSets[GetCurrentFrame()], 0, nullptr);

	vkCmdDrawIndexed(GetCurrentCommandBuffer(), static_cast<uint32_t>(Renderer::m_index.tail), 1, 0, 0, 0);

	Renderer::m_index.tail = 0;
	Renderer::m_vertex.head = 0;
	Renderer::m_vertex.tail = 0;
	Renderer::m_vertex.next = 0;
	Renderer::m_vertex.xy_tail = 0;
}

void PS2::Setup()
{
	CreateDefaultRenderPass();
	CreateGraphicsPipelines();
	PS2_Internal::createDescriptorPool();
	CreateUniformBuffers();
	PS2_Internal::createDescriptorSets();

	Renderer::m_vertex.buff = (Renderer::GSVertex*)_aligned_malloc(sizeof(Renderer::GSVertex) * 0x1000, 32);
	Renderer::m_index.buff = (uint16_t*)_aligned_malloc(sizeof(uint16_t) * 0x1000, 32);
}

void PS2::BeginFrame()
{
	PS2::ResetBuffer();
}

void PS2::Cleanup()
{
	for (auto& pipeline : PS2::GetPipelines()) {
		vkDestroyPipeline(GetDevice(), pipeline.second.pipeline, nullptr);
		vkDestroyPipelineLayout(GetDevice(), pipeline.second.layout, nullptr);
	}
	
	//vkDestroyRenderPass(GetDevice(), PS2_Internal::renderPassPs2, nullptr);
}