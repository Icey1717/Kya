#include "Actors/KyaProjectile.h"

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

bool KyaRangeValid(const TArray<uint8>& Data, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 0 && Start <= Data.Num() && Size <= Data.Num() - Start;
}
}

bool KyaReadProjectileOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaProjectileOffsets& Out)
{
    Out = FKyaProjectileOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    // A valid record consumes exactly one of the two layout sizes; anything else
    // stays raw rather than being guessed from a trailing byte.
    if (Size == 136)
    {
        Out.bHasPatternField24 = true;
        Out.Tail = Start + 136;
        Out.PatternField24 = Start + 128;
    }
    else if (Size == 132) Out.Tail = Start + 132;
    else return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.SubObj = Start + 20;
    Out.Pattern = Start + 92;
    return true;
}

bool KyaReadProjectileActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaProjectileOffsets Offsets;
    if (!KyaReadProjectileOffsets(Data, Start, Size, Offsets)) return false;
    const uint8* Bytes = Data.GetData();

    Out.bHasProjectileData = true;
    Out.ProjectileWeightB = KyaReadF32(Bytes + Offsets.WeightB);
    Out.ProjectileLifeMax = int64(KyaReadU32(Bytes + Offsets.Autonomous));
    Out.ProjectileGravityScale = KyaReadF32(Bytes + Offsets.Autonomous + 4);
    Out.ProjectileDynamicExtField4 = KyaReadF32(Bytes + Offsets.Autonomous + 8);
    Out.ProjectileDynamicExtField8 = KyaReadF32(Bytes + Offsets.Autonomous + 12);

    FKyaProjectileSubObj& Sub = Out.ProjectileSubObj;
    const int64 S = Offsets.SubObj;
    Sub.TimeToExplode = KyaReadF32(Bytes + S);
    Sub.ExplosionRange = KyaReadF32(Bytes + S + 4);
    Sub.AnimDuration = KyaReadF32(Bytes + S + 8);
    Sub.Flags = int32(KyaReadU32(Bytes + S + 12));
    Sub.Damage = KyaReadF32(Bytes + S + 16);
    Sub.HitRadius = KyaReadF32(Bytes + S + 20);
    Sub.WarnRadius = KyaReadF32(Bytes + S + 24);
    Sub.Field1C = KyaReadF32(Bytes + S + 28);
    Sub.Field20 = KyaReadF32(Bytes + S + 32);
    Sub.Field24 = KyaReadF32(Bytes + S + 36);
    Sub.Field28 = int32(KyaReadU32(Bytes + S + 40));
    Sub.MaterialId = int32(KyaReadU32(Bytes + S + 44));
    Sub.Field30 = int32(KyaReadU32(Bytes + S + 48));
    Sub.Field34 = int32(KyaReadU32(Bytes + S + 52));
    Sub.Field38 = int32(KyaReadU32(Bytes + S + 56));
    Sub.Field3C = int32(KyaReadU32(Bytes + S + 60));
    Sub.Field40 = KyaReadF32(Bytes + S + 64);
    Sub.Field44 = KyaReadF32(Bytes + S + 68);

    FKyaPatternPart& Pattern = Out.ProjectilePattern;
    const int64 P = Offsets.Pattern;
    Pattern.Field4 = KyaReadF32(Bytes + P);
    Pattern.Field0 = KyaReadF32(Bytes + P + 4);
    Pattern.FieldC = KyaReadF32(Bytes + P + 8);
    Pattern.Field8 = KyaReadF32(Bytes + P + 12);
    Pattern.Field10 = KyaReadF32(Bytes + P + 16);
    Pattern.Field14 = KyaReadF32(Bytes + P + 20);
    Pattern.MaterialId = int32(KyaReadU32(Bytes + P + 24));
    Pattern.PointCount = KyaReadF32(Bytes + P + 28);
    Pattern.Field20 = KyaReadF32(Bytes + P + 32);
    Pattern.bHasField24 = Offsets.bHasPatternField24;
    if (Offsets.bHasPatternField24) Pattern.Field24 = KyaReadF32(Bytes + P + 36);
    Pattern.Field28 = int32(KyaReadU32(Bytes + P + (Offsets.bHasPatternField24 ? 40 : 36)));

    const int64 TailBytes = Start + Size - Offsets.Tail;
    Out.ClassData.Reset();
    if (TailBytes > 0) Out.ClassData.Append(Bytes + Offsets.Tail, int32(TailBytes));
    return true;
}

