#include "Actors/KyaClusteriser.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadClusterZoneOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    TArray<FKyaClusterZoneOffsets>& OutZones)
{
    OutZones.Reset();
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    const uint32 Count = KyaReadU32(Bytes + Start);
    int64 Pos = Start + 4;
    if (Count > uint64(End - Pos) / 32) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (End - Pos < 8) return false;
        FKyaClusterZoneOffsets Zone;
        Zone.Header = Pos;
        Pos += 8;
        for (int32 List = 0; List < 5; ++List)
        {
            if (End - Pos < 4) return false;
            const uint32 Num = KyaReadU32(Bytes + Pos);
            Pos += 4;
            const int32 Stride = List == 4 ? 8 : 4;
            if (Num > uint64(MAX_int32) || Num > uint64(End - Pos) / Stride) return false;
            Zone.Lists[List] = Pos;
            Zone.Counts[List] = int32(Num);
            Pos += int64(Num) * Stride;
        }
        if (End - Pos < 4) return false;
        Zone.Tail = Pos;
        Pos += 4;
        OutZones.Add(Zone);
    }
    return true;
}

bool KyaReadClusteriserBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Id != 2) return false;
    TArray<FKyaClusterZoneOffsets> Zones;
    if (!KyaReadClusterZoneOffsets(Data, Start, Size, Zones)) return false;
    const uint8* Bytes = Data.GetData();
    Out.ClusterZones.Reset();
    for (const FKyaClusterZoneOffsets& Offsets : Zones)
    {
        FKyaClusterZone& Zone = Out.ClusterZones.AddDefaulted_GetRef();
        Zone.Flags = int64(KyaReadU32(Bytes + Offsets.Header));
        Zone.ZoneRef = int32(KyaReadU32(Bytes + Offsets.Header + 4));
        TArray<int32>* Lists[] = { &Zone.Actors, &Zone.OtherActors, &Zone.Cameras, &Zone.Lights };
        for (int32 List = 0; List < 4; ++List)
            for (int32 Item = 0; Item < Offsets.Counts[List]; ++Item)
                Lists[List]->Add(int32(KyaReadU32(Bytes + Offsets.Lists[List] + int64(Item) * 4)));
        for (int32 Item = 0; Item < Offsets.Counts[4]; ++Item)
        {
            const int64 Hash = Offsets.Lists[4] + int64(Item) * 8;
            Zone.Hashes.Add(int64(uint64(KyaReadU32(Bytes + Hash)) |
                (uint64(KyaReadU32(Bytes + Hash + 4)) << 32)));
        }
        Zone.OtherZoneRef = int32(KyaReadU32(Bytes + Offsets.Tail));
    }
    Out.bHasTypedClusterZones = true;
    return true;
}

bool KyaWriteClusteriserBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id != 2) return false;
    TArray<FKyaClusterZoneOffsets> Zones;
    if (!KyaReadClusterZoneOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size,
        Zones)) return false;
    if (Behaviour.ClusterZones.Num() != Zones.Num()) return false;
    for (int32 Index = 0; Index < Behaviour.ClusterZones.Num(); ++Index)
    {
        const FKyaClusterZone& Zone = Behaviour.ClusterZones[Index];
        const FKyaClusterZoneOffsets& Offsets = Zones[Index];
        const int32 Counts[] = { Zone.Actors.Num(), Zone.OtherActors.Num(), Zone.Cameras.Num(),
            Zone.Lights.Num(), Zone.Hashes.Num() };
        if (Zone.Flags < 0 || Zone.Flags > MAX_uint32) return false;
        for (int32 List = 0; List < 5; ++List)
            if (Counts[List] != Offsets.Counts[List]) return false;
        KyaWriteU32(Data, Offsets.Header, uint32(Zone.Flags));
        KyaWriteU32(Data, Offsets.Header + 4, uint32(Zone.ZoneRef));
        const TArray<int32>* Lists[] = { &Zone.Actors, &Zone.OtherActors, &Zone.Cameras,
            &Zone.Lights };
        for (int32 List = 0; List < 4; ++List)
            for (int32 Item = 0; Item < Lists[List]->Num(); ++Item)
                KyaWriteU32(Data, Offsets.Lists[List] + int64(Item) * 4, uint32((*Lists[List])[Item]));
        for (int32 Item = 0; Item < Zone.Hashes.Num(); ++Item)
        {
            const uint64 Hash = uint64(Zone.Hashes[Item]);
            KyaWriteU32(Data, Offsets.Lists[4] + int64(Item) * 8, uint32(Hash));
            KyaWriteU32(Data, Offsets.Lists[4] + int64(Item) * 8 + 4, uint32(Hash >> 32));
        }
        KyaWriteU32(Data, Offsets.Tail, uint32(Zone.OtherZoneRef));
    }
    return true;
}
