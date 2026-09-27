#include <gtest/gtest.h>

#include <memory>
#include "ActorHero.h"
#include "ActorNativ.h"
#include "../../../src/port/pointer_conv.h"

extern uint CreateHashFromName(char* szInput);

namespace
{
	class NativAkasaTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousHero = CActorHero::_gThis;
			hero = std::make_unique<CActorHero>();
			CActorHero::_gThis = hero.get();
			PointerConv::ResetAll();
			for (auto& combo : behaviour.field_0x60.field_0x10f0) {
				combo = {};
			}
			for (auto& blow : behaviour.field_0x60.field_0x11d0) {
				blow = {};
			}
		}

		void TearDown() override
		{
			PointerConv::ResetAll();
			hero.reset();
			CActorHero::_gThis = previousHero;
		}

		void SetCombos(s_fighter_combo* combos, uint roots, uint branches)
		{
			hero->aCombos = combos;
			hero->nbComboRoots = roots;
			hero->nbCombos = branches;
		}

		void ExpectSortedList(std::initializer_list<int> indices)
		{
			auto& list = behaviour.field_0x60;
			NativComboSequenceEntry* previous = nullptr;
			NativComboSequenceEntry* current = list.pSellerSubObjAA;
			for (int index : indices) {
				ASSERT_EQ(current, &list.field_0x8[index]);
				EXPECT_EQ(current->pNext, previous);
				previous = current;
				current = current->pPrev;
			}
			EXPECT_EQ(current, nullptr);
			EXPECT_EQ(list.pSellerSubObjAB, previous);
		}

		CActorHero* previousHero = nullptr;
		std::unique_ptr<CActorHero> hero;
		CBehaviourNativAkasa behaviour;
	};
}

TEST_F(NativAkasaTest, EmptyComboDatabaseStillBuildsSpecialMoves)
{
	SetCombos(nullptr, 0, 0);
	behaviour.FUN_003f2900();

	auto& list = behaviour.field_0x60;
	ASSERT_EQ(list.field_0x10e8, 2);
	EXPECT_EQ(list.field_0x8[0].field_0x80, 4);
	EXPECT_EQ(list.field_0x8[1].field_0x80, 1);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[0].aRequiredCombos[0], &list.field_0x10f0[4]);
	for (int i = 1; i < 4; i++) {
		EXPECT_EQ(list.field_0x8[0].aSubObjs[i].aRequiredCombos[0], &list.field_0x10f0[3]);
	}
	EXPECT_EQ(list.field_0x8[1].aSubObjs[0].aRequiredCombos[0], &list.field_0x10f0[5]);
	EXPECT_EQ(list.field_0x8[0].field_0x84, 4);
	EXPECT_EQ(list.field_0x8[1].field_0x84, 0x201);
	EXPECT_EQ(list.field_0x10f0[3].pattern.field_0x0uint, 0x00100000u);
	EXPECT_EQ(list.field_0x10f0[4].pattern.field_0x0uint, 0x00800000u);
	EXPECT_EQ(list.field_0x10f0[5].pattern.field_0x0uint, 0x00800000u);
	EXPECT_EQ(LOAD_POINTER(list.field_0x10f0[3].actionHash.pData), &list.field_0x11d0[2]);
	EXPECT_EQ(LOAD_POINTER(list.field_0x10f0[4].actionHash.pData), &list.field_0x11d0[3]);
	EXPECT_EQ(LOAD_POINTER(list.field_0x10f0[5].actionHash.pData), &list.field_0x11d0[4]);
	ExpectSortedList({0, 1});
}

TEST_F(NativAkasaTest, BranchesCopyTheirPrefixAndSortByAllScoreFlags)
{
	s_fighter_combo combos[4] = {};
	s_fighter_move moves[4] = {};
	for (int i = 0; i < 4; i++) {
		combos[i].actionHash.pData = STORE_POINTER(&moves[i]);
	}
	moves[0].field_0x4.field_0x1byte = 1;
	moves[1].field_0x4.field_0x1byte = 0xc; // Flag 0x4 suppresses the movement score.
	moves[2].field_0x4.field_0x1byte = 8;
	moves[3].field_0x4.field_0x1byte = 1;
	combos[2].field_0x4.field_0x0ushort = 0x400;
	combos[2].pattern.nbInputs = 1;
	combos[2].pattern.field_0x3byte = 0x20;
	s_fighter_action_hash rootBranches[2] = {};
	rootBranches[0].pData = STORE_POINTER(&combos[1]);
	rootBranches[1].pData = STORE_POINTER(&combos[3]);
	combos[0].aBranches = rootBranches;
	combos[0].nbBranches = 2;
	s_fighter_action_hash childBranch = {};
	childBranch.pData = STORE_POINTER(&combos[2]);
	combos[1].aBranches = &childBranch;
	combos[1].nbBranches = 1;
	SetCombos(combos, 1, 3);

	behaviour.FUN_003f2900();

	auto& list = behaviour.field_0x60;
	ASSERT_EQ(list.field_0x10e8, 4);
	ASSERT_EQ(list.field_0x8[0].field_0x80, 3);
	ASSERT_EQ(list.field_0x8[1].field_0x80, 2);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[0].aRequiredCombos[0], &combos[0]);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[1].aRequiredCombos[0], &combos[1]);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[2].aRequiredCombos[0], &combos[2]);
	EXPECT_EQ(list.field_0x8[1].aSubObjs[0].aRequiredCombos[0], &combos[0]);
	EXPECT_EQ(list.field_0x8[1].aSubObjs[1].aRequiredCombos[0], &combos[3]);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[0].field_0xc, 0x41);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[1].field_0xc, 1);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[2].field_0xc, 0x9209);
	EXPECT_EQ(list.field_0x8[0].field_0x84, 0x924b);
	EXPECT_EQ(list.field_0x8[1].field_0x84, 0x82);
	ExpectSortedList({2, 1, 3, 0});
}

