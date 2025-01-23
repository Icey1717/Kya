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

#define DEBUG_TEXTURE_NAME "BACKGROUND.g2d"
#define DEBUG_TEXTURE_MATERIAL_INDEX 5

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

		struct LightingData {
			glm::mat4 lightDirection;
			glm::mat4 lightColor;
			glm::vec4 lightAmbient;
		};

		struct VertexConstantBuffer {

			void Reset()
			{
				view = glm::mat4(1.0f);
				proj = glm::mat4(1.0f);
			}

			glm::mat4 view;
			glm::mat4 proj;
		};

		glm::mat4 gFinalViewMatrix;
		glm::mat4 gFinalProjMatrix;

		constexpr int gMaxModelMatrices = 1024;

		static int gMaxAnimationMatrices = 0;

		static PipelineMap gPipelines;
		static FrameBufferBase gFrameBuffer;

		static VkSampler gFrameBufferSampler;

		static VkRenderPass gRenderPass;
		static VkCommandPool gCommandPool;
		static CommandBufferVector gCommandBuffers;

		static UniformBuffer<VertexConstantBuffer> gVertexConstantBuffer;
		static DynamicUniformBuffer<glm::mat4> gModelBuffer;

		struct AlphaConstantBuffer {
			alignas(4) VkBool32 enable; // Use VkBool32 for proper alignment
			alignas(4) int atst;
			alignas(4) int aref;

			AlphaConstantBuffer(const TextureRegisters& textureRegisters)
				: enable(textureRegisters.test.ATE)
				, atst(textureRegisters.test.ATST)
				, aref(textureRegisters.test.AREF)
			{ }
		};

		DynamicUniformBuffer<AlphaConstantBuffer> gAlphaBuffer;

		using NativeVertexBuffer = PS2::FrameVertexBuffers<GSVertexUnprocessedNormal, uint16_t>;
		static NativeVertexBuffer gNativeVertexBuffer;
		// As each draw comes in this buffer is filled.
		static NativeVertexBufferData gNativeVertexBufferDataDraw;

		static DynamicUniformBuffer<glm::mat4> gAnimationBuffer;
		static std::vector<glm::mat4> gAnimationMatrices;

		static std::vector<glm::mat4> gModelMatrices;

		struct Draw {
			SimpleTexture* pTexture = nullptr;

			glm::mat4 projMatrix;
			glm::mat4 viewMatrix;

			struct Instance {
				SimpleMesh* pMesh = nullptr;
				glm::mat4 modelMatrix;
				int indexStart = 0;
				int indexCount = 0;
				int vertexStart = 0;
				int animationMatrixStart = 0;

				LightingData lightingData;

				int modelMatrixIndex = 0;
			};

			std::vector<Instance> instances;
		};

		static std::optional<Draw> gCurrentDraw;

		static glm::mat4 gCachedModelMatrix;
		static glm::mat4 gCachedViewMatrix;
		static glm::mat4 gCachedProjMatrix;

		static LightingData gCachedLightingData;

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
			auto& vertexBufferData = instance.pMesh->GetInternalVertexBufferData();

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

		static bool NearlyEqual(const glm::mat4& mat1, const glm::mat4& mat2, float epsilon = 1e-5f) {
			for (int i = 0; i < 4; ++i) {
				glm::vec4 v1 = mat1[i];
				glm::vec4 v2 = mat2[i];
				if (!glm::all(glm::lessThan(glm::abs(v1 - v2), glm::vec4(epsilon)))) {
					return false;
				}
			}

			return true;
		}

		static void CreateFramebuffer()
		{
			gFrameBuffer.SetupBase({ gWidth, gHeight }, gRenderPass, true);
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

		static void CreatePipeline()
		{
			PipelineKey key;
			key.options.bGlsl = true;
			key.options.bWireframe = false;
			PipelineCreateInfo createInfo{ "shaders/native.vert.spv" , "shaders/native.frag.spv", key };
			CreatePipeline(createInfo, gRenderPass, gPipelines[createInfo.key.key], "Native Previewer GLSL");
		}

		static Pipeline& GetPipeline() {
			PipelineKey pipelineKey;
			pipelineKey.options.bWireframe = false;
			pipelineKey.options.bGlsl = true;

			assert(gPipelines.find(pipelineKey.key) != gPipelines.end());
			return gPipelines[pipelineKey.key];
		}

		class InstanceDataUpdate
		{
		public:
			void UpdateInstanceData(Draw& draw, Pipeline& pipeline)
			{
				SimpleTexture* pTexture = draw.pTexture;

				if (!pTexture) {
					return;
				}

				NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());

				if (pTexture->GetName() == DEBUG_TEXTURE_NAME && pTexture->GetMaterialIndex() == DEBUG_TEXTURE_MATERIAL_INDEX) {
					pTexture->GetName();
				}

				for (auto& instance : draw.instances) {
					assert(modelIndex < gMaxModelMatrices);

					SimpleMesh* pMesh = instance.pMesh;
					NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {}", pMesh->GetName());

					if (lastModelMatrix != instance.modelMatrix)
					{
						gModelBuffer.SetInstanceData(modelIndex, instance.modelMatrix);
						lastModelMatrix = instance.modelMatrix;
						modelIndex++;
					}

					instance.modelMatrixIndex = modelIndex - 1;
				}

				gAlphaBuffer.SetInstanceData(textureIndex, { pTexture->GetTextureRegisters() });
				textureIndex++;
			}

			void Reset()
			{
				modelIndex = 0;
				textureIndex = 0;
				lastModelMatrix = glm::mat4(std::numeric_limits<float>().max());
			}

		private:
			int modelIndex = 0;
			int textureIndex = 0;
			glm::mat4 lastModelMatrix = glm::mat4(std::numeric_limits<float>().max());
		};

		static void UpdateConstantBuffers(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
		{
			gVertexConstantBuffer.GetBufferData().view = viewMatrix;
			gVertexConstantBuffer.GetBufferData().proj = projMatrix;

			gVertexConstantBuffer.Update(GetCurrentFrame());
		}

		static void DynamicDataUpdate()
		{
			gModelBuffer.Update(GetCurrentFrame());
			gAlphaBuffer.Update(GetCurrentFrame());

			for (int i = 0; i < gAnimationMatrices.size() ; i++) {
				if (bForceAnimMatrixIdentity) {
					gAnimationMatrices[i] = glm::mat4(1.0f);
				}

				gAnimationBuffer.SetInstanceData(i, gAnimationMatrices[i]);
			}

			gAnimationBuffer.Update(GetCurrentFrame());
		}

		class DrawCommandRecorder
		{
		public:
			void RecordDrawCommand(Draw& drawCommand, Pipeline& pipeline)
			{
				SimpleTexture* pTexture = drawCommand.pTexture;

				if (pTexture && !drawCommand.instances.empty()) {
					NATIVE_LOG_VERBOSE(LogLevel::Verbose, "RecordDrawCommand {}", pTexture->GetName());

					const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

					Debug::UpdateLabel(pTexture, cmd);

					PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

					std::optional<uint> primState;

					for (auto& instance : drawCommand.instances) {
						if (instance.indexCount == 0) {
							continue;
						}

						vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LightingData), &instance.lightingData);

						if (!primState.has_value() || primState.value() != instance.pMesh->GetPrim().CMD) {
							primState = instance.pMesh->GetPrim().CMD;
							SetBlendingDynamicState(pTexture, instance.pMesh, cmd);
						}

						std::array< uint32_t, 3 > dynamicOffsets = {
							instance.modelMatrixIndex * gModelBuffer.GetDynamicAlignment(),
							instance.animationMatrixStart * gAnimationBuffer.GetDynamicAlignment(),
							textureIndex * gAlphaBuffer.GetDynamicAlignment()
						};

						vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &pTextureData->GetDescriptorSets(pipeline).GetSet(GetCurrentFrame()), dynamicOffsets.size(), dynamicOffsets.data());

						vkCmdDrawIndexed(cmd, static_cast<uint32_t>(instance.indexCount), 1, instance.indexStart, instance.vertexStart, 0);
					}
				}

				textureIndex++;
			}

			void Reset()
			{
				textureIndex = 0;
			}

		private:
			int textureIndex = 0;
		};

		VkDeviceSize CheckBufferSizes()
		{
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);
			const VkDeviceSize maxUboSize = properties.limits.maxUniformBufferRange;

			assert(maxUboSize >= sizeof(VertexConstantBuffer));
			assert(maxUboSize >= (sizeof(glm::mat4) * gMaxModelMatrices));

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

			if (instance.modelMatrix != gCachedModelMatrix) {
				return false;
			}

			return true;
		}

		static void CreateDraw()
		{
			gCurrentDraw = Draw{};
		}

		static void RecordBeginCommandBuffer()
		{
			const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(cmd, &beginInfo);

			Renderer::Debug::BeginLabel(cmd, "Native Render");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = gRenderPass;
			renderPassInfo.framebuffer = gFrameBuffer.framebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { gWidth, gHeight };

			std::array<VkClearValue, 2> clearColors;
			clearColors[0] = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearColors[1] = { {0.0f, 0.0f } };
			renderPassInfo.clearValueCount = clearColors.size();
			renderPassInfo.pClearValues = clearColors.data();

			vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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

			auto& pipeline = GetPipeline();
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

			gNativeVertexBuffer.BindBuffers(cmd);
		}

		static void RecordEndCommandBuffer()
		{
			const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

			Debug::Reset(cmd);

			UpdateConstantBuffers(gFinalViewMatrix, gFinalProjMatrix);
			DynamicDataUpdate();

			gNativeVertexBuffer.MapData();

			gNativeVertexBuffer.GetDrawBufferData().ResetAfterDraw();

			vkCmdEndRenderPass(cmd);
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

			void Run()
			{
				while (!bShouldStop) {
					std::unique_lock<std::mutex> lock(mutex);
					cv.wait(lock, [this] { return !bWaitingForCommands || bShouldStop; });

					if (bShouldStop) break;

					timer.Start();

					RecordBeginCommandBuffer();

					for (auto& draw : draws) {
						for (auto& instance : draw.instances) {
							FillIndexData(instance);
						}

						drawInstanceDataUpdate.UpdateInstanceData(draw, GetPipeline());
					}

					for (auto& draw : draws) {
						drawCommandRecorder.RecordDrawCommand(draw, GetPipeline());
					}

					RecordEndCommandBuffer();

					draws.clear();
					timer.End();
					bRecordedCommands = true;
					bWaitingForCommands = true;
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
			auto& internalBuffer = pMesh->GetInternalVertexBufferData();

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

	} // Native
} // Renderer

// Simple mesh impl

Renderer::NativeVertexBufferData& Renderer::SimpleMesh::GetVertexBufferData()
{
	return internalVertexBufferData;
}

void Renderer::Native::OnVideoFlip()
{
	gRenderThread->SignalEndCommands();
}

void Renderer::Native::CreateRenderPass(VkRenderPass& renderPass, const char* name)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = GetSwapchainImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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

	if (vkCreateRenderPass(GetDevice(), &renderPassCreateInfo, GetAllocator(), &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	SetObjectName(reinterpret_cast<uint64_t>(renderPass), VK_OBJECT_TYPE_RENDER_PASS, name);
}

void Renderer::Native::InitializeDescriptorsSets(SimpleTexture* pTexture)
{
	if (!pTexture) {
		return;
	}

	NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());

	if (pTexture->GetName() == DEBUG_TEXTURE_NAME && pTexture->GetMaterialIndex() == DEBUG_TEXTURE_MATERIAL_INDEX) {
		pTexture->GetName();
	}

	PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

	if (pTextureData->HasDescriptorSets(GetPipeline())) {
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
		const VkDescriptorBufferInfo vertexDescBufferInfo = gVertexConstantBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo modelDescBufferInfo = gModelBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo animDescBufferInfo = gAnimationBuffer.GetDescBufferInfo(i, 27 * sizeof(glm::mat4));

		const VkDescriptorBufferInfo alphaDescBufferInfo = gAlphaBuffer.GetDescBufferInfo(i);

		NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: offset: {} range: {}", animDescBufferInfo.offset, animDescBufferInfo.range);

		DescriptorWriteList writeList;
		writeList.EmplaceWrite({ 0, EBindingStage::Vertex, &vertexDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
		writeList.EmplaceWrite({ 2, EBindingStage::Vertex, &modelDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
		writeList.EmplaceWrite({ 3, EBindingStage::Vertex, &animDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });

		writeList.EmplaceWrite({ 4, EBindingStage::Fragment, &alphaDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
		writeList.EmplaceWrite({ 1, EBindingStage::Fragment, nullptr, &imageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

		pTextureData->UpdateDescriptorSets(GetPipeline(), writeList, i);
	}
}

void Renderer::Native::CreatePipeline(const PipelineCreateInfo& createInfo, const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name)
{
	auto vertShader = Shader::ReflectedModule(createInfo.vertShaderFilename, VK_SHADER_STAGE_VERTEX_BIT);
	auto fragShader = Shader::ReflectedModule(createInfo.fragShaderFilename, VK_SHADER_STAGE_FRAGMENT_BIT);

	vertShader.reflectData.MarkUniformBufferDynamic(0, 2);
	vertShader.reflectData.MarkUniformBufferDynamic(0, 3);
	fragShader.reflectData.MarkUniformBufferDynamic(0, 4);

	pipeline.AddBindings(EBindingStage::Vertex, vertShader.reflectData);
	pipeline.AddBindings(EBindingStage::Fragment, fragShader.reflectData);
	pipeline.CreateDescriptorSetLayouts();

	pipeline.CreateLayout(vertShader.reflectData.pushConstants);

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
	CreateRenderPass(gRenderPass, "Native Render Pass");
	CreateFramebuffer();
	CreateFramebufferSampler();
	gCommandPool = CreateCommandPool("Native Renderer Command Pool");
	CreateCommandBuffers(gCommandPool, gCommandBuffers, "Native Renderer Command Buffer");
	CreatePipeline();

	const VkDeviceSize maxUboSize = CheckBufferSizes();
	gMaxAnimationMatrices = static_cast<int>(maxUboSize / sizeof(glm::mat4));

	gVertexConstantBuffer.Init();
	gNativeVertexBuffer.Init(0x100000, 0x100000);

	gModelBuffer.Init(gMaxModelMatrices);
	gAnimationBuffer.Init(gMaxAnimationMatrices);

	gNativeVertexBufferDataDraw.Init(0x10000, 0x10000);

	gAlphaBuffer.Init(gMaxModelMatrices);

	GetRenderDelegate() += Render;

	gVertexConstantBuffer.GetBufferData().Reset();

	gRenderThread = std::make_unique<RenderThread>();

	PostProcessing::Setup();
	DisplayList::Setup();
}

void Renderer::Native::Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent)
{
	{
		ScopedTimer waitForRenderThread(gRenderWaitTime);
		while (!gRenderThread->IsComplete()) {
			// Spin
		}

		if (!gRenderThread->GetHasRecordedCommands()) {
			RecordBeginCommandBuffer();
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

	NATIVE_LOG(LogLevel::Info, "Renderer::Native::Render Complete!");
}

void Renderer::Native::BindTexture(SimpleTexture* pTexture)
{
	NATIVE_LOG(LogLevel::Info, "BindTexture: {} material: {} layer: {} indices: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex(), gNativeVertexBufferDataDraw.GetIndexTail());

	if (pTexture->GetName() == DEBUG_TEXTURE_NAME && pTexture->GetMaterialIndex() == DEBUG_TEXTURE_MATERIAL_INDEX) {
		pTexture->GetName();
	}

	InitializeDescriptorsSets(pTexture);

	if (gCurrentDraw) {
		gCurrentDraw->pTexture = pTexture;

		gCurrentDraw->projMatrix = gCachedProjMatrix;
		gCurrentDraw->viewMatrix = gCachedViewMatrix;

		for (auto& instance : gCurrentDraw->instances) {
			NATIVE_LOG(LogLevel::Info, "BindTexture: instance anim start: {}", instance.animationMatrixStart);
		}

		gRenderThread->AddDraw(*gCurrentDraw);

		gFinalViewMatrix = gCachedViewMatrix;
		gFinalProjMatrix = gCachedProjMatrix;

		gCurrentDraw.reset();
	}

	NATIVE_LOG(LogLevel::Info, "BindTexture Done\n-------------------------------------------------------\n");
}

void Renderer::Native::AddMesh(SimpleMesh* pMesh)
{
	if (!gCurrentDraw) {
		NATIVE_LOG(LogLevel::Info, "AddMesh Creating new draw!");
		CreateDraw();
	}

	NATIVE_LOG(LogLevel::Info, "AddMesh: {} prim: 0x{:x}", pMesh->GetName(), pMesh->GetPrim().CMD);

	constexpr bool bSanityCheck = false;

	if (bSanityCheck) {
		FLUSH_LOG();

		SanityCheck(pMesh);
	}

	constexpr bool bMergingEnabeld = false;

	if (bMergingEnabeld && CanMergeMesh()) {
		NATIVE_LOG(LogLevel::Info, "AddMesh: merging");
		MergeIndexData();
	}
	else {
		auto& instance = gCurrentDraw->instances.emplace_back();
		instance.animationMatrixStart = gAnimationMatrices.size();
		instance.modelMatrix = gCachedModelMatrix;
		instance.pMesh = pMesh;
		instance.lightingData = gCachedLightingData;

		NATIVE_LOG(LogLevel::Info, "AddMesh: instance anim start: {}", instance.animationMatrixStart);
	}
}

void Renderer::Native::PushGlobalMatrices(float* pModel, float* pView, float* pProj)
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

void Renderer::Native::PushModelMatrix(float* pModel)
{
	NATIVE_LOG(LogLevel::Info, "PushModelMatrix");

	if (!gCurrentDraw) {
		NATIVE_LOG(LogLevel::Info, "PushModelMatrix Creating new draw!");
		CreateDraw();
	}

	if (pModel) {
		gCachedModelMatrix = glm::make_mat4(pModel);
	}
}

void Renderer::Native::PushAnimMatrix(float* pAnim)
{
	NATIVE_LOG(LogLevel::Info, "PushAnimMatrix");
	//assert(!std::isnan(pAnim[0]));
	gAnimationMatrices.push_back(glm::make_mat4(pAnim));
}

void Renderer::Native::PushMatrixPacket(const MatrixPacket* const pPkt)
{
	NATIVE_LOG(LogLevel::Info, "PushMatrixPacket");

	assert(pPkt);

	// Need to convert to matrix ?
	//gCachedLightingData.lightDirection[0] = glm::vec4(glm::make_vec3(pPkt->objLightDirectionsMatrix), 0.0f);
	//gCachedLightingData.lightDirection[1] = glm::vec4(glm::make_vec3(pPkt->objLightDirectionsMatrix + 3), 0.0f);
	//gCachedLightingData.lightDirection[2] = glm::vec4(glm::make_vec3(pPkt->objLightDirectionsMatrix + 6), 0.0f);
	//gCachedLightingData.lightDirection[3] = glm::vec4(glm::make_vec3(pPkt->objLightDirectionsMatrix + 9), 0.0f);

	gCachedLightingData.lightDirection = glm::make_mat4(pPkt->objLightDirectionsMatrix);
	gCachedLightingData.lightColor = glm::make_mat4(pPkt->lightColorMatrix);
	gCachedLightingData.lightAmbient = glm::vec4(pPkt->adjustedLightAmbient[0], pPkt->adjustedLightAmbient[1], pPkt->adjustedLightAmbient[2], pPkt->adjustedLightAmbient[3]);

	NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[0], pPkt->objLightDirectionsMatrix[1], pPkt->objLightDirectionsMatrix[2]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[4], pPkt->objLightDirectionsMatrix[5], pPkt->objLightDirectionsMatrix[6]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[8], pPkt->objLightDirectionsMatrix[9], pPkt->objLightDirectionsMatrix[10]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[0], pPkt->lightColorMatrix[1], pPkt->lightColorMatrix[2], pPkt->lightColorMatrix[3]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[4], pPkt->lightColorMatrix[5], pPkt->lightColorMatrix[6], pPkt->lightColorMatrix[7]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[8], pPkt->lightColorMatrix[9], pPkt->lightColorMatrix[10], pPkt->lightColorMatrix[11]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: ambient: {} {} {} {}", pPkt->adjustedLightAmbient[0], pPkt->adjustedLightAmbient[1], pPkt->adjustedLightAmbient[2], pPkt->adjustedLightAmbient[3]);

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