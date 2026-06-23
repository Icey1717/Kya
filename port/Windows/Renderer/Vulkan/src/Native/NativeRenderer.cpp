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
			gCreateInfo = { "shaders/native.vert.spv" , "shaders/native.frag.spv", "", key };
			Renderer::Native::CreatePipeline(gCreateInfo, gRenderPass, gPipeline, "Native Previewer GLSL");
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
