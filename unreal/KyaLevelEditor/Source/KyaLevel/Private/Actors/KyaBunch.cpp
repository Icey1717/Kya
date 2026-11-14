#include "Actors/KyaBunch.h"

#include "Actors/KyaWolfen.h"
#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadBunchOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBunchOffsets& Out)
{
    Out = FKyaBunchOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };

    FKyaWolfenOffsets Wolfen;
    if (!KyaReadWolfenOffsets(Data, Start, Size, Wolfen)) return false;
    int64 Pos = Wolfen.Tail;
    Out.WolfenEnd = Pos;
    if (End - Pos < 12) return false;
    Out.FieldFC8 = Pos;
    Pos += 4;
    Out.EntryCount = Pos;
    const uint32 Entries = U32(Pos);
    Out.EntryCountValue = int32(Entries);
    Pos += 4;
    for (uint32 Index = 0; Index < Entries; ++Index)
    {
        if (End - Pos < 16) return false;
        FKyaBunchStreamEntryOffsets& Entry = Out.EntriesLayout.AddDefaulted_GetRef();
        Entry.Field0 = Pos;
        Entry.HasBehaviour = Pos + 4;
        Pos += 8;
        if (U32(Entry.HasBehaviour) != 0)
        {
            if (End - Pos < 4) return false;
            Entry.BehaviourId = Pos;
            Entry.BehaviourIdValue = int32(U32(Pos));
            Pos += 4;
        }
        else
        {
            Entry.BehaviourIdValue = -1;
        }
        if (End - Pos < 8) return false;
        Entry.ZoneId = Pos;
        Entry.FieldC = Pos + 4;
        Pos += 8;
    }
    if (End - Pos < 8) return false;
    Out.MeshComponent = Pos;
    Out.MeshIndex = Pos + 4;
    Out.Tail = Pos + 8;
    return true;
}

bool KyaReadBunchActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    FKyaBunchOffsets Off;
    if (!KyaReadBunchOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.BunchFieldFC8 = KyaReadF32(Bytes + Off.FieldFC8);

    Out.BunchStreamEntries.Reset();
    for (const FKyaBunchStreamEntryOffsets& Layout : Off.EntriesLayout)
    {
        FKyaBunchStreamEntry& Entry = Out.BunchStreamEntries.AddDefaulted_GetRef();
        Entry.Field0 = int32(KyaReadU32(Bytes + Layout.Field0));
        Entry.HasBehaviour = int32(KyaReadU32(Bytes + Layout.HasBehaviour));
        Entry.BehaviourId = Layout.BehaviourIdValue;
        Entry.ZoneId = int32(KyaReadU32(Bytes + Layout.ZoneId));
        Entry.FieldC = int32(KyaReadU32(Bytes + Layout.FieldC));
    }
    Out.BunchMeshComponentId = int32(KyaReadU32(Bytes + Off.MeshComponent));
    Out.BunchMeshIndex = int32(KyaReadU32(Bytes + Off.MeshIndex));

    Out.bHasBunchData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteBunchActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    FKyaBunchOffsets Off;
    if (!KyaReadBunchOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.BunchStreamEntries.Num() != Off.EntryCountValue) return false;

    KyaWriteF32(Data, Off.FieldFC8, Actor.BunchFieldFC8);
    for (int32 Index = 0; Index < Actor.BunchStreamEntries.Num(); ++Index)
    {
        const FKyaBunchStreamEntry& Entry = Actor.BunchStreamEntries[Index];
        const FKyaBunchStreamEntryOffsets& Layout = Off.EntriesLayout[Index];
        KyaWriteU32(Data, Layout.Field0, uint32(Entry.Field0));
        const uint32 HasBehaviour = uint32(Entry.HasBehaviour);
        KyaWriteU32(Data, Layout.HasBehaviour, HasBehaviour);
        if (Layout.BehaviourId)
            KyaWriteU32(Data, Layout.BehaviourId, uint32(Entry.BehaviourId));
        KyaWriteU32(Data, Layout.ZoneId, uint32(Entry.ZoneId));
        KyaWriteU32(Data, Layout.FieldC, uint32(Entry.FieldC));
    }
    KyaWriteU32(Data, Off.MeshComponent, uint32(Actor.BunchMeshComponentId));
    KyaWriteU32(Data, Off.MeshIndex, uint32(Actor.BunchMeshIndex));
    return true;
}

