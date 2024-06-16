#include "Mesh.h"
#include "ed3D.h"
#include "FileManager3D.h"
#include "Log.h"
#include "renderer.h"
#include "port.h"
#include "port/vu1_emu.h"

#define MESH_LOG(level, format, ...) MY_LOG_CATEGORY("MeshLibrary", level, format, ##__VA_ARGS__)
#define MESH_LOG_TRACE(level, format, ...) 

namespace Renderer
{
	namespace Kya
	{
		static MeshLibrary gMeshLibrary;

		static std::unordered_map<const ed_3d_strip*, Renderer::Kya::G3D::Strip*> gStripMap;

		static GIFReg::GSPrim ExtractPrimFromVifList(ed_3d_strip* pStrip)
		{
			// Pull the prim reg out from the gif packet, not a big fan of this.
			char* pVifList = reinterpret_cast<char*>(pStrip) + pStrip->vifListOffset;
			edpkt_data* pPkt = reinterpret_cast<edpkt_data*>(pVifList);
			u8* pGifPkt = LOAD_SECTION_CAST(u8*, pPkt[1].asU32[1]);
			Gif_Tag gifTag;
			gifTag.setTag(pGifPkt, true);

			MESH_LOG(LogLevel::Info, "ExtractPrimFromVifList Processing strip gifTag: NLOOP 0x{:x} NREG 0x{:x} PRIM 0x{:x}", (uint)gifTag.tag.NLOOP, (uint)gifTag.tag.NREG, (uint)gifTag.tag.PRIM);
			const uint primReg = gifTag.tag.PRIM;
			return *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);
		}
	}
}

void Renderer::Kya::G3D::Hierarchy::Lod::Object::Strip::KickVertices() const
{
	MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::::Strip::KickVertices vifListOffset: 0x{:x}", pStrip->vifListOffset);

	char* pVifList = reinterpret_cast<char*>(pStrip) + pStrip->vifListOffset;
	const uint incPacketSize = ed3DFlushStripGetIncPacket(pStrip, false, false);

	auto AddVertices = [this]() {
		char* vtxStart = VU1Emu::GetVertexDataStart();

		int* pFlag = reinterpret_cast<int*>(vtxStart - 0x10);

		Gif_Tag gifTag;
		gifTag.setTag((u8*)vtxStart, true);

		vtxStart += 0x10;

		for (int i = 0; i < gifTag.nLoop; i++) {
			const int addr = VU1Emu::GetExecVuCodeAddr();

			int drawMode = 0;

			if ((pStrip->flags & 0x400) != 0) {
				drawMode = 1;
			}

			Renderer::GSVertexUnprocessed vtx;
			memcpy(&vtx.STQ, vtxStart, sizeof(vtx.STQ));
			memcpy(&vtx.RGBA, vtxStart + 0x10, sizeof(vtx.RGBA));
			memcpy(&vtx.XYZSkip, vtxStart + 0x20, sizeof(vtx.XYZSkip));

			// Covert xyz 16
			if (drawMode == 1) {
				vtx.XYZSkip.fXYZ[0] = int12_to_float(vtx.XYZSkip.iXYZ[0]);
				vtx.XYZSkip.fXYZ[1] = int12_to_float(vtx.XYZSkip.iXYZ[1]);
				vtx.XYZSkip.fXYZ[2] = int12_to_float(vtx.XYZSkip.iXYZ[2]);
			}

			const uint vtxAnimMatrix = ((vtx.XYZSkip.Skip & 0x7ff) - 0x3dc) / 4;

			const uint primReg = gifTag.tag.PRIM;
			const GIFReg::GSPrim primPacked = *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);

			const uint skip = vtx.XYZSkip.Skip & 0x8000;

			//const uint shiftedStripIndex = stripIndex << 16;
			//vtx.XYZSkip.Skip |= shiftedStripIndex;
			vtx.XYZSkip.Skip |= (drawMode << 16);

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing vertex: {}, drawMode: {}, primPacked: 0x{:x}, skip: 0x{:x}", i, drawMode, primReg, skip);

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing vertex: {}, (S: {} T: {} Q: {}) (R: {} G: {} B: {} A: {}) (X: {} Y: {} Z: {} Skip: {})\n",
				i, vtx.STQ.ST[0], vtx.STQ.ST[1], vtx.STQ.Q, vtx.RGBA[0], vtx.RGBA[1], vtx.RGBA[2], vtx.RGBA[3], vtx.XYZSkip.fXYZ[0], vtx.XYZSkip.fXYZ[1], vtx.XYZSkip.fXYZ[2], vtx.XYZSkip.Skip);

			Renderer::KickVertex(vtx, primPacked, skip, pSimpleMesh->GetVertexBufferData());

			vtxStart += 0x30;
		};
	};

	uint partialMeshSectionCount = pStrip->meshCount % 3;
	ushort fullMeshSectionCount = pStrip->meshCount - partialMeshSectionCount;

	MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip partialMeshSectionCount: {}, fullMeshSectionCount: {}", partialMeshSectionCount, fullMeshSectionCount);

	if ((pStrip->flags & 4) == 0) {
		bool bCompletedPartial;

		while (bCompletedPartial = partialMeshSectionCount != 0, partialMeshSectionCount = partialMeshSectionCount - 1, bCompletedPartial) {
			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing partial mesh section {}", partialMeshSectionCount);

			VU1Emu::ProcessVifList((edpkt_data*)pVifList, false);
			AddVertices();
			pVifList = pVifList + incPacketSize * 0x10;
		}

		for (; fullMeshSectionCount != 0; fullMeshSectionCount = fullMeshSectionCount + -3) {
			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing full mesh section {}", fullMeshSectionCount);

			char* pVifListB = pVifList + incPacketSize * 0x10;
			char* pVifListC = pVifList + incPacketSize * 0x20;

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing full mesh section A");
			VU1Emu::ProcessVifList((edpkt_data*)pVifList, false);
			AddVertices();

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing full mesh section B");
			VU1Emu::ProcessVifList((edpkt_data*)pVifListB, false);
			AddVertices();

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing full mesh section C");
			VU1Emu::ProcessVifList((edpkt_data*)pVifListC, false);
			AddVertices();

			pVifList = pVifList + incPacketSize * 0x30;
		}
	}
}

