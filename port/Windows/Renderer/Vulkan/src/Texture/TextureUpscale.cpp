#include "TextureUpscale.h"
#include "TextureUpdate.h"
#include "TextureCache.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <future>
#include <mutex>
#include <unordered_set>
#include <vector>

#ifdef UPSCALING_ONNX_ENABLED
#include <onnxruntime_cxx_api.h>
#endif

#ifdef UPSCALING_ENABLED
#include "net.h"
#endif

namespace
{
	// -------------------------------------------------------------------------
	// Per-backend state
	// -------------------------------------------------------------------------

#ifdef UPSCALING_ONNX_ENABLED
	std::unique_ptr<Ort::Env>     gOrtEnv;
	std::unique_ptr<Ort::Session> gOrtSession;
	int                           gOnnxScaleFactor = 4;
	std::atomic<bool>             gOnnxReady{false};
	std::future<void>             gOnnxLoadFuture;
#endif

#ifdef UPSCALING_ENABLED
	ncnn::Net         gNet;
	std::atomic<bool> gNetLoaded{false};
	int               gNcnnScaleFactor = 4;
	std::future<void> gNcnnLoadFuture;
#endif

	// -------------------------------------------------------------------------
	// Shared job infrastructure
	// -------------------------------------------------------------------------

	std::mutex                                gInflightMutex;
	std::unordered_set<PS2::GSSimpleTexture*> gInflightTextures;

	std::mutex                     gFuturesMutex;
	std::vector<std::future<void>> gPendingFutures;

	void StoreFuture(std::future<void>&& f)
	{
		std::lock_guard<std::mutex> lock(gFuturesMutex);
		gPendingFutures.erase(
			std::remove_if(gPendingFutures.begin(), gPendingFutures.end(),
				[](std::future<void>& fut) {
					return fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
				}),
			gPendingFutures.end());
		gPendingFutures.push_back(std::move(f));
	}

	// Nearest-neighbour upscale of the alpha channel from src into an
	// already-sized dst buffer (alpha byte is at offset +3 per pixel).
	void UpscaleAlpha(
		const std::vector<uint8_t>& src, uint32_t srcW, uint32_t srcH,
		std::vector<uint8_t>&       dst, uint32_t dstW, uint32_t dstH)
	{
		for (uint32_t dy = 0; dy < dstH; ++dy)
		{
			const uint32_t sy = (dy * srcH) / dstH;
			for (uint32_t dx = 0; dx < dstW; ++dx)
			{
				const uint32_t sx              = (dx * srcW) / dstW;
				dst[(dy * dstW + dx) * 4 + 3] = src[(sy * srcW + sx) * 4 + 3];
			}
		}
	}

