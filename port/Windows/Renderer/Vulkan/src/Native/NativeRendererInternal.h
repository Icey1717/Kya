#pragma once

#include "NativeRenderer.h"

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
				return static_cast<uint32_t>(clearMode);
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
			}

			EClearMode clearMode = EClearMode::None;
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
		};

		struct FadeConstantBuffer
		{
			glm::vec4 fadeColor;
		};

		constexpr int gMaxInstances = 1024;
		constexpr int gMaxLightingData = 512;
		constexpr int gMaxAnimationMatrices = 4096;

		struct RenderStage
		{
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

			bool operator==(const LightingDynamicBufferData& other) const
			{
				for (int i = 0; i < 4; ++i) {
					if (!glm::all(glm::equal(lightDirection[i], other.lightDirection[i]))) return false;
					if (!glm::all(glm::equal(lightColor[i], other.lightColor[i]))) return false;
				}
				if (!glm::all(glm::equal(lightAmbient, other.lightAmbient))) return false;
				if (!glm::all(glm::equal(flare, other.flare))) return false;
				return true;
			}
		};

		using NativeVertexBuffer = PS2::FrameVertexBuffers<GSVertexUnprocessedNormal, uint16_t>;

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

			bool bIsAfailZOnly = false;
			bool bIsZMask = false;

			const VkDescriptorSet* pDescriptorSets = nullptr;
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

			bool forceAnimMatrixIdentity = false;
			SimpleTexture* whiteTexture = nullptr;

			VkSampler frameBufferSampler = VK_NULL_HANDLE;
			FrameBufferBase frameBuffer;
			std::unordered_map<RenderPassKey, RenderStage, RenderPassKeyHash> renderPass;
			VkCommandPool commandPool = VK_NULL_HANDLE;
			CommandBufferVector commandBuffers;

			StorageDynamicBuffer<glm::mat4, gMaxInstances> modelBuffer;
			UniformBuffer<FadeConstantBuffer> fadeBuffer;
			bool fadeActive = false;

			StorageDynamicBuffer<LightingDynamicBufferData, gMaxLightingData> lightingDynamicBuffer;
			StorageDynamicBuffer<glm::vec4, gMaxInstances> animStBuffer;
			NativeVertexBuffer nativeVertexBuffer;

			StorageBuffer<glm::mat4> animationBuffer;
			std::vector<glm::mat4> animationMatrices;

			RenderPassKey cachedRenderPassKey;
			bool renderPassDirty = true;
			RenderPassKey activeRenderPassKey;
			bool hasActiveRenderPass = false;

			PFN_vkCmdSetColorWriteEnableEXT vkCmdSetColorWriteEnableEXT = nullptr;
			PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT = nullptr;

			std::optional<Draw> currentDraw;

			glm::mat4 cachedViewMatrix = glm::mat4(1.0f);
			glm::mat4 cachedProjMatrix = glm::mat4(1.0f);
			glm::mat4 initialViewMatrix = glm::mat4(1.0f);
			glm::mat4 initialProjMatrix = glm::mat4(1.0f);

			PerDrawData cachedPerDrawData;
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
		void SetColorDepthDynamicState(const VkCommandBuffer& cmd, Draw& drawCommand);
		void ApplyPendingResizeInternal();
		void PushGlobalMatrices(float* pModel, float* pView, float* pProj);
		void PushModelMatrix(float* pModel);
		void StartAnimMatrix();
		void PushAnimMatrix(float* pAnim);
		void PushAnimST(float* pAnimST);

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
