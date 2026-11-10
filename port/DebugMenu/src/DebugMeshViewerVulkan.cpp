#include "DebugMeshViewerVulkan.h"
#include "DebugMeshViewer.h"
#include "DrawTrace.h"
#include "Objects/UniformBuffer.h"
#include "Objects/Pipeline.h"
#include "Native/NativeRenderer.h"
#include "Native/NativeRendererInternal.h"
#include "renderer_debug.h"
#include "Native/Blending.h"
#include "Types.h"
#include "Objects/FrameBuffer.h"
#include "backends/imgui_impl_vulkan.h"

#include "profiling.h"

#include <array>
#include <optional>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include "Texture/TextureCache.h"

namespace DebugMeshViewer {
	namespace Vulkan {
		using namespace Renderer::Native;

		PipelineMap gPipelines;
		Renderer::FrameBufferBase gFrameBuffer;

		VkSampler gFrameBufferSampler;

		VkRenderPass gRenderPass;
		Renderer::CommandBufferVector gCommandBuffers;

		UniformBuffer<VertexConstantBuffer> gVertexConstantBuffer;

		constexpr int maxDrawCommands = 128;

		using PreviewerVertexBuffer = PS2::FrameVertexBuffers<Renderer::GSVertexUnprocessedNormal, uint16_t>;
		using PreviewerDrawCommandArray = std::array<std::pair<Renderer::SimpleTexture*, PreviewerVertexBuffer>, maxDrawCommands>;
		PreviewerDrawCommandArray gPreviewerDrawCommands;
		std::array<std::string, maxDrawCommands> gPreviewerMeshNames;
		std::array<Renderer::SimpleMesh*, maxDrawCommands> gPreviewerMeshes{};
		struct DrawState {
			Renderer::DrawTrace::Submission submission;
			Renderer::DrawTrace::Draw draw;
		};
		std::array<std::optional<DrawState>, maxDrawCommands> gPreviewerDrawStates;

		Renderer::SimpleTexture* pBoundTexture = nullptr;
		int drawCounter = 0;		
		bool screenshotAvailable = false;

		void CreateFramebuffer()
		{
			gFrameBuffer.SetupBase({ gWidth, gHeight }, gRenderPass, true);
		}		

		void CreatePipelineGlsl()
		{
			PipelineKey key; 
			key.options.bGlsl = true;
			key.options.bWireframe = false;
			key.options.topology = topologyTriangleList;
			PipelineCreateInfo<PipelineKey> createInfo { "shaders/meshviewer.vert.spv" , "shaders/native.frag.spv", "", key};
			CreatePipeline(createInfo, gRenderPass, gPipelines[createInfo.key.key], "Mesh Previewer GLSL");

			createInfo.key.options.bWireframe = true;
			CreatePipeline(createInfo, gRenderPass, gPipelines[createInfo.key.key], "Mesh Previewer GLSL Wireframe");
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

		void InitializeFrameDescriptors(Renderer::Pipeline& pipeline);

		Renderer::Pipeline& GetPipeline(const ResolvedBlendState& blend) {
			PipelineKey pipelineKey;
			pipelineKey.options.bWireframe = GetWireframe();
			pipelineKey.options.bGlsl = true;
			pipelineKey.options.topology = topologyTriangleList;

			const uint32_t key = pipelineKey.key | (uint32_t(blend.blendIndex) << 8) |
				(uint32_t(blend.colorBlendAttachment.blendEnable != 0) << 16);

			if (gPipelines.find(key) == gPipelines.end()) {
				GraphicsPipelineState state;
				const auto& attachment = blend.colorBlendAttachment;
				state.blendEnable = attachment.blendEnable;
				state.srcColorBlendFactor = attachment.srcColorBlendFactor;
				state.dstColorBlendFactor = attachment.dstColorBlendFactor;
				state.colorBlendOp = attachment.colorBlendOp;
				state.srcAlphaBlendFactor = attachment.srcAlphaBlendFactor;
				state.dstAlphaBlendFactor = attachment.dstAlphaBlendFactor;
				state.alphaBlendOp = attachment.alphaBlendOp;
				PipelineCreateInfo<PipelineKey> info{ "shaders/meshviewer.vert.spv", "shaders/native.frag.spv", "", pipelineKey };
				CreatePipeline(info, gRenderPass, gPipelines[key], "Mesh Previewer material", state);
				InitializeFrameDescriptors(gPipelines[key]);
			}

			return gPipelines[key];
		}

		void InitializeFrameDescriptors(Renderer::Pipeline& pipeline)
		{
			for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; ++frame) {
				const auto buffer = gVertexConstantBuffer.GetDescBufferInfo(frame);
				Renderer::DescriptorWriteList writes;
				writes.EmplaceWrite({ 0, Renderer::EBindingStage::Vertex, &buffer, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
				const auto updates = writes.CreateWriteDescriptorSetList(pipeline.descriptorSets[frame], pipeline.descriptorSetLayoutBindings);
				vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(updates.size()), updates.data(), 0, nullptr);
			}
		}
	} // Vulkan

