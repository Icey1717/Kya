#include "TextureUpscale.h"
#include "TextureUpdate.h"
#include "TextureCache.h"
#include "TextureUpscaleDiskCache.h"
#include "log.h"
#include "log_categories.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <windows.h>
#ifdef UPSCALING_ONNX_ENABLED
#include <onnxruntime_cxx_api.h>
#ifdef UPSCALING_DIRECTML_ENABLED
#include <dml_provider_factory.h>
#endif
#endif
#ifdef UPSCALING_ENABLED
#include "net.h"
#endif

namespace
{
	using namespace Renderer::Native;
	size_t Index(TextureUpscaleModel model) { return model == TextureUpscaleModel::SPAN ? 1 : 0; }
	const char* Name(TextureUpscaleModel model) { return model == TextureUpscaleModel::SPAN ? "SPAN" : "UltraSharp"; }
	struct Job {
		TextureUpdateTarget target;
		TextureUpscaleModel model;
		uint32_t width, height;
		std::string name;
		std::vector<uint8_t> pixels;
		bool readCache = false, cacheOnly = false;
	};
	struct Upscalers {
		std::mutex mutex;
		std::condition_variable wake;
		std::thread worker;
		bool stopping = false;
		std::deque<std::function<void()>> loads;
		std::deque<Job> jobs;
		struct Inflight { uint64_t revision; bool cacheOnly; };
		std::unordered_map<PS2::GSSimpleTexture*, Inflight> inflight;
		size_t queuedBytes = 0;
		std::array<TextureUpscaleStatus, 2> status;
		TextureUpscaleProgress progress;
		TextureUpscaleModel selected = TextureUpscaleModel::UltraSharp;
		bool loadCache = false;
		std::array<std::string, 2> modelHashes;
		std::array<uint32_t, 2> cacheScales{4, 4};
#ifdef UPSCALING_ONNX_ENABLED
		std::unique_ptr<Ort::Env> environment;
		std::array<std::unique_ptr<Ort::Session>, 2> sessions;
		std::array<int, 2> scales{4, 4};
#endif
#ifdef UPSCALING_ENABLED
		ncnn::Net net;
		bool ncnnReady = false;
		int ncnnScale = 4;
#endif
		~Upscalers() { Stop(); }
		void Start();
		void Stop() {
			{ std::lock_guard<std::mutex> lock(mutex); stopping = true; jobs.clear(); loads.clear(); }
			wake.notify_all();
			if (worker.joinable()) worker.join();
			std::lock_guard<std::mutex> lock(mutex);
			inflight.clear(); queuedBytes = 0;
#ifdef UPSCALING_ONNX_ENABLED
			for (auto& session : sessions) session.reset();
			environment.reset();
#endif
			status = {};
			modelHashes = {};
		}
	};
	Upscalers& GetUpscalers() {
		// Construct after logging so worker shutdown precedes logger teardown.
		LogCategories::TextureUpscale();
		static Upscalers instance;
		return instance;
	}
	void LogResult(const std::string& message) {
		MY_LOG_NAMED_CATEGORY(TextureUpscale, LogLevel::Info, "{}", message);
		OutputDebugStringA((message + "\n").c_str());
	}
	std::vector<uint8_t> Infer(const Job& job, uint32_t& width, uint32_t& height)
	{
		std::vector<uint8_t> output;
#ifdef UPSCALING_ONNX_ENABLED
		if (auto* session = GetUpscalers().sessions[Index(job.model)].get()) {
			const size_t count = size_t(job.width) * job.height;
			std::vector<float> input(count * 3);
			for (size_t i = 0; i < count; ++i)
				for (size_t channel = 0; channel < 3; ++channel)
					input[channel * count + i] = job.pixels[i * 4 + channel] / 255.0f;
			const std::array<int64_t, 4> shape{1, 3, job.height, job.width};
			auto memory = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
			auto tensor = Ort::Value::CreateTensor<float>(memory, input.data(), input.size(), shape.data(), shape.size());
			Ort::AllocatorWithDefaultOptions allocator;
			auto inputName = session->GetInputNameAllocated(0, allocator);
			auto outputName = session->GetOutputNameAllocated(0, allocator);
			const char* inputs[]{inputName.get()};
			const char* outputs[]{outputName.get()};
			auto tensors = session->Run(Ort::RunOptions{nullptr}, inputs, &tensor, 1, outputs, 1);
			const auto info = tensors[0].GetTensorTypeAndShapeInfo();
			const auto dimensions = info.GetShape();
			const int scale = GetUpscalers().scales[Index(job.model)];
			if (info.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT || dimensions.size() != 4 ||
				dimensions[0] != 1 || dimensions[1] != 3 || dimensions[2] != int64_t(job.height) * scale || dimensions[3] != int64_t(job.width) * scale)
				throw std::runtime_error("Expected float32 RGB NCHW output at the configured scale");
			width = static_cast<uint32_t>(dimensions[3]); height = static_cast<uint32_t>(dimensions[2]);
			const size_t outCount = size_t(width) * height;
			output.resize(outCount * 4);
			const float* values = tensors[0].GetTensorData<float>();
			for (size_t i = 0; i < outCount; ++i)
				for (size_t channel = 0; channel < 3; ++channel) {
					const float value = values[channel * outCount + i];
					if (!std::isfinite(value)) throw std::runtime_error("Model produced non-finite pixels");
					output[i * 4 + channel] = static_cast<uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
				}
		} else
#endif
		{
#ifdef UPSCALING_ENABLED
			if (job.model != TextureUpscaleModel::UltraSharp || !GetUpscalers().ncnnReady) throw std::runtime_error("Model is unavailable");
			auto input = ncnn::Mat::from_pixels(job.pixels.data(), ncnn::Mat::PIXEL_RGBA2RGB, job.width, job.height);
			auto extractor = GetUpscalers().net.create_extractor();
			ncnn::Mat result;
			if (extractor.input("data", input) || extractor.extract("output", result) || result.w != int(job.width) * GetUpscalers().ncnnScale || result.h != int(job.height) * GetUpscalers().ncnnScale)
				throw std::runtime_error("ncnn inference failed or returned an unexpected scale");
			width = result.w; height = result.h;
			output.resize(size_t(width) * height * 4);
			result.to_pixels(output.data(), ncnn::Mat::PIXEL_RGB2RGBA);
#else
			throw std::runtime_error("ONNX upscaling was not enabled at build time");
#endif
		}
		for (uint32_t y = 0; y < height; ++y)
			for (uint32_t x = 0; x < width; ++x)
				output[(size_t(y) * width + x) * 4 + 3] = job.pixels[(size_t(y * job.height / height) * job.width + x * job.width / width) * 4 + 3];
		return output;
	}
	void Upscalers::Start()
	{
		// Caller holds mutex. One worker bounds inference concurrency and obeys
		// DirectML's restriction on concurrent Run calls on the same session.
		if (worker.joinable()) return;
		stopping = false;
		worker = std::thread([this] {
			for (;;) {
				Job job{};
				std::function<void()> load;
				{
					std::unique_lock<std::mutex> lock(mutex);
					wake.wait(lock, [this] { return stopping || !loads.empty() || !jobs.empty(); });
					if (stopping) return;
					if (!loads.empty()) { load = std::move(loads.front()); loads.pop_front(); }
					else { job = std::move(jobs.front()); jobs.pop_front(); queuedBytes -= job.pixels.size(); }
				}
				if (load) { load(); continue; }
				bool failed = false;
				bool cacheHit = false, cacheMiss = false, cacheError = false;
				std::string message;
				try {
					if (job.target.IsCurrent()) {
						uint32_t width = 0, height = 0;
						std::vector<uint8_t> pixels;
						std::string key;
						std::filesystem::path directory;
						try {
							key = UpscaleDiskCache::Key(modelHashes[Index(job.model)], job.width, job.height, cacheScales[Index(job.model)], job.pixels);
							directory = UpscaleDiskCache::Directory();
							if (job.readCache) {
								width = job.width * cacheScales[Index(job.model)]; height = job.height * cacheScales[Index(job.model)];
								cacheHit = UpscaleDiskCache::Read(directory, key, width, height, pixels);
								cacheMiss = !cacheHit;
								}
						} catch (const std::exception& error) { cacheError = true; key.clear(); LogResult(std::string("Cache read unavailable: ") + error.what()); }
						if (!job.cacheOnly || cacheHit) {
							if (!cacheHit) {
								pixels = Infer(job, width, height);
								if (!key.empty()) {
									try { UpscaleDiskCache::Write(directory, key, width, height, pixels); }
									catch (const std::exception& error) { cacheError = true; LogResult(std::string("Cache write failed: ") + error.what()); }
								}
							}
							RequestTextureUpdate(job.target, width, height, std::move(pixels));
							message = std::string(Name(job.model)) + (cacheHit ? " (cache): " : ": ") + job.name + " " + std::to_string(job.width) + "x" + std::to_string(job.height) + " -> " + std::to_string(width) + "x" + std::to_string(height);
						}
					}
				} catch (const std::exception& error) { failed = true; message = std::string(Name(job.model)) + ": " + error.what(); }
				{
					std::lock_guard<std::mutex> lock(mutex);
					const auto inflightJob = inflight.find(job.target.pTexture);
					if (inflightJob != inflight.end() && inflightJob->second.revision == job.target.revision)
						inflight.erase(inflightJob);
					if (failed) ++progress.failed; else if (!message.empty()) ++progress.completed;
					if (cacheHit) ++progress.cacheHits;
					if (cacheMiss) ++progress.cacheMisses;
					if (cacheError) ++progress.cacheErrors;
					if (!message.empty()) progress.lastResult = message;
				}
				if (!message.empty()) LogResult(message);
			}
		});
	}
	std::filesystem::path ModelDirectory()
	{
		wchar_t executable[32768];
		const DWORD length = GetModuleFileNameW(nullptr, executable, 32768);
		const auto directory = std::filesystem::path(std::wstring(executable, length)).parent_path();
		const auto packaged = directory / "upscaling" / "models";
		if (std::filesystem::exists(packaged)) return packaged;
		return directory / ".." / ".." / "upscaling" / "models";
	}
}

