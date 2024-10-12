#include "NativeRenderer.h"

#include "NativeBlending.h"
#include "NativeDebug.h"

#include <array>
#include <stdexcept>

#include "VulkanRenderer.h"
#include "renderer.h"
#include "FrameBuffer.h"
#include "UniformBuffer.h"
#include "TextureCache.h"
#include "glm/gtc/type_ptr.inl"
#include "logging.h"
#include "log.h"

#define NATIVE_LOG(level, format, ...) MY_LOG_CATEGORY("NativeRenderer", level, format, ##__VA_ARGS__)
#define NATIVE_LOG_VERBOSE(level, format, ...)

namespace Renderer
{
	namespace Native
	{
		constexpr int gWidth = 0x200;
		constexpr int gHeight = 0x200;

		constexpr int gMaxAnimMatrices = 0x60;
		constexpr int gMaxStripIndex = 0x20;

		static bool bForceAnimMatrixIdentity = false;

		static bool gUsePreprocessedVertices = false;

		struct LightingData {
			glm::mat4 lightDirection;
			glm::mat4 lightColor;
			glm::vec4 lightAmbient;
		};

		struct VertexConstantBuffer {

			void Reset()
			{
				view = glm::mat4(1.0f);
				proj = glm::mat4(1.0f);
			}

			glm::mat4 view;
			glm::mat4 proj;
		};

		constexpr int gMaxModelMatrices = 1024;

		int gMaxAnimationMatrices = 0;

		PipelineMap gPipelines;
		Renderer::FrameBufferBase gFrameBuffer;

		VkSampler gFrameBufferSampler;

		VkRenderPass gRenderPass;
		Renderer::CommandBufferVector gCommandBuffers;

		UniformBuffer<VertexConstantBuffer> gVertexConstantBuffer;
		DynamicUniformBuffer<glm::mat4> gModelBuffer;

		struct AlphaConstantBuffer {
			alignas(4) VkBool32 enable; // Use VkBool32 for proper alignment
			alignas(4) int atst;
			alignas(4) int aref;

			AlphaConstantBuffer(const Renderer::TextureRegisters& textureRegisters)
				: enable(textureRegisters.test.ATE)
				, atst(textureRegisters.test.ATST)
				, aref(textureRegisters.test.AREF)
			{ }
		};

		DynamicUniformBuffer<AlphaConstantBuffer> gAlphaBuffer;

		using NativeVertexBuffer = PS2::FrameVertexBuffers<Renderer::GSVertexUnprocessedNormal, uint16_t>;
		NativeVertexBuffer gNativeVertexBuffer;
		// As each draw comes in this buffer is filled.
		NativeVertexBufferData gNativeVertexBufferDataDraw;

		DynamicUniformBuffer<glm::mat4> gAnimationBuffer;
		std::vector<glm::mat4> gAnimationMatrices;

		std::vector<glm::mat4> gModelMatrices;

		struct Draw {
			SimpleTexture* pTexture = nullptr;

			glm::mat4 projMatrix;
			glm::mat4 viewMatrix;

			LightingData lightingData;

			struct Instance {
				SimpleMesh* pMesh = nullptr;
				glm::mat4 modelMatrix;
				int indexStart = 0;
				int indexCount = 0;
				int vertexStart = 0;
				int animationMatrixStart = 0;
			};

			std::vector<Instance> instances;
		};

		std::optional<Draw> gCurrentDraw;
		std::vector<Draw> gDraws;

		glm::mat4 gCachedModelMatrix;
		glm::mat4 gCachedViewMatrix;
		glm::mat4 gCachedProjMatrix;

		LightingData gCachedLightingData;

