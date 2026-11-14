#include "Actors/KyaAton.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadAtonOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAtonOffsets& Out)
{
    Out = FKyaAtonOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 76) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Fields354 = Start + 20;
    Out.EffectType = Start + 36;
    Out.Texture = Start + 40;
    Out.Mesh = Start + 44;
    Out.Trail = Start + 48;
    Out.PathDelay = Start + 52;
    Out.PathCount = Start + 56;
    const uint32 Paths = KyaReadU32(Bytes + Start + 56);
    Out.PathCountValue = int32(Paths);
    Out.PathEntries = Start + 60;
    int64 Pos = Start + 60;
    if (uint64(Paths) > uint64(End - Pos) / 4) return false;
    Pos += int64(Paths) * 4;
    if (End - Pos < 8) return false;
    Out.AddOn = Pos;
    Out.AddOnField18 = Pos;
    Out.OtherCount = Pos + 4;
    const uint32 Others = KyaReadU32(Bytes + Pos + 4);
    Out.OtherCountValue = int32(Others);
    Out.OtherEntries = Pos + 8;
    Pos += 8;
    for (uint32 Index = 0; Index < Others; ++Index)
    {
        if (End - Pos < 8) return false;
        const uint32 Cinematics = KyaReadU32(Bytes + Pos + 4);
        Out.OtherOffsets.Add(Pos);
        Pos += 8;
        if (uint64(Cinematics) > uint64(End - Pos) / 4) return false;
        Pos += int64(Cinematics) * 4;
    }
    if (End - Pos < 4) return false;
    Out.AtonCount = Pos;
    const uint32 Atons = KyaReadU32(Bytes + Pos);
    Out.AtonCountValue = int32(Atons);
    Out.AtonEntries = Pos + 4;
    Pos += 4;
    for (uint32 Index = 0; Index < Atons; ++Index)
    {
        if (End - Pos < 12) return false;
        const uint32 Cinematics = KyaReadU32(Bytes + Pos + 8);
        Out.AtonOffsets.Add(Pos);
        Pos += 12;
        if (uint64(Cinematics) > uint64(End - Pos) / 4) return false;
        Pos += int64(Cinematics) * 4;
    }
    if (End - Pos < 4) return false;
    Out.AddOnField24 = Pos;
    Out.Tail = Pos + 4;
    return true;
}

