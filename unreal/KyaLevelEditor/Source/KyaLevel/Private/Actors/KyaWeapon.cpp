#include "Actors/KyaWeapon.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadWeaponOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaWeaponOffsets& Out)
{
    Out = FKyaWeaponOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 12) return false;
    Out.WeightB = Start;
    Out.Field1D8 = Start + 4;
    Out.Field1DC = Start + 8;
    Out.Tail = Start + 12;
    return true;
}

bool KyaReadWeaponActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaWeaponOffsets Off;
    if (!KyaReadWeaponOffsets(Data, Start, Size, Off)) return false;
    Out.WeaponWeightB = KyaReadF32(Data.GetData() + Off.WeightB);
    Out.WeaponField1D8 = KyaReadF32(Data.GetData() + Off.Field1D8);
    Out.WeaponField1DC = KyaReadF32(Data.GetData() + Off.Field1DC);
    Out.bHasWeaponData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Data.GetData() + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteWeaponActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaWeaponOffsets Off;
    if (!KyaReadWeaponOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    KyaWriteF32(Data, Off.WeightB, Actor.WeaponWeightB);
    KyaWriteF32(Data, Off.Field1D8, Actor.WeaponField1D8);
    KyaWriteF32(Data, Off.Field1DC, Actor.WeaponField1DC);
    return true;
}

bool KyaReadWeaponBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start, int32 Size,
    FKyaWeaponBehaviourValues& Out)
{
    Out = FKyaWeaponBehaviourValues{};
    if ((Id != 3 && Id != 4 && Id != 6) || !KyaRangeValid(Data, Start, int64(Size)) ||
        Size < 32) return false;
    const uint8* Bytes = Data.GetData();
    for (int32 Index = 0; Index < 2; ++Index)
    {
        FKyaWeaponAdvancedEntry& Entry = Out.Burst.AddDefaulted_GetRef();
        const int64 E = Start + int64(Index) * 16;
        Entry.FieldC = KyaReadF32(Bytes + E);
        Entry.Field4 = int32(KyaReadU32(Bytes + E + 4));
        Entry.Field0 = KyaReadF32(Bytes + E + 8);
        Entry.Field8 = KyaReadF32(Bytes + E + 12);
    }
    if (Id == 3)
    {
        if (Size != 52) return false;
        Out.Field40 = int32(KyaReadU32(Bytes + Start + 32));
        Out.Field44 = int32(KyaReadU32(Bytes + Start + 36));
        Out.SoundIndex = int32(KyaReadU32(Bytes + Start + 40));
        Out.PistolField2E0 = int32(KyaReadU32(Bytes + Start + 44));
        Out.PistolField2E4 = int32(KyaReadU32(Bytes + Start + 48));
        return true;
    }
    if (Id == 4)
    {
        if (Size != 56) return false;
        Out.Field40 = int32(KyaReadU32(Bytes + Start + 32));
        Out.Field44 = int32(KyaReadU32(Bytes + Start + 36));
        Out.SoundIndex = int32(KyaReadU32(Bytes + Start + 40));
        Out.SniperTextureIndex = int32(KyaReadU32(Bytes + Start + 44));
        Out.SniperMeshIndex = int32(KyaReadU32(Bytes + Start + 48));
        Out.SniperMaterialId = int32(KyaReadU32(Bytes + Start + 52));
        return true;
    }
    // CBehaviourWeaponRpg: burst block + actor stream reference.
    if (Size < 36) return false;
    const uint32 Count = KyaReadU32(Bytes + Start + 32);
    if (Count > uint64(Size - 36) / 4 || Size != 36 + int64(Count) * 4) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
        Out.RpgActorRefs.Add(int32(KyaReadU32(Bytes + Start + 36 + int64(Index) * 4)));
    return true;
}

bool KyaReadWeaponBehaviourRecord(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    FKyaWeaponBehaviourValues Values;
    if (!KyaReadWeaponBehaviour(Id, Data, Start, Size, Values)) return false;
    Out.WeaponBurst = MoveTemp(Values.Burst);
    Out.WeaponField40 = Values.Field40;
    Out.WeaponField44 = Values.Field44;
    Out.WeaponSoundIndex = Values.SoundIndex;
    Out.WeaponPistolField2E0 = Values.PistolField2E0;
    Out.WeaponPistolField2E4 = Values.PistolField2E4;
    Out.WeaponSniperTextureIndex = Values.SniperTextureIndex;
    Out.WeaponSniperMeshIndex = Values.SniperMeshIndex;
    Out.WeaponSniperMaterialId = Values.SniperMaterialId;
    Out.WeaponRpgActorRefs = MoveTemp(Values.RpgActorRefs);
    Out.bHasTypedWeaponBehaviour = true;
    return true;
}

bool KyaWriteWeaponBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start,
    int32 Size)
{
    FKyaWeaponBehaviourValues Values;
    if (!KyaReadWeaponBehaviour(Behaviour.Id, MakeArrayView(Data.GetData(), Data.Num()), Start,
        Size, Values)) return false;
    if (Behaviour.WeaponBurst.Num() != 2) return false;
    for (int32 Index = 0; Index < 2; ++Index)
    {
        const FKyaWeaponAdvancedEntry& Entry = Behaviour.WeaponBurst[Index];
        const int64 E = Start + int64(Index) * 16;
        KyaWriteF32(Data, E, Entry.FieldC);
        KyaWriteU32(Data, E + 4, uint32(Entry.Field4));
        KyaWriteF32(Data, E + 8, Entry.Field0);
        KyaWriteF32(Data, E + 12, Entry.Field8);
    }
    if (Behaviour.Id == 3)
    {
        KyaWriteU32(Data, Start + 32, uint32(Behaviour.WeaponField40));
        KyaWriteU32(Data, Start + 36, uint32(Behaviour.WeaponField44));
        KyaWriteU32(Data, Start + 40, uint32(Behaviour.WeaponSoundIndex));
        KyaWriteU32(Data, Start + 44, uint32(Behaviour.WeaponPistolField2E0));
        KyaWriteU32(Data, Start + 48, uint32(Behaviour.WeaponPistolField2E4));
        return true;
    }
    if (Behaviour.Id == 4)
    {
        KyaWriteU32(Data, Start + 32, uint32(Behaviour.WeaponField40));
        KyaWriteU32(Data, Start + 36, uint32(Behaviour.WeaponField44));
        KyaWriteU32(Data, Start + 40, uint32(Behaviour.WeaponSoundIndex));
        KyaWriteU32(Data, Start + 44, uint32(Behaviour.WeaponSniperTextureIndex));
        KyaWriteU32(Data, Start + 48, uint32(Behaviour.WeaponSniperMeshIndex));
        KyaWriteU32(Data, Start + 52, uint32(Behaviour.WeaponSniperMaterialId));
        return true;
    }
    if (Behaviour.WeaponRpgActorRefs.Num() != Values.RpgActorRefs.Num()) return false;
    for (int32 Index = 0; Index < Behaviour.WeaponRpgActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Start + 36 + int64(Index) * 4, uint32(Behaviour.WeaponRpgActorRefs[Index]));
    return true;
}
