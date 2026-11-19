#pragma once
#include "CoreMinimal.h"
#include "KyaGeometryDocument.h"
struct FKyaBankEntry;

namespace KyaGeometry
{
    /** Builds a replacement static COL entry, then decodes it back. No bank mutation. */
    KYALEVEL_API bool EncodeCollision(const FKyaGeometryDocument& Document, TArray<uint8>& OutBytes, FString& OutError);
    /** Enumerates prepared root cluster strips. Unsupported spatial layouts fail
     * explicitly rather than producing a partially editable generic mesh. */
    KYALEVEL_API bool OpenRootRender(UKyaLevelBank& Bank, int32 EntryIndex,
        TArray<FKyaGeometryDocument>& OutDocuments, FString& OutError);
    KYALEVEL_API bool EncodeRootRender(TConstArrayView<FKyaGeometryDocument> Documents,
        TArray<uint8>& OutBytes, FString& OutError);
    /** Appends aligned replacement entries, keeping unrelated payloads verbatim.
     * Rebuilds entry locations, terminal record and bank size accounting. Empty
     * replacement maps produce byte-identical output. Does not mutate the bank. */
    KYALEVEL_API bool ReplaceBankEntries(const UKyaLevelBank& Bank, const TMap<int32, TArray<uint8>>& Replacements,
        TArray<uint8>& OutBytes, TArray<FKyaBankEntry>& OutEntries, FString& OutError);
    /** Revalidates applied resources against the complete staged output bank. */
    KYALEVEL_API bool ValidateAppliedGeometry(const UKyaLevelBank& Bank, const TArray<uint8>& OutputBytes, FString& OutError);
}