namespace
{
bool ParseBunchRepel(TConstArrayView<uint8> Data, int64 Start, int32 Size,
    FKyaBunchRepelOffsets& Out)
{
    Out = FKyaBunchRepelOffsets{};
    if (!KyaRangeValid(Data, Start, int64(Size)) || Size < 80) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    int64 Pos = Start;
    Out.WayPointRef = Pos; Pos += 4;
    Out.ShockWaveCount = Pos;
    const uint32 ShockWaves = U32(Pos);
    Out.ShockWaveCountValue = int32(ShockWaves);
    Pos += 4;
    if (End - Pos < 12) return false;
    Out.ShockWaveFloats = Pos; Pos += 12;
    Out.IndexCount = Pos;
    const uint32 Indices = U32(Pos);
    Out.IndexCountValue = int32(Indices);
    Pos += 4;
    if (uint64(Indices) > uint64(End - Pos) / 4) return false;
    Out.Indices = Pos;
    Pos += int64(Indices) * 4;
    if (End - Pos < 36) return false;
    Out.MsgHit = Pos; Pos += 12;
    Out.Vector20 = Pos; Pos += 12;
    Out.Vector30 = Pos; Pos += 12;
    if (End - Pos < 20) return false;
    Out.MeshComponent = Pos;
    Out.MeshIndex = Pos + 4;
    Out.MeshType = Pos + 8;
    Out.Reserved = Pos + 12;
    Out.DefaultId = Pos + 16;
    Out.Tail = Pos + 20;
    return true;
}

bool ParseBunchFence(TConstArrayView<uint8> Data, int64 Start, int32 Size,
    FKyaBunchFenceOffsets& Out)
{
    Out = FKyaBunchFenceOffsets{};
    if (!KyaRangeValid(Data, Start, int64(Size)) || Size < 44) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    int64 Pos = Start;
    Out.ActorRef = Pos; Pos += 4;
    Out.WayPointRef = Pos; Pos += 4;
    Out.WayPointCount = Pos;
    const uint32 WayPoints = U32(Pos);
    Out.WayPointCountValue = int32(WayPoints);
    Pos += 4;
    if (uint64(WayPoints) > uint64(End - Pos) / 4) return false;
    Out.WayPoints = Pos;
    Pos += int64(WayPoints) * 4;
    if (End - Pos < 4) return false;
    Out.FenceCount = Pos;
    const uint32 Fences = U32(Pos);
    Out.FenceCountValue = int32(Fences);
    Pos += 4;
    for (uint32 Index = 0; Index < Fences; ++Index)
    {
        if (End - Pos < 12) return false;
        FKyaBunchFenceGroupOffsets& Group = Out.Groups.AddDefaulted_GetRef();
        Group.Header = Pos;
        const uint32 Entries = U32(Pos + 8);
        Group.EntryCountValue = int32(Entries);
        Pos += 12;
        if (uint64(Entries) > uint64(End - Pos) / 8) return false;
        for (uint32 Entry = 0; Entry < Entries; ++Entry)
        {
            FKyaBunchFenceEntryOffsets& Item = Group.Entries.AddDefaulted_GetRef();
            Item.Ref = Pos;
            Item.Field4 = Pos + 4;
            Pos += 8;
        }
    }
    if (End - Pos < 32) return false;
    Out.MeshF0 = Pos; Pos += 8;
    Out.Mesh70 = Pos; Pos += 8;
    Out.FxTypes = Pos; Pos += 16;
    Out.Tail = Pos;
    return true;
}
}

