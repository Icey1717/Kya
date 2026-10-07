#pragma once

#include "NativeRenderer.h"
#include "DrawTrace.h"
#include "FogDraw.h"

#include "VulkanRenderer.h"
#include "Objects/UniformBuffer.h"
#include "Texture/TextureCache.h"
#include "logging.h"
#include "log.h"

#include <array>
#include <cassert>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

#define DEBUG_TEXTURE_NAME "BOUCHON_Scene01_for_ilot_11_06.g2d (m: 0 l: 0)"
#define DEBUG_MESH_NAME "Sprite"

#define NATIVE_LOG(level, format, ...) MY_LOG_CATEGORY("NativeRenderer", level, format, ##__VA_ARGS__)
#define NATIVE_LOG_VERBOSE(level, format, ...)

namespace Renderer
{
	namespace Native
	{
		constexpr int gMaxAnimMatrices = 0x60;
		constexpr int gMaxStripIndex = 0x20;

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

			const T& GetInstanceData(const uint32_t index) const
			{
				assert(index < static_cast<uint32_t>(currentInstanceIndex));
				return *gStorageBuffer.GetInstancePtr(index);
			}

            const T& GetLastInstance() const
            {
                assert(currentInstanceIndex > 0);
                return *gStorageBuffer.GetInstancePtr(currentInstanceIndex - 1);
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

			void DestroyResources()
			{
				gStorageBuffer.DestroyResources();
			}

		private:
			int currentInstanceIndex = 0;
			StorageBuffer<T> gStorageBuffer;
		};

		struct RenderPassKey
		{
			static RenderPassKey Empty;

			uint32_t GetKey() const
			{
				return static_cast<uint32_t>(clearMode) |
					(static_cast<uint32_t>(kind) << 8) | (multisampled ? 1u << 16 : 0);
			}

			bool operator==(const RenderPassKey& other) const
			{
				return GetKey() == other.GetKey();
			}

			bool operator!=(const RenderPassKey& other) const
			{
				return !(*this == other);
			}

			void Reset()
			{
				clearMode = EClearMode::None;
				kind = ERenderPassKind::Main;
				multisampled = false;
			}

			EClearMode clearMode = EClearMode::None;
			ERenderPassKind kind = ERenderPassKind::Main;
			bool multisampled = false;
		};

		struct RenderPassKeyHash
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
			VkBool32 alphaEnable = VK_FALSE;
			int32_t  alphaAtst = 0;
			int32_t  alphaAref = 0;
			int32_t  alphaAfail = 0;
			uint32_t modelMatrixIndex = 0;
			uint32_t animStDataIndex = 0;
			uint32_t animMatrixStart = 0;
			uint32_t lightingDataIndex = 0;
			// One 32-bit word, read as globalAlpha by the GLSL push-constant block.
			uint32_t globalAlpha : 8 = 0x80;
			int32_t textureLodBias : 12 = 0; // Signed TEX1.K, in units of 1/16.
			uint32_t textureLodScale : 2 = 0;
			uint32_t textureFixedLod : 1 = 0;
			uint32_t textureMaxMipLevel : 3 = 0;
			uint32_t textureLodPadding : 5 = 0;
			uint32_t textureLodEnable : 1 = 0;
			uint32_t shadowProjectionIndex = 0;
			uint32_t frameBufferMode = 0; // 0: ordinary texture, 1: MODULATE, 2: DECAL
			// Ordinary materials: GS Q denominator coefficients (u*n+v).
			// Framebuffer materials: UV scale. These sampling modes are exclusive.
			glm::vec2 samplingParams{ 0.0f, 1.0f };
			uint32_t stripFlags = 0; // Authored geometry flags; distinct from VU renderFlags.
			uint32_t samplingPadding = 0;
		};
		static_assert(sizeof(PerDrawData) == 128);
		static_assert(offsetof(PerDrawData, shadowProjectionIndex) == 104);
		static_assert(offsetof(PerDrawData, stripFlags) == 120);
		static_assert(offsetof(PerDrawData, samplingParams) == 112);

		struct FadeConstantBuffer
		{
			glm::vec4 fadeColor;
		};

		constexpr int gMaxInstances = 1024;
		constexpr int gMaxLightingData = 512;
		constexpr int gMaxAnimationMatrices = 4096;

		struct RenderStage
		{
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
			ERenderPassKind kind = ERenderPassKind::Main;
			VkRenderPass gRenderPass = VK_NULL_HANDLE;

			PipelineCreateInfo<PipelineKey> gCreateInfo;
			Pipeline gPipeline;
			Renderer::Pipeline gDebugLinePipeline;
			std::unordered_map<uint16_t, VkPipeline> gBlendPipelines;

			void CreatePipeline();

			const Pipeline& GetPipeline() const
			{
				return gPipeline;
			}