	VertexConstantBuffer& GetVertexConstantBuffer() {
		return Vulkan::gVertexConstantBuffer.GetBufferData();
	}

	VkImage GetScreenshotImage()
	{
		return Vulkan::screenshotAvailable ? Vulkan::gFrameBuffer.colorImage : VK_NULL_HANDLE;
	}

	Renderer::NativeVertexBufferData& AddPreviewerDrawCommand(Renderer::SimpleTexture* pTexture, Renderer::SimpleMesh* pMesh)
	{
		assert(Vulkan::drawCounter < Vulkan::maxDrawCommands);

		Vulkan::pBoundTexture = pTexture;

		Vulkan::gVertexConstantBuffer.Map(GetCurrentFrame());

		Vulkan::gPreviewerDrawCommands[Vulkan::drawCounter].first = pTexture;
		Vulkan::gPreviewerMeshNames[Vulkan::drawCounter] = pMesh->GetName();
		Vulkan::gPreviewerMeshes[Vulkan::drawCounter] = pMesh;
		Vulkan::gPreviewerDrawStates[Vulkan::drawCounter].reset();

		return Vulkan::gPreviewerDrawCommands[Vulkan::drawCounter++].second.GetDrawBufferData();
	}

	void SetPreviewerDrawState(const Renderer::DrawTrace::Submission& submission, const Renderer::DrawTrace::Draw& draw)
	{
		assert(Vulkan::drawCounter > 0);
		Vulkan::gPreviewerDrawStates[Vulkan::drawCounter - 1] = Vulkan::DrawState{submission, draw};
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

	std::string ExportPreviewerMeshes()
	{
		// Read the same decoded buffers queued for this preview frame. Keep raw
		// object coordinates so camera/gizmo transforms cannot obscure the axes.
		size_t totalVertices = 0;
		size_t totalFaces = 0;
		for (int i = 0; i < Vulkan::drawCounter; ++i) {
			auto& data = Vulkan::gPreviewerDrawCommands[i].second.GetDrawBufferData();
			if (data.index.tail == 0) continue;
			if ((data.index.tail % 3) != 0) return "Export failed: preview indices are not triangles.";
			for (size_t j = 0; j < data.vertex.tail; ++j) {
				for (int axis = 0; axis < 3; ++axis) {
					if (!std::isfinite(data.vertex.buff[j].XYZFlags.fXYZ[axis])) return "Export failed: non-finite vertex coordinate.";
				}
			}
			for (size_t j = 0; j < data.index.tail; ++j) {
				if (data.index.buff[j] >= data.vertex.tail) return "Export failed: vertex index is out of range.";
			}
			totalVertices += data.vertex.tail;
			totalFaces += data.index.tail / 3;
		}
		if (totalFaces == 0) return "Export failed: no preview triangles are queued.";

		std::error_code error;
		const auto directory = std::filesystem::absolute("logs/mesh-exports", error);
		if (error) return "Export failed: " + error.message();
		std::filesystem::create_directories(directory, error);
		if (error) return "Export failed: " + error.message();
		const auto stamp = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		const auto base = directory / ("preview_" + std::to_string(stamp));
		const auto objPath = std::filesystem::path(base.string() + ".obj");
		const auto infoPath = std::filesystem::path(base.string() + ".txt");
		std::ofstream obj(objPath);
		std::ofstream info(infoPath);
		if (!obj || !info) return "Export failed: could not open output files in " + directory.string();
		obj.imbue(std::locale::classic());
		info.imbue(std::locale::classic());
		obj << std::setprecision(std::numeric_limits<float>::max_digits10);
		info << std::setprecision(std::numeric_limits<float>::max_digits10);
		obj << "# Kya mesh preview: raw object-space XYZ, triangle-list faces.\n"
			<< "# No animation, actor, gizmo, camera, or axis conversion applied.\n";
		info << "Raw object-space geometry; units and XYZ axes preserved.\n"
			<< "No textures or transforms exported. Skinned meshes retain unposed vertex coordinates.\n"
			<< "Meshes queued in the preview are separate OBJ groups.\n"
			<< "Vertices: " << totalVertices << "\nTriangles: " << totalFaces << "\n";
		size_t vertexOffset = 0;
		for (int i = 0; i < Vulkan::drawCounter; ++i) {
			auto& data = Vulkan::gPreviewerDrawCommands[i].second.GetDrawBufferData();
			if (data.index.tail == 0) continue;
			const std::string& name = Vulkan::gPreviewerMeshNames[i];
			// Mesh names are comments/metadata, not filenames or OBJ identifiers.
			obj << "\ng preview_" << i << "\n";
			info << "\nGroup preview_" << i << "\nMesh: " << name
				<< "\nVertices: " << data.vertex.tail << "\nTriangles: " << data.index.tail / 3 << "\n";
			float minimum[3] = { INFINITY, INFINITY, INFINITY };
			float maximum[3] = { -INFINITY, -INFINITY, -INFINITY };
			size_t animatedVertices = 0;
			for (size_t j = 0; j < data.vertex.tail; ++j) {
				const auto& vertex = data.vertex.buff[j];
				const float* xyz = vertex.XYZFlags.fXYZ;
				obj << "v " << xyz[0] << ' ' << xyz[1] << ' ' << xyz[2] << '\n';
				for (int axis = 0; axis < 3; ++axis) {
					minimum[axis] = std::min(minimum[axis], xyz[axis]);
					maximum[axis] = std::max(maximum[axis], xyz[axis]);
				}
				if ((vertex.XYZFlags.flags & 0x7ff) != 0) ++animatedVertices;
			}
			for (size_t j = 0; j < data.index.tail; j += 3) {
				obj << "f " << vertexOffset + data.index.buff[j] + 1 << ' '
					<< vertexOffset + data.index.buff[j + 1] + 1 << ' '
					<< vertexOffset + data.index.buff[j + 2] + 1 << '\n';
			}
			info << "Minimum XYZ: " << minimum[0] << ' ' << minimum[1] << ' ' << minimum[2]
				<< "\nMaximum XYZ: " << maximum[0] << ' ' << maximum[1] << ' ' << maximum[2]
				<< "\nVertices with animation references: " << animatedVertices << '\n';
			vertexOffset += data.vertex.tail;
		}
		obj.close();
		info.close();
		if (!obj || !info) return "Export failed while writing " + base.string();
		return "Exported " + objPath.string() + " (mesh names and bounds in matching .txt)";
	}

	static void CreateRenderPass(VkRenderPass& renderPass, const char* name)
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
} // DebugMeshViewer

void DebugMeshViewer::Vulkan::Setup()
{
	CreateRenderPass(gRenderPass, "Mesh Previewer");
	CreateFramebuffer();
	CreateFramebufferSampler();
	Renderer::CreateCommandBuffers(gCommandBuffers);
	CreatePipelineGlsl();

	gVertexConstantBuffer.Init();
	for (auto& [key, pipeline] : gPipelines) InitializeFrameDescriptors(pipeline);

	for (auto& drawBuffer : gPreviewerDrawCommands) {
		drawBuffer.second.Init(0x12000, 0x12000);
	}

	OnFrameBufferCreated((ImTextureID)(uintptr_t)ImGui_ImplVulkan_AddTexture(gFrameBufferSampler, gFrameBuffer.colorImageView, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL));
}

void DebugMeshViewer::Vulkan::Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent, Renderer::CommandBufferList& commandBufferList)
{
	ZONE_SCOPED;

	if (!pBoundTexture) {
		screenshotAvailable = false;
		return;
	}

	std::vector<Renderer::SimpleMesh*> selection;
	glm::vec3 minimum(INFINITY), maximum(-INFINITY);
	for (int i = 0; i < drawCounter; ++i) {
		selection.push_back(gPreviewerMeshes[i]);
		const auto& data = gPreviewerDrawCommands[i].second.GetDrawBufferData();

		for (size_t j = 0; j < data.vertex.tail; ++j) {
			const auto* xyz = data.vertex.buff[j].XYZFlags.fXYZ;
			const glm::vec3 position(xyz[0], xyz[1], xyz[2]);
			if (!std::isfinite(xyz[0]) || !std::isfinite(xyz[1]) || !std::isfinite(xyz[2])) continue;
			minimum = glm::min(minimum, position);
			maximum = glm::max(maximum, position);
		}
	}

	static std::vector<Renderer::SimpleMesh*> previousSelection;

	if (std::isfinite(minimum.x)) {
		UpdateCamera(&minimum.x, &maximum.x, selection != previousSelection);
	}
	
	previousSelection = std::move(selection);

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
	clearColors[1] = { {0.0f, 0.0f } };
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

	vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);

