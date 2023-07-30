#include "Pipeline.h"
#include "hash.h"
#include "VulkanShader.h"
#include "VulkanReflect.h"
#include "renderer.h"
#include "stdexcept"
#include "VulkanRenderer.h"
#include <iterator>

namespace PS2_Internal {
	std::unordered_map<PS2::PipelineKey, Renderer::Pipeline, PS2::PipelineKeyHash> graphicsPipelines;
	VkRenderPass renderPass;

	Renderer::Pipeline CreateGraphicsPipeline(std::string hash, VkPrimitiveTopology topology, const char* vertShaderFilePath, const char* geomShaderFilePath, const char* pixelShaderFilePath) {
		Renderer::Pipeline pipeline;
		auto vertShader = Shader::ReflectedModule(vertShaderFilePath, VK_SHADER_STAGE_VERTEX_BIT, true);
		auto geomShader = Shader::ReflectedModule(geomShaderFilePath, VK_SHADER_STAGE_GEOMETRY_BIT, true);
		auto fragShader = Shader::ReflectedModule(pixelShaderFilePath, VK_SHADER_STAGE_FRAGMENT_BIT, true);

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShader.shaderStageCreateInfo, geomShader.shaderStageCreateInfo, fragShader.shaderStageCreateInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		constexpr size_t vertSize = sizeof(Renderer::GSVertex);
		assert(vertShader.reflectData.bindingDescription.stride == vertSize);

		auto attributeDescriptions = vertShader.reflectData.attributeDescriptions;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &vertShader.reflectData.bindingDescription;
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

		pipeline.AddBindings(vertShader.reflectData);
		pipeline.AddBindings(geomShader.reflectData);
		pipeline.AddBindings(fragShader.reflectData);
		pipeline.UpdateDescriptorSetLayouts();
		pipeline.CreateLayout();

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

	SetObjectName("PS2 Render Pass", (uint64_t)PS2_Internal::renderPass, VK_OBJECT_TYPE_RENDER_PASS);
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

void Renderer::Pipeline::AddBindings(ReflectData& reflectData) {
	for (auto& layout : reflectData.GetLayouts()) {
		if (descriptorSetLayoutBindings.find(layout.setNumber) == descriptorSetLayoutBindings.end()) {
			descriptorSetLayoutBindings.emplace(layout.setNumber, std::vector<VkDescriptorSetLayoutBinding>());
		}

		// Copy the bindings from vertReflectData into descriptorSetLayoutBindings
		std::copy(layout.bindings.begin(),
			layout.bindings.end(),
			std::back_inserter(descriptorSetLayoutBindings[layout.setNumber]));
	}
}

void Renderer::Pipeline::UpdateDescriptorSetLayouts()
{
	descriptorSetLayouts.resize(descriptorSetLayoutBindings.size());

	for (auto& bindingSet : descriptorSetLayoutBindings) {
		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = bindingSet.second.size();
		createInfo.pBindings = bindingSet.second.data();

		if (vkCreateDescriptorSetLayout(GetDevice(), &createInfo, nullptr, &descriptorSetLayouts[bindingSet.first]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
}

void Renderer::Pipeline::CreateLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

	if (vkCreatePipelineLayout(GetDevice(), &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void Renderer::Pipeline::CreateDescriptorPool()
{
	auto& descriptorSetLayoutBindingsZero = descriptorSetLayoutBindings[0];
	// Create descriptor pool based on the descriptor set count from the shader
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (uint32_t i = 0; i < descriptorSetLayoutBindingsZero.size(); ++i) {
		auto& descriptorSet = descriptorSetLayoutBindingsZero[i];
		poolSizes.push_back({ descriptorSet.descriptorType, descriptorSet.descriptorCount * MAX_FRAMES_IN_FLIGHT });
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(GetDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void Renderer::Pipeline::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayouts[0]);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(GetDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}
