#include "Actors/KyaNativAkasa.h"
#include "Actors/KyaSwitch.h"
#include "KyaLevelBank.h"
namespace
{
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
void Write(TArray<uint8>& Data, int64 Pos, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) Data[int32(Pos) + I] = uint8(Value >> (I * 8));
}
struct FNameSpan { int64 Start = 0, End = 0; };
struct FLayout
{
    TArray<FNameSpan> Names;
    TArray<int64> AddOns;
    FKyaSwitchActorOffsets Switches[4];
    bool bModern = false;
};
bool Parse(TConstArrayView<uint8> Data, int64 Start, int64 Size, bool Modern,
    FKyaAkasaBehaviour& Out, FLayout& Layout)
{
    Out = {};
    Layout = {};
    Layout.bModern = Modern;
    const int64 End = Start + Size;
    int64 Pos = Start;
    auto Word = [&](uint32& Value) -> bool
    {
        if (End - Pos < 4) return false;
        Value = Read(Data.GetData() + Pos);
        Pos += 4;
        return true;
    };
    auto Count = [&](uint32& Value, int64 Minimum) -> bool
    {
        return Word(Value) && Value <= MAX_int32 && uint64(Value) <= uint64(End - Pos) / uint64(Minimum);
    };
    uint32 Value;
    Out.bHasMaterial = Modern;
    if (Modern)
    {
        if (!Word(Value)) return false;
        Out.MaterialId = int32(Value);
    }
    if (!Word(Value)) return false;
    Out.Field8 = Value;
    uint32 Tutorials;
    if (!Count(Tutorials, 4)) return false;
    for (uint32 T = 0; T < Tutorials; ++T)
    {
        uint32 Steps;
        if (!Count(Steps, 4)) return false;
        FKyaAkasaTutorial& Tutorial = Out.Tutorials.AddDefaulted_GetRef();
        for (uint32 S = 0; S < Steps; ++S)
        {
            uint32 Names;
            if (!Word(Names) || Names > 2) return false; // fixed two-combo runtime array
            FKyaAkasaStep& Step = Tutorial.Steps.AddDefaulted_GetRef();
            for (uint32 N = 0; N < Names; ++N)
            {
                FNameSpan Span;
                Span.Start = Pos;
                FString Name;
                bool Terminated = false;
                while (Pos < End)
                {
                    const uint8 Byte = Data[int32(Pos++)];
                    if (Byte == 0) { Terminated = true; break; }
                    if (Byte > 127) return false; // preserve unsupported encodings as raw
                    Name.AppendChar(TCHAR(Byte));
                }
                if (!Terminated) return false;
                Pos = (Pos + 3) & ~int64(3); // ByteCode::GetString
                if (Pos > End) return false;
                Span.End = Pos;
                Layout.Names.Add(Span);
                Step.ComboNames.Add(MoveTemp(Name));
            }
        }
    }
    uint32 AddOns;
    if (!Count(AddOns, 8)) return false;
    for (uint32 A = 0; A < AddOns; ++A)
    {
        Layout.AddOns.Add(Pos);
        FKyaAddOnSubObj& Sub = Out.AddOnSubObjs.AddDefaulted_GetRef();
        if (!Word(Value)) return false;
        Sub.Field0 = int32(Value);
        uint32 Cinematics;
        if (!Count(Cinematics, 4)) return false;
        for (uint32 C = 0; C < Cinematics; ++C)
        {
            if (!Word(Value)) return false;
            Sub.CinematicIds.Add(int32(Value));
        }
    }
    for (int32 I = 0; I < 4; ++I)
    {
        FKyaSwitchActorOffsets& Switch = Layout.Switches[I];
        if (!KyaReadSwitchActorOffsets(Data, Pos, End - Pos, Switch)) return false;
        FKyaSwitch& Model = Out.Switches.AddDefaulted_GetRef();
        if (!KyaReadSwitchValue(Data, Switch.Targets, Switch.Count, Switch.Camera, Model)) return false;
        Pos = Switch.Tail;
    }
    return Pos == End;
}
bool Decode(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAkasaBehaviour& Out, FLayout& Layout)
{
    if (Start < 0 || Size < 156 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    FKyaAkasaBehaviour Old, New;
    FLayout OldLayout, NewLayout;
    const bool HasOld = Parse(Data, Start, Size, false, Old, OldLayout);
    const bool HasNew = Parse(Data, Start, Size, true, New, NewLayout);
    if (HasOld == HasNew) return false;
    Out = HasNew ? MoveTemp(New) : MoveTemp(Old);
    Layout = HasNew ? MoveTemp(NewLayout) : MoveTemp(OldLayout);
    return true;
}
}
bool KyaReadNativAkasaBehaviour(FKyaActorBehaviour& Out, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaAkasaBehaviour Model;
    FLayout Layout;
    if (!Decode(Data, Start, Size, Model, Layout)) return false;
    Out.AkasaBehaviour = MoveTemp(Model);
    Out.bHasTypedNativBehaviour = true;
    return true;
}
bool KyaWriteNativAkasaBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaAkasaBehaviour Original;
    FLayout Layout;
    if (Behaviour.Id != 8 || !Decode(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Original, Layout)) return false;
    const FKyaAkasaBehaviour& V = Behaviour.AkasaBehaviour;
    if (V.bHasMaterial != Original.bHasMaterial || V.Field8 < 0 || V.Field8 > MAX_uint32 ||
        V.Tutorials.Num() != Original.Tutorials.Num() || V.AddOnSubObjs.Num() != Original.AddOnSubObjs.Num() || V.Switches.Num() != 4) return false;
    int32 NameIndex = 0;
    for (int32 T = 0; T < V.Tutorials.Num(); ++T)
    {
        if (V.Tutorials[T].Steps.Num() != Original.Tutorials[T].Steps.Num()) return false;
        for (int32 S = 0; S < V.Tutorials[T].Steps.Num(); ++S)
        {
            const TArray<FString>& Names = V.Tutorials[T].Steps[S].ComboNames;
            if (Names.Num() != Original.Tutorials[T].Steps[S].ComboNames.Num()) return false;
            for (const FString& Name : Names)
            {
                if (!Layout.Names.IsValidIndex(NameIndex)) return false;
                const FNameSpan& Span = Layout.Names[NameIndex++];
                if (((Span.Start + Name.Len() + 1 + 3) & ~int64(3)) != Span.End) return false;
                for (TCHAR Char : Name) if (Char == 0 || uint32(Char) > 127) return false;
            }
        }
    }
    for (int32 A = 0; A < V.AddOnSubObjs.Num(); ++A)
        if (V.AddOnSubObjs[A].CinematicIds.Num() != Original.AddOnSubObjs[A].CinematicIds.Num()) return false;
    for (int32 I = 0; I < 4; ++I)
        if (V.Switches[I].Targets.Num() != Layout.Switches[I].Count) return false;
    if (V.bHasMaterial) Write(Data, Start, uint32(V.MaterialId));
    Write(Data, Start + (V.bHasMaterial ? 4 : 0), uint32(V.Field8));
    NameIndex = 0;
    for (const FKyaAkasaTutorial& Tutorial : V.Tutorials)
        for (const FKyaAkasaStep& Step : Tutorial.Steps)
            for (const FString& Name : Step.ComboNames)
            {
                const FNameSpan& Span = Layout.Names[NameIndex++];
                for (int32 C = 0; C < Name.Len(); ++C) Data[int32(Span.Start) + C] = uint8(Name[C]);
                Data[int32(Span.Start) + Name.Len()] = 0;
            }
    for (int32 A = 0; A < V.AddOnSubObjs.Num(); ++A)
    {
        const FKyaAddOnSubObj& Sub = V.AddOnSubObjs[A];
        const int64 Pos = Layout.AddOns[A];
        Write(Data, Pos, uint32(Sub.Field0));
        for (int32 C = 0; C < Sub.CinematicIds.Num(); ++C) Write(Data, Pos + 8 + int64(C) * 4, uint32(Sub.CinematicIds[C]));
    }
    for (int32 I = 0; I < 4; ++I)
        KyaWriteSwitchValue(Data, V.Switches[I], Layout.Switches[I].Targets, Layout.Switches[I].Camera);
    FKyaAkasaBehaviour Written;
    FLayout WrittenLayout;
    return Decode(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Written, WrittenLayout) && Written.bHasMaterial == V.bHasMaterial;
}
