#include <gtest/gtest.h>
#include "ActorBonus.h"
#include "CameraViewManager.h"
#include "DlistManager.h"
#include "edDlist.h"
#include "MathOps.h"
#include "TimeController.h"
#include "SectorManager.h"

extern DisplayList* gCurDListHandle;

namespace
{
	class BonusFadeOwner : public CActorBonus
	{
	public:
		void SetState(int state, int animationType) override
		{
			lastState = state;
		}

		int lastState = -1;
	};

	class BonusDrawTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousManager = CScene::ptable.g_GlobalDListManager_004516bc;
			previousCamera = CCameraManager::_gThis;
			previousDelta = GetTimer()->cutsceneDeltaTime;
			previousRenderState = gCurRenderState;
			previousList = gCurDList;
			previousListHandle = gCurDListHandle;
			CScene::ptable.g_GlobalDListManager_004516bc = &manager;
			CCameraManager::_gThis = &camera;
			GetTimer()->cutsceneDeltaTime = 0.0f;
			gCurRenderState = 0;
			edF32Matrix4CopyHard(&camera.transMatrix_0x390, &gF32Matrix4Unit);
			manager.field_0x1c = 0;
			manager.nbRegData = 0;
			manager.nbActiveCallFuncElements = 0;
			manager.ppGlobalDlist = &entry;
			entry.pDlistPatch = &patch;
			patch.bEnabled = 1;
			// Use pre-resolved patch buffers without rebuilding renderer packets.
			patch.pDisplayListInternal = &list;
			for (int i = 0; i < 2; ++i) {
				patch.aPatches[i] = &sprites[i];
				sprites[i].pVertex = vertices[i];
				sprites[i].pRgba = colors[i];
				sprites[i].pSt = st[i];
				for (auto& color : colors[i]) color.rgba = 0x00332211;
			}
		}

		void TearDown() override
		{
			patch.pDisplayListInternal = nullptr;
			CScene::ptable.g_GlobalDListManager_004516bc = previousManager;
			CCameraManager::_gThis = previousCamera;
			GetTimer()->cutsceneDeltaTime = previousDelta;
			gCurRenderState = previousRenderState;
			gCurDList = previousList;
			gCurDListHandle = previousListHandle;
		}

		void Prepare(CBehaviourBonusAlone& behaviour, uint flags)
		{
			behaviour.bonusFlarePatchId = 0;
			behaviour.bonusAnimPatchId = 1;
			behaviour.actInstance.flags = flags;
			behaviour.actInstance.instanceIndex = 0;
			behaviour.actInstance.field_0x90 = 0.0f;
			behaviour.actInstance.angleRotY = 11.0f;
			behaviour.actInstance.currentPosition = { 2.0f, 3.0f, 4.0f, 1.0f };
		}

		CGlobalDListManager manager{};
		CCameraManager camera;
		CGlobalDListPatch patch{ 0 };
		GlobalDlistEntry entry{};
		DisplayList list{};
		S_GLOBAL_DLIST_PATCH sprites[2]{};
		edVertex vertices[2][4]{};
		_rgba colors[2][4]{};
		uint st[2][4]{};
		CGlobalDListManager* previousManager;
		CCameraManager* previousCamera;
		DisplayList* previousList;
		DisplayList* previousListHandle;
		float previousDelta;
		int previousRenderState;
	};
}

