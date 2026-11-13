#include "Actors/KyaCredits.h"
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
bool KyaReadCreditsOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaCreditsOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 104 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint32 Count = Read(Data.GetData() + Start);
    if (Count > MAX_int32 || uint64(Count) > uint64(Size - 104) / 8) return false;
    Out.Count = int32(Count);
    Out.Entries = Start + 4;
    Out.Values = Out.Entries + int64(Count) * 8;
    if (!KyaReadSwitchActorOffsets(Data, Out.Values + 64, Start + Size - Out.Values - 64, Out.Switch)) return false;
    Out.Tail = Out.Switch.Tail;
    return true;
}
bool KyaReadCreditsActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaCreditsOffsets Off;
    if (!KyaReadCreditsOffsets(Data, Start, Size, Off)) return false;
    FKyaCreditsData& V = Actor.CreditsData;
    V = {};
    for (int32 I = 0; I < Off.Count; ++I)
    {
        const uint8* P = Data.GetData() + Off.Entries + int64(I) * 8;
        const uint64 Bits = uint64(Read(P)) | uint64(Read(P + 4)) << 32;
        int64 Key;
        FMemory::Memcpy(&Key, &Bits, 8);
        V.MessageKeys.Add(Key);
    }
    const uint8* P = Data.GetData() + Off.Values;
    V.ScrollSpeed = ReadFloat(P + 0);
    V.TextBoxX = ReadFloat(P + 4);
    V.TextBoxY = ReadFloat(P + 8);
    V.TextBoxWidth = ReadFloat(P + 12);
    V.TextBoxHeight = ReadFloat(P + 16);
    V.BoxFillOpacity = ReadFloat(P + 20);
    V.BoxBorderOpacity = ReadFloat(P + 24);
    V.BoxBorderWidth = ReadFloat(P + 28);
    V.BoxBorderHeight = ReadFloat(P + 32);
    V.MusicIndex = Read(P + 36);
    V.MusicFadeInTime = ReadFloat(P + 40);
    V.MusicVolume = ReadFloat(P + 44);
    V.MusicFadeOutTime = ReadFloat(P + 48);
    V.NextLevelId = int32(Read(P + 52));
    V.NextLevelParam = int32(Read(P + 56));
    V.NextLevelSpawnParam = int32(Read(P + 60));
    if (!KyaReadSwitchValue(Data, Off.Switch.Targets, Off.Switch.Count, Off.Switch.Camera, V.CompletionSwitch)) return false;
    Actor.ClassData.Reset();
    Actor.ClassData.Append(Data.GetData() + Off.Tail, int32(Start + Size - Off.Tail));
    Actor.bHasCreditsData = true;
    return true;
}
bool KyaWriteCreditsActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaCreditsOffsets Off;
    const FKyaCreditsData& V = Actor.CreditsData;
    if (!Actor.bHasCreditsData || !KyaReadCreditsOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        V.MessageKeys.Num() != Off.Count || V.CompletionSwitch.Targets.Num() != Off.Switch.Count ||
        V.MusicIndex < 0 || V.MusicIndex > MAX_uint32) return false;
    for (int32 I = 0; I < Off.Count; ++I)
    {
        uint8* P = Data.GetData() + Off.Entries + int64(I) * 8;
        const uint64 Key = uint64(V.MessageKeys[I]);
        Write(P, uint32(Key));
        Write(P + 4, uint32(Key >> 32));
    }
    uint8* P = Data.GetData() + Off.Values;
    WriteFloat(P + 0, V.ScrollSpeed);
    WriteFloat(P + 4, V.TextBoxX);
    WriteFloat(P + 8, V.TextBoxY);
    WriteFloat(P + 12, V.TextBoxWidth);
    WriteFloat(P + 16, V.TextBoxHeight);
    WriteFloat(P + 20, V.BoxFillOpacity);
    WriteFloat(P + 24, V.BoxBorderOpacity);
    WriteFloat(P + 28, V.BoxBorderWidth);
    WriteFloat(P + 32, V.BoxBorderHeight);
    Write(P + 36, uint32(V.MusicIndex));
    WriteFloat(P + 40, V.MusicFadeInTime);
    WriteFloat(P + 44, V.MusicVolume);
    WriteFloat(P + 48, V.MusicFadeOutTime);
    Write(P + 52, uint32(V.NextLevelId));
    Write(P + 56, uint32(V.NextLevelParam));
    Write(P + 60, uint32(V.NextLevelSpawnParam));
    KyaWriteSwitchValue(Data, V.CompletionSwitch, Off.Switch.Targets, Off.Switch.Camera);
    return true;
}
