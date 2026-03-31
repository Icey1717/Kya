#include "NativeRenderer.h"

#include "Blending.h"
#include "NativeDebug.h"
#include "NativeDebugShapes.h"

#include <array>
#include <stdexcept>
#include <queue>
#include <windows.h>
#include <readerwriterqueue.h>

#include "VulkanRenderer.h"
#include "Objects/VulkanImage.h"
#include "Objects/VulkanRenderPass.h"
#include "Objects/UniformBuffer.h"
#include "Texture/TextureCache.h"
#include "glm/gtc/type_ptr.inl"
#include "logging.h"
#include "log.h"
#include "ScopedTimer.h"
#include "PostProcessing.h"
#include "NativeDisplayList.h"
#include <functional> // For std::hash
#include "profiling.h"

//#define DEBUG_TEXTURE_NAME "Dupe (m: 0 l: 0)"
#define DEBUG_TEXTURE_NAME "flam.g2d (m: 0 l: 0)"
#define DEBUG_MESH_NAME "Sprite"

#define NATIVE_LOG(level, format, ...) MY_LOG_CATEGORY("NativeRenderer", level, format, ##__VA_ARGS__)
#define NATIVE_LOG_VERBOSE(level, format, ...)

namespace Renderer
{
	namespace Native
	{
		static double gRenderTime = 0.0;
		static double gRenderWaitTime = 0.0;

		constexpr int gMaxAnimMatrices = 0x60;
		constexpr int gMaxStripIndex = 0x20;

		static bool bForceAnimMatrixIdentity = false;

		template<typename T, int MaxInstances>
		class DynamicBuffer
		{
		public:
			void Init(const uint32_t range = 1)
			{
				this->range = range;

				Validate();

				gUniformBuffer.Init(MaxInstances);
			}

			VkDescriptorBufferInfo GetDescBufferInfo(const int index)
			{
				return gUniformBuffer.GetDescBufferInfo(index);
			}

			void Map(const int index)
			{
				gUniformBuffer.Map(index);
			}

			int GetInstanceIndex() const
			{
				assert(currentInstanceIndex > 0);
				return currentInstanceIndex - 1;
			}

			int GetDebugIndex() const
			{
				return currentInstanceIndex;
			}

			template<typename InstanceDataType>
			bool MatchesLastInstance(const InstanceDataType& data) const
			{
				if (currentInstanceIndex == 0) {
					return false;
				}

				return data == *gUniformBuffer.GetInstancePtr(currentInstanceIndex - 1);
			}

			bool MatchesLastInstance(const glm::vec4& data) const
			{
				if (currentInstanceIndex == 0) {
					return false;
				}

				const glm::vec4& lastData = *gUniformBuffer.GetInstancePtr(currentInstanceIndex - 1);
				return glm::all(glm::equal(data, lastData));
			}

			bool MatchesLastInstance(const glm::mat4& data) const
			{
				if (currentInstanceIndex == 0) {
					return false;
				}

				const glm::mat4& lastData = *gUniformBuffer.GetInstancePtr(currentInstanceIndex - 1);
				for (int i = 0; i < 4; ++i) {
					if (!glm::all(glm::equal(data[i], lastData[i]))) {
						return false;
					}
				}

				return true;
			}

			void AddInstanceData(const T& data)
			{
				// Re-use the current instance index if it matches the data.
				if (MatchesLastInstance(data)) {
					return;
				}

				assert((currentInstanceIndex + (range - 1)) < MaxInstances);

				gUniformBuffer.SetInstanceData(currentInstanceIndex, data);
				currentInstanceIndex++;
				return;
			}

			void Reset()
			{
				currentInstanceIndex = 0;
			}

		private:
			void Validate()
			{
				VkPhysicalDeviceProperties properties{};
				vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);
				const VkDeviceSize maxUboSize = properties.limits.maxUniformBufferRange;

				assert(properties.limits.maxUniformBufferRange >= (sizeof(T) * MaxInstances));
			}

			int currentInstanceIndex = 0;

			uint32_t range = 0;

			// GPU Side Dynamic Buffer
			DynamicUniformBuffer<T> gUniformBuffer;
		};

		// SSBO-backed per-frame buffer with the same deduplication logic as DynamicBuffer.
		// Unlike DynamicBuffer, there is no dynamic-alignment padding; the GPU sees a plain array
		// and the draw index is supplied via push constants.
		template<typename T, int MaxInstances>
		class StorageDynamicBuffer
		{
		public:
			void Init()
			{
				gStorageBuffer.Init(MaxInstances);
			}

			VkDescriptorBufferInfo GetDescBufferInfo(const int frameIndex)
			{
				return gStorageBuffer.GetDescBufferInfo(frameIndex);
			}

			void Map(const int frameIndex)
			{
				gStorageBuffer.Map(frameIndex);
			}

			int GetInstanceIndex() const
			{
				assert(currentInstanceIndex > 0);
				return currentInstanceIndex - 1;
			}

