#include "Actors/KyaWind.h"

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

bool KyaRangeValid(const TArray<uint8>& Data, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 0 && Start <= Data.Num() && Size <= Data.Num() - Start;
}
}

bool KyaReadWindOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaWindOffsets& Out)
{
    Out = FKyaWindOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 60) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    FKyaWindOffsets Result;
    int64 Pos = Start;
    Result.Field160 = Pos; Result.MaxWind = Pos + 4; Result.Field168 = Pos + 8;
    Result.Field16C = Pos + 12; Result.Field174 = Pos + 16; Result.Field180 = Pos + 20;
    Result.Field184 = Pos + 24; Result.Field188 = Pos + 28; Result.Field18C = Pos + 32;
    Result.Field178 = Pos + 36; Result.Field17C = Pos + 40; Result.Field190 = Pos + 44;
    Result.Sound = Pos + 48; Result.ActivationZone = Pos + 52;
    const uint32 ActorCount = KyaReadU32(Bytes + Pos + 56);
    Pos += 60;
    if (ActorCount > uint64(End - Pos) / 4) return false;
    Result.ActorCount = int32(ActorCount); Result.Actors = Pos; Pos += int64(ActorCount) * 4;
    if (End - Pos < 24) return false;
    Result.Material = Pos; Result.Zone1C4 = Pos + 4; Result.Color1C8 = Pos + 8;
    Result.Waypoint1CC = Pos + 12; Result.SectorHash = Pos + 16;
    Pos += 24;
    // The three switches exist only on levels at or above scene version 2.2.
    // Their absence is detected from the remaining size (three empty switches
    // measure 140 bytes).
    if (End - Pos >= 140)
    {
        Result.bHasSwitches = true;
        for (int32 Switch = 0; Switch < 3; ++Switch)
        {
            if (End - Pos < 36) return false;
            const uint32 Count = KyaReadU32(Bytes + Pos);
            Pos += 4;
            if (Count > uint64(End - Pos - 32) / 28) return false;
            FKyaWindEmbeddedSwitchOffsets& Offsets = Result.Switches[Switch];
            Offsets.Count = int32(Count); Offsets.Targets = Pos;
            Pos += int64(Count) * 28;
            Offsets.Camera = Pos; Pos += 32;
        }
    }
    else if (End - Pos < 32) return false;
    if (End - Pos < 32) return false;
    Result.Floats194 = Pos; Pos += 20;
    Result.Waypoint = Pos; Pos += 4;
    Result.Tail = Pos + 8;
    Out = Result;
    return true;
}

