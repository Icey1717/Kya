#pragma once

#include <vector>
#include <string>
#include <functional>

struct ed_g2d_manager;

namespace Renderer
{
	namespace Kya 
	{
		class Texture
		{
		public:
			Texture(ed_g2d_manager* pManager, std::string name);

			inline const std::string& GetName() const { return name; }
			inline const ed_g2d_manager* GetManager() const { return pManager; }
		private:
			std::string name;
			ed_g2d_manager* pManager;
		};

		class TextureLibrary
		{
		public:
			using ForEachTexture = std::function<void(const Texture&)>;

			static void Init();

			inline void ForEach(ForEachTexture func) const
			{
				for (const auto& texture : gTextures)
				{
					func(texture);
				}
			}

		private:
			static void AddTexture(ed_g2d_manager* pManager, std::string name);

			std::vector<Texture> gTextures;
		};

		const TextureLibrary& GetTextureLibrary();
	}
}
