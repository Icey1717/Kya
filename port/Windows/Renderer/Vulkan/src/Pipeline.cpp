#include "Pipeline.h"
#include "hash.h"
#include "VulkanShader.h"
#include "VulkanReflect.h"
#include "renderer.h"
#include "stdexcept"
#include "VulkanRenderer.h"
#include "ShaderConfig.h"
#include <iterator>
#include "pcsx2/VKBuilders.h"
#include "pcsx2/Selectors.h"
#include "log.h"

namespace PS2_Internal {
	std::unordered_map<PS2::PipelineKey, Renderer::Pipeline, PS2::PipelineKeyHash> graphicsPipelines;
	VkRenderPass renderPass;

	Renderer::Pipeline CreateGraphicsPipelinePCSX2(const PS2::PipelineKey& key) 
	{
		std::string gsFilePath;
		std::string psFilePath;
		std::string vsFilePath;

		Shader::PS2::CompileShaders(key.shaderDefinitions, vsFilePath, psFilePath, gsFilePath);

		Renderer::Pipeline pipeline;
		auto vertShader = Shader::ReflectedModule(vsFilePath.c_str(), VK_SHADER_STAGE_VERTEX_BIT, true);
		auto geomShader = Shader::ReflectedModule(gsFilePath.c_str(), VK_SHADER_STAGE_GEOMETRY_BIT, true);
		auto fragShader = Shader::ReflectedModule(psFilePath.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, true);

		pipeline.key = key;

		static constexpr std::array<VkPrimitiveTopology, 3> topology_lookup = { {
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST, // Point
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST, // Line
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // Triangle
		} };

		PipelineSelector p{ key.pipelineSelector };
		GSHWDrawConfig::BlendState pbs{ key.pipelineSelector.bs };
		GSHWDrawConfig::PSSelector pps{ key.pipelineSelector.ps };
		if ((p.cms.wrgba & 0x7) == 0)
		{
			// disable blending when colours are masked
			pbs = {};
			pps.no_color1 = true;
		}

		Vulkan::GraphicsPipelineBuilder gpb;
		//SetPipelineProvokingVertex(m_features, gpb);

		pipeline.AddBindings(Renderer::EBindingStage::Vertex, vertShader.reflectData);
		pipeline.AddBindings(Renderer::EBindingStage::Geometry, geomShader.reflectData);
		pipeline.AddBindings(Renderer::EBindingStage::Fragment, fragShader.reflectData);
		pipeline.CreateDescriptorSetLayouts();
		pipeline.CreateLayout();

		// Common state
		gpb.SetPipelineLayout(pipeline.layout);
		if (IsDATEModePrimIDInit(p.ps.date))
		{
			// DATE image prepass
			//gpb.SetRenderPass(m_date_image_setup_render_passes[p.ds][0], 0);
		}
		else
		{
			//gpb.SetRenderPass(
			//	GetTFXRenderPass(p.rt, p.ds, p.ps.hdr, p.dss.date ? DATE_RENDER_PASS_STENCIL : DATE_RENDER_PASS_NONE,
			//		p.IsRTFeedbackLoop(), p.IsTestingAndSamplingDepth(),
			//		p.rt ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			//		p.ds ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE),
			//	0);
		}

		gpb.SetRenderPass(renderPass, 0);

		gpb.SetPrimitiveTopology(key.topology); //topology_lookup[p.topology]);
		gpb.SetRasterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		if (p.topology == static_cast<u8>(GSHWDrawConfig::Topology::Line))
			gpb.SetLineRasterizationMode(VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT);
		gpb.SetDynamicViewportAndScissorState();
		gpb.AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
		gpb.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);

		// Shaders
		gpb.SetVertexShader(vertShader.shaderModule, vertShader.reflectData.entryPointname.c_str());
		gpb.SetGeometryShader(geomShader.shaderModule, geomShader.reflectData.entryPointname.c_str());
		gpb.SetFragmentShader(fragShader.shaderModule, fragShader.reflectData.entryPointname.c_str());
		
