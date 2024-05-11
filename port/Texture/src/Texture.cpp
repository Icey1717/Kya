#include "Texture.h"
#include "ed3D.h"
#include "Log.h"

#define TEXTURE_LOG(level, format, ...) MY_LOG_CATEGORY("Texture", level, format, ##__VA_ARGS__)

namespace Renderer
{
	namespace Kya
	{
		TextureLibrary gTextureLibrary;
	}
}

Renderer::Kya::Texture::Texture(ed_g2d_manager* pManager, std::string name)
	: pManager(pManager)
	, name(name)
{
	TEXTURE_LOG(LogLevel::Info, "Renderer::Kya::Texture::Texture Beginning processing of texture: {}", name.c_str());


}

void Renderer::Kya::TextureLibrary::Init()
{
	ed3DGetTextureLoadedDelegate() += Renderer::Kya::TextureLibrary::AddTexture;
}

void Renderer::Kya::TextureLibrary::AddTexture(ed_g2d_manager* pManager, std::string name)
{
	gTextureLibrary.gTextures.emplace_back(pManager, name);
}

const Renderer::Kya::TextureLibrary& Renderer::Kya::GetTextureLibrary()
{
	return gTextureLibrary;
}