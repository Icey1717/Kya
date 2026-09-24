#include <gtest/gtest.h>
#include "DrawTrace.h"
#include "../../DebugMenu/src/DebugDrawInspector.h"
#include "../../DebugMenu/src/DebugTexture.h"
#include "../../DebugMenu/src/DebugMeshViewer.h"
#include "imgui.h"
#include <thread>

namespace {
namespace Trace = Renderer::DrawTrace;
class DrawTraceTest : public testing::Test {
protected:
    void SetUp() override {
        Trace::InvalidateSources();
        Trace::AdvanceFrame();
        Trace::AdvanceFrame();
        Trace::ClearSnapshot();
    }
    void TearDown() override {
        Trace::InvalidateSources();
        Trace::AdvanceFrame();
        Trace::ClearSnapshot();
    }
    void Capture() { Trace::RequestCapture(false); Trace::AdvanceFrame(); }
    Trace::Source Source(uintptr_t node = 100, uintptr_t hierarchy = 200) {
        Trace::Source source;
        source.node = node; source.object = 300; source.hierarchy = hierarchy; source.viewport = 400;
        source.bounds = {1, 2, 3, 4};
        return source;
    }
    uint64_t Submit(uintptr_t node = 100, int layer = 0) {
        Trace::FlushScope scope(node, layer);
        Trace::Submission submission;
        Trace::CopyName(submission.mesh, "SECT5.g3d_15_0_1");
        return Trace::Submit(submission);
    }
    bool Record(uint64_t submission) {
        Trace::Draw draw;
        draw.recorded = true; draw.indexCount = 12;
        Trace::CopyName(draw.texture, "SECT5.g2d (m: 74 l:0)");
        return Trace::Record(submission, draw);
    }
};

TEST_F(DrawTraceTest, CaptureStartsAtBoundaryAndPublishesCompletedImmutableFrame) {
    Trace::RequestCapture(false);
    EXPECT_FALSE(Trace::IsEnabled());
    Trace::Link(Source());
    Trace::AdvanceFrame();
    EXPECT_TRUE(Trace::IsEnabled());
    Trace::Link(Source());
    const auto submission = Submit();
    EXPECT_TRUE(Record(submission));
    EXPECT_FALSE(Trace::GetSnapshot());
    Trace::AdvanceFrame();
    auto snapshot = Trace::GetSnapshot();
    ASSERT_TRUE(snapshot);
    ASSERT_EQ(snapshot->sources.size(), 1u);
    ASSERT_EQ(snapshot->submissions.size(), 1u);
    ASSERT_EQ(snapshot->draws.size(), 1u);
    EXPECT_EQ(snapshot->draws[0].submissionId, submission);
    EXPECT_TRUE(snapshot->sources[0].flushed);
    EXPECT_EQ(snapshot->sources[0].linkStack, 0u);
    EXPECT_FALSE(Trace::IsEnabled());
    Trace::InvalidateSources();
    Trace::ClearSnapshot();
    EXPECT_STREQ(snapshot->submissions[0].mesh.data(), "SECT5.g3d_15_0_1");
    EXPECT_FALSE(Trace::IsSourceLive(snapshot->sources[0]));
}

TEST_F(DrawTraceTest, DuplicateNodesLayersSpriteBatchesAndCopiedDrawsRemainDistinct) {
    Capture();
    Trace::Link(Source());
    auto sprite = Source(101); sprite.kind = Trace::Kind::Sprite;
    Trace::Link(sprite);
    const auto first = Submit(100, 0);
    const auto layer = Submit(100, 1);
    uint64_t spriteA, spriteB;
    {
        Trace::FlushScope scope(101);
        spriteA = Trace::Submit({}); spriteB = Trace::Submit({});
    }
    std::thread consumer([&] { Record(first); Record(first); Record(layer); Record(spriteA); Record(spriteB); });
    consumer.join();
    Trace::AdvanceFrame();
    auto snapshot = Trace::GetSnapshot();
    ASSERT_EQ(snapshot->sources.size(), 2u);
    ASSERT_EQ(snapshot->submissions.size(), 4u);
    ASSERT_EQ(snapshot->draws.size(), 5u);
    EXPECT_NE(snapshot->draws[0].id, snapshot->draws[1].id);
    EXPECT_EQ(snapshot->draws[0].submissionId, snapshot->draws[1].submissionId);
    EXPECT_EQ(snapshot->submissions[1].layer, 1);
    EXPECT_NE(snapshot->submissions[0].sourceId, snapshot->submissions[2].sourceId);
    EXPECT_NE(spriteA, spriteB);
}

TEST_F(DrawTraceTest, ReusingListNodeDoesNotAliasPreviousLinkOrFrame) {
    Capture();
    Trace::Link(Source()); const auto first = Submit();
    Trace::Link(Source(100, 201)); const auto second = Submit();
    Record(first); Record(second);
    Trace::AdvanceFrame();
    auto previous = Trace::GetSnapshot();
    ASSERT_EQ(previous->sources.size(), 2u);
    EXPECT_NE(previous->submissions[0].sourceId, previous->submissions[1].sourceId);
    Capture();
    EXPECT_EQ(Submit(), 0u);
    Trace::Link(Source()); Record(Submit());
    Trace::AdvanceFrame();
    EXPECT_NE(previous->sources[0].id, Trace::GetSnapshot()->sources[0].id);
    EXPECT_EQ(previous->sources[1].hierarchy, 201u);
}

TEST_F(DrawTraceTest, OwnerAndBankScopesRestoreAndHierarchyOwnershipResolves) {
    Capture();
    Trace::RegisterOwner(200, 800, "Actor", 1, 2, 5);
    {
        Trace::BankScope bank(900);
        Trace::Link(Source());
        {
            Trace::OwnerScope owner(801, "Effect", 3, 4, 6);
            Trace::BankScope bank2(901);
            Trace::Link(Source(101));
        }
        Trace::Link(Source(102));
    }
    Trace::Link(Source(103, 202));
    Trace::AdvanceFrame();
    const auto snapshot = Trace::GetSnapshot();
    ASSERT_EQ(snapshot->sources.size(), 4u);
    EXPECT_EQ(snapshot->sources[0].owner, 800u);
    EXPECT_EQ(snapshot->sources[1].owner, 801u);
    EXPECT_EQ(snapshot->sources[2].owner, 800u);
    EXPECT_EQ(snapshot->sources[2].bank, 900u);
    EXPECT_EQ(snapshot->sources[3].bank, 0u);
    EXPECT_EQ(snapshot->sources[3].owner, 0u);
}

TEST_F(DrawTraceTest, SourceOverrideKeepsOtherInstancesAndChangesAtFrameBoundary) {
    Capture(); Trace::Link(Source()); Record(Submit()); Trace::AdvanceFrame();
    auto snapshot = Trace::GetSnapshot();
    Trace::SetOverride(snapshot->sources[0], &snapshot->submissions[0], Trace::Action::Hide, Trace::Match::Source);
    EXPECT_FALSE(Trace::IsEnabled());
    Capture();
    Trace::Link(Source(110)); Trace::Link(Source(111, 201));
    EXPECT_FALSE(Record(Submit(110, 1)));
    EXPECT_TRUE(Record(Submit(111)));
    EXPECT_TRUE(Record(0)); // UI/untraced rendering is never suppressed.
    Trace::AdvanceFrame();
    ASSERT_EQ(Trace::GetSnapshot()->draws.size(), 2u);
    EXPECT_TRUE(Trace::GetSnapshot()->draws[0].suppressed);
    EXPECT_FALSE(Trace::GetSnapshot()->draws[0].recorded);
    EXPECT_EQ(Trace::GetStatus().matches, 1u);
    Trace::ClearOverrides(); Trace::AdvanceFrame();
    EXPECT_FALSE(Trace::IsEnabled());
}

TEST_F(DrawTraceTest, AssetIsolationHighlightAndInvalidation) {
    Capture(); Trace::Link(Source()); Record(Submit()); Trace::AdvanceFrame();
    auto snapshot = Trace::GetSnapshot();
    Trace::SetOverride(snapshot->sources[0], &snapshot->submissions[0], Trace::Action::Isolate, Trace::Match::Asset);
    Trace::AdvanceFrame();
    Trace::Link(Source(110, 201));
    EXPECT_TRUE(Record(Submit(110)));
    {
        Trace::FlushScope scope(110);
        Trace::Submission different;
        Trace::CopyName(different.mesh, "Other mesh");
        EXPECT_FALSE(Record(Trace::Submit(different)));
    }
    Trace::SetOverride(snapshot->sources[0], &snapshot->submissions[0], Trace::Action::Highlight, Trace::Match::Asset);
    Trace::AdvanceFrame();
    Trace::Link(Source(110, 201));
    Trace::Source highlighted;
    const auto submission = Submit(110);
    EXPECT_TRUE(Trace::Highlight(submission, highlighted));
    EXPECT_EQ(highlighted.bounds[3], 4);
    Trace::InvalidateSources();
    EXPECT_FALSE(Trace::Highlight(submission, highlighted));
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
    Trace::SetOverride(snapshot->sources[0], &snapshot->submissions[0], Trace::Action::Hide, Trace::Match::Source);
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
}

TEST_F(DrawTraceTest, CollectionBudgetReportsTruncationWithoutBreakingSubmission) {
    Capture();
    for (uintptr_t i = 1; i <= 100000; ++i) Trace::Link(Source(i));
    Trace::Link(Source(1, 999)); // Reused node after overflow cannot inherit stale provenance.
    EXPECT_EQ(Submit(1), 0u);
    EXPECT_TRUE(Record(Submit(100000)));
    Trace::AdvanceFrame();
    auto snapshot = Trace::GetSnapshot();
    EXPECT_TRUE(snapshot->truncated);
    EXPECT_LT(snapshot->sources.size(), 100000u);
}

TEST_F(DrawTraceTest, RetiredAddressCannotReactivateOldSelectionAndUnrelatedDestructionKeepsOverride) {
    Capture(); Trace::Link(Source()); Record(Submit()); Trace::AdvanceFrame();
    const auto old = Trace::GetSnapshot();
    Trace::SetOverride(old->sources[0], &old->submissions[0], Trace::Action::Hide, Trace::Match::Source);
    Trace::AdvanceFrame();
    Trace::InvalidateSource(9999);
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::Hide);
    Trace::InvalidateSource(300);
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
    Capture(); Trace::Link(Source()); Record(Submit()); Trace::AdvanceFrame();
    EXPECT_FALSE(Trace::IsSourceLive(old->sources[0]));
    EXPECT_TRUE(Trace::IsSourceLive(Trace::GetSnapshot()->sources[0]));
    Trace::SetOverride(old->sources[0], &old->submissions[0], Trace::Action::Isolate, Trace::Match::Source);
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
}

