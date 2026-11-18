#include "Misc/AutomationTest.h"
#include "KyaActorCatalog.h"
#include "KyaActorEditService.h"
#include "KyaLevelBank.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaActorCopyPropertiesTest, "Kya.Editor.ActorCopyProperties",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaActorCopyPropertiesTest::RunTest(const FString& Parameters)
{
    FKyaActorPlacement Source;
    Source.Name = TEXT("Source");
    Source.Position = FVector(1, 2, 3);
    Source.VisibilityDistance = 42;
    FKyaActorPlacement Dest = Source;
    Dest.Name = TEXT("Destination");
    Dest.Position = FVector(9, 8, 7);
    Dest.VisibilityDistance = 17;
    FKyaActorEditPatch Patch;
    BuildKyaActorCopyPropertiesPatch(Source, Patch);
    // The patch owns its source snapshot.
    Source.Position = FVector::ZeroVector;
    FString Error;
    TestTrue(TEXT("Compatible copy succeeds"), Patch.Custom(Dest, Error));
    TestEqual(TEXT("Destination name preserved"), Dest.Name, FString(TEXT("Destination")));
    TestTrue(TEXT("Transform copied from snapshot"), Dest.Position.Equals(FVector(1, 2, 3)));
    TestEqual(TEXT("Settings copied"), Dest.VisibilityDistance, 42.0f);

    Dest.Position = FVector(9, 8, 7);
    Dest.Models.AddDefaulted();
    Error.Reset();
    TestTrue(TEXT("Resizable array copy succeeds"), Patch.Custom(Dest, Error));
    TestTrue(TEXT("Other fields still copy"), Dest.Position.Equals(FVector(1, 2, 3)));
    TestEqual(TEXT("Resizable array takes the source count"), Dest.Models.Num(), 0);
    TestTrue(TEXT("Array copy is not an error"), Error.IsEmpty());

    // Different actor classes share the common CActor fields but keep their own
    // class-specific Create payloads. The copy must transfer only the common
    // fields that fit the destination layout.
    FKyaActorPlacement Platform;
    Platform.ClassId = 0x0d;
    Platform.Name = TEXT("Platform");
    Platform.Position = FVector(4, 5, 6);
    Platform.bHasPlatformPrefix = true;
    Platform.PlatformWeight = 9.0f;
    Platform.DefaultBehaviourId = 2;

    FKyaActorPlacement Switch;
    Switch.ClassId = 0x11;
    Switch.Name = TEXT("Switch");
    Switch.Position = FVector::ZeroVector;
    Switch.bHasSwitchData = true;
    Switch.DefaultBehaviourId = 0;

    FKyaActorEditPatch AcrossPatch;
    FString AcrossFieldError;
    TestFalse(TEXT("A type-specific field is still rejected across classes"),
        CanCopyKyaActorField(Platform, Switch,
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformWeight), AcrossFieldError));
    TestTrue(TEXT("Cross-class copy patch builds"), BuildKyaActorCopyPropertiesPatch(Platform, AcrossPatch));
    FString AcrossError;
    TestTrue(TEXT("Cross-class copy succeeds"), AcrossPatch.Custom(Switch, AcrossError));
    TestEqual(TEXT("Cross-class copy preserves destination name"), Switch.Name, FString(TEXT("Switch")));
    TestEqual(TEXT("Cross-class copy preserves destination class"), Switch.ClassId, int64(0x11));
    TestTrue(TEXT("Cross-class copy transfers common transform"), Switch.Position.Equals(FVector(4, 5, 6)));
    TestEqual(TEXT("Cross-class copy preserves class-specific data"), Switch.PlatformWeight, 0.0f);
    TestEqual(TEXT("Cross-class copy preserves the behaviour id"), Switch.DefaultBehaviourId, 0);
    TestTrue(TEXT("Cross-class copy preserves destination presence flags"), Switch.bHasSwitchData);

    // A class-specific resizable stream copies between same-class actors.
    FKyaActorPlacement PlatformSource;
    PlatformSource.ClassId = 0x0d;
    PlatformSource.bHasPlatformPrefix = true;
    PlatformSource.PlatformActors.AddDefaulted(2);
    PlatformSource.PlatformActors[0].ActorRef = 1;
    PlatformSource.PlatformActors[1].CarryMethod = 4;
    FKyaActorPlacement PlatformDest;
    PlatformDest.ClassId = 0x0d;
    PlatformDest.bHasPlatformPrefix = true;
    FKyaActorEditPatch PlatformPatch;
    BuildKyaActorCopyPropertiesPatch(PlatformSource, PlatformPatch);
    FString PlatformError;
    TestTrue(TEXT("Class-specific platform stream copy succeeds"),
        PlatformPatch.Custom(PlatformDest, PlatformError));
    TestEqual(TEXT("Class-specific platform stream count copies"), PlatformDest.PlatformActors.Num(), 2);
    TestEqual(TEXT("Class-specific platform stream value copies"),
        PlatformDest.PlatformActors[1].CarryMethod, 4);

    // A common resizable array now follows the source count instead of being
    // skipped, because export rebuilds the destination actor chunk.
    Switch.Position = FVector::ZeroVector;
    Switch.Models.AddDefaulted();
    FKyaActorEditPatch MismatchedPatch;
    BuildKyaActorCopyPropertiesPatch(Platform, MismatchedPatch);
    TestTrue(TEXT("Cross-class copy replaces a resizable common array"),
        MismatchedPatch.Custom(Switch, AcrossError));
    TestTrue(TEXT("Cross-class copy still transfers the transform"), Switch.Position.Equals(FVector(4, 5, 6)));
    TestEqual(TEXT("Cross-class copy takes the source array count"), Switch.Models.Num(), 0);

    // Same class, different actor types (for example GRUNT_02_BASIC and
    // SCRAWNY_01_FIRE_WEAPON_01_L9, both class 9). Resizable arrays follow the
    // source count; unsupported fixed-size class arrays keep the destination layout.
    FKyaActorPlacement Grunt;
    Grunt.ClassId = 0x09;
    Grunt.Name = TEXT("GRUNT_02_BASIC");
    Grunt.Position = FVector(7.0, 8.0, 9.0);
    Grunt.MacroAnimations.AddDefaulted(3);

    FKyaActorPlacement Scrawny;
    Scrawny.ClassId = 0x09;
    Scrawny.Name = TEXT("SCRAWNY_01_FIRE_WEAPON_01_L9");
    Scrawny.Position = FVector::ZeroVector;
    Scrawny.MacroAnimations.AddDefaulted(5);

    FKyaActorEditPatch DifferentTypePatch;
    BuildKyaActorCopyPropertiesPatch(Grunt, DifferentTypePatch);
    FString DifferentTypeError;
    TestTrue(TEXT("Same-class copy with different macro counts succeeds"),
        DifferentTypePatch.Custom(Scrawny, DifferentTypeError));
    TestEqual(TEXT("Resizable macro count takes the source count"), Scrawny.MacroAnimations.Num(), 3);
    TestTrue(TEXT("Same-class common transform is copied"), Scrawny.Position.Equals(FVector(7.0, 8.0, 9.0)));

    // A behaviour id that is absent from the destination's behaviour list
    // is left alone instead of failing validation later.
    Scrawny.Behaviours.AddDefaulted();
    Grunt.DefaultBehaviourId = 4;
    Scrawny.DefaultBehaviourId = 0;
    FKyaActorEditPatch BehaviourPatch;
    BuildKyaActorCopyPropertiesPatch(Grunt, BehaviourPatch);
    FString BehaviourError;
    TestTrue(TEXT("Copy skips an unavailable behaviour id"), BehaviourPatch.Custom(Scrawny, BehaviourError));
    TestEqual(TEXT("Destination behaviour id is preserved"), Scrawny.DefaultBehaviourId, 0);
    TestEqual(TEXT("Resizable behaviour list takes the source count"), Scrawny.Behaviours.Num(), 0);
    return true;
}
#endif