void Renderer::Kya::G3D::Hierarchy::Lod::Object::CacheStrips()
{
	for (auto& strip : strips) {
		gStripMap[strip.pStrip] = &strip;
	}
}

void Renderer::Kya::G3D::Object::ProcessStrip(ed_3d_strip* pStrip, const int heirarchyIndex, const int lodIndex, const int stripIndex)
{
	assert(pStrip);

	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing strip flags: 0x{:x}", pStrip->flags);

	Strip& strip = strips.emplace_back();
	strip.pStrip = pStrip;
	strip.pParent = this;

	const GIFReg::GSPrim prim = ExtractPrimFromVifList(pStrip);

	// strip everything before the last forward slash 
	const std::string meshName = this->pParent->pParent->pParent->GetName().substr(this->pParent->pParent->pParent->GetName().find_last_of('\\') + 1);

	strip.pSimpleMesh = new SimpleMesh(meshName, heirarchyIndex, lodIndex, stripIndex, prim);
}

void Renderer::Kya::G3D::Lod::ProcessObject(ed_g3d_object* pObject, const int heirarchyIndex, const int lodIndex)
{
	object.pObject = pObject;
	object.pParent = this;

	if (pObject->p3DStrip) {
		ed_3d_strip* pStrip = LOAD_SECTION_CAST(ed_3d_strip*, pObject->p3DStrip);
		int stripIndex = 0;

		while (stripIndex < pObject->stripCount) {
			MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Hierarchy::Lod::ProcessObject Processing strip: {}", stripIndex);

			object.ProcessStrip(pStrip, heirarchyIndex, lodIndex, stripIndex);
			pStrip = LOAD_SECTION_CAST(ed_3d_strip*, pStrip->pNext);
			stripIndex++;
		}

		object.CacheStrips();
	}
}