void Renderer::Native::InitTextureUpscalers()
{
	const auto directory = ModelDirectory();
	const auto ultra = (directory / "4x-UltraSharpV2_fp32_op17.onnx").string();
	const auto span = (directory / "4xNomosUni_span_multijpg_fp32_opset17.onnx").string();
	InitUpscalerONNX(ultra.c_str(), 4, TextureUpscaleModel::UltraSharp);
	InitUpscalerONNX(span.c_str(), 4, TextureUpscaleModel::SPAN);
}
void Renderer::Native::InitUpscalerONNX(const char* modelPath, int scaleFactor, TextureUpscaleModel model)
{
	std::lock_guard<std::mutex> lock(GetUpscalers().mutex);
	auto& state = GetUpscalers().status[Index(model)];
#ifdef UPSCALING_ONNX_ENABLED
	if (state.loading || state.ready) return;
	state.loading = true; state.error.clear();
	const auto path = std::filesystem::path(modelPath);
	GetUpscalers().Start();
	GetUpscalers().loads.push_back([path, scaleFactor, model] {
		TextureUpscaleStatus result;
		try {
			if (!std::filesystem::is_regular_file(path)) throw std::runtime_error("Model file missing: " + path.string());
			if (scaleFactor < 1 || scaleFactor > 8) throw std::runtime_error("Invalid scale factor");
			const auto modelHash = UpscaleDiskCache::HashFile(path);
			if (!GetUpscalers().environment) GetUpscalers().environment = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "TextureUpscale");
			auto create = [&](bool gpu) {
				Ort::SessionOptions options;
				options.SetIntraOpNumThreads(4);
				options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
#ifdef UPSCALING_DIRECTML_ENABLED
				if (gpu) {
					options.DisableMemPattern(); options.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
					Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(options, 0));
				}
#endif
				return std::make_unique<Ort::Session>(*GetUpscalers().environment, path.c_str(), options);
			};
			std::unique_ptr<Ort::Session> session;
#ifdef UPSCALING_DIRECTML_ENABLED
			try { session = create(true); result.backend = "DirectML GPU"; }
			catch (const std::exception& error) { LogResult(std::string(Name(model)) + " GPU unavailable, using CPU: " + error.what()); }
#endif
			if (!session) { session = create(false); result.backend = "CPU"; }
			// Keep TypeInfo alive while using its unowned tensor metadata.
			const auto type = session->GetInputTypeInfo(0);
			const auto input = type.GetTensorTypeAndShapeInfo();
			const auto shape = input.GetShape();
			if (input.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT || shape.size() != 4 || shape[1] != 3)
				throw std::runtime_error("Expected a float32 RGB NCHW model");
			GetUpscalers().scales[Index(model)] = scaleFactor;
			GetUpscalers().sessions[Index(model)] = std::move(session);
			GetUpscalers().modelHashes[Index(model)] = modelHash;
			GetUpscalers().cacheScales[Index(model)] = static_cast<uint32_t>(scaleFactor);
			result.ready = true;
			LogResult(std::string(Name(model)) + " ready (" + result.backend + ")");
		} catch (const std::exception& error) { result.error = error.what(); LogResult(std::string(Name(model)) + ": " + result.error); }
		std::lock_guard<std::mutex> lock(GetUpscalers().mutex);
		GetUpscalers().status[Index(model)] = std::move(result);
	});
	GetUpscalers().wake.notify_one();
