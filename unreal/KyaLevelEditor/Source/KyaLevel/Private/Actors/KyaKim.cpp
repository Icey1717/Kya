#include "Actors/KyaKim.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadKimOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaKimOffsets& Out)
{
    Out = FKyaKimOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 20) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    int64 Pos = 0;
    if (!KyaReadFighterPrefixEnd(Data, Start, Size, Pos)) return false;
    if (!KyaReadFighterPrefixOffsets(Data, Start, Size, Out.Fighter)) return false;
    Out.FighterEnd = Pos;
    if (End - Pos < 40) return false;
    Out.Bones = Pos;
    Pos += 40;
    if (End - Pos < 4) return false;
    Out.ClimbCount = Pos;
    const uint32 Climb = U32(Pos);
    Out.ClimbCountValue = int32(Climb);
    Pos += 4;
    if (uint64(Climb) > uint64(End - Pos) / 4) return false;
    Out.ClimbEntries = Pos;
    Pos += int64(Climb) * 4;
    if (End - Pos < 4) return false;
    Out.TutorialCount = Pos;
    const uint32 Tutorials = U32(Pos);
    Out.TutorialCountValue = int32(Tutorials);
    Pos += 4;
    Out.TutorialEntries = Pos;
    for (uint32 Index = 0; Index < Tutorials; ++Index)
    {
        if (End - Pos < 24) return false;
        FKyaHeroTutorialOffsets& Tutorial = Out.Tutorials.AddDefaulted_GetRef();
        Tutorial.Header = Pos;
        Pos += 24;
        if (End - Pos < 4) return false;
        Tutorial.TargetCount = Pos;
        const uint32 Targets = U32(Pos);
        Tutorial.TargetCountValue = int32(Targets);
        Pos += 4;
        if (uint64(Targets) > uint64(End - Pos) / 28) return false;
        Tutorial.Targets = Pos;
        Pos += int64(Targets) * 28;
        if (End - Pos < 32) return false;
        Tutorial.Camera = Pos;
        Pos += 32;
        Tutorial.Tail = Pos;
    }
    if (End - Pos < 4) return false;
    Out.ProjectileCount = Pos;
    const uint32 Projectiles = U32(Pos);
    Out.ProjectileCountValue = int32(Projectiles);
    Pos += 4;
    if (uint64(Projectiles) > uint64(End - Pos) / 28) return false;
    Out.ProjectileEntries = Pos;
    Pos += int64(Projectiles) * 28;
    if (End - Pos < 4) return false;
    Out.PairCount = Pos;
    const uint32 Pairs = U32(Pos);
    Out.PairCountValue = int32(Pairs);
    Pos += 4;
    if (uint64(Pairs) > uint64(End - Pos) / 8) return false;
    Out.PairEntries = Pos;
    Pos += int64(Pairs) * 8;
    if (End - Pos < 4) return false;
    Out.ZoneStreamCount = Pos;
    const uint32 ZoneStream = U32(Pos);
    Out.ZoneStreamCountValue = int32(ZoneStream);
    Pos += 4;
    if (uint64(ZoneStream) > uint64(End - Pos) / 4) return false;
    Out.ZoneStreamEntries = Pos;
    Pos += int64(ZoneStream) * 4;
    if (End - Pos < 12) return false;
    Out.Material = Pos;
    Out.FxType = Pos + 4;
    Out.JokeRadius = Pos + 8;
    Pos += 12;
    if (End - Pos < 4) return false;
    Out.CameraCount = Pos;
    const uint32 Cameras = U32(Pos);
    Out.CameraCountValue = int32(Cameras);
    Pos += 4;
    const int32 ExpectedCameras = Cameras == 0 ? 0 : (Cameras < 2 ? 2 : int32(Cameras));
    for (int32 Index = 0; Index < ExpectedCameras; ++Index)
    {
        FKyaActorCameraOffsets Camera;
        if (!KyaReadActorCameraOffsets(Data, Pos, End, Camera)) return false;
        Out.Cameras.Add(Camera);
    }
    if (End - Pos < 20) return false;
    Out.Field15D4 = Pos;
    Out.FxInit = Pos + 4;
    Out.FxHandles = Pos + 8;
    Pos += 20;
    Out.Tail = Pos;
    return true;
}