		void FillIndexData()
		{
			assert(gCurrentDraw->instances.size() > 0);

			NATIVE_LOG_VERBOSE(LogLevel::Info, "FillIndexData Filling instance: {}", gCurrentDraw->instances.size() - 1);

			Draw::Instance& instance = gCurrentDraw->instances.back();

			auto& vertexBufferData = gUsePreprocessedVertices ? instance.pMesh->GetInternalVertexBufferData() : gNativeVertexBufferDataDraw;

			instance.indexCount = vertexBufferData.GetIndexTail();
			instance.indexStart = gNativeVertexBuffer.GetDrawBufferData().GetIndexTail();
			instance.vertexStart = gNativeVertexBuffer.GetDrawBufferData().GetVertexTail();

			NATIVE_LOG_VERBOSE(LogLevel::Info, "FillIndexData Filled instance: {} indexCount: {} indexStart: {} vertexStart: {}",
				gCurrentDraw->instances.size() - 1, instance.indexCount, instance.indexStart, instance.vertexStart);

			// Copy into the real buffer.
			gNativeVertexBuffer.MergeData(vertexBufferData);

			if (!gUsePreprocessedVertices) {
				vertexBufferData.ResetAfterDraw();
			}
		}

		void MergeIndexData()
		{
			assert(gCurrentDraw->instances.size() > 0);
			NATIVE_LOG_VERBOSE(LogLevel::Info, "MergeIndexData Merging instance: {}", gCurrentDraw->instances.size() - 1);

			Draw::Instance& instance = gCurrentDraw->instances.back();
			instance.indexCount += gNativeVertexBufferDataDraw.GetIndexTail();

			NATIVE_LOG_VERBOSE(LogLevel::Info, "MergeIndexData Merging instance: {} indexCount: {} indexStart: {} vertexStart: {}",
				gCurrentDraw->instances.size() - 1, instance.indexCount, instance.indexStart, instance.vertexStart);

			// Copy into the real buffer.
			gNativeVertexBuffer.MergeData(gNativeVertexBufferDataDraw);
			gNativeVertexBufferDataDraw.ResetAfterDraw();
		}

		static bool NearlyEqual(const glm::mat4& mat1, const glm::mat4& mat2, float epsilon = 1e-5f) {
			for (int i = 0; i < 4; ++i) {
				glm::vec4 v1 = mat1[i];
				glm::vec4 v2 = mat2[i];
				if (!glm::all(glm::lessThan(glm::abs(v1 - v2), glm::vec4(epsilon)))) {
					return false;
				}
			}

			return true;
		}

