#include "Actors/KyaExplosiveDistributor.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadExplosiveDistributorOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaExplosiveDistributorOffsets& Out)
{
    Out = FKyaExplosiveDistributorOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 44) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Radius = Start;
    Out.Angle = Start + 4;
    Out.Count = Start + 8;
    const uint32 Count = KyaReadU32(Bytes + Out.Count);
    Out.CountValue = int32(Count);
    Out.Entries = Start + 12;
    int64 Pos = Out.Entries;
    if (uint64(Count) > uint64(End - Pos) / 20) return false;
    Pos += int64(Count) * 20;
    if (End - Pos < 32) return false;
    Out.Camera = Pos;
    Pos += 32;
    Out.Tail = Pos;
    return true;
}

bool KyaReadExplosiveDistributorActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    FKyaExplosiveDistributorOffsets Off;
    if (!KyaReadExplosiveDistributorOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.ExplosiveDistributorRadius = KyaReadF32(Bytes + Off.Radius);
    Out.ExplosiveDistributorAngle = KyaReadF32(Bytes + Off.Angle);
    Out.ExplosiveDistributorProjectiles.Reset();
    for (int32 Index = 0; Index < Off.CountValue; ++Index)
    {
        FKyaExplosiveProjectile& Projectile = Out.ExplosiveDistributorProjectiles.AddDefaulted_GetRef();
        const int64 P = Off.Entries + int64(Index) * 20;
        Projectile.ActorRef = int32(KyaReadU32(Bytes + P));
        Projectile.Direction = FVector(KyaReadF32(Bytes + P + 4), KyaReadF32(Bytes + P + 8),
            KyaReadF32(Bytes + P + 12));
        Projectile.EjectForce = KyaReadF32(Bytes + P + 16);
    }
    const int64 C = Off.Camera;
    FKyaStreamEventCamera& Camera = Out.ExplosiveDistributorCamera;
    Camera.CameraIndex = int32(KyaReadU32(Bytes + C));
    Camera.Flags = int32(KyaReadU32(Bytes + C + 4));
    Camera.Field8 = KyaReadF32(Bytes + C + 8);
    Camera.FieldC = KyaReadF32(Bytes + C + 12);
    Camera.Field10 = KyaReadF32(Bytes + C + 16);
    Camera.ActorRef = int32(KyaReadU32(Bytes + C + 20));
    Camera.ZoneRef = int32(KyaReadU32(Bytes + C + 24));
    Camera.Field1C = KyaReadF32(Bytes + C + 28);

    Out.bHasExplosiveDistributorData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteExplosiveDistributorActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    FKyaExplosiveDistributorOffsets Off;
    if (!KyaReadExplosiveDistributorOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start,
        Size, Off)) return false;
    if (Actor.ExplosiveDistributorProjectiles.Num() != Off.CountValue) return false;
    KyaWriteF32(Data, Off.Radius, Actor.ExplosiveDistributorRadius);
    KyaWriteF32(Data, Off.Angle, Actor.ExplosiveDistributorAngle);
    for (int32 Index = 0; Index < Actor.ExplosiveDistributorProjectiles.Num(); ++Index)
    {
        const FKyaExplosiveProjectile& Projectile = Actor.ExplosiveDistributorProjectiles[Index];
        const int64 P = Off.Entries + int64(Index) * 20;
        KyaWriteU32(Data, P, uint32(Projectile.ActorRef));
        KyaWriteF32(Data, P + 4, float(Projectile.Direction.X));
        KyaWriteF32(Data, P + 8, float(Projectile.Direction.Y));
        KyaWriteF32(Data, P + 12, float(Projectile.Direction.Z));
        KyaWriteF32(Data, P + 16, Projectile.EjectForce);
    }
    const FKyaStreamEventCamera& Camera = Actor.ExplosiveDistributorCamera;
    const int64 C = Off.Camera;
    KyaWriteU32(Data, C, uint32(Camera.CameraIndex));
    KyaWriteU32(Data, C + 4, uint32(Camera.Flags));
    KyaWriteF32(Data, C + 8, Camera.Field8);
    KyaWriteF32(Data, C + 12, Camera.FieldC);
    KyaWriteF32(Data, C + 16, Camera.Field10);
    KyaWriteU32(Data, C + 20, uint32(Camera.ActorRef));
    KyaWriteU32(Data, C + 24, uint32(Camera.ZoneRef));
    KyaWriteF32(Data, C + 28, Camera.Field1C);
    return true;
}
