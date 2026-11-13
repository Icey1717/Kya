#include "Actors/KyaBoomy.h"

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
}

bool KyaReadBoomyOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBoomyOffsets& Out)
{
    Out = FKyaBoomyOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 4) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Weight = Start;
    Out.Vision = Start + 4;
    Out.Sound2B8 = Start + 24;
    Out.Sound2BC = Start + 28;
    Out.Field744 = Start + 32;
    Out.TailWidthAxis = Start + 36;
    Out.Field740 = Start + 40;
    Out.TailSegments = Start + 44;
    Out.Particle3D0 = Start + 48;
    Out.Particle3D4 = Start + 52;
    Out.Particle3DC = Start + 56;
    Out.Field3CC = Start + 60;
    Out.Texture = Start + 64;
    Out.Mesh = Start + 68;
    Out.CameraStart = Start + 72;

    // Weight + 20-byte vision record + tail/fx words, then the camera record.
    int64 Pos = Out.CameraStart;
    if (!KyaReadActorCameraOffsets(Data, Pos, End, Out.Camera)) return false;
    Out.Tail = Pos;
    if (End - Pos < 4) return false;
    Out.TypeCount = Pos;
    const uint32 Types = KyaReadU32(Bytes + Pos);
    Out.TypeCountValue = int32(Types);
    Pos += 4;
    if (uint64(Types) > uint64(End - Pos) / 16) return false;
    Out.TypeEntries = Pos;
    Pos += int64(Types) * 16;
    Out.Tail = Pos;
    return true;
}

bool KyaReadBoomyActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaBoomyOffsets Off;
    if (!KyaReadBoomyOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    const FKyaActorCameraOffsets& C = Off.Camera;

    Out.BoomyWeight = KyaReadF32(Bytes + Off.Weight);
    Out.BoomyVisionRange = KyaReadF32(Bytes + Off.Vision);
    Out.BoomyVisionAngle = KyaReadF32(Bytes + Off.Vision + 4);
    Out.BoomyVisionField38 = KyaReadF32(Bytes + Off.Vision + 8);
    Out.BoomyVisionField3C = KyaReadF32(Bytes + Off.Vision + 12);
    Out.BoomyVisionFlags = int32(KyaReadU32(Bytes + Off.Vision + 16));
    Out.BoomySound2B8 = int32(KyaReadU32(Bytes + Off.Sound2B8));
    Out.BoomySound2BC = int32(KyaReadU32(Bytes + Off.Sound2BC));
    Out.BoomyField744 = KyaReadF32(Bytes + Off.Field744);
    Out.BoomyTailWidthAxis = int32(KyaReadU32(Bytes + Off.TailWidthAxis));
    Out.BoomyField740 = KyaReadF32(Bytes + Off.Field740);
    Out.BoomyTailSegments = int32(KyaReadU32(Bytes + Off.TailSegments));
    Out.BoomyParticle3D0 = int32(KyaReadU32(Bytes + Off.Particle3D0));
    Out.BoomyParticle3D4 = int32(KyaReadU32(Bytes + Off.Particle3D4));
    Out.BoomyParticle3DC = int32(KyaReadU32(Bytes + Off.Particle3DC));
    Out.BoomyField3CC = int32(KyaReadU32(Bytes + Off.Field3CC));
    Out.BoomyTextureIndex = int32(KyaReadU32(Bytes + Off.Texture));
    Out.BoomyMeshIndex = int32(KyaReadU32(Bytes + Off.Mesh));

    Out.BoomyCamera = KyaReadActorCameraValues(Data, C);

    Out.BoomyTypes.Reset();
    for (int32 Index = 0; Index < Off.TypeCountValue; ++Index)
    {
        FKyaBoomyType& Type = Out.BoomyTypes.AddDefaulted_GetRef();
        const int64 T = Off.TypeEntries + int64(Index) * 16;
        Type.Field4 = KyaReadF32(Bytes + T);
        Type.VisionRange = KyaReadF32(Bytes + T + 4);
        Type.HitDamage = int32(KyaReadU32(Bytes + T + 8));
        Type.FxColorA = int32(KyaReadU32(Bytes + T + 12));
    }

    Out.bHasBoomyData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteBoomyActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaBoomyOffsets Off;
    if (!KyaReadBoomyOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.BoomyTypes.Num() != Off.TypeCountValue) return false;
    if (!KyaWriteActorCameraValues(Actor.BoomyCamera, Off.Camera, Data)) return false;

    // Vision fields sit immediately before the camera record.
    KyaWriteF32(Data, Off.Weight, Actor.BoomyWeight);
    KyaWriteF32(Data, Off.Vision, Actor.BoomyVisionRange);
    KyaWriteF32(Data, Off.Vision + 4, Actor.BoomyVisionAngle);
    KyaWriteF32(Data, Off.Vision + 8, Actor.BoomyVisionField38);
    KyaWriteF32(Data, Off.Vision + 12, Actor.BoomyVisionField3C);
    KyaWriteU32(Data, Off.Vision + 16, uint32(Actor.BoomyVisionFlags));
    KyaWriteU32(Data, Off.Sound2B8, uint32(Actor.BoomySound2B8));
    KyaWriteU32(Data, Off.Sound2BC, uint32(Actor.BoomySound2BC));
    KyaWriteF32(Data, Off.Field744, Actor.BoomyField744);
    KyaWriteU32(Data, Off.TailWidthAxis, uint32(Actor.BoomyTailWidthAxis));
    KyaWriteF32(Data, Off.Field740, Actor.BoomyField740);
    KyaWriteU32(Data, Off.TailSegments, uint32(Actor.BoomyTailSegments));
    KyaWriteU32(Data, Off.Particle3D0, uint32(Actor.BoomyParticle3D0));
    KyaWriteU32(Data, Off.Particle3D4, uint32(Actor.BoomyParticle3D4));
    KyaWriteU32(Data, Off.Particle3DC, uint32(Actor.BoomyParticle3DC));
    KyaWriteU32(Data, Off.Field3CC, uint32(Actor.BoomyField3CC));
    KyaWriteU32(Data, Off.Texture, uint32(Actor.BoomyTextureIndex));
    KyaWriteU32(Data, Off.Mesh, uint32(Actor.BoomyMeshIndex));

    for (int32 Index = 0; Index < Actor.BoomyTypes.Num(); ++Index)
    {
        const FKyaBoomyType& Type = Actor.BoomyTypes[Index];
        const int64 T = Off.TypeEntries + int64(Index) * 16;
        KyaWriteF32(Data, T, Type.Field4);
        KyaWriteF32(Data, T + 4, Type.VisionRange);
        KyaWriteU32(Data, T + 8, uint32(Type.HitDamage));
        KyaWriteU32(Data, T + 12, uint32(Type.FxColorA));
    }
    return true;
}
