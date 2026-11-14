#include "Actors/KyaBrazul.h"

#include "Actors/KyaWolfen.h"
#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadBrazulOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBrazulOffsets& Out)
{
    Out = FKyaBrazulOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    FKyaWolfenOffsets Wolfen;
    if (!KyaReadWolfenOffsets(Data, Start, Size, Wolfen)) return false;
    const int64 End = Start + Size;
    int64 Pos = Wolfen.Tail;
    Out.WolfenEnd = Pos;
    if (End - Pos < 64) return false;
    Out.Floats = Pos; Pos += 8;
    Out.PathIndex = Pos; Pos += 4;
    Out.Field1060 = Pos; Pos += 4;
    Out.Reserved = Pos; Pos += 4;
    Out.MaterialId = Pos; Pos += 4;
    Out.SparkParams = Pos; Pos += 8;
    Out.FxHandles = Pos; Pos += 12; // three CFxHandleExt ids
    Out.FxInit = Pos; Pos += 4;
    Pos += 8; // two more CFxHandleExt ids
    Out.MeshA = Pos; Pos += 8;
    Out.MeshB = Pos; Pos += 8;
    Out.Tail = Pos;
    return true;
}

bool KyaReadBrazulActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    FKyaBrazulOffsets Off;
    if (!KyaReadBrazulOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.BrazulFieldFC0 = KyaReadF32(Bytes + Off.Floats);
    Out.BrazulFieldFC4 = KyaReadF32(Bytes + Off.Floats + 4);
    Out.BrazulPathIndex = int32(KyaReadU32(Bytes + Off.PathIndex));
    Out.BrazulField1060 = int32(KyaReadU32(Bytes + Off.Field1060));
    Out.BrazulReserved = int32(KyaReadU32(Bytes + Off.Reserved));
    Out.BrazulMaterialId = int32(KyaReadU32(Bytes + Off.MaterialId));
    Out.BrazulSparkParamA = int32(KyaReadU32(Bytes + Off.SparkParams));
    Out.BrazulSparkParamB = int32(KyaReadU32(Bytes + Off.SparkParams + 4));
    Out.BrazulFxHandles.Reset();
    for (int32 Index = 0; Index < 3; ++Index)
        Out.BrazulFxHandles.Add(int32(KyaReadU32(Bytes + Off.FxHandles + int64(Index) * 4)));
    Out.BrazulFxInit = int32(KyaReadU32(Bytes + Off.FxInit));
    for (int32 Index = 0; Index < 2; ++Index)
        Out.BrazulFxHandles.Add(int32(KyaReadU32(Bytes + Off.FxInit + 4 + int64(Index) * 4)));
    Out.BrazulMeshATexture = int32(KyaReadU32(Bytes + Off.MeshA));
    Out.BrazulMeshAMesh = int32(KyaReadU32(Bytes + Off.MeshA + 4));
    Out.BrazulMeshBTexture = int32(KyaReadU32(Bytes + Off.MeshB));
    Out.BrazulMeshBMesh = int32(KyaReadU32(Bytes + Off.MeshB + 4));

    Out.bHasBrazulData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteBrazulActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    FKyaBrazulOffsets Off;
    if (!KyaReadBrazulOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.BrazulFxHandles.Num() != 5) return false;
    KyaWriteF32(Data, Off.Floats, Actor.BrazulFieldFC0);
    KyaWriteF32(Data, Off.Floats + 4, Actor.BrazulFieldFC4);
    KyaWriteU32(Data, Off.PathIndex, uint32(Actor.BrazulPathIndex));
    KyaWriteU32(Data, Off.Field1060, uint32(Actor.BrazulField1060));
    KyaWriteU32(Data, Off.Reserved, uint32(Actor.BrazulReserved));
    KyaWriteU32(Data, Off.MaterialId, uint32(Actor.BrazulMaterialId));
    KyaWriteU32(Data, Off.SparkParams, uint32(Actor.BrazulSparkParamA));
    KyaWriteU32(Data, Off.SparkParams + 4, uint32(Actor.BrazulSparkParamB));
    for (int32 Index = 0; Index < 3; ++Index)
        KyaWriteU32(Data, Off.FxHandles + int64(Index) * 4, uint32(Actor.BrazulFxHandles[Index]));
    KyaWriteU32(Data, Off.FxInit, uint32(Actor.BrazulFxInit));
    for (int32 Index = 0; Index < 2; ++Index)
        KyaWriteU32(Data, Off.FxInit + 4 + int64(Index) * 4, uint32(Actor.BrazulFxHandles[3 + Index]));
    KyaWriteU32(Data, Off.MeshA, uint32(Actor.BrazulMeshATexture));
    KyaWriteU32(Data, Off.MeshA + 4, uint32(Actor.BrazulMeshAMesh));
    KyaWriteU32(Data, Off.MeshB, uint32(Actor.BrazulMeshBTexture));
    KyaWriteU32(Data, Off.MeshB + 4, uint32(Actor.BrazulMeshBMesh));
    return true;
}

