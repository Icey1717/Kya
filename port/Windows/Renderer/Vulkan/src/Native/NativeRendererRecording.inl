namespace Renderer
{
	namespace Native
	{
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

			gActiveRenderPassKey = key;
			gHasActiveRenderPass = true;
		}

		static void RecordEndRenderPass()
		{
			const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];
			vkCmdEndRenderPass(cmd);

			Renderer::Debug::EndLabel(cmd);
			gActiveRenderPassKey.Reset();
			gHasActiveRenderPass = false;
		}

		static void SetColorDepthDynamicState(const VkCommandBuffer& cmd, Draw& drawCommand)
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
				if (!bInRenderPass || drawCommand.bRenderPassDirty) {
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

					if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
						pTexture->GetName();
					}

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

						vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, drawCommand.pDescriptorSets, dynamicOffsets.size(), dynamicOffsets.data());

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

			// Explicitly clear the framebuffer attachments
			VkClearColorValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f} };
			VkClearDepthStencilValue depthStencil = { 0.0f, 0 };

			// Color attachment subresource range
			VkImageSubresourceRange colorRange{};
			colorRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorRange.baseMipLevel = 0;
			colorRange.levelCount = 1;
			colorRange.baseArrayLayer = 0;
			colorRange.layerCount = 1;

			// Depth attachment subresource range
			VkImageSubresourceRange depthRange{};
			depthRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			depthRange.baseMipLevel = 0;
			depthRange.levelCount = 1;
			depthRange.baseArrayLayer = 0;
			depthRange.layerCount = 1;

			VulkanImage::TransitionImageLayout(gFrameBuffer.colorImage, GetSwapchainImageFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, colorRange.aspectMask, cmd);
			VulkanImage::TransitionImageLayout(gFrameBuffer.depthImage, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, depthRange.aspectMask, cmd);

			vkCmdClearColorImage(cmd, gFrameBuffer.colorImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &colorRange);
			vkCmdClearDepthStencilImage(cmd, gFrameBuffer.depthImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &depthStencil, 1, &depthRange);

			// Transition to read optimal for rendering
			VulkanImage::TransitionImageLayout(gFrameBuffer.colorImage, GetSwapchainImageFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, colorRange.aspectMask, cmd);
			VulkanImage::TransitionImageLayout(gFrameBuffer.depthImage, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, depthRange.aspectMask, cmd);
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
			if (gHasActiveRenderPass) {
				const RenderStage& stage = gRenderPass[gActiveRenderPassKey];
				DebugShapes::Record(cmd, stage.GetDebugLinePipeline());
			}

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

			void UpdateInstanceDataForDraw(Draw& draw)
			{
				for (auto& instance : draw.instances) {
					FillIndexData(instance);
				}

				drawInstanceDataUpdate.UpdateInstanceData(draw);
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
				drawInstanceDataUpdate.Reset();
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
				NATIVE_LOG(LogLevel::Info, "SignalEndCommands");
			}

		private:

			std::thread thread;
			moodycamel::ReaderWriterQueue<Draw> draws;
			InstanceDataUpdate drawInstanceDataUpdate;
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

		std::unique_ptr<RenderThread> gRenderThread;

		double GetRenderThreadTime()
		{
			return gRenderThread->GetRenderThreadTime();
		}
	} // Native
} // Renderer
