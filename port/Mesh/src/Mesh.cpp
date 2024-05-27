#include "Mesh.h"
#include "ed3D.h"
#include "Log.h"
#include "renderer.h"
#include "port.h"

#define MESH_LOG(level, format, ...) MY_LOG_CATEGORY("MeshLibrary", level, format, ##__VA_ARGS__)

namespace Renderer
{
	namespace Kya
	{
		MeshLibrary gMeshLibrary;
	}
}

Renderer::Kya::G3D::G3D(ed_g3d_manager* pManager, std::string name)
	: pManager(pManager)
	, name(name)
{
	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::G3D Beginning processing of mesh: {}", name.c_str());

	
}

void Renderer::Kya::MeshLibrary::Init()
{
	ed3DGetMeshLoadedDelegate() += Renderer::Kya::MeshLibrary::AddMesh;
}

void Renderer::Kya::MeshLibrary::AddMesh(ed_g3d_manager* pManager, std::string name)
{
	gMeshLibrary.gMeshes.emplace_back(pManager, name);
}

const Renderer::Kya::MeshLibrary& Renderer::Kya::GetMeshLibrary()
{
	return gMeshLibrary;
}
