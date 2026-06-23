#include "NativeRendererInternal.h"

#include "NativeDebugShapes.h"
#include "NativeDisplayList.h"
#include "PostProcessing.h"
#include "ScopedTimer.h"
#include "VulkanRenderer.h"
#include "profiling.h"

#include "glm/gtc/type_ptr.inl"

namespace Renderer
{
	namespace Native
	{
		static void CreateDraw()
		{
			GetNativeRendererState().currentDraw = Draw{};
			GetNativeRendererState().currentDraw->renderPassKey = GetNativeRendererState().cachedRenderPassKey;
			GetNativeRendererState().currentDraw->bRenderPassDirty = GetNativeRendererState().renderPassDirty;
			GetNativeRendererState().renderPassDirty = false;
		}

		void RenderMesh(SimpleMesh* pMesh, const uint32_t renderFlags)
		{
			GetNativeRendererState().cachedPerDrawData.renderFlags = renderFlags;

			if (pMesh->GetName() == DEBUG_MESH_NAME) {
				pMesh->GetName();
			}

			if (!GetNativeRendererState().currentDraw) {
				NATIVE_LOG_VERBOSE(LogLevel::Info, "RenderMesh Creating new draw!");
				CreateDraw();
			}

			NATIVE_LOG_VERBOSE(LogLevel::Info, "RenderMesh: {} prim: 0x{:x}", pMesh->GetName(), pMesh->GetPrim().CMD);

			auto& instance = GetNativeRendererState().currentDraw->instances.emplace_back();
			instance.animationMatrixStart = GetNativeRendererState().currentAnimMatrixIndex;
			instance.pMesh = pMesh;
			instance.perDrawData = GetNativeRendererState().cachedPerDrawData;
			instance.perDrawData.modelMatrixIndex  = static_cast<uint32_t>(GetNativeRendererState().modelBuffer.GetInstanceIndex());
			instance.perDrawData.animMatrixStart   = static_cast<uint32_t>(instance.animationMatrixStart);
			instance.perDrawData.lightingDataIndex = static_cast<uint32_t>(GetNativeRendererState().lightingDynamicBuffer.GetInstanceIndex());
			instance.perDrawData.animStDataIndex   = static_cast<uint32_t>(GetNativeRendererState().animStBuffer.GetInstanceIndex());

			NATIVE_LOG_VERBOSE(LogLevel::Info, "RenderMesh Model index: {} instance anim start: {}", instance.perDrawData.modelMatrixIndex, instance.animationMatrixStart);
		}

		void PushGlobalMatrices(float* pModel, float* pView, float* pProj)
		{
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushGlobalMatrices");

			// copy into model.
			if (pProj) {
				GetNativeRendererState().cachedProjMatrix = glm::make_mat4(pProj);
			}

			if (pView) {
				GetNativeRendererState().cachedViewMatrix = glm::make_mat4(pView);
			}

			PushModelMatrix(pModel);
		}

		void PushModelMatrix(float* pModel)
		{
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushModelMatrix: {}", GetNativeRendererState().modelBuffer.GetDebugIndex());
			const glm::mat4 modelMatrix = glm::make_mat4(pModel);
			GetNativeRendererState().modelBuffer.AddInstanceData(modelMatrix);
		}

		void PushAnimMatrix(float* pAnim)
		{
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushAnimMatrix: {}", GetNativeRendererState().animationMatrices.size());
			assert(GetNativeRendererState().animationMatrices.size() < static_cast<size_t>(gMaxAnimationMatrices));
			GetNativeRendererState().animationMatrices.push_back(glm::make_mat4(pAnim));
		}

		void StartAnimMatrix()
		{
			GetNativeRendererState().currentAnimMatrixIndex = GetNativeRendererState().animationMatrices.size();
		}

		void SetAnimStInstanceData(const glm::vec4& data)
		{
			GetNativeRendererState().animStBuffer.AddInstanceData(data);
		}

