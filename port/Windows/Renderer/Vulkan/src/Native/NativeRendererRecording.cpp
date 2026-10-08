#include "NativeRendererInternal.h"
#include "Blending.h"

#include "NativeDebug.h"
#include "NativeDebugShapes.h"
#include "NativeShadow.h"
#include "NativeFrameBufferCopy.h"
#include "NativeFlare.h"
#include "NativeFog.h"
#include "NativeAntiAliasing.h"
#include "NativeMSAA.h"
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

		double GetAlphaTestSlowPathTime()
		{
			return GetNativeRendererState().alphaTestSlowPathTime;
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
			if (!instance.vertexColors.empty()) {
				assert(instance.vertexColors.size() == vertexBufferData.GetVertexTail());
				auto* pVertices = GetNativeRendererState().nativeVertexBuffer.GetDrawBufferData().vertex.buff + instance.vertexStart;
				for (size_t i = 0; i < instance.vertexColors.size(); ++i) {
					for (uint32_t channel = 0; channel < 4; ++channel) {
						pVertices[i].RGBA[channel] = (instance.vertexColors[i] >> (channel * 8)) & 0xff;
					}
				}
			}
		}

		static void UpdateInstanceData(Draw& draw)
		{
			SimpleTexture* pTexture = draw.pTexture;

			if (!pTexture) {
				return;
			}

			TextureRegisters textureRegisters = pTexture->GetTextureRegisters();

			NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());

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
			if (key.multisampled) MSAA::Seed(cmd, key);

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = stage.gRenderPass;
			renderPassInfo.framebuffer = key.multisampled ? MSAA::GetFramebuffer() : Shadow::GetFramebuffer(key.kind);
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

		void SetColorDepthDynamicState(const VkCommandBuffer& cmd, const Draw& drawCommand, const Draw::Instance& instance)
		{
			VkBool32 colorWriteEnable = VK_TRUE;
			VkBool32 depthWriteEnable = VK_TRUE;

			if (drawCommand.bIsAfailZOnly) {
				depthWriteEnable = VK_TRUE;
				colorWriteEnable = VK_FALSE;
			}

			if (instance.bIsZMask || !instance.gsTest.ZTE) {
				depthWriteEnable = VK_FALSE;
			}

			// Depth.
			// Both GS and native reversed-Z use larger values for nearer fragments.
			static constexpr VkCompareOp depthCompareOps[] = {
				VK_COMPARE_OP_NEVER, VK_COMPARE_OP_ALWAYS,
				VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_GREATER
			};
			vkCmdSetDepthTestEnable(cmd, instance.gsTest.ZTE ? VK_TRUE : VK_FALSE);
			vkCmdSetDepthWriteEnable(cmd, depthWriteEnable);
			vkCmdSetDepthCompareOp(cmd, depthCompareOps[instance.gsTest.ZTST]);

			// Color.
			GetNativeRendererState().vkCmdSetColorWriteEnableEXT(cmd, 1, &colorWriteEnable);

			std::array<VkBool32, 1> colorWriteMasks = {
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			};

			if ((instance.perDrawData.alphaAfail & 16) != 0 && (instance.perDrawData.alphaAfail & 3) == AFAIL_RGB_ONLY) {
				// Enable only RGB channels (disable alpha write)
				colorWriteMasks[0] = {
					VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
				};
			}

			GetNativeRendererState().vkCmdSetColorWriteMaskEXT(cmd, 0, colorWriteMasks.size(), colorWriteMasks.data());
		}

		void RecordAlphaTestedDraw(VkCommandBuffer cmd, VkPipelineLayout layout, const Draw& draw, const Draw::Instance& instance, const PerDrawData& data)
		{
			auto push = [&](const PerDrawData& values) {
				vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(values), &values);
			};
			auto record = [&]() {
				vkCmdDrawIndexed(cmd, static_cast<uint32_t>(instance.indexCount), 1, instance.indexStart, instance.vertexStart, 0);
			};
			auto setFailureWrites = [&]() {
				const VkBool32 colorWrite = data.alphaAfail != AFAIL_ZB_ONLY;
				GetNativeRendererState().vkCmdSetColorWriteEnableEXT(cmd, 1, &colorWrite);
				const VkColorComponentFlags mask = data.alphaAfail == AFAIL_RGB_ONLY ? 7 : 15;
				GetNativeRendererState().vkCmdSetColorWriteMaskEXT(cmd, 0, 1, &mask);
				vkCmdSetDepthWriteEnable(cmd, data.alphaAfail == AFAIL_ZB_ONLY && !instance.bIsZMask && instance.gsTest.ZTE);
			};

			const bool depthWrites = !instance.bIsZMask && instance.gsTest.ZTE;
			if (data.alphaEnable && data.alphaAtst == ATST_NEVER) {
				// Every fragment takes the same failure path. Preserve mesh batching.
				if (data.alphaAfail == AFAIL_KEEP || (data.alphaAfail == AFAIL_ZB_ONLY && !depthWrites)) return;
				PerDrawData failure = data;
				failure.alphaAfail |= 16;
				push(failure);
				setFailureWrites();
				record();
				SetColorDepthDynamicState(cmd, draw, instance);
				return;
			}

			if (data.alphaEnable && !depthWrites && data.alphaAfail == AFAIL_FB_ONLY) {
				// Pass and fail both write RGBA and neither can write depth.
				PerDrawData allFragments = data;
				allFragments.alphaEnable = VK_FALSE;
				push(allFragments);
				record();
				return;
			}

			const bool bReplay = data.alphaEnable && data.alphaAtst != ATST_ALWAYS && data.alphaAfail != AFAIL_KEEP &&
				(data.alphaAfail != AFAIL_ZB_ONLY || depthWrites);
			if (!bReplay) {
				ZONE_SCOPED_NAME("No Replay");
				push(data);
				record();
				return;
			}

			ZONE_SCOPED_NAME("Replay");
			const auto slowPathStart = std::chrono::steady_clock::now();

			// Keep primitive order: a whole-mesh replay changes blending/depth for overlapping triangles.
			assert(instance.indexCount % 3 == 0);

			for (uint32_t index = 0; index < instance.indexCount; index += 3) {
				ZONE_SCOPED_NAME("Replay Iteration");
				SetColorDepthDynamicState(cmd, draw, instance);
				push(data);
				vkCmdDrawIndexed(cmd, 3, 1, instance.indexStart + index, instance.vertexStart, 0);
				PerDrawData failure = data;
				failure.alphaAfail |= 16;
				push(failure);
				setFailureWrites();
				vkCmdDrawIndexed(cmd, 3, 1, instance.indexStart + index, instance.vertexStart, 0);
			}

			{
				ZONE_SCOPED_NAME("SetColorDepthDynamicState");
				SetColorDepthDynamicState(cmd, draw, instance);
			}

			GetNativeRendererState().accumulatedAlphaTestSlowPathTime +=
				std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - slowPathStart).count();
		}

		static bool TraceDraw(const Draw& draw, const Draw::Instance& instance, bool canRecord)
		{
			if (!instance.traceSubmission) {
				return true;
			}

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

				if (draw.frameBufferMaterial) {
					trace.alpha = draw.frameBufferMaterial->alpha;
				}

				auto effectiveTest = registers.test;
				effectiveTest.ZTE = instance.gsTest.ZTE;
				effectiveTest.ZTST = instance.gsTest.ZTST;
				effectiveTest.DATE = instance.gsTest.DATE;
				effectiveTest.DATM = instance.gsTest.DATM;
				trace.test = effectiveTest.CMD;
				trace.tex = registers.tex.CMD;
				trace.clamp = registers.clamp.CMD;
				trace.blend = instance.pMesh->GetPrim().ABE || (data.renderFlags & 0x20);
				trace.depthWrite = true;
				if (draw.bIsAfailZOnly) {
					trace.depthWrite = true; trace.colorWrite = false;
				}

				if (instance.bIsZMask || !instance.gsTest.ZTE) {
					trace.depthWrite = false;
				}

				trace.depthTest = instance.gsTest.ZTE != 0;
				trace.depthMode = instance.gsTest.ZTST;

				if (draw.renderPassKey.kind == ERenderPassKind::ShadowReceiver) {
					trace.depthWrite = false; trace.colorWrite = true; trace.colorMask = 15;
					trace.blend = true; trace.depthTest = true; trace.depthMode = 3;
				}

				if (draw.renderPassKey.kind == ERenderPassKind::ShadowMask) {
					trace.blend = false;
				}
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
				ZONE_SCOPED_NAME("DrawCommandRecorder::RecordDrawCommand");
				if (!bInRenderPass || drawCommand.bRenderPassDirty || currentRenderPassKey.multisampled != drawCommand.renderPassKey.multisampled) {
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
					if (drawCommand.frameBufferMaterial) {
						Renderer::Debug::BeginLabel(cmd, "Framebuffer Material TFX %u", drawCommand.frameBufferMaterial->textureFunction);
					}

					PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

					std::optional<uint> primState;
					std::optional<bool> alphaBlendState;
					std::optional<uint64_t> effectiveAlphaState;

					if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
						pTexture->GetName();
					}

					for (auto& instance : drawCommand.instances) {
						ZONE_SCOPED_NAME("Instance");
						if (instance.indexCount == 0) {
							TraceDraw(drawCommand, instance, false);
							continue;
						}

						NATIVE_LOG_VERBOSE(LogLevel::Verbose, "RecordDrawCommand: {} LD {} AST {}", pTexture->GetName(), instance.lightingDataIndex, instance.animStDataIndex);

						Renderer::Debug::BeginLabel(cmd, "%s", instance.pMesh->GetName().c_str());
						
						instance.perDrawData.projXView = drawCommand.projMatrix * drawCommand.viewMatrix;

						GIFReg::GSAlpha effectiveAlpha = pTexture->GetTextureRegisters().alpha;
						if ((instance.perDrawData.renderFlags & 0x20) != 0) {
							effectiveAlpha = instance.gsAlpha;
						}

						if (drawCommand.frameBufferMaterial) {
							effectiveAlpha.CMD = drawCommand.frameBufferMaterial->alpha;
						}

						SetBlendConstants(effectiveAlpha, cmd);

						const bool bAlphaBlendEnabled = instance.pMesh->GetPrim().ABE || ((instance.perDrawData.renderFlags & 0x20) != 0);
						instance.perDrawData.blendMode = (bShadowReceiver || bShadowMask) ? 0 : ResolveBlendState(effectiveAlpha, bAlphaBlendEnabled).hwBlendMode;
						vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PerDrawData), &instance.perDrawData);
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
							vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
							vkCmdSetDepthTestEnable(cmd, VK_TRUE);
							vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
							vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
							VkBool32 colorWriteEnable = VK_TRUE;
							GetNativeRendererState().vkCmdSetColorWriteEnableEXT(cmd, 1, &colorWriteEnable);
							const VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
							GetNativeRendererState().vkCmdSetColorWriteMaskEXT(cmd, 0, 1, &colorWriteMask);
						}
						else {
							SetColorDepthDynamicState(cmd, drawCommand, instance);
							vkCmdSetCullMode(cmd, bShadowMask ? VK_CULL_MODE_NONE : instance.cullMode);
#ifndef NDEBUG
							// Review the mesh and cull mode on a submitted scene draw.
							if (!bShadowMask && instance.cullMode != VK_CULL_MODE_NONE && IsDebuggerPresent()) {
								//__debugbreak();
							}
#endif
						}

						VkDescriptorSet descriptorSet = instance.descriptorSet ? instance.descriptorSet : drawCommand.descriptorSet;
						if (bShadowReceiver) {
							descriptorSet = Shadow::GetReceiverDescriptorSet(GetCurrentFrame());
						}

						if (drawCommand.frameBufferMaterial) {
							descriptorSet = FrameBufferCopy::GetDescriptorSet(GetCurrentFrame());
						}

						const std::array sets{ GetNativeRendererState().frameDescriptorSets[GetCurrentFrame()], descriptorSet };
						vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);

						if (TraceDraw(drawCommand, instance, true)) {
#ifndef NDEBUG
							// Report activation once without interrupting rendering.
							static bool environmentMappingReported = false;
							if (!environmentMappingReported && currentRenderPassKey.kind == ERenderPassKind::Main &&
								(instance.perDrawData.renderFlags & 0x40) != 0 && (instance.perDrawData.stripFlags & 0x8000000) != 0) {
								environmentMappingReported = true;
								NATIVE_LOG(LogLevel::Info, "Environment mapping draw: mesh={} texture={} layer={} flags=0x{:x} data={} indices={}",
									instance.pMesh->GetName(), pTexture->GetName(), pTexture->GetLayerIndex(), instance.perDrawData.renderFlags,
									instance.perDrawData.lightingDataIndex, instance.indexCount);
							}
							static bool normalExtrusionReported = false;
							if (!normalExtrusionReported && currentRenderPassKey.kind == ERenderPassKind::Main &&
								(instance.perDrawData.renderFlags & 0x100) != 0) {
								normalExtrusionReported = true;
								const float normalExtrusionAmount = GetNativeRendererState().animStBuffer.GetInstanceData(instance.perDrawData.animStDataIndex).z;
								NATIVE_LOG(LogLevel::Info, "Normal extrusion draw: mesh={} flags=0x{:x} amount={} animST={} indices={}",
									instance.pMesh->GetName(), instance.perDrawData.renderFlags, normalExtrusionAmount,
									instance.perDrawData.animStDataIndex, instance.indexCount);
							}
#endif
							if (bShadowMask || bShadowReceiver) { 
								vkCmdDrawIndexed(cmd, static_cast<uint32_t>(instance.indexCount), 1, instance.indexStart, instance.vertexStart, 0); 
							}
							else {
								ZONE_SCOPED_NAME("RecordAlphaTestedDraw");
								RecordAlphaTestedDraw(cmd, pipeline.layout, drawCommand, instance, instance.perDrawData);
							}

							if (bShadowMask) {
								ZONE_SCOPED_NAME("RecordShadowMaskDraw");
								Renderer::Native::RecordShadowMaskDraw();
							}
						}

						Renderer::Debug::EndLabel(cmd);

						instanceIndex++;
					}

					if (drawCommand.frameBufferMaterial) {
						Renderer::Debug::EndLabel(cmd);
					}
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
			Flare::BeginFrame();
			Fog::BeginFrame();
			AntiAliasing::BeginFrame();
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
				enum class Type { Draw, ShadowBegin, ShadowBlur, ShadowReceiver, ShadowEnd, FrameBufferCopy, Flare, Fog, AntiAliasing } type = Type::Draw;
				Draw draw;
				FlareDraw flare;
				FogDraw fog;
				AntiAliasingDraw aa;
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
					{
						ZONE_SCOPED_NAME("Draw");
						{
							ZONE_SCOPED_NAME("Update Instance Data");
							UpdateInstanceDataForDraw(command.draw);
						}
						// The second-camera preview has no matching scene-color capture.
						if (GetNativeRendererState().preview.IsSetup() && command.draw.renderPassKey.kind == ERenderPassKind::Main && !command.draw.frameBufferMaterial) {
							ZONE_SCOPED_NAME("Save Preview Draw");
							GetNativeRendererState().preview.SaveDraw(command.draw);
						}
						RecordDrawCommands(command.draw);
						break;
					}
					case Command::Type::ShadowBegin:
					{
						ZONE_SCOPED_NAME("Shadow Begin");
						drawCommandRecorder.EndActivePass();
						Shadow::BeginMask(command.settings);
						drawCommandRecorder.BeginPass(RenderPassKey{ EClearMode::ColorDepth, ERenderPassKind::ShadowMask });
						break;
					}
					case Command::Type::ShadowBlur:
					{
						ZONE_SCOPED_NAME("Shadow Blur");
						drawCommandRecorder.EndActivePass();
						Shadow::RecordBlur(GetNativeRendererState().commandBuffers[GetCurrentFrame()]);
						break;
					}
					case Command::Type::ShadowReceiver:
					{
						ZONE_SCOPED_NAME("Shadow Receiver");
						drawCommandRecorder.EndActivePass();
						Shadow::BeginReceiver(command.viewport);
						break;
					}
					case Command::Type::ShadowEnd:
					{
						ZONE_SCOPED_NAME("Shadow End");
						drawCommandRecorder.EndActivePass();
						Shadow::End();
						break;
					}
					case Command::Type::FrameBufferCopy:
					{
						ZONE_SCOPED_NAME("Frame Buffer Copy");
						if (command.clearPending) drawCommandRecorder.BeginPass(command.capturePassKey);
						drawCommandRecorder.EndActivePass();
						FrameBufferCopy::Record(GetNativeRendererState().commandBuffers[GetCurrentFrame()]);
						break;
					}
					case Command::Type::Flare:
					{
						ZONE_SCOPED_NAME("Flare");
						if (command.clearPending) drawCommandRecorder.BeginPass(command.capturePassKey);
						drawCommandRecorder.EndActivePass();
						Flare::Record(GetNativeRendererState().commandBuffers[GetCurrentFrame()], command.flare);
						break;
					}
					case Command::Type::Fog:
					{
						ZONE_SCOPED_NAME("Fog");
						if (command.clearPending) drawCommandRecorder.BeginPass(command.capturePassKey);
						drawCommandRecorder.EndActivePass();
						Fog::Record(GetNativeRendererState().commandBuffers[GetCurrentFrame()], command.fog);
						break;
					}
					case Command::Type::AntiAliasing:
					{
						ZONE_SCOPED_NAME("Anti-Aliasing");
						if (command.clearPending) drawCommandRecorder.BeginPass(command.capturePassKey);
						drawCommandRecorder.EndActivePass();
						AntiAliasing::Record(GetNativeRendererState().commandBuffers[GetCurrentFrame()], command.aa);
						break;
					}
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
						{
							ZONE_SCOPED_NAME("Record Begin Command Buffer");
							RecordBeginCommandBuffer();
						}
						bShouldRecordBegin = false;
					}

					{
						ZONE_SCOPED_NAME("Process Commands");
						ProcessCommands();
					}
				}
			}

			void MainThreadEndCommands()
			{
				std::unique_lock<std::mutex> lock(mutex);

				if (!bRecordedCommands) {
					return;
				}

				// The main thread can drain a short queue before Run acquires the mutex.
				if (bShouldRecordBegin) {
					RecordBeginCommandBuffer();
					bShouldRecordBegin = false;
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

			void AddFog(const FogDraw& fog, const RenderPassKey& key, bool clearPending)
			{
				Command command;
				command.type = Command::Type::Fog;
				command.fog = fog;
				command.capturePassKey = key;
				command.clearPending = clearPending;
				AddCommand(command);
			}

			void AddAntiAliasing(const AntiAliasingDraw& aa, const RenderPassKey& key, bool clearPending)
			{
				Command command;
				command.type = Command::Type::AntiAliasing;
				command.aa = aa;
				command.capturePassKey = key;
				command.clearPending = clearPending;
				AddCommand(command);
			}

			void AddFlare(const FlareDraw& flare, const RenderPassKey& key, bool clearPending)
			{
				Command command;
				command.type = Command::Type::Flare;
				command.flare = flare;
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

		void AddRenderThreadFog(RenderThread* renderThread, const FogDraw& fog, const RenderPassKey& key, bool clearPending)
		{
			renderThread->AddFog(fog, key, clearPending);
		}

		void AddRenderThreadAntiAliasing(RenderThread* renderThread, const AntiAliasingDraw& aa, const RenderPassKey& key, bool clearPending)
		{
			renderThread->AddAntiAliasing(aa, key, clearPending);
		}

		void AddRenderThreadFlare(RenderThread* renderThread, const FlareDraw& flare, const RenderPassKey& key, bool clearPending)
		{
			renderThread->AddFlare(flare, key, clearPending);
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
