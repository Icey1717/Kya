#include "TextureCache.h"
#include "VulkanIncludes.h"
#include "Pipeline.h"
#include "VulkanRenderer.h"
#include "stdexcept"
#include "renderer.h"
#include "GSVector.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include <array>
#include "UniformBuffer.h"
#include "log.h"
#include "TextureCacheRowOffset.h"

static const int* rowOffset[8] = {
	rowOffset32,
	rowOffset32,
	rowOffset32,
	rowOffset32,
	rowOffset32,
	rowOffset32,
	rowOffset32,
	rowOffset32,
};

#define RESTRICT __restrict__

#define LOG_TEXCACHE(fmt, ...) Log::GetInstance().AddLog(LogLevel::Info, "Texture Cache", fmt, ##__VA_ARGS__);

void Renderer::ImageData::Log(const char* prefix) const
{
	LOG_TEXCACHE("{} bpp: {}, w: {}, h: {}, rw: {}, rh: {} field_0x6: {}", prefix, bpp, canvasWidth, canvasHeight, readWidth, readHeight, maxMipLevel);
}

namespace PS2_Internal {
	template<int i, int alignment, uint32_t mask> static void WriteColumn32(uint8_t* dst, const uint8_t* src, int srcpitch)
	{
		const uint8_t* s0 = &src[srcpitch * 0];
		const uint8_t* s1 = &src[srcpitch * 1];

#if _M_SSE >= 0x501

		GSVector8i v0, v1;

		if (alignment == 32)
		{
			v0 = GSVector8i::load<true>(s0).acbd();
			v1 = GSVector8i::load<true>(s1).acbd();

			GSVector8i::sw64(v0, v1);
		}
		else
		{
			if (alignment == 16)
			{
				v0 = GSVector8i::load(&s0[0], &s0[16]).acbd();
				v1 = GSVector8i::load(&s1[0], &s1[16]).acbd();

				GSVector8i::sw64(v0, v1);
			}
			else
			{
				//v0 = GSVector8i::load(&s0[0], &s0[16], &s0[8], &s0[24]);
				//v1 = GSVector8i::load(&s1[0], &s1[16], &s1[8], &s1[24]);

				GSVector4i v4 = GSVector4i::load(&s0[0], &s1[0]);
				GSVector4i v5 = GSVector4i::load(&s0[8], &s1[8]);
				GSVector4i v6 = GSVector4i::load(&s0[16], &s1[16]);
				GSVector4i v7 = GSVector4i::load(&s0[24], &s1[24]);

				if (mask == 0xffffffff)
				{
					// just write them out directly

					((GSVector4i*)dst)[i * 4 + 0] = v4;
					((GSVector4i*)dst)[i * 4 + 1] = v5;
					((GSVector4i*)dst)[i * 4 + 2] = v6;
					((GSVector4i*)dst)[i * 4 + 3] = v7;

					return;
				}

				v0 = GSVector8i::cast(v4).insert<1>(v5);
				v1 = GSVector8i::cast(v6).insert<1>(v7);
			}
		}

		if (mask == 0xffffffff)
		{
			((GSVector8i*)dst)[i * 2 + 0] = v0;
			((GSVector8i*)dst)[i * 2 + 1] = v1;
		}
		else
		{
			GSVector8i v2((int)mask);

			if (mask == 0xff000000 || mask == 0x00ffffff)
			{
				((GSVector8i*)dst)[i * 2 + 0] = ((GSVector8i*)dst)[i * 2 + 0].blend8(v0, v2);
				((GSVector8i*)dst)[i * 2 + 1] = ((GSVector8i*)dst)[i * 2 + 1].blend8(v1, v2);
			}
			else
			{
				((GSVector8i*)dst)[i * 2 + 0] = ((GSVector8i*)dst)[i * 2 + 0].blend(v0, v2);
				((GSVector8i*)dst)[i * 2 + 1] = ((GSVector8i*)dst)[i * 2 + 1].blend(v1, v2);
			}
		}

#else

		GSVector4i v0, v1, v2, v3;

		if (alignment != 0)
		{
			v0 = GSVector4i::load<true>(&s0[0]);
			v1 = GSVector4i::load<true>(&s0[16]);
			v2 = GSVector4i::load<true>(&s1[0]);
			v3 = GSVector4i::load<true>(&s1[16]);

			GSVector4i::sw64(v0, v2, v1, v3);
		}
		else
		{
			v0 = GSVector4i::load(&s0[0], &s1[0]);
			v1 = GSVector4i::load(&s0[8], &s1[8]);
			v2 = GSVector4i::load(&s0[16], &s1[16]);
			v3 = GSVector4i::load(&s0[24], &s1[24]);
		}

		if (mask == 0xffffffff)
		{
			((GSVector4i*)dst)[i * 4 + 0] = v0;
			((GSVector4i*)dst)[i * 4 + 1] = v1;
			((GSVector4i*)dst)[i * 4 + 2] = v2;
			((GSVector4i*)dst)[i * 4 + 3] = v3;
		}
		else
		{
			GSVector4i v4((int)mask);

#if _M_SSE >= 0x401

			if (mask == 0xff000000 || mask == 0x00ffffff)
			{
				((GSVector4i*)dst)[i * 4 + 0] = ((GSVector4i*)dst)[i * 4 + 0].blend8(v0, v4);
				((GSVector4i*)dst)[i * 4 + 1] = ((GSVector4i*)dst)[i * 4 + 1].blend8(v1, v4);
				((GSVector4i*)dst)[i * 4 + 2] = ((GSVector4i*)dst)[i * 4 + 2].blend8(v2, v4);
				((GSVector4i*)dst)[i * 4 + 3] = ((GSVector4i*)dst)[i * 4 + 3].blend8(v3, v4);
			}
			else
			{

#endif

				((GSVector4i*)dst)[i * 4 + 0] = ((GSVector4i*)dst)[i * 4 + 0].blend(v0, v4);
				((GSVector4i*)dst)[i * 4 + 1] = ((GSVector4i*)dst)[i * 4 + 1].blend(v1, v4);
				((GSVector4i*)dst)[i * 4 + 2] = ((GSVector4i*)dst)[i * 4 + 2].blend(v2, v4);
				((GSVector4i*)dst)[i * 4 + 3] = ((GSVector4i*)dst)[i * 4 + 3].blend(v3, v4);

#if _M_SSE >= 0x401

			}

#endif
		}

#endif
	}