			int GetDebugIndex() const
			{
				return currentInstanceIndex;
			}

			bool MatchesLastInstance(const glm::vec4& data) const
			{
				if (currentInstanceIndex == 0) return false;
				return glm::all(glm::equal(data, *gStorageBuffer.GetInstancePtr(currentInstanceIndex - 1)));
			}

			bool MatchesLastInstance(const glm::mat4& data) const
			{
				if (currentInstanceIndex == 0) return false;
				const glm::mat4& last = *gStorageBuffer.GetInstancePtr(currentInstanceIndex - 1);
				for (int i = 0; i < 4; ++i) {
					if (!glm::all(glm::equal(data[i], last[i]))) return false;
				}
				return true;
			}

			template<typename InstanceDataType>
			bool MatchesLastInstance(const InstanceDataType& data) const
			{
				if (currentInstanceIndex == 0) return false;
				return data == *gStorageBuffer.GetInstancePtr(currentInstanceIndex - 1);
			}

			void AddInstanceData(const T& data)
			{
				if (MatchesLastInstance(data)) return;
				assert(currentInstanceIndex < MaxInstances);
				gStorageBuffer.SetInstanceData(currentInstanceIndex, data);
				currentInstanceIndex++;
			}

			void Reset()
			{
				currentInstanceIndex = 0;
			}

		private:
			int currentInstanceIndex = 0;
			StorageBuffer<T> gStorageBuffer;
		};

		// Uniquely identifies a Vulkan render pass configuration.
		// Used as the key into gRenderPass (a map of pre-built RenderStages). All valid combinations are
		// created up-front at setup time, so a lookup always succeeds.
		// When the key changes between draws (bRenderPassDirty), the recorder ends the active pass and
		// begins a new one with the appropriate VkRenderPass for the new attachment load behaviour.
		struct RenderPassKey
		{
			// Default key used when no game draws were submitted this frame — clears both color and depth.
			static RenderPassKey Empty;

			// Packs all key fields into a single uint32 suitable for hashing and equality comparisons.
			uint32_t GetKey() const
			{
				union
				{
					uint32_t key = 0;

					struct
					{
						uint32_t clearMode : 2; // maps directly to EClearMode (None=0, Depth=1, ColorDepth=2, Color=3)
					};
				} key;

				key.clearMode = static_cast<uint32_t>(clearMode);

				return key.key;
			}

			bool operator==(const RenderPassKey& other) const {
				return GetKey() == other.GetKey();
			}

			bool operator!=(const RenderPassKey& other) const {
				return !(*this == other);
			}

			// Resets to EClearMode::None (continuation pass — no clearing).
			void Reset()
			{
				clearMode = EClearMode::None;
			}

			// Determines the VkAttachmentLoadOp for color and depth at pass begin.
			EClearMode clearMode = EClearMode::None;
		};

	} // Native
} // Renderer

// Specialised in namespace std, outside Renderer::Native, before gRenderPass is
// instantiated — placing it inside the Renderer::Native namespace would trigger
// a Clang/MSVC extension warning (template specialization not in enclosing namespace).
template <>
struct std::hash<Renderer::Native::RenderPassKey>
{
	std::size_t operator()(const Renderer::Native::RenderPassKey& k) const noexcept
	{
		return std::hash<uint32_t>{}(k.GetKey());
	}
};

namespace Renderer
{
	namespace Native
	{
		struct PerDrawData
		{
			glm::mat4 projXView;
			uint32_t renderFlags = 0;
			VkBool32 alphaEnable = VK_FALSE;
			int32_t  alphaAtst   = 0;
			int32_t  alphaAref   = 0;
			int32_t  alphaAfail  = 0;
			uint32_t modelMatrixIndex  = 0;
			uint32_t animStDataIndex   = 0;
			uint32_t animMatrixStart   = 0;
			uint32_t lightingDataIndex = 0;
		};

		struct FadeConstantBuffer
		{
			glm::vec4 fadeColor;
		};

		// Maximum number of instances we can draw in a single frame. (Meshes are split into instances for rendering)
		constexpr int gMaxInstances = 1000;

		constexpr int gMaxLightingData = 200;

		// Maximum number of animation matrices stored per frame across all animated actors.
		constexpr int gMaxAnimationMatrices = 4096;

		static VkSampler gFrameBufferSampler;

		static FrameBufferBase gFrameBuffer;

		struct RenderStage
		{
			VkRenderPass gRenderPass = VK_NULL_HANDLE;

			PipelineMap gPipelines;
			Renderer::Pipeline gDebugLinePipeline;

			void CreatePipeline()
			{
				PipelineKey key;
				key.options.bGlsl = true;
				key.options.bWireframe = false;
				key.options.topology = topologyTriangleList;
				PipelineCreateInfo<PipelineKey> createInfo{ "shaders/native.vert.spv" , "shaders/native.frag.spv", "", key };
				Renderer::Native::CreatePipeline(createInfo, gRenderPass, gPipelines[createInfo.key.key], "Native Previewer GLSL");
			}

