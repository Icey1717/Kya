#include "NativeRendererInternal.h"

namespace Renderer
{
	namespace Native
	{
		RenderPassKey RenderPassKey::Empty = RenderPassKey{ EClearMode::ColorDepth };

		static NativeRendererState gNativeRendererState;

		NativeRendererState& GetNativeRendererState()
		{
			return gNativeRendererState;
		}

		const glm::mat4& GetInitialViewMatrix()
		{
			return GetNativeRendererState().initialViewMatrix;
		}

		const glm::mat4& GetInitialProjMatrix()
		{
			return GetNativeRendererState().initialProjMatrix;
		}

		void RenderStage::CreatePipeline()
		{
			PipelineKey key;
			key.options.bGlsl = true;
			key.options.bWireframe = false;
			key.options.topology = topologyTriangleList;
			GraphicsPipelineState state{};
			const char* name = "Native Renderer GLSL";
			if (kind == ERenderPassKind::ShadowMask) {
				gCreateInfo = { "shaders/shadow_mask.vert.spv", "shaders/shadow_mask.frag.spv", "", key };
				name = "Native Shadow Mask Pipeline";
			}
			else if (kind == ERenderPassKind::ShadowReceiver) {
				gCreateInfo = { "shaders/shadow_receiver.vert.spv", "shaders/shadow_receiver.frag.spv", "", key };
				state.depthWriteEnable = false;
				state.blendEnable = true;
				state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				name = "Native Shadow Receiver Pipeline";
			}
			else {
				gCreateInfo = { "shaders/native.vert.spv" , "shaders/native.frag.spv", "", key };
			}
			Renderer::Native::CreatePipeline(gCreateInfo, gRenderPass, gPipeline, name, state);
			gBlendPipelines.emplace(0, gPipeline.pipeline);
		}
	}
}

bool& Renderer::GetForceAnimMatrixIdentity()
{
	return Native::GetNativeRendererState().forceAnimMatrixIdentity;
}

void Renderer::RenderMesh(SimpleMesh* pNewMesh, const uint32_t renderFlags)
{
	assert(pNewMesh);
	Native::RenderMesh(pNewMesh, renderFlags);
}


void Renderer::SetGlobalAlpha(uint32_t alpha)
{
	Native::GetNativeRendererState().cachedPerDrawData.globalAlpha = alpha;
}

void Renderer::PushGlobalMatrices(float* pModel, float* pView, float* pProj)
{
	Native::PushGlobalMatrices(pModel, pView, pProj);
}

void Renderer::PushModelMatrix(float* pModel)
{
	Native::PushModelMatrix(pModel);
}

void Renderer::StartAnimMatrix()
{
	Native::StartAnimMatrix();
}

void Renderer::PushAnimMatrix(float* pAnim)
{
	Native::PushAnimMatrix(pAnim);
}

void Renderer::PushAnimST(float* pAnimST)
{
	Native::PushAnimST(pAnimST);
}

void Renderer::BindNull()
{
	Native::GetNativeRendererState().currentDraw.reset();
}

void Renderer::BindUntextured()
{
	Native::BindUntextured();
}
