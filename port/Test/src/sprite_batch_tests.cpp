#include <gtest/gtest.h>

#include "Sprite.h"
#include "ed3D.h"
#include "renderer.h"
#include "port.h"

#include <array>

namespace
{
	class SpriteBatches : public testing::Test
	{
	protected:
		struct SpritePackets {
			ed_3d_sprite sprite{};
			edpkt_data packets[12]{};
		} data;
		std::array<edF32VECTOR4, 152> centers{};
		std::array<uint32_t, 152> colors{};
		std::array<uint32_t, 152> st{};
		std::array<uint32_t, 44> sizes{};
		alignas(16) uint64_t gif[3][2]{};
		edF32VECTOR4 savedX, savedY;
		uint32_t savedFlags;

		void SetUp() override
		{
			savedX = gCamNormal_X;
			savedY = gCamNormal_Y;
			savedFlags = g_stSpriteWidthHeightHeader[1].asU32[3];
			gCamNormal_X = { 1, 0, 0, 0 };
			gCamNormal_Y = { 0, 1, 0, 0 };
			g_stSpriteWidthHeightHeader[1].asU32[3] = 0xc000;
			data.sprite.offsetA = offsetof(SpritePackets, packets);
			data.sprite.nbBatches = 3;
			data.sprite.nbRemainderRects = 2;
			data.sprite.nbRemainderVertices = 8;
			data.sprite.pVertexBuf = STORE_POINTER(centers.data());
			data.sprite.pColorBuf = STORE_POINTER(colors.data());
			data.sprite.pSTBuf = STORE_POINTER(st.data());
			data.sprite.pWHBuf = STORE_POINTER(sizes.data());
			for (size_t batch = 0; batch < 3; ++batch) {
				const uint64_t count = batch < 2 ? 72 : 8;
				gif[batch][0] = count | (uint64_t(Renderer::GS_TRIANGLESTRIP) << 47) | (3ull << 60);
				gif[batch][1] = 0x512;
				data.packets[batch * 4 + 1].asU32[1] = STORE_POINTER(gif[batch]);
				data.packets[batch * 4 + 1].asU32[3] = 0x6c018000;
				data.packets[batch * 4 + 2].asU32[3] = 0x14000000;
				data.packets[batch * 4 + 3].asU32[0] = gVifEndCode;
			}
			for (size_t vertex = 0; vertex < centers.size(); ++vertex) {
				centers[vertex] = { static_cast<float>((vertex / 4) * 10), 0, 0, 1 };
				colors[vertex] = 0x80000000 | static_cast<uint32_t>(vertex);
				st[vertex] = static_cast<uint32_t>(vertex);
			}
			// Poison the alignment padding between blocks of 18 quads.
			sizes.fill(0x7fff7fff);
			for (size_t quad = 0; quad < 38; ++quad) {
				const auto width = static_cast<uint32_t>(1024 + quad * 16);
				const auto height = static_cast<uint32_t>(512 + quad * 8);
				sizes[(quad / 18) * 20 + quad % 18] = width | (height << 16);
			}
		}

		void TearDown() override
		{
			gCamNormal_X = savedX;
			gCamNormal_Y = savedY;
			g_stSpriteWidthHeightHeader[1].asU32[3] = savedFlags;
		}

