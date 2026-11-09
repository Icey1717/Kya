#if WITH_DEV_AUTOMATION_TESTS
#include "KyaLevelBankFactory.h"
#include "KyaLevelBankExporter.h"
#include "KyaLevelBank.h"
#include "KyaG3DChunks.h"
#include "KyaG2D.h"
#include "KyaCameraStream.h"
#include "KyaLevelPreview.h"
#include "KyaWaypointComponent.h"
#include "KyaPathPointComponent.h"
#include "KyaActorMarkerComponent.h"
#include "KyaActorTransform.h"
#include "ProceduralMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "Materials/Material.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Serialization/MemoryWriter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBankRoundTripTest, "Kya.Bank.LevelOneRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBankRoundTripTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Material hash folds ASCII case"), KyaG2D::HashName(TEXT("abc")), uint64(0x434241));
    TestEqual(TEXT("Material hash packs eight lanes"), KyaG2D::HashName(TEXT("ABCDEFGH")), uint64(0x4847464544434241));
    TestEqual(TEXT("Material hash wraps lane sums"), KyaG2D::HashName(TEXT("abcdefgha")), uint64(0x4847464544434282));
    {
        TArray<uint8> Data;
        Data.SetNumZeroed(672);
        auto Write = [&Data](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Data[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        FMemory::Memcpy(Data.GetData() + 12, ".G3D", 4);
        Write(16, 0x4c4c4148); Write(24, 80);
        Write(32, 0x48534148); Write(40, 64);
        Write(56, 96); Write(72, 288); Write(88, 480);
        Write(80, 0xb2a49e87); Write(84, 0x4c464e99);
        for (int32 Chunk : { 96, 288, 480 }) { Write(Chunk, 0x52454948); Write(Chunk + 8, 192); }
        Data[112 + 0xac] = 2;
        Write(304 + 0x90, 96); Write(496 + 0x90, 96);
        TArray<int32> CameraRelative;
        if (TestTrue(TEXT("Classify synthetic background"), KyaG3D::ReadCameraRelativeBackgroundHierarchies(Data, CameraRelative)))
        {
            TestEqual(TEXT("Synthetic background excludes fixed hash"), CameraRelative.Num(), 1);
            if (CameraRelative.Num() == 1) TestEqual(TEXT("Synthetic background follows camera"), CameraRelative[0], 304);
        }
        Write(304 + 0x90, 288);
        TestFalse(TEXT("Reject cyclic background hierarchy"), KyaG3D::ReadCameraRelativeBackgroundHierarchies(Data, CameraRelative));
    }
    {
        TArray<uint8> Data;
        Data.SetNumZeroed(640);
        auto Write = [&Data](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Data[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        for (int32 Chunk : { 16, 208, 400 }) FMemory::Memcpy(Data.GetData() + Chunk, "HIER", 4);
        Write(32 + 0x90, 208);
        Write(224 + 0x90, 400);
        const TArray<int32> Roots = { 32, 224 };
        TArray<int32> Expanded;
        if (TestTrue(TEXT("Follow linked cluster hierarchies"), KyaG3D::ExpandClusterHierarchies(Data, Roots, Expanded)))
        {
            TestEqual(TEXT("Linked hierarchy count without duplicates"), Expanded.Num(), 3);
            TestTrue(TEXT("Linked tail included"), Expanded.Contains(416));
        }
        Write(416 + 0x90, 16);
        TestFalse(TEXT("Reject cyclic hierarchy links"), KyaG3D::ExpandClusterHierarchies(Data, Roots, Expanded));
        Write(416 + 0x90, 600);
        TestFalse(TEXT("Reject invalid hierarchy link"), KyaG3D::ExpandClusterHierarchies(Data, Roots, Expanded));
    }
    {
        // CSTA / CDQA / CDQU with a nested CDQU. Each node uses a distinct MBNK.
        TArray<uint8> Data;
        Data.SetNumZeroed(448);
        auto Write = [&Data](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Data[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        FMemory::Memcpy(Data.GetData() + 12, ".G3D", 4);
        Write(16, 0x41545343); Write(24, 224); // CSTA
        Write(32, 0x41514443); Write(40, 208); // CDQA
        Write(80, 0x55514443); Write(88, 160); // root CDQU
        Write(104, 1); Write(144, 240); Write(152, 320);
        Write(160, 0x55514443); Write(168, 80); // nested CDQU
        Write(184, 1); Write(224, 272); Write(232, 384);
        Write(240, 0x4b4e424d); Write(248, 32);
        Write(272, 0x4b4e424d); Write(280, 32);
        Write(304, 0x41544144); Write(312, 144);
        TArray<KyaG3D::FClusterStripReference> Strips;
        if (TestTrue(TEXT("Read nested cluster fixture"), KyaG3D::ReadClusterTreeStrips(Data, Strips)) &&
            TestEqual(TEXT("Nested cluster strip count"), Strips.Num(), 2))
        {
            TestEqual(TEXT("Root strip and bank"), Strips[0].StripOffset, 320);
            TestEqual(TEXT("Root material bank"), Strips[0].MaterialBank, 240);
            TestEqual(TEXT("Child strip and bank"), Strips[1].StripOffset, 384);
            TestEqual(TEXT("Child material bank"), Strips[1].MaterialBank, 272);
        }
        Write(168, 81);
        TestFalse(TEXT("Reject nested cluster exceeding parent"), KyaG3D::ReadClusterTreeStrips(Data, Strips));
    }
    {
        TArray<uint8> PlatformData;
        PlatformData.SetNumZeroed(40);
        PlatformData[4] = 1; PlatformData[8] = 7; PlatformData[12] = 2;
        PlatformData[16] = 1; PlatformData[20] = 9;
        PlatformData[24] = 1; PlatformData[28] = 11; PlatformData[32] = 1; PlatformData[36] = 13;
        FKyaPlatformPrefixOffsets Offsets;
        if (TestTrue(TEXT("Read platform prefix with no-friction actor"), KyaReadPlatformPrefix(PlatformData, 0, PlatformData.Num(), Offsets)))
        {
            TestEqual(TEXT("Platform actor count"), Offsets.ActorCount, 1);
            TestEqual(TEXT("Platform zone count"), Offsets.ZoneCount, 1);
            TestEqual(TEXT("Platform no-friction zone count"), Offsets.NoFrictionZones.Num(), 1);
            TestEqual(TEXT("Platform no-friction zone offset"), Offsets.NoFrictionZones[0], int64(28));
            TestEqual(TEXT("Platform suffix offset"), Offsets.Suffix, int64(40));
        }
        PlatformData.SetNum(39);
        TestFalse(TEXT("Reject truncated no-friction actor list"), KyaReadPlatformPrefix(PlatformData, 0, PlatformData.Num(), Offsets));
    }
    {
        TArray<uint8> Details;
        Details.SetNumZeroed(192);
        Details[0] = 1; Details[152] = 1; Details[172] = 1;
        FKyaPlatformDetailsOffsets Offsets;
        if (TestTrue(TEXT("Read platform camera, property and sound fixture"), KyaReadPlatformDetails(Details, 0, Details.Num(), Offsets)))
        {
            TestEqual(TEXT("Platform camera fixture count"), Offsets.CameraCount, 1);
            TestEqual(TEXT("Platform properties fixture offset"), Offsets.Properties, int64(76));
            TestEqual(TEXT("Platform sound A fixture count"), Offsets.SoundCounts[0], 1);
            TestEqual(TEXT("Platform sound B fixture count"), Offsets.SoundCounts[1], 1);
            TestEqual(TEXT("Platform details fixture end"), Offsets.Tail, int64(192));
        }
        Details.SetNum(191);
        TestFalse(TEXT("Reject truncated platform sound record"), KyaReadPlatformDetails(Details, 0, Details.Num(), Offsets));
    }
    {
        TArray<uint8> CameraStream;
        CameraStream.SetNumZeroed(112 + 4 + 16 + 56);
        CameraStream[112] = 1;
        int64 Conditions = 0, Config = 0;
        TestTrue(TEXT("Locate camera condition record"), KyaFindCameraConditions(CameraStream, 0, Conditions));
        TestEqual(TEXT("Camera condition count offset"), Conditions, int64(112));
        TestTrue(TEXT("Locate configuration after camera condition"), KyaFindCameraConfig(CameraStream, 0, Config));
        TestEqual(TEXT("Camera configuration offset"), Config, int64(132));
        CameraStream.SetNum(130);
        TestFalse(TEXT("Reject truncated camera condition"), KyaFindCameraConditions(CameraStream, 0, Conditions));
    }
    TestTrue(TEXT("Actor yaw rotates forward toward game X"), KyaActorTransform::ApplyDefaultRotation(
        FVector(0, 0, 1), FVector(0, PI / 2, 0)).Equals(FVector(1, 0, 0), 0.0001));
    TestTrue(TEXT("Actor pitch rotates forward toward negative game Y"), KyaActorTransform::ApplyDefaultRotation(
        FVector(0, 0, 1), FVector(PI / 2, 0, 0)).Equals(FVector(0, -1, 0), 0.0001));
    TestTrue(TEXT("Actor roll rotates right toward game Y"), KyaActorTransform::ApplyDefaultRotation(
        FVector(1, 0, 0), FVector(0, 0, PI / 2)).Equals(FVector(0, 1, 0), 0.0001));
    TestTrue(TEXT("Yaw-only actor class removes pitch"), KyaActorTransform::ApplyClassRotation(
        FVector(0, 0, 1), FVector(PI / 4, 0, 0), 6).Equals(FVector(0, 0, 1), 0.0001));
    TestTrue(TEXT("Euler-matrix actor class retains pitch"), KyaActorTransform::ApplyClassRotation(
        FVector(0, 0, 1), FVector(PI / 4, 0, 0), 1).Equals(FVector(0, -FMath::Sqrt(0.5), FMath::Sqrt(0.5)), 0.0001));
    {
        // One block with one operation and one condition.
        TArray<uint8> Stream;
        Stream.SetNumZeroed(40);
        Stream[0] = 1; Stream[4] = 1; Stream[20] = 1;
        TArray<FKyaConditionedOffsets> Blocks;
        int64 End = 0;
        if (TestTrue(TEXT("Read conditioned operation stream"), KyaReadConditionedOffsets(Stream, 0, Blocks, End)))
        {
            TestEqual(TEXT("Conditioned block count"), Blocks.Num(), 1);
            TestEqual(TEXT("Operation position"), Blocks[0].Operations[0], int64(8));
            TestEqual(TEXT("Condition position"), Blocks[0].Conditions[0], int64(24));
            TestEqual(TEXT("Conditioned stream end"), End, int64(40));
        }
        TestFalse(TEXT("Reject truncated conditioned operation stream"), KyaReadConditionedOffsets(
            MakeArrayView(Stream.GetData(), Stream.Num() - 1), 0, Blocks, End));
    }
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the level bank round-trip test."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import bank"), Bank)) return false;
    TestTrue(TEXT("Imported bank has current schema"), Bank->HasCurrentImportSchema());
    Bank->ImportSchemaVersion = 0;
    TestFalse(TEXT("Old bank schema is detected"), Bank->HasCurrentImportSchema());
    Bank->ImportSchemaVersion = UKyaLevelBank::CurrentImportSchemaVersion;
    UWorld* PreviewWorld = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create preview world"), PreviewWorld)) return false;
    AKyaLevelPreview* Preview = PreviewWorld->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn level preview"), Preview))
    {
        PreviewWorld->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->OnConstruction(FTransform::Identity);
    Preview->PreviewPathIndex = 0;
    Preview->PreviewActorIndex = 0;
    Preview->OnConstruction(FTransform::Identity);
    auto PreviewVertexCount = [&]() {
        int32 Count = 0;
        for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
            if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section)) Count += MeshSection->ProcVertexBuffer.Num();
        return Count;
    };
    const int32 VerticesWithoutActor = PreviewVertexCount();
    int32 ActorWithMesh = INDEX_NONE;
    for (int32 ActorIndex = 0; ActorIndex < Bank->SceneEnvironment.Actors.Num() && ActorWithMesh == INDEX_NONE; ++ActorIndex)
    {
        int32 Ordinal = Bank->SceneEnvironment.Actors[ActorIndex].MeshIndex;
        if (Ordinal < 0) continue;
        for (const FKyaBankEntry& MeshEntry : Bank->Entries)
            if (MeshEntry.Type == 4 && MeshEntry.Subtype == 1 && Ordinal-- == 0)
            {
                TArray<FKyaG3DDecodedStrip> ModelStrips;
                if (KyaG3D::DecodeHierarchyStrips(MakeArrayView(Input.GetData() + MeshEntry.Offset, int32(MeshEntry.Size)), ModelStrips) && !ModelStrips.IsEmpty()) ActorWithMesh = ActorIndex;
                break;
            }
    }
    if (TestTrue(TEXT("Find a supported level-one actor mesh"), ActorWithMesh != INDEX_NONE))
    {
        Preview->PreviewActorIndex = ActorWithMesh;
        Preview->bShowActorMesh = true;
        Preview->OnConstruction(FTransform::Identity);
        TestTrue(TEXT("Actor mesh adds preview geometry"), PreviewVertexCount() > VerticesWithoutActor);
        Preview->bShowActorMesh = false;
        Preview->PreviewActorIndex = 0;
        Preview->OnConstruction(FTransform::Identity);
    }
    TArray<UKyaActorMarkerComponent*> ActorMarkers;
    Preview->GetComponents<UKyaActorMarkerComponent>(ActorMarkers);
    if (TestEqual(TEXT("Actor marker count"), ActorMarkers.Num(), 1))
    {
        const FVector OriginalActorPosition = Bank->SceneEnvironment.Actors[0].Position;
        ActorMarkers[0]->SetRelativeLocation(FVector(100, 200, 300));
        ActorMarkers[0]->PostEditComponentMove(true);
        TestEqual(TEXT("Moving actor marker updates bank"), Bank->SceneEnvironment.Actors[0].Position, FVector(1, 3, 2));
        Bank->SceneEnvironment.Actors[0].Position = OriginalActorPosition;
        Preview->OnConstruction(FTransform::Identity);
    }
    TArray<USplineComponent*> PathPreviews;
    Preview->GetComponents<USplineComponent>(PathPreviews);
    if (TestEqual(TEXT("Path preview component count"), PathPreviews.Num(), 1))
    {
        TestEqual(TEXT("Path preview point count"), PathPreviews[0]->GetNumberOfSplinePoints(), Bank->SceneEnvironment.Paths[0].Points.Num());
        const FVector4 P = Bank->SceneEnvironment.Paths[0].Points[0];
        TestTrue(TEXT("Path preview coordinate conversion"), PathPreviews[0]->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local).Equals(FVector(P.X, P.Z, P.Y) * 100.0));
        if (PathPreviews[0]->GetNumberOfSplinePoints() >= 2)
        {
            const FVector A = PathPreviews[0]->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
            const FVector B = PathPreviews[0]->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local);
            TestTrue(TEXT("Path guide interpolates linearly between points"),
                PathPreviews[0]->GetLocationAtSplineInputKey(0.5f, ESplineCoordinateSpace::Local).Equals((A + B) * 0.5));
        }
        TArray<UKyaPathPointComponent*> PathMarkers;
        Preview->GetComponents<UKyaPathPointComponent>(PathMarkers);
        if (TestEqual(TEXT("Path marker count"), PathMarkers.Num(), Bank->SceneEnvironment.Paths[0].Points.Num()) && !PathMarkers.IsEmpty())
        {
            UKyaPathPointComponent** Found = PathMarkers.FindByPredicate([](const UKyaPathPointComponent* Item) { return Item->PointIndex == 0; });
            if (TestNotNull(TEXT("First path marker"), Found))
            {
                (*Found)->SetRelativeLocation(FVector(100, 200, 300));
                (*Found)->PostEditComponentMove(true);
                TestTrue(TEXT("Moving path point updates bank and preserves W"), Bank->SceneEnvironment.Paths[0].Points[0].Equals(FVector4(1, 3, 2, P.W)));
                TestTrue(TEXT("Moving path point updates guide"), PathPreviews[0]->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local).Equals(FVector(100, 200, 300)));
                Bank->SceneEnvironment.Paths[0].Points[0] = P;
                Preview->OnConstruction(FTransform::Identity);
            }
        }
    }
    TArray<UKyaWaypointComponent*> Markers;
    Preview->GetComponents<UKyaWaypointComponent>(Markers);
    if (TestEqual(TEXT("Waypoint marker count"), Markers.Num(), Bank->SceneEnvironment.Waypoints.Num()) && !Markers.IsEmpty())
    {
        UKyaWaypointComponent* Marker = *Markers.FindByPredicate([](const UKyaWaypointComponent* Item) { return Item->WaypointIndex == 0; });
        const FVector OriginalPosition = Bank->SceneEnvironment.Waypoints[0].Position;
        Marker->SetRelativeLocation(FVector(100, 200, 300));
        Marker->PostEditComponentMove(true);
        TestEqual(TEXT("Moving waypoint marker updates game coordinates"), Bank->SceneEnvironment.Waypoints[0].Position, FVector(1, 3, 2));
        Preview->OnConstruction(FTransform::Identity);
        TArray<UKyaWaypointComponent*> ReconstructedMarkers;
        Preview->GetComponents<UKyaWaypointComponent>(ReconstructedMarkers);
        TestTrue(TEXT("Construction preserves selected marker identity"), ReconstructedMarkers.Contains(Marker));
        Bank->SceneEnvironment.Waypoints[0].Position = OriginalPosition;
        Preview->OnConstruction(FTransform::Identity);
        TestTrue(TEXT("Reconstruction restores marker position from bank"), Marker->GetRelativeLocation().Equals(
            FVector(OriginalPosition.X, OriginalPosition.Z, OriginalPosition.Y) * 100.0));
    }
    TestTrue(TEXT("Default preview creates textured sections without a material override"), Preview->Mesh->GetNumSections() > 1);
    bool HasPreviewTriangles = false;
    for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
        if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section))
            HasPreviewTriangles |= !MeshSection->ProcIndexBuffer.IsEmpty();
    TestTrue(TEXT("Preview has background geometry"), HasPreviewTriangles);
    TestTrue(TEXT("Untextured preview uses a generated material"), Preview->Mesh->GetMaterial(0) &&
        Preview->Mesh->GetMaterial(0) != UMaterial::GetDefaultMaterial(MD_Surface));
    auto CountPreviewVertices = [&]() {
        int64 Count = 0;
        for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
            if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section))
                Count += MeshSection->ProcVertexBuffer.Num();
        return Count;
    };
    const int64 BackgroundVertices = CountPreviewVertices();
    Preview->bShowAllActorMeshes = true;
    Preview->OnConstruction(FTransform::Identity);
    TestTrue(TEXT("All actor preview adds model geometry"), CountPreviewVertices() > BackgroundVertices);
    Preview->bShowAllActorMeshes = false;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Disabling all actor preview restores background geometry"), CountPreviewVertices(), BackgroundVertices);
    const FString SectorFilename = FPaths::Combine(FPaths::GetPath(Filename), TEXT("SECT1.BNK"));
    TArray<uint8> SectorInput;
    if (TestTrue(TEXT("Read sector bank"), FFileHelper::LoadFileToArray(SectorInput, *SectorFilename)))
    {
        const uint8* SectorCursor = SectorInput.GetData();
        UKyaLevelBank* Sector = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), SectorCursor,
            SectorInput.GetData() + SectorInput.Num(), GWarn));
        if (TestNotNull(TEXT("Import sector bank"), Sector))
        {
            const FKyaBankEntry* SectorMesh = Sector->Entries.FindByPredicate([](const FKyaBankEntry& Entry) {
                return Entry.Type == 4 && Entry.Subtype == 1;
            });
            if (TestNotNull(TEXT("Sector mesh entry"), SectorMesh))
            {
                TArray<int32> ClusterStrips;
                int32 ClusterMaterialBank = INDEX_NONE;
                TestTrue(TEXT("Read sector cluster strips"), KyaG3D::ReadClusterStrips(
                    MakeArrayView(SectorInput.GetData() + SectorMesh->Offset, int32(SectorMesh->Size)), ClusterStrips, &ClusterMaterialBank));
                TestEqual(TEXT("Sector cluster strip count"), ClusterStrips.Num(), 17);
                const auto MeshData = MakeArrayView(SectorInput.GetData() + SectorMesh->Offset, int32(SectorMesh->Size));
                TArray<KyaG3D::FClusterStripReference> TreeStrips;
                TArray<int32> ClusterHierarchies;
                if (TestTrue(TEXT("Read nested sector cluster strips"), KyaG3D::ReadClusterTreeStrips(MeshData, TreeStrips, &ClusterHierarchies)))
                {
                    TestTrue(TEXT("Cluster tree includes root strips"), TreeStrips.Num() >= ClusterStrips.Num());
                    TestEqual(TEXT("Sector one cluster hierarchy count"), ClusterHierarchies.Num(), 6);
                    TArray<int32> Expanded;
                    TestTrue(TEXT("Follow sector one hierarchy links"), KyaG3D::ExpandClusterHierarchies(MeshData, ClusterHierarchies, Expanded));
                    TestTrue(TEXT("Sector one linked hierarchies include roots"), Expanded.Num() >= ClusterHierarchies.Num());
                    TArray<FKyaG3DStripReference> HierarchyStrips;
                    if (TestTrue(TEXT("Read sector hierarchy strips"), KyaG3D::ReadHierarchyStrips(MeshData, HierarchyStrips)))
                    {
                        int32 Referenced = 0;
                        for (const FKyaG3DStripReference& Strip : HierarchyStrips)
                            if (ClusterHierarchies.Contains(Strip.HierarchyOffset)) ++Referenced;
                        TestEqual(TEXT("Sector one cluster hierarchy strip count"), Referenced, 589);
                    }
                }
                int32 ResolvedMaterials = 0;
                for (int32 Strip : ClusterStrips)
                {
                    uint64 MaterialHash = 0;
                    if (KyaG3D::ReadStripMaterialHash(MeshData, Strip, ClusterMaterialBank, MaterialHash)) ++ResolvedMaterials;
                    uint32 Count = 0, Primitive = 0;
                    TArray<FKyaG3DVertex> Vertices;
                    TArray<FKyaG3DVertexAttributes> Attributes;
                    TArray<uint32> Sections, Indices;
                    if (!KyaG3D::ReadFirstStripGifTag(MeshData, Strip, Count, Primitive) ||
                        !KyaG3D::ReadStripVertices(MeshData, Strip, Vertices, Sections) ||
                        !KyaG3D::ReadStripAttributes(MeshData, Strip, Attributes) ||
                        !KyaG3D::BuildStripTriangles(Vertices, Sections, Primitive, Indices))
                        AddError(FString::Printf(TEXT("Cannot decode sector cluster strip at 0x%x"), Strip));
                }
                TestTrue(TEXT("Sector cluster material hashes resolve"), ResolvedMaterials > 0);
            }
            Preview->SectorBanks.Add(Sector);
            Preview->OnConstruction(FTransform::Identity);
            TestTrue(TEXT("Sector adds preview geometry"), CountPreviewVertices() > BackgroundVertices);
            FKyaBankEntry* SectorBackground = Sector->Entries.FindByPredicate([](const FKyaBankEntry& Item) {
                return Item.Type == 4 && Item.Subtype == 2;
            });
            if (TestNotNull(TEXT("Sector one background entry"), SectorBackground))
            {
                TArray<FKyaG3DDecodedStrip> BackgroundStrips;
                const auto BackgroundData = MakeArrayView(SectorInput.GetData() + SectorBackground->Offset, int32(SectorBackground->Size));
                TArray<int32> BackgroundHierarchies, CameraRelative;
                TestTrue(TEXT("Read sector background hierarchies"), KyaG3D::ReadHierarchyOffsets(BackgroundData, BackgroundHierarchies));
                if (TestTrue(TEXT("Classify sector background camera-follow hierarchies"), KyaG3D::ReadCameraRelativeBackgroundHierarchies(BackgroundData, CameraRelative)))
                {
                    TestEqual(TEXT("Sector one camera-follow hierarchy count"), CameraRelative.Num(), 1);
                    if (CameraRelative.Num() == 1)
                    {
                        FMatrix AtOrigin, AtCamera;
                        const FVector Camera(12, 34, 56);
                        if (TestTrue(TEXT("Read background matrix at origin"), KyaG3D::ReadBackgroundWorldMatrix(BackgroundData, CameraRelative[0], CameraRelative, FVector::ZeroVector, AtOrigin)) &&
                            TestTrue(TEXT("Read background matrix at camera"), KyaG3D::ReadBackgroundWorldMatrix(BackgroundData, CameraRelative[0], CameraRelative, Camera, AtCamera)))
                            TestTrue(TEXT("Camera affects background transform"), !AtOrigin.Equals(AtCamera));
                    }
                }
                if (TestTrue(TEXT("Decode sector background for visibility test"), KyaG3D::DecodeHierarchyStrips(
                    BackgroundData, BackgroundStrips)))
                {
                    int64 SectorBackgroundVertices = 0;
                    for (const FKyaG3DDecodedStrip& Strip : BackgroundStrips) SectorBackgroundVertices += Strip.Vertices.Num();
                    const int64 WithSectorBackground = CountPreviewVertices();
                    SectorBackground->Subtype = 0;
                    Preview->OnConstruction(FTransform::Identity);
                    TestEqual(TEXT("Sector background replaces common background"),
                        WithSectorBackground - CountPreviewVertices(), SectorBackgroundVertices - BackgroundVertices);
                    SectorBackground->Subtype = 2;
                    Preview->OnConstruction(FTransform::Identity);
                }
            }
            const int64 OriginalBackgroundFlags = Bank->SceneEnvironment.BackgroundFlags;
            auto CollectPreviewPositions = [&]() {
                TArray<FVector> Result;
                for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
                    if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section))
                        for (const FProcMeshVertex& Vertex : MeshSection->ProcVertexBuffer) Result.Add(Vertex.Position);
                return Result;
            };
            Bank->SceneEnvironment.BackgroundFlags |= 1;
            Preview->OnConstruction(FTransform::Identity);
            Preview->SetBackgroundCameraPosition(FVector::ZeroVector);
            const TArray<FVector> AtOrigin = CollectPreviewPositions();
            Preview->SetBackgroundCameraPosition(FVector(1200, 3400, 5600));
            const TArray<FVector> AtCamera = CollectPreviewPositions();
            int32 MovedVertices = 0;
            if (TestEqual(TEXT("Camera movement preserves vertex count"), AtCamera.Num(), AtOrigin.Num()))
                for (int32 Index = 0; Index < AtCamera.Num(); ++Index)
                    if (!AtOrigin[Index].Equals(AtCamera[Index], 0.01)) ++MovedVertices;
            TestTrue(TEXT("Camera-follow background moves"), MovedVertices > 0 && MovedVertices < AtCamera.Num());
            Bank->SceneEnvironment.BackgroundFlags &= ~int64(1);
            Preview->OnConstruction(FTransform::Identity);
            const TArray<FVector> FixedBackground = CollectPreviewPositions();
            Preview->SetBackgroundCameraPosition(FVector(1200, 3400, 5600));
            const TArray<FVector> FixedAfterCamera = CollectPreviewPositions();
            TestTrue(TEXT("Disabled background flag keeps all vertices fixed"), FixedBackground == FixedAfterCamera);
            Bank->SceneEnvironment.BackgroundFlags = OriginalBackgroundFlags;
            Preview->OnConstruction(FTransform::Identity);
            bool HasMaskedMaterial = false;
            for (int32 Section = 1; Section < Preview->Mesh->GetNumSections(); ++Section)
                if (UMaterialInterface* Material = Preview->Mesh->GetMaterial(Section))
                    HasMaskedMaterial |= Material->GetBlendMode() == BLEND_Masked;
            TestTrue(TEXT("Alpha-tested sector materials use masked preview"), HasMaskedMaterial);
            Preview->TextureMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
            Preview->OnConstruction(FTransform::Identity);
            TestTrue(TEXT("Supported background textures create material sections"), Preview->Mesh->GetNumSections() > 1);
            const int32 BackgroundTextureEntry = Bank->BackgroundTextureEntry;
            Bank->BackgroundTextureEntry = INDEX_NONE;
            Preview->OnConstruction(FTransform::Identity);
            TestTrue(TEXT("Sector background creates textured sections independently"), Preview->Mesh->GetNumSections() > 1);
            Bank->BackgroundTextureEntry = BackgroundTextureEntry;
        }
    }
    {
        TArray<uint8> Sector7Input;
        const FString Sector7Filename = FPaths::Combine(FPaths::GetPath(Filename), TEXT("SECT7.BNK"));
        if (TestTrue(TEXT("Read sector seven bank"), FFileHelper::LoadFileToArray(Sector7Input, *Sector7Filename)))
        {
            const uint8* SectorCursor = Sector7Input.GetData();
            UKyaLevelBank* Sector = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
                GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), SectorCursor,
                Sector7Input.GetData() + Sector7Input.Num(), GWarn));
            if (TestNotNull(TEXT("Import sector seven bank"), Sector))
            {
                Preview->SectorBanks.Reset();
                Preview->SectorBanks.Add(Sector);
                Preview->OnConstruction(FTransform::Identity);
                TestTrue(TEXT("Hierarchy-only sector adds preview geometry"), CountPreviewVertices() > BackgroundVertices);
                const FKyaBankEntry* SectorMesh = Sector->Entries.FindByPredicate([](const FKyaBankEntry& Entry) {
                    return Entry.Type == 4 && Entry.Subtype == 1;
                });
                if (TestNotNull(TEXT("Sector seven mesh entry"), SectorMesh))
                {
                    const auto MeshData = MakeArrayView(Sector7Input.GetData() + SectorMesh->Offset, int32(SectorMesh->Size));
                    TArray<KyaG3D::FClusterStripReference> StaticStrips;
                    TArray<int32> Hierarchies;
                    if (TestTrue(TEXT("Read sector seven cluster"), KyaG3D::ReadClusterTreeStrips(MeshData, StaticStrips, &Hierarchies)))
                    {
                        TestEqual(TEXT("Sector seven has no static strips"), StaticStrips.Num(), 0);
                        TestEqual(TEXT("Sector seven cluster hierarchy count"), Hierarchies.Num(), 1);
                        TArray<int32> Expanded;
                        TestTrue(TEXT("Follow sector seven hierarchy links"), KyaG3D::ExpandClusterHierarchies(MeshData, Hierarchies, Expanded));
                        TestTrue(TEXT("Sector seven linked hierarchies include root"), Expanded.Num() >= Hierarchies.Num());
                        TArray<FKyaG3DStripReference> References;
                        if (TestTrue(TEXT("Read sector seven hierarchy strips"), KyaG3D::ReadHierarchyStrips(MeshData, References)))
                        {
                            int32 Referenced = 0;
                            for (const FKyaG3DStripReference& Strip : References)
                                if (Hierarchies.Contains(Strip.HierarchyOffset)) ++Referenced;
                            TestEqual(TEXT("Sector seven referenced hierarchy strips"), Referenced, 27);
                        }
                    }
                }
            }
        }
    }
    {
        const FKyaBankEntry& CommonEntry = Bank->Entries[Bank->SceneTextureEntry];
        const auto CommonData = MakeArrayView(Input.GetData() + CommonEntry.Offset, int32(CommonEntry.Size));
        TMap<uint64, int32> CommonMaterials;
        KyaG2D::ReadMaterials(CommonData, CommonMaterials);
        for (int32 SectorNumber = 1; SectorNumber <= 13; ++SectorNumber)
        {
            TArray<uint8> Bytes;
            const FString Path = FPaths::Combine(FPaths::GetPath(Filename), FString::Printf(TEXT("SECT%d.BNK"), SectorNumber));
            if (!FFileHelper::LoadFileToArray(Bytes, *Path)) continue;
            const uint8* Position = Bytes.GetData();
            UKyaLevelBank* Sector = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
                GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Position, Bytes.GetData() + Bytes.Num(), GWarn));
            if (!Sector) continue;
            const FKyaBankEntry* MeshEntry = Sector->Entries.FindByPredicate([](const FKyaBankEntry& E) { return E.Type == 4 && E.Subtype == 1; });
            const FKyaBankEntry* TextureEntry = Sector->Entries.FindByPredicate([](const FKyaBankEntry& E) { return E.Type == 5 && E.Subtype == 1; });
            if (!MeshEntry || !TextureEntry) continue;
            const auto MeshData = MakeArrayView(Bytes.GetData() + MeshEntry->Offset, int32(MeshEntry->Size));
            const auto TextureData = MakeArrayView(Bytes.GetData() + TextureEntry->Offset, int32(TextureEntry->Size));
            TMap<uint64, int32> Materials;
            KyaG2D::ReadMaterials(TextureData, Materials);
            TArray<KyaG3D::FClusterStripReference> Strips;
            if (!KyaG3D::ReadClusterTreeStrips(MeshData, Strips)) continue;
            int32 NoHash = 0, Missing = 0, Decoded = 0, Failed = 0;
            TMap<uint32, int32> Formats;
            TMap<FString, int32> FailureReasons;
            TMap<FString, int32> AlphaTests;
            for (const auto& Strip : Strips)
            {
                uint64 Hash = 0;
                if (!KyaG3D::ReadStripMaterialHash(MeshData, Strip.StripOffset, Strip.MaterialBank, Hash)) { ++NoHash; continue; }
                const int32* Offset = Materials.Find(Hash);
                const bool Local = Offset != nullptr;
                if (!Offset) Offset = CommonMaterials.Find(Hash);
                if (!Offset) { ++Missing; continue; }
                const TConstArrayView<uint8> Source = Local ? TConstArrayView<uint8>(TextureData) : TConstArrayView<uint8>(CommonData);
                uint64 Tex0 = 0;
                if (KyaG2D::ReadMaterialTex0(Source, *Offset, 0, Tex0)) ++Formats.FindOrAdd(uint32((Tex0 >> 20) & 63));
                uint64 TestRegister = 0;
                if (KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x47, TestRegister))
                    ++AlphaTests.FindOrAdd(FString::Printf(TEXT("ATE=%d ATST=%d AREF=%d AFAIL=%d"),
                        uint32(TestRegister & 1), uint32((TestRegister >> 1) & 7),
                        uint32((TestRegister >> 4) & 255), uint32((TestRegister >> 12) & 3)));
                else ++AlphaTests.FindOrAdd(TEXT("no TEST register"));
                int32 Width = 0, Height = 0;
                TArray<FColor> Pixels;
                if (KyaG2D::DecodeMaterialTexture(Source, *Offset, Width, Height, Pixels)) ++Decoded;
                else
                {
                    ++Failed;
                    KyaG2D::FBitmapReference Bitmap;
                    TArray<TPair<int32, int32>> UploadLists;
                    TArray<KyaG2D::FUploadTransfer> Uploads;
                    FString Reason;
                    if (!KyaG2D::ReadMaterialBitmap(Source, *Offset, Bitmap)) Reason = TEXT("bitmap");
                    else if (!KyaG2D::ReadMaterialTex0(Source, *Offset, 0, Tex0)) Reason = TEXT("tex0");
                    else if (!KyaG2D::ReadUploadLists(Source, Bitmap.PaletteOffset == INDEX_NONE ? Bitmap.BitmapOffset : Bitmap.PaletteOffset, UploadLists)) Reason = TEXT("upload lists");
                    else if (UploadLists.IsEmpty() || !KyaG2D::ReadUploadTransfers(Source, UploadLists[0].Key, UploadLists[0].Value, Uploads)) Reason = TEXT("upload transfers");
                    else
                    {
                        TArray<uint8> Memory;
                        Memory.SetNumZeroed(4 * 1024 * 1024);
                        bool UploadOk = true;
                        for (const auto& Upload : Uploads)
                            if (Upload.DestinationFormat() == 0 ? !KyaG2D::WriteGs32(Source, Upload, Memory) : !KyaG2D::WriteGsIndexed(Source, Upload, Memory)) UploadOk = false;
                        if (!UploadOk)
                        {
                            Reason = TEXT("upload format");
                            for (const auto& Upload : Uploads) Reason += FString::Printf(TEXT(" %x"), Upload.DestinationFormat());
                        }
                        else
                        {
                            TArray<uint8> Indices;
                            TArray<FColor> Colors;
                            const uint32 Format = uint32((Tex0 >> 20) & 63);
                            if (Format == 0 && !KyaG2D::ReadGs32(Memory, Tex0, Colors)) Reason = TEXT("direct pixels");
                            else if (Format == 0x13 && !KyaG2D::ReadGs8(Memory, Tex0, Indices)) Reason = TEXT("8-bit pixels");
                            else if (Format == 0x14 && !KyaG2D::ReadGs4(Memory, Tex0, Indices)) Reason = TEXT("4-bit pixels");
                            else Reason = FString::Printf(TEXT("palette format=%x csm=%d csa=%d psm=%x"),
                                uint32((Tex0 >> 51) & 15), uint32((Tex0 >> 55) & 1), uint32((Tex0 >> 56) & 31), Format);
                        }
                    }
                    ++FailureReasons.FindOrAdd(Reason);
                }
            }
            AddInfo(FString::Printf(TEXT("SECT%d cluster materials: strips=%d no-hash=%d missing=%d decoded=%d decode-failed=%d"),
                SectorNumber, Strips.Num(), NoHash, Missing, Decoded, Failed));
            for (const auto& Format : Formats)
                AddInfo(FString::Printf(TEXT("SECT%d TEX0 format 0x%x: %d strips"), SectorNumber, Format.Key, Format.Value));
            for (const auto& Reason : FailureReasons)
                AddInfo(FString::Printf(TEXT("SECT%d texture failure %s: %d strips"), SectorNumber, *Reason.Key, Reason.Value));
            for (const auto& Alpha : AlphaTests)
                AddInfo(FString::Printf(TEXT("SECT%d alpha %s: %d strips"), SectorNumber, *Alpha.Key, Alpha.Value));
        }
    }
    PreviewWorld->DestroyWorld(false);
    TestEqual(TEXT("Entry count"), Bank->EntryCount, 1531);
    TestEqual(TEXT("Scene mesh entry"), Bank->SceneMeshEntry, 278);
    TestEqual(TEXT("Scene texture entry"), Bank->SceneTextureEntry, 108);
    TestEqual(TEXT("Background mesh entry"), Bank->BackgroundMeshEntry, 240);
    TestEqual(TEXT("Background texture entry"), Bank->BackgroundTextureEntry, 170);
    if (Bank->Entries.IsValidIndex(Bank->BackgroundTextureEntry))
    {
        const FKyaBankEntry& Texture = Bank->Entries[Bank->BackgroundTextureEntry];
        TMap<uint64, int32> Materials;
        if (TestTrue(TEXT("Read background G2D materials"), KyaG2D::ReadMaterials(
            MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), Materials)))
        {
            const int32* Material = Materials.Find(0x197097db0edf22dbULL);
            if (TestNotNull(TEXT("Background contains first mesh material"), Material))
            {
                uint64 Tex0 = 0;
                TestTrue(TEXT("Read material TEX0"), KyaG2D::ReadMaterialTex0(
                    MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), *Material, 0, Tex0));
                TestEqual(TEXT("Material TEX0 value"), Tex0, uint64(0x60007005dd308000ULL));
                int32 DecodedWidth = 0, DecodedHeight = 0;
                TArray<FColor> DecodedPixels;
                if (TestTrue(TEXT("Decode indexed material texture"), KyaG2D::DecodeMaterialTexture(
                    MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), *Material,
                    DecodedWidth, DecodedHeight, DecodedPixels)))
                {
                    TestEqual(TEXT("Decoded material width"), DecodedWidth, 128);
                    TestEqual(TEXT("Decoded material height"), DecodedHeight, 128);
                    TestEqual(TEXT("Decoded material pixels"), DecodedPixels.Num(), 128 * 128);
                }
                KyaG2D::FBitmapReference Bitmap;
                if (TestTrue(TEXT("Resolve material bitmap"), KyaG2D::ReadMaterialBitmap(
                    MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), *Material, Bitmap)))
                {
                    TestEqual(TEXT("Bitmap width"), Bitmap.Width, uint16(128));
                    TestEqual(TEXT("Bitmap height"), Bitmap.Height, uint16(128));
                    TestEqual(TEXT("Bitmap format"), Bitmap.PixelFormat, uint16(8));
                    TestEqual(TEXT("Bitmap mip count"), Bitmap.MipCount, uint16(3));
                    TestEqual(TEXT("Bitmap chunk size"), Bitmap.BitmapChunkSize, 0x5420);
                    TestTrue(TEXT("Bitmap has palette"), Bitmap.PaletteOffset != INDEX_NONE);
                    TArray<TPair<int32, int32>> UploadLists;
                    if (TestTrue(TEXT("Resolve palette upload lists"), KyaG2D::ReadUploadLists(
                        MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), Bitmap.PaletteOffset, UploadLists)))
                    {
                        TestEqual(TEXT("First upload packet count"), UploadLists[0].Value, 37);
                        TArray<KyaG2D::FUploadTransfer> Transfers;
                        if (TestTrue(TEXT("Read upload transfers"), KyaG2D::ReadUploadTransfers(
                            MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), UploadLists[0].Key, UploadLists[0].Value, Transfers)))
                        {
                            TestEqual(TEXT("Upload transfer count"), Transfers.Num(), 4);
                            if (Transfers.Num() == 4)
                            {
                                TestEqual(TEXT("First transfer width"), Transfers[0].Width(), uint32(64));
                                TestEqual(TEXT("First transfer height"), Transfers[0].Height(), uint32(64));
                                TestEqual(TEXT("First transfer format"), Transfers[0].DestinationFormat(), uint32(0));
                                TestEqual(TEXT("First transfer bytes"), Transfers[0].DataSize, 16384);
                                TArray<uint8> GsMemory;
                                GsMemory.SetNumZeroed(4 * 1024 * 1024);
                                TestTrue(TEXT("Apply first GS transfer"), KyaG2D::WriteGs32(
                                    MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), Transfers[0], GsMemory));
                                TestEqual(TEXT("First uploaded byte"), GsMemory[Transfers[0].DestinationBase() * 256],
                                    Input[int32(Texture.Offset) + Transfers[0].DataOffset]);
                                TArray<uint8> Indices;
                                TestTrue(TEXT("Read uploaded PSMT8 texture"), KyaG2D::ReadGs8(GsMemory, Tex0, Indices));
                                TestEqual(TEXT("Decoded texture pixel count"), Indices.Num(), 128 * 128);
                            }
                        }
                    }
                    TArray<FColor> Palette;
                    if (TestTrue(TEXT("Read PA32 palette"), KyaG2D::ReadPalette32(
                        MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), Bitmap.PaletteOffset, Palette)))
                    {
                        TestEqual(TEXT("Palette entries"), Palette.Num(), 256);
                        if (!Palette.IsEmpty()) TestEqual(TEXT("First palette entry"), Palette[0], FColor(0xb2, 0x90, 0x51, 0x80));
                    }
                }
            }
        }
    }
    {
        // Independent GS column-table examples, including row and block boundaries.
        TArray<uint8> Memory;
        Memory.SetNumZeroed(4 * 1024 * 1024);
        Memory[0] = 11;
        Memory[33] = 22; // (0, 2)
        Memory[96] = 33; // (0, 4)
        Memory[256] = 44; // (16, 0), next block
        // CSM1 index 16 maps to word 64 in the palette block.
        Memory[8192 + 64 * 4] = 73;
        TArray<uint8> Pixels;
        const uint64 Tex0 = (uint64(2) << 14) | (uint64(0x13) << 20) | (uint64(7) << 26) | (uint64(6) << 30);
        if (TestTrue(TEXT("Read synthetic PSMT8 texture"), KyaG2D::ReadGs8(Memory, Tex0, Pixels)))
        {
            TestEqual(TEXT("GS pixel origin"), Pixels[0], uint8(11));
            TestEqual(TEXT("GS pixel row 2"), Pixels[2 * 128], uint8(22));
            TestEqual(TEXT("GS pixel row 4"), Pixels[4 * 128], uint8(33));
            TestEqual(TEXT("GS pixel next block"), Pixels[16], uint8(44));
        }
        TArray<FColor> Clut;
        if (TestTrue(TEXT("Read synthetic CSM1 palette"), KyaG2D::ReadGsClut32(Memory, Tex0 | (uint64(32) << 37), Clut)))
            TestEqual(TEXT("CSM1 index 16"), Clut[16].R, uint8(73));
        TArray<uint8> FourBitMemory;
        FourBitMemory.SetNumZeroed(4 * 1024 * 1024);
        FourBitMemory[0] = 0x03;
        FourBitMemory[4] = 0x07; // (1, 0), nibble 8
        FourBitMemory[32] = 0xa0; // (0, 2), nibble 65
        FourBitMemory[256] = 0x09; // (0, 16), next block
        FourBitMemory[8192 + 2 * 4] = 61; // palette index 8 maps to word 2
        const uint64 FourBitTex0 = (uint64(2) << 14) | (uint64(0x14) << 20) |
            (uint64(7) << 26) | (uint64(7) << 30) | (uint64(32) << 37);
        TArray<uint8> FourBitPixels;
        if (TestTrue(TEXT("Read synthetic PSMT4 texture"), KyaG2D::ReadGs4(FourBitMemory, FourBitTex0, FourBitPixels)))
        {
            TestEqual(TEXT("PSMT4 origin low nibble"), FourBitPixels[0], uint8(3));
            TestEqual(TEXT("PSMT4 next pixel"), FourBitPixels[1], uint8(7));
            TestEqual(TEXT("PSMT4 row two high nibble"), FourBitPixels[2 * 128], uint8(10));
            TestEqual(TEXT("PSMT4 next block"), FourBitPixels[16 * 128], uint8(9));
        }
        TestTrue(TEXT("Read PSMT4 texture with zero TEX0 buffer width"), KyaG2D::ReadGs4(
            FourBitMemory, FourBitTex0 & ~(uint64(63) << 14), FourBitPixels));
        TestTrue(TEXT("Read PSMT8 texture with zero TEX0 buffer width"), KyaG2D::ReadGs8(
            Memory, Tex0 & ~(uint64(63) << 14), Pixels));
        TArray<FColor> FourBitClut;
        if (TestTrue(TEXT("Read synthetic PSMT4 CSM1 palette"), KyaG2D::ReadGsClut4(FourBitMemory, FourBitTex0, FourBitClut)))
            TestEqual(TEXT("PSMT4 palette index 8"), FourBitClut[8].R, uint8(61));
        KyaG2D::FUploadTransfer Upload;
        Upload.DataOffset = 0; Upload.DataSize = 2;
        Upload.BitBlt = (uint64(1) << 48) | (uint64(0x14) << 56);
        Upload.Region = uint64(4) | (uint64(1) << 32);
        const uint8 Packed[] = { 0x21, 0x43 };
        TArray<uint8> UploadedMemory;
        UploadedMemory.SetNumZeroed(4 * 1024 * 1024);
        if (TestTrue(TEXT("Upload packed PSMT4 pixels"), KyaG2D::WriteGsIndexed(MakeArrayView(Packed), Upload, UploadedMemory)))
        {
            TArray<uint8> UploadedPixels;
            const uint64 NarrowTex0 = (uint64(1) << 14) | (uint64(0x14) << 20) | (uint64(2) << 26);
            if (TestTrue(TEXT("Read narrow PSMT4 upload"), KyaG2D::ReadGs4(UploadedMemory, NarrowTex0, UploadedPixels)))
                for (int32 Index = 0; Index < 4; ++Index)
                    TestEqual(TEXT("Uploaded nibble value"), UploadedPixels[Index], uint8(Index + 1));
        }
        TArray<uint8> DirectMemory;
        DirectMemory.SetNumZeroed(4 * 1024 * 1024);
        DirectMemory[0] = 17;
        DirectMemory[4] = 42; // (1, 0)
        DirectMemory[256] = 91; // (8, 0), next block
        DirectMemory[8192] = 123; // (0, 32), next page
        TArray<FColor> DirectPixels;
        const uint64 DirectTex0 = (uint64(1) << 14) | (uint64(6) << 26) | (uint64(6) << 30);
        if (TestTrue(TEXT("Read synthetic PSMCT32 texture"), KyaG2D::ReadGs32(DirectMemory, DirectTex0, DirectPixels)))
        {
            TestEqual(TEXT("Direct texture origin"), DirectPixels[0].R, uint8(17));
            TestEqual(TEXT("Direct texture adjacent pixel"), DirectPixels[1].R, uint8(42));
            TestEqual(TEXT("Direct texture next block"), DirectPixels[8].R, uint8(91));
            TestEqual(TEXT("Direct texture next page"), DirectPixels[32 * 64].R, uint8(123));
        }
    }
    for (const int32 MeshIndex : { Bank->SceneMeshEntry, Bank->BackgroundMeshEntry })
    {
        if (!TestTrue(TEXT("Mesh entry index is valid"), Bank->Entries.IsValidIndex(MeshIndex))) return false;
        const FKyaBankEntry& Mesh = Bank->Entries[MeshIndex];
        TArray<FKyaG3DChunk> Chunks;
        if (!TestTrue(TEXT("Read G3D chunks"), KyaG3D::ReadChunks(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Chunks))) return false;
        TestTrue(TEXT("Contains PSX2 geometry"), Chunks.ContainsByPredicate(
            [](const FKyaG3DChunk& Chunk) { return Chunk.Tag == 0x32585350; }));
        if (MeshIndex == Bank->SceneMeshEntry)
        {
            TArray<int32> ClusterStrips;
            if (TestTrue(TEXT("Read main scene cluster strips"), KyaG3D::ReadClusterStrips(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), ClusterStrips)))
                TestEqual(TEXT("Level one main scene cluster strip count"), ClusterStrips.Num(), 0);
        }
        TMap<int32, int32> Relocations;
        TArray<int32> InvalidFields;
        TestTrue(TEXT("Read G3D relocations"), KyaG3D::ReadRelocations(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Relocations, InvalidFields));
        TestTrue(TEXT("G3D has relocations"), !Relocations.IsEmpty());
        TArray<int32> Hierarchies;
        TestTrue(TEXT("Read G3D hierarchies"), KyaG3D::ReadHierarchyOffsets(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Hierarchies));
        TestEqual(TEXT("Hierarchy count"), Hierarchies.Num(), MeshIndex == 240 ? 97 : 0);
        for (int32 Hierarchy : Hierarchies)
        {
            FMatrix World;
            if (!KyaG3D::ReadHierarchyWorldMatrix(MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Hierarchy, World))
                AddError(FString::Printf(TEXT("Cannot resolve hierarchy transform at 0x%x"), Hierarchy));
        }
        TArray<FKyaG3DStripReference> Strips;
        TestTrue(TEXT("Read hierarchy strips"), KyaG3D::ReadHierarchyStrips(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips));
        TestEqual(TEXT("First LOD strip count"), Strips.Num(), MeshIndex == 240 ? 119 : 0);
        for (const FKyaG3DStripReference& Strip : Strips)
        {
            TArray<uint32> Counts;
            if (!KyaG3D::ReadStripSectionCounts(MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strip.StripOffset, Counts))
                AddError(FString::Printf(TEXT("Cannot read strip sections at G3D offset 0x%x"), Strip.StripOffset));
        }
        TArray<FKyaG3DDecodedStrip> DecodedStrips;
        if (TestTrue(TEXT("Decode hierarchy strips"), KyaG3D::DecodeHierarchyStrips(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), DecodedStrips)))
        {
            TestEqual(TEXT("Decoded strip count"), DecodedStrips.Num(), Strips.Num());
            if (MeshIndex == 240 && !DecodedStrips.IsEmpty())
            {
                TestEqual(TEXT("First strip material index"), DecodedStrips[0].MaterialIndex, 0);
                TestEqual(TEXT("First strip material hash"), DecodedStrips[0].MaterialHash, uint64(0x197097db0edf22dbULL));
            }
        }
        if (MeshIndex == 240 && !Strips.IsEmpty())
        {
            uint32 VertexCount = 0, Primitive = 0;
            TestTrue(TEXT("Read first GIF tag"), KyaG3D::ReadFirstStripGifTag(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset,
                VertexCount, Primitive));
            TestEqual(TEXT("First strip vertex count"), VertexCount, uint32(96));
            TArray<uint32> SectionCounts;
            if (TestTrue(TEXT("Read strip section counts"), KyaG3D::ReadStripSectionCounts(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset, SectionCounts)))
            {
                TestEqual(TEXT("Section count"), SectionCounts.Num(), 3);
                if (SectionCounts.Num() == 3)
                    TestEqual(TEXT("Final section vertices"), SectionCounts[2], uint32(20));
            }
            TArray<FKyaG3DVertex> Vertices;
            if (TestTrue(TEXT("Read first strip vertices"), KyaG3D::ReadFirstStripVertices(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset, Vertices)))
            {
                TestEqual(TEXT("Decoded vertex count"), Vertices.Num(), 96);
                TestTrue(TEXT("First vertex X"), FMath::IsNearlyEqual(Vertices[0].Position.X, -0.65968186f));
                TestEqual(TEXT("First vertex flags"), Vertices[0].Flags, uint32(49152));
            }
            TArray<FKyaG3DVertex> AllVertices;
            TArray<uint32> AllSections;
            TArray<FKyaG3DVertexAttributes> Attributes;
            if (TestTrue(TEXT("Read strip attributes"), KyaG3D::ReadStripAttributes(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset, Attributes)))
                TestEqual(TEXT("Section-order attribute count"), Attributes.Num(), 212);
            if (TestTrue(TEXT("Read full strip vertices"), KyaG3D::ReadStripVertices(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset, AllVertices, AllSections)))
            {
                TestEqual(TEXT("Full strip unique vertices"), AllVertices.Num(), 208);
                TArray<uint32> Indices;
                TestTrue(TEXT("Build strip triangles"), KyaG3D::BuildStripTriangles(AllVertices, AllSections, Primitive, Indices));
                TestTrue(TEXT("Strip contains triangles"), !Indices.IsEmpty() && Indices.Num() % 3 == 0);
            }
        }
        TestEqual(TEXT("Out-of-range relocation count"), InvalidFields.Num(), MeshIndex == 240 ? 1 : 0);
        if (MeshIndex == 240 && InvalidFields.Num() == 1)
            TestEqual(TEXT("Out-of-range relocation field"), InvalidFields[0], 0x235cc);
        TestFalse(TEXT("Truncated mesh rejected"), KyaG3D::ReadChunks(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size) - 1), Chunks));
    }
    if (!TestEqual(TEXT("Collision bank count"), Bank->SceneConfiguration.CollisionBankValues.Num(), 91)) return false;
    TestEqual(TEXT("First collision bank value"), Bank->SceneConfiguration.CollisionBankValues[0], 1);
    TestEqual(TEXT("Animation count"), Bank->SceneConfiguration.LevelAnimationCount, 1052);
    if (!TestEqual(TEXT("Actor class count"), Bank->SceneConfiguration.ActorClassAllocations.Num(), 36)) return false;
    TestEqual(TEXT("First actor class"), Bank->SceneConfiguration.ActorClassAllocations[0].ClassId, 5);
    TestEqual(TEXT("First actor allocation"), Bank->SceneConfiguration.ActorClassAllocations[0].Count, 1);
    TArray<uint8> Output;
    FMemoryWriter Writer(Output);
    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    if (!TestTrue(TEXT("Export bank"), Exporter->ExportBinary(Bank, TEXT("bnk"), Writer, GWarn))) return false;
    TestTrue(TEXT("Unchanged bank is byte-identical"), Input == Output);
    Bank->SceneConfiguration.Field1C = 4.5f;
    Bank->SceneConfiguration.Field20 = 123456;
    Bank->SceneConfiguration.Field24 = -0.75f;
    Bank->SceneConfiguration.CommonTextureCount = 123;
    if (!TestEqual(TEXT("Scene header word count"), Bank->SceneConfiguration.HeaderWords.Num(), 2) ||
        !TestEqual(TEXT("Scene post-animation word count"), Bank->SceneConfiguration.PostAnimationWords.Num(), 4) ||
        !TestTrue(TEXT("Scene mesh values present"), !Bank->SceneConfiguration.CommonMeshValues.IsEmpty())) return false;
    Bank->SceneConfiguration.HeaderWords[1] = 0x12345678;
    Bank->SceneConfiguration.CommonMeshValues[0] = -27;
    Bank->SceneConfiguration.TextureWord = -19;
    Bank->SceneConfiguration.PostAnimationWords[2] = 0x87654321;
    Bank->SceneConfiguration.ParticleWord = 0xabcdef12;
    Bank->SceneEnvironment.ClipDistance = 456.0f;
    Bank->SceneEnvironment.FogColor = FColor(12, 34, 56, 78);
    Bank->SceneEnvironment.MipmapK = -12;
    Bank->SceneEnvironment.CameraFov = 1.25f;
    Bank->SceneEnvironment.CameraPosition = FVector(2.0, -3.0, 4.5);
    Bank->SceneEnvironment.CameraFlags = 0x1234;
    Bank->SceneEnvironment.CameraTargetOffset = FVector(1.0, 2.0, 3.0);
    Bank->SceneEnvironment.CameraConfigAngle = 55.0f;
    if (Bank->SceneEnvironment.bHasCameraSwitchData)
    {
        Bank->SceneEnvironment.CameraSwitchActor = 47;
        Bank->SceneEnvironment.CameraSwitchValueB = 9.25f;
    }
    if (!Bank->SceneEnvironment.CameraConditions.IsEmpty())
        Bank->SceneEnvironment.CameraConditions[0].Value = 8.25f;
    if (!Bank->SceneEnvironment.CameraOptionalConfigValues.IsEmpty())
        Bank->SceneEnvironment.CameraOptionalConfigValues[0] = 6.25f;
    if (!Bank->SceneEnvironment.CameraExtraValues.IsEmpty())
        Bank->SceneEnvironment.CameraExtraValues[0] = 7.25f;
    Bank->SceneEnvironment.CameraPitchAngles = FVector(10.0, 20.0, 30.0);
    Bank->SceneEnvironment.Field118 = 25.0f;
    Bank->SceneEnvironment.Field11C = 1;
    Bank->SceneEnvironment.Count120 = 3;
    Bank->SceneEnvironment.MapAngle = 42.0f;
    Bank->SceneEnvironment.MapActorRef = 123;
    Bank->SceneEnvironment.DefaultMaterialIndex = 7;
    if (!TestEqual(TEXT("Effect pool category count"), Bank->SceneEnvironment.EffectPoolCounts.Num(), 7)) return false;
    Bank->SceneEnvironment.EffectPoolCounts[3] = 37;
    if (!TestEqual(TEXT("Objective count"), Bank->SceneEnvironment.Objectives.Num(), 3)) return false;
    Bank->SceneEnvironment.Objectives[0].ScenarioVariable = 91;
    Bank->SceneEnvironment.Objectives[0].Fields[0] = 321;
    Bank->SceneEnvironment.Field10C = 20;
    Bank->SceneEnvironment.Field110 = 21;
    if (!TestEqual(TEXT("Waypoint count"), Bank->SceneEnvironment.Waypoints.Num(), 60)) return false;
    Bank->SceneEnvironment.Waypoints[0].Position = FVector(1.0, 2.0, 3.0);
    if (!TestEqual(TEXT("Path count"), Bank->SceneEnvironment.Paths.Num(), 231)) return false;
    if (!TestTrue(TEXT("First path has points"), !Bank->SceneEnvironment.Paths[0].Points.IsEmpty())) return false;
    Bank->SceneEnvironment.Paths[0].Points[0] = FVector4(1, 2, 3, 1);
    if (!TestEqual(TEXT("Actor count"), Bank->SceneEnvironment.Actors.Num(), 616)) return false;
    {
        int32 Platforms = 0, Prefixes = 0, Details = 0, Cameras = 0, SoundA = 0, SoundB = 0, Friction = 0, FrictionActors = 0;
        for (const FKyaActorPlacement& Item : Bank->SceneEnvironment.Actors)
        {
            if (Item.ClassId != 0x0d) continue;
            ++Platforms;
            Prefixes += Item.bHasPlatformPrefix ? 1 : 0;
            Details += Item.bHasPlatformDetails ? 1 : 0;
            Cameras += Item.PlatformCameras.Num(); SoundA += Item.PlatformSoundsA.Num(); SoundB += Item.PlatformSoundsB.Num();
            Friction += Item.PlatformNoFrictionZones.Num();
            for (const FKyaPlatformNoFrictionZone& Zone : Item.PlatformNoFrictionZones) FrictionActors += Zone.Actors.Num();
        }
        AddInfo(FString::Printf(TEXT("Platform inventory: actors=%d prefixes=%d details=%d camera-records=%d sound-a=%d sound-b=%d friction-zones=%d friction-actors=%d"),
            Platforms, Prefixes, Details, Cameras, SoundA, SoundB, Friction, FrictionActors));
    }
    Bank->SceneEnvironment.Actors[0].Position = FVector(4, 5, 6);
    Bank->SceneEnvironment.Actors[0].VisibilityDistance = 123.0f;
    Bank->SceneEnvironment.Actors[0].DefaultBehaviourId = 7;
    const int32 ConditionActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return !Actor.Conditions.IsEmpty(); });
    const int32 ClassDataActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.ClassId != 0x1e && !Actor.bHasPlatformPrefix && !Actor.ClassData.IsEmpty(); });
    const int32 PlatformActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformPrefix; });
    const int32 PlatformDetailActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformDetails; });
    const int32 PlatformCameraActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformDetails && !Actor.PlatformCameras.IsEmpty(); });
    const int32 PlatformSoundActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformDetails && !Actor.PlatformSoundsA.IsEmpty(); });
    const int32 CheckpointActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.ClassId == 0x1e && !Actor.Checkpoints.IsEmpty(); });
    if (!TestTrue(TEXT("Level has actor scenario conditions"), ConditionActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has actor class data"), ClassDataActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has checkpoint records"), CheckpointActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has moving platform data"), PlatformActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has moving platform details"), PlatformDetailActor != INDEX_NONE)) return false;
    Bank->SceneEnvironment.Actors[ConditionActor].Conditions[0].Value = 197;
    Bank->SceneEnvironment.Actors[ClassDataActor].ClassData[0] ^= 1;
    Bank->SceneEnvironment.Actors[CheckpointActor].Checkpoints[0].SectorId = 87;
    Bank->SceneEnvironment.Actors[PlatformActor].PlatformWeight = 12.5f;
    Bank->SceneEnvironment.Actors[PlatformDetailActor].PlatformProperties.TiltValue = 6.75f;
    if (PlatformCameraActor != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformCameraActor].PlatformCameras[0].Data[0] ^= 1;
    if (PlatformSoundActor != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformSoundActor].PlatformSoundsA[0].SoundRef = 81;
    const int32 PlatformWithActors = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformPrefix && !Actor.PlatformActors.IsEmpty(); });
    const int32 PlatformWithZones = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformPrefix && !Actor.PlatformZones.IsEmpty(); });
    const int32 PlatformWithFriction = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformPrefix && !Actor.PlatformNoFrictionZones.IsEmpty(); });
    const int32 PlatformWithFrictionActors = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasPlatformPrefix && !Actor.PlatformNoFrictionZones.IsEmpty() && !Actor.PlatformNoFrictionZones[0].Actors.IsEmpty();
    });
    if (PlatformWithActors != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformWithActors].PlatformActors[0].CarryMethod = 77;
    if (PlatformWithZones != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformWithZones].PlatformZones[0] = 78;
    if (PlatformWithFriction != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformWithFriction].PlatformNoFrictionZones[0].ZoneRef = 79;
    if (PlatformWithFrictionActors != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformWithFrictionActors].PlatformNoFrictionZones[0].Actors[0] = 80;
    const int32 AnimatedActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return !Actor.MacroAnimations.IsEmpty(); });
    if (!TestTrue(TEXT("Level has an actor with macro animations"), AnimatedActor != INDEX_NONE)) return false;
    Bank->SceneEnvironment.Actors[AnimatedActor].MacroAnimations[0].MacroAnimationId = 123;
    const int32 BehaviourActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.ClassId != 0x18 && Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return !Behaviour.Data.IsEmpty(); });
    });
    if (!TestTrue(TEXT("Level has an actor behavior payload"), BehaviourActor != INDEX_NONE)) return false;
    const int32 BehaviourIndex = Bank->SceneEnvironment.Actors[BehaviourActor].Behaviours.IndexOfByPredicate([](const FKyaActorBehaviour& Behaviour) { return !Behaviour.Data.IsEmpty(); });
    Bank->SceneEnvironment.Actors[BehaviourActor].Behaviours[BehaviourIndex].Data[0] ^= 1;
    const int32 ClusterActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.ClassId == 0x18 && Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2 && !Behaviour.ClusterZones.IsEmpty(); });
    });
    if (!TestTrue(TEXT("Level has clusteriser zone data"), ClusterActor != INDEX_NONE)) return false;
    const int32 ClusterBehaviour = Bank->SceneEnvironment.Actors[ClusterActor].Behaviours.IndexOfByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2 && !Behaviour.ClusterZones.IsEmpty(); });
    Bank->SceneEnvironment.Actors[ClusterActor].Behaviours[ClusterBehaviour].ClusterZones[0].ZoneRef = 1234;
    const int32 ModelActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return !Actor.Models.IsEmpty() && !Actor.Models[0].MeshName.IsEmpty();
    });
    if (!TestTrue(TEXT("Level has an actor model reference"), ModelActor != INDEX_NONE)) return false;
    FString& ModelName = Bank->SceneEnvironment.Actors[ModelActor].Models[0].MeshName;
    ModelName[0] = ModelName[0] == TEXT('X') ? TEXT('Y') : TEXT('X');
    Bank->SceneConfiguration.CollisionBankValues[0] = 42;
    Bank->SceneConfiguration.LevelAnimationCount = 1053;
    Bank->SceneConfiguration.ActorClassAllocations[0].Count = 2;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited bank"), Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited bank"), Reimported)) return false;
    TestEqual(TEXT("Edited field 1c"), Reimported->SceneConfiguration.Field1C, 4.5f);
    TestEqual(TEXT("Edited field 20"), Reimported->SceneConfiguration.Field20, int64(123456));
    TestEqual(TEXT("Edited field 24"), Reimported->SceneConfiguration.Field24, -0.75f);
    TestEqual(TEXT("Edited common texture count"), Reimported->SceneConfiguration.CommonTextureCount, 123);
    TestEqual(TEXT("Edited scene header word"), Reimported->SceneConfiguration.HeaderWords[1], int64(0x12345678));
    TestEqual(TEXT("Edited common mesh value"), Reimported->SceneConfiguration.CommonMeshValues[0], -27);
    TestEqual(TEXT("Edited texture word"), Reimported->SceneConfiguration.TextureWord, -19);
    TestEqual(TEXT("Edited post-animation word"), Reimported->SceneConfiguration.PostAnimationWords[2], int64(0x87654321));
    TestEqual(TEXT("Edited particle word"), Reimported->SceneConfiguration.ParticleWord, int64(0xabcdef12));
    TestEqual(TEXT("Edited clip distance"), Reimported->SceneEnvironment.ClipDistance, 456.0f);
    TestEqual(TEXT("Edited fog color"), Reimported->SceneEnvironment.FogColor, FColor(12, 34, 56, 78));
    TestEqual(TEXT("Edited mipmap K"), Reimported->SceneEnvironment.MipmapK, -12);
    TestEqual(TEXT("Edited camera FOV"), Reimported->SceneEnvironment.CameraFov, 1.25f);
    TestEqual(TEXT("Edited camera position"), Reimported->SceneEnvironment.CameraPosition, FVector(2.0, -3.0, 4.5));
    TestEqual(TEXT("Edited camera flags"), Reimported->SceneEnvironment.CameraFlags, int64(0x1234));
    TestEqual(TEXT("Edited camera target offset"), Reimported->SceneEnvironment.CameraTargetOffset, FVector(1.0, 2.0, 3.0));
    TestEqual(TEXT("Edited camera configuration angle"), Reimported->SceneEnvironment.CameraConfigAngle, 55.0f);
    if (Bank->SceneEnvironment.bHasCameraSwitchData)
    {
        TestEqual(TEXT("Camera switch actor round trips"), Reimported->SceneEnvironment.CameraSwitchActor, 47);
        TestEqual(TEXT("Camera switch value round trips"), Reimported->SceneEnvironment.CameraSwitchValueB, 9.25f);
    }
    if (!Bank->SceneEnvironment.CameraConditions.IsEmpty() && TestEqual(TEXT("Camera condition count round trips"), Reimported->SceneEnvironment.CameraConditions.Num(), Bank->SceneEnvironment.CameraConditions.Num()))
        TestEqual(TEXT("Edited camera condition round trips"), Reimported->SceneEnvironment.CameraConditions[0].Value, 8.25f);
    TestTrue(TEXT("Optional camera configuration round trips"), Reimported->SceneEnvironment.CameraOptionalConfigValues == Bank->SceneEnvironment.CameraOptionalConfigValues);
    TestTrue(TEXT("Extra camera values round trip"), Reimported->SceneEnvironment.CameraExtraValues == Bank->SceneEnvironment.CameraExtraValues);
    TestEqual(TEXT("Edited camera pitch angles"), Reimported->SceneEnvironment.CameraPitchAngles, FVector(10.0, 20.0, 30.0));
    TestEqual(TEXT("Edited scene field 118"), Reimported->SceneEnvironment.Field118, 25.0f);
    TestEqual(TEXT("Edited scene field 11c"), Reimported->SceneEnvironment.Field11C, int64(1));
    TestEqual(TEXT("Edited scene count 120"), Reimported->SceneEnvironment.Count120, int64(3));
    TestEqual(TEXT("Edited map angle"), Reimported->SceneEnvironment.MapAngle, 42.0f);
    TestEqual(TEXT("Edited map actor"), Reimported->SceneEnvironment.MapActorRef, 123);
    TestEqual(TEXT("Edited default material"), Reimported->SceneEnvironment.DefaultMaterialIndex, 7);
    TestEqual(TEXT("Edited particle pool count"), Reimported->SceneEnvironment.EffectPoolCounts[3], int64(37));
    TestEqual(TEXT("Edited objective variable"), Reimported->SceneEnvironment.Objectives[0].ScenarioVariable, 91);
    TestEqual(TEXT("Edited objective field"), Reimported->SceneEnvironment.Objectives[0].Fields[0], int64(321));
    TestEqual(TEXT("Edited field 10c"), Reimported->SceneEnvironment.Field10C, 20);
    TestEqual(TEXT("Edited field 110"), Reimported->SceneEnvironment.Field110, 21);
    TestEqual(TEXT("Edited waypoint position"), Reimported->SceneEnvironment.Waypoints[0].Position, FVector(1.0, 2.0, 3.0));
    TestTrue(TEXT("Edited path point"), Reimported->SceneEnvironment.Paths[0].Points[0].Equals(FVector4(1, 2, 3, 1)));
    TestEqual(TEXT("Edited actor position"), Reimported->SceneEnvironment.Actors[0].Position, FVector(4, 5, 6));
    TestEqual(TEXT("Edited actor visibility"), Reimported->SceneEnvironment.Actors[0].VisibilityDistance, 123.0f);
    TestEqual(TEXT("Edited actor default behavior"), Reimported->SceneEnvironment.Actors[0].DefaultBehaviourId, 7);
    TestEqual(TEXT("Edited actor scenario condition"), Reimported->SceneEnvironment.Actors[ConditionActor].Conditions[0].Value, 197);
    TestEqual(TEXT("Edited actor class payload"), Reimported->SceneEnvironment.Actors[ClassDataActor].ClassData[0],
        Bank->SceneEnvironment.Actors[ClassDataActor].ClassData[0]);
    TestEqual(TEXT("Edited checkpoint sector"), Reimported->SceneEnvironment.Actors[CheckpointActor].Checkpoints[0].SectorId, 87);
    TestEqual(TEXT("Edited moving platform weight"), Reimported->SceneEnvironment.Actors[PlatformActor].PlatformWeight, 12.5f);
    TestEqual(TEXT("Edited platform tilt value"), Reimported->SceneEnvironment.Actors[PlatformDetailActor].PlatformProperties.TiltValue, 6.75f);
    if (PlatformCameraActor != INDEX_NONE) TestEqual(TEXT("Edited platform camera byte"), Reimported->SceneEnvironment.Actors[PlatformCameraActor].PlatformCameras[0].Data[0], Bank->SceneEnvironment.Actors[PlatformCameraActor].PlatformCameras[0].Data[0]);
    if (PlatformSoundActor != INDEX_NONE) TestEqual(TEXT("Edited platform sound reference"), Reimported->SceneEnvironment.Actors[PlatformSoundActor].PlatformSoundsA[0].SoundRef, 81);
    AddInfo(FString::Printf(TEXT("Platform detail coverage: camera=%d sound=%d"), PlatformCameraActor != INDEX_NONE, PlatformSoundActor != INDEX_NONE));
    if (PlatformWithActors != INDEX_NONE) TestEqual(TEXT("Edited platform carry method"), Reimported->SceneEnvironment.Actors[PlatformWithActors].PlatformActors[0].CarryMethod, 77);
    if (PlatformWithZones != INDEX_NONE) TestEqual(TEXT("Edited platform zone"), Reimported->SceneEnvironment.Actors[PlatformWithZones].PlatformZones[0], 78);
    if (PlatformWithFriction != INDEX_NONE) TestEqual(TEXT("Edited platform no-friction zone"), Reimported->SceneEnvironment.Actors[PlatformWithFriction].PlatformNoFrictionZones[0].ZoneRef, 79);
    if (PlatformWithFrictionActors != INDEX_NONE) TestEqual(TEXT("Edited platform no-friction actor"), Reimported->SceneEnvironment.Actors[PlatformWithFrictionActors].PlatformNoFrictionZones[0].Actors[0], 80);
    AddInfo(FString::Printf(TEXT("Platform list coverage: actors=%d zones=%d friction-zones=%d friction-actors=%d"),
        PlatformWithActors != INDEX_NONE, PlatformWithZones != INDEX_NONE, PlatformWithFriction != INDEX_NONE, PlatformWithFrictionActors != INDEX_NONE));
    TestEqual(TEXT("Edited actor macro animation"), Reimported->SceneEnvironment.Actors[AnimatedActor].MacroAnimations[0].MacroAnimationId, 123);
    TestEqual(TEXT("Edited behavior payload"), Reimported->SceneEnvironment.Actors[BehaviourActor].Behaviours[BehaviourIndex].Data[0],
        Bank->SceneEnvironment.Actors[BehaviourActor].Behaviours[BehaviourIndex].Data[0]);
    TestEqual(TEXT("Edited cluster zone reference"), Reimported->SceneEnvironment.Actors[ClusterActor].Behaviours[ClusterBehaviour].ClusterZones[0].ZoneRef, 1234);
    TestEqual(TEXT("Edited actor model name"), Reimported->SceneEnvironment.Actors[ModelActor].Models[0].MeshName, ModelName);
    TestEqual(TEXT("Edited collision bank value"), Reimported->SceneConfiguration.CollisionBankValues[0], 42);
    TestEqual(TEXT("Edited animation count"), Reimported->SceneConfiguration.LevelAnimationCount, 1053);
    TestEqual(TEXT("Edited actor allocation"), Reimported->SceneConfiguration.ActorClassAllocations[0].Count, 2);
    const int64 SceneOffset = Bank->Entries[0].Offset;
    const FKyaBankEntry* SceneData = Bank->Entries.FindByPredicate([](const FKyaBankEntry& Entry) { return Entry.Type == 6 && Entry.Subtype == 1; });
    if (!TestNotNull(TEXT("Scene data entry"), SceneData)) return false;
    auto ReadWord = [&Input](int64 Offset) -> uint32 {
        return uint32(Input[Offset]) | (uint32(Input[Offset + 1]) << 8) | (uint32(Input[Offset + 2]) << 16) | (uint32(Input[Offset + 3]) << 24);
    };
    const int64 MeshValuesOffset = SceneOffset + 40;
    const int64 TextureWordsOffset = MeshValuesOffset + int64(ReadWord(SceneOffset + 36)) * 4;
    const int64 AnimationOffset = TextureWordsOffset + 12 + int64(ReadWord(TextureWordsOffset + 8)) * 4;
    const int64 ClassDataOffset = AnimationOffset + 24;
    const int64 ParticleOffset = ClassDataOffset + int64(ReadWord(AnimationOffset + 20)) * 8;
    int64 SetupOffset = SceneData->Offset + 20;
    if (ReadWord(SceneData->Offset + 16)) SetupOffset += 12;
    const int32 BackgroundId = int32(ReadWord(SetupOffset));
    SetupOffset += 4 + (BackgroundId != -1 ? 4 : 0);
    int64 CameraConfigOffset = 0;
    if (!TestTrue(TEXT("Locate camera configuration"), KyaFindCameraConfig(Input, SetupOffset, CameraConfigOffset))) return false;
    const int64 CameraValuesOffset = CameraConfigOffset + 20 + (ReadWord(CameraConfigOffset + 16) == 1 ? 20 : 0);
    int64 SceneValuesOffset = 0;
    if (!TestTrue(TEXT("Locate scene values after camera"), KyaFindSceneAfterCamera(Input, CameraConfigOffset, SceneValuesOffset))) return false;
    int64 MapEndOffset = 0;
    if (!TestTrue(TEXT("Locate map end"), KyaFindMapEnd(Input, SceneValuesOffset + 12, MapEndOffset))) return false;
    TArray<FKyaConditionedOffsets> ScenarioOffsets;
    int64 ScenarioEndOffset = 0;
    if (!TestTrue(TEXT("Locate scenario end"), KyaReadConditionedOffsets(Input, MapEndOffset + 48, ScenarioOffsets, ScenarioEndOffset))) return false;
    TArray<FKyaObjectiveOffsets> ObjectiveOffsets;
    int64 ObjectiveEndOffset = 0;
    if (!TestTrue(TEXT("Locate objectives"), KyaReadObjectiveOffsets(Input, ScenarioEndOffset, ObjectiveOffsets, ObjectiveEndOffset)) || ObjectiveOffsets.IsEmpty()) return false;
    int64 WaypointOffset = 0;
    int32 WaypointCount = 0;
    if (!TestTrue(TEXT("Locate waypoints"), KyaFindWaypoints(Input, ObjectiveEndOffset, WaypointOffset, WaypointCount))) return false;
    TArray<FKyaPathOffsets> PathOffsets;
    if (!TestTrue(TEXT("Locate paths"), KyaReadPathOffsets(Input, WaypointOffset, WaypointCount, SceneData->Offset, PathOffsets)) || PathOffsets.IsEmpty()) return false;
    TArray<FKyaActorOffsets> ActorOffsets;
    if (!TestTrue(TEXT("Locate actors"), KyaReadActorOffsets(Input, WaypointOffset, SceneData->Offset, ActorOffsets)) || ActorOffsets.IsEmpty()) return false;
    TArray<int64> PlatformChangedOffsets;
    auto PlatformOffsetsFor = [&](int32 ActorIndex, FKyaPlatformPrefixOffsets& Offsets) {
        return ActorIndex != INDEX_NONE && KyaReadPlatformPrefix(Input, ActorOffsets[ActorIndex].ClassData, ActorOffsets[ActorIndex].ClassDataSize, Offsets);
    };
    FKyaPlatformPrefixOffsets PlatformOffsets;
    if (PlatformOffsetsFor(PlatformWithActors, PlatformOffsets)) PlatformChangedOffsets.Add(PlatformOffsets.Actors + 4);
    if (PlatformOffsetsFor(PlatformWithZones, PlatformOffsets)) PlatformChangedOffsets.Add(PlatformOffsets.Zones);
    if (PlatformOffsetsFor(PlatformWithFriction, PlatformOffsets)) PlatformChangedOffsets.Add(PlatformOffsets.NoFrictionZones[0]);
    if (PlatformOffsetsFor(PlatformWithFrictionActors, PlatformOffsets)) PlatformChangedOffsets.Add(PlatformOffsets.NoFrictionZones[0] + 8);
    if (PlatformOffsetsFor(PlatformDetailActor, PlatformOffsets))
    {
        FKyaPlatformDetailsOffsets Details;
        if (TestTrue(TEXT("Locate moving platform details"), KyaReadPlatformDetails(Input, PlatformOffsets.Suffix,
            ActorOffsets[PlatformDetailActor].ClassData + ActorOffsets[PlatformDetailActor].ClassDataSize - PlatformOffsets.Suffix, Details)))
            PlatformChangedOffsets.Add(Details.Properties + 44);
    }
    if (PlatformOffsetsFor(PlatformCameraActor, PlatformOffsets))
    {
        FKyaPlatformDetailsOffsets Details;
        if (KyaReadPlatformDetails(Input, PlatformOffsets.Suffix, ActorOffsets[PlatformCameraActor].ClassData + ActorOffsets[PlatformCameraActor].ClassDataSize - PlatformOffsets.Suffix, Details))
            PlatformChangedOffsets.Add(Details.Cameras);
    }
    if (PlatformOffsetsFor(PlatformSoundActor, PlatformOffsets))
    {
        FKyaPlatformDetailsOffsets Details;
        if (KyaReadPlatformDetails(Input, PlatformOffsets.Suffix, ActorOffsets[PlatformSoundActor].ClassData + ActorOffsets[PlatformSoundActor].ClassDataSize - PlatformOffsets.Suffix, Details))
            PlatformChangedOffsets.Add(Details.Sounds[0] + 8);
    }
    bool OtherBytesUnchanged = Input.Num() == Edited.Num();
    for (int64 Index = 0; OtherBytesUnchanged && Index < Input.Num(); ++Index)
        if ((Index < SceneOffset + 16 || Index >= SceneOffset + 28) &&
            (Index < SceneOffset + 32 || Index >= SceneOffset + 36) &&
            (Index < MeshValuesOffset || Index >= MeshValuesOffset + 4) &&
            (Index < TextureWordsOffset + 4 || Index >= TextureWordsOffset + 8) &&
            (Index < AnimationOffset + 12 || Index >= AnimationOffset + 16) &&
            (Index < ParticleOffset || Index >= ParticleOffset + 4) &&
            (Index < SceneOffset + 508 || Index >= SceneOffset + 512) &&
            (Index < SetupOffset + 24 || Index >= SetupOffset + 28) &&
            (Index < SetupOffset + 32 || Index >= SetupOffset + 36) &&
            (Index < SetupOffset + 44 || Index >= SetupOffset + 48) &&
            (Index < SetupOffset + 48 || Index >= SetupOffset + 64) &&
            (Index < SetupOffset + 96 || Index >= SetupOffset + 100) &&
            (Index < SetupOffset + 100 || Index >= SetupOffset + 112) &&
            (Index < CameraConfigOffset + 12 || Index >= CameraConfigOffset + 16) &&
            (Index < CameraValuesOffset || Index >= CameraValuesOffset + 12) &&
            (Index < SceneValuesOffset || Index >= SceneValuesOffset + 12) &&
            (Index < SceneValuesOffset + 20 || Index >= SceneValuesOffset + 24) &&
            (Index < MapEndOffset + 4 || Index >= MapEndOffset + 8) &&
            (Index < MapEndOffset + 16 || Index >= MapEndOffset + 20) &&
            (Index < MapEndOffset + 32 || Index >= MapEndOffset + 36) &&
            (Index < ObjectiveOffsets[0].Header || Index >= ObjectiveOffsets[0].Header + 8) &&
            (Index < ObjectiveEndOffset || Index >= ObjectiveEndOffset + 8) &&
            (Index < WaypointOffset || Index >= WaypointOffset + 12) &&
            (Index < PathOffsets[0].Arrays[0] || Index >= PathOffsets[0].Arrays[0] + 16) &&
            (Index < ActorOffsets[0].Placement + 12 || Index >= ActorOffsets[0].Placement + 24) &&
            (Index < ActorOffsets[0].Common || Index >= ActorOffsets[0].Common + 4) &&
            (Index < ActorOffsets[0].Common + 24 || Index >= ActorOffsets[0].Common + 28) &&
            (Index < ActorOffsets[ConditionActor].Conditions + 12 || Index >= ActorOffsets[ConditionActor].Conditions + 16) &&
            Index != ActorOffsets[ClassDataActor].ClassData &&
            (Index < ActorOffsets[CheckpointActor].ClassData + 12 || Index >= ActorOffsets[CheckpointActor].ClassData + 16) &&
            (Index < ActorOffsets[PlatformActor].ClassData || Index >= ActorOffsets[PlatformActor].ClassData + 4) &&
            !PlatformChangedOffsets.ContainsByPredicate([Index](int64 Offset) { return Index >= Offset && Index < Offset + 4; }) &&
            (Index < ActorOffsets[AnimatedActor].MacroAnimations + 4 || Index >= ActorOffsets[AnimatedActor].MacroAnimations + 8) &&
            Index != ActorOffsets[BehaviourActor].Behaviours[BehaviourIndex].Payload &&
            (Index < ActorOffsets[ClusterActor].Behaviours[ClusterBehaviour].Payload + 8 || Index >= ActorOffsets[ClusterActor].Behaviours[ClusterBehaviour].Payload + 12) &&
            (Index < ActorOffsets[ModelActor].ModelStrings[0].Offset || Index >= ActorOffsets[ModelActor].ModelStrings[0].Offset + ActorOffsets[ModelActor].ModelStrings[0].Capacity) &&
            (Index < SceneOffset + 520 || Index >= SceneOffset + 524) &&
            (Index < SceneOffset + 884 || Index >= SceneOffset + 888) &&
            (Index < SceneOffset + 912 || Index >= SceneOffset + 916) && Input[Index] != Edited[Index])
            OtherBytesUnchanged = false;
    TestTrue(TEXT("Edited export preserves other bytes"), OtherBytesUnchanged);
    return true;
}
#endif