	template<int alignment, uint32_t mask> static void WriteBlock32(uint8_t* dst, const uint8_t* src, int srcpitch)
	{
		WriteColumn32<0, alignment, mask>(dst, src, srcpitch);
		src += srcpitch * 2;
		WriteColumn32<1, alignment, mask>(dst, src, srcpitch);
		src += srcpitch * 2;
		WriteColumn32<2, alignment, mask>(dst, src, srcpitch);
		src += srcpitch * 2;
		WriteColumn32<3, alignment, mask>(dst, src, srcpitch);
	}

	template<int psm, int bsx, int bsy, int alignment>
	void WriteImageBlock(int l, int r, int y, int h, const uint8_t* src, uint8_t* dst, int srcpitch)
	{
		int i = 0;
		for (int offset = srcpitch * bsy; h >= bsy; h -= bsy, y += bsy, src += offset)
		{
			for (int x = l; x < r; x += bsx)
			{
				WriteBlock32<alignment, 0xffffffff>(dst + (i * 256), &src[x * 4], srcpitch);
				i++;
			}
		}
	}

	template<int i> __forceinline static void ReadColumn4(const uint8_t* src, uint8_t* dst, int dstpitch)
	{
		//printf("ReadColumn4\n");

#if _M_SSE >= 0x301

		const GSVector4i* s = (const GSVector4i*)src;

		GSVector4i v0 = s[i * 4 + 0].xzyw();
		GSVector4i v1 = s[i * 4 + 1].xzyw();
		GSVector4i v2 = s[i * 4 + 2].xzyw();
		GSVector4i v3 = s[i * 4 + 3].xzyw();

		GSVector4i::sw64(v0, v1, v2, v3);
		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);

		v0 = v0.shuffle8(m_r4mask);
		v1 = v1.shuffle8(m_r4mask);
		v2 = v2.shuffle8(m_r4mask);
		v3 = v3.shuffle8(m_r4mask);

		if ((i & 1) == 0)
		{
			GSVector4i::sw16rh(v0, v1, v2, v3);
		}
		else
		{
			GSVector4i::sw16rl(v0, v1, v2, v3);
		}

		GSVector4i::store<true>(&dst[dstpitch * 0], v0);
		GSVector4i::store<true>(&dst[dstpitch * 1], v1);
		GSVector4i::store<true>(&dst[dstpitch * 2], v2);
		GSVector4i::store<true>(&dst[dstpitch * 3], v3);

#else

		const GSVector4i* s = (const GSVector4i*)src;

		GSVector4i v0 = s[i * 4 + 0];
		GSVector4i v1 = s[i * 4 + 1];
		GSVector4i v2 = s[i * 4 + 2];
		GSVector4i v3 = s[i * 4 + 3];

		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v2, v1, v3);

		v0 = v0.xzyw();
		v1 = v1.xzyw();
		v2 = v2.xzyw();
		v3 = v3.xzyw();

		GSVector4i::sw64(v0, v1, v2, v3);

		if ((i & 1) == 0)
		{
			v2 = v2.yxwzlh();
			v3 = v3.yxwzlh();
		}
		else
		{
			v0 = v0.yxwzlh();
			v1 = v1.yxwzlh();
		}

		GSVector4i::store<true>(&dst[dstpitch * 0], v0);
		GSVector4i::store<true>(&dst[dstpitch * 1], v1);
		GSVector4i::store<true>(&dst[dstpitch * 2], v2);
		GSVector4i::store<true>(&dst[dstpitch * 3], v3);

#endif
	}

	template<int i> __forceinline static void ReadColumn8(const uint8_t* src, uint8_t* dst, int dstpitch)
	{
		//for(int j = 0; j < 64; j++) ((uint8*)src)[j] = (uint8)j;

#if 0//_M_SSE >= 0x501

		const GSVector8i* s = (const GSVector8i*)src;

		GSVector8i v0 = s[i * 2 + 0];
		GSVector8i v1 = s[i * 2 + 1];

		GSVector8i::sw8(v0, v1);
		GSVector8i::sw16(v0, v1);
		GSVector8i::sw8(v0, v1);
		GSVector8i::sw128(v0, v1);
		GSVector8i::sw16(v0, v1);

		v0 = v0.acbd();
		v1 = v1.acbd();
		v1 = v1.yxwz();

		GSVector8i::storel(&dst[dstpitch * 0], v0);
		GSVector8i::storeh(&dst[dstpitch * 1], v0);
		GSVector8i::storel(&dst[dstpitch * 2], v1);
		GSVector8i::storeh(&dst[dstpitch * 3], v1);

		// TODO: not sure if this is worth it, not in this form, there should be a shorter path

#elif _M_SSE >= 0x301

		const GSVector4i* s = (const GSVector4i*)src;

		GSVector4i v0, v1, v2, v3;

		if ((i & 1) == 0)
		{
			v0 = s[i * 4 + 0];
			v1 = s[i * 4 + 1];
			v2 = s[i * 4 + 2];
			v3 = s[i * 4 + 3];
		}
		else
		{
			v2 = s[i * 4 + 0];
			v3 = s[i * 4 + 1];
			v0 = s[i * 4 + 2];
			v1 = s[i * 4 + 3];
		}

		v0 = v0.shuffle8(m_r8mask);
		v1 = v1.shuffle8(m_r8mask);
		v2 = v2.shuffle8(m_r8mask);
		v3 = v3.shuffle8(m_r8mask);

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v3, v2);

		GSVector4i::store<true>(&dst[dstpitch * 0], v0);
		GSVector4i::store<true>(&dst[dstpitch * 1], v3);
		GSVector4i::store<true>(&dst[dstpitch * 2], v1);
		GSVector4i::store<true>(&dst[dstpitch * 3], v2);

#else

		const GSVector4i* s = (const GSVector4i*)src;

		GSVector4i v0 = s[i * 4 + 0];
		GSVector4i v1 = s[i * 4 + 1];
		GSVector4i v2 = s[i * 4 + 2];
		GSVector4i v3 = s[i * 4 + 3];

		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		if ((i & 1) == 0)
		{
			v2 = v2.yxwz();
			v3 = v3.yxwz();
		}
		else
		{
			v0 = v0.yxwz();
			v1 = v1.yxwz();
		}

		GSVector4i::store<true>(&dst[dstpitch * 0], v0);
		GSVector4i::store<true>(&dst[dstpitch * 1], v1);
		GSVector4i::store<true>(&dst[dstpitch * 2], v2);
		GSVector4i::store<true>(&dst[dstpitch * 3], v3);

