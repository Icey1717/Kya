#include "PostProcessing.h"

#include <array>
#include <stdexcept>
#include <unordered_map>

#include "Pipeline.h"
#include "VulkanRenderer.h"
#include "FrameBuffer.h"
#include "NativeRenderer.h"
#include "../../include/renderer.h"

namespace Renderer
{
	namespace Native
	{
		namespace PostProcessing
		{
			VkRenderPass gRenderPass;
			Renderer::FrameBufferBase gFrameBuffer;
			std::unordered_map<Effect, Pipeline> gPipelines;
			VkSampler gSampler;

			void CreateRenderPass(VkRenderPass& renderPass)
			{
				VkAttachmentDescription colorAttachment{};
				colorAttachment.format = GetSwapchainImageFormat();
				colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

				colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

				VkAttachmentReference colorAttachmentRef{};
				colorAttachmentRef.attachment = 0;
				colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


				VkSubpassDescription subpass{};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.colorAttachmentCount = 1;
				subpass.pColorAttachments = &colorAttachmentRef;

				VkSubpassDependency dependency{};
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = 0;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				std::array<VkAttachmentDescription, 1> attachments = { colorAttachment };

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

				SetObjectName(reinterpret_cast<uint64_t>(renderPass), VK_OBJECT_TYPE_RENDER_PASS, "PostProcessing Render Pass");
			}

			void CreatePipeline(Effect effect, const char* fragShaderFilename)
			{
				auto& pipeline = gPipelines[effect];

				auto vertShader = Shader::ReflectedModule("shaders/postprocess.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
				auto fragShader = Shader::ReflectedModule(fragShaderFilename, VK_SHADER_STAGE_FRAGMENT_BIT);

				pipeline.AddBindings(Renderer::EBindingStage::Vertex, vertShader.reflectData);
				pipeline.AddBindings(Renderer::EBindingStage::Fragment, fragShader.reflectData);
				pipeline.CreateDescriptorSetLayouts();

				pipeline.CreateLayout(vertShader.reflectData.pushConstants);

				pipeline.CreateDescriptorPool();
				pipeline.CreateDescriptorSets();

				VkPipelineShaderStageCreateInfo shaderStages[] = { vertShader.shaderStageCreateInfo, fragShader.shaderStageCreateInfo };

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
				rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
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

				VkPipelineColorBlendStateCreateInfo colorBlending{};
				colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				colorBlending.logicOpEnable = VK_FALSE;
				colorBlending.attachmentCount = 1;
				colorBlending.pAttachments = &colorBlendAttachment;

				std::vector<VkDynamicState> dynamicStates = {
					VK_DYNAMIC_STATE_VIEWPORT,
					VK_DYNAMIC_STATE_SCISSOR,
				};

				VkPipelineDynamicStateCreateInfo dynamicState{};
				dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
				dynamicState.pDynamicStates = dynamicStates.data();

				VkPipelineDepthStencilStateCreateInfo depthState{};
				depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				depthState.depthTestEnable = VK_FALSE;
				depthState.depthWriteEnable = VK_FALSE;
				depthState.depthCompareOp = VK_COMPARE_OP_GREATER;

				VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
				vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				vertexInputInfo.vertexBindingDescriptionCount = 0;
				vertexInputInfo.pVertexBindingDescriptions = nullptr;
				vertexInputInfo.vertexAttributeDescriptionCount = 0;
				vertexInputInfo.pVertexAttributeDescriptions = nullptr;

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
				pipelineInfo.renderPass = gRenderPass;
				pipelineInfo.subpass = 0;
				pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

				if (vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, GetAllocator(), &pipeline.pipeline) != VK_SUCCESS) {
					throw std::runtime_error("failed to create graphics pipeline!");
				}

				SetObjectName(reinterpret_cast<uint64_t>(pipeline.pipeline), VK_OBJECT_TYPE_PIPELINE, "PostProcessing Pipeline");
			}

