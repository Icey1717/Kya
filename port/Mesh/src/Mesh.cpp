 #include "Mesh.h"
#include "ed3D.h"
#include "Log.h"
#include "renderer.h"
#include "port.h"
#include "port/vu1_emu.h"

#define MESH_LOG(level, format, ...) MY_LOG_CATEGORY("MeshLibrary", level, format, ##__VA_ARGS__)
//#define MESH_LOG_TRACE(level, format, ...) MY_LOG_CATEGORY("MeshLibrary", level, format, ##__VA_ARGS__)
#define MESH_LOG_TRACE(level, format, ...)

namespace Renderer
{
	namespace Kya
	{
		constexpr uint32_t gGifTagCopyCode = 0x6c018000;

		static MeshLibrary gMeshLibrary;

		using StripCache = std::unordered_map<const ed_3d_strip*, Renderer::Kya::G3D::Strip*>;
		static StripCache gStripCache;

		static std::unordered_map<const ed_3d_strip*, Renderer::Kya::G3D::Object*> gObjectCache;

		static Gif_Tag ExtractGifTagFromVifList(ed_3d_strip* pStrip, int index = 0)
		{
			// Pull the prim reg out from the gif packet, not a big fan of this.
			char* const pVifList = reinterpret_cast<char*>(pStrip) + pStrip->vifListOffset;
			edpkt_data* pPkt = reinterpret_cast<edpkt_data*>(pVifList);

			while (index > 0) {
				if (pPkt->asU32[0] == gVifEndCode) {
					index--;
				}

				pPkt++;
			}

			if (pPkt[1].asU32[3] != gGifTagCopyCode) {
				pPkt = reinterpret_cast<edpkt_data*>(pVifList);
			}

			assert(pPkt[1].asU32[3] == gGifTagCopyCode);

			u8* const pGifPkt = LOAD_SECTION_CAST(u8*, pPkt[1].asU32[1]);
			Gif_Tag gifTag;
			gifTag.setTag(pGifPkt, true);
			return gifTag;
		}

		static void EmplaceHierarchy(std::vector<G3D::Hierarchy>& hierarchies, ed_g3d_hierarchy* pHierarchy, const int heirarchyIndex, G3D* pParent)
		{
			assert(pHierarchy);

			G3D::Hierarchy& hierarchy = hierarchies.emplace_back();
			hierarchy.pHierarchy = pHierarchy;
			hierarchy.pParent = pParent; // Probably should have the cluster as its parent.

			MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessHierarchy Processing hierarchy: {}", pHierarchy->hash.ToString());

			for (int i = 0; i < pHierarchy->lodCount; i++) {
				MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessHierarchy Processing lod: {}", i);

				ed3DLod* pLod = pHierarchy->aLods + i;

				hierarchy.ProcessLod(pLod, heirarchyIndex, i);
			}
		}

		enum class DrawMode {
			v12,
			v32
		};

		static DrawMode GetDrawMode(ed_3d_strip* pStrip)
		{
			if ((pStrip->flags & 0x400) != 0) {
				return DrawMode::v12;
			}

			return DrawMode::v32;
		}
	}
}

constexpr const char* gDebugMeshName = "SECT1.g3d_17_0_0";

