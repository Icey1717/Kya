#pragma once

#include "CoreMinimal.h"
#include "KyaLevelBank.h"

/** Shared CCamera record used by Kim, Boomy, Aton and actor-camera streams. */
struct KYALEVEL_API FKyaActorCameraOffsets
{
    int64 Base = 0, Fov = 0, Position = 0, LookAt = 0, Up = 0, Sector = 0, Field08 = 0, Flags = 0, TargetOffset = 0;
    bool bHasSwitch = false;
    int64 SwitchActor = 0, SwitchListA = 0, SwitchListB = 0, SwitchValues = 0;
    int32 SwitchCountA = 0, SwitchCountB = 0;
    int64 Conditions = 0, Config = 0, ConfigOptional = 0, ConfigAngles = 0, Extra = 0;
    int32 ConditionCount = 0, ConfigMode = 0, ExtraCount = 0;
    int64 Tail = 0;
};

inline bool KyaReadActorCameraOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End,
    FKyaActorCameraOffsets& Out)
{
    Out = FKyaActorCameraOffsets{};
    auto U32 = [&Data](int64 Offset) {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    };
    if (Pos < 0 || End > Data.Num() || End - Pos < 64) return false;
    Out.Base = Pos;
    Out.Fov = Pos;
    Out.Position = Pos + 4;
    Out.LookAt = Pos + 16;
    Out.Up = Pos + 28;
    Out.Sector = Pos + 40;
    Out.Field08 = Pos + 44;
    Out.Flags = Pos + 48;
    Out.TargetOffset = Pos + 52;
    Pos += 64;
    if (U32(Out.Flags) & 2)
    {
        Out.bHasSwitch = true;
        if (End - Pos < 4) return false;
        Out.SwitchActor = Pos;
        Pos += 4;
        for (int32 List = 0; List < 2; ++List)
        {
            if (End - Pos < 4) return false;
            const uint32 Count = U32(Pos);
            if (List == 0) { Out.SwitchListA = Pos + 4; Out.SwitchCountA = int32(Count); }
            else { Out.SwitchListB = Pos + 4; Out.SwitchCountB = int32(Count); }
            Pos += 4;
            if (uint64(Count) > uint64(End - Pos) / 4) return false;
            Pos += int64(Count) * 4;
        }
        if (End - Pos < 20) return false;
        Out.SwitchValues = Pos;
        Pos += 20;
    }
    if (End - Pos < 4) return false;
    Out.Conditions = Pos;
    const uint32 Conditions = U32(Pos);
    Out.ConditionCount = int32(Conditions);
    Pos += 4;
    if (uint64(Conditions) > uint64(End - Pos) / 16) return false;
    Pos += int64(Conditions) * 16;
    if (End - Pos < 20) return false;
    Out.Config = Pos;
    const uint32 Mode = U32(Pos + 16);
    Out.ConfigMode = int32(Mode);
    if (Mode > 1) return false;
    Pos += 20;
    if (Mode == 1)
    {
        if (End - Pos < 20) return false;
        Out.ConfigOptional = Pos;
        Pos += 20;
    }
    if (End - Pos < 36) return false;
    Out.ConfigAngles = Pos;
    Pos += 36;
    if (End - Pos < 4) return false;
    Out.Extra = Pos;
    const uint32 ExtraCount = U32(Pos);
    Out.ExtraCount = int32(ExtraCount);
    Pos += 4;
    if (ExtraCount == 1)
    {
        if (End - Pos < 48) return false;
        Pos += 48;
    }
    Out.Tail = Pos;
    return true;
}