#endif
	}

	static void ReadBlock4(const uint8_t* src, uint8_t* dst, int dstpitch)
	{
		ReadColumn4<0>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn4<1>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn4<2>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn4<3>(src, dst, dstpitch);
	}

	static void ReadBlock8(const uint8_t* src, uint8_t* dst, int dstpitch)
	{
		ReadColumn8<0>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn8<1>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn8<2>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn8<3>(src, dst, dstpitch);
	}

	//static std::vector<uint32_t> ConvertPalette(int bpp, const Renderer::ImageData& pal)
	//{
	//	std::vector<uint32_t> newPal;
	//	newPal.resize(pal.readWidth * pal.readHeight);
	//
	//	if (bpp == 8) {
	//		uint32_t* palVal = (uint32_t*)pal.pImage;
	//
	//		for (int j = 0; j < ((pal.readWidth * pal.readHeight) / 32); j += 1)
	//		{
	//			for (int i = 0; i < 2; i += 1)
	//			{
	//				memcpy(&newPal[(i * 8 * 2) + (j * 32)], palVal + (i * 8) + (j * 32), 8 * sizeof(uint32_t));
	//				memcpy(&newPal[(i * 8 * 2) + 8 + (j * 32)], palVal + ((i * 8) + 16) + (j * 32), 8 * sizeof(uint32_t));
	//			}
	//		}
	//	}
	//	else if (bpp == 4) {
	//		memcpy(newPal.data(), pal.pImage, newPal.size() * sizeof(uint32_t));
	//	}
	//
	//	return newPal;
	//}

	__forceinline static void ExpandBlock4_32(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, int dstpitch, const uint64_t* RESTRICT pal)
	{
		for (int j = 0; j < 16; j++, dst += dstpitch)
		{
			((const GSVector4i*)src)[j].gather64_8(pal, (GSVector4i*)dst);
			//((uint32_t *)dst)[0] = 0xFF0000FF;
		}
	}

	static void ReadAndExpandBlock4_32(const uint8_t* src, uint8_t* dst, int dstpitch, const PS2::GSTexImage& pal, uint8_t* outPaletteIndexes)
	{
		//printf("ReadAndExpandBlock4_32\n");

#if _M_SSE >= 0x401

		const GSVector4i* s = (const GSVector4i*)src;

		GSVector4i v0, v1, v2, v3;
		GSVector4i mask = m_r4mask;

		for (int i = 0; i < 2; i++)
		{
			v0 = s[i * 8 + 0].xzyw();
			v1 = s[i * 8 + 1].xzyw();
			v2 = s[i * 8 + 2].xzyw();
			v3 = s[i * 8 + 3].xzyw();

			GSVector4i::sw64(v0, v1, v2, v3);
			GSVector4i::sw4(v0, v2, v1, v3);
			GSVector4i::sw8(v0, v1, v2, v3);

			v0 = v0.shuffle8(mask);
			v1 = v1.shuffle8(mask);
			v2 = v2.shuffle8(mask);
			v3 = v3.shuffle8(mask);

			GSVector4i::sw16rh(v0, v1, v2, v3);

			v0.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v1.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v2.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v3.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;

			v0 = s[i * 8 + 4].xzyw();
			v1 = s[i * 8 + 5].xzyw();
			v2 = s[i * 8 + 6].xzyw();
			v3 = s[i * 8 + 7].xzyw();

			GSVector4i::sw64(v0, v1, v2, v3);
			GSVector4i::sw4(v0, v2, v1, v3);
			GSVector4i::sw8(v0, v1, v2, v3);

			v0 = v0.shuffle8(mask);
			v1 = v1.shuffle8(mask);
			v2 = v2.shuffle8(mask);
			v3 = v3.shuffle8(mask);

			GSVector4i::sw16rl(v0, v1, v2, v3);

			v0.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v1.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v2.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v3.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
		}

#else

		alignas(32) uint8_t block[(32 / 2) * 16];

		ReadBlock4(src, block, sizeof(block) / 16);

		ExpandBlock4_32(block, dst, dstpitch, (uint64_t*)pal.readBuffer.data());
		ExpandBlock4_32(block, outPaletteIndexes, dstpitch, (uint64_t*)pal.readBuffer.data());
#endif
	}

	__forceinline static void ExpandBlock8_32(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, int dstpitch, const uint32_t* RESTRICT pal)
	{
		for (int j = 0; j < 16; j++, dst += dstpitch)
		{
			((const GSVector4i*)src)[j].gather32_8(pal, (GSVector4i*)dst);
		}
	}

	static void ReadAndExpandBlock8_32(const uint8_t* src, uint8_t* dst, int dstpitch, const PS2::GSTexImage& pal, uint8_t* outPaletteIndexes)
	{
		//printf("ReadAndExpandBlock8_32\n");

#if _M_SSE >= 0x401

		const GSVector4i* s = (const GSVector4i*)src;

		GSVector4i v0, v1, v2, v3;
		GSVector4i mask = m_r8mask;

		for (int i = 0; i < 2; i++)
		{
			v0 = s[i * 8 + 0].shuffle8(mask);
			v1 = s[i * 8 + 1].shuffle8(mask);
			v2 = s[i * 8 + 2].shuffle8(mask);
			v3 = s[i * 8 + 3].shuffle8(mask);

			GSVector4i::sw16(v0, v1, v2, v3);
			GSVector4i::sw32(v0, v1, v3, v2);

			v0.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v3.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v1.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v2.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;

			v2 = s[i * 8 + 4].shuffle8(mask);
			v3 = s[i * 8 + 5].shuffle8(mask);
			v0 = s[i * 8 + 6].shuffle8(mask);
			v1 = s[i * 8 + 7].shuffle8(mask);

			GSVector4i::sw16(v0, v1, v2, v3);
			GSVector4i::sw32(v0, v1, v3, v2);

			v0.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v3.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v1.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v2.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
		}

#else

		alignas(32) uint8_t block[16 * 16];

		ReadBlock8(src, (uint8_t*)block, sizeof(block) / 16);

		//auto palBlock = ConvertPalette(8, pal);
		//
		//for (int y = 0; y < 16; y++)
		//{
		//	for (int x = 0; x < 16; x++) // 16 across, with 1 pixels per byte.
		//	{
		//		const uint8_t byte = block[(y * 16) + x];
		//
		//		// Get the two colors.
		//		const uint32_t color = palBlock[byte];
		//
		//		// Get the destination.
		//		const uint32_t xOffset = x * 4; // 8 pixels per byte
		//		const uint32_t yOffset = y * dstpitch; // 8 pixels per byte
		//		uint32_t* const dstu32 = (uint32_t*)(dst + xOffset + yOffset);
		//
		//		// Write the colors.
		//		dstu32[0] = color;
		//
		//		uint32_t* const dstPalu32 = (uint32_t*)(outPaletteIndexes + xOffset + yOffset);
		//		dstPalu32[0] = byte;
		//	}
		//}

		ExpandBlock8_32(block, dst, dstpitch, (uint32_t*)pal.readBuffer.data());
		ExpandBlock8_32(block, outPaletteIndexes, dstpitch, (uint32_t*)pal.readBuffer.data());

#endif
	}

	const int blockSize = 128;

	const int srcBPP = 4;

	// Destination is 2x2 Big Blocks of 128 x 128 (32 bpp) pixels.
	// Each block is 4x8 of 32x16 mini blocks
	const int dstBPP = 32;
	const int dstBigBlockSize = 128;
	const int dstSmallBlockY = 8;

	const int sourceBlockSize = 0x100;


	void ReadTextureBlock4(uint8_t* src, uint8_t* dst, int dstpitch, const PS2::GSTexImage& pal, uint32_t width, uint32_t height, uint8_t* outPaletteIndexes)
	{
		const int dstSmallBlockWidth = 32;
		const int dstSmallBlockHeight = 16;
		int xBlocks = std::min<int>(dstBigBlockSize, width) / dstSmallBlockWidth;
		int yBlocks = std::min<int>(dstBigBlockSize, height) / dstSmallBlockHeight;
		int _offset = dstpitch * (dstSmallBlockHeight);
		const int dstSmallBlockX = 4;

		for (int y = 0; y < yBlocks; y += 1, dst += _offset, outPaletteIndexes += _offset)
		{
			for (int x = 0; x < xBlocks; x += 1)
			{
				uint8_t* read_dst = &dst[x * dstSmallBlockX * dstBPP];
				uint8_t* pal_dst = &outPaletteIndexes[x * dstSmallBlockX * dstBPP];
				ReadAndExpandBlock4_32(src + (x * width * 0x10) + (y * sourceBlockSize), read_dst, dstpitch, pal, pal_dst);

				// Magenta
				//((uint32_t*)read_dst)[0] = 0xFFFF00FF;
			}
		}
	}

	void ReadTextureBlock8(uint8_t* src, uint8_t* dst, int dstpitch, const PS2::GSTexImage& pal, uint32_t width, uint32_t height, int bigBlockX, int bigBlockY, uint8_t* outPaletteIndexes)
	{
		const int dstSmallBlockWidth = 16;
		const int dstSmallBlockHeight = 16;
		int xBlocks = std::min<int>(dstBigBlockSize, width) / dstSmallBlockWidth;
		int yBlocks = std::min<int>(dstBigBlockSize, height) / dstSmallBlockHeight;

		int _offset = dstpitch * (dstSmallBlockHeight);

		const int sourceBlockStride = blockSize * dstSmallBlockWidth;

		const int dstSmallBlockX = 2;

		uint32_t* base = ((uint32_t*)dst);

		for (int y = 0; y < yBlocks; y += 1, dst += _offset, outPaletteIndexes += _offset)
		{
			for (int x = 0; x < xBlocks; x += 1)
			{
				uint8_t* read_dst = &dst[x * dstSmallBlockX * dstBPP];
				uint8_t* pal_dst = &outPaletteIndexes[x * dstSmallBlockX * dstBPP];
				ReadAndExpandBlock8_32(src + (x * 0x100) + (y * 0x100 * xBlocks * bigBlockX), read_dst, dstpitch, pal, pal_dst);

				// Cyan
				//((uint32_t*)read_dst)[0] = 0xFFFFFF00;
			}
		}

		//base[0] = 0xFF0000FF;
	}

	void WriteCLUT_T32_I4_CSM1(const uint32_t* RESTRICT src, uint16_t* RESTRICT clut)
	{
		// 1 block

#if _M_SSE >= 0x501

		GSVector8i* s = (GSVector8i*)src;
		GSVector8i* d = (GSVector8i*)clut;

		GSVector8i v0 = s[0].acbd();
		GSVector8i v1 = s[1].acbd();

		GSVector8i::sw16(v0, v1);
		GSVector8i::sw16(v0, v1);
		GSVector8i::sw16(v0, v1);

		d[0] = v0;
		d[16] = v1;

#else

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)clut;

		GSVector4i v0 = s[0];
		GSVector4i v1 = s[1];
		GSVector4i v2 = s[2];
		GSVector4i v3 = s[3];

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v2, v1, v3);

		d[0] = v0;
		d[1] = v2;
		d[32] = v1;
		d[33] = v3;

