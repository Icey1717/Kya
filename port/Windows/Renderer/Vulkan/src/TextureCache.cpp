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

Renderer::TextureData gImageData;

#define LOG_TEXCACHE(fmt, ...) Log::GetInstance().AddLog(LogLevel::Info, "Texture Cache", fmt, ##__VA_ARGS__);

void Renderer::SetImagePointer(Renderer::TextureData inImage)
{
	gImageData = inImage;
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

	static void ReadAndExpandBlock4_32(const uint8_t* src, uint8_t* dst, int dstpitch, const Renderer::ImageData& pal)
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

		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 32 >> 1; x++) // 32 across, with 2 pixels per byte.
			{
				const uint8_t byte = block[(y * 16) + x];

				// Extract the two indexes.
				const uint8_t lower = byte & 0x0F;
				const uint8_t higher = (byte >> 4) & 0x0F;

				// Get the two colors.
				const uint32_t colorl = ((uint32_t*)pal.pImage)[lower];
				const uint32_t colorh = ((uint32_t*)pal.pImage)[higher];

				// Get the destination.
				const uint32_t xOffset = x * 8; // 8 pixels per byte
				const uint32_t yOffset = y * dstpitch;
				uint32_t* const dstu32 = (uint32_t*)(dst + xOffset + yOffset);

				// Write the colors.
				dstu32[0] = colorl;
				dstu32[1] = colorh;
			}
		}
#endif
	}

	static void ReadAndExpandBlock8_32(const uint8_t* src, uint8_t* dst, int dstpitch, const Renderer::ImageData& pal)
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

		alignas(32) uint32_t palBlock[pal.readWidth * pal.readHeight];
		uint32_t* palVal = (uint32_t*)pal.pImage;
		
		for (int j = 0; j < ((pal.readWidth * pal.readHeight) / 32); j += 1)
		{
			for (int i = 0; i < 2; i += 1)
			{
				memcpy(&palBlock[(i * 8 * 2) + (j * 32)], palVal + (i * 8) + (j * 32), 8 * sizeof(uint32_t));
				memcpy(&palBlock[(i * 8 * 2) + 8 + (j * 32)], palVal + ((i * 8) + 16) + (j * 32), 8 * sizeof(uint32_t));
			}
		}



		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++) // 16 across, with 1 pixels per byte.
			{
				const uint8_t byte = block[(y * 16) + x];

				// Get the two colors.
				const uint32_t color = palBlock[byte];

				// Get the destination.
				const uint32_t xOffset = x * 4; // 8 pixels per byte
				const uint32_t yOffset = y * dstpitch; // 8 pixels per byte
				uint32_t* const dstu32 = (uint32_t*)(dst + xOffset + yOffset);

				// Write the colors.
				dstu32[0] = color;
			}
		}

		//ExpandBlock8_32(block, dst, dstpitch, pal);

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


	void ReadTextureBlock4(uint8_t* src, uint8_t* dst, int dstpitch, const Renderer::ImageData& pal, uint32_t width, uint32_t height)
	{
		const int dstSmallBlockWidth = 32;
		const int dstSmallBlockHeight = 16;
		int xBlocks = std::min<int>(dstBigBlockSize, width) / dstSmallBlockWidth;
		int yBlocks = std::min<int>(dstBigBlockSize, height) / dstSmallBlockHeight;
		int _offset = dstpitch * (dstSmallBlockHeight);
		const int dstSmallBlockX = 4;

		for (int y = 0; y < yBlocks; y += 1, dst += _offset)
		{
			for (int x = 0; x < xBlocks; x += 1)
			{
				uint8_t* read_dst = &dst[x * dstSmallBlockX * dstBPP];
				ReadAndExpandBlock4_32(src + (x * width * 0x10) + (y * sourceBlockSize), read_dst, dstpitch, pal);

				// Magenta
				((uint32_t*)read_dst)[0] = 0xFFFF00FF;
			}
		}
	}

	void ReadTextureBlock8(uint8_t* src, uint8_t* dst, int dstpitch, const Renderer::ImageData& pal, uint32_t width, uint32_t height, int bigBlockX, int bigBlockY)
	{
		const int dstSmallBlockWidth = 16;
		const int dstSmallBlockHeight = 16;
		int xBlocks = std::min<int>(dstBigBlockSize, width) / dstSmallBlockWidth;
		int yBlocks = std::min<int>(dstBigBlockSize, height) / dstSmallBlockHeight;
		int _offset = dstpitch * (dstSmallBlockHeight);

		const int sourceBlockStride = blockSize * dstSmallBlockWidth;

		const int dstSmallBlockX = 2;

		uint32_t* base = ((uint32_t*)dst);

		for (int y = 0; y < yBlocks; y += 1, dst += _offset)
		{
			for (int x = 0; x < xBlocks; x += 1)
			{
				uint8_t* read_dst = &dst[x * dstSmallBlockX * dstBPP];
				ReadAndExpandBlock8_32(src + (x * 0x100) + (y * 0x100 * xBlocks * bigBlockX), read_dst, dstpitch, pal);

				// Cyan
				((uint32_t*)read_dst)[0] = 0xFFFFFF00;
			}
		}

		base[0] = 0xFF0000FF;
	}
}

