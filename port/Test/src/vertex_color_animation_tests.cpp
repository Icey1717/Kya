#include <gtest/gtest.h>

#include "VertexColorAnimation.h"
#include "Mesh.h"
#include "ed3D.h"
#include "ed3D/ed3DG3D.h"
#include "renderer.h"
#include "port.h"
#include "port/vu1_emu.h"

#include <array>
#include <bit>
#include <limits>

TEST(VertexColorAnimation, SelectsAdjacentBasesAndHoldsTheEnd)
{
	const auto frames = Renderer::SelectVertexColorFrames(1.25f, 3);
	EXPECT_EQ(frames.first, 1u);
	EXPECT_EQ(frames.second, 2u);
	EXPECT_FLOAT_EQ(frames.fraction, 0.25f);
	for (const float frame : { 2.0f, 2.75f, 3.0f, 100.0f }) {
		const auto end = Renderer::SelectVertexColorFrames(frame, 3);
		EXPECT_EQ(end.first, 2u);
		EXPECT_EQ(end.second, 2u);
	}
	EXPECT_EQ(Renderer::SelectVertexColorFrames(0.0f, 3).first, 0u);
	EXPECT_EQ(Renderer::SelectVertexColorFrames(1.0f, 3).first, 1u);
	EXPECT_EQ(Renderer::SelectVertexColorFrames(1.0f, 1).second, 0u);
	EXPECT_EQ(Renderer::SelectVertexColorFrames(-1.0f, 3).first, 0u);
	EXPECT_EQ(Renderer::SelectVertexColorFrames(std::numeric_limits<float>::quiet_NaN(), 3).first, 0u);
}

TEST(VertexColorAnimation, InterpolatesAllChannelsInGsUnits)
{
	constexpr uint32_t first = 0x000080ff;
	constexpr uint32_t second = 0x80ff8000;
	EXPECT_EQ(Renderer::InterpolateVertexColor(first, second, 0.0f), first);
	EXPECT_EQ(Renderer::InterpolateVertexColor(first, second, 1.0f), second);
	EXPECT_EQ(Renderer::InterpolateVertexColor(first, second, 0.5f), 0x407f807fu);
	EXPECT_EQ(Renderer::InterpolateVertexColor(first, second, 0.25f), 0x203f80bfu);
}

TEST(VertexColorAnimation, ExistingControllerProvidesLoopClampAndReverseFrames)
{
	const int savedTime = gCurTime;
	const int savedFrame = gCurFrame;
	// Two keys: frame 0 at time 0, frame 2 at time 100.
	ed_g3d_Anim_def animation{};
	animation.field_0x0 = 2;
	animation.field_0x4 = 4 | 1;
	animation.field_0x14 = 1;
	animation.field_0x20 = 0;
	animation.field_0x24 = 0.0f;
	animation.field_0x30 = 100;
	// Authored key values are integer words; the recovered structure currently
	// labels these two fields as floats, but ed3DManageAnim reads uint keys.
	animation.field_0x34 = std::bit_cast<float>(2u);
	gCurTime = 126;
	gCurFrame = 1;
	ed3DManageAnim(&animation);
	EXPECT_FLOAT_EQ(animation.field_0x10, 0.5f);
	EXPECT_EQ(Renderer::SelectVertexColorFrames(animation.field_0x10, 3).first, 0u);
	animation.field_0x4 = 4; // Non-looping forward playback holds the final base.
	gCurFrame = 2;
	ed3DManageAnim(&animation);
	EXPECT_FLOAT_EQ(animation.field_0x10, 2.0f);
	animation.field_0x4 = 8 | 1;
	gCurFrame = 3;
	ed3DManageAnim(&animation);
	EXPECT_FLOAT_EQ(animation.field_0x10, 1.5f);
	gCurTime = savedTime;
	gCurFrame = savedFrame;
}

