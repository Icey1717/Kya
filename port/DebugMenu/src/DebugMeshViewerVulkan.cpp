#include "DebugMeshViewerVulkan.h"
#include "DebugMeshViewer.h"
#include "UniformBuffer.h"
#include "Pipeline.h"
#include "NativeRenderer.h"
#include "Types.h"
#include "FrameBuffer.h"
#include "backends/imgui_impl_vulkan.h"

#include <array>
#include <optional>
#include "TextureCache.h"

namespace DebugMeshViewer {
	namespace Vulkan {
		using namespace Renderer::Native;

		PipelineMap gPipelines;
		Renderer::FrameBufferBase gFrameBuffer;

		VkSampler gFrameBufferSampler;

		VkRenderPass gRenderPass;
		Renderer::CommandBufferVector gCommandBuffers;

		UniformBuffer<VertexConstantBuffer> gVertexConstantBuffer;

		constexpr int maxDrawCommands = 64;

		using PreviewerVertexBuffer = PS2::FrameVertexBuffers<Renderer::GSVertexUnprocessed, uint16_t>;
		using PreviewerDrawCommandArray = std::array<std::pair<Renderer::SimpleTexture*, PreviewerVertexBuffer>, maxDrawCommands>;
		PreviewerDrawCommandArray gPreviewerDrawCommands;

		Renderer::SimpleTexture* pBoundTexture = nullptr;
		int drawCounter = 0;		

		void CreateFramebuffer()
		{
			gFrameBuffer.SetupBase({ gWidth, gHeight }, gRenderPass, true);
		}		

		void CreatePipelineGlsl()
		{
			PipelineKey key; 
			key.options.bGlsl = true;
			key.options.bWireframe = false;
			PipelineCreateInfo createInfo { "shaders/meshviewer.vert.spv" , "shaders/meshviewer.frag.spv", key };
			CreatePipeline(createInfo, gRenderPass, gPipelines[createInfo.key.key], "Mesh Previewer GLSL");

			createInfo.key.options.bWireframe = true;
			CreatePipeline(createInfo, gRenderPass, gPipelines[createInfo.key.key], "Mesh Previewer GLSL Wireframe");
		}

		void CreatePipelineHlsl()
		{
			Shader::CompileShader("vs_6_0", "vs_main", "meshviewer.hlsl", "shaders/meshviewer_hlsl.vert.spv", "");
			Shader::CompileShader("ps_6_0", "ps_main", "meshviewer.hlsl", "shaders/meshviewer_hlsl.frag.spv", "");

			PipelineKey key;
			key.options.bGlsl = false;
			key.options.bWireframe = false;
			PipelineCreateInfo createInfo{ "shaders/meshviewer_hlsl.vert.spv" , "shaders/meshviewer_hlsl.frag.spv", key };
			CreatePipeline(createInfo, gRenderPass, gPipelines[createInfo.key.key], "Mesh Previewer HLSL");

			createInfo.key.options.bWireframe = true;
			CreatePipeline(createInfo, gRenderPass, gPipelines[createInfo.key.key], "Mesh Previewer HLSL Wireframe");
		}

		void CreateFramebufferSampler() 
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

		Renderer::Pipeline& GetPipeline() {
			PipelineKey pipelineKey;
			pipelineKey.options.bWireframe = GetWireframe();
			pipelineKey.options.bGlsl = GetUseGlslPipeline();

			assert(gPipelines.find(pipelineKey.key) != gPipelines.end());
			return gPipelines[pipelineKey.key];
		}

