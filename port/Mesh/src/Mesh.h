#pragma once

#include <vector>
#include <string>
#include <functional>

struct ed_g3d_manager;

namespace Renderer
{
	struct SimpleTexture;

	namespace Kya 
	{
		class G3D
		{
		public:
			G3D(ed_g3d_manager* pManager, std::string name);

			inline const std::string& GetName() const { return name; }
			inline ed_g3d_manager* GetManager() const { return pManager; }

		private:
			std::string name;
			ed_g3d_manager* pManager = nullptr;
		};

		class MeshLibrary
		{
		public:
			using ForEachMesh = std::function<void(const G3D&)>;

			static void Init();

			inline void ForEach(ForEachMesh func) const
			{
				for (const auto& texture : gMeshes)
				{
					func(texture);
				}
			}

			inline int GetMeshCount() const { return gMeshes.size(); }

		private:
			static void AddMesh(ed_g3d_manager* pManager, std::string name);

			std::vector<G3D> gMeshes;
		};

		const MeshLibrary& GetMeshLibrary();
	}
}