	// -------------------------------------------------------------------------
	// ONNX Runtime backend
	// -------------------------------------------------------------------------

#ifdef UPSCALING_ONNX_ENABLED
	void RunOnnxUpscale(
		Renderer::SimpleTexture* pTexture,
		PS2::GSSimpleTexture*    pRenderer,
		uint32_t                 srcWidth,
		uint32_t                 srcHeight,
		std::vector<uint8_t>     pixels)
	{
		const size_t pixelCount = static_cast<size_t>(srcWidth) * srcHeight;

		// RGBA8 HWC → float32 RGB NCHW, normalised [0, 1]
		std::vector<float> inputData(3 * pixelCount);
		for (size_t i = 0; i < pixelCount; ++i)
		{
			inputData[0 * pixelCount + i] = pixels[i * 4 + 0] / 255.0f;
			inputData[1 * pixelCount + i] = pixels[i * 4 + 1] / 255.0f;
			inputData[2 * pixelCount + i] = pixels[i * 4 + 2] / 255.0f;
		}

		const std::array<int64_t, 4> inputShape = {
			1, 3,
			static_cast<int64_t>(srcHeight),
			static_cast<int64_t>(srcWidth)
		};
		Ort::MemoryInfo memInfo    = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
		Ort::Value      inputTensor = Ort::Value::CreateTensor<float>(
			memInfo, inputData.data(), inputData.size(),
			inputShape.data(), inputShape.size());

		Ort::AllocatorWithDefaultOptions allocator;
		auto inputName  = gOrtSession->GetInputNameAllocated(0, allocator);
		auto outputName = gOrtSession->GetOutputNameAllocated(0, allocator);
		const char* inputNames[]  = { inputName.get() };
		const char* outputNames[] = { outputName.get() };

		auto outputTensors = gOrtSession->Run(
			Ort::RunOptions{ nullptr },
			inputNames,  &inputTensor, 1,
			outputNames, 1);

		const auto     outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
		const uint32_t dstHeight   = static_cast<uint32_t>(outputShape[2]);
		const uint32_t dstWidth    = static_cast<uint32_t>(outputShape[3]);
		const float*   outputData  = outputTensors[0].GetTensorMutableData<float>();
		const size_t   dstCount    = static_cast<size_t>(dstWidth) * dstHeight;

		// float32 NCHW → RGBA8 HWC; alpha via nearest-neighbour upscale
		std::vector<uint8_t> outPixels(dstCount * 4);
		for (size_t i = 0; i < dstCount; ++i)
		{
			outPixels[i * 4 + 0] = static_cast<uint8_t>(std::clamp(outputData[0 * dstCount + i], 0.0f, 1.0f) * 255.0f + 0.5f);
			outPixels[i * 4 + 1] = static_cast<uint8_t>(std::clamp(outputData[1 * dstCount + i], 0.0f, 1.0f) * 255.0f + 0.5f);
			outPixels[i * 4 + 2] = static_cast<uint8_t>(std::clamp(outputData[2 * dstCount + i], 0.0f, 1.0f) * 255.0f + 0.5f);
		}
		UpscaleAlpha(pixels, srcWidth, srcHeight, outPixels, dstWidth, dstHeight);

		Renderer::Native::RequestTextureUpdate(pTexture, dstWidth, dstHeight, std::move(outPixels));

		{
			std::lock_guard<std::mutex> lock(gInflightMutex);
			gInflightTextures.erase(pRenderer);
		}
	}
#endif // UPSCALING_ONNX_ENABLED

	// -------------------------------------------------------------------------
	// ncnn backend
	// -------------------------------------------------------------------------

#ifdef UPSCALING_ENABLED
	void RunNcnnUpscale(
		Renderer::SimpleTexture* pTexture,
		PS2::GSSimpleTexture*    pRenderer,
		uint32_t                 srcWidth,
		uint32_t                 srcHeight,
		std::vector<uint8_t>     pixels)
	{
		ncnn::Mat in = ncnn::Mat::from_pixels(
			pixels.data(),
			ncnn::Mat::PIXEL_RGBA2RGB,
			static_cast<int>(srcWidth),
			static_cast<int>(srcHeight));

		ncnn::Extractor ex = gNet.create_extractor();
		ex.input("data", in);

		ncnn::Mat out;
		ex.extract("output", out);

		const uint32_t dstWidth  = static_cast<uint32_t>(out.w);
		const uint32_t dstHeight = static_cast<uint32_t>(out.h);
		std::vector<uint8_t> outPixels(dstWidth * dstHeight * 4);
		out.to_pixels(outPixels.data(), ncnn::Mat::PIXEL_RGB2RGBA);
		UpscaleAlpha(pixels, srcWidth, srcHeight, outPixels, dstWidth, dstHeight);

		Renderer::Native::RequestTextureUpdate(pTexture, dstWidth, dstHeight, std::move(outPixels));

		{
			std::lock_guard<std::mutex> lock(gInflightMutex);
			gInflightTextures.erase(pRenderer);
		}
	}
#endif // UPSCALING_ENABLED

} // anonymous namespace

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void Renderer::Native::InitUpscalerONNX(const char* modelPath, int scaleFactor)
{
#ifdef UPSCALING_ONNX_ENABLED
	gOnnxScaleFactor = scaleFactor;

	const auto path = std::filesystem::path(modelPath);

	// Check it exists before trying to load, to avoid a long delay followed by an opaque error from ONNX Runtime.
	if (!std::filesystem::exists(path))
	{
		OutputDebugStringA(("Upscaling model not found at: " + path.string() + "\n").c_str());
		return;
	}

	// Load the model asynchronously so startup isn't blocked.
	// std::filesystem::path::c_str() returns wchar_t* on Windows, matching ORTCHAR_T.
	gOnnxLoadFuture = std::async(std::launch::async, [nativePath = path.wstring()]()
	{
		auto env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "TextureUpscale");

		Ort::SessionOptions sessionOptions;
		sessionOptions.SetIntraOpNumThreads(1);
		sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

		auto session = std::make_unique<Ort::Session>(*env, nativePath.c_str(), sessionOptions);

		gOrtEnv     = std::move(env);
		gOrtSession = std::move(session);
		gOnnxReady.store(true);
	});
