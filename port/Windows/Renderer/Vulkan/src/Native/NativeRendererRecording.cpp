#include "NativeRendererInternal.h"

#include "NativeDebug.h"
#include "NativeDebugShapes.h"
#include "NativeShadow.h"
#include "NativeFrameBufferCopy.h"
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
				instance.perDrawData.alphaEnable = draw.frameBufferMaterial ? VK_FALSE : textureRegisters.test.ATE;
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
			GetNativeRendererState().shadowProjectionBuffer.Map(GetCurrentFrame());

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

		static const char* GetRenderPassKindName(ERenderPassKind kind)
		{
			switch (kind)
			{
			case ERenderPassKind::Main: return "Main";
			case ERenderPassKind::ShadowMask: return "ShadowMask";
			case ERenderPassKind::ShadowReceiver: return "ShadowReceiver";
			default: return "Unknown";
			}
		}

		void RecordBeginRenderPass(const RenderPassKey& key)
		{
			const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];

			const RenderStage& stage = GetNativeRendererState().renderPass[key];

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = stage.gRenderPass;
			renderPassInfo.framebuffer = Shadow::GetFramebuffer(key.kind);
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = Shadow::GetExtent(key.kind);

			std::array<VkClearValue, 2> clearColors;
			clearColors[0] = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearColors[1] = { {0.0f, 0.0f } };
			renderPassInfo.clearValueCount = clearColors.size();
			renderPassInfo.pClearValues = clearColors.data();

			Renderer::Debug::BeginLabel(cmd, "Render Pass [%s, clear: %s]", GetRenderPassKindName(key.kind), GetClearModeName(key.clearMode));

			vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(renderPassInfo.renderArea.extent.width);
			viewport.height = static_cast<float>(renderPassInfo.renderArea.extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmd, 0, 1, &viewport);
			VkRect2D scissor = { { 0, 0 }, renderPassInfo.renderArea.extent };
			if (key.kind == ERenderPassKind::ShadowReceiver) scissor = Shadow::GetReceiverScissor();
			vkCmdSetScissor(cmd, 0, 1, &scissor);

			const auto& pipeline = stage.GetPipeline();
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
			vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);

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
			if (GetNativeRendererState().activeRenderPassKey.kind == ERenderPassKind::Main) {
				DebugShapes::SaveDepth(cmd, GetNativeRendererState().frameBuffer.depthImage);
			}

			Renderer::Debug::EndLabel(cmd);
			GetNativeRendererState().activeRenderPassKey.Reset();
			GetNativeRendererState().hasActiveRenderPass = false;
		}

		void SetColorDepthDynamicState(const VkCommandBuffer& cmd, Draw& drawCommand)
		{
			VkBool32 colorWriteEnable = VK_TRUE;
			VkBool32 depthWriteEnable = drawCommand.pTexture->GetTextureRegisters().test.AFAIL != AFAIL_FB_ONLY ? VK_TRUE : VK_FALSE;
			if (drawCommand.frameBufferMaterial) depthWriteEnable = VK_TRUE;

			if (drawCommand.bIsAfailZOnly) {
				depthWriteEnable = VK_TRUE;
				colorWriteEnable = VK_FALSE;
			}

			if (drawCommand.bIsZMask) {
				depthWriteEnable = VK_FALSE;
			}

			// Depth.
			vkCmdSetDepthWriteEnable(cmd, depthWriteEnable);
			vkCmdSetDepthCompareOp(cmd, drawCommand.frameBufferMaterial ? VK_COMPARE_OP_GREATER_OR_EQUAL : VK_COMPARE_OP_GREATER);

			// Color.
			GetNativeRendererState().vkCmdSetColorWriteEnableEXT(cmd, 1, &colorWriteEnable);

			std::array<VkBool32, 1> colorWriteMasks = {
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			};

			if (!drawCommand.frameBufferMaterial && drawCommand.pTexture->GetTextureRegisters().test.AFAIL == AFAIL_RGB_ONLY) {
				// Enable only RGB channels (disable alpha write)
				colorWriteMasks[0] = {
					VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
				};
			}

			GetNativeRendererState().vkCmdSetColorWriteMaskEXT(cmd, 0, colorWriteMasks.size(), colorWriteMasks.data());
		}

        static bool TraceDraw(const Draw& draw, const Draw::Instance& instance, bool canRecord)
        {
            if (!instance.traceSubmission) return true;
            DrawTrace::Draw trace;
            trace.recorded = canRecord;
            trace.pass = static_cast<int>(draw.renderPassKey.kind);
            trace.indexStart = instance.indexStart;
            trace.indexCount = instance.indexCount;
            trace.vertexStart = instance.vertexStart;
            trace.framebuffer = draw.frameBufferMaterial.has_value();
            trace.zOnly = draw.bIsAfailZOnly;
            memcpy(trace.view.data(), &draw.viewMatrix, sizeof(float) * 16);
            memcpy(trace.projection.data(), &draw.projMatrix, sizeof(float) * 16);
            const auto& data = instance.perDrawData;
            trace.alphaTest = data.alphaEnable != 0;
            trace.alphaAtst = data.alphaAtst;
            trace.alphaAref = data.alphaAref;
            trace.alphaAfail = data.alphaAfail;
            if (draw.pTexture) {
                DrawTrace::CopyName(trace.texture, draw.pTexture->GetName().c_str());
                trace.material = draw.pTexture->GetMaterialIndex();
                trace.layer = draw.pTexture->GetLayerIndex();
                const auto& registers = draw.pTexture->GetTextureRegisters();
                trace.alpha = (data.renderFlags & 0x20) ? instance.gsAlpha.CMD : registers.alpha.CMD;
                if (draw.frameBufferMaterial) trace.alpha = draw.frameBufferMaterial->alpha;
                trace.test = registers.test.CMD;
                trace.tex = registers.tex.CMD;
                trace.clamp = registers.clamp.CMD;
                trace.blend = instance.pMesh->GetPrim().ABE || (data.renderFlags & 0x20);
                trace.depthWrite = registers.test.AFAIL != AFAIL_FB_ONLY || trace.framebuffer;
                if (draw.bIsAfailZOnly) { trace.depthWrite = true; trace.colorWrite = false; }
                if (draw.bIsZMask) trace.depthWrite = false;
                if (!trace.framebuffer && registers.test.AFAIL == AFAIL_RGB_ONLY) trace.colorMask = 7;
                trace.depthGreaterEqual = trace.framebuffer;
                if (draw.renderPassKey.kind == ERenderPassKind::ShadowReceiver) {
                    trace.depthWrite = false; trace.colorWrite = true; trace.colorMask = 15;
                    trace.blend = true; trace.depthGreaterEqual = false;
                }
                if (draw.renderPassKey.kind == ERenderPassKind::ShadowMask) trace.blend = false;
            }
            return DrawTrace::Record(instance.traceSubmission, trace);
        }

		class DrawCommandRecorder
		{
		public:
			void BeginPass(const RenderPassKey& key)
			{
				EndActivePass();
				currentRenderPassKey = key;
				RecordBeginRenderPass(currentRenderPassKey);
				bInRenderPass = true;
			}

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
                if (!pTexture) {
                    for (const auto& instance : drawCommand.instances) TraceDraw(drawCommand, instance, false);
                }

				if (pTexture && !drawCommand.instances.empty()) {
					NATIVE_LOG_VERBOSE(LogLevel::Verbose, "RecordDrawCommand {}", pTexture->GetName());

					const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];

					const Pipeline& pipeline = GetNativeRendererState().renderPass[currentRenderPassKey].GetPipeline();
					const bool bShadowReceiver = currentRenderPassKey.kind == ERenderPassKind::ShadowReceiver;
					const bool bShadowMask = currentRenderPassKey.kind == ERenderPassKind::ShadowMask;

					Debug::UpdateLabel(pTexture, cmd);
					if (drawCommand.frameBufferMaterial) Renderer::Debug::BeginLabel(cmd, "Framebuffer Material TFX %u", drawCommand.frameBufferMaterial->textureFunction);

					PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

					std::optional<uint> primState;
					std::optional<bool> alphaBlendState;
					std::optional<uint64_t> effectiveAlphaState;

					if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
						pTexture->GetName();
					}

					for (auto& instance : drawCommand.instances) {
						if (instance.indexCount == 0) {
                            TraceDraw(drawCommand, instance, false);
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
						if (drawCommand.frameBufferMaterial) effectiveAlpha.CMD = drawCommand.frameBufferMaterial->alpha;

						const bool bAlphaBlendEnabled = instance.pMesh->GetPrim().ABE || ((instance.perDrawData.renderFlags & 0x20) != 0);
						if (bShadowReceiver || bShadowMask) {
							if (!primState.has_value()) {
								vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
								primState = instance.pMesh->GetPrim().CMD;
							}
						}
						else if (!primState.has_value() || primState.value() != instance.pMesh->GetPrim().CMD || !alphaBlendState.has_value() || alphaBlendState.value() != bAlphaBlendEnabled || !effectiveAlphaState.has_value() || effectiveAlphaState.value() != effectiveAlpha.CMD) {
							primState = instance.pMesh->GetPrim().CMD;
							alphaBlendState = bAlphaBlendEnabled;
							effectiveAlphaState = effectiveAlpha.CMD;
							vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, GetBlendPipeline(currentRenderPassKey, effectiveAlpha, bAlphaBlendEnabled));
						}

						if (bShadowReceiver) {
							vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
							VkBool32 colorWriteEnable = VK_TRUE;
							GetNativeRendererState().vkCmdSetColorWriteEnableEXT(cmd, 1, &colorWriteEnable);
							const VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
							GetNativeRendererState().vkCmdSetColorWriteMaskEXT(cmd, 0, 1, &colorWriteMask);
						}
						else {
							SetColorDepthDynamicState(cmd, drawCommand);
						}

						const VkDescriptorSet* descriptorSet = drawCommand.pDescriptorSets;
						if (bShadowReceiver) descriptorSet = &Shadow::GetReceiverDescriptorSet(GetCurrentFrame());
						if (drawCommand.frameBufferMaterial) descriptorSet = &FrameBufferCopy::GetDescriptorSet(GetCurrentFrame());
						vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, descriptorSet, 0, nullptr);

                        if (TraceDraw(drawCommand, instance, true)) {
						    vkCmdDrawIndexed(cmd, static_cast<uint32_t>(instance.indexCount), 1, instance.indexStart, instance.vertexStart, 0);
                        }

						Renderer::Debug::EndLabel(cmd);

						instanceIndex++;
					}
					if (drawCommand.frameBufferMaterial) Renderer::Debug::EndLabel(cmd);
				}
			}

			void EndActivePass()
			{
				if (!bInRenderPass) return;
				const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];
				Debug::Reset(cmd);
				RecordEndRenderPass();
				bInRenderPass = false;
				currentRenderPassKey.Reset();
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

			struct Command
			{
				enum class Type { Draw, ShadowBegin, ShadowBlur, ShadowReceiver, ShadowEnd, FrameBufferCopy } type = Type::Draw;
				Draw draw;
				ShadowPassSettings settings;
				ShadowReceiverViewport viewport;
				RenderPassKey capturePassKey;
				bool clearPending = false;
			};

			void ProcessCommands()
			{
				Command command;
				while (commands.try_dequeue(command)) {
					switch (command.type) {
					case Command::Type::Draw:
						UpdateInstanceDataForDraw(command.draw);
						// The second-camera preview has no matching scene-color capture.
						if (GetNativeRendererState().preview.IsSetup() && command.draw.renderPassKey.kind == ERenderPassKind::Main && !command.draw.frameBufferMaterial) {
							GetNativeRendererState().preview.SaveDraw(command.draw);
						}
						RecordDrawCommands(command.draw);
						break;
					case Command::Type::ShadowBegin:
						drawCommandRecorder.EndActivePass();
						Shadow::BeginMask(command.settings);
						drawCommandRecorder.BeginPass(RenderPassKey{ EClearMode::ColorDepth, ERenderPassKind::ShadowMask });
						break;
					case Command::Type::ShadowBlur:
						drawCommandRecorder.EndActivePass();
						Shadow::RecordBlur(GetNativeRendererState().commandBuffers[GetCurrentFrame()]);
						break;
					case Command::Type::ShadowReceiver:
						drawCommandRecorder.EndActivePass();
						Shadow::BeginReceiver(command.viewport);
						break;
					case Command::Type::ShadowEnd:
						drawCommandRecorder.EndActivePass();
						Shadow::End();
						break;
					case Command::Type::FrameBufferCopy:
						if (command.clearPending) drawCommandRecorder.BeginPass(command.capturePassKey);
						drawCommandRecorder.EndActivePass();
						FrameBufferCopy::Record(GetNativeRendererState().commandBuffers[GetCurrentFrame()]);
						break;
					}
				}
			}

			void Run()
			{
				while (!bShouldStop) {
					std::unique_lock<std::mutex> lock(mutex);
					cv.wait(lock, [this] { return commands.peek() || bShouldStop; });

					ZONE_SCOPED_NAME("RenderThread::Run");

					if (bShouldStop) break;

					if (bShouldRecordBegin) {
						RecordBeginCommandBuffer();
						bShouldRecordBegin = false;
					}

					ProcessCommands();
				}
			}

			void MainThreadEndCommands()
			{
				std::unique_lock<std::mutex> lock(mutex);

				if (!bRecordedCommands) {
					return;
				}

				// Any leftover draws to process.
				ProcessCommands();

				MapBuffers();
				RecordEndCommandBuffer();
				timer.End();
			}

			void AddDraw(const Draw& draw)
			{
				Command command;
				command.type = Command::Type::Draw;
				command.draw = draw;
				AddCommand(command);
			}

			void AddShadowBegin(const ShadowPassSettings& settings)
			{
				Command command;
				command.type = Command::Type::ShadowBegin;
				command.settings = settings;
				AddCommand(command);
			}

			void AddShadowBlur()
			{
				Command command;
				command.type = Command::Type::ShadowBlur;
				AddCommand(command);
			}

			void AddFrameBufferCopy(const RenderPassKey& key, bool clearPending)
			{
				Command command;
				command.type = Command::Type::FrameBufferCopy;
				command.capturePassKey = key;
				command.clearPending = clearPending;
				AddCommand(command);
			}

			void AddShadowReceiver(const ShadowReceiverViewport& viewport)
			{
				Command command;
				command.type = Command::Type::ShadowReceiver;
				command.viewport = viewport;
				AddCommand(command);
			}

			void AddShadowEnd()
			{
				Command command;
				command.type = Command::Type::ShadowEnd;
				AddCommand(command);
			}

			void AddCommand(const Command& command)
			{
				commands.enqueue(command);

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
			moodycamel::ReaderWriterQueue<Command> commands;
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

		void AddRenderThreadFrameBufferCopy(RenderThread* renderThread, const RenderPassKey& key, bool clearPending)
		{
			renderThread->AddFrameBufferCopy(key, clearPending);
		}

		void AddRenderThreadShadowBegin(RenderThread* renderThread, const ShadowPassSettings& settings)
		{
			renderThread->AddShadowBegin(settings);
		}

		void AddRenderThreadShadowBlur(RenderThread* renderThread)
		{
			renderThread->AddShadowBlur();
		}

		void AddRenderThreadShadowReceiver(RenderThread* renderThread, const ShadowReceiverViewport& viewport)
		{
			renderThread->AddShadowReceiver(viewport);
		}

		void AddRenderThreadShadowEnd(RenderThread* renderThread)
		{
			renderThread->AddShadowEnd();
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
