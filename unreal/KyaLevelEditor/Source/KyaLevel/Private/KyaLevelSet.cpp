#include "KyaLevelSet.h"

UKyaLevelBank* UKyaLevelSet::FindSector(int32 SectorId) const
{
    for (const FKyaLevelSetSector& Sector : Sectors)
        if (Sector.SectorId == SectorId) return Sector.Bank;
    return nullptr;
}

void UKyaLevelSet::SetSector(int32 SectorId, UKyaLevelBank* Bank, const FString& InSourceFilename, bool bManual)
{
    for (FKyaLevelSetSector& Sector : Sectors)
    {
        if (Sector.SectorId != SectorId) continue;
        Sector.Bank = Bank;
        if (!InSourceFilename.IsEmpty()) Sector.SourceFilename = InSourceFilename;
        Sector.bManualOverride = Sector.bManualOverride || bManual;
        return;
    }
    FKyaLevelSetSector& Added = Sectors.AddDefaulted_GetRef();
    Added.SectorId = SectorId;
    Added.Bank = Bank;
    Added.SourceFilename = InSourceFilename;
    Added.bManualOverride = bManual;
}

void UKyaLevelSet::RemoveInvalidAssociations()
{
    Sectors.RemoveAll([](const FKyaLevelSetSector& Sector) { return !Sector.Bank; });
}