inline FKyaActorCamera KyaReadActorCameraValues(TConstArrayView<uint8> Data,
    const FKyaActorCameraOffsets& Off)
{
    auto U32 = [&Data](int64 Offset) {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    };
    auto F32 = [&U32](int64 Offset) {
        const uint32 Bits = U32(Offset);
        float Value;
        FMemory::Memcpy(&Value, &Bits, sizeof(Value));
        return Value;
    };
    FKyaActorCamera Camera;
    Camera.Fov = F32(Off.Fov);
    Camera.Position = FVector(F32(Off.Position), F32(Off.Position + 4), F32(Off.Position + 8));
    Camera.LookAt = FVector(F32(Off.LookAt), F32(Off.LookAt + 4), F32(Off.LookAt + 8));
    Camera.Up = FVector(F32(Off.Up), F32(Off.Up + 4), F32(Off.Up + 8));
    Camera.SectorId = int32(U32(Off.Sector));
    Camera.Field08 = int32(U32(Off.Field08));
    Camera.Flags = int32(U32(Off.Flags));
    Camera.TargetOffset = FVector(F32(Off.TargetOffset), F32(Off.TargetOffset + 4),
        F32(Off.TargetOffset + 8));
    Camera.bHasSwitch = Off.bHasSwitch;
    if (Off.bHasSwitch)
    {
        Camera.SwitchActor = int32(U32(Off.SwitchActor));
        for (int32 Index = 0; Index < Off.SwitchCountA; ++Index)
            Camera.SwitchActorsA.Add(int32(U32(Off.SwitchListA + Index * 4)));
        for (int32 Index = 0; Index < Off.SwitchCountB; ++Index)
            Camera.SwitchActorsB.Add(int32(U32(Off.SwitchListB + Index * 4)));
        Camera.SwitchValueA = F32(Off.SwitchValues);
        Camera.SwitchModeA = int32(U32(Off.SwitchValues + 4));
        Camera.SwitchValueB = F32(Off.SwitchValues + 8);
        Camera.SwitchModeB = int32(U32(Off.SwitchValues + 12));
        Camera.SwitchValueC = F32(Off.SwitchValues + 16);
    }
    for (int32 Index = 0; Index < Off.ConditionCount; ++Index)
    {
        FKyaCameraCondition& Condition = Camera.Conditions.AddDefaulted_GetRef();
        const int64 C = Off.Conditions + 4 + int64(Index) * 16;
        Condition.Combiner = int32(U32(C));
        Condition.ActorRef = int32(U32(C + 4));
        Condition.Type = int32(U32(C + 8));
        Condition.Value = F32(C + 12);
    }
    Camera.ConfigFlags = int32(U32(Off.Config));
    Camera.ConfigActorRefA = int32(U32(Off.Config + 4));
    Camera.ConfigActorRefB = int32(U32(Off.Config + 8));
    Camera.ConfigAngle = F32(Off.Config + 12);
    if (Off.ConfigMode == 1)
        for (int32 Index = 0; Index < 5; ++Index)
            Camera.ConfigOptionalValues.Add(F32(Off.ConfigOptional + Index * 4));
    Camera.PitchAngles = FVector(F32(Off.ConfigAngles), F32(Off.ConfigAngles + 4),
        F32(Off.ConfigAngles + 8));
    Camera.OtherAngles = FVector(F32(Off.ConfigAngles + 12), F32(Off.ConfigAngles + 16),
        F32(Off.ConfigAngles + 20));
    Camera.Distances = FVector(F32(Off.ConfigAngles + 24), F32(Off.ConfigAngles + 28),
        F32(Off.ConfigAngles + 32));
    if (Off.ExtraCount == 1)
        for (int32 Index = 0; Index < 12; ++Index)
            Camera.ExtraValues.Add(F32(Off.Extra + 4 + Index * 4));
    return Camera;
}

/**
 * Writes one parsed camera record back in place. Rejects edited list sizes,
 * the layout-changing Flags bit and the optional blocks so a mismatched edit
 * cannot write outside the record.
 */
