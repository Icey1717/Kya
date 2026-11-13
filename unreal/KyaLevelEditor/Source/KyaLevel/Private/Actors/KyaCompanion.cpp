#include "Actors/KyaCompanion.h"

#include "Actors/KyaSwitch.h"
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

bool KyaReadCompanionOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaCompanionOffsets& Out)
{
    Out = FKyaCompanionOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 32) return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Field358 = Start + 20;
    Out.Tail = Start + 32;
    return true;
}

bool KyaReadCompanionActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaCompanionOffsets Off;
    if (!KyaReadCompanionOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.CompanionWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.CompanionLifeMax = int64(KyaReadU32(Bytes + Off.Autonomous));
    Out.CompanionGravityScale = KyaReadF32(Bytes + Off.Autonomous + 4);
    Out.CompanionDynamicExtField4 = KyaReadF32(Bytes + Off.Autonomous + 8);
    Out.CompanionDynamicExtField8 = KyaReadF32(Bytes + Off.Autonomous + 12);
    Out.CompanionField358 = int32(KyaReadU32(Bytes + Off.Field358));
    Out.CompanionMaterial35C = int32(KyaReadU32(Bytes + Off.Field358 + 4));
    Out.CompanionField360 = int32(KyaReadU32(Bytes + Off.Field358 + 8));

    Out.bHasCompanionData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteCompanionActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaCompanionOffsets Off;
    if (!KyaReadCompanionOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.CompanionLifeMax < 0 || Actor.CompanionLifeMax > MAX_uint32) return false;
    KyaWriteF32(Data, Off.WeightB, Actor.CompanionWeightB);
    KyaWriteU32(Data, Off.Autonomous, uint32(uint64(Actor.CompanionLifeMax)));
    KyaWriteF32(Data, Off.Autonomous + 4, Actor.CompanionGravityScale);
    KyaWriteF32(Data, Off.Autonomous + 8, Actor.CompanionDynamicExtField4);
    KyaWriteF32(Data, Off.Autonomous + 12, Actor.CompanionDynamicExtField8);
    KyaWriteU32(Data, Off.Field358, uint32(Actor.CompanionField358));
    KyaWriteU32(Data, Off.Field358 + 4, uint32(Actor.CompanionMaterial35C));
    KyaWriteU32(Data, Off.Field358 + 8, uint32(Actor.CompanionField360));
    return true;
}

bool KyaReadCompanionBehaviourOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaCompanionBehaviourOffsets& Out)
{
    Out = FKyaCompanionBehaviourOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 4) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Count = Start;
    Out.Entries = Start + 4;
    const uint32 Count = KyaReadU32(Bytes + Start);
    Out.CountValue = int32(Count);
    int64 Pos = Start + 4;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        FKyaCompanionAlertOffsets Alert;
        if (!KyaReadScenarioConditionOffsets(Data, Pos, End, Alert.Conditions)) return false;
        if (End - Pos < 20) return false;
        Alert.Flags = Pos;
        Alert.Field08 = Pos + 4;
        Alert.Field10 = Pos + 8;
        Alert.Field14 = Pos + 12;
        Alert.Field18 = Pos + 16;
        Pos += 20;
        if (End - Pos < 4) return false;
        Alert.ActorRef = Pos;
        Pos += 4;
        if (End - Pos < 12) return false;
        Alert.Vector20 = Pos;
        Pos += 12;
        if (End - Pos < 4) return false;
        Alert.CameraRef = Pos;
        Pos += 4;
        if (End - Pos < 4) return false;
        Alert.ListCount = Pos;
        const uint32 Items = KyaReadU32(Bytes + Pos);
        Alert.ListCountValue = int32(Items);
        Pos += 4;
        if (uint64(Items) > uint64(End - Pos) / 4) return false;
        Alert.ListEntries = Pos;
        Pos += int64(Items) * 4;
        if (End - Pos < 20) return false;
        Alert.LevelStringKey = Pos;
        Alert.Field50 = Pos + 8;
        Alert.Field58 = Pos + 12;
        Alert.Field5C = Pos + 16;
        Pos += 20;
        if (End - Pos < 4) return false;
        const uint32 SwitchTargets = KyaReadU32(Bytes + Pos);
        Alert.Switch.Count = int32(SwitchTargets);
        Alert.Switch.Targets = Pos + 4;
        if (SwitchTargets > uint64(End - Pos - 4) / 28) return false;
        Alert.Switch.Camera = Alert.Switch.Targets + int64(SwitchTargets) * 28;
        if (End - Alert.Switch.Camera < 32) return false;
        Pos = Alert.Switch.Camera + 32;
        Alert.Conditioned = Pos;
        if (!KyaReadConditionedOffsets(Data, Pos, Alert.Blocks, Pos)) return false;
        if (Pos > End) return false;
        Alert.Tail = Pos;
        Out.Alerts.Add(MoveTemp(Alert));
    }
    Out.Tail = Pos;
    return true;
}

