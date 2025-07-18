#include "NativeDisplayList.h"

#include <stdexcept>

#include "displaylist.h"
#include "VulkanRenderer.h"
#include "NativeRenderer.h"
#include "renderer.h"
#include "UniformBuffer.h"
#include "TextureCache.h"
#include "Blending.h"

namespace Renderer::Native::DisplayList
{
	static VkCommandPool gCommandPool;
	static CommandBufferVector gCommandBuffers;

	static bool gRecordingCommandBuffer = false;

	static SimpleTexture* gBoundTexture = nullptr;

	static VkRenderPass gRenderPass;
	static PipelineMap gPipelines;

	struct DisplayListPipelineKey
	{
		union {
			// 32 bit key
			struct {
				uint32_t topology : 2;
				uint32_t bBoundTexture : 1;
			} options;

			uint32_t key{};
		};
	};

	static Pipeline& GetPipeline() {
		DisplayListPipelineKey pipelineKey;
		pipelineKey.options.bBoundTexture = gBoundTexture != nullptr;
		pipelineKey.options.topology = (PS2::GetGSState().PRIM.PRIM == 6) ? topologyLineList : topologyTriangleList;

		assert(gPipelines.find(pipelineKey.key) != gPipelines.end());
		return gPipelines[pipelineKey.key];
	}

	static PS2::FrameVertexBuffers<Renderer::DisplayListVertex, uint16_t> gVertexBuffers;

	int gIndexStart = 0;
	int gVertexStart = 0;

	struct {
		float width;
		float height;
	} gViewport;

	VkCommandBuffer& GetCommandBuffer()
	{
		return gCommandBuffers[GetCurrentFrame()];
	}

	static void BeginCommandBufferRecording()
	{
		assert(!gRecordingCommandBuffer);

		VkCommandBuffer& cmd = GetCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd, &beginInfo);

		Renderer::Debug::BeginLabel(GetCommandBuffer(), "Display List Render");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = gRenderPass;
		renderPassInfo.framebuffer = GetFrameBuffer().framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { gWidth, gHeight };

		std::array<VkClearValue, 0> clearColors;
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

		gVertexBuffers.BindBuffers(cmd);

