#include "KyaSceneRenderSettings.h"

bool KyaScene::IsClusterStripVisible(const FVector4f& Object, const FVector4f& Strip,
    double TanX, double TanY, float FarDistance)
{
    auto Classify = [&](const FVector4f& S, bool Far) {
        const double Radius = FMath::Abs(S.W);
        const double XScale = FMath::Sqrt(1 + TanX * TanX), YScale = FMath::Sqrt(1 + TanY * TanY);
        const double Distances[] = { S.Z - 0.001, Far ? FarDistance - S.Z : DBL_MAX,
            (S.Z * TanX + S.X) / XScale, (S.Z * TanX - S.X) / XScale,
            (S.Z * TanY + S.Y) / YScale, (S.Z * TanY - S.Y) / YScale };
        int32 Result = 1;
        for (double Distance : Distances)
        {
            if (Distance <= -Radius) return 0;
            if (Distance < Radius) Result = 2;
        }
        return Result;
    };
    const int32 ObjectResult = Classify(Object, true);
    if (!ObjectResult) return false;
    // ed3DRenderClusterObject deliberately skips strip tests for an object
    // intersecting only the far plane. A hard per-pixel far clip is different.
    if (ObjectResult == 1 || Classify(Object, false) == 1) return true;
    return Classify(Strip, true) != 0;
}

bool KyaScene::ReadSectorRenderSettings(TConstArrayView<uint8> Scene, TMap<int32, FKyaSectorRenderSettings>& Out)
{
    Out.Reset();
    auto U32 = [&Scene](int32 P) { uint32 V; FMemory::Memcpy(&V, Scene.GetData() + P, 4); return V; };
    auto F32 = [&Scene](int32 P) { float V; FMemory::Memcpy(&V, Scene.GetData() + P, 4); return V; };
    // CSBS is the final scene chunk. Validate its extent as well as its tag;
    // matching bytes inside an actor payload are not sector records.
    int32 Start = INDEX_NONE;
    for (int32 P = 0; P <= Scene.Num() - 12; P += 4)
        if (U32(P) == 0x53425343 && U32(P + 4) == uint32(Scene.Num() - P)) { Start = P; break; }
    if (Start == INDEX_NONE) return false;
    const uint32 Count = U32(Start + 8);
    if (Count > 30) return false;
    int64 P = int64(Start) + 12;
    TMap<int32, FKyaSectorRenderSettings> Result;
    for (uint32 I = 0; I < Count; ++I)
    {
        if (P > Scene.Num() - 40) return false;
        const uint32 Id = U32(int32(P + 4));
        if (Id >= 30 || Result.Contains(int32(Id))) return false;
        FKyaSectorRenderSettings Settings;
        Settings.BackgroundFlags = U32(int32(P));
        Settings.ClipDistance = F32(int32(P + 12));
        Settings.AnimatedHierarchyClipDistance = F32(int32(P + 32));
        if (!FMath::IsFinite(Settings.ClipDistance) || Settings.ClipDistance < 0 ||
            !FMath::IsFinite(Settings.AnimatedHierarchyClipDistance) || Settings.AnimatedHierarchyClipDistance < 0) return false;
        Result.Add(int32(Id), Settings);
        P += 36;
        const uint32 Dependencies = U32(int32(P)); P += 4;
        if (Dependencies > uint64(Scene.Num() - P) / 8) return false;
        for (uint32 D = 0; D < Dependencies; ++D)
        {
            if (P > Scene.Num() - 8) return false;
            P += 4; // Referenced sector index.
            const uint32 Conditions = U32(int32(P)); P += 4;
            if (Conditions > uint64(Scene.Num() - P) / 16) return false;
            P += int64(Conditions) * 16;
        }
    }
    if (P != Scene.Num()) return false;
    Out = MoveTemp(Result);
    return true;
}