TEST_F(DrawTraceTest, SceneContextRestoresAndSourceOverrideIncludesOtherViewports) {
    Capture();
    {
        Trace::SceneScope scene(900, "Main scene", 400);
        Trace::Link(Source()); Record(Submit());
    }
    Trace::Link(Source(101));
    Trace::AdvanceFrame();
    auto snapshot = Trace::GetSnapshot();
    ASSERT_EQ(snapshot->sources.size(), 2u);
    EXPECT_EQ(snapshot->sources[0].scene, 900u);
    EXPECT_STREQ(snapshot->sources[0].sceneName.data(), "Main scene");
    EXPECT_EQ(snapshot->sources[1].scene, 0u);
    Trace::SetOverride(snapshot->sources[0], &snapshot->submissions[0], Trace::Action::Hide, Trace::Match::Source);
    Trace::AdvanceFrame();
    {
        Trace::SceneScope shadow(900, "Main scene", 401);
        Trace::Link(Source()); EXPECT_FALSE(Record(Submit()));
    }
    {
        Trace::SceneScope differentScene(901, "Another scene", 400);
        Trace::Link(Source()); EXPECT_TRUE(Record(Submit()));
    }
}

TEST_F(DrawTraceTest, AssetKeysIncludeNamesBeyondDisplayLength) {
    std::string first(300, 'a'), second = first;
    second.back() = 'b';
    Trace::Name firstDisplay, secondDisplay;
    Trace::CopyName(firstDisplay, first.c_str()); Trace::CopyName(secondDisplay, second.c_str());
    EXPECT_EQ(firstDisplay, secondDisplay);
    EXPECT_NE(Trace::AssetKey(first.c_str()), Trace::AssetKey(second.c_str()));
}