#endif
	}

	void WriteCLUT_T32_I8_CSM1(const uint32_t* src, uint16_t* clut)
	{
		// 4 blocks

		for (int i = 0; i < 64; i += 16)
		{
			WriteCLUT_T32_I4_CSM1(&src[i + 0], &clut[i * 2 + 0]);
			WriteCLUT_T32_I4_CSM1(&src[i + 64], &clut[i * 2 + 16]);
			WriteCLUT_T32_I4_CSM1(&src[i + 128], &clut[i * 2 + 128]);
			WriteCLUT_T32_I4_CSM1(&src[i + 192], &clut[i * 2 + 144]);
		}
	}

	__forceinline void ReadCLUT_T32_I4(const uint16_t* RESTRICT clut, uint32_t* RESTRICT dst)
	{
		GSVector4i* s = (GSVector4i*)clut;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v0 = s[0];
		GSVector4i v1 = s[1];
		GSVector4i v2 = s[32];
		GSVector4i v3 = s[33];

		GSVector4i::sw16(v0, v2, v1, v3);

		d[0] = v0;
		d[1] = v1;
		d[2] = v2;
		d[3] = v3;
	}

	void ReadCLUT_T32_I8(const uint16_t* RESTRICT clut, uint32_t* RESTRICT dst)
	{
		for (int i = 0; i < 256; i += 16)
		{
			ReadCLUT_T32_I4(&clut[i], &dst[i]);
		}
	}

	template<int psm, int bsx, int bsy, int alignment>
	void WriteImageColumn(int l, int r, int y, int h, const uint8_t* src, uint8_t* dst, int srcpitch)
	{
		uint32_t bp = 0; //BITBLTBUF.DBP;
		uint32_t bw = 0; //BITBLTBUF.DBW;

		const int csy = bsy / 4;

		for (int offset = srcpitch * csy; h >= csy; h -= csy, y += csy, src += offset)
		{
			for (int x = l; x < r; x += bsx)
			{
				WriteColumn32<0, alignment, 0xffffffff>(dst, &src[x * 4], srcpitch);
			}
		}
	}

	template<int psm, int bsx, int bsy, int trbpp>
	void WriteImageTopBottom(int l, int r, int y, int h, const uint8_t* src, uint8_t* dst, int srcpitch)
	{
		alignas(32) uint8_t buff[64]; // merge buffer for one column

		uint32_t bp = 0; //BITBLTBUF.DBP;
		uint32_t bw = 0; //BITBLTBUF.DBW;

		const int csy = bsy / 4;

		// merge incomplete column

		int y2 = y & (csy - 1);

		if (y2 > 0)
		{
			assert(false);
			//int h2 = std::min(h, csy - y2);
			//
			//for (int x = l; x < r; x += bsx)
			//{
			//	uint8_t* dst = NULL;
			//
			//	switch (psm)
			//	{
			//	case PSM_PSMCT32: dst = BlockPtr32(x, y, bp, bw); break;
			//	case PSM_PSMCT16: dst = BlockPtr16(x, y, bp, bw); break;
			//	case PSM_PSMCT16S: dst = BlockPtr16S(x, y, bp, bw); break;
			//	case PSM_PSMT8: dst = BlockPtr8(x, y, bp, bw); break;
			//	case PSM_PSMT4: dst = BlockPtr4(x, y, bp, bw); break;
			//	case PSM_PSMZ32: dst = BlockPtr32Z(x, y, bp, bw); break;
			//	case PSM_PSMZ16: dst = BlockPtr16Z(x, y, bp, bw); break;
			//	case PSM_PSMZ16S: dst = BlockPtr16SZ(x, y, bp, bw); break;
			//		// TODO
			//	default: __assume(0);
			//	}
			//
			//	switch (psm)
			//	{
			//	case PSM_PSMCT32:
			//	case PSM_PSMZ32:
			//		GSBlock::ReadColumn32(y, dst, buff, 32);
			//		memcpy(&buff[32], &src[x * 4], 32);
			//		GSBlock::WriteColumn32<32, 0xffffffff>(y, dst, buff, 32);
			//		break;
			//	case PSM_PSMCT16:
			//	case PSM_PSMCT16S:
			//	case PSM_PSMZ16:
			//	case PSM_PSMZ16S:
			//		GSBlock::ReadColumn16(y, dst, buff, 32);
			//		memcpy(&buff[32], &src[x * 2], 32);
			//		GSBlock::WriteColumn16<32>(y, dst, buff, 32);
			//		break;
			//	case PSM_PSMT8:
			//		GSBlock::ReadColumn8(y, dst, buff, 16);
			//		for (int i = 0, j = y2; i < h2; i++, j++) memcpy(&buff[j * 16], &src[i * srcpitch + x], 16);
			//		GSBlock::WriteColumn8<32>(y, dst, buff, 16);
			//		break;
			//	case PSM_PSMT4:
			//		GSBlock::ReadColumn4(y, dst, buff, 16);
			//		for (int i = 0, j = y2; i < h2; i++, j++) memcpy(&buff[j * 16], &src[i * srcpitch + (x >> 1)], 16);
			//		GSBlock::WriteColumn4<32>(y, dst, buff, 16);
			//		break;
			//		// TODO
			//	default:
			//		__assume(0);
			//	}
			//}
			//
			//src += srcpitch * h2;
			//y += h2;
			//h -= h2;
		}

		// write whole columns

		{
			int h2 = h & ~(csy - 1);

			if (h2 > 0)
			{
				size_t addr = (size_t)&src[l * trbpp >> 3];

				if ((addr & 31) == 0 && (srcpitch & 31) == 0)
				{
					WriteImageColumn<psm, bsx, bsy, 32>(l, r, y, h2, src, dst, srcpitch);
				}
				else if ((addr & 15) == 0 && (srcpitch & 15) == 0)
				{
					WriteImageColumn<psm, bsx, bsy, 16>(l, r, y, h2, src, dst, srcpitch);
				}
				else
				{
					WriteImageColumn<psm, bsx, bsy, 0>(l, r, y, h2, src, dst, srcpitch);
				}

				src += srcpitch * h2;
				y += h2;
				h -= h2;
			}
		}

		// merge incomplete column

		if (h >= 1)
		{
			assert(false);
			//for (int x = l; x < r; x += bsx)
			//{
			//	uint8* dst = NULL;
			//
			//	switch (psm)
			//	{
			//	case PSM_PSMCT32: dst = BlockPtr32(x, y, bp, bw); break;
			//	case PSM_PSMCT16: dst = BlockPtr16(x, y, bp, bw); break;
			//	case PSM_PSMCT16S: dst = BlockPtr16S(x, y, bp, bw); break;
			//	case PSM_PSMT8: dst = BlockPtr8(x, y, bp, bw); break;
			//	case PSM_PSMT4: dst = BlockPtr4(x, y, bp, bw); break;
			//	case PSM_PSMZ32: dst = BlockPtr32Z(x, y, bp, bw); break;
			//	case PSM_PSMZ16: dst = BlockPtr16Z(x, y, bp, bw); break;
			//	case PSM_PSMZ16S: dst = BlockPtr16SZ(x, y, bp, bw); break;
			//		// TODO
			//	default: __assume(0);
			//	}
			//
			//	switch (psm)
			//	{
			//	case PSM_PSMCT32:
			//	case PSM_PSMZ32:
			//		GSBlock::ReadColumn32(y, dst, buff, 32);
			//		memcpy(&buff[0], &src[x * 4], 32);
			//		GSBlock::WriteColumn32<32, 0xffffffff>(y, dst, buff, 32);
			//		break;
			//	case PSM_PSMCT16:
			//	case PSM_PSMCT16S:
			//	case PSM_PSMZ16:
			//	case PSM_PSMZ16S:
			//		GSBlock::ReadColumn16(y, dst, buff, 32);
			//		memcpy(&buff[0], &src[x * 2], 32);
			//		GSBlock::WriteColumn16<32>(y, dst, buff, 32);
			//		break;
			//	case PSM_PSMT8:
			//		GSBlock::ReadColumn8(y, dst, buff, 16);
			//		for (int i = 0; i < h; i++) memcpy(&buff[i * 16], &src[i * srcpitch + x], 16);
			//		GSBlock::WriteColumn8<32>(y, dst, buff, 16);
			//		break;
			//	case PSM_PSMT4:
			//		GSBlock::ReadColumn4(y, dst, buff, 16);
			//		for (int i = 0; i < h; i++) memcpy(&buff[i * 16], &src[i * srcpitch + (x >> 1)], 16);
			//		GSBlock::WriteColumn4<32>(y, dst, buff, 16);
			//		break;
			//		// TODO
			//	default:
			//		__assume(0);
			//	}
			//}
		}
	}

	__forceinline void ExpandCLUT64_T32(const GSVector4i& hi, const GSVector4i& lo, GSVector4i* dst)
	{
		dst[0] = lo.upl32(hi);
		dst[1] = lo.uph32(hi);
	}

	__forceinline void ExpandCLUT64_T32(const GSVector4i& hi, const GSVector4i& lo0, const GSVector4i& lo1, const GSVector4i& lo2, const GSVector4i& lo3, GSVector4i* dst)
	{
		ExpandCLUT64_T32(hi.xxxx(), lo0, &dst[0]);
		ExpandCLUT64_T32(hi.xxxx(), lo1, &dst[2]);
		ExpandCLUT64_T32(hi.xxxx(), lo2, &dst[4]);
		ExpandCLUT64_T32(hi.xxxx(), lo3, &dst[6]);
		ExpandCLUT64_T32(hi.yyyy(), lo0, &dst[8]);
		ExpandCLUT64_T32(hi.yyyy(), lo1, &dst[10]);
		ExpandCLUT64_T32(hi.yyyy(), lo2, &dst[12]);
		ExpandCLUT64_T32(hi.yyyy(), lo3, &dst[14]);
		ExpandCLUT64_T32(hi.zzzz(), lo0, &dst[16]);
		ExpandCLUT64_T32(hi.zzzz(), lo1, &dst[18]);
		ExpandCLUT64_T32(hi.zzzz(), lo2, &dst[20]);
		ExpandCLUT64_T32(hi.zzzz(), lo3, &dst[22]);
		ExpandCLUT64_T32(hi.wwww(), lo0, &dst[24]);
		ExpandCLUT64_T32(hi.wwww(), lo1, &dst[26]);
		ExpandCLUT64_T32(hi.wwww(), lo2, &dst[28]);
		ExpandCLUT64_T32(hi.wwww(), lo3, &dst[30]);
	}

	void ExpandCLUT64_T32_I8(const uint32_t* RESTRICT src, uint64_t* RESTRICT dst)
	{
		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i s0 = s[0];
		GSVector4i s1 = s[1];
		GSVector4i s2 = s[2];
		GSVector4i s3 = s[3];

		ExpandCLUT64_T32(s0, s0, s1, s2, s3, &d[0]);
		ExpandCLUT64_T32(s1, s0, s1, s2, s3, &d[32]);
		ExpandCLUT64_T32(s2, s0, s1, s2, s3, &d[64]);
		ExpandCLUT64_T32(s3, s0, s1, s2, s3, &d[96]);
	}

	void WriteImageX(int psm, int len, void* src, uint32_t* dst, int rw, int rh)
	{
		if (len <= 0) return;

		const uint8_t* pb = (uint8_t*)src;
		const uint16_t* pw = (uint16_t*)src;
		const uint32_t* pd = (uint32_t*)src;

		//uint32 bp = BITBLTBUF.DBP;
		//uint32 bw = BITBLTBUF.DBW;
		//psm_t* psm = &m_psm[BITBLTBUF.DPSM];

		int x = 0;
		int y = 0;
		int sx = (int)0;
		int ex = sx + (int)rw;

		switch (psm)
		{
		case 0x20:

			len /= 4;

			while (len > 0)
			{
				constexpr int yOffsets[8] = {0x0, 0x2, 0x10, 0x12, 0x20, 0x22, 0x30, 0x32};

				assert(y < 8);

				uint32_t addr = yOffsets[y];
				const int* offset = rowOffset[y & 7];

				for (; len > 0 && x < ex; len--, x++, pd++)
				{
					dst[addr + offset[x]] = *pd;
					//WritePixel32(addr + offset[x], *pd);
				}

				if (x >= ex) { x = sx; y++; }
			}

			break;
		}
	}
}

