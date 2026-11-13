#include "Actors/KyaAmbre.h"

#include "KyaLevelBank.h"

namespace
{
uint32 KyaReadU32(const uint8* P)
{
    return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
}

void KyaWriteU32(TArray<uint8>& Data, int64 Offset, uint32 Value)
{
    for (int32 Byte = 0; Byte < 4; ++Byte) Data[int32(Offset) + Byte] = uint8(Value >> (Byte * 8));
}

bool KyaRangeValid(TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 0 && Start <= Data.Num() && Size <= Data.Num() - Start;
}
}

bool KyaReadAmbreOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaAmbreOffsets& Out)
{
    Out = FKyaAmbreOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 56) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Material = Start;
    Out.EffectType = Start + 4;
    Out.Texture = Start + 8;
    Out.Mesh = Start + 12;
    Out.Field1D0 = Start + 16;
    int64 Pos = Start + 20;
    const uint32 Count = KyaReadU32(Bytes + Pos);
    Out.Switch.Count = int32(Count);
    Out.Switch.Targets = Pos + 4;
    if (Count > uint64(End - Pos - 4) / 28) return false;
    Out.Switch.Camera = Out.Switch.Targets + int64(Count) * 28;
    if (End - Out.Switch.Camera < 32) return false;
    Out.Switch.Tail = Out.Switch.Camera + 32;
    Out.Tail = Out.Switch.Tail;
    return true;
}

bool KyaReadAmbreActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaAmbreOffsets Off;
    if (!KyaReadAmbreOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.AmbreMaterialId = int32(KyaReadU32(Bytes + Off.Material));
    Out.AmbreEffectType = int32(KyaReadU32(Bytes + Off.EffectType));
    Out.AmbreTextureIndex = int32(KyaReadU32(Bytes + Off.Texture));
    Out.AmbreMeshIndex = int32(KyaReadU32(Bytes + Off.Mesh));
    Out.AmbreField1D0 = int32(KyaReadU32(Bytes + Off.Field1D0));
    Out.AmbreSwitch = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Off.Switch.Targets, Off.Switch.Count, Off.Switch.Camera,
        Out.AmbreSwitch)) return false;

    Out.bHasAmbreData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteAmbreActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaAmbreOffsets Off;
    if (!KyaReadAmbreOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.AmbreSwitch.Targets.Num() != Off.Switch.Count) return false;
    KyaWriteU32(Data, Off.Material, uint32(Actor.AmbreMaterialId));
    KyaWriteU32(Data, Off.EffectType, uint32(Actor.AmbreEffectType));
    KyaWriteU32(Data, Off.Texture, uint32(Actor.AmbreTextureIndex));
    KyaWriteU32(Data, Off.Mesh, uint32(Actor.AmbreMeshIndex));
    KyaWriteU32(Data, Off.Field1D0, uint32(Actor.AmbreField1D0));
    KyaWriteSwitchValue(Data, Actor.AmbreSwitch, Off.Switch.Targets, Off.Switch.Camera);
    return true;
}
