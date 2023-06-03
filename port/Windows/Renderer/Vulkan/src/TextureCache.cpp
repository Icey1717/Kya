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

Renderer::TextureData gImageData;

void Renderer::SetImagePointer(Renderer::TextureData inImage)
{
	gImageData = inImage;
}

namespace PS2_Internal {
	template<int i, int alignment, uint32_t mask> __forceinline static void WriteColumn32(uint8_t* dst, const uint8_t* src, int srcpitch)
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

	static void ExpandBlock4_32(const uint8_t* src, uint8_t* dst, int dstpitch, const uint64_t* pal)
	{
		for (int j = 0; j < 16; j++, dst += dstpitch)
		{

		}
	}

	static void ReadAndExpandBlock4_32(const uint8_t* src, uint8_t* dst, int dstpitch)
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
				const uint32_t colorl = ((uint32_t*)gImageData.pallete.pImage)[lower];
				const uint32_t colorh = ((uint32_t*)gImageData.pallete.pImage)[higher];

				// Get the destination.
				const uint32_t xOffset = x * 8; // 8 pixels per byte
				const uint32_t yOffset = y * dstpitch; // 8 pixels per byte
				uint32_t* const dstu32 = (uint32_t*)(dst + xOffset + yOffset);

				// Write the colors.
				dstu32[0] = colorl;
				dstu32[1] = colorh;
			}
		}

		((uint32_t*)dst)[0] = 0xFFFF00FF;
#endif
	}

	const int blockSize = 0x80;

	const int srcBPP = 4;

	// Destination is 2x2 Big Blocks of 128 x 128 (32 bpp) pixels.
	// Each block is 4x8 of 32x16 mini blocks
	const int dstBPP = 32;
	const int dstBigBlockSize = 128;
	const int dstSmallBlockX = 4;
	const int dstSmallBlockY = 8;
	const int dstSmallBlockWidth = 32;
	const int dstSmallBlockHeight = 16;

	const int xBlocks = dstBigBlockSize / dstSmallBlockWidth;
	const int yBlocks = dstBigBlockSize / dstSmallBlockHeight;

	const int sourceBlockSize = 0x100;
	const int sourceBlockStride = blockSize * dstSmallBlockWidth;


	void ReadTextureBlock4(uint8_t* src, uint8_t* dst, int dstpitch)
	{
		int _offset = dstpitch * (dstSmallBlockHeight);
		for (int y = 0; y < yBlocks; y += 1, dst += _offset)
		{
			for (int x = 0; x < xBlocks; x += 1)
			{
				uint8_t* read_dst = &dst[x * dstSmallBlockX * dstBPP];
				ReadAndExpandBlock4_32(src + (x * sourceBlockStride) + (y * sourceBlockSize), read_dst, dstpitch);
			}
		}
	}
}

PS2::GSTexValue::GSTexValue(const GSTexValueCreateInfo& createInfo)
	: width(1 << createInfo.key.TW)
	, height(1 << createInfo.key.TH)
{
	CreateResources();
	UploadImage();
	CreateDescriptorPool(createInfo.descriptorSetLayoutBindings);
	CreateDescriptorSets(createInfo.descriptorSetLayouts);
}

void PS2::GSTexValue::Cleanup() {
	vkDestroySampler(GetDevice(), sampler, nullptr);

	vkDestroyBuffer(GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(GetDevice(), stagingBufferMemory, nullptr);

	vkDestroyImageView(GetDevice(), imageView, nullptr);
	vkDestroyImage(GetDevice(), image, nullptr);
	vkFreeMemory(GetDevice(), imageMemory, nullptr);
}

void PS2::GSTexValue::UploadImage() {
	const VkDeviceSize bufferSize = width * height * 4;

	std::vector<uint8_t> pixelBuffer;
	std::vector<uint8_t> imageBuffer;
	pixelBuffer.resize(width * height * 16);
	imageBuffer.resize(width * height * 16);
	memset(pixelBuffer.data(), 0, width * height * 4);
	memset(imageBuffer.data(), 0, width * height * 4);
	PS2_Internal::WriteImageBlock<0, 8, 8, 32>(0, gImageData.image.height, 0, gImageData.image.width, (uint8_t*)gImageData.image.pImage, imageBuffer.data(), 0x200);

	if (gImageData.image.bpp == 4) {
		GSVector4i r;
		r.left = 0;
		r.top = 0x0;
		r.right = PS2_Internal::dstBigBlockSize;
		r.bottom = PS2_Internal::dstBigBlockSize;

		for (int y = 0; y < (height / PS2_Internal::dstBigBlockSize); y++) {
			for (int x = 0; x < (width / PS2_Internal::dstBigBlockSize); x++) {
				PS2_Internal::ReadTextureBlock4(imageBuffer.data() + (x * 0x800) + (y * 0x800 * PS2_Internal::yBlocks), pixelBuffer.data() + (PS2_Internal::dstBigBlockSize * 4 * x) + (PS2_Internal::dstBigBlockSize * PS2_Internal::dstBigBlockSize * 8 * y), 0x400);
			}
		}
	}

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, pixelBuffer.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(GetDevice(), stagingBufferMemory);

	VulkanImage::TransitionImageLayout(image, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VulkanImage::CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	VulkanImage::TransitionImageLayout(image, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}


void PS2::GSTexValue::CreateResources() {
	const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	// Create the images that will be used as render targets
	VkImageCreateInfo colorImageCreateInfo = {};
	colorImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	colorImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	colorImageCreateInfo.extent.width = width;
	colorImageCreateInfo.extent.height = height;
	colorImageCreateInfo.extent.depth = 1;
	colorImageCreateInfo.mipLevels = 1;
	colorImageCreateInfo.arrayLayers = 1;
	colorImageCreateInfo.format = format;
	colorImageCreateInfo.tiling = tiling;
	colorImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorImageCreateInfo.usage = usage;
	colorImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	colorImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateImage(GetDevice(), &colorImageCreateInfo, nullptr, &image);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan image.");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(GetDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(GetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(GetDevice(), image, imageMemory, 0);

	// Create an image view for the color image
	VkImageViewCreateInfo colorImageViewCreateInfo = {};

	colorImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageViewCreateInfo.image = image;
	colorImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageViewCreateInfo.format = format;
	colorImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	colorImageViewCreateInfo.subresourceRange.levelCount = 1;
	colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	colorImageViewCreateInfo.subresourceRange.layerCount = 1;

	vkCreateImageView(GetDevice(), &colorImageViewCreateInfo, nullptr, &imageView);

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

void PS2::GSTexValue::CreateDescriptorSets(const LayoutVector& descriptorSetLayouts) {
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

void PS2::GSTexValue::CreateDescriptorPool(const LayoutBindingMap& descriptorSetLayoutBindingsMap) {
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

PS2::GSTexEntry& PS2::TextureCache::Create(const GSState::GSTex& TEX, const LayoutVector& descriptorSetLayouts, const LayoutBindingMap& descriptorSetLayoutBindings)
{
	const GSTexKey key = GSTexKey::CreateFromTEX(TEX);
	const GSTexValueCreateInfo createInfo = GSTexValueCreateInfo(key, descriptorSetLayouts, descriptorSetLayoutBindings);
	texcache.emplace_back(GSTexEntry(createInfo));
	return texcache.back();
}

PS2::GSTexEntry& PS2::TextureCache::Lookup(const GSState::GSTex& TEX, const LayoutVector& descriptorSetLayouts, const LayoutBindingMap& descriptorSetLayoutBindings)
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
