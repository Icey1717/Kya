#include "Actors/KyaElectrolla.h"
#include "KyaLevelBank.h"
namespace
{
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
void Write(uint8* P, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) P[I] = uint8(Value >> (I * 8));
}
}
bool KyaReadElectrollaOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaElectrollaOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 72 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    // S_NTF_SWITCH_ONOFF uses the same 28-byte target and 32-byte camera layout
    // as S_NTF_SWITCH; only runtime SwitchOn/SwitchOff semantics differ.
    if (!KyaReadSwitchActorOffsets(Data, Start + 36, Size - 36, Out.Switch)) return false;
    Out.Tail = Out.Switch.Tail;
    return true;
}
bool KyaReadElectrollaActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaElectrollaOffsets Off;
    if (!KyaReadElectrollaOffsets(Data, Start, Size, Off)) return false;
    const uint8* P = Data.GetData() + Start;
    Actor.ElectrollaSparkCount = int32(Read(P));
    float* Fields[] = {&Actor.ElectrollaField178, &Actor.ElectrollaField17C, &Actor.ElectrollaField180,
        &Actor.ElectrollaField194, &Actor.ElectrollaField198, &Actor.ElectrollaField19C};
    for (int32 I = 0; I < 6; ++I)
    {
        uint32 Bits = Read(P + 4 + I * 4);
        FMemory::Memcpy(Fields[I], &Bits, 4);
    }
    Actor.ElectrollaMaterialId = int32(Read(P + 28));
    Actor.ElectrollaSparkMaterialId = int32(Read(P + 32));
    if (!KyaReadSwitchValue(Data, Off.Switch.Targets, Off.Switch.Count, Off.Switch.Camera, Actor.ElectrollaSwitch)) return false;
    Actor.ClassData.Reset();
    Actor.ClassData.Append(Data.GetData() + Off.Tail, int32(Start + Size - Off.Tail));
    Actor.bHasElectrollaData = true;
    return true;
}
bool KyaWriteElectrollaActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaElectrollaOffsets Off;
    if (!Actor.bHasElectrollaData || !KyaReadElectrollaOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        Actor.ElectrollaSwitch.Targets.Num() != Off.Switch.Count || Actor.ElectrollaSparkCount < 0) return false;
    uint8* P = Data.GetData() + Start;
    Write(P, uint32(Actor.ElectrollaSparkCount));
    const float Fields[] = {Actor.ElectrollaField178, Actor.ElectrollaField17C, Actor.ElectrollaField180,
        Actor.ElectrollaField194, Actor.ElectrollaField198, Actor.ElectrollaField19C};
    for (int32 I = 0; I < 6; ++I)
    {
        uint32 Bits;
        FMemory::Memcpy(&Bits, &Fields[I], 4);
        Write(P + 4 + I * 4, Bits);
    }
    Write(P + 28, uint32(Actor.ElectrollaMaterialId));
    Write(P + 32, uint32(Actor.ElectrollaSparkMaterialId));
    KyaWriteSwitchValue(Data, Actor.ElectrollaSwitch, Off.Switch.Targets, Off.Switch.Camera);
    return true;
}
