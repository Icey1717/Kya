#include "Actors/KyaNativCmd.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadNativCmdOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaNativCmdOffsets& Out)
{
    Out = FKyaNativCmdOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 24) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.ActorCount = Start;
    const uint32 Actors = KyaReadU32(Bytes + Start);
    Out.ActorCountValue = int32(Actors);
    Out.ActorEntries = Start + 4;
    int64 Pos = Start + 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Pos += int64(Actors) * 4;
    if (End - Pos < 4) return false;
    Out.Field164 = Pos;
    Pos += 4;
    Out.AddOns = Pos;
    // The actor owns four CAddOnNativ blocks (A-D), each serialized as a
    // sub-object count followed by CAddOnSubObj records.
    for (int32 AddOn = 0; AddOn < 4; ++AddOn)
    {
        if (End - Pos < 4) return false;
        const int64 CountOffset = Pos;
        const uint32 SubObjs = KyaReadU32(Bytes + Pos);
        Pos += 4;
        Out.AddOnCounts.Add(CountOffset);
        Out.AddOnCountValues.Add(int32(SubObjs));
        for (uint32 Sub = 0; Sub < SubObjs; ++Sub)
        {
            if (End - Pos < 8) return false;
            const uint32 Cinematics = KyaReadU32(Bytes + Pos + 4);
            Out.SubObjOffsets.Add(Pos);
            Pos += 8;
            if (uint64(Cinematics) > uint64(End - Pos) / 4) return false;
            Pos += int64(Cinematics) * 4;
        }
    }
    Out.Tail = Pos;
    return true;
}

bool KyaReadNativCmdActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaNativCmdOffsets Off;
    if (!KyaReadNativCmdOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.NativCmdActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ActorCountValue; ++Index)
        Out.NativCmdActorRefs.Add(int32(KyaReadU32(Bytes + Off.ActorEntries + int64(Index) * 4)));
    Out.NativCmdField164 = int32(KyaReadU32(Bytes + Off.Field164));

    Out.NativCmdAddOns.Reset();
    int32 SubObjIndex = 0;
    for (int32 AddOnIndex = 0; AddOnIndex < Off.AddOnCountValues.Num(); ++AddOnIndex)
    {
        FKyaNativCmdAddOn& AddOn = Out.NativCmdAddOns.AddDefaulted_GetRef();
        for (int32 Sub = 0; Sub < Off.AddOnCountValues[AddOnIndex]; ++Sub)
        {
            const int64 SubOffset = Off.SubObjOffsets[SubObjIndex++];
            FKyaAddOnSubObj& Item = AddOn.SubObjs.AddDefaulted_GetRef();
            Item.Field0 = int32(KyaReadU32(Bytes + SubOffset));
            const uint32 Cinematics = KyaReadU32(Bytes + SubOffset + 4);
            for (uint32 Cine = 0; Cine < Cinematics; ++Cine)
                Item.CinematicIds.Add(int32(KyaReadU32(Bytes + SubOffset + 8 + int64(Cine) * 4)));
        }
    }

    Out.bHasNativCmdData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteNativCmdActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaNativCmdOffsets Off;
    if (!KyaReadNativCmdOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.NativCmdActorRefs.Num() != Off.ActorCountValue) return false;
    if (Actor.NativCmdAddOns.Num() != Off.AddOnCountValues.Num()) return false;

    int32 SubObjIndex = 0;
    for (int32 AddOnIndex = 0; AddOnIndex < Actor.NativCmdAddOns.Num(); ++AddOnIndex)
    {
        const FKyaNativCmdAddOn& AddOn = Actor.NativCmdAddOns[AddOnIndex];
        if (AddOn.SubObjs.Num() != Off.AddOnCountValues[AddOnIndex]) return false;
        for (const FKyaAddOnSubObj& Sub : AddOn.SubObjs)
        {
            if (SubObjIndex >= Off.SubObjOffsets.Num()) return false;
            const int64 SubOffset = Off.SubObjOffsets[SubObjIndex++];
            if (Sub.CinematicIds.Num() != int32(KyaReadU32(Data.GetData() + SubOffset + 4)))
                return false;
        }
    }

    for (int32 Index = 0; Index < Actor.NativCmdActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.ActorEntries + int64(Index) * 4, uint32(Actor.NativCmdActorRefs[Index]));
    KyaWriteU32(Data, Off.Field164, uint32(Actor.NativCmdField164));

    SubObjIndex = 0;
    for (int32 AddOnIndex = 0; AddOnIndex < Actor.NativCmdAddOns.Num(); ++AddOnIndex)
    {
        const FKyaNativCmdAddOn& AddOn = Actor.NativCmdAddOns[AddOnIndex];
        for (int32 Sub = 0; Sub < AddOn.SubObjs.Num(); ++Sub)
        {
            const FKyaAddOnSubObj& Item = AddOn.SubObjs[Sub];
            const int64 SubOffset = Off.SubObjOffsets[SubObjIndex++];
            KyaWriteU32(Data, SubOffset, uint32(Item.Field0));
            for (int32 Cine = 0; Cine < Item.CinematicIds.Num(); ++Cine)
                KyaWriteU32(Data, SubOffset + 8 + int64(Cine) * 4, uint32(Item.CinematicIds[Cine]));
        }
    }
    return true;
}
