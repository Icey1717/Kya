#include <gtest/gtest.h>

#include "Texture.h"
#include "TextureSampling.h"
#include "renderer.h"
#include "port.h"
#include "ed3D.h"
#include "../../../src/port/pointer_conv.h"
#include "../../Windows/Renderer/Vulkan/src/VulkanPS2.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include <array>

TEST(TextureMipmaps, RecoversGsQScaleFromProjection)
{
	std::array<float, 16> native{};
	std::array<float, 16> gs{};
	native[11] = -1.0f;
	gs[11] = -1.0f / 0.03f;
	EXPECT_FLOAT_EQ(Renderer::TextureSampling::GetGsQScale(native.data(), gs.data()), 0.03f);
	EXPECT_FLOAT_EQ(Renderer::TextureSampling::GetGsQScale(native.data(), nullptr), 1.0f);
	native[11] = gs[11] = 0.0f;
	native[15] = 1.0f;
	gs[15] = 4.0f;
	EXPECT_FLOAT_EQ(Renderer::TextureSampling::GetGsQScale(native.data(), gs.data()), 0.25f);
}

TEST(TextureMipmaps, PreservesExtraAuthoredLevelsAndSeparatesPalette)
{
	std::array<edpkt_data, 22> packets{};
	std::array<std::array<uint32_t, 16>, 4> payloads{};
	for (uint32_t level = 0; level < 4; ++level) {
		auto* transfer = &packets[level * 5];
		transfer[0].asU32[2] = SCE_GIF_PACKED_AD;
		GIFReg::GSBitBltBuf blt{};
		blt.DBP = 0x100 + level * 0x40;
		blt.DBW = 1;
		transfer[1].cmdA = blt.CMD;
		transfer[1].cmdB = SCE_GS_BITBLTBUF;
		transfer[2].cmdB = SCE_GS_TRXPOS;
		GIFReg::GSTrxReg region{};
		region.RRW = level == 3 ? 16 : 8 >> level;
		region.RRH = level == 3 ? 1 : 4 >> level;
		transfer[3].cmdA = region.CMD;
		transfer[3].cmdB = SCE_GS_TRXREG;
		transfer[4].asU32[0] = 0x30000000;
		transfer[4].asU32[1] = STORE_POINTER(payloads[level].data());
	}
	packets[20].asU32[2] = SCE_GIF_PACKED_AD;
	packets[21].asU32[2] = SCE_GS_TEXFLUSH;
	Renderer::CombinedImageData image{};
	image.bitmaps.resize(2); // Header under-reports the third authored level.
	image.bitmaps[0].canvasWidth = 8;
	image.bitmaps[0].canvasHeight = 4;
	image.palette.canvasWidth = 16;
	image.palette.canvasHeight = 1;
	Renderer::Kya::ProcessUploadCommandList(image, { packets.data(), int(packets.size()) });
	ASSERT_EQ(image.bitmaps.size(), 3u);
	for (uint32_t level = 0; level < 3; ++level) {
		EXPECT_EQ(image.bitmaps[level].pImage, payloads[level].data());
		EXPECT_EQ(image.bitmaps[level].canvasWidth, 8u >> level);
		EXPECT_EQ(image.bitmaps[level].canvasHeight, 4u >> level);
		EXPECT_EQ(image.bitmaps[level].bitBltBuf.DBP, 0x100u + level * 0x40);
	}
	EXPECT_EQ(image.palette.pImage, payloads[3].data());
	EXPECT_EQ(image.palette.canvasWidth, 16u);

	// A texture without a palette must retain its final transfer as a mip.
	image = {};
	image.bitmaps.resize(1);
	image.bitmaps[0].canvasWidth = 8;
	image.bitmaps[0].canvasHeight = 4;
	Renderer::Kya::ProcessUploadCommandList(image, { packets.data(), 15 });
	ASSERT_EQ(image.bitmaps.size(), 3u);
	EXPECT_EQ(image.bitmaps.back().pImage, payloads[2].data());
	EXPECT_EQ(image.palette.pImage, nullptr);
	for (uint32_t level = 0; level < 4; ++level) RELEASE_POINTER(packets[level * 5 + 4].asU32[1]);
}

TEST(TextureMipmaps, IndependentMinificationMagnificationAndMipFilters)
{
	GIFReg::GSClamp clamp{};
	clamp.WMS = 1;
	clamp.WMT = 0;
	constexpr bool linearMin[] = { false, true, false, false, true, true };
	constexpr bool linearMip[] = { false, false, false, true, false, true };
	for (uint32_t filter = 0; filter < 6; ++filter) {
		for (uint32_t mag = 0; mag < 2; ++mag) {
			GIFReg::GSTex1 tex1{};
			tex1.CMD = SCE_GS_PACK_TEX1(0, 6, mag, filter, 0, 0, 0);
			const auto selector = PS2::GetTextureSamplerSelector(clamp, tex1, 4);
			EXPECT_EQ(selector.ltf, !linearMin[filter]);
			EXPECT_EQ(selector.magPoint, !mag);
			EXPECT_EQ(selector.mipLinear, linearMip[filter]);
			EXPECT_EQ(selector.maxLevel, filter < 2 ? 0u : 3u);
			EXPECT_EQ(selector.tau, 0u);
			EXPECT_EQ(selector.tav, 1u);
			EXPECT_EQ(PS2::GetTextureSamplerSelector(clamp, tex1, 1).maxLevel, 0u);
		}
	}
}