bool KyaReadCompanionBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Id != 3) return false;
    FKyaCompanionBehaviourOffsets Off;
    if (!KyaReadCompanionBehaviourOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.CompanionAlerts.Reset();
    for (const FKyaCompanionAlertOffsets& AlertOffsets : Off.Alerts)
    {
        FKyaCompanionAlert& Alert = Out.CompanionAlerts.AddDefaulted_GetRef();
        for (int32 Index = 0; Index < AlertOffsets.Conditions.CountValue; ++Index)
        {
            FKyaScenarioCondition& Condition = Alert.Conditions.AddDefaulted_GetRef();
            const int64 C = AlertOffsets.Conditions.Entries + int64(Index) * 16;
            Condition.Combiner = int32(KyaReadU32(Bytes + C));
            Condition.Variable = int32(KyaReadU32(Bytes + C + 4));
            Condition.Comparison = int32(KyaReadU32(Bytes + C + 8));
            Condition.Value = int32(KyaReadU32(Bytes + C + 12));
        }
        Alert.Flags = int32(KyaReadU32(Bytes + AlertOffsets.Flags));
        Alert.Field08 = int32(KyaReadU32(Bytes + AlertOffsets.Field08));
        Alert.Field10 = int32(KyaReadU32(Bytes + AlertOffsets.Field10));
        Alert.Field14 = int32(KyaReadU32(Bytes + AlertOffsets.Field14));
        Alert.Field18 = KyaReadF32(Bytes + AlertOffsets.Field18);
        Alert.ActorRef = int32(KyaReadU32(Bytes + AlertOffsets.ActorRef));
        Alert.Field20 = FVector(KyaReadF32(Bytes + AlertOffsets.Vector20),
            KyaReadF32(Bytes + AlertOffsets.Vector20 + 4),
            KyaReadF32(Bytes + AlertOffsets.Vector20 + 8));
        Alert.CameraRef = int32(KyaReadU32(Bytes + AlertOffsets.CameraRef));
        for (int32 Index = 0; Index < AlertOffsets.ListCountValue; ++Index)
            Alert.Field34.Add(int32(KyaReadU32(Bytes + AlertOffsets.ListEntries + int64(Index) * 4)));
        Alert.LevelStringKey = int64(uint64(KyaReadU32(Bytes + AlertOffsets.LevelStringKey)) |
            (uint64(KyaReadU32(Bytes + AlertOffsets.LevelStringKey + 4)) << 32));
        Alert.Field50 = KyaReadF32(Bytes + AlertOffsets.Field50);
        Alert.Field58 = int32(KyaReadU32(Bytes + AlertOffsets.Field58));
        Alert.Field5C = KyaReadF32(Bytes + AlertOffsets.Field5C);
        Alert.TargetSwitch = FKyaSwitch{};
        if (!KyaReadSwitchValue(Data, AlertOffsets.Switch.Targets, AlertOffsets.Switch.Count,
            AlertOffsets.Switch.Camera, Alert.TargetSwitch)) return false;
        for (const FKyaConditionedOffsets& Block : AlertOffsets.Blocks)
        {
            FKyaConditionedOperation& Result = Alert.Field70.AddDefaulted_GetRef();
            for (int64 Offset : Block.Operations)
            {
                FKyaScenarioOperation& Op = Result.Operations.AddDefaulted_GetRef();
                Op.Variable = int32(KyaReadU32(Bytes + Offset));
                Op.Type = int32(KyaReadU32(Bytes + Offset + 4));
                Op.Value = int32(KyaReadU32(Bytes + Offset + 8));
            }
            for (int64 Offset : Block.Conditions)
            {
                FKyaScenarioCondition& Condition = Result.Conditions.AddDefaulted_GetRef();
                Condition.Combiner = int32(KyaReadU32(Bytes + Offset));
                Condition.Variable = int32(KyaReadU32(Bytes + Offset + 4));
                Condition.Comparison = int32(KyaReadU32(Bytes + Offset + 8));
                Condition.Value = int32(KyaReadU32(Bytes + Offset + 12));
            }
        }
    }
    Out.bHasTypedCompanionBehaviour = true;
    return true;
}

