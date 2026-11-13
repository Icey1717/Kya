#include "Actors/KyaMovingPlatform.h"

#include "KyaLevelBank.h"

namespace
{
uint32 KyaReadU32(const uint8* P)
{
    return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
}

float KyaReadF32(const uint8* P)
{
    const uint32 Bits = KyaReadU32(P);
    float Value;
    FMemory::Memcpy(&Value, &Bits, sizeof(Value));
    return Value;
}

void KyaWriteU32(TArray<uint8>& Data, int64 Offset, uint32 Value)
{
    for (int32 Byte = 0; Byte < 4; ++Byte) Data[int32(Offset) + Byte] = uint8(Value >> (Byte * 8));
}

void KyaWriteF32(TArray<uint8>& Data, int64 Offset, float Value)
{
    uint32 Bits = 0;
    FMemory::Memcpy(&Bits, &Value, sizeof(Bits));
    KyaWriteU32(Data, Offset, Bits);
}

bool KyaRangeValid(TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 0 && Start <= Data.Num() && Size <= Data.Num() - Start;
}

bool KyaRangeValid(const TArray<uint8>& Data, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 0 && Start <= Data.Num() && Size <= Data.Num() - Start;
}

bool KyaReadSwitchData(TConstArrayView<uint8> Data, int64 Targets, int32 Count, int64 Camera,
    FKyaSwitch& Out)
{
    Out = FKyaSwitch{};
    if (Targets < 0 || Count < 0 || Camera < 0 || Targets > Data.Num() || Camera > Data.Num() ||
        int64(Count) * 28 > Data.Num() - Targets || 32 > Data.Num() - Camera) return false;
    const uint8* Bytes = Data.GetData();
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FKyaSwitchTarget& Target = Out.Targets.AddDefaulted_GetRef();
        const int64 T = Targets + int64(Index) * 28;
        Target.TargetRef = int32(KyaReadU32(Bytes + T));
        Target.CutsceneId = int32(KyaReadU32(Bytes + T + 4));
        Target.Flags = int32(KyaReadU32(Bytes + T + 8));
        Target.MessageId = int32(KyaReadU32(Bytes + T + 12));
        Target.MessageFlags = int32(KyaReadU32(Bytes + T + 16));
        Target.Field14 = int32(KyaReadU32(Bytes + T + 20));
        Target.Field18 = int32(KyaReadU32(Bytes + T + 24));
    }
    Out.Camera.CameraIndex = int32(KyaReadU32(Bytes + Camera));
    Out.Camera.Flags = int32(KyaReadU32(Bytes + Camera + 4));
    Out.Camera.Field8 = KyaReadF32(Bytes + Camera + 8);
    Out.Camera.FieldC = KyaReadF32(Bytes + Camera + 12);
    Out.Camera.Field10 = KyaReadF32(Bytes + Camera + 16);
    Out.Camera.ActorRef = int32(KyaReadU32(Bytes + Camera + 20));
    Out.Camera.ZoneRef = int32(KyaReadU32(Bytes + Camera + 24));
    Out.Camera.Field1C = KyaReadF32(Bytes + Camera + 28);
    return true;
}

void KyaWriteSwitchData(TArray<uint8>& Data, const FKyaSwitch& Switch, int64 Targets, int64 Camera)
{
    for (int32 Index = 0; Index < Switch.Targets.Num(); ++Index)
    {
        const FKyaSwitchTarget& Target = Switch.Targets[Index];
        const int64 T = Targets + int64(Index) * 28;
        KyaWriteU32(Data, T, uint32(Target.TargetRef));
        KyaWriteU32(Data, T + 4, uint32(Target.CutsceneId));
        KyaWriteU32(Data, T + 8, uint32(Target.Flags));
        KyaWriteU32(Data, T + 12, uint32(Target.MessageId));
        KyaWriteU32(Data, T + 16, uint32(Target.MessageFlags));
        KyaWriteU32(Data, T + 20, uint32(Target.Field14));
        KyaWriteU32(Data, T + 24, uint32(Target.Field18));
    }
    const FKyaStreamEventCamera& C = Switch.Camera;
    KyaWriteU32(Data, Camera, uint32(C.CameraIndex));
    KyaWriteU32(Data, Camera + 4, uint32(C.Flags));
    KyaWriteF32(Data, Camera + 8, C.Field8);
    KyaWriteF32(Data, Camera + 12, C.FieldC);
    KyaWriteF32(Data, Camera + 16, C.Field10);
    KyaWriteU32(Data, Camera + 20, uint32(C.ActorRef));
    KyaWriteU32(Data, Camera + 24, uint32(C.ZoneRef));
    KyaWriteF32(Data, Camera + 28, C.Field1C);
}
}