			const Pipeline& GetDebugLinePipeline() const
			{
				return gDebugLinePipeline;
			}
		};

		struct alignas(16) LightingDynamicBufferData
		{
			glm::mat4 lightDirection;
			glm::mat4 lightColor;
			glm::vec4 lightAmbient;
			glm::vec4 flare;
			glm::mat4 environmentNormalTransform = glm::mat4(1.0f);
			glm::vec4 environmentCameraX = glm::vec4(0.0f);
			glm::vec4 environmentCameraY = glm::vec4(0.0f);

			bool operator==(const LightingDynamicBufferData& other) const
			{
				for (int i = 0; i < 4; ++i) {
					if (!glm::all(glm::equal(lightDirection[i], other.lightDirection[i]))) return false;
					if (!glm::all(glm::equal(lightColor[i], other.lightColor[i]))) return false;
					if (!glm::all(glm::equal(environmentNormalTransform[i], other.environmentNormalTransform[i]))) return false;
				}
				if (!glm::all(glm::equal(lightAmbient, other.lightAmbient))) return false;
				if (!glm::all(glm::equal(flare, other.flare))) return false;
				if (!glm::all(glm::equal(environmentCameraX, other.environmentCameraX))) return false;
				if (!glm::all(glm::equal(environmentCameraY, other.environmentCameraY))) return false;
				return true;
			}
		};
		static_assert(sizeof(LightingDynamicBufferData) == 256);
		static_assert(offsetof(LightingDynamicBufferData, environmentNormalTransform) == 160);
		static_assert(offsetof(LightingDynamicBufferData, environmentCameraX) == 224);
		static_assert(offsetof(LightingDynamicBufferData, environmentCameraY) == 240);

		using NativeVertexBuffer = PS2::FrameVertexBuffers<GSVertexUnprocessedNormal, uint16_t>;

		struct Draw
		{
			SimpleTexture* pTexture = nullptr;

			glm::mat4 projMatrix;
			glm::mat4 viewMatrix;
			std::optional<glm::mat4> gsProjection; // Retained for projection changes during preview replay.

			RenderPassKey renderPassKey;
			bool bRenderPassDirty = true;

			struct Instance {
                uint64_t traceSubmission = 0;
				SimpleMesh* pMesh = nullptr;
				std::vector<uint32_t> vertexColors;
				int indexStart = 0;
				int indexCount = 0;
				int vertexStart = 0;
				int animationMatrixStart = 0;

				GIFReg::GSAlpha gsAlpha = {};
				GIFReg::GSTest gsTest = {};
				GIFReg::GSTex1 gsTex1 = {};
				VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
				bool bIsZMask = false;
				PerDrawData perDrawData;
			};

			std::vector<Instance> instances;

			bool bIsAfailZOnly = false;
			std::optional<FrameBufferMaterialSettings> frameBufferMaterial;

			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		};

		class NativePreviewRenderer
		{
		public:
			void Setup(int width, int height, const VkRenderPass& renderPass);
			void RecordPass(Renderer::CommandBufferList& commandBufferList,
				std::unordered_map<RenderPassKey, RenderStage, RenderPassKeyHash>& renderPasses,
				NativeVertexBuffer& nativeVertexBuffer,
				PFN_vkCmdSetColorWriteEnableEXT vkCmdSetColorWriteEnableEXT,
				PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT);
			void SaveDraw(const Draw& draw);
			void ClearSavedDraws();
			void SetCamera(const float* viewMatrix, const float* projMatrix);
			void ClearCamera();
			bool IsSetup() const;
			const VkSampler& GetSampler() const;
			const VkImageView& GetColorImageView() const;

		private:
			bool setup = false;
			bool enabled = false;
			glm::mat4 viewMatrix = glm::mat4(1.0f);
			glm::mat4 projMatrix = glm::mat4(1.0f);
			FrameBufferBase frameBuffer;
			VkSampler frameBufferSampler = VK_NULL_HANDLE;
			CommandBufferVector commandBuffers;
			std::vector<Draw> savedDraws;
			int width = 512;
			int height = 512;
		};

		class RenderThread;

		struct NativeRendererState
		{
			double renderTime = 0.0;
			double renderWaitTime = 0.0;
			double alphaTestSlowPathTime = 0.0;
			// Accumulated while recording; published and reset after main/preview work completes.
			double accumulatedAlphaTestSlowPathTime = 0.0;

			bool forceAnimMatrixIdentity = false;
			SimpleTexture* whiteTexture = nullptr;

			VkDescriptorPool frameDescriptorPool = VK_NULL_HANDLE;
			std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> frameDescriptorSets{};
			VkSampler frameBufferSampler = VK_NULL_HANDLE;
			FrameBufferBase frameBuffer;
			std::unordered_map<RenderPassKey, RenderStage, RenderPassKeyHash> renderPass;
			VkCommandPool commandPool = VK_NULL_HANDLE;
			CommandBufferVector commandBuffers;