bool KyaWriteProjectileActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaProjectileOffsets Offsets;
    if (!KyaReadProjectileOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Offsets))
        return false;
    if (Actor.ProjectileLifeMax < 0 || Actor.ProjectileLifeMax > MAX_uint32) return false;
    if (Actor.ProjectilePattern.bHasField24 != Offsets.bHasPatternField24) return false;

    KyaWriteF32(Data, Offsets.WeightB, Actor.ProjectileWeightB);
    KyaWriteU32(Data, Offsets.Autonomous, uint32(uint64(Actor.ProjectileLifeMax)));
    KyaWriteF32(Data, Offsets.Autonomous + 4, Actor.ProjectileGravityScale);
    KyaWriteF32(Data, Offsets.Autonomous + 8, Actor.ProjectileDynamicExtField4);
    KyaWriteF32(Data, Offsets.Autonomous + 12, Actor.ProjectileDynamicExtField8);

    const FKyaProjectileSubObj& Sub = Actor.ProjectileSubObj;
    const int64 S = Offsets.SubObj;
    KyaWriteF32(Data, S, Sub.TimeToExplode);
    KyaWriteF32(Data, S + 4, Sub.ExplosionRange);
    KyaWriteF32(Data, S + 8, Sub.AnimDuration);
    KyaWriteU32(Data, S + 12, uint32(Sub.Flags));
    KyaWriteF32(Data, S + 16, Sub.Damage);
    KyaWriteF32(Data, S + 20, Sub.HitRadius);
    KyaWriteF32(Data, S + 24, Sub.WarnRadius);
    KyaWriteF32(Data, S + 28, Sub.Field1C);
    KyaWriteF32(Data, S + 32, Sub.Field20);
    KyaWriteF32(Data, S + 36, Sub.Field24);
    KyaWriteU32(Data, S + 40, uint32(Sub.Field28));
    KyaWriteU32(Data, S + 44, uint32(Sub.MaterialId));
    KyaWriteU32(Data, S + 48, uint32(Sub.Field30));
    KyaWriteU32(Data, S + 52, uint32(Sub.Field34));
    KyaWriteU32(Data, S + 56, uint32(Sub.Field38));
    KyaWriteU32(Data, S + 60, uint32(Sub.Field3C));
    KyaWriteF32(Data, S + 64, Sub.Field40);
    KyaWriteF32(Data, S + 68, Sub.Field44);

    const FKyaPatternPart& Pattern = Actor.ProjectilePattern;
    const int64 P = Offsets.Pattern;
    KyaWriteF32(Data, P, Pattern.Field4);
    KyaWriteF32(Data, P + 4, Pattern.Field0);
    KyaWriteF32(Data, P + 8, Pattern.FieldC);
    KyaWriteF32(Data, P + 12, Pattern.Field8);
    KyaWriteF32(Data, P + 16, Pattern.Field10);
    KyaWriteF32(Data, P + 20, Pattern.Field14);
    KyaWriteU32(Data, P + 24, uint32(Pattern.MaterialId));
    KyaWriteF32(Data, P + 28, Pattern.PointCount);
    KyaWriteF32(Data, P + 32, Pattern.Field20);
    if (Offsets.bHasPatternField24) KyaWriteF32(Data, P + 36, Pattern.Field24);
    KyaWriteU32(Data, P + (Offsets.bHasPatternField24 ? 40 : 36), uint32(Pattern.Field28));
    return true;
}

