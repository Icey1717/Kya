#if WITH_DEV_AUTOMATION_TESTS
#pragma warning(disable : 4883)
#include "KyaActorEditService.h"
#include "KyaActorCatalog.h"
#include "KyaActorGroup.h"
#include "KyaActorHandle.h"
#include "KyaActorTransform.h"
#include "KyaBankExport.h"
#include "KyaLevelActorFactory.h"
#include "KyaLevelBank.h"
#include "KyaLevelBankFactory.h"
#include "KyaLevelEditorActions.h"
#include "KyaLevelPreview.h"
#include "KyaLevelSet.h"
#include "KyaLevelSetFactory.h"
#include "KyaPathPointComponent.h"
#include "KyaSpatialRecordHandle.h"
#include "KyaWaypointComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/SplineComponent.h"
#include "ProceduralMeshComponent.h"
#include "UObject/Package.h"
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
    const FKyaActorRecordKey CurrentKey = Service.MakeKey(Bank, 1);
    FKyaActorEditPatch SecondPatch;
    SecondPatch.SectorId = 3;
    TestTrue(TEXT("Apply patch through the service"), Service.Apply(CurrentKey, SecondPatch, EKyaEditNotify::Notify, &Error));
    TestEqual(TEXT("Preview A refreshed exactly once"), PreviewA->BuildCount, BuildsA + 1);
    TestEqual(TEXT("Preview B refreshed exactly once"), PreviewB->BuildCount, BuildsB + 1);
    TestEqual(TEXT("Sector written"), Bank->SceneEnvironment.Actors[1].SectorId, 3);

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
    TestEqual(TEXT("One rebuild per drag"), Preview->BuildCount, BuildsBeforeMove + 1);
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
    TestEqual(TEXT("Multi-actor drag refreshes once"), Preview->BuildCount, BuildsBeforeMulti + 1);
    TestTrue(TEXT("First actor moved by the multi-drag"),
        Bank->SceneEnvironment.Actors[0].Position.Equals(FVector(3.0, 2.0, 3.0)));

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
        TestEqual(TEXT("Waypoint handle label"), WaypointHandle->GetActorLabel(), FString(TEXT("Waypoint 0")));
        TestEqual(TEXT("Path-point handle label"), PathHandle->GetActorLabel(), FString(TEXT("Path 0 Point 1")));
        TestTrue(TEXT("Spatial handles are transient"), WaypointHandle->HasAnyFlags(RF_Transient));

        // Waypoint translation through the gizmo lifecycle.
        WaypointHandle->AddActorWorldOffset(FVector(100.0, 0.0, 0.0));
        WaypointHandle->PostEditMove(true);
        TestTrue(TEXT("Waypoint handle writes the bank"),
            Bank->SceneEnvironment.Waypoints[0].Position.Equals(FVector(8.0, 8.0, 9.0)));
        Service.FlushPendingNotifications();

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
