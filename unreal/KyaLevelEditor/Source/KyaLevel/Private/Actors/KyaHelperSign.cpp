#include "Actors/KyaHelperSign.h"
#include "KyaLevelBank.h"

namespace
{
bool Valid(int64 Count, int64 Start, int64 Size, int64 Minimum)
{
    return Start >= 0 && Size >= Minimum && Start <= Count && Size <= Count - Start;
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

bool KyaReadHelperSignActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    if (!Valid(Data.Num(), Start, Size, 12)) return false;
    const uint8* P = Data.GetData() + Start;
    Actor.HelperSignField168 = int32(Read(P));
    const uint64 Hash = uint64(Read(P + 4)) | uint64(Read(P + 8)) << 32;
    FMemory::Memcpy(&Actor.HelperSignTextHash, &Hash, 8);
    Actor.bHasHelperSignData = true;
    Actor.ClassData.Reset();
    Actor.ClassData.Append(P + 12, int32(Size - 12));
    return true;
}

bool KyaWriteHelperSignActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    if (!Actor.bHasHelperSignData || !Valid(Data.Num(), Start, Size, 12)) return false;
    uint8* P = Data.GetData() + Start;
    Write(P, uint32(Actor.HelperSignField168));
    const uint64 Hash = uint64(Actor.HelperSignTextHash);
    Write(P + 4, uint32(Hash));
    Write(P + 8, uint32(Hash >> 32));
    return true;
}

bool KyaReadHelperSignBehaviour(FKyaActorBehaviour& Behaviour, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    if (Behaviour.Id != 2 && Behaviour.Id != 3) return false;
    if (!Valid(Data.Num(), Start, Size, 0) || Size != (Behaviour.Id == 3 ? 4 : 0)) return false;
    if (Behaviour.Id == 3) Behaviour.HelperSignOrganizerRef = int32(Read(Data.GetData() + Start));
    Behaviour.bHasTypedHelperSignBehaviour = true;
    return true;
}

bool KyaWriteHelperSignBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start, int64 Size)
{
    if (!Behaviour.bHasTypedHelperSignBehaviour || (Behaviour.Id != 2 && Behaviour.Id != 3) ||
        !Valid(Data.Num(), Start, Size, 0) || Size != (Behaviour.Id == 3 ? 4 : 0)) return false;
    if (Behaviour.Id == 3) Write(Data.GetData() + Start, uint32(Behaviour.HelperSignOrganizerRef));
    return true;
}