bool KyaReadProjectileBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int32 Size, FKyaProjectileBehaviourOffsets& Out)
{
    Out = FKyaProjectileBehaviourOffsets{};
    if (!KyaRangeValid(Data, Start, int64(Size))) return false;
    Out.BehaviourId = Start;
    Out.Tail = Start + Size;

    if (Id == 13)
    {
        // CBehaviourProjectileNew::Create only calls CBehaviourProjectile::Create.
        if (Size != 4) return false;
        Out.bNew = true;
        return true;
    }
    if (Size < 12) return false;

    if (Id == 3)
    {
        // CBehaviourProjectileStand::Create: behaviourId, field_c, materialId.
        if (Size != 12) return false;
        Out.bStand = true;
        Out.Field0C = Start + 4;
        Out.MaterialId = Start + 8;
        return true;
    }
    if (Id == 4)
    {
        // CBehaviourProjectileStraight::Create: behaviourId, field_c, speed,
        // acceleration, field_0x18.
        if (Size != 20) return false;
        Out.bStraight = true;
        Out.Field0C = Start + 4;
        Out.Speed = Start + 8;
        Out.Acceleration = Start + 12;
        Out.Field18 = Start + 16;
        return true;
    }
    if (Id == 5)
    {
        // CBehaviourProjectilePortable::Create: behaviourId, field_c, field_0x10.
        if (Size != 12) return false;
        Out.bPortable = true;
        Out.Field0C = Start + 4;
        Out.Field10 = Start + 8;
        return true;
    }
    if (Id == 10)
    {
        // CBehaviourProjectileLavaBall::Create. field_0x18 is present only on the
        // newer scene version, so the payload size selects the layout.
        if (Size != 40 && Size != 44) return false;
        Out.bLavaBall = true;
        Out.Field0C = Start + 4;
        Out.Field10 = Start + 8;
        Out.LavaBallField14 = Start + 12;
        Out.LavaBallField30 = Start + 16;
        int64 Pos = Start + 20;
        if (Size == 44)
        {
            Out.bHasLavaBallField18 = true;
            Out.LavaBallField18 = Pos;
            Pos += 4;
        }
        Out.LavaBallPathIndex = Pos;
        Out.LavaBallField2C = Pos + 4;
        Out.LavaBallField34 = Pos + 8;
        Out.LavaBallField38 = Pos + 12;
        Out.LavaBallField3C = Pos + 16;
        return true;
    }
    if (Id == 11)
    {
        // CBehaviourProjectileExcuse::Create: behaviourId, field_c, actorRef.
        if (Size != 12) return false;
        Out.bExcuse = true;
        Out.Field0C = Start + 4;
        Out.ActorRef = Start + 8;
        return true;
    }
    if (Id == 12)
    {
        // CBehaviourInactive::Create: activateMsgId, flags, activeBehaviourId.
        if (Size != 12) return false;
        Out.bInactive = true;
        Out.InactiveActivateMessageId = Start;
        Out.InactiveFlags = Start + 4;
        Out.InactiveActiveBehaviourId = Start + 8;
        return true;
    }
    return false;
}

bool KyaReadProjectileBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    FKyaProjectileBehaviourOffsets Offsets;
    if (!KyaReadProjectileBehaviourOffsets(Id, Data, Start, Size, Offsets)) return false;
    const uint8* Bytes = Data.GetData();
    Out.ProjectileBehaviour = FKyaProjectileBehaviour{};
    FKyaProjectileBehaviour& Values = Out.ProjectileBehaviour;

    if (Offsets.bInactive)
    {
        Values.InactiveActivateMessageId = int32(KyaReadU32(Bytes + Offsets.InactiveActivateMessageId));
        Values.InactiveFlags = int32(KyaReadU32(Bytes + Offsets.InactiveFlags));
        Values.InactiveActiveBehaviourId = int32(KyaReadU32(Bytes + Offsets.InactiveActiveBehaviourId));
    }
    else
    {
        Values.BehaviourId = int32(KyaReadU32(Bytes + Offsets.BehaviourId));
        if (!Offsets.bNew) Values.Field0C = int32(KyaReadU32(Bytes + Offsets.Field0C));
        if (Offsets.bStand) Values.MaterialId = int32(KyaReadU32(Bytes + Offsets.MaterialId));
        if (Offsets.bStraight)
        {
            Values.Speed = KyaReadF32(Bytes + Offsets.Speed);
            Values.Acceleration = KyaReadF32(Bytes + Offsets.Acceleration);
            Values.Field18 = KyaReadF32(Bytes + Offsets.Field18);
        }
        if (Offsets.bPortable) Values.Field10 = KyaReadF32(Bytes + Offsets.Field10);
        if (Offsets.bExcuse) Values.ActorRef = int32(KyaReadU32(Bytes + Offsets.ActorRef));
        if (Offsets.bLavaBall)
        {
            Values.Field10 = KyaReadF32(Bytes + Offsets.Field10);
            Values.LavaBallField14 = KyaReadF32(Bytes + Offsets.LavaBallField14);
            Values.LavaBallField30 = int32(KyaReadU32(Bytes + Offsets.LavaBallField30));
            Values.bHasLavaBallField18 = Offsets.bHasLavaBallField18;
            if (Offsets.bHasLavaBallField18)
                Values.LavaBallField18 = KyaReadF32(Bytes + Offsets.LavaBallField18);
            Values.LavaBallPathIndex = int32(KyaReadU32(Bytes + Offsets.LavaBallPathIndex));
            Values.LavaBallField2C = int32(KyaReadU32(Bytes + Offsets.LavaBallField2C));
            Values.LavaBallField34 = int32(KyaReadU32(Bytes + Offsets.LavaBallField34));
            Values.LavaBallField38 = int32(KyaReadU32(Bytes + Offsets.LavaBallField38));
            Values.LavaBallField3C = int32(KyaReadU32(Bytes + Offsets.LavaBallField3C));
        }
    }
    Out.bHasTypedProjectileBehaviour = true;
    return true;
}

