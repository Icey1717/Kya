#include "DebugRenderer.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "DebugMenu.h"

#include <stdexcept>

#include "renderer.h"
#include "VulkanRenderer.h"
#include "VulkanCommands.h"
#include "TextureCache.h"
#include "FrameBuffer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ed3D.h"
#include "port/vu1_emu.h"
#include "ed3DScratchPadGlobalVar.h"
#include "port/pointer_conv.h"
#include "port.h"

namespace DebugRendererImgui {
	VkRenderPass gImguiRenderPass;
	Renderer::CommandBufferVector gCommandBuffers;

	void Setup()
	{
		VkDescriptorPool imguiPool;

		// Create Descriptor Pool
		{
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			if (vkCreateDescriptorPool(GetDevice(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}

		int minImageCount = 2;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = GetSwapchainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &gImguiRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		SetObjectName("Imgui Render Pass", (uint64_t)gImguiRenderPass, VK_OBJECT_TYPE_RENDER_PASS);

		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = GetInstance();
		initInfo.PhysicalDevice = GetPhysicalDevice();
		initInfo.Device = GetDevice();
		initInfo.QueueFamily = GetGraphicsQueueFamily();
		initInfo.Queue = GetGraphicsQueue();
		initInfo.PipelineCache = nullptr;
		initInfo.DescriptorPool = imguiPool;
		initInfo.Subpass = 0;
		initInfo.MinImageCount = minImageCount;
		initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&initInfo, gImguiRenderPass);
		ImGui_ImplGlfw_InitForVulkan(GetGLFWWindow(), true);

		// Upload Fonts
		{
			VkCommandBuffer fontCommandBuffer = BeginSingleTimeCommands();

			ImGui_ImplVulkan_CreateFontsTexture(fontCommandBuffer);
			EndSingleTimeCommands(fontCommandBuffer);

			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		Renderer::CreateCommandBuffers(gCommandBuffers);
	}

	void Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent)
	{
		const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd, &beginInfo);
		Renderer::Debug::BeginLabel(cmd, "DebugMenu");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = gImguiRenderPass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DebugMenu::BuildImguiCommands();

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(drawData, cmd);

		vkCmdEndRenderPass(cmd);

		Renderer::Debug::EndLabel(cmd);
		vkEndCommandBuffer(cmd);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(GetGraphicsQueue());
	}
}

namespace DebugMenu_Internal {
	extern edNODE* pNodeTest;
	extern ed_3D_Scene* pSceneTest;
}

namespace DebugRendererMeshViewer
{
	Renderer::Pipeline gPipeline;
	Renderer::FrameBufferBase gFrameBuffer;

	VkRenderPass gRenderPass;
	Renderer::CommandBufferVector gCommandBuffers;

	constexpr int gWidth = 0x500;
	constexpr int gHeight = 0x400;

	edpkt_data gPkt[0x100];

	struct VertexConstantBuffer {
		edF32MATRIX4 objToScreen;
		edF32MATRIX4 worldToCamera;
		edF32MATRIX4 objectToCamera;
		edF32MATRIX4 animMatrices[0x18];
	};

	UniformBuffer<VertexConstantBuffer> gVertexConstantBuffer;

	void CreateRenderPass()
	{
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

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &colorAttachment;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(GetDevice(), &renderPassCreateInfo, nullptr, &gRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		SetObjectName("Mesh Viewer Pass", (uint64_t)gRenderPass, VK_OBJECT_TYPE_RENDER_PASS);
	}

	void CreateFramebuffer()
	{
		gFrameBuffer.SetupBase({ gWidth, gHeight }, gRenderPass, false);
	}

	void CreatePipeline()
	{
		Shader::CompileShader("vs_6_0", "vs_main", "meshviewer.hlsl", "shaders/meshviewer.vert.spv", "");
		Shader::CompileShader("ps_6_0", "ps_main", "meshviewer.hlsl", "shaders/meshviewer.frag.spv", "");
		auto vertShader = Shader::ReflectedModule("shaders/meshviewer.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		auto fragShader = Shader::ReflectedModule("shaders/meshviewer.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		//assert(vertShader.reflectData.bindingDescription.stride == sizeof(Renderer::GSVertexUprocessed));

		gPipeline.AddBindings(Renderer::EBindingStage::Vertex, vertShader.reflectData);
		gPipeline.AddBindings(Renderer::EBindingStage::Fragment, fragShader.reflectData);
		gPipeline.CreateDescriptorSetLayouts();
		gPipeline.CreateLayout();

		gPipeline.CreateDescriptorPool();
		gPipeline.CreateDescriptorSets();

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
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = gPipeline.layout;
		pipelineInfo.renderPass = gRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &gPipeline.pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}
	PS2::FrameVertexBuffers<Renderer::GSVertexUnprocessed, uint16_t> gVertexBuffers;

	void Setup() 
	{
		CreateRenderPass();
		CreateFramebuffer();
		Renderer::CreateCommandBuffers(gCommandBuffers);
		CreatePipeline();

		gVertexBuffers.Init(0x678, 0x2000);

		gVertexConstantBuffer.Init();
	}

	void Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent) {
		const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd, &beginInfo);

		Renderer::Debug::BeginLabel(cmd, "Mesh Viewer");

		VkExtent2D extentTemp = { gWidth, gHeight };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = gRenderPass;
		renderPassInfo.framebuffer = gFrameBuffer.framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extentTemp;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

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

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gPipeline.pipeline);

