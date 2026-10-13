#include <gtest/gtest.h>
#include "Texture/TextureUpscale.h"
#include "Texture/TextureUpdate.h"
#include "Texture/TextureCache.h"
#include "Texture/TextureUpscaleDiskCache.h"
#include "../../DebugMenu/src/DebugSetting.h"
#include "Objects/VulkanBuffer.h"
#include "Objects/VulkanImage.h"
#include "Objects/VulkanCommands.h"
#include "VulkanRenderer.h"
#include "Native/NativeRenderer.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <fstream>
#include <thread>
#include <cstdlib>

namespace Debug { extern const char* gSettingsFile; }

class TextureDiskCacheTest : public ::testing::Test
{
protected:
	std::filesystem::path root;
	void SetUp() override {
		root = std::filesystem::temp_directory_path() / ("kya-upscale-cache-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
		std::filesystem::create_directory(root);
	}
	void TearDown() override {
		// Only this test's freshly created flat directory is removed.
		std::error_code error;
		for (const auto& file : std::filesystem::directory_iterator(root)) std::filesystem::remove(file.path(), error);
		std::filesystem::remove(root, error);
	}
};

TEST_F(TextureDiskCacheTest, SeparatesModelSourceDimensionsScaleAndAlpha)
{
	using namespace Renderer::Native::UpscaleDiskCache;
	const std::string model(64, 'a'), other(64, 'b');
	const std::vector<uint8_t> source{1, 2, 3, 4, 5, 6, 7, 8};
	const auto key = Key(model, 2, 1, 4, source);
	EXPECT_EQ(key, Key(model, 2, 1, 4, source));
	EXPECT_NE(key, Key(other, 2, 1, 4, source));
	EXPECT_NE(key, Key(model, 1, 2, 4, source));
	EXPECT_NE(key, Key(model, 2, 1, 2, source));
	auto changed = source; changed[3] ^= 1;
	EXPECT_NE(key, Key(model, 2, 1, 4, changed));
	changed = source; changed[0] ^= 1;
	EXPECT_NE(key, Key(model, 2, 1, 4, changed));
	std::ofstream(root / "model.onnx", std::ios::binary) << "abc";
	EXPECT_EQ(HashFile(root / "model.onnx"), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST_F(TextureDiskCacheTest, ReloadsAndRejectsCorruptTruncatedAndMismatchedEntries)
{
	using namespace Renderer::Native::UpscaleDiskCache;
	const auto key = Key(std::string(64, 'a'), 1, 1, 4, {1, 2, 3, 4});
	std::vector<uint8_t> source(4 * 4 * 4, 77), result;
	EXPECT_FALSE(Read(root, key, 4, 4, result));
	Write(root, key, 4, 4, source);
	ASSERT_TRUE(Read(root, key, 4, 4, result)); EXPECT_EQ(result, source);
	EXPECT_FALSE(Read(root, key, 8, 2, result));
	EXPECT_FALSE(Read(root, "../invalid", 4, 4, result));
	const auto filePath = root / (key + ".rgba");
	{ std::fstream file(filePath, std::ios::binary | std::ios::in | std::ios::out); file.seekp(-1, std::ios::end); file.put(0); }
	EXPECT_FALSE(Read(root, key, 4, 4, result));
	Write(root, key, 4, 4, source); EXPECT_TRUE(Read(root, key, 4, 4, result));
	std::filesystem::resize_file(filePath, 12);
	EXPECT_FALSE(Read(root, key, 4, 4, result));
	EXPECT_THROW(Write(root, key, 4, 4, {1}), std::runtime_error);
	std::ofstream(root / "blocked") << "file";
	EXPECT_THROW(Write(root / "blocked", key, 4, 4, source), std::exception);
}

TEST_F(TextureDiskCacheTest, DebugSettingPersistsLoadFromCachePreference)
{
	struct RestoreSettings { const char* old; ~RestoreSettings() { Debug::gSettingsFile = old; } } restore{Debug::gSettingsFile};
	const auto file = (root / "settings.json").string(); Debug::gSettingsFile = file.c_str();
	Debug::Setting<bool> first("Load Upscaled Textures From Cache", false);
	EXPECT_FALSE(first.get()); first = true;
	Debug::Setting<bool> reloaded("Load Upscaled Textures From Cache", false);
	EXPECT_TRUE(reloaded.get()); reloaded = false;
	Debug::Setting<bool> disabled("Load Upscaled Textures From Cache", true);
	EXPECT_FALSE(disabled.get());
}

TEST(TextureUpdates, RejectsUnloadedAndSupersededTargets)
{
	using namespace Renderer::Native;
	auto state = std::make_shared<Renderer::TextureUpdateState>();
	TextureUpdateTarget target{reinterpret_cast<PS2::GSSimpleTexture*>(uintptr_t(1)), state, 0};
	EXPECT_TRUE(target.IsCurrent());
	++state->revision;
	EXPECT_FALSE(target.IsCurrent());
	target.revision = 1;
	EXPECT_TRUE(target.IsCurrent());
	state.reset();
	EXPECT_FALSE(target.IsCurrent());
}

namespace
{
	bool WaitFor(const std::function<bool()>& predicate)
	{
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
		while (!predicate() && std::chrono::steady_clock::now() < deadline)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		return predicate();
	}
	std::vector<uint8_t> ReadTexture(PS2::GSSimpleTexture* texture)
	{
		VulkanBuffer buffer(size_t(texture->width) * texture->height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		const auto cmd = BeginSingleTimeCommands();
		VulkanImage::TransitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		VkBufferImageCopy copy{};
		copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
		copy.imageExtent = {texture->width, texture->height, 1};
		vkCmdCopyImageToBuffer(cmd, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.Get(), 1, &copy);
		VulkanImage::TransitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		EndSingleTimeCommands(cmd);
		void* mapped = nullptr;
		if (vkMapMemory(GetDevice(), buffer.Memory(), 0, VK_WHOLE_SIZE, 0, &mapped) != VK_SUCCESS)
			throw std::runtime_error("Texture readback mapping failed");
		std::vector<uint8_t> pixels(size_t(texture->width) * texture->height * 4);
		memcpy(pixels.data(), mapped, pixels.size());
		vkUnmapMemory(GetDevice(), buffer.Memory());
		return pixels;
	}
}

// Requires local game data: see tools/upscale_compare/in-game.md. Run alone.
TEST(TextureUpscaleRendering, DISABLED_BothModelsUploadAndRevertRealGameTexture)
{
	using namespace Renderer;
	using namespace Renderer::Native;
	std::ifstream file("../../out/upscale-compare/selected/CMP_ICONS_M0_L0.rgba", std::ios::binary);
	if (!file) GTEST_SKIP() << "Export the real game texture fixture first";
	uint32_t width = 0, height = 0;
	file.read(reinterpret_cast<char*>(&width), 4); file.read(reinterpret_cast<char*>(&height), 4);
	ASSERT_GT(width, 0u); ASSERT_GT(height, 0u); ASSERT_LE(width, 512u); ASSERT_LE(height, 512u);
	std::vector<uint8_t> original(size_t(width) * height * 4);
	ASSERT_TRUE(file.read(reinterpret_cast<char*>(original.data()), original.size()));
	ASSERT_TRUE(glfwInit()); glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Renderer::SetHeadless(false); Renderer::Setup(); Renderer::Native::Setup();
	struct Cleanup { ~Cleanup() { ShutdownTextureUpscalers(); Renderer::Native::Cleanup(); } } cleanup;
	for (auto model : {TextureUpscaleModel::UltraSharp, TextureUpscaleModel::SPAN}) {
		ASSERT_TRUE(WaitFor([&] { auto status = GetTextureUpscaleStatus(model); return !status.loading; }));
		const auto status = GetTextureUpscaleStatus(model);
		ASSERT_TRUE(status.ready) << status.error;
		std::cout << (model == TextureUpscaleModel::SPAN ? "SPAN" : "UltraSharp") << " backend: " << status.backend << '\n';
	}
	ImageData bitmap{}; bitmap.canvasWidth = width; bitmap.canvasHeight = height; bitmap.pImage = original.data();
	SimpleTexture texture("Real game CMP_ICONS", {}, {}); texture.CreateRenderer(bitmap);
	auto* renderer = texture.GetRenderer();
	const auto binding = renderer->GetOrCreateTextureBinding(PS2::GetPreviewSamplerDescription(true));
	const bool cacheOnlyVerification = std::getenv("KYA_TEST_UPSCALE_CACHE_ONLY") != nullptr;
	for (auto model : {TextureUpscaleModel::UltraSharp, TextureUpscaleModel::SPAN}) {
		SetTextureUpscaleCacheLoading(cacheOnlyVerification);
		SetTextureUpscaleModel(model);
		const auto initialHits = GetTextureUpscaleProgress().cacheHits;
		if (cacheOnlyVerification) ASSERT_TRUE(LoadTextureUpscaleFromCache(&texture));
		else { EXPECT_FALSE(LoadTextureUpscaleFromCache(&texture)); UpscaleTexture(&texture); }
		ASSERT_TRUE(WaitFor([] { return GetTextureUpscaleProgress().pending == 0; }));
		ASSERT_EQ(GetTextureUpscaleProgress().failed, 0u);
		DrainPendingTextureUpdates();
		if (cacheOnlyVerification) EXPECT_EQ(GetTextureUpscaleProgress().cacheHits, initialHits + 1);
		ASSERT_EQ(renderer->width, width * 4); ASSERT_EQ(renderer->height, height * 4);
		EXPECT_EQ(renderer->GetOrCreateTextureBinding(PS2::GetPreviewSamplerDescription(true)), binding);
		const auto pixels = ReadTexture(renderer);
		size_t different = 0;
		for (uint32_t y = 0; y < height * 4; ++y)
			for (uint32_t x = 0; x < width * 4; ++x) {
				const size_t offset = (size_t(y) * width * 4 + x) * 4;
				const size_t source = (size_t(y / 4) * width + x / 4) * 4;
				ASSERT_EQ(pixels[offset + 3], original[source + 3]);
				if (memcmp(pixels.data() + offset, original.data() + source, 3)) ++different;
			}
		EXPECT_GT(different, size_t(width) * height);
		RevertTexture(&texture); DrainPendingTextureUpdates();
		ASSERT_EQ(renderer->width, width); ASSERT_EQ(renderer->height, height);
		EXPECT_EQ(ReadTexture(renderer), original);
		const auto hits = GetTextureUpscaleProgress().cacheHits;
		SetTextureUpscaleCacheLoading(true);
		ASSERT_TRUE(LoadTextureUpscaleFromCache(&texture));
		ASSERT_TRUE(WaitFor([] { return GetTextureUpscaleProgress().pending == 0; }));
		DrainPendingTextureUpdates();
		EXPECT_EQ(GetTextureUpscaleProgress().cacheHits, hits + 1);
		EXPECT_EQ(ReadTexture(renderer), pixels);
		RevertTexture(&texture); DrainPendingTextureUpdates();
		UpscaleTexture(&texture);
		ASSERT_TRUE(WaitFor([] { return GetTextureUpscaleProgress().pending == 0; }));
		DrainPendingTextureUpdates();
		EXPECT_EQ(GetTextureUpscaleProgress().cacheHits, hits + 2);
		EXPECT_EQ(ReadTexture(renderer), pixels);
		RevertTexture(&texture); DrainPendingTextureUpdates();
	}
	// Cache-only misses never trigger inference or change the GPU image.
	auto changed = original;
	const auto nonce = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
	for (size_t i = 0; i < 8; ++i) changed[(i / 3) * 4 + i % 3] ^= static_cast<uint8_t>(nonce >> (i * 8));
	bitmap.pImage = changed.data();
	SimpleTexture missing("Cache-only miss", {}, {}); missing.CreateRenderer(bitmap);
	const auto completed = GetTextureUpscaleProgress().completed;
	const auto misses = GetTextureUpscaleProgress().cacheMisses;
	ASSERT_TRUE(LoadTextureUpscaleFromCache(&missing));
	ASSERT_TRUE(WaitFor([] { return GetTextureUpscaleProgress().pending == 0; }));
	DrainPendingTextureUpdates();
	EXPECT_EQ(GetTextureUpscaleProgress().completed, completed);
	EXPECT_EQ(GetTextureUpscaleProgress().cacheMisses, misses + 1);
	EXPECT_EQ(ReadTexture(missing.GetRenderer()), changed);
	// Clicking Upscale during a cache-only miss still performs inference.
	ASSERT_TRUE(LoadTextureUpscaleFromCache(&missing));
	UpscaleTexture(&missing);
	ASSERT_TRUE(WaitFor([] { return GetTextureUpscaleProgress().pending == 0; }));
	DrainPendingTextureUpdates();
	EXPECT_EQ(missing.GetRenderer()->width, width * 4);
	EXPECT_EQ(missing.GetRenderer()->height, height * 4);
	missing.GetRenderer()->DestroyImageResources();
	// Revert wins even if inference is still running or waiting in the queue.
	UpscaleTexture(&texture); RevertTexture(&texture);
	ASSERT_TRUE(WaitFor([] { return GetTextureUpscaleProgress().pending == 0; }));
	DrainPendingTextureUpdates();
	EXPECT_EQ(ReadTexture(renderer), original);
	renderer->DestroyImageResources();
}
