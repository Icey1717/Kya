#pragma once

#include "CoreMinimal.h"
#include "Logging/TokenizedMessage.h"

class AActor;
class UKyaLevelBank;

/**
 * Tokenized Message Log entries with navigation links.
 *
 * Every entry names the affected bank asset as a clickable token and can add a
 * "Select ..." action that selects the matching generated actor handle (or the
 * preview/level-set context actor) in the current editor world. Validation, sector
 * discovery and export use these so a problem can be inspected without searching
 * raw logs.
 */
TSharedRef<FTokenizedMessage> KyaMakeBankMessage(EMessageSeverity::Type Severity, UKyaLevelBank* Bank,
    const FText& Message, int32 ActorIndex = INDEX_NONE);

TSharedRef<FTokenizedMessage> KyaMakeActorMessage(EMessageSeverity::Type Severity, UKyaLevelBank* Bank,
    AActor* ContextActor, const FText& Message, int32 ActorIndex = INDEX_NONE);

/**
 * Validation/export entry that names the bank, the actor identity (index, serialized
 * name, class label) and the property path, with a "Select ..." action that selects
 * the generated representation so the level Details panel shows the affected record.
 */
TSharedRef<FTokenizedMessage> KyaMakeValidationIssueMessage(EMessageSeverity::Type Severity,
    UKyaLevelBank* Bank, int32 ActorIndex, const FString& FieldPath, const FText& Message);

/** Open the bank asset editor. */
void KyaOpenBankAsset(UKyaLevelBank* Bank);

/** Select (and focus) the generated handle for one bank actor in every preview. */
bool KyaSelectBankActor(UKyaLevelBank* Bank, int32 ActorIndex, UWorld* World = nullptr);
