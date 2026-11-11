#if WITH_DEV_AUTOMATION_TESTS
#include "KyaG3DChunks.h"
#include "KyaG2D.h"
#include "KyaSceneRenderSettings.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBackgroundRenderingTest, "Kya.Rendering.BackgroundOmissions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBackgroundRenderingTest::RunTest(const FString& Parameters)
{
    TestFalse(TEXT("Reject cluster object wholly beyond native clip"),
        KyaScene::IsClusterStripVisible(FVector4f(0, 0, 100, 5), FVector4f(0, 0, 100, 1), 1, 1, 75));
    TestTrue(TEXT("Far-only parent intersection preserves beyond-far strips"),
        KyaScene::IsClusterStripVisible(FVector4f(0, 0, 74, 5), FVector4f(0, 0, 78, 1), 1, 1, 75));
    TestFalse(TEXT("Side-intersecting parent still tests each strip"),
        KyaScene::IsClusterStripVisible(FVector4f(49, 0, 50, 5), FVector4f(60, 0, 50, 1), 1, 1, 75));
    TestTrue(TEXT("Visible strip survives intersecting parent"),
        KyaScene::IsClusterStripVisible(FVector4f(49, 0, 50, 5), FVector4f(47, 0, 50, 1), 1, 1, 75));
    const FString BankPath = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (BankPath.IsEmpty()) { AddError(TEXT("Set KYA_TEST_BANK to LEVEL_1/LEVEL.BNK to validate the real background assets")); return false; }
    auto Entry = [this, &BankPath](const TCHAR* Name, uint32 Type, TArray<uint8>& Out) {
        TArray<uint8> Bank;
        if (!FFileHelper::LoadFileToArray(Bank, *(FPaths::GetPath(BankPath) / Name)) || Bank.Num() < 64) return false;
        auto U32 = [&Bank](int32 P) { uint32 V; FMemory::Memcpy(&V, Bank.GetData() + P, 4); return V; };
        const uint64 Count = U32(0x28), Table = uint64(U32(0x30)) + 8, Types = uint64(U32(0x34)) + 8;
        if (Table > uint64(Bank.Num()) || Count * 16 > uint64(Bank.Num()) - Table ||
            Types > uint64(Bank.Num()) || Count * 4 > uint64(Bank.Num()) - Types) return false;
        for (uint32 I = 0; I < Count; ++I)
        {
            if ((U32(0x14) ? U32(0x14) : U32(int32(Types + I * 4))) != Type) continue;
            const uint64 Offset = U32(int32(Table + I * 16)), Size = U32(int32(Table + I * 16 + 4));
            if (Offset > uint64(Bank.Num()) || Size > uint64(Bank.Num()) - Offset) return false;
            Out.Append(Bank.GetData() + Offset, int32(Size));
            return true;
        }
        return false;
    };
    TArray<uint8> Scene;
    TestTrue(TEXT("Read scene entry"), Entry(TEXT("LEVEL.BNK"), 0x60001, Scene));
    TMap<int32, FKyaSectorRenderSettings> Settings;
    if (TestTrue(TEXT("Read native sector render settings"), KyaScene::ReadSectorRenderSettings(Scene, Settings)))
    {
        TestEqual(TEXT("Sector record count"), Settings.Num(), 12);
        const auto* Sector1 = Settings.Find(1);
        if (TestNotNull(TEXT("Sector one settings"), Sector1))
        {
            TestEqual(TEXT("Sector one background is fixed despite common flag"), Sector1->BackgroundFlags, uint32(0));
            TestEqual(TEXT("Sector one native draw distance"), Sector1->ClipDistance, 75.0f);
        }
        if (const auto* Sector = Settings.Find(7)) TestEqual(TEXT("Sector seven follow flag"), Sector->BackgroundFlags, uint32(1));
        Scene.Pop();
        TestFalse(TEXT("Reject truncated sector records"), KyaScene::ReadSectorRenderSettings(Scene, Settings));
        TestTrue(TEXT("Invalid sector data publishes no partial settings"), Settings.IsEmpty());
    }
    TArray<uint8> Mesh, Texture, Sector7;
    if (!TestTrue(TEXT("SECT1 background mesh"), Entry(TEXT("SECT1.BNK"), 0x40002, Mesh)) ||
        !TestTrue(TEXT("SECT1 background texture"), Entry(TEXT("SECT1.BNK"), 0x50003, Texture))) return false;
    TestFalse(TEXT("SECT7 has no competing background"), Entry(TEXT("SECT7.BNK"), 0x40002, Sector7));
    TArray<FKyaG3DBackgroundFlare> Flares;
    if (TestTrue(TEXT("Decode hierarchy sprite without SPR. chunk"), KyaG3D::ReadBackgroundFlares(Mesh, Flares)) &&
        TestEqual(TEXT("One fixed background flare"), Flares.Num(), 1))
    {
        TestEqual(TEXT("Flare material hash"), Flares[0].MaterialHash, uint64(0xda780ed5bb30d8ad));
        FMatrix Transform;
        TestTrue(TEXT("Flare transform"), KyaG3D::ReadHierarchyWorldMatrix(Mesh, Flares[0].HierarchyOffset, Transform));
        const FVector P = Transform.TransformPosition(FVector(Flares[0].Center));
        TestTrue(TEXT("Flare source stays fixed in world"), P.Equals(FVector(445.31146, 201.62788, -162.30597), 0.001));
        TArray<uint8> Bad = Mesh;
        const uint32 Invalid = MAX_uint32;
        FMemory::Memcpy(Bad.GetData() + Flares[0].HierarchyOffset + 0xb0, &Invalid, 4);
        TestFalse(TEXT("Reject out-of-range sprite reference"), KyaG3D::ReadBackgroundFlares(Bad, Flares));
        TestTrue(TEXT("Failed decoder publishes no partial flares"), Flares.IsEmpty());
    }
    TMap<uint64, int32> Materials;
    TestTrue(TEXT("Read background materials"), KyaG2D::ReadMaterials(Texture, Materials));
    TArray<FKyaG3DDecodedStrip> Strips;
    if (!TestTrue(TEXT("Decode background strips"), KyaG3D::DecodeHierarchyStrips(Mesh, Strips))) return false;
    TestEqual(TEXT("Keep all background strips"), Strips.Num(), 264);
    int32 Additive = 0;
    for (const FKyaG3DDecodedStrip& Strip : Strips)
    {
        if (Strip.MaterialHash != 0xfc84df6a8295cad5) continue;
        ++Additive;
        TestTrue(TEXT("Preserve GIF PRIM ABE bit 6"), (Strip.Primitive & 0x40) != 0);
        const int32* Offset = Materials.Find(Strip.MaterialHash);
        if (!TestNotNull(TEXT("Additive material resolves"), Offset)) continue;
        uint64 Alpha = 0;
        TestTrue(TEXT("Read actual GS ALPHA register 0x42"), KyaG2D::ReadMaterialRegister(Texture, *Offset, 0, 0x42, Alpha));
        TestEqual(TEXT("Source-alpha additive equation with unused FIX=30"), Alpha, uint64(0x1e00000048));
        TestEqual(TEXT("C selector is source alpha, not FIX"), (Alpha >> 4) & 3, uint64(0));
    }
    if (const int32* OpeningShell = Materials.Find(0xfca9cf536c08464e))
    {
        uint64 Clamp = 0;
        TestTrue(TEXT("Read opening background address modes"), KyaG2D::ReadMaterialRegister(Texture, *OpeningShell, 0, 0x08, Clamp));
        TestEqual(TEXT("Opening background clamps both axes"), Clamp & 15, uint64(5));
    }
    TestEqual(TEXT("Three strips use the additive background material"), Additive, 3);
    TArray<uint8> SectorMesh, SectorAnimation;
    TestTrue(TEXT("SECT1 scene mesh"), Entry(TEXT("SECT1.BNK"), 0x40001, SectorMesh));
    TestTrue(TEXT("SECT1 animation hierarchy roots"), Entry(TEXT("SECT1.BNK"), 0x170001, SectorAnimation));
    TArray<int32> SceneHierarchies;
    if (TestTrue(TEXT("Resolve native DYNAMIC and ANHR scene instances"),
        KyaG3D::ReadSectorSceneHierarchies(SectorMesh, SectorAnimation, SceneHierarchies)))
    {
        TestEqual(TEXT("DYNAMIC root, eleven rays and ten animated trees"), SceneHierarchies.Num(), 22);
        TArray<FKyaG3DDecodedStrip> SectorStrips;
        TestTrue(TEXT("Decode sector scene strips"), KyaG3D::DecodeHierarchyStrips(SectorMesh, SectorStrips));
        int32 SceneStrips = 0;
        for (const auto& Strip : SectorStrips) if (SceneHierarchies.Contains(Strip.HierarchyOffset)) ++SceneStrips;
        TestEqual(TEXT("Twenty-one strips are separate from CSTA submission"), SceneStrips, 21);
        TestTrue(TEXT("DYNAMIC root and children without ANHR"), KyaG3D::ReadSectorSceneHierarchies(SectorMesh, {}, SceneHierarchies));
        TestEqual(TEXT("Eleven rays plus dynamic root"), SceneHierarchies.Num(), 12);
        const uint32 Invalid = MAX_uint32;
        FMemory::Memcpy(SectorAnimation.GetData() + 16, &Invalid, 4);
        TestFalse(TEXT("Reject invalid ANHR root pointer"), KyaG3D::ReadSectorSceneHierarchies(SectorMesh, SectorAnimation, SceneHierarchies));
        TestTrue(TEXT("Invalid ANHR publishes no partial instance list"), SceneHierarchies.IsEmpty());
    }
    TArray<uint8> ClusterTextures;
    TestTrue(TEXT("SECT1 cluster textures"), Entry(TEXT("SECT1.BNK"), 0x50001, ClusterTextures));
    TMap<uint64, int32> ClusterMaterials;
    TestTrue(TEXT("Read cluster materials"), KyaG2D::ReadMaterials(ClusterTextures, ClusterMaterials));
    const int32* Opening = ClusterMaterials.Find(0xc13117095d02aa80);
    if (TestNotNull(TEXT("Opening surface material"), Opening))
    {
        uint64 Alpha = 0;
        TestTrue(TEXT("Read opening blend equation"), KyaG2D::ReadMaterialRegister(ClusterTextures, *Opening, 0, 0x42, Alpha));
        TestEqual(TEXT("Opening uses source alpha rather than opaque replacement"), Alpha, uint64(0x44));
    }
    return true;
}
#endif
