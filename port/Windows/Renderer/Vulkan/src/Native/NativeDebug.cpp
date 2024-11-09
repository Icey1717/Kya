#include "NativeDebug.h"
#include "renderer.h"


namespace Renderer
{
	namespace Native
	{
		struct LabelState {
			std::string lastTextureName;
			int lastMaterialIndex = -1;
			int lastLayerIndex = -1;
		} gLabelState;

	} // Native
} // Renderer

void Renderer::Native::Debug::Reset()
{
	gLabelState = {};
}

void Renderer::Native::Debug::UpdateLabel(const SimpleTexture* pTexture, const VkCommandBuffer& cmd)
{
	if (gLabelState.lastTextureName != pTexture->GetName())
	{
		if (!gLabelState.lastTextureName.empty())
		{
			Renderer::Debug::EndLabel(cmd);

			if (gLabelState.lastMaterialIndex != -1) {
				Renderer::Debug::EndLabel(cmd);
				gLabelState.lastMaterialIndex = -1;
			}

			if (gLabelState.lastLayerIndex != -1) {
				Renderer::Debug::EndLabel(cmd);
				gLabelState.lastLayerIndex = -1;
			}
		}

		Renderer::Debug::BeginLabel(cmd, "Native %s", pTexture->GetName().c_str());

		gLabelState.lastTextureName = pTexture->GetName();
	}

	if (gLabelState.lastMaterialIndex != pTexture->GetMaterialIndex() && pTexture->GetMaterialCount() > 1) {
		if (gLabelState.lastMaterialIndex != -1) {
			Renderer::Debug::EndLabel(cmd);

			if (gLabelState.lastLayerIndex != -1) {
				Renderer::Debug::EndLabel(cmd);
				gLabelState.lastLayerIndex = -1;
			}
		}

		Renderer::Debug::BeginLabel(cmd, "Material %d", pTexture->GetMaterialIndex());
		gLabelState.lastMaterialIndex = pTexture->GetMaterialIndex();
	}

	if (gLabelState.lastLayerIndex != pTexture->GetLayerIndex() && pTexture->GetLayerCount() > 1) {
		if (gLabelState.lastLayerIndex != -1) {
			Renderer::Debug::EndLabel(cmd);
		}

		Renderer::Debug::BeginLabel(cmd, "Layer %d", pTexture->GetLayerIndex());
		gLabelState.lastLayerIndex = pTexture->GetLayerIndex();
	}
}