PS2::GSTexValue::GSTexValue(const GSTexValueCreateInfo& createInfo)
	: image(createInfo.textureData.image)
	, paletteImage(createInfo.textureData.palettes.at(createInfo.key.CBP))
{
	assert(createInfo.textureData.palettes.at(createInfo.key.CBP).canvasWidth);
	assert(createInfo.textureData.palettes.at(createInfo.key.CBP).canvasHeight);

	CreateResources();
	UploadImage();
}

PS2::GSTexValue::GSTexValue(const Renderer::TextureData& inTextureData, uint32_t CBP)
	: image(inTextureData.image)
	, paletteImage(inTextureData.palettes.at(CBP))
{
	assert(inTextureData.palettes.at(CBP).canvasWidth);
	assert(inTextureData.palettes.at(CBP).canvasHeight);

	if (!Renderer::gHeadless) {
		CreateResources();
	}

	UploadImage();

	image.CreateSampler(false);
}

void PS2::GSTexValue::Cleanup() {
	image.Cleanup();
	paletteImage.Cleanup();
}

using namespace PS2_Internal;

void PS2::GSTexValue::UploadImage() 
{
	const Renderer::ImageData& textureImageData = image.imageData;

	const VkDeviceSize bufferSize = textureImageData.canvasWidth * textureImageData.canvasHeight * 4;

	debugData.paletteIndexes.resize(textureImageData.canvasWidth * textureImageData.canvasHeight);

	// 0x40 * 8 = 0x200
	// 0x80 * 4 = 0x200
	const int pixelPerByte = (32 / textureImageData.bpp);

	textureImageData.Log("Uploading texture TEX - ");
	paletteImage.imageData.Log("Uploading texture PAL - ");

	LOG_TEXCACHE("Uploading texture - srcpitch: {}", textureImageData.readWidth * 4);

	uint8_t* const pWriteData = image.writeBuffer.data();
	uint8_t* const pReadData = image.readBuffer.data();

	uint8_t* const pPaletteWriteData = paletteImage.writeBuffer.data();

	if (textureImageData.readHeight < 8) {
		WriteImageX(32, 0x40, textureImageData.pImage, (uint32_t*)pWriteData, textureImageData.readWidth, textureImageData.readHeight);
	}
	else {
		WriteImageBlock<0, 8, 8, 32>(0, textureImageData.readWidth, 0, textureImageData.readHeight, (uint8_t*)textureImageData.pImage, pWriteData, textureImageData.readWidth * 4);
	}
	
	if (paletteImage.imageData.readHeight < 8) {
		WriteImageTopBottom<0, 8, 8, 16>(0, paletteImage.imageData.readWidth, 0, paletteImage.imageData.readHeight, (uint8_t*)paletteImage.imageData.pImage, pPaletteWriteData, paletteImage.imageData.readWidth * 4);
	}
	else {
		WriteImageBlock<0, 8, 8, 16>(0, paletteImage.imageData.readWidth, 0, paletteImage.imageData.readHeight, (uint8_t*)paletteImage.imageData.pImage, pPaletteWriteData, paletteImage.imageData.readWidth * 4);
	}

	std::vector<uint16_t> clut;
	clut.resize(2048);

	uint16_t* const pClutWriteData = clut.data();

	if (textureImageData.bpp == 4) {
		WriteCLUT_T32_I4_CSM1((uint32_t*)pPaletteWriteData, pClutWriteData);
	}
	else if (textureImageData.bpp == 8) {
		WriteCLUT_T32_I8_CSM1((uint32_t*)pPaletteWriteData, pClutWriteData);
	}

	uint32_t* const pClutReadData = (uint32_t*)paletteImage.readBuffer.data();

	if (textureImageData.bpp == 4) {
		ReadCLUT_T32_I4(pClutWriteData, pClutReadData);
	
		memcpy(pClutWriteData, pClutReadData, sizeof(clut[0]) * clut.size());
		ExpandCLUT64_T32_I8((uint32_t*)pClutWriteData, (uint64_t*)pClutReadData);
	}
	else if (textureImageData.bpp == 8) {
		ReadCLUT_T32_I8(pClutWriteData, pClutReadData);
	}

	const int yBlocks = std::max<int>(1, (textureImageData.canvasHeight / dstBigBlockSize));
	const int xBlocks = std::max<int>(1, (textureImageData.canvasWidth / dstBigBlockSize));

	if (textureImageData.bpp == 4) {
		const int writeOffset = pixelPerByte * textureImageData.canvasWidth;

		for (int y = 0; y < yBlocks; y++) {
			for (int x = 0; x < xBlocks; x++) {
				const int dstOffset = (dstBigBlockSize * textureImageData.bpp * x) + (dstBigBlockSize * dstBigBlockSize * 8 * y);
				ReadTextureBlock4(image.writeBuffer.data() + (x * writeOffset) + (y * writeOffset * 0x8)
					, image.readBuffer.data() + dstOffset
					, textureImageData.canvasWidth * 4
					, paletteImage
					, textureImageData.canvasWidth
					, textureImageData.canvasHeight
					, ((uint8_t*)debugData.paletteIndexes.data()) + dstOffset);
			}
		}
	}
	else if (textureImageData.bpp == 8) {
		if (textureImageData.readHeight < 8) {
			assert(textureImageData.readWidth == 4 && textureImageData.readHeight == 4);
			debugData.paletteIndexes.resize(16 * 16);
			ReadAndExpandBlock8_32(pWriteData, pReadData, 0x20, paletteImage, (uint8_t*)debugData.paletteIndexes.data());
		}
		else {
			for (int y = 0; y < yBlocks; y++) {
				for (int x = 0; x < xBlocks; x++) {
					const int dstOffset = (dstBigBlockSize * 4 * x) + (dstBigBlockSize * dstBigBlockSize * 8 * y);
					ReadTextureBlock8(pWriteData + (x * 0x800) + (y * 0x1000 * 0x8)
						, pReadData + dstOffset
						, textureImageData.canvasWidth * 4
						, paletteImage
						, textureImageData.canvasWidth
						, textureImageData.canvasHeight
						, xBlocks
						, yBlocks
						, ((uint8_t*)debugData.paletteIndexes.data()) + dstOffset);
				}
			}
		}
	}
	else {
		// Missing 32 bit texture support.
		
	}

	for (int i = 0; i < debugData.paletteIndexes.size(); i++) {

		for (int j = 0; j < paletteImage.readBuffer.size() / 4; j++) {
			if (((uint32_t*)paletteImage.readBuffer.data())[j] == debugData.paletteIndexes[i]) {
				debugData.paletteIndexes[i] = j;
				break;
			}
		}

	}

	if (!Renderer::gHeadless) {
		image.UploadData(bufferSize, image.readBuffer);

		const VkDeviceSize paletteSize = paletteImage.imageData.readWidth * paletteImage.imageData.readHeight * 4;

		std::vector<uint8_t> paletteReadBuffer;
		paletteReadBuffer.reserve(paletteSize);

		for (int i = 0; i < paletteSize; i++) {
			paletteReadBuffer.push_back(((uint8_t*)paletteImage.readBuffer.data())[i]);
		}

		paletteImage.UploadData(paletteSize, paletteReadBuffer);
	}
}

