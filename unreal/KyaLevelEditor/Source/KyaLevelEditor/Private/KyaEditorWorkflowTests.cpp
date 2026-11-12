#if WITH_DEV_AUTOMATION_TESTS
#pragma warning(disable : 4883)
#include "KyaActorEditService.h"
#include "KyaActorHandle.h"
#include "KyaActorTransform.h"
#include "KyaBankExport.h"
#include "KyaLevelActorFactory.h"
#include "KyaLevelBank.h"
#include "KyaLevelBankFactory.h"
#include "KyaLevelPreview.h"
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
