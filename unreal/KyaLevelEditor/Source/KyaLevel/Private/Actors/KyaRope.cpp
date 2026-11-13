#include "Actors/KyaRope.h"

#include "Actors/KyaSwitch.h"
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
}

bool KyaReadRopeOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaRopeOffsets& Out)
{
    Out = FKyaRopeOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 76) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Path = Start;
    Out.Scale = Start + 4;
    Out.Field17C = Start + 8;
    Out.Material = Start + 12;
    Out.Color = Start + 16;
    Out.Sound = Start + 20;
    Out.Field194 = Start + 24;
    Out.ActorA = Start + 28;
    Out.ActorB = Start + 32;
    int64 Pos = Start + 36;
    const uint32 Count = KyaReadU32(Bytes + Pos);
    Out.Switch.Count = int32(Count);
    Out.Switch.Targets = Pos + 4;
    if (Count > uint64(End - Pos - 4) / 28) return false;
    Out.Switch.Camera = Out.Switch.Targets + int64(Count) * 28;
    if (End - Out.Switch.Camera < 36) return false;
    Out.Flags = Out.Switch.Camera + 32;
    Out.Tail = Out.Flags + 4;
    return true;
}

bool KyaReadRopeActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaRopeOffsets Off;
    if (!KyaReadRopeOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.RopePathIndex = int32(KyaReadU32(Bytes + Off.Path));
    Out.RopeScale = KyaReadF32(Bytes + Off.Scale);
    Out.RopeField17C = KyaReadF32(Bytes + Off.Field17C);
    Out.RopeMaterialId = int32(KyaReadU32(Bytes + Off.Material));
    Out.RopeColor = int32(KyaReadU32(Bytes + Off.Color));
    Out.RopeSoundRef = int32(KyaReadU32(Bytes + Off.Sound));
    Out.RopeField194 = int32(KyaReadU32(Bytes + Off.Field194));
    Out.RopeActorRefA = int32(KyaReadU32(Bytes + Off.ActorA));
    Out.RopeActorRefB = int32(KyaReadU32(Bytes + Off.ActorB));
    Out.RopeSwitch = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Off.Switch.Targets, Off.Switch.Count, Off.Switch.Camera,
        Out.RopeSwitch)) return false;
    Out.RopeFlags = int32(KyaReadU32(Bytes + Off.Flags));

    Out.bHasRopeData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteRopeActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaRopeOffsets Off;
    if (!KyaReadRopeOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.RopeSwitch.Targets.Num() != Off.Switch.Count) return false;
    KyaWriteU32(Data, Off.Path, uint32(Actor.RopePathIndex));
    KyaWriteF32(Data, Off.Scale, Actor.RopeScale);
    KyaWriteF32(Data, Off.Field17C, Actor.RopeField17C);
    KyaWriteU32(Data, Off.Material, uint32(Actor.RopeMaterialId));
    KyaWriteU32(Data, Off.Color, uint32(Actor.RopeColor));
    KyaWriteU32(Data, Off.Sound, uint32(Actor.RopeSoundRef));
    KyaWriteU32(Data, Off.Field194, uint32(Actor.RopeField194));
    KyaWriteU32(Data, Off.ActorA, uint32(Actor.RopeActorRefA));
    KyaWriteU32(Data, Off.ActorB, uint32(Actor.RopeActorRefB));
    KyaWriteSwitchValue(Data, Actor.RopeSwitch, Off.Switch.Targets, Off.Switch.Camera);
    KyaWriteU32(Data, Off.Flags, uint32(Actor.RopeFlags));
    return true;
}
