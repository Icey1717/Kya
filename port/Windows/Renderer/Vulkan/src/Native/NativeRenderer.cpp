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
#define DEBUG_TEXTURE_NAME "LINER_LAVA_TRAIL_TEXTURE.G2D (m: 0 l: 0)"

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
			void Init()
			{
				Validate();

				gUniformBuffer.Init(MaxInstances);
			}

			VkDescriptorBufferInfo GetDescBufferInfo(const int index)
			{
				return gUniformBuffer.GetDescBufferInfo(index);
			}

			uint32_t GetDynamicAlignment() const
			{
				return gUniformBuffer.GetDynamicAlignment();
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

			template<typename T>
			bool MatchesLastInstance(const T& data) const
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

				assert(currentInstanceIndex < MaxInstances);

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

			// GPU Side Dynamic Buffer
			DynamicUniformBuffer<T> gUniformBuffer;
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

		// Specialize std::hash for RenderPassKey
		template <>
		struct std::hash<RenderPassKey>
		{
			std::size_t operator()(const RenderPassKey& k) const noexcept
			{
				return std::hash<uint32_t>{}(k.GetKey());
			}
		};

		struct PerDrawData
		{
			glm::mat4 projXView;
			uint32_t renderFlags = 0;
		};

		struct FadeConstantBuffer
		{
			glm::vec4 fadeColor;
		};

		// Maximum number of instances we can draw in a single frame. (Meshes are split into instances for rendering)
		constexpr int gMaxInstances = 1000;

		constexpr int gMaxLightingData = 200;
		constexpr int gMaxAnimStData = 1000;

		// Maximum number of meshes we can handle in a single frame.
		constexpr int gMaxMeshes = 256;

		static int gMaxAnimationMatrices = 0;

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

		static DynamicBuffer<glm::mat4, gMaxInstances> gModelBuffer;

		static UniformBuffer<FadeConstantBuffer> gFadeBuffer;
		static bool bFadeActive = false;

		struct AlphaDynamicBufferData
		{
			alignas(4) VkBool32 enable; // Use VkBool32 for proper alignment
			alignas(4) int atst;
			alignas(4) int aref;
			alignas(4) int afail;

			AlphaDynamicBufferData(const TextureRegisters& textureRegisters)
				: enable(textureRegisters.test.ATE)
				, atst(textureRegisters.test.ATST)
				, aref(textureRegisters.test.AREF)
				, afail(textureRegisters.test.AFAIL)
			{ }
		};

		DynamicUniformBuffer<AlphaDynamicBufferData> gAlphaBuffer;

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

		static_assert(alignof(LightingDynamicBufferData) >= 16, "Alignment must be at least 16");

		DynamicBuffer<LightingDynamicBufferData, gMaxLightingData> gLightingDynamicBuffer;
		DynamicBuffer<glm::vec4, gMaxInstances> gAnimStBuffer;

		using NativeVertexBuffer = PS2::FrameVertexBuffers<GSVertexUnprocessedNormal, uint16_t>;
		static NativeVertexBuffer gNativeVertexBuffer;

		// As each draw comes in this buffer is filled.
		static NativeVertexBufferData gNativeVertexBufferDataDraw;

		static DynamicUniformBuffer<glm::mat4> gAnimationBuffer;
		static std::vector<glm::mat4> gAnimationMatrices;

		RenderPassKey gCachedRenderPassKey;
		bool gRenderPassDirty = true;
		RenderPassKey gActiveRenderPassKey;
		bool gHasActiveRenderPass = false;

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

				// Updated when the data is pushed to the native renderer.
				int lightingDataIndex = 0;
				int animStDataIndex = 0;

				PerDrawData perDrawData;

				int modelMatrixIndex = 0;
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

	} // Native
} // Renderer

#include "NativeRendererRecording.inl"
#include "NativeRendererSetup.inl"
#include "NativeRendererSubmission.inl"