bool KyaReadAtonActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaAtonOffsets Off;
    if (!KyaReadAtonOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.AtonWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.AtonLifeMax = int64(KyaReadU32(Bytes + Off.Autonomous));
    Out.AtonGravityScale = KyaReadF32(Bytes + Off.Autonomous + 4);
    Out.AtonDynamicExtField4 = KyaReadF32(Bytes + Off.Autonomous + 8);
    Out.AtonDynamicExtField8 = KyaReadF32(Bytes + Off.Autonomous + 12);
    Out.AtonField354 = int32(KyaReadU32(Bytes + Off.Fields354));
    Out.AtonField358 = int32(KyaReadU32(Bytes + Off.Fields354 + 4));
    Out.AtonField35C = int32(KyaReadU32(Bytes + Off.Fields354 + 8));
    Out.AtonField360 = int32(KyaReadU32(Bytes + Off.Fields354 + 12));
    Out.AtonEffectType = int32(KyaReadU32(Bytes + Off.EffectType));
    Out.AtonTextureIndex = int32(KyaReadU32(Bytes + Off.Texture));
    Out.AtonMeshIndex = int32(KyaReadU32(Bytes + Off.Mesh));
    Out.AtonTrailMaterialId = int32(KyaReadU32(Bytes + Off.Trail));
    Out.AtonPathDefaultDelay = KyaReadF32(Bytes + Off.PathDelay);

    Out.AtonPathIndices.Reset();
    for (int32 Index = 0; Index < Off.PathCountValue; ++Index)
        Out.AtonPathIndices.Add(int32(KyaReadU32(Bytes + Off.PathEntries + int64(Index) * 4)));

    Out.AtonAddOnField18 = int32(KyaReadU32(Bytes + Off.AddOnField18));
    Out.AtonOtherSubObjs.Reset();
    for (int64 Offset : Off.OtherOffsets)
    {
        FKyaAddOnSubObj& Item = Out.AtonOtherSubObjs.AddDefaulted_GetRef();
        Item.Field0 = int32(KyaReadU32(Bytes + Offset));
        const uint32 Cinematics = KyaReadU32(Bytes + Offset + 4);
        for (uint32 Cine = 0; Cine < Cinematics; ++Cine)
            Item.CinematicIds.Add(int32(KyaReadU32(Bytes + Offset + 8 + int64(Cine) * 4)));
    }

    Out.AtonSubObjs.Reset();
    for (int64 Offset : Off.AtonOffsets)
    {
        FKyaAtonSubObj& Item = Out.AtonSubObjs.AddDefaulted_GetRef();
        Item.Field18 = int32(KyaReadU32(Bytes + Offset));
        Item.Field0 = int32(KyaReadU32(Bytes + Offset + 4));
        const uint32 Cinematics = KyaReadU32(Bytes + Offset + 8);
        for (uint32 Cine = 0; Cine < Cinematics; ++Cine)
            Item.CinematicIds.Add(int32(KyaReadU32(Bytes + Offset + 12 + int64(Cine) * 4)));
    }
    Out.AtonAddOnField24 = KyaReadF32(Bytes + Off.AddOnField24);

    Out.bHasAtonData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteAtonActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaAtonOffsets Off;
    if (!KyaReadAtonOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.AtonLifeMax < 0 || Actor.AtonLifeMax > MAX_uint32) return false;
    if (Actor.AtonPathIndices.Num() != Off.PathCountValue ||
        Actor.AtonOtherSubObjs.Num() != Off.OtherCountValue ||
        Actor.AtonSubObjs.Num() != Off.AtonCountValue) return false;
    for (int32 Index = 0; Index < Actor.AtonOtherSubObjs.Num(); ++Index)
        if (Actor.AtonOtherSubObjs[Index].CinematicIds.Num() !=
            int32(KyaReadU32(Data.GetData() + Off.OtherOffsets[Index] + 4))) return false;
    for (int32 Index = 0; Index < Actor.AtonSubObjs.Num(); ++Index)
        if (Actor.AtonSubObjs[Index].CinematicIds.Num() !=
            int32(KyaReadU32(Data.GetData() + Off.AtonOffsets[Index] + 8))) return false;

    KyaWriteF32(Data, Off.WeightB, Actor.AtonWeightB);
    KyaWriteU32(Data, Off.Autonomous, uint32(uint64(Actor.AtonLifeMax)));
    KyaWriteF32(Data, Off.Autonomous + 4, Actor.AtonGravityScale);
    KyaWriteF32(Data, Off.Autonomous + 8, Actor.AtonDynamicExtField4);
    KyaWriteF32(Data, Off.Autonomous + 12, Actor.AtonDynamicExtField8);
    KyaWriteU32(Data, Off.Fields354, uint32(Actor.AtonField354));
    KyaWriteU32(Data, Off.Fields354 + 4, uint32(Actor.AtonField358));
    KyaWriteU32(Data, Off.Fields354 + 8, uint32(Actor.AtonField35C));
    KyaWriteU32(Data, Off.Fields354 + 12, uint32(Actor.AtonField360));
    KyaWriteU32(Data, Off.EffectType, uint32(Actor.AtonEffectType));
    KyaWriteU32(Data, Off.Texture, uint32(Actor.AtonTextureIndex));
    KyaWriteU32(Data, Off.Mesh, uint32(Actor.AtonMeshIndex));
    KyaWriteU32(Data, Off.Trail, uint32(Actor.AtonTrailMaterialId));
    KyaWriteF32(Data, Off.PathDelay, Actor.AtonPathDefaultDelay);
    for (int32 Index = 0; Index < Actor.AtonPathIndices.Num(); ++Index)
        KyaWriteU32(Data, Off.PathEntries + int64(Index) * 4, uint32(Actor.AtonPathIndices[Index]));
    KyaWriteU32(Data, Off.AddOnField18, uint32(Actor.AtonAddOnField18));
    for (int32 Index = 0; Index < Actor.AtonOtherSubObjs.Num(); ++Index)
    {
        const FKyaAddOnSubObj& Item = Actor.AtonOtherSubObjs[Index];
        const int64 Offset = Off.OtherOffsets[Index];
        KyaWriteU32(Data, Offset, uint32(Item.Field0));
        for (int32 Cine = 0; Cine < Item.CinematicIds.Num(); ++Cine)
            KyaWriteU32(Data, Offset + 8 + int64(Cine) * 4, uint32(Item.CinematicIds[Cine]));
    }
    for (int32 Index = 0; Index < Actor.AtonSubObjs.Num(); ++Index)
    {
        const FKyaAtonSubObj& Item = Actor.AtonSubObjs[Index];
        const int64 Offset = Off.AtonOffsets[Index];
        KyaWriteU32(Data, Offset, uint32(Item.Field18));
        KyaWriteU32(Data, Offset + 4, uint32(Item.Field0));
        for (int32 Cine = 0; Cine < Item.CinematicIds.Num(); ++Cine)
            KyaWriteU32(Data, Offset + 12 + int64(Cine) * 4, uint32(Item.CinematicIds[Cine]));
    }
    KyaWriteF32(Data, Off.AddOnField24, Actor.AtonAddOnField24);
    return true;
}
