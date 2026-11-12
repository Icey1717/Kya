#if WITH_DEV_AUTOMATION_TESTS
#pragma warning(disable : 4883)
#include "KyaActorEditService.h"
#include "KyaActorCatalog.h"
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