		// IA
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		//constexpr size_t vertSize = sizeof(Renderer::GSVertex);
		// Don't actually need the stride to be reflected, just use the vertex size and we pad the rest.
		//assert(vertShader.reflectData.bindingDescription.stride == vertSize);
		vertShader.reflectData.bindingDescription.stride = key.stride;

		auto attributeDescriptions = vertShader.reflectData.GetAttributes();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &vertShader.reflectData.bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		if (p.vs.expand == GSHWDrawConfig::VSExpand::None)
		{
			//constexpr size_t vertSize = sizeof(Renderer::GSVertex);
			//assert(vertShader.reflectData.bindingDescription.stride == vertSize);
			//
			//gpb.AddVertexBuffer(0, sizeof(vertSize));
			//
			//auto attributeDescriptions = vertShader.reflectData.attributeDescriptions;
			//
			//for (auto& desc : attributeDescriptions) {
			//	gpb.AddVertexAttribute(desc.location, desc.binding, desc.format, desc.offset);
			//}

			//	gpb.AddVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, 0); // ST
			//	gpb.AddVertexAttribute(1, 0, VK_FORMAT_R8G8B8A8_UINT, 8); // RGBA
			//	gpb.AddVertexAttribute(2, 0, VK_FORMAT_R32_SFLOAT, 12); // Q
			//	gpb.AddVertexAttribute(3, 0, VK_FORMAT_R16G16_UINT, 16); // XY
			//	gpb.AddVertexAttribute(4, 0, VK_FORMAT_R32_UINT, 20); // Z
			//	gpb.AddVertexAttribute(5, 0, VK_FORMAT_R16G16_UINT, 24); // UV
			//	gpb.AddVertexAttribute(6, 0, VK_FORMAT_R8G8B8A8_UNORM, 28); // FOG
			gpb.AddVertexInputState(vertexInputInfo);
		}

		// DepthStencil
		static const VkCompareOp ztst[] = {
			VK_COMPARE_OP_NEVER, VK_COMPARE_OP_ALWAYS, VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_GREATER };
		//gpb.SetDepthState((p.dss.ztst != ZTST_ALWAYS || p.dss.zwe), p.dss.zwe, ztst[p.dss.ztst]);
		gpb.SetDepthState(true, true, VK_COMPARE_OP_GREATER_OR_EQUAL);
		if (p.dss.date)
		{
			const VkStencilOpState sos{ VK_STENCIL_OP_KEEP, p.dss.date_one ? VK_STENCIL_OP_ZERO : VK_STENCIL_OP_KEEP,
				VK_STENCIL_OP_KEEP, VK_COMPARE_OP_EQUAL, 1u, 1u, 1u };
			gpb.SetStencilState(true, sos, sos);
		}

		// Blending
		if (IsDATEModePrimIDInit(p.ps.date))
		{
			// image DATE prepass
			gpb.SetBlendAttachment(0, true, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_MIN, VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_COLOR_COMPONENT_R_BIT);
		}
		else if (pbs.enable)
		{
			// clang-format off
			static constexpr std::array<VkBlendFactor, 16> vk_blend_factors = { {
				VK_BLEND_FACTOR_SRC_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
				VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_FACTOR_DST_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_FACTOR_SRC1_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
				VK_BLEND_FACTOR_CONSTANT_COLOR, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
			} };
			static constexpr std::array<VkBlendOp, 3> vk_blend_ops = { {
					VK_BLEND_OP_ADD, VK_BLEND_OP_SUBTRACT, VK_BLEND_OP_REVERSE_SUBTRACT
			} };
			// clang-format on

			gpb.SetBlendAttachment(0, true, vk_blend_factors[pbs.src_factor], vk_blend_factors[pbs.dst_factor],
				vk_blend_ops[pbs.op], VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, p.cms.wrgba);
		}
		else
		{
			gpb.SetBlendAttachment(0, false, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, p.cms.wrgba);
		}