			const Pipeline& GetPipeline() const
			{
				PipelineKey pipelineKey;
				pipelineKey.options.bWireframe = false;
				pipelineKey.options.bGlsl = true;
				pipelineKey.options.topology = topologyTriangleList;

				assert(gPipelines.find(pipelineKey.key) != gPipelines.end());
				return gPipelines.at(pipelineKey.key);
			}

			const Pipeline& GetDebugLinePipeline() const
			{
				return gDebugLinePipeline;
			}
		};

		static std::unordered_map<RenderPassKey, RenderStage> gRenderPass;
		static VkCommandPool gCommandPool;
		static CommandBufferVector gCommandBuffers;

		static StorageDynamicBuffer<glm::mat4, gMaxInstances> gModelBuffer;

		static UniformBuffer<FadeConstantBuffer> gFadeBuffer;
		static bool bFadeActive = false;

		struct alignas(16) LightingDynamicBufferData
		{
			glm::mat4 lightDirection;
			glm::mat4 lightColor;
			glm::vec4 lightAmbient;
			glm::vec4 flare;

			bool operator==(const LightingDynamicBufferData& other) const {
				// Compare mat4 by columns
				for (int i = 0; i < 4; ++i) {
					if (!glm::all(glm::equal(lightDirection[i], other.lightDirection[i]))) return false;
					if (!glm::all(glm::equal(lightColor[i], other.lightColor[i]))) return false;
				}
				// Compare vec4
				if (!glm::all(glm::equal(lightAmbient, other.lightAmbient))) return false;
				if (!glm::all(glm::equal(flare, other.flare))) return false;
				return true;
			}
		};

		static StorageDynamicBuffer<LightingDynamicBufferData, gMaxLightingData> gLightingDynamicBuffer;
		static StorageDynamicBuffer<glm::vec4, gMaxInstances> gAnimStBuffer;

		using NativeVertexBuffer = PS2::FrameVertexBuffers<GSVertexUnprocessedNormal, uint16_t>;
		static NativeVertexBuffer gNativeVertexBuffer;

		// As each draw comes in this buffer is filled.
		static NativeVertexBufferData gNativeVertexBufferDataDraw;

		static StorageBuffer<glm::mat4> gAnimationBuffer;
		static std::vector<glm::mat4> gAnimationMatrices;

		RenderPassKey gCachedRenderPassKey;
		bool gRenderPassDirty = true;
		RenderPassKey gActiveRenderPassKey;
		bool gHasActiveRenderPass = false;
		bool gHasSubmittedDebugLineDraw = false;

		RenderPassKey RenderPassKey::Empty = RenderPassKey{ EClearMode::ColorDepth };

		struct Draw
		{
			SimpleTexture* pTexture = nullptr;

			glm::mat4 projMatrix;
			glm::mat4 viewMatrix;

			RenderPassKey renderPassKey;
			bool bRenderPassDirty = true;

			struct Instance {
				SimpleMesh* pMesh = nullptr;
				int indexStart = 0;
				int indexCount = 0;
				int vertexStart = 0;
				int animationMatrixStart = 0;

				PerDrawData perDrawData;
			};

			std::vector<Instance> instances;

			// When true, is a fake second draw that only draws the Z buffer.
			bool bIsAfailZOnly = false;
			bool bIsZMask = false;

			const VkDescriptorSet* pDescriptorSets = nullptr;
		};

		static std::optional<Draw> gCurrentDraw;

		static glm::mat4 gCachedViewMatrix = glm::mat4(1.0f);
		static glm::mat4 gCachedProjMatrix = glm::mat4(1.0f);
		glm::mat4 gInitialViewMatrix = glm::mat4(1.0f);
		glm::mat4 gInitialProjMatrix = glm::mat4(1.0f);

		static PerDrawData gCachedPerDrawData;

		static int gCurrentAnimMatrixIndex = 0;

		// Actor preview: second-camera render into an offscreen framebuffer displayed in ImGui.
		static bool gPreviewSetup = false;
		static bool gPreviewEnabled = false;
		static glm::mat4 gPreviewViewMatrix = glm::mat4(1.0f);
		static glm::mat4 gPreviewProjMatrix = glm::mat4(1.0f);
		static FrameBufferBase gPreviewFrameBuffer;
		static VkSampler gPreviewFrameBufferSampler = VK_NULL_HANDLE;
		static CommandBufferVector gPreviewCommandBuffers;
		static std::vector<Draw> gSavedDraws;
		static int gPreviewWidth = 512;
		static int gPreviewHeight = 512;

	} // Native
} // Renderer

#include "NativeRendererRecording.inl"
#include "NativeRendererSetup.inl"
#include "NativeRendererSubmission.inl"