		void CreateFramebuffer()
		{
			gFrameBuffer.SetupBase({ gWidth, gHeight }, gRenderPass, true);
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

		void CreatePipeline()
		{
			PipelineKey key;
			key.options.bGlsl = true;
			key.options.bWireframe = false;
			PipelineCreateInfo createInfo{ "shaders/native.vert.spv" , "shaders/native.frag.spv", key };
			CreatePipeline(createInfo, gRenderPass, gPipelines[createInfo.key.key], "Native Previewer GLSL");
		}

		Renderer::Pipeline& GetPipeline() {
			PipelineKey pipelineKey;
			pipelineKey.options.bWireframe = false;
			pipelineKey.options.bGlsl = true;

			assert(gPipelines.find(pipelineKey.key) != gPipelines.end());
			return gPipelines[pipelineKey.key];
		}

		void UpdateDescriptors()
		{
			auto& pipeline = GetPipeline();
			
			int modelIndex = 0;
			int textureIndex = 0;

			glm::mat4 viewMatrix;
			glm::mat4 projMatrix;

			if (!gDraws.empty()) {
				viewMatrix = gDraws.back().viewMatrix;
				projMatrix = gDraws.back().projMatrix;

				gVertexConstantBuffer.GetBufferData().view = viewMatrix;
				gVertexConstantBuffer.GetBufferData().proj = projMatrix;
			}

			gVertexConstantBuffer.Update(GetCurrentFrame());
			
			for (auto& drawCommand : gDraws) {
				Renderer::SimpleTexture* pTexture = drawCommand.pTexture;
			
				if (!pTexture) {
					break;
				}

				//assert(NearlyEqual(viewMatrix, drawCommand.viewMatrix));
				//assert(NearlyEqual(projMatrix, drawCommand.projMatrix));

				NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());
			
				PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

				// Work out the sampler
				auto& textureRegisters = pTexture->GetTextureRegisters();
				PS2::PSSamplerSelector selector = PS2::EmulateTextureSampler(pTextureData->width, pTextureData->height, textureRegisters.clamp, textureRegisters.tex, {});

				VkSampler& sampler = PS2::GetSampler(selector);
			
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = pTextureData->imageView;
				imageInfo.sampler = sampler;
			
				const VkDescriptorBufferInfo vertexDescBufferInfo = gVertexConstantBuffer.GetDescBufferInfo(GetCurrentFrame());
				const VkDescriptorBufferInfo modelDescBufferInfo = gModelBuffer.GetDescBufferInfo(GetCurrentFrame());
				const VkDescriptorBufferInfo animDescBufferInfo = gAnimationBuffer.GetDescBufferInfo(GetCurrentFrame(), 27 * sizeof(glm::mat4));

				const VkDescriptorBufferInfo alphaDescBufferInfo = gAlphaBuffer.GetDescBufferInfo(GetCurrentFrame());

				NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: offset: {} range: {}", animDescBufferInfo.offset, animDescBufferInfo.range);
			
				Renderer::DescriptorWriteList writeList;
				writeList.EmplaceWrite({ 0, Renderer::EBindingStage::Vertex, &vertexDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
				writeList.EmplaceWrite({ 2, Renderer::EBindingStage::Vertex, &modelDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
				writeList.EmplaceWrite({ 3, Renderer::EBindingStage::Vertex, &animDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });

				writeList.EmplaceWrite({ 4, Renderer::EBindingStage::Fragment, &alphaDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
				writeList.EmplaceWrite({ 1, Renderer::EBindingStage::Fragment, nullptr, &imageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

				pTextureData->UpdateDescriptorSets(pipeline, writeList);

				for (auto& instance : drawCommand.instances) {
					assert(modelIndex < gMaxModelMatrices);

					Renderer::SimpleMesh* pMesh = instance.pMesh;
					NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {}", pMesh->GetName());

					gModelBuffer.SetInstanceData(modelIndex, instance.modelMatrix);
					modelIndex++;
				}

				gAlphaBuffer.SetInstanceData(textureIndex, { pTexture->GetTextureRegisters() });
				textureIndex++;
			}

			gModelBuffer.Update(GetCurrentFrame());
			gAlphaBuffer.Update(GetCurrentFrame());

			for (int i = 0; i < gAnimationMatrices.size() ; i++) {
				if (bForceAnimMatrixIdentity) {
					gAnimationMatrices[i] = glm::mat4(1.0f);
				}

				gAnimationBuffer.SetInstanceData(i, gAnimationMatrices[i]);
			}

			gAnimationBuffer.Update(GetCurrentFrame());
		}

		VkDeviceSize CheckBufferSizes()
		{
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);
			const VkDeviceSize maxUboSize = properties.limits.maxUniformBufferRange;

			assert(maxUboSize >= sizeof(VertexConstantBuffer));
			assert(maxUboSize >= (sizeof(glm::mat4) * gMaxModelMatrices));

			return maxUboSize;
		}

		bool CanMergeMesh()
		{
			if (!gCurrentDraw) {
				return false;
			}

			if (gCurrentDraw->instances.empty()) {
				return false;
			}

			auto& instance = gCurrentDraw->instances.back();

			if (instance.animationMatrixStart != gAnimationMatrices.size()) {
				return false;
			}

			if (instance.modelMatrix != gCachedModelMatrix) {
				return false;
			}

			return true;
		}

		void CreateDraw()
		{
			gCurrentDraw = Draw{};
		}

	} // Native
} // Renderer

// Simple mesh impl

Renderer::NativeVertexBufferData& Renderer::SimpleMesh::GetVertexBufferData()
{
	return Native::gUsePreprocessedVertices ? internalVertexBufferData : Native::gNativeVertexBufferDataDraw;
}

void Renderer::Native::CreateRenderPass(VkRenderPass& renderPass, const char* name)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = GetSwapchainImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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

void Renderer::Native::CreatePipeline(const PipelineCreateInfo& createInfo, const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name)
{
	auto vertShader = Shader::ReflectedModule(createInfo.vertShaderFilename, VK_SHADER_STAGE_VERTEX_BIT);
	auto fragShader = Shader::ReflectedModule(createInfo.fragShaderFilename, VK_SHADER_STAGE_FRAGMENT_BIT);

	vertShader.reflectData.MarkUniformBufferDynamic(0, 2);
	vertShader.reflectData.MarkUniformBufferDynamic(0, 3);
	fragShader.reflectData.MarkUniformBufferDynamic(0, 4);

	pipeline.AddBindings(Renderer::EBindingStage::Vertex, vertShader.reflectData);
	pipeline.AddBindings(Renderer::EBindingStage::Fragment, fragShader.reflectData);
	pipeline.CreateDescriptorSetLayouts();

	pipeline.CreateLayout(vertShader.reflectData.pushConstants);

	pipeline.CreateDescriptorPool();
	pipeline.CreateDescriptorSets();

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShader.shaderStageCreateInfo, fragShader.shaderStageCreateInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto& bindingDescription = vertShader.reflectData.bindingDescription;
	const auto& attributeDescriptions = vertShader.reflectData.GetAttributes();

	bindingDescription.stride = sizeof(Renderer::GSVertexUnprocessed);

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
	rasterizer.polygonMode = createInfo.key.options.bWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
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
	depthState.depthTestEnable = VK_TRUE;
	depthState.depthWriteEnable = VK_TRUE;
	depthState.depthCompareOp = VK_COMPARE_OP_LESS;

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
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, GetAllocator(), &pipeline.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	SetObjectName(reinterpret_cast<uint64_t>(pipeline.pipeline), VK_OBJECT_TYPE_PIPELINE, name);
}

void Renderer::Native::Setup()
{
	CreateRenderPass(gRenderPass, "Mesh Previewer");
	CreateFramebuffer();
	CreateFramebufferSampler();
	Renderer::CreateCommandBuffers(gCommandBuffers);
	CreatePipeline();

	const VkDeviceSize maxUboSize = CheckBufferSizes();
	gMaxAnimationMatrices = static_cast<int>(maxUboSize / sizeof(glm::mat4));

	gVertexConstantBuffer.Init();
	gNativeVertexBuffer.Init(0x100000, 0x100000);

	gModelBuffer.Init(gMaxModelMatrices);
	gAnimationBuffer.Init(gMaxAnimationMatrices);

	gNativeVertexBufferDataDraw.Init(0x10000, 0x10000);

	gAlphaBuffer.Init(gMaxModelMatrices);

	Renderer::GetRenderDelegate() += Renderer::Native::Render;

	gVertexConstantBuffer.GetBufferData().Reset();
}

void Renderer::Native::Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent)
{
	const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd, &beginInfo);

	Renderer::Debug::BeginLabel(cmd, "Native Render");

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

	if (!gDraws.empty() && gNativeVertexBuffer.GetDrawBufferData().GetIndexTail() > 0) {
		gNativeVertexBuffer.BindData(cmd);
	}

	int textureIndex = 0;
	int modelIndex = 0;

	Debug::Reset();

	for (auto& drawCommand : gDraws) {
		Renderer::SimpleTexture* pTexture = drawCommand.pTexture;

		if (!pTexture) {
			continue;
		}

		if (drawCommand.instances.empty()) {
			continue;
		}

		Debug::UpdateLabel(pTexture, cmd);

		vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LightingData), &drawCommand.lightingData);

		PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

		std::optional<uint> primState;

		for (auto& instance : drawCommand.instances) {
			if (instance.indexCount == 0) {
				continue;
			}

			if (instance.pMesh->GetName() == "KIM_Scene01_L.O.D..g3d_0_1_0") {
				modelIndex = modelIndex;
			}

			if (!primState.has_value() || primState.value() != instance.pMesh->GetPrim().CMD) {
				primState = instance.pMesh->GetPrim().CMD;
				SetBlendingDynamicState(pTexture, instance.pMesh, cmd);
			}

			std::array< uint32_t, 3 > dynamicOffsets = {
				modelIndex * gModelBuffer.GetDynamicAlignment(),
				instance.animationMatrixStart * gAnimationBuffer.GetDynamicAlignment(),
				textureIndex * gAlphaBuffer.GetDynamicAlignment()
			};

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &pTextureData->GetDescriptorSets(pipeline).GetSet(GetCurrentFrame()), dynamicOffsets.size(), dynamicOffsets.data());

			vkCmdDrawIndexed(cmd, static_cast<uint32_t>(instance.indexCount), 1, instance.indexStart, instance.vertexStart, 0);

			modelIndex++;
		}

		textureIndex++;
	}

	gNativeVertexBuffer.GetDrawBufferData().ResetAfterDraw();

	vkCmdEndRenderPass(cmd);

	Renderer::Debug::EndLabel(cmd);
	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GetGraphicsQueue());

