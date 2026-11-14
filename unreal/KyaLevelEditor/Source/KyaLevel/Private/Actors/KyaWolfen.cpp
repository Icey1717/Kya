#include "Actors/KyaWolfen.h"

#include "Actors/KyaFighter.h"
#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadWolfenOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaWolfenOffsets& Out)
{
    Out = FKyaWolfenOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    FKyaFighterPrefixOffsets Prefix;
    if (!KyaReadFighterPrefixOffsets(Data, Start, Size, Prefix)) return false;
    Out.FighterPrefix = Prefix.Weight;
    Out.FighterFixed = Prefix.Fixed;
    Out.Blows = Prefix.Blows;
    Out.Grabs = Prefix.Grabs;
    Out.GrabReacts = Prefix.GrabReacts;
    Out.Combos = Prefix.Combos;
    Out.WeaponIndex = Prefix.WeaponIndex;

    int64 Pos = Prefix.Tail;
    Out.WolfenStart = Pos;
    if (End - Pos < 96) return false;
    Out.FieldB74 = Pos; Out.Vision = Pos + 4;
    Pos += 24; // field 0xb74 and the 20-byte CVision record.
    Out.StartSector = Pos; Out.Hearing = Pos + 4; Out.VisionDetection = Pos + 12;
    Out.FieldCF4 = Pos + 16; Out.WalkValues = Pos + 20;
    Out.MagicRequired = Pos + 52; Out.FieldD30 = Pos + 56; Out.FieldB90 = Pos + 60;
    Pos += 72;
    if (End - Pos < 108) return false;
    Out.FightParam = Pos;
    Pos += 20 + 20 + 16;
    const auto U32At = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    auto SkipCapability = [&]() -> bool {
        if (End - Pos < 4) return false;
        const uint32 Items = U32At(Pos); Pos += 4;
        for (uint32 Index = 0; Index < Items; ++Index)
        {
            if (End - Pos < 4) return false;
            const uint32 Chains = U32At(Pos); Pos += 4;
            if (Chains > uint64(End - Pos) / 4) return false;
            Pos += int64(Chains) * 4;
            if (End - Pos < 8) return false;
            Pos += 8;
        }
        return true;
    };
    if (!SkipCapability() || !SkipCapability() || !SkipCapability()) return false;
    for (int32 List = 0; List < 2; ++List)
    {
        if (End - Pos < 4) return false;
        const uint32 Items = U32At(Pos); Pos += 4;
        if (Items > uint64(End - Pos) / 4) return false;
        Pos += int64(Items) * 4;
    }
    if (End - Pos < 48) return false;
    Pos += 48;
    Out.Tail = Pos;
    return true;
}

bool KyaReadWolfenFightParamOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End,
    FKyaWolfenFightParamOffsets& Out)
{
    Out = FKyaWolfenFightParamOffsets{};
    if (Start < 0 || Start > Data.Num() || End > Data.Num() || End - Start < 56) return false;
    const uint8* Bytes = Data.GetData();
    const auto U32At = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    Out.Capability0Fields = Start;
    Out.Capability2Fields = Start + 20;
    Out.Floats = Start + 40;
    int64 Pos = Start + 56;
    for (int32 Cap = 0; Cap < 3; ++Cap)
    {
        if (End - Pos < 4) return false;
        FKyaWolfenCapabilityOffsets& CapOffsets = Out.Capabilities[Cap];
        CapOffsets.CountOffset = Pos;
        const uint32 Count = U32At(Pos); Pos += 4;
        for (uint32 Index = 0; Index < Count; ++Index)
        {
            if (End - Pos < 4) return false;
            FKyaWolfenCapabilityItemOffsets& Item = CapOffsets.Items.AddDefaulted_GetRef();
            Item.CountOffset = Pos;
            Item.ChainCount = int32(U32At(Pos)); Pos += 4;
            if (uint64(Item.ChainCount) > uint64(End - Pos) / 4) return false;
            Item.Chains = Pos; Pos += int64(Item.ChainCount) * 4;
            if (End - Pos < 8) return false;
            Item.Floats = Pos; Pos += 8;
        }
        CapOffsets.Tail = Pos;
    }
    for (int32 List = 0; List < 2; ++List)
    {
        if (End - Pos < 4) return false;
        const int64 CountOffset = Pos;
        const uint32 Count = U32At(Pos); Pos += 4;
        if (uint64(Count) > uint64(End - Pos) / 4) return false;
        if (List == 0) { Out.ComboList1Count = CountOffset; Out.ComboList1Data = Pos; }
        else { Out.ComboList2Count = CountOffset; Out.ComboList2Data = Pos; }
        Pos += int64(Count) * 4;
    }
    if (End - Pos < 48) return false;
    Out.Trailing = Pos;
    Pos += 48;
    Out.Tail = Pos;
    return true;
}

