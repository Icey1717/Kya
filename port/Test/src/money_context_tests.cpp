#include <gtest/gtest.h>

#include "ActorMoney.h"
#include "SectorManager.h"

TEST(MoneyContext, LoadingAddOnContextClearsSpawnedMoney)
{
	CSectorManager sectorManager;
	auto* previousSectorManager = CScene::ptable.g_SectorManager_00451670;
	CScene::ptable.g_SectorManager_00451670 = &sectorManager;
	sectorManager.baseSector.desiredSectorID = 0;

	CActorMoney actor;
	CBehaviourMoneyAddOn behaviour;
	CMnyInstance instances[3];
	for (auto& instance : instances) {
		instance.flags = ACTOR_INSTANCE_FLAG_ALIVE | ACTOR_INSTANCE_FLAG_VISIBLE;
	}
	behaviour.pOwner = &actor;
	behaviour.nbMoneyInstances = 3;
	behaviour.aMoneyInstances = instances;
	behaviour.field_0x18 = 17;
	actor.flags = 0x82;
	actor.sectorId = 0;
	actor.curBehaviourId = MONEY_BEHAVIOUR_ADD_ON;
	BehaviourList<1> components{};
	components.count = 1;
	components.aComponents[0].id = MONEY_BEHAVIOUR_ADD_ON;
	components.aComponents[0].SetBehaviour(&behaviour);
	actor.aComponents = reinterpret_cast<BehaviourEntry*>(&components);

	// Exercise actor dispatch, the behaviour's LoadContext, and the real reset.
	actor.LoadContext(nullptr, 0, 0);

	for (const auto& instance : instances) {
		EXPECT_EQ(instance.flags & (ACTOR_INSTANCE_FLAG_ALIVE | ACTOR_INSTANCE_FLAG_VISIBLE), 0u);
	}
	EXPECT_EQ(behaviour.field_0x18, 0);
	EXPECT_EQ(actor.flags & 0xa3, 0x21u);
	CScene::ptable.g_SectorManager_00451670 = previousSectorManager;
}