bool KyaReadBunchBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Start < 0 || !KyaRangeValid(Data, Start, Size)) return false;
    if (Id == 26)
    {
        FKyaBunchRepelOffsets Off;
        if (!ParseBunchRepel(Data, Start, Size, Off)) return false;
        const uint8* Bytes = Data.GetData();
        Out.BunchRepelWayPointRef = int32(KyaReadU32(Bytes + Off.WayPointRef));
        Out.BunchRepelShockWaveCount = Off.ShockWaveCountValue;
        Out.BunchRepelField14 = KyaReadF32(Bytes + Off.ShockWaveFloats);
        Out.BunchRepelShockWaveField10 = KyaReadF32(Bytes + Off.ShockWaveFloats + 4);
        Out.BunchRepelShockWaveField14 = KyaReadF32(Bytes + Off.ShockWaveFloats + 8);
        Out.BunchRepelIndices.Reset();
        for (int32 Index = 0; Index < Off.IndexCountValue; ++Index)
            Out.BunchRepelIndices.Add(int32(KyaReadU32(Bytes + Off.Indices + int64(Index) * 4)));
        Out.BunchRepelDamage = KyaReadF32(Bytes + Off.MsgHit);
        Out.BunchRepelMsgHitField30 = KyaReadF32(Bytes + Off.MsgHit + 4);
        Out.BunchRepelMsgHitField70 = KyaReadF32(Bytes + Off.MsgHit + 8);
        Out.BunchRepelVector20 = FVector(KyaReadF32(Bytes + Off.Vector20),
            KyaReadF32(Bytes + Off.Vector20 + 4), KyaReadF32(Bytes + Off.Vector20 + 8));
        Out.BunchRepelVector30 = FVector(KyaReadF32(Bytes + Off.Vector30),
            KyaReadF32(Bytes + Off.Vector30 + 4), KyaReadF32(Bytes + Off.Vector30 + 8));
        Out.BunchRepelMeshComponentId = int32(KyaReadU32(Bytes + Off.MeshComponent));
        Out.BunchRepelMeshIndex = int32(KyaReadU32(Bytes + Off.MeshIndex));
        Out.BunchRepelMeshType = int32(KyaReadU32(Bytes + Off.MeshType));
        Out.BunchRepelReserved = int32(KyaReadU32(Bytes + Off.Reserved));
        Out.BunchRepelDefaultId = int32(KyaReadU32(Bytes + Off.DefaultId));
        Out.bHasTypedBunchRepel = true;
        return true;
    }
    if (Id == 27)
    {
        FKyaBunchFenceOffsets Off;
        if (!ParseBunchFence(Data, Start, Size, Off)) return false;
        const uint8* Bytes = Data.GetData();
        Out.BunchFenceActorRef = int32(KyaReadU32(Bytes + Off.ActorRef));
        Out.BunchFenceWayPointRef = int32(KyaReadU32(Bytes + Off.WayPointRef));
        Out.BunchFenceWayPoints.Reset();
        for (int32 Index = 0; Index < Off.WayPointCountValue; ++Index)
            Out.BunchFenceWayPoints.Add(int32(KyaReadU32(Bytes + Off.WayPoints + int64(Index) * 4)));
        Out.BunchFenceGroups.Reset();
        for (const FKyaBunchFenceGroupOffsets& Layout : Off.Groups)
        {
            FKyaBunchFenceGroup& Group = Out.BunchFenceGroups.AddDefaulted_GetRef();
            Group.Field0 = int32(KyaReadU32(Bytes + Layout.Header));
            Group.Field4 = int32(KyaReadU32(Bytes + Layout.Header + 4));
            for (const FKyaBunchFenceEntryOffsets& EntryLayout : Layout.Entries)
            {
                FKyaBunchFenceEntry& Entry = Group.Entries.AddDefaulted_GetRef();
                Entry.Ref = int32(KyaReadU32(Bytes + EntryLayout.Ref));
                Entry.Field4 = int32(KyaReadU32(Bytes + EntryLayout.Field4));
            }
        }
        Out.BunchFenceMeshF0Texture = int32(KyaReadU32(Bytes + Off.MeshF0));
        Out.BunchFenceMeshF0Mesh = int32(KyaReadU32(Bytes + Off.MeshF0 + 4));
        Out.BunchFenceMesh70Texture = int32(KyaReadU32(Bytes + Off.Mesh70));
        Out.BunchFenceMesh70Mesh = int32(KyaReadU32(Bytes + Off.Mesh70 + 4));
        Out.BunchFenceFxTypes.Reset();
        for (int32 Index = 0; Index < 4; ++Index)
            Out.BunchFenceFxTypes.Add(int32(KyaReadU32(Bytes + Off.FxTypes + int64(Index) * 4)));
        Out.bHasTypedBunchFence = true;
        return true;
    }
    return false;
}