	gNativeVertexBuffer.Reset();
	gDraws.clear();

	gAnimationMatrices.clear();
}

void Renderer::Native::BindTexture(SimpleTexture* pTexture)
{
	NATIVE_LOG(LogLevel::Info, "BindTexture: {} material: {} layer: {} indices: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex(), gNativeVertexBufferDataDraw.GetIndexTail());

	if (gCurrentDraw) {
		gCurrentDraw->pTexture = pTexture;

		gCurrentDraw->projMatrix = gCachedProjMatrix;
		gCurrentDraw->viewMatrix = gCachedViewMatrix;
		gCurrentDraw->lightingData = gCachedLightingData;

		for (auto& instance : gCurrentDraw->instances) {
			NATIVE_LOG(LogLevel::Info, "BindTexture: instance anim start: {}", instance.animationMatrixStart);
		}

		gDraws.push_back(*gCurrentDraw);

		gCurrentDraw.reset();
	}

	NATIVE_LOG(LogLevel::Info, "BindTexture Done\n-------------------------------------------------------\n");
}

void Renderer::Native::AddMesh(SimpleMesh* pMesh)
{
	if (!gCurrentDraw) {
		NATIVE_LOG(LogLevel::Info, "AddMesh Creating new draw!");
		CreateDraw();
	}

	NATIVE_LOG(LogLevel::Info, "AddMesh: {} prim: 0x{:x}", pMesh->GetName(), pMesh->GetPrim().CMD);

	constexpr bool bSanityCheck = false;

	if (bSanityCheck) {
		FLUSH_LOG();

		auto& internalBuffer = pMesh->GetInternalVertexBufferData();

		assert(internalBuffer.GetIndexTail() == gNativeVertexBufferDataDraw.GetIndexTail());
		assert(internalBuffer.GetVertexTail() == gNativeVertexBufferDataDraw.GetVertexTail());

		for (int i = 0; i < internalBuffer.GetIndexTail(); i++) {
			assert(internalBuffer.index.buff[i] == gNativeVertexBufferDataDraw.index.buff[i]);
		}

		for (int i = 0; i < internalBuffer.GetVertexTail(); i++) {
			{
				const auto& vtx = internalBuffer.vertex.buff[i];
				NATIVE_LOG(LogLevel::Info, "Renderer::Native::AddMesh Preprocess vertex: {}, (S: {} T: {} Q: {}) (R: {} G: {} B: {} A: {}) (X: {} Y: {} Z: {} Skip: {}) ({}, {}, {})\n",
					i, vtx.STQ.ST[0], vtx.STQ.ST[1], vtx.STQ.Q, vtx.RGBA[0], vtx.RGBA[1], vtx.RGBA[2], vtx.RGBA[3], vtx.XYZFlags.fXYZ[0], vtx.XYZFlags.fXYZ[1], vtx.XYZFlags.fXYZ[2], vtx.XYZFlags.flags,
					vtx.normal.fNormal[0], vtx.normal.fNormal[1], vtx.normal.fNormal[2]);
			}

			{
				const auto& vtx = gNativeVertexBufferDataDraw.vertex.buff[i];
				NATIVE_LOG(LogLevel::Info, "Renderer::Native::AddMesh Live vertex: {}, (S: {} T: {} Q: {}) (R: {} G: {} B: {} A: {}) (X: {} Y: {} Z: {} Skip: {})({}, {}, {})\n",
					i, vtx.STQ.ST[0], vtx.STQ.ST[1], vtx.STQ.Q, vtx.RGBA[0], vtx.RGBA[1], vtx.RGBA[2], vtx.RGBA[3], vtx.XYZFlags.fXYZ[0], vtx.XYZFlags.fXYZ[1], vtx.XYZFlags.fXYZ[2], vtx.XYZFlags.flags,
					vtx.normal.fNormal[0], vtx.normal.fNormal[1], vtx.normal.fNormal[2]);
			}

			assert(internalBuffer.vertex.buff[i].XYZFlags.fXYZ[0] == gNativeVertexBufferDataDraw.vertex.buff[i].XYZFlags.fXYZ[0]);
			assert(internalBuffer.vertex.buff[i].XYZFlags.fXYZ[1] == gNativeVertexBufferDataDraw.vertex.buff[i].XYZFlags.fXYZ[1]);
			assert(internalBuffer.vertex.buff[i].XYZFlags.fXYZ[2] == gNativeVertexBufferDataDraw.vertex.buff[i].XYZFlags.fXYZ[2]);

			assert(internalBuffer.vertex.buff[i].normal.fNormal[0] == gNativeVertexBufferDataDraw.vertex.buff[i].normal.fNormal[0]);
			assert(internalBuffer.vertex.buff[i].normal.fNormal[1] == gNativeVertexBufferDataDraw.vertex.buff[i].normal.fNormal[1]);
			assert(internalBuffer.vertex.buff[i].normal.fNormal[2] == gNativeVertexBufferDataDraw.vertex.buff[i].normal.fNormal[2]);

			assert(internalBuffer.vertex.buff[i].STQ.ST[0] == gNativeVertexBufferDataDraw.vertex.buff[i].STQ.ST[0]);
			assert(internalBuffer.vertex.buff[i].STQ.ST[1] == gNativeVertexBufferDataDraw.vertex.buff[i].STQ.ST[1]);
			assert(internalBuffer.vertex.buff[i].STQ.Q == gNativeVertexBufferDataDraw.vertex.buff[i].STQ.Q);
		}
	}

	constexpr bool bMergingEnabeld = false;

	if (bMergingEnabeld && CanMergeMesh()) {
		NATIVE_LOG(LogLevel::Info, "AddMesh: merging");
		MergeIndexData();
	}
	else {
		auto& instance = gCurrentDraw->instances.emplace_back();
		instance.animationMatrixStart = gAnimationMatrices.size();
		instance.modelMatrix = gCachedModelMatrix;
		instance.pMesh = pMesh;

		NATIVE_LOG(LogLevel::Info, "AddMesh: instance anim start: {}", instance.animationMatrixStart);
		FillIndexData();
	}
}

void Renderer::Native::PushGlobalMatrices(float* pModel, float* pView, float* pProj)
{
	NATIVE_LOG(LogLevel::Info, "PushGlobalMatrices");

	// copy into model.
	if (pProj) {
		gCachedProjMatrix = glm::make_mat4(pProj);
	}

	if (pView) {
		gCachedViewMatrix = glm::make_mat4(pView);
	}

	PushModelMatrix(pModel);
}

void Renderer::Native::PushLightData(float* pDirection, float* pColor, float* pAmbient)
{
	NATIVE_LOG(LogLevel::Info, "PushLightData");

	if (pDirection) {
		gCachedLightingData.lightDirection = glm::make_mat4(pDirection);	
	}

	if (pColor) {
		gCachedLightingData.lightColor = glm::make_mat4(pColor);
	}

	if (pAmbient) {
		gCachedLightingData.lightAmbient = glm::vec4(pAmbient[0], pAmbient[1], pAmbient[2], pAmbient[3]);
	}

	NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pDirection[0], pDirection[1], pDirection[2]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pDirection[4], pDirection[5], pDirection[6]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pDirection[8], pDirection[9], pDirection[10]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pColor[0], pColor[1], pColor[2], pColor[3]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pColor[4], pColor[5], pColor[6], pColor[7]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pColor[8], pColor[9], pColor[10], pColor[11]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: ambient: {} {} {} {}", pAmbient[0], pAmbient[1], pAmbient[2], pAmbient[3]);
}

void Renderer::Native::PushModelMatrix(float* pModel)
{
	NATIVE_LOG(LogLevel::Info, "PushModelMatrix");

	if (!gCurrentDraw) {
		NATIVE_LOG(LogLevel::Info, "PushModelMatrix Creating new draw!");
		CreateDraw();
	}

	if (pModel) {
		gCachedModelMatrix = glm::make_mat4(pModel);
	}
}

void Renderer::Native::PushAnimMatrix(float* pAnim)
{
	NATIVE_LOG(LogLevel::Info, "PushAnimMatrix");
	gAnimationMatrices.push_back(glm::make_mat4(pAnim));
}

void Renderer::Native::PushMatrixPacket(const MatrixPacket* const pPkt)
{
	NATIVE_LOG(LogLevel::Info, "PushMatrixPacket");

	assert(pPkt);

	gCachedLightingData.lightDirection = glm::make_mat4(pPkt->objLightDirectionsMatrix);
	gCachedLightingData.lightColor = glm::make_mat4(pPkt->lightColorMatrix);
	gCachedLightingData.lightAmbient = glm::vec4(pPkt->adjustedLightAmbient[0], pPkt->adjustedLightAmbient[1], pPkt->adjustedLightAmbient[2], pPkt->adjustedLightAmbient[3]);

	NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[0], pPkt->objLightDirectionsMatrix[1], pPkt->objLightDirectionsMatrix[2]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[4], pPkt->objLightDirectionsMatrix[5], pPkt->objLightDirectionsMatrix[6]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[8], pPkt->objLightDirectionsMatrix[9], pPkt->objLightDirectionsMatrix[10]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[0], pPkt->lightColorMatrix[1], pPkt->lightColorMatrix[2], pPkt->lightColorMatrix[3]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[4], pPkt->lightColorMatrix[5], pPkt->lightColorMatrix[6], pPkt->lightColorMatrix[7]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[8], pPkt->lightColorMatrix[9], pPkt->lightColorMatrix[10], pPkt->lightColorMatrix[11]);
	NATIVE_LOG(LogLevel::Info, "PushLightData: ambient: {} {} {} {}", pPkt->adjustedLightAmbient[0], pPkt->adjustedLightAmbient[1], pPkt->adjustedLightAmbient[2], pPkt->adjustedLightAmbient[3]);

}

const VkSampler& Renderer::Native::GetSampler()
{
	return gFrameBufferSampler;
}

const VkImageView& Renderer::Native::GetColorImageView()
{
	return gFrameBuffer.colorImageView;
}

bool& Renderer::Native::GetUsePreprocessedVertices()
{
	return gUsePreprocessedVertices;
}

bool& Renderer::GetForceAnimMatrixIdentity()
{
	return Native::bForceAnimMatrixIdentity;
}