bool KyaReadKimActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaKimOffsets Off;
    if (!KyaReadKimOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.KimBoneKeys.Reset();
    for (int32 Index = 0; Index < 10; ++Index)
        Out.KimBoneKeys.Add(int32(KyaReadU32(Bytes + Off.Bones + Index * 4)));
    Out.KimClimbZones.Reset();
    for (int32 Index = 0; Index < Off.ClimbCountValue; ++Index)
        Out.KimClimbZones.Add(int32(KyaReadU32(Bytes + Off.ClimbEntries + Index * 4)));
    Out.KimTutorialZones.Reset();
    for (int32 Index = 0; Index < Off.TutorialCountValue; ++Index)
    {
        const FKyaHeroTutorialOffsets& T = Off.Tutorials[Index];
        FKyaHeroTutorialZone& Zone = Out.KimTutorialZones.AddDefaulted_GetRef();
        Zone.ZoneRef = int32(KyaReadU32(Bytes + T.Header));
        Zone.MessageKey = int64(uint64(KyaReadU32(Bytes + T.Header + 4)) |
            (uint64(KyaReadU32(Bytes + T.Header + 8)) << 32));
        Zone.Flags = int32(KyaReadU32(Bytes + T.Header + 12));
        Zone.Action = int32(KyaReadU32(Bytes + T.Header + 16));
        Zone.Duration = KyaReadF32(Bytes + T.Header + 20);
        for (int32 Target = 0; Target < T.TargetCountValue; ++Target)
        {
            FKyaSwitchTarget& Item = Zone.Targets.AddDefaulted_GetRef();
            const int64 S = T.Targets + int64(Target) * 28;
            Item.TargetRef = int32(KyaReadU32(Bytes + S));
            Item.CutsceneId = int32(KyaReadU32(Bytes + S + 4));
            Item.Flags = int32(KyaReadU32(Bytes + S + 8));
            Item.MessageId = int32(KyaReadU32(Bytes + S + 12));
            Item.MessageFlags = int32(KyaReadU32(Bytes + S + 16));
            Item.Field14 = int32(KyaReadU32(Bytes + S + 20));
            Item.Field18 = int32(KyaReadU32(Bytes + S + 24));
        }
        const int64 C = T.Camera;
        Zone.Camera.CameraIndex = int32(KyaReadU32(Bytes + C));
        Zone.Camera.Flags = int32(KyaReadU32(Bytes + C + 4));
        Zone.Camera.Field8 = KyaReadF32(Bytes + C + 8);
        Zone.Camera.FieldC = KyaReadF32(Bytes + C + 12);
        Zone.Camera.Field10 = KyaReadF32(Bytes + C + 16);
        Zone.Camera.ActorRef = int32(KyaReadU32(Bytes + C + 20));
        Zone.Camera.ZoneRef = int32(KyaReadU32(Bytes + C + 24));
        Zone.Camera.Field1C = KyaReadF32(Bytes + C + 28);
    }
    Out.KimProjectileZones.Reset();
    for (int32 Index = 0; Index < Off.ProjectileCountValue; ++Index)
    {
        FKyaHeroProjectileZone& Zone = Out.KimProjectileZones.AddDefaulted_GetRef();
        const int64 P = Off.ProjectileEntries + int64(Index) * 28;
        Zone.ZoneRef = int32(KyaReadU32(Bytes + P));
        Zone.ProjectileType = int32(KyaReadU32(Bytes + P + 4));
        Zone.Damage = KyaReadF32(Bytes + P + 8);
        Zone.WaypointIndex = int32(KyaReadU32(Bytes + P + 12));
        Zone.Force = KyaReadF32(Bytes + P + 16);
        Zone.SampleCount = int32(KyaReadU32(Bytes + P + 20));
        Zone.SectorId = int32(KyaReadU32(Bytes + P + 24));
    }
    Out.KimBoomyZonePairs.Reset();
    for (int32 Index = 0; Index < Off.PairCountValue; ++Index)
    {
        FKyaHeroZonePair& Pair = Out.KimBoomyZonePairs.AddDefaulted_GetRef();
        Pair.ZoneRef = int32(KyaReadU32(Bytes + Off.PairEntries + Index * 8));
        Pair.Field4 = int32(KyaReadU32(Bytes + Off.PairEntries + Index * 8 + 4));
    }
    Out.KimZoneStream.Reset();
    for (int32 Index = 0; Index < Off.ZoneStreamCountValue; ++Index)
        Out.KimZoneStream.Add(int32(KyaReadU32(Bytes + Off.ZoneStreamEntries + Index * 4)));
    Out.KimMaterialId = int32(KyaReadU32(Bytes + Off.Material));
    Out.KimFxType = int32(KyaReadU32(Bytes + Off.FxType));
    Out.KimJokeWarnRadius = KyaReadF32(Bytes + Off.JokeRadius);
    Out.KimCameras.Reset();
    for (const FKyaActorCameraOffsets& Camera : Off.Cameras)
        Out.KimCameras.Add(KyaReadActorCameraValues(Data, Camera));
    Out.KimField15D4 = int32(KyaReadU32(Bytes + Off.Field15D4));
    Out.KimFxInitType = int32(KyaReadU32(Bytes + Off.FxInit));
    Out.KimFxHandleTypes.Reset();
    for (int32 Index = 0; Index < 3; ++Index)
        Out.KimFxHandleTypes.Add(int32(KyaReadU32(Bytes + Off.FxHandles + Index * 4)));

    Out.bHasKimData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteKimActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaKimOffsets Off;
    if (!KyaReadKimOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.KimBoneKeys.Num() != 10 ||
        Actor.KimClimbZones.Num() != Off.ClimbCountValue ||
        Actor.KimTutorialZones.Num() != Off.TutorialCountValue ||
        Actor.KimProjectileZones.Num() != Off.ProjectileCountValue ||
        Actor.KimBoomyZonePairs.Num() != Off.PairCountValue ||
        Actor.KimZoneStream.Num() != Off.ZoneStreamCountValue ||
        Actor.KimCameras.Num() != Off.Cameras.Num() ||
        Actor.KimFxHandleTypes.Num() != 3) return false;
    for (int32 Index = 0; Index < Actor.KimTutorialZones.Num(); ++Index)
        if (Actor.KimTutorialZones[Index].Targets.Num() != Off.Tutorials[Index].TargetCountValue)
            return false;

    for (int32 Index = 0; Index < Actor.KimBoneKeys.Num(); ++Index)
        KyaWriteU32(Data, Off.Bones + Index * 4, uint32(Actor.KimBoneKeys[Index]));
    for (int32 Index = 0; Index < Actor.KimClimbZones.Num(); ++Index)
        KyaWriteU32(Data, Off.ClimbEntries + Index * 4, uint32(Actor.KimClimbZones[Index]));
    for (int32 Index = 0; Index < Actor.KimTutorialZones.Num(); ++Index)
    {
        const FKyaHeroTutorialZone& Zone = Actor.KimTutorialZones[Index];
        const int64 H = Off.Tutorials[Index].Header;
        KyaWriteU32(Data, H, uint32(Zone.ZoneRef));
        KyaWriteU32(Data, H + 4, uint32(uint64(Zone.MessageKey)));
        KyaWriteU32(Data, H + 8, uint32(uint64(Zone.MessageKey) >> 32));
        KyaWriteU32(Data, H + 12, uint32(Zone.Flags));
        KyaWriteU32(Data, H + 16, uint32(Zone.Action));
        KyaWriteF32(Data, H + 20, Zone.Duration);
        for (int32 Target = 0; Target < Zone.Targets.Num(); ++Target)
        {
            const FKyaSwitchTarget& Item = Zone.Targets[Target];
            const int64 S = Off.Tutorials[Index].Targets + int64(Target) * 28;
            KyaWriteU32(Data, S, uint32(Item.TargetRef));
            KyaWriteU32(Data, S + 4, uint32(Item.CutsceneId));
            KyaWriteU32(Data, S + 8, uint32(Item.Flags));
            KyaWriteU32(Data, S + 12, uint32(Item.MessageId));
            KyaWriteU32(Data, S + 16, uint32(Item.MessageFlags));
            KyaWriteU32(Data, S + 20, uint32(Item.Field14));
            KyaWriteU32(Data, S + 24, uint32(Item.Field18));
        }
        const int64 C = Off.Tutorials[Index].Camera;
        KyaWriteU32(Data, C, uint32(Zone.Camera.CameraIndex));
        KyaWriteU32(Data, C + 4, uint32(Zone.Camera.Flags));
        KyaWriteF32(Data, C + 8, Zone.Camera.Field8);
        KyaWriteF32(Data, C + 12, Zone.Camera.FieldC);
        KyaWriteF32(Data, C + 16, Zone.Camera.Field10);
        KyaWriteU32(Data, C + 20, uint32(Zone.Camera.ActorRef));
        KyaWriteU32(Data, C + 24, uint32(Zone.Camera.ZoneRef));
        KyaWriteF32(Data, C + 28, Zone.Camera.Field1C);
    }
    for (int32 Index = 0; Index < Actor.KimProjectileZones.Num(); ++Index)
    {
        const FKyaHeroProjectileZone& Zone = Actor.KimProjectileZones[Index];
        const int64 P = Off.ProjectileEntries + int64(Index) * 28;
        KyaWriteU32(Data, P, uint32(Zone.ZoneRef));
        KyaWriteU32(Data, P + 4, uint32(Zone.ProjectileType));
        KyaWriteF32(Data, P + 8, Zone.Damage);
        KyaWriteU32(Data, P + 12, uint32(Zone.WaypointIndex));
        KyaWriteF32(Data, P + 16, Zone.Force);
        KyaWriteU32(Data, P + 20, uint32(Zone.SampleCount));
        KyaWriteU32(Data, P + 24, uint32(Zone.SectorId));
    }
    for (int32 Index = 0; Index < Actor.KimBoomyZonePairs.Num(); ++Index)
    {
        KyaWriteU32(Data, Off.PairEntries + Index * 8, uint32(Actor.KimBoomyZonePairs[Index].ZoneRef));
        KyaWriteU32(Data, Off.PairEntries + Index * 8 + 4, uint32(Actor.KimBoomyZonePairs[Index].Field4));
    }
    for (int32 Index = 0; Index < Actor.KimZoneStream.Num(); ++Index)
        KyaWriteU32(Data, Off.ZoneStreamEntries + Index * 4, uint32(Actor.KimZoneStream[Index]));
    KyaWriteU32(Data, Off.Material, uint32(Actor.KimMaterialId));
    KyaWriteU32(Data, Off.FxType, uint32(Actor.KimFxType));
    KyaWriteF32(Data, Off.JokeRadius, Actor.KimJokeWarnRadius);
    for (int32 Index = 0; Index < Actor.KimCameras.Num(); ++Index)
    {
        if (!KyaWriteActorCameraValues(Actor.KimCameras[Index], Off.Cameras[Index], Data))
            return false;
    }
    KyaWriteU32(Data, Off.Field15D4, uint32(Actor.KimField15D4));
    KyaWriteU32(Data, Off.FxInit, uint32(Actor.KimFxInitType));
    for (int32 Index = 0; Index < Actor.KimFxHandleTypes.Num(); ++Index)
        KyaWriteU32(Data, Off.FxHandles + Index * 4, uint32(Actor.KimFxHandleTypes[Index]));
    return true;
}
