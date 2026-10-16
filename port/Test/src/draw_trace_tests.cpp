#include <gtest/gtest.h>
#include "DrawTrace.h"
#include "../../DebugMenu/src/DebugDrawInspector.h"
#include "../../DebugMenu/src/DebugTexture.h"
#include "../../DebugMenu/src/DebugMeshViewer.h"
#include "imgui.h"
#include "edDlist.h"
#include "Rendering/DisplayList.h"
#include <cstddef>
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

TEST_F(DrawTraceTest, DrawLimitIncludesSelectedDrawAcrossSourcesPassesAndFrames) {
    Capture();
    Trace::Link(Source());
    auto sprite = Source(101, 201); sprite.kind = Trace::Kind::Sprite;
    Trace::Link(sprite);
    const auto first = Submit();
    const auto second = Submit(101, 1);
    Record(first); Record(second); Record(second); Record(first);
    Trace::AdvanceFrame();
    const auto snapshot = Trace::GetSnapshot();
    ASSERT_EQ(snapshot->draws.size(), 4u);
    for (size_t i = 0; i < snapshot->draws.size(); ++i) EXPECT_EQ(snapshot->draws[i].order, i + 1);
    Trace::SetDrawLimit(*snapshot, snapshot->draws[1]);
    EXPECT_FALSE(Trace::IsEnabled()); // Overrides take effect at the next frame boundary.
    EXPECT_EQ(Trace::GetStatus().drawLimit, 2u);
    for (int frame = 0; frame < 2; ++frame) {
        Capture();
        Trace::Link(Source()); Trace::Link(sprite);
        const auto a = Submit(), b = Submit(101, 1);
        EXPECT_TRUE(Record(a));
        EXPECT_FALSE(Record(0)); // Missing native provenance cannot bypass the cutoff.
        Trace::Draw shadow;
        shadow.recorded = true; shadow.pass = 1;
        EXPECT_TRUE(Trace::Record(b, shadow));
        shadow.pass = 2;
        EXPECT_FALSE(Trace::Record(b, shadow)); // Copies of the same submission remain distinct.
        EXPECT_FALSE(Record(a));
        Trace::AdvanceFrame();
        const auto limited = Trace::GetSnapshot();
        ASSERT_EQ(limited->draws.size(), 4u);
        EXPECT_GT(limited->draws[0].id, snapshot->draws.back().id);
        for (size_t i = 0; i < limited->draws.size(); ++i) {
            EXPECT_EQ(limited->draws[i].order, i + 1);
            EXPECT_EQ(limited->draws[i].suppressed, i >= 2);
            EXPECT_EQ(limited->draws[i].recorded, i < 2);
        }
        EXPECT_EQ(Trace::GetStatus().matches, 2u);
    }
    Trace::ClearOverrides(); Trace::AdvanceFrame();
    EXPECT_EQ(Trace::GetStatus().drawLimit, 0u);
    EXPECT_FALSE(Trace::IsEnabled());
    EXPECT_TRUE(Record(0));
}

TEST_F(DrawTraceTest, DrawLimitHandlesFirstLastSelectionAndRejectsStaleSnapshots) {
    Capture(); Trace::Link(Source());
    const auto submission = Submit();
    Record(submission); Record(submission); Record(submission);
    Trace::AdvanceFrame();
    const auto snapshot = Trace::GetSnapshot();
    ASSERT_EQ(snapshot->draws.size(), 3u);
    for (const size_t selected : {size_t{0}, size_t{2}}) {
        Trace::SetDrawLimit(*snapshot, snapshot->draws[selected]);
        Trace::AdvanceFrame();
        Trace::Link(Source());
        const auto current = Submit();
        for (size_t i = 0; i < 3; ++i) EXPECT_EQ(Record(current), i <= selected);
        Trace::AdvanceFrame();
        EXPECT_EQ(Trace::GetStatus().matches, selected + 1);
    }
    Trace::InvalidateSource(300); // A frame position has no dependency on a live object.
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::DrawUpTo);
    Trace::ClearOverrides();
    Trace::Draw missing;
    Trace::SetDrawLimit(*snapshot, missing);
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
    Trace::SetDrawLimit(*snapshot, snapshot->draws[0]);
    Trace::InvalidateSources();
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
    Trace::SetDrawLimit(*snapshot, snapshot->draws[0]);
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
}