		auto& bufferData = gVertexBuffers.GetBufferData();

		edNODE* pNode = DebugMenu_Internal::pNodeTest;

		if (pNode) {

			ed_3d_hierarchy* p3dHier = (ed_3d_hierarchy*)pNode->pData;
			ed3DLod* pLod = ed3DChooseGoodLOD(p3dHier);
			ed_hash_code* pObjHash = (ed_hash_code*)LOAD_SECTION(pLod->pObj);
			MeshData_OBJ* pMeshOBJ = (MeshData_OBJ*)LOAD_SECTION(pObjHash->pData);

			ed_3D_Scene* p3dScene = DebugMenu_Internal::pSceneTest;
			ed_3d_strip* p3dStrip = (ed_3d_strip*)LOAD_SECTION(pMeshOBJ->body.p3DStrip);

			if (p3dStrip && p3dHier && p3dHier->pAnimMatrix) {
				ed3DInitRenderEnvironement(p3dScene, 0);

				std::vector<ed_3d_strip*> strips;

				for (int stripIndex = 0; stripIndex < pMeshOBJ->body.stripCount; stripIndex++) {
					strips.push_back(p3dStrip);
					p3dStrip = (ed_3d_strip*)LOAD_SECTION(p3dStrip->pNext);
				}

				std::reverse(strips.begin(), strips.end());

				for(auto& pCurrentStrip : strips) {

					char* pVifList = ((char*)pCurrentStrip) + pCurrentStrip->vifListOffset;

					short* pAnimIndexes = (short*)((char*)pCurrentStrip + pCurrentStrip->vifListOffset + -0x30);

					VertexConstantBuffer& vertexConstantBuffer = gVertexConstantBuffer.GetBufferData();

					uint incPacketSize = ed3DFlushStripGetIncPacket(pCurrentStrip, 0, 0);
					uint partialMeshSectionCount = (uint)(ushort)pCurrentStrip->meshCount % 3;
					ushort fullMeshSectionCount = (ushort)pCurrentStrip->meshCount - partialMeshSectionCount;
					uint mode = 1;
					uint maxAnimMatrixCount = 0;

					ScratchPadRenderInfo renderInfo;
					renderInfo.pSharedMeshTransform = &p3dHier->transformB;
					renderInfo.biggerScale = ed3DMatrixGetBigerScale(&p3dHier->transformB);
					auto pDma = ed3DListCreateDmaMatrixNode(&renderInfo, p3dHier);
					ed3DPKTAddMatrixPacket(gPkt, pDma);
					edF32MATRIX4* pObjToScreen = SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_SCREEN_MATRIX, edF32MATRIX4*);
					edF32MATRIX4* pObjToCamera = SCRATCHPAD_ADDRESS_TYPE(OBJECT_TO_CAMERA_MATRIX_SPR, edF32MATRIX4*);

					vertexConstantBuffer.objToScreen = *pObjToScreen;
					vertexConstantBuffer.worldToCamera = *WorldToCamera_Matrix;
					vertexConstantBuffer.objectToCamera = *pObjToCamera;

					if ((pCurrentStrip->flags & 0x8000000) == 0) {
						maxAnimMatrixCount = 0x18;
					}
					else {
						if (mode < 2) {
							maxAnimMatrixCount = 9;
						}
						else {
							maxAnimMatrixCount = 7;
						}
					}

					bool bVar1;
					int animMatrixIndex = 0;

					// Push anim matrices.
					for (; (bVar1 = maxAnimMatrixCount != 0, maxAnimMatrixCount = maxAnimMatrixCount + -1, bVar1 && (-1 < *pAnimIndexes)); pAnimIndexes = pAnimIndexes + 1)
					{
						vertexConstantBuffer.animMatrices[animMatrixIndex] = p3dHier->pAnimMatrix[*pAnimIndexes];
						animMatrixIndex++;
					}

					auto AddVertices = [&bufferData]() {
						char* vtxStart = VU1Emu::GetVertexDataStart();

						Gif_Tag gifTag;
						gifTag.setTag((u8*)vtxStart, true);

						vtxStart += 0x10;

						for (int i = 0; i < gifTag.nLoop; i++) {
							Renderer::GSVertexUnprocessed vtx;
							memcpy(&vtx.STQ, vtxStart, sizeof(vtx.STQ));
							memcpy(&vtx.RGBA, vtxStart + 0x10, sizeof(vtx.RGBA));
							memcpy(&vtx.XYZSkip, vtxStart + 0x20, sizeof(vtx.XYZSkip));

							const uint primReg = gifTag.tag.PRIM;
							const GIFReg::GSPrimPacked primPacked = *reinterpret_cast<const GIFReg::GSPrimPacked*>(&primReg);

							Renderer::KickVertex(vtx, primPacked, vtx.XYZSkip[3] & 0x8000, gVertexBuffers.GetBufferData());

							vtxStart += 0x30;
						};
					};

					if ((pCurrentStrip->flags & 4) == 0) {
						while (bVar1 = partialMeshSectionCount != 0, partialMeshSectionCount = partialMeshSectionCount - 1, bVar1) {
							VU1Emu::ProcessVifList((edpkt_data*)pVifList, false);
							AddVertices();
							pVifList = pVifList + incPacketSize * 0x10;
						}

						for (; fullMeshSectionCount != 0; fullMeshSectionCount = fullMeshSectionCount + -3) {

							char* pVifListB = pVifList + incPacketSize * 0x10;
							char* pVifListC = pVifList + incPacketSize * 0x20;

							VU1Emu::ProcessVifList((edpkt_data*)pVifList, false);
							AddVertices();
							VU1Emu::ProcessVifList((edpkt_data*)pVifListB, false);
							AddVertices();
							VU1Emu::ProcessVifList((edpkt_data*)pVifListC, false);
							AddVertices();

							pVifList = pVifList + incPacketSize * 0x30;
						}
					}
				}
			}
		}