			void CreateSampler()
			{
				VkSamplerCreateInfo samplerInfo{};
				samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerInfo.magFilter = VK_FILTER_LINEAR;
				samplerInfo.minFilter = VK_FILTER_LINEAR;

				samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

				VkPhysicalDeviceProperties properties{};
				vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);

				samplerInfo.anisotropyEnable = VK_TRUE;
				samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

				samplerInfo.anisotropyEnable = VK_FALSE;
				samplerInfo.maxAnisotropy = 0.0f;

				samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
				samplerInfo.unnormalizedCoordinates = VK_FALSE;
				samplerInfo.compareEnable = VK_FALSE;
				samplerInfo.compareOp = VK_COMPARE_OP_NEVER;

				samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				samplerInfo.mipLodBias = 0.0f;
				samplerInfo.minLod = -FLT_MAX;
				samplerInfo.maxLod = FLT_MAX;

				if (vkCreateSampler(GetDevice(), &samplerInfo, GetAllocator(), &gSampler) != VK_SUCCESS) {
					throw std::runtime_error("failed to create texture sampler!");
				}
			}

			void UpdateDescriptorSets()
			{
				VkImageView colorImageView = GetFrameBuffer().colorImageView;

				for (auto& [effect, pipeline] : gPipelines) {
					for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
						std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

						VkDescriptorImageInfo imageInfo{};
						imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfo.imageView = colorImageView;
						imageInfo.sampler = gSampler;

						descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						descriptorWrites[0].dstSet = pipeline.descriptorSets[i];
						descriptorWrites[0].dstBinding = 0;
						descriptorWrites[0].dstArrayElement = 0;
						descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						descriptorWrites[0].descriptorCount = 1;
						descriptorWrites[0].pImageInfo = &imageInfo;

						vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
					}
				}
			}
		}
	}
}

void Renderer::Native::PostProcessing::Setup()
{
	CreateRenderPass(gRenderPass);
	gFrameBuffer.SetupBase({ gWidth, gHeight }, gRenderPass, false);
	CreatePipeline(Effect::AlphaFix, "shaders/alphafix.frag.spv");
	CreatePipeline(Effect::Fade, "shaders/fade.frag.spv");
	CreateSampler();
	UpdateDescriptorSets();
}

void Renderer::Native::PostProcessing::AddPostProcessEffect(const VkCommandBuffer& cmd, Effect effect)
{
	auto& pipeline = gPipelines[effect];

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = gRenderPass;
	renderPassInfo.framebuffer = gFrameBuffer.framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { gWidth, gHeight };

	Debug::BeginLabel(cmd, "Post Processing - %s", GetEffectName(effect).c_str());

	vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &pipeline.descriptorSets[GetCurrentFrame()], 0, nullptr);
	vkCmdDraw(cmd, 3, 1, 0, 0); // Draw a single triangle to cover the screen
	vkCmdEndRenderPass(cmd);

	Debug::EndLabel(cmd);
}

const VkImageView& Renderer::Native::PostProcessing::GetColorImageView()
{
	return gFrameBuffer.colorImageView;
}

void Renderer::Native::PostProcessing::UpdateDescriptorSets(const Effect effect, const int frameIndex, const DescriptorWriteList& writeList)
{
	auto& pipeline = gPipelines[effect];

	std::vector<VkWriteDescriptorSet> descriptorWrites = writeList.CreateWriteDescriptorSetList(pipeline.descriptorSets[frameIndex], pipeline.descriptorSetLayoutBindings);

	if (descriptorWrites.size() > 0) {
		vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

std::string Renderer::Native::PostProcessing::GetEffectName(Effect effect)
{
	switch (effect) {
	case Effect::Greyscale:
		return "Greyscale";
	case Effect::AlphaFix:
		return "Alpha Fix";
	case Effect::Fade:
		return "Fade";
	default:
		return "Unknown";
	}
}
