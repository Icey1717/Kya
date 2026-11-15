#include "KyaPrimitives.h"
#include "KyaLevelBank.h"

namespace
{
struct FReader
{
    const uint8* Data;
    int64 Size;
    bool Has(int64 Offset, int64 Bytes) const { return Offset >= 0 && Bytes >= 0 && Offset <= Size && Bytes <= Size - Offset; }
    uint32 U32(int64 Offset) const
    {
        const uint8* P = Data + Offset;
        return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    }
    bool Matrix(int64 Offset, FMatrix& Out) const
    {
        if (!Has(Offset, 64)) return false;
        // Eden columns become Unreal rows: both then evaluate the same point map.
        for (int32 Row = 0; Row < 4; ++Row)
            for (int32 Col = 0; Col < 4; ++Col)
            {
                float Value;
                FMemory::Memcpy(&Value, Data + Offset + Row * 16 + Col * 4, 4);
                if (!FMath::IsFinite(Value)) return false;
                Out.M[Row][Col] = Value;
            }
        return true;
    }
    bool Vector(int64 Offset, FVector& Out) const
    {
        if (!Has(Offset, 12)) return false;
        float V[3]; FMemory::Memcpy(V, Data + Offset, 12);
        Out = FVector(V[0], V[1], V[2]);
        return !Out.ContainsNaN();
    }
};

bool DecodeCollision(const FReader& R, TArray<FKyaPrimitiveRecord>& Out)
{
    if (!R.Has(0, 104) || FMemory::Memcmp(R.Data + 12, ".COLCOLI", 8) != 0) return false;
    const int64 Vertices = R.U32(72);
    for (int32 Points : {3, 4})
    {
        const int64 Count = R.U32(Points == 3 ? 40 : 48);
        const int64 Table = R.U32(Points == 3 ? 76 : 84);
        const int64 Stride = (Points + 1) * 4;
        if (Count && (Table < 104 || !R.Has(Table, Count * Stride) || Vertices < 104)) return false;
        for (int64 I = 0; I < Count; ++I)
        {
            FKyaPrimitiveRecord P;
            P.SourceOffset = int32(Table + I * Stride);
            P.PrimitiveIndex = int32(I);
            P.Shape = Points == 3 ? EKyaPrimitiveShape::Triangle : EKyaPrimitiveShape::Quad;
            P.Flags = R.U32(P.SourceOffset + Points * 4);
            for (int32 J = 0; J < Points; ++J)
            {
                FVector V;
                const int64 Offset = Vertices + int64(R.U32(P.SourceOffset + J * 4)) * 16;
                if (!R.Vector(Offset, V)) return false;
                P.Vertices.Add(V);
                P.VertexOffsets.Add(int32(Offset));
            }
            Out.Add(MoveTemp(P));
        }
    }
    // OBB leaves reference boxes/spheres by index; decode only referenced shapes.
    const int64 Nodes = R.U32(64), Tree = R.U32(100);
    if (Nodes && (Tree < 104 || !R.Has(Tree, Nodes * 0x70))) return false;
    TSet<uint64> Seen;
    for (int64 I = 0; I < Nodes; ++I)
    {
        const int64 Node = Tree + I * 0x70;
        const uint8 Type = R.Data[Node + 0x51];
        if (Type != 10 && Type != 11) continue;
        const uint32 First = R.U32(Node + 0x54);
        for (uint32 J = 0; J < R.Data[Node + 0x52]; ++J)
        {
            const uint64 Index = uint64(First) + J;
            const uint64 Key = uint64(Type) << 32 | Index;
            if (Seen.Contains(Key)) continue;
            Seen.Add(Key);
            const int64 Offset = int64(R.U32(Type == 10 ? 88 : 92)) + int64(Index) * 0x90;
            FKyaPrimitiveRecord P;
            if (Offset < 104 || !R.Has(Offset, 0x90) || !R.Matrix(Offset, P.LocalToGame)) return false;
            P.SourceOffset = P.TransformOffset = int32(Offset);
            P.PrimitiveIndex = int32(Index);
            P.Shape = Type == 10 ? EKyaPrimitiveShape::Box : EKyaPrimitiveShape::Sphere;
            P.Flags = R.U32(Offset + 0x80);
            Out.Add(MoveTemp(P));
        }
    }
    return true;
}

bool DecodeEvents(const FReader& R, TArray<FKyaPrimitiveRecord>& Out)
{
    // Outer bank file header, EVNT chunk, compiler version, then matrix count.
    if (!R.Has(0, 24) || FMemory::Memcmp(R.Data + 8, "TNVE", 4) != 0 || R.U32(16) < 0x36) return false;
    const int64 Count = R.U32(20), Types = 24;
    const int64 Matrices = Count ? Align(Types + Count, int64(16)) : Types;
    int64 Pos = Matrices + Count * 64;
    if (!R.Has(Types, Count) || !R.Has(Matrices, Count * 64) || !R.Has(Pos, 4)) return false;
    const int64 Groups = R.U32(Pos), Table = Pos + 4;
    if (!R.Has(Table, (Groups + 1) * 4)) return false;
    const int64 Zones = Table + R.U32(Table + Groups * 4);
    if (Zones < Table + (Groups + 1) * 4 || !R.Has(Zones, 4)) return false;
    const int64 ZoneCount = R.U32(Zones), ZoneTable = Zones + 4;
    // Packed PS2 ed_zone_3d is 36 bytes (sphere starts at +12).
    if (!R.Has(ZoneTable, ZoneCount * 36)) return false;
    static const EKyaPrimitiveShape Shapes[] = { EKyaPrimitiveShape::HalfSpace,
        EKyaPrimitiveShape::OpenTopBox, EKyaPrimitiveShape::Box, EKyaPrimitiveShape::Cylinder,
        EKyaPrimitiveShape::Cone, EKyaPrimitiveShape::Sphere };
    for (int64 Z = 0; Z < ZoneCount; ++Z)
    {
        const int64 Zone = ZoneTable + Z * 36;
        for (int32 G = 0; G < 3; ++G)
        {
            const uint32 Group = R.U32(Zone + G * 4);
            if (Group == MAX_uint32) continue;
            if (Group >= Groups) return false;
            const int64 List = Table + R.U32(Table + int64(Group) * 4);
            if (List < Table + (Groups + 1) * 4 || !R.Has(List, 4) || List >= Zones) return false;
            const int64 N = R.U32(List);
            if (!R.Has(List + 4, N * 4) || List + 4 + N * 4 > Zones) return false;
            for (int64 I = 0; I < N; ++I)
            {
                const uint32 Index = R.U32(List + 4 + I * 4);
                if (Index >= Count || R.Data[Types + Index] >= UE_ARRAY_COUNT(Shapes)) return false;
                FKyaPrimitiveRecord P;
                P.ZoneIndex = int32(Z); P.GroupIndex = G; P.PrimitiveIndex = int32(Index);
                P.SourceOffset = int32(Zone); P.TransformOffset = int32(Matrices + int64(Index) * 64);
                P.MembershipOffset = int32(List + 4 + I * 4);
                P.bSerializedWorldToLocal = true;
                P.Shape = Shapes[R.Data[Types + Index]];
                FMatrix WorldToLocal;
                if (!R.Matrix(P.TransformOffset, WorldToLocal) || FMath::Abs(WorldToLocal.Determinant()) < 1.e-12) return false;
                P.LocalToGame = WorldToLocal.Inverse();
                Out.Add(MoveTemp(P));
            }
        }
    }
    return true;
}
}

