namespace Renderer
{
	namespace Native
	{
		static bool CanMergeMesh()
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

			return true;
		}

		static void CreateDraw()
		{
			gCurrentDraw = Draw{};
			gCurrentDraw->renderPassKey = gCachedRenderPassKey;
			gCurrentDraw->bRenderPassDirty = gRenderPassDirty;
			gRenderPassDirty = false;
		}
		static void SanityCheck(SimpleMesh* pMesh)
		{
			auto& internalBuffer = pMesh->GetVertexBufferData();

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
		void RenderMesh(SimpleMesh* pMesh, const uint32_t renderFlags)
		{
			gCachedPerDrawData.renderFlags = renderFlags;

			if (!gCurrentDraw) {
				NATIVE_LOG_VERBOSE(LogLevel::Info, "RenderMesh Creating new draw!");
				CreateDraw();
			}

			NATIVE_LOG_VERBOSE(LogLevel::Info, "RenderMesh: {} prim: 0x{:x}", pMesh->GetName(), pMesh->GetPrim().CMD);

			constexpr bool bSanityCheck = false;

			if (bSanityCheck) {
				FLUSH_LOG();

				SanityCheck(pMesh);
			}

			constexpr bool bMergingEnabeld = false;

			if (bMergingEnabeld && CanMergeMesh()) {
				NATIVE_LOG_VERBOSE(LogLevel::Info, "RenderMesh: merging");
				MergeIndexData();
			}
			else {
				auto& instance = gCurrentDraw->instances.emplace_back();
				instance.animationMatrixStart = gCurrentAnimMatrixIndex;
				instance.modelMatrixIndex = gModelBuffer.GetInstanceIndex();
				instance.pMesh = pMesh;
				instance.perDrawData = gCachedPerDrawData;
				instance.lightingDataIndex = gLightingDynamicBuffer.GetInstanceIndex();
				instance.animStDataIndex = gAnimStBuffer.GetInstanceIndex();

				NATIVE_LOG_VERBOSE(LogLevel::Info, "RenderMesh Model index: {} instance anim start: {}", instance.modelMatrixIndex, instance.animationMatrixStart);
			}
		}

		void PushGlobalMatrices(float* pModel, float* pView, float* pProj)
		{
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushGlobalMatrices");

			// copy into model.
			if (pProj) {
				gCachedProjMatrix = glm::make_mat4(pProj);
			}

			if (pView) {
				gCachedViewMatrix = glm::make_mat4(pView);
			}

			PushModelMatrix(pModel);
		}

		void PushModelMatrix(float* pModel)
		{
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushModelMatrix: {}", gModelBuffer.GetDebugIndex());
			const glm::mat4 modelMatrix = glm::make_mat4(pModel);
			gModelBuffer.AddInstanceData(modelMatrix);
		}

		void PushAnimMatrix(float* pAnim)
		{
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushAnimMatrix: {}", gAnimationMatrices.size());
			gAnimationMatrices.push_back(glm::make_mat4(pAnim));
		}

		void StartAnimMatrix()
		{
			gCurrentAnimMatrixIndex = gAnimationMatrices.size();
		}

		void SetAnimStInstanceData(const glm::vec4& data)
		{
			gAnimStBuffer.AddInstanceData(data);
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

				gLightingDynamicBuffer.AddInstanceData(data);
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
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushAnimST: {}", gAnimStBuffer.GetDebugIndex());
			assert(pAnimST);
			NATIVE_LOG_VERBOSE(LogLevel::Info, "PushAnimST: {} {} {} {}", pAnimST[0], pAnimST[1], pAnimST[2], pAnimST[3]);

			SetAnimStInstanceData(glm::make_vec4(pAnimST));
		}
	} // Native
} // Renderer

