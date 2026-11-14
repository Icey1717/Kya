#include "Actors/KyaEventGenerator.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadEventGeneratorOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaEventGeneratorOffsets& Out)
{
    Out = FKyaEventGeneratorOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 8) return false;
    Out.Enabled = Start;
    Out.ZoneRef = Start + 4;
    Out.Tail = Start + 8;
    return true;
}

bool KyaReadEventGeneratorActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    FKyaEventGeneratorOffsets Off;
    if (!KyaReadEventGeneratorOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.EventGeneratorEnabled = int32(KyaReadU32(Bytes + Off.Enabled));
    Out.EventGeneratorZoneRef = int32(KyaReadU32(Bytes + Off.ZoneRef));

    Out.bHasEventGeneratorData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteEventGeneratorActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    FKyaEventGeneratorOffsets Off;
    if (!KyaReadEventGeneratorOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size,
        Off)) return false;
    KyaWriteU32(Data, Off.Enabled, uint32(Actor.EventGeneratorEnabled));
    KyaWriteU32(Data, Off.ZoneRef, uint32(Actor.EventGeneratorZoneRef));
    return true;
}

bool KyaReadEventGeneratorBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size)
{
    // CBehaviourEventGen::Create: s32 field 0xC, f32 field 0x10, then nine
    // manager Create calls. Only the fixed header is typed; manager bytes stay
    // raw because several source Create paths are incomplete.
    if (Id != 2 || Start < 0 || Size < 8 || !KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    Out.EventGeneratorFieldC = int32(KyaReadU32(Bytes + Start));
    const uint32 Bits = KyaReadU32(Bytes + Start + 4);
    FMemory::Memcpy(&Out.EventGeneratorField10, &Bits, sizeof(Out.EventGeneratorField10));
    Out.bHasTypedEventGeneratorBehaviour = true;
    return true;
}

bool KyaWriteEventGeneratorBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id != 2 || Start < 0 || Size < 8 || !KyaRangeValid(Data, Start, Size))
        return false;
    KyaWriteU32(Data, Start, uint32(Behaviour.EventGeneratorFieldC));
    KyaWriteF32(Data, Start + 4, Behaviour.EventGeneratorField10);
    return true;
}
