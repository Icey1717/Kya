#pragma once

#include <vector>
#include <string>
#include <functional>

struct ed_g2d_manager;
struct ed_g2d_material;
struct ed_g2d_layer;
struct ed_g2d_texture;
struct ed_g2d_bitmap;
union edpkt_data;

namespace Renderer
{
	namespace Kya 
	{
		class Texture
		{
		public:
			struct CommandList
			{
				edpkt_data* pList = nullptr;
				size_t size = 0;
			};

			struct Material
			{
				struct Layer
				{
					struct Texture
					{
						struct Bitmap {
							void SetBitmap(ed_g2d_bitmap* pBitmap);
						private:
							void UpdateCommands();

							ed_g2d_bitmap* pBitmap = nullptr;
							CommandList uploadCommands[2];
						};

						ed_g2d_texture* pTexture = nullptr;

						Bitmap bitmap;
						Bitmap palette;
					};

					void ProcessTexture(ed_g2d_texture* pTexture);

					ed_g2d_layer* pLayer = nullptr;
					std::vector<Texture> textures;
				};

				void ProcessLayer(ed_g2d_layer* pLayer);

				ed_g2d_material* pMaterial = nullptr;
				CommandList renderCommands;
				std::vector<Layer> layers;
			};

			Texture(ed_g2d_manager* pManager, std::string name);

			inline const std::string& GetName() const { return name; }
			inline ed_g2d_manager* GetManager() const { return pManager; }

		private:
			void ProcessMaterial(ed_g2d_material* pMaterial);

			std::string name;
			ed_g2d_manager* pManager = nullptr;

			std::vector<Material> materials;
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