void Renderer::Native::OnVideoFlip()
{
	gRenderThread->SignalEndCommands();
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

	const Pipeline& pipeline = gRenderPass[gCachedRenderPassKey].GetPipeline();

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
		const VkDescriptorBufferInfo modelDescBufferInfo = gModelBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo animDescBufferInfo = gAnimationBuffer.GetDescBufferInfo(i, 27 * sizeof(glm::mat4));

		const VkDescriptorBufferInfo alphaDescBufferInfo = gAlphaBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo vertexDynamicsDescBufferInfo = gLightingDynamicBuffer.GetDescBufferInfo(i);
		const VkDescriptorBufferInfo animStDescBufferInfo = gAnimStBuffer.GetDescBufferInfo(i);

		NATIVE_LOG_VERBOSE(LogLevel::Info, "UpdateDescriptors: offset: {} range: {}", animDescBufferInfo.offset, animDescBufferInfo.range);

		DescriptorWriteList writeList;
		writeList.EmplaceWrite({ 2, EBindingStage::Vertex, &modelDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
		writeList.EmplaceWrite({ 3, EBindingStage::Vertex, &animDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
		writeList.EmplaceWrite({ 4, EBindingStage::Vertex, &vertexDynamicsDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
		writeList.EmplaceWrite({ 5, EBindingStage::Vertex, &animStDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });

		writeList.EmplaceWrite({ 6, EBindingStage::Fragment, &alphaDescBufferInfo, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
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
			ScopedTimer waitForRenderThread(gRenderWaitTime);
			gRenderThread->MainThreadEndCommands();
		}

		if (!gRenderThread->GetHasRecordedCommands()) {
			RecordBeginCommandBuffer();
			RecordBeginRenderPass(RenderPassKey::Empty);
			RecordEndCommandBuffer();
		}

		gRenderThread->Reset();
	}

	ScopedTimer timer(gRenderTime);

	std::array<VkCommandBuffer, 2> cmdBuffers;

	{
		const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

		Renderer::Debug::EndLabel(cmd);
		vkEndCommandBuffer(cmd);

		cmdBuffers[0] = cmd;
	}

	{
		const VkCommandBuffer& cmd = DisplayList::FinalizeCommandBuffer(false);
		PostProcessing::AddPostProcessEffect(cmd, PostProcessing::Effect::AlphaFix);

		if (bFadeActive) {
			PostProcessing::AddPostProcessEffect(cmd, PostProcessing::Effect::Fade); // Currently these effects don't chain, so fade also does alpha fix
			bFadeActive = false;
		}

		vkEndCommandBuffer(cmd);

		cmdBuffers[1] = cmd;
	}

	for (const auto& cmd : cmdBuffers) {
		commandBufferList.push_back(cmd);
	}

	gNativeVertexBuffer.Reset();
	gAnimationMatrices.clear();

	gModelBuffer.Reset();
	gLightingDynamicBuffer.Reset();
	gAnimStBuffer.Reset();
	DebugShapes::ResetFrame();

	NATIVE_LOG(LogLevel::Info, "Renderer::Native::Render Complete!");
}

void Renderer::Native::BindTexture(SimpleTexture* pTexture)
{
	NATIVE_LOG(LogLevel::Info, "BindTexture: {} material: {} layer: {} indices: {}", pTexture->GetName(), pTexture->GetMaterialIndex(), pTexture->GetLayerIndex(), gNativeVertexBufferDataDraw.GetIndexTail());

	if (pTexture->GetName() == DEBUG_TEXTURE_NAME) {
		pTexture->GetName();
	}

	InitializeDescriptorsSets(pTexture);

	if (gCurrentDraw) {
		gCurrentDraw->pTexture = pTexture;

		gCurrentDraw->pDescriptorSets = &pTexture->GetRenderer()->GetDescriptorSets(gRenderPass[gCachedRenderPassKey].GetPipeline()).GetSet(GetCurrentFrame());

		gCurrentDraw->projMatrix = gCachedProjMatrix;
		gCurrentDraw->viewMatrix = gCachedViewMatrix;

		int instanceIndex = 0;
		for (auto& instance : gCurrentDraw->instances) {
			NATIVE_LOG(LogLevel::Info, "BindTexture: instance ({}) anim start: {}", instanceIndex++, instance.animationMatrixStart);
		}

		gCurrentDraw->bIsZMask = PS2::GetGSState().ZBUF.ZMSK != 0;

		if (!gRenderThread->GetHasRecordedCommands()) {
			gInitialViewMatrix = gCachedViewMatrix;
			gInitialProjMatrix = gCachedProjMatrix;
			DebugShapes::SetInitialViewProjection(gCachedViewMatrix, gCachedProjMatrix);
		}

		gRenderThread->AddDraw(*gCurrentDraw);

		// If the texture is expecting to do a Z only draw, need to duplicate it.
		if (pTexture->GetTextureRegisters().test.AFAIL == AFAIL_ZB_ONLY && pTexture->GetTextureRegisters().test.ATST == ATST_NEVER) {
			gCurrentDraw->bIsAfailZOnly = true;
			gRenderThread->AddDraw(*gCurrentDraw);
		}

		gCurrentDraw.reset();
	}

	NATIVE_LOG(LogLevel::Info, "BindTexture Done\n-------------------------------------------------------\n");
}

const VkSampler& Renderer::Native::GetSampler()
{
	return gFrameBufferSampler;
}

const VkImageView& Renderer::Native::GetColorImageView()
{
	return PostProcessing::GetColorImageView();
}

bool& Renderer::GetForceAnimMatrixIdentity()
{
	return Native::bForceAnimMatrixIdentity;
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
	Native::gCurrentDraw.reset();
}

void Renderer::Native::DrawFade(uint8_t r, uint8_t g, uint8_t b, int a)
{
	bFadeActive = true;

	gFadeBuffer.GetBufferData().fadeColor = glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 127.0f);
	gFadeBuffer.Map(GetCurrentFrame());
}

void Renderer::Native::UpdateRenderPassKey(Renderer::Native::EClearMode clearMode)
{
	gCachedRenderPassKey.clearMode = clearMode;

	if (clearMode != EClearMode::None) {
		gRenderPassDirty = true;
	}
}
