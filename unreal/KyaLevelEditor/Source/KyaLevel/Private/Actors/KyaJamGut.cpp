#include "Actors/KyaJamGut.h"
#include "KyaLevelBank.h"
namespace
{
bool Valid(int64 Count, int64 Start, int64 Size)
{
    return Start >= 0 && Size == 16 && Start <= Count && Size <= Count - Start;
}
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
void Write(uint8* P, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) P[I] = uint8(Value >> (I * 8));
}
}
bool KyaReadJamGutBehaviour(FKyaActorBehaviour& Out, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    if (Out.Id != 4 || !Valid(Data.Num(), Start, Size)) return false;
    const uint8* P = Data.GetData() + Start;
    Out.JamGutMusicIndex = Read(P);
    float* Fields[] = {&Out.JamGutField14, &Out.JamGutField18, &Out.JamGutField1C};
    for (int32 I = 0; I < 3; ++I)
    {
        const uint32 Bits = Read(P + 4 + I * 4);
        FMemory::Memcpy(Fields[I], &Bits, 4);
    }
    Out.bHasTypedJamGutBehaviour = true;
    return true;
}
bool KyaWriteJamGutBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start, int64 Size)
{
    if (!Behaviour.bHasTypedJamGutBehaviour || Behaviour.Id != 4 || !Valid(Data.Num(), Start, Size) ||
        Behaviour.JamGutMusicIndex < 0 || Behaviour.JamGutMusicIndex > MAX_uint32) return false;
    uint8* P = Data.GetData() + Start;
    Write(P, uint32(Behaviour.JamGutMusicIndex));
    const float Fields[] = {Behaviour.JamGutField14, Behaviour.JamGutField18, Behaviour.JamGutField1C};
    for (int32 I = 0; I < 3; ++I)
    {
        uint32 Bits;
        FMemory::Memcpy(&Bits, &Fields[I], 4);
        Write(P + 4 + I * 4, Bits);
    }
    return true;
}
