#include "NativeDebugShapes.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <vector>

#include "Objects/VulkanBuffer.h"
#include "Objects/VulkanImage.h"
#include "Objects/VulkanRenderPass.h"
#include "VulkanRenderer.h"
#include "renderer.h"
#include "NativeDebug.h"
#include "glm/glm.hpp"

namespace Renderer
{
	namespace Native
	{
		namespace DebugShapes
		{
			struct DebugLinePushConstant
			{
				glm::mat4 projXView;
			};

			struct DebugLineVertex
			{
				glm::vec4 position;
				glm::vec4 color;
			};

			constexpr uint32_t gMaxDebugLineVertexCount = 8192*2*16;
			static std::array<DebugLineVertex, gMaxDebugLineVertexCount> gDebugLineVertices;
			static uint32_t gDebugLineVertexCount = 0;

			static VkBuffer gDebugLineVertexBuffer = VK_NULL_HANDLE;
			static VkDeviceMemory gDebugLineVertexBufferMemory = VK_NULL_HANDLE;

			static glm::mat4 gInitialViewMatrix = glm::mat4(1.0f);
			static glm::mat4 gInitialProjMatrix = glm::mat4(1.0f);
			static glm::mat4 gLastModelMatrix = glm::mat4(1.0f);
			static bool gHasLastModelMatrix = false;
			static bool gHasInitialViewProjection = false;

			// Dedicated end-of-frame debug render pass resources.
			static OwnedImage         gSavedDepth          = {};
			static VkRenderPass       gDebugRenderPass     = VK_NULL_HANDLE;
			static VkFramebuffer      gDebugFramebuffer    = VK_NULL_HANDLE;
			static Renderer::Pipeline gDebugPipeline;
			static int                gDedicatedWidth      = 0;
			static int                gDedicatedHeight     = 0;
			static bool               gHasSavedDepth       = false;

			static void PushLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
			{
				if (gDebugLineVertexCount + 2 > gMaxDebugLineVertexCount) {
					return;
				}
				gDebugLineVertices[gDebugLineVertexCount++] = { glm::vec4(start, 1.0f), color };
				gDebugLineVertices[gDebugLineVertexCount++] = { glm::vec4(end, 1.0f), color };
			}

			static glm::vec3 GetDebugShapeWorldPosition()
			{
				if (gHasLastModelMatrix) {
					return glm::vec3(gLastModelMatrix[3]);
				}

				const glm::mat4 inverseView = glm::inverse(gInitialViewMatrix);
				const glm::vec3 cameraPosition = glm::vec3(inverseView[3]);

				glm::vec3 cameraForward = -glm::vec3(inverseView[2]);
				const float forwardLength = glm::length(cameraForward);

				if (forwardLength > 0.0f) {
					cameraForward /= forwardLength;
				}
				else {
					cameraForward = glm::vec3(0.0f, 0.0f, 1.0f);
				}

				return cameraPosition + (cameraForward * 100.0f);
			}