inline bool KyaWriteActorCameraValues(const FKyaActorCamera& Camera,
    const FKyaActorCameraOffsets& Off, TArray<uint8>& Data)
{
    auto W32 = [&Data](int64 Offset, uint32 Value) {
        for (int32 Byte = 0; Byte < 4; ++Byte) Data[int32(Offset) + Byte] = uint8(Value >> (Byte * 8));
    };
    auto W32F = [&W32](int64 Offset, float Value) {
        uint32 Bits = 0;
        FMemory::Memcpy(&Bits, &Value, sizeof(Bits));
        W32(Offset, Bits);
    };
    if (Camera.bHasSwitch != Off.bHasSwitch) return false;
    if (((Camera.Flags & 2) != 0) != Off.bHasSwitch) return false;
    if (Off.bHasSwitch && (Camera.SwitchActorsA.Num() != Off.SwitchCountA ||
        Camera.SwitchActorsB.Num() != Off.SwitchCountB)) return false;
    if (Camera.Conditions.Num() != Off.ConditionCount) return false;
    if (Off.ConfigMode == 1 ? Camera.ConfigOptionalValues.Num() != 5
        : !Camera.ConfigOptionalValues.IsEmpty()) return false;
    if (Off.ExtraCount == 1 ? Camera.ExtraValues.Num() != 12
        : !Camera.ExtraValues.IsEmpty()) return false;

    W32F(Off.Fov, Camera.Fov);
    for (int32 Axis = 0; Axis < 3; ++Axis) W32F(Off.Position + Axis * 4, float(Camera.Position[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis) W32F(Off.LookAt + Axis * 4, float(Camera.LookAt[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis) W32F(Off.Up + Axis * 4, float(Camera.Up[Axis]));
    W32(Off.Sector, uint32(Camera.SectorId));
    W32(Off.Field08, uint32(Camera.Field08));
    W32(Off.Flags, uint32(Camera.Flags));
    for (int32 Axis = 0; Axis < 3; ++Axis)
        W32F(Off.TargetOffset + Axis * 4, float(Camera.TargetOffset[Axis]));
    if (Off.bHasSwitch)
    {
        W32(Off.SwitchActor, uint32(Camera.SwitchActor));
        for (int32 Item = 0; Item < Camera.SwitchActorsA.Num(); ++Item)
            W32(Off.SwitchListA + Item * 4, uint32(Camera.SwitchActorsA[Item]));
        for (int32 Item = 0; Item < Camera.SwitchActorsB.Num(); ++Item)
            W32(Off.SwitchListB + Item * 4, uint32(Camera.SwitchActorsB[Item]));
        W32F(Off.SwitchValues, Camera.SwitchValueA);
        W32(Off.SwitchValues + 4, uint32(Camera.SwitchModeA));
        W32F(Off.SwitchValues + 8, Camera.SwitchValueB);
        W32(Off.SwitchValues + 12, uint32(Camera.SwitchModeB));
        W32F(Off.SwitchValues + 16, Camera.SwitchValueC);
    }
    for (int32 Item = 0; Item < Camera.Conditions.Num(); ++Item)
    {
        const FKyaCameraCondition& Condition = Camera.Conditions[Item];
        const int64 C = Off.Conditions + 4 + int64(Item) * 16;
        W32(C, uint32(Condition.Combiner));
        W32(C + 4, uint32(Condition.ActorRef));
        W32(C + 8, uint32(Condition.Type));
        W32F(C + 12, Condition.Value);
    }
    W32(Off.Config, uint32(Camera.ConfigFlags));
    W32(Off.Config + 4, uint32(Camera.ConfigActorRefA));
    W32(Off.Config + 8, uint32(Camera.ConfigActorRefB));
    W32F(Off.Config + 12, Camera.ConfigAngle);
    for (int32 Item = 0; Item < Camera.ConfigOptionalValues.Num(); ++Item)
        W32F(Off.ConfigOptional + Item * 4, Camera.ConfigOptionalValues[Item]);
    for (int32 Axis = 0; Axis < 3; ++Axis)
        W32F(Off.ConfigAngles + Axis * 4, float(Camera.PitchAngles[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis)
        W32F(Off.ConfigAngles + 12 + Axis * 4, float(Camera.OtherAngles[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis)
        W32F(Off.ConfigAngles + 24 + Axis * 4, float(Camera.Distances[Axis]));
    for (int32 Item = 0; Item < Camera.ExtraValues.Num(); ++Item)
        W32F(Off.Extra + 4 + Item * 4, Camera.ExtraValues[Item]);
    return true;
}