bool KyaWriteBunchBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Start < 0 || !KyaRangeValid(Data, Start, Size)) return false;
    if (Behaviour.Id == 26)
    {
        FKyaBunchRepelOffsets Off;
        if (!ParseBunchRepel(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
            return false;
        if (Behaviour.BunchRepelIndices.Num() != Off.IndexCountValue) return false;
        KyaWriteU32(Data, Off.WayPointRef, uint32(Behaviour.BunchRepelWayPointRef));
        KyaWriteF32(Data, Off.ShockWaveFloats, Behaviour.BunchRepelField14);
        KyaWriteF32(Data, Off.ShockWaveFloats + 4, Behaviour.BunchRepelShockWaveField10);
        KyaWriteF32(Data, Off.ShockWaveFloats + 8, Behaviour.BunchRepelShockWaveField14);
        for (int32 Index = 0; Index < Behaviour.BunchRepelIndices.Num(); ++Index)
            KyaWriteU32(Data, Off.Indices + int64(Index) * 4, uint32(Behaviour.BunchRepelIndices[Index]));
        KyaWriteF32(Data, Off.MsgHit, Behaviour.BunchRepelDamage);
        KyaWriteF32(Data, Off.MsgHit + 4, Behaviour.BunchRepelMsgHitField30);
        KyaWriteF32(Data, Off.MsgHit + 8, Behaviour.BunchRepelMsgHitField70);
        KyaWriteF32(Data, Off.Vector20, float(Behaviour.BunchRepelVector20.X));
        KyaWriteF32(Data, Off.Vector20 + 4, float(Behaviour.BunchRepelVector20.Y));
        KyaWriteF32(Data, Off.Vector20 + 8, float(Behaviour.BunchRepelVector20.Z));
        KyaWriteF32(Data, Off.Vector30, float(Behaviour.BunchRepelVector30.X));
        KyaWriteF32(Data, Off.Vector30 + 4, float(Behaviour.BunchRepelVector30.Y));
        KyaWriteF32(Data, Off.Vector30 + 8, float(Behaviour.BunchRepelVector30.Z));
        KyaWriteU32(Data, Off.MeshComponent, uint32(Behaviour.BunchRepelMeshComponentId));
        KyaWriteU32(Data, Off.MeshIndex, uint32(Behaviour.BunchRepelMeshIndex));
        KyaWriteU32(Data, Off.MeshType, uint32(Behaviour.BunchRepelMeshType));
        KyaWriteU32(Data, Off.Reserved, uint32(Behaviour.BunchRepelReserved));
        KyaWriteU32(Data, Off.DefaultId, uint32(Behaviour.BunchRepelDefaultId));
        return true;
    }
    if (Behaviour.Id == 27)
    {
        FKyaBunchFenceOffsets Off;
        if (!ParseBunchFence(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
            return false;
        if (Behaviour.BunchFenceWayPoints.Num() != Off.WayPointCountValue) return false;
        if (Behaviour.BunchFenceGroups.Num() != Off.FenceCountValue) return false;
        for (int32 Group = 0; Group < Behaviour.BunchFenceGroups.Num(); ++Group)
            if (Behaviour.BunchFenceGroups[Group].Entries.Num() != Off.Groups[Group].EntryCountValue)
                return false;
        if (Behaviour.BunchFenceFxTypes.Num() != 4) return false;
        KyaWriteU32(Data, Off.ActorRef, uint32(Behaviour.BunchFenceActorRef));
        KyaWriteU32(Data, Off.WayPointRef, uint32(Behaviour.BunchFenceWayPointRef));
        for (int32 Index = 0; Index < Behaviour.BunchFenceWayPoints.Num(); ++Index)
            KyaWriteU32(Data, Off.WayPoints + int64(Index) * 4, uint32(Behaviour.BunchFenceWayPoints[Index]));
        for (int32 Group = 0; Group < Behaviour.BunchFenceGroups.Num(); ++Group)
        {
            const FKyaBunchFenceGroup& Data2 = Behaviour.BunchFenceGroups[Group];
            const FKyaBunchFenceGroupOffsets& Layout = Off.Groups[Group];
            KyaWriteU32(Data, Layout.Header, uint32(Data2.Field0));
            KyaWriteU32(Data, Layout.Header + 4, uint32(Data2.Field4));
            for (int32 Entry = 0; Entry < Data2.Entries.Num(); ++Entry)
            {
                KyaWriteU32(Data, Layout.Entries[Entry].Ref, uint32(Data2.Entries[Entry].Ref));
                KyaWriteU32(Data, Layout.Entries[Entry].Field4, uint32(Data2.Entries[Entry].Field4));
            }
        }
        KyaWriteU32(Data, Off.MeshF0, uint32(Behaviour.BunchFenceMeshF0Texture));
        KyaWriteU32(Data, Off.MeshF0 + 4, uint32(Behaviour.BunchFenceMeshF0Mesh));
        KyaWriteU32(Data, Off.Mesh70, uint32(Behaviour.BunchFenceMesh70Texture));
        KyaWriteU32(Data, Off.Mesh70 + 4, uint32(Behaviour.BunchFenceMesh70Mesh));
        for (int32 Index = 0; Index < 4; ++Index)
            KyaWriteU32(Data, Off.FxTypes + int64(Index) * 4, uint32(Behaviour.BunchFenceFxTypes[Index]));
        return true;
    }
    return false;
}