		void PushMatrixPacket(const MatrixPacket* const pPkt)
		{
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushMatrixPacket");

			assert(pPkt);

			{
				LightingDynamicBufferData data;

				data.lightDirection = glm::make_mat4(pPkt->objLightDirectionsMatrix);
				data.lightColor = glm::make_mat4(pPkt->lightColorMatrix);
				data.lightAmbient = glm::vec4(pPkt->adjustedLightAmbient[0], pPkt->adjustedLightAmbient[1], pPkt->adjustedLightAmbient[2], pPkt->adjustedLightAmbient[3]);
				data.flare = glm::vec4(pPkt->flare[0], pPkt->flare[1], pPkt->flare[2], pPkt->flare[3]);

				GetNativeRendererState().lightingDynamicBuffer.AddInstanceData(data);
			}

			SetAnimStInstanceData(glm::make_vec4(pPkt->animStNormalExtruder));

			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[0], pPkt->objLightDirectionsMatrix[1], pPkt->objLightDirectionsMatrix[2]);
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[4], pPkt->objLightDirectionsMatrix[5], pPkt->objLightDirectionsMatrix[6]);
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushLightData: direction: {} {} {}", pPkt->objLightDirectionsMatrix[8], pPkt->objLightDirectionsMatrix[9], pPkt->objLightDirectionsMatrix[10]);

			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[0], pPkt->lightColorMatrix[1], pPkt->lightColorMatrix[2], pPkt->lightColorMatrix[3]);
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[4], pPkt->lightColorMatrix[5], pPkt->lightColorMatrix[6], pPkt->lightColorMatrix[7]);
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushLightData: color: {} {} {} {}", pPkt->lightColorMatrix[8], pPkt->lightColorMatrix[9], pPkt->lightColorMatrix[10], pPkt->lightColorMatrix[11]);

			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushLightData: ambient: {} {} {} {}", pPkt->adjustedLightAmbient[0], pPkt->adjustedLightAmbient[1], pPkt->adjustedLightAmbient[2], pPkt->adjustedLightAmbient[3]);

			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushLightData: flare: {} {} {} {}", pPkt->flare[0], pPkt->flare[1], pPkt->flare[2], pPkt->flare[3]);

			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushLightData: animST: {} {} {} {}", pPkt->animStNormalExtruder[0], pPkt->animStNormalExtruder[1], pPkt->animStNormalExtruder[2], pPkt->animStNormalExtruder[3]);
		}

		void PushAnimST(float* pAnimST)
		{
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushAnimST: {}", GetNativeRendererState().animStBuffer.GetDebugIndex());
			assert(pAnimST);
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushAnimST: {} {} {} {}", pAnimST[0], pAnimST[1], pAnimST[2], pAnimST[3]);

			SetAnimStInstanceData(glm::make_vec4(pAnimST));
		}
	} // Native
} // Renderer

void Renderer::Native::OnVideoFlip()
{
	SignalRenderThreadEndCommands(GetNativeRendererState().renderThread);
}

void Renderer::Native::ApplyPendingResizeIfNeeded()
{
	ApplyPendingResizeInternal();
}


