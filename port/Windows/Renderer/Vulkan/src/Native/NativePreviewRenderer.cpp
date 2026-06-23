#include "NativeRendererInternal.h"

#include "NativeDebug.h"
#include "VulkanRenderer.h"

#include "glm/gtc/type_ptr.inl"

#include <array>
#include <stdexcept>

namespace Renderer
{
	namespace Native
	{
		void NativePreviewRenderer::Setup(int width, int height, const VkRenderPass& renderPass)
		{
			this->width = width;
			this->height = height;

			frameBuffer.SetupBase({ width, height }, renderPass, true);

			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.maxAnisotropy = 1.0f;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			if (vkCreateSampler(GetDevice(), &samplerInfo, GetAllocator(), &frameBufferSampler) != VK_SUCCESS) {
				throw std::runtime_error("failed to create preview framebuffer sampler");
			}

			Renderer::CreateCommandBuffers(commandBuffers);

			SetObjectName(reinterpret_cast<uint64_t>(frameBufferSampler), VK_OBJECT_TYPE_SAMPLER, "Actor Preview Sampler");
			SetObjectName(reinterpret_cast<uint64_t>(frameBuffer.framebuffer), VK_OBJECT_TYPE_FRAMEBUFFER, "Actor Preview Framebuffer");

			setup = true;
		}

		void NativePreviewRenderer::RecordPass(Renderer::CommandBufferList& commandBufferList,
			std::unordered_map<RenderPassKey, RenderStage, RenderPassKeyHash>& renderPasses,
			NativeVertexBuffer& nativeVertexBuffer,
			PFN_vkCmdSetColorWriteEnableEXT vkCmdSetColorWriteEnableEXT,
			PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT)
		{
			if (!enabled || !setup || savedDraws.empty()) {
				return;
			}

			const VkCommandBuffer& cmd = commandBuffers[GetCurrentFrame()];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(cmd, &beginInfo);

			Renderer::Debug::BeginLabel(cmd, "Actor Preview");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPasses[RenderPassKey::Empty].gRenderPass;
			renderPassInfo.framebuffer = frameBuffer.framebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			std::array<VkClearValue, 2> clearColors;
			clearColors[0] = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearColors[1] = { {0.0f, 0.0f} };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
			renderPassInfo.pClearValues = clearColors.data();

			vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(width);
			viewport.height = static_cast<float>(height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmd, 0, 1, &viewport);

			const VkRect2D scissor = { {0, 0}, { static_cast<uint32_t>(width), static_cast<uint32_t>(height) } };
			vkCmdSetScissor(cmd, 0, 1, &scissor);

			nativeVertexBuffer.BindBuffers(cmd);

			const Pipeline& pipeline = renderPasses[RenderPassKey::Empty].GetPipeline();
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

			const std::array<VkBool32, 1> colorWriteMasks = {
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			};

			const glm::mat4 previewProjXView = projMatrix * viewMatrix;

			for (auto& draw : savedDraws) {
				if (!draw.pTexture || draw.instances.empty()) {
					continue;
				}

				SetColorDepthDynamicState(cmd, draw);

				for (auto& instance : draw.instances) {
					if (instance.indexCount == 0) {
						continue;
					}

					vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, GetBlendPipeline(RenderPassKey::Empty, draw.pTexture->GetTextureRegisters().alpha, instance.pMesh->GetPrim().ABE));

					PerDrawData previewPerDrawData = instance.perDrawData;
					previewPerDrawData.projXView = previewProjXView;
					vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PerDrawData), &previewPerDrawData);

					const VkBool32 colorWriteEnable = draw.bIsAfailZOnly ? VK_FALSE : VK_TRUE;
					vkCmdSetColorWriteEnableEXT(cmd, 1, &colorWriteEnable);
					vkCmdSetColorWriteMaskEXT(cmd, 0, colorWriteMasks.size(), colorWriteMasks.data());

					vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, draw.pDescriptorSets, 0, nullptr);
					vkCmdDrawIndexed(cmd, static_cast<uint32_t>(instance.indexCount), 1, instance.indexStart, instance.vertexStart, 0);
				}
			}

			vkCmdEndRenderPass(cmd);
			Renderer::Debug::EndLabel(cmd);
			vkEndCommandBuffer(cmd);

			commandBufferList.push_back(cmd);
		}

		void NativePreviewRenderer::SaveDraw(const Draw& draw)
		{
			savedDraws.push_back(draw);
		}

		void NativePreviewRenderer::ClearSavedDraws()
		{
			savedDraws.clear();
		}

		void NativePreviewRenderer::SetCamera(const float* viewMatrix, const float* projMatrix)
		{
			this->viewMatrix = glm::make_mat4(viewMatrix);
			this->projMatrix = glm::make_mat4(projMatrix);
			enabled = true;
		}

		void NativePreviewRenderer::ClearCamera()
		{
			enabled = false;
		}

		bool NativePreviewRenderer::IsSetup() const
		{
			return setup;
		}

		const VkSampler& NativePreviewRenderer::GetSampler() const
		{
			return frameBufferSampler;
		}

		const VkImageView& NativePreviewRenderer::GetColorImageView() const
		{
			return frameBuffer.colorImageView;
		}

		void SetupPreview(int width, int height)
		{
			GetNativeRendererState().preview.Setup(width, height, GetNativeRendererState().renderPass[RenderPassKey::Empty].gRenderPass);
		}

		void SetPreviewCamera(const float* viewMatrix, const float* projMatrix)
		{
			GetNativeRendererState().preview.SetCamera(viewMatrix, projMatrix);
		}

		void ClearPreviewCamera()
		{
			GetNativeRendererState().preview.ClearCamera();
		}

		const VkSampler& GetPreviewSampler()
		{
			return GetNativeRendererState().preview.GetSampler();
		}

		const VkImageView& GetPreviewColorImageView()
		{
			return GetNativeRendererState().preview.GetColorImageView();
		}
	}
}