void KyaPrimitives::Decode(const UKyaLevelBank& Bank, bool bCollision, bool bEvents,
    TArray<FKyaPrimitiveRecord>& Records, TArray<FString>& Diagnostics)
{
    Records.Reset(); Diagnostics.Reset();
    for (int32 E = 0; E < Bank.Entries.Num(); ++E)
    {
        const FKyaBankEntry& Entry = Bank.Entries[E];
        if (Entry.Subtype != 1 || !((bCollision && Entry.Type == 7) || (bEvents && Entry.Type == 8))) continue;
        TArray<FKyaPrimitiveRecord> Decoded;
        const bool bRange = Entry.Offset >= 0 && Entry.Size >= 0 && Entry.Offset <= Bank.BankBytes.Num() && Entry.Size <= Bank.BankBytes.Num() - Entry.Offset;
        const bool bOK = bRange && (Entry.Type == 7
            ? DecodeCollision({Bank.BankBytes.GetData() + Entry.Offset, Entry.Size}, Decoded)
            : DecodeEvents({Bank.BankBytes.GetData() + Entry.Offset, Entry.Size}, Decoded));
        if (!bOK) { Diagnostics.Add(FString::Printf(TEXT("%s entry %d (%d/1): invalid or unsupported primitive data"), *Bank.GetName(), E, Entry.Type)); continue; }
        for (FKyaPrimitiveRecord& P : Decoded) { P.EntryIndex = E; Records.Add(MoveTemp(P)); }
    }
}