TEST(TextureMipmaps, SignedBiasFixedLodAndAlphaPacking)
{
	for (int32_t bias : { -2048, -255, -1, 0, 24, 2047 }) {
		GIFReg::GSTex1 tex1{};
		tex1.CMD = SCE_GS_PACK_TEX1(1, 3, 1, 5, 0, 2, uint32_t(bias));
		const auto settings = Renderer::TextureSampling::Decode(tex1);
		EXPECT_TRUE(settings.fixedLod);
		EXPECT_EQ(settings.lodBias, bias);
		EXPECT_EQ(settings.lodScale, 2u);
		Renderer::Native::PerDrawData data{};
		data.globalAlpha = 0x45;
		data.textureLodBias = settings.lodBias;
		data.textureLodScale = settings.lodScale;
		data.textureFixedLod = settings.fixedLod;
		data.textureMaxMipLevel = settings.maxLevel;
		data.textureLodEnable = 1;
		uint32_t packed;
		// Bitfields cannot be addressed directly. Verify the word at the GLSL
		// globalAlpha offset, including signed bias and the following field.
		memcpy(&packed, reinterpret_cast<const uint8_t*>(&data) + 100, sizeof(packed));
		EXPECT_EQ(packed & 0xffu, 0x45u);
		EXPECT_EQ((packed >> 8) & 0xfffu, uint32_t(bias) & 0xfffu);
		EXPECT_EQ((packed >> 20) & 3u, 2u);
		EXPECT_EQ((packed >> 23) & 7u, 3u);
		EXPECT_NE(packed & 0x80000000u, 0u);
		EXPECT_EQ(int32_t(data.textureLodBias), bias);
		data.globalAlpha = 0x21;
		uint32_t updated;
		memcpy(&updated, reinterpret_cast<const uint8_t*>(&data) + 100, sizeof(updated));
		EXPECT_EQ(updated, (packed & ~0xffu) | 0x21u);
	}
}

TEST(TextureMipmaps, DecodesPerLayerRegistersAndWindowsTex1Bridge)
{
	// MIPTBP's middle address straddles the 32-bit boundary.
	GIFReg::GSMipTbp1 mipTbp1{};
	mipTbp1.TBP1 = 0x1234;
	mipTbp1.TBW1 = 0x15;
	mipTbp1.TBP2 = 0x2345;
	mipTbp1.TBW2 = 0x26;
	mipTbp1.TBP3 = 0x3456;
	mipTbp1.TBW3 = 0x37;
	const uint64_t packedMips = 0x1234ull | (0x15ull << 14) | (0x2345ull << 20)
		| (0x26ull << 34) | (0x3456ull << 40) | (0x37ull << 54);
	EXPECT_EQ(mipTbp1.CMD, packedMips);
	GIFReg::GSMipTbp2 mipTbp2{};
	mipTbp2.CMD = packedMips;
	EXPECT_EQ(uint32_t(mipTbp2.TBP4), 0x1234u);
	EXPECT_EQ(uint32_t(mipTbp2.TBW4), 0x15u);
	EXPECT_EQ(uint32_t(mipTbp2.TBP5), 0x2345u);
	EXPECT_EQ(uint32_t(mipTbp2.TBW5), 0x26u);
	EXPECT_EQ(uint32_t(mipTbp2.TBP6), 0x3456u);
	EXPECT_EQ(uint32_t(mipTbp2.TBW6), 0x37u);
	edpkt_data packets[8]{};
	const uint32_t regs[] = { SCE_GS_TEX0_1, SCE_GS_TEX1_1, SCE_GS_MIPTBP1_1, SCE_GS_MIPTBP2_1 };
	for (uint32_t layer = 0; layer < 2; ++layer) {
		for (uint32_t reg = 0; reg < 4; ++reg) {
			packets[layer * 4 + reg].asU32[2] = regs[reg];
			packets[layer * 4 + reg].cmdA = 0x1000 * layer + reg;
		}
		Renderer::CombinedImageData image{};
		Renderer::Kya::ProcessRenderCommandList(image, { packets, 4 }, layer);
		EXPECT_EQ(image.registers.tex1.CMD, 0x1000u * layer + 1);
		EXPECT_EQ(image.registers.mipTbp1.CMD, 0x1000u * layer + 2);
		EXPECT_EQ(image.registers.mipTbp2.CMD, 0x1000u * layer + 3);
	}
	const auto savedState = PS2::GetGSState();
	const auto first = SCE_GS_SET_TEX1(0x20, 3, 0, 5, 0, 2, 0xffffff01);
	EXPECT_EQ(PS2::GetGSState().TEX1.CMD, first);
	EXPECT_TRUE(PS2::GetGSState().tex1Set);
	const auto snapshot = PS2::GetGSState().TEX1;
	GIFReg::GSTex1 next{};
	next.CMD = SCE_GS_PACK_TEX1(0, 0, 1, 1, 0, 0, 0);
	Renderer::SetTex1(next);
	EXPECT_EQ(snapshot.CMD, first);
	PS2::GetGSState() = savedState;
}
