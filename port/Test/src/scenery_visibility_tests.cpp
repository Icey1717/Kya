#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include "ActorClusteriser.h"
#include "MemoryStream.h"
#include "SectorManager.h"
#include "FileManager3D.h"
#include "ed3D/ed3DG3D.h"

TEST(SceneryVisibility, ReadsPackedHashListWithoutSkippingFirstFourBytes)
{
	// A zone, four empty reference lists, two 64-bit scenery hashes, then a zone index.
	uint32_t words[] = { 1, 8, uint32_t(-1), 0, 0, 0, 0, 2,
		0x55667788, 0x11223344, 0xddeeff00, 0x99aabbcc, 42 };
	ByteCode stream;
	stream.currentSeekPos = reinterpret_cast<char*>(words);
	CBehaviourClusteriserZones behaviour;
	behaviour.Create(&stream);
	ASSERT_EQ(behaviour.nbZoneClusters, 1);
	const auto& zone = behaviour.aZoneClusters[0];
	ASSERT_EQ(zone.field_0x34->entryCount, 2);
	EXPECT_EQ(zone.field_0x34->aEntries[0], 0x1122334455667788ull);
	EXPECT_EQ(zone.field_0x34->aEntries[1], 0x99aabbccddeeff00ull);
	EXPECT_EQ(zone.field_0x8.index, 42);
	EXPECT_EQ(stream.currentSeekPos, reinterpret_cast<char*>(words + std::size(words)));
	delete[] behaviour.aZoneClusters;
}

TEST(SceneryVisibility, ZoneTransitionsHideAndShowSceneryAndDescendants)
{
	struct HierarchyChunk {
		ed_Chunck chunk{};
		ed_g3d_hierarchy hierarchy{};
	};
	struct Hierarchies {
		std::array<HierarchyChunk, 4> entries{};
		ed_Chunck end{};
	} data;
	for (auto& entry : data.entries) {
		entry.chunk.hash = HASH_CODE_HIER;
		entry.chunk.size = sizeof(HierarchyChunk);
		entry.hierarchy.flags_0x9e = 0x202;
	}
	const int rootHandle = STORE_POINTER(&data.entries[0].chunk);
	const int childHandle = STORE_POINTER(&data.entries[1].chunk);
	data.entries[1].hierarchy.pLinkTransformData = rootHandle;
	data.entries[2].hierarchy.pLinkTransformData = childHandle;
	struct HashBank {
		ed_Chunck hall{};
		ed_Chunck hash{};
		ed_hash_code entry{};
	} bank;
	bank.hash.hash = HASH_CODE_HASH;
	bank.hash.nextChunckOffset = sizeof(ed_Chunck) + sizeof(ed_hash_code);
	bank.entry.hash.number = 0x1122334455667788ull;
	bank.entry.pData = rootHandle;
	ed_g3d_manager mesh{};
	mesh.HALL = &bank.hall;
	C3DFileManager fileManager;
	CSectorManager sectorManager;
	auto* savedFileManager = CScene::ptable.g_C3DFileManager_00451664;
	auto* savedSectorManager = CScene::ptable.g_SectorManager_00451670;
	CScene::ptable.g_C3DFileManager_00451664 = &fileManager;
	CScene::ptable.g_SectorManager_00451670 = &sectorManager;
	std::array<uint32_t, 4> hashWords{ 1, 0x55667788, 0x11223344, 42 };
	CBehaviourClusteriserZones::_S_ZONE_CLUSTER zone;
	zone.field_0x34 = reinterpret_cast<S_HASH_STREAM_REF*>(hashWords.data());
	// The old unpacked structure read at offset 8, combining the high half
	// of the intended hash with the following zone index. That lookup failed
	// before reaching the IMPLEMENTATION_GUARD in TriggerSceneries.
	ulong legacyHash;
	memcpy(&legacyHash, reinterpret_cast<const char*>(hashWords.data()) + 8, sizeof(legacyHash));
	EXPECT_EQ(legacyHash, 0x0000002a11223344ull);
	EXPECT_EQ(ed3DG3DHierarchyGetFromHashcode(&mesh, legacyHash), nullptr);
	EXPECT_EQ(ed3DG3DHierarchyGetFromHashcode(&mesh, zone.field_0x34->aEntries[0]), &data.entries[0].hierarchy);
	CBehaviourClusteriserZones behaviour;
	// Test both the common mesh and loaded-sector lookup paths.
	for (bool commonMesh : { true, false }) {
		fileManager.pMeshInfo = commonMesh ? &mesh : nullptr;
		sectorManager.baseSector.loadStage_0x8 = 2;
		sectorManager.baseSector.sectorMesh = mesh;
		for (int active : { 0, 1, 0, 1 }) {
			behaviour.TriggerSceneries(&zone, active);
			for (size_t i = 0; i < 3; ++i) {
				EXPECT_EQ(data.entries[i].hierarchy.flags_0x9e, active ? 0x202 : 0x242);
			}
			EXPECT_EQ(data.entries[3].hierarchy.flags_0x9e, 0x202);
		}
	}
	CScene::ptable.g_C3DFileManager_00451664 = savedFileManager;
	CScene::ptable.g_SectorManager_00451670 = savedSectorManager;
	delete[] fileManager.pParticleInfoArray_0x50;
	RELEASE_POINTER(childHandle);
	RELEASE_POINTER(rootHandle);
}