		void UpdateDescriptors()
		{
			auto& pipeline = GetPipeline();

			gVertexConstantBuffer.Update(GetCurrentFrame());

			for (auto& drawCommand : gPreviewerDrawCommands) {
				Renderer::SimpleTexture* pTexture = drawCommand.first;

				if (!pTexture) {
					break;
				}

				PS2::GSSimpleTexture* pTextureData = pTexture->pRenderer;
				VkSampler& sampler = PS2::GetSampler(pTextureData->samplerSelector);

				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = pTextureData->imageView;
				imageInfo.sampler = sampler;

				const VkDescriptorBufferInfo vertexDescBufferInfo = gVertexConstantBuffer.GetDescBufferInfo(GetCurrentFrame());

				Renderer::DescriptorWriteList writeList;
				writeList.EmplaceWrite({ Renderer::EBindingStage::Vertex, &vertexDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
				writeList.EmplaceWrite({ Renderer::EBindingStage::Fragment, nullptr, &imageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

				pTextureData->UpdateDescriptorSets(pipeline, writeList);

			}
		}
	} // Vulkan

	VertexConstantBuffer& GetVertexConstantBuffer() {
		return Vulkan::gVertexConstantBuffer.GetBufferData();
	}

	PreviewerVertexBufferData& AddPreviewerDrawCommand(Renderer::SimpleTexture* pTexture)
	{
		if (Vulkan::pBoundTexture != pTexture) {
			if (Vulkan::pBoundTexture != nullptr) {
				Vulkan::drawCounter++;
			}

			assert(Vulkan::drawCounter < Vulkan::maxDrawCommands);

			Vulkan::pBoundTexture = pTexture;

			Vulkan::gPreviewerDrawCommands[Vulkan::drawCounter].first = pTexture;
		}

		return Vulkan::gPreviewerDrawCommands[Vulkan::drawCounter].second.GetDrawBufferData();
	}

	int GetPreviewerDrawCommandCount()
	{
		int count = 0;

		for (auto& drawCommand : Vulkan::gPreviewerDrawCommands) {
			if (drawCommand.first != nullptr) {
				count++;
			}
		}

		return count;
	}
} // DebugMeshViewer

void DebugMeshViewer::Vulkan::Setup()
{
	CreateRenderPass(gRenderPass, "Mesh Previewer");
	CreateFramebuffer();
	CreateFramebufferSampler();
	Renderer::CreateCommandBuffers(gCommandBuffers);
	CreatePipelineHlsl();
	CreatePipelineGlsl();

	gVertexConstantBuffer.Init();

	for (auto& drawBuffer : gPreviewerDrawCommands) {
		drawBuffer.second.Init(0x6000, 0x6000);
	}

	OnFrameBufferCreated(ImGui_ImplVulkan_AddTexture(gFrameBufferSampler, gFrameBuffer.colorImageView, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL));
}

void DebugMeshViewer::Vulkan::Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent)
{
	if (!pBoundTexture) {
		return;
	}

	const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd, &beginInfo);

	Renderer::Debug::BeginLabel(cmd, "Mesh Viewer");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = gRenderPass;
	renderPassInfo.framebuffer = gFrameBuffer.framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { gWidth, gHeight };

	std::array<VkClearValue, 2> clearColors;
	clearColors[0] = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearColors[1] = { {1.0f, 0.0f } };
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

	UpdateDescriptors();

	for (auto& drawCommand : gPreviewerDrawCommands) {
		Renderer::SimpleTexture* pTexture = drawCommand.first;

		if (!pTexture) {
			break;
		}

		PreviewerVertexBuffer& vertexBuffer = drawCommand.second;

		auto& bufferData = vertexBuffer.GetDrawBufferData();

		if (bufferData.index.tail > 0) {
			vertexBuffer.BindData(cmd);

			PS2::GSSimpleTexture* pTextureData = pTexture->pRenderer;
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &pTextureData->GetDescriptorSets(pipeline).GetSet(GetCurrentFrame()), 0, nullptr);

			vkCmdDrawIndexed(cmd, static_cast<uint32_t>(bufferData.index.tail), 1, 0, 0, 0);
		}
	}

	vkCmdEndRenderPass(cmd);

	Renderer::Debug::EndLabel(cmd);
	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GetGraphicsQueue());

	for (auto& drawCommand : gPreviewerDrawCommands) {
		drawCommand.first = nullptr;
		drawCommand.second.Reset();
		drawCommand.second.GetDrawBufferData().ResetAfterDraw();
	}

	pBoundTexture = nullptr;
	drawCounter = 0;
}