		void CheckGeometry(bool sharedSize)
		{
			GIFReg::GSPrim prim{};
			prim.PRIM = Renderer::GS_TRIANGLESTRIP;
			Renderer::SimpleMesh mesh("sprite batches test", prim, 0);
			mesh.GetVertexBufferData().Init(156, 228);
			Renderer::Kya::Sprite::ProcessVertices(&data.sprite, &mesh);
			const auto& buffer = mesh.GetVertexBufferData();
			ASSERT_EQ(buffer.GetVertexTail(), 152u);
			ASSERT_EQ(buffer.GetIndexTail(), 228u); // 38 quads, exactly once each.
			for (size_t quad = 0; quad < 38; ++quad) {
				const float width = static_cast<float>(1024 + (sharedSize ? 0 : quad) * 16) / 2048.0f;
				const float height = static_cast<float>(512 + (sharedSize ? 0 : quad) * 8) / 2048.0f;
				const float center = static_cast<float>(quad * 10);
				for (size_t corner = 0; corner < 4; ++corner) {
					const auto& vertex = buffer.vertex.buff[quad * 4 + corner];
					EXPECT_FLOAT_EQ(vertex.XYZFlags.fXYZ[0], center + (corner < 2 ? -width : width));
					EXPECT_FLOAT_EQ(vertex.XYZFlags.fXYZ[1], corner % 2 == 0 ? height : -height);
					EXPECT_EQ(vertex.RGBA[0], quad * 4 + corner);
					EXPECT_EQ(vertex.STQ.ST[0], quad * 4 + corner);
					EXPECT_FLOAT_EQ(vertex.STQ._pad, 0.0f);
					EXPECT_EQ(vertex.XYZFlags.flags & 0x7ff, 0u);
				}
			}
		}
	};
}

TEST_F(SpriteBatches, SkipsWidthHeightPaddingAcrossFullAndPartialBatches)
{
	CheckGeometry(false);
}

TEST_F(SpriteBatches, ReusesSharedWidthHeightAcrossAllBatches)
{
	data.sprite.pRenderFrame30 |= 1;
	CheckGeometry(true);
}

TEST_F(SpriteBatches, SingleBatchUsesOnlyItsRemainderQuads)
{
	data.sprite.nbBatches = 1;
	data.sprite.nbRemainderVertices = 8;
	gif[0][0] = 8 | (uint64_t(Renderer::GS_TRIANGLESTRIP) << 47) | (3ull << 60);
	GIFReg::GSPrim prim{};
	prim.PRIM = Renderer::GS_TRIANGLESTRIP;
	Renderer::SimpleMesh mesh("single sprite batch test", prim, 0);
	mesh.GetVertexBufferData().Init(12, 12);
	Renderer::Kya::Sprite::ProcessVertices(&data.sprite, &mesh);
	EXPECT_EQ(mesh.GetVertexBufferData().GetVertexTail(), 8u);
	EXPECT_EQ(mesh.GetVertexBufferData().GetIndexTail(), 12u);
}

TEST_F(SpriteBatches, RotatedNonuniformModelKeepsBillboardsFacingCamera)
{
	// A 90-degree rotation and nonuniform scale must affect centers, not camera axes.
	edF32MATRIX4 model = { 0, 2, 0, 0, -3, 0, 0, 0, 0, 0, 4, 0, 100, 200, 300, 1 };
	GIFReg::GSPrim prim{};
	prim.PRIM = Renderer::GS_TRIANGLESTRIP;
	Renderer::SimpleMesh mesh("transformed sprite test", prim, 0);
	mesh.GetVertexBufferData().Init(156, 228);
	Renderer::Kya::Sprite::ProcessVertices(&data.sprite, &mesh, &model, 4.0f);
	const auto& buffer = mesh.GetVertexBufferData();
	ASSERT_EQ(buffer.GetVertexTail(), 152u);
	for (size_t quad = 0; quad < 38; ++quad) {
		const float width = static_cast<float>(1024 + quad * 16) / 2048.0f * 4;
		const float height = static_cast<float>(512 + quad * 8) / 2048.0f * 4;
		for (size_t corner = 0; corner < 4; ++corner) {
			const auto& vertex = buffer.vertex.buff[quad * 4 + corner];
			EXPECT_FLOAT_EQ(vertex.XYZFlags.fXYZ[0], 100 + (corner < 2 ? -width : width));
			EXPECT_FLOAT_EQ(vertex.XYZFlags.fXYZ[1], 200 + static_cast<float>(quad * 20) + (corner % 2 == 0 ? height : -height));
			EXPECT_FLOAT_EQ(vertex.XYZFlags.fXYZ[2], 300);
		}
	}
}