TEST_F(DrawTraceTest, NativeMeshesWithoutLinkedSourcesAreCapturedAndLimited) {
    Capture();
    Trace::Submission native;
    Trace::CopyName(native.mesh, "Persistent scenery");
    const auto unlinked = Trace::Submit(native, 700);
    ASSERT_NE(unlinked, 0u);
    EXPECT_TRUE(Record(unlinked));
    Trace::Link(Source()); EXPECT_TRUE(Record(Submit()));
    const auto later = Trace::Submit(native, 701);
    ASSERT_NE(later, 0u);
    EXPECT_TRUE(Record(later));
    Trace::AdvanceFrame();
    const auto snapshot = Trace::GetSnapshot();
    ASSERT_EQ(snapshot->sources.size(), 3u);
    ASSERT_EQ(snapshot->draws.size(), 3u);
    EXPECT_EQ(snapshot->sources[0].kind, Trace::Kind::Native);
    EXPECT_EQ(snapshot->sources[0].object, 700u);
    EXPECT_EQ(snapshot->draws[1].order, 2u);
    Trace::SetDrawLimit(*snapshot, snapshot->draws[0]);
    EXPECT_FALSE(Trace::IsDrawLimitActive());
    Trace::AdvanceFrame();
    EXPECT_TRUE(Trace::IsDrawLimitActive()); // Also gates the later display-list command buffer.
    EXPECT_FALSE(Trace::IsDrawLimitReached());
    EXPECT_TRUE(Record(Trace::Submit(native, 700)));
    EXPECT_TRUE(Trace::IsDrawLimitReached()); // Later fog/flare commands must not cover the partial frame.
    Trace::Link(Source()); EXPECT_FALSE(Record(Submit()));
    EXPECT_FALSE(Record(Trace::Submit(native, 701)));
    EXPECT_FALSE(Record(0));
    EXPECT_FALSE(Record(unlinked)); // A stale submission cannot bypass it either.
    Trace::ClearOverrides(); Trace::AdvanceFrame();
    EXPECT_FALSE(Trace::IsDrawLimitActive());
    EXPECT_TRUE(Record(0));
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

TEST_F(DrawTraceTest, SavedStripAndSpriteSelectionsStayLiveAcrossAlternatingBuffersAndDrawCutoff) {
    for (auto kind : {Trace::Kind::Strip, Trace::Kind::Sprite}) {
        // Pair registration happens at construction, even before tracing is enabled.
        Trace::RegisterSourcePair(1000, 300, 301);
        auto source = Source(); source.kind = kind; source.hierarchy = 0;
        Capture(); Trace::Link(source); Record(Submit()); Trace::AdvanceFrame();
        const auto snapshot = Trace::GetSnapshot();
        ASSERT_NE(snapshot->sources[0].lifetime, 0u);
        const auto selected = snapshot->sources[0];
        Trace::SetDrawLimit(*snapshot, snapshot->draws[0]);
        Trace::AdvanceFrame();
        for (int frame = 0; frame < 6; ++frame) {
            source.object = frame % 2 ? 300 : 301;
            source.node = 110 + frame; // List nodes are transient provenance, not identity.
            Trace::Link(source);
            EXPECT_TRUE(Record(Submit(source.node)));
            EXPECT_FALSE(Record(Submit(source.node)));
            Trace::AdvanceFrame();
            EXPECT_TRUE(Trace::IsSourceLive(selected));
            Trace::Source resolved;
            ASSERT_TRUE(Trace::ResolveSource(selected, resolved));
            EXPECT_EQ(resolved.object, source.object);
            EXPECT_EQ(resolved.lifetime, selected.lifetime);
        }
        // Allocation lifetime also survives culling and switching collection off.
        Trace::AdvanceFrame();
        EXPECT_TRUE(Trace::IsSourceLive(selected));
        Trace::Source resolved;
        EXPECT_FALSE(Trace::ResolveSource(selected, resolved)); // No recent instance to access.
        Trace::ClearOverrides(); Trace::AdvanceFrame();
        EXPECT_TRUE(Trace::IsSourceLive(selected));
        Trace::InvalidateSource(1000);
        EXPECT_FALSE(Trace::IsSourceLive(selected));
    }
}

TEST_F(DrawTraceTest, SavedSourceOverridesMatchBothCopiesButKeepOtherCommandsAndMaterialsSeparate) {
    Trace::RegisterSourcePair(1000, 300, 301);
    Trace::RegisterSourcePair(1000, 302, 303);
    Capture(); Trace::Link(Source()); Record(Submit()); Trace::AdvanceFrame();
    const auto snapshot = Trace::GetSnapshot();
    const auto selected = snapshot->sources[0];
    for (auto action : {Trace::Action::Hide, Trace::Action::Isolate, Trace::Action::Highlight}) {
        Trace::SetOverride(selected, &snapshot->submissions[0], action, Trace::Match::Source);
        Trace::AdvanceFrame();
        for (uintptr_t address : {300, 301, 302, 303}) {
            auto source = Source(); source.object = address;
            Trace::Link(source);
            const auto submission = Submit();
            const bool match = address == 300 || address == 301;
            EXPECT_EQ(Record(submission), action == Trace::Action::Hide ? !match : action == Trace::Action::Isolate ? match : true);
            Trace::Source highlighted;
            EXPECT_EQ(Trace::Highlight(submission, highlighted), action == Trace::Action::Highlight && match);
        }
        auto otherMaterial = Source(); otherMaterial.object = 301; otherMaterial.material = 9;
        Trace::Link(otherMaterial);
        EXPECT_EQ(Record(Submit()), action != Trace::Action::Isolate);
        Trace::AdvanceFrame();
        EXPECT_EQ(Trace::GetStatus().matches, 2u);
    }
    Trace::InvalidateSource(1000);
}

TEST_F(DrawTraceTest, SavedSourceRetiresOnEitherCopyOrContainerAndCannotReviveAfterReuse) {
    for (uintptr_t retired : {300, 301, 1000}) {
        Trace::RegisterSourcePair(1000, 300, 301);
        Capture(); Trace::Link(Source()); Record(Submit()); Trace::AdvanceFrame();
        const auto old = Trace::GetSnapshot();
        Trace::SetOverride(old->sources[0], &old->submissions[0], Trace::Action::Hide, Trace::Match::Source);
        Trace::AdvanceFrame();
        // Invalidate after linking too: publishing this frame must not resurrect it.
        auto peer = Source(); peer.object = 301;
        Trace::Link(peer); Record(Submit());
        Trace::InvalidateSource(retired);
        EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
        Trace::AdvanceFrame();
        EXPECT_FALSE(Trace::IsSourceLive(old->sources[0]));
        Trace::Source resolved;
        EXPECT_FALSE(Trace::ResolveSource(old->sources[0], resolved));
        Trace::RegisterSourcePair(1000, 300, 301);
        Capture(); Trace::Link(peer); Record(Submit()); Trace::AdvanceFrame();
        EXPECT_TRUE(Trace::IsSourceLive(Trace::GetSnapshot()->sources[0]));
        EXPECT_FALSE(Trace::IsSourceLive(old->sources[0]));
        Trace::SetOverride(old->sources[0], &old->submissions[0], Trace::Action::Hide, Trace::Match::Source);
        EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
        Trace::InvalidateSource(1000);
    }
}

TEST_F(DrawTraceTest, RebuildingPairAndSceneInvalidationRejectOldSelectionEvenWhenTracingIsOff) {
    Trace::RegisterSourcePair(1000, 300, 301);
    Capture();
    { Trace::SceneScope scene(900, "Scene", 400); Trace::Link(Source()); Record(Submit()); }
    Trace::AdvanceFrame();
    const auto old = Trace::GetSnapshot();
    ASSERT_FALSE(Trace::IsEnabled());
    Trace::RegisterSourcePair(1000, 300, 301); // Same addresses, new command lifetime.
    EXPECT_FALSE(Trace::IsSourceLive(old->sources[0]));
    Capture();
    { Trace::SceneScope scene(900, "Scene", 400); Trace::Link(Source()); Record(Submit()); }
    Trace::AdvanceFrame();
    const auto current = Trace::GetSnapshot();
    Trace::InvalidateSource(900);
    EXPECT_FALSE(Trace::IsSourceLive(current->sources[0]));
    Trace::SetOverride(current->sources[0], &current->submissions[0], Trace::Action::Hide, Trace::Match::Asset);
    EXPECT_EQ(Trace::GetStatus().action, Trace::Action::None);
    Trace::InvalidateSources();
    EXPECT_FALSE(Trace::IsSourceLive(current->sources[0]));
    Trace::InvalidateSource(1000);
}

TEST_F(DrawTraceTest, GameSavedListResetAndInPlaceRecreationRetireSelections) {
    const uint flags = 0x101; // 3D saved commands, with two primitive buffers.
    const auto bytes = edDListGetBufSizeNeeded(flags, 4, 4, 1, nullptr, nullptr);
    std::vector<std::max_align_t> storage((bytes + sizeof(std::max_align_t) - 1) / sizeof(std::max_align_t));
    auto* list = edDListNew(TO_HEAP(H_MAIN), flags, 4, 4, 1, 0, storage.data());
    const auto container = reinterpret_cast<uintptr_t>(list);
    for (bool recreate : {false, true}) {
        const auto first = reinterpret_cast<uintptr_t>(list[0].pCommandBuffer);
        const auto second = reinterpret_cast<uintptr_t>(list[1].pCommandBuffer);
        Trace::RegisterSourcePair(container, first, second);
        auto source = Source(); source.object = first;
        Capture(); Trace::Link(source); Record(Submit()); Trace::AdvanceFrame();
        const auto old = Trace::GetSnapshot();
        EXPECT_TRUE(Trace::IsSourceLive(old->sources[0]));
        if (recreate) edDListNew(TO_HEAP(H_MAIN), flags, 4, 4, 1, 0, storage.data());
        else edDListPatchableReset(list, 0xffffffff, 0xffffffff, 0xffffffff);
        EXPECT_FALSE(Trace::IsSourceLive(old->sources[0]));
        Trace::Source resolved;
        EXPECT_FALSE(Trace::ResolveSource(old->sources[0], resolved));
    }
    Trace::InvalidateSource(container);
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
        if (frame == 2) {
            const auto snapshot = Trace::GetSnapshot();
            Trace::SetDrawLimit(*snapshot, snapshot->draws[0]);
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
