#include "NativeRenderer.h"

#include "Blending.h"
#include "NativeDebug.h"

#include <array>
#include <stdexcept>
#include <queue>
#include <windows.h>
#include <readerwriterqueue.h>

#include "VulkanRenderer.h"
#include "renderer.h"
#include "UniformBuffer.h"
#include "TextureCache.h"
#include "glm/gtc/type_ptr.inl"
#include "logging.h"
#include "log.h"
#include "ScopedTimer.h"
#include "PostProcessing.h"
#include "NativeDisplayList.h"
#include <functional> // For std::hash
#include "profiling.h"

#define DEBUG_TEXTURE_NAME "BACKGROUND_SECT1_Scene01_Back_sect1.g2d (m: 1 l: 0)"

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

			void AddInstanceData(const T& data)
			{
				// Re-use the current instance index if it matches the data.
				if (currentInstanceIndex > 0 && data == *gUniformBuffer.GetInstancePtr(currentInstanceIndex - 1)) {
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

		struct RenderPassKey
		{
			static RenderPassKey Empty;

			uint32_t GetKey() const
			{
				union
				{
					uint32_t key = 0;

					struct
					{
						uint32_t clearMode : 2; // 0: Color, 1: Depth, 2: Color + Depth
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

			void Reset()
			{
				clearMode = EClearMode::None;
			}

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

		glm::mat4 gFinalViewMatrix;
		glm::mat4 gFinalProjMatrix;

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
		};

		static std::unordered_map<RenderPassKey, RenderStage> gRenderPass;
		static VkCommandPool gCommandPool;
		static CommandBufferVector gCommandBuffers;
		static CommandBufferVector gCommandBuffersDummy;

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
				return lightDirection == other.lightDirection &&
					lightColor == other.lightColor &&
					lightAmbient == other.lightAmbient && 
					flare == other.flare;
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

		RenderPassKey RenderPassKey::Empty = RenderPassKey{ EClearMode::ColorDepth };

		struct Draw
		{
			SimpleTexture* pTexture = nullptr;

			glm::mat4 projMatrix;
			glm::mat4 viewMatrix;

			RenderPassKey renderPassKey;

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
		};

		static std::optional<Draw> gCurrentDraw;

		static glm::mat4 gCachedViewMatrix;
		static glm::mat4 gCachedProjMatrix;

		static PerDrawData gCachedPerDrawData;

		static int gCurrentAnimMatrixIndex = 0;

		static void CreateFramebuffer()
		{
			gFrameBuffer.SetupBase({ gWidth, gHeight }, gRenderPass[RenderPassKey::Empty].gRenderPass, true);
		}

		double GetRenderTime()
		{
			return gRenderTime;
		}

		double GetRenderWaitTime()
		{
			return gRenderWaitTime;
		}

		static void FillIndexData(Draw::Instance& instance)
		{
			auto& vertexBufferData = instance.pMesh->GetVertexBufferData();

			instance.indexCount = vertexBufferData.GetIndexTail();
			instance.indexStart = gNativeVertexBuffer.GetDrawBufferData().GetIndexTail();
			instance.vertexStart = gNativeVertexBuffer.GetDrawBufferData().GetVertexTail();

			NATIVE_LOG_VERBOSE(LogLevel::Info, "FillIndexData Filled indexCount: {} indexStart: {} vertexStart: {}",
				instance.indexCount, instance.indexStart, instance.vertexStart);

			// Copy into the real buffer.
			gNativeVertexBuffer.MergeData(vertexBufferData);
		}

		static void MergeIndexData()
		{
			assert(gCurrentDraw->instances.size() > 0);
			NATIVE_LOG_VERBOSE(LogLevel::Info, "MergeIndexData Merging instance: {}", gCurrentDraw->instances.size() - 1);

			Draw::Instance& instance = gCurrentDraw->instances.back();
			instance.indexCount += gNativeVertexBufferDataDraw.GetIndexTail();

			NATIVE_LOG_VERBOSE(LogLevel::Info, "MergeIndexData Merging instance: {} indexCount: {} indexStart: {} vertexStart: {}",
				gCurrentDraw->instances.size() - 1, instance.indexCount, instance.indexStart, instance.vertexStart);

			// Copy into the real buffer.
			gNativeVertexBuffer.MergeData(gNativeVertexBufferDataDraw);
			gNativeVertexBufferDataDraw.ResetAfterDraw();
		}

		static void CreateFramebufferSampler()
		{
			VkSamplerCreateInfo samplerCreateInfo{};
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
			samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerCreateInfo.anisotropyEnable = VK_FALSE;
			samplerCreateInfo.maxAnisotropy = 1.0f;
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
			samplerCreateInfo.compareEnable = VK_FALSE;
			samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerCreateInfo.mipLodBias = 0.0f;
			samplerCreateInfo.minLod = 0.0f;
			samplerCreateInfo.maxLod = 0.0f;

			VkResult result = vkCreateSampler(GetDevice(), &samplerCreateInfo, GetAllocator(), &gFrameBufferSampler);
			if (result != VK_SUCCESS) {
				// Handle sampler creation failure
			}
		}

		class InstanceDataUpdate
		{
		public:
			void UpdateInstanceData(Draw& draw)
			{
				SimpleTexture* pTexture = draw.pTexture;

				if (!pTexture) {
					return;
				}

				TextureRegisters textureRegisters = pTexture->GetTextureRegisters();

				NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());

				if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
					pTexture->GetName();
				}

				if (pTexture->GetTextureRegisters().test.ATST == ATST_NEVER) {
					pTexture->GetName();
				}

				if (draw.bIsAfailZOnly) {
					static const uint32_t inverted_atst[] = { ATST_ALWAYS, ATST_NEVER, ATST_GEQUAL, ATST_GREATER, ATST_NOTEQUAL, ATST_LESS, ATST_LEQUAL, ATST_EQUAL };
					textureRegisters.test.ATST = inverted_atst[textureRegisters.test.ATST];
				}

				gAlphaBuffer.SetInstanceData(drawCommandIndex, { textureRegisters });
				drawCommandIndex++;
			}

			void Reset()
			{
				drawCommandIndex = 0;
			}

		private:
			int drawCommandIndex = 0;
		};

		// Updates GPU side memory (Static Uniform Buffers | Per Frame Data)
		static void MapStaticUniformBuffer()
		{

		}

		// Updates GPU side memory (Dynamic Uniform Buffers | Per Instance Data)
		static void MapDynamicUniformBuffers()
		{
			gModelBuffer.Map(GetCurrentFrame());
			gAlphaBuffer.Map(GetCurrentFrame());
			gLightingDynamicBuffer.Map(GetCurrentFrame());
			gAnimStBuffer.Map(GetCurrentFrame());

			for (int i = 0; i < gAnimationMatrices.size() ; i++) {
				if (bForceAnimMatrixIdentity) {
					gAnimationMatrices[i] = glm::mat4(1.0f);
				}

				gAnimationBuffer.SetInstanceData(i, gAnimationMatrices[i]);
			}

			gAnimationBuffer.Map(GetCurrentFrame());
		}


		static void RecordBeginRenderPass(const RenderPassKey& key)
		{
			const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

			const RenderStage& stage = gRenderPass[key];

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = stage.gRenderPass;
			renderPassInfo.framebuffer = gFrameBuffer.framebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { gWidth, gHeight };

			std::array<VkClearValue, 2> clearColors;
			clearColors[0] = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearColors[1] = { {0.0f, 0.0f } };
			renderPassInfo.clearValueCount = clearColors.size();
			renderPassInfo.pClearValues = clearColors.data();

			Renderer::Debug::BeginLabel(cmd, "Begin Pass 0x%x", key.GetKey());

			vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			const auto& pipeline = stage.GetPipeline();
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
		}

		static void RecordEndRenderPass()
		{
			const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];
			vkCmdEndRenderPass(cmd);

			Renderer::Debug::EndLabel(cmd);
		}

		static void SetColorDepthDynamicState(const VkCommandBuffer& cmd, Draw& drawCommand)
		{
			VkBool32 colorWriteEnable = VK_TRUE;
			VkBool32 depthWriteEnable = drawCommand.pTexture->GetTextureRegisters().test.AFAIL != AFAIL_FB_ONLY ? VK_TRUE : VK_FALSE;

			if (drawCommand.bIsAfailZOnly) {
				depthWriteEnable = VK_TRUE;
				colorWriteEnable = VK_FALSE;
			}

			// Depth.
			vkCmdSetDepthWriteEnable(cmd, depthWriteEnable);

			// Color.
			static auto pvkCmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)vkGetInstanceProcAddr(GetInstance(), "vkCmdSetColorWriteEnableEXT");
			assert(pvkCmdSetColorWriteEnableEXT);
			pvkCmdSetColorWriteEnableEXT(cmd, 1, &colorWriteEnable);

			std::array<VkBool32, 1> colorWriteMasks = {
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			};

			if (drawCommand.pTexture->GetTextureRegisters().test.AFAIL == AFAIL_RGB_ONLY) {
				// Enable only RGB channels (disable alpha write)
				colorWriteMasks[0] = {
					VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
				};
			}

			static auto pvkCmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)vkGetInstanceProcAddr(GetInstance(), "vkCmdSetColorWriteMaskEXT");
			assert(pvkCmdSetColorWriteMaskEXT);
			pvkCmdSetColorWriteMaskEXT(cmd, 0, colorWriteMasks.size(), colorWriteMasks.data());
		}

		class DrawCommandRecorder
		{
		public:
			void RecordDrawCommand(Draw& drawCommand)
			{
				if (!bInRenderPass || drawCommand.renderPassKey != currentRenderPassKey) {
					if (bInRenderPass) {
						const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];
						Debug::Reset(cmd);
						RecordEndRenderPass();
					}

					currentRenderPassKey = drawCommand.renderPassKey;

					RecordBeginRenderPass(currentRenderPassKey);

					bInRenderPass = true;
				}

				SimpleTexture* pTexture = drawCommand.pTexture;

				if (pTexture && !drawCommand.instances.empty()) {
					NATIVE_LOG_VERBOSE(LogLevel::Verbose, "RecordDrawCommand {}", pTexture->GetName());

					const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

					const Pipeline& pipeline = gRenderPass[currentRenderPassKey].GetPipeline();

					Debug::UpdateLabel(pTexture, cmd);

					PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

					std::optional<uint> primState;

					for (auto& instance : drawCommand.instances) {
						if (instance.indexCount == 0) {
							continue;
						}

						NATIVE_LOG(LogLevel::Verbose, "RecordDrawCommand: {} LD {} AST {}", pTexture->GetName(), instance.lightingDataIndex, instance.animStDataIndex);

						Renderer::Debug::BeginLabel(cmd, "%s", instance.pMesh->GetName().c_str());
						
						instance.perDrawData.projXView = drawCommand.projMatrix * drawCommand.viewMatrix;

						vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PerDrawData), &instance.perDrawData);

						if (!primState.has_value() || primState.value() != instance.pMesh->GetPrim().CMD) {
							primState = instance.pMesh->GetPrim().CMD;
							SetBlendingDynamicState(pTexture, instance.pMesh, cmd);
						}

						SetColorDepthDynamicState(cmd, drawCommand);

						std::array< uint32_t, 5> dynamicOffsets = {
							instance.modelMatrixIndex * gModelBuffer.GetDynamicAlignment(),
							instance.animationMatrixStart * gAnimationBuffer.GetDynamicAlignment(),
							instance.lightingDataIndex * gLightingDynamicBuffer.GetDynamicAlignment(),
							instance.animStDataIndex* gAnimStBuffer.GetDynamicAlignment(),
							drawCommandIndex * gAlphaBuffer.GetDynamicAlignment(),
						};

						vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &pTextureData->GetDescriptorSets(pipeline).GetSet(GetCurrentFrame()), dynamicOffsets.size(), dynamicOffsets.data());

						vkCmdDrawIndexed(cmd, static_cast<uint32_t>(instance.indexCount), 1, instance.indexStart, instance.vertexStart, 0);

						Renderer::Debug::EndLabel(cmd);

						instanceIndex++;
					}
				}

				drawCommandIndex++;
			}

			void Reset()
			{
				drawCommandIndex = 0;
				instanceIndex = 0;
				bInRenderPass = false;
				currentRenderPassKey.Reset();
			}

		private:
			int drawCommandIndex = 0;
			int instanceIndex = 0;
			bool bInRenderPass = false;
			RenderPassKey currentRenderPassKey;
		};

		VkDeviceSize CheckBufferSizes()
		{
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);
			const VkDeviceSize maxUboSize = properties.limits.maxUniformBufferRange;

			// Model
			assert(maxUboSize >= (sizeof(glm::mat4) * gMaxInstances));

			// Lighting Data
			assert(maxUboSize >= (sizeof(LightingDynamicBufferData) * gMaxLightingData));

			// Anim ST
			assert(maxUboSize >= (sizeof(glm::vec4) * gMaxAnimStData));

			assert(properties.limits.maxPushConstantsSize >= sizeof(PerDrawData));

			return maxUboSize;
		}

		static bool CanMergeMesh()
		{
			if (!gCurrentDraw) {
				return false;
			}

			if (gCurrentDraw->instances.empty()) {
				return false;
			}

			auto& instance = gCurrentDraw->instances.back();

			if (instance.animationMatrixStart != gAnimationMatrices.size()) {
				return false;
			}

			return true;
		}

		static void CreateDraw()
		{
			gCurrentDraw = Draw{};
			gCurrentDraw->renderPassKey = gCachedRenderPassKey;
		}

		static void RecordBeginCommandBuffer()
		{
			const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(cmd, &beginInfo);

			Renderer::Debug::BeginLabel(cmd, "Native Render");

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)gWidth;
			viewport.height = (float)gHeight;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmd, 0, 1, &viewport);

			const VkRect2D scissor = { {0, 0}, { gWidth, gHeight } };
			vkCmdSetScissor(cmd, 0, 1, &scissor);

			gNativeVertexBuffer.BindBuffers(cmd);
		}

		// Copy all our data to the GPU.
		static void MapBuffers()
		{
			MapStaticUniformBuffer();
			MapDynamicUniformBuffers();

			gNativeVertexBuffer.MapData();

			// Reset the index and vertex heads for the next frame.
			gNativeVertexBuffer.GetDrawBufferData().ResetAfterDraw();
		}

		static void RecordEndCommandBuffer()
		{
			const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

			Debug::Reset(cmd);

			RecordEndRenderPass();
		}

		class RenderThread
		{
		public:
			RenderThread()
			{
				thread = std::thread(&RenderThread::Run, this);

				// Set thread name
				SetThreadDescription(thread.native_handle(), L"RenderThread");
			}

			~RenderThread()
			{
				bShouldStop = true;
				cv.notify_all();  // Ensure the thread wakes up to exit
				thread.join();
			}

			void UpdateInstanceDataForDraws()
			{
				for (auto& draw : draws) {
					for (auto& instance : draw.instances) {
						FillIndexData(instance);
					}

					drawInstanceDataUpdate.UpdateInstanceData(draw);
				}
			}

			void RecordDrawCommands()
			{
				if (draws.size() == 0) {
					RecordBeginRenderPass(RenderPassKey::Empty);
					return;
				}

				for (auto& draw : draws) {
					drawCommandRecorder.RecordDrawCommand(draw);
				}
			}

			void BeginWaitingForCommands()
			{
				draws.clear();
				bRecordedCommands = true;
				bWaitingForCommands = true;
			}

			void Run()
			{
				while (!bShouldStop) {
					std::unique_lock<std::mutex> lock(mutex);
					cv.wait(lock, [this] { return !bWaitingForCommands || bShouldStop; });

					if (bShouldStop) break;

					timer.Start();

					// Begins render pass, binds pipeline, etc.
					RecordBeginCommandBuffer();

					UpdateInstanceDataForDraws();

					RecordDrawCommands();

					MapBuffers();

					// Ends renderpass etc. 
					RecordEndCommandBuffer();

					BeginWaitingForCommands();

					timer.End();
				}
			}

			void AddDraw(const Draw& draw)
			{
				std::lock_guard<std::mutex> lock(mutex);
				draws.emplace_back(draw);
			}

			bool IsComplete()
			{
				return bWaitingForCommands;
			}

			bool GetHasRecordedCommands()
			{
				return bRecordedCommands;
			}

			void Reset()
			{
				drawInstanceDataUpdate.Reset();
				drawCommandRecorder.Reset();

				bRecordedCommands = false;
			}

			double GetRenderThreadTime()
			{
				return timer.duration.count();
			}

			void SignalEndCommands()
			{
				{
					std::lock_guard<std::mutex> lock(mutex);
					bWaitingForCommands = false;
				}
				cv.notify_one();

				NATIVE_LOG(LogLevel::Info, "SignalEndCommands");
			}

		private:

			std::thread thread;
			std::vector<Draw> draws;
			InstanceDataUpdate drawInstanceDataUpdate;
			DrawCommandRecorder drawCommandRecorder;
			std::atomic<bool> bShouldStop = false;
			std::atomic<bool> bWaitingForCommands = true;
			std::atomic<bool> bRecordedCommands = false;

			std::mutex mutex;
			std::condition_variable cv;

			struct Timer
			{
				void Start()
				{
					start = std::chrono::high_resolution_clock::now();
				}

				void End()
				{
					auto end = std::chrono::high_resolution_clock::now();
					duration = end - start;
				}

				std::chrono::time_point<std::chrono::high_resolution_clock> start;
				std::chrono::duration<double, std::milli> duration;
			} timer;
		};

		std::unique_ptr<RenderThread> gRenderThread;

		double GetRenderThreadTime()
		{
			return gRenderThread->GetRenderThreadTime();
		}

		static void SanityCheck(SimpleMesh* pMesh)
		{
			auto& internalBuffer = pMesh->GetVertexBufferData();

			assert(internalBuffer.GetIndexTail() == gNativeVertexBufferDataDraw.GetIndexTail());
			assert(internalBuffer.GetVertexTail() == gNativeVertexBufferDataDraw.GetVertexTail());

			for (int i = 0; i < internalBuffer.GetIndexTail(); i++) {
				assert(internalBuffer.index.buff[i] == gNativeVertexBufferDataDraw.index.buff[i]);
			}

			for (int i = 0; i < internalBuffer.GetVertexTail(); i++) {
				{
					const auto& vtx = internalBuffer.vertex.buff[i];
					NATIVE_LOG(LogLevel::Info, "Renderer::Native::AddMesh Preprocess vertex: {}, (S: {} T: {} Q: {}) (R: {} G: {} B: {} A: {}) (X: {} Y: {} Z: {} Skip: {}) ({}, {}, {})\n",
						i, vtx.STQ.ST[0], vtx.STQ.ST[1], vtx.STQ.Q, vtx.RGBA[0], vtx.RGBA[1], vtx.RGBA[2], vtx.RGBA[3], vtx.XYZFlags.fXYZ[0], vtx.XYZFlags.fXYZ[1], vtx.XYZFlags.fXYZ[2], vtx.XYZFlags.flags,
						vtx.normal.fNormal[0], vtx.normal.fNormal[1], vtx.normal.fNormal[2]);
				}

				{
					const auto& vtx = gNativeVertexBufferDataDraw.vertex.buff[i];
					NATIVE_LOG(LogLevel::Info, "Renderer::Native::AddMesh Live vertex: {}, (S: {} T: {} Q: {}) (R: {} G: {} B: {} A: {}) (X: {} Y: {} Z: {} Skip: {})({}, {}, {})\n",
						i, vtx.STQ.ST[0], vtx.STQ.ST[1], vtx.STQ.Q, vtx.RGBA[0], vtx.RGBA[1], vtx.RGBA[2], vtx.RGBA[3], vtx.XYZFlags.fXYZ[0], vtx.XYZFlags.fXYZ[1], vtx.XYZFlags.fXYZ[2], vtx.XYZFlags.flags,
						vtx.normal.fNormal[0], vtx.normal.fNormal[1], vtx.normal.fNormal[2]);
				}

				assert(internalBuffer.vertex.buff[i].XYZFlags.fXYZ[0] == gNativeVertexBufferDataDraw.vertex.buff[i].XYZFlags.fXYZ[0]);
				assert(internalBuffer.vertex.buff[i].XYZFlags.fXYZ[1] == gNativeVertexBufferDataDraw.vertex.buff[i].XYZFlags.fXYZ[1]);
				assert(internalBuffer.vertex.buff[i].XYZFlags.fXYZ[2] == gNativeVertexBufferDataDraw.vertex.buff[i].XYZFlags.fXYZ[2]);

				assert(internalBuffer.vertex.buff[i].normal.fNormal[0] == gNativeVertexBufferDataDraw.vertex.buff[i].normal.fNormal[0]);
				assert(internalBuffer.vertex.buff[i].normal.fNormal[1] == gNativeVertexBufferDataDraw.vertex.buff[i].normal.fNormal[1]);
				assert(internalBuffer.vertex.buff[i].normal.fNormal[2] == gNativeVertexBufferDataDraw.vertex.buff[i].normal.fNormal[2]);

				assert(internalBuffer.vertex.buff[i].STQ.ST[0] == gNativeVertexBufferDataDraw.vertex.buff[i].STQ.ST[0]);
				assert(internalBuffer.vertex.buff[i].STQ.ST[1] == gNativeVertexBufferDataDraw.vertex.buff[i].STQ.ST[1]);
				assert(internalBuffer.vertex.buff[i].STQ.Q == gNativeVertexBufferDataDraw.vertex.buff[i].STQ.Q);
			}
		}

		static void InitFade()
		{
			gFadeBuffer.Init();

			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				const VkDescriptorBufferInfo vertexDescBufferInfo = gFadeBuffer.GetDescBufferInfo(i);

				DescriptorWriteList writeList;
				writeList.EmplaceWrite({ 1, EBindingStage::Fragment, &vertexDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
				PostProcessing::UpdateDescriptorSets(PostProcessing::Effect::Fade, i, writeList);
			}
		}

		void CreateRenderStage(const RenderPassKey& key, const char* name)
		{
			RenderStage& stage = gRenderPass[key];

			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = GetSwapchainImageFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = (key.clearMode == EClearMode::None || key.clearMode == EClearMode::Depth) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = (key.clearMode == EClearMode::None || key.clearMode == EClearMode::Depth) ? VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = key.clearMode == EClearMode::None ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = key.clearMode == EClearMode::None ? VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

			VkRenderPassCreateInfo renderPassCreateInfo{};
			renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCreateInfo.attachmentCount = attachments.size();
			renderPassCreateInfo.pAttachments = attachments.data();
			renderPassCreateInfo.subpassCount = 1;
			renderPassCreateInfo.pSubpasses = &subpass;
			renderPassCreateInfo.dependencyCount = 1;
			renderPassCreateInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(GetDevice(), &renderPassCreateInfo, GetAllocator(), &stage.gRenderPass) != VK_SUCCESS) {
				throw std::runtime_error("failed to create render pass!");
			}

			SetObjectName(reinterpret_cast<uint64_t>(stage.gRenderPass), VK_OBJECT_TYPE_RENDER_PASS, name);

			stage.CreatePipeline();
		}

		void Renderer::Native::RenderMesh(SimpleMesh* pMesh, const uint32_t renderFlags)
		{
			gCachedPerDrawData.renderFlags = renderFlags;

			if (!gCurrentDraw) {
				NATIVE_LOG(LogLevel::Info, "RenderMesh Creating new draw!");
				CreateDraw();
			}

			NATIVE_LOG(LogLevel::Info, "RenderMesh: {} prim: 0x{:x}", pMesh->GetName(), pMesh->GetPrim().CMD);

			constexpr bool bSanityCheck = false;

			if (bSanityCheck) {
				FLUSH_LOG();

				SanityCheck(pMesh);
			}

			constexpr bool bMergingEnabeld = false;

			if (bMergingEnabeld && CanMergeMesh()) {
				NATIVE_LOG(LogLevel::Info, "RenderMesh: merging");
				MergeIndexData();
			}
			else {
				auto& instance = gCurrentDraw->instances.emplace_back();
				instance.animationMatrixStart = gCurrentAnimMatrixIndex;
				instance.modelMatrixIndex = gModelBuffer.GetInstanceIndex();
				instance.pMesh = pMesh;
				instance.perDrawData = gCachedPerDrawData;
				instance.lightingDataIndex = gLightingDynamicBuffer.GetInstanceIndex();
				instance.animStDataIndex = gAnimStBuffer.GetInstanceIndex();

				NATIVE_LOG(LogLevel::Info, "RenderMesh Model index: {} instance anim start: {}", instance.modelMatrixIndex, instance.animationMatrixStart);
			}
		}

		void PushGlobalMatrices(float* pModel, float* pView, float* pProj)
		{
			NATIVE_LOG(LogLevel::Info, "PushGlobalMatrices");

			// copy into model.
			if (pProj) {
				gCachedProjMatrix = glm::make_mat4(pProj);
			}

			if (pView) {
				gCachedViewMatrix = glm::make_mat4(pView);
			}

			PushModelMatrix(pModel);
		}

		void PushModelMatrix(float* pModel)
		{
			NATIVE_LOG(LogLevel::Info, "PushModelMatrix: {}", gModelBuffer.GetDebugIndex());
			gModelBuffer.AddInstanceData(glm::make_mat4(pModel));
		}

		void PushAnimMatrix(float* pAnim)
		{
			NATIVE_LOG(LogLevel::Info, "PushAnimMatrix: {}", gAnimationMatrices.size());
			gAnimationMatrices.push_back(glm::make_mat4(pAnim));
		}

		void StartAnimMatrix()
		{
			gCurrentAnimMatrixIndex = gAnimationMatrices.size();
		}

		void SetAnimStInstanceData(const glm::vec4& data)
		{
			gAnimStBuffer.AddInstanceData(data);
		}

		void PushMatrixPacket(const MatrixPacket* const pPkt)
		{
			NATIVE_LOG(LogLevel::Info, "PushMatrixPacket");

			assert(pPkt);

			{
				LightingDynamicBufferData data;

				data.lightDirection = glm::make_mat4(pPkt->objLightDirectionsMatrix);
				data.lightColor = glm::make_mat4(pPkt->lightColorMatrix);
				data.lightAmbient = glm::vec4(pPkt->adjustedLightAmbient[0], pPkt->adjustedLightAmbient[1], pPkt->adjustedLightAmbient[2], pPkt->adjustedLightAmbient[3]);
				data.flare = glm::vec4(pPkt->flare[0], pPkt->flare[1], pPkt->flare[2], pPkt->flare[3]);

				gLightingDynamicBuffer.AddInstanceData(data);
			}

			SetAnimStInstanceData(glm::make_vec4(pPkt->animStNormalExtruder));

			NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[0], pPkt->objLightDirectionsMatrix[1], pPkt->objLightDirectionsMatrix[2]);
			NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[4], pPkt->objLightDirectionsMatrix[5], pPkt->objLightDirectionsMatrix[6]);
			NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[8], pPkt->objLightDirectionsMatrix[9], pPkt->objLightDirectionsMatrix[10]);

			NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[0], pPkt->lightColorMatrix[1], pPkt->lightColorMatrix[2], pPkt->lightColorMatrix[3]);
			NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[4], pPkt->lightColorMatrix[5], pPkt->lightColorMatrix[6], pPkt->lightColorMatrix[7]);
			NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[8], pPkt->lightColorMatrix[9], pPkt->lightColorMatrix[10], pPkt->lightColorMatrix[11]);

			NATIVE_LOG(LogLevel::Info, "PushLightData: ambient: {} {} {} {}", pPkt->adjustedLightAmbient[0], pPkt->adjustedLightAmbient[1], pPkt->adjustedLightAmbient[2], pPkt->adjustedLightAmbient[3]);

			NATIVE_LOG(LogLevel::Info, "PushLightData: flare: {} {} {} {}", pPkt->flare[0], pPkt->flare[1], pPkt->flare[2], pPkt->flare[3]);

			NATIVE_LOG(LogLevel::Info, "PushLightData: animST: {} {} {} {}", pPkt->animStNormalExtruder[0], pPkt->animStNormalExtruder[1], pPkt->animStNormalExtruder[2], pPkt->animStNormalExtruder[3]);
		}

		void PushAnimST(float* pAnimST)
		{
			NATIVE_LOG(LogLevel::Info, "PushAnimST: {}", gAnimStBuffer.GetDebugIndex());
			assert(pAnimST);
			NATIVE_LOG(LogLevel::Info, "PushAnimST: {} {} {} {}", pAnimST[0], pAnimST[1], pAnimST[2], pAnimST[3]);

			SetAnimStInstanceData(glm::make_vec4(pAnimST));
		}
	} // Native
} // Renderer

