#include "Actors/KyaAddOnPurchase.h"
#include "KyaLevelBank.h"

namespace
{
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
int64 ReadHash(const uint8* P)
{
    const uint64 Bits = uint64(Read(P)) | uint64(Read(P + 4)) << 32;
    int64 Value;
    FMemory::Memcpy(&Value, &Bits, 8);
    return Value;
}
void Write(uint8* P, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) P[I] = uint8(Value >> (I * 8));
}
void WriteHash(uint8* P, int64 Value)
{
    Write(P, uint32(uint64(Value)));
    Write(P + 4, uint32(uint64(Value) >> 32));
}
}

bool KyaReadAddOnPurchaseOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAddOnPurchaseOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint8* P = Data.GetData() + Start;
    Out.bPresent = Read(P) != 0;
    if (!Out.bPresent)
    {
        if (Size != 4) return false;
    }
    else if (Size != 36)
    {
        if (Size != 40 && Size != 52) return false;
        Out.bHasObjectFlag = true;
        Out.bHasObject = Read(P + 36) != 0;
        if (Size != (Out.bHasObject ? 52 : 40)) return false;
    }
    Out.Tail = Start + Size;
    return true;
}

bool KyaReadAddOnPurchaseActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaAddOnPurchaseOffsets Off;
    if (!KyaReadAddOnPurchaseOffsets(Data, Start, Size, Off)) return false;
    const uint8* P = Data.GetData() + Start;
    FKyaPurchaseInventory& Info = Actor.AddOnPurchaseInventory;
    Info = {};
    Info.Presence = int32(Read(P));
    Info.bHasObjectFlag = Off.bHasObjectFlag;
    Info.bHasObject = Off.bHasObject;
    if (Off.bPresent)
    {
        Info.Field8 = ReadHash(P + 4);
        Info.Field10 = ReadHash(P + 12);
        Info.PurchaseId = int32(Read(P + 20));
        Info.Field28 = int32(Read(P + 24));
        Info.MoneyCost = int32(Read(P + 28));
        Info.Field2C = int32(Read(P + 32));
        if (Off.bHasObjectFlag) Info.ObjectFlag = int32(Read(P + 36));
        if (Off.bHasObject)
        {
            Info.ObjectId = int32(Read(P + 40));
            Info.Field18 = ReadHash(P + 44);
        }
    }
    Actor.bHasAddOnPurchaseData = true;
    Actor.ClassData.Reset();
    return true;
}

bool KyaWriteAddOnPurchaseActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaAddOnPurchaseOffsets Off;
    if (!Actor.bHasAddOnPurchaseData || !KyaReadAddOnPurchaseOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off)) return false;
    const FKyaPurchaseInventory& Info = Actor.AddOnPurchaseInventory;
    uint8* P = Data.GetData() + Start;
    // Presence and object flags select the layout and retain their original bit patterns.
    if (uint32(Info.Presence) != Read(P) || Info.bHasObjectFlag != Off.bHasObjectFlag ||
        Info.bHasObject != Off.bHasObject || (Off.bHasObjectFlag && uint32(Info.ObjectFlag) != Read(P + 36))) return false;
    if (Off.bPresent)
    {
        WriteHash(P + 4, Info.Field8);
        WriteHash(P + 12, Info.Field10);
        Write(P + 20, uint32(Info.PurchaseId));
        Write(P + 24, uint32(Info.Field28));
        Write(P + 28, uint32(Info.MoneyCost));
        Write(P + 32, uint32(Info.Field2C));
        if (Off.bHasObject)
        {
            Write(P + 40, uint32(Info.ObjectId));
            WriteHash(P + 44, Info.Field18);
        }
    }
    return true;
}