bool KyaReadPlatformPrefix(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaPlatformPrefixOffsets& Out)
{
    Out = FKyaPlatformPrefixOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 16) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    FKyaPlatformPrefixOffsets Result;
    Result.Weight = Start;
    int64 Pos = Start + 4;
    const auto ReadList = [&](int32 Stride, int64& Offset, int32& Count)
    {
        if (End - Pos < 4) return false;
        const uint32 Num = KyaReadU32(Bytes + Pos); Pos += 4;
        if (Num > MAX_int32 || Num > uint64(End - Pos) / Stride) return false;
        Offset = Pos; Count = int32(Num); Pos += int64(Num) * Stride;
        return true;
    };
    if (!ReadList(8, Result.Actors, Result.ActorCount) ||
        !ReadList(4, Result.Zones, Result.ZoneCount) || End - Pos < 4) return false;
    const uint32 ZoneCount = KyaReadU32(Bytes + Pos); Pos += 4;
    if (ZoneCount > uint64(End - Pos) / 8) return false;
    for (uint32 Index = 0; Index < ZoneCount; ++Index)
    {
        if (End - Pos < 8) return false;
        Result.NoFrictionZones.Add(Pos);
        const uint32 ActorCount = KyaReadU32(Bytes + Pos + 4); Pos += 8;
        if (ActorCount > uint64(End - Pos) / 4) return false;
        Pos += int64(ActorCount) * 4;
    }
    Result.Suffix = Pos;
    Out = MoveTemp(Result);
    return true;
}

bool KyaReadPlatformDetails(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaPlatformDetailsOffsets& Out)
{
    Out = FKyaPlatformDetailsOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 88) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    FKyaPlatformDetailsOffsets Result;
    int64 Pos = Start;
    const auto ReadList = [&](int32 Stride, int64& Offset, int32& Count)
    {
        if (End - Pos < 4) return false;
        const uint32 Num = KyaReadU32(Bytes + Pos); Pos += 4;
        if (Num > MAX_int32 || Num > uint64(End - Pos) / Stride) return false;
        Offset = Pos; Count = int32(Num); Pos += int64(Num) * Stride;
        return true;
    };
    if (!ReadList(72, Result.Cameras, Result.CameraCount) || End - Pos < 76) return false;
    Result.Properties = Pos; Pos += 76;
    if (!ReadList(16, Result.Sounds[0], Result.SoundCounts[0]) ||
        !ReadList(16, Result.Sounds[1], Result.SoundCounts[1])) return false;
    Result.Tail = Pos;
    Out = Result;
    return true;
}

bool KyaReadPlatformCamera(TConstArrayView<uint8> Data, int64 Offset, FKyaPlatformCamera& Out)
{
    Out = FKyaPlatformCamera{};
    if (Offset < 0 || !KyaRangeValid(Data, Offset, 72)) return false;
    const uint8* Bytes = Data.GetData();
    Out.EventType = int32(KyaReadU32(Bytes + Offset));
    Out.Flags = int32(KyaReadU32(Bytes + Offset + 4));
    Out.TargetRef = int32(KyaReadU32(Bytes + Offset + 8));
    Out.CutsceneId = int32(KyaReadU32(Bytes + Offset + 12));
    Out.TargetFlags = int32(KyaReadU32(Bytes + Offset + 16));
    Out.MessageId = int32(KyaReadU32(Bytes + Offset + 20));
    Out.MessageFlags = int32(KyaReadU32(Bytes + Offset + 24));
    Out.TargetField14 = int32(KyaReadU32(Bytes + Offset + 28));
    Out.TargetField18 = int32(KyaReadU32(Bytes + Offset + 32));
    Out.CameraIndex = int32(KyaReadU32(Bytes + Offset + 36));
    Out.CameraFlags = int32(KyaReadU32(Bytes + Offset + 40));
    Out.Field8 = KyaReadF32(Bytes + Offset + 44);
    Out.FieldC = KyaReadF32(Bytes + Offset + 48);
    Out.Field10 = KyaReadF32(Bytes + Offset + 52);
    Out.ActorRef = int32(KyaReadU32(Bytes + Offset + 56));
    Out.ZoneRef = int32(KyaReadU32(Bytes + Offset + 60));
    Out.Field1C = KyaReadF32(Bytes + Offset + 64);
    Out.SoundRef = int32(KyaReadU32(Bytes + Offset + 68));
    return true;
}

bool KyaReadMovingPlatformActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaPlatformPrefixOffsets Prefix;
    if (!KyaReadPlatformPrefix(Data, Start, Size, Prefix)) return false;
    const uint8* Bytes = Data.GetData();

    Out.bHasPlatformPrefix = true;
    Out.PlatformWeight = KyaReadF32(Bytes + Prefix.Weight);
    Out.PlatformActors.Reset();
    for (int32 Index = 0; Index < Prefix.ActorCount; ++Index)
    {
        FKyaPlatformActor& Ref = Out.PlatformActors.AddDefaulted_GetRef();
        const int64 At = Prefix.Actors + int64(Index) * 8;
        Ref.ActorRef = int32(KyaReadU32(Bytes + At));
        Ref.CarryMethod = int32(KyaReadU32(Bytes + At + 4));
    }
    Out.PlatformZones.Reset();
    for (int32 Index = 0; Index < Prefix.ZoneCount; ++Index)
        Out.PlatformZones.Add(int32(KyaReadU32(Bytes + Prefix.Zones + int64(Index) * 4)));
    Out.PlatformNoFrictionZones.Reset();
    for (int64 Offset : Prefix.NoFrictionZones)
    {
        FKyaPlatformNoFrictionZone& Zone = Out.PlatformNoFrictionZones.AddDefaulted_GetRef();
        Zone.ZoneRef = int32(KyaReadU32(Bytes + Offset));
        const uint32 Count = KyaReadU32(Bytes + Offset + 4);
        for (uint32 Index = 0; Index < Count; ++Index)
            Zone.Actors.Add(int32(KyaReadU32(Bytes + Offset + 8 + int64(Index) * 4)));
    }

    const int64 Remaining = Start + Size - Prefix.Suffix;
    FKyaPlatformDetailsOffsets Details;
    Out.bHasPlatformDetails = KyaReadPlatformDetails(Data, Prefix.Suffix, Remaining, Details);
    if (!Out.bHasPlatformDetails)
    {
        Out.ClassData.Reset();
        if (Remaining > 0) Out.ClassData.Append(Bytes + Prefix.Suffix, int32(Remaining));
        return true;
    }

    Out.PlatformCameras.Reset();
    for (int32 Index = 0; Index < Details.CameraCount; ++Index)
    {
        FKyaPlatformCamera& Camera = Out.PlatformCameras.AddDefaulted_GetRef();
        if (!KyaReadPlatformCamera(Data, Details.Cameras + int64(Index) * 72, Camera)) return false;
    }

    FKyaPlatformProperties& P = Out.PlatformProperties;
    const int64 Offset = Details.Properties;
    P.SoundRefA = int32(KyaReadU32(Bytes + Offset));
    P.SampleRefA = int32(KyaReadU32(Bytes + Offset + 4));
    P.SoundRefB = int32(KyaReadU32(Bytes + Offset + 8));
    P.SampleRefB = int32(KyaReadU32(Bytes + Offset + 12));
    P.Offset = FVector(KyaReadF32(Bytes + Offset + 16), KyaReadF32(Bytes + Offset + 20),
        KyaReadF32(Bytes + Offset + 24));
    P.EffectRef1C = int64(KyaReadU32(Bytes + Offset + 28));
    P.EffectRef20 = int64(KyaReadU32(Bytes + Offset + 32));
    P.PlatformFlags = int64(KyaReadU32(Bytes + Offset + 36));
    P.CameraIndex = int32(KyaReadU32(Bytes + Offset + 40));
    P.TiltValue = KyaReadF32(Bytes + Offset + 44);
    P.TiltMode = int32(KyaReadU32(Bytes + Offset + 48));
    P.TiltAndPush = FVector4(KyaReadF32(Bytes + Offset + 52), KyaReadF32(Bytes + Offset + 56),
        KyaReadF32(Bytes + Offset + 60), KyaReadF32(Bytes + Offset + 64));
    P.PushOscillation = FVector2D(KyaReadF32(Bytes + Offset + 68), KyaReadF32(Bytes + Offset + 72));

    Out.PlatformSoundsA.Reset();
    Out.PlatformSoundsB.Reset();
    TArray<FKyaPlatformSound>* SoundLists[] = { &Out.PlatformSoundsA, &Out.PlatformSoundsB };
    for (int32 List = 0; List < 2; ++List)
        for (int32 Item = 0; Item < Details.SoundCounts[List]; ++Item)
        {
            const int64 SoundOffset = Details.Sounds[List] + int64(Item) * 16;
            FKyaPlatformSound& Sound = SoundLists[List]->AddDefaulted_GetRef();
            Sound.SegmentStart = int32(KyaReadU32(Bytes + SoundOffset));
            Sound.SegmentEnd = int32(KyaReadU32(Bytes + SoundOffset + 4));
            Sound.SoundRef = int32(KyaReadU32(Bytes + SoundOffset + 8));
            Sound.SampleRef = int32(KyaReadU32(Bytes + SoundOffset + 12));
        }

    Out.ClassData.Reset();
    if (Start + Size > Details.Tail)
        Out.ClassData.Append(Bytes + Details.Tail, int32(Start + Size - Details.Tail));
    return true;
}

bool KyaWriteMovingPlatformActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaPlatformPrefixOffsets Prefix;
    if (!KyaReadPlatformPrefix(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Prefix))
        return false;
    if (!Actor.bHasPlatformPrefix) return false;
    if (Actor.PlatformActors.Num() != Prefix.ActorCount || Actor.PlatformZones.Num() != Prefix.ZoneCount ||
        Actor.PlatformNoFrictionZones.Num() != Prefix.NoFrictionZones.Num()) return false;

    KyaWriteF32(Data, Prefix.Weight, Actor.PlatformWeight);
    for (int32 Index = 0; Index < Prefix.ActorCount; ++Index)
    {
        const FKyaPlatformActor& Ref = Actor.PlatformActors[Index];
        const int64 At = Prefix.Actors + int64(Index) * 8;
        KyaWriteU32(Data, At, uint32(Ref.ActorRef));
        KyaWriteU32(Data, At + 4, uint32(Ref.CarryMethod));
    }
    for (int32 Index = 0; Index < Prefix.ZoneCount; ++Index)
        KyaWriteU32(Data, Prefix.Zones + int64(Index) * 4, uint32(Actor.PlatformZones[Index]));
    for (int32 Index = 0; Index < Prefix.NoFrictionZones.Num(); ++Index)
    {
        const FKyaPlatformNoFrictionZone& Zone = Actor.PlatformNoFrictionZones[Index];
        const int64 Offset = Prefix.NoFrictionZones[Index];
        if (uint32(Zone.Actors.Num()) != KyaReadU32(Data.GetData() + Offset + 4)) return false;
        KyaWriteU32(Data, Offset, uint32(Zone.ZoneRef));
        for (int32 Item = 0; Item < Zone.Actors.Num(); ++Item)
            KyaWriteU32(Data, Offset + 8 + int64(Item) * 4, uint32(Zone.Actors[Item]));
    }

    if (!Actor.bHasPlatformDetails) return true;
    FKyaPlatformDetailsOffsets Details;
    const int64 Remaining = Start + Size - Prefix.Suffix;
    if (!KyaReadPlatformDetails(MakeArrayView(Data.GetData(), Data.Num()), Prefix.Suffix, Remaining,
        Details)) return false;
    if (Actor.PlatformCameras.Num() != Details.CameraCount ||
        Actor.PlatformSoundsA.Num() != Details.SoundCounts[0] ||
        Actor.PlatformSoundsB.Num() != Details.SoundCounts[1]) return false;

    for (int32 Index = 0; Index < Details.CameraCount; ++Index)
    {
        const FKyaPlatformCamera& Camera = Actor.PlatformCameras[Index];
        const int64 O = Details.Cameras + int64(Index) * 72;
        KyaWriteU32(Data, O, uint32(Camera.EventType));
        KyaWriteU32(Data, O + 4, uint32(Camera.Flags));
        KyaWriteU32(Data, O + 8, uint32(Camera.TargetRef));
        KyaWriteU32(Data, O + 12, uint32(Camera.CutsceneId));
        KyaWriteU32(Data, O + 16, uint32(Camera.TargetFlags));
        KyaWriteU32(Data, O + 20, uint32(Camera.MessageId));
        KyaWriteU32(Data, O + 24, uint32(Camera.MessageFlags));
        KyaWriteU32(Data, O + 28, uint32(Camera.TargetField14));
        KyaWriteU32(Data, O + 32, uint32(Camera.TargetField18));
        KyaWriteU32(Data, O + 36, uint32(Camera.CameraIndex));
        KyaWriteU32(Data, O + 40, uint32(Camera.CameraFlags));
        KyaWriteF32(Data, O + 44, Camera.Field8);
        KyaWriteF32(Data, O + 48, Camera.FieldC);
        KyaWriteF32(Data, O + 52, Camera.Field10);
        KyaWriteU32(Data, O + 56, uint32(Camera.ActorRef));
        KyaWriteU32(Data, O + 60, uint32(Camera.ZoneRef));
        KyaWriteF32(Data, O + 64, Camera.Field1C);
        KyaWriteU32(Data, O + 68, uint32(Camera.SoundRef));
    }

    const FKyaPlatformProperties& P = Actor.PlatformProperties;
    const int64 Offset = Details.Properties;
    KyaWriteU32(Data, Offset, uint32(P.SoundRefA));
    KyaWriteU32(Data, Offset + 4, uint32(P.SampleRefA));
    KyaWriteU32(Data, Offset + 8, uint32(P.SoundRefB));
    KyaWriteU32(Data, Offset + 12, uint32(P.SampleRefB));
    KyaWriteF32(Data, Offset + 16, P.Offset.X);
    KyaWriteF32(Data, Offset + 20, P.Offset.Y);
    KyaWriteF32(Data, Offset + 24, P.Offset.Z);
    KyaWriteU32(Data, Offset + 28, uint32(uint64(P.EffectRef1C)));
    KyaWriteU32(Data, Offset + 32, uint32(uint64(P.EffectRef20)));
    KyaWriteU32(Data, Offset + 36, uint32(uint64(P.PlatformFlags)));
    KyaWriteU32(Data, Offset + 40, uint32(P.CameraIndex));
    KyaWriteF32(Data, Offset + 44, P.TiltValue);
    KyaWriteU32(Data, Offset + 48, uint32(P.TiltMode));
    KyaWriteF32(Data, Offset + 52, P.TiltAndPush.X);
    KyaWriteF32(Data, Offset + 56, P.TiltAndPush.Y);
    KyaWriteF32(Data, Offset + 60, P.TiltAndPush.Z);
    KyaWriteF32(Data, Offset + 64, P.TiltAndPush.W);
    KyaWriteF32(Data, Offset + 68, float(P.PushOscillation.X));
    KyaWriteF32(Data, Offset + 72, float(P.PushOscillation.Y));

    const TArray<FKyaPlatformSound>* SoundLists[] = { &Actor.PlatformSoundsA, &Actor.PlatformSoundsB };
    for (int32 List = 0; List < 2; ++List)
        for (int32 Item = 0; Item < Details.SoundCounts[List]; ++Item)
        {
            const FKyaPlatformSound& Sound = (*SoundLists[List])[Item];
            const int64 SoundOffset = Details.Sounds[List] + int64(Item) * 16;
            KyaWriteU32(Data, SoundOffset, uint32(Sound.SegmentStart));
            KyaWriteU32(Data, SoundOffset + 4, uint32(Sound.SegmentEnd));
            KyaWriteU32(Data, SoundOffset + 8, uint32(Sound.SoundRef));
            KyaWriteU32(Data, SoundOffset + 12, uint32(Sound.SampleRef));
        }
    return true;
}

