#include "KyaMessageLog.h"

#include "Editor.h"
#include "EngineUtils.h"
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
        const FString Label = Bank ? FString::Printf(TEXT("%s [%d]"), *Bank->GetName(), ActorIndex)
            : FString::Printf(TEXT("Actor %d"), ActorIndex);
        const TWeakObjectPtr<UKyaLevelBank> WeakBank(Bank);
        Entry->AddToken(FActionToken::Create(
            FText::FromString(FString::Printf(TEXT("Select %s"), *Label)),
            NSLOCTEXT("KyaLevelEditor", "SelectBankActorTip",
                "Select and focus this actor's generated representation in the editor."),
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
