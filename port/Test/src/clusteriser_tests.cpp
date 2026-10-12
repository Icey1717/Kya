#include <gtest/gtest.h>
#include <memory>
#include "ActorClusteriser.h"
#include "EventManager.h"
#include "MathOps.h"
#include "edMem.h"
#include "EdenLib/edSys/sources/EdSystem.h"

extern ed_event_chunk** pedEventChunks;
extern EventQueue EventQueue_00448fbc;

namespace
{
	using ClusterCallback = void (*)(CActor*, void*);

	class ClusteriserMembershipTest : public testing::TestWithParam<ClusterCallback>
	{
	public:
		static void SetUpTestSuite()
		{
			previousCallbacks = CEventManager::callbackFunctions;
			previousQueue = EventQueue_00448fbc;
			if (!edSystem.system.startAddress) {
				ASSERT_TRUE(_edSystemInit(0, nullptr));
				edMemInit(0x1000);
			}
		}

	protected:
		void SetUp() override
		{
			previousChunks = pedEventChunks;
			chunks[0] = &chunk;
			pedEventChunks = chunks;
			chunk.field_0x8 = &primitiveType;
			chunk.aMatrices = &matrix;
			zone.field_0x0[0] = STORE_POINTER(primitiveList);
			eventManager.activeChunkId = 0;
			candidate.sectorId = 4;
			candidate.typeID = ACTOR;
			exclusionStorage = std::make_unique<unsigned char[]>(sizeof(S_ACTOR_STREAM_REF) + 3 * sizeof(S_STREAM_REF<CActor>));
			exclusions = reinterpret_cast<S_ACTOR_STREAM_REF*>(exclusionStorage.get());
			exclusions->entryCount = 3;
			for (int i = 0; i < 3; ++i) {
				exclusions->aEntries[i].pObj = STORE_POINTER(&otherActors[i]);
			}
			params = { &eventManager, &zone, output, 0, &exclusions, 4 };
		}

		void TearDown() override
		{
			pedEventChunks = previousChunks;
			edMemFree(EventQueue_00448fbc.aEntries);
			EventQueue_00448fbc = previousQueue;
			CEventManager::callbackFunctions = previousCallbacks;
		}

		inline static CEventManagerInternal previousCallbacks{};
		inline static EventQueue previousQueue{};
		ed_event_chunk** previousChunks = nullptr;
		ed_event_chunk* chunks[1]{};
		ed_event_chunk chunk{};
		CEventManager eventManager;
		ed_zone_3d zone{};
		edF32MATRIX4 matrix = gF32Matrix4Unit;
		byte primitiveType = 2; // Unit box around the actor's default position.
		uint primitiveList[2] = { 1, 0 };
		CActor candidate;
		CActor otherActors[3];
		CActor* output[1]{};
		std::unique_ptr<unsigned char[]> exclusionStorage;
		S_ACTOR_STREAM_REF* exclusions = nullptr;
		ClusterizerCallbackParams params{};
	};

	TEST_P(ClusteriserMembershipTest, ExcludesActorsAtEveryListPosition)
	{
		for (int entry = 0; entry < 3; ++entry) {
			SCOPED_TRACE(entry);
			exclusions->aEntries[entry].pObj = STORE_POINTER(&candidate);
			GetParam()(&candidate, &params);
			EXPECT_EQ(params.count, 0);
			EXPECT_EQ(output[0], nullptr);
			exclusions->aEntries[entry].pObj = STORE_POINTER(&otherActors[entry]);
			params.count = 0;
			output[0] = nullptr;
		}
	}

	TEST_P(ClusteriserMembershipTest, IncludesActorAbsentFromExclusions)
	{
		GetParam()(&candidate, &params);
		EXPECT_EQ(params.count, 1);
		EXPECT_EQ(output[0], &candidate);
	}

	TEST_P(ClusteriserMembershipTest, IncludesActorWithEmptyExclusionList)
	{
		exclusions->entryCount = 0;
		GetParam()(&candidate, &params);
		EXPECT_EQ(params.count, 1);
		EXPECT_EQ(output[0], &candidate);
	}

	TEST_P(ClusteriserMembershipTest, RejectsActorOutsideZone)
	{
		candidate.currentLocation.x = 2.0f;
		GetParam()(&candidate, &params);
		EXPECT_EQ(params.count, 0);
		EXPECT_EQ(output[0], nullptr);
	}

	TEST_P(ClusteriserMembershipTest, PreservesSectorFiltering)
	{
		candidate.sectorId = 5;
		GetParam()(&candidate, &params);
		EXPECT_EQ(params.count, GetParam() == gClusterCallback_Clusteriser_All ? 1 : 0);
	}

	TEST_P(ClusteriserMembershipTest, IncludesGlobalActor)
	{
		candidate.sectorId = -1;
		GetParam()(&candidate, &params);
		EXPECT_EQ(params.count, 1);
		EXPECT_EQ(output[0], &candidate);
	}

	INSTANTIATE_TEST_SUITE_P(AllAndSector, ClusteriserMembershipTest,
		testing::Values(gClusterCallback_Clusteriser_All, gClusterCallback_Clusteriser_Sector));
}