// Simple mesh impl

void Renderer::Native::OnVideoFlip()
{
	gRenderThread->SignalEndCommands();
}

void Renderer::Native::InitializeDescriptorsSets(SimpleTexture* pTexture)
{
	if (!pTexture) {
		return;
	}

	NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());

	if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
		pTexture->GetName();
	}

	PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

	const Pipeline& pipeline = gRenderPass[gCachedRenderPassKey].GetPipeline();

	if (pTextureData->HasDescriptorSets(pipeline)) {
		// Already have descriptor sets, no need to initialize them.
		return;
	}

	// Work out the sampler
	auto& textureRegisters = pTexture->GetTextureRegisters();
	PS2::PSSamplerSelector selector = PS2::EmulateTextureSampler(pTextureData->width, pTextureData->height, textureRegisters.clamp, textureRegisters.tex, {});

	VkSampler& sampler = PS2::GetSampler(selector);

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = pTextureData->imageView;
	imageInfo.sampler = sampler;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		const VkDescriptorBufferInfo modelDescBufferInfo = gModelBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo animDescBufferInfo = gAnimationBuffer.GetDescBufferInfo(i, 27 * sizeof(glm::mat4));

		const VkDescriptorBufferInfo alphaDescBufferInfo = gAlphaBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo vertexDynamicsDescBufferInfo = gLightingDynamicBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo animStDescBufferInfo = gAnimStBuffer.GetDescBufferInfo(i);

		NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: offset: {} range: {}", animDescBufferInfo.offset, animDescBufferInfo.range);

		DescriptorWriteList writeList;
		writeList.EmplaceWrite({ 2, EBindingStage::Vertex, &modelDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
		writeList.EmplaceWrite({ 3, EBindingStage::Vertex, &animDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
		writeList.EmplaceWrite({ 4, EBindingStage::Vertex, &vertexDynamicsDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
		writeList.EmplaceWrite({ 5, EBindingStage::Vertex, &animStDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });

		writeList.EmplaceWrite({ 6, EBindingStage::Fragment, &alphaDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
		writeList.EmplaceWrite({ 1, EBindingStage::Fragment, nullptr, &imageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

		pTextureData->UpdateDescriptorSets(pipeline, writeList, i);
	}
}

void Renderer::Native::CreatePipeline(const PipelineCreateInfo<PipelineKey>& createInfo, const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name)
{
	auto vertShader = Shader::ReflectedModule(createInfo.vertShaderFilename, VK_SHADER_STAGE_VERTEX_BIT);
	auto fragShader = Shader::ReflectedModule(createInfo.fragShaderFilename, VK_SHADER_STAGE_FRAGMENT_BIT);

	// Vert
	vertShader.reflectData.MarkUniformBufferDynamic(0, 2);
	vertShader.reflectData.MarkUniformBufferDynamic(0, 3);
	vertShader.reflectData.MarkUniformBufferDynamic(0, 4);
	vertShader.reflectData.MarkUniformBufferDynamic(0, 5);

	// Frag
	fragShader.reflectData.MarkUniformBufferDynamic(0, 6);

	pipeline.AddBindings(EBindingStage::Vertex, vertShader.reflectData);
	pipeline.AddBindings(EBindingStage::Fragment, fragShader.reflectData);
	pipeline.CreateDescriptorSetLayouts();

	pipeline.CreateLayout();

	pipeline.CreateDescriptorPool();
	pipeline.CreateDescriptorSets();

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShader.shaderStageCreateInfo, fragShader.shaderStageCreateInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto& bindingDescription = vertShader.reflectData.bindingDescription;
	const auto& attributeDescriptions = vertShader.reflectData.GetAttributes();

	bindingDescription.stride = sizeof(GSVertexUnprocessed);

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = createInfo.key.options.bWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
		VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
		VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
		VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT,
		VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineDepthStencilStateCreateInfo depthState{};
	depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthState.depthTestEnable = VK_TRUE;
	depthState.depthWriteEnable = VK_TRUE;
	depthState.depthCompareOp = VK_COMPARE_OP_GREATER;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depthState;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipeline.layout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, GetAllocator(), &pipeline.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	SetObjectName(reinterpret_cast<uint64_t>(pipeline.pipeline), VK_OBJECT_TYPE_PIPELINE, name);
}

Renderer::FrameBufferBase& Renderer::Native::GetFrameBuffer()
{
	return gFrameBuffer;
}

void Renderer::Native::Setup()
{
	const VkDeviceSize maxUboSize = CheckBufferSizes();

	RenderPassKey key;
	key.clearMode = EClearMode::None;

	CreateRenderStage(key, "Native Render Pass CM None");
	key.clearMode = EClearMode::Depth;
	CreateRenderStage(key, "Native Render Pass CM Depth");
	key.clearMode = EClearMode::ColorDepth;
	CreateRenderStage(key, "Native Render Pass CM ColorDepth");

	CreateFramebuffer();
	CreateFramebufferSampler();
	gCommandPool = CreateCommandPool("Native Renderer Command Pool");
	CreateCommandBuffers(gCommandPool, gCommandBuffers, "Native Renderer Command Buffer");
	CreateCommandBuffers(gCommandPool, gCommandBuffersDummy, "Native Renderer Command Buffer Dummy");

	gMaxAnimationMatrices = static_cast<int>(maxUboSize / sizeof(glm::mat4));

	gNativeVertexBuffer.Init(0x100000, 0x100000);

	gModelBuffer.Init();
	gAnimationBuffer.Init(gMaxAnimationMatrices);

	gNativeVertexBufferDataDraw.Init(0x10000, 0x10000);

	gAlphaBuffer.Init(gMaxInstances);

	gLightingDynamicBuffer.Init();
	gAnimStBuffer.Init();

	GetRenderDelegate() += Render;

	gRenderThread = std::make_unique<RenderThread>();

	PostProcessing::Setup();
	DisplayList::Setup();

	InitFade();
}

void Renderer::Native::Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent)
{
	ZONE_SCOPED;

	{
		ZONE_SCOPED_NAME("Render Thread Wait");

		ScopedTimer waitForRenderThread(gRenderWaitTime);
		while (!gRenderThread->IsComplete()) {
			// Spin
		}

		if (!gRenderThread->GetHasRecordedCommands()) {
			RecordBeginCommandBuffer();
			RecordBeginRenderPass(RenderPassKey::Empty);
			RecordEndCommandBuffer();
		}

		gRenderThread->Reset();
	}

	ScopedTimer timer(gRenderTime);

	std::array<VkCommandBuffer, 2> cmdBuffers;

	{
		const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

		Renderer::Debug::EndLabel(cmd);
		vkEndCommandBuffer(cmd);

		cmdBuffers[0] = cmd;
	}

	{
		const VkCommandBuffer& cmd = DisplayList::FinalizeCommandBuffer(false);
		PostProcessing::AddPostProcessEffect(cmd, PostProcessing::Effect::AlphaFix);

		if (bFadeActive) {
			PostProcessing::AddPostProcessEffect(cmd, PostProcessing::Effect::Fade); // Currently these effects don't chain, so fade also does alpha fix
			bFadeActive = false;
		}

		vkEndCommandBuffer(cmd);

		cmdBuffers[1] = cmd;
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = cmdBuffers.size();
	submitInfo.pCommandBuffers = cmdBuffers.data();

	vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	gNativeVertexBuffer.Reset();
	gAnimationMatrices.clear();

	gModelBuffer.Reset();
	gLightingDynamicBuffer.Reset();
	gAnimStBuffer.Reset();

	NATIVE_LOG(LogLevel::Info, "Renderer::Native::Render Complete!");
}

void Renderer::Native::BindTexture(SimpleTexture* pTexture)
{
	NATIVE_LOG(LogLevel::Info, "BindTexture: {} material: {} layer: {} indices: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex(), gNativeVertexBufferDataDraw.GetIndexTail());

	if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
		pTexture->GetName();
	}

	InitializeDescriptorsSets(pTexture);

	if (gCurrentDraw) {
		gCurrentDraw->pTexture = pTexture;

		gCurrentDraw->projMatrix = gCachedProjMatrix;
		gCurrentDraw->viewMatrix = gCachedViewMatrix;

		int instanceIndex = 0;
		for (auto& instance : gCurrentDraw->instances) {
			NATIVE_LOG(LogLevel::Info, "BindTexture: instance ({}) anim start: {}", instanceIndex++, instance.animationMatrixStart);
		}

		gRenderThread->AddDraw(*gCurrentDraw);

		// If the texture is expecting to do a Z only draw, need to duplicate it.
		if (pTexture->GetTextureRegisters().test.AFAIL == AFAIL_ZB_ONLY && pTexture->GetTextureRegisters().test.ATST == ATST_NEVER) {
			gCurrentDraw->bIsAfailZOnly = true;
			gRenderThread->AddDraw(*gCurrentDraw);
		}

		gFinalViewMatrix = gCachedViewMatrix;
		gFinalProjMatrix = gCachedProjMatrix;

		gCurrentDraw.reset();
	}

	NATIVE_LOG(LogLevel::Info, "BindTexture Done\n-------------------------------------------------------\n");
}

const VkSampler& Renderer::Native::GetSampler()
{
	return gFrameBufferSampler;
}

const VkImageView& Renderer::Native::GetColorImageView()
{
	return PostProcessing::GetColorImageView();
}

bool& Renderer::GetForceAnimMatrixIdentity()
{
	return Native::bForceAnimMatrixIdentity;
}

void Renderer::RenderMesh(SimpleMesh* pNewMesh, const uint32_t renderFlags)
{
	assert(pNewMesh);
	Native::RenderMesh(pNewMesh, renderFlags);
}

void Renderer::PushGlobalMatrices(float* pModel, float* pView, float* pProj)
{
	Native::PushGlobalMatrices(pModel, pView, pProj);
}

void Renderer::PushModelMatrix(float* pModel)
{
	Native::PushModelMatrix(pModel);
}

void Renderer::StartAnimMatrix()
{
	Native::StartAnimMatrix();
}

void Renderer::PushAnimMatrix(float* pAnim)
{
	Native::PushAnimMatrix(pAnim);
}

void Renderer::PushAnimST(float* pAnimST)
{
	Native::PushAnimST(pAnimST);
}

void Renderer::BindNull()
{
	Native::gCurrentDraw.reset();
}

void Renderer::Native::DrawFade(uint8_t r, uint8_t g, uint8_t b, int a)
{
	bFadeActive = true;

	gFadeBuffer.GetBufferData().fadeColor = glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 127.0f);
	gFadeBuffer.Map(GetCurrentFrame());
}

void Renderer::Native::UpdateRenderPassKey(Renderer::Native::EClearMode clearMode)
{
	gCachedRenderPassKey.clearMode = clearMode;
}