		gRecordingCommandBuffer = true;
	}

	static struct
	{
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;
		uint8_t a = 0;
	} gColor;

	static struct
	{
		float s = 0.0f;
		float t = 0.0f;
	} gTexCoord;

	static float gQ;

	static void CreateRenderPass(VkRenderPass& renderPass, const char* name)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = GetSwapchainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

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

		SetObjectName(reinterpret_cast<uint64_t>(renderPass), VK_OBJECT_TYPE_RENDER_PASS, name);
	}

	static void CreatePipeline(const PipelineCreateInfo<DisplayListPipelineKey>& createInfo, const VkRenderPass& renderPass, const char* name)
	{
		Renderer::Pipeline& pipeline = gPipelines[createInfo.key.key];

		const bool bHasGeometryShader = !createInfo.geomShaderFilename.empty();

		auto vertShader = Shader::ReflectedModule(createInfo.vertShaderFilename, VK_SHADER_STAGE_VERTEX_BIT);
		auto fragShader = Shader::ReflectedModule(createInfo.fragShaderFilename, VK_SHADER_STAGE_FRAGMENT_BIT);
		auto geomShader = Shader::ReflectedModule(createInfo.geomShaderFilename, VK_SHADER_STAGE_GEOMETRY_BIT);

		pipeline.AddBindings(EBindingStage::Vertex, vertShader.reflectData);
		pipeline.AddBindings(EBindingStage::Fragment, fragShader.reflectData);
		pipeline.AddBindings(EBindingStage::Geometry, geomShader.reflectData);
		pipeline.CreateDescriptorSetLayouts();

		pipeline.CreateLayout();

		pipeline.CreateDescriptorPool();
		pipeline.CreateDescriptorSets();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertShader.shaderStageCreateInfo, fragShader.shaderStageCreateInfo };

		if (bHasGeometryShader) {
			shaderStages.push_back(geomShader.shaderStageCreateInfo);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto& bindingDescription = vertShader.reflectData.bindingDescription;
		const auto& attributeDescriptions = vertShader.reflectData.GetAttributes();

		bindingDescription.stride = sizeof(DisplayListVertex);

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = (createInfo.key.options.topology == topologyLineList) ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
			VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineDepthStencilStateCreateInfo depthState{};
		depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthState.depthTestEnable = VK_FALSE;
		depthState.depthWriteEnable = VK_FALSE;
		depthState.depthCompareOp = VK_COMPARE_OP_ALWAYS;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipeline.layout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, GetAllocator(), &pipeline.pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		SetObjectName(reinterpret_cast<uint64_t>(pipeline.pipeline), VK_OBJECT_TYPE_PIPELINE, name);
	}

	static void CreatePipelines()
	{
		DisplayListPipelineKey key;
		key.options.topology = topologyTriangleList;
		{
			key.options.bBoundTexture = true;
			PipelineCreateInfo<DisplayListPipelineKey> createInfo{ "shaders/displaylist.vert.spv" , "shaders/displaylist.frag.spv", "", key};
			DisplayList::CreatePipeline(createInfo, gRenderPass, "Native Display List TriList");
		}
		{
			key.options.bBoundTexture = false;
			PipelineCreateInfo<DisplayListPipelineKey> createInfo{ "shaders/displaylist.vert.spv" , "shaders/displaylistnotex.frag.spv", "", key };
			DisplayList::CreatePipeline(createInfo, gRenderPass, "Native Display List TriList No Tex");
		}
		{
			key.options.bBoundTexture = true;
			key.options.topology = topologyLineList;
			PipelineCreateInfo<DisplayListPipelineKey> createInfo{ "shaders/displaylist.vert.spv" , "shaders/displaylist.frag.spv", "shaders/displaylist.geom.spv", key};
			DisplayList::CreatePipeline(createInfo, gRenderPass, "Native Display List LineList");
		}
	}

	void CheckBufferSizes()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);

		assert(properties.limits.maxPushConstantsSize >= sizeof(BlendingState));
	}

	static void InitializeDescriptorsSets(Renderer::SimpleTexture* pTexture)
	{
		if (!pTexture) {
			return;
		}

		if (pTexture->GetRenderer()->HasDescriptorSets(GetPipeline())) {
			return;
		}

		PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

		// Work out the sampler
		auto& textureRegisters = pTexture->GetTextureRegisters();
		PS2::PSSamplerSelector selector = PS2::EmulateTextureSampler(pTextureData->width, pTextureData->height, textureRegisters.clamp, textureRegisters.tex, {});

		VkSampler& sampler = PS2::GetSampler(selector);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = pTextureData->imageView;
		imageInfo.sampler = sampler;

		DescriptorWriteList writeList;
		writeList.EmplaceWrite({ 0, EBindingStage::Fragment, nullptr, &imageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			pTextureData->UpdateDescriptorSets(GetPipeline(), writeList, i);
		}
	}

	static void FinalizeDraw()
	{
		const int indexCount = gVertexBuffers.GetDrawBufferData().GetIndexTail() - gIndexStart;

		if (indexCount > 0) {
			VkCommandBuffer& cmd = GetCommandBuffer();

			BlendingState blendState = [&cmd]()
				{
					return gBoundTexture ? Native::SetBlendingDynamicState(gBoundTexture, true, cmd) : Native::SetBlendingDynamicState(PS2::GetGSState().ALPHA, true, cmd);
				}();

			auto& pipeline = GetPipeline();

			if (gBoundTexture) {
				
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &gBoundTexture->GetRenderer()->GetDescriptorSets(pipeline).GetSet(GetCurrentFrame()), 0, NULL);
			}
			else {
				vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(BlendingState), &blendState);
			}

			vkCmdDrawIndexed(cmd, static_cast<uint32_t>(indexCount), 1, gIndexStart, 0, 0);

			Renderer::Debug::EndLabel(GetCommandBuffer());
		}

		gVertexBuffers.Reset();
	}
}

// Implementations from "displaylist.h"

static bool bCalledBegin = false;

