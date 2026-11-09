#pragma once
#include "CoreMinimal.h"

/** Default CActor basis from SetVectorFromAngles and UpdatePosition. Angles are radians. */
namespace KyaActorTransform
{
    // Class IDs with flags 0x2 and 0x800 in CActorFactory::gClassProperties.
    inline uint32 RotationClassFlags(int64 ClassId)
    {
        constexpr int32 YawOnly[] = {6,7,8,9,10,22,25,28,34,35,39,40,42,43,48,51,54,56,62,63,65,72,75,78,79,82};
        constexpr int32 EulerMatrix[] = {1,5,11,13,14,15,17,18,20,27,29,32,33,36,37,38,41,43,44,45,47,48,53,55,57,58,74,76,77};
        uint32 Flags = 0;
        for (int32 Id : YawOnly) if (Id == ClassId) { Flags |= 2; break; }
        for (int32 Id : EulerMatrix) if (Id == ClassId) { Flags |= 0x800; break; }
        return Flags;
    }

    inline FVector EulerForward(const FVector& Angles)
    {
        const double Sx = FMath::Sin(Angles.X), Cx = FMath::Cos(Angles.X);
        const double Sy = FMath::Sin(Angles.Y), Cy = FMath::Cos(Angles.Y);
        const double Sz = FMath::Sin(Angles.Z), Cz = FMath::Cos(Angles.Z);
        return FVector(Sx * Sz + Sy * Cx * Cz, Sy * Cx * Sz - Sx * Cz, Cx * Cy);
    }
    inline FVector ApplyDefaultRotation(const FVector& Point, const FVector& Angles)
    {
        const double Sy = FMath::Sin(Angles.Y), Cy = FMath::Cos(Angles.Y);
        const double Sz = FMath::Sin(Angles.Z), Cz = FMath::Cos(Angles.Z);
        FVector Forward;
        if (FMath::Abs(Angles.X) > 0.0001 || FMath::Abs(Angles.Z) > 0.0001)
            Forward = EulerForward(Angles);
        else
            Forward = FVector(Sy, 0, Cy);
        const FVector Reference = FMath::Abs(Forward.Y) > 0.999 ? FVector(0, 0, 1) : FVector(0, 1, 0);
        const FVector Right = FVector::CrossProduct(Reference, Forward).GetSafeNormal();
        const FVector Up = FVector::CrossProduct(Forward, Right).GetSafeNormal();
        if (FMath::Abs(Angles.Z) < 0.0001)
            return Right * Point.X + Up * Point.Y + Forward * Point.Z;
        return (Right * Cz + Up * Sz) * Point.X + (Up * Cz - Right * Sz) * Point.Y + Forward * Point.Z;
    }

    inline FVector ApplyClassRotation(const FVector& Point, FVector Angles, int64 ClassId)
    {
        const uint32 Flags = RotationClassFlags(ClassId);
        if (Flags & 2)
        {
            const FVector Forward = EulerForward(Angles);
            Angles = FVector(0, FMath::Atan2(Forward.X, Forward.Z), 0);
        }
        if (!(Flags & 0x800)) return ApplyDefaultRotation(Point, Angles);
        const double Sx = FMath::Sin(Angles.X), Cx = FMath::Cos(Angles.X);
        const double Sy = FMath::Sin(Angles.Y), Cy = FMath::Cos(Angles.Y);
        const double Sz = FMath::Sin(Angles.Z), Cz = FMath::Cos(Angles.Z);
        const FVector X(Cy * Cz, Cy * Sz, -Sy);
        const FVector Y(Sy * Sx * Cz - Cx * Sz, Cx * Cz + Sx * Sz * Sy, Cy * Sx);
        const FVector Z(Sx * Sz + Sy * Cx * Cz, Sy * Cx * Sz - Sx * Cz, Cx * Cy);
        return X * Point.X + Y * Point.Y + Z * Point.Z;
    }
}
