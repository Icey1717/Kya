#include "DebugSaveScreenshot.h"
#include "VulkanRenderer.h"
#include "Native/NativeRenderer.h"
#include "Objects/VulkanBuffer.h"
#include "Objects/VulkanCommands.h"
#include "Objects/VulkanImage.h"
#include "backends/imgui_impl_vulkan.h"
#include <windows.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Windows/Renderer/ext/glfw/deps/stb_image_write.h"

namespace Debug::SaveLoad
{
	namespace
	{
		std::map<std::filesystem::path, std::unique_ptr<ImTextureData>> thumbnails;
		constexpr int thumbnailWidth = 320;
		constexpr int maxHeight = 640;
	}

	static std::vector<char> CaptureScreenshotImpl(bool fullResolution)
	{
		if (Renderer::gHeadless || !GetDevice()) return {};
		const auto extent = Renderer::Native::GetFrameBufferSize();
		const auto source = Renderer::Native::GetFrameBuffer().colorImage;
		const auto format = GetSwapchainImageFormat();
		const bool bgra = format == VK_FORMAT_B8G8R8A8_SRGB || format == VK_FORMAT_B8G8R8A8_UNORM;
		if (!source || !extent.width || !extent.height) return {};
		if (!bgra && format != VK_FORMAT_R8G8B8A8_SRGB && format != VK_FORMAT_R8G8B8A8_UNORM)
			throw std::runtime_error("Unsupported screenshot framebuffer format");
		const int width = fullResolution ? int(extent.width) : thumbnailWidth;
		const int height = fullResolution ? int(extent.height) : std::clamp(int(uint64_t(width) * extent.height / extent.width), 1, maxHeight);
		const VkDeviceSize byteCount = VkDeviceSize(extent.width) * extent.height * 4;
		VulkanBuffer readback(byteCount, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		auto cmd = BeginSingleTimeCommands();
		VulkanImage::TransitionImageLayout(source, format, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		VkBufferImageCopy copy{};
		copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copy.imageExtent = { extent.width, extent.height, 1 };
		vkCmdCopyImageToBuffer(cmd, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.Get(), 1, &copy);
		VulkanImage::TransitionImageLayout(source, format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		EndSingleTimeCommands(cmd); // waits for the submitted copy before mapping

		BITMAPFILEHEADER file{};
		BITMAPINFOHEADER info{};
		file.bfType = 0x4d42;
		file.bfOffBits = sizeof(file) + sizeof(info);
		file.bfSize = file.bfOffBits + width * height * 4;
		info.biSize = sizeof(info);
		info.biWidth = width;
		info.biHeight = -height; // top-down
		info.biPlanes = 1;
		info.biBitCount = 32;
		info.biSizeImage = width * height * 4;
		std::vector<char> bytes(file.bfSize);
		std::memcpy(bytes.data(), &file, sizeof(file));
		std::memcpy(bytes.data() + sizeof(file), &info, sizeof(info));
		void* mapped = nullptr;
		if (vkMapMemory(GetDevice(), readback.Memory(), 0, byteCount, 0, &mapped) != VK_SUCCESS)
			throw std::runtime_error("Could not map screenshot pixels");
		const auto* pixels = static_cast<const unsigned char*>(mapped);
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				const auto* src = pixels + (size_t(y * extent.height / height) * extent.width + x * extent.width / width) * 4;
				auto* dst = bytes.data() + file.bfOffBits + (y * width + x) * 4;
				dst[0] = src[bgra ? 0 : 2]; dst[1] = src[1]; dst[2] = src[bgra ? 2 : 0]; dst[3] = char(255);
			}
		}
		vkUnmapMemory(GetDevice(), readback.Memory());
		return bytes;
	}

	std::vector<char> CaptureScreenshot()
	{
		return CaptureScreenshotImpl(false);
	}

	std::vector<char> CaptureScreenshotPng()
	{
		auto bitmap = CaptureScreenshotImpl(true);
		if (bitmap.empty()) return {};
		BITMAPINFOHEADER info;
		std::memcpy(&info, bitmap.data() + sizeof(BITMAPFILEHEADER), sizeof(info));
		auto* pixels = bitmap.data() + sizeof(BITMAPFILEHEADER) + sizeof(info);
		const int height = -info.biHeight;
		for (size_t i = 0; i < size_t(info.biWidth) * height; ++i) std::swap(pixels[i * 4], pixels[i * 4 + 2]);
		std::vector<char> png;
		auto write = [](void* context, void* data, int size) {
			auto& bytes = *static_cast<std::vector<char>*>(context);
			const auto* begin = static_cast<const char*>(data);
			bytes.insert(bytes.end(), begin, begin + size);
		};
		if (!stbi_write_png_to_func(write, &png, info.biWidth, height, 4, pixels, info.biWidth * 4))
			throw std::runtime_error("Could not encode screenshot PNG");
		return png;
	}

	void ClearScreenshots()
	{
		if (!thumbnails.empty()) vkDeviceWaitIdle(GetDevice());
		for (auto& entry : thumbnails) {
			if (!entry.second) continue;
			auto& texture = *entry.second;
			texture.WantDestroyNextFrame = true;
			texture.UnusedFrames = INT_MAX;
			texture.SetStatus(ImTextureStatus_WantDestroy);
			ImGui_ImplVulkan_UpdateTexture(&texture);
		}
		thumbnails.clear();
	}

	void DrawScreenshot(const std::filesystem::path& path)
	{
		auto [entry, inserted] = thumbnails.try_emplace(path);
		if (inserted) {
			std::ifstream input(path, std::ios::binary | std::ios::ate);
			const auto size = input.tellg();
			if (input && size >= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) &&
				size <= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + thumbnailWidth * maxHeight * 4) {
				BITMAPFILEHEADER file{};
				BITMAPINFOHEADER info{};
				input.seekg(0);
				input.read(reinterpret_cast<char*>(&file), sizeof(file));
				input.read(reinterpret_cast<char*>(&info), sizeof(info));
				if (file.bfType == 0x4d42 && file.bfOffBits == sizeof(file) + sizeof(info) &&
					info.biSize == sizeof(info) && info.biWidth == thumbnailWidth && info.biHeight < 0 &&
					info.biHeight >= -maxHeight && info.biPlanes == 1 && info.biBitCount == 32 && info.biCompression == BI_RGB &&
					size == file.bfOffBits + thumbnailWidth * -info.biHeight * 4) {
					auto texture = std::make_unique<ImTextureData>();
					texture->Create(ImTextureFormat_RGBA32, thumbnailWidth, -info.biHeight);
					if (input.read(reinterpret_cast<char*>(texture->Pixels), texture->GetSizeInBytes())) {
						for (int i = 0; i < texture->GetSizeInBytes(); i += 4) std::swap(texture->Pixels[i], texture->Pixels[i + 2]);
						ImGui_ImplVulkan_UpdateTexture(texture.get());
						entry->second = std::move(texture);
					}
				}
			}
		}
		if (entry->second) {
			const auto& texture = *entry->second;
			ImGui::Image(texture.GetTexID(), ImVec2(160, 160.0f * float(texture.Height) / float(texture.Width)));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Image(texture.GetTexID(), ImVec2(float(texture.Width), float(texture.Height)));
				ImGui::EndTooltip();
			}
		}
		else {
			ImGui::Dummy(ImVec2(160, 0));
			ImGui::TextDisabled("No screenshot");
		}
	}
}
