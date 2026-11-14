#include "Actors/KyaNoseMonster.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

namespace
{
bool ParseNoseMonster(TConstArrayView<uint8> Data, int64 Start, int64 Size, bool bVersionFloats,
    FKyaNoseMonsterOffsets& Out)
{
    Out = FKyaNoseMonsterOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };

    if (Size < 20) return false;
    int64 Pos = Start + 20; // CActorMovable weight + CActorAutonomous block.
    Out.AutonomousEnd = Pos;
    if (End - Pos < 44 + (bVersionFloats ? 8 : 0)) return false;
    Out.ZoneRef = Pos;
    Pos += 4;
    Out.Floats = Pos;
    Pos += 20;
    Out.bHasVersionFloats = bVersionFloats;
    if (bVersionFloats)
    {
        Out.VersionFloats = Pos;
        Pos += 8;
    }
    Out.Field364 = Pos;
    Out.Field370 = Pos + 4;
    Out.Field374 = Pos + 8;
    Pos += 12;
    if (End - Pos < 4) return false;
    Out.ZonePathCount = Pos;
    const uint32 ZonePaths = U32(Pos);
    Out.ZonePathCountValue = int32(ZonePaths);
    Pos += 4;
    if (uint64(ZonePaths) > uint64(End - Pos) / 8) return false;
    Out.ZonePathEntries = Pos;
    Pos += int64(ZonePaths) * 8;
    if (End - Pos < 8) return false;
    Out.Field36C = Pos;
    Pos += 4;
    Out.FireShotCount = Pos;
    const uint32 FireShots = U32(Pos);
    Out.FireShotCountValue = int32(FireShots);
    Pos += 4;
    if (uint64(FireShots) > uint64(End - Pos) / 4) return false;
    Out.FireShotEntries = Pos;
    Pos += int64(FireShots) * 4;
    if (End - Pos < 40) return false;
    Out.Field630 = Pos;
    Out.Field380 = Pos + 4;
    Out.Floats690 = Pos + 8;
    Out.Words6A0 = Pos + 24;
    Pos += 36;
    if (End - Pos < 4) return false;
    const uint32 Targets = U32(Pos);
    Out.SwitchCountValue = int32(Targets);
    Out.SwitchTargets = Pos + 4;
    Pos += 4;
    if (uint64(Targets) > uint64(End - Pos) / 28) return false;
    Out.SwitchCamera = Pos + int64(Targets) * 28;
    if (End - Out.SwitchCamera < 32) return false;
    Pos = Out.SwitchCamera + 32;
    Out.Tail = Pos;
    return true;
}
}

bool KyaReadNoseMonsterOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaNoseMonsterOffsets& Out)
{
    Out = FKyaNoseMonsterOffsets{};
    FKyaNoseMonsterOffsets WithVersion;
    FKyaNoseMonsterOffsets WithoutVersion;
    const bool bWith = ParseNoseMonster(Data, Start, Size, true, WithVersion);
    const bool bWithout = ParseNoseMonster(Data, Start, Size, false, WithoutVersion);
    // The scene-version gate changes the payload by eight bytes. Prefer the
    // shape that consumes the record exactly; otherwise keep the modern shape.
    if (bWith && WithVersion.Tail == Start + Size) { Out = WithVersion; return true; }
    if (bWithout && WithoutVersion.Tail == Start + Size) { Out = WithoutVersion; return true; }
    if (bWith) { Out = WithVersion; return true; }
    if (bWithout) { Out = WithoutVersion; return true; }
    return false;
}

bool KyaReadNoseMonsterActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    FKyaNoseMonsterOffsets Off;
    if (!KyaReadNoseMonsterOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.NoseMonsterZoneRef = int32(KyaReadU32(Bytes + Off.ZoneRef));
    Out.NoseMonsterField384 = KyaReadF32(Bytes + Off.Floats);
    Out.NoseMonsterField354 = KyaReadF32(Bytes + Off.Floats + 4);
    Out.NoseMonsterField358 = KyaReadF32(Bytes + Off.Floats + 8);
    Out.NoseMonsterField35C = KyaReadF32(Bytes + Off.Floats + 12);
    Out.NoseMonsterField360 = KyaReadF32(Bytes + Off.Floats + 16);
    Out.bNoseMonsterHasVersionFloats = Off.bHasVersionFloats;
    if (Off.bHasVersionFloats)
    {
        Out.NoseMonsterField378 = KyaReadF32(Bytes + Off.VersionFloats);
        Out.NoseMonsterField37C = KyaReadF32(Bytes + Off.VersionFloats + 4);
    }
    Out.NoseMonsterField364 = int32(KyaReadU32(Bytes + Off.Field364));
    Out.NoseMonsterField370 = KyaReadF32(Bytes + Off.Field370);
    Out.NoseMonsterField374 = KyaReadF32(Bytes + Off.Field374);

    Out.NoseMonsterZonePathEntries.Reset();
    for (int32 Index = 0; Index < Off.ZonePathCountValue; ++Index)
    {
        FKyaNoseMonsterZonePath& Entry = Out.NoseMonsterZonePathEntries.AddDefaulted_GetRef();
        const int64 E = Off.ZonePathEntries + int64(Index) * 8;
        Entry.ZoneRef = int32(KyaReadU32(Bytes + E));
        Entry.PathFollowRef = int32(KyaReadU32(Bytes + E + 4));
    }
    Out.NoseMonsterField36C = int32(KyaReadU32(Bytes + Off.Field36C));

    Out.NoseMonsterFireShotActorRefs.Reset();
    for (int32 Index = 0; Index < Off.FireShotCountValue; ++Index)
        Out.NoseMonsterFireShotActorRefs.Add(
            int32(KyaReadU32(Bytes + Off.FireShotEntries + int64(Index) * 4)));

    Out.NoseMonsterField630 = int32(KyaReadU32(Bytes + Off.Field630));
    Out.NoseMonsterField380 = int32(KyaReadU32(Bytes + Off.Field380));
    Out.NoseMonsterField690 = KyaReadF32(Bytes + Off.Floats690);
    Out.NoseMonsterField694 = KyaReadF32(Bytes + Off.Floats690 + 4);
    Out.NoseMonsterField698 = KyaReadF32(Bytes + Off.Floats690 + 8);
    Out.NoseMonsterField69C = KyaReadF32(Bytes + Off.Floats690 + 12);
    Out.NoseMonsterField6A0 = int32(KyaReadU32(Bytes + Off.Words6A0));
    Out.NoseMonsterField6A4 = int32(KyaReadU32(Bytes + Off.Words6A0 + 4));
    Out.NoseMonsterField6A8 = int32(KyaReadU32(Bytes + Off.Words6A0 + 8));

    Out.NoseMonsterSwitch = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Off.SwitchTargets, Off.SwitchCountValue, Off.SwitchCamera,
        Out.NoseMonsterSwitch)) return false;

    Out.bHasNoseMonsterData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteNoseMonsterActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    FKyaNoseMonsterOffsets Off;
    if (!KyaReadNoseMonsterOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.bNoseMonsterHasVersionFloats != Off.bHasVersionFloats) return false;
    if (Actor.NoseMonsterZonePathEntries.Num() != Off.ZonePathCountValue) return false;
    if (Actor.NoseMonsterFireShotActorRefs.Num() != Off.FireShotCountValue) return false;
    if (Actor.NoseMonsterSwitch.Targets.Num() != Off.SwitchCountValue) return false;

    KyaWriteU32(Data, Off.ZoneRef, uint32(Actor.NoseMonsterZoneRef));
    KyaWriteF32(Data, Off.Floats, Actor.NoseMonsterField384);
    KyaWriteF32(Data, Off.Floats + 4, Actor.NoseMonsterField354);
    KyaWriteF32(Data, Off.Floats + 8, Actor.NoseMonsterField358);
    KyaWriteF32(Data, Off.Floats + 12, Actor.NoseMonsterField35C);
    KyaWriteF32(Data, Off.Floats + 16, Actor.NoseMonsterField360);
    if (Off.bHasVersionFloats)
    {
        KyaWriteF32(Data, Off.VersionFloats, Actor.NoseMonsterField378);
        KyaWriteF32(Data, Off.VersionFloats + 4, Actor.NoseMonsterField37C);
    }
    KyaWriteU32(Data, Off.Field364, uint32(Actor.NoseMonsterField364));
    KyaWriteF32(Data, Off.Field370, Actor.NoseMonsterField370);
    KyaWriteF32(Data, Off.Field374, Actor.NoseMonsterField374);

    for (int32 Index = 0; Index < Actor.NoseMonsterZonePathEntries.Num(); ++Index)
    {
        const FKyaNoseMonsterZonePath& Entry = Actor.NoseMonsterZonePathEntries[Index];
        const int64 E = Off.ZonePathEntries + int64(Index) * 8;
        KyaWriteU32(Data, E, uint32(Entry.ZoneRef));
        KyaWriteU32(Data, E + 4, uint32(Entry.PathFollowRef));
    }
    KyaWriteU32(Data, Off.Field36C, uint32(Actor.NoseMonsterField36C));
    for (int32 Index = 0; Index < Actor.NoseMonsterFireShotActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.FireShotEntries + int64(Index) * 4,
            uint32(Actor.NoseMonsterFireShotActorRefs[Index]));

    KyaWriteU32(Data, Off.Field630, uint32(Actor.NoseMonsterField630));
    KyaWriteU32(Data, Off.Field380, uint32(Actor.NoseMonsterField380));
    KyaWriteF32(Data, Off.Floats690, Actor.NoseMonsterField690);
    KyaWriteF32(Data, Off.Floats690 + 4, Actor.NoseMonsterField694);
    KyaWriteF32(Data, Off.Floats690 + 8, Actor.NoseMonsterField698);
    KyaWriteF32(Data, Off.Floats690 + 12, Actor.NoseMonsterField69C);
    KyaWriteU32(Data, Off.Words6A0, uint32(Actor.NoseMonsterField6A0));
    KyaWriteU32(Data, Off.Words6A0 + 4, uint32(Actor.NoseMonsterField6A4));
    KyaWriteU32(Data, Off.Words6A0 + 8, uint32(Actor.NoseMonsterField6A8));
    KyaWriteSwitchValue(Data, Actor.NoseMonsterSwitch, Off.SwitchTargets, Off.SwitchCamera);
    return true;
}
