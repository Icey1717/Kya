#include "KyaActorFamily.h"
extern const FKyaActorFamily GKyaHunterActorFamily;
extern const FKyaActorFamily GKyaHelperSignActorFamily;
extern const FKyaActorFamily GKyaWoodMonsterActorFamily;
extern const FKyaActorFamily GKyaShockerCmdActorFamily;
extern const FKyaActorFamily GKyaAcceleratosActorFamily;
extern const FKyaActorFamily GKyaAddOnPurchaseActorFamily;
extern const FKyaActorFamily GKyaAmortosCmdActorFamily;
extern const FKyaActorFamily GKyaPatternCmdActorFamily;
extern const FKyaActorFamily GKyaEggActorFamily;
extern const FKyaActorFamily GKyaCareBoyActorFamily;
extern const FKyaActorFamily GKyaElectrollaActorFamily;
extern const FKyaActorFamily GKyaBombLauncherActorFamily;
extern const FKyaActorFamily GKyaBlazerActorFamily;
extern const FKyaActorFamily GKyaWoofActorFamily;
extern const FKyaActorFamily GKyaMiniGamesManagerActorFamily;
extern const FKyaActorFamily GKyaMiniGamesOrganizerActorFamily;
extern const FKyaActorFamily GKyaCreditsActorFamily;
extern const FKyaActorFamily GKyaWantedZooActorFamily;
extern const FKyaActorFamily GKyaJamGutActorFamily;

extern const FKyaActorFamily GKyaBoomyActorFamily;
extern const FKyaActorFamily GKyaKimHeroActorFamily;
extern const FKyaActorFamily GKyaMickenActorFamily;
extern const FKyaActorFamily GKyaWolfenActorFamily;
extern const FKyaActorFamily GKyaNativActorFamily;
extern const FKyaActorFamily GKyaMovingPlatformActorFamily;
extern const FKyaActorFamily GKyaBonusActorFamily;
extern const FKyaActorFamily GKyaRopeActorFamily;
extern const FKyaActorFamily GKyaGravityAwareActorFamily;
extern const FKyaActorFamily GKyaSwitchActorFamily;
extern const FKyaActorFamily GKyaWindActorFamily;
extern const FKyaActorFamily GKyaCommanderActorFamily;
extern const FKyaActorFamily GKyaAmbreActorFamily;
extern const FKyaActorFamily GKyaFruitActorFamily;
extern const FKyaActorFamily GKyaBridgeActorFamily;
extern const FKyaActorFamily GKyaCompanionActorFamily;
extern const FKyaActorFamily GKyaDcaActorFamily;
extern const FKyaActorFamily GKyaProjectileActorFamily;
extern const FKyaActorFamily GKyaCheckpointManagerActorFamily;
extern const FKyaActorFamily GKyaWeaponActorFamily;
extern const FKyaActorFamily GKyaAraignosActorFamily;
extern const FKyaActorFamily GKyaClusteriserActorFamily;
extern const FKyaActorFamily GKyaHedgehogActorFamily;
extern const FKyaActorFamily GKyaTeleporterActorFamily;
extern const FKyaActorFamily GKyaAmortosActorFamily;
extern const FKyaActorFamily GKyaTrapActorFamily;
extern const FKyaActorFamily GKyaFxActorFamily;
extern const FKyaActorFamily GKyaMoneyActorFamily;
extern const FKyaActorFamily GKyaBoxActorFamily;
extern const FKyaActorFamily GKyaAtonActorFamily;
extern const FKyaActorFamily GKyaBasicBoxActorFamily;
extern const FKyaActorFamily GKyaNativCommandActorFamily;
extern const FKyaActorFamily GKyaFogManagerActorFamily;
extern const FKyaActorFamily GKyaShootActorFamily;
extern const FKyaActorFamily GKyaStillerActorFamily;
extern const FKyaActorFamily GKyaRuneActorFamily;
extern const FKyaActorFamily GKyaBonusFountainActorFamily;
const FKyaActorFamily* const Families[] = {
    &GKyaHunterActorFamily,
    &GKyaHelperSignActorFamily,
    &GKyaWoodMonsterActorFamily,
    &GKyaShockerCmdActorFamily,
    &GKyaAcceleratosActorFamily,
    &GKyaAddOnPurchaseActorFamily,
    &GKyaAmortosCmdActorFamily,
    &GKyaPatternCmdActorFamily,
    &GKyaEggActorFamily,
    &GKyaCareBoyActorFamily,
    &GKyaElectrollaActorFamily,
    &GKyaBombLauncherActorFamily,
    &GKyaBlazerActorFamily,
    &GKyaWoofActorFamily,
    &GKyaMiniGamesManagerActorFamily,
    &GKyaMiniGamesOrganizerActorFamily,
    &GKyaCreditsActorFamily,
    &GKyaWantedZooActorFamily,
    &GKyaJamGutActorFamily,
    &GKyaBoomyActorFamily,
    &GKyaKimHeroActorFamily,
    &GKyaMickenActorFamily,
    &GKyaWolfenActorFamily,
    &GKyaNativActorFamily,
    &GKyaMovingPlatformActorFamily,
    &GKyaBonusActorFamily,
    &GKyaRopeActorFamily,
    &GKyaGravityAwareActorFamily,
    &GKyaSwitchActorFamily,
    &GKyaWindActorFamily,
    &GKyaCommanderActorFamily,
    &GKyaAmbreActorFamily,
    &GKyaFruitActorFamily,
    &GKyaBridgeActorFamily,
    &GKyaCompanionActorFamily,
    &GKyaDcaActorFamily,
    &GKyaProjectileActorFamily,
    &GKyaCheckpointManagerActorFamily,
    &GKyaWeaponActorFamily,
    &GKyaAraignosActorFamily,
    &GKyaClusteriserActorFamily,
    &GKyaHedgehogActorFamily,
    &GKyaTeleporterActorFamily,
    &GKyaAmortosActorFamily,
    &GKyaTrapActorFamily,
    &GKyaFxActorFamily,
    &GKyaMoneyActorFamily,
    &GKyaBoxActorFamily,
    &GKyaAtonActorFamily,
    &GKyaBasicBoxActorFamily,
    &GKyaNativCommandActorFamily,
    &GKyaFogManagerActorFamily,
    &GKyaShootActorFamily,
    &GKyaStillerActorFamily,
    &GKyaRuneActorFamily,
    &GKyaBonusFountainActorFamily,
};