		if (bufferData.index.tail > 0) {

			gVertexBuffers.BindData(cmd);
			gVertexConstantBuffer.Update(GetCurrentFrame());

			const VkDescriptorBufferInfo vertexDescBufferInfo = gVertexConstantBuffer.GetDescBufferInfo(GetCurrentFrame());

			Renderer::DescriptorWriteList writeList;
			writeList.EmplaceWrite({ Renderer::EBindingStage::Vertex, &vertexDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });

			std::vector<VkWriteDescriptorSet> descriptorWrites = writeList.CreateWriteDescriptorSetList(gPipeline.descriptorSets[GetCurrentFrame()], gPipeline.descriptorSetLayoutBindings);

			if (descriptorWrites.size() > 0) {
				vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gPipeline.layout, 0, 1, &gPipeline.descriptorSets[GetCurrentFrame()], 0, nullptr);

			vkCmdDrawIndexed(cmd, static_cast<uint32_t>(bufferData.index.tail), 1, 0, 0, 0);
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

		gVertexBuffers.Reset();		
		bufferData.ResetAfterDraw();
	}
}

void DebugMenu::SetupRenderer()
{
	DebugRendererMeshViewer::Setup();
	Renderer::GetRenderDelegate() += DebugRendererMeshViewer::Render;

	DebugRendererImgui::Setup();
	Renderer::GetRenderDelegate() += DebugRendererImgui::Render;
}

ImTextureID DebugMenu::AddTexture(const PS2::GSTexImage& texImage)
{
	return ImGui_ImplVulkan_AddTexture(texImage.sampler, texImage.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

struct DebugFrameBuffer {
	DebugFrameBuffer(ImTextureID inTexID, VkSampler inSampler)
		: texID(inTexID)
		, sampler(inSampler)
	{}

	DebugFrameBuffer()
		: texID(nullptr)
		, sampler(VK_NULL_HANDLE)
	{}

	ImTextureID texID;
	VkSampler sampler;
};

std::unordered_map<int, DebugFrameBuffer> gDebugFrameBuffers;

ImTextureID DebugMenu::AddFrameBuffer(const PS2::FrameBuffer& frameBuffer)
{
	if (gDebugFrameBuffers.find(frameBuffer.FBP) == gDebugFrameBuffers.end()) {
		// Create sampler
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;

		VkSampler sampler;
		VkResult result = vkCreateSampler(GetDevice(), &samplerCreateInfo, nullptr, &sampler);
		if (result != VK_SUCCESS) {
			// Handle sampler creation failure
		}

		gDebugFrameBuffers.emplace(frameBuffer.FBP, DebugFrameBuffer(ImGui_ImplVulkan_AddTexture(sampler, frameBuffer.finalImageView, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL), sampler));
	}

	return gDebugFrameBuffers[frameBuffer.FBP].texID;
}