#if WITH_DEV_AUTOMATION_TESTS
#pragma warning(disable : 4883)
#include "KyaActorEditService.h"
#include "KyaActorCatalog.h"
#include "KyaActorGroup.h"
#include "KyaActorHandle.h"
#include "KyaHierarchyHandle.h"
#include "KyaSceneVisualizer.h"
#include "KyaActorTransform.h"
#include "KyaBankExport.h"
#include "KyaBankStructuralWriter.h"
#include "KyaLevelActorFactory.h"
#include "KyaLevelBank.h"
#include "KyaLevelBankFactory.h"
#include "KyaLevelBankReimportHandler.h"
#include "KyaActorDetailsCustomization.h"
#include "EditorFramework/AssetImportData.h"
#include "KyaCameraStream.h"
#include "KyaLevelEditorActions.h"
#include "KyaMessageLog.h"
#include "KyaLevelPreview.h"
#include "KyaLevelSettings.h"
#include "KyaLevelSet.h"
#include "KyaLevelSetFactory.h"
#include "KyaPathPointComponent.h"
#include "KyaSpatialRecordHandle.h"
#include "KyaStructuralEdits.h"
#include "KyaWaypointComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Editor.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/Selection.h"
#include "IMessageLogListing.h"
#include "Logging/TokenizedMessage.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"
#include "ScopedTransaction.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/SplineComponent.h"
#include "ProceduralMeshComponent.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "PackageTools.h"
#include "UObject/UnrealType.h"

namespace
{
/** Synthetic bank with two actor records; no background entry so handles use the incomplete-scene path. */
UKyaLevelBank* MakeKyaTestBank(UObject* Outer, int64 SecondClassId = 6)
{
    UKyaLevelBank* Bank = NewObject<UKyaLevelBank>(Outer, NAME_None, RF_Transient | RF_Transactional);
    Bank->ImportSchemaVersion = UKyaLevelBank::CurrentImportSchemaVersion;
    Bank->BumpEditGeneration();
    FKyaActorPlacement& First = Bank->SceneEnvironment.Actors.AddDefaulted_GetRef();
    First.Name = TEXT("FIRST");
    First.ClassId = 0;
    First.Position = FVector(1.0, 2.0, 3.0);
    First.Rotation = FVector(0.0, 0.6, 0.0);
    First.Scale = FVector(1.0, 1.0, 1.0);
    First.SectorId = -1;
    First.MeshIndex = -1;
    First.TextureIndex = -1;
    FKyaActorPlacement& Second = Bank->SceneEnvironment.Actors.AddDefaulted_GetRef();
    Second.Name = TEXT("SECOND");
    Second.ClassId = SecondClassId;
    Second.Position = FVector(-4.0, 1.0, 0.5);
    Second.Rotation = FVector(0.2, 1.3, -0.4);
    Second.Scale = FVector(2.0, 1.0, 0.5);
    Second.SectorId = -1;
    Second.MeshIndex = -1;
    Second.TextureIndex = -1;
    return Bank;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaActorTransformConversionTest, "Kya.Editor.ActorTransformConversion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaActorTransformConversionTest::RunTest(const FString& Parameters)
{
    struct FCase
    {
        int64 ClassId;
        const TCHAR* Label;
        FVector Angles;
    };
    const FCase Cases[] = {
        {0, TEXT("default basis"), FVector(0.3, 1.1, -0.2)},
        {0, TEXT("default yaw only"), FVector(0.0, 0.7, 0.0)},
        {0, TEXT("default near vertical"), FVector(HALF_PI * 0.999, 1.3, 0.2)},
        {1, TEXT("euler matrix"), FVector(0.3, 1.1, -0.2)},
        {1, TEXT("euler identity"), FVector(0.0, 0.0, 0.0)},
        {1, TEXT("euler roll"), FVector(0.9, -2.0, 0.4)},
        {6, TEXT("yaw only"), FVector(0.3, 1.1, -0.2)},
        {43, TEXT("yaw and euler"), FVector(0.3, 1.1, -0.2)},
    };

    for (const FCase& Case : Cases)
    {
        FVector BasisX, BasisY, BasisZ;
        KyaActorTransform::BuildRotationBasis(Case.Angles, Case.ClassId, BasisX, BasisY, BasisZ);
        const FQuat Rotation = KyaActorTransform::QuatFromGameBasis(BasisX, BasisY, BasisZ);

        FVector RoundX, RoundY, RoundZ;
        KyaActorTransform::GameBasisFromQuat(Rotation, RoundX, RoundY, RoundZ);
        TestTrue(FString::Printf(TEXT("%s: Unreal basis round trip X"), Case.Label), RoundX.Equals(BasisX, 1.0e-4));
        TestTrue(FString::Printf(TEXT("%s: Unreal basis round trip Y"), Case.Label), RoundY.Equals(BasisY, 1.0e-4));
        TestTrue(FString::Printf(TEXT("%s: Unreal basis round trip Z"), Case.Label), RoundZ.Equals(BasisZ, 1.0e-4));

        const FVector GamePosition(1.5, -2.25, 3.75);
        const FVector GameScale(1.5, 2.0, 0.5);
        const FTransform Local(Rotation, KyaActorTransform::SwapGameAxes(GamePosition) * 100.0,
            KyaActorTransform::SwapGameAxes(GameScale));
        KyaActorTransform::FGameTransform Result;
        FString Error;
        const bool bConverted = KyaActorTransform::UnrealToGameTransform(Local, Case.ClassId, Case.Angles, Result, Error);
        TestTrue(FString::Printf(TEXT("%s: transform converts (%s)"), Case.Label, *Error), bConverted);
        if (!bConverted) continue;
        TestTrue(FString::Printf(TEXT("%s: position round trip"), Case.Label), Result.Position.Equals(GamePosition, 1.0e-4));
        TestTrue(FString::Printf(TEXT("%s: scale round trip"), Case.Label), Result.Scale.Equals(GameScale, 1.0e-4));

        FVector ResultX, ResultY, ResultZ;
        KyaActorTransform::BuildRotationBasis(Result.Rotation, Case.ClassId, ResultX, ResultY, ResultZ);
        TestTrue(FString::Printf(TEXT("%s: rotation basis round trip"), Case.Label), ResultX.Equals(BasisX, 2.0e-3) &&
            ResultY.Equals(BasisY, 2.0e-3) && ResultZ.Equals(BasisZ, 2.0e-3));

        // Equivalent angles are chosen nearest the previous serialized values.
        const FVector Wrapped = Case.Angles + FVector(2.0 * PI, -2.0 * PI, 4.0 * PI);
        KyaActorTransform::FGameTransform Rewrapped;
        if (KyaActorTransform::UnrealToGameTransform(Local, Case.ClassId, Wrapped, Rewrapped, Error))
        {
            TestTrue(FString::Printf(TEXT("%s: angle wrapping stays near previous values"), Case.Label),
                FMath::Abs(Rewrapped.Rotation.X - Wrapped.X) <= PI + KINDA_SMALL_NUMBER &&
                FMath::Abs(Rewrapped.Rotation.Y - Wrapped.Y) <= PI + KINDA_SMALL_NUMBER &&
                FMath::Abs(Rewrapped.Rotation.Z - Wrapped.Z) <= PI + KINDA_SMALL_NUMBER);
        }
    }

    {
        // Gimbal lock (yaw = +/-90 degrees) with a different previous angle set forces
        // the Euler inverse instead of the preserve-existing-angles shortcut.
        for (const double Yaw : {HALF_PI, -HALF_PI})
        {
            const FVector Angles(0.7, Yaw, 0.4);
            FVector GimbalX, GimbalY, GimbalZ;
            KyaActorTransform::BuildRotationBasis(Angles, 1, GimbalX, GimbalY, GimbalZ);
            const FQuat GimbalRotation = KyaActorTransform::QuatFromGameBasis(GimbalX, GimbalY, GimbalZ);
            KyaActorTransform::FGameTransform Result;
            FString Error;
            const bool bConverted = KyaActorTransform::UnrealToGameTransform(
                FTransform(GimbalRotation, FVector::ZeroVector, FVector::OneVector), 1,
                FVector::ZeroVector, Result, Error);
            TestTrue(FString::Printf(TEXT("Euler gimbal lock at yaw %.2f converts (%s)"), Yaw, *Error), bConverted);
            if (!bConverted) continue;
            FVector ResultX, ResultY, ResultZ;
            KyaActorTransform::BuildRotationBasis(Result.Rotation, 1, ResultX, ResultY, ResultZ);
            TestTrue(FString::Printf(TEXT("Euler gimbal lock at yaw %.2f keeps the basis"), Yaw),
                ResultX.Equals(GimbalX, 2.0e-3) && ResultY.Equals(GimbalY, 2.0e-3) &&
                ResultZ.Equals(GimbalZ, 2.0e-3));
        }
    }
    {
        // A yaw edit on a yaw-only class keeps only yaw, matching the native
        // SV_BuildAngleWithOnlyY setup that clears pitch and roll.
        FVector YawX, YawY, YawZ;
        KyaActorTransform::BuildRotationBasis(FVector(0.0, 2.2, 0.0), 6, YawX, YawY, YawZ);
        const FQuat YawRotation = KyaActorTransform::QuatFromGameBasis(YawX, YawY, YawZ);
        KyaActorTransform::FGameTransform Result;
        FString Error;
        TestTrue(TEXT("Yaw-only yaw edit converts"),
            KyaActorTransform::UnrealToGameTransform(FTransform(YawRotation, FVector::ZeroVector, FVector::OneVector),
                6, FVector(0.2, 1.1, -0.4), Result, Error));
        TestTrue(TEXT("Yaw-only result keeps only yaw"),
            FMath::IsNearlyZero(Result.Rotation.X) && FMath::IsNearlyZero(Result.Rotation.Z) &&
            FMath::Abs(Result.Rotation.Y - 2.2) < 1.0e-4);
    }
    {
        // Pitch cannot be represented by a yaw-only class: reject it instead of
        // silently storing a misleading rotation.
        const FTransform Pitched(FRotator(30.0, 0.0, 0.0).Quaternion(), FVector::ZeroVector, FVector::OneVector);
        KyaActorTransform::FGameTransform Result;
        FString Error;
        TestFalse(TEXT("Yaw-only class rejects a pitched transform"),
            KyaActorTransform::UnrealToGameTransform(Pitched, 6, FVector::ZeroVector, Result, Error));
        TestFalse(TEXT("Rejected rotation has a reason"), Error.IsEmpty());
    }
    {
        const FTransform Mirrored(FQuat::Identity, FVector::ZeroVector, FVector(-1.0, 1.0, 1.0));
        KyaActorTransform::FGameTransform Result;
        FString Error;
        TestFalse(TEXT("Negative scale is rejected"),
            KyaActorTransform::UnrealToGameTransform(Mirrored, 0, FVector::ZeroVector, Result, Error));
        const FTransform Collapsed(FQuat::Identity, FVector::ZeroVector, FVector(1.0, 0.0, 1.0));
        TestFalse(TEXT("Zero scale is rejected"),
            KyaActorTransform::UnrealToGameTransform(Collapsed, 0, FVector::ZeroVector, Result, Error));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaActorEditServiceTest, "Kya.Editor.ActorEditService",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaActorEditServiceTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());

    const FKyaActorRecordKey Key = Service.MakeKey(Bank, 0);
    TestTrue(TEXT("Key carries the bank generation"), Key.Generation == uint32(Bank->EditGeneration));
    FKyaActorPlacement* Placement = nullptr;
    FString Error;
    TestTrue(TEXT("Resolve valid key"), Service.Resolve(Key, Placement, &Error));
    TestTrue(TEXT("Resolve returns the record"), Placement == &Bank->SceneEnvironment.Actors[0]);

    FKyaActorEditPatch Patch;
    Patch.Position = FVector(9.0, 8.0, 7.0);
    TestTrue(TEXT("Apply position patch"), Service.Apply(Key, Patch, EKyaEditNotify::Notify, &Error));
    TestTrue(TEXT("Position written to bank"), Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(9.0, 8.0, 7.0)));
    TestTrue(TEXT("Untouched fields preserved"), Bank->SceneEnvironment.Actors[0].Name == TEXT("FIRST") &&
        Bank->SceneEnvironment.Actors[0].Rotation.Equals(FVector(0.0, 0.6, 0.0)));

    const FVector BeforeInvalid = Bank->SceneEnvironment.Actors[0].Position;
    FKyaActorEditPatch Invalid;
    Invalid.Position = FVector(NAN, 0.0, 0.0);
    TestFalse(TEXT("NaN position rejected"), Service.Apply(Key, Invalid, EKyaEditNotify::Notify, &Error));
    TestTrue(TEXT("Rejected patch leaves the bank unchanged"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(BeforeInvalid));
    TestFalse(TEXT("Rejected patch explains why"), Error.IsEmpty());

    FKyaActorEditPatch Empty;
    TestFalse(TEXT("Empty patch rejected"), Service.Apply(Key, Empty, EKyaEditNotify::Notify, &Error));
    TestFalse(TEXT("Out-of-range record rejected"), Service.Resolve(Service.MakeKey(Bank, 99), Placement, &Error));

    // Record identity is generation guarded: a reimport invalidates old keys.
    Bank->BumpEditGeneration();
    TestFalse(TEXT("Stale key does not resolve"), Service.Resolve(Key, Placement, &Error));
    TestFalse(TEXT("Stale key does not edit"), Service.Apply(Key, Patch, EKyaEditNotify::Notify, &Error));
    TestTrue(TEXT("Stale key explains the reimport"), Error.Contains(TEXT("stale")));

    // Full-record Details support: a presentation copy edits any top-level property
    // (including nested fixed-size arrays) and writes back exactly that property.
    {
        Bank->SceneEnvironment.Actors[0].ActorFlags = 3;
        Bank->SceneEnvironment.Actors[0].LightingFlags = 9;
        Bank->SceneEnvironment.Actors[0].Conditions.Reset();
        FKyaScenarioCondition& Condition = Bank->SceneEnvironment.Actors[0].Conditions.AddDefaulted_GetRef();
        Condition.Variable = 11;
        Condition.Value = 22;

        FKyaActorPlacement Presentation = Bank->SceneEnvironment.Actors[0];
        Presentation.ActorFlags = 77;
        Presentation.Conditions[0].Value = 99;
        FKyaActorEditPatch DetailsPatch;
        TestTrue(TEXT("Details patch builds for a scalar field"),
            KyaMakePlacementPropertyPatch(Presentation, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ActorFlags), DetailsPatch));
        const FKyaActorRecordKey CurrentKey = Service.MakeKey(Bank, 0);
        TestTrue(TEXT("Details patch applies"), Service.Apply(CurrentKey, DetailsPatch, EKyaEditNotify::Silent, &Error));
        TestEqual(TEXT("Details patch writes the edited scalar"), Bank->SceneEnvironment.Actors[0].ActorFlags, 77);
        TestEqual(TEXT("Details patch preserves other scalars"), Bank->SceneEnvironment.Actors[0].LightingFlags, int64(9));
        TestEqual(TEXT("Details patch preserves untouched nested values"),
            Bank->SceneEnvironment.Actors[0].Conditions[0].Value, 22);

        // A nested array element change is carried by the containing property.
        Presentation.Conditions[0].Value = 123;
        FKyaActorEditPatch NestedPatch;
        TestTrue(TEXT("Details patch builds for a fixed-size array"),
            KyaMakePlacementPropertyPatch(Presentation, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Conditions), NestedPatch));
        TestTrue(TEXT("Nested details patch applies"),
            Service.Apply(CurrentKey, NestedPatch, EKyaEditNotify::Silent, &Error));
        TestEqual(TEXT("Nested details patch writes the element"),
            Bank->SceneEnvironment.Actors[0].Conditions[0].Value, 123);
        TestEqual(TEXT("Nested details patch keeps the array size"),
            Bank->SceneEnvironment.Actors[0].Conditions.Num(), 1);
        TestFalse(TEXT("Unknown or non-editable property is rejected"),
            KyaMakePlacementPropertyPatch(Presentation, TEXT("NotAProperty"), DetailsPatch));
    }

    // Field capability table: derived from the schema and the writer's fixed-layout
    // constraints, and used to gate the Details rows and patch construction.
    {
        FKyaActorPlacement CapabilitySource;
        FKyaActorEditPatch CapabilityPatch;
        FString CapabilityError;
        TestTrue(TEXT("Typed scalar is editable and exportable"),
            GetKyaActorFieldCapability(GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ActorFlags)).Kind ==
            EKyaFieldCapability::EditableExportable);
        TestTrue(TEXT("Actor name explains the fixed name slot"),
            GetKyaActorFieldCapability(GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Name)).Reason.Contains(TEXT("slot")));
        TestTrue(TEXT("Fixed-size arrays explain their restriction"),
            GetKyaActorFieldCapability(GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Conditions)).Reason.Contains(TEXT("Fixed-size")));
        TestTrue(TEXT("Presence flags are inspect-only"),
            GetKyaActorFieldCapability(TEXT("bHasPlatformPrefix")).Kind == EKyaFieldCapability::InspectOnly);
        TestTrue(TEXT("Serialized record index is inspect-only"),
            GetKyaActorFieldCapability(TEXT("Index")).Kind == EKyaFieldCapability::InspectOnly);
        TestTrue(TEXT("Unknown properties are unsupported"),
            GetKyaActorFieldCapability(TEXT("NotAProperty")).Kind == EKyaFieldCapability::Unsupported);
        TestTrue(TEXT("Fields the writer never encodes are inspect-only"),
            GetKyaActorFieldCapability(GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, FighterPrefixRaw)).Kind ==
            EKyaFieldCapability::InspectOnly);
        TestTrue(TEXT("Actor name explains the aligned slot constraint"),
            GetKyaActorFieldCapability(GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Name)).Reason.Contains(TEXT("Align")));
        TestTrue(TEXT("Nested strings report the aligned slot constraint"),
            GetKyaNestedFieldCapability(TEXT("Models.MeshName")).Kind == EKyaFieldCapability::EditableExportable &&
            GetKyaNestedFieldCapability(TEXT("Models.MeshName")).Reason.Contains(TEXT("aligned")));
        TestTrue(TEXT("Unknown nested fields are unsupported"),
            GetKyaNestedFieldCapability(TEXT("Models.NoSuchField")).Kind == EKyaFieldCapability::Unsupported);
        TestTrue(TEXT("Missing nested path segments are unsupported"),
            GetKyaNestedFieldCapability(TEXT("Models")).Kind == EKyaFieldCapability::Unsupported);
        TestTrue(TEXT("Read-only nested layout fields are inspect-only"),
            GetKyaNestedFieldCapability(TEXT("FighterBlows.NameCapacity")).Kind ==
            EKyaFieldCapability::InspectOnly);
        TestTrue(TEXT("Nested presence flags are inspect-only"),
            GetKyaNestedFieldCapability(TEXT("Behaviours.bHasTypedClusterZones")).Kind ==
            EKyaFieldCapability::InspectOnly);
        TestFalse(TEXT("Inspect-only fields cannot build a patch"),
            KyaMakePlacementPropertyPatch(CapabilitySource, TEXT("bHasPlatformPrefix"), CapabilityPatch, &CapabilityError));
        TestFalse(TEXT("Inspect-only rejection explains why"), CapabilityError.IsEmpty());
        TestFalse(TEXT("Capability descriptions are localized"),
            DescribeKyaFieldCapability(EKyaFieldCapability::InspectOnly).IsEmpty());
    }

    // Two previews of the same bank refresh once per edit.
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create preview world"), World)) return false;
    AKyaLevelPreview* PreviewA = World->SpawnActor<AKyaLevelPreview>();
    AKyaLevelPreview* PreviewB = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn preview A"), PreviewA) || !TestNotNull(TEXT("Spawn preview B"), PreviewB))
    {
        World->DestroyWorld(false);
        return false;
    }
    PreviewA->Bank = Bank;
    PreviewB->Bank = Bank;
    PreviewA->OnConstruction(FTransform::Identity);
    PreviewB->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Both previews registered as views"), Service.GetViewCount(Bank), 2);
    TestEqual(TEXT("Preview A created one handle per actor"), PreviewA->GetActorHandleCount(), 2);
    TestTrue(TEXT("Actor handles are editor-only"), PreviewA->GetActorHandle(0) &&
        PreviewA->GetActorHandle(0)->HasAnyFlags(RF_Transient));

    const int64 BuildsA = PreviewA->BuildCount;
    const int64 BuildsB = PreviewB->BuildCount;
    AKyaActorHandle* HandleBeforeEdit = Cast<AKyaActorHandle>(PreviewA->GetActorHandle(1));
    const FKyaActorRecordKey CurrentKey = Service.MakeKey(Bank, 1);
    FKyaActorEditPatch SecondPatch;
    SecondPatch.VisibilityDistance = 11.0f;
    TestTrue(TEXT("Apply patch through the service"), Service.Apply(CurrentKey, SecondPatch, EKyaEditNotify::Notify, &Error));
    TestEqual(TEXT("Targeted property edit does not rebuild preview A"), PreviewA->BuildCount, BuildsA);
    TestEqual(TEXT("Targeted property edit does not rebuild preview B"), PreviewB->BuildCount, BuildsB);
    TestEqual(TEXT("Visibility written"), Bank->SceneEnvironment.Actors[1].VisibilityDistance, 11.0f);
    if (AKyaActorHandle* EditedHandle = Cast<AKyaActorHandle>(PreviewA->GetActorHandle(1)))
        TestEqual(TEXT("Targeted edit synchronizes the handle"), EditedHandle->ActorVisibilityDistance, 11.0f);
    TestTrue(TEXT("Targeted edit preserves the selected handle object"),
        PreviewA->GetActorHandle(1) == HandleBeforeEdit);

    // Geometry-affecting reference changes still need a capture rebuild.
    FKyaBankEntry& TextureEntry = Bank->Entries.AddDefaulted_GetRef();
    TextureEntry.Type = 5;
    TextureEntry.Subtype = 1;
    FKyaActorEditPatch TexturePatch;
    TexturePatch.TextureIndex = 0;
    TestTrue(TEXT("Apply texture patch through the service"),
        Service.Apply(CurrentKey, TexturePatch, EKyaEditNotify::Notify, &Error));
    TestEqual(TEXT("Geometry-affecting edit rebuilds preview A"), PreviewA->BuildCount, BuildsA + 1);

    // Silent edits defer the refresh until the flush (one rebuild per drag).
    FKyaActorEditPatch Silent;
    Silent.VisibilityDistance = 42.0f;
    const int64 BuildsBeforeSilent = PreviewA->BuildCount;
    TestTrue(TEXT("Apply silent patch"), Service.Apply(CurrentKey, Silent, EKyaEditNotify::Silent, &Error));
    TestEqual(TEXT("Silent patch does not rebuild yet"), PreviewA->BuildCount, BuildsBeforeSilent);
    Service.FlushPendingNotifications();
    TestEqual(TEXT("Flush rebuilds once"), PreviewA->BuildCount, BuildsBeforeSilent + 1);

    // Sector banks are tracked as views too, so undo/redo or asset-editor changes of a
    // sector bank refresh the preview that displays it and nothing else.
    UKyaLevelBank* SectorBank = MakeKyaTestBank(GetTransientPackage(), 1);
    PreviewA->SectorBanks.Add(SectorBank);
    PreviewA->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Sector bank has one registered view"), Service.GetViewCount(SectorBank), 1);
    const int64 BuildsWithSector = PreviewA->BuildCount;
    const int64 BuildsOtherPreview = PreviewB->BuildCount;
    Service.NotifyBankChanged(SectorBank, true);
    TestEqual(TEXT("Sector bank change refreshes its preview"), PreviewA->BuildCount, BuildsWithSector + 1);
    TestEqual(TEXT("Sector bank change leaves other previews alone"), PreviewB->BuildCount, BuildsOtherPreview);
    PreviewA->SectorBanks.Reset();
    PreviewA->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Removing a sector bank unregisters it"), Service.GetViewCount(SectorBank), 0);

    PreviewA->Destroy();
    PreviewB->Destroy();
    World->DestroyWorld(false);
    TestEqual(TEXT("Destroyed previews unregister"), Service.GetViewCount(Bank), 0);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaActorHandleTransformTest, "Kya.Editor.ActorHandleTransform",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaActorHandleTransformTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create preview world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->OnConstruction(FTransform::Identity);

    AKyaActorHandle* First = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
    AKyaActorHandle* Second = Cast<AKyaActorHandle>(Preview->GetActorHandle(1));
    if (!TestNotNull(TEXT("First handle"), First) || !TestNotNull(TEXT("Second handle"), Second))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }
    TestEqual(TEXT("Handle mirrors the record index"), First->ActorIndex, 0);
    TestEqual(TEXT("Meshless actor has no sections"), First->RenderSectionCount, 0);
    TestTrue(TEXT("Handle world transform matches the bank position"),
        First->GetActorLocation().Equals(KyaActorTransform::SwapGameAxes(Bank->SceneEnvironment.Actors[0].Position) * 100.0, 0.01));

    // Translate: the visible component moves live; the bank write happens once when
    // the interaction ends and the dependent previews refresh once, at the end of the
    // frame (coalesced).
    const int64 BuildsBeforeMove = Preview->BuildCount;
    First->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
    First->PostEditMove(false);
    TestTrue(TEXT("Drag does not write the bank before the interaction ends"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(1.0, 2.0, 3.0)));
    First->PostEditMove(true);
    TestTrue(TEXT("Drag commit writes the bank position"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(2.0, 2.0, 3.0)));
    TestEqual(TEXT("Drag commit defers the refresh"), Preview->BuildCount, BuildsBeforeMove);
    Service.FlushPendingNotifications();
    TestEqual(TEXT("Targeted transform edit never rebuilds the preview"), Preview->BuildCount, BuildsBeforeMove);
    TestTrue(TEXT("Rotation preserved by translate"),
        Bank->SceneEnvironment.Actors[0].Rotation.Equals(FVector(0.0, 0.6, 0.0)));
    TestTrue(TEXT("Scale preserved by translate"),
        Bank->SceneEnvironment.Actors[0].Scale.Equals(FVector(1.0, 1.0, 1.0)));

    // The commit is its own undo step when the editor did not already own the
    // transaction, and undo restores the authoritative bank value.
    if (GEditor)
    {
        GEditor->UndoTransaction();
        TestTrue(TEXT("Undo restores the bank placement"),
            Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(1.0, 2.0, 3.0)));
        GEditor->RedoTransaction();
        TestTrue(TEXT("Redo reapplies the bank placement"),
            Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(2.0, 2.0, 3.0)));
        Service.FlushPendingNotifications();
    }

    // A multi-actor drag writes each record but refreshes the preview once.
    const int64 BuildsBeforeMulti = Preview->BuildCount;
    First->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
    Second->AddActorWorldOffset(FVector(0.0, 100.0, 0.0));
    First->PostEditMove(true);
    Second->PostEditMove(true);
    TestEqual(TEXT("Multi-actor drag does not refresh per actor"), Preview->BuildCount, BuildsBeforeMulti);
    Service.FlushPendingNotifications();
    TestEqual(TEXT("Multi-actor targeted drag never rebuilds"), Preview->BuildCount, BuildsBeforeMulti);
    TestTrue(TEXT("First actor moved by the multi-drag"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(3.0, 2.0, 3.0)));
    TestTrue(TEXT("Second actor keeps its own multi-drag movement"),
        Bank->SceneEnvironment.Actors[1].Position.Equals(FVector(-4.0, 1.0, 1.5)));

    // Multi-selection rotation/scale must also finish all commits before any rebuild.
    const FVector FirstRotationBefore = Bank->SceneEnvironment.Actors[0].Rotation;
    const FVector SecondRotationBefore = Bank->SceneEnvironment.Actors[1].Rotation;
    const int64 BuildsBeforeRotate = Preview->BuildCount;
    First->SetActorRelativeRotation(FQuat(FVector::UpVector, 0.4));
    Second->SetActorRelativeRotation(FQuat(FVector::UpVector, 0.7));
    First->PostEditMove(true);
    Second->PostEditMove(true);
    TestTrue(TEXT("First actor rotation committed"),
        !Bank->SceneEnvironment.Actors[0].Rotation.Equals(FirstRotationBefore, 1.0e-4));
    TestTrue(TEXT("Second actor rotation committed"),
        !Bank->SceneEnvironment.Actors[1].Rotation.Equals(SecondRotationBefore, 1.0e-4));
    TestEqual(TEXT("Multi-rotation commits coalesce before rebuilding"),
        Preview->BuildCount, BuildsBeforeRotate);
    Service.FlushPendingNotifications();
    TestEqual(TEXT("Multi-rotation rebuilds once after all commits"),
        Preview->BuildCount, BuildsBeforeRotate + 1);

    // A click or a canceled drag restores the original transform and writes nothing.
    const FVector BeforeNoop = Bank->SceneEnvironment.Actors[0].Position;
    First->PostEditMove(true);
    TestTrue(TEXT("No-op move leaves the bank unchanged"), Bank->SceneEnvironment.Actors[0].Position.Equals(BeforeNoop));

    // Scale: non-uniform scale round trips through the game axes.
    First->SetActorRelativeScale3D(KyaActorTransform::SwapGameAxes(FVector(2.0, 3.0, 4.0)));
    First->PostEditMove(true);
    TestTrue(TEXT("Scale writeback uses game axis order"),
        Bank->SceneEnvironment.Actors[0].Scale.Equals(FVector(2.0, 3.0, 4.0)));

    // Yaw-only class: pitch is not representable, so the edit is rejected and the
    // visible transform is restored from the bank instead of storing a misleading value.
    Bank->SceneEnvironment.Actors[1].ClassId = 6;
    Second->SyncFromBank();
    const FVector BeforeRejected = Bank->SceneEnvironment.Actors[1].Rotation;
    Second->SetActorRelativeRotation(FRotator(25.0, 40.0, 0.0));
    Second->PostEditMove(true);
    TestTrue(TEXT("Yaw-only rotation edit rejected"), Bank->SceneEnvironment.Actors[1].Rotation.Equals(BeforeRejected));

    // Shared diagnostic mesh mode also coalesces transform commits before rebuilding.
    {
        Preview->bShowAllActorMeshes = true;
        Preview->OnConstruction(FTransform::Identity);
        AKyaActorHandle* SharedFirst = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
        AKyaActorHandle* SharedSecond = Cast<AKyaActorHandle>(Preview->GetActorHandle(1));
        if (TestNotNull(TEXT("Shared-mode first handle"), SharedFirst) &&
            TestNotNull(TEXT("Shared-mode second handle"), SharedSecond))
        {
            const FVector FirstBefore = Bank->SceneEnvironment.Actors[0].Position;
            const FVector SecondBefore = Bank->SceneEnvironment.Actors[1].Position;
            const int64 BuildsBeforeShared = Preview->BuildCount;
            SharedFirst->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
            SharedSecond->AddActorWorldOffset(FVector(0.0, 100.0, 0.0));
            SharedFirst->PostEditMove(true);
            SharedSecond->PostEditMove(true);
            TestTrue(TEXT("Shared-mode first commit keeps its movement"),
                Bank->SceneEnvironment.Actors[0].Position.Equals(FirstBefore + FVector(1.0, 0.0, 0.0)));
            TestTrue(TEXT("Shared-mode second commit keeps its movement"),
                Bank->SceneEnvironment.Actors[1].Position.Equals(SecondBefore + FVector(0.0, 0.0, 1.0)));
            TestEqual(TEXT("Shared-mode commits do not rebuild mid-commit"),
                Preview->BuildCount, BuildsBeforeShared);
            Service.FlushPendingNotifications();
            TestEqual(TEXT("Shared-mode commits rebuild once after flush"),
                Preview->BuildCount, BuildsBeforeShared + 1);
        }
        Preview->bShowAllActorMeshes = false;
        Preview->OnConstruction(FTransform::Identity);
    }

    // Details edits write only the edited field and never the whole record.
    Bank->SceneEnvironment.Actors[0].CullingDistance = 12.5f;
    First->ActorVisibilityDistance = 77.0f;
    FPropertyChangedEvent ChangedEvent(
        FindFProperty<FProperty>(AKyaActorHandle::StaticClass(), GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorVisibilityDistance)));
    First->PostEditChangeProperty(ChangedEvent);
    TestEqual(TEXT("Details edit writes the edited field"), Bank->SceneEnvironment.Actors[0].VisibilityDistance, 77.0f);
    TestEqual(TEXT("Details edit preserves unrelated fields"), Bank->SceneEnvironment.Actors[0].CullingDistance, 12.5f);

    // A nested coordinate edit reports the leaf property; the containing member must
    // still identify the field that changed.
    FProperty* PositionProperty = FindFProperty<FProperty>(AKyaActorHandle::StaticClass(),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorPosition));
    FProperty* XProperty = FindFProperty<FProperty>(TBaseStructure<FVector>::Get(), TEXT("X"));
    if (TestNotNull(TEXT("Actor position property"), PositionProperty) &&
        TestNotNull(TEXT("Vector X property"), XProperty))
    {
        First->ActorPosition = FVector(6.0, 7.0, 8.0);
        FPropertyChangedEvent NestedEvent(XProperty);
        NestedEvent.SetActiveMemberProperty(PositionProperty);
        First->PostEditChangeProperty(NestedEvent);
        TestTrue(TEXT("Nested coordinate edit writes the containing field"),
            Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(6.0, 7.0, 8.0)));
        TestTrue(TEXT("Nested coordinate edit keeps other fields"),
            Bank->SceneEnvironment.Actors[0].Scale.Equals(FVector(2.0, 3.0, 4.0)));
    }

    // Outliner organization: duplicates are disambiguated without touching the bank,
    // and generated actors live under a per-bank/per-sector folder.
    {
        Bank->SceneEnvironment.Actors[1].Name = Bank->SceneEnvironment.Actors[0].Name;
        Preview->OnConstruction(FTransform::Identity);
        AKyaActorHandle* RenamedFirst = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
        AKyaActorHandle* RenamedSecond = Cast<AKyaActorHandle>(Preview->GetActorHandle(1));
        if (TestNotNull(TEXT("First handle after rename"), RenamedFirst) &&
            TestNotNull(TEXT("Second handle after rename"), RenamedSecond))
        {
            TestTrue(TEXT("Duplicate display names are disambiguated"),
                RenamedFirst->GetActorLabel() != RenamedSecond->GetActorLabel());
            TestTrue(TEXT("Display labels carry the game class"),
                RenamedFirst->GetActorLabel().Contains(TEXT("[class")));
            TestEqual(TEXT("Serialized names are untouched"),
                Bank->SceneEnvironment.Actors[0].Name, Bank->SceneEnvironment.Actors[1].Name);
            AActor* GroupParent = RenamedFirst->GetAttachParentActor();
            if (TestNotNull(TEXT("Handles are attached to a sector grouping actor"), GroupParent))
            {
                TestTrue(TEXT("Sector grouping actor identifies its sector"),
                    GroupParent->GetActorLabel().Contains(TEXT("sector")) ||
                    GroupParent->GetActorLabel().Contains(TEXT("global")));
                TestTrue(TEXT("Grouping actor is transient"),
                    GroupParent->HasAnyFlags(RF_Transient));
            }
            const FString FirstLabel = RenamedFirst->GetActorLabel();
            Preview->OnConstruction(FTransform::Identity);
            if (AKyaActorHandle* Rebuilt = Cast<AKyaActorHandle>(Preview->GetActorHandle(0)))
                TestEqual(TEXT("Display labels stay stable across rebuilds"), Rebuilt->GetActorLabel(), FirstLabel);

            if (GroupParent)
            {
                // Grouping actors are organizational only: moving one cannot move the
                // level or corrupt a child's writeback.
                const FVector BankBefore = Bank->SceneEnvironment.Actors[0].Position;
                const FVector ChildWorldBefore = RenamedFirst->GetActorLocation();
                GroupParent->AddActorWorldOffset(FVector(500.0, 0.0, 0.0));
                GroupParent->PostEditMove(true);
                TestTrue(TEXT("Grouping actor cannot move the level"),
                    RenamedFirst->GetActorLocation().Equals(ChildWorldBefore, 0.01));
                RenamedFirst->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
                RenamedFirst->PostEditMove(true);
                TestTrue(TEXT("Child writeback is independent of the grouping actor"),
                    Bank->SceneEnvironment.Actors[0].Position.Equals(BankBefore + FVector(1.0, 0.0, 0.0)));
                Service.FlushPendingNotifications();
            }

            // Reusing the preview for another bank refreshes the group label.
            UKyaLevelBank* OtherBank = MakeKyaTestBank(GetTransientPackage(), 1);
            Preview->Bank = OtherBank;
            Preview->OnConstruction(FTransform::Identity);
            if (AKyaActorHandle* OtherHandle = Cast<AKyaActorHandle>(Preview->GetActorHandle(0)))
            {
                if (AActor* OtherGroup = OtherHandle->GetAttachParentActor())
                    TestTrue(TEXT("Reused grouping actor takes the new bank name"),
                        OtherGroup->GetActorLabel().Contains(OtherBank->GetName()));
            }
        }
    }

    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaActorOwnedGeometryTest, "Kya.Editor.ActorOwnedGeometry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaActorOwnedGeometryTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the actor-owned geometry test."));
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

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create preview world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;

    auto PreviewVertexCount = [Preview]() {
        int32 Count = 0;
        for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
            if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section))
                Count += MeshSection->ProcVertexBuffer.Num();
        return Count;
    };

    // Handle-owned actor geometry is the default editing route.
    Preview->bSpawnActorHandles = true;
    Preview->bShowActorMeshesOnHandles = true;
    Preview->bShowAllActorMeshes = false;
    Preview->bShowActorMesh = false;
    Preview->OnConstruction(FTransform::Identity);

    int32 Handles = 0;
    int32 HandledTriangles = 0;
    int32 MeshedHandles = 0;
    for (int32 ActorIndex = 0; ActorIndex < Bank->SceneEnvironment.Actors.Num(); ++ActorIndex)
    {
        AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Preview->GetActorHandle(ActorIndex));
        if (!Handle) continue;
        ++Handles;
        if (Handle->RenderSectionCount == 0) continue;
        ++MeshedHandles;
        for (int32 Section = 0; Section < Handle->ActorMesh->GetNumSections(); ++Section)
            if (const FProcMeshSection* MeshSection = Handle->ActorMesh->GetProcMeshSection(Section))
                HandledTriangles += MeshSection->ProcIndexBuffer.Num() / 3;
    }
    AddInfo(FString::Printf(TEXT("handles=%d meshed_handles=%d handled_triangles=%d shared_vertices=%d"),
        Handles, MeshedHandles, HandledTriangles, PreviewVertexCount()));
    TestTrue(TEXT("Actor handles exist for the loaded level"), Handles > 0);
    TestTrue(TEXT("At least one actor mesh is owned by its handle"), MeshedHandles > 0);
    TestTrue(TEXT("Handle sections carry triangles"), HandledTriangles > 0);

    const int32 SharedWithHandles = PreviewVertexCount();

    // Without handles nothing renders the actor meshes, so the shared count must be
    // unchanged: actor triangles are not duplicated into the preview mesh.
    Preview->bSpawnActorHandles = false;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Actor geometry is not duplicated into the shared preview mesh"),
        PreviewVertexCount(), SharedWithHandles);
    TestEqual(TEXT("Disabling handles removes the representations"), Preview->GetActorHandleCount(), 0);

    // The diagnostic all-actor mesh mode keeps the shared route; handles then hold no mesh.
    Preview->bShowAllActorMeshes = true;
    Preview->bSpawnActorHandles = true;
    Preview->OnConstruction(FTransform::Identity);
    TestTrue(TEXT("Diagnostic mesh mode renders actors in the shared mesh"),
        PreviewVertexCount() > SharedWithHandles);
    int32 DiagnosticSections = 0;
    for (int32 ActorIndex = 0; ActorIndex < Bank->SceneEnvironment.Actors.Num(); ++ActorIndex)
        if (AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Preview->GetActorHandle(ActorIndex)))
            DiagnosticSections += Handle->RenderSectionCount;
    TestEqual(TEXT("Diagnostic mesh mode does not draw actors twice"), DiagnosticSections, 0);

    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMeshAssignmentTest, "Kya.Editor.MeshAssignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMeshAssignmentTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run mesh assignment regression coverage."));
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

    // Keep the real mesh/material bytes, but isolate assignment to one meshless actor.
    Bank->SceneEnvironment.Actors.Reset();
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors.AddDefaulted_GetRef();
    Actor.Name = TEXT("MESH_ASSIGNMENT");
    Actor.ClassId = 0;
    Actor.Scale = FVector::OneVector;
    Actor.MeshIndex = Actor.TextureIndex = -1;
    Actor.SectorId = -1;
    // Force material resolution through the union of texture banks. This used to
    // assert on the first lookup in the initially empty TextureBankSections map.
    Bank->SceneEnvironment.AlternateTextureIndex = -1;

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bAsyncDecodeMeshes = false;
    Preview->bSpawnActorHandles = true;
    Preview->bShowActorMeshesOnHandles = true;
    Preview->bShowAllActorMeshes = false;
    Preview->bShowActorMesh = false;
    Preview->OnConstruction(FTransform::Identity);
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.RegisterView(Bank, Preview);

    auto Assign = [&](int32 MeshIndex)
    {
        FKyaActorPlacement Copy = Bank->SceneEnvironment.Actors[0];
        Copy.MeshIndex = MeshIndex;
        FKyaActorEditPatch Patch;
        FString Error;
        if (!TestTrue(TEXT("Build the same property patch as the mesh picker"),
            KyaMakePlacementPropertyPatch(Copy, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, MeshIndex), Patch, &Error))) return;
        TestTrue(FString::Printf(TEXT("Assign mesh %d and notify preview: %s"), MeshIndex, *Error),
            Service.Apply(Service.MakeKey(Bank, 0), Patch, EKyaEditNotify::Notify, &Error));
        TestEqual(TEXT("Assignment stored"), Bank->SceneEnvironment.Actors[0].MeshIndex, MeshIndex);
    };

    int32 MeshCount = 0;
    int32 RenderedCount = 0;
    for (const FKyaBankEntry& Entry : Bank->Entries)
    {
        if (Entry.Type != 4 || Entry.Subtype != 1) continue;
        AddInfo(FString::Printf(TEXT("Assigning mesh %d"), MeshCount));
        Assign(MeshCount++);
        const TArray<FKyaActorMeshSection>* Sections = Preview->FindActorMeshSections(0);
        if (Sections && !Sections->IsEmpty()) ++RenderedCount;
        Assign(-1);
        Sections = Preview->FindActorMeshSections(0);
        TestTrue(TEXT("Clearing mesh removes captured geometry"), !Sections || Sections->IsEmpty());
    }
    TestTrue(TEXT("Exercised real mesh assignments"), MeshCount > 0);
    TestTrue(TEXT("Assigned meshes produced geometry"), RenderedCount > 0);
    // Repeat in the diagnostic shared-mesh route, which uses the same resolver.
    Preview->bShowAllActorMeshes = true;
    Assign(0);
    Assign(-1);
    Service.UnregisterView(Preview);
    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaExportValidationTest, "Kya.Editor.ExportValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaExportValidationTest::RunTest(const FString& Parameters)
{
    // A bank without a scene configuration is rejected before anything is written.
    UKyaLevelBank* Unsupported = MakeKyaTestBank(GetTransientPackage());
    FString Reason;
    TestFalse(TEXT("Bank without scene configuration is not exportable"),
        KyaBankSupportsExport(Unsupported, Reason));
    TestFalse(TEXT("Unsupported bank explains why"), Reason.IsEmpty());
    {
        FKyaBankExportDiagnostics Diagnostics;
        TArray<uint8> Bytes;
        TestFalse(TEXT("Export builder rejects the unsupported bank"),
            KyaBuildBankExportBytes(Unsupported, Bytes, Diagnostics));
        TestTrue(TEXT("Builder reports diagnostics"), Diagnostics.HasErrors());
        TestTrue(TEXT("Rejected bank leaves no bytes"), Bytes.IsEmpty());
    }

    // A failed preflight must not replace an existing destination file.
    const FString TempDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaExportTest"));
    const FString SentinelPath = FPaths::Combine(TempDirectory, TEXT("Unsupported.bnk"));
    IFileManager::Get().MakeDirectory(*TempDirectory, true);
    const TArray<uint8> Sentinel = {1, 2, 3, 4};
    TestTrue(TEXT("Write sentinel destination"), FFileHelper::SaveArrayToFile(Sentinel, *SentinelPath));
    {
        FKyaBankExportRequest Request;
        Request.Bank = Unsupported;
        Request.DestinationFilename = SentinelPath;
        const TArray<FKyaBankExportOutcome> Outcomes = KyaWriteValidatedExportSet({Request});
        TestEqual(TEXT("One outcome per request"), Outcomes.Num(), 1);
        if (!Outcomes.IsEmpty())
        {
            TestFalse(TEXT("Unsupported bank is not published"), Outcomes[0].bPublished);
            TestTrue(TEXT("Unsupported bank reports its reason"), !Outcomes[0].Errors.IsEmpty());
        }
        TArray<uint8> After;
        TestTrue(TEXT("Read sentinel destination"), FFileHelper::LoadFileToArray(After, *SentinelPath));
        TestTrue(TEXT("Failed export preserves the existing file"), After == Sentinel);
        TestFalse(TEXT("No staging file remains"),
            FPaths::FileExists(SentinelPath + TEXT(".kya-staging")));
    }

    // Two requests must not share a destination: the collision is rejected before
    // either is staged, so no bank's bytes can be published under another's name.
    {
        const FString SharedPath = FPaths::Combine(TempDirectory, TEXT("Shared.bnk"));
        FKyaBankExportRequest RequestA;
        RequestA.Bank = Unsupported;
        RequestA.DestinationFilename = SharedPath;
        FKyaBankExportRequest RequestB = RequestA;
        const TArray<FKyaBankExportOutcome> Outcomes = KyaWriteValidatedExportSet({RequestA, RequestB});
        if (TestEqual(TEXT("Duplicate requests report one outcome each"), Outcomes.Num(), 2))
        {
            TestTrue(TEXT("Duplicate destination is rejected"),
                Outcomes[1].Errors.ContainsByPredicate([](const FString& Error)
                {
                    return Error.Contains(TEXT("Duplicate export destination"));
                }));
            TestFalse(TEXT("No duplicate destination is published"), Outcomes[1].bPublished);
        }
    }

    // Real-bank coverage: build, stage, publish and reimport the exported bytes.
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (!Filename.IsEmpty())
    {
        TArray<uint8> Input;
        if (TestTrue(TEXT("Read test bank"), FFileHelper::LoadFileToArray(Input, *Filename)))
        {
            const uint8* Cursor = Input.GetData();
            UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
            UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
                GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
                Input.GetData() + Input.Num(), GWarn));
            if (TestNotNull(TEXT("Import test bank"), Bank))
            {
                TestTrue(TEXT("Real bank is exportable"), KyaBankSupportsExport(Bank, Reason));
                FKyaBankExportDiagnostics Diagnostics;
                TArray<uint8> Built;
                if (TestTrue(TEXT("Build export bytes"), KyaBuildBankExportBytes(Bank, Built, Diagnostics)))
                {
                    const FString PublishedPath = FPaths::Combine(TempDirectory, TEXT("LevelOne.bnk"));
                    FKyaBankExportRequest Request;
                    Request.Bank = Bank;
                    Request.DestinationFilename = PublishedPath;
                    const TArray<FKyaBankExportOutcome> Outcomes = KyaWriteValidatedExportSet({Request});
                    if (TestTrue(TEXT("One export outcome"), Outcomes.Num() == 1))
                    {
                        TestTrue(TEXT("Validated export is published"), Outcomes[0].bPublished);
                    }
                    TArray<uint8> Published;
                    if (TestTrue(TEXT("Read published export"), FFileHelper::LoadFileToArray(Published, *PublishedPath)))
                        TestTrue(TEXT("Published bytes match the builder"), Published == Built);

                    const uint8* ReimportCursor = Built.GetData();
                    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
                        UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
                        TEXT("bnk"), ReimportCursor, Built.GetData() + Built.Num(), GWarn));
                    if (TestNotNull(TEXT("Reimport exported bytes"), Reimported))
                    {
                        TestEqual(TEXT("Reimport keeps the actor count"),
                            Reimported->SceneEnvironment.Actors.Num(), Bank->SceneEnvironment.Actors.Num());
                        TestTrue(TEXT("Reimport keeps the original bank size"),
                            Reimported->BankBytes.Num() == Bank->BankBytes.Num());
                        if (Bank->SceneEnvironment.Actors.Num() > 0)
                        {
                            const FKyaActorPlacement& Original = Bank->SceneEnvironment.Actors[0];
                            const FKyaActorPlacement& Copy = Reimported->SceneEnvironment.Actors[0];
                            TestEqual(TEXT("Reimport keeps the actor name"), Copy.Name, Original.Name);
                            TestTrue(TEXT("Reimport keeps the actor position"), Copy.Position.Equals(Original.Position));
                            TestTrue(TEXT("Reimport keeps the actor rotation"), Copy.Rotation.Equals(Original.Rotation));
                            TestTrue(TEXT("Reimport keeps the actor scale"), Copy.Scale.Equals(Original.Scale));
                        }
                    }
                }
            }
        }
    }
    else
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the real-bank export validation."));
    }
    IFileManager::Get().DeleteDirectory(*TempDirectory, false, true);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaActorCatalogTest, "Kya.Editor.ActorCatalog",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaActorCatalogTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    for (int32 Index = 0; Index < 2; ++Index)
    {
        FKyaBankEntry& MeshEntry = Bank->Entries.AddDefaulted_GetRef();
        MeshEntry.Type = 4;
        MeshEntry.Subtype = 1;
    }
    for (int32 Index = 0; Index < 3; ++Index)
    {
        FKyaBankEntry& TextureEntry = Bank->Entries.AddDefaulted_GetRef();
        TextureEntry.Type = 5;
        TextureEntry.Subtype = 1;
    }

    // Recovered class and behaviour labels, with explicit unknown labels.
    TestEqual(TEXT("Moving platform class label"), GetKyaActorClassLabel(0x0d), FString(TEXT("Moving platform (13)")));
    TestTrue(TEXT("Unknown class is labelled"), GetKyaActorClassLabel(0x7f).Contains(TEXT("Unknown")));
    TestTrue(TEXT("Class notes are recovered"), GetKyaActorClassInfo(0x0d).Notes.Contains(TEXT("carried actors")));
    TestEqual(TEXT("Bridge wind behaviour label"), GetKyaBehaviourLabel(0x17, 3), FString(TEXT("Bridge wind (3)")));
    TestEqual(TEXT("Switch multi-condition label"), GetKyaBehaviourLabel(0x11, 5),
        FString(TEXT("Switch multi-condition (5)")));
    TestTrue(TEXT("Unknown behaviour is labelled"), GetKyaBehaviourLabel(0x17, 4).Contains(TEXT("unknown")));
    TestEqual(TEXT("Hedgehog class label"), GetKyaActorClassLabel(0x22), FString(TEXT("Hedgehog (34)")));
    TestTrue(TEXT("Hedgehog class notes are recovered"),
        GetKyaActorClassInfo(0x22).Notes.Contains(TEXT("CAddOnGenerator")));
    TestEqual(TEXT("Hedgehog guard area label"), GetKyaBehaviourLabel(0x22, 3), FString(TEXT("Guard area (3)")));
    TestEqual(TEXT("Hedgehog watchdog label"), GetKyaBehaviourLabel(0x22, 4), FString(TEXT("Watch dog (4)")));
    TestEqual(TEXT("Hedgehog guard area armor label"), GetKyaBehaviourLabel(0x22, 5),
        FString(TEXT("Guard area armor (5)")));
    TestEqual(TEXT("Hedgehog watchdog armor label"), GetKyaBehaviourLabel(0x22, 6),
        FString(TEXT("Watch dog armor (6)")));
    TestTrue(TEXT("Hedgehog inherited behaviour is not invented"),
        GetKyaBehaviourLabel(0x22, 2).Contains(TEXT("unknown")));
    TestTrue(TEXT("Hedgehog family is registered"), KyaActorFamilies::Find(0x22) != nullptr);
    TestEqual(TEXT("Clusteriser class label"), GetKyaActorClassLabel(0x18), FString(TEXT("Clusteriser (24)")));
    TestEqual(TEXT("Clusteriser behaviour label"), GetKyaBehaviourLabel(0x18, 2), FString(TEXT("Cluster zones (2)")));
    TestEqual(TEXT("Moving platform trajectory label"), GetKyaBehaviourLabel(0x0d, 2),
        FString(TEXT("Trajectory (2)")));
    TestEqual(TEXT("Moving platform teleport-random label"), GetKyaBehaviourLabel(0x0d, 10),
        FString(TEXT("Teleport random (10)")));
    TestEqual(TEXT("Moving platform source type"), FString(KyaActorFamilies::Find(0x0d)->SourceType),
        FString(TEXT("CActorMovingPlatform")));
    TestEqual(TEXT("Switch Wolfen-counter label"), GetKyaBehaviourLabel(0x11, 4),
        FString(TEXT("Wolfen counter (4)")));
    TestEqual(TEXT("Switch sequence label"), GetKyaBehaviourLabel(0x11, 7),
        FString(TEXT("Switch sequence (7)")));
    TestEqual(TEXT("Switch timed label"), GetKyaBehaviourLabel(0x11, 8),
        FString(TEXT("Switch timed (8)")));
    TestEqual(TEXT("Wind behaviour label"), GetKyaBehaviourLabel(0x12, 2), FString(TEXT("Wind (2)")));
    TestEqual(TEXT("Wind source type"), FString(KyaActorFamilies::Find(0x12)->SourceType),
        FString(TEXT("CActorWind")));
    TestEqual(TEXT("Trap stand label"), GetKyaBehaviourLabel(0x2c, 2), FString(TEXT("Trap stand (2)")));
    TestEqual(TEXT("Trap inactive label"), GetKyaBehaviourLabel(0x2c, 3), FString(TEXT("Inactive (3)")));
    TestEqual(TEXT("Trap source type"), FString(KyaActorFamilies::Find(0x2c)->SourceType),
        FString(TEXT("CActorTrap")));
    TestEqual(TEXT("Weapon pistol label"), GetKyaBehaviourLabel(0x20, 3), FString(TEXT("Pistol (3)")));
    TestEqual(TEXT("Weapon RPG label"), GetKyaBehaviourLabel(0x20, 6), FString(TEXT("RPG (6)")));
    TestEqual(TEXT("Weapon source type"), FString(KyaActorFamilies::Find(0x20)->SourceType),
        FString(TEXT("CActorWeapon")));
    TestEqual(TEXT("Araignos default label"), GetKyaBehaviourLabel(0x21, 2),
        FString(TEXT("Araignos default (2)")));
    TestEqual(TEXT("Araignos source type"), FString(KyaActorFamilies::Find(0x21)->SourceType),
        FString(TEXT("CActorAraignos")));
    TestEqual(TEXT("Bonus turn label"), GetKyaBehaviourLabel(0x0e, 3), FString(TEXT("Bonus turn (3)")));
    TestEqual(TEXT("Bonus path label"), GetKyaBehaviourLabel(0x0e, 4), FString(TEXT("Bonus path (4)")));
    TestEqual(TEXT("Bonus source type"), FString(KyaActorFamilies::Find(0x0e)->SourceType),
        FString(TEXT("CActorBonus")));
    TestEqual(TEXT("Teleporter default label"), GetKyaBehaviourLabel(0x24, 2),
        FString(TEXT("Teleporter default (2)")));
    TestEqual(TEXT("Teleporter source type"), FString(KyaActorFamilies::Find(0x24)->SourceType),
        FString(TEXT("CActorTeleporter")));
    TestEqual(TEXT("Amortos stand label"), GetKyaBehaviourLabel(0x29, 2),
        FString(TEXT("Amortos stand (2)")));
    TestEqual(TEXT("Amortos source type"), FString(KyaActorFamilies::Find(0x29)->SourceType),
        FString(TEXT("CActorAmortos")));
    TestEqual(TEXT("DCA controlled label"), GetKyaBehaviourLabel(0x1b, 3),
        FString(TEXT("DCA controlled (3)")));
    TestEqual(TEXT("DCA trajectory label"), GetKyaBehaviourLabel(0x1b, 5),
        FString(TEXT("DCA trajectory (5)")));
    TestEqual(TEXT("DCA source type"), FString(KyaActorFamilies::Find(0x1b)->SourceType),
        FString(TEXT("CActorDCA")));
    TestEqual(TEXT("Fruit squashed label"), GetKyaBehaviourLabel(0x16, 4),
        FString(TEXT("Fruit squashed (4)")));
    TestEqual(TEXT("Fruit bonus label"), GetKyaBehaviourLabel(0x16, 6),
        FString(TEXT("Fruit bonus (6)")));
    TestEqual(TEXT("Fruit source type"), FString(KyaActorFamilies::Find(0x16)->SourceType),
        FString(TEXT("CActorFruit")));
    TestEqual(TEXT("Nativ take-and-put label"), GetKyaBehaviourLabel(0x0a, 6),
        FString(TEXT("Nativ take and put (6)")));
    TestEqual(TEXT("Nativ source type"), FString(KyaActorFamilies::Find(0x0a)->SourceType),
        FString(TEXT("CActorNativ")));
    TestEqual(TEXT("Wolfen track label"), GetKyaBehaviourLabel(0x09, 15),
        FString(TEXT("Track (15)")));
    TestEqual(TEXT("Wolfen DCA label"), GetKyaBehaviourLabel(0x09, 23),
        FString(TEXT("Wolfen DCA (23)")));
    TestEqual(TEXT("Wolfen source type"), FString(KyaActorFamilies::Find(0x09)->SourceType),
        FString(TEXT("CActorWolfen")));
    TestEqual(TEXT("Commander label"), GetKyaBehaviourLabel(0x13, 2),
        FString(TEXT("Commander (2)")));
    TestEqual(TEXT("Commander source type"), FString(KyaActorFamilies::Find(0x13)->SourceType),
        FString(TEXT("CActorCommander")));
    TestEqual(TEXT("Kim source type"), FString(KyaActorFamilies::Find(0x06)->SourceType),
        FString(TEXT("CActorHeroPrivate")));
    TestEqual(TEXT("Boomy launch label"), GetKyaBehaviourLabel(0x05, 2),
        FString(TEXT("Boomy launch (2)")));
    TestEqual(TEXT("Boomy source type"), FString(KyaActorFamilies::Find(0x05)->SourceType),
        FString(TEXT("CActorBoomy")));
    TestEqual(TEXT("Aton escape label"), GetKyaBehaviourLabel(0x38, 3),
        FString(TEXT("Aton escape (3)")));
    TestEqual(TEXT("Aton source type"), FString(KyaActorFamilies::Find(0x38)->SourceType),
        FString(TEXT("CActorAton")));
    TestEqual(TEXT("Rope stand label"), GetKyaBehaviourLabel(0x0f, 2),
        FString(TEXT("Rope stand (2)")));
    TestEqual(TEXT("Rope source type"), FString(KyaActorFamilies::Find(0x0f)->SourceType),
        FString(TEXT("CActorRope")));
    TestEqual(TEXT("Stiller stand label"), GetKyaBehaviourLabel(0x4c, 2),
        FString(TEXT("Stiller stand (2)")));
    TestEqual(TEXT("Stiller source type"), FString(KyaActorFamilies::Find(0x4c)->SourceType),
        FString(TEXT("CActorStiller")));
    TestEqual(TEXT("Ambre label"), GetKyaBehaviourLabel(0x14, 2),
        FString(TEXT("Ambre (2)")));
    TestEqual(TEXT("Ambre unknown label"), GetKyaBehaviourLabel(0x14, 3),
        FString(TEXT("Ambre unknown (3)")));
    TestEqual(TEXT("Ambre source type"), FString(KyaActorFamilies::Find(0x14)->SourceType),
        FString(TEXT("CActorAmbre")));
    TestEqual(TEXT("Fountain label"), GetKyaBehaviourLabel(0x52, 2),
        FString(TEXT("Bonus fountain stand (2)")));
    TestEqual(TEXT("Fountain source type"), FString(KyaActorFamilies::Find(0x52)->SourceType),
        FString(TEXT("CActorBonusFountain")));
    TestEqual(TEXT("Checkpoints label"), GetKyaBehaviourLabel(0x1e, 2),
        FString(TEXT("Checkpoint (2)")));
    TestEqual(TEXT("Checkpoints source type"), FString(KyaActorFamilies::Find(0x1e)->SourceType),
        FString(TEXT("CActorCheckpointManager")));
    TestEqual(TEXT("Bridge basic label"), GetKyaBehaviourLabel(0x17, 2),
        FString(TEXT("Bridge basic (2)")));
    TestEqual(TEXT("Bridge wind label"), GetKyaBehaviourLabel(0x17, 3),
        FString(TEXT("Bridge wind (3)")));
    TestEqual(TEXT("Bridge source type"), FString(KyaActorFamilies::Find(0x17)->SourceType),
        FString(TEXT("CActorBridge")));
    TestEqual(TEXT("Fog zones label"), GetKyaBehaviourLabel(0x3d, 2),
        FString(TEXT("Fog zones (2)")));
    TestEqual(TEXT("Fog source type"), FString(KyaActorFamilies::Find(0x3d)->SourceType),
        FString(TEXT("CActorFogManager")));
    TestEqual(TEXT("Shoot fire label"), GetKyaBehaviourLabel(0x3e, 3),
        FString(TEXT("Shoot fire (3)")));
    TestEqual(TEXT("Shoot wave label"), GetKyaBehaviourLabel(0x3e, 4),
        FString(TEXT("Shoot fire wave (4)")));
    TestEqual(TEXT("Shoot source type"), FString(KyaActorFamilies::Find(0x3e)->SourceType),
        FString(TEXT("CActorShoot")));
    TestEqual(TEXT("Money flock label"), GetKyaBehaviourLabel(0x32, 2),
        FString(TEXT("Money flock (2)")));
    TestEqual(TEXT("Money add-on label"), GetKyaBehaviourLabel(0x32, 3),
        FString(TEXT("Money add-on (3)")));
    TestEqual(TEXT("Money source type"), FString(KyaActorFamilies::Find(0x32)->SourceType),
        FString(TEXT("CActorMoney")));
    TestEqual(TEXT("Box stand label"), GetKyaBehaviourLabel(0x35, 3),
        FString(TEXT("Box stand (3)")));
    TestEqual(TEXT("Box source type"), FString(KyaActorFamilies::Find(0x35)->SourceType),
        FString(TEXT("CActorBox")));
    TestEqual(TEXT("FX single label"), GetKyaBehaviourLabel(0x2e, 2),
        FString(TEXT("FX single (2)")));
    TestEqual(TEXT("FX path label"), GetKyaBehaviourLabel(0x2e, 3),
        FString(TEXT("FX path (3)")));
    TestEqual(TEXT("FX source type"), FString(KyaActorFamilies::Find(0x2e)->SourceType),
        FString(TEXT("CActorFx")));
    TestEqual(TEXT("Cluster zones label"), GetKyaBehaviourLabel(0x18, 2),
        FString(TEXT("Cluster zones (2)")));
    TestEqual(TEXT("Clusteriser source type"), FString(KyaActorFamilies::Find(0x18)->SourceType),
        FString(TEXT("CActorClusteriser")));
    TestEqual(TEXT("Companion alert label"), GetKyaBehaviourLabel(0x19, 3),
        FString(TEXT("Companion alert (3)")));
    TestEqual(TEXT("Companion source type"), FString(KyaActorFamilies::Find(0x19)->SourceType),
        FString(TEXT("CActorCompanion")));
    TestEqual(TEXT("Gravity fall label"), GetKyaBehaviourLabel(0x10, 3),
        FString(TEXT("Gravity fall (3)")));
    TestEqual(TEXT("Gravity bell label"), GetKyaBehaviourLabel(0x10, 4),
        FString(TEXT("Gravity bell (4)")));
    TestEqual(TEXT("Gravity source type"), FString(KyaActorFamilies::Find(0x10)->SourceType),
        FString(TEXT("CActorGravityAware")));
    TestEqual(TEXT("Rune label"), GetKyaBehaviourLabel(0x51, 2),
        FString(TEXT("Rune (2)")));
    TestEqual(TEXT("Rune source type"), FString(KyaActorFamilies::Find(0x51)->SourceType),
        FString(TEXT("CActorRune")));
    TestEqual(TEXT("Micken eat label"), GetKyaBehaviourLabel(0x07, 4),
        FString(TEXT("Micken eat (4)")));
    TestEqual(TEXT("Micken source type"), FString(KyaActorFamilies::Find(0x07)->SourceType),
        FString(TEXT("CActorMicken")));
    TestEqual(TEXT("Basic box stand label"), GetKyaBehaviourLabel(0x39, 2),
        FString(TEXT("Basic box stand (2)")));
    TestEqual(TEXT("Basic box source type"), FString(KyaActorFamilies::Find(0x39)->SourceType),
        FString(TEXT("CActorBasicBox")));
    TestEqual(TEXT("Nativ command stand label"), GetKyaBehaviourLabel(0x3c, 2),
        FString(TEXT("Nativ command stand (2)")));
    TestEqual(TEXT("Nativ command source type"), FString(KyaActorFamilies::Find(0x3c)->SourceType),
        FString(TEXT("CActorNativCmd")));
    TestEqual(TEXT("Event generator label"), GetKyaBehaviourLabel(0x3b, 2),
        FString(TEXT("Event generator (2)")));
    TestEqual(TEXT("Event generator source type"), FString(KyaActorFamilies::Find(0x3b)->SourceType),
        FString(TEXT("CActorEventGenerator")));
    TestEqual(TEXT("Explosive distributor label"), GetKyaBehaviourLabel(0x1d, 2),
        FString(TEXT("Explosive distributor (2)")));
    TestEqual(TEXT("Explosive distributor source type"), FString(KyaActorFamilies::Find(0x1d)->SourceType),
        FString(TEXT("CActorExplosiveDistributor")));
    TestEqual(TEXT("Shocker fire wave label"), GetKyaBehaviourLabel(0x4b, 3),
        FString(TEXT("Shocker fire wave (3)")));
    TestEqual(TEXT("Shocker source type"), FString(KyaActorFamilies::Find(0x4b)->SourceType),
        FString(TEXT("CActorShocker")));
    TestEqual(TEXT("Nativ shop label"), GetKyaBehaviourLabel(0x34, 2),
        FString(TEXT("Nativ shop sell (2)")));
    TestEqual(TEXT("Nativ shop source type"), FString(KyaActorFamilies::Find(0x34)->SourceType),
        FString(TEXT("CActorNativShop")));
    TestEqual(TEXT("Punching ball label"), GetKyaBehaviourLabel(0x30, 7),
        FString(TEXT("Punching ball stand (7)")));
    TestEqual(TEXT("Punching ball source type"), FString(KyaActorFamilies::Find(0x30)->SourceType),
        FString(TEXT("CActorPunchingBall")));
    TestEqual(TEXT("Nose monster label"), GetKyaBehaviourLabel(0x26, 3),
        FString(TEXT("Nose monster attack (3)")));
    TestEqual(TEXT("Nose monster source type"), FString(KyaActorFamilies::Find(0x26)->SourceType),
        FString(TEXT("CActorNoseMonster")));
    TestEqual(TEXT("Pattern alone label"), GetKyaBehaviourLabel(0x3f, 3),
        FString(TEXT("Pattern alone (3)")));
    TestEqual(TEXT("Pattern source type"), FString(KyaActorFamilies::Find(0x3f)->SourceType),
        FString(TEXT("CActorPattern")));
    TestEqual(TEXT("Ship fly label"), GetKyaBehaviourLabel(0x36, 2),
        FString(TEXT("Ship fly (2)")));
    TestEqual(TEXT("Ship source type"), FString(KyaActorFamilies::Find(0x36)->SourceType),
        FString(TEXT("CActorShip")));
    TestEqual(TEXT("Bunch label"), GetKyaBehaviourLabel(0x4e, 26),
        FString(TEXT("Bunch repel (26)")));
    TestEqual(TEXT("Bunch source type"), FString(KyaActorFamilies::Find(0x4e)->SourceType),
        FString(TEXT("CActorBunch")));
    TestEqual(TEXT("Brazul label"), GetKyaBehaviourLabel(0x4f, 26),
        FString(TEXT("Brazul A (26)")));
    TestEqual(TEXT("Brazul source type"), FString(KyaActorFamilies::Find(0x4f)->SourceType),
        FString(TEXT("CActorBrazul")));
    TestEqual(TEXT("Minigame betting label"), GetKyaBehaviourLabel(0x44, 3),
        FString(TEXT("Minigame betting (3)")));
    TestEqual(TEXT("Minigame source type"), FString(KyaActorFamilies::Find(0x44)->SourceType),
        FString(TEXT("CActorMiniGame")));
    TestEqual(TEXT("Minigame time-attack source type"), FString(KyaActorFamilies::Find(0x45)->SourceType),
        FString(TEXT("CActorMiniGameTimeAttack")));
    TestEqual(TEXT("Minigame box-counter source type"), FString(KyaActorFamilies::Find(0x46)->SourceType),
        FString(TEXT("CActorMiniGameBoxCounter")));
    TestEqual(TEXT("Minigame Boomy source type"), FString(KyaActorFamilies::Find(0x47)->SourceType),
        FString(TEXT("CActorMiniGameBoomy")));
    TestEqual(TEXT("Minigame distance source type"), FString(KyaActorFamilies::Find(0x55)->SourceType),
        FString(TEXT("CActorMiniGameDistance")));
    TestFalse(TEXT("Hedgehog behaviour path index is not a top-level reference"), GetKyaReferenceFields().ContainsByPredicate(
        [](const FKyaReferenceField& Field) { return Field.PropertyName == FName(TEXT("HedgehogPathFollowIndex")); }));

    // Bank-local reference resolution and pickers.
    TestEqual(TEXT("Model ordinals follow type-4 entries"), GetKyaModelOrdinals(*Bank).Num(), 2);
    TestEqual(TEXT("Texture ordinals follow type-5 entries"), GetKyaTextureBankOrdinals(*Bank).Num(), 3);
    const FKyaBankReference Mesh0 = ResolveKyaModelReference(*Bank, 0);
    TestTrue(TEXT("Mesh 0 resolves"), Mesh0.bResolved);
    TestEqual(TEXT("Mesh 0 label"), Mesh0.Label, FString(TEXT("Mesh #0")));
    const FKyaBankReference MeshMissing = ResolveKyaModelReference(*Bank, 5);
    TestFalse(TEXT("Mesh 5 does not resolve"), MeshMissing.bResolved);
    TestTrue(TEXT("Unresolved mesh explains why"), MeshMissing.Warning.Contains(TEXT("no type-4 mesh entry")));
    TestTrue(TEXT("Alternate texture bank resolves"), ResolveKyaTextureReference(*Bank, -1).bResolved);
    TestEqual(TEXT("Texture bank label"), ResolveKyaTextureReference(*Bank, 1).Label, FString(TEXT("Texture bank #1")));
    TestFalse(TEXT("Missing texture bank does not resolve"), ResolveKyaTextureReference(*Bank, 9).bResolved);

    TMap<int32, FString> LoadedSectors;
    LoadedSectors.Add(7, TEXT("SECT7"));
    TestEqual(TEXT("Loaded sector label"), GetKyaSectorLabel(LoadedSectors, 7), FString(TEXT("Sector 7 (SECT7)")));
    TestEqual(TEXT("Missing sector label"), GetKyaSectorLabel(LoadedSectors, 9), FString(TEXT("Sector 9 (not loaded)")));
    TestEqual(TEXT("Global sector label"), GetKyaSectorLabel(LoadedSectors, -1), FString(TEXT("Global (no sector)")));

    FKyaActorPlacement Placement = Bank->SceneEnvironment.Actors[0];
    Placement.ClassId = 0x17;
    Placement.Behaviours.Reset();
    FKyaActorBehaviour& Behaviour = Placement.Behaviours.AddDefaulted_GetRef();
    Behaviour.Id = 3;
    const TArray<FKyaBehaviourOption> BehaviourOptions = GetKyaBehaviourOptions(Placement);
    if (TestEqual(TEXT("Behaviour option count"), BehaviourOptions.Num(), 1))
        TestEqual(TEXT("Behaviour option label"), BehaviourOptions[0].Label,
            FString(TEXT("Behaviour 0: Bridge wind (3)")));

    // Reference scope validation: mesh/texture/sector are blocking, behaviour is a
    // warning shown inline in Details.
    Placement.MeshIndex = 5;
    Placement.TextureIndex = 9;
    Placement.SectorId = -2;
    Placement.DefaultBehaviourId = 4;
    const TArray<FKyaActorReferenceIssue> Issues = ValidateKyaActorReferences(*Bank, Placement);
    const auto FindIssue = [&Issues](const TCHAR* Field, bool bBlocking)
    {
        return Issues.ContainsByPredicate([Field, bBlocking](const FKyaActorReferenceIssue& Issue)
        {
            return Issue.Field == Field && Issue.bBlocking == bBlocking;
        });
    };
    TestTrue(TEXT("Unresolved mesh is blocking"), FindIssue(TEXT("MeshIndex"), true));
    TestTrue(TEXT("Unresolved texture is blocking"), FindIssue(TEXT("TextureIndex"), true));
    TestTrue(TEXT("Invalid sector id is blocking"), FindIssue(TEXT("SectorId"), true));
    TestTrue(TEXT("Out-of-range default behaviour is a warning"), FindIssue(TEXT("DefaultBehaviourId"), false));

    // Scalar reference fields are class-gated and resolve inside the bank.
    EKyaReferenceKind Kind = EKyaReferenceKind::Sector;
    TestTrue(TEXT("Micken waypoint field is a waypoint reference"),
        FindKyaReferenceField(TEXT("MickenWaypointIndex"), Kind) && Kind == EKyaReferenceKind::Waypoint);
    TestTrue(TEXT("Bridge waypoint field is a waypoint reference"),
        FindKyaReferenceField(TEXT("BridgeWaypointA"), Kind) && Kind == EKyaReferenceKind::Waypoint);
    TestFalse(TEXT("Non-reference fields are not reference fields"), FindKyaReferenceField(TEXT("Position"), Kind));
    TestTrue(TEXT("Reference kinds have descriptions"),
        !DescribeKyaReferenceKind(EKyaReferenceKind::Actor).IsEmpty());

    Bank->SceneEnvironment.Waypoints.AddDefaulted();
    FKyaActorPlacement ClassPlacement = Placement;
    ClassPlacement.ClassId = 0x07;
    ClassPlacement.MickenWaypointIndex = 5;
    TestTrue(TEXT("Out-of-range class reference is reported"),
        ValidateKyaActorReferences(*Bank, ClassPlacement).ContainsByPredicate(
            [](const FKyaActorReferenceIssue& Issue)
            {
                return Issue.Field == TEXT("MickenWaypointIndex") && Issue.bBlocking;
            }));
    ClassPlacement.ClassId = 0x0d;
    TestFalse(TEXT("Class-specific references are ignored for other classes"),
        ValidateKyaActorReferences(*Bank, ClassPlacement).ContainsByPredicate(
            [](const FKyaActorReferenceIssue& Issue)
            {
                return Issue.Field == TEXT("MickenWaypointIndex");
            }));
    TestTrue(TEXT("Numeric reference fields can be written"),
        SetKyaNumericField(ClassPlacement, TEXT("MickenWaypointIndex"), 0));
    TestEqual(TEXT("Numeric reference fields can be read"),
        GetKyaNumericField(ClassPlacement, TEXT("MickenWaypointIndex")), int64(0));


    // The edit service rejects bad references before mutating the record.
    const FKyaActorRecordKey Key = Service.MakeKey(Bank, 0);
    FString Error;
    FKyaActorEditPatch Patch;
    Patch.MeshIndex = 5;
    TestFalse(TEXT("Service rejects an unresolved mesh"), Service.Apply(Key, Patch, EKyaEditNotify::Silent, &Error));
    TestTrue(TEXT("Mesh rejection explains scope"), Error.Contains(TEXT("type-4 mesh entry")));
    Patch.MeshIndex = 0;
    TestTrue(TEXT("Service accepts a resolved mesh"), Service.Apply(Key, Patch, EKyaEditNotify::Silent, &Error));
    TestEqual(TEXT("Mesh index written"), Bank->SceneEnvironment.Actors[0].MeshIndex, 0);
    FKyaActorEditPatch BadSector;
    BadSector.SectorId = -2;
    TestFalse(TEXT("Service rejects an invalid sector id"),
        Service.Apply(Key, BadSector, EKyaEditNotify::Silent, &Error));
    FKyaActorEditPatch BadBehaviour;
    BadBehaviour.DefaultBehaviourId = 4;
    TestFalse(TEXT("Service rejects an out-of-range default behaviour"),
        Service.Apply(Key, BadBehaviour, EKyaEditNotify::Silent, &Error));

    // Details edits arrive as Custom patches; changed references are still validated.
    Bank->SceneEnvironment.Actors[0].MeshIndex = -1;
    FKyaActorPlacement DetailsCopy = Bank->SceneEnvironment.Actors[0];
    DetailsCopy.MeshIndex = 5;
    FKyaActorEditPatch DetailsPatch;
    TestTrue(TEXT("Details patch builds for a reference field"),
        KyaMakePlacementPropertyPatch(DetailsCopy, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, MeshIndex),
            DetailsPatch, &Error));
    TestFalse(TEXT("Service rejects a Details patch with an unresolved mesh"),
        Service.Apply(Key, DetailsPatch, EKyaEditNotify::Silent, &Error));
    TestTrue(TEXT("Details reference rejection explains scope"), Error.Contains(TEXT("type-4 mesh entry")));

    // Serialized widths and name-slot buckets are validated before mutation.
    FKyaActorEditPatch NegativeFlags;
    NegativeFlags.LightingFlags = -1;
    TestFalse(TEXT("Service rejects a negative lighting-flags value"),
        Service.Apply(Key, NegativeFlags, EKyaEditNotify::Silent, &Error));
    Bank->SceneEnvironment.Actors[0].Name = TEXT("FIRST");
    FKyaActorPlacement SameBucket = Bank->SceneEnvironment.Actors[0];
    SameBucket.Name = TEXT("FIRS");
    FKyaActorEditPatch SameBucketPatch;
    TestTrue(TEXT("Same-bucket name patch builds"),
        KyaMakePlacementPropertyPatch(SameBucket, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Name),
            SameBucketPatch, &Error));
    TestTrue(TEXT("Service accepts a name in the same 4-byte bucket"),
        Service.Apply(Key, SameBucketPatch, EKyaEditNotify::Silent, &Error));
    FKyaActorPlacement SmallerBucket = Bank->SceneEnvironment.Actors[0];
    SmallerBucket.Name = TEXT("FIR");
    FKyaActorEditPatch SmallerBucketPatch;
    TestTrue(TEXT("Smaller-bucket name patch builds"),
        KyaMakePlacementPropertyPatch(SmallerBucket, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Name),
            SmallerBucketPatch, &Error));
    TestFalse(TEXT("Service rejects a name that changes the 4-byte bucket"),
        Service.Apply(Key, SmallerBucketPatch, EKyaEditNotify::Silent, &Error));
    TestTrue(TEXT("Name slot rejection explains the writer rule"), Error.Contains(TEXT("Align")));

    // int32 serialized fields keep their full bit pattern (0x80000000 is a valid value).
    FKyaActorPlacement HighBits = Bank->SceneEnvironment.Actors[0];
    HighBits.ActorFlags = MIN_int32;
    HighBits.AnimationLayers = MIN_int32;
    HighBits.Field40 = MIN_int32;
    FString WidthError;
    TestTrue(TEXT("int32 bit patterns are accepted"), ValidateKyaActorPlacement(HighBits, WidthError));

    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeneratedActorLifecycleTest, "Kya.Editor.GeneratedActorLifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaGeneratedActorLifecycleTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    Bank->SceneEnvironment.Waypoints.AddDefaulted();
    FKyaPath& Path = Bank->SceneEnvironment.Paths.AddDefaulted_GetRef();
    Path.Points.Add(FVector4(1.0, 2.0, 3.0, 1.0));

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create lifecycle world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn lifecycle preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bSpawnWaypointHandles = true;
    Preview->bSpawnPathPointHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
    AKyaSpatialRecordHandle* Spatial = Cast<AKyaSpatialRecordHandle>(Preview->GetWaypointHandle(0));
    if (TestNotNull(TEXT("Actor handle exists"), Handle) && TestNotNull(TEXT("Waypoint handle exists"), Spatial))
    {
        TestTrue(TEXT("Actor handles are transient"), Handle->HasAnyFlags(RF_Transient));
        TestTrue(TEXT("Actor handles are transactional"), Handle->HasAnyFlags(RF_Transactional));
        TestTrue(TEXT("Actor handles are editor-only"), Handle->IsEditorOnly());
        TestFalse(TEXT("Actor handles are not templates"), Handle->IsTemplate());
        TestTrue(TEXT("Spatial handles are transient"), Spatial->HasAnyFlags(RF_Transient));
        TestTrue(TEXT("Spatial handles are editor-only"), Spatial->IsEditorOnly());
        AActor* Group = Handle->GetAttachParentActor();
        if (TestNotNull(TEXT("Sector grouping actor exists"), Group))
        {
            TestTrue(TEXT("Grouping actors are transient"), Group->HasAnyFlags(RF_Transient));
            TestTrue(TEXT("Grouping actors are editor-only"), Group->IsEditorOnly());
            TestFalse(TEXT("Preview owns its grouping actor"), !Preview->OwnsGeneratedActor(Group));
        }
        TestTrue(TEXT("Preview owns its handles"), Preview->OwnsGeneratedActor(Handle) &&
            Preview->OwnsGeneratedActor(Spatial));

        // A duplicated/pasted generated actor is not owned and is removed on rebuild.
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.ObjectFlags |= RF_Transient | RF_Transactional;
        AKyaActorHandle* Clone = World->SpawnActor<AKyaActorHandle>(
            AKyaActorHandle::StaticClass(), FTransform::Identity, Params);
        if (TestNotNull(TEXT("Duplicate handle spawns"), Clone))
        {
            Clone->Initialize(Preview, Bank, 0, true);
            TestFalse(TEXT("Duplicate handle is not owned"), Preview->OwnsGeneratedActor(Clone));
            Preview->OnConstruction(FTransform::Identity);
            TestFalse(TEXT("Rebuild removes the duplicate handle"), IsValid(Clone));
            TestTrue(TEXT("Rebuild keeps the owned handle"), IsValid(Preview->GetActorHandle(0)));
        }
    }

    // A second preview of the same bank has its own independent representations.
    AKyaLevelPreview* SecondPreview = World->SpawnActor<AKyaLevelPreview>();
    if (TestNotNull(TEXT("Second preview spawns"), SecondPreview))
    {
        SecondPreview->Bank = Bank;
        SecondPreview->OnConstruction(FTransform::Identity);
        TestTrue(TEXT("Previews have independent handles"),
            SecondPreview->GetActorHandle(0) != Preview->GetActorHandle(0));
        TestTrue(TEXT("Each preview owns its own handle"),
            Preview->OwnsGeneratedActor(Preview->GetActorHandle(0)) &&
            SecondPreview->OwnsGeneratedActor(SecondPreview->GetActorHandle(0)) &&
            !Preview->OwnsGeneratedActor(SecondPreview->GetActorHandle(0)));
        SecondPreview->Destroy();
    }

    // Map reload: destroying the preview cleans up its representations, and a new
    // preview in the same world recreates them without duplicates.
    Preview->Destroy();
    TestFalse(TEXT("Destroying the preview cleans up its generated actors"), IsValid(Handle));
    Preview = World->SpawnActor<AKyaLevelPreview>();
    if (TestNotNull(TEXT("Reloaded preview spawns"), Preview))
    {
        Preview->Bank = Bank;
        Preview->OnConstruction(FTransform::Identity);
        TestEqual(TEXT("Reloading recreates one representation per actor"),
            Preview->GetActorHandleCount(), Bank->SceneEnvironment.Actors.Num());
        TestTrue(TEXT("Reloaded handles are new objects"), Preview->GetActorHandle(0) != Handle);
    }

    // Editor-generated representations must not exist in a game (PIE/packaged) world.
    UWorld* GameWorld = UWorld::CreateWorld(EWorldType::Game, false);
    if (TestNotNull(TEXT("Create game world"), GameWorld))
    {
        AKyaLevelPreview* GamePreview = GameWorld->SpawnActor<AKyaLevelPreview>();
        if (TestNotNull(TEXT("Spawn game preview"), GamePreview))
        {
            GamePreview->Bank = Bank;
            GamePreview->bSpawnWaypointHandles = true;
            GamePreview->bSpawnPathPointHandles = true;
            GamePreview->OnConstruction(FTransform::Identity);
            TestEqual(TEXT("No actor handles in a game world"), GamePreview->GetActorHandleCount(), 0);
            TestEqual(TEXT("No spatial handles in a game world"), GamePreview->GetSpatialHandleCount(), 0);
            int32 GeneratedActors = 0;
            for (TActorIterator<AActor> It(GameWorld); It; ++It)
                if (KyaIsGeneratedActor(*It)) ++GeneratedActors;
            TestEqual(TEXT("No generated actors in a game world"), GeneratedActors, 0);
            GamePreview->Destroy();
        }
        GameWorld->DestroyWorld(false);
    }

    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaStructuralCommandsTest, "Kya.Editor.StructuralCommands",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaStructuralCommandsTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    Bank->SceneEnvironment.Waypoints.AddDefaulted();
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create structural world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn structural preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bSpawnWaypointHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    AActor* Handle = Preview->GetActorHandle(0);
    AActor* Spatial = Preview->GetWaypointHandle(0);
    AActor* Group = Handle ? Handle->GetAttachParentActor() : nullptr;
    if (TestNotNull(TEXT("Actor handle exists"), Handle))
    {
        FText Reason;
        TestFalse(TEXT("Delete is blocked for actor handles"),
            Cast<AKyaActorHandle>(Handle)->CanDeleteSelectedActor(Reason));
        TestFalse(TEXT("Delete rejection explains why"), Reason.IsEmpty());
        TestTrue(TEXT("Actor handle is a generated actor"), KyaIsGeneratedActor(Handle));
        TestFalse(TEXT("Preview is not a generated representation"), KyaIsGeneratedActor(Preview));
        TestTrue(TEXT("Generated actors are detected in a selection"),
            KyaContainsGeneratedActor({Handle}));
        TestFalse(TEXT("Preview alone is not a generated selection"),
            KyaContainsGeneratedActor({Preview}));
    }
    if (TestNotNull(TEXT("Spatial handle exists"), Spatial))
    {
        FText Reason;
        TestFalse(TEXT("Delete is blocked for spatial handles"),
            Cast<AKyaSpatialRecordHandle>(Spatial)->CanDeleteSelectedActor(Reason));
        TestFalse(TEXT("Spatial delete rejection explains why"), Reason.IsEmpty());
    }
    if (TestNotNull(TEXT("Grouping actor exists"), Group))
    {
        FText Reason;
        TestFalse(TEXT("Delete is blocked for grouping actors"),
            Cast<AKyaActorGroup>(Group)->CanDeleteSelectedActor(Reason));
    }

    // Orphaned generated actors (for example from a copy/paste that lost its preview)
    // are removed, while owned representations are kept.
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.ObjectFlags |= RF_Transient | RF_Transactional;
    AKyaActorHandle* Orphan = World->SpawnActor<AKyaActorHandle>(
        AKyaActorHandle::StaticClass(), FTransform::Identity, Params);
    if (TestNotNull(TEXT("Orphan handle spawns"), Orphan))
    {
        TestTrue(TEXT("Orphan handle is a generated actor"), KyaIsGeneratedActor(Orphan));
        TestTrue(TEXT("Stray cleanup removes at least the orphan"),
            KyaDestroyStrayGeneratedActors(World) >= 1);
        TestFalse(TEXT("Orphan handle is destroyed"), IsValid(Orphan));
        TestTrue(TEXT("Owned handle survives stray cleanup"), IsValid(Handle));
    }

    // A duplicated handle must never write through the source identity, even before
    // the command-end cleanup runs.
    AKyaActorHandle* Clone = World->SpawnActor<AKyaActorHandle>(
        AKyaActorHandle::StaticClass(), FTransform::Identity, Params);
    if (TestNotNull(TEXT("Duplicate handle spawns"), Clone))
    {
        Clone->Initialize(Preview, Bank, 0, true);
        const FVector BeforeCloneMove = Bank->SceneEnvironment.Actors[0].Position;
        Clone->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
        Clone->PostEditMove(true);
        TestTrue(TEXT("Duplicate handle cannot write the bank"),
            Bank->SceneEnvironment.Actors[0].Position.Equals(BeforeCloneMove));
        Clone->ActorVisibilityDistance = 123.0f;
        FPropertyChangedEvent CloneEvent(FindFProperty<FProperty>(AKyaActorHandle::StaticClass(),
            GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorVisibilityDistance)));
        Clone->PostEditChangeProperty(CloneEvent);
        TestEqual(TEXT("Duplicate handle Details edits are ignored"),
            Bank->SceneEnvironment.Actors[0].VisibilityDistance, 0.0f);
        Clone->ApplyToBank();
        TestTrue(TEXT("Duplicate handle script write is ignored"),
            Bank->SceneEnvironment.Actors[0].Position.Equals(BeforeCloneMove));
    }

    // The sort of copy/paste duplicate that arrives with no generated actor selected
    // is still removed by the command-end sweep.
    AKyaActorHandle* Pasted = World->SpawnActor<AKyaActorHandle>(
        AKyaActorHandle::StaticClass(), FTransform::Identity, Params);
    if (TestNotNull(TEXT("Pasted handle spawns"), Pasted))
    {
        Pasted->Initialize(Preview, Bank, 1, true);
        const int32 Removed = KyaDestroyStrayGeneratedActors(World);
        TestTrue(TEXT("Paste cleanup removes the generated copy"), Removed >= 1);
        TestFalse(TEXT("Pasted handle is destroyed"), IsValid(Pasted));
        TestTrue(TEXT("Original handle survives paste cleanup"), IsValid(Handle));
    }

    if (GEditor && Handle)
    {
        GEditor->SelectNone(false, true);
        GEditor->SelectActor(Handle, true, true);
        TestTrue(TEXT("Selection reports generated actors"), KyaSelectionContainsGeneratedActor());
        GEditor->SelectNone(false, true);
        TestFalse(TEXT("Empty selection has no generated actors"), KyaSelectionContainsGeneratedActor());
    }

    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaLightExportTest, "Kya.Editor.LightExport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaLightExportTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the light export test."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read light bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import light bank"), Bank)) return false;
    if (Bank->Lights.IsEmpty())
    {
        AddWarning(TEXT("The test bank has no light archive; skipping light export."));
        return true;
    }

    const int32 LightCount = Bank->Lights.Num();
    const FVector OriginalPosition = Bank->Lights[0].Position;
    const float OriginalMultiplier = Bank->Lights[0].Multiplier;
    Bank->Lights[0].Position = OriginalPosition + FVector(1.0, 2.0, 3.0);
    Bank->Lights[0].Multiplier = OriginalMultiplier + 0.25f;

    FKyaBankExportDiagnostics Diagnostics;
    TArray<uint8> Built;
    if (!TestTrue(TEXT("Build light export bytes"), KyaBuildBankExportBytes(Bank, Built, Diagnostics))) return false;
    for (const FKyaBankEntry& Entry : Bank->Entries)
    {
        if (Entry.Type != 16 || Entry.Subtype != 1) continue;
        TArray<FKyaLightOffsets> OriginalOffsets, BuiltOffsets;
        if (!KyaReadLightOffsets(MakeArrayView(Bank->BankBytes.GetData(), Bank->BankBytes.Num()),
            Entry.Offset + 16, Entry.Size - 16, OriginalOffsets)) continue;
        if (!KyaReadLightOffsets(MakeArrayView(Built.GetData(), Built.Num()),
            Entry.Offset + 16, Entry.Size - 16, BuiltOffsets)) continue;
        if (OriginalOffsets.IsValidIndex(0) && BuiltOffsets.IsValidIndex(0))
        {
            bool bColorsPreserved = true;
            for (const int64 ColorOffset : {OriginalOffsets[0].Ambient, OriginalOffsets[0].Color, OriginalOffsets[0].Field20})
            {
                const int64 Relative = ColorOffset - OriginalOffsets[0].Ambient;
                for (int32 Channel = 0; Channel < 4; ++Channel)
                    if (Bank->BankBytes[ColorOffset + Channel] != Built[BuiltOffsets[0].Ambient + Relative + Channel])
                        bColorsPreserved = false;
            }
            TestTrue(TEXT("Untouched saturated light colors keep their original bytes"), bColorsPreserved);

            // Editing only red must not disturb the neighboring channels.
            const int64 ColorBase = OriginalOffsets[0].Color;
            const uint8 OriginalG = Bank->BankBytes[ColorBase + 1];
            const uint8 OriginalB = Bank->BankBytes[ColorBase + 2];
            const uint8 OriginalA = Bank->BankBytes[ColorBase + 3];
            Bank->Lights[0].Color.R = Bank->Lights[0].Color.R < 0.9f
                ? Bank->Lights[0].Color.R + 0.1f : Bank->Lights[0].Color.R - 0.1f;
            TArray<uint8> RedBuilt;
            TestTrue(TEXT("Rebuild after a red-only edit"), KyaBuildBankExportBytes(Bank, RedBuilt, Diagnostics));
            TArray<FKyaLightOffsets> RedOffsets;
            if (KyaReadLightOffsets(MakeArrayView(RedBuilt.GetData(), RedBuilt.Num()),
                Entry.Offset + 16, Entry.Size - 16, RedOffsets) && RedOffsets.IsValidIndex(0))
            {
                TestEqual(TEXT("Red-only edit keeps the green byte"),
                    int32(RedBuilt[RedOffsets[0].Color + 1]), int32(OriginalG));
                TestEqual(TEXT("Red-only edit keeps the blue byte"),
                    int32(RedBuilt[RedOffsets[0].Color + 2]), int32(OriginalB));
                TestEqual(TEXT("Red-only edit keeps the alpha byte"),
                    int32(RedBuilt[RedOffsets[0].Color + 3]), int32(OriginalA));
            }
        }
        break;
    }

    const uint8* ReimportCursor = Built.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), ReimportCursor,
        Built.GetData() + Built.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport light bank"), Reimported)) return false;
    TestEqual(TEXT("Light count survives export"), Reimported->Lights.Num(), LightCount);
    if (Reimported->Lights.IsValidIndex(0))
    {
        TestEqual(TEXT("Light type survives export"), Reimported->Lights[0].LightType, Bank->Lights[0].LightType);
        TestTrue(TEXT("Light position survives export"),
            Reimported->Lights[0].Position.Equals(Bank->Lights[0].Position, 1.0e-3));
        TestTrue(TEXT("Light multiplier survives export"),
            FMath::IsNearlyEqual(Reimported->Lights[0].Multiplier, Bank->Lights[0].Multiplier, 1.0e-3f));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaRunningPortExportValidationTest, "Kya.Editor.RunningPortExportValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaRunningPortExportValidationTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the running-port export validation."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read running-port bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import running-port bank"), Bank)) return false;
    if (!TestTrue(TEXT("Running-port bank has actors"), Bank->SceneEnvironment.Actors.Num() > 0)) return false;

    const int32 PortActorIndex = 0;
    if (!TestTrue(TEXT("Running-port bank has a first actor"), Bank->SceneEnvironment.Actors.Num() > 0))
        return false;
    // The validation copy clears the mesh/texture/collision references so the native
    // Create path needs no scene/file-manager resources; the edit under test is the
    // position/visibility/culling values. The source bank asset is not modified.
    FKyaActorPlacement& PortRecord = Bank->SceneEnvironment.Actors[PortActorIndex];
    PortRecord.MeshIndex = -1;
    PortRecord.TextureIndex = 0;
    PortRecord.CollisionIndex = -1;
    const FKyaActorPlacement& Original = PortRecord;
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    FKyaActorEditPatch Patch;
    Patch.Position = Original.Position + FVector(1.25, -2.5, 3.75);
    Patch.VisibilityDistance = 4321.0f;
    Patch.CullingDistance = 8765.0f;
    FString Error;
    if (!TestTrue(TEXT("Apply running-port validation edit"),
        Service.Apply(Service.MakeKey(Bank, PortActorIndex), Patch, EKyaEditNotify::Silent, &Error))) return false;
    const FKyaActorPlacement& Edited = Bank->SceneEnvironment.Actors[PortActorIndex];

    // Locate actor 0's serialized chunk for the port parser.
    auto ReadWord = [&Input](int64 Offset) -> uint32 {
        return uint32(Input[Offset]) | (uint32(Input[Offset + 1]) << 8) |
            (uint32(Input[Offset + 2]) << 16) | (uint32(Input[Offset + 3]) << 24);
    };
    const FKyaBankEntry* SceneData = Bank->Entries.FindByPredicate(
        [](const FKyaBankEntry& Entry) { return Entry.Type == 6 && Entry.Subtype == 1; });
    if (!TestNotNull(TEXT("Running-port scene data entry"), SceneData)) return false;
    int64 SetupOffset = SceneData->Offset + 20;
    if (ReadWord(SceneData->Offset + 16)) SetupOffset += 12;
    const int32 BackgroundId = int32(ReadWord(SetupOffset));
    SetupOffset += 4 + (BackgroundId != -1 ? 4 : 0);
    int64 CameraConfigOffset = 0, SceneValuesOffset = 0, MapEndOffset = 0, ScenarioEndOffset = 0, ObjectiveEndOffset = 0;
    int64 WaypointOffset = 0;
    int32 WaypointCount = 0;
    TArray<FKyaConditionedOffsets> ScenarioOffsets;
    TArray<FKyaObjectiveOffsets> ObjectiveOffsets;
    TArray<FKyaPathOffsets> PathOffsets;
    TArray<FKyaActorOffsets> ActorOffsets;
    if (!TestTrue(TEXT("Locate running-port camera config"), KyaFindCameraConfig(Input, SetupOffset, CameraConfigOffset)) ||
        !TestTrue(TEXT("Locate running-port scene values"), KyaFindSceneAfterCamera(Input, CameraConfigOffset, SceneValuesOffset)) ||
        !TestTrue(TEXT("Locate running-port map end"), KyaFindMapEnd(Input, SceneValuesOffset + 12, MapEndOffset)) ||
        !TestTrue(TEXT("Locate running-port scenario"), KyaReadConditionedOffsets(Input, MapEndOffset + 48, ScenarioOffsets, ScenarioEndOffset)) ||
        !TestTrue(TEXT("Locate running-port objectives"), KyaReadObjectiveOffsets(Input, ScenarioEndOffset, ObjectiveOffsets, ObjectiveEndOffset)) ||
        !TestTrue(TEXT("Locate running-port waypoints"), KyaFindWaypoints(Input, ObjectiveEndOffset, WaypointOffset, WaypointCount)) ||
        !TestTrue(TEXT("Locate running-port paths"), KyaReadPathOffsets(Input, WaypointOffset, WaypointCount, SceneData->Offset, PathOffsets)) ||
        !TestTrue(TEXT("Locate running-port actors"), KyaReadActorOffsets(Input, WaypointOffset, SceneData->Offset, ActorOffsets)) ||
        !TestTrue(TEXT("Running-port actor offsets exist"), ActorOffsets.Num() > 0))
        return false;

    TArray<uint8> Exported;
    FKyaBankExportDiagnostics Diagnostics;
    if (!TestTrue(TEXT("Build running-port validation bytes"),
        KyaBuildBankExportBytes(Bank, Exported, Diagnostics))) return false;
    const FString OutputDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaRunningPortValidation"));
    IFileManager::Get().MakeDirectory(*OutputDirectory, true);
    const FString OutputFilename = FPaths::Combine(OutputDirectory, TEXT("LEVEL.BNK"));
    if (!TestTrue(TEXT("Write running-port validation bank"), FFileHelper::SaveArrayToFile(Exported, *OutputFilename)))
        return false;

    const FString PortTest = FPaths::ConvertRelativePathToFull(
        FPaths::Combine(FPaths::ProjectDir(), TEXT("../../bin/WIN/KyaPortTest.exe")));
    if (!FPaths::FileExists(PortTest))
    {
        AddWarning(FString::Printf(TEXT("Running-port validation skipped: %s does not exist."), *PortTest));
        return true;
    }

    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_BANK"), *OutputFilename);
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_ACTOR_OFFSET"),
        *FString::Printf(TEXT("%lld"), ActorOffsets[PortActorIndex].Chunk));
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_ACTOR_INDEX"),
        *FString::Printf(TEXT("%d"), PortActorIndex));
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_ACTOR_TYPE"),
        *FString::Printf(TEXT("%lld"), Edited.ClassId));
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_ACTOR_NAME"), *Edited.Name);
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_POSITION_X"), *FString::SanitizeFloat(Edited.Position.X));
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_POSITION_Y"), *FString::SanitizeFloat(Edited.Position.Y));
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_POSITION_Z"), *FString::SanitizeFloat(Edited.Position.Z));
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_VISIBILITY"), *FString::SanitizeFloat(Edited.VisibilityDistance));
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_CULLING"), *FString::SanitizeFloat(Edited.CullingDistance));
    FPlatformMisc::SetEnvironmentVar(TEXT("KYA_VALIDATION_NATIVE_CREATE"), TEXT("1"));
    AddInfo(FString::Printf(TEXT("KYA_VALIDATION_BANK=%s OFFSET=%lld TYPE=%lld NAME=%s POS=(%f,%f,%f) VIS=%f CULL=%f"),
        *OutputFilename, ActorOffsets[PortActorIndex].Chunk, Edited.ClassId, *Edited.Name,
        Edited.Position.X, Edited.Position.Y, Edited.Position.Z,
        Edited.VisibilityDistance, Edited.CullingDistance));

    const FString ResultXml = FPaths::Combine(OutputDirectory, TEXT("port-result.xml"));
    IFileManager::Get().Delete(*ResultXml, false, true, true);
    uint32 ProcessId = 0;
    const FString Arguments = FString::Printf(
        TEXT("--gtest_filter=RunningPortExport.ValidatesEditedBank --gtest_output=xml:%s"), *ResultXml);
    FProcHandle Process = FPlatformProcess::CreateProc(*PortTest, *Arguments, true, false, false, &ProcessId, 0,
        *FPaths::GetPath(PortTest), nullptr, nullptr);
    if (!TestTrue(TEXT("Start the port validation test"), Process.IsValid()))
    {
        Service.Reset();
        return false;
    }
    int32 ReturnCode = -1;
    FPlatformProcess::WaitForProc(Process);
    FPlatformProcess::GetProcReturnCode(Process, &ReturnCode);
    FPlatformProcess::CloseProc(Process);

    // Exit code alone is not enough: a stale binary that lacks the test would also
    // exit zero. Require XML evidence that exactly this test ran and passed.
    FString Xml;
    const bool bXmlRead = FFileHelper::LoadFileToString(Xml, *ResultXml);
    const bool bRanAndPassed = bXmlRead &&
        Xml.Contains(TEXT("tests=\"1\"")) && Xml.Contains(TEXT("failures=\"0\"")) &&
        Xml.Contains(TEXT("errors=\"0\"")) && Xml.Contains(TEXT("skipped=\"0\"")) &&
        Xml.Contains(TEXT("name=\"RunningPortExport\""));
    if (ReturnCode != 0 || !bRanAndPassed)
    {
        AddError(FString::Printf(TEXT("The running port did not validate the exported bank (exit %d, xml=%s). Bank: %s"),
            ReturnCode, bRanAndPassed ? TEXT("ok") : TEXT("missing/invalid"), *OutputFilename));
    }
    else
    {
        AddInfo(FString::Printf(TEXT("Running port loaded the exported bank: %s (actor %d chunk %lld, native create)"),
            *OutputFilename, PortActorIndex, ActorOffsets[PortActorIndex].Chunk));
    }
    TestTrue(TEXT("Running port executes and passes the exported-edit test"), bRanAndPassed);
    TestEqual(TEXT("Running port validates the exported edit"), ReturnCode, 0);
    Service.Reset();
    return ReturnCode == 0 && bRanAndPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaStructuralEditGateTest, "Kya.Editor.StructuralEditGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaStructuralEditGateTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage(), 6);
    Bank->SceneEnvironment.Actors[0].ClassId = 0x2c;
    Bank->SceneEnvironment.Actors[0].TrapActorRef = 2;
    FKyaActorPlacement& Third = Bank->SceneEnvironment.Actors.AddDefaulted_GetRef();
    Third.Name = TEXT("THIRD");
    Third.ClassId = 6;
    FKyaActorPlacement& Fourth = Bank->SceneEnvironment.Actors.AddDefaulted_GetRef();
    Fourth.Name = TEXT("FOURTH");
    Fourth.ClassId = 6;
    FKyaBankEntry& Preserved = Bank->Entries.AddDefaulted_GetRef();
    Preserved.Type = 5;
    Preserved.Subtype = 1;

    // Removal analysis: index map, allocation delta, dangling references and
    // preserved entries are computed even though the writer remains disabled.
    FKyaStructuralEditAnalysis Delete;
    TestTrue(TEXT("Delete analysis runs"), AnalyzeKyaStructuralEdit(*Bank,
        EKyaStructuralEditKind::DeleteActor, 2, Delete));
    TestFalse(TEXT("Delete remains blocked until relocation exists"), Delete.bWriterSupported);
    TestEqual(TEXT("Delete remaps higher actors"), Delete.ActorIndexRemap.FindRef(3), 2);
    TestEqual(TEXT("Delete marks the removed actor"), Delete.ActorIndexRemap.FindRef(2), -1);
    TestEqual(TEXT("Delete decrements the class allocation"),
        Delete.ClassAllocationDelta.FindRef(Bank->SceneEnvironment.Actors[2].ClassId), -1);
    TestEqual(TEXT("Delete counts references to the removed actor"),
        Delete.ReferencesToRemovedActor, 1);
    TestEqual(TEXT("Delete preserves unrelated bank entries"), Delete.PreservedBankEntries, 1);
    TestTrue(TEXT("Delete explains the rebuild requirement"), Delete.BlockingReasons.Num() > 0 &&
        Delete.BlockingReasons[0].Contains(TEXT("actor stream")));

    FKyaStructuralEditAnalysis Duplicate;
    TestTrue(TEXT("Duplicate analysis runs"), AnalyzeKyaStructuralEdit(*Bank,
        EKyaStructuralEditKind::DuplicateActor, 1, Duplicate));
    TestFalse(TEXT("Duplicate remains blocked"), Duplicate.bWriterSupported);
    TestEqual(TEXT("Duplicate increments the class allocation"),
        Duplicate.ClassAllocationDelta.FindRef(6), 1);
    TestEqual(TEXT("Duplicate preserves existing indices"), Duplicate.ActorIndexRemap.FindRef(1), 1);

    // In-place edits reject layout changes before mutating the bank.
    FKyaActorPlacement Candidate = Bank->SceneEnvironment.Actors[0];
    FString Error;
    Candidate.ClassId = 0x0d;
    TestFalse(TEXT("Class replacement is rejected"),
        ValidateKyaActorStructuralEdit(Bank->SceneEnvironment.Actors[0], Candidate, Error));
    TestTrue(TEXT("Class rejection explains the milestone"), Error.Contains(TEXT("Class replacement")));

    Candidate = Bank->SceneEnvironment.Actors[0];
    Candidate.Name = TEXT("FIRST123");
    TestFalse(TEXT("Variable-length name is rejected"),
        ValidateKyaActorStructuralEdit(Bank->SceneEnvironment.Actors[0], Candidate, Error));
    TestTrue(TEXT("Name rejection explains relocation"), Error.Contains(TEXT("relocat")));

    Candidate = Bank->SceneEnvironment.Actors[0];
    Candidate.Behaviours.AddDefaulted();
    TestFalse(TEXT("Array resizing is rejected"),
        ValidateKyaActorStructuralEdit(Bank->SceneEnvironment.Actors[0], Candidate, Error));
    TestTrue(TEXT("Array rejection explains resizing"), Error.Contains(TEXT("Resizing")));

    // Nested arrays are validated recursively, including struct arrays inside the
    // class-specific records.
    Bank->SceneEnvironment.Actors[0].Checkpoints.AddDefaulted();
    Candidate = Bank->SceneEnvironment.Actors[0];
    Candidate.Checkpoints[0].ActorWaypoints.AddDefaulted();
    TestFalse(TEXT("Nested array resizing is rejected"),
        ValidateKyaActorStructuralEdit(Bank->SceneEnvironment.Actors[0], Candidate, Error));
    TestTrue(TEXT("Nested rejection names the nested field"), Error.Contains(TEXT("ActorWaypoints")));

    // The edit service refuses a cross-slot rename and leaves the bank unchanged.
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    FKyaActorEditPatch Rename;
    Rename.Name = TEXT("FIRST123");
    TestFalse(TEXT("Service rejects a cross-slot rename"),
        Service.Apply(Service.MakeKey(Bank, 0), Rename, EKyaEditNotify::Silent, &Error));
    TestEqual(TEXT("Rejected rename leaves the bank untouched"),
        Bank->SceneEnvironment.Actors[0].Name, FString(TEXT("FIRST")));
    Service.Reset();

    // Bulk edits must use the same structural gate as single-record edits.
    FKyaActorEditPatch BulkResize;
    BulkResize.Custom = [](FKyaActorPlacement& Placement, FString& OutError)
    {
        Placement.Behaviours.AddDefaulted();
        return true;
    };
    FKyaLevelEditService& BulkService = FKyaLevelEditService::Get();
    BulkService.Reset();
    const int32 BehavioursBeforeBulk = Bank->SceneEnvironment.Actors[0].Behaviours.Num();
    TestFalse(TEXT("Bulk edit cannot resize a fixed array"),
        BulkService.ApplyBulk({ BulkService.MakeKey(Bank, 0) }, BulkResize, EKyaEditNotify::Silent, &Error));
    TestEqual(TEXT("Rejected bulk resize leaves the bank untouched"),
        Bank->SceneEnvironment.Actors[0].Behaviours.Num(), BehavioursBeforeBulk);
    BulkService.Reset();

    // Writer preflight rejects actor-count changes before any output is staged.
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (!Filename.IsEmpty())
    {
        TArray<uint8> Input;
        if (TestTrue(TEXT("Read structural bank"), FFileHelper::LoadFileToArray(Input, *Filename)))
        {
            const uint8* Cursor = Input.GetData();
            UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
            UKyaLevelBank* RealBank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
                UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
                TEXT("bnk"), Cursor, Input.GetData() + Input.Num(), GWarn));
            if (TestNotNull(TEXT("Import structural bank"), RealBank) &&
                RealBank->SceneEnvironment.Actors.Num() > 0)
            {
                FString SupportReason;
                TestTrue(TEXT("The real bank supports structural actor edits"),
                    KyaBankSupportsStructuralEdit(*RealBank, SupportReason));
                FKyaStructuralEditAnalysis RealDelete;
                TestTrue(TEXT("Real-bank delete analysis runs"), AnalyzeKyaStructuralEdit(*RealBank,
                    EKyaStructuralEditKind::DeleteActor, 0, RealDelete));
                TestTrue(TEXT("Real-bank delete reports writer support"), RealDelete.bWriterSupported);

                UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
                AKyaLevelPreview* Preview = World ? World->SpawnActor<AKyaLevelPreview>() : nullptr;
                if (TestNotNull(TEXT("Create the structural command preview"), Preview))
                {
                    Preview->Bank = RealBank;
                    Preview->bShowRebuildProgress = false;
                    Preview->bShowBackgroundMeshes = false;
                    Preview->bSpawnActorHandles = true;
                    Preview->bShowActorMeshesOnHandles = false;
                    Preview->OnConstruction(FTransform::Identity);
                    const int32 Before = RealBank->SceneEnvironment.Actors.Num();
                    if (GEditor)
                    {
                        GEditor->SelectNone(false, true, false);
                        TestFalse(TEXT("Structural actions are unavailable without a selected record"), KyaCanEditSelectedActor());
                        GEditor->SelectActor(Preview->GetActorHandle(0), true, true);
                        TestTrue(TEXT("One owned record enables structural actions"), KyaCanEditSelectedActor());
                        GEditor->SelectActor(Preview, true, true);
                        TestFalse(TEXT("Mixed selection disables structural actions"), KyaCanEditSelectedActor());
                        GEditor->SelectActor(Preview, false, true);
                    }
                    const FKyaStructuralActionResult Created = KyaDuplicateSelectedActor();
                    TestTrue(TEXT("The duplicate command succeeds"), Created.bSuccess);
                    if (!Created.bSuccess) AddInfo(Created.Error);
                    TestEqual(TEXT("The duplicate command appends a record"),
                        RealBank->SceneEnvironment.Actors.Num(), Before + 1);
                    if (Created.bSuccess)
                    {
                        const FKyaStructuralActionResult Deleted = KyaDeleteSelectedActor();
                        TestTrue(TEXT("The delete command succeeds"), Deleted.bSuccess);
                        if (!Deleted.bSuccess) AddInfo(Deleted.Error);
                        TestEqual(TEXT("The delete command restores the actor count"),
                            RealBank->SceneEnvironment.Actors.Num(), Before);
                    }
                    Preview->Destroy();
                }
                if (World) World->DestroyWorld(false);

                FKyaActorPlacement Copy = RealBank->SceneEnvironment.Actors[0];
                Copy.Name = TEXT("DUPLICATE");
                RealBank->SceneEnvironment.Actors.Add(Copy);
                FKyaBankExportDiagnostics Diagnostics;
                TArray<uint8> Bytes;
                TestFalse(TEXT("Writer rejects an unapplied actor-count change"),
                    KyaBuildBankExportBytes(RealBank, Bytes, Diagnostics));
                TestTrue(TEXT("Writer rejection points at the structural commands"),
                    Diagnostics.Errors.ContainsByPredicate([](const FString& Message)
                    {
                        return Message.Contains(TEXT("actor count")) && Message.Contains(TEXT("structural"));
                    }));
                RealBank->SceneEnvironment.Actors.RemoveAt(RealBank->SceneEnvironment.Actors.Num() - 1);
            }
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaStructuralWriterTest, "Kya.Editor.StructuralWriter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaStructuralWriterTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the structural writer test."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read the structural writer bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import the structural writer bank"), Bank)) return false;
    if (!TestTrue(TEXT("The structural writer bank has actors"), Bank->SceneEnvironment.Actors.Num() > 0)) return false;

    FString Reason;
    if (!TestTrue(TEXT("The bank supports structural edits"), KyaBankSupportsStructuralEdit(*Bank, Reason)))
    {
        AddInfo(Reason);
        return false;
    }
    const int32 OriginalCount = Bank->SceneEnvironment.Actors.Num();
    const FKyaActorPlacement OriginalFirst = Bank->SceneEnvironment.Actors[0];

    // Direct remap coverage for switch/platform camera targets and inactive fields.
    {
        TMap<int32, int32> Remap;
        Remap.Add(5, 4);
        FKyaActorPlacement Placement;
        Placement.bHasSwitchData = true;
        Placement.SwitchTarget.Targets.AddDefaulted();
        Placement.SwitchTarget.Targets[0].TargetRef = 5;
        Placement.bHasPlatformDetails = true;
        Placement.PlatformCameras.AddDefaulted();
        Placement.PlatformCameras[0].TargetRef = 5;
        Placement.ActorFlags = 5;
        TArray<FString> Dangling;
        TestTrue(TEXT("Typed reference remap succeeds"), KyaRemapActorReferences(Placement, Remap, &Dangling));
        TestEqual(TEXT("Switch-target references are remapped"),
            Placement.SwitchTarget.Targets[0].TargetRef, 4);
        TestEqual(TEXT("Platform-camera targets are remapped"), Placement.PlatformCameras[0].TargetRef, 4);
        TestEqual(TEXT("Non-reference actor flags are untouched"), Placement.ActorFlags, 5);

        FKyaActorPlacement Inactive;
        TMap<int32, int32> RemoveFirst;
        RemoveFirst.Add(0, -1);
        TArray<FString> InactiveDangling;
        TestTrue(TEXT("Inactive class defaults are ignored"),
            KyaRemapActorReferences(Inactive, RemoveFirst, &InactiveDangling));
        TestTrue(TEXT("Inactive defaults do not create false dangling references"),
            InactiveDangling.IsEmpty());
    }

    // --- append a duplicate and verify the serialized result ----------------------
    const FKyaStructuralWriteResult Duplicate = KyaDuplicateActorRecord(*Bank, 0);
    if (!TestTrue(TEXT("Duplicating an actor succeeds"), Duplicate.bSuccess))
    {
        AddInfo(Duplicate.Error);
        return false;
    }
    TestEqual(TEXT("Duplicate appends one record"), Bank->SceneEnvironment.Actors.Num(), OriginalCount + 1);
    TestEqual(TEXT("Duplicate reports its new index"), Duplicate.ActorIndex, OriginalCount);
    TestTrue(TEXT("Duplicate keeps the source class"),
        Bank->SceneEnvironment.Actors[Duplicate.ActorIndex].ClassId == OriginalFirst.ClassId);
    TestTrue(TEXT("Duplicate gets a distinct name"),
        Bank->SceneEnvironment.Actors[Duplicate.ActorIndex].Name != OriginalFirst.Name);
    TestEqual(TEXT("Duplicate gets the new serialized index"),
        Bank->SceneEnvironment.Actors[Duplicate.ActorIndex].Index, Duplicate.ActorIndex);

    auto Export = [&](TArray<uint8>& OutBytes) -> UKyaLevelBank*
    {
        FKyaBankExportDiagnostics Diagnostics;
        OutBytes.Reset();
        if (!KyaBuildBankExportBytes(Bank, OutBytes, Diagnostics))
        {
            for (const FString& Message : Diagnostics.Errors) AddInfo(Message);
            return nullptr;
        }
        const uint8* OutCursor = OutBytes.GetData();
        return Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), OutCursor,
            OutBytes.GetData() + OutBytes.Num(), GWarn));
    };

    TArray<uint8> Exported;
    UKyaLevelBank* Reimported = Export(Exported);
    if (TestNotNull(TEXT("Exporting after duplicate succeeds"), Reimported))
    {
        TestEqual(TEXT("Reimport keeps the appended actor count"),
            Reimported->SceneEnvironment.Actors.Num(), OriginalCount + 1);
        TestTrue(TEXT("Reimport keeps the duplicate name"),
            Reimported->SceneEnvironment.Actors[Duplicate.ActorIndex].Name ==
                Bank->SceneEnvironment.Actors[Duplicate.ActorIndex].Name);
        TestTrue(TEXT("Reimport keeps the duplicate position"),
            Reimported->SceneEnvironment.Actors[Duplicate.ActorIndex].Position.Equals(OriginalFirst.Position, 1.0e-4));
        TestEqual(TEXT("Reimport keeps the source actor count"),
            Reimported->SceneEnvironment.Actors.Num() - 1, OriginalCount);
    }

    // --- typed reference remap on a middle delete --------------------------------
    TArray<FKyaReferenceField> ActorRefFields;
    for (const FKyaReferenceField& Field : GetKyaReferenceFields())
        if (Field.Kind == EKyaReferenceKind::Actor) ActorRefFields.Add(Field);

    int32 RefActorIndex = INDEX_NONE;
    FKyaReferenceField RefField;
    for (int32 Index = 0; Index < Bank->SceneEnvironment.Actors.Num() && RefActorIndex == INDEX_NONE; ++Index)
    {
        const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[Index];
        for (const FKyaReferenceField& Field : ActorRefFields)
        {
            if (Field.ClassId != -1 && Field.ClassId != Placement.ClassId) continue;
            if (GetKyaNumericField(Placement, Field.PropertyName) < 0) continue;
            RefActorIndex = Index;
            RefField = Field;
            break;
        }
    }

    if (TestTrue(TEXT("A typed actor reference is available"), RefActorIndex != INDEX_NONE))
    {
        const int32 FirstDuplicate = Duplicate.ActorIndex;
        const FKyaStructuralWriteResult Second = KyaDuplicateActorRecord(*Bank, RefActorIndex);
        if (TestTrue(TEXT("A second duplicate succeeds"), Second.bSuccess))
        {
            const int32 RemappedTarget = Second.ActorIndex;
            const FString LastDuplicateName = Bank->SceneEnvironment.Actors[RemappedTarget].Name;
            FKyaActorPlacement ReferenceSource = Bank->SceneEnvironment.Actors[RefActorIndex];
            TestTrue(TEXT("The test can point a typed reference at the last duplicate"),
                SetKyaNumericField(ReferenceSource, RefField.PropertyName, RemappedTarget));
            Bank->SceneEnvironment.Actors[RefActorIndex] = ReferenceSource;
            TestEqual(TEXT("The typed reference starts at the last duplicate"),
                GetKyaNumericField(Bank->SceneEnvironment.Actors[RefActorIndex], RefField.PropertyName), int64(RemappedTarget));

            const FKyaStructuralWriteResult DeleteMiddle = KyaDeleteActorRecord(*Bank, FirstDuplicate);
            if (TestTrue(TEXT("Deleting a middle actor succeeds"), DeleteMiddle.bSuccess))
            {
                TestEqual(TEXT("Delete removes one record"), Bank->SceneEnvironment.Actors.Num(), OriginalCount + 1);
                TestEqual(TEXT("Typed references shift with the deleted actor"),
                    GetKyaNumericField(Bank->SceneEnvironment.Actors[RefActorIndex], RefField.PropertyName),
                    int64(RemappedTarget - 1));
                TestEqual(TEXT("The shifted record is the last duplicate"),
                    Bank->SceneEnvironment.Actors[RemappedTarget - 1].Name, LastDuplicateName);
                TestEqual(TEXT("All serialized indices are contiguous"),
                    Bank->SceneEnvironment.Actors[RemappedTarget - 1].Index, RemappedTarget - 1);
                TArray<uint8> RemapBytes;
                UKyaLevelBank* RemapReimport = Export(RemapBytes);
                if (TestNotNull(TEXT("Exporting after the remapped delete succeeds"), RemapReimport))
                {
                    TestEqual(TEXT("Reimport keeps the remapped reference"),
                        GetKyaNumericField(RemapReimport->SceneEnvironment.Actors[RefActorIndex],
                            RefField.PropertyName), int64(RemappedTarget - 1));
                    TestEqual(TEXT("Reimport keeps the post-delete actor count"),
                        RemapReimport->SceneEnvironment.Actors.Num(), OriginalCount + 1);
                }
            }
            else
            {
                AddInfo(DeleteMiddle.Error);
            }
        }
        else
        {
            AddInfo(Second.Error);
        }
    }

    // --- delete the appended last record -----------------------------------------
    if (RefActorIndex != INDEX_NONE)
    {
        FKyaActorPlacement ReferenceSource = Bank->SceneEnvironment.Actors[RefActorIndex];
        SetKyaNumericField(ReferenceSource, RefField.PropertyName, 0);
        Bank->SceneEnvironment.Actors[RefActorIndex] = ReferenceSource;
    }
    const FKyaStructuralWriteResult DeleteLast = KyaDeleteActorRecord(*Bank, Bank->SceneEnvironment.Actors.Num() - 1);
    if (TestTrue(TEXT("Deleting the last actor succeeds"), DeleteLast.bSuccess))
    {
        TestEqual(TEXT("Delete restores the original actor count"),
            Bank->SceneEnvironment.Actors.Num(), OriginalCount);
        TestTrue(TEXT("Delete keeps the first record unchanged"),
            Bank->SceneEnvironment.Actors[0].Position.Equals(OriginalFirst.Position, 1.0e-4) &&
            Bank->SceneEnvironment.Actors[0].Name == OriginalFirst.Name);
        TArray<uint8> FinalBytes;
        UKyaLevelBank* FinalReimport = Export(FinalBytes);
        if (TestNotNull(TEXT("Exporting after the final delete succeeds"), FinalReimport))
            TestEqual(TEXT("Reimport keeps the final actor count"),
                FinalReimport->SceneEnvironment.Actors.Num(), OriginalCount);
    }
    else
    {
        AddInfo(DeleteLast.Error);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaViewportClickSelectionTest, "Kya.Editor.ViewportClickSelection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaViewportClickSelectionTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create click world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn click preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = true;
    Preview->bShowActorMeshesOnHandles = false;
    Preview->OnConstruction(FTransform::Identity);

    AKyaActorHandle* First = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
    AKyaActorHandle* Second = Cast<AKyaActorHandle>(Preview->GetActorHandle(1));
    if (!TestNotNull(TEXT("First click handle exists"), First) ||
        !TestNotNull(TEXT("Second click handle exists"), Second))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }

    // Two material strips per actor keep batching intact; the pick must not depend on
    // a broad proxy volume.
    const auto MakeQuad = [](const FVector& Center, float HalfSize)
    {
        FKyaActorMeshSection Section;
        Section.Positions = {
            Center + FVector(-HalfSize, -HalfSize, 0.0),
            Center + FVector(HalfSize, -HalfSize, 0.0),
            Center + FVector(HalfSize, HalfSize, 0.0),
            Center + FVector(-HalfSize, HalfSize, 0.0) };
        Section.Triangles = { 0, 1, 2, 0, 2, 3 };
        return Section;
    };
    First->SetActorMeshSections({
        MakeQuad(FVector(0.0, 0.0, 0.0), 25.0), MakeQuad(FVector(200.0, 0.0, 0.0), 25.0) });
    Second->SetActorMeshSections({ MakeQuad(FVector(0.0, 0.0, 0.0), 25.0) });
    TestEqual(TEXT("The first actor keeps two material sections"),
        First->GetRenderedSectionCount(), 2);

    const auto LocalToWorld = [](const AKyaActorHandle* Handle, const FVector& Local)
    {
        return Handle->ActorMesh->GetComponentTransform().TransformPosition(Local);
    };
    const auto CastDownAt = [Preview](const FVector& WorldTarget)
    {
        return Preview->HitTestActorMesh(WorldTarget + FVector(0.0, 0.0, 1000.0), FVector(0.0, 0.0, -1.0));
    };

    // --- any strip of the actor resolves to the owning record ---------------------
    TestEqual(TEXT("Clicking the first strip selects the actor"),
        CastDownAt(LocalToWorld(First, FVector(0.0, 0.0, 0.0))), First);
    const FString FirstLabel = First->GetActorLabel();
    TestEqual(TEXT("Clicking another strip selects the same actor"),
        CastDownAt(LocalToWorld(First, FVector(200.0, 0.0, 0.0))), First);
    TestEqual(TEXT("Clicking a second actor selects that actor"),
        CastDownAt(LocalToWorld(Second, FVector(0.0, 0.0, 0.0))), Second);
    TestNull(TEXT("A ray that misses every actor selects nothing"),
        Preview->HitTestActorMesh(FVector(0.0, 0.0, 1.0e6), FVector(0.0, 0.0, 1.0)));

    // --- nearest visible strip wins ------------------------------------------------
    {
        const FVector Target = LocalToWorld(First, FVector(200.0, 0.0, 0.0)) + FVector(0.0, 0.0, -200.0);
        Second->SetActorMeshSections({ MakeQuad(
            Second->ActorMesh->GetComponentTransform().InverseTransformPosition(Target), 25.0) });
        TestEqual(TEXT("The nearest strip wins over an occluded actor"),
            CastDownAt(LocalToWorld(First, FVector(200.0, 0.0, 0.0))), First);
    }

    // --- selection highlights the full mesh and gives Focus Selected bounds --------
    Preview->SetGeneratedActorHighlighted(First, true);
    TestTrue(TEXT("The picked actor is highlighted"), First->IsMeshHighlighted());
    TestFalse(TEXT("Other actors are not highlighted"), Second->IsMeshHighlighted());
    const FBox FocusBounds = First->GetRenderedMeshBounds();
    TestTrue(TEXT("Focus bounds cover the first strip"),
        FocusBounds.IsInsideOrOn(LocalToWorld(First, FVector(0.0, 0.0, 0.0))));
    TestTrue(TEXT("Focus bounds cover the second strip"),
        FocusBounds.IsInsideOrOn(LocalToWorld(First, FVector(200.0, 0.0, 0.0))));
    TestFalse(TEXT("Focus bounds exclude the other actor"),
        FocusBounds.IsInsideOrOn(LocalToWorld(Second, FVector(0.0, 0.0, 0.0))));

    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        const FVector ClickOrigin = LocalToWorld(First, FVector(200.0, 0.0, 0.0)) + FVector(0.0, 0.0, 1000.0);
        AKyaActorHandle* Picked = KyaSelectRenderedActorAt(Preview, ClickOrigin, FVector(0.0, 0.0, -1.0));
        TestEqual(TEXT("The click path selects the hit actor"), Picked, First);
        TestTrue(TEXT("The clicked actor is selected so Details follows"), First->IsSelected());
        TestTrue(TEXT("The click path highlights every material strip"), First->IsMeshHighlighted());
        TestEqual(TEXT("Highlighting keeps the material batching"),
            First->GetRenderedSectionCount(), 2);
        TestFalse(TEXT("Clicking clears the previous highlight"), Second->IsMeshHighlighted());
    }
    TestEqual(TEXT("Clicking does not change the actor label"), First->GetActorLabel(), FirstLabel);

    // --- nearest hit across previews (the cursor path uses this) -------------------
    {
        AKyaLevelPreview* SecondPreview = World->SpawnActor<AKyaLevelPreview>();
        if (TestNotNull(TEXT("Second preview spawns"), SecondPreview))
        {
            UKyaLevelBank* SecondBank = MakeKyaTestBank(GetTransientPackage());
            SecondPreview->Bank = SecondBank;
            SecondPreview->bShowRebuildProgress = false;
            SecondPreview->bSpawnActorHandles = true;
            SecondPreview->OnConstruction(FTransform::Identity);
            AKyaActorHandle* Third = Cast<AKyaActorHandle>(SecondPreview->GetActorHandle(0));
            if (TestNotNull(TEXT("Second preview handle exists"), Third))
            {
                const FVector RayStart = LocalToWorld(First, FVector(200.0, 0.0, 0.0)) + FVector(0.0, 0.0, 1000.0);
                const FVector NearPoint = RayStart + FVector(0.0, 0.0, -500.0);
                Third->SetActorMeshSections({ MakeQuad(
                    Third->ActorMesh->GetComponentTransform().InverseTransformPosition(NearPoint), 25.0) });
                AKyaLevelPreview* Owner = nullptr;
                TestEqual(TEXT("The nearest actor across previews wins"),
                    KyaHitTestRenderedActorAcrossPreviews({ Preview, SecondPreview }, RayStart,
                        FVector(0.0, 0.0, -1.0), &Owner), Third);
                TestEqual(TEXT("The owning preview is reported"), Owner, SecondPreview);

                const FVector FarPoint = RayStart + FVector(0.0, 0.0, -1400.0);
                Third->SetActorMeshSections({ MakeQuad(
                    Third->ActorMesh->GetComponentTransform().InverseTransformPosition(FarPoint), 25.0) });
                Owner = nullptr;
                TestEqual(TEXT("A farther preview does not occlude the nearer actor"),
                    KyaHitTestRenderedActorAcrossPreviews({ Preview, SecondPreview }, RayStart,
                        FVector(0.0, 0.0, -1.0), &Owner), First);
                TestEqual(TEXT("The owning preview follows the nearest hit"), Owner, Preview);
            }
            SecondPreview->Destroy();
        }
    }

    // Plain editor selection (viewport or Outliner) drives the same full-mesh highlight.
    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        GEditor->NoteSelectionChange();
        TestFalse(TEXT("Clearing the selection clears the highlight"), First->IsMeshHighlighted());
        GEditor->SelectActor(First, true, true);
        TestTrue(TEXT("Selection changes highlight the full mesh"), First->IsMeshHighlighted());
        TestEqual(TEXT("Selection highlights every material section"),
            First->GetRenderedSectionCount(), 2);
        GEditor->SelectNone(false, true, false);
        GEditor->NoteSelectionChange();
    }

    // --- locked representations are not pickable -----------------------------------
    First->bSelectionLocked = true;
    TestNull(TEXT("Locked actor geometry cannot be picked"),
        CastDownAt(LocalToWorld(First, FVector(0.0, 0.0, 0.0))));
    First->bSelectionLocked = false;

    // --- hidden or isolated geometry does not win the pick -------------------------
    Preview->SetGeneratedActorTemporarilyHidden(First, true);
    TestNull(TEXT("Temporarily hidden geometry cannot be picked"),
        CastDownAt(LocalToWorld(First, FVector(0.0, 0.0, 0.0))));
    Preview->SetGeneratedActorTemporarilyHidden(First, false);

    Preview->IsolateGeneratedActors({ Second });
    TestNull(TEXT("An isolated-out actor cannot be picked"),
        CastDownAt(LocalToWorld(First, FVector(0.0, 0.0, 0.0))));
    Preview->ClearGeneratedActorIsolation();

    if (First->ActorMesh)
    {
        First->ActorMesh->SetMeshSectionVisible(0, false);
        TestNull(TEXT("A hidden material strip cannot be picked"),
            CastDownAt(LocalToWorld(First, FVector(0.0, 0.0, 0.0))));
        TestEqual(TEXT("The remaining visible strip still picks the actor"),
            CastDownAt(LocalToWorld(First, FVector(200.0, 0.0, 0.0))), First);
        First->ActorMesh->SetMeshSectionVisible(0, true);
    }

    // The interactive entry point degrades gracefully when no level viewport exists.
    FString CursorError;
    TestNull(TEXT("Cursor picking without a level viewport selects nothing"),
        KyaSelectRenderedActorAtCursor(&CursorError));
    TestTrue(TEXT("Cursor picking explains why it failed"), !CursorError.IsEmpty());

    First->ClearActorMesh();
    Second->ClearActorMesh();
    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSharedEditPathTest, "Kya.Editor.SharedEditPath",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSharedEditPathTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the shared edit-path acceptance."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read shared edit-path bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import shared edit-path bank"), Bank)) return false;
    if (!TestTrue(TEXT("The shared bank has actors and paths"),
        Bank->SceneEnvironment.Actors.Num() > 1 && !Bank->SceneEnvironment.Paths.IsEmpty() &&
        Bank->SceneEnvironment.Paths[0].Points.Num() > 1)) return false;

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    if (UPackage* Package = Bank->GetOutermost()) Package->SetDirtyFlag(false);

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create shared edit-path world"), World)) return false;
    AKyaLevelPreview* FirstPreview = World->SpawnActor<AKyaLevelPreview>();
    AKyaLevelPreview* SecondPreview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("First preview spawns"), FirstPreview) ||
        !TestNotNull(TEXT("Second preview spawns"), SecondPreview))
    {
        World->DestroyWorld(false);
        return false;
    }
    for (AKyaLevelPreview* Preview : { FirstPreview, SecondPreview })
    {
        Preview->Bank = Bank;
        Preview->bShowRebuildProgress = false;
        Preview->bSpawnActorHandles = true;
        Preview->bSpawnWaypointHandles = true;
        Preview->bSpawnPathPointHandles = true;
    }
    FirstPreview->OnConstruction(FTransform::Identity);
    SecondPreview->OnConstruction(FTransform::Identity);
    if (UPackage* Package = Bank->GetOutermost())
        TestFalse(TEXT("Building previews does not dirty the bank"), Package->IsDirty());

    AKyaActorHandle* FirstHandle = Cast<AKyaActorHandle>(FirstPreview->GetActorHandle(0));
    AKyaActorHandle* SecondHandle = Cast<AKyaActorHandle>(SecondPreview->GetActorHandle(0));
    if (!TestNotNull(TEXT("First preview handle exists"), FirstHandle) ||
        !TestNotNull(TEXT("Second preview handle exists"), SecondHandle))
    {
        FirstPreview->Destroy();
        SecondPreview->Destroy();
        World->DestroyWorld(false);
        return false;
    }

    // --- Entry point 1: viewport/gizmo transform (the transaction path) -------------
    const FVector ActorBefore = Bank->SceneEnvironment.Actors[0].Position;
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "SharedEditMove", "Shared edit move"));
        FirstHandle->AddActorWorldOffset(FVector(150.0, 0.0, 225.0));
        FirstHandle->PostEditMove(true);
        Service.FlushPendingNotifications();
    }
    TestFalse(TEXT("Viewport move writes the authoritative record"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(ActorBefore));
    TestTrue(TEXT("The second preview follows the viewport edit"),
        SecondHandle->GetActorTransform().GetLocation().Equals(
            FirstHandle->GetActorTransform().GetLocation(), 0.01));
    TestTrue(TEXT("The second preview mirror matches the bank"),
        SecondHandle->ActorPosition.Equals(Bank->SceneEnvironment.Actors[0].Position, 1.0e-4));

    // --- Entry point 2: Details property patch -------------------------------------
    FKyaActorPlacement DetailsCopy = Bank->SceneEnvironment.Actors[0];
    DetailsCopy.VisibilityDistance = 4321.0f;
    FKyaActorEditPatch DetailsPatch;
    FString Error;
    if (TestTrue(TEXT("Details patch builds"),
        KyaMakePlacementPropertyPatch(DetailsCopy,
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, VisibilityDistance), DetailsPatch, &Error)))
    {
        TestTrue(TEXT("Details edit applies through the shared service"),
            Service.Apply(Service.MakeKey(Bank, 0), DetailsPatch, EKyaEditNotify::Notify, &Error));
        Service.FlushPendingNotifications();
        TestEqual(TEXT("Details edit writes the authoritative record"),
            Bank->SceneEnvironment.Actors[0].VisibilityDistance, 4321.0f);
        TestEqual(TEXT("First preview mirror matches Details"),
            FirstHandle->ActorVisibilityDistance, 4321.0f);
        TestEqual(TEXT("Second preview mirror matches Details"),
            SecondHandle->ActorVisibilityDistance, 4321.0f);
    }

    // --- Entry point 3: scripted ApplyToBank ---------------------------------------
    SecondHandle->ActorLightingValue = 1234.0f;
    SecondHandle->ApplyToBank();
    Service.FlushPendingNotifications();
    TestEqual(TEXT("Scripted edit writes the authoritative record"),
        Bank->SceneEnvironment.Actors[0].LightingValue, 1234.0f);
    TestEqual(TEXT("First preview mirror matches the scripted edit"),
        FirstHandle->ActorLightingValue, 1234.0f);

    // --- Entry point 4: legacy path-point marker component --------------------------
    FirstPreview->PreviewPathIndex = 0;
    FirstPreview->OnConstruction(FTransform::Identity);
    if (UKyaPathPointComponent* Marker = FirstPreview->GetPathMarker(1))
    {
        const FVector4 PointBefore = Bank->SceneEnvironment.Paths[0].Points[1];
        const int32 PointCount = Bank->SceneEnvironment.Paths[0].Points.Num();
        Marker->SetRelativeLocation(FVector(200.0, 300.0, 400.0));
        Marker->PostEditComponentMove(true);
        Service.FlushPendingNotifications();
        TestFalse(TEXT("Marker edit writes the authoritative record"),
            Bank->SceneEnvironment.Paths[0].Points[1].Equals(PointBefore));
        TestEqual(TEXT("Marker edit keeps the point record count"),
            Bank->SceneEnvironment.Paths[0].Points.Num(), PointCount);
    }
    else
    {
        AddWarning(TEXT("No path-point marker was created for path 0; skipping the marker entry point."));
    }

    // --- Entry point 5: bulk edit over a multi-selection ----------------------------
    FKyaActorEditPatch BulkPatch;
    BulkPatch.VisibilityDistance = 777.0f;
    {
        TArray<FKyaActorRecordKey> Keys;
        Keys.Add(Service.MakeKey(Bank, 0));
        Keys.Add(Service.MakeKey(Bank, 1));
        TestTrue(TEXT("Bulk edit applies through the shared service"),
            Service.ApplyBulk(Keys, BulkPatch, EKyaEditNotify::Notify, &Error));
        Service.FlushPendingNotifications();
    }
    TestEqual(TEXT("Bulk edit writes actor 0"), Bank->SceneEnvironment.Actors[0].VisibilityDistance, 777.0f);
    TestEqual(TEXT("Bulk edit writes actor 1"), Bank->SceneEnvironment.Actors[1].VisibilityDistance, 777.0f);
    TestEqual(TEXT("First preview agrees on the bulk edit"),
        FirstHandle->ActorVisibilityDistance, 777.0f);
    if (AKyaActorHandle* FirstBulk = Cast<AKyaActorHandle>(FirstPreview->GetActorHandle(1)))
        TestEqual(TEXT("First preview agrees on the second bulk record"),
            FirstBulk->ActorVisibilityDistance, 777.0f);
    if (AKyaActorHandle* SecondBulk = Cast<AKyaActorHandle>(SecondPreview->GetActorHandle(1)))
        TestEqual(TEXT("Second preview agrees on the bulk edit"),
            SecondBulk->ActorVisibilityDistance, 777.0f);

    // --- Direct service/script edits are undoable and no-ops stay clean ------------
    Service.FlushPendingNotifications();
    if (UPackage* Package = Bank->GetOutermost()) Package->SetDirtyFlag(false);
    {
        const FKyaActorPlacement Same = Bank->SceneEnvironment.Actors[0];
        FKyaActorEditPatch NoOpPatch;
        TestTrue(TEXT("Identical Details patch builds"),
            KyaMakePlacementPropertyPatch(Same,
                GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, VisibilityDistance), NoOpPatch, &Error));
        TestTrue(TEXT("Identical Details patch is accepted"),
            Service.Apply(Service.MakeKey(Bank, 0), NoOpPatch, EKyaEditNotify::Notify, &Error));
        Service.FlushPendingNotifications();
        if (UPackage* Package = Bank->GetOutermost())
            TestFalse(TEXT("An identical edit does not dirty the bank"), Package->IsDirty());
        if (AKyaActorHandle* SameHandle = Cast<AKyaActorHandle>(FirstPreview->GetActorHandle(0)))
        {
            SameHandle->SyncFromBank();
            SameHandle->ApplyToBank();
            Service.FlushPendingNotifications();
            if (UPackage* Package = Bank->GetOutermost())
                TestFalse(TEXT("Re-applying a synced record does not dirty the bank"), Package->IsDirty());
        }
    }

    const float VisibilityBeforeDirect = Bank->SceneEnvironment.Actors[0].VisibilityDistance;
    FKyaActorEditPatch DirectPatch;
    DirectPatch.VisibilityDistance = 2222.0f;
    TestTrue(TEXT("Direct service edit applies outside a transaction"),
        Service.Apply(Service.MakeKey(Bank, 0), DirectPatch, EKyaEditNotify::Notify, &Error));
    Service.FlushPendingNotifications();
    TestEqual(TEXT("Direct service edit writes the record"),
        Bank->SceneEnvironment.Actors[0].VisibilityDistance, 2222.0f);
    if (GEditor)
    {
        TestTrue(TEXT("A direct service edit produces an undo step"), GEditor->UndoTransaction());
        TestEqual(TEXT("Undo restores the direct edit"),
            Bank->SceneEnvironment.Actors[0].VisibilityDistance, VisibilityBeforeDirect);
        TestTrue(TEXT("Redo reapplies the direct edit"), GEditor->RedoTransaction());
        TestEqual(TEXT("Redo restores the direct edit value"),
            Bank->SceneEnvironment.Actors[0].VisibilityDistance, 2222.0f);
    }

    const float BulkUndoBefore0 = Bank->SceneEnvironment.Actors[0].VisibilityDistance;
    const float BulkUndoBefore1 = Bank->SceneEnvironment.Actors[1].VisibilityDistance;
    FKyaActorEditPatch BulkUndoPatch;
    BulkUndoPatch.VisibilityDistance = 3333.0f;
    {
        TArray<FKyaActorRecordKey> Keys;
        Keys.Add(Service.MakeKey(Bank, 0));
        Keys.Add(Service.MakeKey(Bank, 1));
        TestTrue(TEXT("Bulk edit applies outside a transaction"),
            Service.ApplyBulk(Keys, BulkUndoPatch, EKyaEditNotify::Notify, &Error));
        Service.FlushPendingNotifications();
    }
    if (GEditor)
    {
        TestTrue(TEXT("A bulk edit produces one undo step"), GEditor->UndoTransaction());
        TestEqual(TEXT("Bulk undo restores the first record"),
            Bank->SceneEnvironment.Actors[0].VisibilityDistance, BulkUndoBefore0);
        TestEqual(TEXT("Bulk undo restores the second record"),
            Bank->SceneEnvironment.Actors[1].VisibilityDistance, BulkUndoBefore1);
        TestTrue(TEXT("Bulk redo reapplies both records"), GEditor->RedoTransaction());
        TestEqual(TEXT("Bulk redo restores the first record"),
            Bank->SceneEnvironment.Actors[0].VisibilityDistance, 3333.0f);
        TestEqual(TEXT("Bulk redo restores the second record"),
            Bank->SceneEnvironment.Actors[1].VisibilityDistance, 3333.0f);
    }

    // --- Spatial entry points: undoable direct edits, clean re-applies ---------------
    if (Bank->SceneEnvironment.Waypoints.Num() > 0)
    {
        const FKyaWaypointRecordKey WaypointKey = Service.MakeWaypointKey(Bank, 0);
        if (UPackage* Package = Bank->GetOutermost()) Package->SetDirtyFlag(false);
        FKyaWaypointEditPatch SameWaypoint;
        SameWaypoint.Position = Bank->SceneEnvironment.Waypoints[0].Position;
        SameWaypoint.Rotation = Bank->SceneEnvironment.Waypoints[0].Rotation;
        SameWaypoint.Parameters = Bank->SceneEnvironment.Waypoints[0].Parameters;
        TestTrue(TEXT("Identical waypoint patch is accepted"),
            Service.ApplyWaypoint(WaypointKey, SameWaypoint, EKyaEditNotify::Notify, &Error));
        Service.FlushPendingNotifications();
        if (UPackage* Package = Bank->GetOutermost())
            TestFalse(TEXT("An identical waypoint edit does not dirty the bank"), Package->IsDirty());

        if (AKyaSpatialRecordHandle* WaypointHandle =
            Cast<AKyaSpatialRecordHandle>(FirstPreview->GetWaypointHandle(0)))
        {
            if (UPackage* Package = Bank->GetOutermost()) Package->SetDirtyFlag(false);
            WaypointHandle->SyncFromBank();
            WaypointHandle->ApplyToBank();
            Service.FlushPendingNotifications();
            if (UPackage* Package = Bank->GetOutermost())
                TestFalse(TEXT("Re-applying a synced waypoint does not dirty the bank"), Package->IsDirty());
        }

        const FVector WaypointBeforeEdit = Bank->SceneEnvironment.Waypoints[0].Position;
        FKyaWaypointEditPatch WaypointEdit;
        WaypointEdit.Position = WaypointBeforeEdit + FVector(1.0, 2.0, 3.0);
        TestTrue(TEXT("Direct waypoint edit applies outside a transaction"),
            Service.ApplyWaypoint(WaypointKey, WaypointEdit, EKyaEditNotify::Notify, &Error));
        Service.FlushPendingNotifications();
        if (GEditor)
        {
            TestTrue(TEXT("A direct waypoint edit produces an undo step"), GEditor->UndoTransaction());
            TestTrue(TEXT("Waypoint undo restores the record"),
                Bank->SceneEnvironment.Waypoints[0].Position.Equals(WaypointBeforeEdit));
            TestTrue(TEXT("Waypoint redo reapplies the record"), GEditor->RedoTransaction());
            TestTrue(TEXT("Waypoint redo restores the edit"),
                Bank->SceneEnvironment.Waypoints[0].Position.Equals(WaypointBeforeEdit + FVector(1.0, 2.0, 3.0)));
        }
    }

    // --- Rebuilds neither duplicate representations nor dirty the bank -------------
    const int32 FirstHandleCount = FirstPreview->GetActorHandleCount();
    const int32 SecondHandleCount = SecondPreview->GetActorHandleCount();
    const int32 SpatialCount = FirstPreview->GetSpatialHandleCount();
    if (UPackage* Package = Bank->GetOutermost()) Package->SetDirtyFlag(false);
    FirstPreview->OnConstruction(FTransform::Identity);
    SecondPreview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Rebuilding keeps one handle per record"),
        FirstPreview->GetActorHandleCount(), Bank->SceneEnvironment.Actors.Num());
    TestEqual(TEXT("Rebuilding does not duplicate the first preview"),
        FirstPreview->GetActorHandleCount(), FirstHandleCount);
    TestEqual(TEXT("Rebuilding does not duplicate the second preview"),
        SecondPreview->GetActorHandleCount(), SecondHandleCount);
    TestEqual(TEXT("Rebuilding does not duplicate spatial handles"),
        FirstPreview->GetSpatialHandleCount(), SpatialCount);
    if (UPackage* Package = Bank->GetOutermost())
        TestFalse(TEXT("Rebuilding does not dirty an unchanged bank"), Package->IsDirty());

    FirstPreview->Destroy();
    SecondPreview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMilestoneDGateTest, "Kya.Editor.MilestoneDGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMilestoneDGateTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the Milestone D gate."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read Milestone D bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Milestone D bank"), Bank)) return false;
    if (!TestTrue(TEXT("Milestone D bank has actors"), Bank->SceneEnvironment.Actors.Num() > 0)) return false;
    if (!TestTrue(TEXT("Milestone D bank has waypoints and paths"),
        Bank->SceneEnvironment.Waypoints.Num() > 0 && !Bank->SceneEnvironment.Paths.IsEmpty() &&
        !Bank->SceneEnvironment.Paths[0].Points.IsEmpty())) return false;

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create Milestone D world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn Milestone D preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = true;
    Preview->bSpawnWaypointHandles = true;
    Preview->bSpawnPathPointHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    // --- 1: discovery and selection from the Outliner label, not a numeric index ---
    const FString TargetName = Bank->SceneEnvironment.Actors[0].Name;
    AKyaActorHandle* Found = nullptr;
    for (TActorIterator<AKyaActorHandle> It(World); It; ++It)
    {
        if (!It->GetActorLabel().StartsWith(TargetName + TEXT(" [class"))) continue;
        Found = *It;
        break;
    }
    if (!TestNotNull(TEXT("Actor is discoverable by its Outliner label"), Found))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }
    TestTrue(TEXT("The label identifies the serialized record"), Found->ActorName == TargetName);
    TestFalse(TEXT("No capture camera is required"), Preview->HasExplicitPreviewCamera());
    TestEqual(TEXT("No capture actor index is required"), Preview->PreviewActorIndex, -1);
    TestTrue(TEXT("Authoring discovery is active by default"), Preview->IsAuthoringVisibilityActive());
    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        GEditor->SelectActor(Found, true, true);
    }
    TestTrue(TEXT("Outliner selection selects the representation"), Found->IsSelected());

    // --- 2: organization commands and unsupported-command diagnostics --------------
    Preview->SetGeneratedActorTemporarilyHidden(Found, true);
    TestTrue(TEXT("Hide works from the session"), Found->IsTemporarilyHiddenInEditor());
    Preview->SetGeneratedActorTemporarilyHidden(Found, false);
    Preview->IsolateGeneratedActors({Found});
    TestTrue(TEXT("Isolate works from the session"), Preview->IsGeneratedActorIsolated(Found));
    Preview->ClearGeneratedActorIsolation();
    TestFalse(TEXT("Clearing isolation restores the session"), Found->IsTemporarilyHiddenInEditor());
    Preview->bLockGeneratedActorSelection = true;
    TestFalse(TEXT("Selection lock prevents picking"), Found->IsSelectable());
    Preview->bLockGeneratedActorSelection = false;
    FText DeleteReason;
    TestFalse(TEXT("Generated actors cannot be structurally deleted"), Found->CanDeleteSelectedActor(DeleteReason));
    TestTrue(TEXT("Delete refusal points at the bank record"),
        DeleteReason.ToString().Contains(TEXT("bank")));

    // --- 3: spatial records are identifiable and editable without indices ----------
    // Find the spatial representations the way a user would: by their labels in the
    // Outliner, without supplying a record index.
    AKyaSpatialRecordHandle* Waypoint = nullptr;
    AKyaSpatialRecordHandle* PathPoint = nullptr;
    for (TActorIterator<AKyaSpatialRecordHandle> It(World); It; ++It)
    {
        if (It->Kind == EKyaSpatialRecordKind::Waypoint && !Waypoint) Waypoint = *It;
        if (It->Kind == EKyaSpatialRecordKind::PathPoint && !PathPoint) PathPoint = *It;
        if (Waypoint && PathPoint) break;
    }
    const int32 WaypointIndex = Waypoint ? Waypoint->RecordIndex : INDEX_NONE;
    const int32 PathIndex = PathPoint ? PathPoint->RecordIndex : INDEX_NONE;
    const int32 PathPointIndex = PathPoint ? PathPoint->PointIndex : INDEX_NONE;
    if (!TestNotNull(TEXT("Waypoint handle exists"), Waypoint) ||
        !TestNotNull(TEXT("Path point handle exists"), PathPoint))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }
    TestTrue(TEXT("Waypoint label carries its position"), Waypoint->GetActorLabel().Contains(TEXT("(")));
    TestTrue(TEXT("Path point label carries its position"), PathPoint->GetActorLabel().Contains(TEXT("(")));

    const int64 BuildsBefore = Preview->BuildCount;
    const int32 DecodesBefore = Preview->GetGeometryDecodeCount();
    const FString WaypointLabelBefore = Waypoint->GetActorLabel();
    const FVector WaypointBefore = Bank->SceneEnvironment.Waypoints[WaypointIndex].Position;
    Waypoint->AddActorWorldOffset(FVector(150.0, 0.0, 0.0));
    Waypoint->PostEditMove(true);
    Service.FlushPendingNotifications();
    TestFalse(TEXT("Waypoint drag writes the bank without an index"),
        Bank->SceneEnvironment.Waypoints[WaypointIndex].Position.Equals(WaypointBefore));
    TestFalse(TEXT("Waypoint label refreshes after the edit"),
        Waypoint->GetActorLabel() == WaypointLabelBefore);

    const FVector4 PathBefore = Bank->SceneEnvironment.Paths[PathIndex].Points[PathPointIndex];
    PathPoint->AddActorWorldOffset(FVector(0.0, 150.0, 0.0));
    PathPoint->PostEditMove(true);
    Service.FlushPendingNotifications();
    TestFalse(TEXT("Path point drag writes the bank without an index"),
        Bank->SceneEnvironment.Paths[PathIndex].Points[PathPointIndex].Equals(PathBefore));

    // --- 4: actor transform editing is incremental (no rebuild, no re-decode) ------
    const FVector ActorBefore = Bank->SceneEnvironment.Actors[Found->ActorIndex].Position;
    Found->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
    Found->PostEditMove(true);
    Service.FlushPendingNotifications();
    TestFalse(TEXT("Actor drag writes the bank"), 
        Bank->SceneEnvironment.Actors[Found->ActorIndex].Position.Equals(ActorBefore));
    TestEqual(TEXT("The editing session never rebuilds the level"), Preview->BuildCount, BuildsBefore);
    TestEqual(TEXT("The editing session never re-decodes level geometry"),
        Preview->GetGeometryDecodeCount(), DecodesBefore);

    // --- 5: diagnostics name the record without searching raw logs -----------------
    const int32 SavedMesh = Bank->SceneEnvironment.Actors[Found->ActorIndex].MeshIndex;
    Bank->SceneEnvironment.Actors[Found->ActorIndex].MeshIndex = 99999;
    const FKyaBankValidationReport Report = KyaValidateBank(Bank);
    bool bNamedIssue = false;
    for (const FKyaValidationIssue& Issue : Report.Issues)
    {
        if (Issue.ActorIndex != Found->ActorIndex || Issue.Field != TEXT("MeshIndex")) continue;
        bNamedIssue = Issue.bBlocking && !Issue.Message.IsEmpty();
        const FString Text = KyaMakeValidationIssueMessage(EMessageSeverity::Error, Bank, Issue.ActorIndex,
            Issue.Field, FText::FromString(Issue.Message))->ToText().ToString();
        TestTrue(TEXT("The diagnostic names the actor and property"),
            Text.Contains(TargetName) && Text.Contains(TEXT("MeshIndex")));
    }
    TestTrue(TEXT("Blocking diagnostics identify the affected actor"), bNamedIssue);
    Bank->SceneEnvironment.Actors[Found->ActorIndex].MeshIndex = SavedMesh;

    // Drop the transient selection before the preview is destroyed so typed-element
    // selection state does not outlive the actors it references.
    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        GEditor->NoteSelectionChange();
    }
    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMilestoneCGateTest, "Kya.Editor.MilestoneCGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMilestoneCGateTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the Milestone C gate."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read Milestone C bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Milestone C bank"), Bank)) return false;
    if (!TestTrue(TEXT("Milestone C bank has actors"), Bank->SceneEnvironment.Actors.Num() > 0)) return false;

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();

    // Accessibility audit: every export-supported property must have a working
    // Details edit path, and every non-editable property must explain why.
    int32 EditableCount = 0;
    int32 InspectCount = 0;
    int32 PresentationCount = 0;
    bool bAuditOk = true;
    FString AuditError;
    TMap<FName, int32> FieldActors;
    TMap<FName, FString> FieldTouchedPaths;
    TMap<FName, FKyaActorPlacement> FieldBaselines;
    TArray<FString> UncoveredFields;
    TArray<FString> InapplicableFields;

    // A class-specific field is only serialized when its presence record is set, so
    // only records whose presence flags satisfy the schema's EditCondition can carry a
    // meaningful edit. Unresolvable conditions are treated as applicable.
    const auto ConditionHolds = [](const UScriptStruct* Struct, const void* Container,
        const FString& Condition) -> bool
    {
        // Evaluate simple presence-flag conditions ("bHasX", "!bHasX",
        // "bHasX && bHasY", "bHasX || bHasY"). Anything else is assumed applicable.
        TArray<FString> Groups;
        Condition.ParseIntoArray(Groups, TEXT("||"), true);
        bool bFoundFlag = false;
        for (FString Group : Groups)
        {
            TArray<FString> Terms;
            Group.ParseIntoArray(Terms, TEXT("&&"), true);
            bool bGroupTrue = true;
            bool bGroupUsable = false;
            for (FString Term : Terms)
            {
                Term.TrimStartAndEndInline();
                bool bNegated = false;
                if (Term.StartsWith(TEXT("!")))
                {
                    bNegated = true;
                    Term = Term.Mid(1).TrimStartAndEnd();
                }
                const FBoolProperty* Flag = CastField<FBoolProperty>(
                    Struct->FindPropertyByName(FName(*Term)));
                if (!Flag)
                {
                    bGroupTrue = false;
                    continue;
                }
                bGroupUsable = true;
                bFoundFlag = true;
                const bool bValue = Flag->GetPropertyValue_InContainer(Container);
                if (bValue == bNegated) bGroupTrue = false;
            }
            if (bGroupUsable && bGroupTrue) return true;
        }
        return !bFoundFlag;
    };
    const auto IsFieldApplicable = [&ConditionHolds](const FKyaActorPlacement& Record,
        const FProperty* Property) -> bool
    {
        const FString* Condition = Property->FindMetaData(TEXT("EditCondition"));
        if (!Condition || Condition->IsEmpty()) return true;
        return ConditionHolds(FKyaActorPlacement::StaticStruct(), &Record, *Condition);
    };

    // Nudge one serializable leaf so each field is applied with a distinct value.
    int32 LeafSkipRemaining = 0;
    TFunction<bool(FProperty*, void*, FString&)> MutateValue =
        [&MutateValue, &LeafSkipRemaining, &ConditionHolds](FProperty* Property, void* Value, FString& OutPath) -> bool
    {
        OutPath = Property->GetName();
        if (!CastField<FStructProperty>(Property) && !CastField<FArrayProperty>(Property) && LeafSkipRemaining > 0)
        {
            --LeafSkipRemaining;
            return false;
        }
        if (FIntProperty* Int = CastField<FIntProperty>(Property))
        {
            const int32 Current = Int->GetPropertyValue(Value);
            Int->SetPropertyValue(Value, Current == 0 ? 1 : 0);
            return true;
        }
        if (FInt64Property* Int = CastField<FInt64Property>(Property))
        {
            const int64 Current = Int->GetPropertyValue(Value);
            Int->SetPropertyValue(Value, Current == 0 ? 1 : 0);
            return true;
        }
        if (FUInt32Property* Int = CastField<FUInt32Property>(Property))
        {
            const uint32 Current = Int->GetPropertyValue(Value);
            Int->SetPropertyValue(Value, Current == 0 ? 1u : 0u);
            return true;
        }
        if (FUInt64Property* Int = CastField<FUInt64Property>(Property))
        {
            const uint64 Current = Int->GetPropertyValue(Value);
            Int->SetPropertyValue(Value, Current == 0 ? 1ull : 0ull);
            return true;
        }
        if (FFloatProperty* Float = CastField<FFloatProperty>(Property))
        {
            // Integral values so fields the writer encodes as integers still round trip.
            Float->SetPropertyValue(Value, Float->GetPropertyValue(Value) == 1.0f ? 2.0f : 1.0f);
            return true;
        }
        if (FDoubleProperty* Float = CastField<FDoubleProperty>(Property))
        {
            Float->SetPropertyValue(Value, Float->GetPropertyValue(Value) == 1.0 ? 2.0 : 1.0);
            return true;
        }
        if (FBoolProperty* Bool = CastField<FBoolProperty>(Property))
        {
            Bool->SetPropertyValue(Value, !Bool->GetPropertyValue(Value));
            return true;
        }
        if (FByteProperty* Byte = CastField<FByteProperty>(Property))
        {
            Byte->SetPropertyValue(Value, Byte->GetPropertyValue(Value) == 0 ? 1 : 0);
            return true;
        }
        if (FStrProperty* String = CastField<FStrProperty>(Property))
        {
            FString Text = String->GetPropertyValue(Value);
            // An empty string cannot carry a distinct value in this bank.
            if (Text.IsEmpty()) return false;
            Text[0] = Text[0] == TEXT('A') ? TEXT('B') : TEXT('A');
            String->SetPropertyValue(Value, Text);
            return true;
        }
        if (FStructProperty* Struct = CastField<FStructProperty>(Property))
        {
            for (TFieldIterator<FProperty> It(Struct->Struct); It; ++It)
            {
                if (It->GetName().StartsWith(TEXT("bHas"))) continue;
                // Id fields select the payload layout; changing one is not a
                // layout-compatible edit (matching the Details layout gate).
                if (It->GetName() == TEXT("Id")) continue;
                if (!It->HasAnyPropertyFlags(CPF_Edit) || It->HasAnyPropertyFlags(CPF_EditConst)) continue;
                const FString* Condition = It->FindMetaData(TEXT("EditCondition"));
                if (Condition && !Condition->IsEmpty() &&
                    !ConditionHolds(Struct->Struct, Value, *Condition)) continue;
                FString NestedPath;
                if (MutateValue(*It, It->ContainerPtrToValuePtr<void>(Value), NestedPath))
                {
                    OutPath = FString::Printf(TEXT("%s.%s"), *OutPath, *NestedPath);
                    return true;
                }
            }
            return false;
        }
        if (FArrayProperty* Array = CastField<FArrayProperty>(Property))
        {
            FScriptArrayHelper Helper(Array, Value);
            if (Helper.Num() == 0)
            {
                if (LeafSkipRemaining > 0) --LeafSkipRemaining;
                return false;
            }
            FString NestedPath;
            if (!MutateValue(Array->Inner, Helper.GetRawPtr(0), NestedPath)) return false;
            OutPath = FString::Printf(TEXT("%s[0].%s"), *OutPath, *NestedPath);
            return true;
        }
        return false;
    };

    // This audit is one scripted bulk operation: participate in a single undo step so
    // the per-field applies do not create hundreds of undo snapshots.
    TUniquePtr<FScopedTransaction> AuditTransaction;
    if (GEditor && !GIsTransacting)
        AuditTransaction = MakeUnique<FScopedTransaction>(
            NSLOCTEXT("KyaTests", "MilestoneCAudit", "Audit Kya actor fields"));
    for (TFieldIterator<FProperty> It(FKyaActorPlacement::StaticStruct()); It; ++It)
    {
        const FName PropertyName = It->GetFName();
        const FKyaFieldCapability Capability = GetKyaActorFieldCapability(PropertyName);
        if (Capability.Kind == EKyaFieldCapability::Unsupported)
        {
            bAuditOk = false;
            AuditError = FString::Printf(TEXT("%s is unsupported: %s"), *PropertyName.ToString(), *Capability.Reason);
            break;
        }
        if (Capability.Kind != EKyaFieldCapability::EditableExportable)
        {
            if (Capability.Reason.IsEmpty())
            {
                bAuditOk = false;
                AuditError = FString::Printf(TEXT("%s has no capability reason."), *PropertyName.ToString());
                break;
            }
            if (Capability.Kind == EKyaFieldCapability::InspectOnly) ++InspectCount;
            else ++PresentationCount;
            continue;
        }

        // Apply a distinct value to a suitable record. Class-gated fields are tried
        // against every actor until one with the required optional record accepts it.
        bool bApplied = false;
        bool bAnyApplicable = false;
        bool bHasDistinctValue = false;
        FString LastError;
        for (int32 ActorIndex = 0; ActorIndex < Bank->SceneEnvironment.Actors.Num() && !bApplied; ++ActorIndex)
        {
            if (!IsFieldApplicable(Bank->SceneEnvironment.Actors[ActorIndex], *It)) continue;
            bAnyApplicable = true;
            const FKyaActorPlacement BaselineRecord = Bank->SceneEnvironment.Actors[ActorIndex];
            FKyaActorPlacement EditedRecord = BaselineRecord;
            void* Value = It->ContainerPtrToValuePtr<void>(&EditedRecord);
            FString TouchedPath;
            if (!MutateValue(*It, Value, TouchedPath)) continue;
            // A distinct candidate exists; a rejection or a no-op commit from here on
            // is an uncovered field, not an inapplicable one.
            bHasDistinctValue = true;
            FKyaActorEditPatch FieldPatch;
            FString PatchError;
            if (!KyaMakePlacementPropertyPatch(EditedRecord, PropertyName, FieldPatch, &PatchError))
            {
                LastError = PatchError;
                continue;
            }
            if (Service.Apply(Service.MakeKey(Bank, ActorIndex), FieldPatch, EKyaEditNotify::Silent, &PatchError))
            {
                // The committed record must actually differ, otherwise the field is
                // not proven editable just because the patch was accepted.
                if (It->Identical_InContainer(&BaselineRecord, &Bank->SceneEnvironment.Actors[ActorIndex]))
                {
                    LastError = TEXT("the committed value is unchanged");
                    continue;
                }
                bApplied = true;
                FieldActors.Add(PropertyName, ActorIndex);
                FieldTouchedPaths.Add(PropertyName, TouchedPath);
                FieldBaselines.Add(PropertyName, BaselineRecord);
            }
            else
            {
                LastError = PatchError;
            }
        }
        if (bApplied) ++EditableCount;
        else if (!bAnyApplicable) InapplicableFields.Add(FString::Printf(TEXT("%s: no actor of a class carrying this record"),
            *PropertyName.ToString()));
        else if (!bHasDistinctValue) InapplicableFields.Add(FString::Printf(
            TEXT("%s: no distinct serialized value to edit in this bank"), *PropertyName.ToString()));
        else UncoveredFields.Add(FString::Printf(TEXT("%s: %s"), *PropertyName.ToString(), *LastError));
    }
    AuditTransaction.Reset();
    if (!bAuditOk) AddError(AuditError);
    for (const FString& Uncovered : UncoveredFields) AddInfo(FString::Printf(TEXT("Uncovered field: %s"), *Uncovered));
    for (const FString& Skipped : InapplicableFields) AddInfo(FString::Printf(TEXT("Not applicable: %s"), *Skipped));
    TestTrue(TEXT("Every export-supported field is reachable through Details"), bAuditOk);
    TestTrue(TEXT("Every applicable export-supported field accepts an edit"), UncoveredFields.Num() == 0);
    TestTrue(TEXT("The audit finds editable export-supported fields"), EditableCount > 0);
    TestTrue(TEXT("Non-editable fields are classified with reasons"), InspectCount + PresentationCount > 0);

    // Details accessibility: the panel's own row filter must include every editable
    // field, so a supported property cannot be missing from the selected actor's UI.
    const TArray<FName> DetailsProperties = KyaGetActorDetailsProperties();
    TArray<FString> MissingDetailsRows;
    for (TFieldIterator<FProperty> It(FKyaActorPlacement::StaticStruct()); It; ++It)
    {
        const EKyaFieldCapability Kind = GetKyaActorFieldCapability(It->GetFName()).Kind;
        if (Kind != EKyaFieldCapability::EditableExportable &&
            Kind != EKyaFieldCapability::EditablePresentationOnly) continue;
        if (!DetailsProperties.Contains(It->GetFName())) MissingDetailsRows.Add(It->GetName());
    }
    for (const FString& Missing : MissingDetailsRows)
        AddInfo(FString::Printf(TEXT("Missing Details row: %s"), *Missing));
    TestEqual(TEXT("Details exposes a row for every editable actor field"), MissingDetailsRows.Num(), 0);

    // A behaviour's raw payload array is a snapshot the writer regenerates from the
    // typed fields, so compare typed content only.
    const auto FieldMatches = [](FProperty* Property, const FKyaActorPlacement& A,
        const FKyaActorPlacement& B) -> bool
    {
        if (!Property) return false;
        if (Property->GetFName() != GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Behaviours))
            return Property->Identical_InContainer(&A, &B);
        const FArrayProperty* Array = CastField<FArrayProperty>(Property);
        const FStructProperty* Inner = Array ? CastField<FStructProperty>(Array->Inner) : nullptr;
        if (!Array || !Inner) return Property->Identical_InContainer(&A, &B);
        FScriptArrayHelper HelperA(Array, Array->ContainerPtrToValuePtr<void>(&A));
        FScriptArrayHelper HelperB(Array, Array->ContainerPtrToValuePtr<void>(&B));
        if (HelperA.Num() != HelperB.Num()) return false;
        for (int32 Index = 0; Index < HelperA.Num(); ++Index)
            for (TFieldIterator<FProperty> It(Inner->Struct); It; ++It)
            {
                if (It->GetName() == TEXT("Data")) continue;
                if (!It->Identical_InContainer(HelperA.GetRawPtr(Index), HelperB.GetRawPtr(Index)))
                    return false;
            }
        return true;
    };

    // Persistence: export once and compare every edited field for its actor.
    TArray<FName> MismatchedFields;
    const auto ExportAndCompareFields = [&](TArray<FName>& OutMismatched) -> bool
    {
        OutMismatched.Reset();
        FKyaBankExportDiagnostics Diagnostics;
        TArray<uint8> Exported;
        if (!KyaBuildBankExportBytes(Bank, Exported, Diagnostics))
        {
            for (const FString& Message : Diagnostics.Errors)
                AddInfo(FString::Printf(TEXT("Export error: %s"), *Message));
            return false;
        }
        const uint8* ExportedCursor = Exported.GetData();
        UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), ExportedCursor,
            Exported.GetData() + Exported.Num(), GWarn));
        if (!Reimported) return false;
        for (const TPair<FName, int32>& Pair : FieldActors)
        {
            FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(Pair.Key);
            const FKyaActorPlacement& Source = Bank->SceneEnvironment.Actors[Pair.Value];
            const FKyaActorPlacement& Dest = Reimported->SceneEnvironment.Actors[Pair.Value];
            if (!FieldMatches(Property, Source, Dest))
                OutMismatched.Add(Pair.Key);
        }
        return true;
    };

    bool bFieldExportOk = ExportAndCompareFields(MismatchedFields);
    TestTrue(TEXT("Export Milestone C bank"), bFieldExportOk);

    // Some nested leaves only exist for one serialized variant (for example a
    // platform behaviour's stand record). Try the next layout-compatible leaf until
    // the field accepts an edit that the writer actually emits.
    for (int32 Attempt = 0; bFieldExportOk && !MismatchedFields.IsEmpty() && Attempt < 512; ++Attempt)
    {
        TUniquePtr<FScopedTransaction> RetryTransaction;
        if (GEditor && !GIsTransacting)
            RetryTransaction = MakeUnique<FScopedTransaction>(
                NSLOCTEXT("KyaTests", "MilestoneCRetry", "Retry Kya actor field edits"));
        for (const FName Field : MismatchedFields)
        {
            const int32 ActorIndex = FieldActors.FindRef(Field);
            FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(Field);
            if (!Property || !FieldBaselines.Contains(Field) ||
                !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex)) continue;
            FKyaActorPlacement EditedRecord = FieldBaselines.FindRef(Field);
            LeafSkipRemaining = Attempt + 1;
            FString Path;
            if (!MutateValue(Property, Property->ContainerPtrToValuePtr<void>(&EditedRecord), Path))
                continue;
            FKyaActorEditPatch Patch;
            FString PatchError;
            if (!KyaMakePlacementPropertyPatch(EditedRecord, Field, Patch, &PatchError)) continue;
            if (!Service.Apply(Service.MakeKey(Bank, ActorIndex), Patch, EKyaEditNotify::Silent, &PatchError))
                continue;
            FieldTouchedPaths.Add(Field, Path);
        }
        RetryTransaction.Reset();
        LeafSkipRemaining = 0;
        bFieldExportOk = ExportAndCompareFields(MismatchedFields);
    }
    LeafSkipRemaining = 0;
    for (const FName Field : MismatchedFields)
        AddInfo(FString::Printf(TEXT("Field did not persist: %s (touched %s)"),
            *Field.ToString(), *FieldTouchedPaths.FindRef(Field)));
    TestTrue(TEXT("Milestone C export succeeds after the per-field edits"), bFieldExportOk);
    TestEqual(TEXT("Every edited export-supported field persists"), MismatchedFields.Num(), 0);
    // Nested edit end-to-end: change a model name within its fixed slot.
    int32 ModelActor = INDEX_NONE;
    for (int32 Index = 0; Index < Bank->SceneEnvironment.Actors.Num(); ++Index)
        if (!Bank->SceneEnvironment.Actors[Index].Models.IsEmpty())
        {
            ModelActor = Index;
            break;
        }
    if (TestTrue(TEXT("A real actor has model references"), ModelActor != INDEX_NONE))
    {
        FKyaActorPlacement ModelsCopy = Bank->SceneEnvironment.Actors[ModelActor];
        FString& MeshName = ModelsCopy.Models[0].MeshName;
        if (!MeshName.IsEmpty()) MeshName[MeshName.Len() - 1] = MeshName[MeshName.Len() - 1] == TEXT('A') ? TEXT('B') : TEXT('A');
        FKyaActorEditPatch ModelsPatch;
        FString Error;
        TestTrue(TEXT("Nested model-name patch builds"),
            KyaMakePlacementPropertyPatch(ModelsCopy, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Models),
                ModelsPatch, &Error));
        TestTrue(TEXT("Nested model-name edit applies"),
            Service.Apply(Service.MakeKey(Bank, ModelActor), ModelsPatch, EKyaEditNotify::Silent, &Error));
    }

    // Reference-picker end-to-end on a class-gated actor reference.
    int32 ReferenceActor = INDEX_NONE;
    for (int32 Index = 0; Index < Bank->SceneEnvironment.Actors.Num(); ++Index)
        if (Bank->SceneEnvironment.Actors[Index].ClassId == 0x2c)
        {
            ReferenceActor = Index;
            break;
        }
    if (!TestTrue(TEXT("The real bank has a TrapActorRef actor"), ReferenceActor != INDEX_NONE))
    {
        Service.Reset();
        return false;
    }
    if (!TestTrue(TEXT("The real bank can supply a distinct reference target"),
        Bank->SceneEnvironment.Actors.Num() > 1))
    {
        Service.Reset();
        return false;
    }
    const int32 CurrentReference = Bank->SceneEnvironment.Actors[ReferenceActor].TrapActorRef;
    const int32 TargetReference = CurrentReference == 0 ? 1 : 0;
    FKyaActorPlacement ReferenceCopy = Bank->SceneEnvironment.Actors[ReferenceActor];
    // Commit through the same helper the Details picker uses, including its
    // out-of-range rejection.
    TestTrue(TEXT("Reference picker commits a valid target"),
        KyaSetActorReferencePick(*Bank, ReferenceCopy,
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TrapActorRef), TargetReference));
    TestEqual(TEXT("Reference picker sets the chosen id"), ReferenceCopy.TrapActorRef, TargetReference);
    FKyaActorPlacement RejectedPick = ReferenceCopy;
    TestFalse(TEXT("Reference picker rejects an out-of-range id"),
        KyaSetActorReferencePick(*Bank, RejectedPick,
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TrapActorRef),
            Bank->SceneEnvironment.Actors.Num()));
    TestEqual(TEXT("Rejected pick leaves the value untouched"),
        RejectedPick.TrapActorRef, TargetReference);
    FKyaActorEditPatch ReferencePatch;
    FString ReferenceError;
    TestTrue(TEXT("Reference picker patch builds"),
        KyaMakePlacementPropertyPatch(ReferenceCopy,
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TrapActorRef), ReferencePatch, &ReferenceError));
    TestTrue(TEXT("Reference picker edit applies"),
        Service.Apply(Service.MakeKey(Bank, ReferenceActor), ReferencePatch,
            EKyaEditNotify::Silent, &ReferenceError));
    TestEqual(TEXT("Reference picker commits a distinct value"),
        Bank->SceneEnvironment.Actors[ReferenceActor].TrapActorRef, TargetReference);

    // Multi-edit end-to-end: the same patch applies to several selected actors.
    const int32 FirstActor = 0;
    const int32 SecondActor = 1;
    FKyaActorEditPatch MultiPatch;
    MultiPatch.VisibilityDistance = 1234.0f;
    if (TestTrue(TEXT("Milestone C has two actors"), Bank->SceneEnvironment.Actors.Num() > 1))
    {
        TArray<FKyaActorRecordKey> Keys;
        Keys.Add(Service.MakeKey(Bank, FirstActor));
        Keys.Add(Service.MakeKey(Bank, SecondActor));
        FString Error;
        TestTrue(TEXT("Multi-edit applies to the selected actors"),
            Service.ApplyBulk(Keys, MultiPatch, EKyaEditNotify::Silent, &Error));
        TestEqual(TEXT("Multi-edit writes actor 0"), Bank->SceneEnvironment.Actors[FirstActor].VisibilityDistance, 1234.0f);
        TestEqual(TEXT("Multi-edit writes actor 1"), Bank->SceneEnvironment.Actors[SecondActor].VisibilityDistance, 1234.0f);
    }

    // Persistence: export and reimport, then compare the nested, reference and
    // multi-edit values on the reimported typed schema.
    FKyaBankExportDiagnostics Diagnostics2;
    TArray<uint8> Exported2;
    if (TestTrue(TEXT("Export Milestone C bank"), KyaBuildBankExportBytes(Bank, Exported2, Diagnostics2)))
    {
        const uint8* ExportedCursor = Exported2.GetData();
        UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), ExportedCursor,
            Exported2.GetData() + Exported2.Num(), GWarn));
        if (TestNotNull(TEXT("Reimport Milestone C bank"), Reimported))
        {
            if (ModelActor != INDEX_NONE)
                TestEqual(TEXT("Nested model name persists"),
                    Reimported->SceneEnvironment.Actors[ModelActor].Models[0].MeshName,
                    Bank->SceneEnvironment.Actors[ModelActor].Models[0].MeshName);
            TestEqual(TEXT("Reference picker value persists"),
                Reimported->SceneEnvironment.Actors[ReferenceActor].TrapActorRef, TargetReference);
            TestEqual(TEXT("Multi-edit actor 0 persists"),
                Reimported->SceneEnvironment.Actors[FirstActor].VisibilityDistance, 1234.0f);
            if (Bank->SceneEnvironment.Actors.Num() > 1)
                TestEqual(TEXT("Multi-edit actor 1 persists"),
                    Reimported->SceneEnvironment.Actors[SecondActor].VisibilityDistance, 1234.0f);
        }
    }

    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMilestoneBGateTest, "Kya.Editor.MilestoneBGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMilestoneBGateTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the Milestone B gate."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read Milestone B bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* MainBank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Milestone B main bank"), MainBank)) return false;
    // A unique object name gives discovery a unique destination folder, so this gate
    // always exercises fresh imports and never touches another run's project assets.
    MainBank->Rename(*FString::Printf(TEXT("KyaMilestoneB_%s"),
        *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    // Copy the sector fixtures into a unique source directory so discovery cannot
    // reuse or reimport another run's project assets.
    const FString SourceRoot = FPaths::Combine(FPaths::ProjectSavedDir(), FString::Printf(
        TEXT("KyaMilestoneBSource_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    IFileManager::Get().MakeDirectory(*SourceRoot, true);
    TArray<FString> SectorFiles;
    IFileManager::Get().FindFiles(SectorFiles, *(FPaths::Combine(FPaths::GetPath(Filename), TEXT("SECT*.BNK"))),
        true, false);
    for (const FString& SectorFile : SectorFiles)
        IFileManager::Get().Copy(*(FPaths::Combine(SourceRoot, SectorFile)),
            *(FPaths::Combine(FPaths::GetPath(Filename), SectorFile)), true, true);
    MainBank->SourceFilename = FPaths::Combine(SourceRoot, TEXT("LEVEL.BNK"));
    if (!MainBank->AssetImportData) MainBank->AssetImportData = NewObject<UAssetImportData>(MainBank);
    MainBank->AssetImportData->Update(MainBank->SourceFilename);

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create Milestone B world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn Milestone B preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = MainBank;
    Preview->bShowRebuildProgress = false;
    Preview->OnConstruction(FTransform::Identity);

    // Empty-project setup: create the descriptor, then discover every sibling sector.
    FString Error;
    const FString ExpectedDestination = KyaGetLevelSetDestination(Preview);
    UKyaLevelSet* LevelSet = KyaCreateLevelSetFromPreview(Preview, Error);
    if (!TestNotNull(TEXT("Create level-set descriptor"), LevelSet))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }
    Preview->LevelSet = LevelSet;
    TestEqual(TEXT("Level-set preview predicts the actual create destination"), LevelSet->GetPathName(), ExpectedDestination);
    TestEqual(TEXT("Level-set preview predicts the same destination for update"), KyaGetLevelSetDestination(Preview), ExpectedDestination);
    TestEqual(TEXT("A fresh descriptor has no sector associations"), LevelSet->Sectors.Num(), 0);

    const int64 BuildsBeforePreview = Preview->BuildCount;
    const int32 RevisionBeforePreview = LevelSet->ImportRevision;
    const FKyaSectorDiscoveryResult Scan = KyaDiscoverSectors(Preview, true, true);
    TestTrue(TEXT("Discovery preview identifies missing imports"), Scan.MissingIds.Num() > 0);
    TestEqual(TEXT("Discovery preview imports no assets"), Scan.Imported, 0);
    TestEqual(TEXT("Discovery preview leaves associations unchanged"), Preview->SectorBanks.Num(), 0);
    TestEqual(TEXT("Discovery preview does not rebuild the scene"), Preview->BuildCount, BuildsBeforePreview);
    TestEqual(TEXT("Discovery preview does not update the descriptor"), LevelSet->ImportRevision, RevisionBeforePreview);
    const FKyaSectorDiscoveryResult Discovery = KyaDiscoverSectors(Preview, true);
    TestEqual(TEXT("Discovery finds no missing sectors"), Discovery.MissingIds.Num(), 0);
    TestEqual(TEXT("Discovery finds no ambiguous sectors"), Discovery.Ambiguous, 0);
    TestTrue(TEXT("Discovery imports or reuses sector banks"), Discovery.Imported + Discovery.Reused > 0);
    TestTrue(TEXT("The empty setup imports fresh sector assets"), Discovery.Imported > 0);
    TestEqual(TEXT("The empty setup reuses no pre-existing sectors"), Discovery.Reused, 0);
    TestTrue(TEXT("The preview has sector banks"), Preview->SectorBanks.Num() > 0);
    TestEqual(TEXT("Descriptor matches preview associations"),
        LevelSet->Sectors.Num(), Preview->SectorBanks.Num());

    // Provenance and filename-to-sector identity for every association.
    bool bProvenance = true;
    for (const TObjectPtr<UKyaLevelBank>& SectorBank : Preview->SectorBanks)
    {
        if (!SectorBank) { bProvenance = false; continue; }
        bProvenance &= !SectorBank->SourceFilename.IsEmpty();
        FString ProvenancePath = SectorBank->AssetImportData
            ? SectorBank->AssetImportData->GetFirstFilename() : FString();
        FString SourcePath = SectorBank->SourceFilename;
        FPaths::NormalizeFilename(ProvenancePath);
        FPaths::NormalizeFilename(SourcePath);
        bProvenance &= !ProvenancePath.IsEmpty() &&
            FPaths::ConvertRelativePathToFull(ProvenancePath)
                .Equals(FPaths::ConvertRelativePathToFull(SourcePath), ESearchCase::IgnoreCase);
        bProvenance &= SectorBank->HasCurrentImportSchema();
    }
    TestTrue(TEXT("Sector banks carry matching import provenance"), bProvenance);

    Preview->OnConstruction(FTransform::Identity);
    const auto CountPreviewGeometry = [Preview](int32& OutVertices, int32& OutTriangles)
    {
        OutVertices = 0;
        OutTriangles = 0;
        for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
        {
            if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section))
            {
                OutVertices += MeshSection->ProcVertexBuffer.Num();
                OutTriangles += MeshSection->ProcIndexBuffer.Num() / 3;
            }
        }
    };
    int32 SceneVertices = 0, SceneTriangles = 0;
    CountPreviewGeometry(SceneVertices, SceneTriangles);
    TestTrue(TEXT("Discovered level reaches a usable scene with geometry"),
        SceneVertices > 0 && SceneTriangles > 0);

    // Re-running discovery is idempotent and creates no duplicates.
    const int32 SectorCount = Preview->SectorBanks.Num();
    const int32 DescriptorCount = LevelSet->Sectors.Num();
    const FKyaSectorDiscoveryResult Second = KyaDiscoverSectors(Preview, true);
    TestEqual(TEXT("Repeated discovery imports nothing"), Second.Imported, 0);
    TestTrue(TEXT("Repeated discovery reuses associations"), Second.Reused > 0);
    TestEqual(TEXT("Repeated discovery keeps sector count"), Preview->SectorBanks.Num(), SectorCount);
    TestEqual(TEXT("Repeated discovery keeps descriptor rows"), LevelSet->Sectors.Num(), DescriptorCount);

    // Safe reimport: sector identity and preview association survive, typed data refreshes.
    if (Preview->SectorBanks.Num() > 0 && Preview->SectorBanks[0])
    {
        UKyaLevelBank* SectorBank = Preview->SectorBanks[0];
        UKyaLevelBank* SectorBefore = SectorBank;
        const uint32 GenerationBefore = uint32(SectorBank->EditGeneration);
        // Only the fresh, this-run destination is allowed to be reimported; never touch
        // another project asset's unsaved state.
        if (TestTrue(TEXT("Reimport target belongs to the fresh destination"),
            SectorBank->GetOutermost()->GetName().Contains(MainBank->GetName())))
        {
            if (SectorBank->GetOutermost()) SectorBank->GetOutermost()->SetDirtyFlag(false);
            FKyaLevelBankReimportHandler Handler;
            TestEqual(TEXT("Safe sector reimport succeeds"), (int32)Handler.Reimport(SectorBank),
                (int32)EReimportResult::Succeeded);
            TestTrue(TEXT("Reimport keeps the same sector asset"), Preview->SectorBanks.Contains(SectorBefore));
            TestTrue(TEXT("Reimport bumps the edit generation"),
                uint32(SectorBank->EditGeneration) != GenerationBefore);
            TestEqual(TEXT("Reimport keeps the descriptor association count"),
                LevelSet->Sectors.Num(), DescriptorCount);
        }
    }

    Preview->Destroy();
    World->DestroyWorld(false);
    IFileManager::Get().DeleteDirectory(*SourceRoot, false, true);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSectorIsolationTest, "Kya.Editor.SectorIsolation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSectorIsolationTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the sector isolation acceptance."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read isolation bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const FString SourceDirectory = FPaths::GetPath(Filename);
    TArray<FString> SectorFiles;
    IFileManager::Get().FindFiles(SectorFiles, *(FPaths::Combine(SourceDirectory, TEXT("SECT*.BNK"))),
        true, false);
    if (!TestTrue(TEXT("The isolation bank has sibling sector files"), SectorFiles.Num() > 0)) return false;

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create isolation world"), World)) return false;
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();

    struct FIsolatedLevel
    {
        FString Tag;
        FString Root;
        UKyaLevelBank* Bank = nullptr;
        AKyaLevelPreview* Preview = nullptr;
        UKyaLevelSet* LevelSet = nullptr;
        int32 SectorCount = 0;
    };
    TArray<FIsolatedLevel> Levels;
    TArray<FString> CreatedRoots;
    bool bSetupOk = true;

    for (const TCHAR* Tag : { TEXT("A"), TEXT("B") })
    {
        FIsolatedLevel& Level = Levels.AddDefaulted_GetRef();
        Level.Tag = Tag;
        // Two levels whose sector files have identical names but distinct roots.
        Level.Root = FPaths::Combine(FPaths::ProjectSavedDir(), FString::Printf(
            TEXT("KyaSectorIsolation%s_%s"), Tag, *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
        CreatedRoots.Add(Level.Root);
        IFileManager::Get().MakeDirectory(*Level.Root, true);
        IFileManager::Get().Copy(*(FPaths::Combine(Level.Root, TEXT("LEVEL.BNK"))), *Filename, true, true);
        for (const FString& SectorFile : SectorFiles)
            IFileManager::Get().Copy(*(FPaths::Combine(Level.Root, SectorFile)),
                *(FPaths::Combine(SourceDirectory, SectorFile)), true, true);

        const uint8* Cursor = Input.GetData();
        Level.Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
            Input.GetData() + Input.Num(), GWarn));
        if (!TestNotNull(TEXT("Import isolated main bank"), Level.Bank))
        {
            bSetupOk = false;
            break;
        }
        Level.Bank->Rename(*FString::Printf(TEXT("KyaIsolation%s_%s"), Tag,
            *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
        Level.Bank->SourceFilename = FPaths::Combine(Level.Root, TEXT("LEVEL.BNK"));
        if (!Level.Bank->AssetImportData) Level.Bank->AssetImportData = NewObject<UAssetImportData>(Level.Bank);
        Level.Bank->AssetImportData->Update(Level.Bank->SourceFilename);

        // Discovery needs a saveable main-bank asset for the reopen half of the gate:
        // the raw import lives in the transient package and cannot be written to disk.
        {
            const FString BankName = Level.Bank->GetName();
            UPackage* BankPackage = CreatePackage(*FString::Printf(TEXT("/Game/KyaLevels/%s/LEVEL"),
                *BankName));
            UKyaLevelBank* SaveableBank = DuplicateObject<UKyaLevelBank>(Level.Bank, BankPackage, TEXT("LEVEL"));
            if (!TestNotNull(TEXT("Duplicate the isolated main bank for saving"), SaveableBank))
            {
                bSetupOk = false;
                break;
            }
            SaveableBank->ClearFlags(RF_Transient);
            SaveableBank->SetFlags(RF_Public | RF_Standalone);
            BankPackage->ClearFlags(RF_Transient);
            BankPackage->SetFlags(RF_Public | RF_Standalone);
            Level.Bank = SaveableBank;
        }

        Level.Preview = World->SpawnActor<AKyaLevelPreview>();
        if (!TestNotNull(TEXT("Spawn isolated preview"), Level.Preview))
        {
            bSetupOk = false;
            break;
        }
        Level.Preview->Bank = Level.Bank;
        Level.Preview->bShowRebuildProgress = false;
        Level.Preview->OnConstruction(FTransform::Identity);

        FString Error;
        Level.LevelSet = KyaCreateLevelSetFromPreview(Level.Preview, Error);
        if (!TestNotNull(TEXT("Create isolated level-set descriptor"), Level.LevelSet))
        {
            bSetupOk = false;
            break;
        }
        Level.Preview->LevelSet = Level.LevelSet;
        TestEqual(TEXT("Both root banks have the real shared name"), Level.Bank->GetName(), FString(TEXT("LEVEL")));
        TestEqual(TEXT("Descriptor is scoped to the full root bank package"),
            FPackageName::GetLongPackagePath(Level.LevelSet->GetOutermost()->GetName()),
            Level.Bank->GetOutermost()->GetName());
        const FKyaSectorDiscoveryResult Discovery = KyaDiscoverSectors(Level.Preview, true);
        TestTrue(TEXT("The isolated level imports fresh sectors"), Discovery.Imported > 0);
        TestEqual(TEXT("The isolated level reuses nothing on first discovery"), Discovery.Reused, 0);
        TestEqual(TEXT("Discovery has no ambiguous sectors"), Discovery.Ambiguous, 0);
        TestEqual(TEXT("Discovery has no missing sectors"), Discovery.MissingIds.Num(), 0);
        TestEqual(TEXT("Descriptor matches the preview associations"),
            Level.LevelSet->Sectors.Num(), Level.Preview->SectorBanks.Num());
        Level.SectorCount = Level.LevelSet->Sectors.Num();
        TestTrue(TEXT("The isolated level reached a scene"), Level.Preview->BuildCount > 0);
    }
    if (!bSetupOk)
    {
        for (const FString& Root : CreatedRoots) IFileManager::Get().DeleteDirectory(*Root, false, true);
        World->DestroyWorld(false);
        return false;
    }

    // --- the two levels stay isolated despite identical sector filenames -----------
    const auto ContainsBank = [](const UKyaLevelSet* Set, const UKyaLevelBank* Bank)
    {
        for (const FKyaLevelSetSector& Sector : Set->Sectors)
            if (Sector.Bank == Bank) return true;
        return false;
    };
    for (int32 Index = 0; Index < Levels.Num(); ++Index)
    {
        const FIsolatedLevel& Level = Levels[Index];
        const FIsolatedLevel& Other = Levels[1 - Index];
        for (const FKyaLevelSetSector& Sector : Level.LevelSet->Sectors)
        {
            TestNotNull(TEXT("Every association resolves to a bank"), Sector.Bank.Get());
            if (!Sector.Bank) continue;
            TestEqual(TEXT("Sector destination is scoped to the full root bank package"),
                FPackageName::GetLongPackagePath(Sector.Bank->GetOutermost()->GetName()),
                Level.Bank->GetOutermost()->GetName());
            TestTrue(TEXT("Associations stay inside their own source root"),
                FPaths::ConvertRelativePathToFull(Sector.Bank->SourceFilename)
                    .StartsWith(FPaths::ConvertRelativePathToFull(Level.Root), ESearchCase::IgnoreCase));
            TestFalse(TEXT("No association leaks into the other level"),
                ContainsBank(Other.LevelSet, Sector.Bank));
        }
        TestNotNull(TEXT("The descriptor keeps its main bank"), Level.LevelSet->MainBank.Get());
        TestEqual(TEXT("Both levels share the same sector count"), Level.SectorCount, Levels[0].SectorCount);
    }

    // Sector visibility affects presentation without removing associations or bytes.
    {
        FIsolatedLevel& Level = Levels[0];
        const FKyaLevelSetSector& Sector = Level.LevelSet->Sectors[0];
        const int32 Id = Sector.SectorId;
        const TArray<uint8> OriginalBytes = Sector.Bank->BankBytes;
        const int32 AssociatedCount = Level.Preview->SectorBanks.Num();
        const auto TriangleCount = [](AKyaLevelPreview* Preview) {
            int32 Count = 0;
            for (int32 Index = 0; Index < Preview->Mesh->GetNumSections(); ++Index)
                if (const FProcMeshSection* Section = Preview->Mesh->GetProcMeshSection(Index); Section && Section->bSectionVisible)
                    for (int32 Triangle = 0; Triangle + 2 < Section->ProcIndexBuffer.Num(); Triangle += 3)
                        if (Section->ProcIndexBuffer[Triangle] != Section->ProcIndexBuffer[Triangle + 1] &&
                            Section->ProcIndexBuffer[Triangle] != Section->ProcIndexBuffer[Triangle + 2] &&
                            Section->ProcIndexBuffer[Triangle + 1] != Section->ProcIndexBuffer[Triangle + 2]) ++Count;
            return Count;
        };
        const int32 VisibleTriangles = TriangleCount(Level.Preview);
        const int64 BuildsBeforeVisibility = Level.Preview->BuildCount;
        const int32 DecodesBeforeVisibility = Level.Preview->GetGeometryDecodeCount();
        Level.Preview->SetSectorVisible(Id, false);
        TestFalse(TEXT("Sector checkbox hides its sector"), Level.Preview->IsSectorVisible(Id));
        TestTrue(TEXT("Hiding a sector removes its shared geometry"),
            TriangleCount(Level.Preview) < VisibleTriangles);
        TestEqual(TEXT("Hiding keeps bank associations"), Level.Preview->SectorBanks.Num(), AssociatedCount);
        TestTrue(TEXT("Hiding leaves bank bytes intact"), Sector.Bank->BankBytes == OriginalBytes);
        TestTrue(TEXT("Other preview stays visible"), Levels[1].Preview->IsSectorVisible(Id));
        TestTrue(TEXT("Global content stays visible"), Level.Preview->IsSectorVisible(-1));
        Level.Preview->SetSectorVisible(Id, true);
        TestEqual(TEXT("Showing restores shared geometry"), TriangleCount(Level.Preview), VisibleTriangles);
        TestEqual(TEXT("Sector toggles never rebuild the preview"), Level.Preview->BuildCount, BuildsBeforeVisibility);
        TestEqual(TEXT("Sector toggles never decode geometry"),
            Level.Preview->GetGeometryDecodeCount(), DecodesBeforeVisibility);
    }

    // --- repeated discovery creates no duplicates or new assets --------------------
    for (FIsolatedLevel& Level : Levels)
    {
        const int32 PreviewCount = Level.Preview->SectorBanks.Num();
        const int32 DescriptorCount = Level.LevelSet->Sectors.Num();
        const FKyaSectorDiscoveryResult Again = KyaDiscoverSectors(Level.Preview, true);
        TestEqual(TEXT("Repeated discovery imports nothing"), Again.Imported, 0);
        TestTrue(TEXT("Repeated discovery reuses associations"), Again.Reused > 0);
        TestEqual(TEXT("Repeated discovery keeps preview associations"),
            Level.Preview->SectorBanks.Num(), PreviewCount);
        TestEqual(TEXT("Repeated discovery keeps descriptor rows"),
            Level.LevelSet->Sectors.Num(), DescriptorCount);
    }

    // --- reopening restores associations from disk ---------------------------------
    FIsolatedLevel& LevelA = Levels[0];
    const FString DescriptorPath = LevelA.LevelSet->GetPathName();
    const FString MainBankPath = LevelA.Bank->GetPathName();
    TArray<TPair<int32, FString>> ExpectedSectors;
    for (const FKyaLevelSetSector& Sector : LevelA.LevelSet->Sectors)
        ExpectedSectors.Add({ Sector.SectorId, Sector.SourceFilename });

    // Drop the previews before unloading: their handles and descriptor pointers would be
    // stale after the reload. A fresh preview reopens from the reloaded descriptor.
    for (FIsolatedLevel& Level : Levels)
    {
        if (IsValid(Level.Preview)) Level.Preview->Destroy();
        Level.Preview = nullptr;
    }

    TArray<UPackage*> PackagesToSave;
    TArray<TPair<UPackage*, UObject*>> AssetsToSave;
    const auto AddSavable = [&PackagesToSave, &AssetsToSave](UObject* Asset)
    {
        if (!Asset) return;
        UPackage* Package = Asset->GetOutermost();
        if (!Package || PackagesToSave.Contains(Package)) return;
        PackagesToSave.Add(Package);
        AssetsToSave.Add({ Package, Asset });
    };
    AddSavable(LevelA.LevelSet);
    AddSavable(LevelA.Bank);
    for (const FKyaLevelSetSector& Sector : LevelA.LevelSet->Sectors) AddSavable(Sector.Bank);

    TArray<FString> SavedPackageNames;
    for (const TPair<UPackage*, UObject*>& Pair : AssetsToSave)
        SavedPackageNames.Add(Pair.Key->GetName());

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    bool bSaved = true;
    for (const TPair<UPackage*, UObject*>& Pair : AssetsToSave)
    {
        Pair.Key->MarkPackageDirty();
        const FString PackageFilename = FPackageName::LongPackageNameToFilename(
            Pair.Key->GetName(), FPackageName::GetAssetPackageExtension());
        const bool bSavedOne = UPackage::SavePackage(Pair.Key, Pair.Value, *PackageFilename, SaveArgs);
        if (!bSavedOne)
            AddInfo(FString::Printf(TEXT("SAVE_FAIL %s -> %s"), *Pair.Key->GetName(), *PackageFilename));
        bSaved &= bSavedOne;
    }
    TestTrue(TEXT("Isolated level assets save for the reopen check"), bSaved);

    FText ReloadError;
    const bool bReloaded = UPackageTools::ReloadPackages(PackagesToSave, ReloadError,
        EReloadPackagesInteractionMode::AssumePositive);
    TestTrue(FString::Printf(TEXT("Reopening the isolated descriptors succeeds (%s)"), *ReloadError.ToString()),
        bReloaded);

    UKyaLevelSet* ReloadedDescriptor = FindObject<UKyaLevelSet>(nullptr, *DescriptorPath);
    UKyaLevelBank* ReloadedMainBank = FindObject<UKyaLevelBank>(nullptr, *MainBankPath);
    if (TestNotNull(TEXT("The descriptor reloads from disk"), ReloadedDescriptor) &&
        TestNotNull(TEXT("The main bank reloads from disk"), ReloadedMainBank))
    {
        TestEqual(TEXT("Reopened descriptor keeps every association"),
            ReloadedDescriptor->Sectors.Num(), ExpectedSectors.Num());
        for (const TPair<int32, FString>& Expected : ExpectedSectors)
        {
            const FKyaLevelSetSector* Match = ReloadedDescriptor->Sectors.FindByPredicate(
                [&Expected](const FKyaLevelSetSector& Sector) { return Sector.SectorId == Expected.Key; });
            TestNotNull(TEXT("Reopened descriptor keeps the sector id"), Match);
            if (!Match) continue;
            TestNotNull(TEXT("Reopened association resolves its bank"), Match->Bank.Get());
            TestEqual(TEXT("Reopened association keeps its source"), Match->SourceFilename, Expected.Value);
            if (Match->Bank)
                TestTrue(TEXT("Reopened association stays in its own root"),
                    FPaths::ConvertRelativePathToFull(Match->Bank->SourceFilename)
                        .StartsWith(FPaths::ConvertRelativePathToFull(LevelA.Root), ESearchCase::IgnoreCase));
        }
        TestEqual(TEXT("Reopened main bank keeps its source"), ReloadedMainBank->SourceFilename,
            FPaths::Combine(LevelA.Root, TEXT("LEVEL.BNK")));

        // A preview attached to the reopened descriptor restores the same mapping.
        AKyaLevelPreview* ReopenedPreview = World->SpawnActor<AKyaLevelPreview>();
        if (TestNotNull(TEXT("Reopened preview spawns"), ReopenedPreview))
        {
            ReopenedPreview->Bank = ReloadedMainBank;
            ReopenedPreview->bShowRebuildProgress = false;
            ReopenedPreview->LevelSet = ReloadedDescriptor;
            ReopenedPreview->OnConstruction(FTransform::Identity);
            TestEqual(TEXT("Reopening restores the preview associations"),
                ReopenedPreview->SectorBanks.Num(), ExpectedSectors.Num());
            for (const FKyaLevelSetSector& Sector : ReloadedDescriptor->Sectors)
            {
                if (!Sector.Bank) continue;
                TestEqual(TEXT("Reopened preview resolves the descriptor sector id"),
                    ReopenedPreview->ResolveSectorId(*Sector.Bank), Sector.SectorId);
            }
        }

        // And the other level is untouched by the reopen.
        TestTrue(TEXT("The other level stays isolated after a reopen"),
            Levels[1].LevelSet && Levels[1].LevelSet->Sectors.Num() == Levels[1].SectorCount);
    }

    for (const FString& Root : CreatedRoots) IFileManager::Get().DeleteDirectory(*Root, false, true);
    // The reload unloaded the original packages, so use the captured names.
    for (const FString& PackageName : SavedPackageNames)
    {
        const FString PackageFilename = FPackageName::LongPackageNameToFilename(
            PackageName, FPackageName::GetAssetPackageExtension());
        IFileManager::Get().Delete(*PackageFilename, false, true, true);
    }
    for (AKyaLevelPreview* Preview : { Levels[0].Preview, Levels[1].Preview })
        if (IsValid(Preview)) Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSelectionOrganisationTest, "Kya.Editor.SelectionOrganisation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSelectionOrganisationTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    Bank->SceneEnvironment.Waypoints.AddDefaulted();

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create the organization world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn the organization preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = true;
    Preview->bSpawnWaypointHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    AKyaActorHandle* First = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
    AKyaActorHandle* Second = Cast<AKyaActorHandle>(Preview->GetActorHandle(1));
    if (!TestNotNull(TEXT("First organization handle exists"), First) ||
        !TestNotNull(TEXT("Second organization handle exists"), Second))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }

    const auto CountActorHandles = [World]()
    {
        int32 Count = 0;
        for (TActorIterator<AKyaActorHandle> It(World); It; ++It) ++Count;
        return Count;
    };
    const int32 HandleCountBefore = CountActorHandles();
    const int32 BankActorCount = Bank->SceneEnvironment.Actors.Num();
    const int32 EditGenerationBefore = Bank->EditGeneration;
    const FKyaActorPlacement FirstBefore = Bank->SceneEnvironment.Actors[0];
    const FKyaActorPlacement SecondBefore = Bank->SceneEnvironment.Actors[1];

    // --- familiar selection commands ----------------------------------------------
    if (UUnrealEdEngine* EdEngine = Cast<UUnrealEdEngine>(GEditor))
    {
        EdEngine->SelectNone(false, true, false);
        TestFalse(TEXT("Select None clears the generated selection"), KyaSelectionContainsGeneratedActor());
        EdEngine->edactSelectAll(World);
        TestTrue(TEXT("Select All includes generated representations"), KyaSelectionContainsGeneratedActor());
        TestTrue(TEXT("Select All selects every actor handle"), First->IsSelected() && Second->IsSelected());
        EdEngine->SelectNone(false, true, false);
        EdEngine->SelectActor(First, true, true);
        TestTrue(TEXT("A single actor handle is selected"), First->IsSelected());
        EdEngine->edactSelectInvert(World);
        TestFalse(TEXT("Select Invert deselects the previous handle"), First->IsSelected());
        TestTrue(TEXT("Select Invert selects the preview"), Preview->IsSelected());
        EdEngine->SelectNone(false, true, false);
        EdEngine->SelectActor(First, true, true);
        EdEngine->SelectActor(Second, true, true);
        TestTrue(TEXT("Multi-selection keeps both handles"), First->IsSelected() && Second->IsSelected());
        TestTrue(TEXT("Multi-selection detects generated actors"), KyaSelectionContainsGeneratedActor());
        EdEngine->SelectNone(false, true, false);
    }

    // --- organization commands keep the bank untouched ----------------------------
    Preview->SetGeneratedActorTemporarilyHidden(First, true);
    TestTrue(TEXT("Hide command hides the representation"), First->IsTemporarilyHiddenInEditor());
    TestTrue(TEXT("Hide command does not change the bank"), Bank->SceneEnvironment.Actors[0].Position.Equals(FirstBefore.Position));
    Preview->SetGeneratedActorTemporarilyHidden(First, false);
    TestFalse(TEXT("Show command restores the representation"), First->IsTemporarilyHiddenInEditor());

    Preview->IsolateGeneratedActors({ First });
    TestTrue(TEXT("Isolate command isolates the representation"), Preview->IsGeneratedActorIsolated(First));
    TestTrue(TEXT("Isolate command hides other handles"), Second->IsTemporarilyHiddenInEditor());
    Preview->ClearGeneratedActorIsolation();

    Preview->bLockGeneratedActorSelection = true;
    TestFalse(TEXT("Lock command blocks viewport picking"), First->IsSelectable());
    Preview->bLockGeneratedActorSelection = false;
    TestTrue(TEXT("Unlock command restores viewport picking"), First->IsSelectable());

    if (GEditor)
    {
        AKyaLevelPreview* OtherPreview = World->SpawnActor<AKyaLevelPreview>();
        GEditor->SelectNone(false, true, false);
        GEditor->SelectActor(Preview, true, true);
        TestEqual(TEXT("The displayed scope identifies one selected preview"), KyaGetPreviewScopeText(), FString(TEXT("Selected previews (1)")));
        KyaSetGeneratedActorSelectionLocked(true);
        TestTrue(TEXT("Selection lock affects the selected preview"), Preview->bLockGeneratedActorSelection);
        if (OtherPreview) TestFalse(TEXT("Selection lock leaves unselected previews unchanged"), OtherPreview->bLockGeneratedActorSelection);
        KyaSetGeneratedActorSelectionLocked(false);
        GEditor->SelectNone(false, true, false);
        TestTrue(TEXT("No selected previews explicitly displays the all-preview fallback"), KyaGetPreviewScopeText().StartsWith(TEXT("All previews (")));
        if (OtherPreview) OtherPreview->Destroy();
    }

    // --- unsupported structural operations keep map and bank consistent -----------
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.ObjectFlags |= RF_Transient | RF_Transactional;
    AKyaActorHandle* Duplicate = World->SpawnActor<AKyaActorHandle>(
        AKyaActorHandle::StaticClass(), FTransform::Identity, Params);
    if (TestNotNull(TEXT("A duplicated representation spawns"), Duplicate))
    {
        Duplicate->Initialize(Preview, Bank, 0, true);
        Duplicate->AddActorWorldOffset(FVector(300.0, 0.0, 0.0));
        Duplicate->PostEditMove(true);
        TestTrue(TEXT("A duplicated handle cannot move the bank record"),
            Bank->SceneEnvironment.Actors[0].Position.Equals(FirstBefore.Position));
        TestTrue(TEXT("Stray cleanup removes the duplicated representation"), KyaDestroyStrayGeneratedActors(World) >= 1);
        TestFalse(TEXT("The duplicated representation is gone"), IsValid(Duplicate));
    }

    AKyaActorHandle* Pasted = World->SpawnActor<AKyaActorHandle>(
        AKyaActorHandle::StaticClass(), FTransform::Identity, Params);
    if (TestNotNull(TEXT("A pasted representation spawns"), Pasted))
    {
        Pasted->Initialize(Preview, Bank, 1, true);
        TestTrue(TEXT("Stray cleanup removes the pasted representation"), KyaDestroyStrayGeneratedActors(World) >= 1);
        TestFalse(TEXT("The pasted representation is gone"), IsValid(Pasted));
    }

    FText DeleteReason;
    TestFalse(TEXT("Delete is rejected for generated handles"), First->CanDeleteSelectedActor(DeleteReason));
    TestTrue(TEXT("The delete rejection names the bank record"),
        DeleteReason.ToString().Contains(TEXT("bank")));
    TestTrue(TEXT("The rejected delete keeps the handle"), IsValid(First) && Preview->GetActorHandle(0) == First);

    TestEqual(TEXT("Structural commands keep the world representation count"), CountActorHandles(), HandleCountBefore);
    TestEqual(TEXT("Structural commands keep the bank actor count"),
        Bank->SceneEnvironment.Actors.Num(), BankActorCount);
    TestEqual(TEXT("Structural commands do not bump the bank generation"),
        Bank->EditGeneration, EditGenerationBefore);
    TestTrue(TEXT("Structural commands keep the first record"), Bank->SceneEnvironment.Actors[0].Position.Equals(FirstBefore.Position) &&
        Bank->SceneEnvironment.Actors[0].VisibilityDistance == FirstBefore.VisibilityDistance &&
        Bank->SceneEnvironment.Actors[0].SectorId == FirstBefore.SectorId);
    TestTrue(TEXT("Structural commands keep the second record"), Bank->SceneEnvironment.Actors[1].Position.Equals(SecondBefore.Position) &&
        Bank->SceneEnvironment.Actors[1].VisibilityDistance == SecondBefore.VisibilityDistance &&
        Bank->SceneEnvironment.Actors[1].SectorId == SecondBefore.SectorId);

    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSpatialRecordAcceptanceTest, "Kya.Editor.SpatialRecordAcceptance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSpatialRecordAcceptanceTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the spatial-record acceptance."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read the spatial-record bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();

    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    Factory->AddToRoot();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Imported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import the spatial-record bank"), Imported) ||
        !TestTrue(TEXT("The spatial-record bank has waypoints and paths"),
            Imported->SceneEnvironment.Waypoints.Num() > 0 && !Imported->SceneEnvironment.Paths.IsEmpty() &&
            !Imported->SceneEnvironment.Paths[0].Points.IsEmpty()))
    {
        Factory->RemoveFromRoot();
        return false;
    }

    const FString PackageName = FString::Printf(TEXT("/Game/KyaSpatialTest/Bank_%s"),
        *FGuid::NewGuid().ToString(EGuidFormats::Digits));
    UPackage* Package = CreatePackage(*PackageName);
    UKyaLevelBank* Bank = DuplicateObject<UKyaLevelBank>(Imported, Package, TEXT("Bank"));
    if (!TestNotNull(TEXT("Duplicate the spatial bank into a saveable package"), Bank))
    {
        Factory->RemoveFromRoot();
        return false;
    }
    Bank->ClearFlags(RF_Transient);
    Bank->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
    Package->ClearFlags(RF_Transient);
    Package->SetFlags(RF_Public | RF_Standalone);
    Bank->SourceFilename = Filename;
    if (!Bank->AssetImportData) Bank->AssetImportData = NewObject<UAssetImportData>(Bank);
    Bank->AssetImportData->Update(Bank->SourceFilename);
    Package->SetDirtyFlag(false);

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create the spatial-record world"), World))
    {
        Factory->RemoveFromRoot();
        return false;
    }
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn the spatial-record preview"), Preview))
    {
        World->DestroyWorld(false);
        Factory->RemoveFromRoot();
        return false;
    }
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = false;
    Preview->bSpawnWaypointHandles = true;
    Preview->bSpawnPathPointHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    const auto FindHandle = [World](EKyaSpatialRecordKind Kind, const TCHAR* Prefix)
        -> AKyaSpatialRecordHandle*
    {
        for (TActorIterator<AKyaSpatialRecordHandle> It(World); It; ++It)
            if (It->Kind == Kind && It->GetActorLabel().StartsWith(Prefix)) return *It;
        return nullptr;
    };
    AKyaSpatialRecordHandle* Waypoint = FindHandle(EKyaSpatialRecordKind::Waypoint, TEXT("Waypoint "));
    AKyaSpatialRecordHandle* PathPoint = FindHandle(EKyaSpatialRecordKind::PathPoint, TEXT("Path "));
    if (!TestNotNull(TEXT("A waypoint is discoverable from the Outliner"), Waypoint) ||
        !TestNotNull(TEXT("A path point is discoverable from the Outliner"), PathPoint))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        Factory->RemoveFromRoot();
        return false;
    }
    const int32 WaypointIndex = Waypoint->RecordIndex;
    const int32 PathIndex = PathPoint->RecordIndex;
    const int32 PointIndex = PathPoint->PointIndex;
    TestTrue(TEXT("The waypoint label identifies its kind and position"),
        Waypoint->GetActorLabel().Contains(TEXT("Waypoint")) && Waypoint->GetActorLabel().Contains(TEXT("(")));
    TestTrue(TEXT("The path-point label identifies its path, point and position"),
        PathPoint->GetActorLabel().Contains(FString::Printf(TEXT("Path %d Point %d"), PathIndex, PointIndex)));
    TestTrue(TEXT("The waypoint describes its editable scope"), Waypoint->EditableScope.Contains(TEXT("Editable")));
    TestEqual(TEXT("The waypoint mirrors the recorded position"),
        Waypoint->WaypointPosition, Bank->SceneEnvironment.Waypoints[WaypointIndex].Position);
    TestTrue(TEXT("The path point mirrors the recorded position"),
        PathPoint->PathPoint.Equals(Bank->SceneEnvironment.Paths[PathIndex].Points[PointIndex]));

    // Scope hints must match the arrays actually serialized for this path.
    const FKyaPath& Path = Bank->SceneEnvironment.Paths[PathIndex];
    TestEqual(TEXT("The rotation hint matches the serialized record"),
        PathPoint->bHasRotationRecord, Path.Rotations.IsValidIndex(PointIndex));
    TestEqual(TEXT("The delay hint matches the serialized record"),
        PathPoint->bHasDelayRecord, Path.Delays.IsValidIndex(PointIndex));
    TestEqual(TEXT("The flags hint matches the serialized record"),
        PathPoint->bHasFlagsRecord, Path.PointFlags.IsValidIndex(PointIndex));    TestTrue(TEXT("The path-point scope states rotation is numeric only"),
        PathPoint->EditableScope.Contains(TEXT("numeric only")));

    // --- waypoint Details edit, undo and redo -------------------------------------
    const FKyaWaypoint WaypointBefore = Bank->SceneEnvironment.Waypoints[WaypointIndex];
    const FVector EditedWaypoint = WaypointBefore.Position + FVector(2.5, 1.5, -0.5);
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "WaypointDetails", "Waypoint details"));
        Waypoint->WaypointPosition = EditedWaypoint;
        FPropertyChangedEvent ChangedEvent(FindFProperty<FProperty>(AKyaSpatialRecordHandle::StaticClass(),
            GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, WaypointPosition)));
        Waypoint->PostEditChangeProperty(ChangedEvent);
        Service.FlushPendingNotifications();
    }
    TestTrue(TEXT("The waypoint Details edit writes its position"),
        Bank->SceneEnvironment.Waypoints[WaypointIndex].Position.Equals(EditedWaypoint, 1.0e-4));
    TestTrue(TEXT("The waypoint edit preserves its rotation"),
        Bank->SceneEnvironment.Waypoints[WaypointIndex].Rotation.Equals(WaypointBefore.Rotation, 1.0e-5));
    TestTrue(TEXT("The waypoint edit preserves its parameters"),
        Bank->SceneEnvironment.Waypoints[WaypointIndex].Parameters.Equals(WaypointBefore.Parameters, 1.0e-5));
    TestTrue(TEXT("The waypoint label refreshes to the edited position"),
        Waypoint->GetActorLabel().Contains(FString::Printf(TEXT("(%.1f, %.1f, %.1f)"),
            EditedWaypoint.X, EditedWaypoint.Y, EditedWaypoint.Z)));
    if (GEditor)
    {
        TestTrue(TEXT("Undo returns the waypoint transaction"), GEditor->UndoTransaction());
        Service.FlushPendingNotifications();
        TestTrue(TEXT("Undo restores the waypoint position"),
            Bank->SceneEnvironment.Waypoints[WaypointIndex].Position.Equals(WaypointBefore.Position, 1.0e-4));
        TestEqual(TEXT("Undo refreshes the waypoint handle"), Waypoint->WaypointPosition, WaypointBefore.Position);
        GEditor->RedoTransaction();
        Service.FlushPendingNotifications();
        TestTrue(TEXT("Redo reapplies the waypoint position"),
            Bank->SceneEnvironment.Waypoints[WaypointIndex].Position.Equals(EditedWaypoint, 1.0e-4));
    }

    // --- path-point Details edits -------------------------------------------------
    const FVector4 PointBefore = Bank->SceneEnvironment.Paths[PathIndex].Points[PointIndex];
    const FVector4 EditedPoint(PointBefore.X + 3.0, PointBefore.Y - 1.5, PointBefore.Z + 0.5, PointBefore.W);
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "PathPointDetails", "Path point details"));
        PathPoint->PathPoint = EditedPoint;
        FPropertyChangedEvent ChangedEvent(FindFProperty<FProperty>(AKyaSpatialRecordHandle::StaticClass(),
            GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathPoint)));
        PathPoint->PostEditChangeProperty(ChangedEvent);
        Service.FlushPendingNotifications();
    }
    TestTrue(TEXT("The path-point Details edit writes its position"),
        Bank->SceneEnvironment.Paths[PathIndex].Points[PointIndex].Equals(EditedPoint));
    TestTrue(TEXT("The path-point label refreshes to the edited position"),
        PathPoint->GetActorLabel().Contains(FString::Printf(TEXT("(%.1f, %.1f, %.1f)"),
            EditedPoint.X, EditedPoint.Y, EditedPoint.Z)));

    if (PathPoint->bHasDelayRecord)
    {
        const float DelayBefore = Bank->SceneEnvironment.Paths[PathIndex].Delays[PointIndex];
        const float EditedDelay = DelayBefore + 0.25f;
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "PathDelayDetails", "Path delay details"));
        PathPoint->PathDelay = EditedDelay;
        FPropertyChangedEvent ChangedEvent(FindFProperty<FProperty>(AKyaSpatialRecordHandle::StaticClass(),
            GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathDelay)));
        PathPoint->PostEditChangeProperty(ChangedEvent);
        Service.FlushPendingNotifications();
        TestEqual(TEXT("The path-point Details edit writes its delay"),
            Bank->SceneEnvironment.Paths[PathIndex].Delays[PointIndex], EditedDelay);
    }

    if (PathPoint->bHasRotationRecord)
    {
        const FVector4 RotationBefore = Bank->SceneEnvironment.Paths[PathIndex].Rotations[PointIndex];
        const FVector4 EditedRotation(RotationBefore.X + 0.1, RotationBefore.Y + 0.2,
            RotationBefore.Z + 0.3, RotationBefore.W);
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "PathRotationDetails", "Path rotation details"));
        PathPoint->PathRotation = EditedRotation;
        FPropertyChangedEvent ChangedEvent(FindFProperty<FProperty>(AKyaSpatialRecordHandle::StaticClass(),
            GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathRotation)));
        PathPoint->PostEditChangeProperty(ChangedEvent);
        Service.FlushPendingNotifications();
        TestEqual(TEXT("The path-point Details edit writes its rotation"),
            Bank->SceneEnvironment.Paths[PathIndex].Rotations[PointIndex], EditedRotation);
    }
    else
    {
        const int32 RotationCount = Bank->SceneEnvironment.Paths[PathIndex].Rotations.Num();
        const FVector4 Before = PathPoint->PathRotation;
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "PathRotationReject", "Path rotation reject"));
        PathPoint->PathRotation = FVector4(1.0, 2.0, 3.0, 4.0);
        FPropertyChangedEvent ChangedEvent(FindFProperty<FProperty>(AKyaSpatialRecordHandle::StaticClass(),
            GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathRotation)));
        PathPoint->PostEditChangeProperty(ChangedEvent);
        Service.FlushPendingNotifications();
        TestEqual(TEXT("A path without rotation records keeps that array unchanged"),
            Bank->SceneEnvironment.Paths[PathIndex].Rotations.Num(), RotationCount);
        TestTrue(TEXT("A rejected path rotation resynchronizes the handle"),
            PathPoint->PathRotation.Equals(Before));
        TestTrue(TEXT("The scope explains the rejected rotation"),
            PathPoint->EditableScope.Contains(TEXT("Missing per-point records")));
    }

    // --- save, close, reopen and export ------------------------------------------
    const FKyaWaypoint ExpectedWaypoint = Bank->SceneEnvironment.Waypoints[WaypointIndex];
    const FVector4 ExpectedPoint = Bank->SceneEnvironment.Paths[PathIndex].Points[PointIndex];
    const int32 ExpectedRotationCount = Bank->SceneEnvironment.Paths[PathIndex].Rotations.Num();
    const bool bHadRotationRecord = PathPoint->bHasRotationRecord;
    const bool bHadDelayRecord = PathPoint->bHasDelayRecord;
    const FVector4 ExpectedRotation = bHadRotationRecord
        ? Bank->SceneEnvironment.Paths[PathIndex].Rotations[PointIndex] : FVector4(0, 0, 0, 0);
    const float ExpectedDelay = bHadDelayRecord
        ? Bank->SceneEnvironment.Paths[PathIndex].Delays[PointIndex] : 0.0f;

    const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName,
        FPackageName::GetAssetPackageExtension());
    Package->MarkPackageDirty();
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    const bool bSaved = UPackage::SavePackage(Package, Bank, *PackageFilename, SaveArgs);
    TestTrue(TEXT("The edited spatial bank saves"), bSaved);

    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();

    FText ReloadError;
    const bool bReloaded = bSaved && UPackageTools::ReloadPackages({ Package }, ReloadError,
        EReloadPackagesInteractionMode::AssumePositive);
    TestTrue(FString::Printf(TEXT("The edited spatial bank reopens from disk (%s)"), *ReloadError.ToString()),
        bReloaded);
    UPackage* LoadedPackage = FindPackage(nullptr, *PackageName);
    UKyaLevelBank* LoadedBank = LoadedPackage ? FindObject<UKyaLevelBank>(LoadedPackage, TEXT("Bank")) : nullptr;
    if (TestNotNull(TEXT("The reopened package holds the spatial bank"), LoadedBank) &&
        TestTrue(TEXT("The reopened bank keeps the waypoint and path"),
            LoadedBank->SceneEnvironment.Waypoints.IsValidIndex(WaypointIndex) &&
            LoadedBank->SceneEnvironment.Paths.IsValidIndex(PathIndex) &&
            LoadedBank->SceneEnvironment.Paths[PathIndex].Points.IsValidIndex(PointIndex)))
    {
        TestTrue(TEXT("Reopen preserves the waypoint position"),
            LoadedBank->SceneEnvironment.Waypoints[WaypointIndex].Position.Equals(ExpectedWaypoint.Position, 1.0e-4));
        TestTrue(TEXT("Reopen preserves the waypoint rotation"),
            LoadedBank->SceneEnvironment.Waypoints[WaypointIndex].Rotation.Equals(ExpectedWaypoint.Rotation, 1.0e-5));
        TestTrue(TEXT("Reopen preserves the path point"),
            LoadedBank->SceneEnvironment.Paths[PathIndex].Points[PointIndex].Equals(ExpectedPoint));

        UWorld* ReopenedWorld = UWorld::CreateWorld(EWorldType::EditorPreview, false);
        if (TestNotNull(TEXT("Create the reopened spatial world"), ReopenedWorld))
        {
            AKyaLevelPreview* ReopenedPreview = ReopenedWorld->SpawnActor<AKyaLevelPreview>();
            if (TestNotNull(TEXT("Spawn the reopened spatial preview"), ReopenedPreview))
            {
                ReopenedPreview->Bank = LoadedBank;
                ReopenedPreview->bShowRebuildProgress = false;
                ReopenedPreview->bSpawnActorHandles = false;
                ReopenedPreview->bSpawnWaypointHandles = true;
                ReopenedPreview->bSpawnPathPointHandles = true;
                ReopenedPreview->OnConstruction(FTransform::Identity);
                AKyaSpatialRecordHandle* ReopenedWaypoint = nullptr;
                AKyaSpatialRecordHandle* ReopenedPoint = nullptr;
                for (TActorIterator<AKyaSpatialRecordHandle> It(ReopenedWorld); It; ++It)
                {
                    if (It->Kind == EKyaSpatialRecordKind::Waypoint && It->RecordIndex == WaypointIndex)
                        ReopenedWaypoint = *It;
                    if (It->Kind == EKyaSpatialRecordKind::PathPoint &&
                        It->RecordIndex == PathIndex && It->PointIndex == PointIndex)
                        ReopenedPoint = *It;
                    if (ReopenedWaypoint && ReopenedPoint) break;
                }
                if (TestNotNull(TEXT("The reopened preview shows the waypoint"), ReopenedWaypoint))
                {
                    TestTrue(TEXT("The reopened waypoint mirrors the saved position"),
                        ReopenedWaypoint->WaypointPosition.Equals(ExpectedWaypoint.Position, 1.0e-4));
                    TestTrue(TEXT("The reopened waypoint keeps its editable scope"),
                        ReopenedWaypoint->EditableScope.Contains(TEXT("Editable")));
                }
                if (TestNotNull(TEXT("The reopened preview shows the path point"), ReopenedPoint))
                {
                    TestTrue(TEXT("The reopened path point mirrors the saved position"),
                        ReopenedPoint->PathPoint.Equals(ExpectedPoint));
                    TestEqual(TEXT("The reopened path point keeps its record hints"),
                        ReopenedPoint->bHasRotationRecord, ExpectedRotationCount > PointIndex);
                }
            }
            if (ReopenedPreview) ReopenedPreview->Destroy();
            ReopenedWorld->DestroyWorld(false);
        }

        FKyaBankExportDiagnostics Diagnostics;
        TArray<uint8> Exported;
        if (TestTrue(TEXT("Export the reopened spatial bank"),
            KyaBuildBankExportBytes(LoadedBank, Exported, Diagnostics)))
        {
            const uint8* ExportedCursor = Exported.GetData();
            UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
                UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
                TEXT("bnk"), ExportedCursor, Exported.GetData() + Exported.Num(), GWarn));
            if (TestNotNull(TEXT("Reimport the exported spatial bank"), Reimported) &&
                TestTrue(TEXT("The reimported bank keeps the waypoint and path"),
                    Reimported->SceneEnvironment.Waypoints.IsValidIndex(WaypointIndex) &&
                    Reimported->SceneEnvironment.Paths.IsValidIndex(PathIndex) &&
                    Reimported->SceneEnvironment.Paths[PathIndex].Points.IsValidIndex(PointIndex)))
            {
                TestTrue(TEXT("Export preserves the waypoint position"),
                    Reimported->SceneEnvironment.Waypoints[WaypointIndex].Position.Equals(ExpectedWaypoint.Position, 1.0e-4));
                TestTrue(TEXT("Export preserves the waypoint rotation"),
                    Reimported->SceneEnvironment.Waypoints[WaypointIndex].Rotation.Equals(ExpectedWaypoint.Rotation, 1.0e-5));
                TestTrue(TEXT("Export preserves the path point"),
                    Reimported->SceneEnvironment.Paths[PathIndex].Points[PointIndex].Equals(ExpectedPoint));
                if (bHadRotationRecord)
                    TestTrue(TEXT("Export preserves the path-point rotation"),
                        Reimported->SceneEnvironment.Paths[PathIndex].Rotations[PointIndex].Equals(
                            ExpectedRotation, 1.0e-4));
                if (bHadDelayRecord)
                    TestEqual(TEXT("Export preserves the path-point delay"),
                        Reimported->SceneEnvironment.Paths[PathIndex].Delays[PointIndex], ExpectedDelay);
            }
        }
    }

    if (LoadedPackage) LoadedPackage->SetDirtyFlag(false);
    IFileManager::Get().Delete(*PackageFilename, false, true, true);
    Service.Reset();
    Factory->RemoveFromRoot();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSectorAssignmentTest, "Kya.Editor.SectorAssignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSectorAssignmentTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the sector-assignment acceptance."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read the sector-assignment bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();

    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import the sector-assignment bank"), Bank)) return false;

    const FString SectorFilename = FPaths::Combine(FPaths::GetPath(Filename), TEXT("SECT1.BNK"));
    TArray<uint8> SectorInput;
    UKyaLevelBank* SectorBank = nullptr;
    if (TestTrue(TEXT("Read the SECT1 bank"), FFileHelper::LoadFileToArray(SectorInput, *SectorFilename)))
    {
        const uint8* SectorCursor = SectorInput.GetData();
        SectorBank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), SectorCursor,
            SectorInput.GetData() + SectorInput.Num(), GWarn));
        if (TestNotNull(TEXT("Import the SECT1 bank"), SectorBank))
            SectorBank->SourceFilename = SectorFilename;
    }
    if (!SectorBank)
    {
        Service.Reset();
        return false;
    }

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create the sector-assignment world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn the sector-assignment preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->SectorBanks.Add(SectorBank);
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    const FBox* SectorOneBounds = Preview->GetSectorBackgroundBounds().Find(1);
    if (!TestNotNull(TEXT("The loaded sector background contributes bounds"), SectorOneBounds) ||
        !TestTrue(TEXT("The sector background bounds enclose geometry"), SectorOneBounds->IsValid != 0))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        Service.Reset();
        return false;
    }

    // Pick a handle whose authored sector is not SECT1 and whose current position is
    // outside the SECT1 bounds, so the move creates a real candidate disagreement.
    AKyaActorHandle* Handle = nullptr;
    int32 HandleIndex = INDEX_NONE;
    for (int32 Index = 0; Index < Bank->SceneEnvironment.Actors.Num(); ++Index)
    {
        AKyaActorHandle* Candidate = Cast<AKyaActorHandle>(Preview->GetActorHandle(Index));
        if (!Candidate) continue;
        const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[Index];
        if (Placement.SectorId == 1) continue;
        if (Preview->GetCandidateSectorIdsForPosition(Placement.Position).Contains(1)) continue;
        Handle = Candidate;
        HandleIndex = Index;
        break;
    }
    if (!TestNotNull(TEXT("An actor outside SECT1 is available"), Handle))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        Service.Reset();
        return false;
    }

    const int32 AuthoredSector = Bank->SceneEnvironment.Actors[HandleIndex].SectorId;
    const FString AssignmentBefore = Preview->DescribeSectorAssignment(AuthoredSector,
        Bank->SceneEnvironment.Actors[HandleIndex].Position);
    TestTrue(TEXT("The diagnostic states the explicit assignment"), AssignmentBefore.Contains(TEXT("Explicit assignment")));
    TestFalse(TEXT("The pre-move position is not inside SECT1"),
        Preview->GetCandidateSectorIdsForPosition(Bank->SceneEnvironment.Actors[HandleIndex].Position).Contains(1));

    // Move the representation into the loaded SECT1 background. This is the normal
    // gizmo commit path; it must not rewrite the authored SectorId.
    const FVector LocalTarget = SectorOneBounds->GetCenter();
    const FVector WorldTarget = Preview->GetActorTransform().TransformPosition(LocalTarget);
    const FVector GameTarget(LocalTarget.X / 100.0, LocalTarget.Z / 100.0, LocalTarget.Y / 100.0);
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "SectorMove", "Sector move"));
        Handle->AddActorWorldOffset(WorldTarget - Handle->GetActorLocation());
        Handle->PostEditMove(true);
        Service.FlushPendingNotifications();
    }
    const FKyaActorPlacement& Moved = Bank->SceneEnvironment.Actors[HandleIndex];
    TestEqual(TEXT("Moving an actor keeps its explicit sector"), Moved.SectorId, AuthoredSector);
    TestTrue(TEXT("Moving writes the new position"), Moved.Position.Equals(GameTarget, 1.0e-3));

    const TArray<int32> Candidates = Preview->GetCandidateSectorIdsForPosition(Moved.Position);
    TestTrue(TEXT("The moved position has SECT1 as a candidate sector"), Candidates.Contains(1));
    const FString AssignmentAfter = Preview->DescribeSectorAssignment(Moved.SectorId, Moved.Position);
    AddInfo(AssignmentAfter);
    TestTrue(TEXT("The diagnostic reports the SECT1 candidate"), AssignmentAfter.Contains(TEXT("sector 1")));
    TestTrue(TEXT("The diagnostic states it is not applied"), AssignmentAfter.Contains(TEXT("not applied")));
    TestTrue(TEXT("The diagnostic keeps the authored sector"),
        AssignmentAfter.Contains(FString::Printf(TEXT("sector %d"), AuthoredSector)) ||
        (AuthoredSector < 0 && AssignmentAfter.Contains(TEXT("global"))));
    if (AuthoredSector != 1)
        TestTrue(TEXT("A disagreeing candidate never rewrites membership"),
            AssignmentAfter.Contains(TEXT("never changed automatically")));

    // The explicit id also survives export/reimport.
    FKyaBankExportDiagnostics Diagnostics;
    TArray<uint8> Exported;
    if (TestTrue(TEXT("Export the sector-assignment bank"),
        KyaBuildBankExportBytes(Bank, Exported, Diagnostics)))
    {
        const uint8* ExportedCursor = Exported.GetData();
        UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
            UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
            TEXT("bnk"), ExportedCursor, Exported.GetData() + Exported.Num(), GWarn));
        if (TestNotNull(TEXT("Reimport the sector-assignment bank"), Reimported) &&
            TestTrue(TEXT("The reimported bank keeps the moved actor"),
                Reimported->SceneEnvironment.Actors.IsValidIndex(HandleIndex)))
        {
            TestEqual(TEXT("Export keeps the explicit sector"),
                Reimported->SceneEnvironment.Actors[HandleIndex].SectorId, AuthoredSector);
            TestTrue(TEXT("Export keeps the moved position"),
                Reimported->SceneEnvironment.Actors[HandleIndex].Position.Equals(GameTarget, 1.0e-3));
        }
    }

    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSaveReopenConsistencyTest, "Kya.Editor.SaveReopenConsistency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSaveReopenConsistencyTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the save/reopen consistency acceptance."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read the save/reopen bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();

    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    Factory->AddToRoot();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Imported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import the save/reopen bank"), Imported) ||
        !TestTrue(TEXT("The save/reopen bank has actors"), Imported->SceneEnvironment.Actors.Num() > 0))
    {
        Factory->RemoveFromRoot();
        return false;
    }

    const FString PackageName = FString::Printf(TEXT("/Game/KyaSaveReopenTest/Bank_%s"),
        *FGuid::NewGuid().ToString(EGuidFormats::Digits));
    UPackage* Package = CreatePackage(*PackageName);
    UKyaLevelBank* Bank = DuplicateObject<UKyaLevelBank>(Imported, Package, TEXT("Bank"));
    if (!TestNotNull(TEXT("Duplicate the save/reopen bank into a saveable package"), Bank))
    {
        Factory->RemoveFromRoot();
        return false;
    }
    Bank->ClearFlags(RF_Transient);
    Bank->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
    Package->ClearFlags(RF_Transient);
    Package->SetFlags(RF_Public | RF_Standalone);
    Bank->SourceFilename = Filename;
    if (!Bank->AssetImportData) Bank->AssetImportData = NewObject<UAssetImportData>(Bank);
    Bank->AssetImportData->Update(Bank->SourceFilename);
    Package->SetDirtyFlag(false);

    const FTransform PreviewTransformA(FRotator(0.0, 23.0, 0.0), FVector(1400.0, -600.0, 300.0));
    const FTransform PreviewTransformB(FRotator(0.0, -41.0, 0.0), FVector(-900.0, 750.0, 120.0));
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create the save/reopen world"), World))
    {
        Factory->RemoveFromRoot();
        return false;
    }
    const auto SpawnPreview = [World](UKyaLevelBank* InBank, const FTransform& Transform)
        -> AKyaLevelPreview*
    {
        AKyaLevelPreview* Spawned = World->SpawnActor<AKyaLevelPreview>(
            AKyaLevelPreview::StaticClass(), Transform);
        if (!Spawned) return nullptr;
        Spawned->Bank = InBank;
        Spawned->bShowRebuildProgress = false;
        Spawned->bSpawnActorHandles = true;
        Spawned->OnConstruction(Transform);
        return Spawned;
    };
    AKyaLevelPreview* PreviewA = SpawnPreview(Bank, PreviewTransformA);
    AKyaLevelPreview* PreviewB = SpawnPreview(Bank, PreviewTransformB);
    if (!TestNotNull(TEXT("First preview spawns"), PreviewA) ||
        !TestNotNull(TEXT("Second preview spawns"), PreviewB))
    {
        if (PreviewA) PreviewA->Destroy();
        if (PreviewB) PreviewB->Destroy();
        World->DestroyWorld(false);
        Factory->RemoveFromRoot();
        return false;
    }
    AKyaActorHandle* HandleA = Cast<AKyaActorHandle>(PreviewA->GetActorHandle(0));
    AKyaActorHandle* HandleB = Cast<AKyaActorHandle>(PreviewB->GetActorHandle(0));
    if (!TestNotNull(TEXT("First preview has the shared actor"), HandleA) ||
        !TestNotNull(TEXT("Second preview has the shared actor"), HandleB))
    {
        PreviewA->Destroy();
        PreviewB->Destroy();
        World->DestroyWorld(false);
        Factory->RemoveFromRoot();
        return false;
    }
    TestTrue(TEXT("Both previews show the same record"),
        HandleA->ActorPosition.Equals(HandleB->ActorPosition, 1.0e-4));

    const int32 TargetIndex = 0;
    const FKyaActorPlacement Original = Bank->SceneEnvironment.Actors[TargetIndex];
    const float DetailsValue = Original.VisibilityDistance + 1234.0f;

    // --- Details edit on the first preview, with undo and redo --------------------
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "DetailsEdit", "Details edit"));
        HandleA->ActorVisibilityDistance = DetailsValue;
        FPropertyChangedEvent ChangedEvent(FindFProperty<FProperty>(AKyaActorHandle::StaticClass(),
            GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorVisibilityDistance)));
        HandleA->PostEditChangeProperty(ChangedEvent);
        Service.FlushPendingNotifications();
    }
    TestEqual(TEXT("The Details edit writes the bank"), Bank->SceneEnvironment.Actors[TargetIndex].VisibilityDistance, DetailsValue);
    TestEqual(TEXT("The edited preview mirrors the Details value"), HandleA->ActorVisibilityDistance, DetailsValue);
    TestEqual(TEXT("The other preview mirrors the Details value"), HandleB->ActorVisibilityDistance, DetailsValue);
    TestTrue(TEXT("The Details edit keeps the placement"), Bank->SceneEnvironment.Actors[TargetIndex].Position.Equals(Original.Position, 1.0e-4));
    TestTrue(TEXT("The Details edit keeps the selected handle"), PreviewA->GetActorHandle(TargetIndex) == HandleA);

    if (GEditor)
    {
        TestTrue(TEXT("Undo returns the Details transaction"), GEditor->UndoTransaction());
        Service.FlushPendingNotifications();
        TestEqual(TEXT("Undo restores the Details value"), Bank->SceneEnvironment.Actors[TargetIndex].VisibilityDistance, Original.VisibilityDistance);
        TestEqual(TEXT("Undo refreshes the first preview"), HandleA->ActorVisibilityDistance, Original.VisibilityDistance);
        TestEqual(TEXT("Undo refreshes the second preview"), HandleB->ActorVisibilityDistance, Original.VisibilityDistance);
        GEditor->RedoTransaction();
        Service.FlushPendingNotifications();
        TestEqual(TEXT("Redo reapplies the Details value"), Bank->SceneEnvironment.Actors[TargetIndex].VisibilityDistance, DetailsValue);
        TestTrue(TEXT("Redo refreshes both previews"),
            HandleA->ActorVisibilityDistance == DetailsValue && HandleB->ActorVisibilityDistance == DetailsValue);
    }

    // --- rapid consecutive property edits in one transaction ----------------------
    const float RapidStart = Bank->SceneEnvironment.Actors[TargetIndex].VisibilityDistance;
    const int32 RapidEdits = 25;
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "RapidEdits", "Rapid property edits"));
        for (int32 Edit = 1; Edit <= RapidEdits; ++Edit)
        {
            FKyaActorEditPatch Patch;
            Patch.VisibilityDistance = RapidStart + float(Edit);
            FString Error;
            TestTrue(TEXT("Each rapid property edit applies"),
                Service.Apply(Service.MakeKey(Bank, TargetIndex), Patch, EKyaEditNotify::Deferred, &Error));
        }
        Service.FlushPendingNotifications();
    }
    const float RapidEnd = RapidStart + float(RapidEdits);
    TestEqual(TEXT("Rapid edits leave the final value"), Bank->SceneEnvironment.Actors[TargetIndex].VisibilityDistance, RapidEnd);
    TestEqual(TEXT("Rapid edits refresh the first preview"), HandleA->ActorVisibilityDistance, RapidEnd);
    TestEqual(TEXT("Rapid edits refresh the second preview"), HandleB->ActorVisibilityDistance, RapidEnd);
    TestTrue(TEXT("Rapid edits keep the first handle"), PreviewA->GetActorHandle(TargetIndex) == HandleA);
    TestTrue(TEXT("Rapid edits keep the second handle"), PreviewB->GetActorHandle(TargetIndex) == HandleB);
    if (GEditor)
    {
        TestTrue(TEXT("Undo returns the rapid-edit transaction"), GEditor->UndoTransaction());
        Service.FlushPendingNotifications();
        TestEqual(TEXT("One undo reverts every rapid edit"), Bank->SceneEnvironment.Actors[TargetIndex].VisibilityDistance, RapidStart);
        TestEqual(TEXT("The first preview shows the undone value"), HandleA->ActorVisibilityDistance, RapidStart);
        TestEqual(TEXT("The second preview shows the undone value"), HandleB->ActorVisibilityDistance, RapidStart);
        GEditor->RedoTransaction();
        Service.FlushPendingNotifications();
        TestEqual(TEXT("Redo restores every rapid edit"), Bank->SceneEnvironment.Actors[TargetIndex].VisibilityDistance, RapidEnd);
    }

    // --- save, close and reopen from disk -----------------------------------------
    const FKyaActorPlacement Expected = Bank->SceneEnvironment.Actors[TargetIndex];
    const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName,
        FPackageName::GetAssetPackageExtension());
    Package->MarkPackageDirty();
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    const bool bSaved = UPackage::SavePackage(Package, Bank, *PackageFilename, SaveArgs);
    TestTrue(TEXT("The edited save/reopen bank saves"), bSaved);

    PreviewA->Destroy();
    PreviewB->Destroy();
    World->DestroyWorld(false);
    Service.Reset();

    FText ReloadError;
    const bool bReloaded = bSaved && UPackageTools::ReloadPackages({ Package }, ReloadError,
        EReloadPackagesInteractionMode::AssumePositive);
    TestTrue(FString::Printf(TEXT("The edited save/reopen bank reopens from disk (%s)"), *ReloadError.ToString()),
        bReloaded);
    UPackage* LoadedPackage = FindPackage(nullptr, *PackageName);
    UKyaLevelBank* LoadedBank = LoadedPackage ? FindObject<UKyaLevelBank>(LoadedPackage, TEXT("Bank")) : nullptr;
    if (TestNotNull(TEXT("The reopened package holds the bank"), LoadedBank))
    {
        const FKyaActorPlacement& Reopened = LoadedBank->SceneEnvironment.Actors[TargetIndex];
        TestEqual(TEXT("Reopen preserves the edited Details value"), Reopened.VisibilityDistance, Expected.VisibilityDistance);
        TestTrue(TEXT("Reopen preserves the placement"), Reopened.Position.Equals(Expected.Position, 1.0e-4) &&
            Reopened.Rotation.Equals(Expected.Rotation, 1.0e-5) && Reopened.Scale.Equals(Expected.Scale, 1.0e-4));

        UWorld* ReopenedWorld = UWorld::CreateWorld(EWorldType::EditorPreview, false);
        if (TestNotNull(TEXT("Create the reopened world"), ReopenedWorld))
        {
            const auto SpawnReopened = [ReopenedWorld](UKyaLevelBank* InBank, const FTransform& Transform)
                -> AKyaLevelPreview*
            {
                AKyaLevelPreview* Spawned = ReopenedWorld->SpawnActor<AKyaLevelPreview>(
                    AKyaLevelPreview::StaticClass(), Transform);
                if (!Spawned) return nullptr;
                Spawned->Bank = InBank;
                Spawned->bShowRebuildProgress = false;
                Spawned->bSpawnActorHandles = true;
                Spawned->OnConstruction(Transform);
                return Spawned;
            };
            AKyaLevelPreview* ReopenedA = SpawnReopened(LoadedBank, PreviewTransformA);
            AKyaLevelPreview* ReopenedB = SpawnReopened(LoadedBank, PreviewTransformB);
            if (TestNotNull(TEXT("The reopened first preview spawns"), ReopenedA) &&
                TestNotNull(TEXT("The reopened second preview spawns"), ReopenedB))
            {
                FTransform Forward;
                KyaActorTransform::MakeUnrealTransform(Expected.Position, Expected.Rotation,
                    Expected.Scale, Expected.ClassId, Forward);
                for (AKyaLevelPreview* ReopenedPreview : { ReopenedA, ReopenedB })
                {
                    AKyaActorHandle* Handle = Cast<AKyaActorHandle>(ReopenedPreview->GetActorHandle(TargetIndex));
                    if (!TestNotNull(TEXT("The reopened preview shows the actor"), Handle)) continue;
                    TestEqual(TEXT("Reopened Details show the saved value"),
                        Handle->ActorVisibilityDistance, Expected.VisibilityDistance);
                    TestTrue(TEXT("Reopened Details show the saved position"),
                        Handle->ActorPosition.Equals(Expected.Position, 1.0e-4));
                    const FTransform Local = Handle->GetActorTransform().GetRelativeTransform(
                        ReopenedPreview->GetActorTransform());
                    TestTrue(TEXT("The reopened rendered placement matches the record"),
                        Local.GetLocation().Equals(Forward.GetLocation(), 0.1) &&
                        Local.GetRotation().Equals(Forward.GetRotation(), 1.0e-4) &&
                        Local.GetScale3D().Equals(Forward.GetScale3D(), 1.0e-4));
                }
            }
            if (ReopenedA) ReopenedA->Destroy();
            if (ReopenedB) ReopenedB->Destroy();
            ReopenedWorld->DestroyWorld(false);
        }
    }

    if (LoadedPackage) LoadedPackage->SetDirtyFlag(false);
    IFileManager::Get().Delete(*PackageFilename, false, true, true);
    Service.Reset();
    Factory->RemoveFromRoot();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaRepresentativeActorEditsTest, "Kya.Editor.RepresentativeActorEdits",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaRepresentativeActorEditsTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the representative-actor acceptance."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read the representative-actor bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import the representative-actor bank"), Bank)) return false;

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create the representative-actor world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn the representative-actor preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    enum class ERepKind : uint8 { Platform, Wolfen, Hero, Trigger, Meshless };
    struct FRepSpec
    {
        const TCHAR* Label;
        int64 ClassId;
        ERepKind Kind;
    };
    // Representative classes from Types.h: moving platform, Wolfen, hero, checkpoint
    // manager (the trigger records) and a meshless placement.
    const FRepSpec Specs[] = {
        { TEXT("platform"), 0x0d, ERepKind::Platform },
        { TEXT("Wolfen"), 0x09, ERepKind::Wolfen },
        { TEXT("hero"), 0x06, ERepKind::Hero },
        { TEXT("trigger"), 0x1e, ERepKind::Trigger },
        { TEXT("meshless"), INDEX_NONE, ERepKind::Meshless },
    };

    struct FRepEdit
    {
        FName Property;
        FString Leaf;
        TFunction<void(FKyaActorPlacement&)> Mutate;
    };
    const auto MakePlan = [](ERepKind Kind, const FKyaActorPlacement& Placement, FRepEdit& Out) -> bool
    {
        switch (Kind)
        {
        case ERepKind::Platform:
            if (Placement.bHasPlatformPrefix && !Placement.PlatformActors.IsEmpty())
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformActors);
                Out.Leaf = TEXT("PlatformActors[0].CarryMethod");
                Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.PlatformActors[0].CarryMethod ^= 1; };
                return true;
            }
            if (Placement.bHasPlatformDetails)
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformProperties);
                Out.Leaf = TEXT("PlatformProperties.PlatformFlags");
                Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.PlatformProperties.PlatformFlags ^= 1; };
                return true;
            }
            if (Placement.bHasPlatformPrefix)
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformWeight);
                Out.Leaf = TEXT("PlatformWeight");
                Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.PlatformWeight += 3.25f; };
                return true;
            }
            return false;
        case ERepKind::Wolfen:
            if (Placement.bHasWolfenData && !Placement.WolfenFightParamData.ConfigurationValues.IsEmpty())
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, WolfenFightParamData);
                Out.Leaf = TEXT("WolfenFightParamData.ConfigurationValues[0]");
                Out.Mutate = [](FKyaActorPlacement& Copy)
                { Copy.WolfenFightParamData.ConfigurationValues[0] += 4.5f; };
                return true;
            }
            if (!Placement.WolfenFightParamData.TuningValues.IsEmpty())
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, WolfenFightParamData);
                Out.Leaf = TEXT("WolfenFightParamData.TuningValues[0]");
                Out.Mutate = [](FKyaActorPlacement& Copy)
                { Copy.WolfenFightParamData.TuningValues[0] += 2.25f; };
                return true;
            }
            if (Placement.bHasWolfenData)
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, WolfenVisionRange);
                Out.Leaf = TEXT("WolfenVisionRange");
                Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.WolfenVisionRange += 6.0f; };
                return true;
            }
            return false;
        case ERepKind::Hero:
            if (Placement.bHasKimData && !Placement.FighterBlows.IsEmpty())
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, FighterBlows);
                Out.Leaf = TEXT("FighterBlows[0].Damage");
                Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.FighterBlows[0].Damage += 7.5f; };
                return true;
            }
            if (Placement.bHasKimData && !Placement.KimCameras.IsEmpty())
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, KimCameras);
                Out.Leaf = TEXT("KimCameras[0].Fov");
                Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.KimCameras[0].Fov += 2.0f; };
                return true;
            }
            if (Placement.bHasKimData && !Placement.KimBoneKeys.IsEmpty())
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, KimBoneKeys);
                Out.Leaf = TEXT("KimBoneKeys[0]");
                Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.KimBoneKeys[0] += 1; };
                return true;
            }
            if (Placement.bHasKimData)
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, FighterWeightB);
                Out.Leaf = TEXT("FighterWeightB");
                Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.FighterWeightB += 1.25f; };
                return true;
            }
            return false;
        case ERepKind::Trigger:
            if (Placement.bHasTypedCheckpoints && !Placement.Checkpoints.IsEmpty())
            {
                Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Checkpoints);
                Out.Leaf = TEXT("Checkpoints[0].Flags");
                Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.Checkpoints[0].Flags ^= 2; };
                return true;
            }
            return false;
        case ERepKind::Meshless:
            if (!Placement.Models.IsEmpty() || Placement.MeshIndex >= 0) return false;
            Out.Property = GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, VisibilityDistance);
            Out.Leaf = TEXT("VisibilityDistance");
            Out.Mutate = [](FKyaActorPlacement& Copy) { Copy.VisibilityDistance += 17.0f; };
            return true;
        }
        return false;
    };

    struct FEditedRecord
    {
        int32 Index = INDEX_NONE;
        FName Property;
        FString Leaf;
        FKyaActorPlacement Expected;
    };
    TArray<FEditedRecord> Edited;
    int32 NestedEdits = 0;

    for (const FRepSpec& Spec : Specs)
    {
        // Find the representative through its generated representation/Outliner label,
        // never by scanning the bank's global actor array for a class id.
        AKyaActorHandle* Found = nullptr;
        FRepEdit Chosen;
        if (Spec.ClassId != INDEX_NONE)
        {
            const FString ClassTag = FString::Printf(TEXT("[class %lld]"), Spec.ClassId);
            for (TActorIterator<AKyaActorHandle> It(World); It; ++It)
            {
                if (!Bank->SceneEnvironment.Actors.IsValidIndex(It->ActorIndex)) continue;
                if (!It->GetActorLabel().Contains(ClassTag)) continue;
                FRepEdit Candidate;
                if (!MakePlan(Spec.Kind, Bank->SceneEnvironment.Actors[It->ActorIndex], Candidate)) continue;
                Found = *It;
                Chosen = Candidate;
                break;
            }
        }
        else
        {
            for (TActorIterator<AKyaActorHandle> It(World); It; ++It)
            {
                if (!Bank->SceneEnvironment.Actors.IsValidIndex(It->ActorIndex)) continue;
                if (It->GetRenderedSectionCount() != 0) continue;
                FRepEdit Candidate;
                if (!MakePlan(Spec.Kind, Bank->SceneEnvironment.Actors[It->ActorIndex], Candidate)) continue;
                Found = *It;
                Chosen = Candidate;
                break;
            }
        }
        if (!TestNotNull(FString::Printf(TEXT("The %s actor is discoverable from its Outliner label"), Spec.Label), Found))
        {
            int32 ClassCount = 0;
            for (const FKyaActorPlacement& Placement : Bank->SceneEnvironment.Actors)
                if (Spec.ClassId == INDEX_NONE ? Placement.Models.IsEmpty() : Placement.ClassId == Spec.ClassId)
                    ++ClassCount;
            AddInfo(FString::Printf(TEXT("No applicable %s representation (bank carries %d matching records)"),
                Spec.Label, ClassCount));
            continue;
        }

        const int32 Index = Found->ActorIndex;
        const FKyaActorPlacement Before = Bank->SceneEnvironment.Actors[Index];
        FKyaActorPlacement Copy = Before;
        Chosen.Mutate(Copy);
        FKyaActorEditPatch Patch;
        FString Error;
        if (!TestTrue(FString::Printf(TEXT("The %s nested patch builds (%s)"), Spec.Label, *Chosen.Leaf),
            KyaMakePlacementPropertyPatch(Copy, Chosen.Property, Patch, &Error)))
        {
            AddInfo(Error);
            continue;
        }
        if (!TestTrue(FString::Printf(TEXT("The %s nested edit applies"), Spec.Label),
            Service.Apply(Service.MakeKey(Bank, Index), Patch, EKyaEditNotify::Notify, &Error)))
        {
            AddInfo(Error);
            continue;
        }
        Service.FlushPendingNotifications();
        const FKyaActorPlacement& After = Bank->SceneEnvironment.Actors[Index];
        FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(Chosen.Property);
        if (TestNotNull(FString::Printf(TEXT("The %s edit targets a real property"), Spec.Label), Property))
        {
            TestFalse(FString::Printf(TEXT("The %s edit changes the nested value"), Spec.Label),
                Property->Identical(Property->ContainerPtrToValuePtr<void>(&Before),
                    Property->ContainerPtrToValuePtr<void>(&After), 0));
            TestTrue(FString::Printf(TEXT("The %s edit preserves the placement"), Spec.Label),
                After.Position.Equals(Before.Position, 1.0e-4) && After.ClassId == Before.ClassId);
        }
        TestTrue(FString::Printf(TEXT("The %s edit keeps the actor discoverable"), Spec.Label),
            Found->GetActorLabel().Contains(FString::Printf(TEXT("[class %lld]"), Before.ClassId)));
        if (Chosen.Leaf.Contains(TEXT(".")) || Chosen.Leaf.Contains(TEXT("["))) ++NestedEdits;
        AddInfo(FString::Printf(TEXT("Edited %s actor %d (%s): %s"),
            Spec.Label, Index, *Found->GetActorLabel(), *Chosen.Leaf));
        Edited.Add({ Index, Chosen.Property, Chosen.Leaf, After });
    }

    TestEqual(TEXT("Every representative actor was found and edited"), Edited.Num(),
        int32(UE_ARRAY_COUNT(Specs)));
    TestTrue(TEXT("Most representative edits reach a nested leaf"), NestedEdits >= 3);

    // --- verify the corresponding exported values --------------------------------
    FKyaBankExportDiagnostics Diagnostics;
    TArray<uint8> Exported;
    if (TestTrue(TEXT("Export the representative-actor bank"),
        KyaBuildBankExportBytes(Bank, Exported, Diagnostics)))
    {
        const uint8* ExportedCursor = Exported.GetData();
        UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
            UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
            TEXT("bnk"), ExportedCursor, Exported.GetData() + Exported.Num(), GWarn));
        if (TestNotNull(TEXT("Reimport the representative-actor bank"), Reimported))
        {
            TestEqual(TEXT("Export preserves the actor count"),
                Reimported->SceneEnvironment.Actors.Num(), Bank->SceneEnvironment.Actors.Num());
            for (const FEditedRecord& Edit : Edited)
            {
                if (!TestTrue(FString::Printf(TEXT("The exported bank keeps the %s actor"), *Edit.Leaf),
                    Reimported->SceneEnvironment.Actors.IsValidIndex(Edit.Index))) continue;
                FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(Edit.Property);
                if (!TestNotNull(FString::Printf(TEXT("The %s property resolves"), *Edit.Leaf), Property)) continue;
                const FKyaActorPlacement& ExportedActor = Reimported->SceneEnvironment.Actors[Edit.Index];
                TestTrue(FString::Printf(TEXT("Export preserves the edited %s"), *Edit.Leaf),
                    Property->Identical(Property->ContainerPtrToValuePtr<void>(&ExportedActor),
                        Property->ContainerPtrToValuePtr<void>(&Edit.Expected), 0));
                TestTrue(FString::Printf(TEXT("Export preserves the %s placement"), *Edit.Leaf),
                    ExportedActor.Position.Equals(Edit.Expected.Position, 1.0e-4) &&
                    ExportedActor.ClassId == Edit.Expected.ClassId);
            }
        }
    }

    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGizmoAcceptanceTest, "Kya.Editor.GizmoAcceptance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaGizmoAcceptanceTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the gizmo acceptance."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read the gizmo bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();

    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    Factory->AddToRoot();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Imported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import the gizmo bank"), Imported) ||
        !TestTrue(TEXT("The gizmo bank has actors"), Imported->SceneEnvironment.Actors.Num() > 0))
    {
        Factory->RemoveFromRoot();
        return false;
    }

    // The gate saves and reopens the bank, so edit a real package instead of the
    // transient import.
    const FString PackageName = FString::Printf(TEXT("/Game/KyaGizmoAcceptance/Bank_%s"),
        *FGuid::NewGuid().ToString(EGuidFormats::Digits));
    UPackage* Package = CreatePackage(*PackageName);
    UKyaLevelBank* Bank = DuplicateObject<UKyaLevelBank>(Imported, Package, TEXT("Bank"));
    if (!TestNotNull(TEXT("Duplicate the gizmo bank into a saveable package"), Bank))
    {
        Factory->RemoveFromRoot();
        return false;
    }
    Bank->ClearFlags(RF_Transient);
    Bank->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
    Package->ClearFlags(RF_Transient);
    Package->SetFlags(RF_Public | RF_Standalone);
    Bank->SourceFilename = Filename;
    if (!Bank->AssetImportData) Bank->AssetImportData = NewObject<UAssetImportData>(Bank);
    Bank->AssetImportData->Update(Bank->SourceFilename);
    Package->SetDirtyFlag(false);

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create the gizmo world"), World))
    {
        Factory->RemoveFromRoot();
        return false;
    }
    // A nonidentity preview root: every gizmo edit below is resolved relative to it.
    const FTransform PreviewTransform(FRotator(0.0, 37.0, 0.0), FVector(5200.0, -3100.0, 1250.0));
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>(
        AKyaLevelPreview::StaticClass(), PreviewTransform);
    if (!TestNotNull(TEXT("Spawn the gizmo preview"), Preview))
    {
        World->DestroyWorld(false);
        Factory->RemoveFromRoot();
        return false;
    }
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = true;
    Preview->OnConstruction(PreviewTransform);
    const FTransform PreviewWorld = Preview->GetActorTransform();
    TestTrue(TEXT("The preview keeps its placed transform"), PreviewWorld.GetLocation().Equals(PreviewTransform.GetLocation(), 0.01));
    if (!TestTrue(TEXT("The gizmo preview has generated handles"), Preview->GetActorHandleCount() > 0))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        Factory->RemoveFromRoot();
        return false;
    }

    const auto LocalOf = [Preview](const AKyaActorHandle* Handle)
    {
        return Handle->GetActorTransform().GetRelativeTransform(Preview->GetActorTransform());
    };
    const auto ForwardOf = [](const FKyaActorPlacement& Placement, FTransform& OutTransform)
    {
        KyaActorTransform::MakeUnrealTransform(Placement.Position, Placement.Rotation,
            Placement.Scale, Placement.ClassId, OutTransform);
    };
    const auto FindHandle = [Bank, Preview](bool bYawOnly, bool bRequireMesh) -> AKyaActorHandle*
    {
        for (int32 Index = 0; Index < Bank->SceneEnvironment.Actors.Num(); ++Index)
        {
            const bool bIsYawOnly = KyaActorTransform::RotationPath(
                Bank->SceneEnvironment.Actors[Index].ClassId) == KyaActorTransform::ERotationPath::YawOnly;
            if (bIsYawOnly != bYawOnly) continue;
            AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Preview->GetActorHandle(Index));
            if (!Handle) continue;
            if (bRequireMesh && Handle->GetRenderedSectionCount() == 0) continue;
            return Handle;
        }
        return nullptr;
    };

    // --- W: click a visible mesh, then translate it --------------------------------
    AKyaActorHandle* Clicked = nullptr;
    AKyaActorHandle* ClickCandidate = nullptr;
    FVector ClickOrigin = FVector::ZeroVector;
    const FVector ClickDirection(0.0, 0.0, -1.0);
    for (int32 Index = 0; Index < Bank->SceneEnvironment.Actors.Num() && !Clicked; ++Index)
    {
        AKyaActorHandle* Candidate = Cast<AKyaActorHandle>(Preview->GetActorHandle(Index));
        if (!Candidate || Candidate->GetRenderedSectionCount() == 0) continue;
        const FBox Bounds = Candidate->GetRenderedMeshBounds();
        if (!Bounds.IsValid) continue;
        const FVector Center = Bounds.GetCenter();
        const FVector Extent = Bounds.GetExtent();
        for (const double OffsetX : { -0.35, 0.0, 0.35 })
        {
            for (const double OffsetY : { -0.35, 0.0, 0.35 })
            {
                const FVector Sample(Center.X + Extent.X * OffsetX, Center.Y + Extent.Y * OffsetY,
                    Bounds.Max.Z + 50.0);
                if (AKyaActorHandle* Hit = Preview->HitTestActorMesh(Sample, ClickDirection))
                {
                    Clicked = Hit;
                    ClickCandidate = Candidate;
                    ClickOrigin = Sample;
                    break;
                }
            }
            if (Clicked) break;
        }
    }
    if (!TestNotNull(TEXT("Clicking a visible generated mesh hits an actor"), Clicked) ||
        !TestEqual(TEXT("The nearest visible mesh resolves to its own handle"), Clicked, ClickCandidate))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        Factory->RemoveFromRoot();
        return false;
    }
    TestTrue(TEXT("The clicked actor carries renderable geometry"), Clicked->GetRenderedSectionCount() > 0);
    TestTrue(TEXT("The clicked actor maps to a bank record"),
        Bank->SceneEnvironment.Actors.IsValidIndex(Clicked->ActorIndex));
    AddInfo(FString::Printf(TEXT("Gizmo click target: actor %d, class %lld, %d sections"),
        Clicked->ActorIndex, Bank->SceneEnvironment.Actors[Clicked->ActorIndex].ClassId,
        Clicked->GetRenderedSectionCount()));
    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        AKyaActorHandle* Picked = KyaSelectRenderedActorAt(Preview, ClickOrigin, ClickDirection);
        TestEqual(TEXT("The editor click path selects the clicked mesh"), Picked, Clicked);
        TestTrue(TEXT("The clicked mesh is selected so the gizmo targets it"), Clicked->IsSelected());
        TestTrue(TEXT("The clicked mesh is highlighted"), Clicked->IsMeshHighlighted());
    }
    TestFalse(TEXT("Selecting does not dirty the bank"), Package->IsDirty());

    const int32 ClickIndex = Clicked->ActorIndex;
    const FKyaActorPlacement ClickBefore = Bank->SceneEnvironment.Actors[ClickIndex];
    const FVector MoveOffset(250.0, -150.0, 100.0);
    const FVector InitialWorldLocation = Clicked->GetActorLocation();
    FVector IntendedLocalLocation;
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "GizmoTranslate", "Gizmo translate"));
        Clicked->AddActorWorldOffset(MoveOffset);
        const FVector IntendedWorldLocation = Clicked->GetActorLocation();
        IntendedLocalLocation = PreviewWorld.InverseTransformPosition(IntendedWorldLocation);
        TestTrue(TEXT("The translate gizmo moved the visible actor"),
            IntendedWorldLocation.Equals(InitialWorldLocation + MoveOffset, 0.01));
        Clicked->PostEditMove(true);
        Service.FlushPendingNotifications();
    }
    {
        const FKyaActorPlacement& After = Bank->SceneEnvironment.Actors[ClickIndex];
        const FVector ExpectedPosition = KyaActorTransform::SwapGameAxes(IntendedLocalLocation / 100.0);
        TestTrue(TEXT("Translate writes the intended game-space position"), After.Position.Equals(ExpectedPosition, 1.0e-3));
        TestFalse(TEXT("Translate actually changed the record"), After.Position.Equals(ClickBefore.Position, 1.0e-4));
        TestTrue(TEXT("Translate preserves the record rotation"), After.Rotation.Equals(ClickBefore.Rotation, 1.0e-5));
        TestTrue(TEXT("Translate preserves the record scale"), After.Scale.Equals(ClickBefore.Scale, 1.0e-4));
        FTransform RoundTrip;
        ForwardOf(After, RoundTrip);
        TestTrue(TEXT("The stored position reopens at the intended placement"),
            RoundTrip.GetLocation().Equals(IntendedLocalLocation, 0.1));
        TestTrue(TEXT("The handle mirrors the stored position"),
            Clicked->GetActorLocation().Equals(PreviewWorld.TransformPosition(IntendedLocalLocation), 0.1));
    }

    // --- E: rotate a class that supports a full basis, and a yaw-only class ---------
    AKyaActorHandle* RotateHandle = (KyaActorTransform::RotationPath(ClickBefore.ClassId) !=
        KyaActorTransform::ERotationPath::YawOnly) ? Clicked : FindHandle(false, true);
    if (!RotateHandle) RotateHandle = FindHandle(false, false);
    if (TestNotNull(TEXT("A non-yaw actor can be rotated"), RotateHandle))
    {
        const int32 RotateIndex = RotateHandle->ActorIndex;
        AddInfo(FString::Printf(TEXT("Gizmo rotate target: actor %d, class %lld"),
            RotateIndex, Bank->SceneEnvironment.Actors[RotateIndex].ClassId));
        const FKyaActorPlacement RotateBefore = Bank->SceneEnvironment.Actors[RotateIndex];
        const FQuat IntendedRotation = LocalOf(RotateHandle).GetRotation() *
            FQuat(FVector::UpVector, 0.4);
        {
            FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "GizmoRotate", "Gizmo rotate"));
            RotateHandle->SetActorRelativeRotation(IntendedRotation);
            RotateHandle->PostEditMove(true);
            Service.FlushPendingNotifications();
        }
        const FKyaActorPlacement& After = Bank->SceneEnvironment.Actors[RotateIndex];
        TestFalse(TEXT("Rotate writes a new record rotation"), After.Rotation.Equals(RotateBefore.Rotation, 1.0e-4));
        TestTrue(TEXT("Rotate preserves the record position"), After.Position.Equals(RotateBefore.Position, 1.0e-4));
        TestTrue(TEXT("Rotate preserves the record scale"), After.Scale.Equals(RotateBefore.Scale, 1.0e-4));
        FTransform RoundTrip;
        ForwardOf(After, RoundTrip);
        TestTrue(TEXT("The stored rotation reproduces the gizmo orientation"),
            (RoundTrip.GetRotation()).AngularDistance(IntendedRotation) < 2.0e-3);
        TestTrue(TEXT("The handle mirrors the stored rotation"),
            (LocalOf(RotateHandle).GetRotation()).AngularDistance(RoundTrip.GetRotation()) < 2.0e-3);
    }

    AKyaActorHandle* YawHandle = FindHandle(true, true);
    if (!YawHandle) YawHandle = FindHandle(true, false);
    if (TestNotNull(TEXT("The bank has a yaw-only actor"), YawHandle))
    {
        const int32 YawIndex = YawHandle->ActorIndex;
        AddInfo(FString::Printf(TEXT("Gizmo yaw-only target: actor %d, class %lld"),
            YawIndex, Bank->SceneEnvironment.Actors[YawIndex].ClassId));
        const FKyaActorPlacement YawBefore = Bank->SceneEnvironment.Actors[YawIndex];
        const FQuat IntendedYaw = LocalOf(YawHandle).GetRotation() *
            FQuat(FVector::UpVector, 0.6);
        {
            FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "GizmoYaw", "Gizmo yaw"));
            YawHandle->SetActorRelativeRotation(IntendedYaw);
            YawHandle->PostEditMove(true);
            Service.FlushPendingNotifications();
        }
        const FVector YawAfterRotation = Bank->SceneEnvironment.Actors[YawIndex].Rotation;
        TestFalse(TEXT("Yaw-only rotation writes a new record rotation"),
            YawAfterRotation.Equals(YawBefore.Rotation, 1.0e-4));
        TestTrue(TEXT("Yaw-only rotation stores no pitch"),
            FMath::Abs(YawAfterRotation.X) < 1.0e-5);
        TestTrue(TEXT("Yaw-only rotation stores no roll"),
            FMath::Abs(YawAfterRotation.Z) < 1.0e-5);
        TestTrue(TEXT("Yaw-only rotation preserves the record position"),
            Bank->SceneEnvironment.Actors[YawIndex].Position.Equals(YawBefore.Position, 1.0e-4));
        FTransform RoundTrip;
        ForwardOf(Bank->SceneEnvironment.Actors[YawIndex], RoundTrip);
        TestTrue(TEXT("The stored yaw reproduces the gizmo orientation"),
            (RoundTrip.GetRotation()).AngularDistance(IntendedYaw) < 2.0e-3);

        // A pitch edit cannot be represented by the class and must be rejected.
        {
            FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "GizmoYawReject", "Gizmo yaw reject"));
            YawHandle->SetActorRelativeRotation(IntendedYaw * FQuat(FVector::RightVector, 0.3));
            YawHandle->PostEditMove(true);
            Service.FlushPendingNotifications();
        }
        TestTrue(TEXT("An unrepresentable yaw-only pitch is rejected"),
            Bank->SceneEnvironment.Actors[YawIndex].Rotation.Equals(YawAfterRotation, 1.0e-5));
        TestTrue(TEXT("A rejected edit restores the handle from the record"),
            (LocalOf(YawHandle).GetRotation()).AngularDistance(RoundTrip.GetRotation()) < 2.0e-3);
    }

    // --- R: scale the clicked actor, preserving its stored placement ---------------
    {
        const FKyaActorPlacement ScaleBefore = Bank->SceneEnvironment.Actors[ClickIndex];
        const FQuat RotationBeforeScale = LocalOf(Clicked).GetRotation();
        const FVector IntendedScale = LocalOf(Clicked).GetScale3D() * FVector(1.5, 0.8, 2.0);
        {
            FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "GizmoScale", "Gizmo scale"));
            Clicked->SetActorRelativeScale3D(IntendedScale);
            Clicked->PostEditMove(true);
            Service.FlushPendingNotifications();
        }
        const FKyaActorPlacement& After = Bank->SceneEnvironment.Actors[ClickIndex];
        TestTrue(TEXT("Scale writes the intended game-space scale"),
            After.Scale.Equals(KyaActorTransform::SwapGameAxes(IntendedScale), 1.0e-4));
        TestTrue(TEXT("Scale preserves the record position"), After.Position.Equals(ScaleBefore.Position, 1.0e-4));
        TestTrue(TEXT("Scale preserves the record rotation"), After.Rotation.Equals(ScaleBefore.Rotation, 1.0e-5));
        FTransform RoundTrip;
        ForwardOf(After, RoundTrip);
        TestTrue(TEXT("The stored scale reproduces the gizmo scale"), RoundTrip.GetScale3D().Equals(IntendedScale, 1.0e-4));
        TestTrue(TEXT("The handle keeps its rotation through the scale edit"),
            (LocalOf(Clicked).GetRotation()).AngularDistance(RotationBeforeScale) < 1.0e-4);
        TestTrue(TEXT("The handle mirrors the stored scale"),
            LocalOf(Clicked).GetScale3D().Equals(IntendedScale, 1.0e-4));
    }

    // --- persistence: save, reopen, export and reimport ----------------------------
    TSet<int32> EditedIndexes;
    EditedIndexes.Add(ClickIndex);
    if (RotateHandle) EditedIndexes.Add(RotateHandle->ActorIndex);
    if (YawHandle) EditedIndexes.Add(YawHandle->ActorIndex);
    TMap<int32, FKyaActorPlacement> Expected;
    for (const int32 Index : EditedIndexes)
        Expected.Add(Index, Bank->SceneEnvironment.Actors[Index]);

    const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName,
        FPackageName::GetAssetPackageExtension());
    Package->MarkPackageDirty();
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    const bool bSaved = UPackage::SavePackage(Package, Bank, *PackageFilename, SaveArgs);
    TestTrue(TEXT("The edited gizmo bank saves"), bSaved);

    // Drop editor selection before the unload: a selected transient handle would keep
    // typed-element references alive past the preview and the package reload.
    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        GEditor->NoteSelectionChange();
    }
    Preview->Destroy();
    World->DestroyWorld(false);

    FText ReloadError;
    const bool bReloaded = bSaved && UPackageTools::ReloadPackages({ Package }, ReloadError,
        EReloadPackagesInteractionMode::AssumePositive);
    TestTrue(FString::Printf(TEXT("The edited gizmo bank reopens from disk (%s)"), *ReloadError.ToString()),
        bReloaded);
    UPackage* LoadedPackage = FindPackage(nullptr, *PackageName);
    UKyaLevelBank* LoadedBank = LoadedPackage ? FindObject<UKyaLevelBank>(LoadedPackage, TEXT("Bank")) : nullptr;
    if (TestNotNull(TEXT("The reopened package holds the bank"), LoadedBank))
    {
        for (const TPair<int32, FKyaActorPlacement>& Pair : Expected)
        {
            if (!TestTrue(TEXT("The reopened bank keeps the edited actor"), LoadedBank->SceneEnvironment.Actors.IsValidIndex(Pair.Key)))
                continue;
            const FKyaActorPlacement& Reopened = LoadedBank->SceneEnvironment.Actors[Pair.Key];
            TestTrue(TEXT("Reopen preserves the edited position"), Reopened.Position.Equals(Pair.Value.Position, 1.0e-3));
            TestTrue(TEXT("Reopen preserves the edited rotation"), Reopened.Rotation.Equals(Pair.Value.Rotation, 1.0e-5));
            TestTrue(TEXT("Reopen preserves the edited scale"), Reopened.Scale.Equals(Pair.Value.Scale, 1.0e-4));
            FTransform RoundTrip;
            ForwardOf(Reopened, RoundTrip);
            FTransform ExpectedTransform;
            ForwardOf(Pair.Value, ExpectedTransform);
            TestTrue(TEXT("The reopened record renders at the intended placement"),
                RoundTrip.GetLocation().Equals(ExpectedTransform.GetLocation(), 0.1) &&
                (RoundTrip.GetRotation()).AngularDistance(ExpectedTransform.GetRotation()) < 2.0e-3 &&
                RoundTrip.GetScale3D().Equals(ExpectedTransform.GetScale3D(), 1.0e-4));
        }

        FKyaBankExportDiagnostics Diagnostics;
        TArray<uint8> Exported;
        if (TestTrue(TEXT("Export the reopened bank"), KyaBuildBankExportBytes(LoadedBank, Exported, Diagnostics)))
        {
            const uint8* ExportedCursor = Exported.GetData();
            UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
                UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
                TEXT("bnk"), ExportedCursor, Exported.GetData() + Exported.Num(), GWarn));
            if (TestNotNull(TEXT("Reimport the exported gizmo bank"), Reimported))
            {
                for (const TPair<int32, FKyaActorPlacement>& Pair : Expected)
                {
                    if (!TestTrue(TEXT("The reimported bank keeps the edited actor"),
                        Reimported->SceneEnvironment.Actors.IsValidIndex(Pair.Key))) continue;
                    const FKyaActorPlacement& ReimportedActor = Reimported->SceneEnvironment.Actors[Pair.Key];
                    TestTrue(TEXT("Export/reimport preserves the edited position"),
                        ReimportedActor.Position.Equals(Pair.Value.Position, 1.0e-3));
                    TestTrue(TEXT("Export/reimport preserves the edited rotation"),
                        ReimportedActor.Rotation.Equals(Pair.Value.Rotation, 1.0e-5));
                    TestTrue(TEXT("Export/reimport preserves the edited scale"),
                        ReimportedActor.Scale.Equals(Pair.Value.Scale, 1.0e-4));
                }
            }
        }
    }

    if (LoadedPackage) LoadedPackage->SetDirtyFlag(false);
    IFileManager::Get().Delete(*PackageFilename, false, true, true);
    Factory->RemoveFromRoot();
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMilestoneAGateTest, "Kya.Editor.MilestoneAGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMilestoneAGateTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the Milestone A gate."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read Milestone A bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    Factory->AddToRoot();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Milestone A bank"), Bank))
    {
        Factory->RemoveFromRoot();
        return false;
    }
    Bank->AddToRoot();
    if (!TestTrue(TEXT("Milestone A bank has actors"), Bank->SceneEnvironment.Actors.Num() > 0)) return false;

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create Milestone A world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn Milestone A preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = true;
    Preview->OnConstruction(FTransform::Identity);
    AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
    if (!TestNotNull(TEXT("Milestone A handle exists"), Handle))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }

    // Gate step 1: selection (scripted click equivalent for headless automation).
    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        GEditor->SelectActor(Handle, true, true);
    }
    TestTrue(TEXT("Clicking selects the actor handle"), Handle->IsSelected());

    // Gate step 2: move through the transaction/edit service path.
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    Preview->OnConstruction(FTransform::Identity);
    Handle = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
    if (!TestNotNull(TEXT("Milestone A handle survives refresh"), Handle))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }
    const FKyaActorPlacement& Original = Bank->SceneEnvironment.Actors[0];
    const FVector OriginalPosition = Original.Position;
    // Unreal world offset (150, 0, 225) cm maps to game axes (1.5, 2.25, 0) m.
    const FVector MovedPosition = OriginalPosition + FVector(1.5, 2.25, 0.0);
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "MilestoneAMove", "Milestone A move"));
        Handle->AddActorWorldOffset(FVector(150.0, 0.0, 225.0));
        Handle->PostEditMove(true);
        Service.FlushPendingNotifications();
    }
    TestTrue(TEXT("Move writes the bank"), Bank->SceneEnvironment.Actors[0].Position.Equals(MovedPosition, 1.0e-3));

    // Gate step 3: undo and redo.
    if (GEditor)
    {
        TestTrue(TEXT("Undo returns a transaction"), GEditor->UndoTransaction());
        TestTrue(TEXT("Undo restores the original position"),
            Bank->SceneEnvironment.Actors[0].Position.Equals(OriginalPosition, 1.0e-3));
        GEditor->RedoTransaction();
        TestTrue(TEXT("Redo reapplies the moved position"),
            Bank->SceneEnvironment.Actors[0].Position.Equals(MovedPosition, 1.0e-3));
    }
    Service.FlushPendingNotifications();

    // Gate step 4: save and reopen a real package.
    const FString PackageRoot = FPaths::ConvertRelativePathToFull(
        FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaMilestoneA")));
    IFileManager::Get().MakeDirectory(*PackageRoot, true);
    const FString PackageName = FString::Printf(TEXT("/Game/KyaMilestoneATest/Bank_%s"),
        *FGuid::NewGuid().ToString(EGuidFormats::Digits));
    UPackage* SavePackage = CreatePackage(*PackageName);
    UKyaLevelBank* SaveBank = DuplicateObject<UKyaLevelBank>(Bank, SavePackage, TEXT("Bank"));
    if (TestNotNull(TEXT("Duplicate bank for save"), SaveBank))
    {
        SaveBank->ClearFlags(RF_Transient);
        SaveBank->SetFlags(RF_Public | RF_Standalone);
        SavePackage->ClearFlags(RF_Transient);
        SavePackage->SetFlags(RF_Public | RF_Standalone);
        SavePackage->MarkPackageDirty();
        const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName,
            FPackageName::GetAssetPackageExtension());
        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
        SaveArgs.SaveFlags = SAVE_NoError;
        TestTrue(TEXT("Save the bank package"), UPackage::SavePackage(SavePackage, SaveBank, *PackageFilename, SaveArgs));
        // Prove disk deserialization: reload the saved package from its file.
        const FString SavedPackageName = SavePackage->GetName();
        FText ReloadError;
        const bool bReloaded = UPackageTools::ReloadPackages({ SavePackage }, ReloadError,
            EReloadPackagesInteractionMode::AssumePositive);
        TestTrue(FString::Printf(TEXT("Reload the saved package from disk (%s)"), *ReloadError.ToString()), bReloaded);
        UPackage* LoadedPackage = FindPackage(nullptr, *SavedPackageName);
        TestNotNull(TEXT("Reload creates a package"), LoadedPackage);
        UKyaLevelBank* LoadedBank = LoadedPackage ? FindObject<UKyaLevelBank>(LoadedPackage, TEXT("Bank")) : nullptr;
        if (TestNotNull(TEXT("Reopen the saved bank"), LoadedBank))
        {
            TestTrue(TEXT("Saved position survives reopen"),
                LoadedBank->SceneEnvironment.Actors[0].Position.Equals(MovedPosition, 1.0e-3));
            TestEqual(TEXT("Saved actor count survives reopen"),
                LoadedBank->SceneEnvironment.Actors.Num(), Bank->SceneEnvironment.Actors.Num());

            // Export the reopened asset, so the persistence and export gates are connected.
            FKyaBankExportDiagnostics PersistedDiagnostics;
            TArray<uint8> PersistedBytes;
            if (TestTrue(TEXT("Export the reopened bank"),
                KyaBuildBankExportBytes(LoadedBank, PersistedBytes, PersistedDiagnostics)))
            {
                const uint8* PersistedCursor = PersistedBytes.GetData();
                UKyaLevelBank* PersistedReimport = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
                    UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
                    TEXT("bnk"), PersistedCursor, PersistedBytes.GetData() + PersistedBytes.Num(), GWarn));
                if (TestNotNull(TEXT("Reimport the reopened bank"), PersistedReimport))
                    TestTrue(TEXT("Reopened export/reimport preserves the moved position"),
                        PersistedReimport->SceneEnvironment.Actors[0].Position.Equals(MovedPosition, 1.0e-3));
            }
        }
        if (LoadedPackage) LoadedPackage->SetDirtyFlag(false);
        IFileManager::Get().Delete(*PackageFilename, false, true, true);
    }

    // Gate step 5: export and reimport the current state as well.
    FKyaBankExportDiagnostics Diagnostics;
    TArray<uint8> Exported;
    if (TestTrue(TEXT("Export Milestone A bank"), KyaBuildBankExportBytes(Bank, Exported, Diagnostics)))
    {
        const uint8* ExportedCursor = Exported.GetData();
        UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), ExportedCursor,
            Exported.GetData() + Exported.Num(), GWarn));
        if (TestNotNull(TEXT("Reimport Milestone A bank"), Reimported))
            TestTrue(TEXT("Export/reimport preserves the moved position"),
                Reimported->SceneEnvironment.Actors[0].Position.Equals(MovedPosition, 1.0e-3));
    }

    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        GEditor->NoteSelectionChange();
    }
    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    Bank->RemoveFromRoot();
    Factory->RemoveFromRoot();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaExportAcceptanceTest, "Kya.Editor.ExportAcceptance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaExportAcceptanceTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the export acceptance test."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read acceptance bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import acceptance bank"), Bank)) return false;
    if (!TestTrue(TEXT("Acceptance bank has actors"), Bank->SceneEnvironment.Actors.Num() > 0)) return false;

    // A successful export describes what it actually writes and keeps an untouched
    // bank byte-identical.
    FKyaBankExportDiagnostics PlainDiagnostics;
    TArray<uint8> Plain;
    if (!TestTrue(TEXT("Build plain acceptance bytes"),
        KyaBuildBankExportBytes(Bank, Plain, PlainDiagnostics))) return false;
    TestTrue(TEXT("Plain export is byte-identical"), Plain == Input);
    TestTrue(TEXT("Export describes the in-place edit scope"), PlainDiagnostics.SupportedEdits.Num() > 0 &&
        PlainDiagnostics.SupportedEdits[0].Contains(TEXT("patched in place")));
    TestTrue(TEXT("Untouched bank reports no changed actors"),
        PlainDiagnostics.SupportedEdits[0].Contains(TEXT("0 actor record(s) differ")));

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    const FVector OriginalPosition = Bank->SceneEnvironment.Actors[0].Position;
    FKyaActorEditPatch Patch;
    Patch.Position = OriginalPosition + FVector(1.25, -2.5, 3.75);
    Patch.VisibilityDistance = 4321.0f;
    Patch.CullingDistance = 8765.0f;
    FString Error;
    if (!TestTrue(TEXT("Apply acceptance edit"),
        Service.Apply(Service.MakeKey(Bank, 0), Patch, EKyaEditNotify::Silent, &Error))) return false;

    FKyaBankExportDiagnostics EditedDiagnostics;
    TArray<uint8> Edited;
    if (!TestTrue(TEXT("Build edited acceptance bytes"),
        KyaBuildBankExportBytes(Bank, Edited, EditedDiagnostics))) return false;
    TestTrue(TEXT("Edited export reports the changed actor"), EditedDiagnostics.SupportedEdits.Num() > 0 &&
        EditedDiagnostics.SupportedEdits[0].Contains(TEXT("1 actor record(s) differ")));
    TestEqual(TEXT("Edited export keeps the file size"), Edited.Num(), Input.Num());
    int32 ChangedBytes = 0;
    for (int32 Index = 0; Index < Input.Num(); ++Index)
        if (Input[Index] != Edited[Index]) ++ChangedBytes;
    TestTrue(TEXT("Accepts only the selected-actor edit"), ChangedBytes > 0 && ChangedBytes <= 40);

    // Reimport compares typed values and unrelated records.
    const FVector NeighbourPosition = Bank->SceneEnvironment.Actors.Num() > 1
        ? Bank->SceneEnvironment.Actors[1].Position : FVector::ZeroVector;
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport acceptance bank"), Reimported)) return false;
    TestTrue(TEXT("Acceptance position round trips"),
        Reimported->SceneEnvironment.Actors[0].Position.Equals(Patch.Position.GetValue(), 1.0e-4));
    TestTrue(TEXT("Acceptance visibility round trips"),
        FMath::IsNearlyEqual(Reimported->SceneEnvironment.Actors[0].VisibilityDistance, 4321.0f, 1.0e-3f));
    if (Reimported->SceneEnvironment.Actors.Num() > 1)
        TestTrue(TEXT("Unrelated actor data is preserved"),
            Reimported->SceneEnvironment.Actors[1].Position.Equals(NeighbourPosition));

    // The export action surfaces the supported-edit description, not just a path.
    TArray<FString> SuccessMessages;
    const FString SuccessDestination = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaExportAcceptanceOk"));
    TestTrue(TEXT("Acceptance export publishes"),
        KyaExportLevelBanks({ Bank }, SuccessDestination, SuccessMessages));
    TestTrue(TEXT("Export result describes its supported edits"),
        SuccessMessages.ContainsByPredicate([](const FString& Message) { return Message.Contains(TEXT("edits:")); }));
    TestTrue(TEXT("Export result names the in-place scope"),
        SuccessMessages.ContainsByPredicate([](const FString& Message)
        {
            return Message.Contains(TEXT("patched in place"));
        }));

    // Unsupported structural edits fail before any output replacement.
    Cursor = Input.GetData();
    UKyaLevelBank* Structural = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (TestNotNull(TEXT("Import structural acceptance bank"), Structural))
    {
        FKyaActorPlacement StructuralCopy = Structural->SceneEnvironment.Actors[0];
        Structural->SceneEnvironment.Actors.Add(StructuralCopy);
        const FString Destination = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaExportAcceptance"));
        IFileManager::Get().MakeDirectory(*Destination, true);
        const FString SentinelPath = FPaths::Combine(Destination, TEXT("KEEP.txt"));
        FFileHelper::SaveStringToFile(TEXT("SENTINEL"), *SentinelPath);
        const FString BankDestination = FPaths::Combine(Destination, Structural->GetName() + TEXT(".bnk"));
        FFileHelper::SaveStringToFile(TEXT("EXISTING"), *BankDestination);
        FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
        MessageLogModule.GetLogListing("KyaLevelEditor")->ClearMessages();
        AddExpectedError(TEXT("actor count"), EAutomationExpectedErrorFlags::Contains, 0);
        AddExpectedError(TEXT("Export stopped"), EAutomationExpectedErrorFlags::Contains, 0);
        TArray<FString> Messages;
        TestFalse(TEXT("Structural export fails"), KyaExportLevelBanks({ Structural }, Destination, Messages));
        TestTrue(TEXT("Structural failure reports why"),
            Messages.ContainsByPredicate([](const FString& Message)
            {
                return Message.Contains(TEXT("blocking validation issues"));
            }));
        FString Sentinel;
        TestTrue(TEXT("Existing unrelated file is unchanged"),
            FFileHelper::LoadFileToString(Sentinel, *SentinelPath) && Sentinel == TEXT("SENTINEL"));
        FString ExistingBank;
        TestTrue(TEXT("Existing destination bank is not replaced"),
            FFileHelper::LoadFileToString(ExistingBank, *BankDestination) && ExistingBank == TEXT("EXISTING"));
        TestFalse(TEXT("No staging file is left behind"),
            [&Destination]()
            {
                TArray<FString> Files;
                IFileManager::Get().FindFiles(Files, *(Destination / TEXT("*.kya-staging*")), true, false);
                return Files.Num() > 0;
            }());
    }
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaLevelActionsReportingTest, "Kya.Editor.LevelActionsReporting",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaLevelActionsReportingTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage(), 0x2c);
    Bank->SceneEnvironment.Actors[0].ClassId = 0x2c;
    Bank->SceneEnvironment.Actors[0].Name = TEXT("FIRST");
    Bank->SceneEnvironment.Actors[0].TrapActorRef = 99;
    Bank->ImportSchemaVersion = 0;

    FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
    const TSharedRef<IMessageLogListing> Listing = MessageLogModule.GetLogListing("KyaLevelEditor");
    Listing->ClearMessages();
    AddExpectedError(TEXT("cannot be exported"), EAutomationExpectedErrorFlags::Contains, 0);
    AddExpectedError(TEXT("No bank bytes are retained"), EAutomationExpectedErrorFlags::Contains, 0);
    AddExpectedError(TEXT("TrapActorRef"), EAutomationExpectedErrorFlags::Contains, 0);
    AddExpectedError(TEXT("Export stopped"), EAutomationExpectedErrorFlags::Contains, 0);

    // Validate Kya Level reports bank + actor identity + property path, and the
    // Message Log entry carries navigation tokens.
    const TArray<FKyaBankValidationReport> Reports = KyaValidateLevelBanks({ Bank });
    TestEqual(TEXT("Validate reports the linked bank"), Reports.Num(), 1);
    bool bIdentity = false;
    bool bPropertyPath = false;
    bool bAction = false;
    for (const TSharedRef<FTokenizedMessage>& Message : Listing->GetFilteredMessages())
    {
        const FString Text = Message->ToText().ToString();
        if (Text.Contains(TEXT("FIRST")) && Text.Contains(TEXT("Actor 0"))) bIdentity = true;
        if (Text.Contains(TEXT(".TrapActorRef"))) bPropertyPath = true;
        for (const TSharedRef<IMessageToken>& Token : Message->GetMessageTokens())
            if (Token->GetType() == EMessageToken::Action) bAction = true;
    }
    TestTrue(TEXT("Validation names the actor identity (index, name, class)"), bIdentity);
    TestTrue(TEXT("Validation names the property path"), bPropertyPath);
    TestTrue(TEXT("Validation offers Details navigation"), bAction);

    // Export Kya Level reports the same findings before replacing any output and logs
    // the unsupported bank as a blocking error with a bank link.
    Listing->ClearMessages();
    AddExpectedError(TEXT("Cannot export bank:"), EAutomationExpectedErrorFlags::Contains, 1);
    TArray<FString> Messages;
    const FString Destination = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaExportReportingTest"));
    const bool bExported = KyaExportLevelBanks({ Bank }, Destination, Messages);
    TestFalse(TEXT("Unsupported bank is not exported"), bExported);
    bool bExportBlocked = false;
    bool bExportIssue = false;
    for (const TSharedRef<FTokenizedMessage>& Message : Listing->GetFilteredMessages())
    {
        const FString Text = Message->ToText().ToString();
        if (Text.Contains(TEXT("Cannot export bank:")) && Message->GetSeverity() == EMessageSeverity::Error)
        {
            for (const TSharedRef<IMessageToken>& Token : Message->GetMessageTokens())
                if (Token->GetType() == EMessageToken::AssetName) bExportBlocked = true;
        }
        if (Text.Contains(TEXT("TrapActorRef"))) bExportIssue = true;
    }
    TestTrue(TEXT("Export logs the unsupported bank as a blocking error with a bank link"), bExportBlocked);
    TestTrue(TEXT("Export reports actor/property findings"), bExportIssue);

    // A writer rejection with actor context is reported by record identity and
    // property path, and blocks publication before any destination is replaced.
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (!Filename.IsEmpty())
    {
        TArray<uint8> Input;
        if (TestTrue(TEXT("Read reporting bank"), FFileHelper::LoadFileToArray(Input, *Filename)))
        {
            const uint8* Cursor = Input.GetData();
            UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
            UKyaLevelBank* RealBank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
                UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
                TEXT("bnk"), Cursor, Input.GetData() + Input.Num(), GWarn));
            if (TestNotNull(TEXT("Import reporting bank"), RealBank) &&
                TestTrue(TEXT("Reporting bank has actors"), RealBank->SceneEnvironment.Actors.Num() > 0))
            {
                const FString OriginalName = RealBank->SceneEnvironment.Actors[0].Name;
                FString FailingName;
                for (const FString& Candidate : {FString::ChrN(61, TEXT('A')), FString(TEXT("AB"))})
                {
                    RealBank->SceneEnvironment.Actors[0].Name = Candidate;
                    FKyaBankExportDiagnostics NameDiagnostics;
                    TArray<uint8> NameBytes;
                    if (!KyaBuildBankExportBytes(RealBank, NameBytes, NameDiagnostics))
                    {
                        if (NameDiagnostics.FailedActorIndex == 0 && NameDiagnostics.FailedProperty == TEXT("Name"))
                        {
                            FailingName = Candidate;
                            break;
                        }
                    }
                }
                RealBank->SceneEnvironment.Actors[0].Name = OriginalName;
                if (TestFalse(TEXT("A name-slot change is detectable"), FailingName.IsEmpty()))
                {
                    RealBank->SceneEnvironment.Actors[0].Name = FailingName;
                    Listing->ClearMessages();
                    AddExpectedError(TEXT("name needs"), EAutomationExpectedErrorFlags::Contains, 0);
                    AddExpectedError(TEXT("Export stopped"), EAutomationExpectedErrorFlags::Contains, 0);
                    TArray<FString> NameMessages;
                    const FString NameDestination = FPaths::Combine(FPaths::ProjectSavedDir(),
                        TEXT("KyaExportReportingNameTest"));
                    TestFalse(TEXT("Blocking writer issues stop export"),
                        KyaExportLevelBanks({ RealBank }, NameDestination, NameMessages));
                    bool bNamedProperty = false;
                    for (const TSharedRef<FTokenizedMessage>& Message : Listing->GetFilteredMessages())
                        if (Message->ToText().ToString().Contains(TEXT(".Name"))) bNamedProperty = true;
                    TestTrue(TEXT("Writer rejection names the actor property path"), bNamedProperty);
                    TestFalse(TEXT("Blocked export writes no output"),
                        FPaths::FileExists(FPaths::Combine(NameDestination,
                            RealBank->GetName() + TEXT(".bnk"))));
                    RealBank->SceneEnvironment.Actors[0].Name = OriginalName;

                    // A model-count change also carries actor/property context, and one
                    // bad bank must stop publication for every bank in the set.
                    FKyaActorPlacement& ModelActor = RealBank->SceneEnvironment.Actors[0];
                    const TArray<FKyaActorModelReference> OriginalModels = ModelActor.Models;
                    ModelActor.Models.AddDefaulted();
                    const uint8* SecondCursor = Input.GetData();
                    UKyaLevelBank* ValidBank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
                        UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
                        TEXT("bnk"), SecondCursor, Input.GetData() + Input.Num(), GWarn));
                    if (TestNotNull(TEXT("Import second reporting bank"), ValidBank))
                    {
                        Listing->ClearMessages();
                        AddExpectedError(TEXT("model reference count changed"), EAutomationExpectedErrorFlags::Contains, 0);
                        TArray<FString> ModelMessages;
                        const FString ModelDestination = FPaths::Combine(FPaths::ProjectSavedDir(),
                            TEXT("KyaExportReportingModelTest"));
                        TestFalse(TEXT("Model-count failure stops the linked-set export"),
                            KyaExportLevelBanks({ RealBank, ValidBank }, ModelDestination, ModelMessages));
                        TestFalse(TEXT("Blocked linked-set export writes the bad bank"),
                            FPaths::FileExists(FPaths::Combine(ModelDestination,
                                RealBank->GetName() + TEXT(".bnk"))));
                        TestFalse(TEXT("Blocked linked-set export writes the valid bank"),
                            FPaths::FileExists(FPaths::Combine(ModelDestination,
                                ValidBank->GetName() + TEXT(".bnk"))));
                        bool bModelProperty = false;
                        for (const TSharedRef<FTokenizedMessage>& Message : Listing->GetFilteredMessages())
                            if (Message->ToText().ToString().Contains(TEXT(".Models"))) bModelProperty = true;
                        TestTrue(TEXT("Model failure names the actor property path"), bModelProperty);
                    }
                    ModelActor.Models = OriginalModels;
                }
            }
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaTargetedWorkflowAcceptanceTest, "Kya.Editor.TargetedWorkflowAcceptance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaTargetedWorkflowAcceptanceTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage(), 6);

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create acceptance world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn acceptance preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
    if (!TestNotNull(TEXT("Acceptance handle exists"), Handle))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }

    // --- Acceptance 1: dragging must not decode or rebuild the level ---------------
    const int64 BuildsBeforeDrag = Preview->BuildCount;
    const int32 SyncDecodesBeforeDrag = Preview->GetSynchronousActorDecodeCount();
    Handle->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
    Handle->PostEditMove(false);
    TestTrue(TEXT("Drag does not write the bank before commit"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(1.0, 2.0, 3.0)));
    Handle->PostEditMove(true);
    Service.FlushPendingNotifications();
    TestTrue(TEXT("Drag commit writes the bank"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(2.0, 2.0, 3.0)));
    TestEqual(TEXT("Drag commit does not rebuild or decode the level"),
        Preview->BuildCount, BuildsBeforeDrag);
    TestEqual(TEXT("Drag commit performs no synchronous actor decode"),
        Preview->GetSynchronousActorDecodeCount(), SyncDecodesBeforeDrag);

    // --- Acceptance 2: selection survives targeted refresh and full rebuilds --------
    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        GEditor->SelectActor(Handle, true, true);
        TestTrue(TEXT("Acceptance handle is selected"), Handle->IsSelected());

        FKyaActorEditPatch Patch;
        Patch.VisibilityDistance = 42.0f;
        FString Error;
        TestTrue(TEXT("Targeted edit applies"),
            Service.Apply(Service.MakeKey(Bank, 0), Patch, EKyaEditNotify::Notify, &Error));
        TestEqual(TEXT("Targeted edit preserves the handle object"), Preview->GetActorHandle(0),
            static_cast<AActor*>(Handle));
        TestTrue(TEXT("Targeted edit preserves selection"), Handle->IsSelected());

        Bank->BumpEditGeneration();
        Preview->OnConstruction(FTransform::Identity);
        TestEqual(TEXT("Full rebuild preserves the handle object"), Preview->GetActorHandle(0),
            static_cast<AActor*>(Handle));
        TestTrue(TEXT("Full rebuild preserves selection"), Handle->IsSelected());
    }

    // --- Acceptance 3: missing content is diagnosable from structured messages ------
    Bank->SceneEnvironment.Actors[0].ClassId = 0x2c;
    Bank->SceneEnvironment.Actors[0].TrapActorRef = 99;
    Bank->ImportSchemaVersion = 0;
    FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
    const TSharedRef<IMessageLogListing> Listing = MessageLogModule.GetLogListing("KyaLevelEditor");
    Listing->ClearMessages();
    // The acceptance case intentionally produces blocking errors; whitelist them so
    // the automation framework reports only the assertions below.
    AddExpectedError(TEXT("cannot be exported"), EAutomationExpectedErrorFlags::Contains, 0);
    AddExpectedError(TEXT("No bank bytes are retained"), EAutomationExpectedErrorFlags::Contains, 0);
    AddExpectedError(TEXT("TrapActorRef"), EAutomationExpectedErrorFlags::Contains, 0);
    const TArray<FKyaBankValidationReport> Reports = KyaValidateLevelBanks({ Bank });
    TestTrue(TEXT("Validation produces a report"), Reports.Num() == 1);
    const TArray<TSharedRef<FTokenizedMessage>> Messages = Listing->GetFilteredMessages();
    TestTrue(TEXT("Validation writes to the Message Log"), Messages.Num() > 0);
    bool bAssetLink = false;
    bool bActorLink = false;
    for (const TSharedRef<FTokenizedMessage>& Message : Messages)
    {
        for (const TSharedRef<IMessageToken>& Token : Message->GetMessageTokens())
        {
            if (Token->GetType() == EMessageToken::AssetName) bAssetLink = true;
            if (Token->GetType() == EMessageToken::Action) bActorLink = true;
        }
    }
    TestTrue(TEXT("Missing content entries link the affected asset"), bAssetLink);
    TestTrue(TEXT("Missing content entries offer actor navigation"), bActorLink);

    // --- Acceptance 1b: real-level transforms reuse decoded geometry ---------------
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the real-level transform acceptance cases."));
    }
    else
    {
        TArray<uint8> Input;
        if (TestTrue(TEXT("Read real acceptance bank"), FFileHelper::LoadFileToArray(Input, *Filename)))
        {
            const uint8* Cursor = Input.GetData();
            UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
            UKyaLevelBank* RealBank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
                UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient, nullptr,
                TEXT("bnk"), Cursor, Input.GetData() + Input.Num(), GWarn));
            if (TestNotNull(TEXT("Import real acceptance bank"), RealBank) &&
                TestTrue(TEXT("The real bank has actors"), RealBank->SceneEnvironment.Actors.Num() > 0))
            {
                UWorld* RealWorld = UWorld::CreateWorld(EWorldType::EditorPreview, false);
                if (TestNotNull(TEXT("Create real acceptance world"), RealWorld))
                {
                    AKyaLevelPreview* RealPreview = RealWorld->SpawnActor<AKyaLevelPreview>();
                    if (TestNotNull(TEXT("Spawn real acceptance preview"), RealPreview))
                    {
                        RealPreview->Bank = RealBank;
                        RealPreview->bShowRebuildProgress = false;
                        RealPreview->bSpawnActorHandles = true;
                        RealPreview->bShowActorMeshesOnHandles = true;
                        RealPreview->OnConstruction(FTransform::Identity);
                        TestTrue(TEXT("Real level decode is measured"), RealPreview->GetGeometryDecodeCount() > 0);

                        AKyaActorHandle* RealHandle = Cast<AKyaActorHandle>(RealPreview->GetActorHandle(0));
                        if (TestNotNull(TEXT("Real acceptance handle exists"), RealHandle))
                        {
                            const int32 GeometryDecodes = RealPreview->GetGeometryDecodeCount();
                            int32 SyncDecodes = RealPreview->GetSynchronousActorDecodeCount();
                            const auto CheckTransformAcceptance = [this, RealPreview, GeometryDecodes, &SyncDecodes](
                                const TCHAR* Label)
                            {
                                FKyaLevelEditService::Get().FlushPendingNotifications();
                                TestEqual(FString::Printf(TEXT("%s does not decode level geometry"), Label),
                                    RealPreview->GetGeometryDecodeCount(), GeometryDecodes);
                                const int32 CurrentSync = RealPreview->GetSynchronousActorDecodeCount();
                                TestTrue(FString::Printf(TEXT("%s decodes at most the affected actor"), Label),
                                    CurrentSync - SyncDecodes <= 1);
                                SyncDecodes = CurrentSync;
                            };

                            RealHandle->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
                            RealHandle->PostEditMove(true);
                            CheckTransformAcceptance(TEXT("Real-level translation"));

                            RealHandle->SetActorRelativeRotation(FQuat(FVector::UpVector, 0.5));
                            RealHandle->PostEditMove(true);
                            CheckTransformAcceptance(TEXT("Real-level rotation"));

                            const FVector Scale = KyaActorTransform::SwapGameAxes(FVector(1.5, 1.0, 1.0));
                            RealHandle->SetActorRelativeScale3D(Scale);
                            RealHandle->PostEditMove(true);
                            CheckTransformAcceptance(TEXT("Real-level scale"));

                            TestTrue(TEXT("Real-level transforms reused cached geometry"),
                                RealPreview->GetCachedGeometryDecodeCount() > 0);
                        }
                        RealPreview->Destroy();
                    }
                    RealWorld->DestroyWorld(false);
                }
            }
        }
    }

    if (GEditor)
    {
        GEditor->SelectNone(false, true, false);
        GEditor->NoteSelectionChange();
    }
    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaAuthoringVisibilityTest, "Kya.Editor.AuthoringVisibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaAuthoringVisibilityTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage(), 6);
    Bank->SceneEnvironment.Actors[0].SectorId = -1;
    Bank->SceneEnvironment.Actors[1].SectorId = 1;
    FKyaActorPlacement& Unloaded = Bank->SceneEnvironment.Actors.AddDefaulted_GetRef();
    Unloaded.Name = TEXT("THIRD");
    Unloaded.ClassId = 6;
    Unloaded.Position = FVector(9.0, 1.0, 2.0);
    Unloaded.SectorId = 2;

    UKyaLevelBank* SectorBank = MakeKyaTestBank(GetTransientPackage(), 6);
    SectorBank->SourceFilename = TEXT("G:/tmp/SECT1.BNK");

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create authoring world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn authoring preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->SectorBanks.Add(SectorBank);
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = true;

    // Native (comparison) visibility only surfaces actors of loaded sectors.
    Preview->bAuthoringVisibility = false;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Native visibility hides unloaded-sector actors"), Preview->GetActorHandleCount(), 2);
    TestNull(TEXT("Unloaded-sector actor has no native handle"), Preview->GetActorHandle(2));
    TestEqual(TEXT("Native visibility reports no authoring-only handles"),
        Preview->GetAuthoringOnlyHandleCount(), 0);
    TestFalse(TEXT("Native visibility disables the authoring override"),
        Preview->IsAuthoringVisibilityActive());

    // Authoring discovery keeps every actor selectable even when its sector is not loaded.
    Preview->bAuthoringVisibility = true;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Authoring visibility keeps unloaded-sector actors discoverable"),
        Preview->GetActorHandleCount(), 3);
    TestNotNull(TEXT("Unloaded-sector actor gets an authoring handle"), Preview->GetActorHandle(2));
    TestEqual(TEXT("Authoring visibility reports the authoring-only handle"),
        Preview->GetAuthoringOnlyHandleCount(), 1);
    TestTrue(TEXT("Authoring visibility is active"), Preview->IsAuthoringVisibilityActive());
    if (AKyaActorHandle* Authored = Cast<AKyaActorHandle>(Preview->GetActorHandle(2)))
    {
        TestEqual(TEXT("Authoring handle keeps the sector grouping"), Authored->ActorSectorId, 2);
        TestTrue(TEXT("Authoring handle is selectable"), Authored->IsSelectable());
    }

    {
        const FScopedTransaction Transaction(FText::FromString(TEXT("Hide sector test")));
        Preview->SetSectorVisible(1, false);
    }
    TestTrue(TEXT("Sector checkbox hides loaded actor handles"), Preview->GetActorHandle(1)->IsTemporarilyHiddenInEditor());
    TestNotNull(TEXT("Global actor remains visible"), Preview->GetActorHandle(0));
    if (GEditor)
    {
        TestTrue(TEXT("Sector visibility supports undo"), GEditor->UndoTransaction());
        TestTrue(TEXT("Undo restores checkbox state"), Preview->IsSectorVisible(1));
        TestNotNull(TEXT("Undo restores sector actor handles"), Preview->GetActorHandle(1));
        TestTrue(TEXT("Sector visibility supports redo"), GEditor->RedoTransaction());
        TestTrue(TEXT("Redo hides sector actor handles again"), Preview->GetActorHandle(1)->IsTemporarilyHiddenInEditor());
    }
    Preview->SetSectorVisible(1, true);
    Preview->SetSectorVisible(2, false);
    TestTrue(TEXT("Explicit hide overrides unloaded-sector authoring visibility"), Preview->GetActorHandle(2)->IsTemporarilyHiddenInEditor());
    Preview->SetSectorVisible(2, true);
    TestEqual(TEXT("Visibility preserves sector associations"), Preview->SectorBanks.Num(), 1);

    // A locked comparison/capture camera always uses native culling and refreshes
    // membership immediately (no extra rebuild required).
    Preview->SetPreviewCamera(FVector(0.0, 0.0, 0.0), FVector::ForwardVector, FVector::UpVector,
        70.0f, 1280, 720);
    TestFalse(TEXT("Capture camera disables authoring discovery"),
        Preview->IsAuthoringVisibilityActive());
    TestEqual(TEXT("Capture camera hides unloaded-sector actors"), Preview->GetActorHandleCount(), 2);
    TestEqual(TEXT("Capture camera reports no authoring-only handles"),
        Preview->GetAuthoringOnlyHandleCount(), 0);

    // Returning to the editor camera restores authoring discovery.
    Preview->ClearPreviewCamera();
    TestTrue(TEXT("Returning to the editor camera restores authoring discovery"),
        Preview->IsAuthoringVisibilityActive());
    TestEqual(TEXT("Editor camera shows unloaded-sector actors again"), Preview->GetActorHandleCount(), 3);

    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMessageLogLinksTest, "Kya.Editor.MessageLogLinks",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMessageLogLinksTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage(), 0x2c);
    Bank->SceneEnvironment.Actors[0].ClassId = 0x2c;
    Bank->BumpEditGeneration();
    Bank->SceneEnvironment.Actors[0].TrapActorRef = 99;

    // Missing references are reported against the offending actor so the Message Log
    // can offer a navigation action, not just raw text.
    const FKyaBankValidationReport BrokenReference = KyaValidateBank(Bank);
    bool bFoundActorIssue = false;
    for (const FKyaValidationIssue& Issue : BrokenReference.Issues)
    {
        if (Issue.ActorIndex == 0 && Issue.Field == TEXT("TrapActorRef"))
        {
            bFoundActorIssue = true;
            TestTrue(TEXT("Missing reference issue explains the missing actor"), !Issue.Message.IsEmpty());
        }
    }
    TestTrue(TEXT("Validation reports the missing actor reference"), bFoundActorIssue);
    TestTrue(TEXT("Blocking reference issues are flagged"),
        BrokenReference.Issues.ContainsByPredicate(
            [](const FKyaValidationIssue& Issue) { return Issue.bBlocking; }));
    TestFalse(TEXT("Blocking reference issue preserves an explanatory reason"),
        BrokenReference.UnsupportedReason.IsEmpty());

    // A stale schema is surfaced as an actionable issue instead of only a raw log line.
    UKyaLevelBank* StaleBank = MakeKyaTestBank(GetTransientPackage());
    StaleBank->ImportSchemaVersion = 0;
    const FKyaBankValidationReport Stale = KyaValidateBank(StaleBank);
    bool bFoundSchemaIssue = false;
    for (const FKyaValidationIssue& Issue : Stale.Issues)
        if (Issue.Field == TEXT("schema")) bFoundSchemaIssue = true;
    TestTrue(TEXT("Validation reports a stale import schema"), bFoundSchemaIssue);

    // Tokenized messages carry a clickable asset link and (for actor issues) a
    // select action that resolves the generated representation.
    const TSharedRef<FTokenizedMessage> BankEntry = KyaMakeBankMessage(
        EMessageSeverity::Error, Bank, FText::FromString(TEXT("Missing reference")), 0);
    bool bAssetToken = false;
    bool bActionToken = false;
    for (const TSharedRef<IMessageToken>& Token : BankEntry->GetMessageTokens())
    {
        if (Token->GetType() == EMessageToken::AssetName) bAssetToken = true;
        if (Token->GetType() == EMessageToken::Action) bActionToken = true;
    }
    TestTrue(TEXT("Bank message links the asset"), bAssetToken);
    TestTrue(TEXT("Actor issue offers a navigation action"), bActionToken);

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (TestNotNull(TEXT("Create message-log world"), World))
    {
        AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
        if (TestNotNull(TEXT("Spawn message-log preview"), Preview))
        {
            Preview->Bank = Bank;
            Preview->OnConstruction(FTransform::Identity);
            const TSharedRef<FTokenizedMessage> PreviewEntry = KyaMakeActorMessage(
                EMessageSeverity::Warning, Bank, Preview, FText::FromString(TEXT("Missing sector")));
            bool bPreviewAction = false;
            for (const TSharedRef<IMessageToken>& Token : PreviewEntry->GetMessageTokens())
                if (Token->GetType() == EMessageToken::Action) bPreviewAction = true;
            TestTrue(TEXT("Preview message offers a select-preview action"), bPreviewAction);
            if (AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Preview->GetActorHandle(0)))
            {
                TestTrue(TEXT("Select action resolves the generated handle"),
                    KyaSelectBankActor(Bank, 0, World));
                TestTrue(TEXT("Select action selects the handle"), Handle->IsSelected());
            }
        }
        World->DestroyWorld(false);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaAsyncDecodeTest, "Kya.Editor.AsyncDecode",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaAsyncDecodeTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the async decode test."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read async bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import async bank"), Bank)) return false;

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create async world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn async preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bAsyncDecodeMeshes = true;
    Preview->bSpawnActorHandles = true;
    Preview->bShowActorMeshesOnHandles = true;

    Preview->OnConstruction(FTransform::Identity);
    Preview->WaitForAsyncDecode();
    TestTrue(TEXT("Async actor decode completes"), Preview->GetAsyncDecodeCompletionCount() > 0);

    // Clearing the per-actor capture cache and rebuilding again must reuse the
    // background-decoded strips instead of synchronously decoding them.
    Preview->bSpawnActorHandles = false;
    Preview->OnConstruction(FTransform::Identity);
    Preview->bSpawnActorHandles = true;
    Preview->OnConstruction(FTransform::Identity);
    TestTrue(TEXT("Rebuild uses the background decode cache"), Preview->GetCachedActorDecodeCount() > 0);

    // A source revision change makes the old background result stale.
    const int32 StaleBefore = Preview->GetStaleAsyncDecodeCount();
    Bank->BumpEditGeneration();
    Preview->OnConstruction(FTransform::Identity);
    Preview->WaitForAsyncDecode();
    TestTrue(TEXT("Stale background decode is discarded"),
        Preview->GetStaleAsyncDecodeCount() > StaleBefore);

    // Cancellation aborts the rebuild at a phase boundary and keeps the displayed
    // geometry instead of publishing a partially built mesh.
    Preview->RequestCancelRebuild();
    const int64 BuildsBeforeCancel = Preview->BuildCount;
    const int32 SectionsBeforeCancel = Preview->Mesh->GetNumSections();
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Cancelled rebuild reports its status"), Preview->RebuildStatus, FString(TEXT("Cancelled")));
    TestEqual(TEXT("Cancelled rebuild does not complete"), Preview->BuildCount, BuildsBeforeCancel);
    TestEqual(TEXT("Cancelled rebuild preserves the previous geometry"),
        Preview->Mesh->GetNumSections(), SectionsBeforeCancel);

    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaLevelSettingsTest, "Kya.Editor.LevelSettings",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaLevelSettingsTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    Bank->SceneEnvironment.CameraPosition = FVector(1.0, 2.0, 3.0);
    Bank->SceneEnvironment.CameraLookAt = FVector(4.0, 5.0, 6.0);
    Bank->SceneEnvironment.CameraFov = 65.0f;
    Bank->SceneEnvironment.FogParameter = 0.25f;
    Bank->SceneEnvironment.FogColor = FColor(10, 20, 30, 255);
    Bank->SceneConfiguration.Field20 = 7;

    UKyaLevelSettings* Settings = NewObject<UKyaLevelSettings>(GetTransientPackage(), NAME_None, RF_Transient);
    Settings->Bank = Bank;
    Settings->LoadFromBank();
    TestTrue(TEXT("Settings load the camera position"), Settings->Environment.CameraPosition.Equals(FVector(1.0, 2.0, 3.0)));
    TestEqual(TEXT("Settings load the camera FOV"), Settings->Environment.CameraFov, 65.0f);
    TestEqual(TEXT("Settings load the fog parameter"), Settings->Environment.FogParameter, 0.25f);
    TestEqual(TEXT("Settings load the scene configuration"), int64(Settings->SceneConfiguration.Field20), int64(7));

    Settings->Environment.CameraFov = 80.0f;
    Settings->Environment.FogParameter = 0.5f;
    Settings->Environment.FogColor = FColor(1, 2, 3, 255);
    Settings->SceneConfiguration.Field20 = 9;
    // A stale settings snapshot must not overwrite placement records edited elsewhere.
    Bank->SceneEnvironment.Actors[0].Position = FVector(9.0, 8.0, 7.0);
    const float OriginalFov = Bank->SceneEnvironment.CameraFov;
    const float OriginalFog = Bank->SceneEnvironment.FogParameter;
    FString Error;
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "LevelSettings", "Level Settings"));
        TestTrue(TEXT("Level settings apply"), Settings->ApplyToBank(Error));
    }
    TestEqual(TEXT("Camera FOV is written"), Bank->SceneEnvironment.CameraFov, 80.0f);
    TestEqual(TEXT("Fog parameter is written"), Bank->SceneEnvironment.FogParameter, 0.5f);
    TestEqual(TEXT("Scene configuration is written"), int64(Bank->SceneConfiguration.Field20), int64(9));
    TestEqual(TEXT("Fog color is written"), uint32(Bank->SceneEnvironment.FogColor.ToPackedARGB()), uint32(FColor(1, 2, 3, 255).ToPackedARGB()));
    TestTrue(TEXT("Settings apply preserves externally edited actor records"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(9.0, 8.0, 7.0)));
    if (GEditor)
    {
        GEditor->UndoTransaction();
        TestTrue(TEXT("Undo restores the camera FOV"),
            FMath::IsNearlyEqual(Bank->SceneEnvironment.CameraFov, OriginalFov));
        TestTrue(TEXT("Undo restores the fog parameter"),
            FMath::IsNearlyEqual(Bank->SceneEnvironment.FogParameter, OriginalFog));
    }

    // Property edits remain on the proxy until explicitly saved.
    const float BeforeDraftEdit = Bank->SceneEnvironment.FogParameter;
    Settings->Environment.FogParameter = 0.75f;
    if (FProperty* EnvironmentProperty = FindFProperty<FProperty>(UKyaLevelSettings::StaticClass(), TEXT("Environment")))
    {
        FPropertyChangedEvent Changed(EnvironmentProperty);
        Settings->PostEditChangeProperty(Changed);
        TestTrue(TEXT("Settings property edits leave the bank unchanged"),
            FMath::IsNearlyEqual(Bank->SceneEnvironment.FogParameter, BeforeDraftEdit));
        TestTrue(TEXT("Settings draft retains the edit"),
            FMath::IsNearlyEqual(Settings->Environment.FogParameter, 0.75f));
    }
    TestNull(TEXT("Proxy excludes actor property trees"), FindFProperty<FProperty>(FKyaLevelSettingsEnvironment::StaticStruct(), TEXT("Actors")));
    TestNull(TEXT("Proxy excludes waypoint property trees"), FindFProperty<FProperty>(FKyaLevelSettingsEnvironment::StaticStruct(), TEXT("Waypoints")));
    TestNull(TEXT("Proxy excludes path property trees"), FindFProperty<FProperty>(FKyaLevelSettingsEnvironment::StaticStruct(), TEXT("Paths")));
    Service.OnBankEdited.Broadcast(Bank);
    TestEqual(TEXT("External edits do not discard the draft"), Settings->Environment.FogParameter, 0.75f);
    Settings->LoadFromBank();
    TestEqual(TEXT("Reload discards the draft"), Settings->Environment.FogParameter, Bank->SceneEnvironment.FogParameter);
    Settings->Environment.FogParameter = 0.75f;
    TestTrue(TEXT("Save commits the draft"), Settings->ApplyToBank(Error));
    TestEqual(TEXT("Saved draft reaches the bank"), Bank->SceneEnvironment.FogParameter, 0.75f);

    // Selecting another bank reloads the settings instead of applying the old ones.
    UKyaLevelBank* OtherBank = MakeKyaTestBank(GetTransientPackage());
    OtherBank->SceneEnvironment.FogParameter = 0.125f;
    Settings->Bank = OtherBank;
    if (FProperty* BankField = FindFProperty<FProperty>(UKyaLevelSettings::StaticClass(), TEXT("Bank")))
    {
        FPropertyChangedEvent BankChanged(BankField);
        Settings->PostEditChangeProperty(BankChanged);
        TestTrue(TEXT("Bank change reloads from the new bank"),
            FMath::IsNearlyEqual(Settings->Environment.FogParameter, 0.125f));
        TestTrue(TEXT("Bank change does not overwrite the new bank"),
            FMath::IsNearlyEqual(OtherBank->SceneEnvironment.FogParameter, 0.125f));
    }
    Settings->Bank = Bank;
    Settings->LoadFromBank();

    // Invalid settings are rejected before mutation.
    const float BeforeInvalid = Bank->SceneEnvironment.FogParameter;
    Settings->Environment.FogParameter = NAN;
    TestFalse(TEXT("Invalid level settings are rejected"), Settings->ApplyToBank(Error));
    TestFalse(TEXT("Level settings rejection explains why"), Error.IsEmpty());
    TestTrue(TEXT("Rejected level settings leave the bank unchanged"),
        FMath::IsNearlyEqual(Bank->SceneEnvironment.FogParameter, BeforeInvalid));

    // The preview exposes a discoverable, stable authoring object.
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (TestNotNull(TEXT("Create level settings world"), World))
    {
        AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
        if (TestNotNull(TEXT("Spawn level settings preview"), Preview))
        {
            Preview->Bank = Bank;
            UKyaLevelSettings* First = Preview->GetOrCreateLevelSettings();
            UKyaLevelSettings* Second = Preview->GetOrCreateLevelSettings();
            TestTrue(TEXT("Level settings object is stable"), First == Second);
            TestEqual(TEXT("Preview settings use the preview bank"), First->Bank.Get(), Bank);
        }
        World->DestroyWorld(false);
    }

    // Authoring and diagnostics are separated in the preview Details metadata.
    const FProperty* BankProperty = FindFProperty<FProperty>(AKyaLevelPreview::StaticClass(), TEXT("Bank"));
    const FProperty* HandlesProperty = FindFProperty<FProperty>(AKyaLevelPreview::StaticClass(), TEXT("bSpawnActorHandles"));
    if (TestNotNull(TEXT("Bank property exists"), BankProperty) &&
        TestNotNull(TEXT("Handle option property exists"), HandlesProperty))
    {
        TestTrue(TEXT("Bank is an authoring property"),
            BankProperty->HasMetaData(TEXT("Category")) &&
            BankProperty->GetMetaData(TEXT("Category")).Contains(TEXT("Authoring")));
        TestTrue(TEXT("Handle options are diagnostics"),
            HandlesProperty->HasMetaData(TEXT("Category")) &&
            HandlesProperty->GetMetaData(TEXT("Category")).Contains(TEXT("Diagnostics")));
    }

    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSceneVisualizersTest, "Kya.Editor.SceneVisualizers",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSceneVisualizersTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());

    FKyaLight& Spot = Bank->Lights.AddDefaulted_GetRef();
    Spot.LightType = 0;
    Spot.ShapeType = 4;
    Spot.Position = FVector(1.0, 2.0, 3.0);
    Spot.Direction = FVector(0.0, 0.0, 1.0);
    Spot.Color = FLinearColor::Red;
    Spot.Falloff0 = 2.0f;
    FKyaLight& Sun = Bank->Lights.AddDefaulted_GetRef();
    Sun.ShapeType = 2;
    Sun.Position = FVector(5.0, 5.0, 5.0);
    Sun.Direction = FVector(0.0, -1.0, 0.0);

    Bank->SceneEnvironment.CameraPosition = FVector(-5.0, 3.0, 1.0);
    Bank->SceneEnvironment.CameraLookAt = FVector(0.0, 3.0, 1.0);
    Bank->SceneEnvironment.CameraFov = 70.0f;

    Bank->SceneEnvironment.Actors[0].ClassId = 0x2c;
    Bank->SceneEnvironment.Actors[0].Position = FVector(1.0, 2.0, 3.0);
    Bank->SceneEnvironment.Actors[1].Position = FVector(5.0, 2.0, 3.0);
    Bank->SceneEnvironment.Actors[0].TrapActorRef = 1;
    Bank->SceneEnvironment.Waypoints.AddDefaulted();
    Bank->SceneEnvironment.Waypoints[0].Position = FVector(0.0, 2.0, 3.0);
    FKyaCheckpoint& Checkpoint = Bank->SceneEnvironment.Actors[0].Checkpoints.AddDefaulted_GetRef();
    Checkpoint.WaypointA = 0;

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create visualizer world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn visualizer preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bSpawnActorHandles = false;
    Preview->bSpawnLightVisualizers = true;
    Preview->bSpawnCameraVisualizer = true;
    Preview->bSpawnActorLinkVisualizers = true;
    Preview->bSpawnTriggerVisualizers = true;
    Preview->OnConstruction(FTransform::Identity);

    TestEqual(TEXT("Two lights, one camera, one link and one trigger are visualized"),
        Preview->GetSceneVisualizerCount(), 5);
    TestTrue(TEXT("Unsupported zone geometry is stated explicitly"),
        Preview->VisualizerLimitations.Contains(TEXT("Zones are not visualized")));
    AKyaSceneVisualizer* Light = Preview->GetSceneVisualizer(
        FString::Printf(TEXT("light:%s:0"), *Bank->GetName()));
    AKyaSceneVisualizer* Camera = Preview->GetSceneVisualizer(
        FString::Printf(TEXT("camera:%s"), *Bank->GetName()));
    AKyaSceneVisualizer* Link = Preview->GetSceneVisualizer(
        FString::Printf(TEXT("link:%s:0:TrapActorRef"), *Bank->GetName()));
    AKyaSceneVisualizer* Trigger = Preview->GetSceneVisualizer(
        FString::Printf(TEXT("trigger:%s:0:0"), *Bank->GetName()));
    if (TestNotNull(TEXT("Light visualizer exists"), Light) &&
        TestNotNull(TEXT("Camera visualizer exists"), Camera) &&
        TestNotNull(TEXT("Actor-link visualizer exists"), Link) &&
        TestNotNull(TEXT("Trigger visualizer exists"), Trigger))
    {
        TestTrue(TEXT("Light fields mirror the bank"), Light->LightPosition.Equals(FVector(1.0, 2.0, 3.0)));
        TestTrue(TEXT("Spot light shows a direction arrow"),
            Light->DirectionArrow && Light->DirectionArrow->GetVisibleFlag());
        TestTrue(TEXT("Light semantics distinguish approximate shape"),
            Light->Semantics.Contains(TEXT("approximate")));
        TestTrue(TEXT("Camera semantics distinguish approximate frustum"),
            Camera->Semantics.Contains(TEXT("approximate")));
        TestTrue(TEXT("Link semantics mark derived diagnostics"),
            Link->Semantics.Contains(TEXT("Derived")));
        TestTrue(TEXT("Trigger semantics mark the bound approximate"),
            Trigger->Semantics.Contains(TEXT("Approximate")));
        TestFalse(TEXT("Read-only link visualizer reports not editable"), Link->bRecordEditable);
        TestFalse(TEXT("Read-only trigger visualizer reports not editable"), Trigger->bRecordEditable);

        // Link midpoint maps both endpoints into editor space.
        TestTrue(TEXT("Actor link marker sits at the mapped midpoint"),
            Link->GetActorLocation().Equals(FVector(3.0, 3.0, 2.0) * 100.0, 1.0));
        TestTrue(TEXT("Trigger marker sits at the checkpoint waypoint"),
            Trigger->GetActorLocation().Equals(FVector(0.0, 3.0, 2.0) * 100.0, 1.0));

        // Type/shape edits that change direction/FOV presence are rejected before the
        // record layout can diverge from the archive.
        FKyaLightEditPatch LayoutChange;
        LayoutChange.ShapeType = 0;
        FString LayoutError;
        TestFalse(TEXT("Layout-changing light shape edit is rejected"),
            Service.ApplyLight(Service.MakeLightKey(Bank, 0), LayoutChange, EKyaEditNotify::Silent, &LayoutError));
        TestTrue(TEXT("Light layout rejection explains the archive constraint"),
            LayoutError.Contains(TEXT("layout")));
        TestEqual(TEXT("Rejected light shape edit leaves the record"), Bank->Lights[0].ShapeType, 4);

        // Light gizmo move and rotation write through the service.
        const int64 BuildsBeforeLightEdit = Preview->BuildCount;
        Light->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
        Light->PostEditMove(true);
        TestTrue(TEXT("Light position edit writes the bank"),
            Bank->Lights[0].Position.Equals(FVector(2.0, 2.0, 3.0), 1.0e-4));
        TestEqual(TEXT("Light edit defers the capture rebuild"),
            Preview->BuildCount, BuildsBeforeLightEdit);
        Service.FlushPendingNotifications();
        TestEqual(TEXT("Light edit rebuilds captured lighting after flush"),
            Preview->BuildCount, BuildsBeforeLightEdit + 1);
        TestTrue(TEXT("Translating a light preserves its direction"),
            Bank->Lights[0].Direction.Equals(FVector(0.0, 0.0, 1.0), 1.0e-4));
        Light->SetActorRelativeRotation(FQuat(FVector::UpVector, HALF_PI));
        Light->PostEditMove(true);
        const FVector ExpectedDirection = KyaActorTransform::SwapGameAxes(
            FQuat(FVector::UpVector, HALF_PI).GetForwardVector()).GetSafeNormal();
        TestTrue(TEXT("Spot light rotation writes the bank direction"),
            Bank->Lights[0].Direction.Equals(ExpectedDirection, 1.0e-3));
        Service.FlushPendingNotifications();

        // Two light commits coalesce their capture rebuilds.
        AKyaSceneVisualizer* SunVisualizer = Preview->GetSceneVisualizer(
            FString::Printf(TEXT("light:%s:1"), *Bank->GetName()));
        if (TestNotNull(TEXT("Second light visualizer exists"), SunVisualizer))
        {
            const int64 BuildsBeforeTwoLights = Preview->BuildCount;
            Light->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
            SunVisualizer->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
            Light->PostEditMove(true);
            SunVisualizer->PostEditMove(true);
            TestTrue(TEXT("Both light moves write the bank"),
                Bank->Lights[0].Position.Equals(FVector(3.0, 2.0, 3.0), 1.0e-4) &&
                Bank->Lights[1].Position.Equals(FVector(6.0, 5.0, 5.0), 1.0e-4));
            TestEqual(TEXT("Two light moves do not rebuild mid-commit"),
                Preview->BuildCount, BuildsBeforeTwoLights);
            Service.FlushPendingNotifications();
            TestEqual(TEXT("Two light moves rebuild once after flush"),
                Preview->BuildCount, BuildsBeforeTwoLights + 1);
        }

        // Camera move writes the serialized camera position.
        Camera->AddActorWorldOffset(FVector(0.0, 100.0, 0.0));
        Camera->PostEditMove(true);
        TestTrue(TEXT("Camera position edit writes the bank"),
            Bank->SceneEnvironment.CameraPosition.Equals(FVector(-5.0, 3.0, 2.0), 1.0e-4));
        TestTrue(TEXT("Camera translation preserves the stored look-at"),
            Bank->SceneEnvironment.CameraLookAt.Equals(FVector(0.0, 3.0, 1.0), 1.0e-4));
        const FVector CameraLookAtBefore = Bank->SceneEnvironment.CameraLookAt;
        Camera->PostEditMove(true);
        TestTrue(TEXT("Camera no-op move preserves the look-at"),
            Bank->SceneEnvironment.CameraLookAt.Equals(CameraLookAtBefore, 1.0e-4));

        // Derived visualizers are read-only and explain why.
        const int64 LightCount = Bank->Lights.Num();
        const FVector LinkBefore = Link->GetActorLocation();
        Link->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
        Link->PostEditMove(true);
        TestTrue(TEXT("Link transform is reset"), Link->GetActorLocation().Equals(LinkBefore, 0.01));
        TestTrue(TEXT("Link edit status explains the restriction"),
            Link->EditStatus.Contains(TEXT("derived")));
        Trigger->AddActorWorldOffset(FVector(0.0, 0.0, 100.0));
        Trigger->PostEditMove(true);
        TestTrue(TEXT("Trigger edit status explains the restriction"),
            Trigger->EditStatus.Contains(TEXT("approximate")));
        TestEqual(TEXT("Read-only edits do not add light records"), int64(Bank->Lights.Num()), LightCount);
    }

    // Light arrows follow the preview transform (handle-space direction).
    {
        const FQuat PreviewRotation = FRotator(0.0, 30.0, 0.0).Quaternion();
        Preview->SetActorRotation(PreviewRotation);
        Preview->OnConstruction(Preview->GetActorTransform());
        if (AKyaSceneVisualizer* RebuiltLight = Preview->GetSceneVisualizer(
            FString::Printf(TEXT("light:%s:0"), *Bank->GetName())))
        {
            const FVector Expected = PreviewRotation.RotateVector(
                KyaActorTransform::SwapGameAxes(Bank->Lights[0].Direction).GetSafeNormal());
            TestTrue(TEXT("Light arrow follows a rotated preview"),
                RebuiltLight->DirectionArrow->GetForwardVector().Equals(Expected, 1.0e-3));
        }
        Preview->SetActorRotation(FQuat::Identity);
        Preview->OnConstruction(Preview->GetActorTransform());
    }

    // Turning the options off removes every visualizer.
    Preview->bSpawnLightVisualizers = false;
    Preview->bSpawnCameraVisualizer = false;
    Preview->bSpawnActorLinkVisualizers = false;
    Preview->bSpawnTriggerVisualizers = false;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Disabling visualizers removes them"), Preview->GetSceneVisualizerCount(), 0);

    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaHierarchyGeometryTest, "Kya.Editor.HierarchyGeometry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaHierarchyGeometryTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the hierarchy geometry test."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read hierarchy bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import hierarchy bank"), Bank)) return false;

    // Cluster hierarchies come from sector banks; load one alongside the main bank.
    const FString SectorFilename = FPaths::Combine(FPaths::GetPath(Filename), TEXT("SECT1.BNK"));
    TArray<uint8> SectorInput;
    UKyaLevelBank* SectorBank = nullptr;
    if (TestTrue(TEXT("Read sector bank"), FFileHelper::LoadFileToArray(SectorInput, *SectorFilename)))
    {
        const uint8* SectorCursor = SectorInput.GetData();
        SectorBank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), SectorCursor,
            SectorInput.GetData() + SectorInput.Num(), GWarn));
        if (TestNotNull(TEXT("Import sector bank"), SectorBank))
            SectorBank->SourceFilename = SectorFilename;
    }

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create hierarchy world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn hierarchy preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    if (SectorBank) Preview->SectorBanks.Add(SectorBank);
    Preview->bSpawnActorHandles = false;
    Preview->bShowClusterMeshes = true;

    const auto CountMesh = [](UProceduralMeshComponent* Mesh, int32& OutVertices, int32& OutTriangles)
    {
        OutVertices = 0;
        OutTriangles = 0;
        for (int32 Section = 0; Section < Mesh->GetNumSections(); ++Section)
        {
            if (const FProcMeshSection* MeshSection = Mesh->GetProcMeshSection(Section))
            {
                OutVertices += MeshSection->ProcVertexBuffer.Num();
                OutTriangles += MeshSection->ProcIndexBuffer.Num() / 3;
            }
        }
    };

    // Baseline: hierarchy geometry lives in the shared preview mesh.
    Preview->bSpawnHierarchyHandles = false;
    Preview->OnConstruction(FTransform::Identity);
    int32 SharedVertices = 0, SharedTriangles = 0;
    CountMesh(Preview->Mesh, SharedVertices, SharedTriangles);
    TestTrue(TEXT("The level has decoded hierarchies"), Preview->GetHierarchySelectionCount() > 0);

    // Editing mode: each hierarchy owns its rendered strips and is selectable by click.
    Preview->bSpawnHierarchyHandles = true;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("One handle per decoded hierarchy"),
        Preview->GetHierarchyHandleCount(), Preview->GetHierarchySelectionCount());
    int32 HandleTriangles = 0;
    int32 MeshedHandles = 0;
    AKyaHierarchyHandle* Sample = nullptr;
    for (int32 HandleIndex = 0; HandleIndex < Preview->GetHierarchyHandleCount(); ++HandleIndex)
    {
        AKyaHierarchyHandle* Handle = Preview->GetHierarchyHandle(HandleIndex);
        if (!IsValid(Handle)) continue;
        int32 HandleVertices = 0, Triangles = 0;
        CountMesh(Handle->HierarchyMesh, HandleVertices, Triangles);
        if (Triangles == 0) continue;
        ++MeshedHandles;
        HandleTriangles += Triangles;
        if (!Sample) Sample = Handle;
    }
    int32 SharedVerticesWithHandles = 0, SharedTrianglesWithHandles = 0;
    CountMesh(Preview->Mesh, SharedVerticesWithHandles, SharedTrianglesWithHandles);
    AddInfo(FString::Printf(TEXT("hierarchies=%d meshed=%d handle_triangles=%d shared_before=%d shared_after=%d"),
        Preview->GetHierarchyHandleCount(), MeshedHandles, HandleTriangles, SharedTriangles, SharedTrianglesWithHandles));

    TestTrue(TEXT("Hierarchy handles own rendered geometry"), MeshedHandles > 0);
    TestTrue(TEXT("Hierarchy handles carry triangles"), HandleTriangles > 0);
    TestTrue(TEXT("Hierarchy geometry is not duplicated in the shared mesh"),
        SharedTrianglesWithHandles < SharedTriangles);
    if (TestNotNull(TEXT("A meshed hierarchy handle exists"), Sample))
    {
        TestTrue(TEXT("Handle reports valid world bounds"), Sample->WorldBounds.IsValid != 0);
        TestTrue(TEXT("Handle reports material information"), !Sample->MaterialNames.IsEmpty());
        TestTrue(TEXT("Handle reports its source offset"), Sample->HierarchyOffset >= 0);
        TestTrue(TEXT("Handle reports its owning sector"), Sample->SectorId >= -1);
        TestTrue(TEXT("Handle mesh sections are selectable"), Sample->HierarchyMesh->bSelectable);
        TestFalse(TEXT("Meshed handles hide the fallback marker"),
            Sample->Marker && Sample->Marker->GetVisibleFlag());

        // Hierarchy transforms are presentation-only and cannot drift.
        Sample->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
        Sample->PostEditMove(true);
        TestTrue(TEXT("Hierarchy handle transform is reset"),
            Sample->GetActorTransform().GetRelativeTransform(Preview->GetActorTransform())
                .Equals(FTransform::Identity, 0.01));
        TestTrue(TEXT("Transform reset explains that writeback is unsupported"),
            Sample->TransformStatus.Contains(TEXT("not serialized")));
        TestTrue(TEXT("Hierarchy mesh stays at the preview transform"),
            Sample->HierarchyMesh->GetComponentLocation().Equals(Preview->GetActorLocation(), 0.01));

        // Source origin is serialized game-space data and must not change with the
        // preview transform or a rebuild.
        const int32 Offset = Sample->HierarchyOffset;
        UKyaLevelBank* SampleBank = Sample->Bank;
        const FVector SourceOrigin = Sample->SourceOrigin;
        Preview->SetActorRotation(FRotator(0.0, 30.0, 0.0).Quaternion());
        Preview->OnConstruction(Preview->GetActorTransform());
        for (int32 HandleIndex = 0; HandleIndex < Preview->GetHierarchyHandleCount(); ++HandleIndex)
        {
            AKyaHierarchyHandle* Rebuilt = Preview->GetHierarchyHandle(HandleIndex);
            if (IsValid(Rebuilt) && Rebuilt->Bank == SampleBank && Rebuilt->HierarchyOffset == Offset)
            {
                TestTrue(TEXT("Source origin is stable across a transformed rebuild"),
                    Rebuilt->SourceOrigin.Equals(SourceOrigin, 1.0e-4));
                break;
            }
        }
        Preview->SetActorRotation(FQuat::Identity);
        Preview->OnConstruction(Preview->GetActorTransform());

        // Visibility remains presentation-only: hiding removes the geometry, showing
        // rebuilds it, and the bank bytes are untouched.
        const int64 BeforeBytes = SampleBank ? SampleBank->BankBytes.Num() : 0;
        Preview->SetHierarchyVisible(SampleBank, Offset, false);
        AKyaHierarchyHandle* Hidden = nullptr;
        for (int32 HandleIndex = 0; HandleIndex < Preview->GetHierarchyHandleCount(); ++HandleIndex)
        {
            AKyaHierarchyHandle* Handle = Preview->GetHierarchyHandle(HandleIndex);
            if (IsValid(Handle) && Handle->Bank == SampleBank && Handle->HierarchyOffset == Offset)
                Hidden = Handle;
        }
        if (TestNotNull(TEXT("Hidden hierarchy handle still exists"), Hidden))
        {
            TestFalse(TEXT("Hidden hierarchy reports not visible"), Hidden->bVisible);
            int32 HiddenVertices = 0, HiddenTriangles = 0;
            CountMesh(Hidden->HierarchyMesh, HiddenVertices, HiddenTriangles);
            TestEqual(TEXT("Hidden hierarchy has no rendered geometry"), HiddenTriangles, 0);
            TestTrue(TEXT("Hidden hierarchy shows its fallback marker"), Hidden->Marker->GetVisibleFlag());
        }
        Preview->SetHierarchyVisible(SampleBank, Offset, true);
        TestEqual(TEXT("Visibility toggles do not modify bank bytes"),
            int64(SampleBank ? SampleBank->BankBytes.Num() : 0), BeforeBytes);
    }

    // Hide and isolate are presentation-only for hierarchy handles too.
    if (AKyaHierarchyHandle* Presentation = Preview->GetHierarchyHandle(0))
    {
        AKyaHierarchyHandle* Other = nullptr;
        for (int32 HandleIndex = 1; HandleIndex < Preview->GetHierarchyHandleCount(); ++HandleIndex)
        {
            AKyaHierarchyHandle* Candidate = Preview->GetHierarchyHandle(HandleIndex);
            if (IsValid(Candidate) && Candidate != Presentation)
            {
                Other = Candidate;
                break;
            }
        }
        const int64 MainBytes = Bank->BankBytes.Num();
        const int64 SectorBytes = SectorBank ? SectorBank->BankBytes.Num() : 0;

        Preview->SetGeneratedActorTemporarilyHidden(Presentation, true);
        TestTrue(TEXT("Hierarchy handle can be temporarily hidden"),
            Presentation->IsTemporarilyHiddenInEditor());
        if (Other) TestFalse(TEXT("Hiding one hierarchy leaves others visible"),
            Other->IsTemporarilyHiddenInEditor());
        Preview->OnConstruction(Preview->GetActorTransform());
        if (AKyaHierarchyHandle* Rebuilt = Preview->GetHierarchyHandle(0))
            TestTrue(TEXT("Hierarchy hiding survives a rebuild"), Rebuilt->IsTemporarilyHiddenInEditor());
        Preview->SetGeneratedActorTemporarilyHidden(Presentation, false);
        TestFalse(TEXT("Showing a hierarchy restores it"), Presentation->IsTemporarilyHiddenInEditor());

        Preview->IsolateGeneratedActors({Presentation});
        TestTrue(TEXT("Isolated hierarchy is visible"), !Presentation->IsTemporarilyHiddenInEditor());
        if (Other) TestTrue(TEXT("Isolation hides other hierarchy handles"),
            Other->IsTemporarilyHiddenInEditor());
        Preview->ClearGeneratedActorIsolation();
        if (Other) TestFalse(TEXT("Clearing isolation restores hierarchy handles"),
            Other->IsTemporarilyHiddenInEditor());

        TestEqual(TEXT("Hide/isolate leaves the main bank bytes unchanged"),
            int64(Bank->BankBytes.Num()), MainBytes);
        TestEqual(TEXT("Hide/isolate leaves the sector bank bytes unchanged"),
            int64(SectorBank ? SectorBank->BankBytes.Num() : 0), SectorBytes);
    }

    // A game/PIE world has no handles, so the shared mesh must keep the geometry.
    UWorld* GameWorld = UWorld::CreateWorld(EWorldType::Game, false);
    if (TestNotNull(TEXT("Create hierarchy game world"), GameWorld))
    {
        AKyaLevelPreview* GamePreview = GameWorld->SpawnActor<AKyaLevelPreview>();
        if (TestNotNull(TEXT("Spawn hierarchy game preview"), GamePreview))
        {
            GamePreview->Bank = Bank;
            if (SectorBank) GamePreview->SectorBanks.Add(SectorBank);
            GamePreview->bSpawnActorHandles = false;
            GamePreview->bShowClusterMeshes = true;
            GamePreview->bSpawnHierarchyHandles = true;
            GamePreview->OnConstruction(FTransform::Identity);
            int32 GameVertices = 0, GameTriangles = 0;
            CountMesh(GamePreview->Mesh, GameVertices, GameTriangles);
            TestEqual(TEXT("Game worlds create no hierarchy handles"), GamePreview->GetHierarchyHandleCount(), 0);
            TestEqual(TEXT("Game worlds keep hierarchy geometry in the shared mesh"),
                GameTriangles, SharedTriangles);
            GamePreview->Destroy();
        }
        GameWorld->DestroyWorld(false);
    }

    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaWaypointOrientationTest, "Kya.Editor.WaypointOrientation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaWaypointOrientationTest::RunTest(const FString& Parameters)
{
    // The repository's SetVectorFromAngles uses edF32Matrix4FromEulerSoft(..., "XYZ").
    // With yaw +90 the stored forward image is game +X; after the editor's axis swap
    // the visible arrow must point along Unreal +X.
    FVector BasisX, BasisY, BasisZ;
    KyaActorTransform::BuildEulerRotationBasis(FVector(0.0, HALF_PI, 0.0), BasisX, BasisY, BasisZ);
    TestTrue(TEXT("Stored yaw rotates game forward onto game +X"),
        BasisZ.Equals(FVector(1.0, 0.0, 0.0), 1.0e-4));
    const FQuat Yaw90 = KyaActorTransform::QuatFromGameEuler(FVector(0.0, HALF_PI, 0.0));
    FVector RoundTrip;
    TestTrue(TEXT("Euler inverse accepts the stored orientation"),
        KyaActorTransform::GameEulerFromQuat(Yaw90, FVector(0.0, HALF_PI, 0.0), RoundTrip));
    TestTrue(TEXT("Euler inverse round trips the angles"),
        RoundTrip.Equals(FVector(0.0, HALF_PI, 0.0), 1.0e-4));

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    FKyaWaypoint& Waypoint = Bank->SceneEnvironment.Waypoints.AddDefaulted_GetRef();
    Waypoint.Position = FVector(1.0, 2.0, 3.0);
    Waypoint.Rotation = FVector(0.0, HALF_PI, 0.0);
    FKyaPath& Path = Bank->SceneEnvironment.Paths.AddDefaulted_GetRef();
    Path.Points.Add(FVector4(4.0, 5.0, 6.0, 1.0));
    Path.Rotations.Add(FVector4(0.0, 0.25, 0.0, 0.0));

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create orientation world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn orientation preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bSpawnActorHandles = false;
    Preview->bSpawnWaypointHandles = true;
    Preview->bSpawnPathPointHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    AKyaSpatialRecordHandle* WaypointHandle = Cast<AKyaSpatialRecordHandle>(Preview->GetWaypointHandle(0));
    AKyaSpatialRecordHandle* PathHandle = Cast<AKyaSpatialRecordHandle>(Preview->GetPathPointHandle(0, 0));
    if (TestNotNull(TEXT("Waypoint orientation handle exists"), WaypointHandle) &&
        TestNotNull(TEXT("Path point handle exists"), PathHandle))
    {
        TestTrue(TEXT("Waypoint handle mirrors the stored orientation"),
            WaypointHandle->GetActorQuat().Equals(Yaw90, 1.0e-4));
        TestTrue(TEXT("Waypoint handle shows the forward arrow"),
            WaypointHandle->OrientationArrow && WaypointHandle->OrientationArrow->GetVisibleFlag());
        TestTrue(TEXT("Waypoint arrow shows the stored forward direction"),
            WaypointHandle->OrientationArrow->GetForwardVector().Equals(FVector(1.0, 0.0, 0.0), 1.0e-3));

        // The arrow follows the preview's transform, not just the handle's local space.
        const FQuat PreviewRotation = FRotator(0.0, 45.0, 0.0).Quaternion();
        Preview->SetActorRotation(PreviewRotation);
        Preview->OnConstruction(Preview->GetActorTransform());
        if (AKyaSpatialRecordHandle* RotatedHandle = Cast<AKyaSpatialRecordHandle>(Preview->GetWaypointHandle(0)))
        {
            TestTrue(TEXT("Arrow follows a rotated preview"),
                RotatedHandle->OrientationArrow->GetForwardVector().Equals(
                    PreviewRotation.RotateVector(FVector(1.0, 0.0, 0.0)), 1.0e-3));
        }
        Preview->SetActorRotation(FQuat::Identity);
        Preview->OnConstruction(Preview->GetActorTransform());
        WaypointHandle = Cast<AKyaSpatialRecordHandle>(Preview->GetWaypointHandle(0));
        PathHandle = Cast<AKyaSpatialRecordHandle>(Preview->GetPathPointHandle(0, 0));
        TestFalse(TEXT("Path point handle hides the waypoint arrow"),
            PathHandle->OrientationArrow && PathHandle->OrientationArrow->GetVisibleFlag());
        TestTrue(TEXT("Path point handle stays unrotated"), PathHandle->GetActorQuat().IsIdentity(1.0e-4));

        // Gizmo rotation writes the stored Euler triple.
        const FQuat Yaw180 = KyaActorTransform::QuatFromGameEuler(FVector(0.0, PI, 0.0));
        WaypointHandle->SetActorRelativeRotation(Yaw180);
        WaypointHandle->PostEditMove(true);
        FVector RotatedX, RotatedY, RotatedZ;
        KyaActorTransform::BuildEulerRotationBasis(Bank->SceneEnvironment.Waypoints[0].Rotation,
            RotatedX, RotatedY, RotatedZ);
        TestTrue(TEXT("Gizmo rotation writes the bank orientation"),
            RotatedZ.Equals(FVector(0.0, 0.0, -1.0), 1.0e-3));
        TestTrue(TEXT("Handle arrow follows the committed orientation"),
            WaypointHandle->OrientationArrow->GetForwardVector().Equals(FVector(0.0, -1.0, 0.0), 1.0e-3));
        Service.FlushPendingNotifications();

        // Translation must preserve the stored orientation exactly.
        const FVector StoredRotation = Bank->SceneEnvironment.Waypoints[0].Rotation;
        WaypointHandle->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
        WaypointHandle->PostEditMove(true);
        TestTrue(TEXT("Translate keeps the stored orientation"),
            Bank->SceneEnvironment.Waypoints[0].Rotation.Equals(StoredRotation, 1.0e-4));
        TestTrue(TEXT("Translate still writes the position"),
            Bank->SceneEnvironment.Waypoints[0].Position.Equals(FVector(2.0, 2.0, 3.0), 1.0e-4));

        // Path-point rotation remains a Details/export field, not a gizmo edit.
        const FVector4 StoredPathRotation = Bank->SceneEnvironment.Paths[0].Rotations[0];
        PathHandle->SetActorRelativeRotation(FQuat(FVector::UpVector, 0.5));
        PathHandle->PostEditMove(true);
        TestTrue(TEXT("Path point gizmo rotation does not change serialized rotation"),
            Bank->SceneEnvironment.Paths[0].Rotations[0].Equals(StoredPathRotation, 1.0e-6));
        TestTrue(TEXT("Path point handle rotation is reset after commit"),
            PathHandle->GetActorQuat().IsIdentity(1.0e-4));
        Service.FlushPendingNotifications();
    }

    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaPathVisualizationTest, "Kya.Editor.PathVisualization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaPathVisualizationTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    FKyaPath& Path0 = Bank->SceneEnvironment.Paths.AddDefaulted_GetRef();
    Path0.Points = {FVector4(1.0, 2.0, 3.0, 1.0), FVector4(4.0, 5.0, 6.0, 1.0)};
    FKyaPath& Path1 = Bank->SceneEnvironment.Paths.AddDefaulted_GetRef();
    Path1.Points = {FVector4(-1.0, -2.0, -3.0, 1.0), FVector4(7.0, 8.0, 9.0, 1.0),
        FVector4(10.0, 11.0, 12.0, 1.0)};

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create path world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn path preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->PreviewPathIndex = -1;
    Preview->bSpawnActorHandles = false;
    Preview->bShowAllPaths = true;
    Preview->OnConstruction(FTransform::Identity);

    // Every path is shown without a numeric preview index, with the game's linear
    // point-to-point interpolation.
    TestEqual(TEXT("All paths produce guides"), Preview->GetPathGuideCount(), 2);
    USplineComponent* Guide0 = Preview->GetPathGuide(0);
    USplineComponent* Guide1 = Preview->GetPathGuide(1);
    if (TestNotNull(TEXT("Path 0 guide exists"), Guide0) && TestNotNull(TEXT("Path 1 guide exists"), Guide1))
    {
        TestEqual(TEXT("Path 0 guide point count"), Guide0->GetNumberOfSplinePoints(), 2);
        TestEqual(TEXT("Path 1 guide point count"), Guide1->GetNumberOfSplinePoints(), 3);
        const auto IsLinear = [](const USplineComponent* Guide)
        {
            for (int32 Index = 0; Index < Guide->GetNumberOfSplinePoints(); ++Index)
                if (Guide->GetSplinePointType(Index) != ESplinePointType::Linear) return false;
            return true;
        };
        TestTrue(TEXT("Path 0 keeps linear interpolation"), IsLinear(Guide0));
        TestTrue(TEXT("Path 1 keeps linear interpolation"), IsLinear(Guide1));
        TestTrue(TEXT("Path 0 guide maps game axes"),
            Guide0->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local).Equals(
                FVector(1.0, 3.0, 2.0) * 100.0, 0.01));
    }

    // Selectable points are created for every path and grouped by path ownership.
    AKyaSpatialRecordHandle* Point = Cast<AKyaSpatialRecordHandle>(Preview->GetPathPointHandle(0, 0));
    AKyaSpatialRecordHandle* LastPoint = Cast<AKyaSpatialRecordHandle>(Preview->GetPathPointHandle(1, 2));
    AActor* PathGroup0 = Preview->GetPathGroup(0);
    AActor* PathGroup1 = Preview->GetPathGroup(1);
    if (TestNotNull(TEXT("First path point handle exists"), Point) &&
        TestNotNull(TEXT("Last path point handle exists"), LastPoint))
    {
        TestEqual(TEXT("All-path mode creates selectable points"), Preview->GetSpatialHandleCount(), 5);
        TestTrue(TEXT("Path points are owned by their path group"), Point->GetAttachParentActor() == PathGroup0);
        TestEqual(TEXT("Path 0 group names its ownership"), PathGroup0
            ? PathGroup0->GetActorLabel() : FString(), FString::Printf(TEXT("%s path 0"), *Bank->GetName()));
        TestTrue(TEXT("Path 1 group identifies a different path"),
            PathGroup1 && PathGroup1->GetActorLabel().Contains(TEXT("path 1")));
        TestTrue(TEXT("Path groups are transient"), PathGroup0 && PathGroup0->HasAnyFlags(RF_Transient));

        // Moving a point writes the bank and updates the guide it owns.
        Point->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
        Point->PostEditMove(true);
        TestTrue(TEXT("Path point handle writes the bank"),
            Bank->SceneEnvironment.Paths[0].Points[0].Equals(FVector4(2.0, 2.0, 3.0, 1.0)));
        if (Guide0)
            TestTrue(TEXT("Path guide follows the edited point"),
                Guide0->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local).Equals(
                    FVector(2.0, 3.0, 2.0) * 100.0, 0.01));
        FKyaLevelEditService::Get().FlushPendingNotifications();
    }

    // Turning the mode off removes guides, point handles and path groups.
    Preview->bShowAllPaths = false;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Disabling all-path mode removes guides"), Preview->GetPathGuideCount(), 0);
    TestEqual(TEXT("Disabling all-path mode removes point handles"), Preview->GetSpatialHandleCount(), 0);
    TestNull(TEXT("Disabling all-path mode removes path groups"), Preview->GetPathGroup(0));

    // The legacy numeric index still draws exactly one guide when all-path mode is off.
    Preview->PreviewPathIndex = 1;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Preview path index draws one guide"), Preview->GetPathGuideCount(), 1);
    TestNotNull(TEXT("Preview path index draws the requested guide"), Preview->GetPathGuide(1));
    TestNull(TEXT("Preview path index does not draw other guides"), Preview->GetPathGuide(0));

    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaOutlinerPolicyTest, "Kya.Editor.OutlinerPolicy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaOutlinerPolicyTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    Bank->SceneEnvironment.Waypoints.AddDefaulted();
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create outliner world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn outliner preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bSpawnWaypointHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    AKyaActorHandle* First = Cast<AKyaActorHandle>(Preview->GetActorHandle(0));
    AKyaActorHandle* Second = Cast<AKyaActorHandle>(Preview->GetActorHandle(1));
    AKyaSpatialRecordHandle* Waypoint = Cast<AKyaSpatialRecordHandle>(Preview->GetWaypointHandle(0));
    if (!TestNotNull(TEXT("First handle exists"), First) || !TestNotNull(TEXT("Second handle exists"), Second) ||
        !TestNotNull(TEXT("Waypoint handle exists"), Waypoint))
    {
        Preview->Destroy();
        World->DestroyWorld(false);
        return false;
    }

    // --- Rename policy: the label is presentation, the serialized name is the edit ---
    FString Error;
    TestFalse(TEXT("Rename crossing the 4-byte slot is rejected"),
        ValidateKyaActorNameChange(TEXT("FIRST"), TEXT("FIR"), Error));
    TestTrue(TEXT("Slot rejection explains the alignment"), Error.Contains(TEXT("4-byte")));
    TestFalse(TEXT("Empty rename is rejected"),
        ValidateKyaActorNameChange(TEXT("FIRST"), TEXT(""), Error));
    TestFalse(TEXT("Non-ASCII rename is rejected"),
        ValidateKyaActorNameChange(TEXT("FIRST"), TEXT("FIRST\u00e9"), Error));
    TestTrue(TEXT("Same-bucket rename is accepted"),
        ValidateKyaActorNameChange(TEXT("FIRST"), TEXT("FIRS"), Error));

    TestTrue(TEXT("Direct same-bucket rename applies"), First->RenameActorRecord(TEXT("FIRS"), Error));
    TestEqual(TEXT("Rename writes the serialized name"), Bank->SceneEnvironment.Actors[0].Name, FString(TEXT("FIRS")));
    TestTrue(TEXT("Rename reapplies the class suffix"),
        First->GetActorLabel().StartsWith(TEXT("FIRS [class 0]")));
    TestFalse(TEXT("Direct boundary-crossing rename is rejected"),
        First->RenameActorRecord(TEXT("FIRSTX12"), Error));
    TestEqual(TEXT("Rejected rename leaves the bank untouched"),
        Bank->SceneEnvironment.Actors[0].Name, FString(TEXT("FIRS")));

    // An Outliner rename goes through SetActorLabel -> PostEditChangeProperty.
    First->SetActorLabel(TEXT("FIRSX [class 0]"));
    TestEqual(TEXT("Outliner rename strips the class suffix and writes the bank"),
        Bank->SceneEnvironment.Actors[0].Name, FString(TEXT("FIRSX")));
    First->SetActorLabel(TEXT("NO"));
    TestEqual(TEXT("Rejected Outliner rename leaves the bank untouched"),
        Bank->SceneEnvironment.Actors[0].Name, FString(TEXT("FIRSX")));
    TestTrue(TEXT("Rejected Outliner rename restores the derived label"),
        First->GetActorLabel().StartsWith(TEXT("FIRSX [class 0]")));

    // Non-actor representations have generated labels that cannot diverge from records.
    const FString WaypointLabel = Waypoint->GetActorLabel();
    Waypoint->SetActorLabel(TEXT("Renamed waypoint"));
    TestEqual(TEXT("Waypoint rename is reverted"), Waypoint->GetActorLabel(), WaypointLabel);
    if (AActor* Group = First->GetAttachParentActor())
    {
        const FString GroupLabel = Group->GetActorLabel();
        Group->SetActorLabel(TEXT("Renamed group"));
        TestEqual(TEXT("Group rename is reverted"), Group->GetActorLabel(), GroupLabel);
    }

    // --- Editor-only temporary hide and isolate ------------------------------------
    Preview->SetGeneratedActorTemporarilyHidden(First, true);
    TestTrue(TEXT("Hidden representation reports hidden"), First->IsTemporarilyHiddenInEditor());
    TestFalse(TEXT("Other representation stays visible"), Second->IsTemporarilyHiddenInEditor());
    TestFalse(TEXT("Temporary hiding does not use game visibility"), First->IsHidden());
    Preview->OnConstruction(FTransform::Identity);
    TestTrue(TEXT("Temporary hiding survives a rebuild"), First->IsTemporarilyHiddenInEditor());
    Preview->SetGeneratedActorTemporarilyHidden(First, false);
    TestFalse(TEXT("Showing a representation restores it"), First->IsTemporarilyHiddenInEditor());

    Preview->IsolateGeneratedActors({First});
    TestTrue(TEXT("Isolation reports the isolated actor"), Preview->IsGeneratedActorIsolated(First));
    TestFalse(TEXT("Isolated actor is visible"), First->IsTemporarilyHiddenInEditor());
    TestTrue(TEXT("Isolation hides other representations"), Second->IsTemporarilyHiddenInEditor());
    Preview->OnConstruction(FTransform::Identity);
    TestTrue(TEXT("Isolation survives a rebuild"), Second->IsTemporarilyHiddenInEditor());
    Preview->ClearGeneratedActorIsolation();
    TestFalse(TEXT("Clearing isolation shows every representation"), Second->IsTemporarilyHiddenInEditor());

    // Generated duplicate suffixes are stripped in the correct order (index first,
    // then class) and only when they match this record.
    Bank->SceneEnvironment.Actors[0].Name = TEXT("DUP");
    Bank->SceneEnvironment.Actors[1].Name = TEXT("DUP");
    Preview->OnConstruction(FTransform::Identity);
    if (AKyaActorHandle* DupFirst = Cast<AKyaActorHandle>(Preview->GetActorHandle(0)))
    {
        TestTrue(TEXT("Duplicate label carries both generated suffixes"),
            DupFirst->GetActorLabel() == TEXT("DUP [class 0] [0]"));
        DupFirst->SetActorLabel(TEXT("DU [class 0] [0]"));
        TestEqual(TEXT("Rename strips the index suffix before the class suffix"),
            Bank->SceneEnvironment.Actors[0].Name, FString(TEXT("DU")));
    }
    // Bracketed text that is not this record's generated suffix is authored content.
    Bank->SceneEnvironment.Actors[1].Name = TEXT("FIRST");
    Preview->OnConstruction(FTransform::Identity);
    if (AKyaActorHandle* BracketHandle = Cast<AKyaActorHandle>(Preview->GetActorHandle(1)))
    {
        TestTrue(TEXT("Literal bracket name is accepted"),
            BracketHandle->RenameActorRecord(TEXT("B [9]"), Error));
        TestEqual(TEXT("Literal bracket text is preserved"),
            Bank->SceneEnvironment.Actors[1].Name, FString(TEXT("B [9]")));
        TestTrue(TEXT("Authored text matching the record index is accepted"),
            BracketHandle->RenameActorRecord(TEXT("B [1]"), Error));
        TestEqual(TEXT("Authored index-like text is not stripped"),
            Bank->SceneEnvironment.Actors[1].Name, FString(TEXT("B [1]")));
    }
    // A duplicated copy cannot rename the source record.
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.ObjectFlags |= RF_Transient | RF_Transactional;
        AKyaActorHandle* Clone = World->SpawnActor<AKyaActorHandle>(
            AKyaActorHandle::StaticClass(), FTransform::Identity, Params);
        if (TestNotNull(TEXT("Rename clone spawns"), Clone))
        {
            Clone->Initialize(Preview, Bank, 1, true);
            const FString BeforeRename = Bank->SceneEnvironment.Actors[1].Name;
            TestFalse(TEXT("Unowned copy cannot rename the record"),
                Clone->RenameActorRecord(TEXT("CLONE"), Error));
            TestEqual(TEXT("Unowned rename leaves the bank untouched"),
                Bank->SceneEnvironment.Actors[1].Name, BeforeRename);
            Clone->Destroy();
        }
    }

    // Grouping actors inherit hide/isolate state to their handles.
    Bank->SceneEnvironment.Actors[1].SectorId = 1;
    Preview->OnConstruction(FTransform::Identity);
    if (AKyaActorHandle* GroupedFirst = Cast<AKyaActorHandle>(Preview->GetActorHandle(0)))
    {
        if (AKyaActorHandle* GroupedSecond = Cast<AKyaActorHandle>(Preview->GetActorHandle(1)))
        {
            AActor* Group0 = GroupedFirst->GetAttachParentActor();
            AActor* Group1 = GroupedSecond->GetAttachParentActor();
            if (TestNotNull(TEXT("First sector group exists"), Group0) &&
                TestNotNull(TEXT("Second sector group exists"), Group1) && Group0 != Group1)
            {
                Preview->SetGeneratedActorTemporarilyHidden(Group0, true);
                TestTrue(TEXT("Hiding a group hides its handle"), GroupedFirst->IsTemporarilyHiddenInEditor());
                TestFalse(TEXT("Hiding a group leaves other handles visible"),
                    GroupedSecond->IsTemporarilyHiddenInEditor());
                Preview->SetGeneratedActorTemporarilyHidden(Group0, false);

                Preview->IsolateGeneratedActors({Group0});
                TestFalse(TEXT("Isolating a group keeps its handle visible"),
                    GroupedFirst->IsTemporarilyHiddenInEditor());
                TestTrue(TEXT("Isolating a group hides other groups' handles"),
                    GroupedSecond->IsTemporarilyHiddenInEditor());
                Preview->ClearGeneratedActorIsolation();
                TestFalse(TEXT("Clearing group isolation restores everything"),
                    GroupedSecond->IsTemporarilyHiddenInEditor());
            }
        }
    }

    // --- Selection locking ---------------------------------------------------------
    TestTrue(TEXT("Unlocked representations are selectable"), First->IsSelectable());
    Preview->bLockGeneratedActorSelection = true;
    TestFalse(TEXT("Preview lock disables picking"), First->IsSelectable());
    TestFalse(TEXT("Preview lock also covers spatial records"), Waypoint->IsSelectable());
    Preview->bLockGeneratedActorSelection = false;
    First->bSelectionLocked = true;
    TestFalse(TEXT("Per-representation lock disables picking"), First->IsSelectable());
    TestTrue(TEXT("Other representations remain selectable"), Second->IsSelectable());
    First->bSelectionLocked = false;

    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMultiEditBaselineTest, "Kya.Editor.MultiEditBaseline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMultiEditBaselineTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    CaptureKyaImportedBaseline(*Bank);
    if (!TestTrue(TEXT("Imported baseline is captured"), HasKyaImportedBaseline(*Bank))) return false;

    const FKyaActorRecordKey Key0 = Service.MakeKey(Bank, 0);
    const FKyaActorRecordKey Key1 = Service.MakeKey(Bank, 1);
    FString Error;

    // Reset-to-imported restores the whole record.
    FKyaActorEditPatch Move;
    Move.Position = FVector(9.0, 9.0, 9.0);
    Move.VisibilityDistance = 42.0f;
    TestTrue(TEXT("Edit applies before reset"), Service.Apply(Key0, Move, EKyaEditNotify::Silent, &Error));
    TestTrue(TEXT("Record is modified"), KyaActorDiffersFromImported(*Bank, 0));
    TestFalse(TEXT("Untouched record matches baseline"), KyaActorDiffersFromImported(*Bank, 1));
    FKyaActorEditPatch ResetActor;
    TestTrue(TEXT("Reset patch builds"), BuildKyaActorResetPatch(*Bank, 0, ResetActor));
    TestTrue(TEXT("Reset patch applies"), Service.Apply(Key0, ResetActor, EKyaEditNotify::Silent, &Error));
    TestFalse(TEXT("Reset record matches baseline"), KyaActorDiffersFromImported(*Bank, 0));
    TestTrue(TEXT("Reset restores position"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(1.0, 2.0, 3.0)));
    TestEqual(TEXT("Reset restores visibility"), Bank->SceneEnvironment.Actors[0].VisibilityDistance, 0.0f);

    // Field reset restores one property and leaves the other edit in place.
    FKyaActorEditPatch EditTwo;
    EditTwo.Position = FVector(5.0, 6.0, 7.0);
    EditTwo.VisibilityDistance = 99.0f;
    TestTrue(TEXT("Second edit applies"), Service.Apply(Key0, EditTwo, EKyaEditNotify::Silent, &Error));
    FKyaActorEditPatch ResetField;
    TestTrue(TEXT("Field reset patch builds"),
        BuildKyaActorFieldResetPatch(*Bank, 0, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, VisibilityDistance),
            ResetField));
    TestTrue(TEXT("Field reset applies"), Service.Apply(Key0, ResetField, EKyaEditNotify::Silent, &Error));
    TestEqual(TEXT("Field reset restores the field"), Bank->SceneEnvironment.Actors[0].VisibilityDistance, 0.0f);
    TestTrue(TEXT("Field reset keeps the other edit"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(5.0, 6.0, 7.0)));

    // Property copy from another actor copies only the chosen field.
    Bank->SceneEnvironment.Actors[1].VisibilityDistance = 77.0f;
    Bank->SceneEnvironment.Actors[1].Position = FVector(0.0, 1.0, 0.0);
    FKyaActorEditPatch CopyPatch;
    TestTrue(TEXT("Copy patch builds"), KyaMakePlacementPropertyPatch(Bank->SceneEnvironment.Actors[1],
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, VisibilityDistance), CopyPatch, &Error));
    TestTrue(TEXT("Copy patch applies"), Service.Apply(Key0, CopyPatch, EKyaEditNotify::Silent, &Error));
    TestEqual(TEXT("Copy writes the field"), Bank->SceneEnvironment.Actors[0].VisibilityDistance, 77.0f);
    TestTrue(TEXT("Copy keeps the target position"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(5.0, 6.0, 7.0)));

    // Vector/float accessors used by the multi-edit panel.
    FVector ReadVector;
    TestTrue(TEXT("Vector accessor reads"), GetKyaVectorField(Bank->SceneEnvironment.Actors[0],
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Position), ReadVector) && ReadVector.Equals(FVector(5.0, 6.0, 7.0)));
    FKyaActorPlacement VectorTarget = Bank->SceneEnvironment.Actors[0];
    TestTrue(TEXT("Vector accessor writes"), SetKyaVectorField(VectorTarget,
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Rotation), FVector(0.3, 0.4, 0.5)));
    FVector ReadRotation;
    TestTrue(TEXT("Vector accessor round trips"), GetKyaVectorField(VectorTarget,
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Rotation), ReadRotation) &&
        ReadRotation.Equals(FVector(0.3, 0.4, 0.5)));

    // Bulk edit applies one patch to all records and is one undo step.
    const int32 BeforeSector0 = Bank->SceneEnvironment.Actors[0].SectorId;
    const int32 BeforeSector1 = Bank->SceneEnvironment.Actors[1].SectorId;
    FKyaActorEditPatch Bulk;
    Bulk.SectorId = 3;
    TestTrue(TEXT("Bulk edit applies"), Service.ApplyBulk({Key0, Key1}, Bulk, EKyaEditNotify::Silent, &Error));
    TestEqual(TEXT("Bulk edit writes record 0"), Bank->SceneEnvironment.Actors[0].SectorId, 3);
    TestEqual(TEXT("Bulk edit writes record 1"), Bank->SceneEnvironment.Actors[1].SectorId, 3);
    TestTrue(TEXT("Bulk edit changed record 0"), BeforeSector0 != 3);
    TestTrue(TEXT("Bulk edit changed record 1"), BeforeSector1 != 3);
    FKyaActorEditPatch Bulk2;
    Bulk2.SectorId = 5;
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaTests", "BulkEdit", "Bulk Edit"));
        TestTrue(TEXT("Transactional bulk edit applies"),
            Service.ApplyBulk({Key0, Key1}, Bulk2, EKyaEditNotify::Silent, &Error));
    }
    if (GEditor)
    {
        GEditor->UndoTransaction();
        TestEqual(TEXT("Undo restores record 0"), Bank->SceneEnvironment.Actors[0].SectorId, 3);
        TestEqual(TEXT("Undo restores record 1"), Bank->SceneEnvironment.Actors[1].SectorId, 3);
    }

    // A per-record bulk mutator edits one vector axis without overwriting the other
    // components of the other selected records.
    const FVector BeforeAxis0 = Bank->SceneEnvironment.Actors[0].Position;
    const FVector BeforeAxis1 = Bank->SceneEnvironment.Actors[1].Position;
    FKyaActorEditPatch AxisEdit;
    AxisEdit.Custom = [](FKyaActorPlacement& Placement, FString& OutError) -> bool
    {
        FVector Vector;
        if (!GetKyaVectorField(Placement, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Position), Vector))
        {
            OutError = TEXT("Position is not a vector.");
            return false;
        }
        Vector.X = 10.0;
        SetKyaVectorField(Placement, GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Position), Vector);
        return true;
    };
    TestTrue(TEXT("Axis bulk edit applies"),
        Service.ApplyBulk({Key0, Key1}, AxisEdit, EKyaEditNotify::Silent, &Error));
    TestTrue(TEXT("Axis edit keeps record 0 Y/Z"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(10.0, BeforeAxis0.Y, BeforeAxis0.Z)));
    TestTrue(TEXT("Axis edit keeps record 1 Y/Z"),
        Bank->SceneEnvironment.Actors[1].Position.Equals(FVector(10.0, BeforeAxis1.Y, BeforeAxis1.Z)));

    // Property copy validates serialized layout compatibility first.
    const int64 SavedSecondClass = Bank->SceneEnvironment.Actors[1].ClassId;
    Bank->SceneEnvironment.Actors[1].ClassId = Bank->SceneEnvironment.Actors[0].ClassId;
    Bank->SceneEnvironment.Actors[0].Models.Reset();
    Bank->SceneEnvironment.Actors[0].Models.AddDefaulted();
    Bank->SceneEnvironment.Actors[0].Models[0].MeshName = TEXT("MESH");
    Bank->SceneEnvironment.Actors[1].Models.Reset();
    FString CopyError;
    TestFalse(TEXT("Copy rejects different fixed array counts"), CanCopyKyaActorField(
        Bank->SceneEnvironment.Actors[0], Bank->SceneEnvironment.Actors[1],
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Models), CopyError));
    TestTrue(TEXT("Copy count rejection explains the layout"), CopyError.Contains(TEXT("fixed")));
    TestTrue(TEXT("Copy accepts a compatible record"),
        CanCopyKyaActorField(Bank->SceneEnvironment.Actors[0], Bank->SceneEnvironment.Actors[0],
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Models), CopyError));
    Bank->SceneEnvironment.Actors[0].ClassId = 0x0d;
    Bank->SceneEnvironment.Actors[1].ClassId = 0x0f;
    TestFalse(TEXT("Copy rejects a class-specific record across classes"), CanCopyKyaActorField(
        Bank->SceneEnvironment.Actors[0], Bank->SceneEnvironment.Actors[1],
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformWeight), CopyError));
    TestTrue(TEXT("Copy still allows shared settings across classes"), CanCopyKyaActorField(
        Bank->SceneEnvironment.Actors[0], Bank->SceneEnvironment.Actors[1],
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, VisibilityDistance), CopyError));
    Bank->SceneEnvironment.Actors[0].ClassId = 0;
    Bank->SceneEnvironment.Actors[1].ClassId = SavedSecondClass;

    // Behaviour copies require identical ids, nested presence flags and class.
    Bank->SceneEnvironment.Actors[0].Behaviours.Reset();
    Bank->SceneEnvironment.Actors[1].Behaviours.Reset();
    Bank->SceneEnvironment.Actors[0].Behaviours.AddDefaulted();
    Bank->SceneEnvironment.Actors[1].Behaviours.AddDefaulted();
    Bank->SceneEnvironment.Actors[0].Behaviours[0].Id = 3;
    Bank->SceneEnvironment.Actors[1].Behaviours[0].Id = 4;
    Bank->SceneEnvironment.Actors[1].ClassId = Bank->SceneEnvironment.Actors[0].ClassId;
    TestFalse(TEXT("Copy rejects different behaviour ids"), CanCopyKyaActorField(
        Bank->SceneEnvironment.Actors[0], Bank->SceneEnvironment.Actors[1],
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Behaviours), CopyError));
    Bank->SceneEnvironment.Actors[1].Behaviours[0].Id = 3;
    Bank->SceneEnvironment.Actors[0].Behaviours[0].bHasTypedFxBehaviour = true;
    Bank->SceneEnvironment.Actors[1].Behaviours[0].bHasTypedFxBehaviour = false;
    TestFalse(TEXT("Copy rejects different nested presence flags"), CanCopyKyaActorField(
        Bank->SceneEnvironment.Actors[0], Bank->SceneEnvironment.Actors[1],
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Behaviours), CopyError));
    Bank->SceneEnvironment.Actors[1].Behaviours[0].bHasTypedFxBehaviour = true;
    TestTrue(TEXT("Copy accepts identical behaviour records"), CanCopyKyaActorField(
        Bank->SceneEnvironment.Actors[0], Bank->SceneEnvironment.Actors[1],
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Behaviours), CopyError));
    Bank->SceneEnvironment.Actors[1].ClassId = 6;
    TestFalse(TEXT("Copy rejects behaviour records across classes"), CanCopyKyaActorField(
        Bank->SceneEnvironment.Actors[0], Bank->SceneEnvironment.Actors[1],
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Behaviours), CopyError));
    Bank->SceneEnvironment.Actors[1].ClassId = SavedSecondClass;

    // A rejected bulk edit mutates nothing.
    FKyaActorEditPatch BadBulk;
    BadBulk.MeshIndex = 9;
    TestFalse(TEXT("Bulk edit rejects unresolved references"),
        Service.ApplyBulk({Key0, Key1}, BadBulk, EKyaEditNotify::Silent, &Error));
    TestEqual(TEXT("Rejected bulk edit leaves record 0"), Bank->SceneEnvironment.Actors[0].SectorId, 3);
    TestEqual(TEXT("Rejected bulk edit leaves record 1"), Bank->SceneEnvironment.Actors[1].SectorId, 3);
    TestTrue(TEXT("Rejected bulk edit reports the record"), Error.Contains(TEXT("Record")));

    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSpatialRecordsTest, "Kya.Editor.SpatialRecords",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSpatialRecordsTest::RunTest(const FString& Parameters)
{
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());

    FKyaWaypoint& Waypoint = Bank->SceneEnvironment.Waypoints.AddDefaulted_GetRef();
    Waypoint.Position = FVector(1.0, 2.0, 3.0);
    Waypoint.Rotation = FVector(0.1, 0.2, 0.3);
    Waypoint.Parameters = FVector(0.4, 0.5, 0.6);

    FKyaPath& Path = Bank->SceneEnvironment.Paths.AddDefaulted_GetRef();
    Path.Points.Add(FVector4(1.0, 2.0, 3.0, 1.0));
    Path.Points.Add(FVector4(4.0, 5.0, 6.0, 2.0));
    Path.Rotations.Add(FVector4(0.1, 0.2, 0.3, 0.4));
    Path.Rotations.Add(FVector4(0.5, 0.6, 0.7, 0.8));
    Path.Delays.Add(0.25f);
    Path.Delays.Add(0.5f);
    Path.PointFlags.Add(3);
    Path.PointFlags.Add(4);
    Path.ExtraValues.Add(1.5f);
    Path.ExtraValues.Add(2.5f);
    Path.ExtraWords.Add(9);
    Path.ExtraWords.Add(10);
    Path.Mode = 1;
    Path.Type = 2;
    Path.PathType = 3;

    // A path with only the required point array: optional per-point records absent.
    FKyaPath& PointsOnlyPath = Bank->SceneEnvironment.Paths.AddDefaulted_GetRef();
    PointsOnlyPath.Points.Add(FVector4(1.0, 1.0, 1.0, 1.0));

    // --- Service: waypoints -------------------------------------------------------
    const FKyaWaypointRecordKey WaypointKey = Service.MakeWaypointKey(Bank, 0);
    FKyaWaypoint* ResolvedWaypoint = nullptr;
    FString Error;
    TestTrue(TEXT("Waypoint key resolves"), Service.ResolveWaypoint(WaypointKey, ResolvedWaypoint, &Error));
    TestTrue(TEXT("Waypoint key resolves to the record"), ResolvedWaypoint == &Bank->SceneEnvironment.Waypoints[0]);

    FKyaWaypointEditPatch WaypointPatch;
    WaypointPatch.Position = FVector(7.0, 8.0, 9.0);
    WaypointPatch.Rotation = FVector(1.1, 1.2, 1.3);
    TestTrue(TEXT("Waypoint patch applies"), Service.ApplyWaypoint(WaypointKey, WaypointPatch, EKyaEditNotify::Silent, &Error));
    TestTrue(TEXT("Waypoint position written"), Bank->SceneEnvironment.Waypoints[0].Position.Equals(FVector(7.0, 8.0, 9.0)));
    TestTrue(TEXT("Waypoint rotation written"), Bank->SceneEnvironment.Waypoints[0].Rotation.Equals(FVector(1.1, 1.2, 1.3)));
    TestTrue(TEXT("Waypoint parameters preserved"),
        Bank->SceneEnvironment.Waypoints[0].Parameters.Equals(FVector(0.4, 0.5, 0.6)));

    FKyaWaypointEditPatch InvalidWaypoint;
    InvalidWaypoint.Position = FVector(NAN, 0.0, 0.0);
    TestFalse(TEXT("NaN waypoint position rejected"),
        Service.ApplyWaypoint(WaypointKey, InvalidWaypoint, EKyaEditNotify::Silent, &Error));
    TestFalse(TEXT("Rejected waypoint patch explains why"), Error.IsEmpty());
    FKyaWaypoint* MissingWaypoint = nullptr;
    TestFalse(TEXT("Out-of-range waypoint rejected"),
        Service.ResolveWaypoint(Service.MakeWaypointKey(Bank, 99), MissingWaypoint, &Error));

    // --- Service: path points -----------------------------------------------------
    const FKyaPathPointRecordKey PointKey = Service.MakePathPointKey(Bank, 0, 1);
    FKyaPath* ResolvedPath = nullptr;
    TestTrue(TEXT("Path point key resolves"), Service.ResolvePathPoint(PointKey, ResolvedPath, &Error));

    FKyaPathPointEditPatch PointPatch;
    PointPatch.Point = FVector4(4.5, 5.5, 6.5, 2.0);
    PointPatch.Rotation = FVector4(1.5, 1.6, 1.7, 1.8);
    PointPatch.Delay = 0.75f;
    PointPatch.Flags = 77;
    PointPatch.ExtraValue = 3.5f;
    PointPatch.ExtraWord = 11;
    TestTrue(TEXT("Path point patch applies"), Service.ApplyPathPoint(PointKey, PointPatch, EKyaEditNotify::Silent, &Error));
    TestTrue(TEXT("Path point position written"),
        Bank->SceneEnvironment.Paths[0].Points[1].Equals(FVector4(4.5, 5.5, 6.5, 2.0)));
    TestTrue(TEXT("Path point rotation written"),
        Bank->SceneEnvironment.Paths[0].Rotations[1].Equals(FVector4(1.5, 1.6, 1.7, 1.8)));
    TestEqual(TEXT("Path point delay written"), Bank->SceneEnvironment.Paths[0].Delays[1], 0.75f);
    TestEqual(TEXT("Path point flags written"), Bank->SceneEnvironment.Paths[0].PointFlags[1], int64(77));
    TestEqual(TEXT("Path point extra value written"), Bank->SceneEnvironment.Paths[0].ExtraValues[1], 3.5f);
    TestEqual(TEXT("Path point extra word written"), Bank->SceneEnvironment.Paths[0].ExtraWords[1], int64(11));
    TestTrue(TEXT("Other path points preserved"),
        Bank->SceneEnvironment.Paths[0].Points[0].Equals(FVector4(1.0, 2.0, 3.0, 1.0)));

    // A point without a parallel rotation record cannot accept a rotation edit.
    Bank->SceneEnvironment.Paths[0].Points.Add(FVector4(7.0, 8.0, 9.0, 1.0));
    FKyaPathPointEditPatch MissingRecord;
    MissingRecord.Rotation = FVector4(0.0, 0.0, 0.0, 0.0);
    TestFalse(TEXT("Missing per-point rotation record rejected"),
        Service.ApplyPathPoint(Service.MakePathPointKey(Bank, 0, 2), MissingRecord, EKyaEditNotify::Silent, &Error));
    TestTrue(TEXT("Missing per-point record explains why"), Error.Contains(TEXT("rotation")));
    FKyaPathPointEditPatch MissingFlags;
    MissingFlags.Flags = 5;
    TestFalse(TEXT("Path with only points rejects a flags edit"),
        Service.ApplyPathPoint(Service.MakePathPointKey(Bank, 1, 0), MissingFlags, EKyaEditNotify::Silent, &Error));
    FKyaPathPointEditPatch PointsOnlyPatch;
    PointsOnlyPatch.Point = FVector4(2.0, 2.0, 2.0, 1.0);
    TestTrue(TEXT("Path with only points accepts a point edit"),
        Service.ApplyPathPoint(Service.MakePathPointKey(Bank, 1, 0), PointsOnlyPatch, EKyaEditNotify::Silent, &Error));
    TestFalse(TEXT("Negative delay rejected"),
        Service.ApplyPathPoint(PointKey, [] { FKyaPathPointEditPatch P; P.Delay = -1.0f; return P; }(),
            EKyaEditNotify::Silent, &Error));

    // Generation guard covers waypoints and path points too.
    Bank->BumpEditGeneration();
    TestFalse(TEXT("Stale waypoint key rejected"), Service.ResolveWaypoint(WaypointKey, ResolvedWaypoint, &Error));
    TestFalse(TEXT("Stale path-point key rejected"), Service.ResolvePathPoint(PointKey, ResolvedPath, &Error));

    // --- Preview handles ----------------------------------------------------------
    Service.FlushPendingNotifications();
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create spatial world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn spatial preview"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->bSpawnActorHandles = false;
    Preview->bSpawnWaypointHandles = true;
    Preview->bSpawnPathPointHandles = true;
    Preview->OnConstruction(FTransform::Identity);

    TArray<UKyaWaypointComponent*> ArrowMarkers;
    Preview->GetComponents<UKyaWaypointComponent>(ArrowMarkers);
    TestEqual(TEXT("Waypoint arrows are suppressed when handles are enabled"), ArrowMarkers.Num(), 0);
    TestEqual(TEXT("One handle per waypoint and path point"), Preview->GetSpatialHandleCount(), 5);

    AKyaSpatialRecordHandle* WaypointHandle = Cast<AKyaSpatialRecordHandle>(Preview->GetWaypointHandle(0));
    AKyaSpatialRecordHandle* PathHandle = Cast<AKyaSpatialRecordHandle>(Preview->GetPathPointHandle(0, 1));
    if (TestNotNull(TEXT("Waypoint handle exists"), WaypointHandle) &&
        TestNotNull(TEXT("Path-point handle exists"), PathHandle))
    {
        TestTrue(TEXT("Waypoint handle label"),
            WaypointHandle->GetActorLabel().StartsWith(TEXT("Waypoint 0 (")));
        TestTrue(TEXT("Path-point handle label"),
            PathHandle->GetActorLabel().StartsWith(TEXT("Path 0 Point 1 (")));
        TestTrue(TEXT("Spatial handles are transient"), WaypointHandle->HasAnyFlags(RF_Transient));

        // Waypoint translation through the gizmo lifecycle.
        const int64 BuildsBeforeWaypointMove = Preview->BuildCount;
        WaypointHandle->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
        WaypointHandle->PostEditMove(true);
        TestTrue(TEXT("Waypoint handle writes the bank"),
            Bank->SceneEnvironment.Waypoints[0].Position.Equals(FVector(8.0, 8.0, 9.0)));
        TestEqual(TEXT("Waypoint edit does not rebuild level geometry"),
            Preview->BuildCount, BuildsBeforeWaypointMove);
        Service.FlushPendingNotifications();
        TestEqual(TEXT("Waypoint edit still does not rebuild after flush"),
            Preview->BuildCount, BuildsBeforeWaypointMove);

        // Waypoint rotation is edited numerically and keeps position untouched.
        WaypointHandle->WaypointRotation = FVector(2.1, 2.2, 2.3);
        FPropertyChangedEvent RotationEvent(FindFProperty<FProperty>(AKyaSpatialRecordHandle::StaticClass(),
            GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, WaypointRotation)));
        WaypointHandle->PostEditChangeProperty(RotationEvent);
        TestTrue(TEXT("Waypoint rotation writes the bank"),
            Bank->SceneEnvironment.Waypoints[0].Rotation.Equals(FVector(2.1, 2.2, 2.3)));
        TestTrue(TEXT("Waypoint position preserved by rotation edit"),
            Bank->SceneEnvironment.Waypoints[0].Position.Equals(FVector(8.0, 8.0, 9.0)));

        // Path point translation preserves the serialized fourth component.
        PathHandle->AddActorWorldOffset(FVector(0.0, 0.0, 100.0));
        PathHandle->PostEditMove(true);
        TestTrue(TEXT("Path-point handle writes x/z and keeps w"),
            Bank->SceneEnvironment.Paths[0].Points[1].Equals(FVector4(4.5, 6.5, 6.5, 2.0)));

        // Flags are edited through the same service path.
        PathHandle->PathFlags = 123;
        FPropertyChangedEvent FlagsEvent(FindFProperty<FProperty>(AKyaSpatialRecordHandle::StaticClass(),
            GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathFlags)));
        PathHandle->PostEditChangeProperty(FlagsEvent);
        TestEqual(TEXT("Path-point flags write the bank"), Bank->SceneEnvironment.Paths[0].PointFlags[1], int64(123));
        Service.FlushPendingNotifications();

        // A points-only path has no optional records: ApplyToBank must still succeed
        // for the fields that exist instead of rejecting the whole patch.
        AKyaSpatialRecordHandle* PointsOnlyHandle = Cast<AKyaSpatialRecordHandle>(Preview->GetPathPointHandle(1, 0));
        if (TestNotNull(TEXT("Points-only path handle exists"), PointsOnlyHandle))
        {
            TestFalse(TEXT("Points-only path reports no flags record"), PointsOnlyHandle->bHasFlagsRecord);
            TestFalse(TEXT("Points-only path reports no rotation record"), PointsOnlyHandle->bHasRotationRecord);
            PointsOnlyHandle->PathPoint = FVector4(3.0, 4.0, 5.0, 1.0);
            PointsOnlyHandle->ApplyToBank();
            TestTrue(TEXT("Points-only ApplyToBank writes the point"),
                Bank->SceneEnvironment.Paths[1].Points[0].Equals(FVector4(3.0, 4.0, 5.0, 1.0)));
            TestTrue(TEXT("Points-only path gains no optional records"),
                Bank->SceneEnvironment.Paths[1].PointFlags.IsEmpty() &&
                Bank->SceneEnvironment.Paths[1].ExtraValues.IsEmpty());
        }
    }

    // Disabling waypoint handles restores the diagnostic arrows.
    Preview->bSpawnWaypointHandles = false;
    Preview->bSpawnPathPointHandles = false;
    Preview->OnConstruction(FTransform::Identity);
    Preview->GetComponents<UKyaWaypointComponent>(ArrowMarkers);
    TestEqual(TEXT("Waypoint arrows return when handles are disabled"), ArrowMarkers.Num(), 1);
    TestEqual(TEXT("Disabling spatial handles destroys them"), Preview->GetSpatialHandleCount(), 0);

    // --- Component migration: the old markers use the same edit service -------------
    ArrowMarkers[0]->SetRelativeLocation(FVector(100.0, 200.0, 300.0));
    ArrowMarkers[0]->PostEditComponentMove(true);
    TestTrue(TEXT("Waypoint arrow writes through the service"),
        Bank->SceneEnvironment.Waypoints[0].Position.Equals(FVector(1.0, 3.0, 2.0)));
    TestTrue(TEXT("Waypoint arrow preserves rotation"),
        Bank->SceneEnvironment.Waypoints[0].Rotation.Equals(FVector(2.1, 2.2, 2.3)));

    Preview->PreviewPathIndex = 0;
    Preview->OnConstruction(FTransform::Identity);
    if (UKyaPathPointComponent* PathMarker = Preview->GetPathMarker(1))
    {
        PathMarker->SetRelativeLocation(FVector(200.0, 300.0, 400.0));
        PathMarker->PostEditComponentMove(true);
        TestTrue(TEXT("Path-point marker writes through the service"),
            Bank->SceneEnvironment.Paths[0].Points[1].Equals(FVector4(2.0, 4.0, 3.0, 2.0)));
    }
    else
    {
        AddError(TEXT("Expected a path-point marker for the previewed path"));
    }

    Preview->Destroy();
    World->DestroyWorld(false);
    Service.Reset();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaLevelSetTest, "Kya.Editor.LevelSet",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaLevelSetTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* MainBank = MakeKyaTestBank(GetTransientPackage());
    UKyaLevelBank* SectorBank = MakeKyaTestBank(GetTransientPackage(), 1);
    SectorBank->SourceFilename = TEXT("X:/levels/level_1/SECT1.BNK");

    UKyaLevelSetFactory* Factory = NewObject<UKyaLevelSetFactory>();
    UKyaLevelSet* LevelSet = Cast<UKyaLevelSet>(Factory->FactoryCreateNew(UKyaLevelSet::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, GWarn));
    if (!TestNotNull(TEXT("Level set factory creates a descriptor"), LevelSet)) return false;
    TestEqual(TEXT("Descriptor starts at the current version"), LevelSet->DescriptorVersion,
        UKyaLevelSet::CurrentDescriptorVersion);

    LevelSet->MainBank = MainBank;
    LevelSet->SourceRoot = TEXT("X:/levels/level_1");
    LevelSet->SetSector(1, SectorBank, TEXT("X:/levels/level_1/SECT1.BNK"), false);
    TestEqual(TEXT("Sector association added"), LevelSet->Sectors.Num(), 1);
    TestEqual(TEXT("Sector resolves by id"), LevelSet->FindSector(1), SectorBank);
    LevelSet->SetSector(1, SectorBank, TEXT("X:/levels/level_1/SECT1.BNK"), true);
    TestEqual(TEXT("Re-setting a sector does not duplicate it"), LevelSet->Sectors.Num(), 1);
    TestTrue(TEXT("Manual override is recorded"), LevelSet->Sectors[0].bManualOverride);

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create level set world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn preview for level set"), Preview))
    {
        World->DestroyWorld(false);
        return false;
    }
    Preview->LevelSet = LevelSet;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Level set supplies the main bank"), Preview->Bank.Get(), MainBank);
    TestEqual(TEXT("Level set supplies the sector banks"), Preview->SectorBanks.Num(), 1);
    if (Preview->SectorBanks.Num() == 1)
        TestEqual(TEXT("Level set sector bank matches"), Preview->SectorBanks[0].Get(), SectorBank);
    TestTrue(TEXT("Level set preview creates actor handles"), Preview->GetActorHandleCount() > 0);

    // The descriptor owns the sector ID mapping: a manual association that does not
    // match the bank's SECT<number> filename must still resolve to the stored ID.
    UKyaLevelBank* MislabelledSector = MakeKyaTestBank(GetTransientPackage(), 1);
    MislabelledSector->SourceFilename = TEXT("X:/levels/level_1/SECT3.BNK");
    LevelSet->SetSector(7, MislabelledSector, MislabelledSector->SourceFilename, true);
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Descriptor sector ID overrides the filename"), Preview->ResolveSectorId(*MislabelledSector), 7);
    TestEqual(TEXT("Unmapped sector banks still use the filename"), Preview->ResolveSectorId(*SectorBank), 1);

    // Discovery refresh keeps manual associations authoritative and never duplicates
    // the same bank under its filename-derived sector ID.
    Preview->SectorBanks.AddUnique(MislabelledSector);
    KyaSyncLevelSetFromPreview(*LevelSet, *Preview, TEXT("X:/levels/level_1"));
    TestEqual(TEXT("Discovery refresh keeps manual associations"), LevelSet->FindSector(7), MislabelledSector);
    TestNull(TEXT("Discovery refresh does not duplicate a manual bank under its filename ID"),
        LevelSet->FindSector(3));
    TestEqual(TEXT("Discovery refresh keeps discovered associations"), LevelSet->FindSector(1), SectorBank);
    TestEqual(TEXT("Discovery refresh bumps the revision"), LevelSet->ImportRevision, 1);

    // Changing the descriptor's main bank takes effect on the next build, which is how
    // a reopened map restores its associations.
    UKyaLevelBank* OtherMain = MakeKyaTestBank(GetTransientPackage(), 1);
    LevelSet->MainBank = OtherMain;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Rebuilding follows the descriptor's main bank"), Preview->Bank.Get(), OtherMain);

    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaLevelPreviewPlacementTest, "Kya.Editor.LevelPreviewPlacement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaLevelPreviewPlacementTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = MakeKyaTestBank(GetTransientPackage());
    UKyaLevelPreviewFactory* Factory = NewObject<UKyaLevelPreviewFactory>();
    if (!TestNotNull(TEXT("Create level preview factory"), Factory)) return false;

    FText Error;
    TestTrue(TEXT("Factory accepts a Kya bank asset"), Factory->CanCreateActorFrom(FAssetData(Bank), Error));
    TestFalse(TEXT("Factory rejects an empty asset"), Factory->CanCreateActorFrom(FAssetData(), Error));
    TestTrue(TEXT("Factory has an error message"), !Error.IsEmpty());

    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create placement world"), World)) return false;
    AActor* Spawned = Factory->SpawnActor(Bank, World->PersistentLevel, FTransform::Identity, FActorSpawnParameters());
    AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(Spawned);
    if (TestNotNull(TEXT("Factory spawns a level preview actor"), Preview))
    {
        TestEqual(TEXT("Placed preview uses the dragged bank"), Preview->Bank.Get(), Bank);
        TestTrue(TEXT("Placed preview builds its scene"), Preview->BuildCount > 0);
        TestTrue(TEXT("Placed preview creates actor handles"), Preview->GetActorHandleCount() > 0);
        Preview->Destroy();
    }
    World->DestroyWorld(false);
    return true;
}
#endif

