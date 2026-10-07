#include "../../Windows/Renderer/Vulkan/src/pcsx2/TextureUpload/src/common/Pcsx2Types.h"
#include <gtest/gtest.h>
#include "../../Windows/Renderer/Vulkan/src/pcsx2/TextureUpload/src/GSLocalMemory.h"
#include <vector>
#include <algorithm>

// Decode the original packet independently of the native shader's offset helper.
TEST(FogPackets, SignedOffsetBlendRegisters)
{
	auto greenBlend = [](uint64_t alpha, int source, int destination) {
		const int colors[3]{ source, destination, 0 };
		const int a = alpha & 3;
		const int b = (alpha >> 2) & 3;
		const int c = (alpha >> 4) & 3;
		const int d = (alpha >> 6) & 3;
		EXPECT_EQ(c, 2); // Fixed blend factor, independent of source/destination alpha.
		const int fix = (alpha >> 32) & 255;
		return std::clamp((colors[a] - colors[b]) * fix / 128 + colors[d], 0, 255);
	};
	EXPECT_EQ(greenBlend(0x8000000068ull, 80, 0), 80);
	EXPECT_EQ(greenBlend(0x8000000068ull, 32, 64), 96);
	EXPECT_EQ(greenBlend(0x8000000062ull, 32, 64), 32);
	EXPECT_EQ(greenBlend(0x8000000068ull, 80, 240), 255);
	EXPECT_EQ(greenBlend(0x8000000062ull, 80, 16), 0);
}

// Independently replay the recovered PSMZ16 -> PSMCT16 channel shuffle using
// the GS address tables. This checks which depth bits actually reach alpha.
TEST(FogPackets, DepthChannelShuffle)
{
	std::vector<uint16_t> halves(2 * 1024 * 1024);
	constexpr uint32_t depthBase = 0x1000;
	for (uint32_t z : { 0u, 0x1234u, 0x4000u, 0x7fffu, 0x8000u, 0xffffu }) {
		std::fill(halves.begin(), halves.end(), 0);
		for (int y = 0; y < 448; ++y) {
			for (int x = 0; x < 512; ++x) {
				halves[GSLocalMemory::PixelAddress16Z(x, y, depthBase, 8)] = z;
				const auto colorAddress = GSLocalMemory::PixelAddress32(x, y, 0, 8) * 2;
				halves[colorAddress] = halves[colorAddress + 1] = 0x8080;
			}
		}
		for (int y = 0; y < 894; ++y) {
			for (int x = 0; x < 512; ++x) {
				if ((x & 15) < 8) continue;
				const auto address = GSLocalMemory::PixelAddress16(x, y, 0, 8);
				const auto previous = halves[address];
				// Expand16To32 followed by WriteFrame16 preserves RGB bits;
				// TEXA.TA0=TA1=0 clears bit 15 when packing alpha back to CT16.
				const auto texel = halves[GSLocalMemory::PixelAddress16Z(x - 8, y, depthBase, 8)] & 0x7fff;
				// FRAME.FBMSK=0x3fff converts to mask 0x7f in CT16 space.
				// See PCSX2 GSRendererSW's gd.sel.fpsm==2 mask conversion.
				halves[address] = (texel & ~0x7fu) | (previous & 0x7fu);
			}
		}
		const auto colorAddress = GSLocalMemory::PixelAddress32(256, 128, 0, 8) * 2;
		const uint32_t pixel = halves[colorAddress] | (uint32_t(halves[colorAddress + 1]) << 16);
		EXPECT_EQ(pixel >> 24, (z >> 8) & 127u) << std::hex << z << " pixel " << pixel;
	}
}