void Renderer::Native::InitializeDescriptorsSets(SimpleTexture* pTexture)
{
	if (!pTexture) {
		return;
	}

	NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());

	if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
		pTexture->GetName();
	}

	PS2::GSSimpleTexture* pTextureData = pTexture->GetRenderer();

	const Pipeline& pipeline = GetNativeRendererState().renderPass[GetNativeRendererState().cachedRenderPassKey].GetPipeline();

	if (pTextureData->HasDescriptorSets(pipeline)) {
		// Already have descriptor sets, no need to initialize them.
		return;
	}

	// Work out the sampler
	auto& textureRegisters = pTexture->GetTextureRegisters();
	PS2::PSSamplerSelector selector = PS2::EmulateTextureSampler(pTextureData->width, pTextureData->height, textureRegisters.clamp, textureRegisters.tex, {});

	VkSampler& sampler = PS2::GetSampler(selector);

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = pTextureData->imageView;
	imageInfo.sampler = sampler;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		const VkDescriptorBufferInfo modelDescBufferInfo = GetNativeRendererState().modelBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo animDescBufferInfo = GetNativeRendererState().animationBuffer.GetDescBufferInfo(i);

		const VkDescriptorBufferInfo lightingDescBufferInfo = GetNativeRendererState().lightingDynamicBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo animStDescBufferInfo = GetNativeRendererState().animStBuffer.GetDescBufferInfo(i);

		NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: offset: {} range: {}", animDescBufferInfo.offset, animDescBufferInfo.range);

		DescriptorWriteList writeList;
		writeList.EmplaceWrite({ 2, EBindingStage::Vertex, &modelDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
		writeList.EmplaceWrite({ 3, EBindingStage::Vertex, &animDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
		writeList.EmplaceWrite({ 4, EBindingStage::Vertex, &lightingDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
		writeList.EmplaceWrite({ 5, EBindingStage::Vertex, &animStDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });

		writeList.EmplaceWrite({ 1, EBindingStage::Fragment, nullptr, &imageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

		pTextureData->UpdateDescriptorSets(pipeline, writeList, i);
	}
}
void Renderer::Native::Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent, Renderer::CommandBufferList& commandBufferList)
{
	ZONE_SCOPED;

	{
		ZONE_SCOPED_NAME("Render Thread Wait");

		{
			ScopedTimer waitForRenderThread(GetNativeRendererState().renderWaitTime);
			MainThreadEndCommands(GetNativeRendererState().renderThread);
		}

		if (!GetRenderThreadHasRecordedCommands(GetNativeRendererState().renderThread)) {
			RecordBeginCommandBuffer();
			RecordBeginRenderPass(RenderPassKey::Empty);
			RecordEndCommandBuffer();
		}

		ResetRenderThread(GetNativeRendererState().renderThread);
	}

	ScopedTimer timer(GetNativeRendererState().renderTime);

	std::array<VkCommandBuffer, 2> cmdBuffers;

	{
		const VkCommandBuffer& cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];

		Renderer::Debug::EndLabel(cmd);
		vkEndCommandBuffer(cmd);

		cmdBuffers[0] = cmd;
	}

	{
		const VkCommandBuffer& cmd = DisplayList::FinalizeCommandBuffer(false);
		PostProcessing::AddPostProcessEffect(cmd, PostProcessing::Effect::AlphaFix);

		if (GetNativeRendererState().fadeActive) {
			PostProcessing::AddPostProcessEffect(cmd, PostProcessing::Effect::Fade); // Currently these effects don't chain, so fade also does alpha fix
			GetNativeRendererState().fadeActive = false;
		}

		vkEndCommandBuffer(cmd);

		cmdBuffers[1] = cmd;
	}

	for (const auto& cmd : cmdBuffers) {
		commandBufferList.push_back(cmd);
	}

	GetNativeRendererState().preview.RecordPass(commandBufferList,
		GetNativeRendererState().renderPass,
		GetNativeRendererState().nativeVertexBuffer,
		GetNativeRendererState().vkCmdSetColorWriteEnableEXT,
		GetNativeRendererState().vkCmdSetColorWriteMaskEXT);
	GetNativeRendererState().preview.ClearSavedDraws();

	GetNativeRendererState().nativeVertexBuffer.Reset();
	GetNativeRendererState().animationMatrices.clear();

	GetNativeRendererState().modelBuffer.Reset();
	GetNativeRendererState().lightingDynamicBuffer.Reset();
	GetNativeRendererState().animStBuffer.Reset();
	DebugShapes::ResetFrame();

	NATIVE_LOG(LogLevel::Info, "Renderer::Native::Render Complete!");
}

void Renderer::Native::BindTexture(SimpleTexture* pTexture)
{
	NATIVE_LOG(LogLevel::Info, "BindTexture: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());

	if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
		pTexture->GetName();
	}

	InitializeDescriptorsSets(pTexture);

	if (GetNativeRendererState().currentDraw) {
		GetNativeRendererState().currentDraw->pTexture = pTexture;

		GetNativeRendererState().currentDraw->pDescriptorSets = &pTexture->GetRenderer()->GetDescriptorSets(GetNativeRendererState().renderPass[GetNativeRendererState().cachedRenderPassKey].GetPipeline()).GetSet(GetCurrentFrame());

		GetNativeRendererState().currentDraw->projMatrix = GetNativeRendererState().cachedProjMatrix;
		GetNativeRendererState().currentDraw->viewMatrix = GetNativeRendererState().cachedViewMatrix;

		int instanceIndex = 0;
		for (auto& instance : GetNativeRendererState().currentDraw->instances) {
			NATIVE_LOG(LogLevel::Info, "BindTexture: instance ({}) anim start: {}", instanceIndex++, instance.animationMatrixStart);
		}

		GetNativeRendererState().currentDraw->bIsZMask = PS2::GetGSState().ZBUF.ZMSK != 0;

		if (!GetRenderThreadHasRecordedCommands(GetNativeRendererState().renderThread)) {
			GetNativeRendererState().initialViewMatrix = GetNativeRendererState().cachedViewMatrix;
			GetNativeRendererState().initialProjMatrix = GetNativeRendererState().cachedProjMatrix;
			DebugShapes::SetInitialViewProjection(GetNativeRendererState().cachedViewMatrix, GetNativeRendererState().cachedProjMatrix);
		}

		AddRenderThreadDraw(GetNativeRendererState().renderThread, *GetNativeRendererState().currentDraw);

		// If the texture is expecting to do a Z only draw, need to duplicate it.
		if (pTexture->GetTextureRegisters().test.AFAIL == AFAIL_ZB_ONLY && pTexture->GetTextureRegisters().test.ATST == ATST_NEVER) {
			GetNativeRendererState().currentDraw->bIsAfailZOnly = true;
			AddRenderThreadDraw(GetNativeRendererState().renderThread, *GetNativeRendererState().currentDraw);
		}

		GetNativeRendererState().currentDraw.reset();
	}

	NATIVE_LOG(LogLevel::Info, "BindTexture Done\n-------------------------------------------------------\n");
}

void Renderer::Native::BindUntextured()
{
	Renderer::Native::BindTexture(GetNativeRendererState().whiteTexture);
}

const VkSampler& Renderer::Native::GetSampler()
{
	return GetNativeRendererState().frameBufferSampler;
}

const VkImageView& Renderer::Native::GetColorImageView()
{
	return PostProcessing::GetColorImageView();
}

void Renderer::Native::DrawFade(uint8_t r, uint8_t g, uint8_t b, int a)
{
	GetNativeRendererState().fadeActive = true;

	GetNativeRendererState().fadeBuffer.GetBufferData().fadeColor = glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 127.0f);
	GetNativeRendererState().fadeBuffer.Map(GetCurrentFrame());
}

void Renderer::Native::UpdateRenderPassKey(Renderer::Native::EClearMode clearMode)
{
	GetNativeRendererState().cachedRenderPassKey.clearMode = clearMode;

	if (clearMode != EClearMode::None) {
		GetNativeRendererState().renderPassDirty = true;
	}
}