void Renderer::Kya::G3D::Strip::PreProcessVertices()
{
	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Object::Strip::PreProcessVertices Processing strip name: {}", pSimpleMesh->GetName());

	if (pSimpleMesh->GetName() == gDebugMeshName) {
		MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Object::Strip::PreProcessVertices Processing strip name: {}", pSimpleMesh->GetName());
	}

	const Gif_Tag firstGifTag = ExtractGifTagFromVifList(pStrip);

	const DrawMode drawMode = GetDrawMode(pStrip);

	auto& vertexBufferData = pSimpleMesh->GetVertexBufferData();

	// Assume that the first gif tag has the largest vtx count.
	int totalVtxCount = 0;

	for (int j = 0; j < pStrip->meshCount; j++) {
		Gif_Tag gifTag = ExtractGifTagFromVifList(pStrip, j);
		totalVtxCount += gifTag.nLoop;
	}

	assert(totalVtxCount > 0);

	vertexBufferData.Init(totalVtxCount * 2, totalVtxCount * 4);

	union VertexColor {
		uint32_t rgba;

		struct {
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};
	};

	static_assert(sizeof(VertexColor) == 4);

	union TextureData {
		uint32_t st;

		struct {
			int16_t s;
			int16_t t;
		};
	};

	static_assert(sizeof(TextureData) == 4);

	VertexColor* pRgba = LOAD_SECTION_CAST(VertexColor*, pStrip->pColorBuf);
	TextureData* pStq = LOAD_SECTION_CAST(TextureData*, pStrip->pSTBuf);
	pStq += 4;

	// This increases by 2 every loop because we start the next vtx at the end of the previous vtx.
	int vtxOffset = 0;

	int meshOffset = 0;

	for (int j = 0; j < pStrip->meshCount; j++) {
		MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Strip::PreProcessVertices Starting section: {}", j);
		Gif_Tag gifTag = ExtractGifTagFromVifList(pStrip, j);

		for (int i = 0; i < gifTag.nLoop; i++) {
			const int index = i + meshOffset;
			const int adjustedIndex = index - vtxOffset;

			Renderer::GSVertexUnprocessedNormal vtx;
			vtx.RGBA[0] = pRgba[index].r;
			vtx.RGBA[1] = pRgba[index].g;
			vtx.RGBA[2] = pRgba[index].b;
			vtx.RGBA[3] = pRgba[index].a;

			vtx.STQ.ST[0] = pStq[index].s;
			vtx.STQ.ST[1] = pStq[index].t;
			vtx.STQ.Q = 1.0f;

			if (pStrip->field_0x2c) {
				struct Normal15 {
					int16_t x;
					int16_t y;
					int16_t z;
					int16_t pad;
				};

				Normal15* pNormal = LOAD_SECTION_CAST(Normal15*, pStrip->field_0x2c);

				vtx.normal.fNormal[0] = int15_to_float(pNormal[adjustedIndex].x);
				vtx.normal.fNormal[1] = int15_to_float(pNormal[adjustedIndex].y);
				vtx.normal.fNormal[2] = int15_to_float(pNormal[adjustedIndex].z);
				vtx.normal.fNormal[3] = int15_to_float(pNormal[adjustedIndex].pad);

				MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Strip::PreProcessVertices Processing vertex: {}, normal: ({}, {}, {})", i, vtx.normal.fNormal[0], vtx.normal.fNormal[1], vtx.normal.fNormal[2]);
			}
			else {
				memset(&vtx.normal, 0, sizeof(vtx.normal));
			}

			// Covert xyz 16
			if (drawMode == DrawMode::v12) {
				struct Vertex12 {
					int16_t x;
					int16_t y;
					int16_t z;
					int16_t flags;
				};

				Vertex12* pVertex = LOAD_SECTION_CAST(Vertex12*, pStrip->pVertexBuf);

				vtx.XYZFlags.fXYZ[0] = int12_to_float(pVertex[adjustedIndex].x);
				vtx.XYZFlags.fXYZ[1] = int12_to_float(pVertex[adjustedIndex].y);
				vtx.XYZFlags.fXYZ[2] = int12_to_float(pVertex[adjustedIndex].z);
				vtx.XYZFlags.flags = pVertex[adjustedIndex].flags;
			}
			else {
				GSVertexUnprocessed::Vertex* pVertex = LOAD_SECTION_CAST(GSVertexUnprocessed::Vertex*, pStrip->pVertexBuf);
				vtx.XYZFlags = pVertex[adjustedIndex];
			}

			const uint primReg = firstGifTag.tag.PRIM;
			const GIFReg::GSPrim primPacked = *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);

			const uint skip = vtx.XYZFlags.flags & 0x8000;

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Strip::PreProcessVertices Processing vertex: {}, drawMode: {}, primPacked: 0x{:x}, nloop: 0x{:x}, skip: 0x{:x}", i, (int)drawMode, primReg, gifTag.nLoop, skip);

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Strip::PreProcessVertices Processing vertex: {}, (S: {} T: {} Q: {}) (R: {} G: {} B: {} A: {}) (X: {} Y: {} Z: {} Skip: {})\n",
				i, vtx.STQ.ST[0], vtx.STQ.ST[1], vtx.STQ.Q, vtx.RGBA[0], vtx.RGBA[1], vtx.RGBA[2], vtx.RGBA[3], vtx.XYZFlags.fXYZ[0], vtx.XYZFlags.fXYZ[1], vtx.XYZFlags.fXYZ[2], vtx.XYZFlags.flags);

			Renderer::KickVertex(vtx, primPacked, skip, vertexBufferData);

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Strip::PreProcessVertices Kick complete vtx tail: 0x{:x} index tail: 0x{:x}",
				vertexBufferData.GetVertexTail(), vertexBufferData.GetIndexTail());
		}

		meshOffset += gifTag.nLoop;
		vtxOffset += 2;
	}

	//assert(internalVertexBuffer.GetIndexTail() > 0);
}