			StorageDynamicBuffer<glm::mat4, gMaxInstances> modelBuffer;
			UniformBuffer<FadeConstantBuffer> fadeBuffer;
			bool fadeActive = false;

			StorageDynamicBuffer<LightingDynamicBufferData, gMaxLightingData> lightingDynamicBuffer;
			glm::mat4 environmentNormalTransform = glm::mat4(1.0f);
			glm::vec4 environmentCameraX = glm::vec4(0.0f);
			glm::vec4 environmentCameraY = glm::vec4(0.0f);
			StorageDynamicBuffer<glm::vec4, gMaxInstances> animStBuffer;
			StorageDynamicBuffer<glm::mat4, gMaxInstances> shadowProjectionBuffer;
			NativeVertexBuffer nativeVertexBuffer;

			StorageBuffer<glm::mat4> animationBuffer;
			std::vector<glm::mat4> animationMatrices;

			RenderPassKey cachedRenderPassKey;
			bool renderPassDirty = true;
			bool msaaSceneEnded = false;
			RenderPassKey activeRenderPassKey;
			bool hasActiveRenderPass = false;

			PFN_vkCmdSetColorWriteEnableEXT vkCmdSetColorWriteEnableEXT = nullptr;
			PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT = nullptr;

			std::optional<Draw> currentDraw;

			glm::mat4 cachedViewMatrix = glm::mat4(1.0f);
			glm::mat4 cachedProjMatrix = glm::mat4(1.0f);
			std::optional<glm::mat4> cachedGsProjection;
			glm::mat4 initialViewMatrix = glm::mat4(1.0f);
			glm::mat4 initialProjMatrix = glm::mat4(1.0f);

			PerDrawData cachedPerDrawData;
			uint32_t shadowAlpha = 0x30;
			FrameBufferMaterialSettings frameBufferMaterial;
			int currentAnimMatrixIndex = 0;

			NativePreviewRenderer preview;

			RenderThread* renderThread = nullptr;
		};

		NativeRendererState& GetNativeRendererState();
		const glm::mat4& GetInitialViewMatrix();
		const glm::mat4& GetInitialProjMatrix();

		VkPipeline GetBlendPipeline(const RenderPassKey& key, const GIFReg::GSAlpha& alpha, bool bAlphaBlendEnabled);
		void RecordBeginRenderPass(const RenderPassKey& key);
		void RecordEndRenderPass();
		void RecordBeginCommandBuffer();
		void RecordEndCommandBuffer();
		void SetColorDepthDynamicState(const VkCommandBuffer& cmd, const Draw& drawCommand, const Draw::Instance& instance);
		void RecordAlphaTestedDraw(VkCommandBuffer cmd, VkPipelineLayout layout, const Draw& draw, const Draw::Instance& instance, const PerDrawData& data);
		void ApplyPendingResizeInternal();
		void PushGlobalMatrices(float* pModel, float* pView, float* pProj, const float* pGsProj = nullptr);
		void PushModelMatrix(float* pModel);
		void StartAnimMatrix();
		void PushAnimMatrix(float* pAnim);
		void PushAnimST(float* pAnimST);
		void AddRenderThreadShadowBegin(RenderThread* renderThread, const ShadowPassSettings& settings);
		void AddRenderThreadShadowBlur(RenderThread* renderThread);
		void AddRenderThreadShadowReceiver(RenderThread* renderThread, const ShadowReceiverViewport& viewport);
		void AddRenderThreadShadowEnd(RenderThread* renderThread);
		void AddRenderThreadFrameBufferCopy(RenderThread* renderThread, const RenderPassKey& key, bool clearPending);
		void AddRenderThreadFog(RenderThread* renderThread, const FogDraw& fog, const RenderPassKey& key, bool clearPending);
		void AddRenderThreadAntiAliasing(RenderThread* renderThread, const AntiAliasingDraw& aa, const RenderPassKey& key, bool clearPending);
		void AddRenderThreadFlare(RenderThread* renderThread, const FlareDraw& flare, const RenderPassKey& key, bool clearPending);

		RenderThread* CreateRenderThread();
		void DestroyRenderThread(RenderThread*& renderThread);
		void AddRenderThreadDraw(RenderThread* renderThread, const Draw& draw);
		bool GetRenderThreadHasRecordedCommands(RenderThread* renderThread);
		void ResetRenderThread(RenderThread* renderThread);
		void MainThreadEndCommands(RenderThread* renderThread);
		void SignalRenderThreadEndCommands(RenderThread* renderThread);
		double GetRenderThreadDuration(RenderThread* renderThread);
	}
}
