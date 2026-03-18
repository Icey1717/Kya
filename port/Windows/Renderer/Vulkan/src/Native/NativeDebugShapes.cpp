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

			constexpr uint32_t gMaxDebugTriVertexCount = 65536;
			static std::array<DebugLineVertex, gMaxDebugTriVertexCount> gDebugTriVertices;
			static uint32_t gDebugTriVertexCount = 0;

			static VkBuffer gDebugTriVertexBuffer = VK_NULL_HANDLE;
			static VkDeviceMemory gDebugTriVertexBufferMemory = VK_NULL_HANDLE;

			static Renderer::Pipeline gDebugFilledPipeline;

			static glm::mat4 gInitialViewMatrix = glm::mat4(1.0f);
			static glm::mat4 gInitialProjMatrix = glm::mat4(1.0f);
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

			static void PushTri(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color)
			{
				if (gDebugTriVertexCount + 3 > gMaxDebugTriVertexCount) return;
				gDebugTriVertices[gDebugTriVertexCount++] = { glm::vec4(a, 1.0f), color };
				gDebugTriVertices[gDebugTriVertexCount++] = { glm::vec4(b, 1.0f), color };
				gDebugTriVertices[gDebugTriVertexCount++] = { glm::vec4(c, 1.0f), color };
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

			static void UploadTriVertices()
			{
				if (gDebugTriVertexCount == 0) {
					return;
				}

				void* pData = nullptr;
				const VkDeviceSize dataSize = sizeof(DebugLineVertex) * gDebugTriVertexCount;
				vkMapMemory(GetDevice(), gDebugTriVertexBufferMemory, 0, dataSize, 0, &pData);
				memcpy(pData, gDebugTriVertices.data(), static_cast<size_t>(dataSize));
				vkUnmapMemory(GetDevice(), gDebugTriVertexBufferMemory);
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

				const VkDeviceSize triBufferSize = sizeof(DebugLineVertex) * gMaxDebugTriVertexCount;

				CreateBuffer(triBufferSize,
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					gDebugTriVertexBuffer,
					gDebugTriVertexBufferMemory);

				SetObjectName(reinterpret_cast<uint64_t>(gDebugTriVertexBuffer), VK_OBJECT_TYPE_BUFFER, "Native Debug Triangle Vertex Buffer");
				SetObjectName(reinterpret_cast<uint64_t>(gDebugTriVertexBufferMemory), VK_OBJECT_TYPE_DEVICE_MEMORY, "Native Debug Triangle Vertex Buffer Memory");
			}

			void ResetFrame()
			{
				gHasInitialViewProjection = false;
				gDebugLineVertexCount = 0;
				gDebugTriVertexCount = 0;
				gHasSavedDepth = false;
			}

			void SetInitialViewProjection(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
			{
				gInitialViewMatrix = viewMatrix;
				gInitialProjMatrix = projMatrix;
				gHasInitialViewProjection = true;
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

			// Shared helper: decompose a row-major local-to-world matrix (edF32MATRIX4 layout)
			// into three world-space axis vectors and the world-space center.
			static void DecomposeLocalToWorld(const float* mat,
				glm::vec3& axisX, glm::vec3& axisY, glm::vec3& axisZ, glm::vec3& center)
			{
				axisX  = glm::vec3(mat[0],  mat[1],  mat[2]);
				axisY  = glm::vec3(mat[4],  mat[5],  mat[6]);
				axisZ  = glm::vec3(mat[8],  mat[9],  mat[10]);
				center = glm::vec3(mat[12], mat[13], mat[14]);
			}

			// Type 3: cylinder with y ∈ [-0.5, 0.5] and xz radius = 1 in local space.
			void AddCylinder(const float* mat, float r, float g, float b, float a)
			{
				glm::vec3 axisX, axisY, axisZ, center;
				DecomposeLocalToWorld(mat, axisX, axisY, axisZ, center);

				constexpr int   kSegs = 24;
				constexpr float kStep = 2.0f * 3.14159265358979323846f / kSegs;
				const glm::vec4 color(r, g, b, a);

				const glm::vec3 topCenter    = center + 0.5f * axisY;
				const glm::vec3 bottomCenter = center - 0.5f * axisY;

				for (int i = 0; i < kSegs; ++i) {
					const float a0 = kStep * i, a1 = kStep * (i + 1);
					const float c0 = std::cos(a0), s0 = std::sin(a0);
					const float c1 = std::cos(a1), s1 = std::sin(a1);

					// Top and bottom rings
					PushLine(topCenter    + c0 * axisX + s0 * axisZ,
					         topCenter    + c1 * axisX + s1 * axisZ, color);
					PushLine(bottomCenter + c0 * axisX + s0 * axisZ,
					         bottomCenter + c1 * axisX + s1 * axisZ, color);

					// Vertical lines at 4 cardinal angles
					if (i % (kSegs / 4) == 0) {
						PushLine(topCenter    + c0 * axisX + s0 * axisZ,
						         bottomCenter + c0 * axisX + s0 * axisZ, color);
					}
				}
			}

			// Type 4: cone with apex at y=0.5 and base circle at y=-0.5 with xz radius=1 in local space.
			// From the collision test: x²+z² ≤ (0.5-y)², so radius at height y is (0.5-y).
			void AddCone(const float* mat, float r, float g, float b, float a)
			{
				glm::vec3 axisX, axisY, axisZ, center;
				DecomposeLocalToWorld(mat, axisX, axisY, axisZ, center);

				constexpr int   kSegs = 24;
				constexpr float kStep = 2.0f * 3.14159265358979323846f / kSegs;
				const glm::vec4 color(r, g, b, a);

				const glm::vec3 apex       = center + 0.5f * axisY;
				const glm::vec3 baseCenter = center - 0.5f * axisY;

				for (int i = 0; i < kSegs; ++i) {
					const float a0 = kStep * i, a1 = kStep * (i + 1);
					const float c0 = std::cos(a0), s0 = std::sin(a0);
					const float c1 = std::cos(a1), s1 = std::sin(a1);

					// Base circle
					PushLine(baseCenter + c0 * axisX + s0 * axisZ,
					         baseCenter + c1 * axisX + s1 * axisZ, color);

					// Lines from apex at 4 cardinal angles
					if (i % (kSegs / 4) == 0) {
						PushLine(apex, baseCenter + c0 * axisX + s0 * axisZ, color);
					}
				}
			}

			// Type 1: plane+box: x,z ∈ [-0.5, 0.5], y ≥ 0 (open top) in local space.
			// Drawn as an open-top box with floor at y=0 and walls to y=1; tick marks at
			// the top of each wall indicate the zone continues upward indefinitely.
			void AddOpenTopBox(const float* mat, float r, float g, float b, float a)
			{
				glm::vec3 axisX, axisY, axisZ, center;
				DecomposeLocalToWorld(mat, axisX, axisY, axisZ, center);

				const glm::vec4 color(r, g, b, a);
				const glm::vec3 ex = 0.5f * axisX;
				const glm::vec3 ez = 0.5f * axisZ;

				// Floor corners at y=0 in local space
				const glm::vec3 f[4] = {
					center - ex - ez,
					center + ex - ez,
					center + ex + ez,
					center - ex + ez,
				};
				// Wall tops at y=1 in local space (no cap drawn)
				const glm::vec3 t[4] = {
					f[0] + axisY, f[1] + axisY, f[2] + axisY, f[3] + axisY,
				};

				PushLine(f[0], f[1], color); PushLine(f[1], f[2], color);
				PushLine(f[2], f[3], color); PushLine(f[3], f[0], color);
				PushLine(f[0], t[0], color); PushLine(f[1], t[1], color);
				PushLine(f[2], t[2], color); PushLine(f[3], t[3], color);

				// Outward ticks to indicate walls continue upward
				const glm::vec3 tick = 0.15f * axisY;
				for (int i = 0; i < 4; ++i) {
					PushLine(t[i], t[i] + tick, color);
				}
			}

			// Type 0: half-space y ≥ 0 — draw a cross on the y=0 boundary plane and an arrow in +y.
			void AddHalfSpace(const float* mat, float r, float g, float b, float a)
			{
				glm::vec3 axisX, axisY, axisZ, center;
				DecomposeLocalToWorld(mat, axisX, axisY, axisZ, center);

				const glm::vec4 color(r, g, b, a);
				// Cross in the boundary plane
				PushLine(center - axisX, center + axisX, color);
				PushLine(center - axisZ, center + axisZ, color);
				// Arrow shaft in +y
				PushLine(center, center + axisY, color);
				// Arrowhead (four fins)
				const glm::vec3 tip  = center + axisY;
				const glm::vec3 base = center + 0.8f * axisY;
				const float kHead = 0.15f;
				PushLine(tip, base + kHead * axisX, color);
				PushLine(tip, base - kHead * axisX, color);
				PushLine(tip, base + kHead * axisZ, color);
				PushLine(tip, base - kHead * axisZ, color);
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

			static void CreateFilledPipeline(const VkRenderPass& renderPass, Renderer::Pipeline& pipeline, const char* name)
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
				rasterizer.cullMode = VK_CULL_MODE_NONE;
				rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
					throw std::runtime_error("failed to create debug filled graphics pipeline!");
				}

				SetObjectName(reinterpret_cast<uint64_t>(pipeline.pipeline), VK_OBJECT_TYPE_PIPELINE, name);
			}

			void AddFilledOBB(const float* mat, float hw, float hh, float hd, float r, float g, float b, float a)
			{
				const glm::vec3 axisX(mat[0], mat[1], mat[2]);
				const glm::vec3 axisY(mat[4], mat[5], mat[6]);
				const glm::vec3 axisZ(mat[8], mat[9], mat[10]);
				const glm::vec3 center(mat[12], mat[13], mat[14]);
				const glm::vec3 ex = axisX * hw, ey = axisY * hh, ez = axisZ * hd;
				const glm::vec3 c[8] = {
					center - ex - ey - ez, center + ex - ey - ez,
					center + ex + ey - ez, center - ex + ey - ez,
					center - ex - ey + ez, center + ex - ey + ez,
					center + ex + ey + ez, center - ex + ey + ez,
				};
				const glm::vec4 color(r, g, b, a);
				PushTri(c[0],c[1],c[5],color); PushTri(c[0],c[5],c[4],color); // -Y
				PushTri(c[3],c[7],c[6],color); PushTri(c[3],c[6],c[2],color); // +Y
				PushTri(c[0],c[3],c[2],color); PushTri(c[0],c[2],c[1],color); // -Z
				PushTri(c[4],c[5],c[6],color); PushTri(c[4],c[6],c[7],color); // +Z
				PushTri(c[0],c[4],c[7],color); PushTri(c[0],c[7],c[3],color); // -X
				PushTri(c[1],c[2],c[6],color); PushTri(c[1],c[6],c[5],color); // +X
			}

			void AddFilledCylinder(const float* mat, float r, float g, float b, float a)
			{
				glm::vec3 axisX, axisY, axisZ, center;
				DecomposeLocalToWorld(mat, axisX, axisY, axisZ, center);
				constexpr int kSegs = 24;
				constexpr float kStep = 2.0f * 3.14159265358979323846f / kSegs;
				const glm::vec4 color(r, g, b, a);
				const glm::vec3 top = center + 0.5f * axisY;
				const glm::vec3 bot = center - 0.5f * axisY;
				for (int i = 0; i < kSegs; i++) {
					const float a0 = kStep * i, a1 = kStep * (i + 1);
					const glm::vec3 r0 = std::cos(a0) * axisX + std::sin(a0) * axisZ;
					const glm::vec3 r1 = std::cos(a1) * axisX + std::sin(a1) * axisZ;
					PushTri(top, top + r0, top + r1, color);
					PushTri(bot, bot + r1, bot + r0, color);
					PushTri(top + r0, bot + r0, bot + r1, color);
					PushTri(top + r0, bot + r1, top + r1, color);
				}
			}

			void AddFilledCone(const float* mat, float r, float g, float b, float a)
			{
				glm::vec3 axisX, axisY, axisZ, center;
				DecomposeLocalToWorld(mat, axisX, axisY, axisZ, center);
				constexpr int kSegs = 24;
				constexpr float kStep = 2.0f * 3.14159265358979323846f / kSegs;
				const glm::vec4 color(r, g, b, a);
				const glm::vec3 apex = center + 0.5f * axisY;
				const glm::vec3 baseCtr = center - 0.5f * axisY;
				for (int i = 0; i < kSegs; i++) {
					const float a0 = kStep * i, a1 = kStep * (i + 1);
					const glm::vec3 r0 = std::cos(a0) * axisX + std::sin(a0) * axisZ;
					const glm::vec3 r1 = std::cos(a1) * axisX + std::sin(a1) * axisZ;
					PushTri(baseCtr, baseCtr + r1, baseCtr + r0, color);
					PushTri(apex, baseCtr + r0, baseCtr + r1, color);
				}
			}

			void AddFilledSphere(float cx, float cy, float cz, float radius, float r, float g, float b, float a)
			{
				constexpr int kStacks = 12, kSlices = 20;
				const glm::vec4 color(r, g, b, a);
				const glm::vec3 center(cx, cy, cz);
				auto vert = [&](int st, int sl) -> glm::vec3 {
					const float phi   = 3.14159265358979323846f * st / kStacks;
					const float theta = 2.0f * 3.14159265358979323846f * sl / kSlices;
					return center + radius * glm::vec3(std::sin(phi) * std::cos(theta),
					                                   std::cos(phi),
					                                   std::sin(phi) * std::sin(theta));
				};
				for (int st = 0; st < kStacks; st++) {
					for (int sl = 0; sl < kSlices; sl++) {
						const int sl1 = (sl + 1) % kSlices;
						const glm::vec3 tl = vert(st,   sl),  tr = vert(st,   sl1);
						const glm::vec3 bl = vert(st+1, sl),  br = vert(st+1, sl1);
						if (st > 0)             PushTri(tl, br, bl, color);
						if (st < kStacks - 1)   PushTri(tl, tr, br, color);
					}
				}
			}

			void Record(const VkCommandBuffer& cmd, const Renderer::Pipeline& pipeline)
			{
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
					CreateFilledPipeline(gDebugRenderPass, gDebugFilledPipeline, "Debug Shapes Dedicated Filled Pipeline");
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

				if (gDebugLineVertexCount == 0 && gDebugTriVertexCount == 0) {
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

				DebugLinePushConstant pushConstantData;
				pushConstantData.projXView = gInitialProjMatrix * gInitialViewMatrix;

				if (gDebugLineVertexCount > 0) {
					vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gDebugPipeline.pipeline);
					vkCmdPushConstants(cmd, gDebugPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DebugLinePushConstant), &pushConstantData);

					const VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(cmd, 0, 1, &gDebugLineVertexBuffer, offsets);
					vkCmdDraw(cmd, gDebugLineVertexCount, 1, 0, 0);
				}

				if (gDebugTriVertexCount > 0) {
					UploadTriVertices();

					vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gDebugFilledPipeline.pipeline);
					vkCmdPushConstants(cmd, gDebugFilledPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DebugLinePushConstant), &pushConstantData);

					const VkDeviceSize triOffsets[] = { 0 };
					vkCmdBindVertexBuffers(cmd, 0, 1, &gDebugTriVertexBuffer, triOffsets);
					vkCmdDraw(cmd, gDebugTriVertexCount, 1, 0, 0);
				}

				vkCmdEndRenderPass(cmd);
				Renderer::Debug::EndLabel(cmd);
			}
		}
	}
}
