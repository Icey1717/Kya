#include "NativeRendererInternal.h"

#include "NativeDebugShapes.h"
#include "NativeDisplayList.h"
#include "NativeFrameBufferCopy.h"
#include "FlareDraw.h"
#include "PostProcessing.h"
#include "ScopedTimer.h"
#include "VulkanRenderer.h"
#include "profiling.h"
#include "TextureSampling.h"

#include "glm/gtc/type_ptr.inl"
#include <atomic>

namespace Renderer
{
	namespace Native
	{
		using PS2::MipOverride;
		static std::atomic<MipOverride> gMipOverride = MipOverride::None;

		void SetForceHighestMipLevel(bool enabled)
		{
			if (enabled) gMipOverride.store(MipOverride::Highest, std::memory_order_relaxed);
			else {
				auto expected = MipOverride::Highest;
				gMipOverride.compare_exchange_strong(expected, MipOverride::None, std::memory_order_relaxed);
			}
		}

		void SetForceLowestMipLevel(bool enabled)
		{
			if (enabled) gMipOverride.store(MipOverride::Lowest, std::memory_order_relaxed);
			else {
				auto expected = MipOverride::Lowest;
				gMipOverride.compare_exchange_strong(expected, MipOverride::None, std::memory_order_relaxed);
			}
		}

		static void CreateDraw()
		{
			GetNativeRendererState().currentDraw = Draw{};
			GetNativeRendererState().currentDraw->renderPassKey = GetNativeRendererState().cachedRenderPassKey;
			GetNativeRendererState().currentDraw->bRenderPassDirty = GetNativeRendererState().renderPassDirty;
			GetNativeRendererState().renderPassDirty = false;
		}

		void RenderMesh(SimpleMesh* pMesh, const uint32_t renderFlags, const uint32_t* pColors)
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
			if (pColors) {
				instance.vertexColors.assign(pColors, pColors + pMesh->GetVertexBufferData().GetVertexTail());
			}
			instance.gsAlpha = PS2::GetGSState().ALPHA;
			// Option packets and full-alpha masks can change within a material batch.
			instance.gsTest = PS2::GetGSState().TEST;
			// Linear base filtering for synthetic draws.
			instance.gsTex1.MMAG = 1;
			instance.gsTex1.MMIN = 1;
			if (PS2::GetGSState().tex1Set) {
				instance.gsTex1 = PS2::GetGSState().TEX1;
			}

			instance.bIsZMask = PS2::GetGSState().ZBUF.ZMSK != 0;
			assert(!instance.gsTest.DATE && "Native destination-alpha testing is not implemented");
			instance.perDrawData = GetNativeRendererState().cachedPerDrawData;
			instance.perDrawData.modelMatrixIndex  = static_cast<uint32_t>(GetNativeRendererState().modelBuffer.GetInstanceIndex());
			instance.perDrawData.animMatrixStart   = static_cast<uint32_t>(instance.animationMatrixStart);
			instance.perDrawData.lightingDataIndex = static_cast<uint32_t>(GetNativeRendererState().lightingDynamicBuffer.GetInstanceIndex());
			instance.perDrawData.animStDataIndex   = static_cast<uint32_t>(GetNativeRendererState().animStBuffer.GetInstanceIndex());
			if ((renderFlags & 0x20) == 0) {
				instance.perDrawData.globalAlpha = 0x80;
			}

			instance.perDrawData.stripFlags = pMesh->GetStripFlags();

			NATIVE_LOG_VERBOSE(LogLevel::Info, "RenderMesh Model index: {} instance anim start: {}", instance.perDrawData.modelMatrixIndex, instance.animationMatrixStart);
            if (DrawTrace::IsEnabled()) {
                DrawTrace::Submission trace;
                DrawTrace::CopyName(trace.mesh, pMesh->GetName().c_str());
                trace.assetKey = DrawTrace::AssetKey(pMesh->GetName().c_str());
                trace.flags = renderFlags;
                trace.primitive = pMesh->GetPrim().CMD;
                trace.modelIndex = instance.perDrawData.modelMatrixIndex;
                trace.animationIndex = instance.perDrawData.animMatrixStart;
                trace.lightingIndex = instance.perDrawData.lightingDataIndex;
                trace.animStIndex = instance.perDrawData.animStDataIndex;
                trace.globalAlpha = instance.perDrawData.globalAlpha;
                memcpy(trace.model.data(), &GetNativeRendererState().modelBuffer.GetLastInstance(), sizeof(float) * 16);
                instance.traceSubmission = DrawTrace::Submit(trace);
                DrawTrace::Source source;
                if (DrawTrace::Highlight(instance.traceSubmission, source)) {
                    const glm::mat4 model = glm::make_mat4(source.model.data());
                    const glm::vec3 center = glm::vec3(model * glm::vec4(source.bounds[0], source.bounds[1], source.bounds[2], 1.0f));
                    const float scale = std::max({glm::length(glm::vec3(model[0])), glm::length(glm::vec3(model[1])), glm::length(glm::vec3(model[2]))});
                    DebugShapes::AddSphere(center, std::abs(source.bounds[3]) * scale, glm::vec4(1.0f, 0.75f, 0.15f, 1.0f));
                }
            }
		}