const FKyaActorFamilyReference Shared[] = {
    {TEXT("SectorId"), EKyaReferenceKind::Sector},
    {TEXT("MeshIndex"), EKyaReferenceKind::Mesh},
    {TEXT("TextureIndex"), EKyaReferenceKind::Texture},
    {TEXT("DefaultBehaviourId"), EKyaReferenceKind::Behaviour},
};

TConstArrayView<const FKyaActorFamily*> KyaActorFamilies::All()
{
    return MakeArrayView(Families);
}

const FKyaActorFamily* KyaActorFamilies::Find(int64 ClassId)
{
    for (const FKyaActorFamily* Family : Families)
        if (Family->ClassId == ClassId) return Family;
    return nullptr;
}

const FKyaActorFamilyBehaviour* KyaActorFamilies::FindBehaviour(int64 ClassId, int32 BehaviourId)
{
    const FKyaActorFamily* Family = Find(ClassId);
    if (!Family) return nullptr;
    for (const FKyaActorFamilyBehaviour& Behaviour : Family->Behaviours)
        if (Behaviour.Id == BehaviourId) return &Behaviour;
    return nullptr;
}

TConstArrayView<FKyaActorFamilyReference> KyaActorFamilies::SharedReferences()
{
    return Shared;
}

bool KyaActorFamilies::HasEmptyPayload(int64 ClassId, int32 BehaviourId)
{
    // Keep this explicit: an unknown zero-sized payload does not prove emptiness.
    if (BehaviourId == 0)
    {
        // These recovered BuildBehaviour chains delegate id 0 to CActor;
        // CBehaviourStand::Create is explicitly empty in Actor.h.
        switch (ClassId)
        {
        case 0x0a: // Nativ -> Autonomous
        case 0x0b: // Electrolla -> Actor
        case 0x27: // WoodMonster -> Actor
        case 0x2a: // Woof -> Autonomous
        case 0x2b: // HelperSign -> Actor
        case 0x2f: // Egg -> Actor
        case 0x31: // AmortosCmd -> Actor
        case 0x33: // JamGut -> Autonomous
        case 0x37: // CareBoy -> Actor
        case 0x3a: // Acceleratos -> Actor
        case 0x40: // PatternCmd -> Actor
        case 0x42: // MiniGamesManager -> Actor
        case 0x43: // MiniGamesOrganizer -> Actor
        case 0x48: // Hunter -> Autonomous
        case 0x49: // AddOnPurchase -> Actor
        case 0x4d: // Blazer -> Autonomous
        case 0x50: // ShockerCmd -> Actor
        case 0x53: // BombLauncher -> Actor
        case 0x54: // WantedZoo inherits Actor
        case 0x56: // Credits -> Actor
            return true;
        default:
            return false;
        }
    }
    if (ClassId == 0x48) return BehaviourId == 3; // HunterWatch::Create
    if (ClassId == 0x4d) return BehaviourId == 3; // BlazerStand::Create
    if (ClassId == 0x2a) return BehaviourId == 3 || BehaviourId == 4; // Woof Guard/VerticalJump::Create
    if (ClassId == 0x0a) return BehaviourId == 4; // NativExorcisme inherits empty Nativ::Create
    if (ClassId == 0x33) return BehaviourId == 3; // JamGutStand::Create
    if (BehaviourId != 2) return false;
    switch (ClassId)
    {
    case 0x0b: // CBehaviourElectrolla::Create
    case 0x2f: // EggMove::Create
    case 0x31: // AmortosCmdStand::Create
    case 0x37: // CareBoySteady::Create
    case 0x3a: // AcceleratosStand::Create
    case 0x40: // PatternCmdStand::Create
    case 0x42: // MiniGamesManagerStand::Create
    case 0x43: // MiniGamesOrganizerStand::Create
    case 0x49: // CBhvAddOnPurchase::Create
    case 0x50: // ShockerCmdStand::Create
    case 0x53: // BombLauncherStand::Create
    case 0x56: // CreditsDefault inherits empty CBehaviour::Create
        return true;
    default:
        return false;
    }
}

bool KyaActorFamilies::UsesCatchByTrap(int64 ClassId, int32 BehaviourId)
{
    if (BehaviourId != 2) return false;
    switch (ClassId)
    {
    case 0x07: // Micken
    case 0x0a: // Nativ
    case 0x1c: // Projectile
    case 0x22: // Hedgehog
    case 0x2a: // Woof
    case 0x33: // JamGut
    case 0x38: // Aton
    case 0x3e: // Shoot
    case 0x48: // Hunter
    case 0x4d: // Blazer
        return true;
    default:
        return false;
    }
}
