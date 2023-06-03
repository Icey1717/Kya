#include "Pipeline.h"
#include "MD5.h"
#include "VulkanShader.h"
#include "VulkanReflect.h"
#include "renderer.h"
#include "stdexcept"
#include "VulkanRenderer.h"
#include <iterator>

namespace PS2_Internal {
	std::unordered_map<PS2::PipelineKey, PS2::Pipeline, PS2::PipelineKeyHash> graphicsPipelines;
	VkRenderPass renderPass;

	PS2::Pipeline CreateGraphicsPipeline(std::string hash, VkPrimitiveTopology topology, const char* vertShaderFilePath, const char* geomShaderFilePath, const char* pixelShaderFilePath) {
		PS2::Pipeline pipeline;
		auto vertShaderCode = Shader::readFile(vertShaderFilePath);
		auto geomShaderCode = Shader::readFile(geomShaderFilePath);
		auto fragShaderCode = Shader::readFile(pixelShaderFilePath);

		VkShaderModule vertShaderModule = Shader::createShaderModule(vertShaderCode);
		VkShaderModule geomShaderModule = Shader::createShaderModule(geomShaderCode);
		VkShaderModule fragShaderModule = Shader::createShaderModule(fragShaderCode);

		ReflectData vertReflectData;
		ReflectData geomReflectData;
		ReflectData fragReflectData;

		vertReflectData = reflectDescriptorSetLayout(vertShaderCode);
		geomReflectData = reflectDescriptorSetLayout(geomShaderCode);
		fragReflectData = reflectDescriptorSetLayout(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = vertReflectData.entryPointname.c_str();

		VkPipelineShaderStageCreateInfo geomShaderStageInfo{};
		geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geomShaderStageInfo.module = geomShaderModule;
		geomShaderStageInfo.pName = geomReflectData.entryPointname.c_str();

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = fragReflectData.entryPointname.c_str();

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, geomShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		constexpr size_t vertSize = sizeof(Renderer::GSVertex);
		assert(vertReflectData.bindingDescription.stride == vertSize);

		auto attributeDescriptions = vertReflectData.attributeDescriptions;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &vertReflectData.bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = topology;
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
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineDepthStencilStateCreateInfo depthStencilAttachment{};
		depthStencilAttachment.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilAttachment.depthTestEnable = VK_FALSE;
		depthStencilAttachment.depthWriteEnable = VK_TRUE;
		depthStencilAttachment.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilAttachment.depthBoundsTestEnable = VK_FALSE;
		depthStencilAttachment.minDepthBounds = 0.0f; // Optional
		depthStencilAttachment.maxDepthBounds = 1.0f; // Optional
		depthStencilAttachment.stencilTestEnable = VK_FALSE;
		depthStencilAttachment.front = {}; // Optional
		depthStencilAttachment.back = {}; // Optional

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
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		auto AddBindings = [&pipeline](ReflectData& reflectData) {
			for (auto& layout : reflectData.GetLayouts()) {
				if (pipeline.descriptorSetLayoutBindings.find(layout.setNumber) == pipeline.descriptorSetLayoutBindings.end()) {
					pipeline.descriptorSetLayoutBindings.emplace(layout.setNumber, std::vector<VkDescriptorSetLayoutBinding>());
				}

				// Copy the bindings from vertReflectData into descriptorSetLayoutBindings
				std::copy(layout.bindings.begin(),
					layout.bindings.end(),
					std::back_inserter(pipeline.descriptorSetLayoutBindings[layout.setNumber]));
			}
		};

		AddBindings(vertReflectData);
		AddBindings(geomReflectData);
		AddBindings(fragReflectData);

		pipeline.descriptorSetLayouts.resize(pipeline.descriptorSetLayoutBindings.size());

		for (auto& bindingSet : pipeline.descriptorSetLayoutBindings) {
			VkDescriptorSetLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount = bindingSet.second.size();
			createInfo.pBindings = bindingSet.second.data();

			if (vkCreateDescriptorSetLayout(GetDevice(), &createInfo, nullptr, &pipeline.descriptorSetLayouts[bindingSet.first]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = pipeline.descriptorSetLayouts.size();
		pipelineLayoutInfo.pSetLayouts = pipeline.descriptorSetLayouts.data();

		if (vkCreatePipelineLayout(GetDevice(), &pipelineLayoutInfo, nullptr, &pipeline.layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 3;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencilAttachment;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipeline.layout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(GetDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(GetDevice(), geomShaderModule, nullptr);
		vkDestroyShaderModule(GetDevice(), vertShaderModule, nullptr);

		return pipeline;
	}
}

void PS2::CreateDefaultRenderPass() 
{
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = GetSwapchainImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription colorSubpass{};
	colorSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	colorSubpass.colorAttachmentCount = 1;
	colorSubpass.pColorAttachments = &colorAttachmentRef;
	colorSubpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency colorDependency{};
	colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	colorDependency.dstSubpass = 0;
	colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorDependency.srcAccessMask = 0;
	colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depthDependency{};
	depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depthDependency.dstSubpass = 0;
	depthDependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depthDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	depthDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	depthDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
	VkSubpassDescription subpasses[] = { colorSubpass };
	VkSubpassDependency dependencies[] = { colorDependency, depthDependency };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = subpasses;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;

	if (vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &PS2_Internal::renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void PS2::CreateGraphicsPipelines() {
	const std::vector<int> GS_PRIM_VALUES = { 0, 1, 2, 3 };
	const std::vector<int> GS_POINT_VALUES = { 0, 1 };
	const std::vector<int> GS_LINE_VALUES = { 0, 1 };
	const std::vector<int> GS_IIP_VALUES = { 0, 1 };
	const std::vector<VkPrimitiveTopology> TOPOLOGY_VALUES = { VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };

	for (VkPrimitiveTopology TOPOLOGY : TOPOLOGY_VALUES)
	{
		for (int GS_PRIM : GS_PRIM_VALUES) {
			for (int GS_POINT : GS_POINT_VALUES) {
				for (int GS_LINE : GS_LINE_VALUES) {
					for (int GS_IIP : GS_IIP_VALUES) {
						const std::string CONFIG_NAME = "GS_PRIM_" + std::to_string(GS_PRIM)
							+ "_POINT_" + std::to_string(GS_POINT)
							+ "_LINE_" + std::to_string(GS_LINE)
							+ "_IIP_" + std::to_string(GS_IIP);

						// Compute a hash value for the string
						const std::string hash = GetMD5String(CONFIG_NAME);
						const std::string gsFilePath = "shaders/ps2/" + hash + ".gs.spv";

						const PipelineKey key = { hash, TOPOLOGY };
						PS2_Internal::graphicsPipelines.emplace(key, PS2_Internal::CreateGraphicsPipeline(hash, TOPOLOGY, "shaders/ps2/ps2.vs.spv", gsFilePath.c_str(), "shaders/ps2/ps2.ps.spv"));
					}
				}
			}
		}
	}
}

PS2::PipelineMap& PS2::GetPipelines()
{
	return PS2_Internal::graphicsPipelines;
}