void PS2::GSTexValue::CreateResources() {
	image.CreateResources(false);
	paletteImage.CreateResources(true);
}

PS2::GSTexImage::GSTexImage(const Renderer::ImageData& inImageData)
	: imageData(inImageData)
{
	const int bufferSize = (imageData.canvasWidth * imageData.canvasHeight * 4) + 0x10000;

	readBuffer.resize(bufferSize);
	writeBuffer.resize(bufferSize);
	memset(readBuffer.data(), 0, bufferSize);
	memset(writeBuffer.data(), 0, bufferSize);
}

void PS2::GSTexImage::UploadData(int bufferSize, std::vector<uint8_t>& readBuffer)
{
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, readBuffer.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(GetDevice(), stagingBufferMemory);

	VulkanImage::TransitionImageLayout(image, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VulkanImage::CopyBufferToImage(stagingBuffer, image, width, height);

	VulkanImage::TransitionImageLayout(image, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void PS2::GSTexImage::CreateResources(const bool bPalette)
{
	const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	width = bPalette ? imageData.readWidth : imageData.canvasWidth;
	height = bPalette ? imageData.readHeight : imageData.canvasHeight;

	VulkanImage::CreateImage(width, height , format, tiling, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
	VulkanImage::CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, imageView);

	if (bPalette) {
		CreateSampler(true);
	}
}


void PS2::GSTexImage::Cleanup()
{
	vkDestroySampler(GetDevice(), sampler, nullptr);

	vkDestroyBuffer(GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(GetDevice(), stagingBufferMemory, nullptr);

	vkDestroyImageView(GetDevice(), imageView, nullptr);
	vkDestroyImage(GetDevice(), image, nullptr);
	vkFreeMemory(GetDevice(), imageMemory, nullptr);
}

PS2::GSTexDescriptor& PS2::GSTexImage::AddDescriptorSets(const Renderer::Pipeline& pipeline)
{
	auto& descriptorSets = descriptorMap[&pipeline];

	// Create descriptor pool based on the descriptor set count from the shader
	Renderer::CreateDescriptorPool(pipeline.descriptorSetLayoutBindings, descriptorSets.descriptorPool);

	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline.descriptorSetLayouts[0]);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorSets.descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.layoutBindingMap = pipeline.descriptorSetLayoutBindings;

	descriptorSets.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(GetDevice(), &allocInfo, descriptorSets.descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		SetObjectName(reinterpret_cast<uint64_t>(descriptorSets.descriptorSets[i]), VK_OBJECT_TYPE_DESCRIPTOR_SET, "GSTexImage descriptor set %d", i);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView;

		assert(sampler != VK_NULL_HANDLE);
		imageInfo.sampler = sampler;



		Renderer::DescriptorWriteList writeList;
		const VkDescriptorBufferInfo vertexDescBufferInfo = descriptorSets.vertexConstBuffer.GetDescBufferInfo(GetCurrentFrame());
		writeList.EmplaceWrite({ Renderer::EBindingStage::Vertex, &vertexDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
		const VkDescriptorBufferInfo fragmentDescBufferInfo = descriptorSets.pixelConstBuffer.GetDescBufferInfo(GetCurrentFrame());
		writeList.EmplaceWrite({ Renderer::EBindingStage::Fragment, &fragmentDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
		writeList.EmplaceWrite({ Renderer::EBindingStage::Fragment, nullptr, &imageInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE });
		writeList.EmplaceWrite({ Renderer::EBindingStage::Fragment, nullptr, &imageInfo, VK_DESCRIPTOR_TYPE_SAMPLER });

		std::vector<VkWriteDescriptorSet> descriptorWrites = writeList.CreateWriteDescriptorSetList(descriptorSets.descriptorSets[i], pipeline.descriptorSetLayoutBindings);

		if (descriptorWrites.size() > 0) {
			vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	return descriptorSets;
}

PS2::GSTexDescriptor& PS2::GSTexImage::GetDescriptorSets(const Renderer::Pipeline& pipeline)
{
	auto& gsDescriptor = descriptorMap[&pipeline];

	Log::GetInstance().AddLog(LogLevel::Verbose, "TexImage", "PS2::GSTexImage::GetDescriptorSets Looking for descriptor sets this: 0x{:x}, pipeline: 0x{:x}, Found: {}", (uintptr_t)this, (uintptr_t)&pipeline, gsDescriptor.descriptorPool != VK_NULL_HANDLE);

	if (gsDescriptor.descriptorPool == VK_NULL_HANDLE) {
		Log::GetInstance().AddLog(LogLevel::Verbose, "TexImage", "PS2::GSTexImage::GetDescriptorSets Creating sets this: 0x{:x}, pipeline: 0x{:x}", (uintptr_t)this, (uintptr_t)&pipeline);
		return AddDescriptorSets(pipeline);
	}

	return gsDescriptor;
}

void PS2::GSTexImage::CreateSampler(bool bPalette /*= false*/)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	if (bPalette) {
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	}
	else {
		samplerInfo.magFilter = samplerSelector.ltf ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
		samplerInfo.minFilter = samplerSelector.ltf ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;

		samplerInfo.addressModeU = samplerSelector.tau ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerSelector.tav ? VK_SAMPLER_ADDRESS_MODE_REPEAT: VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	}

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);

	//samplerInfo.anisotropyEnable = VK_TRUE;
	//samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 0.0f;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = -FLT_MAX;
	samplerInfo.maxLod = FLT_MAX;

	if (vkCreateSampler(GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void PS2::GSTexImage::UpdateSampler(PSSamplerSelector selector)
{
	if (sampler == VK_NULL_HANDLE) {
		samplerSelector = selector;
		CreateSampler();
	}
	else {
		if (selector.key != samplerSelector.key) {
			assert(false);
		}
	}
}

void PS2::GSTexImage::UpdateSampler()
{
	UpdateSampler(samplerSelector);
}

namespace PS2_Internal {
	PS2::TextureCache gTextureCache;
}

PS2::GSTexEntry& PS2::TextureCache::Create(const GIFReg::GSTex& TEX, Renderer::TextureData& textureData, const uint32_t CBP)
{
	const GSTexKey key = GSTexKey::CreateFromTEX(TEX, CBP, textureData.image.pImage, textureData.palettes[CBP].pImage);
	const GSTexValueCreateInfo createInfo = GSTexValueCreateInfo(key, textureData);
	texcache.emplace_back(GSTexEntry(createInfo));
	return texcache.back();
}

PS2::GSTexEntry& PS2::TextureCache::Lookup(const GIFReg::GSTex& TEX, Renderer::TextureData& textureData, uint32_t CBP)
{
	if (textureData.palettes.find(CBP) != textureData.palettes.end()) {
		const GSTexKey key = GSTexKey::CreateFromTEX(TEX, CBP, textureData.image.pImage, textureData.palettes[CBP].pImage);

		for (auto& entry : texcache) {
			if (entry == key) {
				return entry;
			}
		}
	}
	else {
		LOG_TEXCACHE("PS2::GSTexValue::GSTexValue MISSING PALETTE!!! CBP: {0} (0x{0:x})", CBP);
		Log::GetInstance().ForceFlush();
		assert(textureData.palettes.size() > 0);
		CBP = textureData.palettes.begin()->first;

		if (textureData.palettes.find(CBP) != textureData.palettes.end()) {
			const GSTexKey key = GSTexKey::CreateFromTEX(TEX, CBP, textureData.image.pImage, textureData.palettes[CBP].pImage);

			for (auto& entry : texcache) {
				if (entry == key) {
					return entry;
				}
			}
		}
	}

	return Create(TEX, textureData, CBP);
}

PS2::TextureCache& PS2::GetTextureCache()
{
	return gTextureCache;
}

PS2::GSTexDescriptor::GSTexDescriptor()
{
	descriptorPool = VK_NULL_HANDLE;
	vertexConstBuffer.Init();
	pixelConstBuffer.Init();
}

void PS2::GSTexDescriptor::UpdateSet(int index) const
{
	const VkDescriptorBufferInfo vertexDescBufferInfo = vertexConstBuffer.GetDescBufferInfo(GetCurrentFrame());
	const VkDescriptorBufferInfo pixelDescBufferInfo = pixelConstBuffer.GetDescBufferInfo(GetCurrentFrame());

	Renderer::DescriptorWriteList writeList;
	writeList.EmplaceWrite({ Renderer::EBindingStage::Vertex, &vertexDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
	writeList.EmplaceWrite({ Renderer::EBindingStage::Fragment, &pixelDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });

	std::vector<VkWriteDescriptorSet> descriptorWrites = writeList.CreateWriteDescriptorSetList(GetSet(GetCurrentFrame()), layoutBindingMap);

	if (descriptorWrites.size() > 0) {
		vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
