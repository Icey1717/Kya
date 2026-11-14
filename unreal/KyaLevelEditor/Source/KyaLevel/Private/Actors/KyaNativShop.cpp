#include "Actors/KyaNativShop.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadNativShopOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaNativShopOffsets& Out)
{
    Out = FKyaNativShopOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 24) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Radius = Start;
    int64 Pos = Start + 4;
    Out.StringStart = Pos;
    bool Terminated = false;
    while (Pos < End)
    {
        if (Bytes[Pos++] == 0)
        {
            Terminated = true;
            break;
        }
    }
    if (!Terminated) return false;
    Pos = (Pos + 3) & ~int64(3); // ByteCode::GetString aligns the seek to 4.
    if (Pos > End) return false;
    Out.StringEnd = Pos;
    if (End - Pos < 4) return false;
    Out.ActorCount = Pos;
    const uint32 Actors = KyaReadU32(Bytes + Pos);
    Out.ActorCountValue = int32(Actors);
    Out.ActorEntries = Pos + 4;
    Pos += 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Pos += int64(Actors) * 4;
    if (End - Pos < 12) return false;
    Out.EpisodeThresholdLink = Pos;
    Out.Field174 = Pos + 4;
    Out.ActorRef = Pos + 8;
    Out.Tail = Pos + 12;
    return true;
}

bool KyaReadNativShopActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    FKyaNativShopOffsets Off;
    if (!KyaReadNativShopOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.NativShopRadius = KyaReadF32(Bytes + Off.Radius);

    Out.NativShopName.Reset();
    for (int64 Pos = Off.StringStart; Pos < Off.StringEnd && Bytes[Pos] != 0; ++Pos)
        Out.NativShopName.AppendChar(TCHAR(Bytes[Pos]));

    Out.NativShopActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ActorCountValue; ++Index)
        Out.NativShopActorRefs.Add(int32(KyaReadU32(Bytes + Off.ActorEntries + int64(Index) * 4)));
    Out.NativShopEpisodeThresholdLink = int32(KyaReadU32(Bytes + Off.EpisodeThresholdLink));
    Out.NativShopField174 = int32(KyaReadU32(Bytes + Off.Field174));
    Out.NativShopActorRef = int32(KyaReadU32(Bytes + Off.ActorRef));

    Out.bHasNativShopData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteNativShopActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    FKyaNativShopOffsets Off;
    if (!KyaReadNativShopOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.NativShopActorRefs.Num() != Off.ActorCountValue) return false;

    const int64 SpanLength = Off.StringEnd - Off.StringStart;
    const int32 NameLength = Actor.NativShopName.Len();
    if (NameLength + 1 > SpanLength) return false;
    for (int32 Char = 0; Char < NameLength; ++Char)
        Data[int32(Off.StringStart) + Char] = uint8(Actor.NativShopName[Char]);
    for (int64 Pad = NameLength; Pad < SpanLength; ++Pad)
        Data[int32(Off.StringStart) + int32(Pad)] = 0;

    KyaWriteF32(Data, Off.Radius, Actor.NativShopRadius);
    for (int32 Index = 0; Index < Actor.NativShopActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.ActorEntries + int64(Index) * 4, uint32(Actor.NativShopActorRefs[Index]));
    KyaWriteU32(Data, Off.EpisodeThresholdLink, uint32(Actor.NativShopEpisodeThresholdLink));
    KyaWriteU32(Data, Off.Field174, uint32(Actor.NativShopField174));
    KyaWriteU32(Data, Off.ActorRef, uint32(Actor.NativShopActorRef));
    return true;
}

bool KyaReadNativShopBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Id != 2 || Start < 0 || Size != 52 || !KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    FKyaNativShopSellOffsets Off;
    Off.Camera = Start;
    Off.ActorRefs = Start + 4;
    Off.PathIndex = Start + 24;
    Off.MaterialId = Start + 28;
    Off.MusicIndex = Start + 32;
    Off.ZoneRef = Start + 36;
    Off.Floats = Start + 40;

    Out.NativShopCameraRef = int32(KyaReadU32(Bytes + Off.Camera));
    Out.NativShopSubObjActorRefs.Reset();
    for (int32 Index = 0; Index < 5; ++Index)
        Out.NativShopSubObjActorRefs.Add(int32(KyaReadU32(Bytes + Off.ActorRefs + int64(Index) * 4)));
    Out.NativShopPathIndex = int32(KyaReadU32(Bytes + Off.PathIndex));
    Out.NativShopMaterialId = int32(KyaReadU32(Bytes + Off.MaterialId));
    Out.NativShopMusicIndex = int32(KyaReadU32(Bytes + Off.MusicIndex));
    Out.NativShopZoneRef = int32(KyaReadU32(Bytes + Off.ZoneRef));
    Out.NativShopFieldC4 = KyaReadF32(Bytes + Off.Floats);
    Out.NativShopFieldC8 = KyaReadF32(Bytes + Off.Floats + 4);
    Out.NativShopFieldCC = KyaReadF32(Bytes + Off.Floats + 8);
    Out.bHasTypedNativShopBehaviour = true;
    return true;
}

bool KyaWriteNativShopBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id != 2 || Start < 0 || Size != 52 || !KyaRangeValid(Data, Start, Size))
        return false;
    if (Behaviour.NativShopSubObjActorRefs.Num() != 5) return false;
    KyaWriteU32(Data, Start, uint32(Behaviour.NativShopCameraRef));
    for (int32 Index = 0; Index < 5; ++Index)
        KyaWriteU32(Data, Start + 4 + int64(Index) * 4,
            uint32(Behaviour.NativShopSubObjActorRefs[Index]));
    KyaWriteU32(Data, Start + 24, uint32(Behaviour.NativShopPathIndex));
    KyaWriteU32(Data, Start + 28, uint32(Behaviour.NativShopMaterialId));
    KyaWriteU32(Data, Start + 32, uint32(Behaviour.NativShopMusicIndex));
    KyaWriteU32(Data, Start + 36, uint32(Behaviour.NativShopZoneRef));
    KyaWriteF32(Data, Start + 40, Behaviour.NativShopFieldC4);
    KyaWriteF32(Data, Start + 44, Behaviour.NativShopFieldC8);
    KyaWriteF32(Data, Start + 48, Behaviour.NativShopFieldCC);
    return true;
}
