#include "Actors/KyaMicken.h"

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

bool KyaReadMickenOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaMickenOffsets& Out)
{
    Out = FKyaMickenOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 40) return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Waypoint = Start + 20;
    Out.Actor = Start + 24;
    Out.Field358 = Start + 28;
    Out.Field35C = Start + 32;
    Out.Field360 = Start + 36;
    Out.Tail = Start + 40;
    return true;
}

bool KyaReadMickenActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaMickenOffsets Off;
    if (!KyaReadMickenOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.MickenWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.MickenLifeMax = int64(KyaReadU32(Bytes + Off.Autonomous));
    Out.MickenGravityScale = KyaReadF32(Bytes + Off.Autonomous + 4);
    Out.MickenDynamicExtField4 = KyaReadF32(Bytes + Off.Autonomous + 8);
    Out.MickenDynamicExtField8 = KyaReadF32(Bytes + Off.Autonomous + 12);
    Out.MickenWaypointIndex = int32(KyaReadU32(Bytes + Off.Waypoint));
    Out.MickenActorIndex = int32(KyaReadU32(Bytes + Off.Actor));
    Out.MickenField358 = int32(KyaReadU32(Bytes + Off.Field358));
    Out.MickenField35C = KyaReadF32(Bytes + Off.Field35C);
    Out.MickenField360 = int32(KyaReadU32(Bytes + Off.Field360));

    Out.bHasMickenData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteMickenActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaMickenOffsets Off;
    if (!KyaReadMickenOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.MickenLifeMax < 0 || Actor.MickenLifeMax > MAX_uint32) return false;
    KyaWriteF32(Data, Off.WeightB, Actor.MickenWeightB);
    KyaWriteU32(Data, Off.Autonomous, uint32(uint64(Actor.MickenLifeMax)));
    KyaWriteF32(Data, Off.Autonomous + 4, Actor.MickenGravityScale);
    KyaWriteF32(Data, Off.Autonomous + 8, Actor.MickenDynamicExtField4);
    KyaWriteF32(Data, Off.Autonomous + 12, Actor.MickenDynamicExtField8);
    KyaWriteU32(Data, Off.Waypoint, uint32(Actor.MickenWaypointIndex));
    KyaWriteU32(Data, Off.Actor, uint32(Actor.MickenActorIndex));
    KyaWriteU32(Data, Off.Field358, uint32(Actor.MickenField358));
    KyaWriteF32(Data, Off.Field35C, Actor.MickenField35C);
    KyaWriteU32(Data, Off.Field360, uint32(Actor.MickenField360));
    return true;
}

bool KyaReadMickenBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaMickenBehaviourValues& Out)
{
    Out = FKyaMickenBehaviourValues{};
    if (Id != 4 || Start < 0 || Size != 16 || !KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    Out.Field8 = KyaReadF32(Bytes + Start);
    Out.FieldC = KyaReadF32(Bytes + Start + 4);
    Out.Field10 = int32(KyaReadU32(Bytes + Start + 8));
    Out.PathIndex = int32(KyaReadU32(Bytes + Start + 12));
    return true;
}

bool KyaReadMickenBehaviourValue(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    FKyaMickenBehaviourValues Values;
    if (!KyaReadMickenBehaviour(Id, Data, Start, Size, Values)) return false;
    Out.MickenEatField8 = Values.Field8;
    Out.MickenEatFieldC = Values.FieldC;
    Out.MickenEatField10 = Values.Field10;
    Out.MickenEatPathIndex = Values.PathIndex;
    Out.bHasTypedMickenBehaviour = true;
    return true;
}

bool KyaWriteMickenBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    FKyaMickenBehaviourValues Values;
    if (!KyaReadMickenBehaviour(Behaviour.Id, MakeArrayView(Data.GetData(), Data.Num()), Start,
        Size, Values)) return false;
    KyaWriteF32(Data, Start, Behaviour.MickenEatField8);
    KyaWriteF32(Data, Start + 4, Behaviour.MickenEatFieldC);
    KyaWriteU32(Data, Start + 8, uint32(Behaviour.MickenEatField10));
    KyaWriteU32(Data, Start + 12, uint32(Behaviour.MickenEatPathIndex));
    return true;
}