bool KyaReadWolfenBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaWolfenBehaviourOffsets& Out)
{
    Out = FKyaWolfenBehaviourOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 8) return false;
    const uint8* Bytes = Data.GetData();
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    Out.Flags = Start;
    if (Id == 15)
    {
        if (Size != 8) return false;
        Out.bTrack = true;
        Out.TrackField = Start + 4;
        Out.Tail = Start + 8;
        return true;
    }
    if (Id == 23)
    {
        if (Size != 12) return false;
        Out.bDca = true;
        Out.DcaActorRef = Start + 4;
        Out.DcaShootCooldown = Start + 8;
        Out.Tail = Start + 12;
        return true;
    }
    if (Id == 24)
    {
        if (Size != 12) return false;
        Out.bAvoid = true;
        Out.Avoid80 = Start + 4;
        Out.Avoid84 = Start + 8;
        Out.Tail = Start + 12;
        return true;
    }
    if (Id == 14)
    {
        if (Size != 48) return false;
        Out.bExorcism = true;
        Out.ExorcismMaterial = Start;
        Out.ExorcismField24 = Start + 4;
        Out.ExorcismMeshes = Start + 8;
        Out.ExorcismField18 = Start + 24;
        Out.ExorcismWords = Start + 28;
        Out.Tail = Start + 48;
        return true;
    }
    if (Id == 10)
    {
        if (Size < 52) return false;
        Out.bGuard = true;
        Out.GuardPath = Start + 4;
        Out.GuardField98 = Start + 8;
        Out.GuardTrack = Start + 12;
        Out.GuardField90 = Start + 16;
        if (!KyaReadSwitchScriptOffsets(Data, Start + 20, Start + Size, Out.Switch)) return false;
        int64 Pos = Out.Switch.Tail;
        if (Start + Size - Pos < 4) return false;
        Out.OnOffCount = Pos;
        const uint32 Targets = U32(Pos);
        Out.OnOffCountValue = int32(Targets);
        Pos += 4;
        if (uint64(Targets) > uint64(Start + Size - Pos) / 28) return false;
        Out.OnOffEntries = Pos;
        Pos += int64(Targets) * 28;
        if (Start + Size - Pos < 32) return false;
        Out.OnOffCamera = Pos;
        Pos += 32;
        if (Pos != Start + Size) return false;
        Out.Tail = Pos;
        return true;
    }
    if (Id == 13)
    {
        if (Size < 40) return false;
        Out.bEscape = true;
        Out.EscapeCount = Start + 4;
        const uint32 Readers = U32(Out.EscapeCount);
        Out.EscapeCountValue = int32(Readers);
        Out.EscapeEntries = Start + 8;
        int64 Pos = Out.EscapeEntries;
        if (uint64(Readers) > uint64(Start + Size - Pos) / 4) return false;
        Pos += int64(Readers) * 4;
        if (Start + Size - Pos < 28) return false;
        Out.EscapeField88 = Pos;
        Out.EscapeFloats = Pos + 4;
        Out.EscapeFieldA0 = Pos + 24;
        Pos += 28;
        if (!KyaReadSwitchScriptOffsets(Data, Pos, Start + Size, Out.Switch)) return false;
        if (Out.Switch.Tail != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 22)
    {
        Out.bLost = true;
        if (!KyaReadSwitchScriptOffsets(Data, Start + 4, Start + Size, Out.Switch)) return false;
        if (Out.Switch.Tail != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 8)
    {
        if (Size < 48) return false;
        Out.bWatchDog = true;
        Out.Field80 = Start + 4;
        Out.TrackBehaviourId = Start + 8;
        if (!KyaReadSwitchScriptOffsets(Data, Start + 12, Start + Size, Out.Switch)) return false;
        int64 Pos = Out.Switch.Tail;
        if (Start + Size - Pos < 4) return false;
        Out.OnOffCount = Pos;
        const uint32 Targets = U32(Pos);
        Out.OnOffCountValue = int32(Targets);
        Pos += 4;
        if (uint64(Targets) > uint64(Start + Size - Pos) / 28) return false;
        Out.OnOffEntries = Pos;
        Pos += int64(Targets) * 28;
        if (Start + Size - Pos < 32) return false;
        Out.OnOffCamera = Pos;
        Pos += 32;
        if (Pos != Start + Size) return false;
        Out.Tail = Pos;
        return true;
    }
    if (Id == 21)
    {
        if (Size < 96) return false;
        Out.bSnipe = true;
        Out.SnipeField90 = Start + 4;
        Out.SnipeField94 = Start + 8;
        Out.SpotTrack = Start + 24;
        Out.SnipeTrack = Start + 28;
        Out.ShadowMaterial = Start + 32;
        Out.SnipeFieldAC = Start + 36;
        Out.SnipeFieldB0 = Start + 40;
        Out.SnipeFieldB4 = Start + 44;
        Out.NbSpot = Start + 48;
        Out.NbShadow = Start + 52;
        Out.RotationSpeed = Start + 56;
        Out.SnipeFieldB8 = Start + 60;
        if (!KyaReadSwitchScriptOffsets(Data, Start + 64, Start + Size, Out.Switch)) return false;
        if (Out.Switch.Tail != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 17 || Id == 18 || Id == 19)
    {
        if (Size < 20) return false;
        Out.bWeapon = Id == 17;
        Out.bWeaponStand = Id == 18;
        Out.bWeaponSnipe = Id == 19;
        Out.WeaponField90 = Start + 4;
        Out.WeaponField94 = Start + 8;
        Out.WeaponField98 = Start + 12;
        Out.WeaponCount = Start + 16;
        const uint32 Count = U32(Start + 16);
        Out.WeaponCountValue = int32(Count);
        int64 Pos = Start + 20;
        if (uint64(Count) > uint64(Size - 20) / 20) return false;
        Out.WeaponEntries = Pos;
        for (uint32 Index = 0; Index < Count; ++Index)
        {
            FKyaTrackSubObjOffsets SubObj;
            SubObj.Field0 = Pos;
            SubObj.AField0 = Pos + 4;
            SubObj.AField4 = Pos + 8;
            SubObj.AField8 = Pos + 12;
            SubObj.Field10 = Pos + 16;
            Out.SubObjs.Add(SubObj);
            Pos += 20;
        }
        const int64 Expected = Id == 18 ? Pos : (Id == 17 ? Pos + 8 : Pos + 4);
        if (Start + Size != Expected) return false;
        if (Id == 17)
        {
            Out.WeaponFieldF0 = Pos;
            Out.WeaponTargetField = Pos + 4;
        }
        else if (Id == 19) Out.WeaponFieldF0 = Pos;
        Out.Tail = Start + Size;
        return true;
    }
    return false;
}

bool KyaReadWolfenActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaWolfenOffsets Off;
    if (!KyaReadWolfenOffsets(Data, Start, Size, Off)) return false;
    FKyaWolfenFightParamOffsets FightParam;
    if (!KyaReadWolfenFightParamOffsets(Data, Off.FightParam, Off.Tail, FightParam) ||
        FightParam.Tail != Off.Tail) return false;
    const uint8* Bytes = Data.GetData();

    FKyaWolfenFightParam& Params = Out.WolfenFightParamData;
    for (int32 Index = 0; Index < 5; ++Index)
    {
        Params.Capability0Fields.Add(int32(KyaReadU32(Bytes + FightParam.Capability0Fields + Index * 4)));
        Params.Capability2Fields.Add(int32(KyaReadU32(Bytes + FightParam.Capability2Fields + Index * 4)));
    }
    for (int32 Index = 0; Index < 4; ++Index)
        Params.ConfigurationValues.Add(KyaReadF32(Bytes + FightParam.Floats + Index * 4));
    for (int32 Cap = 0; Cap < 3; ++Cap)
    {
        FKyaWolfenCapability& Capability = Params.Capabilities.AddDefaulted_GetRef();
        for (const FKyaWolfenCapabilityItemOffsets& ItemOffsets : FightParam.Capabilities[Cap].Items)
        {
            FKyaWolfenCapabilityItem& Item = Capability.Items.AddDefaulted_GetRef();
            for (int32 Index = 0; Index < ItemOffsets.ChainCount; ++Index)
                Item.Chains.Add(int32(KyaReadU32(Bytes + ItemOffsets.Chains + Index * 4)));
            Item.Field0 = KyaReadF32(Bytes + ItemOffsets.Floats);
            Item.Field4 = KyaReadF32(Bytes + ItemOffsets.Floats + 4);
        }
    }
    const uint32 Combo1Count = KyaReadU32(Bytes + FightParam.ComboList1Count);
    for (uint32 Index = 0; Index < Combo1Count; ++Index)
        Params.ComboList1.Add(int32(KyaReadU32(Bytes + FightParam.ComboList1Data + Index * 4)));
    const uint32 Combo2Count = KyaReadU32(Bytes + FightParam.ComboList2Count);
    for (uint32 Index = 0; Index < Combo2Count; ++Index)
        Params.ComboList2.Add(int32(KyaReadU32(Bytes + FightParam.ComboList2Data + Index * 4)));
    Params.FieldAfterLists = int32(KyaReadU32(Bytes + FightParam.Trailing));
    Params.FieldB44 = int32(KyaReadU32(Bytes + FightParam.Trailing + 4));
    for (int32 Index = 0; Index < 6; ++Index)
        Params.TuningValues.Add(KyaReadF32(Bytes + FightParam.Trailing + 8 + Index * 4));
    for (int32 Index = 0; Index < 4; ++Index)
        Params.Knowledge.Add(int32(KyaReadU32(Bytes + FightParam.Trailing + 32 + Index * 4)));

    Out.WolfenFieldB74 = int32(KyaReadU32(Bytes + Off.FieldB74));
    Out.WolfenVisionRange = KyaReadF32(Bytes + Off.Vision);
    Out.WolfenVisionFov = KyaReadF32(Bytes + Off.Vision + 4);
    Out.WolfenVisionField38 = KyaReadF32(Bytes + Off.Vision + 8);
    Out.WolfenVisionField3C = KyaReadF32(Bytes + Off.Vision + 12);
    Out.WolfenVisionFlags = int32(KyaReadU32(Bytes + Off.Vision + 16));
    Out.WolfenStartSectorId = int32(KyaReadU32(Bytes + Off.StartSector));
    Out.WolfenHearingRangeSquared = KyaReadF32(Bytes + Off.Hearing);
    Out.WolfenHearingMaxHeight = KyaReadF32(Bytes + Off.Hearing + 4);
    Out.WolfenVisionDetectionRange = KyaReadF32(Bytes + Off.VisionDetection);
    Out.WolfenFieldCF4 = KyaReadF32(Bytes + Off.FieldCF4);
    Out.WolfenWalkSpeed = KyaReadF32(Bytes + Off.WalkValues);
    Out.WolfenWalkAcceleration = KyaReadF32(Bytes + Off.WalkValues + 4);
    Out.WolfenWalkRotSpeed = KyaReadF32(Bytes + Off.WalkValues + 8);
    Out.WolfenRunSpeedScale = KyaReadF32(Bytes + Off.WalkValues + 12);
    Out.WolfenDefaultRunSpeed = KyaReadF32(Bytes + Off.WalkValues + 16);
    Out.WolfenRunAcceleration = KyaReadF32(Bytes + Off.WalkValues + 20);
    Out.WolfenRotRunSpeed = KyaReadF32(Bytes + Off.WalkValues + 24);
    Out.WolfenMagicRequired = int32(KyaReadU32(Bytes + Off.MagicRequired));
    Out.WolfenFieldD30 = int32(KyaReadU32(Bytes + Off.FieldD30));
    Out.WolfenFieldB90 = KyaReadF32(Bytes + Off.FieldB90);
    Out.WolfenFieldB94 = KyaReadF32(Bytes + Off.FieldB90 + 4);
    Out.WolfenFieldB98 = KyaReadF32(Bytes + Off.FieldB90 + 8);

    Out.bHasWolfenData = true;
    Out.ClassData.Reset();
    return true;
}

bool KyaWriteWolfenActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaWolfenOffsets Off;
    if (!KyaReadWolfenOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    FKyaWolfenFightParamOffsets FightParam;
    if (!KyaReadWolfenFightParamOffsets(MakeArrayView(Data.GetData(), Data.Num()), Off.FightParam,
        Off.Tail, FightParam) || FightParam.Tail != Off.Tail)
        return false;
    const FKyaWolfenFightParam& Params = Actor.WolfenFightParamData;
    if (Params.Capability0Fields.Num() != 5 || Params.Capability2Fields.Num() != 5 ||
        Params.ConfigurationValues.Num() != 4 || Params.TuningValues.Num() != 6 ||
        Params.Knowledge.Num() != 4 || Params.Capabilities.Num() != 3)
        return false;
    for (int32 Cap = 0; Cap < 3; ++Cap)
        if (Params.Capabilities[Cap].Items.Num() != FightParam.Capabilities[Cap].Items.Num())
            return false;
    if (Params.ComboList1.Num() != int32(KyaReadU32(Data.GetData() + FightParam.ComboList1Count)) ||
        Params.ComboList2.Num() != int32(KyaReadU32(Data.GetData() + FightParam.ComboList2Count)))
        return false;

    KyaWriteU32(Data, Off.FieldB74, uint32(Actor.WolfenFieldB74));
    KyaWriteF32(Data, Off.Vision, Actor.WolfenVisionRange);
    KyaWriteF32(Data, Off.Vision + 4, Actor.WolfenVisionFov);
    KyaWriteF32(Data, Off.Vision + 8, Actor.WolfenVisionField38);
    KyaWriteF32(Data, Off.Vision + 12, Actor.WolfenVisionField3C);
    KyaWriteU32(Data, Off.Vision + 16, uint32(Actor.WolfenVisionFlags));
    KyaWriteU32(Data, Off.StartSector, uint32(Actor.WolfenStartSectorId));
    KyaWriteF32(Data, Off.Hearing, Actor.WolfenHearingRangeSquared);
    KyaWriteF32(Data, Off.Hearing + 4, Actor.WolfenHearingMaxHeight);
    KyaWriteF32(Data, Off.VisionDetection, Actor.WolfenVisionDetectionRange);
    KyaWriteF32(Data, Off.FieldCF4, Actor.WolfenFieldCF4);
    KyaWriteF32(Data, Off.WalkValues, Actor.WolfenWalkSpeed);
    KyaWriteF32(Data, Off.WalkValues + 4, Actor.WolfenWalkAcceleration);
    KyaWriteF32(Data, Off.WalkValues + 8, Actor.WolfenWalkRotSpeed);
    KyaWriteF32(Data, Off.WalkValues + 12, Actor.WolfenRunSpeedScale);
    KyaWriteF32(Data, Off.WalkValues + 16, Actor.WolfenDefaultRunSpeed);
    KyaWriteF32(Data, Off.WalkValues + 20, Actor.WolfenRunAcceleration);
    KyaWriteF32(Data, Off.WalkValues + 24, Actor.WolfenRotRunSpeed);
    KyaWriteU32(Data, Off.MagicRequired, uint32(Actor.WolfenMagicRequired));
    KyaWriteU32(Data, Off.FieldD30, uint32(Actor.WolfenFieldD30));
    KyaWriteF32(Data, Off.FieldB90, Actor.WolfenFieldB90);
    KyaWriteF32(Data, Off.FieldB90 + 4, Actor.WolfenFieldB94);
    KyaWriteF32(Data, Off.FieldB90 + 8, Actor.WolfenFieldB98);

    for (int32 Index = 0; Index < 5; ++Index)
    {
        KyaWriteU32(Data, FightParam.Capability0Fields + Index * 4, uint32(Params.Capability0Fields[Index]));
        KyaWriteU32(Data, FightParam.Capability2Fields + Index * 4, uint32(Params.Capability2Fields[Index]));
    }
    for (int32 Index = 0; Index < 4; ++Index)
        KyaWriteF32(Data, FightParam.Floats + Index * 4, Params.ConfigurationValues[Index]);
    for (int32 Cap = 0; Cap < 3; ++Cap)
    {
        const FKyaWolfenCapability& Capability = Params.Capabilities[Cap];
        const FKyaWolfenCapabilityOffsets& CapOffsets = FightParam.Capabilities[Cap];
        for (int32 ItemIndex = 0; ItemIndex < Capability.Items.Num(); ++ItemIndex)
        {
            const FKyaWolfenCapabilityItem& Item = Capability.Items[ItemIndex];
            const FKyaWolfenCapabilityItemOffsets& ItemOffsets = CapOffsets.Items[ItemIndex];
            if (Item.Chains.Num() != ItemOffsets.ChainCount) return false;
            for (int32 Chain = 0; Chain < Item.Chains.Num(); ++Chain)
                KyaWriteU32(Data, ItemOffsets.Chains + Chain * 4, uint32(Item.Chains[Chain]));
            KyaWriteF32(Data, ItemOffsets.Floats, Item.Field0);
            KyaWriteF32(Data, ItemOffsets.Floats + 4, Item.Field4);
        }
    }
    for (int32 Index = 0; Index < Params.ComboList1.Num(); ++Index)
        KyaWriteU32(Data, FightParam.ComboList1Data + Index * 4, uint32(Params.ComboList1[Index]));
    for (int32 Index = 0; Index < Params.ComboList2.Num(); ++Index)
        KyaWriteU32(Data, FightParam.ComboList2Data + Index * 4, uint32(Params.ComboList2[Index]));
    KyaWriteU32(Data, FightParam.Trailing, uint32(Params.FieldAfterLists));
    KyaWriteU32(Data, FightParam.Trailing + 4, uint32(Params.FieldB44));
    for (int32 Index = 0; Index < 6; ++Index)
        KyaWriteF32(Data, FightParam.Trailing + 8 + Index * 4, Params.TuningValues[Index]);
    for (int32 Index = 0; Index < 4; ++Index)
        KyaWriteU32(Data, FightParam.Trailing + 32 + Index * 4, uint32(Params.Knowledge[Index]));
    return true;
}

bool KyaReadWolfenBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    FKyaWolfenBehaviourOffsets Off;
    if (!KyaReadWolfenBehaviourOffsets(Id, Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    FKyaWolfenBehaviour& Values = Out.WolfenBehaviour;
    Values = FKyaWolfenBehaviour{};
    if (!Off.bExorcism) Values.Flags = int32(KyaReadU32(Bytes + Off.Flags));
    if (Off.bTrack) Values.TrackField = KyaReadF32(Bytes + Off.TrackField);
    if (Off.bDca)
    {
        Values.DcaActorRef = int32(KyaReadU32(Bytes + Off.DcaActorRef));
        Values.DcaShootCooldown = KyaReadF32(Bytes + Off.DcaShootCooldown);
    }
    if (Off.bAvoid)
    {
        Values.AvoidField80 = KyaReadF32(Bytes + Off.Avoid80);
        Values.AvoidField84 = KyaReadF32(Bytes + Off.Avoid84);
    }
    auto PopulateSwitchScript = [&](const FKyaSwitchScriptOffsets& Script) {
        for (int32 RecordIndex = 0; RecordIndex < Script.RecordOffsets.Num(); ++RecordIndex)
        {
            FKyaSwitchBehaviourRecord& Record = Values.SwitchRecords.AddDefaulted_GetRef();
            const int32 Category = Script.RecordCategories[RecordIndex];
            const int64 P = Script.RecordOffsets[RecordIndex];
            Record.Category = Category;
            if (Category == 0)
            {
                Record.MessageType = int32(KyaReadU32(Bytes + P));
                Record.BehaviourId = int32(KyaReadU32(Bytes + P + 4));
            }
            else if (Category == 3)
            {
                Record.SeeActorRef = int32(KyaReadU32(Bytes + P));
                Record.BehaviourId = int32(KyaReadU32(Bytes + P + 4));
            }
            else
            {
                Record.ActorRef = int32(KyaReadU32(Bytes + P));
                Record.ZoneRef = int32(KyaReadU32(Bytes + P + 4));
                Record.Flags = int32(KyaReadU32(Bytes + P + 8));
                Record.BehaviourId = int32(KyaReadU32(Bytes + P + 12));
            }
        }
    };
    auto PopulateOnOff = [&](int32 Count, int64 Entries, int64 Camera) {
        for (int32 Index = 0; Index < Count; ++Index)
        {
            FKyaSwitchTarget& Target = Values.OnOffTargets.AddDefaulted_GetRef();
            const int64 T = Entries + int64(Index) * 28;
            Target.TargetRef = int32(KyaReadU32(Bytes + T));
            Target.CutsceneId = int32(KyaReadU32(Bytes + T + 4));
            Target.Flags = int32(KyaReadU32(Bytes + T + 8));
            Target.MessageId = int32(KyaReadU32(Bytes + T + 12));
            Target.MessageFlags = int32(KyaReadU32(Bytes + T + 16));
            Target.Field14 = int32(KyaReadU32(Bytes + T + 20));
            Target.Field18 = int32(KyaReadU32(Bytes + T + 24));
        }
        Values.OnOffCamera.CameraIndex = int32(KyaReadU32(Bytes + Camera));
        Values.OnOffCamera.Flags = int32(KyaReadU32(Bytes + Camera + 4));
        Values.OnOffCamera.Field8 = KyaReadF32(Bytes + Camera + 8);
        Values.OnOffCamera.FieldC = KyaReadF32(Bytes + Camera + 12);
        Values.OnOffCamera.Field10 = KyaReadF32(Bytes + Camera + 16);
        Values.OnOffCamera.ActorRef = int32(KyaReadU32(Bytes + Camera + 20));
        Values.OnOffCamera.ZoneRef = int32(KyaReadU32(Bytes + Camera + 24));
        Values.OnOffCamera.Field1C = KyaReadF32(Bytes + Camera + 28);
    };
    if (Off.bLost) PopulateSwitchScript(Off.Switch);
    if (Off.bWatchDog)
    {
        Values.Field80 = int32(KyaReadU32(Bytes + Off.Field80));
        Values.TrackBehaviourId = int32(KyaReadU32(Bytes + Off.TrackBehaviourId));
        PopulateSwitchScript(Off.Switch);
        PopulateOnOff(Off.OnOffCountValue, Off.OnOffEntries, Off.OnOffCamera);
    }
    if (Off.bExorcism)
    {
        Values.ExorcismDigitMaterialId = int32(KyaReadU32(Bytes + Off.ExorcismMaterial));
        Values.ExorcismField24 = int32(KyaReadU32(Bytes + Off.ExorcismField24));
        Values.ExorcismTextureA = int32(KyaReadU32(Bytes + Off.ExorcismMeshes));
        Values.ExorcismMeshA = int32(KyaReadU32(Bytes + Off.ExorcismMeshes + 4));
        Values.ExorcismTextureB = int32(KyaReadU32(Bytes + Off.ExorcismMeshes + 8));
        Values.ExorcismMeshB = int32(KyaReadU32(Bytes + Off.ExorcismMeshes + 12));
        Values.ExorcismField18 = int32(KyaReadU32(Bytes + Off.ExorcismField18));
        for (int32 Index = 0; Index < 5; ++Index)
            Values.ExorcismWords.Add(int32(KyaReadU32(Bytes + Off.ExorcismWords + Index * 4)));
    }
    if (Off.bGuard)
    {
        Values.GuardPathIndex = int32(KyaReadU32(Bytes + Off.GuardPath));
        Values.GuardField98 = KyaReadF32(Bytes + Off.GuardField98);
        Values.GuardTrackBehaviourId = int32(KyaReadU32(Bytes + Off.GuardTrack));
        Values.GuardField90 = int32(KyaReadU32(Bytes + Off.GuardField90));
        PopulateSwitchScript(Off.Switch);
        PopulateOnOff(Off.OnOffCountValue, Off.OnOffEntries, Off.OnOffCamera);
    }
    if (Off.bEscape)
    {
        Values.EscapePathIndices.Reset();
        for (int32 Index = 0; Index < Off.EscapeCountValue; ++Index)
            Values.EscapePathIndices.Add(int32(KyaReadU32(Bytes + Off.EscapeEntries + int64(Index) * 4)));
        Values.EscapeField88 = int32(KyaReadU32(Bytes + Off.EscapeField88));
        Values.EscapeField9C = KyaReadF32(Bytes + Off.EscapeFloats);
        Values.EscapeField90 = KyaReadF32(Bytes + Off.EscapeFloats + 4);
        Values.EscapeField8C = KyaReadF32(Bytes + Off.EscapeFloats + 8);
        Values.EscapeField94 = KyaReadF32(Bytes + Off.EscapeFloats + 12);
        Values.EscapeField98 = KyaReadF32(Bytes + Off.EscapeFloats + 16);
        Values.EscapeFieldA0 = int32(KyaReadU32(Bytes + Off.EscapeFieldA0));
        PopulateSwitchScript(Off.Switch);
    }
    if (Off.bSnipe)
    {
        Values.SnipeField90 = int32(KyaReadU32(Bytes + Off.SnipeField90));
        Values.SnipeField94 = KyaReadF32(Bytes + Off.SnipeField94);
        Values.SnipeField98 = KyaReadF32(Bytes + Off.SnipeField94 + 4);
        Values.SnipeField9C = KyaReadF32(Bytes + Off.SnipeField94 + 8);
        Values.SnipeFieldA0 = KyaReadF32(Bytes + Off.SnipeField94 + 12);
        Values.SpotTrackBehaviourId = int32(KyaReadU32(Bytes + Off.SpotTrack));
        Values.SnipeTrackBehaviourId = int32(KyaReadU32(Bytes + Off.SnipeTrack));
        Values.ShadowMaterialId = int32(KyaReadU32(Bytes + Off.ShadowMaterial));
        Values.SnipeFieldAC = int32(KyaReadU32(Bytes + Off.SnipeFieldAC));
        Values.SnipeFieldB0 = KyaReadF32(Bytes + Off.SnipeFieldB0);
        Values.SnipeFieldB4 = KyaReadF32(Bytes + Off.SnipeFieldB4);
        Values.NbSpot = int32(KyaReadU32(Bytes + Off.NbSpot));
        Values.NbShadow = int32(KyaReadU32(Bytes + Off.NbShadow));
        Values.RotationSpeed = KyaReadF32(Bytes + Off.RotationSpeed);
        Values.SnipeFieldB8 = int32(KyaReadU32(Bytes + Off.SnipeFieldB8));
        PopulateSwitchScript(Off.Switch);
    }
    if (Off.bWeaponStand || Off.bWeapon || Off.bWeaponSnipe)
    {
        Values.WeaponField90 = KyaReadF32(Bytes + Off.WeaponField90);
        Values.WeaponField94 = int32(KyaReadU32(Bytes + Off.WeaponField94));
        Values.WeaponField98 = int32(KyaReadU32(Bytes + Off.WeaponField98));
        for (const FKyaTrackSubObjOffsets& SubObjOffsets : Off.SubObjs)
        {
            FKyaTrackSubObj& SubObj = Values.WeaponSubObjs.AddDefaulted_GetRef();
            SubObj.Field0 = int32(KyaReadU32(Bytes + SubObjOffsets.Field0));
            SubObj.AField0 = int32(KyaReadU32(Bytes + SubObjOffsets.AField0));
            SubObj.AField4 = KyaReadF32(Bytes + SubObjOffsets.AField4);
            SubObj.AField8 = KyaReadF32(Bytes + SubObjOffsets.AField8);
            SubObj.Field10 = int32(KyaReadU32(Bytes + SubObjOffsets.Field10));
        }
        if (Off.bWeapon || Off.bWeaponSnipe) Values.WeaponFieldF0 = KyaReadF32(Bytes + Off.WeaponFieldF0);
        if (Off.bWeapon) Values.WeaponTargetField = KyaReadF32(Bytes + Off.WeaponTargetField);
    }
    Out.bHasTypedWolfenBehaviour = true;
    return true;
}

bool KyaWriteWolfenBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    FKyaWolfenBehaviourOffsets Off;
    if (!KyaReadWolfenBehaviourOffsets(Behaviour.Id, MakeArrayView(Data.GetData(), Data.Num()),
        Start, Size, Off))
        return false;
    const FKyaWolfenBehaviour& Values = Behaviour.WolfenBehaviour;
    if (!Off.bExorcism) KyaWriteU32(Data, Off.Flags, uint32(Values.Flags));
    if (Off.bTrack) KyaWriteF32(Data, Off.TrackField, Values.TrackField);
    if (Off.bDca)
    {
        KyaWriteU32(Data, Off.DcaActorRef, uint32(Values.DcaActorRef));
        KyaWriteF32(Data, Off.DcaShootCooldown, Values.DcaShootCooldown);
    }
    if (Off.bAvoid)
    {
        KyaWriteF32(Data, Off.Avoid80, Values.AvoidField80);
        KyaWriteF32(Data, Off.Avoid84, Values.AvoidField84);
    }
    if (Off.bWeaponStand || Off.bWeapon || Off.bWeaponSnipe)
    {
        KyaWriteF32(Data, Off.WeaponField90, Values.WeaponField90);
        KyaWriteU32(Data, Off.WeaponField94, uint32(Values.WeaponField94));
        KyaWriteU32(Data, Off.WeaponField98, uint32(Values.WeaponField98));
        if (Values.WeaponSubObjs.Num() != Off.SubObjs.Num())
            return false;
        for (int32 Index = 0; Index < Values.WeaponSubObjs.Num(); ++Index)
        {
            const FKyaTrackSubObj& SubObj = Values.WeaponSubObjs[Index];
            const FKyaTrackSubObjOffsets& SubObjOffsets = Off.SubObjs[Index];
            KyaWriteU32(Data, SubObjOffsets.Field0, uint32(SubObj.Field0));
            KyaWriteU32(Data, SubObjOffsets.AField0, uint32(SubObj.AField0));
            KyaWriteF32(Data, SubObjOffsets.AField4, SubObj.AField4);
            KyaWriteF32(Data, SubObjOffsets.AField8, SubObj.AField8);
            KyaWriteU32(Data, SubObjOffsets.Field10, uint32(SubObj.Field10));
        }
        if (Off.bWeapon || Off.bWeaponSnipe) KyaWriteF32(Data, Off.WeaponFieldF0, Values.WeaponFieldF0);
        if (Off.bWeapon) KyaWriteF32(Data, Off.WeaponTargetField, Values.WeaponTargetField);
    }
    auto WriteSwitchScript = [&](const FKyaSwitchScriptOffsets& Script) -> bool {
        if (Values.SwitchRecords.Num() != Script.RecordOffsets.Num())
            return false;
        int32 CategoryCounts[4] = { 0, 0, 0, 0 };
        for (const FKyaSwitchBehaviourRecord& Record : Values.SwitchRecords)
        {
            if (Record.Category < 0 || Record.Category > 3) return false;
            ++CategoryCounts[Record.Category];
        }
        for (int32 Category = 0; Category < 4; ++Category)
        {
            if (CategoryCounts[Category] != Script.CountValues[Category])
                return false;
            KyaWriteU32(Data, Script.Counts[Category], uint32(Script.CountValues[Category]));
        }
        for (int32 Index = 0; Index < Values.SwitchRecords.Num(); ++Index)
        {
            const FKyaSwitchBehaviourRecord& Record = Values.SwitchRecords[Index];
            const int64 P = Script.RecordOffsets[Index];
            if (Record.Category != Script.RecordCategories[Index]) return false;
            if (Record.Category == 0)
            {
                KyaWriteU32(Data, P, uint32(Record.MessageType));
                KyaWriteU32(Data, P + 4, uint32(Record.BehaviourId));
            }
            else if (Record.Category == 3)
            {
                KyaWriteU32(Data, P, uint32(Record.SeeActorRef));
                KyaWriteU32(Data, P + 4, uint32(Record.BehaviourId));
            }
            else
            {
                KyaWriteU32(Data, P, uint32(Record.ActorRef));
                KyaWriteU32(Data, P + 4, uint32(Record.ZoneRef));
                KyaWriteU32(Data, P + 8, uint32(Record.Flags));
                KyaWriteU32(Data, P + 12, uint32(Record.BehaviourId));
            }
        }
        return true;
    };
    auto WriteOnOff = [&](int32 Count, int64 Entries, int64 Camera) -> bool {
        if (Values.OnOffTargets.Num() != Count)
            return false;
        for (int32 Index = 0; Index < Values.OnOffTargets.Num(); ++Index)
        {
            const FKyaSwitchTarget& Target = Values.OnOffTargets[Index];
            const int64 T = Entries + int64(Index) * 28;
            KyaWriteU32(Data, T, uint32(Target.TargetRef));
            KyaWriteU32(Data, T + 4, uint32(Target.CutsceneId));
            KyaWriteU32(Data, T + 8, uint32(Target.Flags));
            KyaWriteU32(Data, T + 12, uint32(Target.MessageId));
            KyaWriteU32(Data, T + 16, uint32(Target.MessageFlags));
            KyaWriteU32(Data, T + 20, uint32(Target.Field14));
            KyaWriteU32(Data, T + 24, uint32(Target.Field18));
        }
        KyaWriteU32(Data, Camera, uint32(Values.OnOffCamera.CameraIndex));
        KyaWriteU32(Data, Camera + 4, uint32(Values.OnOffCamera.Flags));
        KyaWriteF32(Data, Camera + 8, Values.OnOffCamera.Field8);
        KyaWriteF32(Data, Camera + 12, Values.OnOffCamera.FieldC);
        KyaWriteF32(Data, Camera + 16, Values.OnOffCamera.Field10);
        KyaWriteU32(Data, Camera + 20, uint32(Values.OnOffCamera.ActorRef));
        KyaWriteU32(Data, Camera + 24, uint32(Values.OnOffCamera.ZoneRef));
        KyaWriteF32(Data, Camera + 28, Values.OnOffCamera.Field1C);
        return true;
    };
    if (Off.bLost && !WriteSwitchScript(Off.Switch)) return false;
    if (Off.bWatchDog)
    {
        KyaWriteU32(Data, Off.Field80, uint32(Values.Field80));
        KyaWriteU32(Data, Off.TrackBehaviourId, uint32(Values.TrackBehaviourId));
        if (!WriteSwitchScript(Off.Switch) || !WriteOnOff(Off.OnOffCountValue, Off.OnOffEntries, Off.OnOffCamera)) return false;
    }
    if (Off.bExorcism)
    {
        if (Values.ExorcismWords.Num() != 5)
            return false;
        KyaWriteU32(Data, Off.ExorcismMaterial, uint32(Values.ExorcismDigitMaterialId));
        KyaWriteU32(Data, Off.ExorcismField24, uint32(Values.ExorcismField24));
        KyaWriteU32(Data, Off.ExorcismMeshes, uint32(Values.ExorcismTextureA));
        KyaWriteU32(Data, Off.ExorcismMeshes + 4, uint32(Values.ExorcismMeshA));
        KyaWriteU32(Data, Off.ExorcismMeshes + 8, uint32(Values.ExorcismTextureB));
        KyaWriteU32(Data, Off.ExorcismMeshes + 12, uint32(Values.ExorcismMeshB));
        KyaWriteU32(Data, Off.ExorcismField18, uint32(Values.ExorcismField18));
        for (int32 Index = 0; Index < Values.ExorcismWords.Num(); ++Index)
            KyaWriteU32(Data, Off.ExorcismWords + Index * 4, uint32(Values.ExorcismWords[Index]));
    }
    if (Off.bGuard)
    {
        KyaWriteU32(Data, Off.GuardPath, uint32(Values.GuardPathIndex));
        KyaWriteF32(Data, Off.GuardField98, Values.GuardField98);
        KyaWriteU32(Data, Off.GuardTrack, uint32(Values.GuardTrackBehaviourId));
        KyaWriteU32(Data, Off.GuardField90, uint32(Values.GuardField90));
        if (!WriteSwitchScript(Off.Switch) || !WriteOnOff(Off.OnOffCountValue, Off.OnOffEntries, Off.OnOffCamera)) return false;
    }
    if (Off.bEscape)
    {
        if (Values.EscapePathIndices.Num() != Off.EscapeCountValue)
            return false;
        for (int32 Index = 0; Index < Values.EscapePathIndices.Num(); ++Index)
            KyaWriteU32(Data, Off.EscapeEntries + Index * 4, uint32(Values.EscapePathIndices[Index]));
        KyaWriteU32(Data, Off.EscapeField88, uint32(Values.EscapeField88));
        KyaWriteF32(Data, Off.EscapeFloats, Values.EscapeField9C);
        KyaWriteF32(Data, Off.EscapeFloats + 4, Values.EscapeField90);
        KyaWriteF32(Data, Off.EscapeFloats + 8, Values.EscapeField8C);
        KyaWriteF32(Data, Off.EscapeFloats + 12, Values.EscapeField94);
        KyaWriteF32(Data, Off.EscapeFloats + 16, Values.EscapeField98);
        KyaWriteU32(Data, Off.EscapeFieldA0, uint32(Values.EscapeFieldA0));
        if (!WriteSwitchScript(Off.Switch)) return false;
    }
    if (Off.bSnipe)
    {
        KyaWriteU32(Data, Off.SnipeField90, uint32(Values.SnipeField90));
        KyaWriteF32(Data, Off.SnipeField94, Values.SnipeField94);
        KyaWriteF32(Data, Off.SnipeField94 + 4, Values.SnipeField98);
        KyaWriteF32(Data, Off.SnipeField94 + 8, Values.SnipeField9C);
        KyaWriteF32(Data, Off.SnipeField94 + 12, Values.SnipeFieldA0);
        KyaWriteU32(Data, Off.SpotTrack, uint32(Values.SpotTrackBehaviourId));
        KyaWriteU32(Data, Off.SnipeTrack, uint32(Values.SnipeTrackBehaviourId));
        KyaWriteU32(Data, Off.ShadowMaterial, uint32(Values.ShadowMaterialId));
        KyaWriteU32(Data, Off.SnipeFieldAC, uint32(Values.SnipeFieldAC));
        KyaWriteF32(Data, Off.SnipeFieldB0, Values.SnipeFieldB0);
        KyaWriteF32(Data, Off.SnipeFieldB4, Values.SnipeFieldB4);
        KyaWriteU32(Data, Off.NbSpot, uint32(Values.NbSpot));
        KyaWriteU32(Data, Off.NbShadow, uint32(Values.NbShadow));
        KyaWriteF32(Data, Off.RotationSpeed, Values.RotationSpeed);
        KyaWriteU32(Data, Off.SnipeFieldB8, uint32(Values.SnipeFieldB8));
        if (!WriteSwitchScript(Off.Switch)) return false;
    }
    return true;
}