void KyaPrimitives::BuildEdges(const FKyaPrimitiveRecord& R, TArray<FVector>& Edges)
{
    const auto Line = [&R, &Edges](FVector A, FVector B)
    {
        const FVector X = R.LocalToGame.TransformPosition(A), Y = R.LocalToGame.TransformPosition(B);
        if (!X.ContainsNaN() && !Y.ContainsNaN()) { Edges.Add(X); Edges.Add(Y); }
    };
    if (!R.Vertices.IsEmpty())
    {
        for (int32 I = 0; I < R.Vertices.Num(); ++I) Line(R.Vertices[I], R.Vertices[(I + 1) % R.Vertices.Num()]);
        return;
    }
    if (R.Shape == EKyaPrimitiveShape::Box || R.Shape == EKyaPrimitiveShape::OpenTopBox || R.Shape == EKyaPrimitiveShape::HalfSpace)
    {
        const bool bInfinite = R.Shape != EKyaPrimitiveShape::Box;
        const double Bottom = bInfinite ? 0.0 : -0.5, Top = bInfinite ? 2.0 : 0.5;
        for (int32 I = 0; I < 4; ++I)
        {
            const FVector A((I & 1) ? 0.5 : -0.5, Bottom, (I & 2) ? 0.5 : -0.5);
            const int32 J = I ^ 1, K = I ^ 2;
            if (I < J) Line(A, FVector((J & 1) ? 0.5 : -0.5, Bottom, (J & 2) ? 0.5 : -0.5));
            if (I < K) Line(A, FVector((K & 1) ? 0.5 : -0.5, Bottom, (K & 2) ? 0.5 : -0.5));
            Line(A, FVector(A.X, Top, A.Z));
            if (!bInfinite)
            {
                if (I < J) Line(FVector(A.X, Top, A.Z), FVector((J & 1) ? 0.5 : -0.5, Top, (J & 2) ? 0.5 : -0.5));
                if (I < K) Line(FVector(A.X, Top, A.Z), FVector((K & 1) ? 0.5 : -0.5, Top, (K & 2) ? 0.5 : -0.5));
            }
        }
        return;
    }
    constexpr int32 Segments = 32;
    for (int32 I = 0; I < Segments; ++I)
    {
        const double A = 2 * PI * I / Segments, B = 2 * PI * (I + 1) / Segments;
        const double C = FMath::Cos(A), S = FMath::Sin(A), D = FMath::Cos(B), T = FMath::Sin(B);
        if (R.Shape == EKyaPrimitiveShape::Sphere)
        {
            Line(FVector(C, S, 0), FVector(D, T, 0));
            Line(FVector(C, 0, S), FVector(D, 0, T));
            Line(FVector(0, C, S), FVector(0, D, T));
        }
        else
        {
            Line(FVector(C, -0.5, S), FVector(D, -0.5, T));
            if (R.Shape == EKyaPrimitiveShape::Cylinder) Line(FVector(C, 0.5, S), FVector(D, 0.5, T));
            if (I % 8 == 0) Line(FVector(C, -0.5, S), R.Shape == EKyaPrimitiveShape::Cone ? FVector(0, 0.5, 0) : FVector(C, 0.5, S));
        }
    }
}