		void PushGlobalMatrices(float* pModel, float* pView, float* pProj, const float* pGsProj)
		{
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushGlobalMatrices");

			// copy into model.
			if (pProj) {
				GetNativeRendererState().cachedProjMatrix = glm::make_mat4(pProj);
				GetNativeRendererState().cachedPerDrawData.gsTextureQScale = TextureSampling::GetGsQScale(pProj, pGsProj);
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

		void PushShadowProjectionMatrix(const float* matrix)
		{
			assert(matrix);
			GetNativeRendererState().shadowProjectionBuffer.AddInstanceData(glm::make_mat4(matrix));
			GetNativeRendererState().cachedPerDrawData.shadowProjectionIndex =
				static_cast<uint32_t>(GetNativeRendererState().shadowProjectionBuffer.GetInstanceIndex());
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
	FrameBufferCopy::ApplyPendingResize();
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
	GetNativeRendererState().shadowProjectionBuffer.Reset();
	GetNativeRendererState().shadowProjectionBuffer.AddInstanceData(glm::mat4(1.0f));
	GetNativeRendererState().cachedPerDrawData.shadowProjectionIndex = 0;
	DebugShapes::ResetFrame();
    DrawTrace::AdvanceFrame();

	NATIVE_LOG(LogLevel::Info, "Renderer::Native::Render Complete!");
}

void Renderer::Native::BindTexture(SimpleTexture* pTexture)
{
	NATIVE_LOG(LogLevel::Info, "BindTexture: {} material: {} layer: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex());

	if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
		pTexture->GetName();
	}

	if (GetNativeRendererState().currentDraw) {
		GetNativeRendererState().currentDraw->pTexture = pTexture;

		GetNativeRendererState().currentDraw->projMatrix = GetNativeRendererState().cachedProjMatrix;
		GetNativeRendererState().currentDraw->viewMatrix = GetNativeRendererState().cachedViewMatrix;

		int instanceIndex = 0;
		const auto mipOverride = gMipOverride.load(std::memory_order_relaxed);
		for (auto& instance : GetNativeRendererState().currentDraw->instances) {
			const auto sampling = PS2::ResolveTextureSampling(pTexture->GetTextureRegisters().clamp, instance.gsTex1,
				pTexture->GetRenderer()->mipLevels, mipOverride);
			instance.descriptorSet = pTexture->GetRenderer()->GetOrCreateTextureBinding(sampling.sampler);
			const auto& settings = sampling.lod;
			instance.perDrawData.textureLodBias = settings.lodBias;
			instance.perDrawData.textureLodScale = settings.lodScale;
			instance.perDrawData.textureFixedLod = settings.fixedLod;
			instance.perDrawData.textureMaxMipLevel = settings.maxLevel;
			instance.perDrawData.textureLodEnable = 1;
			NATIVE_LOG(LogLevel::Info, "BindTexture: instance ({}) anim start: {}", instanceIndex++, instance.animationMatrixStart);
		}
		if (!GetNativeRendererState().currentDraw->instances.empty()) {
			GetNativeRendererState().currentDraw->descriptorSet = GetNativeRendererState().currentDraw->instances.front().descriptorSet;
		}

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
	if (GetNativeRendererState().cachedRenderPassKey.kind != ERenderPassKind::Main) {
		return;
	}

	GetNativeRendererState().cachedRenderPassKey.clearMode = clearMode;

	if (clearMode != EClearMode::None) {
		GetNativeRendererState().renderPassDirty = true;
	}
}

namespace
{
	std::atomic<bool> flareOcclusionEnabled{ true };
}

void Renderer::Native::SetFlareOcclusionEnabled(bool enabled)
{
	flareOcclusionEnabled = enabled;
}

void Renderer::Native::SubmitFlare(const FlareDraw& flare)
{
	auto& state = GetNativeRendererState();
	if (!state.renderThread || !flare.pTexture || state.cachedRenderPassKey.kind != ERenderPassKind::Main) return;
	// Finish preceding geometry in this material batch before measuring flare visibility.
	if (state.currentDraw) Renderer::Native::BindTexture(flare.pTexture);
	FlareDraw submitted = flare;
	submitted.occlusionEnabled = flareOcclusionEnabled.load();
	AddRenderThreadFlare(state.renderThread, submitted, state.cachedRenderPassKey, state.renderPassDirty);
	// Subsequent geometry resumes with LOAD, preserving the flare color and scene depth.
	state.cachedRenderPassKey = RenderPassKey{ EClearMode::None, ERenderPassKind::Main };
	state.renderPassDirty = true;
}

void Renderer::Native::CaptureFrameBuffer()
{
	auto& state = GetNativeRendererState();
	if (!state.renderThread) return;
	assert(!state.currentDraw); // Materials must submit their geometry before this boundary.
	AddRenderThreadFrameBufferCopy(state.renderThread, state.cachedRenderPassKey, state.renderPassDirty);
	state.cachedRenderPassKey = RenderPassKey{ EClearMode::None, ERenderPassKind::Main };
	state.renderPassDirty = true;
}

void Renderer::Native::SetFrameBufferMaterial(const FrameBufferMaterialSettings& settings)
{
	GetNativeRendererState().frameBufferMaterial = settings;
}

void Renderer::Native::BindFrameBufferTexture()
{
	auto& state = GetNativeRendererState();
	if (!state.currentDraw) return;
	auto& draw = *state.currentDraw;
	draw.frameBufferMaterial = state.frameBufferMaterial;
	for (auto& instance : draw.instances) {
		instance.perDrawData.frameBufferMode = state.frameBufferMaterial.textureFunction == 1 ? 2 : 1;
		instance.perDrawData.frameBufferScaleX = state.frameBufferMaterial.textureWidth / 512.0f;
		instance.perDrawData.frameBufferScaleY = state.frameBufferMaterial.textureHeight / 512.0f;
	}
	// Reuse normal submission and buffer bindings; recording substitutes the capture descriptor.
	Renderer::Native::BindTexture(state.whiteTexture);
}

void Renderer::Native::BeginShadowMask(const ShadowPassSettings& settings)
{
	if (GetNativeRendererState().currentDraw) {
		NATIVE_LOG(LogLevel::Warning, "Discarding incomplete draw at shadow-mask boundary");
		GetNativeRendererState().currentDraw.reset();
	}
	GetNativeRendererState().shadowAlpha = settings.alpha;
	AddRenderThreadShadowBegin(GetNativeRendererState().renderThread, settings);
	GetNativeRendererState().cachedRenderPassKey.kind = ERenderPassKind::ShadowMask;
	GetNativeRendererState().cachedRenderPassKey.clearMode = EClearMode::ColorDepth;
	GetNativeRendererState().renderPassDirty = true;
}

void Renderer::Native::BlurShadowMask()
{
	AddRenderThreadShadowBlur(GetNativeRendererState().renderThread);
}

void Renderer::Native::BeginShadowReceiver(const ShadowReceiverViewport& viewport)
{
	AddRenderThreadShadowReceiver(GetNativeRendererState().renderThread, viewport);
	GetNativeRendererState().cachedRenderPassKey.kind = ERenderPassKind::ShadowReceiver;
	GetNativeRendererState().cachedRenderPassKey.clearMode = EClearMode::None;
	GetNativeRendererState().renderPassDirty = true;
}

void Renderer::Native::EndShadowPass()
{
	if (GetNativeRendererState().currentDraw) {
		NATIVE_LOG(LogLevel::Warning, "Discarding incomplete draw at shadow-pass boundary");
		GetNativeRendererState().currentDraw.reset();
	}
	AddRenderThreadShadowEnd(GetNativeRendererState().renderThread);
	GetNativeRendererState().cachedRenderPassKey.kind = ERenderPassKind::Main;
	GetNativeRendererState().cachedRenderPassKey.clearMode = EClearMode::None;
	GetNativeRendererState().renderPassDirty = true;
}

void Renderer::Native::BindShadowReceiver()
{
	if (!GetNativeRendererState().currentDraw) return;
	Draw& draw = *GetNativeRendererState().currentDraw;
	draw.pTexture = GetNativeRendererState().whiteTexture;
	draw.descriptorSet = VK_NULL_HANDLE;
	draw.projMatrix = GetNativeRendererState().cachedProjMatrix;
	draw.viewMatrix = GetNativeRendererState().cachedViewMatrix;
	for (auto& instance : draw.instances) {
		instance.bIsZMask = true;
		instance.perDrawData.globalAlpha = GetNativeRendererState().shadowAlpha;
	}
	AddRenderThreadDraw(GetNativeRendererState().renderThread, draw);
	GetNativeRendererState().currentDraw.reset();
}