bool KyaWriteProjectileBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    FKyaProjectileBehaviourOffsets Offsets;
    if (!KyaReadProjectileBehaviourOffsets(Behaviour.Id,
        MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Offsets)) return false;
    const FKyaProjectileBehaviour& Values = Behaviour.ProjectileBehaviour;

    if (Offsets.bInactive)
    {
        KyaWriteU32(Data, Offsets.InactiveActivateMessageId, uint32(Values.InactiveActivateMessageId));
        KyaWriteU32(Data, Offsets.InactiveFlags, uint32(Values.InactiveFlags));
        KyaWriteU32(Data, Offsets.InactiveActiveBehaviourId, uint32(Values.InactiveActiveBehaviourId));
        return true;
    }

    KyaWriteU32(Data, Offsets.BehaviourId, uint32(Values.BehaviourId));
    if (!Offsets.bNew) KyaWriteU32(Data, Offsets.Field0C, uint32(Values.Field0C));
    if (Offsets.bStand) KyaWriteU32(Data, Offsets.MaterialId, uint32(Values.MaterialId));
    if (Offsets.bStraight)
    {
        KyaWriteF32(Data, Offsets.Speed, Values.Speed);
        KyaWriteF32(Data, Offsets.Acceleration, Values.Acceleration);
        KyaWriteF32(Data, Offsets.Field18, Values.Field18);
    }
    if (Offsets.bPortable) KyaWriteF32(Data, Offsets.Field10, Values.Field10);
    if (Offsets.bExcuse) KyaWriteU32(Data, Offsets.ActorRef, uint32(Values.ActorRef));
    if (Offsets.bLavaBall)
    {
        KyaWriteF32(Data, Offsets.Field10, Values.Field10);
        KyaWriteF32(Data, Offsets.LavaBallField14, Values.LavaBallField14);
        KyaWriteU32(Data, Offsets.LavaBallField30, uint32(Values.LavaBallField30));
        if (Offsets.bHasLavaBallField18)
            KyaWriteF32(Data, Offsets.LavaBallField18, Values.LavaBallField18);
        KyaWriteU32(Data, Offsets.LavaBallPathIndex, uint32(Values.LavaBallPathIndex));
        KyaWriteU32(Data, Offsets.LavaBallField2C, uint32(Values.LavaBallField2C));
        KyaWriteU32(Data, Offsets.LavaBallField34, uint32(Values.LavaBallField34));
        KyaWriteU32(Data, Offsets.LavaBallField38, uint32(Values.LavaBallField38));
        KyaWriteU32(Data, Offsets.LavaBallField3C, uint32(Values.LavaBallField3C));
    }
    return true;
}

bool KyaReadProjectileInventoryOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaProjectileInventoryOffsets& Out)
{
    Out = FKyaProjectileInventoryOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 8) return false;
    const uint8* Bytes = Data.GetData();
    int64 Pos = Start;
    Out.Field0 = Pos;
    const uint32 Present = KyaReadU32(Bytes + Pos);
    Pos += 4;
    // Before scene version 2.27 CInventoryInfo has no object-present word.
    // The trailing initial-behaviour word makes that exact layout 40 bytes.
    const bool bLegacy = Present != 0 && Size == 40;
    const int64 ExpectedBase = Present == 0 ? 4 : (bLegacy ? 36 : 40);
    if (Size < ExpectedBase + 4) return false;
    if (Present != 0)
    {
        Out.Field8 = Pos; Pos += 8;
        Out.Field10 = Pos; Pos += 8;
        Out.PurchaseId = Pos; Pos += 4;
        Out.Field28 = Pos; Pos += 4;
        Out.MoneyCost = Pos; Pos += 4;
        Out.Field2C = Pos; Pos += 4;
        if (!bLegacy)
        {
            Out.ObjectFlag = Pos;
            Out.bHasObject = KyaReadU32(Bytes + Pos) != 0;
            Pos += 4;
        }
        if (Out.bHasObject)
        {
            Out.ObjectId = Pos; Pos += 4;
            Out.Field18 = Pos; Pos += 8;
        }
    }
    Out.InitialBehaviourId = Pos;
    Pos += 4;
    if (Pos != Start + Size) return false;
    Out.Tail = Pos;
    return true;
}