void KyaPrimitives::BuildTriangles(const FKyaPrimitiveRecord& R, TArray<FVector>& Triangles)
{
    const auto Triangle = [&R, &Triangles](const FVector& A, const FVector& B, const FVector& C)
    {
        const FVector X = R.LocalToGame.TransformPosition(A), Y = R.LocalToGame.TransformPosition(B), Z = R.LocalToGame.TransformPosition(C);
        if (!X.ContainsNaN() && !Y.ContainsNaN() && !Z.ContainsNaN()) Triangles.Append({X, Y, Z});
    };
    const auto Quad = [&Triangle](const FVector& A, const FVector& B, const FVector& C, const FVector& D)
    {
        Triangle(A, B, C); Triangle(A, C, D);
    };
    if (!R.Vertices.IsEmpty())
    {
        for (int32 I = 1; I + 1 < R.Vertices.Num(); ++I) Triangle(R.Vertices[0], R.Vertices[I], R.Vertices[I + 1]);
        return;
    }
    if (R.Shape == EKyaPrimitiveShape::Box || R.Shape == EKyaPrimitiveShape::OpenTopBox || R.Shape == EKyaPrimitiveShape::HalfSpace)
    {
        const double Bottom = R.Shape == EKyaPrimitiveShape::Box ? -0.5 : 0;
        const double Top = R.Shape == EKyaPrimitiveShape::Box ? 0.5 : 2;
        Quad(FVector(-0.5, Bottom, -0.5), FVector(0.5, Bottom, -0.5), FVector(0.5, Bottom, 0.5), FVector(-0.5, Bottom, 0.5));
        if (R.Shape == EKyaPrimitiveShape::HalfSpace) return;
        const FVector Corners[] = {{-0.5, Bottom, -0.5}, {0.5, Bottom, -0.5}, {0.5, Bottom, 0.5}, {-0.5, Bottom, 0.5}};
        for (int32 I = 0; I < 4; ++I)
        {
            const FVector A = Corners[I], B = Corners[(I + 1) % 4];
            Quad(A, B, FVector(B.X, Top, B.Z), FVector(A.X, Top, A.Z));
        }
        if (R.Shape == EKyaPrimitiveShape::Box)
            Quad(FVector(-0.5, Top, -0.5), FVector(-0.5, Top, 0.5), FVector(0.5, Top, 0.5), FVector(0.5, Top, -0.5));
        return;
    }
    constexpr int32 Segments = 32, Rings = 16;
    const auto SpherePoint = [](double Longitude, double Latitude)
    {
        return FVector(FMath::Cos(Longitude) * FMath::Cos(Latitude), FMath::Sin(Latitude), FMath::Sin(Longitude) * FMath::Cos(Latitude));
    };
    for (int32 I = 0; I < Segments; ++I)
    {
        const double A = 2 * PI * I / Segments, B = 2 * PI * (I + 1) / Segments;
        if (R.Shape == EKyaPrimitiveShape::Sphere)
        {
            for (int32 J = 0; J < Rings; ++J)
            {
                const double C = PI * (double(J) / Rings - 0.5), D = PI * (double(J + 1) / Rings - 0.5);
                // Avoid degenerate triangles at the poles.
                if (J > 0) Triangle(SpherePoint(A, C), SpherePoint(B, C), SpherePoint(B, D));
                if (J + 1 < Rings) Triangle(SpherePoint(A, C), SpherePoint(B, D), SpherePoint(A, D));
            }
        }
        else
        {
            const FVector C(FMath::Cos(A), -0.5, FMath::Sin(A)), D(FMath::Cos(B), -0.5, FMath::Sin(B));
            Triangle(FVector(0, -0.5, 0), D, C);
            if (R.Shape == EKyaPrimitiveShape::Cone) Triangle(C, D, FVector(0, 0.5, 0));
            else
            {
                Quad(C, D, FVector(D.X, 0.5, D.Z), FVector(C.X, 0.5, C.Z));
                Triangle(FVector(0, 0.5, 0), FVector(C.X, 0.5, C.Z), FVector(D.X, 0.5, D.Z));
            }
        }
    }
}
