#pragma once

#include <vector>
#include <string>
#include <functional>

struct ed_g3d_manager;
struct ed_g3d_hierarchy;
struct ed3DLod;
struct ed_g3d_object;
struct ed_3d_strip;
struct ed_g3d_cluster;
struct edNODE;

#include <memory>

namespace Renderer
{
	struct SimpleMesh;

	namespace Kya 
	{
		class G3D
		{
		public:

			struct Strip
			{
				void PreProcessVertices();
				void KickVertices() const;

				ed_3d_strip* pStrip = nullptr;
				void* pParent = nullptr;
				std::unique_ptr<SimpleMesh> pSimpleMesh;
			};

			struct Hierarchy {
				struct Lod {
					struct Object {
						void ProcessStrip(ed_3d_strip* pStrip, const int heirarchyIndex, const int lodIndex, const int stripIndex);
						void CacheStrips();

						ed_g3d_object* pObject = nullptr;
						Lod* pParent = nullptr;
						std::vector<Strip> strips;
					};

					void ProcessObject(ed_g3d_object* pObject, const int heirarchyIndex, const int lodIndex);

					ed3DLod* pLod = nullptr;
					Hierarchy* pParent = nullptr;
					Object object;
				};

				void ProcessLod(ed3DLod* pLod, const int heirarchyIndex, const int lodIndex);

				ed_g3d_hierarchy* pHierarchy = nullptr;
				G3D* pParent = nullptr;
				std::vector<Lod> lods;
			};

			struct Cluster {
				void ProcessStrip(ed_3d_strip* pStrip, const int stripIndex);
				void CacheStrips();

				void ProcessHierarchy(ed_g3d_hierarchy* pHierarchy, const int heirarchyIndex);

				ed_g3d_cluster* pData = nullptr;
				G3D* pParent = nullptr;
				std::vector<Strip> strips;
				std::vector<Hierarchy> hierarchies;
			};

			using Lod = G3D::Hierarchy::Lod;
			using Object = G3D::Hierarchy::Lod::Object;

			G3D(ed_g3d_manager* pManager, std::string name);

			inline const std::string& GetName() const { return name; }
			inline ed_g3d_manager* GetManager() const { return pManager; }

			inline const std::vector<Hierarchy>& GetHierarchies() const { return hierarchies; }

		private:
			void ProcessHierarchy(ed_g3d_hierarchy* pHierarchy, const int heirarchyIndex);
			void ProcessHALL();

			void ProcessCluster(ed_g3d_cluster* pCDQUData);
			void ProcessCSTA();

			std::string name;
			ed_g3d_manager* pManager = nullptr;

			std::vector<Hierarchy> hierarchies;
			Cluster cluster;
		};

		class MeshLibrary
		{
		public:
			using ForEachMesh = std::function<void(const G3D&)>;

			static void Init();
			void RenderNode(const edNODE* pNode) const;
			void CacheDlistStrip(ed_3d_strip* pStrip);

			// Debug
			inline void ForEach(ForEachMesh func) const
			{
				for (const auto& texture : gMeshes)
				{
					func(texture);
				}
			}

			inline int GetMeshCount() const { return gMeshes.size(); }

			const G3D::Strip* FindStrip(const ed_3d_strip* pStrip) const;
		private:
			static void AddMesh(ed_g3d_manager* pManager, std::string name);

			std::vector<G3D> gMeshes;
		};

		const MeshLibrary& GetMeshLibrary();
		MeshLibrary& GetMeshLibraryMutable();
	}
}
