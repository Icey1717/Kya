#pragma once
#include "CoreMinimal.h"

/**
 * Forward and inverse conversion between a bank actor placement and the editor's
 * Unreal transform.
 *
 * Geometry is stored in game space: X right, Y up, Z forward, metres. The editor
 * uses Unreal axes (X forward, Y right, Z up, centimetres). The axis map is the
 * permutation P(x, y, z) = (x, z, y), which is its own inverse and a reflection.
 *
 * A class placement produces a rotation basis with BuildRotationBasis (the same
 * forward rules the renderer uses). P conjugates that basis into Unreal space, so
 * the scalar scale stays separate and negative scales are rejected rather than
 * silently folded into the rotation.
 */
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

    /** Which serialized rotation path a class uses. Yaw-only wins when both flags are set. */
    enum class ERotationPath : uint8
    {
        YawOnly,
        EulerMatrix,
        DefaultBasis
    };

    inline ERotationPath RotationPath(int64 ClassId)
    {
        const uint32 Flags = RotationClassFlags(ClassId);
        if (Flags & 2) return ERotationPath::YawOnly;
        if (Flags & 0x800) return ERotationPath::EulerMatrix;
        return ERotationPath::DefaultBasis;
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

    /** Swap the game's Y-up axes with Unreal's Z-up axes. The map is its own inverse. */
    inline FVector SwapGameAxes(const FVector& Value) { return FVector(Value.X, Value.Z, Value.Y); }

    /**
     * Rotation-only basis of a class placement in game axes. The columns are the
     * images of the game unit axes (X right, Y up, Z forward) and form a proper
     * rotation for every supported class path.
     */
    inline void BuildRotationBasis(const FVector& Angles, int64 ClassId,
        FVector& OutX, FVector& OutY, FVector& OutZ)
    {
        OutX = ApplyClassRotation(FVector(1, 0, 0), Angles, ClassId);
        OutY = ApplyClassRotation(FVector(0, 1, 0), Angles, ClassId);
        OutZ = ApplyClassRotation(FVector(0, 0, 1), Angles, ClassId);
    }

    /** Unreal rotation whose basis is the game basis conjugated by the axis permutation. */
    inline FQuat QuatFromGameBasis(const FVector& BasisX, const FVector& BasisY, const FVector& BasisZ)
    {
        // Column i of the Unreal basis is P(image of game unit axis): columns
        // (X, Z, Y). FMatrix stores rows for FRotationMatrix-style access.
        FMatrix Matrix;
        Matrix.SetAxis(0, SwapGameAxes(BasisX));
        Matrix.SetAxis(1, SwapGameAxes(BasisZ));
        Matrix.SetAxis(2, SwapGameAxes(BasisY));
        FQuat Quat(Matrix);
        Quat.Normalize();
        return Quat;
    }

    /** Inverse of QuatFromGameBasis. */
    inline void GameBasisFromQuat(const FQuat& Rotation, FVector& OutX, FVector& OutY, FVector& OutZ)
    {
        const FVector ColX = Rotation.RotateVector(FVector(1, 0, 0));
        const FVector ColY = Rotation.RotateVector(FVector(0, 1, 0));
        const FVector ColZ = Rotation.RotateVector(FVector(0, 0, 1));
        OutX = SwapGameAxes(ColX);
        OutY = SwapGameAxes(ColZ);
        OutZ = SwapGameAxes(ColY);
    }

    /** Pick the equivalent angle (modulo 2*pi) nearest to Reference. */
    inline double NearestAngle(double Candidate, double Reference)
    {
        constexpr double TwoPi = 2.0 * PI;
        return Candidate + TwoPi * FMath::RoundToDouble((Reference - Candidate) / TwoPi);
    }

    /**
     * Invert ApplyClassRotation for a rotation-only basis.
     *
     * Yaw-only classes expose yaw only and keep the previously serialized pitch and
     * roll, because those components cannot be represented by the class. All paths
     * choose angles nearest PreviousAngles and then verify that the forward rules
     * reproduce the requested basis, so an unrepresentable orientation is rejected
     * instead of storing a misleading transform.
     */
    inline bool InvertRotation(const FVector& BasisX, const FVector& BasisY, const FVector& BasisZ,
        int64 ClassId, const FVector& PreviousAngles, FVector& OutAngles)
    {
        const double Determinant = FVector::DotProduct(BasisX, FVector::CrossProduct(BasisY, BasisZ));
        if (!FMath::IsNearlyEqual(Determinant, 1.0, 1.0e-3))
            return false;

        const ERotationPath Path = RotationPath(ClassId);
        double Pitch = 0.0, Yaw = 0.0, Roll = 0.0;

        if (Path == ERotationPath::YawOnly)
        {
            // The native yaw-only setup (SV_BuildAngleWithOnlyY) stores yaw only and
            // clears pitch/roll, so an actual yaw edit writes that representation.
            Yaw = FMath::Atan2(BasisZ.X, BasisZ.Z);
            Pitch = 0.0;
            Roll = 0.0;
        }
        else if (Path == ERotationPath::EulerMatrix)
        {
            // Basis rows from ApplyClassRotation: X = (CyCz, CySz, -Sy),
            // Y = (SySxCz - CxSz, CxCz + SxSzSy, CySx),
            // Z = (SxSz + SyCxCz, SyCxSz - SxCz, CxCy).
            const double Sy = -BasisX.Z;
            const double Cy = FMath::Sqrt(FMath::Max(0.0, 1.0 - Sy * Sy));
            if (Cy > 1.0e-3)
            {
                const double Cx = BasisZ.Z / Cy;
                const double Sx = BasisY.Z / Cy;
                Pitch = FMath::Atan2(Sx, Cx);
                Yaw = FMath::Atan2(Sy, Cy);
                Roll = FMath::Atan2(BasisX.Y, BasisX.X);
            }
            else
            {
                // Yaw at +/-90 degrees: only pitch (+/-) roll is observable. At +90 the
                // Y column carries pitch-roll; at -90 the Z column carries pitch+roll.
                const double Sign = Sy >= 0.0 ? 1.0 : -1.0;
                Yaw = Sign * HALF_PI;
                const double Difference = Sign > 0.0
                    ? FMath::Atan2(BasisY.X, BasisY.Y)
                    : FMath::Atan2(-BasisZ.Y, -BasisZ.X);
                Roll = NearestAngle(PreviousAngles.Z, 0.0);
                Pitch = Difference + Sign * Roll;
            }
        }
        else
        {
            // Default look-at basis: the forward column picks right/up exactly like
            // ApplyDefaultRotation, then the basis X column gives the roll about it.
            const FVector Forward = BasisZ;
            const FVector Reference = FMath::Abs(Forward.Y) > 0.999 ? FVector(0, 0, 1) : FVector(0, 1, 0);
            const FVector Right = FVector::CrossProduct(Reference, Forward).GetSafeNormal();
            const FVector Up = FVector::CrossProduct(Forward, Right).GetSafeNormal();
            Roll = FMath::Atan2(FVector::DotProduct(BasisX, Up), FVector::DotProduct(BasisX, Right));
            const double Sz = FMath::Sin(Roll), Cz = FMath::Cos(Roll);
            // EulerForward with the solved roll: F.x = u*Sz + v*Cz, F.y = v*Sz - u*Cz
            // for u = sin(pitch) and v = sin(yaw)*cos(pitch).
            const double U = Forward.X * Sz - Forward.Y * Cz;
            const double V = Forward.X * Cz + Forward.Y * Sz;
            const double Cx = FMath::Sqrt(FMath::Max(0.0, Forward.Z * Forward.Z + V * V));
            if (Cx > 1.0e-6)
            {
                Pitch = FMath::Atan2(U, Cx);
                Yaw = FMath::Atan2(V / Cx, Forward.Z / Cx);
            }
            else
            {
                // Straight up/down: the yaw is not observable, keep the serialized one.
                Pitch = U >= 0.0 ? HALF_PI : -HALF_PI;
                Yaw = PreviousAngles.Y;
            }
        }

        OutAngles = FVector(
            NearestAngle(Pitch, PreviousAngles.X),
            NearestAngle(Yaw, PreviousAngles.Y),
            NearestAngle(Roll, PreviousAngles.Z));

        // Never write back an angle set the runtime cannot reproduce.
        FVector CheckX, CheckY, CheckZ;
        BuildRotationBasis(OutAngles, ClassId, CheckX, CheckY, CheckZ);
        constexpr double Tolerance = 2.0e-3;
        return CheckX.Equals(BasisX, Tolerance) && CheckY.Equals(BasisY, Tolerance) && CheckZ.Equals(BasisZ, Tolerance);
    }

    /** A bank placement expressed in editor terms. */
    struct FGameTransform
    {
        FVector Position = FVector::ZeroVector;
        FVector Rotation = FVector::ZeroVector;
        FVector Scale = FVector::OneVector;
    };

    /** Editor transform of a class placement, before the preview root transform. */
    inline void MakeUnrealTransform(const FVector& Position, const FVector& Rotation, const FVector& Scale,
        int64 ClassId, FTransform& OutTransform)
    {
        FVector BasisX, BasisY, BasisZ;
        BuildRotationBasis(Rotation, ClassId, BasisX, BasisY, BasisZ);
        OutTransform = FTransform(QuatFromGameBasis(BasisX, BasisY, BasisZ),
            SwapGameAxes(Position) * 100.0, SwapGameAxes(Scale));
    }

    /**
     * Convert an editor (preview-local) actor transform back into game placement
     * values. `Scale` must be strictly positive on every axis: negative or zero
     * scale is reported instead of being folded into the rotation.
     */
    inline bool UnrealToGameTransform(const FTransform& LocalTransform, int64 ClassId,
        const FVector& PreviousRotation, FGameTransform& Out, FString& OutError)
    {
        const FVector UnrealScale = LocalTransform.GetScale3D();
        if (UnrealScale.X <= KINDA_SMALL_NUMBER || UnrealScale.Y <= KINDA_SMALL_NUMBER ||
            UnrealScale.Z <= KINDA_SMALL_NUMBER)
        {
            OutError = TEXT("Negative or zero actor scale cannot be stored in a bank placement.");
            return false;
        }

        FVector BasisX, BasisY, BasisZ;
        GameBasisFromQuat(LocalTransform.GetRotation(), BasisX, BasisY, BasisZ);

        // A translate or scale edit must not rewrite the serialized rotation (yaw-only
        // classes would otherwise lose their stored pitch/roll). Preserve the previous
        // angle set whenever it already renders the same basis.
        FVector PreviousX, PreviousY, PreviousZ;
        BuildRotationBasis(PreviousRotation, ClassId, PreviousX, PreviousY, PreviousZ);
        if (PreviousX.Equals(BasisX, 1.0e-4) && PreviousY.Equals(BasisY, 1.0e-4) &&
            PreviousZ.Equals(BasisZ, 1.0e-4))
        {
            Out.Rotation = PreviousRotation;
        }
        else if (!InvertRotation(BasisX, BasisY, BasisZ, ClassId, PreviousRotation, Out.Rotation))
        {
            OutError = TEXT("This orientation cannot be represented by the actor class rotation rules.");
            return false;
        }
        Out.Position = SwapGameAxes(LocalTransform.GetLocation() / 100.0);
        Out.Scale = SwapGameAxes(UnrealScale);
        return true;
    }
}