#else
	(void)modelPath; (void)scaleFactor;
#endif
}

void Renderer::Native::InitUpscaler(const char* paramPath, const char* binPath, int scaleFactor)
{
#ifdef UPSCALING_ENABLED
	gNcnnScaleFactor = scaleFactor;

	// Load the model asynchronously so startup isn't blocked.
	gNcnnLoadFuture = std::async(std::launch::async,
		[param = std::string(paramPath), bin = std::string(binPath)]()
	{
		gNet.load_param(param.c_str());
		gNet.load_model(bin.c_str());
		gNetLoaded.store(true);
	});
#else
	(void)paramPath; (void)binPath; (void)scaleFactor;
#endif
}

bool Renderer::Native::IsUpscalerReady()
{
#ifdef UPSCALING_ONNX_ENABLED
	if (gOnnxReady) return true;
#endif
#ifdef UPSCALING_ENABLED
	if (gNetLoaded) return true;
#endif
	return false;
}

void Renderer::Native::UpscaleTexture(SimpleTexture* pTexture)
{
	assert(pTexture && pTexture->GetRenderer());

#if !defined(UPSCALING_ONNX_ENABLED) && !defined(UPSCALING_ENABLED)
	return;
#else
	// Determine which backend to use; ONNX takes priority over ncnn.
#if defined(UPSCALING_ONNX_ENABLED) && defined(UPSCALING_ENABLED)
	const bool bUseOnnx = gOnnxReady;
	if (!bUseOnnx && !gNetLoaded)
		return;
#elif defined(UPSCALING_ONNX_ENABLED)
	if (!gOnnxReady)
		return;
#else // UPSCALING_ENABLED only
	if (!gNetLoaded)
		return;
#endif

	PS2::GSSimpleTexture* pRenderer = pTexture->GetRenderer();
	{
		std::lock_guard<std::mutex> lock(gInflightMutex);
		if (!gInflightTextures.insert(pRenderer).second)
			return;
	}

	const TextureUpload::UploadBuffer& uploadBuffer = pRenderer->GetUploadBuffer();
	const uint32_t srcWidth  = uploadBuffer.Width();
	const uint32_t srcHeight = uploadBuffer.Height();
	std::vector<uint8_t> srcPixels(uploadBuffer.Get(), uploadBuffer.Get() + uploadBuffer.Size());

#if defined(UPSCALING_ONNX_ENABLED) && defined(UPSCALING_ENABLED)
	StoreFuture(std::async(std::launch::async,
		[pTexture, pRenderer, srcWidth, srcHeight, bUseOnnx, pixels = std::move(srcPixels)]() mutable
		{
			if (bUseOnnx)
				RunOnnxUpscale(pTexture, pRenderer, srcWidth, srcHeight, std::move(pixels));
			else
				RunNcnnUpscale(pTexture, pRenderer, srcWidth, srcHeight, std::move(pixels));
		}));
#elif defined(UPSCALING_ONNX_ENABLED)
	StoreFuture(std::async(std::launch::async,
		[pTexture, pRenderer, srcWidth, srcHeight, pixels = std::move(srcPixels)]() mutable
		{
			RunOnnxUpscale(pTexture, pRenderer, srcWidth, srcHeight, std::move(pixels));
		}));
#else
	StoreFuture(std::async(std::launch::async,
		[pTexture, pRenderer, srcWidth, srcHeight, pixels = std::move(srcPixels)]() mutable
		{
			RunNcnnUpscale(pTexture, pRenderer, srcWidth, srcHeight, std::move(pixels));
		}));
#endif
#endif // !defined(UPSCALING_ONNX_ENABLED) && !defined(UPSCALING_ENABLED)
}