TEST_F(DrawTraceTest, InspectorBuildsEmptyAndPopulatedImguiFramesWithoutGamePointers) {
    auto* previousContext = ImGui::GetCurrentContext();
    auto* context = ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(1280, 900);
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    bool inspectorOpen = true;
    for (int frame = 0; frame < 3; ++frame) {
        if (frame == 1) {
            Capture(); Trace::Link(Source()); Record(Submit()); Trace::AdvanceFrame();
        }
        ImGui::NewFrame();
        Debug::DrawInspector::ShowMenu(&inspectorOpen);
        ImGui::Render();
        EXPECT_TRUE(inspectorOpen);
        EXPECT_TRUE(ImGui::GetDrawData()->Valid);
        if (frame == 2) EXPECT_GT(ImGui::GetDrawData()->TotalVtxCount, 0);
    }
    inspectorOpen = false;
    ImGui::NewFrame();
    Debug::DrawInspector::ShowMenu(&inspectorOpen);
    ImGui::Render();
    ImGui::DestroyContext(context);
    ImGui::SetCurrentContext(previousContext);
}

TEST_F(DrawTraceTest, ViewerShortcutsRejectUnavailableAssetsWithoutDereferencingSnapshotAddresses) {
    auto source = Source();
    Trace::Submission submission;
    Trace::Draw draw;
    Trace::CopyName(draw.texture, "Missing texture");
    EXPECT_FALSE(Debug::Texture::OpenDraw(source, draw));
    EXPECT_FALSE(DebugMeshViewer::OpenDraw(source, submission, draw));
    Capture();
    source.kind = Trace::Kind::Sprite;
    Trace::Link(source);
    Record(Submit());
    Trace::AdvanceFrame();
    const auto snapshot = Trace::GetSnapshot();
    EXPECT_FALSE(Debug::Texture::OpenDraw(snapshot->sources[0], draw));
    EXPECT_FALSE(DebugMeshViewer::OpenDraw(snapshot->sources[0], snapshot->submissions[0], draw));
}
}