		// Tests have shown that it's faster to just enable rast order on the entire pass, rather than alternating
		// between turning it on and off for different draws, and adding the required barrier between non-rast-order
		// and rast-order draws.
		//if (m_features.framebuffer_fetch && p.IsRTFeedbackLoop())
		//	gpb.AddBlendFlags(VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT);

		VkPipeline vkpipeline = gpb.Create(GetDevice()); // , g_vulkan_shader_cache->GetPipelineCache(true));
		if (vkpipeline != VK_NULL_HANDLE)
		{
			SetObjectName(reinterpret_cast<uint64_t>(vkpipeline), VK_OBJECT_TYPE_PIPELINE, "TFX Pipeline %08X/%llX%08X", p.vs.key, p.ps.key_hi, p.ps.key_lo);
		}

		pipeline.pipeline = vkpipeline;

		return pipeline;
	}
}

void PS2::CreateDefaultRenderPass() 
{
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
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

	if (vkCreateRenderPass(GetDevice(), &renderPassInfo, GetAllocator(), &PS2_Internal::renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	SetObjectName(reinterpret_cast<uint64_t>(PS2_Internal::renderPass), VK_OBJECT_TYPE_RENDER_PASS, "PS2 Default Render Pass");
}

PS2::PipelineMap& PS2::GetPipelines()
{
	return PS2_Internal::graphicsPipelines;
}

const Renderer::Pipeline& PS2::GetPipeline(const PipelineKey& key)
{
	size_t hash = PipelineKeyHash()(key);

	if (PS2_Internal::graphicsPipelines.find(key) == PS2_Internal::graphicsPipelines.end()) {
		VULKAN_LOG(LogLevel::Verbose, "PS2::GetPipeline Cache miss, creating pipeline for hash: {:x} - {}", hash, key.ToString().c_str());

		PS2_Internal::graphicsPipelines[key] = PS2_Internal::CreateGraphicsPipelinePCSX2(key);
		PS2_Internal::graphicsPipelines[key].key.debugData.hash = PipelineKeyHash()(key);

		VULKAN_LOG(LogLevel::Verbose, "PS2::GetPipeline Created 0x{:x}", (uintptr_t)&PS2_Internal::graphicsPipelines[key]);
	}

	auto& pipeline = PS2_Internal::graphicsPipelines[key];

	assert(pipeline.key.debugData.hash == hash);
	assert(key.pipelineSelector.bs.key == pipeline.key.pipelineSelector.bs.key);

	return pipeline;
}

void Renderer::Pipeline::AddBindings(const EBindingStage bindingStage, const ReflectData& reflectData) {
	for (auto& layout : reflectData.GetLayouts()) {
		descriptorSetLayoutBindings[layout.setNumber][bindingStage] = layout.bindings;
	}
}

void Renderer::Pipeline::CreateDescriptorSetLayouts()
{
	descriptorSetLayouts.resize(descriptorSetLayoutBindings.size());

	for (auto& layoutSet : descriptorSetLayoutBindings) {
		const int layoutSetIndex = layoutSet.first;

		const auto& layoutMap = layoutSet.second;

		auto descriptorSet = Renderer::CollectDescriptorSets(layoutMap);

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = descriptorSet.size();
		createInfo.pBindings = descriptorSet.data();

		if (vkCreateDescriptorSetLayout(GetDevice(), &createInfo, GetAllocator(), &descriptorSetLayouts[layoutSetIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		{
			int vtxCount = 0;
			int geomCount = 0;
			int fragCount = 0;

			if (layoutMap.find(EBindingStage::Vertex) != layoutMap.end()) {
				vtxCount = layoutMap.at(EBindingStage::Vertex).size();
			}

			if (layoutMap.find(EBindingStage::Geometry) != layoutMap.end()) {
				geomCount = layoutMap.at(EBindingStage::Geometry).size();
			}

			if (layoutMap.find(EBindingStage::Fragment) != layoutMap.end()) {
				fragCount = layoutMap.at(EBindingStage::Fragment).size();
			}

			SetObjectName(reinterpret_cast<uint64_t>(descriptorSetLayouts[layoutSetIndex]), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
				"Pipeline Descriptor Set Layout (set index: %d binding count: %d, [vtx: %d geom: %d frag: %d])", layoutSetIndex, createInfo.bindingCount, vtxCount, geomCount, fragCount);
		}
	}
}

void Renderer::Pipeline::CreateLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

	if (vkCreatePipelineLayout(GetDevice(), &pipelineLayoutInfo, GetAllocator(), &layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	SetObjectName(reinterpret_cast<uint64_t>(layout), VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Renderer Pipeline Layout (Layout Count: %d)", pipelineLayoutInfo.setLayoutCount);
}

void Renderer::Pipeline::CreateDescriptorPool()
{
	if (descriptorSetLayoutBindings.size() > 0) {
		Renderer::CreateDescriptorPool(descriptorSetLayoutBindings, descriptorPool);
	}
}

void Renderer::Pipeline::CreateDescriptorSets()
{
	if (descriptorSetLayoutBindings.size() > 0) {
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

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			SetObjectName(reinterpret_cast<uint64_t>(descriptorSets[i]), VK_OBJECT_TYPE_DESCRIPTOR_SET, "Pipeline descriptor set %d", i);
		}
	}
}

std::vector<VkDescriptorSetLayoutBinding> Renderer::CollectDescriptorSets(const LayoutStageMap& stageMap)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (auto& bindingStage : stageMap) {
		for (auto& binding : bindingStage.second) {
			bindings.push_back(binding);
		}
	}

	return bindings;
}

void Renderer::CreateDescriptorPool(const LayoutBindingMap& descriptorSetLayoutBindingsMap, VkDescriptorPool& descriptorPool) {
	// Create descriptor pool based on the descriptor set count from the shader

	auto& descriptorSetLayoutBindings = descriptorSetLayoutBindingsMap.at(0);
	std::vector<VkDescriptorPoolSize> poolSizes;

	auto descriptorSets = CollectDescriptorSets(descriptorSetLayoutBindings);

	for (auto& descriptorSet : descriptorSets) {
		poolSizes.push_back({ descriptorSet.descriptorType, descriptorSet.descriptorCount * MAX_FRAMES_IN_FLIGHT });
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(GetDevice(), &poolInfo, GetAllocator(), &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

std::optional<VkWriteDescriptorSet> Renderer::DescriptorWriteList::AddWrite(Renderer::EBindingStage stage, VkDescriptorType type, const VkDescriptorSet& dstSet, int dstBinding) const
{
	for (auto& write : writes) {
		if (write.stage == stage && write.descriptorType == type) {
			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = dstSet;
			writeDescriptorSet.dstBinding = dstBinding;
			writeDescriptorSet.dstArrayElement = 0;
			writeDescriptorSet.descriptorType = type;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.pBufferInfo = write.pBufferInfo;
			writeDescriptorSet.pImageInfo = write.pImageInfo;
			writeDescriptorSet.pNext = nullptr;
			return writeDescriptorSet;
		}
	}

	return std::nullopt;
}

std::vector<VkWriteDescriptorSet> Renderer::DescriptorWriteList::CreateWriteDescriptorSetList(const VkDescriptorSet& dstSet, const Renderer::LayoutBindingMap& layoutBindingMap) const
{
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	for (auto& bindingSet : layoutBindingMap) {
		for (auto& binding : bindingSet.second) {
			for (auto& write : binding.second) {
				auto maybeDescriptorSet = AddWrite(binding.first, write.descriptorType, dstSet, write.binding);

				if (maybeDescriptorSet.has_value()) {
					descriptorWrites.push_back(maybeDescriptorSet.value());
				}				
			}
		}
	}

	return descriptorWrites;
}

std::string PS2::PipelineKey::ToString() const
{
	std::string info = "Pipeline Key:";
	info += " VS: ";
	info += shaderDefinitions.vsDef;
	info += " GS: ";
	info += shaderDefinitions.gsDef;
	info += " PS: ";
	info += shaderDefinitions.psDef;
	info += " Topology: ";
	info += debugData.topology;
	return info;
}