#else
	(void)modelPath; (void)scaleFactor;
	state.error = "Enable ENABLE_UPSCALING_ONNX and rebuild";
#endif
}
void Renderer::Native::InitUpscaler(const char* paramPath, const char* binPath, int scaleFactor)
{
#ifdef UPSCALING_ENABLED
	std::lock_guard<std::mutex> lock(GetUpscalers().mutex);
	GetUpscalers().Start();
	GetUpscalers().loads.push_back([param = std::string(paramPath), bin = std::string(binPath), scaleFactor] {
		const bool ready = !GetUpscalers().net.load_param(param.c_str()) && !GetUpscalers().net.load_model(bin.c_str());
		std::string modelHash;
		if (ready) {
			try {
				const auto binHash = UpscaleDiskCache::HashFile(bin);
				modelHash = UpscaleDiskCache::Key(UpscaleDiskCache::HashFile(param), 16, 1, scaleFactor, std::vector<uint8_t>(binHash.begin(), binHash.end()));
			} catch (const std::exception& error) { LogResult(std::string("ncnn cache unavailable: ") + error.what()); }
		}
		std::lock_guard<std::mutex> lock(GetUpscalers().mutex);
		GetUpscalers().ncnnReady = ready; GetUpscalers().ncnnScale = scaleFactor;
		if (ready && !GetUpscalers().status[0].ready) {
			GetUpscalers().status[0] = {true, false, "ncnn", ""};
			GetUpscalers().modelHashes[0] = modelHash;
			GetUpscalers().cacheScales[0] = scaleFactor;
		}
	});
	GetUpscalers().wake.notify_one();
#else
	(void)paramPath; (void)binPath; (void)scaleFactor;
#endif
}
void Renderer::Native::ShutdownTextureUpscalers() { GetUpscalers().Stop(); }
Renderer::Native::TextureUpscaleStatus Renderer::Native::GetTextureUpscaleStatus(TextureUpscaleModel model) {
	std::lock_guard<std::mutex> lock(GetUpscalers().mutex); return GetUpscalers().status[Index(model)];
}
Renderer::Native::TextureUpscaleProgress Renderer::Native::GetTextureUpscaleProgress() {
	std::lock_guard<std::mutex> lock(GetUpscalers().mutex);
	auto result = GetUpscalers().progress; result.pending = GetUpscalers().inflight.size(); return result;
}
void Renderer::Native::SetTextureUpscaleModel(TextureUpscaleModel model) {
	std::lock_guard<std::mutex> lock(GetUpscalers().mutex); GetUpscalers().selected = model;
}
Renderer::Native::TextureUpscaleModel Renderer::Native::GetTextureUpscaleModel() {
	std::lock_guard<std::mutex> lock(GetUpscalers().mutex); return GetUpscalers().selected;
}
bool Renderer::Native::IsUpscalerReady() { return GetTextureUpscaleStatus(GetTextureUpscaleModel()).ready; }
void Renderer::Native::SetTextureUpscaleCacheLoading(bool enabled) {
	std::lock_guard<std::mutex> lock(GetUpscalers().mutex); GetUpscalers().loadCache = enabled;
}
namespace {
bool QueueTexture(Renderer::SimpleTexture* texture, bool cacheOnly)
{
	if (!texture || !texture->GetRenderer()) return false;
	std::lock_guard<std::mutex> lock(GetUpscalers().mutex);
	const auto model = GetUpscalers().selected;
	const auto inflight = GetUpscalers().inflight.find(texture->GetRenderer());
	// A user-requested Upscale supersedes a background cache-only probe, so
	// clicking Upscale during automatic cache loading cannot become a no-op.
	if (GetUpscalers().stopping || !GetUpscalers().status[Index(model)].ready ||
		(inflight != GetUpscalers().inflight.end() && (cacheOnly || !inflight->second.cacheOnly)) ||
		(cacheOnly && !GetUpscalers().loadCache)) return false;
	const auto* renderer = texture->GetRenderer();
	const auto& source = renderer->GetUploadBuffer();
	const uint32_t width = renderer->imageData.canvasWidth, height = renderer->imageData.canvasHeight;
	const size_t size = size_t(width) * height * 4;
	if (!width || !height || width > source.Width() || height > source.Height() || size > 8 * 1024 * 1024 ||
		GetUpscalers().queuedBytes + size > 128 * 1024 * 1024 || GetUpscalers().jobs.size() >= 2048) {
		GetUpscalers().progress.lastResult = "Texture queue is full or source dimensions are invalid"; return false;
	}
	Job job{CaptureTextureUpdateTarget(texture), model, width, height, texture->GetName(), std::vector<uint8_t>(size)};
	job.readCache = GetUpscalers().loadCache; job.cacheOnly = cacheOnly;
	for (uint32_t y = 0; y < height; ++y)
		memcpy(job.pixels.data() + size_t(y) * width * 4, source.Get() + size_t(y) * source.Width() * 4, size_t(width) * 4);
	GetUpscalers().inflight[texture->GetRenderer()] = {job.target.revision, job.cacheOnly}; GetUpscalers().queuedBytes += size;
	GetUpscalers().jobs.push_back(std::move(job)); GetUpscalers().wake.notify_one();
	return true;
}
}
void Renderer::Native::UpscaleTexture(SimpleTexture* texture) { QueueTexture(texture, false); }
bool Renderer::Native::LoadTextureUpscaleFromCache(SimpleTexture* texture) { return QueueTexture(texture, true); }
