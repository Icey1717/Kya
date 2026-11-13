#include "Actors/KyaWantedZoo.h"
#include "KyaLevelBank.h"
namespace
{
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
float ReadFloat(const uint8* P)
{
    uint32 Bits = Read(P);
    float Value;
    FMemory::Memcpy(&Value, &Bits, 4);
    return Value;
}
void Write(uint8* P, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) P[I] = uint8(Value >> (I * 8));
}
void WriteFloat(uint8* P, float Value)
{
    uint32 Bits;
    FMemory::Memcpy(&Bits, &Value, 4);
    Write(P, Bits);
}
}
bool KyaReadWantedZooOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaWantedZooOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 12 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    int64 Pos = Start;
    auto List = [&](int64 Stride, int64& Entries, int32& Count) -> bool
    {
        if (End - Pos < 4) return false;
        const uint32 N = Read(Data.GetData() + Pos);
        Pos += 4;
        if (N > MAX_int32 || uint64(N) > uint64(End - Pos) / uint64(Stride)) return false;
        Count = int32(N);
        Entries = Pos;
        Pos += int64(N) * Stride;
        return true;
    };
    if (!List(8, Out.TextPositions, Out.TextCount) || !List(8, Out.NumberPositions, Out.NumberCount) || End - Pos < 4) return false;
    const uint32 Count = Read(Data.GetData() + Pos);
    Pos += 4;
    if (Count > MAX_int32 || uint64(Count) > uint64(End - Pos) / 12) return false;
    for (uint32 I = 0; I < Count; ++I)
    {
        FKyaWantedZooEntryOffsets& Entry = Out.Entries.AddDefaulted_GetRef();
        if (!List(16, Entry.Conditions, Entry.ConditionCount) || !List(8, Entry.Keys, Entry.KeyCount) ||
            !List(4, Entry.Numbers, Entry.NumberCount)) return false;
    }
    Out.Tail = Pos;
    return true;
}
bool KyaReadWantedZooActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaWantedZooOffsets Off;
    if (!KyaReadWantedZooOffsets(Data, Start, Size, Off)) return false;
    FKyaWantedZooData& V = Actor.WantedZooData;
    V = {};
    for (int32 List = 0; List < 2; ++List)
    {
        TArray<FVector2D>& Values = List == 0 ? V.TextPositions : V.NumberPositions;
        const int32 Count = List == 0 ? Off.TextCount : Off.NumberCount;
        const int64 StartPos = List == 0 ? Off.TextPositions : Off.NumberPositions;
        for (int32 I = 0; I < Count; ++I)
        {
            const uint8* P = Data.GetData() + StartPos + int64(I) * 8;
            Values.Add(FVector2D(ReadFloat(P), ReadFloat(P + 4)));
        }
    }
    for (const FKyaWantedZooEntryOffsets& E : Off.Entries)
    {
        FKyaWantedZooEntry& Entry = V.Entries.AddDefaulted_GetRef();
        for (int32 I = 0; I < E.ConditionCount; ++I)
        {
            const uint8* P = Data.GetData() + E.Conditions + int64(I) * 16;
            FKyaScenarioCondition& C = Entry.Conditions.AddDefaulted_GetRef();
            C.Combiner = int32(Read(P)); C.Variable = int32(Read(P + 4));
            C.Comparison = int32(Read(P + 8)); C.Value = int32(Read(P + 12));
        }
        for (int32 I = 0; I < E.KeyCount; ++I)
        {
            const uint8* P = Data.GetData() + E.Keys + int64(I) * 8;
            const uint64 Bits = uint64(Read(P)) | uint64(Read(P + 4)) << 32;
            int64 Key;
            FMemory::Memcpy(&Key, &Bits, 8);
            Entry.MessageKeys.Add(Key);
        }
        for (int32 I = 0; I < E.NumberCount; ++I)
            Entry.Numbers.Add(int32(Read(Data.GetData() + E.Numbers + int64(I) * 4)));
    }
    Actor.ClassData.Reset();
    Actor.ClassData.Append(Data.GetData() + Off.Tail, int32(Start + Size - Off.Tail));
    Actor.bHasWantedZooData = true;
    return true;
}
bool KyaWriteWantedZooActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaWantedZooOffsets Off;
    const FKyaWantedZooData& V = Actor.WantedZooData;
    if (!Actor.bHasWantedZooData || !KyaReadWantedZooOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        V.TextPositions.Num() != Off.TextCount || V.NumberPositions.Num() != Off.NumberCount || V.Entries.Num() != Off.Entries.Num()) return false;
    for (int32 I = 0; I < V.Entries.Num(); ++I)
        if (V.Entries[I].Conditions.Num() != Off.Entries[I].ConditionCount || V.Entries[I].MessageKeys.Num() != Off.Entries[I].KeyCount ||
            V.Entries[I].Numbers.Num() != Off.Entries[I].NumberCount) return false;
    for (int32 List = 0; List < 2; ++List)
    {
        const TArray<FVector2D>& Values = List == 0 ? V.TextPositions : V.NumberPositions;
        const int64 Pos = List == 0 ? Off.TextPositions : Off.NumberPositions;
        for (int32 I = 0; I < Values.Num(); ++I)
        {
            uint8* P = Data.GetData() + Pos + int64(I) * 8;
            WriteFloat(P, float(Values[I].X)); WriteFloat(P + 4, float(Values[I].Y));
        }
    }
    for (int32 I = 0; I < V.Entries.Num(); ++I)
    {
        const FKyaWantedZooEntry& Entry = V.Entries[I];
        const FKyaWantedZooEntryOffsets& E = Off.Entries[I];
        for (int32 C = 0; C < Entry.Conditions.Num(); ++C)
        {
            uint8* P = Data.GetData() + E.Conditions + int64(C) * 16;
            const FKyaScenarioCondition& Value = Entry.Conditions[C];
            Write(P, uint32(Value.Combiner)); Write(P + 4, uint32(Value.Variable));
            Write(P + 8, uint32(Value.Comparison)); Write(P + 12, uint32(Value.Value));
        }
        for (int32 K = 0; K < Entry.MessageKeys.Num(); ++K)
        {
            uint8* P = Data.GetData() + E.Keys + int64(K) * 8;
            const uint64 Key = uint64(Entry.MessageKeys[K]);
            Write(P, uint32(Key)); Write(P + 4, uint32(Key >> 32));
        }
        for (int32 N = 0; N < Entry.Numbers.Num(); ++N)
            Write(Data.GetData() + E.Numbers + int64(N) * 4, uint32(Entry.Numbers[N]));
    }
    return true;
}