bool KyaReadPlatformBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaPlatformBehaviourOffsets& Out)
{
    Out = FKyaPlatformBehaviourOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 4) return false;
    const uint8* Bytes = Data.GetData();
    const auto U32 = [Bytes](int64 Offset)
    {
        return KyaReadU32(Bytes + Offset);
    };
    if (Id == 2)
    {
        // CBehaviourPlatformTrajectory::Create: absolute path reader (16 bytes) +
        // segment start time and loop delay.
        if (Size != 24) return false;
        Out.bTrajectory = true;
        Out.PathIndex = Start;
        Out.Timing = Start + 4;
        Out.PathField4 = Start + 8;
        Out.PathTotalTime = Start + 12;
        Out.SegmentStartTime = Start + 16;
        Out.LoopDelay = Start + 20;
        Out.Tail = Start + 24;
        return true;
    }
    if (Id == 3)
    {
        if (Size < 56) return false;
        Out.bSlab = true;
        const uint32 Targets = U32(Start);
        Out.SlabSwitch.Count = int32(Targets);
        Out.SlabSwitch.Targets = Start + 4;
        if (Targets > uint64(Size - 36) / 28) return false;
        const int64 Camera = Out.SlabSwitch.Targets + int64(Targets) * 28;
        if (Start + Size - Camera < 52) return false;
        Out.SlabSwitch.Camera = Camera;
        Out.SlabFloats = Camera + 32;
        Out.SlabField20 = Out.SlabFloats + 16;
        if (Out.SlabField20 + 4 != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 4)
    {
        // CBehaviourWeighingMachineSlave::Create: path index + timing.
        if (Size != 8) return false;
        Out.bSlave = true;
        Out.PathIndex = Start;
        Out.Timing = Start + 4;
        Out.Tail = Start + 8;
        return true;
    }
    if (Id == 5)
    {
        // CBehaviourWeighingMachineMaster::Create: path/timing, stream actor,
        // two floats, analog switch list and camera.
        if (Size < 56) return false;
        Out.bMaster = true;
        Out.MasterPath = Start;
        Out.MasterStream = Start + 8;
        Out.MasterFloats = Start + 12;
        Out.AnalogCount = Start + 20;
        const uint32 Targets = U32(Out.AnalogCount);
        Out.AnalogCountValue = int32(Targets);
        Out.AnalogEntries = Start + 24;
        if (Targets > uint64(Size - 56) / 40) return false;
        Out.AnalogCamera = Out.AnalogEntries + int64(Targets) * 40;
        if (Out.AnalogCamera + 32 != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 6)
    {
        // CBehaviourPlatformDestroyed::Create: absolute path reader + field 40
        // and alternate model id.
        if (Size != 24) return false;
        Out.bDestroyed = true;
        Out.PathIndex = Start;
        Out.Timing = Start + 4;
        Out.PathField4 = Start + 8;
        Out.PathTotalTime = Start + 12;
        Out.Field40 = Start + 16;
        Out.AltModelId = Start + 20;
        Out.Tail = Start + 24;
        return true;
    }
    if (Id == 7)
    {
        // CBehaviourPlatformStand::Create: sound ref + field 0x0c.
        if (Size != 8) return false;
        Out.bStand = true;
        Out.SoundRef = Start;
        Out.Field0C = Start + 4;
        Out.Tail = Start + 8;
        return true;
    }
    if (Id == 8)
    {
        // CBehaviourSelectorMaster::Create: selector base (24) + actor stream
        // count (4) + switch list count (4) + camera (32) = 64 minimum.
        if (Size < 64) return false;
        Out.bSelectorMaster = true;
        Out.Field18 = Start;
        Out.MaterialId = Start + 4;
        Out.Field8 = Start + 8;
        Out.FieldC = Start + 12;
        Out.Field10 = Start + 16;
        Out.SoundRef = Start + 20;
        Out.SelectorActorCount = Start + 24;
        const uint32 Actors = U32(Out.SelectorActorCount);
        Out.SelectorActorCountValue = int32(Actors);
        Out.SelectorActorEntries = Start + 28;
        int64 Pos = Out.SelectorActorEntries + int64(Actors) * 4;
        if (Start + Size - Pos < 4) return false;
        Out.SelectorSwitchCount = Pos;
        const uint32 Switches = U32(Pos);
        Out.SelectorSwitchCountValue = int32(Switches);
        Out.SelectorSwitchEntries = Pos + 4;
        if (Switches > uint64(Start + Size - Out.SelectorSwitchEntries) / 16) return false;
        Out.SelectorCamera = Out.SelectorSwitchEntries + int64(Switches) * 16;
        if (Out.SelectorCamera + 32 != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 9)
    {
        // CBehaviourSelectorSlave::Create: selector base only.
        if (Size != 24) return false;
        Out.bSelectorSlave = true;
        Out.Field18 = Start;
        Out.MaterialId = Start + 4;
        Out.Field8 = Start + 8;
        Out.FieldC = Start + 12;
        Out.Field10 = Start + 16;
        Out.SoundRef = Start + 20;
        Out.Tail = Start + 24;
        return true;
    }
    if (Id == 10)
    {
        // CBehaviourTeleportRandom::Create: one path-follow index.
        if (Size != 4) return false;
        Out.bTeleportRandom = true;
        Out.PathIndex = Start;
        Out.Tail = Start + 4;
        return true;
    }
    return false;
}

bool KyaReadMovingPlatformBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    FKyaPlatformBehaviourOffsets Off;
    if (!KyaReadPlatformBehaviourOffsets(Id, Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.PlatformBehaviour = FKyaPlatformBehaviour{};
    FKyaPlatformBehaviour& Values = Out.PlatformBehaviour;

    if (Off.bStand)
    {
        Values.SoundRef = int32(KyaReadU32(Bytes + Off.SoundRef));
        Values.Field0C = int32(KyaReadU32(Bytes + Off.Field0C));
    }
    if (Off.bSlab)
    {
        if (!KyaReadSwitchData(Data, Off.SlabSwitch.Targets, Off.SlabSwitch.Count,
            Off.SlabSwitch.Camera, Values.SlabSwitch)) return false;
        Values.SlabField10 = KyaReadF32(Bytes + Off.SlabFloats);
        Values.SlabField14 = KyaReadF32(Bytes + Off.SlabFloats + 4);
        Values.SlabField18 = KyaReadF32(Bytes + Off.SlabFloats + 8);
        Values.SlabField1C = KyaReadF32(Bytes + Off.SlabFloats + 12);
        Values.SlabField20 = int32(KyaReadU32(Bytes + Off.SlabField20));
    }
    if (Off.bMaster)
    {
        Values.MasterPathIndex = int32(KyaReadU32(Bytes + Off.MasterPath));
        Values.MasterTiming = int32(KyaReadU32(Bytes + Off.MasterPath + 4));
        Values.MasterStreamActor = int32(KyaReadU32(Bytes + Off.MasterStream));
        Values.MasterField40 = KyaReadF32(Bytes + Off.MasterFloats);
        Values.MasterField44 = KyaReadF32(Bytes + Off.MasterFloats + 4);
        Values.AnalogTargets.Reset();
        for (int32 Index = 0; Index < Off.AnalogCountValue; ++Index)
        {
            FKyaAnalogTarget& Target = Values.AnalogTargets.AddDefaulted_GetRef();
            const int64 T = Off.AnalogEntries + int64(Index) * 40;
            Target.ActorRef = int32(KyaReadU32(Bytes + T));
            Target.CutsceneId = int32(KyaReadU32(Bytes + T + 4));
            Target.Flags = int32(KyaReadU32(Bytes + T + 8));
            Target.Field0C = int32(KyaReadU32(Bytes + T + 12));
            Target.Field10 = int32(KyaReadU32(Bytes + T + 16));
            Target.Field14 = int32(KyaReadU32(Bytes + T + 20));
            Target.Field18 = int32(KyaReadU32(Bytes + T + 24));
            Target.Field1C = int32(KyaReadU32(Bytes + T + 28));
            Target.Field20 = KyaReadF32(Bytes + T + 32);
            Target.Field24 = KyaReadF32(Bytes + T + 36);
        }
        const int64 C = Off.AnalogCamera;
        Values.AnalogCamera.CameraIndex = int32(KyaReadU32(Bytes + C));
        Values.AnalogCamera.Flags = int32(KyaReadU32(Bytes + C + 4));
        Values.AnalogCamera.Field8 = KyaReadF32(Bytes + C + 8);
        Values.AnalogCamera.FieldC = KyaReadF32(Bytes + C + 12);
        Values.AnalogCamera.Field10 = KyaReadF32(Bytes + C + 16);
        Values.AnalogCamera.ActorRef = int32(KyaReadU32(Bytes + C + 20));
        Values.AnalogCamera.ZoneRef = int32(KyaReadU32(Bytes + C + 24));
        Values.AnalogCamera.Field1C = KyaReadF32(Bytes + C + 28);
    }
    if (Off.bSelectorMaster)
    {
        Values.Field18 = int32(KyaReadU32(Bytes + Off.Field18));
        Values.MaterialId = int32(KyaReadU32(Bytes + Off.MaterialId));
        Values.Field8 = KyaReadF32(Bytes + Off.Field8);
        Values.FieldC = KyaReadF32(Bytes + Off.FieldC);
        Values.Field10 = KyaReadF32(Bytes + Off.Field10);
        Values.SoundRef = int32(KyaReadU32(Bytes + Off.SoundRef));
        Values.SelectorActorRefs.Reset();
        for (int32 Index = 0; Index < Off.SelectorActorCountValue; ++Index)
            Values.SelectorActorRefs.Add(int32(KyaReadU32(Bytes + Off.SelectorActorEntries + int64(Index) * 4)));
        Values.SelectorSwitchEx.Reset();
        for (int32 Index = 0; Index < Off.SelectorSwitchCountValue; ++Index)
        {
            FKyaSwitchExTarget& Target = Values.SelectorSwitchEx.AddDefaulted_GetRef();
            const int64 T = Off.SelectorSwitchEntries + int64(Index) * 16;
            Target.ActorRef = int32(KyaReadU32(Bytes + T));
            Target.CutsceneId = int32(KyaReadU32(Bytes + T + 4));
            Target.Flags = int32(KyaReadU32(Bytes + T + 8));
            Target.MessageId = int32(KyaReadU32(Bytes + T + 12));
        }
        const int64 C = Off.SelectorCamera;
        Values.SelectorCamera.CameraIndex = int32(KyaReadU32(Bytes + C));
        Values.SelectorCamera.Flags = int32(KyaReadU32(Bytes + C + 4));
        Values.SelectorCamera.Field8 = KyaReadF32(Bytes + C + 8);
        Values.SelectorCamera.FieldC = KyaReadF32(Bytes + C + 12);
        Values.SelectorCamera.Field10 = KyaReadF32(Bytes + C + 16);
        Values.SelectorCamera.ActorRef = int32(KyaReadU32(Bytes + C + 20));
        Values.SelectorCamera.ZoneRef = int32(KyaReadU32(Bytes + C + 24));
        Values.SelectorCamera.Field1C = KyaReadF32(Bytes + C + 28);
    }
    if (Off.bTrajectory)
    {
        Values.PathIndex = int32(KyaReadU32(Bytes + Off.PathIndex));
        Values.Timing = int32(KyaReadU32(Bytes + Off.Timing));
        Values.PathField4 = KyaReadF32(Bytes + Off.PathField4);
        Values.PathTotalTime = KyaReadF32(Bytes + Off.PathTotalTime);
        Values.SegmentStartTime = KyaReadF32(Bytes + Off.SegmentStartTime);
        Values.LoopDelay = KyaReadF32(Bytes + Off.LoopDelay);
    }
    if (Off.bSlave)
    {
        Values.PathIndex = int32(KyaReadU32(Bytes + Off.PathIndex));
        Values.Timing = int32(KyaReadU32(Bytes + Off.Timing));
    }
    if (Off.bDestroyed)
    {
        Values.PathIndex = int32(KyaReadU32(Bytes + Off.PathIndex));
        Values.Timing = int32(KyaReadU32(Bytes + Off.Timing));
        Values.PathField4 = KyaReadF32(Bytes + Off.PathField4);
        Values.PathTotalTime = KyaReadF32(Bytes + Off.PathTotalTime);
        Values.Field40 = int32(KyaReadU32(Bytes + Off.Field40));
        Values.AltModelId = int32(KyaReadU32(Bytes + Off.AltModelId));
    }
    if (Off.bSelectorSlave)
    {
        Values.Field18 = int32(KyaReadU32(Bytes + Off.Field18));
        Values.MaterialId = int32(KyaReadU32(Bytes + Off.MaterialId));
        Values.Field8 = KyaReadF32(Bytes + Off.Field8);
        Values.FieldC = KyaReadF32(Bytes + Off.FieldC);
        Values.Field10 = KyaReadF32(Bytes + Off.Field10);
        Values.SoundRef = int32(KyaReadU32(Bytes + Off.SoundRef));
    }
    if (Off.bTeleportRandom) Values.PathIndex = int32(KyaReadU32(Bytes + Off.PathIndex));

    Out.bHasTypedPlatformBehaviour = true;
    return true;
}

bool KyaWriteMovingPlatformBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    FKyaPlatformBehaviourOffsets Off;
    if (!KyaReadPlatformBehaviourOffsets(Behaviour.Id, MakeArrayView(Data.GetData(), Data.Num()),
        Start, Size, Off)) return false;
    const FKyaPlatformBehaviour& Values = Behaviour.PlatformBehaviour;

    if (Off.bStand)
    {
        KyaWriteU32(Data, Off.SoundRef, uint32(Values.SoundRef));
        KyaWriteU32(Data, Off.Field0C, uint32(Values.Field0C));
    }
    if (Off.bTrajectory)
    {
        KyaWriteU32(Data, Off.PathIndex, uint32(Values.PathIndex));
        KyaWriteU32(Data, Off.Timing, uint32(Values.Timing));
        KyaWriteF32(Data, Off.PathField4, Values.PathField4);
        KyaWriteF32(Data, Off.PathTotalTime, Values.PathTotalTime);
        KyaWriteF32(Data, Off.SegmentStartTime, Values.SegmentStartTime);
        KyaWriteF32(Data, Off.LoopDelay, Values.LoopDelay);
    }
    if (Off.bSlab)
    {
        if (Values.SlabSwitch.Targets.Num() != Off.SlabSwitch.Count) return false;
        KyaWriteSwitchData(Data, Values.SlabSwitch, Off.SlabSwitch.Targets, Off.SlabSwitch.Camera);
        KyaWriteF32(Data, Off.SlabFloats, Values.SlabField10);
        KyaWriteF32(Data, Off.SlabFloats + 4, Values.SlabField14);
        KyaWriteF32(Data, Off.SlabFloats + 8, Values.SlabField18);
        KyaWriteF32(Data, Off.SlabFloats + 12, Values.SlabField1C);
        KyaWriteU32(Data, Off.SlabField20, uint32(Values.SlabField20));
    }
    if (Off.bMaster)
    {
        KyaWriteU32(Data, Off.MasterPath, uint32(Values.MasterPathIndex));
        KyaWriteU32(Data, Off.MasterPath + 4, uint32(Values.MasterTiming));
        KyaWriteU32(Data, Off.MasterStream, uint32(Values.MasterStreamActor));
        KyaWriteF32(Data, Off.MasterFloats, Values.MasterField40);
        KyaWriteF32(Data, Off.MasterFloats + 4, Values.MasterField44);
        if (Values.AnalogTargets.Num() != Off.AnalogCountValue) return false;
        for (int32 Index = 0; Index < Values.AnalogTargets.Num(); ++Index)
        {
            const FKyaAnalogTarget& Target = Values.AnalogTargets[Index];
            const int64 T = Off.AnalogEntries + int64(Index) * 40;
            KyaWriteU32(Data, T, uint32(Target.ActorRef));
            KyaWriteU32(Data, T + 4, uint32(Target.CutsceneId));
            KyaWriteU32(Data, T + 8, uint32(Target.Flags));
            KyaWriteU32(Data, T + 12, uint32(Target.Field0C));
            KyaWriteU32(Data, T + 16, uint32(Target.Field10));
            KyaWriteU32(Data, T + 20, uint32(Target.Field14));
            KyaWriteU32(Data, T + 24, uint32(Target.Field18));
            KyaWriteU32(Data, T + 28, uint32(Target.Field1C));
            KyaWriteF32(Data, T + 32, Target.Field20);
            KyaWriteF32(Data, T + 36, Target.Field24);
        }
        const FKyaStreamEventCamera& C = Values.AnalogCamera;
        const int64 O = Off.AnalogCamera;
        KyaWriteU32(Data, O, uint32(C.CameraIndex));
        KyaWriteU32(Data, O + 4, uint32(C.Flags));
        KyaWriteF32(Data, O + 8, C.Field8);
        KyaWriteF32(Data, O + 12, C.FieldC);
        KyaWriteF32(Data, O + 16, C.Field10);
        KyaWriteU32(Data, O + 20, uint32(C.ActorRef));
        KyaWriteU32(Data, O + 24, uint32(C.ZoneRef));
        KyaWriteF32(Data, O + 28, C.Field1C);
    }
    if (Off.bSelectorMaster)
    {
        KyaWriteU32(Data, Off.Field18, uint32(Values.Field18));
        KyaWriteU32(Data, Off.MaterialId, uint32(Values.MaterialId));
        KyaWriteF32(Data, Off.Field8, Values.Field8);
        KyaWriteF32(Data, Off.FieldC, Values.FieldC);
        KyaWriteF32(Data, Off.Field10, Values.Field10);
        KyaWriteU32(Data, Off.SoundRef, uint32(Values.SoundRef));
        if (Values.SelectorActorRefs.Num() != Off.SelectorActorCountValue) return false;
        for (int32 Index = 0; Index < Values.SelectorActorRefs.Num(); ++Index)
            KyaWriteU32(Data, Off.SelectorActorEntries + int64(Index) * 4, uint32(Values.SelectorActorRefs[Index]));
        if (Values.SelectorSwitchEx.Num() != Off.SelectorSwitchCountValue) return false;
        for (int32 Index = 0; Index < Values.SelectorSwitchEx.Num(); ++Index)
        {
            const FKyaSwitchExTarget& Target = Values.SelectorSwitchEx[Index];
            const int64 T = Off.SelectorSwitchEntries + int64(Index) * 16;
            KyaWriteU32(Data, T, uint32(Target.ActorRef));
            KyaWriteU32(Data, T + 4, uint32(Target.CutsceneId));
            KyaWriteU32(Data, T + 8, uint32(Target.Flags));
            KyaWriteU32(Data, T + 12, uint32(Target.MessageId));
        }
        const FKyaStreamEventCamera& C = Values.SelectorCamera;
        const int64 O = Off.SelectorCamera;
        KyaWriteU32(Data, O, uint32(C.CameraIndex));
        KyaWriteU32(Data, O + 4, uint32(C.Flags));
        KyaWriteF32(Data, O + 8, C.Field8);
        KyaWriteF32(Data, O + 12, C.FieldC);
        KyaWriteF32(Data, O + 16, C.Field10);
        KyaWriteU32(Data, O + 20, uint32(C.ActorRef));
        KyaWriteU32(Data, O + 24, uint32(C.ZoneRef));
        KyaWriteF32(Data, O + 28, C.Field1C);
    }
    if (Off.bSlave)
    {
        KyaWriteU32(Data, Off.PathIndex, uint32(Values.PathIndex));
        KyaWriteU32(Data, Off.Timing, uint32(Values.Timing));
    }
    if (Off.bDestroyed)
    {
        KyaWriteU32(Data, Off.PathIndex, uint32(Values.PathIndex));
        KyaWriteU32(Data, Off.Timing, uint32(Values.Timing));
        KyaWriteF32(Data, Off.PathField4, Values.PathField4);
        KyaWriteF32(Data, Off.PathTotalTime, Values.PathTotalTime);
        KyaWriteU32(Data, Off.Field40, uint32(Values.Field40));
        KyaWriteU32(Data, Off.AltModelId, uint32(Values.AltModelId));
    }
    if (Off.bSelectorSlave)
    {
        KyaWriteU32(Data, Off.Field18, uint32(Values.Field18));
        KyaWriteU32(Data, Off.MaterialId, uint32(Values.MaterialId));
        KyaWriteF32(Data, Off.Field8, Values.Field8);
        KyaWriteF32(Data, Off.FieldC, Values.FieldC);
        KyaWriteF32(Data, Off.Field10, Values.Field10);
        KyaWriteU32(Data, Off.SoundRef, uint32(Values.SoundRef));
    }
    if (Off.bTeleportRandom) KyaWriteU32(Data, Off.PathIndex, uint32(Values.PathIndex));
    return true;
}
