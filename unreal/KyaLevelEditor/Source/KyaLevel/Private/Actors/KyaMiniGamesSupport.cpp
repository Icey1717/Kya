#include "Actors/KyaMiniGamesSupport.h"
#include "KyaLevelBank.h"
namespace
{
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
void Write(TArray<uint8>& Data, int64 Pos, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) Data[int32(Pos) + I] = uint8(Value >> (I * 8));
}
}
bool KyaReadMiniGamesSupportOffsets(int64 ClassId, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaMiniGamesSupportOffsets& Out)
{
    Out = {};
    if ((ClassId != 0x42 && ClassId != 0x43) || Start < 0 || Size < 8 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    int64 Pos = Start;
    if (ClassId == 0x43)
    {
        bool Terminated = false;
        while (Pos < End)
        {
            const uint8 Byte = Data[int32(Pos++)];
            if (Byte == 0) { Terminated = true; break; }
            if (Byte > 127) return false;
        }
        if (!Terminated) return false;
        Out.NameEnd = (Pos + 3) & ~int64(3);
        Pos = Out.NameEnd;
        if (End - Pos < 44) return false; // four scalars, count, six suffix words
        Out.Scalars = Pos;
        Pos += 16;
    }
    if (End - Pos < 4) return false;
    const uint32 Count = Read(Data.GetData() + Pos);
    Pos += 4;
    const int64 SuffixSize = ClassId == 0x43 ? 24 : 4;
    if (End - Pos < SuffixSize || Count > MAX_int32 || uint64(Count) > uint64(End - Pos - SuffixSize) / 4) return false;
    Out.Count = int32(Count);
    Out.Entries = Pos;
    Out.Suffix = Pos + int64(Count) * 4;
    Out.Tail = Out.Suffix + SuffixSize;
    return true;
}
bool KyaReadMiniGamesSupportActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaMiniGamesSupportOffsets Off;
    if (!KyaReadMiniGamesSupportOffsets(Actor.ClassId, Data, Start, Size, Off)) return false;
    TArray<int32>& Refs = Actor.ClassId == 0x43 ? Actor.MiniGamesOrganizerData.MiniGameRefs : Actor.MiniGamesManagerOrganizerRefs;
    Refs.Reset();
    for (int32 I = 0; I < Off.Count; ++I) Refs.Add(int32(Read(Data.GetData() + Off.Entries + int64(I) * 4)));
    if (Actor.ClassId == 0x42)
    {
        Actor.MiniGamesManagerActorRef = int32(Read(Data.GetData() + Off.Suffix));
        Actor.bHasMiniGamesManagerData = true;
    }
    else
    {
        FKyaMiniGamesOrganizerData& V = Actor.MiniGamesOrganizerData;
        V.Name.Reset();
        for (int64 P = Start; P < Off.NameEnd && Data[int32(P)] != 0; ++P) V.Name.AppendChar(TCHAR(Data[int32(P)]));
        const uint8* P = Data.GetData() + Off.Scalars;
        V.Field16C = int32(Read(P));
        V.TextureIndex = int32(Read(P + 4));
        V.MeshIndex = int32(Read(P + 8));
        V.MaterialId = int32(Read(P + 12));
        P = Data.GetData() + Off.Suffix;
        V.Field180 = Read(P);
        V.Field184 = Read(P + 4);
        V.Field188 = int32(Read(P + 8));
        V.Field18C = int32(Read(P + 12));
        V.Field190 = int32(Read(P + 16));
        uint32 Bits = Read(P + 20);
        FMemory::Memcpy(&V.Field194, &Bits, 4);
        Actor.bHasMiniGamesOrganizerData = true;
    }
    Actor.ClassData.Reset();
    Actor.ClassData.Append(Data.GetData() + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}
bool KyaWriteMiniGamesSupportActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaMiniGamesSupportOffsets Off;
    if (!KyaReadMiniGamesSupportOffsets(Actor.ClassId, MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off)) return false;
    const TArray<int32>& Refs = Actor.ClassId == 0x43 ? Actor.MiniGamesOrganizerData.MiniGameRefs : Actor.MiniGamesManagerOrganizerRefs;
    if (Refs.Num() != Off.Count) return false;
    if (Actor.ClassId == 0x42)
    {
        if (!Actor.bHasMiniGamesManagerData) return false;
        Write(Data, Off.Suffix, uint32(Actor.MiniGamesManagerActorRef));
    }
    else
    {
        const FKyaMiniGamesOrganizerData& V = Actor.MiniGamesOrganizerData;
        if (!Actor.bHasMiniGamesOrganizerData || V.Field180 < 0 || V.Field180 > MAX_uint32 || V.Field184 < 0 || V.Field184 > MAX_uint32 ||
            ((Start + V.Name.Len() + 1 + 3) & ~int64(3)) != Off.NameEnd) return false;
        for (TCHAR C : V.Name) if (C == 0 || uint32(C) > 127) return false;
        for (int32 I = 0; I < V.Name.Len(); ++I) Data[int32(Start) + I] = uint8(V.Name[I]);
        Data[int32(Start) + V.Name.Len()] = 0;
        Write(Data, Off.Scalars, uint32(V.Field16C));
        Write(Data, Off.Scalars + 4, uint32(V.TextureIndex));
        Write(Data, Off.Scalars + 8, uint32(V.MeshIndex));
        Write(Data, Off.Scalars + 12, uint32(V.MaterialId));
        Write(Data, Off.Suffix, uint32(V.Field180));
        Write(Data, Off.Suffix + 4, uint32(V.Field184));
        Write(Data, Off.Suffix + 8, uint32(V.Field188));
        Write(Data, Off.Suffix + 12, uint32(V.Field18C));
        Write(Data, Off.Suffix + 16, uint32(V.Field190));
        uint32 Bits;
        FMemory::Memcpy(&Bits, &V.Field194, 4);
        Write(Data, Off.Suffix + 20, Bits);
    }
    for (int32 I = 0; I < Refs.Num(); ++I) Write(Data, Off.Entries + int64(I) * 4, uint32(Refs[I]));
    return true;
}
