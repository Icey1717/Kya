#include <gtest/gtest.h>

#include "Texture.h"
#include "renderer.h"
#include "ed3D.h"
#include "port.h"

TEST(MaterialLayers, DecodesEachLayersTextureSamplerAlphaTestAndBlendRegisters)
{
	// Two material passes with deliberately distinct authored state.
	edpkt_data packets[8]{};
	constexpr uint32_t registers[] = { SCE_GS_TEX0_1, SCE_GS_CLAMP_1, SCE_GS_TEST_1, SCE_GS_ALPHA_1 };
	for (size_t layer = 0; layer < 2; ++layer) {
		for (size_t i = 0; i < 4; ++i) packets[layer * 4 + i].asU32[2] = registers[i];
	}
	GIFReg::GSTex firstTex{};
	firstTex.TW = 8;
	firstTex.TH = 8;
	firstTex.CBP = 0x40;
	firstTex.TFX = 0;
	GIFReg::GSTex secondTex{};
	secondTex.TW = 6;
	secondTex.TH = 6;
	secondTex.CBP = 0x80;
	secondTex.TFX = 1;
	packets[0].cmdA = firstTex.CMD;
	packets[4].cmdA = secondTex.CMD;
	GIFReg::GSClamp clamp{};
	clamp.WMS = 1;
	clamp.WMT = 1;
	packets[5].cmdA = clamp.CMD;
	GIFReg::GSTest test{};
	test.ATE = 1;
	test.ATST = 6; // GS GREATER alpha comparison.
	test.AREF = 0x40;
	packets[6].cmdA = test.CMD;
	GIFReg::GSAlpha alpha{};
	alpha.A = 0;
	alpha.B = 1;
	alpha.C = 2;
	alpha.D = 1;
	alpha.FIX = 0x60;
	packets[7].cmdA = alpha.CMD;
	Renderer::CombinedImageData first;
	Renderer::CombinedImageData second;
	const Renderer::Kya::G2D::CommandList commands{ packets, 4 };
	Renderer::Kya::ProcessRenderCommandList(first, commands, 0);
	Renderer::Kya::ProcessRenderCommandList(second, commands, 1);
	EXPECT_EQ(first.registers.tex.CMD, firstTex.CMD);
	EXPECT_EQ(second.registers.tex.CMD, secondTex.CMD);
	EXPECT_EQ(first.registers.clamp.CMD, 0u);
	EXPECT_EQ(second.registers.clamp.CMD, clamp.CMD);
	EXPECT_EQ(first.registers.test.CMD, 0u);
	EXPECT_EQ(second.registers.test.CMD, test.CMD);
	EXPECT_EQ(first.registers.alpha.CMD, 0u);
	EXPECT_EQ(second.registers.alpha.CMD, alpha.CMD);
}