void Renderer::Kya::G3D::Hierarchy::ProcessLod(ed3DLod* pLod, const int heirarchyIndex, const int lodIndex)
{
	assert(pLod);

	Lod& lod = lods.emplace_back();
	lod.pLod = pLod;
	lod.pParent = this;

	ed_hash_code* pHash = LOAD_SECTION_CAST(ed_hash_code*, pLod->pObj);
	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Hierarchy::ProcessLod Processing lod: {}", pHash->hash.ToString());

	ed_Chunck* pOBJ = LOAD_SECTION_CAST(ed_Chunck*, pHash->pData);

	if (pOBJ) {
		MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Hierarchy::ProcessLod Object chunk header: {}", pOBJ->GetHeaderString());

		ed_g3d_object* pObject = reinterpret_cast<ed_g3d_object*>(pOBJ + 1);
		lod.ProcessObject(pObject, heirarchyIndex, lodIndex);
	}
}

Renderer::Kya::G3D::G3D(ed_g3d_manager* pManager, std::string name)
	: pManager(pManager)
	, name(name)
{
	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::G3D Beginning processing of mesh: {}", name.c_str());

	ed_Chunck* pHASH = pManager->HALL + 1;
	ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(pHASH + 1);
	const int chunkNb = edChunckGetNb(pHASH, reinterpret_cast<char*>(pManager->HALL) + pManager->HALL->size);

	hierarchies.reserve(chunkNb);
	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::G3D Nb Chunks: {}", chunkNb);

	for (int curIndex = 0; curIndex < chunkNb - 1; curIndex = curIndex + 1) {
		MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::G3D Processing hierarchy {}/{} hash: {}", curIndex, chunkNb, pHashCode->hash.ToString());

		ed_g3d_hierarchy* pHierarchy = ed3DG3DHierarchyGetFromIndex(pManager, curIndex);

		if (pHierarchy) {
			ProcessHierarchy(pHierarchy, curIndex);
		}

		pHashCode++;
	}
}

void Renderer::Kya::G3D::ProcessHierarchy(ed_g3d_hierarchy* pHierarchy, const int heirarchyIndex)
{
	assert(pHierarchy);

	Hierarchy& hierarchy = hierarchies.emplace_back();
	hierarchy.pHierarchy = pHierarchy;
	hierarchy.pParent = this;

	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessHierarchy Processing hierarchy: {}", pHierarchy->hash.ToString());

	for (int i = 0; i < pHierarchy->lodCount; i++) {
		MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessHierarchy Processing lod: {}", i);

		ed3DLod* pLod = pHierarchy->aLods + i;

		hierarchy.ProcessLod(pLod, heirarchyIndex, heirarchyIndex);
	}
}

void Renderer::Kya::MeshLibrary::Init()
{
	ed3DGetMeshLoadedDelegate() += Renderer::Kya::MeshLibrary::AddMesh;
}

const Renderer::Kya::G3D::Strip* Renderer::Kya::MeshLibrary::FindStrip(const ed_3d_strip* pStrip) const
{
	return gStripMap[pStrip];

	int hierarchyIndex = 0;
	int lodIndex = 0;
	int stripIndex = 0;

	for (const auto& mesh : gMeshes) {
		for (const auto& hierarchy : mesh.GetHierarchies()) {
			for (const auto& lod : hierarchy.lods) {
				for (const auto& strip : lod.object.strips) {
					if (strip.pStrip == pStrip) {
						MESH_LOG(LogLevel::Info, "Renderer::Kya::MeshLibrary::FindStrip Found hierarchy: {}, lod: {}, strip: {}", hierarchyIndex, lodIndex, stripIndex);
						return &strip;
					}

					stripIndex++;
				}
			}

			lodIndex++;
		}

		hierarchyIndex++;
	}

	MESH_LOG(LogLevel::Info, "Renderer::Kya::MeshLibrary::FindStrip Strip not found");
	return nullptr;
}

void Renderer::Kya::MeshLibrary::AddFromStrip(const ed_3d_strip* pStrip) const
{
	const G3D::Strip* pRendererStrip = FindStrip(pStrip);
	//assert(pRendererStrip);

	if (pRendererStrip && pRendererStrip->pSimpleMesh) {
		pRendererStrip->KickVertices();
		Renderer::AddMesh(pRendererStrip->pSimpleMesh);
	}
}

void Renderer::Kya::MeshLibrary::AddMesh(ed_g3d_manager* pManager, std::string name)
{
	gMeshLibrary.gMeshes.emplace_back(pManager, name);
}

const Renderer::Kya::MeshLibrary& Renderer::Kya::GetMeshLibrary()
{
	return gMeshLibrary;
}