void Renderer::DisplayList::Begin2D(short viewportWidth, short viewportHeight, uint32_t mode)
{
	using namespace Renderer::Native::DisplayList;

	if (bCalledBegin) {
		return;
	}

	bCalledBegin = true;

	gViewport.width = viewportWidth;
	gViewport.height = viewportHeight;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = gViewport.width;
	viewport.height = gViewport.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(GetCommandBuffer(), 0, 1, &viewport);

	// We also need to apply prim here
	// Based on GSState::ApplyPRIM
	PS2::GetGSState().PRIM.PRIM = mode;
	if (gVertexBuffers.GetDrawBufferData().index.tail == 0) {
		gVertexBuffers.GetDrawBufferData().vertex.next = 0;
	}

	gVertexBuffers.GetDrawBufferData().vertex.head = gVertexBuffers.GetDrawBufferData().vertex.tail = gVertexBuffers.GetDrawBufferData().vertex.next;

	// Need to do this after we have updated prim.
	InitializeDescriptorsSets(gBoundTexture);

	vkCmdBindPipeline(GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipeline().pipeline);
}

void Renderer::DisplayList::SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, float q)
{
	using namespace Renderer::Native::DisplayList;

	gColor.r = r;
	gColor.g = g;
	gColor.b = b;
	gColor.a = a;

	gQ = q;
}

void Renderer::DisplayList::SetTexCoord(float s, float t)
{
	using namespace Renderer::Native::DisplayList;

	gTexCoord.s = s;
	gTexCoord.t = t;
}

void Renderer::DisplayList::SetVertex(float x, float y, float z, uint32_t skip)
{
	using namespace Renderer::Native::DisplayList;

	// Convert x, y to normalized viewport coords for vulkan between -1.0 and 1.0
	float newx = (2.0f * x / gViewport.width) - 1.0f;
	float newy = (2.0f * y / gViewport.height) - 1.0f;

	Renderer::DisplayListVertex vertex{};

	vertex.RGBA[0] = gColor.r;
	vertex.RGBA[1] = gColor.g;
	vertex.RGBA[2] = gColor.b;
	vertex.RGBA[3] = gColor.a;

	vertex.ST[0] = gTexCoord.s;
	vertex.ST[1] = gTexCoord.t;

	vertex.Q = gQ;

	vertex.XYZ[0] = newx;
	vertex.XYZ[1] = newy;
	vertex.XYZ[2] = z;

	KickVertex<DisplayListVertex, uint16_t>(vertex, PS2::GetGSState().PRIM, skip, gVertexBuffers.GetDrawBufferData());
}

void Renderer::DisplayList::End2D()
{
	using namespace Renderer::Native::DisplayList;

	bCalledBegin = false;

	// Nothing for now.
}

void Renderer::DisplayList::BindTexture(SimpleTexture* pNewTexture)
{
	using namespace Renderer::Native::DisplayList;

	FinalizeDraw();

	if (!gRecordingCommandBuffer) {
		BeginCommandBufferRecording();
	}
	
	gBoundTexture = pNewTexture;

	Renderer::Debug::BeginLabel(GetCommandBuffer(), gBoundTexture ? gBoundTexture->GetName().c_str() : "No Texture Binding");

	gIndexStart = gVertexBuffers.GetDrawBufferData().GetIndexTail();
	gVertexStart = gVertexBuffers.GetDrawBufferData().GetVertexTail();
}

// End of "displaylist.h"

void Renderer::Native::DisplayList::Setup()
{
	gCommandPool = CreateCommandPool("Display List Command Pool");
	CreateCommandBuffers(gCommandPool, gCommandBuffers, "Display List Command Buffer");

	DisplayList::CreateRenderPass(gRenderPass, "Display List Render Pass");

	CreatePipelines();
	CheckBufferSizes();

	gVertexBuffers.Init(Renderer::VertexIndexBufferSizeGPU, Renderer::VertexIndexBufferSizeGPU);
}

VkCommandBuffer& Renderer::Native::DisplayList::FinalizeCommandBuffer(bool bEndCommandBuffer /*= true*/)
{
	if (!gRecordingCommandBuffer) {
		BeginCommandBufferRecording();
	}

	if (gBoundTexture) {
		FinalizeDraw();
	}

	VkCommandBuffer& cmd = GetCommandBuffer();

	gVertexBuffers.MapData();

	gVertexBuffers.Reset();
	gVertexBuffers.GetDrawBufferData().ResetAfterDraw();

	vkCmdEndRenderPass(cmd);

	Renderer::Debug::EndLabel(cmd);

	if (bEndCommandBuffer) {
		vkEndCommandBuffer(cmd);
	}

	gRecordingCommandBuffer = false;

	gBoundTexture = nullptr;
	gIndexStart = 0;
	gVertexStart = 0;

	return cmd;
}
