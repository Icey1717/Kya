#include "VulkanPS2.h"
#include "VulkanReflect.h"
#include "renderer.h"
#include "VulkanRenderer.h"
#include <stdexcept>
#include <array>
#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include <unordered_map>
#include "MD5.h"
#include <iterator>

void* pImage = nullptr;

class alignas(16) GSVector4
{
public:
	union
	{
		struct { float x, y, z, w; };
	};

	static GSVector4 zero() {
		GSVector4 zeroed;
		memset(&zeroed, 0, sizeof(GSVector4));
		return zeroed;
	}

	__forceinline explicit GSVector4() {
		
	}

	__forceinline explicit GSVector4(float f)
	{
		x = f;
		y = f;
		z = f;
		w = f;
	}

	__forceinline explicit GSVector4(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};

class alignas(16) GSVector4i
{
	//static GSVector4i m_xff[17];
	//static GSVector4i m_x0f[17];

public:
	union
	{
		struct { int x, y, z, w; };
		struct { int r, g, b, a; };
		struct { int left, top, right, bottom; };
		int v[4];
		float f32[4];
		int8_t i8[16];
		int16_t i16[8];
		int32_t i32[4];
		int64_t  i64[2];
		uint8_t u8[16];
		uint16_t u16[8];
		uint32_t u32[4];
		uint64_t u64[2];
	};

	static GSVector4i zero() {
		GSVector4i zeroed;
		memset(&zeroed, 0, sizeof(GSVector4i));
		return zeroed;
	}
};

using LayoutBindingMap = std::unordered_map<int, std::vector<VkDescriptorSetLayoutBinding>>;
using LayoutVector = std::vector<VkDescriptorSetLayout>;

struct Pipeline {
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	LayoutVector descriptorSetLayouts;
	// Set - Descriptor Binding
	LayoutBindingMap descriptorSetLayoutBindings;
};

namespace PS2_Internal {

	struct PipelineKey {
		std::string hash;
		VkPrimitiveTopology topology;
	};

	bool operator==(const PipelineKey& lhs, const PipelineKey& rhs) {
		return lhs.hash == rhs.hash && lhs.topology == rhs.topology;
	}

	struct PipelineKeyHash {
		size_t operator()(const PipelineKey& key) const {
			size_t hash = 0;
			// combine the hash of the string and the primitive topology
			std::hash<std::string> stringHasher;
			hash ^= stringHasher(key.hash) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<int>()(key.topology) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			return hash;
		}
	};

	std::unordered_map<PipelineKey, Pipeline, PipelineKeyHash> graphicsPs2Pipelines;
	VkRenderPass renderPassPs2;


	struct alignas(32) VSConstantBuffer
	{
		GSVector4 VertexScale;
		GSVector4 VertexOffset;
		GSVector4 Texture_Scale_Offset;

		VSConstantBuffer()
		{
			VertexScale = GSVector4::zero();
			VertexOffset = GSVector4::zero();
			Texture_Scale_Offset = GSVector4::zero();
		}
	};

	struct alignas(32) PSConstantBuffer
	{
		GSVector4 FogColor_AREF;
		GSVector4 HalfTexel;
		GSVector4 WH;
		GSVector4 MinMax;
		GSVector4 MinF_TA;
		GSVector4i MskFix;
		GSVector4i ChannelShuffle;
		GSVector4i FbMask;

		GSVector4 TC_OffsetHack;
		GSVector4 Af;

		PSConstantBuffer()
		{
			FogColor_AREF = GSVector4::zero();
			HalfTexel = GSVector4::zero();
			WH = GSVector4::zero();
			MinMax = GSVector4::zero();
			MinF_TA = GSVector4::zero();
			MskFix = GSVector4i::zero();
			ChannelShuffle = GSVector4i::zero();
			FbMask = GSVector4i::zero();
			Af = GSVector4::zero();
		}
	};

	template<typename VertexType, typename IndexType>
	class VertexBufferPS2 {
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;

		int vertexHead;
		int vertexMax;
		int indexHead;
		int indexMax;

	public:
		VertexBufferPS2(int vertexCount = 0x1000, int indexCount = 0x1000)
			: vertexHead(0)
			, vertexMax(vertexCount)
			, indexHead(0)
			, indexMax(indexCount)
		{
			VkDeviceSize bufferSize = sizeof(VertexType) * vertexCount;
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

			bufferSize = sizeof(IndexType) * indexCount;
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);
		}

		const VkBuffer& GetVertexBuffer() {
			return vertexBuffer;
		}

		const VkBuffer& GetIndexBuffer() {
			return indexBuffer;
		}

		VkDeviceSize MapVertices(VertexType* buff, int vertexCount) {
			assert(vertexHead + vertexCount < vertexMax);
			VkDeviceSize bufferSize = sizeof(VertexType) * vertexCount;
			const VkDeviceSize bufferOffset = vertexHead * sizeof(VertexType);
			void* data;
			vkMapMemory(GetDevice(), vertexBufferMemory, bufferOffset, bufferSize, 0, &data);
			memcpy(data, buff, (size_t)bufferSize);
			vkUnmapMemory(GetDevice(), vertexBufferMemory);

			vertexHead += vertexCount;
			return bufferOffset;
		}

		VkDeviceSize MapIndices(IndexType* buff, int indexCount) {
			assert(indexHead + indexCount < indexMax);
			VkDeviceSize bufferSize = sizeof(IndexType) * indexCount;
			const VkDeviceSize bufferOffset = indexHead * sizeof(IndexType);
			void* data;
			vkMapMemory(GetDevice(), indexBufferMemory, bufferOffset, bufferSize, 0, &data);
			memcpy(data, buff, (size_t)bufferSize);
			vkUnmapMemory(GetDevice(), indexBufferMemory);

			indexHead += indexCount;
			return bufferOffset;
		}

		void Reset() {
			vertexHead = 0;
			indexHead = 0;
		}
	};
	VSConstantBuffer vertexConstBuffer;
	PSConstantBuffer pixelConstBuffer;
	std::vector<VkBuffer> vextexUniformBuffers;
	std::vector<VkDeviceMemory> vertexUniformBuffersMemory;
	std::vector<VkBuffer> pixelUniformBuffers;
	std::vector<VkDeviceMemory> pixelUniformBuffersMemory;

	std::vector<VertexBufferPS2<Renderer::GSVertex, uint16_t>> vertexBuffers;

