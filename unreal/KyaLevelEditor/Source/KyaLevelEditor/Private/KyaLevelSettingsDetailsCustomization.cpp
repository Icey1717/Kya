#include "KyaLevelSettingsDetailsCustomization.h"

#include "DetailLayoutBuilder.h"
#include "KyaLevelSettings.h"

TSharedRef<IDetailCustomization> FKyaLevelSettingsDetailsCustomization::MakeInstance()
{
    return MakeShared<FKyaLevelSettingsDetailsCustomization>();
}

void FKyaLevelSettingsDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    // The tab displays status and actions in its fixed footer.
    DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UKyaLevelSettings, ApplyStatus));
}