	vkCmdSetDepthWriteEnable(cmd, true);
	vkCmdSetDepthTestEnable(cmd, VK_TRUE);
	vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);

	const VkBool32 colorWriteEnable = VK_TRUE;

	static auto pvkCmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)vkGetInstanceProcAddr(GetInstance(), "vkCmdSetColorWriteEnableEXT");
	assert(pvkCmdSetColorWriteEnableEXT);
	pvkCmdSetColorWriteEnableEXT(cmd, 1, &colorWriteEnable);

	std::array<VkBool32, 1> colorWriteMasks = {
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	static auto pvkCmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)vkGetInstanceProcAddr(GetInstance(), "vkCmdSetColorWriteMaskEXT");
	assert(pvkCmdSetColorWriteMaskEXT);
	pvkCmdSetColorWriteMaskEXT(cmd, 0, colorWriteMasks.size(), colorWriteMasks.data());

	gVertexConstantBuffer.Map(GetCurrentFrame());

	for (auto& drawCommand : gPreviewerDrawCommands) {
		Renderer::SimpleTexture* pTexture = drawCommand.first;

		if (!pTexture) {
			break;
		}

		const auto drawIndex = &drawCommand - gPreviewerDrawCommands.data();
		auto* mesh = gPreviewerMeshes[drawIndex];
		const auto& captured = gPreviewerDrawStates[drawIndex];
		auto alpha = pTexture->GetTextureRegisters().alpha;
		if (captured) {
			alpha.CMD = captured->draw.alpha;
		}

		const auto blend = ResolveBlendState(alpha, captured ? captured->draw.blend : mesh->GetPrim().ABE);
		auto& pipeline = GetPipeline(blend);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
		SetBlendConstants(alpha, cmd);

		PreviewerVertexBuffer& vertexBuffer = drawCommand.second;

		auto& bufferData = vertexBuffer.GetDrawBufferData();

		if (bufferData.index.tail > 0) {
			vertexBuffer.MapAndBindData(cmd);

			const auto& registers = pTexture->GetTextureRegisters();
			const auto sampling = PS2::ResolveTextureSampling(registers.clamp, registers.tex1, pTexture->GetRenderer()->mipLevels);
			const std::array sets{ pipeline.descriptorSets[GetCurrentFrame()],
				pTexture->GetRenderer()->GetOrCreateTextureBinding(sampling.sampler) };
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);

			Draw draw;
			Draw::Instance instance;
			instance.pMesh = mesh;
			instance.indexCount = static_cast<int>(bufferData.index.tail);
			instance.gsTest = registers.test;

			if (captured) {
				instance.gsTest.CMD = captured->draw.test;
				instance.gsTest.ZTE = captured->draw.depthTest;
				instance.gsTest.ZTST = captured->draw.depthMode;
				instance.bIsZMask = !captured->draw.depthWrite;
			}

			auto& data = instance.perDrawData;
			data.alphaEnable = registers.test.ATE;
			data.alphaAtst = registers.test.ATST;
			data.alphaAref = registers.test.AREF;
			data.alphaAfail = registers.test.AFAIL;
			data.stripFlags = mesh->GetStripFlags();
			data.blendMode = blend.hwBlendMode;
			data.textureLodBias = sampling.lod.lodBias;
			data.textureLodScale = sampling.lod.lodScale;
			data.textureFixedLod = sampling.lod.fixedLod;
			data.textureMaxMipLevel = sampling.lod.maxLevel;
			data.textureLodEnable = 1;

			if (captured) {
				data.renderFlags = captured->submission.flags;
				data.globalAlpha = captured->submission.globalAlpha;
				data.alphaEnable = captured->draw.alphaTest;
				data.alphaAtst = captured->draw.alphaAtst;
				data.alphaAref = captured->draw.alphaAref;
				data.alphaAfail = captured->draw.alphaAfail;
				draw.bIsAfailZOnly = captured->draw.zOnly;
			}

			SetColorDepthDynamicState(cmd, draw, instance);
			RecordAlphaTestedDraw(cmd, pipeline.layout, draw, instance, data);
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
	screenshotAvailable = std::isfinite(minimum.x);

	for (auto& drawCommand : gPreviewerDrawCommands) {
		drawCommand.first = nullptr;
		drawCommand.second.Reset();
		drawCommand.second.GetDrawBufferData().ResetAfterDraw();
	}

	pBoundTexture = nullptr;
	drawCounter = 0;
}