PS2::GSTexValue::GSTexValue(const GSTexValueCreateInfo& createInfo)
	: width(1 << createInfo.key.TW)
	, height(1 << createInfo.key.TH)
	, bpp(0)
{
	AllocateBuffers();
	CreateResources();
	UploadImage(gImageData);
	CreateDescriptorPool(createInfo.descriptorSetLayoutBindings);
	CreateDescriptorSets(createInfo.descriptorSetLayouts);
}

PS2::GSTexValue::GSTexValue(const Renderer::TextureData& textureData)
	: width(textureData.image.canvasWidth)
	, height(textureData.image.canvasHeight)
	, bpp(textureData.image.bpp)
{
	AllocateBuffers();

	if (!Renderer::gHeadless) {
		CreateResources();
	}

	UploadImage(textureData);
}

void PS2::GSTexValue::AllocateBuffers()
{
	const int bufferSize = width * height * 4;

	readBuffer.resize(bufferSize);
	writeBuffer.resize(bufferSize);
	memset(readBuffer.data(), 0, bufferSize);
	memset(writeBuffer.data(), 0, bufferSize);
}

void PS2::GSTexValue::Cleanup() {
	vkDestroySampler(GetDevice(), sampler, nullptr);

	vkDestroyBuffer(GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(GetDevice(), stagingBufferMemory, nullptr);

	vkDestroyImageView(GetDevice(), imageView, nullptr);
	vkDestroyImage(GetDevice(), image, nullptr);
	vkFreeMemory(GetDevice(), imageMemory, nullptr);
}

void PS2::GSTexValue::UploadImage(const Renderer::TextureData& textureData) {
	const VkDeviceSize bufferSize = width * height * 4;

	// 0x40 * 8 = 0x200
	// 0x80 * 4 = 0x200
	const int pixelPerByte = (32 / textureData.image.bpp);
	const int srcpitch = (textureData.image.readHeight) * 4;

	LOG_TEXCACHE("Uploading texture - bpp: %d, w: %d, h: %d, rw: %d, rh: %d", textureData.image.bpp, width, height, textureData.image.readWidth, textureData.image.readHeight);
	LOG_TEXCACHE("Uploading texture - srcpitch: %d", srcpitch);

	uint8_t* const pWriteData = writeBuffer.data();
	uint8_t* const pReadData = readBuffer.data();
	PS2_Internal::WriteImageBlock<0, 8, 8, 32>(0, textureData.image.readHeight, 0, textureData.image.readWidth, (uint8_t*)textureData.image.pImage, pWriteData, srcpitch);

	const int yBlocks = std::max<int>(1, (height / PS2_Internal::dstBigBlockSize));
	const int xBlocks = std::max<int>(1, (width / PS2_Internal::dstBigBlockSize));

	if (textureData.image.bpp == 4) {
		const int writeOffset = pixelPerByte * textureData.image.canvasWidth;

		for (int y = 0; y < yBlocks; y++) {
			for (int x = 0; x < xBlocks; x++) {
				PS2_Internal::ReadTextureBlock4(writeBuffer.data() + (x * writeOffset) + (y * writeOffset * 0x8)
					, readBuffer.data() + (PS2_Internal::dstBigBlockSize * textureData.image.bpp * x) + (PS2_Internal::dstBigBlockSize * PS2_Internal::dstBigBlockSize * 8 * y)
					, textureData.image.canvasWidth * 4
					, textureData.pallete
					, textureData.image.canvasWidth
					, textureData.image.canvasHeight);
			}
		}
	}
	else if (textureData.image.bpp == 8) {
		for (int y = 0; y < yBlocks; y++) {
			for (int x = 0; x < xBlocks; x++) {
				PS2_Internal::ReadTextureBlock8(pWriteData + (x * 0x800) + (y * 0x1000 * 0x8)
					, pReadData + (PS2_Internal::dstBigBlockSize * 4 * x) + (PS2_Internal::dstBigBlockSize * PS2_Internal::dstBigBlockSize * 8 * y)
					, textureData.image.canvasWidth * 4
					, textureData.pallete
					, textureData.image.canvasWidth
					, textureData.image.canvasHeight
					, xBlocks
					, yBlocks);
			}
		}
	}
	else {
		assert(false);
	}

	if (!Renderer::gHeadless) {
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, readBuffer.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(GetDevice(), stagingBufferMemory);

		VulkanImage::TransitionImageLayout(image, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VulkanImage::CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		VulkanImage::TransitionImageLayout(image, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}

void PS2::GSTexValue::CreateResources() {
	const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VulkanImage::CreateImage(width, height, format, tiling, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
	VulkanImage::CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, imageView);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 0.0f;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = -FLT_MAX;
	samplerInfo.maxLod = FLT_MAX;

	if (vkCreateSampler(GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void PS2::GSTexValue::CreateDescriptorSets(const Renderer::LayoutVector& descriptorSetLayouts) {
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayouts[0]);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(GetDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &imageInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		VkDescriptorBufferInfo vertexConstantBuffer{};
		vertexConstantBuffer.buffer = GetVertexConstantUniformBuffer(i);
		vertexConstantBuffer.offset = 0;
		vertexConstantBuffer.range = sizeof(VSConstantBuffer);

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 5;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = &vertexConstantBuffer;

		VkDescriptorBufferInfo pixelConstantBuffer{};
		pixelConstantBuffer.buffer = GetPixelConstantUniformBuffer(i);
		pixelConstantBuffer.offset = 0;
		pixelConstantBuffer.range = sizeof(PSConstantBuffer);

		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = descriptorSets[i];
		descriptorWrites[3].dstBinding = 6;
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pBufferInfo = &pixelConstantBuffer;

		vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void PS2::GSTexValue::CreateDescriptorPool(const Renderer::LayoutBindingMap& descriptorSetLayoutBindingsMap) {
	// Create descriptor pool based on the descriptor set count from the shader

	auto& descriptorSetLayoutBindings = descriptorSetLayoutBindingsMap.at(0);
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

	if (vkCreateDescriptorPool(GetDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

namespace PS2_Internal {
	PS2::TextureCache gTextureCache;
}

PS2::GSTexEntry& PS2::TextureCache::Create(const GSState::GSTex& TEX, const Renderer::LayoutVector& descriptorSetLayouts, const Renderer::LayoutBindingMap& descriptorSetLayoutBindings)
{
	const GSTexKey key = GSTexKey::CreateFromTEX(TEX);
	const GSTexValueCreateInfo createInfo = GSTexValueCreateInfo(key, descriptorSetLayouts, descriptorSetLayoutBindings);
	texcache.emplace_back(GSTexEntry(createInfo));
	return texcache.back();
}

PS2::GSTexEntry& PS2::TextureCache::Lookup(const GSState::GSTex& TEX, const Renderer::LayoutVector& descriptorSetLayouts, const Renderer::LayoutBindingMap& descriptorSetLayoutBindings)
{
	const GSTexKey key = GSTexKey::CreateFromTEX(TEX);
	for (auto& entry : texcache) {
		if (entry == key) {
			return entry;
		}
	}

	return Create(TEX, descriptorSetLayouts, descriptorSetLayoutBindings);
}

PS2::TextureCache& PS2::GetTextureCache()
{
	return PS2_Internal::gTextureCache;
}