TEST_F(NativAkasaTest, TailRootProducesBothOriginalSpecialSequences)
{
	s_fighter_combo root = {};
	s_fighter_move move = {};
	char name[] = "ROOT_G_tail";
	root.hash.hash = CreateHashFromName(name);
	root.actionHash.pData = STORE_POINTER(&move);
	move.field_0x4.field_0x1byte = 1;
	SetCombos(&root, 1, 0);

	behaviour.FUN_003f2900();

	auto& list = behaviour.field_0x60;
	ASSERT_EQ(list.field_0x10e8, 4);
	ASSERT_EQ(list.field_0x8[0].field_0x80, 2);
	ASSERT_EQ(list.field_0x8[1].field_0x80, 3);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[0].aRequiredCombos[0], &root);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[1].aRequiredCombos[0], &list.field_0x10f0[0]);
	EXPECT_EQ(list.field_0x8[1].aSubObjs[0].aRequiredCombos[0], &root);
	EXPECT_EQ(list.field_0x8[1].aSubObjs[1].aRequiredCombos[0], &list.field_0x10f0[1]);
	EXPECT_EQ(list.field_0x8[1].aSubObjs[2].aRequiredCombos[0], &list.field_0x10f0[2]);
	EXPECT_EQ(list.field_0x10f0[0].pattern.field_0x0uint, 0x00200000u);
	EXPECT_EQ(list.field_0x10f0[1].pattern.field_0x0uint, 0x01000000u);
	EXPECT_EQ(list.field_0x10f0[2].pattern.field_0x0uint, 0x00400000u);
	EXPECT_EQ(LOAD_POINTER(list.field_0x10f0[0].actionHash.pData), &list.field_0x11d0[0]);
	EXPECT_EQ(LOAD_POINTER(list.field_0x10f0[1].actionHash.pData), &list.field_0x11d0[1]);
	EXPECT_EQ(LOAD_POINTER(list.field_0x10f0[2].actionHash.pData), &list.field_0x11d0[1]);
	ExpectSortedList({2, 0, 1, 3});
}

TEST_F(NativAkasaTest, StopFlagEndsTheCurrentPathAndSkipsRemainingBranches)
{
	s_fighter_combo combos[3] = {};
	s_fighter_move move = {};
	combos[0].actionHash.pData = STORE_POINTER(&move);
	combos[1].field_0x4.field_0x0ushort = 0x100;
	s_fighter_action_hash branches[2] = {};
	branches[0].pData = STORE_POINTER(&combos[1]);
	branches[1].pData = STORE_POINTER(&combos[2]);
	combos[0].aBranches = branches;
	combos[0].nbBranches = 2;
	SetCombos(combos, 1, 2);

	behaviour.FUN_003f2900();

	auto& list = behaviour.field_0x60;
	ASSERT_EQ(list.field_0x10e8, 3);
	EXPECT_EQ(list.field_0x8[0].field_0x80, 1);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[0].aRequiredCombos[0], &combos[0]);
	ExpectSortedList({0, 1, 2});
}

TEST_F(NativAkasaTest, SortingPreservesEqualScoreOrderAndBothListLinks)
{
	auto& list = behaviour.field_0x60;
	const int scores[] = {20, 10, 20, 15};
	const int positions[] = {0, 0, 2, 1};
	for (int i = 0; i < 4; i++) {
		list.field_0x8[i].field_0x84 = scores[i];
		EXPECT_EQ(list.InsertComboPathByScore(&list.field_0x8[i]), positions[i]);
	}
	ExpectSortedList({1, 3, 0, 2});
}

TEST_F(NativAkasaTest, TraversalRejectsNullCombosAndDepthEight)
{
	auto& list = behaviour.field_0x60;
	list.field_0x10e8 = 0;
	s_fighter_combo combo = {};
	list.CollectComboBranchPaths(nullptr, 0);
	list.CollectComboBranchPaths(&combo, 8);
	EXPECT_EQ(list.field_0x10e8, 0);
	EXPECT_EQ(list.field_0x8[0].aSubObjs[0].nbRequiredCombos, 0);
}
