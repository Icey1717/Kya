#include "Actors/KyaFog.h"

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

bool KyaReadFogOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaFogOffsets& Out)
{
    Out = FKyaFogOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 4) return false;
    const uint8* Bytes = Data.GetData();
    Out.Count = Start;
    const uint32 Count = KyaReadU32(Bytes + Start);
    Out.CountValue = int32(Count);
    Out.Entries = Start + 4;
    if (uint64(Count) > uint64(Size - 4) / 16) return false;
    Out.Tail = Out.Entries + int64(Count) * 16;
    return true;
}

bool KyaReadFogActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaFogOffsets Off;
    if (!KyaReadFogOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.FogHierarchies.Reset();
    for (int32 Index = 0; Index < Off.CountValue; ++Index)
    {
        FKyaFogHierarchy& Hierarchy = Out.FogHierarchies.AddDefaulted_GetRef();
        const int64 H = Off.Entries + int64(Index) * 16;
        Hierarchy.Hash = int64(uint64(KyaReadU32(Bytes + H)) |
            (uint64(KyaReadU32(Bytes + H + 4)) << 32));
        Hierarchy.Field20 = KyaReadF32(Bytes + H + 8);
        Hierarchy.Field24 = KyaReadF32(Bytes + H + 12);
    }

    Out.bHasFogData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteFogActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaFogOffsets Off;
    if (!KyaReadFogOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.FogHierarchies.Num() != Off.CountValue) return false;
    for (int32 Index = 0; Index < Actor.FogHierarchies.Num(); ++Index)
    {
        const FKyaFogHierarchy& Hierarchy = Actor.FogHierarchies[Index];
        const int64 H = Off.Entries + int64(Index) * 16;
        KyaWriteU32(Data, H, uint32(uint64(Hierarchy.Hash)));
        KyaWriteU32(Data, H + 4, uint32(uint64(Hierarchy.Hash) >> 32));
        KyaWriteF32(Data, H + 8, Hierarchy.Field20);
        KyaWriteF32(Data, H + 12, Hierarchy.Field24);
    }
    return true;
}

bool KyaReadFogZoneOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End,
    FKyaFogZoneOffsets& Out)
{
    Out = FKyaFogZoneOffsets{};
    if (Start < 0 || End > Data.Num() || End - Start < 4) return false;
    const uint8* Bytes = Data.GetData();
    Out.Count = Start;
    const uint32 Count = KyaReadU32(Bytes + Start);
    Out.CountValue = int32(Count);
    Out.Entries = Start + 4;
    if (uint64(Count) > uint64(End - Out.Entries) / 28) return false;
    Out.Tail = Out.Entries + int64(Count) * 28;
    return true;
}

bool KyaReadFogBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Id != 2) return false;
    FKyaFogZoneOffsets Off;
    if (!KyaReadFogZoneOffsets(Data, Start, Start + Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.FogZones.Reset();
    for (int32 Index = 0; Index < Off.CountValue; ++Index)
    {
        FKyaFogZone& Zone = Out.FogZones.AddDefaulted_GetRef();
        const int64 Z = Off.Entries + int64(Index) * 28;
        Zone.Flags = int32(KyaReadU32(Bytes + Z));
        Zone.ZoneRef = int32(KyaReadU32(Bytes + Z + 4));
        Zone.ClipValue = KyaReadF32(Bytes + Z + 8);
        Zone.Field4 = KyaReadF32(Bytes + Z + 12);
        Zone.FogRGBA = int32(KyaReadU32(Bytes + Z + 16));
        Zone.FogFlags = int32(KyaReadU32(Bytes + Z + 20));
        Zone.Field18 = KyaReadF32(Bytes + Z + 24);
    }
    Out.bHasTypedFogBehaviour = true;
    return true;
}

bool KyaWriteFogBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id != 2) return false;
    FKyaFogZoneOffsets Off;
    if (!KyaReadFogZoneOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Start + Size,
        Off)) return false;
    if (Behaviour.FogZones.Num() != Off.CountValue) return false;
    for (int32 Index = 0; Index < Behaviour.FogZones.Num(); ++Index)
    {
        const FKyaFogZone& Zone = Behaviour.FogZones[Index];
        const int64 Z = Off.Entries + int64(Index) * 28;
        KyaWriteU32(Data, Z, uint32(Zone.Flags));
        KyaWriteU32(Data, Z + 4, uint32(Zone.ZoneRef));
        KyaWriteF32(Data, Z + 8, Zone.ClipValue);
        KyaWriteF32(Data, Z + 12, Zone.Field4);
        KyaWriteU32(Data, Z + 16, uint32(Zone.FogRGBA));
        KyaWriteU32(Data, Z + 20, uint32(Zone.FogFlags));
        KyaWriteF32(Data, Z + 24, Zone.Field18);
    }
    return true;
}