bool KyaReadWindActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaWindOffsets Off;
    if (!KyaReadWindOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.WindField160 = int32(KyaReadU32(Bytes + Off.Field160));
    Out.WindMaxWind = KyaReadF32(Bytes + Off.MaxWind);
    Out.WindField168 = KyaReadF32(Bytes + Off.Field168);
    Out.WindField16C = KyaReadF32(Bytes + Off.Field16C);
    Out.WindField174 = KyaReadF32(Bytes + Off.Field174);
    Out.WindField180 = KyaReadF32(Bytes + Off.Field180);
    Out.WindField184 = KyaReadF32(Bytes + Off.Field184);
    Out.WindField188 = KyaReadF32(Bytes + Off.Field188);
    Out.WindField18C = KyaReadF32(Bytes + Off.Field18C);
    Out.WindField178 = KyaReadF32(Bytes + Off.Field178);
    Out.WindField17C = KyaReadF32(Bytes + Off.Field17C);
    Out.WindField190 = int32(KyaReadU32(Bytes + Off.Field190));
    Out.WindSoundRef = int32(KyaReadU32(Bytes + Off.Sound));
    Out.WindActivationZoneIndex = int32(KyaReadU32(Bytes + Off.ActivationZone));
    Out.WindActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ActorCount; ++Index)
        Out.WindActorRefs.Add(int32(KyaReadU32(Bytes + Off.Actors + int64(Index) * 4)));
    Out.WindMaterialId = int32(KyaReadU32(Bytes + Off.Material));
    Out.WindZoneRef1C4 = int32(KyaReadU32(Bytes + Off.Zone1C4));
    Out.WindColor1C8 = int32(KyaReadU32(Bytes + Off.Color1C8));
    Out.WindWaypointRef1CC = int32(KyaReadU32(Bytes + Off.Waypoint1CC));
    Out.WindSectorHierarchyHash = int64(uint64(KyaReadU32(Bytes + Off.SectorHash)) |
        (uint64(KyaReadU32(Bytes + Off.SectorHash + 4)) << 32));

    FKyaSwitch* Switches[] = { &Out.WindSwitch1D0, &Out.WindSwitch1D8, &Out.WindSwitch1E0 };
    for (int32 Index = 0; Index < 3; ++Index)
    {
        *Switches[Index] = FKyaSwitch{};
        if (!Off.bHasSwitches) continue;
        if (!KyaReadSwitchValue(Data, Off.Switches[Index].Targets, Off.Switches[Index].Count,
            Off.Switches[Index].Camera, *Switches[Index])) return false;
    }

    Out.WindField194 = KyaReadF32(Bytes + Off.Floats194);
    Out.WindField198 = KyaReadF32(Bytes + Off.Floats194 + 4);
    Out.WindField19C = KyaReadF32(Bytes + Off.Floats194 + 8);
    Out.WindField1A0 = KyaReadF32(Bytes + Off.Floats194 + 12);
    Out.WindField1A4 = KyaReadF32(Bytes + Off.Floats194 + 16);
    Out.WindWaypointRef = int32(KyaReadU32(Bytes + Off.Waypoint));
    Out.WindField1AC = KyaReadF32(Bytes + Off.Waypoint + 4);
    Out.WindField1B0 = KyaReadF32(Bytes + Off.Waypoint + 8);

    Out.bHasWindData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteWindActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaWindOffsets Off;
    if (!KyaReadWindOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.WindActorRefs.Num() != Off.ActorCount) return false;

    KyaWriteU32(Data, Off.Field160, uint32(Actor.WindField160));
    KyaWriteF32(Data, Off.MaxWind, Actor.WindMaxWind);
    KyaWriteF32(Data, Off.Field168, Actor.WindField168);
    KyaWriteF32(Data, Off.Field16C, Actor.WindField16C);
    KyaWriteF32(Data, Off.Field174, Actor.WindField174);
    KyaWriteF32(Data, Off.Field180, Actor.WindField180);
    KyaWriteF32(Data, Off.Field184, Actor.WindField184);
    KyaWriteF32(Data, Off.Field188, Actor.WindField188);
    KyaWriteF32(Data, Off.Field18C, Actor.WindField18C);
    KyaWriteF32(Data, Off.Field178, Actor.WindField178);
    KyaWriteF32(Data, Off.Field17C, Actor.WindField17C);
    KyaWriteU32(Data, Off.Field190, uint32(Actor.WindField190));
    KyaWriteU32(Data, Off.Sound, uint32(Actor.WindSoundRef));
    KyaWriteU32(Data, Off.ActivationZone, uint32(Actor.WindActivationZoneIndex));
    for (int32 Index = 0; Index < Actor.WindActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.Actors + int64(Index) * 4, uint32(Actor.WindActorRefs[Index]));
    KyaWriteU32(Data, Off.Material, uint32(Actor.WindMaterialId));
    KyaWriteU32(Data, Off.Zone1C4, uint32(Actor.WindZoneRef1C4));
    KyaWriteU32(Data, Off.Color1C8, uint32(Actor.WindColor1C8));
    KyaWriteU32(Data, Off.Waypoint1CC, uint32(Actor.WindWaypointRef1CC));
    KyaWriteU32(Data, Off.SectorHash, uint32(uint64(Actor.WindSectorHierarchyHash)));
    KyaWriteU32(Data, Off.SectorHash + 4, uint32(uint64(Actor.WindSectorHierarchyHash) >> 32));

    const FKyaSwitch* Switches[] = { &Actor.WindSwitch1D0, &Actor.WindSwitch1D8, &Actor.WindSwitch1E0 };
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (!Off.bHasSwitches)
        {
            if (!Switches[Index]->Targets.IsEmpty()) return false;
            continue;
        }
        if (Switches[Index]->Targets.Num() != Off.Switches[Index].Count) return false;
        KyaWriteSwitchValue(Data, *Switches[Index], Off.Switches[Index].Targets,
            Off.Switches[Index].Camera);
    }

    KyaWriteF32(Data, Off.Floats194, Actor.WindField194);
    KyaWriteF32(Data, Off.Floats194 + 4, Actor.WindField198);
    KyaWriteF32(Data, Off.Floats194 + 8, Actor.WindField19C);
    KyaWriteF32(Data, Off.Floats194 + 12, Actor.WindField1A0);
    KyaWriteF32(Data, Off.Floats194 + 16, Actor.WindField1A4);
    KyaWriteU32(Data, Off.Waypoint, uint32(Actor.WindWaypointRef));
    KyaWriteF32(Data, Off.Waypoint + 4, Actor.WindField1AC);
    KyaWriteF32(Data, Off.Waypoint + 8, Actor.WindField1B0);
    return true;
}
