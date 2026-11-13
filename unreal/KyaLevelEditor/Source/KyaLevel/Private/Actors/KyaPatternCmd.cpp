#include "Actors/KyaPatternCmd.h"
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
bool KyaReadPatternCmdOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaPatternCmdOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 60 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint32 Count = Read(Data.GetData() + Start + 8);
    if (Count > MAX_int32 || uint64(Count) > uint64(Size - 60) / 4) return false;
    Out.Count = int32(Count);
    Out.Entries = Start + 12;
    Out.Values = Out.Entries + int64(Count) * 4;
    Out.Pattern = Out.Values + 8;
    const int64 PatternSize = Start + Size - Out.Pattern;
    // CPatternPart is the final read: exact remaining size disambiguates 2.21.
    if (PatternSize != 40 && PatternSize != 44) return false;
    Out.bHasField24 = PatternSize == 44;
    Out.Tail = Start + Size;
    return true;
}
bool KyaReadPatternCmdActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaPatternCmdOffsets Off;
    if (!KyaReadPatternCmdOffsets(Data, Start, Size, Off)) return false;
    const uint8* P = Data.GetData();
    Actor.PatternCmdField164 = Read(P + Start);
    Actor.PatternCmdField160 = Read(P + Start + 4);
    Actor.PatternCmdActorRefs.Reset();
    for (int32 I = 0; I < Off.Count; ++I)
        Actor.PatternCmdActorRefs.Add(int32(Read(P + Off.Entries + int64(I) * 4)));
    Actor.PatternCmdField1F0 = ReadFloat(P + Off.Values);
    Actor.PatternCmdField1F4 = ReadFloat(P + Off.Values + 4);
    FKyaPatternPart& Pattern = Actor.PatternCmdPattern;
    Pattern = {};
    const uint8* B = P + Off.Pattern;
    Pattern.Field4 = ReadFloat(B);
    Pattern.Field0 = ReadFloat(B + 4);
    Pattern.FieldC = ReadFloat(B + 8);
    Pattern.Field8 = ReadFloat(B + 12);
    Pattern.Field10 = ReadFloat(B + 16);
    Pattern.Field14 = ReadFloat(B + 20);
    Pattern.MaterialId = int32(Read(B + 24));
    Pattern.PointCount = ReadFloat(B + 28);
    Pattern.Field20 = ReadFloat(B + 32);
    Pattern.bHasField24 = Off.bHasField24;
    Pattern.Field24 = Off.bHasField24 ? ReadFloat(B + 36) : 1.0f;
    Pattern.Field28 = int32(Read(B + (Off.bHasField24 ? 40 : 36)));
    Actor.ClassData.Reset();
    Actor.bHasPatternCmdData = true;
    return true;
}
bool KyaWritePatternCmdActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaPatternCmdOffsets Off;
    if (!Actor.bHasPatternCmdData || !KyaReadPatternCmdOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        Actor.PatternCmdActorRefs.Num() != Off.Count || Actor.PatternCmdPattern.bHasField24 != Off.bHasField24 ||
        Actor.PatternCmdField164 < 0 || Actor.PatternCmdField164 > MAX_uint32 ||
        Actor.PatternCmdField160 < 0 || Actor.PatternCmdField160 > MAX_uint32) return false;
    uint8* P = Data.GetData();
    Write(P + Start, uint32(Actor.PatternCmdField164));
    Write(P + Start + 4, uint32(Actor.PatternCmdField160));
    for (int32 I = 0; I < Off.Count; ++I)
        Write(P + Off.Entries + int64(I) * 4, uint32(Actor.PatternCmdActorRefs[I]));
    WriteFloat(P + Off.Values, Actor.PatternCmdField1F0);
    WriteFloat(P + Off.Values + 4, Actor.PatternCmdField1F4);
    const FKyaPatternPart& Pattern = Actor.PatternCmdPattern;
    uint8* B = P + Off.Pattern;
    WriteFloat(B, Pattern.Field4);
    WriteFloat(B + 4, Pattern.Field0);
    WriteFloat(B + 8, Pattern.FieldC);
    WriteFloat(B + 12, Pattern.Field8);
    WriteFloat(B + 16, Pattern.Field10);
    WriteFloat(B + 20, Pattern.Field14);
    Write(B + 24, uint32(Pattern.MaterialId));
    WriteFloat(B + 28, Pattern.PointCount);
    WriteFloat(B + 32, Pattern.Field20);
    if (Off.bHasField24) WriteFloat(B + 36, Pattern.Field24);
    Write(B + (Off.bHasField24 ? 40 : 36), uint32(Pattern.Field28));
    return true;
}
