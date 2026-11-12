#pragma once

#include "CoreMinimal.h"
#include "KyaLevelBank.h"
#include "KyaLevelSettings.generated.h"

/**
 * Editable authoring view of one bank's level environment, camera, fog and scene
 * configuration.
 *
 * The object is transient (created by the level preview or the Level Settings tab),
 * loads a copy from the bank and writes the whole settings block back through the
 * shared edit service, so one edit is one undo step and every preview refreshes.
 * Render/capture toggles stay on the preview under Kya|Diagnostics and are not part
 * of this authoring surface.
 */
UCLASS(BlueprintType)
class KYALEVEL_API UKyaLevelSettings : public UObject
{
    GENERATED_BODY()
public:
    /** Bank whose settings are being edited. */
    UPROPERTY(EditAnywhere, Category="Kya|Level")
    TObjectPtr<UKyaLevelBank> Bank;

    UPROPERTY(EditAnywhere, Category="Kya|Environment")
    FKyaSceneEnvironment Environment;

    UPROPERTY(EditAnywhere, Category="Kya|Scene configuration")
    FKyaSceneConfiguration SceneConfiguration;

    /** Copy the bank's current environment and scene configuration into this object. */
    UFUNCTION(BlueprintCallable, Category="Kya|Level")
    void LoadFromBank();

    /** Write the edited settings back to the bank through the edit service. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category="Kya|Level")
    bool ApplyToBank(FString& OutError);

    /** Last apply result, shown in the settings Details panel. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Level")
    FString ApplyStatus;

#if WITH_EDITOR
    /** Auto-apply property edits as one undoable settings write. */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    virtual void BeginDestroy() override;

private:
    FDelegateHandle BankEditedHandle;
    FDelegateHandle BankTransactedHandle;
    bool bApplying = false;
    bool bReloadScheduled = false;
    void RefreshFromBank();
    void ScheduleRefresh();
};
