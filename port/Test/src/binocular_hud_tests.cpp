#include <gtest/gtest.h>

#include "FrontEndBinoculars.h"
#include "FrontEndDisp.h"
#include "CameraViewManager.h"
#include "TimeController.h"

namespace
{
	class BinocularHudTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousCamera = CScene::ptable.g_CameraManager_0045167c;
			previousTime = GetTimer()->totalTime;
			GetTimer()->totalTime = 0.0f;
			camera.aspectRatio = 4.0f / 3.0f;
			CScene::ptable.g_CameraManager_0045167c = &camera;
			widget.Reset();
			widget.SetInterface(&interface);
		}

		void TearDown() override
		{
			CScene::ptable.g_CameraManager_0045167c = previousCamera;
			GetTimer()->totalTime = previousTime;
		}

		void Advance(float delta)
		{
			GetTimer()->totalTime += delta;
			widget.Update(GetTimer()->totalTime);
		}

		void Show()
		{
			interface.Activate(1);
			Advance(0.01f);
			Advance(0.2f);
			Advance(0.01f);
		}

		CCameraManager* previousCamera;
		float previousTime;
		CCameraManager camera;
		CFrontendBinoculars widget;
		OtherInterface interface;
	};
}

TEST_F(BinocularHudTest, ActivationAndDismissalCompleteThePopupTransitions)
{
	EXPECT_FALSE(widget.bVisible);
	Show();
	ASSERT_TRUE(widget.bVisible);
	// First activation copies the constructor slot before Update refreshes slotOn.
	EXPECT_FLOAT_EQ(widget.widgetSlotA.position.x, 0.32f);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.position.y, 0.26f);
	EXPECT_FLOAT_EQ(widget.slotOn.position.x, 0.3125f);
	EXPECT_FLOAT_EQ(widget.slotOn.position.y, 0.25f);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.scale.x, 0.375f);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.scale.y, 0.5f);

	interface.Activate(0);
	Advance(0.01f);
	EXPECT_TRUE(widget.bVisible);
	Advance(0.2f);
	Advance(0.01f);
	EXPECT_FALSE(widget.bVisible);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.position.x, -0.14f);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.position.y, -0.02f);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.scale.x, 0.75f);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.scale.y, 1.0f);
}

TEST_F(BinocularHudTest, ReactivationWaitsForDismissalThenShowsAgain)
{
	Show();
	interface.Activate(0);
	Advance(0.01f);
	interface.Activate(1);
	Advance(0.05f);
	Advance(0.2f);
	Advance(0.01f);
	EXPECT_FALSE(widget.bVisible);
	Advance(0.01f);
	EXPECT_TRUE(widget.bVisible);
	Advance(0.2f);
	Advance(0.01f);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.position.x, 0.3125f);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.scale.x, 0.375f);
}

TEST_F(BinocularHudTest, WidescreenUsesCameraAspectAndDetachedInterfaceStopsUpdates)
{
	camera.aspectRatio = 16.0f / 9.0f;
	// The PS2 flow refreshes slotOn after checking activation.
	Advance(0.01f);
	Show();
	EXPECT_FLOAT_EQ(widget.widgetSlotA.position.x, 0.359375f);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.scale.x, 0.28125f);

	widget.SetInterface(nullptr);
	interface.Activate(0);
	Advance(1.0f);
	EXPECT_TRUE(widget.bVisible);
	EXPECT_FLOAT_EQ(widget.widgetSlotA.scale.x, 0.28125f);
	widget.Level_Term();
	widget.Level_Term();
	EXPECT_EQ(widget.instance3d.pNode, nullptr);
}