	template<typename VertexType>
	VkDeviceSize MapVertices(VertexType* buff, int vertexCount) {
		return vertexBuffers[GetCurrentFrame()].MapVertices(buff, vertexCount);
	}

	template<typename IndexType>
	VkDeviceSize MapIndices(IndexType* buff, int indexCount) {
		return vertexBuffers[GetCurrentFrame()].MapIndices(buff, indexCount);
	}

	const VkBuffer& GetVertexBuffer() {
		return vertexBuffers[GetCurrentFrame()].GetVertexBuffer();
	}

	const VkBuffer& GetIndexBuffer() {
		return vertexBuffers[GetCurrentFrame()].GetIndexBuffer();
	}

	void ResetBuffer() {
		vertexBuffers[GetCurrentFrame()].Reset();
	}

	void createUniformBuffers() {
		const VkDeviceSize vertexBufferSize = sizeof(VSConstantBuffer);
		const VkDeviceSize pixelBufferSize = sizeof(PSConstantBuffer);

		vextexUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		vertexUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

		pixelUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		pixelUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vextexUniformBuffers[i], vertexUniformBuffersMemory[i]);
			CreateBuffer(pixelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, pixelUniformBuffers[i], pixelUniformBuffersMemory[i]);
		}
	}

	void createDescriptorSets() {
		for (auto& pipeline : graphicsPs2Pipelines) {
			std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline.second.descriptorSetLayouts[0]);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = pipeline.second.descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
			allocInfo.pSetLayouts = layouts.data();

			pipeline.second.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(GetDevice(), &allocInfo, pipeline.second.descriptorSets.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				VkDescriptorBufferInfo vertexConstantBuffer{};
				vertexConstantBuffer.buffer = vextexUniformBuffers[i];
				vertexConstantBuffer.offset = 0;
				vertexConstantBuffer.range = sizeof(VSConstantBuffer);

				std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = pipeline.second.descriptorSets[i];
				descriptorWrites[0].dstBinding = 5;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &vertexConstantBuffer;

				VkDescriptorBufferInfo pixelConstantBuffer{};
				pixelConstantBuffer.buffer = pixelUniformBuffers[i];
				pixelConstantBuffer.offset = 0;
				pixelConstantBuffer.range = sizeof(PSConstantBuffer);

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = pipeline.second.descriptorSets[i];
				descriptorWrites[1].dstBinding = 6;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pBufferInfo = &pixelConstantBuffer;

				vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}
	}

	void createDescriptorPool() {
		for (auto& pipeline : graphicsPs2Pipelines) {
			auto& descriptorSetLayoutBindings = pipeline.second.descriptorSetLayoutBindings[0];
			// Create descriptor pool based on the descriptor set count from the shader
			std::vector<VkDescriptorPoolSize> poolSizes;
			for (uint32_t i = 0; i < descriptorSetLayoutBindings.size(); ++i) {
				auto& descriptorSet = descriptorSetLayoutBindings[i];
				poolSizes.push_back({ descriptorSet.descriptorType, descriptorSet.descriptorCount * MAX_FRAMES_IN_FLIGHT });
			}

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

			if (vkCreateDescriptorPool(GetDevice(), &poolInfo, nullptr, &pipeline.second.descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}
	}

	void createPs2RenderPass() {
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

		if (vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &renderPassPs2) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	Pipeline createPs2GraphicsPipeline(std::string hash, VkPrimitiveTopology topology, const char* vertShaderFilePath, const char* geomShaderFilePath, const char* pixelShaderFilePath) {
		Pipeline pipeline;
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
		colorBlendAttachment.blendEnable = VK_FALSE;

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
		pipelineInfo.renderPass = renderPassPs2;
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

	void createPs2GraphicsPipelines() {
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
							//graphicsPs2Pipelines.emplace(key, createPs2GraphicsPipeline(hash, TOPOLOGY, "shaders/ps2/ps2.vs.spv", gsFilePath.c_str(), "shaders/ps2.frag.spv"));
							graphicsPs2Pipelines.emplace(key, createPs2GraphicsPipeline(hash, TOPOLOGY, "shaders/ps2/ps2.vs.spv", gsFilePath.c_str(), "shaders/ps2/ps2.ps.spv"));
						}
					}
				}
			}
		}
	}

	struct alignas(32) GSState
	{
		inline void SetST(float S, float T) {
			this->S = S;
			this->T = T;
		}

		inline void SetRGBAQ(uint32_t R, uint32_t G, uint32_t B, uint32_t A, float Q) {
			this->R = R;
			this->G = G;
			this->B = B;
			this->A = A;
			this->Q = Q;
		}

		uint8_t R;
		uint8_t G;
		uint8_t B;
		uint8_t A;
		float Q;

		float S;
		float T;

		inline void SetFrame(int fbp, int fbw, int psm, int fbmask) {
			this->PSM = psm;
			this->FBP = fbp;
			this->FBW = fbw;
			this->FBMASK = fbmask;
		}
		int PSM;
		int FBP;
		int FBW;
		int FBMASK;

		struct GSTest {
			uint32_t ATE;
			uint32_t ATST;
			uint32_t AREF;
			uint32_t AFAIL;
			uint32_t DATE;
			uint32_t DATM;
			uint32_t ZTE;
			uint32_t ZTST;

			bool operator==(const GSTest& other) const {
				return ATE == other.ATE
					&& ATST == other.ATST
					&& AREF == other.AREF
					&& AFAIL == other.AFAIL
					&& DATE == other.DATE
					&& DATM == other.DATM
					&& ZTE == other.ZTE
					&& ZTST == other.ZTST;
			}

			bool operator!=(const GSTest& other) const {
				return !(*this == other);
			}
		};

		GSTest TEST;

		struct GSPrim {
			uint32_t PRIM;
			uint32_t IIP;
			uint32_t TME;
			uint32_t FGE;
			uint32_t ABE;
			uint32_t AA1;
			uint32_t FST;
			uint32_t CTXT;
			uint32_t FIX;
		};

		GSPrim PRIM;

		struct GSTex {
			uint32_t TBP;
			uint32_t TBW;
			uint32_t PSM;
			uint32_t TW;
			uint32_t TH;
			uint32_t TCC;
			uint32_t TFX;
			uint32_t CBP;
			uint32_t CPSM;
			uint32_t CSM;
			uint32_t CSA;
			uint32_t CLD;
		};

		GSTex TEX;
	};

	GSState state;

	GSState& GetGSState() {
		return state;
	}

	struct GSTexKey {
		uint32_t TBP;
		uint32_t TBW;
		uint32_t PSM;
		uint32_t TW;
		uint32_t TH;

		static GSTexKey CreateFromTEX(const GSState::GSTex& TEX) {
			return { TEX.TBP, TEX.TBW, TEX.PSM, TEX.TW, TEX.TH };
		}

		bool operator==(const GSTexKey& other) const {
			return TBP == other.TBP &&
				TBW == other.TBW &&
				PSM == other.PSM &&
				TW == other.TW &&
				TH == other.TH;
		}
	};

	struct GSTexValueCreateInfo {
		GSTexValueCreateInfo(GSTexKey& inKey, LayoutVector& inDescriptorSetLayouts, LayoutBindingMap& inDescriptorSetLayoutBindings)
			: key(inKey)
			, descriptorSetLayouts(inDescriptorSetLayouts)
			, descriptorSetLayoutBindings(inDescriptorSetLayoutBindings)
		{}

		GSTexValueCreateInfo(const GSTexKey& inKey, const LayoutVector& inDescriptorSetLayouts, const LayoutBindingMap& inDescriptorSetLayoutBindings)
			: key(inKey)
			, descriptorSetLayouts(inDescriptorSetLayouts)
			, descriptorSetLayoutBindings(inDescriptorSetLayoutBindings)
		{}

		const GSTexKey& key;
		const LayoutVector& descriptorSetLayouts;
		const LayoutBindingMap& descriptorSetLayoutBindings;
	};

	struct GSTexValue {
		GSTexValue(const GSTexValueCreateInfo& createInfo)
			: width(1 << createInfo.key.TW)
			, height(1 << createInfo.key.TH)
		{
			CreateResources();
			UploadImage();
			CreateDescriptorPool(createInfo.descriptorSetLayoutBindings);
			CreateDescriptorSets(createInfo.descriptorSetLayouts);
		}

		void Cleanup()  {
			vkDestroySampler(GetDevice(), sampler, nullptr);

			vkDestroyBuffer(GetDevice(), stagingBuffer, nullptr);
			vkFreeMemory(GetDevice(), stagingBufferMemory, nullptr);

			vkDestroyImageView(GetDevice(), imageView, nullptr);
			vkDestroyImage(GetDevice(), image, nullptr);
			vkFreeMemory(GetDevice(), imageMemory, nullptr);
		}

		void UploadImage() {
			const VkDeviceSize bufferSize = width * height * 4;

			std::vector<char> pixelBuffer;

			if (pImage) {
				uint8_t* pData = (uint8_t*)pImage;
				for (int i = 0; i < width * height; i++) {
					pixelBuffer.push_back(pData[0]);
					pixelBuffer.push_back(pData[1]);
					pixelBuffer.push_back(pData[2]);
					pixelBuffer.push_back(pData[3]);
					pData = pData + 4;
				}
			}
			else {
				// Add alpha to the image.
				for (int i = 0; i < width * height; i++) {
					pixelBuffer.push_back(0xff);
					pixelBuffer.push_back(0xff);
					pixelBuffer.push_back(0xff);
					pixelBuffer.push_back(0xff);
				}
			}
		
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, pixelBuffer.data(), static_cast<size_t>(bufferSize));
			vkUnmapMemory(GetDevice(), stagingBufferMemory);

			TransitionImageLayout(image, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

			TransitionImageLayout(image, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		void CreateResources() {
			const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
			const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
			const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

			// Create the images that will be used as render targets
			VkImageCreateInfo colorImageCreateInfo = {};
			colorImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			colorImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			colorImageCreateInfo.extent.width = width;
			colorImageCreateInfo.extent.height = height;
			colorImageCreateInfo.extent.depth = 1;
			colorImageCreateInfo.mipLevels = 1;
			colorImageCreateInfo.arrayLayers = 1;
			colorImageCreateInfo.format = format;
			colorImageCreateInfo.tiling = tiling;
			colorImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorImageCreateInfo.usage = usage;
			colorImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			colorImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkResult result = vkCreateImage(GetDevice(), &colorImageCreateInfo, nullptr, &image);
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create Vulkan image.");
			}

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(GetDevice(), image, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			if (vkAllocateMemory(GetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate image memory!");
			}

			vkBindImageMemory(GetDevice(), image, imageMemory, 0);

			// Create an image view for the color image
			VkImageViewCreateInfo colorImageViewCreateInfo = {};

			colorImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorImageViewCreateInfo.image = image;
			colorImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorImageViewCreateInfo.format = format;
			colorImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			colorImageViewCreateInfo.subresourceRange.levelCount = 1;
			colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			colorImageViewCreateInfo.subresourceRange.layerCount = 1;

			vkCreateImageView(GetDevice(), &colorImageViewCreateInfo, nullptr, &imageView);

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

			if (vkCreateSampler(GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture sampler!");
			}
		}

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkSampler sampler;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		int width, height;

	private:
		void CreateDescriptorSets(const LayoutVector& descriptorSetLayouts) {
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

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = imageView;
				imageInfo.sampler = sampler;

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = descriptorSets[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pImageInfo = &imageInfo;

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = descriptorSets[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;

				VkDescriptorBufferInfo vertexConstantBuffer{};
				vertexConstantBuffer.buffer = vextexUniformBuffers[i];
				vertexConstantBuffer.offset = 0;
				vertexConstantBuffer.range = sizeof(VSConstantBuffer);

				descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[2].dstSet = descriptorSets[i];
				descriptorWrites[2].dstBinding = 5;
				descriptorWrites[2].dstArrayElement = 0;
				descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[2].descriptorCount = 1;
				descriptorWrites[2].pBufferInfo = &vertexConstantBuffer;

				VkDescriptorBufferInfo pixelConstantBuffer{};
				pixelConstantBuffer.buffer = pixelUniformBuffers[i];
				pixelConstantBuffer.offset = 0;
				pixelConstantBuffer.range = sizeof(PSConstantBuffer);

				descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[3].dstSet = descriptorSets[i];
				descriptorWrites[3].dstBinding = 6;
				descriptorWrites[3].dstArrayElement = 0;
				descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[3].descriptorCount = 1;
				descriptorWrites[3].pBufferInfo = &pixelConstantBuffer;

				vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}

		void CreateDescriptorPool(const LayoutBindingMap& descriptorSetLayoutBindingsMap) {
			// Create descriptor pool based on the descriptor set count from the shader

			auto& descriptorSetLayoutBindings = descriptorSetLayoutBindingsMap.at(0);
			std::vector<VkDescriptorPoolSize> poolSizes;
			for (uint32_t i = 0; i < descriptorSetLayoutBindings.size(); ++i) {
				auto& descriptorSet = descriptorSetLayoutBindings[i];
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

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
			VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else {
				throw std::invalid_argument("unsupported layout transition!");
			}

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
			VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height,
				1
			};

			vkCmdCopyBufferToImage(
				commandBuffer,
				buffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);

		}
	};

	struct GSTexEntry {
		GSTexKey key;
		GSTexValue value;

		GSTexEntry(const GSTexValueCreateInfo& createInfo)
		: key(createInfo.key)
		, value(GSTexValue(createInfo)) {

		}

		bool operator==(const GSTexKey& other) const {
			return key == other;
		}
	};

	Renderer::GSVertex MakeVertex(uint16_t x, uint16_t y, uint32_t z) {
		return Renderer::GSVertex{ { GetGSState().S, GetGSState().T }, {GetGSState().R, GetGSState().G, GetGSState().B, GetGSState().A }, GetGSState().Q, { x, y }, z, 0, 0 };
	}
}

namespace Renderer {
#define ASSERT(...)
#define RESTRICT

	struct
	{
		Renderer::GSVertex* buff;
		size_t head, tail, next, maxcount; // head: first vertex, tail: last vertex + 1, next: last indexed + 1
		size_t xy_tail;
		uint64_t xy[4];
	} m_vertex;

	struct
	{
		uint16_t* buff;
		size_t tail;
	} m_index;

	enum GS_PRIM
	{
		GS_POINTLIST = 0,
		GS_LINELIST = 1,
		GS_LINESTRIP = 2,
		GS_TRIANGLELIST = 3,
		GS_TRIANGLESTRIP = 4,
		GS_TRIANGLEFAN = 5,
		GS_SPRITE = 6,
		GS_INVALID = 7,
	};

	enum GS_PRIM_CLASS
	{
		GS_POINT_CLASS = 0,
		GS_LINE_CLASS = 1,
		GS_TRIANGLE_CLASS = 2,
		GS_SPRITE_CLASS = 3,
		GS_INVALID_CLASS = 7,
	};

	uint32_t OFX = 0;
	uint32_t OFY = 0;

	uint32_t Skip = 0;

	void SetVertexSkip(uint32_t inSkip)
	{
		Skip = inSkip;
	}

	void SetXY(uint32_t x, uint32_t y)
	{
		OFX = x;
		OFY = y;
	}

	void SetST(float s, float t)
	{
		PS2_Internal::GetGSState().SetST(s, t);
	}

	void SetRGBAQ(uint32_t R, uint32_t G, uint32_t B, uint32_t A, float Q) {
		PS2_Internal::GetGSState().SetRGBAQ(R, G, B, A, Q);
	}

	void SetFrame(int fbp, int fbw, int psm, int fbmask) {
		PS2_Internal::GetGSState().SetFrame(fbp, fbw, psm, fbmask);
	}

	void SetTest(uint32_t ate, uint32_t atst, uint32_t aref, uint32_t afail, uint32_t date, uint32_t datm, uint32_t zte, uint32_t ztst) {
		PS2_Internal::GSState::GSTest NewTest = { ate, atst, aref, afail, date, datm, zte, ztst };

		if (PS2_Internal::GetGSState().TEST != NewTest) {
			Draw();
		}

		PS2_Internal::GetGSState().TEST = NewTest;
	}

	void SetPrim(uint32_t prim, uint32_t iip, uint32_t tme, uint32_t fge, uint32_t abe, uint32_t aa1, uint32_t fst, uint32_t ctxt, uint32_t fix) {
		PS2_Internal::GetGSState().PRIM = { prim, iip, tme, fge, abe, aa1, fst, ctxt, fix };
	}

	void SetTEX(uint32_t tbp, uint32_t tbw, uint32_t psm, uint32_t tw, uint32_t th, uint32_t tcc, uint32_t tfx, uint32_t cbp, uint32_t cpsm, uint32_t csm, uint32_t csa, uint32_t cld)
	{
		PS2_Internal::GetGSState().TEX = { tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld };
	}

	void KickVertex(uint16_t x, uint16_t y, uint32_t z)
	{
		uint32_t skip = Skip;
		GS_PRIM prim = (GS_PRIM)PS2_Internal::GetGSState().PRIM.PRIM;
		ASSERT(m_vertex.tail < m_vertex.maxcount + 3);

		size_t head = m_vertex.head;
		size_t tail = m_vertex.tail;
		size_t next = m_vertex.next;
		//size_t xy_tail = m_vertex.xy_tail;

		// callers should write XYZUVF to m_v.m[1] in one piece to have this load store-forwarded, either by the CPU or the compiler when this function is in lined
		m_vertex.buff[m_vertex.tail] = PS2_Internal::MakeVertex(x, y, z);

		m_vertex.tail = ++tail;
		//m_vertex.xy_tail = ++xy_tail;

		size_t n = 0;

		switch (prim)
		{
		case GS_POINTLIST: n = 1; break;
		case GS_LINELIST: n = 2; break;
		case GS_LINESTRIP: n = 2; break;
		case GS_TRIANGLELIST: n = 3; break;
		case GS_TRIANGLESTRIP: n = 3; break;
		case GS_TRIANGLEFAN: n = 3; break;
		case GS_SPRITE: n = 2; break;
		case GS_INVALID: n = 1; break;
		}

		size_t m = tail - head;

		if (m < n)
		{
			return;
		}

		if (skip == 0 && (prim != GS_TRIANGLEFAN || m <= 4)) // m_vertex.xy only knows about the last 4 vertices, head could be far behind for fan
		{
			//GSVector4i v0, v1, v2, v3, pmin, pmax;
			//
			//v0 = GSVector4i::loadl(&m_vertex.xy[(xy_tail + 1) & 3]); // T-3
			//v1 = GSVector4i::loadl(&m_vertex.xy[(xy_tail + 2) & 3]); // T-2
			//v2 = GSVector4i::loadl(&m_vertex.xy[(xy_tail + 3) & 3]); // T-1
			//v3 = GSVector4i::loadl(&m_vertex.xy[(xy_tail - m) & 3]); // H
			//
			//GSVector4 cross;
			//
			//switch (prim)
			//{
			//case GS_POINTLIST:
			//	pmin = v2;
			//	pmax = v2;
			//	break;
			//case GS_LINELIST:
			//case GS_LINESTRIP:
			//case GS_SPRITE:
			//	pmin = v2.min_i16(v1);
			//	pmax = v2.max_i16(v1);
			//	break;
			//case GS_TRIANGLELIST:
			//case GS_TRIANGLESTRIP:
			//	pmin = v2.min_i16(v1.min_i16(v0));
			//	pmax = v2.max_i16(v1.max_i16(v0));
			//	break;
			//case GS_TRIANGLEFAN:
			//	pmin = v2.min_i16(v1.min_i16(v3));
			//	pmax = v2.max_i16(v1.max_i16(v3));
			//	break;
			//default:
			//	break;
			//}
			//
			//GSVector4i test = pmax.lt16(m_scissor) | pmin.gt16(m_scissor.zwzwl());
			//
			//switch (prim)
			//{
			//case GS_TRIANGLELIST:
			//case GS_TRIANGLESTRIP:
			//case GS_TRIANGLEFAN:
			//case GS_SPRITE:
			//	// FIXME: GREG I don't understand the purpose of the m_nativeres check
			//	// It impacts badly the number of draw call in the HW renderer.
			//	test |= m_nativeres ? pmin.eq16(pmax).zwzwl() : pmin.eq16(pmax);
			//	break;
			//default:
			//	break;
			//}
			//
			//switch (prim)
			//{
			//case GS_TRIANGLELIST:
			//case GS_TRIANGLESTRIP:
			//	// TODO: any way to do a 16-bit integer cross product?
			//	// cross product is zero most of the time because either of the vertices are the same
			//	/*
			//	cross = GSVector4(v2.xyxyl().i16to32().sub32(v0.upl32(v1).i16to32())); // x20, y20, x21, y21
			//	cross = cross * cross.wzwz(); // x20 * y21, y20 * x21
			//	test |= GSVector4i::cast(cross == cross.yxwz());
			//	*/
			//	test = (test | v0 == v1) | (v1 == v2 | v0 == v2);
			//	break;
			//case GS_TRIANGLEFAN:
			//	/*
			//	cross = GSVector4(v2.xyxyl().i16to32().sub32(v3.upl32(v1).i16to32())); // x23, y23, x21, y21
			//	cross = cross * cross.wzwz(); // x23 * y21, y23 * x21
			//	test |= GSVector4i::cast(cross == cross.yxwz());
			//	*/
			//	test = (test | v3 == v1) | (v1 == v2 | v3 == v2);
			//	break;
			//default:
			//	break;
			//}
			//
			//skip |= test.mask() & 15;
		}

		if (skip != 0)
		{
			switch (prim)
			{
			case GS_POINTLIST:
			case GS_LINELIST:
			case GS_TRIANGLELIST:
			case GS_SPRITE:
			case GS_INVALID:
				m_vertex.tail = head; // no need to check or grow the buffer length
				break;
			case GS_LINESTRIP:
			case GS_TRIANGLESTRIP:
				m_vertex.head = head + 1;
				// fall through
			case GS_TRIANGLEFAN:
				//if (tail >= m_vertex.maxcount) GrowVertexBuffer(); // in case too many vertices were skipped
				break;
			default:
				__assume(0);
			}
		
			return;
		}

		//if (tail >= m_vertex.maxcount) GrowVertexBuffer();

		uint16_t* RESTRICT buff = &m_index.buff[m_index.tail];

		switch (prim)
		{
		case GS_POINTLIST:
			buff[0] = head + 0;
			m_vertex.head = head + 1;
			m_vertex.next = head + 1;
			m_index.tail += 1;
			break;
		case GS_LINELIST:
			buff[0] = head + 0;
			buff[1] = head + 1;
			m_vertex.head = head + 2;
			m_vertex.next = head + 2;
			m_index.tail += 2;
			break;
		case GS_LINESTRIP:
			if (next < head)
			{
				m_vertex.buff[next + 0] = m_vertex.buff[head + 0];
				m_vertex.buff[next + 1] = m_vertex.buff[head + 1];
				head = next;
				m_vertex.tail = next + 2;
			}
			buff[0] = head + 0;
			buff[1] = head + 1;
			m_vertex.head = head + 1;
			m_vertex.next = head + 2;
			m_index.tail += 2;
			break;
		case GS_TRIANGLELIST:
			buff[0] = head + 0;
			buff[1] = head + 1;
			buff[2] = head + 2;
			m_vertex.head = head + 3;
			m_vertex.next = head + 3;
			m_index.tail += 3;
			break;
		case GS_TRIANGLESTRIP:
			if (next < head)
			{
				m_vertex.buff[next + 0] = m_vertex.buff[head + 0];
				m_vertex.buff[next + 1] = m_vertex.buff[head + 1];
				m_vertex.buff[next + 2] = m_vertex.buff[head + 2];
				head = next;
				m_vertex.tail = next + 3;
			}
			buff[0] = head + 0;
			buff[1] = head + 1;
			buff[2] = head + 2;
			m_vertex.head = head + 1;
			m_vertex.next = head + 3;
			m_index.tail += 3;
			break;
		case GS_TRIANGLEFAN:
			// TODO: remove gaps, next == head && head < tail - 3 || next > head && next < tail - 2 (very rare)
			buff[0] = head + 0;
			buff[1] = tail - 2;
			buff[2] = tail - 1;
			m_vertex.next = tail;
			m_index.tail += 3;
			break;
		case GS_SPRITE:
			buff[0] = head + 0;
			buff[1] = head + 1;
			m_vertex.head = head + 2;
			m_vertex.next = head + 2;
			m_index.tail += 2;
			break;
		case GS_INVALID:
			m_vertex.tail = head;
			break;
		default:
			__assume(0);
		}
	}
}

struct Vector2i { int x; int y; };
struct Vector2f { float x; float y; };

struct FrameBuffer {
	VkRenderPass renderPass;
	VkImage colorImage;
	VkImageView colorImageView;
	VkImage depthImage;
	VkImageView depthImageView;
	VkFramebuffer framebuffer;
	VkDeviceMemory imageMemory;
	VkDeviceMemory depthImageMemory;
	int FBP;

	static FrameBuffer Create(Vector2i size, int FBP) {
		FrameBuffer newBuffer = {};

		newBuffer.FBP = FBP;

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

		VkSubpassDependency selfDependency{};
		selfDependency.srcSubpass = 0;
		selfDependency.dstSubpass = 0;
		selfDependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		selfDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		selfDependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		selfDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		selfDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

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

		if (vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &newBuffer.renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		{
			const VkFormat format = GetSwapchainImageFormat();
			const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
			const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			// Create the images that will be used as render targets
			VkImageCreateInfo colorImageCreateInfo = {};
			colorImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			colorImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			colorImageCreateInfo.extent.width = size.x;
			colorImageCreateInfo.extent.height = size.y;
			colorImageCreateInfo.extent.depth = 1;
			colorImageCreateInfo.mipLevels = 1;
			colorImageCreateInfo.arrayLayers = 1;
			colorImageCreateInfo.format = format;
			colorImageCreateInfo.tiling = tiling;
			colorImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorImageCreateInfo.usage = usage;
			colorImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			colorImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			vkCreateImage(GetDevice(), &colorImageCreateInfo, nullptr, &newBuffer.colorImage);

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(GetDevice(), newBuffer.colorImage, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			if (vkAllocateMemory(GetDevice(), &allocInfo, nullptr, &newBuffer.imageMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate image memory!");
			}

			vkBindImageMemory(GetDevice(), newBuffer.colorImage, newBuffer.imageMemory, 0);

			// Create an image view for the color image
			VkImageViewCreateInfo colorImageViewCreateInfo = {};

			colorImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorImageViewCreateInfo.image = newBuffer.colorImage;
			colorImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorImageViewCreateInfo.format = format;
			colorImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			colorImageViewCreateInfo.subresourceRange.levelCount = 1;
			colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			colorImageViewCreateInfo.subresourceRange.layerCount = 1;

			vkCreateImageView(GetDevice(), &colorImageViewCreateInfo, nullptr, &newBuffer.colorImageView);
		}

		{
			VkFormat format = VK_FORMAT_D32_SFLOAT;
			VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
			VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			// Create the images that will be used as render targets
			VkImageCreateInfo depthImageCreateInfo = {};
			depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			depthImageCreateInfo.extent.width = size.x;
			depthImageCreateInfo.extent.height = size.y;
			depthImageCreateInfo.extent.depth = 1;
			depthImageCreateInfo.mipLevels = 1;
			depthImageCreateInfo.arrayLayers = 1;
			depthImageCreateInfo.format = format;
			depthImageCreateInfo.tiling = tiling;
			depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthImageCreateInfo.usage = usage;
			depthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			vkCreateImage(GetDevice(), &depthImageCreateInfo, nullptr, &newBuffer.depthImage);

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(GetDevice(), newBuffer.depthImage, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			if (vkAllocateMemory(GetDevice(), &allocInfo, nullptr, &newBuffer.depthImageMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate image memory!");
			}

			vkBindImageMemory(GetDevice(), newBuffer.depthImage, newBuffer.depthImageMemory, 0);

			// Create an image view for the color image
			VkImageViewCreateInfo depthImageViewCreateInfo = {};

			depthImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			depthImageViewCreateInfo.image = newBuffer.depthImage;
			depthImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			depthImageViewCreateInfo.format = format;
			depthImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			depthImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			depthImageViewCreateInfo.subresourceRange.levelCount = 1;
			depthImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			depthImageViewCreateInfo.subresourceRange.layerCount = 1;

			vkCreateImageView(GetDevice(), &depthImageViewCreateInfo, nullptr, &newBuffer.depthImageView);
		}

		VkImageView imageAttachments[] = { newBuffer.colorImageView , newBuffer.depthImageView };

		// Create the framebuffer that will use the color image as a render target
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = newBuffer.renderPass;
		framebufferCreateInfo.attachmentCount = 2;
		framebufferCreateInfo.pAttachments = imageAttachments;
		framebufferCreateInfo.width = size.x;
		framebufferCreateInfo.height = size.y;
		framebufferCreateInfo.layers = 1;
		vkCreateFramebuffer(GetDevice(), &framebufferCreateInfo, nullptr, &newBuffer.framebuffer);

		return newBuffer;
	}
};

std::unordered_map<int, FrameBuffer> passes;


HardwareState hwState;

HardwareState& GetHardwareState() {
	return hwState;
}

struct GSSelector
{
	uint32_t iip;
	uint32_t prim;
	uint32_t point;
	uint32_t line;

	GSSelector()
		: iip(0)
		, prim(0)
		, point(0)
		, line(0)
	{}

	void ResetStates() {
		*this = GSSelector();
	}
};

static class GSUtilMaps
{
public:
	uint8_t PrimClassField[8];
	uint8_t VertexCountField[8];
	uint8_t ClassVertexCountField[4];
	uint32_t CompatibleBitsField[64][2];
	uint32_t SharedBitsField[64][2];

	// Defer init to avoid AVX2 illegal instructions
	GSUtilMaps()
	{
		PrimClassField[Renderer::GS_POINTLIST] = Renderer::GS_POINT_CLASS;
		PrimClassField[Renderer::GS_LINELIST] = Renderer::GS_LINE_CLASS;
		PrimClassField[Renderer::GS_LINESTRIP] = Renderer::GS_LINE_CLASS;
		PrimClassField[Renderer::GS_TRIANGLELIST] = Renderer::GS_TRIANGLE_CLASS;
		PrimClassField[Renderer::GS_TRIANGLESTRIP] = Renderer::GS_TRIANGLE_CLASS;
		PrimClassField[Renderer::GS_TRIANGLEFAN] = Renderer::GS_TRIANGLE_CLASS;
		PrimClassField[Renderer::GS_SPRITE] = Renderer::GS_SPRITE_CLASS;
		PrimClassField[Renderer::GS_INVALID] = Renderer::GS_INVALID_CLASS;

		VertexCountField[Renderer::GS_POINTLIST] = 1;
		VertexCountField[Renderer::GS_LINELIST] = 2;
		VertexCountField[Renderer::GS_LINESTRIP] = 2;
		VertexCountField[Renderer::GS_TRIANGLELIST] = 3;
		VertexCountField[Renderer::GS_TRIANGLESTRIP] = 3;
		VertexCountField[Renderer::GS_TRIANGLEFAN] = 3;
		VertexCountField[Renderer::GS_SPRITE] = 2;
		VertexCountField[Renderer::GS_INVALID] = 1;

		ClassVertexCountField[Renderer::GS_POINT_CLASS] = 1;
		ClassVertexCountField[Renderer::GS_LINE_CLASS] = 2;
		ClassVertexCountField[Renderer::GS_TRIANGLE_CLASS] = 3;
		ClassVertexCountField[Renderer::GS_SPRITE_CLASS] = 2;

		//memset(CompatibleBitsField, 0, sizeof(CompatibleBitsField));
		//
		//for (int i = 0; i < 64; i++)
		//{
		//	CompatibleBitsField[i][i >> 5] |= 1 << (i & 0x1f);
		//}
		//
		//CompatibleBitsField[PSM_PSMCT32][PSM_PSMCT24 >> 5] |= 1 << (PSM_PSMCT24 & 0x1f);
		//CompatibleBitsField[PSM_PSMCT24][PSM_PSMCT32 >> 5] |= 1 << (PSM_PSMCT32 & 0x1f);
		//CompatibleBitsField[PSM_PSMCT16][PSM_PSMCT16S >> 5] |= 1 << (PSM_PSMCT16S & 0x1f);
		//CompatibleBitsField[PSM_PSMCT16S][PSM_PSMCT16 >> 5] |= 1 << (PSM_PSMCT16 & 0x1f);
		//CompatibleBitsField[PSM_PSMZ32][PSM_PSMZ24 >> 5] |= 1 << (PSM_PSMZ24 & 0x1f);
		//CompatibleBitsField[PSM_PSMZ24][PSM_PSMZ32 >> 5] |= 1 << (PSM_PSMZ32 & 0x1f);
		//CompatibleBitsField[PSM_PSMZ16][PSM_PSMZ16S >> 5] |= 1 << (PSM_PSMZ16S & 0x1f);
		//CompatibleBitsField[PSM_PSMZ16S][PSM_PSMZ16 >> 5] |= 1 << (PSM_PSMZ16 & 0x1f);
		//
		//memset(SharedBitsField, 0, sizeof(SharedBitsField));
		//
		//SharedBitsField[PSM_PSMCT24][PSM_PSMT8H >> 5] |= 1 << (PSM_PSMT8H & 0x1f);
		//SharedBitsField[PSM_PSMCT24][PSM_PSMT4HL >> 5] |= 1 << (PSM_PSMT4HL & 0x1f);
		//SharedBitsField[PSM_PSMCT24][PSM_PSMT4HH >> 5] |= 1 << (PSM_PSMT4HH & 0x1f);
		//SharedBitsField[PSM_PSMZ24][PSM_PSMT8H >> 5] |= 1 << (PSM_PSMT8H & 0x1f);
		//SharedBitsField[PSM_PSMZ24][PSM_PSMT4HL >> 5] |= 1 << (PSM_PSMT4HL & 0x1f);
		//SharedBitsField[PSM_PSMZ24][PSM_PSMT4HH >> 5] |= 1 << (PSM_PSMT4HH & 0x1f);
		//SharedBitsField[PSM_PSMT8H][PSM_PSMCT24 >> 5] |= 1 << (PSM_PSMCT24 & 0x1f);
		//SharedBitsField[PSM_PSMT8H][PSM_PSMZ24 >> 5] |= 1 << (PSM_PSMZ24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HL][PSM_PSMCT24 >> 5] |= 1 << (PSM_PSMCT24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HL][PSM_PSMZ24 >> 5] |= 1 << (PSM_PSMZ24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HL][PSM_PSMT4HH >> 5] |= 1 << (PSM_PSMT4HH & 0x1f);
		//SharedBitsField[PSM_PSMT4HH][PSM_PSMCT24 >> 5] |= 1 << (PSM_PSMCT24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HH][PSM_PSMZ24 >> 5] |= 1 << (PSM_PSMZ24 & 0x1f);
		//SharedBitsField[PSM_PSMT4HH][PSM_PSMT4HL >> 5] |= 1 << (PSM_PSMT4HL & 0x1f);
	}

} s_maps;

Renderer::GS_PRIM_CLASS GetPrimClass(uint32_t prim) {
	return (Renderer::GS_PRIM_CLASS)s_maps.PrimClassField[prim];
}

GSSelector g_GSSelector;
inline PS2_Internal::GSTexKey CreateKeyFromTEX(const PS2_Internal::GSState::GSTex& TEX) {
	return { TEX.TBP, TEX.TBW, TEX.PSM, TEX.TW, TEX.TH };
}

class TextureCache
{
	std::vector<PS2_Internal::GSTexEntry> texcache;

public:
	PS2_Internal::GSTexEntry& Create(const PS2_Internal::GSState::GSTex& TEX, const LayoutVector& descriptorSetLayouts, const LayoutBindingMap& descriptorSetLayoutBindings) {
		const PS2_Internal::GSTexKey key = PS2_Internal::GSTexKey::CreateFromTEX(TEX);
		const PS2_Internal::GSTexValueCreateInfo createInfo = PS2_Internal::GSTexValueCreateInfo(key, descriptorSetLayouts, descriptorSetLayoutBindings);
		texcache.emplace_back(PS2_Internal::GSTexEntry(createInfo));
		return texcache.back();
	}

	PS2_Internal::GSTexEntry& Lookup(const PS2_Internal::GSState::GSTex& TEX, const LayoutVector& descriptorSetLayouts, const LayoutBindingMap& descriptorSetLayoutBindings) {
		const PS2_Internal::GSTexKey key = PS2_Internal::GSTexKey::CreateFromTEX(TEX);
		for (auto& entry : texcache) {
			if (entry == key) {
				return entry;
			}
		}

		return Create(TEX, descriptorSetLayouts, descriptorSetLayoutBindings);
	}
};

TextureCache m_TextureCache;

void Renderer::SetImagePointer(void* pInImage)
{
	pImage = pInImage;
}

void Renderer::Draw() {
	if (Renderer::m_index.tail == 0) {
		return;
	}

	g_GSSelector.ResetStates();

	const PS2_Internal::GSState& state = PS2_Internal::GetGSState();

	const bool unscale_pt_ln = false;

	VkPrimitiveTopology topology;

	switch (GetPrimClass(state.PRIM.PRIM))
	{
	case GS_POINT_CLASS:
		if (unscale_pt_ln)
		{
			g_GSSelector.point = 1;
			//gs_cb.PointSize = GSVector2(16.0f * sx, 16.0f * sy);
		}

		topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case GS_LINE_CLASS:
		if (unscale_pt_ln)
		{
			g_GSSelector.line = 1;
			//gs_cb.PointSize = GSVector2(16.0f * sx, 16.0f * sy);
		}

		topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		break;
	case GS_SPRITE_CLASS:
		topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	case GS_TRIANGLE_CLASS:

		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		break;
	default:
		__assume(0);
	}

	g_GSSelector.iip = state.PRIM.IIP;
	g_GSSelector.prim = GetPrimClass(state.PRIM.PRIM);

	// Custom res settings.
	Vector2i rtsize = { 0x500, 0x400 };
	Vector2f rtscale = { 1.0f, 1.0f };

	if (passes.find(state.FBP) == passes.end())	{
		passes.emplace(state.FBP, FrameBuffer::Create(rtsize, state.FBP));
	}

	const std::string CONFIG_NAME = "GS_PRIM_" + std::to_string(g_GSSelector.prim)
		+ "_POINT_" + std::to_string(g_GSSelector.point)
		+ "_LINE_" + std::to_string(g_GSSelector.line)
		+ "_IIP_" + std::to_string(g_GSSelector.iip);

	const std::string hash = GetMD5String(CONFIG_NAME);

	const PS2_Internal::PipelineKey key = { hash, topology };
	auto pipelineItr = PS2_Internal::graphicsPs2Pipelines.find(key);
	assert(pipelineItr != PS2_Internal::graphicsPs2Pipelines.end());
	auto pipeline = (*pipelineItr).second;

	PS2_Internal::GSTexEntry& tex = m_TextureCache.Lookup(state.TEX, pipeline.descriptorSetLayouts, pipeline.descriptorSetLayoutBindings);

	if (!hwState.bActivePass || hwState.FBP != state.FBP) {

		if (hwState.bActivePass) {
			vkCmdEndRenderPass(GetCurrentCommandBuffer());
		}

		hwState.FBP = state.FBP;
		hwState.bActivePass = true;

		auto& frameBuffer = (*passes.find(state.FBP)).second;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = frameBuffer.renderPass;
		renderPassInfo.framebuffer = frameBuffer.framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { (unsigned int)rtsize.x, (unsigned int)rtsize.y };

		VkClearValue clearColors[] = {{{0.0f, 0.0f, 0.0f, 1.0f}}, {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearColors;

		vkCmdBeginRenderPass(GetCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		float m_hack_topleft_offset = (false) ? -0.01f : 0.0f;
		VkViewport viewport{};
		viewport.x = m_hack_topleft_offset;
		viewport.y = m_hack_topleft_offset;
		viewport.width = (float)rtsize.x;
		viewport.height = (float)rtsize.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(GetCurrentCommandBuffer(), 0, 1, &viewport);

		float sx = 2.0f * rtscale.x / (rtsize.x << 4);
		float sy = 2.0f * rtscale.y / (rtsize.y << 4);
		float ox = Renderer::OFX;
		float oy = Renderer::OFY;
		float ox2 = -1.0f / rtsize.x;
		float oy2 = -1.0f / rtsize.y;

		PS2_Internal::vertexConstBuffer.Texture_Scale_Offset = GSVector4(0.0f);
		PS2_Internal::vertexConstBuffer.VertexScale = GSVector4(sx, -sy, ldexpf(1, -32), 0.0f);
		PS2_Internal::vertexConstBuffer.VertexOffset = GSVector4(ox * sx + ox2 + 1, -(oy * sy + oy2 + 1), 0.0f, -1.0f);

		{
			void* data;
			vkMapMemory(GetDevice(), PS2_Internal::vertexUniformBuffersMemory[GetCurrentFrame()], 0, sizeof(PS2_Internal::VSConstantBuffer), 0, &data);
			memcpy(data, &PS2_Internal::vertexConstBuffer, sizeof(PS2_Internal::VSConstantBuffer));
			vkUnmapMemory(GetDevice(), PS2_Internal::vertexUniformBuffersMemory[GetCurrentFrame()]);
		}

		{
			void* data;
			vkMapMemory(GetDevice(), PS2_Internal::pixelUniformBuffersMemory[GetCurrentFrame()], 0, sizeof(PS2_Internal::PSConstantBuffer), 0, &data);
			memcpy(data, &PS2_Internal::pixelConstBuffer, sizeof(PS2_Internal::PSConstantBuffer));
			vkUnmapMemory(GetDevice(), PS2_Internal::pixelUniformBuffersMemory[GetCurrentFrame()]);
		}
	}

	vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

	const VkDeviceSize vertexOffset = PS2_Internal::MapVertices(Renderer::m_vertex.buff, Renderer::m_vertex.tail);
	const VkDeviceSize indexOffset = PS2_Internal::MapIndices(Renderer::m_index.buff, Renderer::m_index.tail);

	VkBuffer vertexBuffers[] = { PS2_Internal::GetVertexBuffer() };
	VkDeviceSize offsets[] = { vertexOffset };
	vkCmdBindVertexBuffers(GetCurrentCommandBuffer(), 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(GetCurrentCommandBuffer(), PS2_Internal::GetIndexBuffer(), indexOffset, VK_INDEX_TYPE_UINT16);

	vkCmdBindDescriptorSets(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &tex.value.descriptorSets[GetCurrentFrame()], 0, nullptr);

	vkCmdDrawIndexed(GetCurrentCommandBuffer(), static_cast<uint32_t>(Renderer::m_index.tail), 1, 0, 0, 0);

	Renderer::m_index.tail = 0;
	Renderer::m_vertex.head = 0;
	Renderer::m_vertex.tail = 0;
	Renderer::m_vertex.next = 0;
}

void PS2::Setup()
{
	PS2_Internal::createPs2RenderPass();
	PS2_Internal::createPs2GraphicsPipelines();
	PS2_Internal::createDescriptorPool();
	PS2_Internal::createUniformBuffers();
	PS2_Internal::createDescriptorSets();

	PS2_Internal::vertexBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	Renderer::m_vertex.buff = (Renderer::GSVertex*)_aligned_malloc(sizeof(Renderer::GSVertex) * 0x1000, 32);
	Renderer::m_index.buff = (uint16_t*)_aligned_malloc(sizeof(uint16_t) * 0x1000, 32);
}

void PS2::BeginFrame()
{
	PS2_Internal::ResetBuffer();
}

void PS2::Cleanup()
{
	for (auto& pipeline : PS2_Internal::graphicsPs2Pipelines) {
		vkDestroyPipeline(GetDevice(), pipeline.second.pipeline, nullptr);
		vkDestroyPipelineLayout(GetDevice(), pipeline.second.layout, nullptr);
	}
	
	vkDestroyRenderPass(GetDevice(), PS2_Internal::renderPassPs2, nullptr);
}