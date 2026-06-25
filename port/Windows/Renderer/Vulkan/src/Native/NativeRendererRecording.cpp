#include "NativeRendererInternal.h"

#include "NativeDebug.h"
#include "NativeDebugShapes.h"
#include "Objects/VulkanImage.h"
#include "profiling.h"

#include <readerwriterqueue.h>
#include <windows.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace Renderer
{
	namespace Native
	{
		double GetRenderTime()
		{
			return GetNativeRendererState().renderTime;
		}

		double GetRenderWaitTime()
		{
			return GetNativeRendererState().renderWaitTime;
		}

		static void FillIndexData(Draw::Instance& instance)
		{
			auto& vertexBufferData = instance.pMesh->GetVertexBufferData();

			instance.indexCount = vertexBufferData.GetIndexTail();
			instance.indexStart = GetNativeRendererState().nativeVertexBuffer.GetDrawBufferData().GetIndexTail();
			instance.vertexStart = GetNativeRendererState().nativeVertexBuffer.GetDrawBufferData().GetVertexTail();

			NATIVE_LOG_VERBOSE(LogLevel::Info, "FillIndexData Filled indexCount: {} indexStart: {} vertexStart: {}",
				instance.indexCount, instance.indexStart, instance.vertexStart);

			// Copy into the real buffer.
			GetNativeRendererState().nativeVertexBuffer.MergeData(vertexBufferData);
		}

		static void UpdateInstanceData(Draw& draw)
		{
			SimpleTexture* pTexture = draw.pTexture;

			if (!pTexture) {
				return;
			}

			TextureRegisters textureRegisters = pTexture->GetTextureRegisters();

			NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());

			if (draw.bIsAfailZOnly) {
				static const uint32_t inverted_atst[] = { ATST_ALWAYS, ATST_NEVER, ATST_GEQUAL, ATST_GREATER, ATST_NOTEQUAL, ATST_LESS, ATST_LEQUAL, ATST_EQUAL };
				textureRegisters.test.ATST = inverted_atst[textureRegisters.test.ATST];
			}

			for (auto& instance : draw.instances) {
				instance.perDrawData.alphaEnable = textureRegisters.test.ATE;
				instance.perDrawData.alphaAtst   = textureRegisters.test.ATST;
				instance.perDrawData.alphaAref   = textureRegisters.test.AREF;
				instance.perDrawData.alphaAfail  = textureRegisters.test.AFAIL;
			}
		}

		// Updates GPU side memory (Dynamic Storage Buffers | Per Instance Data)
		static void MapStorageBuffers()
		{
			GetNativeRendererState().modelBuffer.Map(GetCurrentFrame());
			GetNativeRendererState().animStBuffer.Map(GetCurrentFrame());
			GetNativeRendererState().lightingDynamicBuffer.Map(GetCurrentFrame());

			for (int i = 0; i < GetNativeRendererState().animationMatrices.size() ; i++) {
				if (GetNativeRendererState().forceAnimMatrixIdentity) {
					GetNativeRendererState().animationMatrices[i] = glm::mat4(1.0f);
				}

				GetNativeRendererState().animationBuffer.SetInstanceData(i, GetNativeRendererState().animationMatrices[i]);
			}

			GetNativeRendererState().animationBuffer.Map(GetCurrentFrame());
		}

		static const char* GetClearModeName(EClearMode clearMode)
		{
			switch (clearMode)
			{
			case EClearMode::None:       return "None";
			case EClearMode::Depth:      return "Depth";
			case EClearMode::ColorDepth: return "Color+Depth";
			case EClearMode::Color:      return "Color";
			default:                     return "Unknown";
			}
		}

		void RecordBeginRenderPass(const RenderPassKey& key)
		{
			const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];

			const RenderStage& stage = GetNativeRendererState().renderPass[key];

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = stage.gRenderPass;
			renderPassInfo.framebuffer = GetNativeRendererState().frameBuffer.framebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { static_cast<uint32_t>(gWidth), static_cast<uint32_t>(gHeight) };

			std::array<VkClearValue, 2> clearColors;
			clearColors[0] = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearColors[1] = { {0.0f, 0.0f } };
			renderPassInfo.clearValueCount = clearColors.size();
			renderPassInfo.pClearValues = clearColors.data();

			Renderer::Debug::BeginLabel(cmd, "Render Pass [clear: %s]", GetClearModeName(key.clearMode));

			vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			const auto& pipeline = stage.GetPipeline();
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

			GetNativeRendererState().activeRenderPassKey = key;
			GetNativeRendererState().hasActiveRenderPass = true;
		}

		void RecordEndRenderPass()
		{
			if (!GetNativeRendererState().hasActiveRenderPass) {
				return;
			}

			const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];
			vkCmdEndRenderPass(cmd);

			// Save depth from the first render pass before any subsequent pass can clear it.
			DebugShapes::SaveDepth(cmd, GetNativeRendererState().frameBuffer.depthImage);

			Renderer::Debug::EndLabel(cmd);
			GetNativeRendererState().activeRenderPassKey.Reset();
			GetNativeRendererState().hasActiveRenderPass = false;
		}

		void SetColorDepthDynamicState(const VkCommandBuffer& cmd, Draw& drawCommand)
		{
			VkBool32 colorWriteEnable = VK_TRUE;
			VkBool32 depthWriteEnable = drawCommand.pTexture->GetTextureRegisters().test.AFAIL != AFAIL_FB_ONLY ? VK_TRUE : VK_FALSE;

			if (drawCommand.bIsAfailZOnly) {
				depthWriteEnable = VK_TRUE;
				colorWriteEnable = VK_FALSE;
			}

			if (drawCommand.bIsZMask) {
				depthWriteEnable = VK_FALSE;
			}

			// Depth.
			vkCmdSetDepthWriteEnable(cmd, depthWriteEnable);

			// Color.
			GetNativeRendererState().vkCmdSetColorWriteEnableEXT(cmd, 1, &colorWriteEnable);

			std::array<VkBool32, 1> colorWriteMasks = {
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			};

			if (drawCommand.pTexture->GetTextureRegisters().test.AFAIL == AFAIL_RGB_ONLY) {
				// Enable only RGB channels (disable alpha write)
				colorWriteMasks[0] = {
					VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
				};
			}

			GetNativeRendererState().vkCmdSetColorWriteMaskEXT(cmd, 0, colorWriteMasks.size(), colorWriteMasks.data());
		}

		class DrawCommandRecorder
		{
		public:
			void RecordDrawCommand(Draw& drawCommand)
			{
				if (!bInRenderPass || drawCommand.bRenderPassDirty) {
					if (bInRenderPass) {
						const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];
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

					const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];

					const Pipeline& pipeline = GetNativeRendererState().renderPass[currentRenderPassKey].GetPipeline();

					Debug::UpdateLabel(pTexture, cmd);

					PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

					std::optional<uint> primState;
					std::optional<bool> alphaBlendState;
					std::optional<uint64_t> effectiveAlphaState;

					if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
						pTexture->GetName();
					}

					for (auto& instance : drawCommand.instances) {
						if (instance.indexCount == 0) {
							continue;
						}

						NATIVE_LOG_VERBOSE(LogLevel::Verbose, "RecordDrawCommand: {} LD {} AST {}", pTexture->GetName(), instance.lightingDataIndex, instance.animStDataIndex);

						Renderer::Debug::BeginLabel(cmd, "%s", instance.pMesh->GetName().c_str());
						
						instance.perDrawData.projXView = drawCommand.projMatrix * drawCommand.viewMatrix;

						vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PerDrawData), &instance.perDrawData);
						GIFReg::GSAlpha effectiveAlpha = pTexture->GetTextureRegisters().alpha;
						if ((instance.perDrawData.renderFlags & 0x20) != 0) {
							effectiveAlpha = instance.gsAlpha;
						}

						const bool bAlphaBlendEnabled = instance.pMesh->GetPrim().ABE || ((instance.perDrawData.renderFlags & 0x20) != 0);
						if (!primState.has_value() || primState.value() != instance.pMesh->GetPrim().CMD || !alphaBlendState.has_value() || alphaBlendState.value() != bAlphaBlendEnabled || !effectiveAlphaState.has_value() || effectiveAlphaState.value() != effectiveAlpha.CMD) {
							primState = instance.pMesh->GetPrim().CMD;
							alphaBlendState = bAlphaBlendEnabled;
							effectiveAlphaState = effectiveAlpha.CMD;
							vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, GetBlendPipeline(currentRenderPassKey, effectiveAlpha, bAlphaBlendEnabled));
						}

						SetColorDepthDynamicState(cmd, drawCommand);

						vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, drawCommand.pDescriptorSets, 0, nullptr);

						vkCmdDrawIndexed(cmd, static_cast<uint32_t>(instance.indexCount), 1, instance.indexStart, instance.vertexStart, 0);

						Renderer::Debug::EndLabel(cmd);

						instanceIndex++;
					}
				}
			}

			void Reset()
			{
				instanceIndex = 0;
				bInRenderPass = false;
				currentRenderPassKey.Reset();
			}

		private:
			int instanceIndex = 0;
			bool bInRenderPass = false;
			RenderPassKey currentRenderPassKey;
		};
		void RecordBeginCommandBuffer()
		{
			const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];

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

			const VkRect2D scissor = { {0, 0}, { static_cast<uint32_t>(gWidth), static_cast<uint32_t>(gHeight) } };
			vkCmdSetScissor(cmd, 0, 1, &scissor);

			GetNativeRendererState().nativeVertexBuffer.BindBuffers(cmd);

			// Transition to TRANSFER_DST_OPTIMAL, clear both attachments, then transition to
			// READ_ONLY_OPTIMAL. This guarantees a clean framebuffer at the start of every frame
			// regardless of which EClearMode the first native render pass uses.
			// Render passes with LOAD_OP_CLEAR (EClearMode::ColorDepth / Depth / Color) use
			// initialLayout = UNDEFINED and will re-clear on their own; this pre-clear is the
			// safety net for frames where the first pass is EClearMode::None (LOAD_OP_LOAD).
			VkClearColorValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f} };
			VkClearDepthStencilValue depthStencil = { 0.0f, 0 };

			VkImageSubresourceRange colorRange{};
			colorRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorRange.baseMipLevel = 0;
			colorRange.levelCount = 1;
			colorRange.baseArrayLayer = 0;
			colorRange.layerCount = 1;

			VkImageSubresourceRange depthRange{};
			depthRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			depthRange.baseMipLevel = 0;
			depthRange.levelCount = 1;
			depthRange.baseArrayLayer = 0;
			depthRange.layerCount = 1;

			VulkanImage::TransitionImageLayout(GetNativeRendererState().frameBuffer.colorImage, GetSwapchainImageFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, colorRange.aspectMask, cmd);
			VulkanImage::TransitionImageLayout(GetNativeRendererState().frameBuffer.depthImage, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, depthRange.aspectMask, cmd);

			vkCmdClearColorImage(cmd, GetNativeRendererState().frameBuffer.colorImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &colorRange);
			vkCmdClearDepthStencilImage(cmd, GetNativeRendererState().frameBuffer.depthImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &depthStencil, 1, &depthRange);

			VulkanImage::TransitionImageLayout(GetNativeRendererState().frameBuffer.colorImage, GetSwapchainImageFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, colorRange.aspectMask, cmd);
			VulkanImage::TransitionImageLayout(GetNativeRendererState().frameBuffer.depthImage, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, depthRange.aspectMask, cmd);
		}

		// Copy all our data to the GPU.
		static void MapBuffers()
		{
			MapStorageBuffers();

			GetNativeRendererState().nativeVertexBuffer.MapData();

			// Reset the index and vertex heads for the next frame.
			GetNativeRendererState().nativeVertexBuffer.GetDrawBufferData().ResetAfterDraw();
		}

		void RecordEndCommandBuffer()
		{
			const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];

			Debug::Reset(cmd);
			RecordEndRenderPass();

			// All game passes are done and all debug shapes have been submitted; record the
			// dedicated debug pass using the depth saved from the first render pass.
			DebugShapes::RecordDedicatedPass(cmd);
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

			void UpdateInstanceDataForDraw(Draw& draw)
			{
				for (auto& instance : draw.instances) {
					FillIndexData(instance);
				}

				UpdateInstanceData(draw);
			}

			void RecordDrawCommands(Draw& draw)
			{
				drawCommandRecorder.RecordDrawCommand(draw);
			}

			void ProcessDraws()
			{
				Draw draw;
				while (draws.try_dequeue(draw)) {
					UpdateInstanceDataForDraw(draw);
					if (GetNativeRendererState().preview.IsSetup()) {
						GetNativeRendererState().preview.SaveDraw(draw);
					}
					RecordDrawCommands(draw);
				}
			}

			void Run()
			{
				while (!bShouldStop) {
					std::unique_lock<std::mutex> lock(mutex);
					cv.wait(lock, [this] { return draws.peek() || bShouldStop; });

					ZONE_SCOPED_NAME("RenderThread::Run");

					if (bShouldStop) break;

					if (bShouldRecordBegin) {
						RecordBeginCommandBuffer();
						bShouldRecordBegin = false;
					}

					ProcessDraws();
				}
			}

			void MainThreadEndCommands()
			{
				std::unique_lock<std::mutex> lock(mutex);

				if (!bRecordedCommands) {
					return;
				}

				// Any leftover draws to process.
				ProcessDraws();

				MapBuffers();
				RecordEndCommandBuffer();
				timer.End();
			}

			void AddDraw(const Draw& draw)
			{
				draws.enqueue(draw); // Lock-free push

				if (!bRecordedCommands) {
					bRecordedCommands = true;
					timer.Start();
				}
				cv.notify_one(); // Wake up render thread if sleeping
			}

			bool GetHasRecordedCommands()
			{
				return bRecordedCommands;
			}

			void Reset()
			{
				drawCommandRecorder.Reset();

				bRecordedCommands = false;
				bShouldRecordBegin = true;
			}

			double GetRenderThreadTime()
			{
				return timer.duration.count();
			}

			void SignalEndCommands()
			{
				NATIVE_LOG_VERBOSE(LogLevel::Info, "SignalEndCommands");
			}

		private:

			std::thread thread;
			moodycamel::ReaderWriterQueue<Draw> draws;
			DrawCommandRecorder drawCommandRecorder;
			std::atomic<bool> bShouldStop = false;
			std::atomic<bool> bRecordedCommands = false;

			std::atomic<bool> bShouldRecordBegin = true;

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

		double GetRenderThreadTime()
		{
			return GetNativeRendererState().renderThread->GetRenderThreadTime();
		}

		RenderThread* CreateRenderThread()
		{
			return new RenderThread();
		}

		void DestroyRenderThread(RenderThread*& renderThread)
		{
			delete renderThread;
			renderThread = nullptr;
		}

		void AddRenderThreadDraw(RenderThread* renderThread, const Draw& draw)
		{
			renderThread->AddDraw(draw);
		}

		bool GetRenderThreadHasRecordedCommands(RenderThread* renderThread)
		{
			return renderThread->GetHasRecordedCommands();
		}

		void ResetRenderThread(RenderThread* renderThread)
		{
			renderThread->Reset();
		}

		void MainThreadEndCommands(RenderThread* renderThread)
		{
			renderThread->MainThreadEndCommands();
		}

		void SignalRenderThreadEndCommands(RenderThread* renderThread)
		{
			renderThread->SignalEndCommands();
		}

		double GetRenderThreadDuration(RenderThread* renderThread)
		{
			return renderThread->GetRenderThreadTime();
		}
	} // Native
} // Renderer