			static void BuildShapeLines()
			{
				if (!gHasInitialViewProjection) {
					return;
				}

				const glm::vec3 center = GetDebugShapeWorldPosition();
				constexpr float axisHalfLength = 25.0f;

				PushLine(center + glm::vec3(-axisHalfLength, 0.0f, 0.0f), center + glm::vec3(axisHalfLength, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
				PushLine(center + glm::vec3(0.0f, -axisHalfLength, 0.0f), center + glm::vec3(0.0f, axisHalfLength, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				PushLine(center + glm::vec3(0.0f, 0.0f, -axisHalfLength), center + glm::vec3(0.0f, 0.0f, axisHalfLength), glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));
			}

			static void UploadLineVertices()
			{
				if (gDebugLineVertexCount == 0) {
					return;
				}

				void* pData = nullptr;
				const VkDeviceSize dataSize = sizeof(DebugLineVertex) * gDebugLineVertexCount;
				vkMapMemory(GetDevice(), gDebugLineVertexBufferMemory, 0, dataSize, 0, &pData);
				memcpy(pData, gDebugLineVertices.data(), static_cast<size_t>(dataSize));
				vkUnmapMemory(GetDevice(), gDebugLineVertexBufferMemory);
			}

			void Setup()
			{
				const VkDeviceSize bufferSize = sizeof(DebugLineVertex) * gMaxDebugLineVertexCount;

				CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					gDebugLineVertexBuffer,
					gDebugLineVertexBufferMemory);

				SetObjectName(reinterpret_cast<uint64_t>(gDebugLineVertexBuffer), VK_OBJECT_TYPE_BUFFER, "Native Debug Line Vertex Buffer");
				SetObjectName(reinterpret_cast<uint64_t>(gDebugLineVertexBufferMemory), VK_OBJECT_TYPE_DEVICE_MEMORY, "Native Debug Line Vertex Buffer Memory");
			}

			void ResetFrame()
			{
				gHasLastModelMatrix = false;
				gHasInitialViewProjection = false;
				gDebugLineVertexCount = 0;
				gHasSavedDepth = false;
			}

			void SetInitialViewProjection(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
			{
				gInitialViewMatrix = viewMatrix;
				gInitialProjMatrix = projMatrix;
				gHasInitialViewProjection = true;
			}

			void SetLastModelMatrix(const glm::mat4& modelMatrix)
			{
				gLastModelMatrix = modelMatrix;
				gHasLastModelMatrix = true;
			}

			void AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
			{
				PushLine(start, end, color);
			}

			void AddLine(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float a)
			{
				AddLine(glm::vec3(sx, sy, sz), glm::vec3(ex, ey, ez), glm::vec4(r, g, b, a));
			}

			void AddOBB(const float* mat, float hw, float hh, float hd, float r, float g, float b, float a)
			{
				// mat layout: rowX[0..3], rowY[4..7], rowZ[8..11], rowT[12..15]
				const glm::vec3 axisX(mat[0], mat[1], mat[2]);
				const glm::vec3 axisY(mat[4], mat[5], mat[6]);
				const glm::vec3 axisZ(mat[8], mat[9], mat[10]);
				const glm::vec3 center(mat[12], mat[13], mat[14]);

				const glm::vec3 ex = axisX * hw;
				const glm::vec3 ey = axisY * hh;
				const glm::vec3 ez = axisZ * hd;

				const glm::vec3 corners[8] = {
					center - ex - ey - ez,
					center + ex - ey - ez,
					center + ex + ey - ez,
					center - ex + ey - ez,
					center - ex - ey + ez,
					center + ex - ey + ez,
					center + ex + ey + ez,
					center - ex + ey + ez,
				};

				const glm::vec4 color(r, g, b, a);
				// Bottom face
				PushLine(corners[0], corners[1], color);
				PushLine(corners[1], corners[2], color);
				PushLine(corners[2], corners[3], color);
				PushLine(corners[3], corners[0], color);
				// Top face
				PushLine(corners[4], corners[5], color);
				PushLine(corners[5], corners[6], color);
				PushLine(corners[6], corners[7], color);
				PushLine(corners[7], corners[4], color);
				// Vertical edges
				PushLine(corners[0], corners[4], color);
				PushLine(corners[1], corners[5], color);
				PushLine(corners[2], corners[6], color);
				PushLine(corners[3], corners[7], color);
			}

			uint32_t GetLineCount()
			{
				return gDebugLineVertexCount / 2;
			}

			void AddSphere(const glm::vec3& center, float radius, const glm::vec4& color)
			{
				constexpr int kSegments = 32;
				constexpr float kStep = 2.0f * 3.14159265358979323846f / kSegments;

				// Draw 3 great circles: XY, XZ, YZ planes
				for (int i = 0; i < kSegments; ++i) {
					const float a0 = kStep * i;
					const float a1 = kStep * (i + 1);
					const float c0 = std::cos(a0), s0 = std::sin(a0);
					const float c1 = std::cos(a1), s1 = std::sin(a1);

					// XY plane
					PushLine(center + glm::vec3(radius * c0, radius * s0, 0.0f),
					         center + glm::vec3(radius * c1, radius * s1, 0.0f), color);
					// XZ plane
					PushLine(center + glm::vec3(radius * c0, 0.0f, radius * s0),
					         center + glm::vec3(radius * c1, 0.0f, radius * s1), color);
					// YZ plane
					PushLine(center + glm::vec3(0.0f, radius * c0, radius * s0),
					         center + glm::vec3(0.0f, radius * c1, radius * s1), color);
				}
			}

			void AddSphere(float cx, float cy, float cz, float radius, float r, float g, float b, float a)
			{
				AddSphere(glm::vec3(cx, cy, cz), radius, glm::vec4(r, g, b, a));
			}

			void CreatePipeline(const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name)
			{
				pipeline.debugName = name;

				auto vertShader = Shader::ReflectedModule("shaders/native_debug_line.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
				auto fragShader = Shader::ReflectedModule("shaders/native_debug_line.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

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

				bindingDescription.stride = sizeof(DebugLineVertex);

				vertexInputInfo.vertexBindingDescriptionCount = 1;
				vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
				vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
				vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

				VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
				inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
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
				};
				VkPipelineDynamicStateCreateInfo dynamicState{};
				dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
				dynamicState.pDynamicStates = dynamicStates.data();

				VkPipelineDepthStencilStateCreateInfo depthState{};
				depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				depthState.depthTestEnable = VK_TRUE;
				depthState.depthWriteEnable = VK_FALSE;
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
					throw std::runtime_error("failed to create debug line graphics pipeline!");
				}

				SetObjectName(reinterpret_cast<uint64_t>(pipeline.pipeline), VK_OBJECT_TYPE_PIPELINE, name);
			}

			void Record(const VkCommandBuffer& cmd, const Renderer::Pipeline& pipeline)
			{
				BuildShapeLines();

				if (gDebugLineVertexCount == 0) {
					return;
				}

				UploadLineVertices();

				Renderer::Debug::BeginLabel(cmd, "Native Debug Shapes");
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

				DebugLinePushConstant pushConstantData;
				pushConstantData.projXView = gInitialProjMatrix * gInitialViewMatrix;
				vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DebugLinePushConstant), &pushConstantData);

				const VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(cmd, 0, 1, &gDebugLineVertexBuffer, offsets);
				vkCmdDraw(cmd, gDebugLineVertexCount, 1, 0, 0);

				Renderer::Debug::EndLabel(cmd);
			}

			void SetupDedicatedPass(VkImageView colorImageView, int width, int height)
			{
				gDedicatedWidth  = width;
				gDedicatedHeight = height;

				// Saved depth image (dimension-dependent, recreated on resize).
				gSavedDepth = VulkanImage::CreateDepth(width, height);
				SetObjectName(reinterpret_cast<uint64_t>(gSavedDepth.image), VK_OBJECT_TYPE_IMAGE,      "Debug Shapes Saved Depth Image");
				SetObjectName(reinterpret_cast<uint64_t>(gSavedDepth.view),  VK_OBJECT_TYPE_IMAGE_VIEW, "Debug Shapes Saved Depth Image View");

				// Render pass and pipeline are dimension-independent; only create once.
				if (gDebugRenderPass == VK_NULL_HANDLE)
				{
					const Renderer::AttachmentInfo colorInfo{
						GetSwapchainImageFormat(),
						VK_ATTACHMENT_LOAD_OP_LOAD,
						VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
						VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
					};

					// Depth loaded read-only from the saved first-pass copy; no writes.
					const Renderer::AttachmentInfo depthInfo{
						VK_FORMAT_D32_SFLOAT_S8_UINT,
						VK_ATTACHMENT_LOAD_OP_LOAD,
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
						VK_ATTACHMENT_STORE_OP_DONT_CARE,
					};

					const VkSubpassDependency dependency{
						VK_SUBPASS_EXTERNAL, 0,
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
						0,
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
						0
					};

					gDebugRenderPass = Renderer::CreateRenderPass2D(
						{ &colorInfo, 1 }, depthInfo, { &dependency, 1 },
						"Debug Shapes Render Pass");

					CreatePipeline(gDebugRenderPass, gDebugPipeline, "Debug Shapes Dedicated Pipeline");
				}

				// Framebuffer (dimension-dependent, recreated on resize).
				const std::array<VkImageView, 2> imageViews = { colorImageView, gSavedDepth.view };
				gDebugFramebuffer = Renderer::CreateFramebuffer(
					gDebugRenderPass, imageViews,
					static_cast<uint32_t>(width), static_cast<uint32_t>(height),
					"Debug Shapes Framebuffer");
			}

			void DestroyDedicatedPass()
			{
				vkDestroyFramebuffer(GetDevice(), gDebugFramebuffer, GetAllocator());
				gDebugFramebuffer = VK_NULL_HANDLE;

				gSavedDepth.Destroy();
			}

			void SaveDepth(VkCommandBuffer cmd, VkImage srcDepthImage)
			{
				if (gHasSavedDepth) {
					return;
				}

				constexpr VkImageAspectFlags kDepthAspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

				// Game depth is in READ_ONLY_OPTIMAL after the render pass ends.
				VulkanImage::TransitionImageLayout(srcDepthImage, VK_FORMAT_D32_SFLOAT_S8_UINT,
					VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, kDepthAspect, cmd);

				VulkanImage::TransitionImageLayout(gSavedDepth.image, VK_FORMAT_D32_SFLOAT_S8_UINT,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, kDepthAspect, cmd);

				VkImageCopy copyRegion{};
				copyRegion.srcSubresource = { kDepthAspect, 0, 0, 1 };
				copyRegion.srcOffset      = { 0, 0, 0 };
				copyRegion.dstSubresource = { kDepthAspect, 0, 0, 1 };
				copyRegion.dstOffset      = { 0, 0, 0 };
				copyRegion.extent         = { static_cast<uint32_t>(gDedicatedWidth), static_cast<uint32_t>(gDedicatedHeight), 1 };

				vkCmdCopyImage(cmd,
					srcDepthImage,     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					gSavedDepth.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &copyRegion);

				// Restore game depth; put saved depth into the layout the debug pass expects.
				VulkanImage::TransitionImageLayout(srcDepthImage, VK_FORMAT_D32_SFLOAT_S8_UINT,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, kDepthAspect, cmd);

				VulkanImage::TransitionImageLayout(gSavedDepth.image, VK_FORMAT_D32_SFLOAT_S8_UINT,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, kDepthAspect, cmd);

				gHasSavedDepth = true;
			}

			void RecordDedicatedPass(VkCommandBuffer cmd)
			{
				if (!gHasSavedDepth) {
					return;
				}

				BuildShapeLines();

				if (gDebugLineVertexCount == 0) {
					return;
				}

				UploadLineVertices();

				VkRenderPassBeginInfo beginInfo{};
				beginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				beginInfo.renderPass      = gDebugRenderPass;
				beginInfo.framebuffer     = gDebugFramebuffer;
				beginInfo.renderArea      = { { 0, 0 }, { static_cast<uint32_t>(gDedicatedWidth), static_cast<uint32_t>(gDedicatedHeight) } };
				beginInfo.clearValueCount = 0;
				beginInfo.pClearValues    = nullptr;

				Renderer::Debug::BeginLabel(cmd, "Debug Shapes Pass");

				vkCmdBeginRenderPass(cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gDebugPipeline.pipeline);

				DebugLinePushConstant pushConstantData;
				pushConstantData.projXView = gInitialProjMatrix * gInitialViewMatrix;
				vkCmdPushConstants(cmd, gDebugPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DebugLinePushConstant), &pushConstantData);

				const VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(cmd, 0, 1, &gDebugLineVertexBuffer, offsets);
				vkCmdDraw(cmd, gDebugLineVertexCount, 1, 0, 0);

				vkCmdEndRenderPass(cmd);
				Renderer::Debug::EndLabel(cmd);
			}
		}
	}
}