void Renderer::Kya::G3D::Strip::KickVertices() const
{
	MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::::Strip::KickVertices {} vifListOffset: 0x{:x}", pSimpleMesh->GetName(), pStrip->vifListOffset);

	if (pSimpleMesh->GetName() == gDebugMeshName) {
		MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Object::Strip::KickVertices Processing strip name: {}", pSimpleMesh->GetName());
	}

	char* pVifList = reinterpret_cast<char*>(pStrip) + pStrip->vifListOffset;
	const uint incPacketSize = ed3DFlushStripGetIncPacket(pStrip, false, false);

	const DrawMode drawMode = GetDrawMode(pStrip);

	char* pStqData = LOAD_SECTION_CAST(char*, pStrip->pSTBuf);

	auto AddVertices = [this, drawMode]() {
		char* pVtx = VU1Emu::GetVertexDataStart();

		bool bProcessNormal = false;

		if (const int addr = VU1Emu::GetExecVuCodeAddr(); addr == 0xfc || addr == 0x19) {
			bProcessNormal = true;
		}

		int* pFlag = reinterpret_cast<int*>(pVtx - 0x10);

		Gif_Tag gifTag;
		gifTag.setTag((u8*)pVtx, true);

		pVtx += 0x10;

		char* pNormal = pVtx + 0xd80;

		for (int i = 0; i < gifTag.nLoop; i++) {
			const int addr = VU1Emu::GetExecVuCodeAddr();

			Renderer::GSVertexUnprocessedNormal vtx;
			memcpy(&vtx.STQ, pVtx, sizeof(vtx.STQ));
			memcpy(&vtx.RGBA, pVtx + 0x10, sizeof(vtx.RGBA));
			memcpy(&vtx.XYZFlags, pVtx + 0x20, sizeof(vtx.XYZFlags));

			if (bProcessNormal) {
				memcpy(&vtx.normal, pNormal, sizeof(vtx.normal));

				vtx.normal.fNormal[0] = int15_to_float(vtx.normal.iNormal[0]);
				vtx.normal.fNormal[1] = int15_to_float(vtx.normal.iNormal[1]);
				vtx.normal.fNormal[2] = int15_to_float(vtx.normal.iNormal[2]);

				MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Strip::KickVertices Processing vertex: {}, normal: ({}, {}, {})", i, vtx.normal.fNormal[0], vtx.normal.fNormal[1], vtx.normal.fNormal[2]);
			}
			else {
				memset(&vtx.normal, 0, sizeof(vtx.normal));
			}

			// Covert xyz 16
			if (drawMode == DrawMode::v12) {
				vtx.XYZFlags.fXYZ[0] = int12_to_float(vtx.XYZFlags.iXYZ[0]);
				vtx.XYZFlags.fXYZ[1] = int12_to_float(vtx.XYZFlags.iXYZ[1]);
				vtx.XYZFlags.fXYZ[2] = int12_to_float(vtx.XYZFlags.iXYZ[2]);
			}

			const uint primReg = gifTag.tag.PRIM;
			const GIFReg::GSPrim primPacked = *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);

			const uint skip = vtx.XYZFlags.flags & 0x8000;

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing vertex: {}, drawMode: {}, primPacked: 0x{:x}, nloop: 0x{:x}, skip: 0x{:x}", i, (int)drawMode, primReg, gifTag.nLoop, skip);

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing vertex: {}, (S: {} T: {} Q: {}) (R: {} G: {} B: {} A: {}) (X: {} Y: {} Z: {} Skip: {})\n",
				i, vtx.STQ.ST[0], vtx.STQ.ST[1], vtx.STQ.Q, vtx.RGBA[0], vtx.RGBA[1], vtx.RGBA[2], vtx.RGBA[3], vtx.XYZFlags.fXYZ[0], vtx.XYZFlags.fXYZ[1], vtx.XYZFlags.fXYZ[2], vtx.XYZFlags.flags);

			Renderer::KickVertex(vtx, primPacked, skip, pSimpleMesh->GetVertexBufferData());

			MESH_LOG_TRACE(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Kick complete vtx tail: 0x{:x} index tail: 0x{:x}",
				pSimpleMesh->GetVertexBufferData().GetVertexTail(), pSimpleMesh->GetVertexBufferData().GetIndexTail());

			pVtx += 0x30;
			pNormal += 0x10;
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

void Renderer::Kya::G3D::Cluster::ProcessStrip(ed_3d_strip* pStrip, const int stripIndex)
{
	assert(pStrip);

	assert(pStrip->meshCount > 0);

	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Cluster::ProcessStrip Processing strip flags: 0x{:x}", pStrip->flags);

	Strip& strip = strips.emplace_back();
	strip.pStrip = pStrip;
	strip.pParent = this;

	const Gif_Tag gifTag  = ExtractGifTagFromVifList(pStrip);

	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Cluster::ProcessStrip Processing strip gifTag: NLOOP 0x{:x} NREG 0x{:x} PRIM 0x{:x}", (uint)gifTag.tag.NLOOP, (uint)gifTag.tag.NREG, (uint)gifTag.tag.PRIM);
	const uint primReg = gifTag.tag.PRIM;
	const GIFReg::GSPrim prim = *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);

	// strip everything before the last forward slash
	std::string meshName = this->pParent->GetName().substr(this->pParent->GetName().find_last_of('\\') + 1);
	meshName += "_";
	meshName += std::to_string(stripIndex);

	strip.pSimpleMesh = std::make_unique<SimpleMesh>(meshName, prim);

	strip.PreProcessVertices();
}

