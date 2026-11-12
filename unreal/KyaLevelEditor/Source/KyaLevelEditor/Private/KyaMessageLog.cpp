#include "KyaMessageLog.h"

#include "Editor.h"
#include "EngineUtils.h"
#include "KyaActorCatalog.h"
#include "KyaActorHandle.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "Subsystems/AssetEditorSubsystem.h"

void KyaOpenBankAsset(UKyaLevelBank* Bank)
{
    if (!Bank || !GEditor) return;
    if (UAssetEditorSubsystem* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
        AssetEditor->OpenEditorForAsset(Bank);
}

bool KyaSelectBankActor(UKyaLevelBank* Bank, int32 ActorIndex, UWorld* World)
{
    if (!Bank || ActorIndex < 0 || !GEditor) return false;
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }
    if (!World) return false;
    AKyaActorHandle* Match = nullptr;
    for (TActorIterator<AKyaLevelPreview> It(World); It && !Match; ++It)
    {
        if (It->Bank != Bank) continue;
        if (AKyaActorHandle* Handle = Cast<AKyaActorHandle>(It->GetActorHandle(ActorIndex)))
            Match = Handle;
    }
    if (!Match) return false;
    GEditor->SelectNone(false, true, false);
    GEditor->SelectActor(Match, true, true);
    if (World == GEditor->GetEditorWorldContext().World())
        GEditor->MoveViewportCamerasToActor(*Match, true);
    return true;
}

TSharedRef<FTokenizedMessage> KyaMakeActorMessage(EMessageSeverity::Type Severity, UKyaLevelBank* Bank,
    AActor* ContextActor, const FText& Message, int32 ActorIndex)
{
    const TSharedRef<FTokenizedMessage> Entry = FTokenizedMessage::Create(Severity, Message);
    if (Bank)
    {
        Entry->AddToken(FAssetNameToken::Create(Bank->GetPathName(),
            FText::FromString(Bank->GetName())));
    }
    if (ActorIndex >= 0)
    {
        FString ActorName;
        if (Bank && Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex))
        {
            const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[ActorIndex];
            ActorName = Placement.Name.IsEmpty()
                ? FString::Printf(TEXT("#%d"), ActorIndex) : Placement.Name;
        }
        const FString Label = Bank
            ? FString::Printf(TEXT("%s: actor %d \"%s\""), *Bank->GetName(), ActorIndex, *ActorName)
            : FString::Printf(TEXT("Actor %d"), ActorIndex);
        const TWeakObjectPtr<UKyaLevelBank> WeakBank(Bank);
        Entry->AddToken(FActionToken::Create(
            FText::FromString(FString::Printf(TEXT("Select %s"), *Label)),
            NSLOCTEXT("KyaLevelEditor", "SelectBankActorTip",
                "Select and focus this actor's generated representation; the Details panel then shows the record."),
            FOnActionTokenExecuted::CreateLambda([WeakBank, ActorIndex]()
            {
                UKyaLevelBank* Target = WeakBank.Get();
                if (!Target) return;
                if (!KyaSelectBankActor(Target, ActorIndex)) KyaOpenBankAsset(Target);
            })));
    }
    else if (ContextActor)
    {
        const TWeakObjectPtr<AActor> WeakActor(ContextActor);
        Entry->AddToken(FActionToken::Create(
            NSLOCTEXT("KyaLevelEditor", "SelectContextActor", "Select preview"),
            NSLOCTEXT("KyaLevelEditor", "SelectContextActorTip",
                "Select and focus the actor that owns this problem."),
            FOnActionTokenExecuted::CreateLambda([WeakActor]()
            {
                AActor* Actor = WeakActor.Get();
                if (!Actor || !GEditor) return;
                GEditor->SelectNone(false, true, false);
                GEditor->SelectActor(Actor, true, true);
                GEditor->MoveViewportCamerasToActor(*Actor, true);
            })));
    }
    return Entry;
}

TSharedRef<FTokenizedMessage> KyaMakeBankMessage(EMessageSeverity::Type Severity, UKyaLevelBank* Bank,
    const FText& Message, int32 ActorIndex)
{
    return KyaMakeActorMessage(Severity, Bank, nullptr, Message, ActorIndex);
}

TSharedRef<FTokenizedMessage> KyaMakeValidationIssueMessage(EMessageSeverity::Type Severity,
    UKyaLevelBank* Bank, int32 ActorIndex, const FString& FieldPath, const FText& Message)
{
    FString Identity = TEXT("Bank");
    if (Bank && Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex))
    {
        const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[ActorIndex];
        const FString Name = Placement.Name.IsEmpty() ? TEXT("<unnamed>") : Placement.Name;
        Identity = FString::Printf(TEXT("Actor %d \"%s\" (%s)"), ActorIndex, *Name,
            *GetKyaActorClassLabel(Placement.ClassId));
    }
    const FString Path = FieldPath.IsEmpty() ? FString() : FString::Printf(TEXT(".%s"), *FieldPath);
    const FString Text = FString::Printf(TEXT("%s%s: %s"), *Identity, *Path, *Message.ToString());
    return KyaMakeActorMessage(Severity, Bank, nullptr, FText::FromString(Text),
        Bank && Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex) ? ActorIndex : INDEX_NONE);
}