bool KyaReadProjectileInventory(FKyaInventoryInfo& Out, TConstArrayView<uint8> Data, int64 Start,
    int32 Size)
{
    FKyaProjectileInventoryOffsets Offsets;
    if (!KyaReadProjectileInventoryOffsets(Data, Start, int64(Size), Offsets)) return false;
    const uint8* Bytes = Data.GetData();
    Out = FKyaInventoryInfo{};
    Out.Field0 = int32(KyaReadU32(Bytes + Offsets.Field0));
    Out.bHasObjectPayload = Offsets.bHasObject;
    if (Offsets.Field8 != 0)
    {
        Out.Field8 = int64(uint64(KyaReadU32(Bytes + Offsets.Field8)) |
            (uint64(KyaReadU32(Bytes + Offsets.Field8 + 4)) << 32));
        Out.Field10 = int64(uint64(KyaReadU32(Bytes + Offsets.Field10)) |
            (uint64(KyaReadU32(Bytes + Offsets.Field10 + 4)) << 32));
        Out.PurchaseId = int32(KyaReadU32(Bytes + Offsets.PurchaseId));
        Out.Field28 = int32(KyaReadU32(Bytes + Offsets.Field28));
        Out.MoneyCost = int32(KyaReadU32(Bytes + Offsets.MoneyCost));
        Out.Field2C = int32(KyaReadU32(Bytes + Offsets.Field2C));
    }
    if (Offsets.bHasObject)
    {
        Out.ObjectId = int32(KyaReadU32(Bytes + Offsets.ObjectId));
        Out.Field18 = int64(uint64(KyaReadU32(Bytes + Offsets.Field18)) |
            (uint64(KyaReadU32(Bytes + Offsets.Field18 + 4)) << 32));
    }
    else if (Offsets.Field8 != 0) Out.ObjectId = 0x20;
    Out.InitialBehaviourId = int32(KyaReadU32(Bytes + Offsets.InitialBehaviourId));
    return true;
}

bool KyaWriteProjectileInventory(const FKyaInventoryInfo& Inventory, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    FKyaProjectileInventoryOffsets Offsets;
    if (!KyaReadProjectileInventoryOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start,
        int64(Size), Offsets)) return false;
    // The first word selects the payload shape. Editing it between zero and
    // non-zero would make the game read past the serialized block.
    const uint32 OriginalPresent = KyaReadU32(Data.GetData() + Offsets.Field0);
    if ((OriginalPresent == 0) != (Inventory.Field0 == 0)) return false;
    KyaWriteU32(Data, Offsets.Field0, uint32(Inventory.Field0));
    if (Offsets.Field8 != 0)
    {
        KyaWriteU32(Data, Offsets.Field8, uint32(uint64(Inventory.Field8)));
        KyaWriteU32(Data, Offsets.Field8 + 4, uint32(uint64(Inventory.Field8) >> 32));
        KyaWriteU32(Data, Offsets.Field10, uint32(uint64(Inventory.Field10)));
        KyaWriteU32(Data, Offsets.Field10 + 4, uint32(uint64(Inventory.Field10) >> 32));
        KyaWriteU32(Data, Offsets.PurchaseId, uint32(Inventory.PurchaseId));
        KyaWriteU32(Data, Offsets.Field28, uint32(Inventory.Field28));
        KyaWriteU32(Data, Offsets.MoneyCost, uint32(Inventory.MoneyCost));
        KyaWriteU32(Data, Offsets.Field2C, uint32(Inventory.Field2C));
    }
    if (Offsets.bHasObject)
    {
        KyaWriteU32(Data, Offsets.ObjectId, uint32(Inventory.ObjectId));
        KyaWriteU32(Data, Offsets.Field18, uint32(uint64(Inventory.Field18)));
        KyaWriteU32(Data, Offsets.Field18 + 4, uint32(uint64(Inventory.Field18) >> 32));
    }
    KyaWriteU32(Data, Offsets.InitialBehaviourId, uint32(Inventory.InitialBehaviourId));
    return true;
}
