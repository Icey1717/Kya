#if WITH_DEV_AUTOMATION_TESTS
#include "KyaG2D.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaTextureScrollTest, "Kya.TextureAnimation.Scroll",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaTextureScrollTest::RunTest(const FString& Parameters)
{
    // Minimal serialized MAT -> LAY -> TEX -> animation vector, with unaligned data.
    TArray<uint8> Data;
    Data.SetNumZeroed(208);
    auto Write = [&Data](int32 Offset, uint32 Value) {
        for (int32 Byte = 0; Byte < 4; ++Byte) Data[Offset + Byte] = uint8(Value >> (Byte * 8));
    };
    auto Float = [&Write](int32 Offset, float Value) {
        uint32 Bits; FMemory::Memcpy(&Bits, &Value, 4); Write(Offset, Bits);
    };
    Write(0, 0x2e54414d); Data[16] = 1; Write(32, 48);
    Write(48, 0x2e59414c); Write(68, 0x10); Write(92, 1); Write(96, 112);
    Write(112, 0x2e584554); Write(148, 177);
    Float(177, 0.25f); Float(181, -0.5f); Float(185, 0.875f); Float(189, -0.75f);
    KyaG2D::FTextureScroll Scroll;
    TestTrue(TEXT("Read scrolling first layer"), KyaG2D::ReadMaterialScroll(Data, 0, Scroll));
    TestEqual(TEXT("Stored U step"), Scroll.Step.X, 0.25f);
    TestEqual(TEXT("Stored initial U"), Scroll.Offset.X, 0.875f);
    TestEqual(TEXT("Stored initial V"), Scroll.Offset.Y, -0.75f);
    Scroll.Advance();
    TestEqual(TEXT("Positive U wraps"), Scroll.Offset.X, 0.125f);
    TestEqual(TEXT("Negative V wraps"), Scroll.Offset.Y, -0.25f);
    Scroll.Step = FVector2f(1, -1); Scroll.Offset = FVector2f::ZeroVector;
    Scroll.Advance();
    TestEqual(TEXT("Exact positive boundary"), Scroll.Offset.X, 0.0f);
    TestEqual(TEXT("Exact negative boundary"), Scroll.Offset.Y, 0.0f);
    Scroll.Step = FVector2f(3, -3); Scroll.Advance();
    TestEqual(TEXT("Source adjusts large U only once"), Scroll.Offset.X, 2.0f);
    TestEqual(TEXT("Source adjusts large V only once"), Scroll.Offset.Y, -2.0f);
    Scroll.Step = FVector2f::ZeroVector; Scroll.Offset = FVector2f(0.5f, -0.5f); Scroll.Advance();
    TestEqual(TEXT("Zero speed retains phase"), Scroll.Offset.X, 0.5f);
    Write(68, 2);
    TestFalse(TEXT("Atlas-only is not scrolling"), KyaG2D::ReadMaterialScroll(Data, 0, Scroll));
    Write(68, 0);
    TestFalse(TEXT("Vector does not enable static layer"), KyaG2D::ReadMaterialScroll(Data, 0, Scroll));
    Write(68, 0x12);
    TestTrue(TEXT("Scrolling takes precedence over atlas flag as in src"), KyaG2D::ReadMaterialScroll(Data, 0, Scroll));
    Write(148, 0);
    TestFalse(TEXT("Null vector"), KyaG2D::ReadMaterialScroll(Data, 0, Scroll));
    Write(148, 200);
    TestFalse(TEXT("Truncated vector"), KyaG2D::ReadMaterialScroll(Data, 0, Scroll));
    Write(148, 0xfffffff0);
    TestFalse(TEXT("Overflowing reference"), KyaG2D::ReadMaterialScroll(Data, 0, Scroll));
    Write(148, 177); Write(177, 0x7fc00000);
    TestFalse(TEXT("NaN speed"), KyaG2D::ReadMaterialScroll(Data, 0, Scroll));
    Float(177, 0); Write(189, 0x7f800000);
    TestFalse(TEXT("Infinite offset"), KyaG2D::ReadMaterialScroll(Data, 0, Scroll));
    TestFalse(TEXT("Negative material offset"), KyaG2D::ReadMaterialScroll(Data, -1, Scroll));
    TestFalse(TEXT("Truncated material"), KyaG2D::ReadMaterialScroll(MakeArrayView(Data.GetData(), 12), 0, Scroll));
    return true;
}
#endif
