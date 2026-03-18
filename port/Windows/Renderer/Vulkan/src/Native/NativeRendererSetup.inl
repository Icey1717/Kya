namespace Renderer
{
	namespace Native
	{
		int gWidth = kDefaultWidth;
		int gHeight = kDefaultHeight;
		int gPendingResizeWidth = 0;
		int gPendingResizeHeight = 0;

		static void CreateFramebuffer()
		{
			gFrameBuffer.SetupBase({ gWidth, gHeight }, gRenderPass[RenderPassKey::Empty].gRenderPass, true);
		}
		static void CreateFramebufferSampler()
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
		VkDeviceSize CheckBufferSizes()
		{
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);
			const VkDeviceSize maxUboSize = properties.limits.maxUniformBufferRange;

			// Model
			assert(maxUboSize >= (sizeof(glm::mat4) * gMaxInstances));

			// Lighting Data
			assert(maxUboSize >= (sizeof(LightingDynamicBufferData) * gMaxLightingData));

			// Anim ST
			assert(maxUboSize >= (sizeof(glm::vec4) * gMaxAnimStData));

			assert(properties.limits.maxPushConstantsSize >= sizeof(PerDrawData));

			return maxUboSize;
		}
		static void InitFade()
		{
			gFadeBuffer.Init();

			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				const VkDescriptorBufferInfo vertexDescBufferInfo = gFadeBuffer.GetDescBufferInfo(i);

				DescriptorWriteList writeList;
				writeList.EmplaceWrite({ 1, EBindingStage::Fragment, &vertexDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
				PostProcessing::UpdateDescriptorSets(PostProcessing::Effect::Fade, i, writeList);
			}
		}
		void CreateRenderStage(const RenderPassKey& key, const char* name)
		{
			RenderStage& stage = gRenderPass[key];

			const bool bClearColor = (key.clearMode != EClearMode::None && key.clearMode != EClearMode::Depth);
			const bool bClearDepth = (key.clearMode != EClearMode::None && key.clearMode != EClearMode::Color);

			const Renderer::AttachmentInfo colorInfo{
				GetSwapchainImageFormat(),
				bClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
				bClearColor ? VK_IMAGE_LAYOUT_UNDEFINED   : VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			};

			const Renderer::AttachmentInfo depthInfo{
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				bClearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
				bClearDepth ? VK_IMAGE_LAYOUT_UNDEFINED   : VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			};

			const VkSubpassDependency dependency{
				VK_SUBPASS_EXTERNAL, 0,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				0
			};

			stage.gRenderPass = Renderer::CreateRenderPass2D({ &colorInfo, 1 }, depthInfo, { &dependency, 1 }, name);

			stage.CreatePipeline();

			std::string debugLineName = std::string(name) + " Debug Lines";
			DebugShapes::CreatePipeline(stage.gRenderPass, stage.gDebugLinePipeline, debugLineName.c_str());
		}

		void CreatePipeline(const PipelineCreateInfo<PipelineKey>& createInfo, const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name)
		{
			pipeline.debugName = name;

			auto vertShader = Shader::ReflectedModule(createInfo.vertShaderFilename, VK_SHADER_STAGE_VERTEX_BIT);
			auto fragShader = Shader::ReflectedModule(createInfo.fragShaderFilename, VK_SHADER_STAGE_FRAGMENT_BIT);

			// Vert
			vertShader.reflectData.MarkUniformBufferDynamic(0, 2);
			vertShader.reflectData.MarkUniformBufferDynamic(0, 3);
			vertShader.reflectData.MarkUniformBufferDynamic(0, 4);
			vertShader.reflectData.MarkUniformBufferDynamic(0, 5);

			// Frag
			fragShader.reflectData.MarkUniformBufferDynamic(0, 6);

			pipeline.AddBindings(EBindingStage::Vertex, vertShader.reflectData);
			pipeline.AddBindings(EBindingStage::Fragment, fragShader.reflectData);
			pipeline.CreateDescriptorSetLayouts();

			pipeline.CreateLayout();

			pipeline.CreateDescriptorPool();
			pipeline.CreateDescriptorSets();

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShader.shaderStageCreateInfo, fragShader.shaderStageCreateInfo };

			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			auto& bindingDescription = vertShader.reflectData.bindingDescription;
			const auto& attributeDescriptions = vertShader.reflectData.GetAttributes();

			bindingDescription.stride = sizeof(GSVertexUnprocessed);

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
				VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
				VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT,
				VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
			};
			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			VkPipelineDepthStencilStateCreateInfo depthState{};
			depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthState.depthTestEnable = VK_TRUE;
			depthState.depthWriteEnable = VK_TRUE;
			depthState.depthCompareOp = VK_COMPARE_OP_GREATER;

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
		Renderer::FrameBufferBase& GetFrameBuffer()
		{
			return gFrameBuffer;
		}

		void Setup()
		{
			const VkDeviceSize maxUboSize = CheckBufferSizes();

			RenderPassKey key;
			key.clearMode = EClearMode::None;

			CreateRenderStage(key, "Native Render Pass CM None");
			key.clearMode = EClearMode::Depth;
			CreateRenderStage(key, "Native Render Pass CM Depth");
			key.clearMode = EClearMode::ColorDepth;
			CreateRenderStage(key, "Native Render Pass CM ColorDepth");
			key.clearMode = EClearMode::Color;
			CreateRenderStage(key, "Native Render Pass CM Color");

			CreateFramebuffer();
			CreateFramebufferSampler();
			gCommandPool = CreateCommandPool("Native Renderer Command Pool");
			CreateCommandBuffers(gCommandPool, gCommandBuffers, "Native Renderer Command Buffer");

			gMaxAnimationMatrices = static_cast<int>(maxUboSize / sizeof(glm::mat4));

			gNativeVertexBuffer.Init(0x100000, 0x100000);
			DebugShapes::Setup();
			DebugShapes::SetupDedicatedPass(gFrameBuffer.colorImageView, gWidth, gHeight);

			gModelBuffer.Init();
			gAnimationBuffer.Init(gMaxAnimationMatrices);

			gNativeVertexBufferDataDraw.Init(0x10000, 0x10000);

			gAlphaBuffer.Init(gMaxInstances);

			gLightingDynamicBuffer.Init();
			gAnimStBuffer.Init();

			GetRenderDelegate() += Render;

			gRenderThread = std::make_unique<RenderThread>();

			PostProcessing::Setup();
			DisplayList::Setup();

			InitFade();
		}

		void ResizeFrameBuffer(int width, int height)
		{
			if (width == gWidth && height == gHeight)
			{
				return;
			}

			gPendingResizeWidth = width;
			gPendingResizeHeight = height;
		}

		static void ApplyPendingResizeInternal()
		{
			if (gPendingResizeWidth == 0 && gPendingResizeHeight == 0)
			{
				return;
			}

			const int width = gPendingResizeWidth;
			const int height = gPendingResizeHeight;
			gPendingResizeWidth = 0;
			gPendingResizeHeight = 0;

			vkDeviceWaitIdle(GetDevice());

			DebugShapes::DestroyDedicatedPass();

			vkDestroyFramebuffer(GetDevice(), gFrameBuffer.framebuffer, GetAllocator());
			vkDestroyImageView(GetDevice(), gFrameBuffer.colorImageView, GetAllocator());
			vkDestroyImage(GetDevice(), gFrameBuffer.colorImage, GetAllocator());
			vkFreeMemory(GetDevice(), gFrameBuffer.imageMemory, GetAllocator());
			vkDestroyImageView(GetDevice(), gFrameBuffer.depthImageView, GetAllocator());
			vkDestroyImage(GetDevice(), gFrameBuffer.depthImage, GetAllocator());
			vkFreeMemory(GetDevice(), gFrameBuffer.depthImageMemory, GetAllocator());
			vkDestroySampler(GetDevice(), gFrameBuffer.sampler, GetAllocator());

			gWidth = width;
			gHeight = height;

			CreateFramebuffer();

			DebugShapes::SetupDedicatedPass(gFrameBuffer.colorImageView, gWidth, gHeight);

			PostProcessing::Resize();
		}

		VkExtent2D GetFrameBufferSize()
		{
			return { static_cast<uint32_t>(gWidth), static_cast<uint32_t>(gHeight) };
		}

	} // Native
} // Renderer