bool KyaWriteCompanionBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id != 3) return false;
    FKyaCompanionBehaviourOffsets Off;
    if (!KyaReadCompanionBehaviourOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start,
        Size, Off)) return false;
    if (Behaviour.CompanionAlerts.Num() != Off.Alerts.Num()) return false;
    for (int32 AlertIndex = 0; AlertIndex < Behaviour.CompanionAlerts.Num(); ++AlertIndex)
    {
        const FKyaCompanionAlert& Alert = Behaviour.CompanionAlerts[AlertIndex];
        const FKyaCompanionAlertOffsets& AlertOffsets = Off.Alerts[AlertIndex];
        if (Alert.Conditions.Num() != AlertOffsets.Conditions.CountValue ||
            Alert.Field34.Num() != AlertOffsets.ListCountValue ||
            Alert.TargetSwitch.Targets.Num() != AlertOffsets.Switch.Count ||
            Alert.Field70.Num() != AlertOffsets.Blocks.Num()) return false;
        for (int32 Block = 0; Block < Alert.Field70.Num(); ++Block)
            if (Alert.Field70[Block].Operations.Num() != AlertOffsets.Blocks[Block].Operations.Num() ||
                Alert.Field70[Block].Conditions.Num() != AlertOffsets.Blocks[Block].Conditions.Num())
                return false;

        for (int32 Item = 0; Item < Alert.Conditions.Num(); ++Item)
        {
            const FKyaScenarioCondition& Condition = Alert.Conditions[Item];
            const int64 C = AlertOffsets.Conditions.Entries + int64(Item) * 16;
            KyaWriteU32(Data, C, uint32(Condition.Combiner));
            KyaWriteU32(Data, C + 4, uint32(Condition.Variable));
            KyaWriteU32(Data, C + 8, uint32(Condition.Comparison));
            KyaWriteU32(Data, C + 12, uint32(Condition.Value));
        }
        KyaWriteU32(Data, AlertOffsets.Flags, uint32(Alert.Flags));
        KyaWriteU32(Data, AlertOffsets.Field08, uint32(Alert.Field08));
        KyaWriteU32(Data, AlertOffsets.Field10, uint32(Alert.Field10));
        KyaWriteU32(Data, AlertOffsets.Field14, uint32(Alert.Field14));
        KyaWriteF32(Data, AlertOffsets.Field18, Alert.Field18);
        KyaWriteU32(Data, AlertOffsets.ActorRef, uint32(Alert.ActorRef));
        for (int32 Axis = 0; Axis < 3; ++Axis)
            KyaWriteF32(Data, AlertOffsets.Vector20 + Axis * 4, float(Alert.Field20[Axis]));
        KyaWriteU32(Data, AlertOffsets.CameraRef, uint32(Alert.CameraRef));
        for (int32 Item = 0; Item < Alert.Field34.Num(); ++Item)
            KyaWriteU32(Data, AlertOffsets.ListEntries + int64(Item) * 4, uint32(Alert.Field34[Item]));
        KyaWriteU32(Data, AlertOffsets.LevelStringKey, uint32(uint64(Alert.LevelStringKey)));
        KyaWriteU32(Data, AlertOffsets.LevelStringKey + 4, uint32(uint64(Alert.LevelStringKey) >> 32));
        KyaWriteF32(Data, AlertOffsets.Field50, Alert.Field50);
        KyaWriteU32(Data, AlertOffsets.Field58, uint32(Alert.Field58));
        KyaWriteF32(Data, AlertOffsets.Field5C, Alert.Field5C);
        KyaWriteSwitchValue(Data, Alert.TargetSwitch, AlertOffsets.Switch.Targets,
            AlertOffsets.Switch.Camera);
        for (int32 Block = 0; Block < Alert.Field70.Num(); ++Block)
        {
            const FKyaConditionedOperation& Result = Alert.Field70[Block];
            const FKyaConditionedOffsets& BlockOffsets = AlertOffsets.Blocks[Block];
            for (int32 Item = 0; Item < Result.Operations.Num(); ++Item)
            {
                const FKyaScenarioOperation& Op = Result.Operations[Item];
                const int64 O = BlockOffsets.Operations[Item];
                KyaWriteU32(Data, O, uint32(Op.Variable));
                KyaWriteU32(Data, O + 4, uint32(Op.Type));
                KyaWriteU32(Data, O + 8, uint32(Op.Value));
            }
            for (int32 Item = 0; Item < Result.Conditions.Num(); ++Item)
            {
                const FKyaScenarioCondition& Condition = Result.Conditions[Item];
                const int64 C = BlockOffsets.Conditions[Item];
                KyaWriteU32(Data, C, uint32(Condition.Combiner));
                KyaWriteU32(Data, C + 4, uint32(Condition.Variable));
                KyaWriteU32(Data, C + 8, uint32(Condition.Comparison));
                KyaWriteU32(Data, C + 12, uint32(Condition.Value));
            }
        }
    }
    return true;
}