namespace
{
bool ParseBrazulA(TConstArrayView<uint8> Data, int64 Start, int32 Size,
    FKyaBrazulAOffsets& Out)
{
    Out = FKyaBrazulAOffsets{};
    if (!KyaRangeValid(Data, Start, int64(Size)) || Size < 4) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    int64 Pos = Start;
    Out.Count = Pos;
    const uint32 Count = KyaReadU32(Bytes + Pos);
    Out.CountValue = int32(Count);
    Pos += 4;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (End - Pos < 24) return false;
        FKyaBrazulSubObjOffsets& Sub = Out.SubObjs.AddDefaulted_GetRef();
        Sub.Header = Pos;
        const uint32 Paths = KyaReadU32(Bytes + Pos + 20);
        Sub.PathCount = int32(Paths);
        Pos += 24;
        if (uint64(Paths) > uint64(End - Pos) / 4) return false;
        Sub.PathIndices = Pos;
        Pos += int64(Paths) * 4;
    }
    return true;
}

bool ParseBrazulB(TConstArrayView<uint8> Data, int64 Start, int32 Size,
    FKyaBrazulBOffsets& Out)
{
    Out = FKyaBrazulBOffsets{};
    if (!KyaRangeValid(Data, Start, int64(Size)) || Size < 40) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    int64 Pos = Start;
    Out.WayPointRef = Pos; Pos += 4;
    const uint32 Targets = KyaReadU32(Bytes + Pos);
    Out.SwitchCount = int32(Targets);
    Out.SwitchTargets = Pos + 4;
    Pos += 4;
    if (uint64(Targets) > uint64(End - Pos) / 28) return false;
    Out.SwitchCamera = Pos + int64(Targets) * 28;
    if (End - Out.SwitchCamera < 32) return false;
    Out.Tail = Out.SwitchCamera + 32;
    return true;
}
}

bool KyaReadBrazulBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Start < 0 || !KyaRangeValid(Data, Start, Size)) return false;
    if (Id == 26)
    {
        FKyaBrazulAOffsets Off;
        if (!ParseBrazulA(Data, Start, Size, Off)) return false;
        const uint8* Bytes = Data.GetData();
        Out.BrazulSubObjs.Reset();
        for (const FKyaBrazulSubObjOffsets& Layout : Off.SubObjs)
        {
            FKyaBrazulSubObj& Sub = Out.BrazulSubObjs.AddDefaulted_GetRef();
            Sub.Field0 = int32(KyaReadU32(Bytes + Layout.Header));
            Sub.Field4 = int32(KyaReadU32(Bytes + Layout.Header + 4));
            Sub.Field8 = KyaReadF32(Bytes + Layout.Header + 8);
            Sub.FieldC = int32(KyaReadU32(Bytes + Layout.Header + 12));
            Sub.Field10 = int32(KyaReadU32(Bytes + Layout.Header + 16));
            for (int32 Index = 0; Index < Layout.PathCount; ++Index)
                Sub.PathIndices.Add(int32(KyaReadU32(Bytes + Layout.PathIndices + int64(Index) * 4)));
        }
        Out.bHasTypedBrazulA = true;
        return true;
    }
    if (Id == 27)
    {
        FKyaBrazulBOffsets Off;
        if (!ParseBrazulB(Data, Start, Size, Off)) return false;
        const uint8* Bytes = Data.GetData();
        Out.BrazulBWayPointRef = int32(KyaReadU32(Bytes + Off.WayPointRef));
        Out.BrazulBSwitch = FKyaSwitch{};
        if (!KyaReadSwitchValue(Data, Off.SwitchTargets, Off.SwitchCount, Off.SwitchCamera,
            Out.BrazulBSwitch)) return false;
        Out.bHasTypedBrazulB = true;
        return true;
    }
    return false;
}

bool KyaWriteBrazulBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Start < 0 || !KyaRangeValid(Data, Start, Size)) return false;
    if (Behaviour.Id == 26)
    {
        FKyaBrazulAOffsets Off;
        if (!ParseBrazulA(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
            return false;
        if (Behaviour.BrazulSubObjs.Num() != Off.CountValue) return false;
        for (int32 Index = 0; Index < Behaviour.BrazulSubObjs.Num(); ++Index)
            if (Behaviour.BrazulSubObjs[Index].PathIndices.Num() != Off.SubObjs[Index].PathCount)
                return false;
        for (int32 Index = 0; Index < Behaviour.BrazulSubObjs.Num(); ++Index)
        {
            const FKyaBrazulSubObj& Sub = Behaviour.BrazulSubObjs[Index];
            const FKyaBrazulSubObjOffsets& Layout = Off.SubObjs[Index];
            KyaWriteU32(Data, Layout.Header, uint32(Sub.Field0));
            KyaWriteU32(Data, Layout.Header + 4, uint32(Sub.Field4));
            KyaWriteF32(Data, Layout.Header + 8, Sub.Field8);
            KyaWriteU32(Data, Layout.Header + 12, uint32(Sub.FieldC));
            KyaWriteU32(Data, Layout.Header + 16, uint32(Sub.Field10));
            for (int32 Path = 0; Path < Sub.PathIndices.Num(); ++Path)
                KyaWriteU32(Data, Layout.PathIndices + int64(Path) * 4, uint32(Sub.PathIndices[Path]));
        }
        return true;
    }
    if (Behaviour.Id == 27)
    {
        FKyaBrazulBOffsets Off;
        if (!ParseBrazulB(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
            return false;
        if (Behaviour.BrazulBSwitch.Targets.Num() != Off.SwitchCount) return false;
        KyaWriteU32(Data, Off.WayPointRef, uint32(Behaviour.BrazulBWayPointRef));
        KyaWriteSwitchValue(Data, Behaviour.BrazulBSwitch, Off.SwitchTargets, Off.SwitchCamera);
        return true;
    }
    return false;
}
