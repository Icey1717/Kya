#if WITH_DEV_AUTOMATION_TESTS
#include "KyaG2D.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMaterialContainerTest, "Kya.Texture.MaterialContainers",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMaterialContainerTest::RunTest(const FString& Parameters)
{
    auto Fixture = [](int32 PrefixSize) {
        TArray<uint8> Data;
        Data.SetNumZeroed(112 + PrefixSize);
        auto Write = [&Data](int32 Offset, uint32 Value) { FMemory::Memcpy(Data.GetData() + Offset, &Value, 4); };
        Write(12, 0x4432472e); // .G2D
        if (PrefixSize)
        {
            Write(16, 0x414d4e41); // ANMA
            Write(24, PrefixSize);
            // A nested lookalike must not be treated as a top-level container.
            Write(32, 0x2a44322a);
        }
        const int32 Root = 16 + PrefixSize;
        Write(Root, 0x2a44322a); Write(Root + 8, 96); // *2D*
        Write(Root + 16, 0x4154414d); Write(Root + 24, 80); // MATA
        Write(Root + 32, 0x48534148); Write(Root + 40, 32); // HASH
        Write(Root + 48, 0x12345678); Write(Root + 56, Root + 64);
        Write(Root + 64, 0x2e54414d); Write(Root + 72, 32); // MAT.
        return Data;
    };
    TMap<uint64, int32> Materials;
    for (int32 Prefix : {0, 48})
    {
        const TArray<uint8> Data = Fixture(Prefix);
        TestTrue(TEXT("Find materials with optional animation prefix"), KyaG2D::ReadMaterials(Data, Materials));
        TestEqual(TEXT("Material count"), Materials.Num(), 1);
        const int32* Material = Materials.Find(0x12345678);
        if (TestNotNull(TEXT("Preserve hash"), Material)) TestEqual(TEXT("File-relative reference"), *Material, 80 + Prefix);
    }
    TArray<uint8> Data = Fixture(48);
    auto Write = [&Data](int32 Offset, uint32 Value) { FMemory::Memcpy(Data.GetData() + Offset, &Value, 4); };
    Write(24, 0);
    TestFalse(TEXT("Reject zero-size prefix"), KyaG2D::ReadMaterials(Data, Materials));
    TestTrue(TEXT("Failure clears previous materials"), Materials.IsEmpty());
    Write(24, 0xffffffff);
    TestFalse(TEXT("Reject overflowing prefix"), KyaG2D::ReadMaterials(Data, Materials));
    Data = Fixture(48); Data.SetNum(64);
    TestFalse(TEXT("Reject missing material container"), KyaG2D::ReadMaterials(Data, Materials));
    Data = Fixture(48); Write(72, 0xffffffff);
    TestFalse(TEXT("Reject overflowing container"), KyaG2D::ReadMaterials(Data, Materials));
    return true;
}
#endif
