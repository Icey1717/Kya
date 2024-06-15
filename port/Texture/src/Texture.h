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

struct ed_dma_material;

namespace Renderer
{
	struct SimpleTexture;

	namespace Kya 
	{
		class G2D
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
							ed_g2d_bitmap* GetBitmap() const { return pBitmap; }

							CommandList* GetUploadCommands() { return uploadCommands; }

							// Upload commands are double bufferred but we only use the first one.
							CommandList GetUploadCommandsDefault() { return uploadCommands[0]; }
						private:
							void UpdateCommands();

							ed_g2d_bitmap* pBitmap = nullptr;
							CommandList uploadCommands[2];
						};

						ed_g2d_texture* pTexture = nullptr;
						Layer* pParent = nullptr;

						Bitmap bitmap;
						Bitmap palette;

						SimpleTexture* pSimpleTexture = nullptr;
					};

					void ProcessTexture(ed_g2d_texture* pTexture, const int materialIndex, const int layerIndex);

					ed_g2d_layer* pLayer = nullptr;
					Material* pParent = nullptr;

					std::vector<Texture> textures;
				};

				void ProcessLayer(ed_g2d_layer* pLayer, const int materialIndex);

				SimpleTexture* FindRenderTextureFromBitmap(ed_g2d_bitmap* pBitmap) const;

				bool GetInUse() const;

				ed_g2d_material* pMaterial = nullptr;
				G2D* pParent = nullptr;

				CommandList renderCommands;
				std::vector<Layer> layers;
			};

			using Layer = G2D::Material::Layer;
			using Texture = G2D::Material::Layer::Texture;
			using Bitmap = G2D::Material::Layer::Texture::Bitmap;

			G2D(ed_g2d_manager* pManager, std::string name);

			inline const std::string& GetName() const { return name; }
			inline ed_g2d_manager* GetManager() const { return pManager; }

			inline const std::vector<Material>& GetMaterials() const { return materials; }

		private:
			void ProcessMaterial(ed_g2d_material* pMaterial, const int materialIndex);

			std::string name;
			ed_g2d_manager* pManager = nullptr;

			std::vector<Material> materials;
		};

		class TextureLibrary
		{
		public:
			using ForEachTexture = std::function<void(const G2D&)>;

			static void Init();

			inline void ForEach(ForEachTexture func) const
			{
				for (const auto& texture : gTextures)
				{
					func(texture);
				}
			}

			const G2D::Material* FindMaterial(const ed_g2d_material* pMaterial) const;

			void BindFromDmaMaterial(const ed_dma_material* pMaterial) const;

			inline int GetTextureCount() const { return gTextures.size(); }

		private:
			static void AddTexture(ed_g2d_manager* pManager, std::string name);

			std::vector<G2D> gTextures;
		};

		const TextureLibrary& GetTextureLibrary();
	}
}
