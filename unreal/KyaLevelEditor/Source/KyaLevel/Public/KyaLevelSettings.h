#pragma once

#include "CoreMinimal.h"
#include "KyaLevelBank.h"
#include "KyaLevelSettingsEnvironment.h"
#include "KyaLevelSettings.generated.h"

/**
 * Editable authoring view of one bank's level environment, camera, fog and scene
 * configuration.
 *
 * The object is transient (created by the level preview or the Level Settings tab),
 * holds a settings draft until Save writes it through the shared edit service
 * as one undo step. Placement arrays are excluded from the draft.
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
    FKyaLevelSettingsEnvironment Environment;

    UPROPERTY(EditAnywhere, Category="Kya|Scene configuration")
    FKyaSceneConfiguration SceneConfiguration;

    /** Copy the bank's current environment and scene configuration into this object. */
    UFUNCTION(BlueprintCallable, Category="Kya|Level")
    void LoadFromBank();

    /** Write the edited settings back to the bank through the edit service. */
    UFUNCTION(BlueprintCallable, Category="Kya|Level")
    bool ApplyToBank(FString& OutError);

    /** Last apply result, shown in the settings Details panel. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Level")
    FString ApplyStatus;

#if WITH_EDITOR
    /** Edit only the draft; selecting a bank loads a new draft. */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    bool bApplying = false;
};