TEST_F(BonusDrawTest, TurnAndPathDrawRestoreFlareAndAnimatedSpriteThroughVirtualDispatch)
{
	CBehaviourBonusTurn turn;
	CBehaviourBonusPath path;
	for (CBehaviourBonusAlone* behaviour : { static_cast<CBehaviourBonusAlone*>(&turn), static_cast<CBehaviourBonusAlone*>(&path) }) {
		Prepare(*behaviour, 5);
		CBehaviour* base = behaviour;
		base->Draw();
		EXPECT_NEAR(vertices[0][0].x, 2.05f, 0.00001f);
		EXPECT_FLOAT_EQ(vertices[0][0].y, 3.0f);
		EXPECT_FLOAT_EQ(vertices[0][0].z, 4.0f);
		EXPECT_FLOAT_EQ(vertices[1][0].x, 2.0f);
		EXPECT_FLOAT_EQ(vertices[1][0].y, 3.0f);
		EXPECT_FLOAT_EQ(vertices[1][0].z, 4.0f);
		for (int i = 0; i < 4; ++i) {
			EXPECT_EQ(colors[0][i].rgba & 0xffffff, 0x00332211u);
			EXPECT_GE(colors[0][i].a, 64);
			EXPECT_LE(colors[0][i].a, 160);
			EXPECT_EQ(colors[1][i].rgba, 0x80332211u);
		}
		const short expected[8] = { 2048, 4096, 1536, 4096, 2048, 2048, 1536, 2048 };
		const short* actual = reinterpret_cast<const short*>(st[1]);
		for (int i = 0; i < 8; ++i) EXPECT_EQ(actual[i], expected[i]);
		EXPECT_EQ(manager.nbActiveCallFuncElements, 0);
	}
}

TEST_F(BonusDrawTest, HiddenInstanceQueuesBothSpritesForHiding)
{
	CBehaviourBonusTurn turn;
	Prepare(turn, 3);
	CBehaviour* base = &turn;
	base->Draw();
	ASSERT_EQ(manager.nbActiveCallFuncElements, 2);
	for (int i = 0; i < 2; ++i) {
		EXPECT_EQ(manager.aActiveCallFuncElements[i].patchId, i);
		EXPECT_EQ(manager.aActiveCallFuncElements[i].type, CALL_ELEMENT_HIDE_SPRITE);
		EXPECT_EQ(manager.aActiveCallFuncElements[i].bActive, 0);
		EXPECT_EQ(colors[i][0].a, 0);
	}
}

TEST_F(BonusDrawTest, CollectedTurnAndPathWaitForTailActiveFlagToClear)
{
	BonusFadeOwner owner;
	CBehaviourBonusTurn turn;
	CBehaviourBonusPath path;
	CPathFollow pathFollow;
	CSectorManager sectorManager;
	auto* previousSectorManager = CScene::ptable.g_SectorManager_00451670;
	CScene::ptable.g_SectorManager_00451670 = &sectorManager;
	sectorManager.baseSector.desiredSectorID = 0;
	owner.sectorId = 0;
	owner.flags = 0;
	owner.pShadow = nullptr;
	path.pathPlane.pathFollowReader.pPathFollow = &pathFollow;
	patch.bEnabled = 0;
	for (CBehaviourBonusAlone* behaviour : { static_cast<CBehaviourBonusAlone*>(&turn), static_cast<CBehaviourBonusAlone*>(&path) }) {
		behaviour->pOwner = &owner;
		behaviour->actInstance.flags = 0;
		behaviour->field_0x1c4 = 0;
		turn.field_0x1dc = 1;
		owner.pFxTail = &behaviour->fxTail;
		owner.pFxTail->flags = 0x1000;
		owner.pFxTail->nbSegments = 4;
		owner.pFxTail->nbUsedSegments = 0;
		owner.pFxTail->dlistPatchId = 0;
		owner.lastState = -1;
		owner.flags = 0;
		CBehaviour* base = behaviour;
		base->Manage();
		EXPECT_EQ(owner.lastState, -1);
		EXPECT_EQ(owner.flags, 0u);
		EXPECT_NE(owner.pFxTail->flags & 0x1000, 0u);

		// Life management clears the active bit once all segment alpha is gone.
		owner.pFxTail->_ManageLife();
		EXPECT_EQ(owner.pFxTail->flags & 0x1000, 0u);
		base->Manage();
		EXPECT_EQ(owner.lastState, 6);
		EXPECT_EQ(owner.flags & 0x23, 0x21u);
	}
	CScene::ptable.g_SectorManager_00451670 = previousSectorManager;
}