void Renderer::Kya::G3D::Cluster::CacheStrips()
{
	for (auto& strip : strips) {
		gStripCache[strip.pStrip] = &strip;
	}
}

void Renderer::Kya::G3D::Cluster::ProcessHierarchy(ed_g3d_hierarchy* pHierarchy, const int heirarchyIndex)
{
	EmplaceHierarchy(hierarchies, pHierarchy, heirarchyIndex, pParent);
}

void Renderer::Kya::G3D::Object::ProcessStrip(ed_3d_strip* pStrip, const int heirarchyIndex, const int lodIndex, const int stripIndex)
{
	assert(pStrip);

	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing strip flags: 0x{:x}", pStrip->flags);

	Strip& strip = strips.emplace_back();
	strip.pStrip = pStrip;
	strip.pParent = this;

	Gif_Tag gifTag = ExtractGifTagFromVifList(pStrip);

	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Object::ProcessStrip Processing strip gifTag: NLOOP 0x{:x} NREG 0x{:x} PRIM 0x{:x}", (uint)gifTag.tag.NLOOP, (uint)gifTag.tag.NREG, (uint)gifTag.tag.PRIM);
	const uint primReg = gifTag.tag.PRIM;
	const GIFReg::GSPrim prim = *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);

	// strip everything before the last forward slash
	std::string meshName = this->pParent ? this->pParent->pParent->pParent->GetName().substr(this->pParent->pParent->pParent->GetName().find_last_of('\\') + 1) :
		"None";
	meshName += "_";
	meshName += std::to_string(heirarchyIndex);
	meshName += "_";
	meshName += std::to_string(lodIndex);
	meshName += "_";
	meshName += std::to_string(stripIndex);

	strip.pSimpleMesh = std::make_unique<SimpleMesh>(meshName, prim);

	strip.PreProcessVertices();
}