TEST(VertexColorAnimation, PreservesPaddedSectionColorIndicesThroughCompactionAndLayers)
{
	struct StripPackets {
		ed_3d_strip strip{};
		edpkt_data packets[6]{};
	} data;
	std::array<uint32_t, 4 + 2 * 76> colors{};
	colors[0] = 19; // Color-base stride in quadwords, including section padding.
	colors[2] = 2;
	for (uint32_t i = 0; i < 76; ++i) colors[4 + i] = 0x80000000 | i;
	std::array<uint32_t, 2 * (4 + 76)> st{};
	st[1] = 20; // Each layer includes its header and padded ST payload.
	for (uint32_t layer = 0; layer < 2; ++layer) {
		for (uint32_t i = 0; i < 76; ++i) {
			const uint32_t s = 100 + layer * 1000 + i;
			const uint32_t t = 200 + layer * 1000 + i;
			st[layer * 80 + 4 + i] = s | (t << 16);
		}
	}
	std::array<Renderer::GSVertexUnprocessed::Vertex, 73> vertices{};
	for (size_t i = 0; i < vertices.size(); ++i) {
		vertices[i].fXYZ[0] = static_cast<float>(i);
		vertices[i].fXYZ[1] = static_cast<float>(i * i);
		vertices[i].flags = i < 2 ? 0x8000 : 0;
	}
	const uint64_t prim = Renderer::GS_TRIANGLESTRIP;
	alignas(16) uint64_t gif[2][2] = {
		{ 72 | (prim << 47) | (3ull << 60), 0x512 },
		{ 3 | (prim << 47) | (3ull << 60), 0x512 }
	};
	for (size_t j = 0; j < 2; ++j) {
		data.packets[j * 3 + 1].asU32[3] = 0x6c018000;
		data.packets[j * 3 + 1].asU32[1] = STORE_POINTER(gif[j]);
		data.packets[j * 3 + 2].asU32[0] = gVifEndCode;
	}
	data.strip.flags = 4;
	data.strip.meshCount = 2;
	data.strip.vifListOffset = offsetof(StripPackets, packets);
	data.strip.pColorBuf = STORE_POINTER(colors.data());
	data.strip.pSTBuf = STORE_POINTER(st.data());
	data.strip.pVertexBuf = STORE_POINTER(vertices.data());
	GIFReg::GSPrim gsPrim{};
	gsPrim.PRIM = Renderer::GS_TRIANGLESTRIP;
	Renderer::Kya::G3D::Strip strip;
	strip.pStrip = &data.strip;
	strip.pSimpleMesh = std::make_unique<Renderer::SimpleMesh>("animated colors test", gsPrim, 4);
	strip.PreProcessVertices(0, strip.pSimpleMesh.get());
	auto* pLayer = strip.GetSimpleMesh(1);
	const auto& indices = strip.pSimpleMesh->GetColorSourceIndices();
	ASSERT_FALSE(indices.empty());
	EXPECT_EQ(indices, pLayer->GetColorSourceIndices());
	EXPECT_EQ(pLayer->GetStripFlags(), 4u);
	EXPECT_NE(std::find(indices.begin(), indices.end(), 72u), indices.end());
	for (size_t i = 0; i < indices.size(); ++i) {
		const auto& vertex = strip.pSimpleMesh->GetVertexBufferData().vertex.buff[i];
		EXPECT_EQ(vertex.RGBA[0], indices[i]);
		EXPECT_EQ(vertex.RGBA[3], 0x80u);
		EXPECT_EQ(vertex.STQ.ST[0], 100 + indices[i]);
		EXPECT_EQ(vertex.STQ.ST[1], 200 + indices[i]);
		EXPECT_FLOAT_EQ(vertex.STQ._pad, 0.0f);
		const auto& layerVertex = pLayer->GetVertexBufferData().vertex.buff[i];
		EXPECT_EQ(layerVertex.STQ.ST[0], 1100 + indices[i]);
		EXPECT_EQ(layerVertex.STQ.ST[1], 1200 + indices[i]);
		EXPECT_FLOAT_EQ(layerVertex.STQ._pad, 0.0f);
	}
}
