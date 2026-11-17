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
    TestFalse(TEXT("Incompatible layout rejected"), Patch.Custom(Dest, Error));
    TestTrue(TEXT("Rejected copy changes no earlier fields"), Dest.Position.Equals(FVector(9, 8, 7)));
    TestEqual(TEXT("Rejected copy preserves array"), Dest.Models.Num(), 1);
    TestFalse(TEXT("Rejection explains incompatibility"), Error.IsEmpty());
    return true;
}
#endif