void Renderer::Kya::G3D::Hierarchy::Lod::Object::CacheStrips()
{
	for (auto& strip : strips) {
		gStripCache[strip.pStrip] = &strip;
	}
}

void Renderer::Kya::G3D::Lod::ProcessObject(ed_g3d_object* pObject, const int heirarchyIndex, const int lodIndex)
{
	object.pObject = pObject;
	object.pParent = this;

	if (pObject->p3DData) {
		ed_3d_strip* pStrip = LOAD_SECTION_CAST(ed_3d_strip*, pObject->p3DData);
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

	if (pLod->pObj) {
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
	else {
		MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Hierarchy::ProcessLod No lod data");
	}
}

Renderer::Kya::G3D::G3D(ed_g3d_manager* pManager, std::string name)
	: pManager(pManager)
	, name(name)
{
	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::G3D Beginning processing of mesh: {}", name.c_str());

	if (pManager->HALL) {
		ProcessHALL();
	}

	if (pManager->CSTA) {
		ProcessCSTA();
	}
}

void Renderer::Kya::G3D::ProcessHierarchy(ed_g3d_hierarchy* pHierarchy, const int heirarchyIndex)
{
	EmplaceHierarchy(hierarchies, pHierarchy, heirarchyIndex, this);
}

void Renderer::Kya::G3D::ProcessHALL()
{
	assert(pManager->HALL);

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

void Renderer::Kya::G3D::ProcessCluster(ed_g3d_cluster* pCluster)
{
	assert(pCluster);

	cluster.pData = pCluster;
	cluster.pParent = this;

	const uint stripCountArrayEntryIndex = 4;

	const uint stripCount = pCluster->aClusterStripCounts[stripCountArrayEntryIndex];

	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessCluster Processing CDQU chunk stripCount: {}", stripCount);

	bool bProcessedStrip = false;

	if ((stripCount != 0) && (bProcessedStrip = true, stripCount != 0)) {
		ed_Chunck* pMBNK = LOAD_SECTION_CAST(ed_Chunck*, pCluster->pMBNK);
		ed_3d_strip* p3DStrip = LOAD_SECTION_CAST(ed_3d_strip*, pCluster->p3DStrip);

		uint stripIndex = 0;

		while (stripIndex < stripCount) {
			MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::Hierarchy::Lod::ProcessObject Processing strip: {}", stripIndex);

			cluster.ProcessStrip(p3DStrip, stripIndex);
			p3DStrip = LOAD_SECTION_CAST(ed_3d_strip*, p3DStrip->pNext);
			stripIndex++;
		}

		cluster.CacheStrips();

		bProcessedStrip = true;
	}

	uint spriteCount = pCluster->clusterDetails.spriteCount;

	if (spriteCount != 0) {

	}

	uint clusterHierCount = pCluster->clusterDetails.clusterHierCount;

	if (clusterHierCount != 0) {
		ed_Chunck* pHASH = reinterpret_cast<ed_Chunck*>(pCluster + 1);
		ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(pHASH + 1);

		for (int i = 0; i < clusterHierCount; i++) {
			MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessCluster Processing cluster hierarchy: {}", pHashCode->hash.ToString());

			ed_Chunck* pHIER = LOAD_SECTION_CAST(ed_Chunck*, pHashCode->pData);
			assert(pHIER->hash == HASH_CODE_HIER);

			ed_g3d_hierarchy* pHierarchy = reinterpret_cast<ed_g3d_hierarchy*>(pHIER + 1);

			if (pHierarchy) {
				cluster.ProcessHierarchy(pHierarchy, i);
			}

			pHashCode++;
		}
	}

	//assert(bProcessedStrip);
}

void Renderer::Kya::G3D::ProcessCSTA()
{
	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessCSTA CSTA chunk header: {}", pManager->CSTA->GetHeaderString());

	ed_Chunck* pClusterTypeChunk = pManager->CSTA + 1;

	MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessCSTA Cluster chunk header: {}", pClusterTypeChunk->GetHeaderString());

	if (pClusterTypeChunk->hash == HASH_CODE_CDOA) {
		assert(false);
		MeshData_CSTA* pCSTA = reinterpret_cast<MeshData_CSTA*>(pClusterTypeChunk + 1);


	}
	else {
		if (pClusterTypeChunk->hash == HASH_CODE_CDQA) {
			MeshData_CSTA* pCSTA = reinterpret_cast<MeshData_CSTA*>(pClusterTypeChunk + 1);

			MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessCSTA Processing CDQA chunk");

			MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessCSTA field_0x20: {}", pCSTA->field_0x20.ToString());
			MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessCSTA worldLocation: {}", pCSTA->worldLocation.ToString());

			ed_Chunck* pCDQU = edChunckGetFirst(pCSTA + 1, reinterpret_cast<char*>(pClusterTypeChunk) + pClusterTypeChunk->size);
			MESH_LOG(LogLevel::Info, "Renderer::Kya::G3D::ProcessCSTA CDQA chunk header: {}", pCDQU->GetHeaderString());

			if (pCDQU) {
				ed_g3d_cluster* pCDQUData = reinterpret_cast<ed_g3d_cluster*>(pCDQU + 1);
				ProcessCluster(pCDQUData);
			}
		}
	}
}

void Renderer::Kya::MeshLibrary::Init()
{
	ed3DGetMeshLoadedDelegate() += Renderer::Kya::MeshLibrary::AddMesh;
}

const Renderer::Kya::G3D::Strip* Renderer::Kya::MeshLibrary::FindStrip(const ed_3d_strip* pStrip) const
{
	constexpr bool bUseStripCache = true;

	if (bUseStripCache) {
		assert(gStripCache.find(pStrip) != gStripCache.end());
		return gStripCache[pStrip];
	}
	
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

void Renderer::Kya::MeshLibrary::RenderNode(const edNODE* pNode) const
{
	ed_3d_strip* pStrip = reinterpret_cast<ed_3d_strip*>(pNode->pData);

	const G3D::Strip* pRendererStrip = FindStrip(pStrip);
	assert(pRendererStrip);

	if (pRendererStrip && pRendererStrip->pSimpleMesh) {	
		Renderer::RenderMesh(pRendererStrip->pSimpleMesh.get(), pNode->header.typeField.flags);
	}
	else {
		MESH_LOG(LogLevel::Error, "Renderer::Kya::MeshLibrary::RenderNode Strip not found or no simple mesh available for rendering");
	}
}

void Renderer::Kya::MeshLibrary::CacheDlistStrip(ed_3d_strip* pStrip)
{
	if (gObjectCache.find(pStrip) == gObjectCache.end()) {
		gObjectCache[pStrip] = new Renderer::Kya::G3D::Object();
	}

	auto* pObj = gObjectCache[pStrip];
	pObj->strips.clear();
	pObj->ProcessStrip(pStrip, 0, 0, 0);
	pObj->CacheStrips();
}

void Renderer::Kya::MeshLibrary::AddMesh(ed_g3d_manager* pManager, std::string name)
{
	gMeshLibrary.gMeshes.emplace_back(pManager, name);
}

const Renderer::Kya::MeshLibrary& Renderer::Kya::GetMeshLibrary()
{
	return gMeshLibrary;
}

Renderer::Kya::MeshLibrary& Renderer::Kya::GetMeshLibraryMutable()
{
	return gMeshLibrary;
}
