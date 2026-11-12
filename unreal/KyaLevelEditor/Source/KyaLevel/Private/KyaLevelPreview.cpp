#include "KyaLevelPreview.h"
#include "KyaG3DChunks.h"
#include "KyaG2D.h"
#include "KyaSceneRenderSettings.h"
#include "Misc/Paths.h"
#include "Misc/App.h"
#include "Misc/ScopedSlowTask.h"
#include "KyaLevelBank.h"
#include "KyaLevelSet.h"
#include "KyaLevelSettings.h"
#include "KyaWaypointComponent.h"
#include "KyaPathPointComponent.h"
#include "KyaActorMarkerComponent.h"
#include "KyaActorHandle.h"
#include "KyaActorGroup.h"
#include "KyaSpatialRecordHandle.h"
#include "KyaSceneVisualizer.h"
#include "KyaActorCatalog.h"
#include "KyaHierarchyHandle.h"
#include "KyaActorTransform.h"
#include "KyaActorEditService.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/TransactionObjectEvent.h"
#include "EngineUtils.h"
#include "Async/Async.h"
#include "HAL/CriticalSection.h"
#include "HAL/PlatformProcess.h"

#include "ProceduralMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#if WITH_EDITOR
#include "Engine/Engine.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionSceneColor.h"
#include "Materials/MaterialExpressionSceneTexture.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionScreenPosition.h"
#endif

/** Data-only actor mesh decode result, safe to build off the game thread. */
struct FKyaActorDecodeCache
{
    TWeakObjectPtr<UKyaLevelBank> Bank;
    int32 Generation = -1;
    int32 Serial = 0;
    TMap<int32, TArray<FKyaG3DDecodedStrip>> StripsByMesh;
};

namespace
{
struct FMaterialSectionKey
{
    uint64 Hash = 0;
    bool bBlend = false;
    bool operator==(const FMaterialSectionKey& Other) const { return Hash == Other.Hash && bBlend == Other.bBlend; }
    friend uint32 GetTypeHash(const FMaterialSectionKey& Key) { return HashCombine(GetTypeHash(Key.Hash), GetTypeHash(Key.bBlend)); }
};
using FMaterialSections = TMap<FMaterialSectionKey, int32>;

FCriticalSection GActorDecodeLock;
TMap<const UKyaLevelBank*, TSharedPtr<FKyaActorDecodeCache>> GActorDecodeCache;
FThreadSafeCounter GActorDecodeRequestId;

/** Hash the actor inputs that affect captured mesh sections and vertex lighting. */
uint64 HashActorSectionRevision(const FKyaActorPlacement& Actor, const UKyaLevelBank* Bank, int32 Generation,
    uint32 LightingRevision)
{
    uint32 Hash = ::GetTypeHash(Generation);
    Hash = HashCombine(Hash, ::GetTypeHash(Bank));
    Hash = HashCombine(Hash, LightingRevision);
    Hash = HashCombine(Hash, ::GetTypeHash(Actor.MeshIndex));
    Hash = HashCombine(Hash, ::GetTypeHash(Actor.TextureIndex));
    Hash = HashCombine(Hash, ::GetTypeHash(Actor.ClassId));
    Hash = HashCombine(Hash, ::GetTypeHash(Actor.SectorId));
    Hash = HashCombine(Hash, ::GetTypeHash(Actor.LightingValue));
    Hash = HashCombine(Hash, ::GetTypeHash(Actor.LightingFlags));
    const auto HashFloat = [&Hash](double Value)
    {
        Hash = HashCombine(Hash, ::GetTypeHash(FMath::RoundToInt(Value * 10000.0)));
    };
    HashFloat(Actor.Rotation.X); HashFloat(Actor.Rotation.Y); HashFloat(Actor.Rotation.Z);
    HashFloat(Actor.Scale.X); HashFloat(Actor.Scale.Y); HashFloat(Actor.Scale.Z);
    HashFloat(Actor.Position.X); HashFloat(Actor.Position.Y); HashFloat(Actor.Position.Z);
    for (const FKyaActorModelReference& Model : Actor.Models)
    {
        Hash = HashCombine(Hash, FCrc::StrCrc32(*Model.MeshName));
        Hash = HashCombine(Hash, FCrc::StrCrc32(*Model.TextureName));
    }
    Hash = HashCombine(Hash, ::GetTypeHash(Actor.Models.Num()));
    return uint64(Hash);
}

/** Hash all lighting inputs that affect captured vertex colours. */
uint32 HashActorLighting(const UKyaLevelBank* Bank, const TArray<TObjectPtr<UKyaLevelBank>>& Sectors, bool bApply)
{
    uint32 Hash = ::GetTypeHash(bApply);
    const auto HashBankLights = [&Hash](const UKyaLevelBank* Source)
    {
        if (!Source) return;
        Hash = HashCombine(Hash, ::GetTypeHash(Source->EditGeneration));
        Hash = HashCombine(Hash, ::GetTypeHash(Source->Lights.Num()));
        for (const FKyaLight& Light : Source->Lights)
        {
            Hash = HashCombine(Hash, ::GetTypeHash(Light.LightType));
            Hash = HashCombine(Hash, ::GetTypeHash(Light.ShapeType));
            Hash = HashCombine(Hash, FMath::RoundToInt(Light.Multiplier * 10000.0f));
            Hash = HashCombine(Hash, FMath::RoundToInt(Light.Falloff0 * 1000.0f));
            Hash = HashCombine(Hash, FMath::RoundToInt(Light.Falloff1 * 1000.0f));
            const float Channels[] = {Light.Color.R, Light.Color.G, Light.Color.B, Light.Color.A,
                Light.AmbientColor.R, Light.AmbientColor.G, Light.AmbientColor.B, Light.AmbientColor.A,
                Light.Direction.X, Light.Direction.Y, Light.Direction.Z,
                Light.Position.X, Light.Position.Y, Light.Position.Z};
            for (const float Channel : Channels)
                Hash = HashCombine(Hash, FMath::RoundToInt(Channel * 1000.0f));
        }
    };
    HashBankLights(Bank);
    for (const TObjectPtr<UKyaLevelBank>& Sector : Sectors) HashBankLights(Sector);
    return Hash;
}

void AppendUnrealTriangles(TArray<int32>& Target, TConstArrayView<uint32> Indices, int32 Base)
{
    for (int32 I = 0; I + 2 < Indices.Num(); I += 3)
    {
        Target.Add(Base + int32(Indices[I]));
        Target.Add(Base + int32(Indices[I + 1]));
        Target.Add(Base + int32(Indices[I + 2]));
    }
}

int32 GetSectorId(const UKyaLevelBank& Sector)
{
    FString Name = FPaths::GetBaseFilename(Sector.SourceFilename);
    if (!Name.RemoveFromStart(TEXT("SECT")))
    {
        Name = Sector.GetName();
        if (!Name.RemoveFromStart(TEXT("SECT"))) return INDEX_NONE;
    }
    return Name.IsNumeric() ? FCString::Atoi(*Name) : INDEX_NONE;
}

// Sector COL entries store vertices and primitive tables as file-relative offsets.
void FindSceneryGroundLight(const UKyaLevelBank& Sector, const FVector& Position, double& Height, uint16& Sample)
{
    for (const FKyaBankEntry& Entry : Sector.Entries)
    {
        if (Entry.Type != 7 || Entry.Subtype != 1 || Entry.Offset < 0 || Entry.Size < 88 ||
            Entry.Offset > Sector.BankBytes.Num() || Entry.Size > Sector.BankBytes.Num() - Entry.Offset)
            continue;
        const uint8* Data = Sector.BankBytes.GetData() + Entry.Offset;
        if (FMemory::Memcmp(Data + 12, ".COLCOLI", 8) != 0) continue;
        const auto ReadWord = [Data](int64 Offset) {
            uint32 Word;
            FMemory::Memcpy(&Word, Data + Offset, sizeof(Word));
            return Word;
        };
        const int64 Vertices = ReadWord(72);
        if (Vertices < 88 || Vertices >= Entry.Size) continue;
        const auto TestTriangle = [&](const FVector3f& A, const FVector3f& B, const FVector3f& C, uint32 Flags) {
            const double BX = B.X - A.X, BZ = B.Z - A.Z;
            const double CX = C.X - A.X, CZ = C.Z - A.Z;
            const double Determinant = BX * CZ - BZ * CX;
            if (FMath::IsNearlyZero(Determinant)) return;
            const double X = Position.X - A.X, Z = Position.Z - A.Z;
            const double U = (X * CZ - Z * CX) / Determinant;
            const double V = (BX * Z - BZ * X) / Determinant;
            if (U < 0.0 || V < 0.0 || U + V > 1.0) return;
            const double Y = A.Y + U * (B.Y - A.Y) + V * (C.Y - A.Y);
            if (Y > Height && Y <= Position.Y + 0.3)
            {
                Height = Y;
                Sample = uint16(Flags >> 16);
            }
        };
        const auto TestTable = [&](int64 Count, int64 Table, int32 Points) {
            const int64 Stride = (Points + 1) * 4;
            if (Table < 88 || Table > Entry.Size || Count > (Entry.Size - Table) / Stride) return;
            for (int64 Index = 0; Index < Count; ++Index)
            {
                const int64 Record = Table + Index * Stride;
                FVector3f V[4];
                bool bValid = true;
                for (int32 Point = 0; Point < Points; ++Point)
                {
                    const int64 Offset = Vertices + int64(ReadWord(Record + Point * 4)) * 16;
                    if (Offset > Entry.Size - 12) { bValid = false; break; }
                    FMemory::Memcpy(&V[Point], Data + Offset, 12);
                }
                if (!bValid) continue;
                const uint32 Flags = ReadWord(Record + Points * 4);
                TestTriangle(V[0], V[1], V[2], Flags);
                if (Points == 4) TestTriangle(V[0], V[2], V[3], Flags);
            }
        };
        TestTable(ReadWord(40), ReadWord(76), 3);
        TestTable(ReadWord(48), ReadWord(84), 4);
    }
}
}

AKyaLevelPreview::AKyaLevelPreview()
{
    PrimaryActorTick.bCanEverTick = true;
    Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FlarePostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("BackgroundFlares"));
    FlarePostProcess->SetupAttachment(RootComponent);
    FlarePostProcess->bUnbound = true;
}

#if WITH_EDITOR
void AKyaLevelPreview::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    // Editor edits (bank, sectors, colour space, actor meshes, markers) must rebuild
    // the procedural mesh; otherwise property changes are silently ignored.
    if (bIsRebuildingPreview) return;
    TGuardValue<bool> Guard(bIsRebuildingPreview, true);
    OnConstruction(GetActorTransform());
}
#endif

void AKyaLevelPreview::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyLevelSet();
#if WITH_EDITOR
    DestroyStrayGeneratedActors();
#endif
    if (Bank && !Bank->HasCurrentImportSchema())
        UE_LOG(LogTemp, Warning, TEXT("KYA_PREVIEW_STALE_SCHEMA bank=%s version=%d current=%d; reimport the bank to refresh typed data"),
            *Bank->GetName(), Bank->ImportSchemaVersion, UKyaLevelBank::CurrentImportSchemaVersion);
    // The mesh is cleared only at publication (after cancellation checks), so a
    // cancelled rebuild leaves the previously displayed geometry intact. Presentation
    // state (ranges, flares, textures, materials, hierarchy metadata) is staged and
    // restored on cancellation as well.
    struct FPresentationSnapshot
    {
        TArray<FBackgroundRange> BackgroundRanges;
        TMap<int32, TArray<FVector>> BackgroundBasePositions;
        bool bHasBackgroundCamera = false;
        TArray<TObjectPtr<UTexture2D>> PreviewTextures;
        TArray<FBackgroundFlare> BackgroundFlares;
        TArray<FClusterVisibilityRange> ClusterVisibilityRanges;
        TMap<int32, TArray<uint32>> ClusterBaseIndices;
        TMap<FString, FHierarchySelectionInfo> HierarchySelectionInfo;
        TArray<TObjectPtr<UMaterialInstanceDynamic>> FlareMaterials;
        TArray<FWeightedBlendable> FlareBlendables;
    } Snapshot;
    Snapshot.BackgroundRanges = BackgroundRanges;
    Snapshot.BackgroundBasePositions = BackgroundBasePositions;
    Snapshot.bHasBackgroundCamera = bHasBackgroundCamera;
    Snapshot.PreviewTextures = PreviewTextures;
    Snapshot.BackgroundFlares = BackgroundFlares;
    Snapshot.ClusterVisibilityRanges = ClusterVisibilityRanges;
    Snapshot.ClusterBaseIndices = ClusterBaseIndices;
    Snapshot.HierarchySelectionInfo = HierarchySelectionInfo;
    Snapshot.FlareMaterials = FlareMaterials;
    Snapshot.FlareBlendables = FlarePostProcess->Settings.WeightedBlendables.Array;
    const auto RestorePresentation = [this, &Snapshot]()
    {
        BackgroundRanges = MoveTemp(Snapshot.BackgroundRanges);
        BackgroundBasePositions = MoveTemp(Snapshot.BackgroundBasePositions);
        bHasBackgroundCamera = Snapshot.bHasBackgroundCamera;
        PreviewTextures = MoveTemp(Snapshot.PreviewTextures);
        BackgroundFlares = MoveTemp(Snapshot.BackgroundFlares);
        ClusterVisibilityRanges = MoveTemp(Snapshot.ClusterVisibilityRanges);
        ClusterBaseIndices = MoveTemp(Snapshot.ClusterBaseIndices);
        HierarchySelectionInfo = MoveTemp(Snapshot.HierarchySelectionInfo);
        FlareMaterials = MoveTemp(Snapshot.FlareMaterials);
        FlarePostProcess->Settings.WeightedBlendables.Array = MoveTemp(Snapshot.FlareBlendables);
    };
    RebuildStatus = TEXT("Building");
    struct FCancelReset
    {
        bool& Value;
        ~FCancelReset() { Value = false; }
    } CancelReset{bCancelRebuildRequested};
    TUniquePtr<FScopedSlowTask> SlowTask;
    if (bShowRebuildProgress && !FApp::IsUnattended() && GetWorld() && !GetWorld()->IsGameWorld())
    {
        // Size the work to the number of phase checks below (sectors + actor batches +
        // publication) so the progress dialog never overflows its range.
        const int32 ActorBatches = Bank ? FMath::DivideAndRoundUp(Bank->SceneEnvironment.Actors.Num(), 32) : 0;
        const float WorkUnits = float(FMath::Max(2, SectorBanks.Num() + 2 + ActorBatches + 1));
        SlowTask = MakeUnique<FScopedSlowTask>(WorkUnits,
            NSLOCTEXT("KyaLevelEditor", "KyaRebuild", "Decoding Kya level"));
        SlowTask->MakeDialog(true);
    }
    const auto CheckRebuildCancel = [this, &SlowTask]()
    {
        if (bCancelRebuildRequested) return true;
        if (!SlowTask) return false;
        SlowTask->EnterProgressFrame();
        if (SlowTask->ShouldCancel()) bCancelRebuildRequested = true;
        return bCancelRebuildRequested;
    };
    BackgroundRanges.Reset();
    BackgroundBasePositions.Reset();
    bHasBackgroundCamera = false;
    ClusterVisibilityRanges.Reset();
    ClusterBaseIndices.Reset();
    HierarchySelectionInfo.Reset();
    // Actor captures are cached by source revision: a rebuild that does not change an
    // actor's render inputs reuses its decoded sections instead of re-decoding.
    const bool bCachingActorGeometry = ShouldActorHandlesRenderGeometry();
    const bool bBankChanged = ActorSectionBank.Get() != Bank;
    if (!bCachingActorGeometry || bBankChanged || !Bank || ActorSectionGeneration != Bank->EditGeneration)
    {
        ActorMeshSections.Reset();
        ActorSectionRevisions.Reset();
        ActorCapturedPositions.Reset();
        ActorSectionBank = Bank;
        ActorSectionGeneration = Bank ? Bank->EditGeneration : -1;
    }
    bHasGroundLightData = false;
    for (const TObjectPtr<UKyaLevelBank>& Sector : SectorBanks)
    {
        if (!Sector) continue;
        for (const FKyaBankEntry& Entry : Sector->Entries)
        {
            if (Entry.Type == 7 && Entry.Subtype == 1 && Entry.Size >= 88)
            {
                bHasGroundLightData = true;
                break;
            }
        }
        if (bHasGroundLightData) break;
    }
    StartAsyncActorDecode();
    TSharedPtr<FKyaActorDecodeCache> DecodeCacheSnapshot;
    {
        FScopeLock Lock(&GActorDecodeLock);
        if (const TSharedPtr<FKyaActorDecodeCache>* Found = GActorDecodeCache.Find(Bank))
        {
            if (Found->IsValid() && (*Found)->Generation == Bank->EditGeneration &&
                (*Found)->Bank == Bank)
                DecodeCacheSnapshot = *Found;
            else
                StaleDecodeDiscards.Increment();
        }
    }
    ActorLightingRevision = HashActorLighting(Bank, SectorBanks, bApplyActorLighting);
    BindBankNotifications();
    // Scene data can be incomplete (missing or invalid background entry) while actor
    // placements are still editable; keep the actor representation route alive.
    const auto FinishBuildWithoutGeometry = [this]()
    {
        Mesh->ClearAllMeshSections();
        PreviewTextures.Reset();
        BackgroundFlares.Reset();
        FlareMaterials.Reset();
        FlarePostProcess->Settings.WeightedBlendables.Array.Reset();
        ++BuildCount;
        RebuildStatus = TEXT("Completed");
#if WITH_EDITOR
        RefreshActorHandles();
        RefreshSpatialHandles();
        RefreshHierarchyHandles();
        ApplyGeneratedActorPresentation();
        RefreshSceneVisualizers();
#endif
    };
#if WITH_EDITOR
    const int32 DesiredMarkers = Bank && bShowWaypoints && !bSpawnWaypointHandles
        ? Bank->SceneEnvironment.Waypoints.Num() : 0;
    if (WaypointMarkerBank != Bank || WaypointMarkers.Num() != DesiredMarkers)
    {
        for (UKyaWaypointComponent* Marker : WaypointMarkers)
            if (Marker) Marker->DestroyComponent();
        WaypointMarkers.Reset();
        WaypointMarkerBank = Bank;
        for (int32 Index = 0; Index < DesiredMarkers; ++Index)
        {
            UKyaWaypointComponent* Marker = NewObject<UKyaWaypointComponent>(this, NAME_None, RF_Transient | RF_Transactional);
            Marker->WaypointIndex = Index;
            Marker->ArrowColor = FColor(255, 180, 40);
            Marker->ArrowSize = 0.5f;
            Marker->SetupAttachment(RootComponent);
            Marker->RegisterComponent();
            WaypointMarkers.Add(Marker);
        }
    }
    for (int32 Index = 0; Index < DesiredMarkers; ++Index)
    {
        const FVector P = Bank->SceneEnvironment.Waypoints[Index].Position;
        WaypointMarkers[Index]->SetRelativeLocation(FVector(P.X, P.Z, P.Y) * 100.0);
    }
    // Path guides: all-path mode shows every path, otherwise a single guide follows
    // PreviewPathIndex for backwards compatibility. Both use linear point types, which
    // matches the game's point-to-point interpolation; timing/lead-in/loop behaviour is
    // not visualized.
    TSet<int32> DesiredGuides;
    if (Bank)
    {
        if (bShowAllPaths)
        {
            for (int32 PathIndex = 0; PathIndex < Bank->SceneEnvironment.Paths.Num(); ++PathIndex)
                if (!Bank->SceneEnvironment.Paths[PathIndex].Points.IsEmpty()) DesiredGuides.Add(PathIndex);
        }
        else if (Bank->SceneEnvironment.Paths.IsValidIndex(PreviewPathIndex))
        {
            DesiredGuides.Add(PreviewPathIndex);
        }
    }
    for (auto It = PathGuides.CreateIterator(); It; ++It)
    {
        if (DesiredGuides.Contains(It.Key())) continue;
        if (IsValid(It.Value())) It.Value()->DestroyComponent();
        It.RemoveCurrent();
    }
    for (const int32 PathIndex : DesiredGuides)
    {
        USplineComponent* Guide = nullptr;
        if (TObjectPtr<USplineComponent>* Existing = PathGuides.Find(PathIndex))
        {
            if (IsValid(*Existing)) Guide = *Existing;
        }
        if (!Guide)
        {
            Guide = NewObject<USplineComponent>(this, NAME_None, RF_Transient);
            Guide->SetupAttachment(RootComponent);
            Guide->SetDrawDebug(true);
            Guide->bSelectable = false;
            Guide->RegisterComponent();
            PathGuides.Add(PathIndex, Guide);
        }
        const FKyaPath& Path = Bank->SceneEnvironment.Paths[PathIndex];
        Guide->ClearSplinePoints(false);
        for (const FVector4& P : Path.Points)
            Guide->AddSplinePoint(FVector(P.X, P.Z, P.Y) * 100.0, ESplineCoordinateSpace::Local, false);
        for (int32 Point = 0; Point < Path.Points.Num(); ++Point)
            Guide->SetSplinePointType(Point, ESplinePointType::Linear, false);
        Guide->UpdateSpline();
        Guide->SetVisibility(true);
    }
    // Legacy arrow markers for the single PreviewPathIndex remain diagnostic; all-path
    // mode uses the selectable AKyaSpatialRecordHandle points instead.
    const int32 DesiredPathMarkers = !bShowAllPaths && Bank &&
        Bank->SceneEnvironment.Paths.IsValidIndex(PreviewPathIndex)
        ? Bank->SceneEnvironment.Paths[PreviewPathIndex].Points.Num() : 0;
    if (PathMarkerBank != Bank || PathMarkerIndex != PreviewPathIndex || PathMarkers.Num() != DesiredPathMarkers)
    {
        for (UKyaPathPointComponent* Marker : PathMarkers) if (Marker) Marker->DestroyComponent();
        PathMarkers.Reset();
        PathMarkerBank = Bank;
        PathMarkerIndex = PreviewPathIndex;
        for (int32 Index = 0; Index < DesiredPathMarkers; ++Index)
        {
            UKyaPathPointComponent* Marker = NewObject<UKyaPathPointComponent>(this, NAME_None, RF_Transient | RF_Transactional);
            Marker->PathIndex = PreviewPathIndex;
            Marker->PointIndex = Index;
            Marker->ArrowColor = FColor(60, 210, 255);
            Marker->ArrowSize = 0.35f;
            Marker->SetupAttachment(RootComponent);
            Marker->RegisterComponent();
            PathMarkers.Add(Marker);
        }
    }
    for (int32 Index = 0; Index < DesiredPathMarkers; ++Index)
    {
        const FVector4 P = Bank->SceneEnvironment.Paths[PreviewPathIndex].Points[Index];
        PathMarkers[Index]->SetRelativeLocation(FVector(P.X, P.Z, P.Y) * 100.0);
    }
    const bool bShowActor = Bank && Bank->SceneEnvironment.Actors.IsValidIndex(PreviewActorIndex);
    if (ActorMarker && (!bShowActor || ActorMarkerBank != Bank || ActorMarker->ActorArrayIndex != PreviewActorIndex))
    {
        ActorMarker->DestroyComponent();
        ActorMarker = nullptr;
    }
    if (bShowActor && !ActorMarker)
    {
        ActorMarker = NewObject<UKyaActorMarkerComponent>(this, NAME_None, RF_Transient | RF_Transactional);
        ActorMarker->ActorArrayIndex = PreviewActorIndex;
        ActorMarker->ArrowColor = FColor(245, 100, 220);
        ActorMarker->ArrowSize = 0.7f;
        ActorMarker->SetupAttachment(RootComponent);
        ActorMarker->RegisterComponent();
        ActorMarkerBank = Bank;
    }
    if (ActorMarker)
    {
        const FVector P = Bank->SceneEnvironment.Actors[PreviewActorIndex].Position;
        ActorMarker->SetRelativeLocation(FVector(P.X, P.Z, P.Y) * 100.0);
    }
#endif
    if (!Bank || !Bank->Entries.IsValidIndex(Bank->BackgroundMeshEntry))
    {
        FinishBuildWithoutGeometry();
        return;
    }
    TMap<int32, FKyaSectorRenderSettings> SectorSettings;
    for (const FKyaBankEntry& SceneEntry : Bank->Entries)
        if (SceneEntry.Type == 6 && SceneEntry.Subtype == 1 && SceneEntry.Offset >= 0 && SceneEntry.Size >= 0 &&
            SceneEntry.Size <= MAX_int32 && SceneEntry.Offset <= Bank->BankBytes.Num() && SceneEntry.Size <= Bank->BankBytes.Num() - SceneEntry.Offset)
        {
            KyaScene::ReadSectorRenderSettings(MakeArrayView(Bank->BankBytes.GetData() + SceneEntry.Offset, int32(SceneEntry.Size)), SectorSettings);
            break;
        }
    ActiveClipDistance = Bank->SceneEnvironment.ClipDistance > 0 ? Bank->SceneEnvironment.ClipDistance : 500;
    int32 BaseSector = PreviewBaseSector;
    if (BaseSector < 0)
        for (const auto& Sector : SectorBanks) if (Sector && Sector != Bank) { BaseSector = ResolveSectorId(*Sector); break; }
    if (const auto* Settings = SectorSettings.Find(BaseSector)) ActiveClipDistance = Settings->ClipDistance;
    if (bLogSectorDiagnostics)
        UE_LOG(LogTemp, Display, TEXT("KYA_NATIVE_CLIP sector=%d distance=%.3f"), BaseSector, ActiveClipDistance);
#if WITH_EDITOR
    if (bGeneratedRawColorModulation != bRawColorModulation)
    {
        GeneratedPreviewMaterial = nullptr;
        GeneratedMaskedPreviewMaterial = nullptr;
        GeneratedAdditivePreviewMaterial = nullptr;
        GeneratedTranslucentPreviewMaterial = nullptr;
        GeneratedUntexturedMaterial = nullptr;
        bGeneratedRawColorModulation = bRawColorModulation;
    }
    // The game multiplies 8-bit texture and vertex values in their encoded colour space. Decode
    // the product so Unreal's linear output transform re-encodes it to the original value.
    auto AddColorOutput = [this](UMaterial* Material, UMaterialExpression* EncodedColor)
    {
        if (bRawColorModulation)
        {
            UMaterialExpressionCustom* Convert = NewObject<UMaterialExpressionCustom>(Material);
            Convert->OutputType = CMOT_Float3;
            Convert->Code = TEXT("float3 c = saturate(encoded.rgb); return lerp(c / 12.92, pow((c + 0.055) / 1.055, 2.4), step(0.04045, c));");
            FCustomInput Input;
            Input.InputName = TEXT("encoded");
            Input.Input.Connect(0, EncodedColor);
            Convert->Inputs.Add(Input);
            Material->GetExpressionCollection().AddExpression(Convert);
            Material->GetEditorOnlyData()->EmissiveColor.Connect(0, Convert);
        }
        else Material->GetEditorOnlyData()->EmissiveColor.Connect(0, EncodedColor);
    };
    if (!GeneratedUntexturedMaterial)
    {
        GeneratedUntexturedMaterial = NewObject<UMaterial>(this, NAME_None, RF_Transient);
        GeneratedUntexturedMaterial->SetShadingModel(MSM_Unlit);
        GeneratedUntexturedMaterial->TwoSided = true;
        UMaterialExpressionVertexColor* VertexColor = NewObject<UMaterialExpressionVertexColor>(GeneratedUntexturedMaterial);
        UMaterialExpressionConstant* Scale = NewObject<UMaterialExpressionConstant>(GeneratedUntexturedMaterial);
        Scale->R = 255.0f / 128.0f;
        UMaterialExpressionMultiply* Normalize = NewObject<UMaterialExpressionMultiply>(GeneratedUntexturedMaterial);
        GeneratedUntexturedMaterial->GetExpressionCollection().AddExpression(VertexColor);
        GeneratedUntexturedMaterial->GetExpressionCollection().AddExpression(Scale);
        GeneratedUntexturedMaterial->GetExpressionCollection().AddExpression(Normalize);
        Normalize->A.Connect(0, VertexColor);
        Normalize->B.Connect(0, Scale);
        AddColorOutput(GeneratedUntexturedMaterial, Normalize);
        GeneratedUntexturedMaterial->PostEditChange();
    }
    auto CreateTexturedMaterial = [this, &AddColorOutput](bool Masked, bool Additive = false, bool Translucent = false) -> UMaterial*
    {
        UMaterial* Material = NewObject<UMaterial>(this, NAME_None, RF_Transient);
        Material->SetShadingModel(MSM_Unlit);
        Material->TwoSided = true;
        if (Masked) { Material->BlendMode = BLEND_Masked; Material->OpacityMaskClipValue = 1.5f / 255.0f; }
        if (Additive) Material->BlendMode = BLEND_Additive;
        if (Translucent) Material->BlendMode = BLEND_AlphaComposite;
        UMaterialExpressionTextureSampleParameter2D* Sample = NewObject<UMaterialExpressionTextureSampleParameter2D>(Material);
        Sample->ParameterName = TEXT("KyaTexture");
        Sample->Texture = GEngine->DefaultTexture;
        if (bRawColorModulation)
        {
            UTexture2D* DefaultRawTexture = UTexture2D::CreateTransient(1, 1, PF_B8G8R8A8);
            DefaultRawTexture->SRGB = false;
            DefaultRawTexture->UpdateResource();
            Sample->Texture = DefaultRawTexture;
            Sample->SamplerType = SAMPLERTYPE_LinearColor;
        }
        Material->GetExpressionCollection().AddExpression(Sample);
        UMaterialExpressionVertexColor* VertexColor = NewObject<UMaterialExpressionVertexColor>(Material);
        UMaterialExpressionConstant* Scale = NewObject<UMaterialExpressionConstant>(Material);
        Scale->R = 255.0f / 128.0f;
        UMaterialExpressionMultiply* Modulate = NewObject<UMaterialExpressionMultiply>(Material);
        UMaterialExpressionMultiply* Normalize = NewObject<UMaterialExpressionMultiply>(Material);
        Material->GetExpressionCollection().AddExpression(VertexColor);
        Material->GetExpressionCollection().AddExpression(Scale);
        Material->GetExpressionCollection().AddExpression(Modulate);
        Material->GetExpressionCollection().AddExpression(Normalize);
        Modulate->A.Connect(0, Sample);
        Modulate->B.Connect(0, VertexColor);
        Normalize->A.Connect(0, Modulate);
        Normalize->B.Connect(0, Scale);
        if (Additive || Translucent)
        {
            // GS adds encoded colour values. Emit the linear delta needed to get
            // that result after Unreal's additive blend, rather than adding linear RGB.
            auto* Scene = NewObject<UMaterialExpressionSceneColor>(Material);
            auto* Factor = NewObject<UMaterialExpressionScalarParameter>(Material);
            Factor->ParameterName = TEXT("BlendFactor");
            Factor->DefaultValue = 1.0f;
            auto* SourceAlpha = NewObject<UMaterialExpressionScalarParameter>(Material);
            SourceAlpha->ParameterName = TEXT("UseSourceAlpha");
            SourceAlpha->DefaultValue = 1;
            auto* AlphaClip = NewObject<UMaterialExpressionScalarParameter>(Material);
            AlphaClip->ParameterName = TEXT("AlphaClip");
            AlphaClip->DefaultValue = -1.0f;
            auto* Blend = NewObject<UMaterialExpressionCustom>(Material);
            Blend->OutputType = CMOT_Float3;
            Blend->Code = bRawColorModulation ? TEXT(
                "float3 d = max(scene.rgb, 0);"
                "float3 e = lerp(d * 12.92, 1.055 * pow(d, 1.0 / 2.4) - 0.055, step(0.0031308, d));"
                "float a = lerp(1, saturate(alpha * vertexAlpha * (255.0 / 128.0) * (255.0 / 128.0)), useSourceAlpha);"
                "float3 c = saturate(e + saturate(encoded.rgb) * factor * a);"
                "float3 result = lerp(c / 12.92, pow((c + 0.055) / 1.055, 2.4), step(0.04045, c));"
                "clip(alpha * vertexAlpha * (255.0 / 128.0) - alphaClip);"
                "return max(result - d, 0);") : TEXT(
                "clip(alpha * vertexAlpha * (255.0 / 128.0) - alphaClip);"
                "float a = lerp(1, saturate(alpha * vertexAlpha * (255.0 / 128.0) * (255.0 / 128.0)), useSourceAlpha);"
                "return encoded.rgb * factor * a;");
            if (Translucent) Blend->Code = bRawColorModulation ? TEXT(
                "clip(alpha * vertexAlpha * (255.0 / 128.0) - alphaClip);"
                "float a = saturate(alpha * vertexAlpha * (255.0 / 128.0) * (255.0 / 128.0));"
                "float3 d = max(scene.rgb, 0);"
                "float3 e = lerp(d * 12.92, 1.055 * pow(d, 1.0 / 2.4) - 0.055, step(0.0031308, d));"
                "float3 c = saturate(lerp(e, saturate(encoded.rgb), a));"
                "float3 result = lerp(c / 12.92, pow((c + 0.055) / 1.055, 2.4), step(0.04045, c));"
                "return max(result - d * (1 - a), 0);") : TEXT(
                "clip(alpha * vertexAlpha * (255.0 / 128.0) - alphaClip);"
                "float a = saturate(alpha * vertexAlpha * (255.0 / 128.0) * (255.0 / 128.0));"
                "return encoded.rgb * a;");
            auto Input = [Blend](const TCHAR* Name, UMaterialExpression* Expression, int32 Output = 0) {
                FCustomInput I; I.InputName = Name; I.Input.Connect(Output, Expression); Blend->Inputs.Add(I);
            };
            Input(TEXT("encoded"), Normalize);
            Input(TEXT("scene"), Scene);
            Input(TEXT("factor"), Factor);
            Input(TEXT("useSourceAlpha"), SourceAlpha);
            Input(TEXT("alphaClip"), AlphaClip);
            Input(TEXT("alpha"), Sample, 4);
            Input(TEXT("vertexAlpha"), VertexColor, 4);
            Material->GetExpressionCollection().AddExpression(Scene);
            Material->GetExpressionCollection().AddExpression(Factor);
            Material->GetExpressionCollection().AddExpression(SourceAlpha);
            Material->GetExpressionCollection().AddExpression(AlphaClip);
            Material->GetExpressionCollection().AddExpression(Blend);
            Material->GetEditorOnlyData()->EmissiveColor.Connect(0, Blend);
            if (Translucent)
            {
                // Preserve the destination at zero alpha, including earlier translucent
                // surfaces. A scene-colour replacement with opacity one would erase them.
                auto* Opacity = NewObject<UMaterialExpressionCustom>(Material);
                Opacity->OutputType = CMOT_Float1;
                Opacity->Code = TEXT("return saturate(alpha * vertexAlpha * (255.0 / 128.0) * (255.0 / 128.0));");
                FCustomInput A; A.InputName = TEXT("alpha"); A.Input.Connect(4, Sample); Opacity->Inputs.Add(A);
                FCustomInput V; V.InputName = TEXT("vertexAlpha"); V.Input.Connect(4, VertexColor); Opacity->Inputs.Add(V);
                Material->GetExpressionCollection().AddExpression(Opacity);
                Material->GetEditorOnlyData()->Opacity.Connect(0, Opacity);
            }
            else
            {
                auto* One = NewObject<UMaterialExpressionConstant>(Material);
                One->R = 1;
                Material->GetExpressionCollection().AddExpression(One);
                Material->GetEditorOnlyData()->Opacity.Connect(0, One);
            }
        }
        else AddColorOutput(Material, Normalize);
        if (Masked)
        {
            UMaterialExpressionMultiply* Alpha = NewObject<UMaterialExpressionMultiply>(Material);
            UMaterialExpressionMultiply* AlphaScale = NewObject<UMaterialExpressionMultiply>(Material);
            Material->GetExpressionCollection().AddExpression(Alpha);
            Material->GetExpressionCollection().AddExpression(AlphaScale);
            Alpha->A.Connect(4, Sample);
            Alpha->B.Connect(4, VertexColor);
            AlphaScale->A.Connect(0, Alpha);
            AlphaScale->B.Connect(0, Scale);
            Material->GetEditorOnlyData()->OpacityMask.Connect(0, AlphaScale);
        }
        Material->PostEditChange();
        return Material;
    };
    if (!TextureMaterial && !GeneratedPreviewMaterial) GeneratedPreviewMaterial = CreateTexturedMaterial(false);
    if (!TextureMaterial && !GeneratedMaskedPreviewMaterial) GeneratedMaskedPreviewMaterial = CreateTexturedMaterial(true);
    if (!TextureMaterial && !GeneratedAdditivePreviewMaterial) GeneratedAdditivePreviewMaterial = CreateTexturedMaterial(false, true);
    if (!TextureMaterial && !GeneratedTranslucentPreviewMaterial) GeneratedTranslucentPreviewMaterial = CreateTexturedMaterial(false, false, true);
    if (!GeneratedFlareMaterial)
    {
        GeneratedFlareMaterial = NewObject<UMaterial>(this, NAME_None, RF_Transient);
        UMaterial* Material = GeneratedFlareMaterial;
        Material->MaterialDomain = MD_PostProcess;
        Material->BlendableLocation = BL_SceneColorBeforeBloom;
        // Each flare must get its own pass, rather than blending parameter sets.
        Material->bIsBlendable = false;
        auto* Scene = NewObject<UMaterialExpressionSceneTexture>(Material);
        Scene->SceneTextureId = PPI_PostProcessInput0;
        auto* Depth = NewObject<UMaterialExpressionSceneTexture>(Material);
        Depth->SceneTextureId = PPI_SceneDepth;
        auto* UV = NewObject<UMaterialExpressionScreenPosition>(Material);
        auto* Texture = NewObject<UMaterialExpressionTextureObjectParameter>(Material);
        Texture->ParameterName = TEXT("FlareTexture");
        UTexture2D* DefaultFlare = UTexture2D::CreateTransient(1, 1, PF_B8G8R8A8);
        DefaultFlare->SRGB = false;
        DefaultFlare->UpdateResource();
        Texture->Texture = DefaultFlare;
        Texture->SamplerType = SAMPLERTYPE_LinearColor;
        auto* Rect = NewObject<UMaterialExpressionVectorParameter>(Material);
        Rect->ParameterName = TEXT("FlareRect");
        Rect->DefaultValue = FLinearColor(0, 0, 0, 0);
        auto* Marker = NewObject<UMaterialExpressionVectorParameter>(Material);
        Marker->ParameterName = TEXT("FlareMarker");
        auto* Composite = NewObject<UMaterialExpressionCustom>(Material);
        Composite->OutputType = CMOT_Float3;
        Composite->Code = TEXT(
            "if (rect.z <= 0 || rect.w <= 0) return scene.rgb;"
            "float2 uv = (screen.xy - rect.xy) / (rect.zw * 2) + 0.5;"
            "if (any(uv < 0) || any(uv > 1)) return scene.rgb;"
            "float visibility = 0;"
            "[loop] for (int y = 0; y < 16; ++y) { [loop] for (int x = 0; x < 16; ++x) {"
            "float2 m = marker.xy + ((float2(x,y) + 0.5) / 8 - 1) * rect.zw / 64;"
            "if (all(m >= 0) && all(m <= 1)) {"
            "float z = SceneTextureLookup(ViewportUVToSceneTextureUV(m, 1), 1, false).r;"
            "visibility += z >= marker.z ? 1 : 0; } } }"
            "float3 light = Texture2DSample(flare, flareSampler, uv).rgb * float3(88,80,64) / 128 * visibility / 256;"
            "float3 d = max(scene.rgb * View.OneOverPreExposure, 0);"
            "float3 e = lerp(d * 12.92, 1.055 * pow(d, 1.0 / 2.4) - 0.055, step(0.0031308, d));"
            "float3 c = saturate(e + light);"
            "return lerp(c / 12.92, pow((c + 0.055) / 1.055, 2.4), step(0.04045, c)) * View.PreExposure + depth.rgb * 0;");
        auto Input = [Composite](const TCHAR* Name, UMaterialExpression* Expression, int32 Output = 0) {
            FCustomInput I; I.InputName = Name; I.Input.Connect(Output, Expression); Composite->Inputs.Add(I);
        };
        Input(TEXT("scene"), Scene); Input(TEXT("depth"), Depth); Input(TEXT("screen"), UV);
        Input(TEXT("flare"), Texture); Input(TEXT("rect"), Rect, 5); Input(TEXT("marker"), Marker, 5);
        for (UMaterialExpression* Expression : TArray<UMaterialExpression*>{Scene, Depth, UV, Texture, Rect, Marker, Composite})
            Material->GetExpressionCollection().AddExpression(Expression);
        Material->GetEditorOnlyData()->EmissiveColor.Connect(0, Composite);
        Material->PostEditChange();
    }
#endif
    UMaterialInterface* EffectiveMaterial = TextureMaterial ? TextureMaterial.Get() : GeneratedPreviewMaterial.Get();
    const FKyaBankEntry& Entry = Bank->Entries[Bank->BackgroundMeshEntry];
    if (Entry.Offset < 0 || Entry.Size < 0 || Entry.Offset > Bank->BankBytes.Num() ||
        Entry.Size > Bank->BankBytes.Num() - Entry.Offset || Entry.Size > MAX_int32)
    {
        FinishBuildWithoutGeometry();
        return;
    }
    TArray<FKyaG3DDecodedStrip> Strips;
    if (!KyaG3D::DecodeHierarchyStrips(MakeArrayView(Bank->BankBytes.GetData() + Entry.Offset, int32(Entry.Size)), Strips))
    {
        FinishBuildWithoutGeometry();
        return;
    }
    TArray<FVector> Positions;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FColor> Colors;
    TMap<uint64, int32> MaterialOffsets;
    TConstArrayView<uint8> TextureData;
    if (Bank->Entries.IsValidIndex(Bank->BackgroundTextureEntry))
    {
        const FKyaBankEntry& TextureEntry = Bank->Entries[Bank->BackgroundTextureEntry];
        if (TextureEntry.Offset >= 0 && TextureEntry.Size >= 0 && TextureEntry.Size <= MAX_int32 &&
            TextureEntry.Offset <= Bank->BankBytes.Num() && TextureEntry.Size <= Bank->BankBytes.Num() - TextureEntry.Offset)
        {
            TextureData = MakeArrayView(Bank->BankBytes.GetData() + TextureEntry.Offset, int32(TextureEntry.Size));
            KyaG2D::ReadMaterials(TextureData, MaterialOffsets);
        }
    }
    int32 NextSection = 1;
    FMaterialSections MaterialSections;
    struct FSectionData { int32 Section = 0; UMaterialInterface* Material = nullptr; TArray<FVector> Positions; TArray<int32> Triangles; TArray<FVector2D> UVs; TArray<FColor> Colors; };
    TMap<int32, FSectionData> TexturedSections;
    TMap<int32, TObjectPtr<UMaterialInterface>> BuiltSectionMaterials;
    TArray<TObjectPtr<UTexture2D>> BuiltPreviewTextures;
    auto GetTexturedSection = [&](uint64 Hash, TConstArrayView<uint8> Source,
        const TMap<uint64, int32>& Offsets, FMaterialSections& Sections,
        const TCHAR* SourceLabel, uint32 Primitive) -> FSectionData*
    {
        const int32* Offset = Offsets.Find(Hash);
        if (!EffectiveMaterial || !Offset) return nullptr;
        const FMaterialSectionKey Key{Hash, (Primitive & 0x40) != 0};
        int32* Found = Sections.Find(Key);
        if (!Found)
        {
            int32 Width = 0, Height = 0;
            TArray<FColor> Pixels;
            int32 Section = INDEX_NONE;
            if (KyaG2D::DecodeMaterialTexture(Source, *Offset, Width, Height, Pixels))
            {
                UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
                if (Texture && Texture->GetPlatformData() && !Texture->GetPlatformData()->Mips.IsEmpty())
                {
                    Texture->SRGB = !bRawColorModulation;
                    uint64 Clamp = 0;
                    if (KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x08, Clamp))
                    {
                        Texture->AddressX = (Clamp & 3) == 1 ? TA_Clamp : TA_Wrap;
                        Texture->AddressY = ((Clamp >> 2) & 3) == 1 ? TA_Clamp : TA_Wrap;
                    }
                    void* Dest = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
                    FMemory::Memcpy(Dest, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
                    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
                    Texture->UpdateResource();
                    uint64 TestRegister = 0;
                    const bool Masked = !TextureMaterial && KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x47, TestRegister) &&
                        (TestRegister & 1) && ((TestRegister >> 1) & 7) == 5 && ((TestRegister >> 4) & 255) == 1 && ((TestRegister >> 12) & 3) == 0;
                    uint64 Alpha = 0;
                    const bool Additive = !TextureMaterial && Key.bBlend &&
                        KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x42, Alpha) &&
                        ((Alpha & 0xff) == 0x48 || (Alpha & 0xff) == 0x68); // (Cs - 0) * As/128 or FIX/128 + Cd.
                    const bool Translucent = !TextureMaterial && Key.bBlend &&
                        KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x42, Alpha) && (Alpha & 0xff) == 0x44;
                    UMaterialInterface* Parent = Translucent ? GeneratedTranslucentPreviewMaterial.Get() : Additive ? GeneratedAdditivePreviewMaterial.Get() :
                        (Masked ? GeneratedMaskedPreviewMaterial.Get() : EffectiveMaterial);
                    UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Parent, this);
                    if (Material)
                    {
                        Material->SetTextureParameterValue(TEXT("KyaTexture"), Texture);
                        if (Additive || Translucent)
                        {
                            const bool UsesSourceAlpha = ((Alpha >> 4) & 3) == 0;
                            Material->SetScalarParameterValue(TEXT("UseSourceAlpha"), UsesSourceAlpha ? 1.0f : 0.0f);
                            Material->SetScalarParameterValue(TEXT("BlendFactor"), UsesSourceAlpha ? 1.0f : float((Alpha >> 32) & 255) / 128.0f);
                            Material->SetScalarParameterValue(TEXT("AlphaClip"), Masked ? 1.0f / 255.0f : -1.0f);
                        }
                        Section = NextSection++;
                        BuiltSectionMaterials.Add(Section, Material);
                        BuiltPreviewTextures.Add(Texture);
                        if (bLogSectorDiagnostics)
                            UE_LOG(LogTemp, Display, TEXT("KYA_TEXTURE_SECTION hash=%016llx source=%s width=%d height=%d section=%d masked=%d additive=%d translucent=%d"),
                                Hash, SourceLabel, Width, Height, Section, Masked ? 1 : 0, Additive ? 1 : 0, Translucent ? 1 : 0);
                    }
                }
            }
            if (bLogSectorDiagnostics && Section == INDEX_NONE)
                UE_LOG(LogTemp, Warning, TEXT("KYA_TEXTURE_DECODE_FAILED hash=%016llx source=%s width=%d height=%d"),
                    Hash, SourceLabel, Width, Height);
            Found = &Sections.Add(Key, Section);
        }
        if (*Found == INDEX_NONE) return nullptr;
        FSectionData& Result = TexturedSections.FindOrAdd(*Found);
        Result.Section = *Found;
        if (const TObjectPtr<UMaterialInterface>* Material = BuiltSectionMaterials.Find(*Found))
            Result.Material = Material->Get();
        return &Result;
    };
    auto RegisterClusterRange = [&](const FKyaG3DDecodedStrip& Strip, const FMatrix& Transform,
        int32 Section, int32 Start, int32 Count, bool bLod = true) {
        if (!Count) return;
        auto Sphere = [&Transform](const FVector4f& S) {
            const FVector P = Transform.TransformPosition(FVector(S.X, S.Y, S.Z));
            return FVector4f(P.X * 100, P.Z * 100, P.Y * 100, FMath::Abs(S.W) * Transform.GetMaximumAxisScale() * 100);
        };
        FClusterVisibilityRange Range;
        Range.Section = Section; Range.Start = Start; Range.Count = Count;
        Range.ObjectSphere = Sphere(Strip.ObjectSphere);
        Range.StripSphere = Sphere(Strip.StripSphere);
        Range.LodSphere = Sphere(Strip.FirstLodSphere);
        if (bLod)
        {
            Range.MinLodBiasSquared = FMath::Square(double(Strip.LodMinSizeBias));
            Range.MaxLodBiasSquared = FMath::Square(double(Strip.LodMaxSizeBias));
        }
        ClusterVisibilityRanges.Add(Range);
    };
    // Sector::Init hides the common background whenever a loaded sector has
    // its own background mesh. Keep the common mesh when no sector supplies one.
    const bool bHasSectorBackground = SectorBanks.ContainsByPredicate([this](const TObjectPtr<UKyaLevelBank>& Sector) {
        return Sector && Sector != Bank && Sector->Entries.ContainsByPredicate([&](const FKyaBankEntry& Candidate) {
            return Candidate.Type == 4 && Candidate.Subtype == 2 && Candidate.Offset >= 0 && Candidate.Size > 0 &&
                Candidate.Offset <= Sector->BankBytes.Num() && Candidate.Size <= Sector->BankBytes.Num() - Candidate.Offset;
        });
    });
    if (bLogSectorDiagnostics)
        UE_LOG(LogTemp, Display, TEXT("KYA_COMMON_BACKGROUND hidden=%d strips=%d"), bHasSectorBackground, Strips.Num());
    auto PrepareBackgroundTransform = [&](TConstArrayView<uint8> Data, int32 Hierarchy,
        TConstArrayView<int32> CameraRelative, int32 Section, int32 Start, int32 Count, FMatrix& Transform) {
        if (CameraRelative.IsEmpty()) return KyaG3D::ReadHierarchyWorldMatrix(Data, Hierarchy, Transform);
        FMatrix X, Y, Z;
        if (!KyaG3D::ReadBackgroundWorldMatrix(Data, Hierarchy, CameraRelative, FVector::ZeroVector, Transform) ||
            !KyaG3D::ReadBackgroundWorldMatrix(Data, Hierarchy, CameraRelative, FVector(1, 0, 0), X) ||
            !KyaG3D::ReadBackgroundWorldMatrix(Data, Hierarchy, CameraRelative, FVector(0, 1, 0), Y) ||
            !KyaG3D::ReadBackgroundWorldMatrix(Data, Hierarchy, CameraRelative, FVector(0, 0, 1), Z)) return false;
        auto ToUnreal = [](const FVector& P) { return FVector(P.X, P.Z, P.Y) * 100.0; };
        const FVector Origin = Transform.GetOrigin();
        FBackgroundRange Range;
        Range.Section = Section; Range.Start = Start; Range.Count = Count;
        Range.CameraX = ToUnreal(X.GetOrigin() - Origin);
        Range.CameraY = ToUnreal(Y.GetOrigin() - Origin);
        Range.CameraZ = ToUnreal(Z.GetOrigin() - Origin);
        if (!Range.CameraX.IsNearlyZero() || !Range.CameraY.IsNearlyZero() || !Range.CameraZ.IsNearlyZero())
            BackgroundRanges.Add(Range);
        return true;
    };
    TArray<TObjectPtr<UMaterialInstanceDynamic>> BuiltFlareMaterials;
    TArray<FBackgroundFlare> BuiltBackgroundFlares;
    auto AddBackgroundFlares = [&](TConstArrayView<uint8> Data, TConstArrayView<uint8> Source,
        const TMap<uint64, int32>& Materials) {
        if (!bShowBackgroundMeshes || !bShowSprites || !GeneratedFlareMaterial) return;
        TArray<FKyaG3DBackgroundFlare> Flares;
        if (!KyaG3D::ReadBackgroundFlares(Data, Flares))
        {
            UE_LOG(LogTemp, Warning, TEXT("KYA_BACKGROUND_FLARES invalid hierarchy sprite data"));
            return;
        }
        for (const FKyaG3DBackgroundFlare& Flare : Flares)
        {
            const int32* MaterialOffset = Materials.Find(Flare.MaterialHash);
            int32 Width = 0, Height = 0;
            TArray<FColor> Pixels;
            FMatrix Transform;
            if (!MaterialOffset || !KyaG2D::DecodeMaterialTexture(Source, *MaterialOffset, Width, Height, Pixels) ||
                !KyaG3D::ReadHierarchyWorldMatrix(Data, Flare.HierarchyOffset, Transform)) continue;
            UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
            Texture->SRGB = false;
            Texture->AddressX = Texture->AddressY = TA_Clamp;
            void* Dest = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
            FMemory::Memcpy(Dest, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
            Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
            Texture->UpdateResource();
            auto* Material = UMaterialInstanceDynamic::Create(GeneratedFlareMaterial, this);
            Material->SetTextureParameterValue(TEXT("FlareTexture"), Texture);
            const FVector P = Transform.TransformPosition(FVector(Flare.Center));
            FBackgroundFlare Item;
            Item.Position = FVector(P.X, P.Z, P.Y) * 100.0;
            Item.MaterialIndex = BuiltFlareMaterials.Add(Material);
            BuiltBackgroundFlares.Add(Item);
            BuiltPreviewTextures.Add(Texture);
            if (bLogSectorDiagnostics)
                UE_LOG(LogTemp, Display, TEXT("KYA_BACKGROUND_FLARE hash=%016llx center=%s texture=%dx%d"),
                    Flare.MaterialHash, *Item.Position.ToString(), Width, Height);
        }
    };
    const auto CommonBackgroundData = MakeArrayView(Bank->BankBytes.GetData() + Entry.Offset, int32(Entry.Size));
    if (!bHasSectorBackground) AddBackgroundFlares(CommonBackgroundData, TextureData, MaterialOffsets);
    TArray<int32> CommonCameraRelative;
    if ((Bank->SceneEnvironment.BackgroundFlags & 1) && !bHasSectorBackground)
        KyaG3D::ReadCameraRelativeBackgroundHierarchies(CommonBackgroundData, CommonCameraRelative);
    if (bShowBackgroundMeshes && !bHasSectorBackground) for (const FKyaG3DDecodedStrip& Strip : Strips)
    {
        FSectionData* Textured = GetTexturedSection(Strip.MaterialHash, TextureData, MaterialOffsets, MaterialSections, TEXT("Background"), Strip.Primitive);
        TArray<FVector>& TargetPositions = Textured ? Textured->Positions : Positions;
        TArray<int32>& TargetTriangles = Textured ? Textured->Triangles : Triangles;
        TArray<FVector2D>& TargetUVs = Textured ? Textured->UVs : UVs;
        TArray<FColor>& TargetColors = Textured ? Textured->Colors : Colors;
        const int32 Base = TargetPositions.Num();
        FMatrix HierarchyTransform;
        if (!PrepareBackgroundTransform(CommonBackgroundData, Strip.HierarchyOffset, CommonCameraRelative,
            Textured ? Textured->Section : 0, Base, Strip.Vertices.Num(), HierarchyTransform)) continue;
        for (const FKyaG3DVertex& Vertex : Strip.Vertices)
        {
            const FVector P = HierarchyTransform.TransformPosition(FVector(Vertex.Position));
            TargetPositions.Add(FVector(P.X, P.Z, P.Y) * 100.0);
        }
        for (const FKyaG3DVertexAttributes& Attribute : Strip.Attributes)
        {
            TargetUVs.Add(FVector2D(Attribute.UV));
            TargetColors.Add(Attribute.Color);
        }
        AppendUnrealTriangles(TargetTriangles, Strip.Indices, Base);
    }
    // Actor geometry is built when a diagnostic mesh mode is active or when the
    // per-actor handles own the actor meshes (the normal editing route).
    const bool bActorHandlesOwnGeometry = ShouldActorHandlesRenderGeometry();
    if (bShowAllActorMeshes || bActorHandlesOwnGeometry ||
        (bShowActorMesh && Bank->SceneEnvironment.Actors.IsValidIndex(PreviewActorIndex)))
    {
        auto FindEntry = [&](int32 Type, int32 Ordinal) -> const FKyaBankEntry* {
            if (Ordinal < 0) return nullptr;
            for (const FKyaBankEntry& Candidate : Bank->Entries)
                if (Candidate.Type == Type && Candidate.Subtype == 1 && Ordinal-- == 0) return &Candidate;
            return nullptr;
        };
        auto EntryData = [&](const FKyaBankEntry* Candidate) -> TConstArrayView<uint8> {
            if (!Candidate || Candidate->Offset < 0 || Candidate->Size < 0 || Candidate->Size > MAX_int32 ||
                Candidate->Offset > Bank->BankBytes.Num() || Candidate->Size > Bank->BankBytes.Num() - Candidate->Offset) return {};
            return MakeArrayView(Bank->BankBytes.GetData() + Candidate->Offset, int32(Candidate->Size));
        };
        // Native links the shared actor-material bank after the model's own bank and
        // replaces any material whose hash also exists there (ed3D.cpp:16152). The
        // shared bank therefore wins over the actor's 8x8/64x64 colour atlas; actor
        // model patches are applied last, as in SV_SetModel (Actor.cpp:2771).
        const auto SharedTextures = EntryData(FindEntry(5, Bank->SceneEnvironment.AlternateTextureIndex));
        TMap<uint64, int32> SharedMaterials;
        FMaterialSections SharedSections;
        KyaG2D::ReadMaterials(SharedTextures, SharedMaterials);
        TMap<int32, TMap<uint64, int32>> MaterialsByTexture;
        TMap<int32, FMaterialSections> SectionsByTexture;
        TMap<int32, TArray<FKyaG3DDecodedStrip>> StripsByMesh;
        // Shared ambient and diffuse lights are selected per actor sector below.
        TArray<const FKyaLight*> DirectionalLights;
        TMap<int32, FLinearColor> SectorAmbient;
        FLinearColor ActorAmbient = FLinearColor::Black;
        if (bApplyActorLighting)
            for (const FKyaLight& Light : Bank->Lights)
            {
                if (Light.LightType == 2 || Light.ShapeType == 4) continue;
                const bool bHasDiffuse = FMath::Max3(FMath::Abs(Light.Color.R), FMath::Abs(Light.Color.G),
                    FMath::Abs(Light.Color.B)) >= 1.e-6f;
                if ((Light.ShapeType == 1 || Light.ShapeType == 2) && bHasDiffuse)
                    DirectionalLights.Add(&Light);
                // Only ambient-only lights add to the shared ambient in Level_Manage.
                if (!bHasDiffuse)
                {
                    const int32 LightSector = Light.TrailingValues.IsEmpty() ? -1 : Light.TrailingValues[0];
                    FLinearColor& Ambient = LightSector == -1 ? ActorAmbient
                        : SectorAmbient.FindOrAdd(LightSector, FLinearColor::Transparent);
                    Ambient += FLinearColor(Light.AmbientColor.R, Light.AmbientColor.G, Light.AmbientColor.B, 0.0f);
                }
                if (bLogSectorDiagnostics)
                    UE_LOG(LogTemp, Display,
                        TEXT("KYA_PREVIEW_LIGHT type=%d shape=%d ambient=(%.3f,%.3f,%.3f,%.3f) color=(%.3f,%.3f,%.3f,%.3f) dir=(%.3f,%.3f,%.3f) multiplier=%.3f"),
                        Light.LightType, Light.ShapeType,
                        Light.AmbientColor.R, Light.AmbientColor.G, Light.AmbientColor.B, Light.AmbientColor.A,
                        Light.Color.R, Light.Color.G, Light.Color.B, Light.Color.A,
                        Light.Direction.X, Light.Direction.Y, Light.Direction.Z, Light.Multiplier);
            }
        if (bLogSectorDiagnostics)
            UE_LOG(LogTemp, Display, TEXT("KYA_PREVIEW_LIGHT_TOTAL ambient=(%.3f,%.3f,%.3f) directional=%d"),
                ActorAmbient.R, ActorAmbient.G, ActorAmbient.B, DirectionalLights.Num());
        auto RelightStrip = [this, &DirectionalLights, &ActorAmbient, &SectorAmbient](const FKyaG3DDecodedStrip& Strip,
            const TArray<FVector3f>& WorldNormals, int32 SectorId, float Gain) -> TArray<FColor>
        {
            TArray<FColor> Colours;
            Colours.Reserve(Strip.Vertices.Num());
            // `pHier->pHierarchySetup->pLightData = &this->lightConfig` runs regardless of
            // the actor's lighting flags (LightManager.cpp:371), so actors that skip their
            // own ComputeLighting still render with the shared global config. Apply the
            // shared sun/ambient config to every actor model.
            if (!bApplyActorLighting || WorldNormals.Num() != Strip.Vertices.Num())
            {
                for (const FKyaG3DVertexAttributes& Attribute : Strip.Attributes) Colours.Add(Attribute.Color);
                return Colours;
            }
            // ed3DPKTCopyMatrixPacket sets ambient W to 1/128. LightColor below
            // uses 0..1 units, so convert the packet's scale for normalized RGB.
            constexpr float AmbientScale = 255.0f / 128.0f;
            FLinearColor Ambient = ActorAmbient;
            if (const FLinearColor* LocalAmbient = SectorAmbient.Find(SectorId)) Ambient += *LocalAmbient;
            Ambient *= Gain;
            struct FDiffuse
            {
                FVector3f Direction = FVector3f::ZeroVector;
                FLinearColor Color = FLinearColor::Transparent;
                float CollisionCost = 0.0f;
            };
            FDiffuse Diffuse[4];
            TArray<const FKyaLight*, TInlineAllocator<4>> AutomaticLights;
            const auto AddLight = [&Diffuse](int32 Slot, const FKyaLight& Light) {
                Diffuse[Slot].Direction -= FVector3f(Light.Direction);
                Diffuse[Slot].Color += Light.Color;
                Diffuse[Slot].CollisionCost += 1.0f;
            };
            for (const FKyaLight* Light : DirectionalLights)
            {
                if (Light->TrailingValues.IsValidIndex(0) && Light->TrailingValues[0] != -1 &&
                    Light->TrailingValues[0] != SectorId) continue;
                const int32 Slot = Light->TrailingValues.IsValidIndex(1) ? Light->TrailingValues[1] : -1;
                if (Slot >= 0 && Slot < UE_ARRAY_COUNT(Diffuse)) AddLight(Slot, *Light);
                else if (Slot == -1) AutomaticLights.Add(Light);
            }
            // Native assigns unbound suns to the slot with the lowest summed
            // angular collision cost, including lights with explicit slots.
            for (const FKyaLight* Light : AutomaticLights)
            {
                int32 Slot = 0;
                const FVector3f Direction(-Light->Direction);
                float LowestCost = TNumericLimits<float>::Max();
                for (int32 Index = 0; Index < UE_ARRAY_COUNT(Diffuse); ++Index)
                {
                    const FDiffuse& Candidate = Diffuse[Index];
                    const float Cost = (Candidate.CollisionCost -
                        FVector3f::DotProduct(Candidate.Direction, Direction)) * 0.5f;
                    if (Cost < LowestCost) { LowestCost = Cost; Slot = Index; }
                }
                AddLight(Slot, *Light);
            }
            for (FDiffuse& Light : Diffuse)
            {
                Light.Direction = Light.Direction.GetSafeNormal();
                Light.Color *= Gain;
            }
            for (int32 Index = 0; Index < Strip.Vertices.Num(); ++Index)
            {
                FLinearColor LightColor = Ambient;
                for (const FDiffuse& Light : Diffuse)
                {
                    const float Dot = FMath::Max(0.0f, FVector3f::DotProduct(WorldNormals[Index], Light.Direction));
                    LightColor += Light.Color * Dot;
                }
                // Native caps the accumulated light colour at 255 before multiplying
                // by the authored vertex colour (native.vert.glsl: min(lightColor, 255)).
                LightColor.R = FMath::Min(LightColor.R, 1.0f);
                LightColor.G = FMath::Min(LightColor.G, 1.0f);
                LightColor.B = FMath::Min(LightColor.B, 1.0f);
                const FColor& Original = Strip.Attributes[Index].Color;
                Colours.Add(FColor(
                    uint8(FMath::Clamp(Original.R / 255.0f * AmbientScale * LightColor.R, 0.0f, 1.0f) * 255.0f),
                    uint8(FMath::Clamp(Original.G / 255.0f * AmbientScale * LightColor.G, 0.0f, 1.0f) * 255.0f),
                    uint8(FMath::Clamp(Original.B / 255.0f * AmbientScale * LightColor.B, 0.0f, 1.0f) * 255.0f),
                    Original.A));
            }
            return Colours;
        };
        // Actors with TextureIndex == -1 do not name a texture bank, but every one of
        // their material hashes exists somewhere in the bank. Index every texture bank
        // by material hash so those actors still receive their real textures instead of
        // the alternate bank or an untextured fallback.
        TMap<uint64, TArray<int32>> TextureBankByHash;
        TMap<int32, TConstArrayView<uint8>> TextureBankData;
        TMap<int32, TMap<uint64, int32>> TextureBankMaterials;
        TMap<int32, FMaterialSections> TextureBankSections;
        for (int32 EntryIndex = 0; EntryIndex < Bank->Entries.Num(); ++EntryIndex)
        {
            const FKyaBankEntry& Candidate = Bank->Entries[EntryIndex];
            if (Candidate.Type != 5 || Candidate.Subtype != 1) continue;
            const TConstArrayView<uint8> CandidateData = EntryData(&Candidate);
            if (CandidateData.IsEmpty()) continue;
            TMap<uint64, int32>& CandidateMaterials = TextureBankMaterials.Add(EntryIndex);
            KyaG2D::ReadMaterials(CandidateData, CandidateMaterials);
            TextureBankData.Add(EntryIndex, CandidateData);
            for (const auto& Pair : CandidateMaterials) TextureBankByHash.FindOrAdd(Pair.Key).Add(EntryIndex);
        }
        for (int32 ActorIndex = 0; ActorIndex < Bank->SceneEnvironment.Actors.Num(); ++ActorIndex)
        {
        if ((ActorIndex & 31) == 0 && CheckRebuildCancel())
        {
            RestorePresentation();
            RebuildStatus = TEXT("Cancelled");
            return;
        }
        if (!bShowAllActorMeshes && !bActorHandlesOwnGeometry && ActorIndex != PreviewActorIndex) continue;
        const FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[ActorIndex];
        if (bActorHandlesOwnGeometry)
        {
            const uint64 Revision = HashActorSectionRevision(Actor, Bank, Bank->EditGeneration,
                ActorLightingRevision);
            if (ActorSectionRevisions.FindRef(ActorIndex) == Revision && ActorMeshSections.Contains(ActorIndex))
                continue;
            ActorSectionRevisions.Add(ActorIndex, Revision);
            ActorCapturedPositions.Add(ActorIndex, Actor.Position);
        }
        TArray<FKyaActorMeshSection>* CapturedSections =
            bActorHandlesOwnGeometry ? &ActorMeshSections.FindOrAdd(ActorIndex) : nullptr;
        if (CapturedSections) CapturedSections->Reset();
        const bool bUseNativeTransform = ActorIndex == PreviewActorIndex && PreviewActorNativeTransform.Num() == 4;
        const FVector ActorPosition = bUseNativeTransform ? PreviewActorNativeTransform[3] : Actor.Position;
        const auto TransformActorVector = [&](const FVector& Vector) {
            if (bUseNativeTransform)
                return PreviewActorNativeTransform[0] * Vector.X + PreviewActorNativeTransform[1] * Vector.Y +
                    PreviewActorNativeTransform[2] * Vector.Z;
            return KyaActorTransform::ApplyClassRotation(Vector * Actor.Scale, Actor.Rotation, Actor.ClassId);
        };
        float LightingGain = Actor.LightingValue;
        // SetupLighting enables ground modulation when serialized bit 1 is clear.
        if (bApplyActorLighting && (Actor.LightingFlags & 2) == 0)
        {
            uint16 Sample = 0xffff;
            double Height = -TNumericLimits<double>::Max();
            for (const UKyaLevelBank* Sector : SectorBanks)
                if (Sector) FindSceneryGroundLight(*Sector, ActorPosition, Height, Sample);
            LightingGain *= FMath::Sqrt((Sample & 15) * 0.01328125f +
                ((Sample >> 4) & 15) * 0.0447125f + ((Sample >> 8) & 15) * 0.00450625f);
        }
        const auto ModelData = EntryData(FindEntry(4, Actor.MeshIndex));
        const int32 TextureIndex = Actor.TextureIndex == -1 ? Bank->SceneEnvironment.AlternateTextureIndex : Actor.TextureIndex;
        const auto ModelTextures = EntryData(FindEntry(5, TextureIndex));
        if (!MaterialsByTexture.Contains(TextureIndex))
        {
            TMap<uint64, int32>& Materials = MaterialsByTexture.Add(TextureIndex);
            KyaG2D::ReadMaterials(ModelTextures, Materials);
        }
        const TMap<uint64, int32>& ModelMaterials = MaterialsByTexture[TextureIndex];
        FMaterialSections& ModelSections = SectionsByTexture.FindOrAdd(TextureIndex);
        TMap<uint64, uint64> MaterialPatches;
        for (const FKyaActorModelReference& Patch : Actor.Models)
        {
            const uint64 Replacement = KyaG2D::HashName(Patch.TextureName);
            if (SharedMaterials.Contains(Replacement)) MaterialPatches.Add(KyaG2D::HashName(Patch.MeshName), Replacement);
        }
        if (!StripsByMesh.Contains(Actor.MeshIndex))
        {
            TArray<FKyaG3DDecodedStrip>& ModelStrips = StripsByMesh.Add(Actor.MeshIndex);
            if (DecodeCacheSnapshot.IsValid() &&
                DecodeCacheSnapshot->StripsByMesh.Contains(Actor.MeshIndex))
            {
                ModelStrips = DecodeCacheSnapshot->StripsByMesh[Actor.MeshIndex];
                CachedActorDecodes.Increment();
            }
            else
            {
                KyaG3D::DecodeHierarchyStrips(ModelData, ModelStrips);
                SynchronousActorDecodes.Increment();
            }
        }
        int32 ActorStripIndex = 0;
        for (const FKyaG3DDecodedStrip& Strip : StripsByMesh[Actor.MeshIndex])
            {
                const int32 StripIndex = ActorStripIndex++;
                if (ActorIndex == PreviewActorIndex && PreviewActorStripIndex >= 0 && StripIndex != PreviewActorStripIndex) continue;
                if (bLogSectorDiagnostics)
                    UE_LOG(LogTemp, Display, TEXT("KYA_PREVIEW_ACTOR_STRIP actor=%s index=%d material=%d hash=%016llx vertices=%d attributes=%d triangles=%d"),
                        *Actor.Name, StripIndex, Strip.MaterialIndex, Strip.MaterialHash,
                        Strip.Vertices.Num(), Strip.Attributes.Num(), Strip.Indices.Num() / 3);
                FMatrix HierarchyTransform;
                if (!KyaG3D::ReadHierarchyWorldMatrix(ModelData, Strip.HierarchyOffset, HierarchyTransform)) continue;
                FSectionData* Textured = nullptr;
                // Native resolves a material hash against linked banks in order and stops
                // at the first bank that yields a usable material. Try each candidate
                // until one decodes, so a failed section does not fall back to untextured
                // geometry.
                auto TryActorSection = [&](uint64 Hash, TConstArrayView<uint8> Source,
                    const TMap<uint64, int32>& Offsets, FMaterialSections& Sections,
                    const TCHAR* Label) -> bool
                {
                    if (!Offsets.Contains(Hash)) return false;
                    Textured = GetTexturedSection(Hash, Source, Offsets, Sections, Label, Strip.Primitive);
                    return Textured != nullptr;
                };
                if (const uint64* Replacement = MaterialPatches.Find(Strip.MaterialHash))
                    TryActorSection(*Replacement, SharedTextures, SharedMaterials, SharedSections, TEXT("ActorPatch"));
                if (!Textured)
                    TryActorSection(Strip.MaterialHash, SharedTextures, SharedMaterials, SharedSections, TEXT("Shared"));
                if (!Textured)
                    TryActorSection(Strip.MaterialHash, ModelTextures, ModelMaterials, ModelSections, TEXT("Model"));
                if (!Textured)
                {
                    const TArray<int32>* Owners = TextureBankByHash.Find(Strip.MaterialHash);
                    if (Owners)
                        for (int32 OwnerEntry : *Owners)
                            if (TryActorSection(Strip.MaterialHash, TextureBankData[OwnerEntry],
                                TextureBankMaterials[OwnerEntry], TextureBankSections[OwnerEntry], TEXT("Union")))
                                break;
                }
                if (!Textured && bLogSectorDiagnostics)
                    UE_LOG(LogTemp, Warning, TEXT("KYA_ACTOR_MATERIAL_MISSING actor=%s strip=%d hash=%016llx"),
                        *Actor.Name, StripIndex, Strip.MaterialHash);
                TArray<FVector>& TargetPositions = Textured ? Textured->Positions : Positions;
                TArray<int32>& TargetTriangles = Textured ? Textured->Triangles : Triangles;
                TArray<FVector2D>& TargetUVs = Textured ? Textured->UVs : UVs;
                TArray<FColor>& TargetColors = Textured ? Textured->Colors : Colors;
                // A captured section belongs to one actor handle: it keeps preview-local
                // vertices without actor translation and its own triangle base.
                TArray<FVector> CapturedPositions;
                TArray<int32> CapturedTriangles;
                TArray<FVector2D> CapturedUVs;
                TArray<FColor> CapturedColors;
                TArray<FVector>& OutPositions = CapturedSections ? CapturedPositions : TargetPositions;
                TArray<int32>& OutTriangles = CapturedSections ? CapturedTriangles : TargetTriangles;
                TArray<FVector2D>& OutUVs = CapturedSections ? CapturedUVs : TargetUVs;
                TArray<FColor>& OutColors = CapturedSections ? CapturedColors : TargetColors;
                const int32 Base = OutPositions.Num();
                for (const FKyaG3DVertex& Vertex : Strip.Vertices)
                {
                    const FVector Local = TransformActorVector(HierarchyTransform.TransformPosition(FVector(Vertex.Position)));
                    if (CapturedSections)
                    {
                        // Rotation and scale live on the handle transform, so captured
                        // vertices keep only the native hierarchy transform.
                        OutPositions.Add(KyaActorTransform::SwapGameAxes(
                            HierarchyTransform.TransformPosition(FVector(Vertex.Position))) * 100.0);
                    }
                    else
                    {
                        const FVector P = Local + ActorPosition;
                        OutPositions.Add(FVector(P.X, P.Z, P.Y) * 100.0);
                    }
                }
                TArray<FVector3f> WorldNormals;
                if (Strip.Normals.Num() == Strip.Vertices.Num())
                {
                    // Native orthonormalizes object-to-world before transforming the
                    // light directions. Apply the equivalent basis to the stored normals.
                    const auto TransformAxis = [&](const FVector& Axis) {
                        return TransformActorVector(HierarchyTransform.TransformVector(Axis));
                    };
                    const FVector X = TransformAxis(FVector::ForwardVector).GetSafeNormal();
                    const FVector Z = FVector::CrossProduct(X, TransformAxis(FVector::RightVector)).GetSafeNormal();
                    const FVector Y = FVector::CrossProduct(Z, X);
                    WorldNormals.Reserve(Strip.Normals.Num());
                    for (const FVector3f& Normal : Strip.Normals)
                        WorldNormals.Add(FVector3f(X * Normal.X + Y * Normal.Y + Z * Normal.Z));
                }
                const TArray<FColor> Relit = RelightStrip(Strip, WorldNormals, Actor.SectorId, LightingGain);
                if (bLogSectorDiagnostics && StripIndex < 3 && Strip.Attributes.Num() > 0 && Relit.Num() > 0)
                {
                    const FColor& Original = Strip.Attributes[0].Color;
                    const FColor& Lit = Relit[0];
                    const FVector3f N0 = WorldNormals.Num() > 0 ? WorldNormals[0] : FVector3f::ZeroVector;
                    UE_LOG(LogTemp, Display,
                        TEXT("KYA_ACTOR_RELIT actor=%s strip=%d vertex0 original=(%d,%d,%d,%d) relit=(%d,%d,%d,%d) normal=(%.3f,%.3f,%.3f) flags=%d value=%.3f"),
                        *Actor.Name, StripIndex, Original.R, Original.G, Original.B, Original.A,
                        Lit.R, Lit.G, Lit.B, Lit.A, N0.X, N0.Y, N0.Z,
                        int32(Actor.LightingFlags), Actor.LightingValue);
                }
                for (int32 AttributeIndex = 0; AttributeIndex < Strip.Attributes.Num(); ++AttributeIndex)
                {
                    OutUVs.Add(FVector2D(Strip.Attributes[AttributeIndex].UV));
                    FColor Color = AttributeIndex < Relit.Num() ? Relit[AttributeIndex] : Strip.Attributes[AttributeIndex].Color;
                    if (Textured && ActorIndex == TextureOnlyActorIndex)
                    {
                        Color.R = Color.G = Color.B = 128;
                    }
                    OutColors.Add(Color);
                }
                AppendUnrealTriangles(OutTriangles, Strip.Indices, Base);
                if (CapturedSections)
                {
                    FKyaActorMeshSection& Section = CapturedSections->AddDefaulted_GetRef();
                    Section.Material = Textured ? Textured->Material
                        : (GeneratedUntexturedMaterial ? GeneratedUntexturedMaterial.Get() : Mesh->GetMaterial(0));
                    Section.MaterialHash = Strip.MaterialHash;
                    Section.StripIndex = StripIndex;
                    Section.Positions = MoveTemp(CapturedPositions);
                    Section.Triangles = MoveTemp(CapturedTriangles);
                    Section.UVs = MoveTemp(CapturedUVs);
                    Section.Colors = MoveTemp(CapturedColors);
                }
            }
        }
        if (bShowAllActorMeshes && bLogSectorDiagnostics)
        {
            int32 ActorStrips = 0;
            for (const auto& Pair : StripsByMesh) ActorStrips += Pair.Value.Num();
            UE_LOG(LogTemp, Display, TEXT("KYA_PREVIEW_ACTORS actors=%d meshes=%d strips=%d positions=%d triangles=%d"),
                Bank->SceneEnvironment.Actors.Num(), StripsByMesh.Num(), ActorStrips, Positions.Num(), Triangles.Num());
        }
    }
    // The common scene mesh is a static cluster, just like sector clusters.
    TArray<const UKyaLevelBank*> ClusterBanks;
    ClusterBanks.Add(Bank);
    for (const UKyaLevelBank* Sector : SectorBanks) ClusterBanks.Add(Sector);
    TConstArrayView<uint8> CommonClusterTextures;
    TMap<uint64, int32> CommonClusterMaterials;
    FMaterialSections CommonClusterSections;
    if (Bank->Entries.IsValidIndex(Bank->SceneTextureEntry))
    {
        const FKyaBankEntry& TextureEntry = Bank->Entries[Bank->SceneTextureEntry];
        if (TextureEntry.Offset >= 0 && TextureEntry.Size >= 0 && TextureEntry.Size <= MAX_int32 &&
            TextureEntry.Offset <= Bank->BankBytes.Num() && TextureEntry.Size <= Bank->BankBytes.Num() - TextureEntry.Offset)
        {
            CommonClusterTextures = MakeArrayView(Bank->BankBytes.GetData() + TextureEntry.Offset, int32(TextureEntry.Size));
            KyaG2D::ReadMaterials(CommonClusterTextures, CommonClusterMaterials);
        }
    }
    // Only capture hierarchy geometry when handles will actually render; in a game
    // world (PIE/packaged) the shared mesh remains the only draw path.
    bool bCaptureHierarchyGeometry = false;
#if WITH_EDITOR
    bCaptureHierarchyGeometry = bSpawnHierarchyHandles && GetWorld() && !GetWorld()->IsGameWorld();
#endif
    for (const UKyaLevelBank* Sector : ClusterBanks)
    {
        if (CheckRebuildCancel())
        {
            RestorePresentation();
            RebuildStatus = TEXT("Cancelled");
            UE_LOG(LogTemp, Display, TEXT("KYA_REBUILD_CANCELLED bank=%s sector=%s"), *GetNameSafe(Bank),
                *GetNameSafe(Sector));
            return;
        }
        if (!Sector) continue;
        const FString DiagnosticName = Sector == Bank ? TEXT("<common>") : Sector->GetName();
        if (bLogSectorDiagnostics)
        {
            UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_BEGIN name=%s entries=%d bytes=%d"), *DiagnosticName, Sector->Entries.Num(), Sector->BankBytes.Num());
            for (int32 EntryIndex = 0; EntryIndex < Sector->Entries.Num(); ++EntryIndex)
            {
                const FKyaBankEntry& E = Sector->Entries[EntryIndex];
                if (Sector == Bank && EntryIndex != Bank->SceneMeshEntry && EntryIndex != Bank->SceneTextureEntry) continue;
                UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_ENTRY name=%s index=%d type=%d subtype=%d offset=%lld size=%lld"),
                    *DiagnosticName, EntryIndex, E.Type, E.Subtype, E.Offset, E.Size);
            }
        }
        TConstArrayView<uint8> SectorTextureData;
        TMap<uint64, int32> SectorMaterialOffsets;
        FMaterialSections SectorMaterialSections;
        TConstArrayView<uint8> ClusterTextureData;
        TMap<uint64, int32> ClusterMaterialOffsets;
        FMaterialSections ClusterMaterialSections;
        for (const FKyaBankEntry& TextureEntry : Sector->Entries)
            if (Sector != Bank && TextureEntry.Type == 5 && TextureEntry.Subtype == 1 && TextureEntry.Offset >= 0 &&
                TextureEntry.Size >= 0 && TextureEntry.Size <= MAX_int32 && TextureEntry.Offset <= Sector->BankBytes.Num() &&
                TextureEntry.Size <= Sector->BankBytes.Num() - TextureEntry.Offset)
            {
                ClusterTextureData = MakeArrayView(Sector->BankBytes.GetData() + TextureEntry.Offset, int32(TextureEntry.Size));
                KyaG2D::ReadMaterials(ClusterTextureData, ClusterMaterialOffsets);
                break;
            }
        for (const FKyaBankEntry& TextureEntry : Sector->Entries)
            if (Sector != Bank && TextureEntry.Type == 5 && TextureEntry.Subtype == 3 && TextureEntry.Offset >= 0 &&
                TextureEntry.Size >= 0 && TextureEntry.Size <= MAX_int32 && TextureEntry.Offset <= Sector->BankBytes.Num() &&
                TextureEntry.Size <= Sector->BankBytes.Num() - TextureEntry.Offset)
            {
                SectorTextureData = MakeArrayView(Sector->BankBytes.GetData() + TextureEntry.Offset, int32(TextureEntry.Size));
                KyaG2D::ReadMaterials(SectorTextureData, SectorMaterialOffsets);
                break;
            }
        for (int32 EntryIndex = 0; EntryIndex < Sector->Entries.Num(); ++EntryIndex)
        {
            const FKyaBankEntry& SectorEntry = Sector->Entries[EntryIndex];
            if (Sector == Bank && EntryIndex != Bank->SceneMeshEntry) continue;
            if (SectorEntry.Type == 4 && SectorEntry.Subtype == 1 && SectorEntry.Offset >= 0 && SectorEntry.Size >= 0 &&
                SectorEntry.Offset <= Sector->BankBytes.Num() && SectorEntry.Size <= Sector->BankBytes.Num() - SectorEntry.Offset && SectorEntry.Size <= MAX_int32)
            {
                const auto Data = MakeArrayView(Sector->BankBytes.GetData() + SectorEntry.Offset, int32(SectorEntry.Size));
                if (bLogSectorDiagnostics)
                {
                    TArray<int32> Hierarchies;
                    TArray<FKyaG3DStripReference> HierarchyStrips;
                    TArray<FKyaG3DDecodedStrip> DecodedHierarchy;
                    const bool HierarchyParsed = KyaG3D::ReadHierarchyOffsets(Data, Hierarchies);
                    const bool ReferencesParsed = KyaG3D::ReadHierarchyStrips(Data, HierarchyStrips);
                    const bool HierarchyDecoded = KyaG3D::DecodeHierarchyStrips(Data, DecodedHierarchy);
                    UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_HIERARCHIES name=%s entry=%d offsets_ok=%d hierarchy_count=%d references_ok=%d strip_references=%d decoded_ok=%d decoded_strips=%d"),
                        *DiagnosticName, EntryIndex, HierarchyParsed, Hierarchies.Num(), ReferencesParsed, HierarchyStrips.Num(), HierarchyDecoded, DecodedHierarchy.Num());
                    TArray<FKyaG3DChunk> DiagnosticChunks;
                    if (KyaG3D::ReadChunks(Data, DiagnosticChunks))
                    {
                        const FKyaG3DChunk* Cluster = DiagnosticChunks.FindByPredicate([](const FKyaG3DChunk& Chunk) { return Chunk.Tag == 0x41545343; });
                        if (Cluster && Cluster->Size >= 144)
                        {
                            auto Word = [&Data](int32 Offset) -> uint32 {
                                const uint8* P = Data.GetData() + Offset;
                                return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
                            };
                            const int32 Root = Cluster->Offset + 64;
                            const int32 Payload = Root + 16, Hash = Payload + 64;
                            const int32 Count = int32(Data[Payload + 0x1a]) | (int32(Data[Payload + 0x1b]) << 8);
                            if (Hash <= Data.Num() - 16 && Word(Hash) == 0x48534148 && Count <= (Data.Num() - Hash - 16) / 16)
                            {
                                UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_CLUSTER_HIERARCHIES name=%s count=%d"), *DiagnosticName, Count);
                                for (int32 HierarchyIndex = 0; HierarchyIndex < Count; ++HierarchyIndex)
                                {
                                    const uint32 Target = Word(Hash + 16 + HierarchyIndex * 16 + 8);
                                    int32 StripCount = 0;
                                    for (const FKyaG3DStripReference& Ref : HierarchyStrips)
                                        if (int64(Ref.HierarchyOffset) == int64(Target) + 16) ++StripCount;
                                    UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_CLUSTER_HIERARCHY name=%s index=%d chunk_offset=%u in_hall=%d strip_references=%d"),
                                        *DiagnosticName, HierarchyIndex, Target, Hierarchies.Contains(int32(Target + 16)), StripCount);
                                }
                            }
                        }
                    }
                }
                TArray<KyaG3D::FClusterStripReference> ClusterStrips;
                TArray<int32> ClusterHierarchies;
                const bool ClusterParsed = KyaG3D::ReadClusterTreeStrips(Data, ClusterStrips, &ClusterHierarchies);
                int32 Decoded = 0, Failed = 0, TexturedCount = 0, MissingHash = 0, MissingMaterial = 0;
                int64 VertexCount = 0, TriangleCount = 0;
                FBox Bounds(ForceInit);
                if (ClusterParsed)
                {
                    for (const KyaG3D::FClusterStripReference& ClusterStrip : ClusterStrips)
                    {
                        const int32 Strip = ClusterStrip.StripOffset;
                        uint32 Count = 0, Primitive = 0;
                        TArray<FKyaG3DVertex> Vertices;
                        TArray<FKyaG3DVertexAttributes> Attributes;
                        TArray<uint32> Sections, Indices;
                        if (!KyaG3D::ReadFirstStripGifTag(Data, Strip, Count, Primitive) ||
                            !KyaG3D::ReadStripVertices(Data, Strip, Vertices, Sections) ||
                            !KyaG3D::ReadStripAttributes(Data, Strip, Attributes)) { ++Failed; continue; }
                        TArray<FKyaG3DVertex> Expanded;
                        uint32 SourceBase = 0;
                        for (int32 Section = 0; Section < Sections.Num(); ++Section)
                        {
                            for (uint32 Index = 0; Index < Sections[Section]; ++Index)
                                Expanded.Add(Vertices[SourceBase + Index]);
                            SourceBase += Sections[Section];
                            if (Section + 1 < Sections.Num()) SourceBase -= 2;
                        }
                        if (Expanded.Num() != Attributes.Num()) { ++Failed; continue; }
                        const uint32 ExpandedCount = Expanded.Num();
                        if (!KyaG3D::BuildStripTriangles(Expanded, MakeArrayView(&ExpandedCount, 1), Primitive, Indices)) { ++Failed; continue; }
                        uint64 MaterialHash = 0;
                        FSectionData* Textured = nullptr;
                        if (KyaG3D::ReadStripMaterialHash(Data, Strip, ClusterStrip.MaterialBank, MaterialHash))
                        {
                            if (!ClusterMaterialOffsets.Contains(MaterialHash) && !CommonClusterMaterials.Contains(MaterialHash)) ++MissingMaterial;
                            if (ClusterMaterialOffsets.Contains(MaterialHash))
                                Textured = GetTexturedSection(MaterialHash, ClusterTextureData, ClusterMaterialOffsets, ClusterMaterialSections, TEXT("Cluster"), Primitive);
                            else
                                Textured = GetTexturedSection(MaterialHash, CommonClusterTextures, CommonClusterMaterials, CommonClusterSections, TEXT("CommonCluster"), Primitive);
                        }
                        else ++MissingHash;
                        ++Decoded;
                        if (Textured) ++TexturedCount;
                        VertexCount += Expanded.Num();
                        TriangleCount += Indices.Num() / 3;
                        if (!bShowClusterMeshes) continue;
                        TArray<FVector>& TargetPositions = Textured ? Textured->Positions : Positions;
                        TArray<int32>& TargetTriangles = Textured ? Textured->Triangles : Triangles;
                        TArray<FVector2D>& TargetUVs = Textured ? Textured->UVs : UVs;
                        TArray<FColor>& TargetColors = Textured ? Textured->Colors : Colors;
                        const int32 Base = TargetPositions.Num();
                        FKyaG3DDecodedStrip Visibility;
                        FMemory::Memcpy(&Visibility.StripSphere, Data.GetData() + Strip + 16, 16);
                        Visibility.ObjectSphere = Visibility.StripSphere;
                        Visibility.FirstLodSphere = Visibility.StripSphere;
                        RegisterClusterRange(Visibility, FMatrix::Identity, Textured ? Textured->Section : 0,
                            TargetTriangles.Num(), Indices.Num(), false);
                        for (int32 VertexIndex = 0; VertexIndex < Expanded.Num(); ++VertexIndex)
                        {
                            const FKyaG3DVertex& Vertex = Expanded[VertexIndex];
                            const FVector Position = FVector(Vertex.Position.X, Vertex.Position.Z, Vertex.Position.Y) * 100.0;
                            TargetPositions.Add(Position);
                            Bounds += Position;
                            TargetUVs.Add(FVector2D(Attributes[VertexIndex].UV));
                            TargetColors.Add(Attributes[VertexIndex].Color);
                        }
                        AppendUnrealTriangles(TargetTriangles, Indices, Base);
                    }
                }
                if (bLogSectorDiagnostics)
                    UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_CLUSTER name=%s entry=%d parsed=%d strips=%d decoded=%d failed=%d textured=%d no_hash=%d missing_material=%d local_materials=%d common_materials=%d vertices=%lld triangles=%lld bounds_cm=%s"),
                        *DiagnosticName, EntryIndex, ClusterParsed, ClusterStrips.Num(), Decoded, Failed, TexturedCount, MissingHash,
                        MissingMaterial, ClusterMaterialOffsets.Num(), CommonClusterMaterials.Num(), VertexCount, TriangleCount, *Bounds.ToString());
                if (ClusterParsed && bShowSprites && Sector != Bank)
                {
                    TArray<FKyaG3DPreparedSprite> Sprites;
                    if (KyaG3D::ReadClusterSprites(Data, Sprites) && !Sprites.IsEmpty())
                    {
                        const FVector Forward = SpriteCameraForward.GetSafeNormal();
                        const FVector Up = SpriteCameraUp.GetSafeNormal();
                        const FVector Right = FVector::CrossProduct(Up, Forward).GetSafeNormal();
                        int32 SpriteRects = 0, SpriteTextured = 0;
                        for (const FKyaG3DPreparedSprite& Sprite : Sprites)
                        {
                            FSectionData* Textured = nullptr;
                            if (ClusterMaterialOffsets.Contains(Sprite.MaterialHash))
                                Textured = GetTexturedSection(Sprite.MaterialHash, ClusterTextureData,
                                    ClusterMaterialOffsets, ClusterMaterialSections, TEXT("SpriteCluster"), 0);
                            else if (CommonClusterMaterials.Contains(Sprite.MaterialHash))
                                Textured = GetTexturedSection(Sprite.MaterialHash, CommonClusterTextures,
                                    CommonClusterMaterials, CommonClusterSections, TEXT("SpriteCommon"), 0);
                            if (!Textured) continue;
                            ++SpriteTextured;
                            for (const FKyaG3DSpriteRect& Rect : Sprite.Rects)
                            {
                                const FVector Center(Rect.Center.X, Rect.Center.Y, Rect.Center.Z);
                                const FVector Half(Rect.HalfSize.X, Rect.HalfSize.Y, 0.0);
                                const FVector Corners[4] = {
                                    Center - Right * Half.X + Up * Half.Y,
                                    Center - Right * Half.X - Up * Half.Y,
                                    Center + Right * Half.X + Up * Half.Y,
                                    Center + Right * Half.X - Up * Half.Y
                                };
                                const int32 Base = Textured->Positions.Num();
                                for (int32 Corner = 0; Corner < 4; ++Corner)
                                {
                                    const FVector P = Corners[Corner];
                                    const FVector Position = FVector(P.X, P.Z, P.Y) * 100.0;
                                    Textured->Positions.Add(Position);
                                    Bounds += Position;
                                    Textured->UVs.Add(FVector2D(Rect.UV[Corner].X, Rect.UV[Corner].Y));
                                    Textured->Colors.Add(Rect.Color[Corner]);
                                }
                                Textured->Triangles.Add(Base + 0);
                                Textured->Triangles.Add(Base + 1);
                                Textured->Triangles.Add(Base + 2);
                                Textured->Triangles.Add(Base + 2);
                                Textured->Triangles.Add(Base + 1);
                                Textured->Triangles.Add(Base + 3);
                                ++SpriteRects;
                            }
                        }
                        if (bLogSectorDiagnostics)
                            UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_SPRITES name=%s sprites=%d textured=%d rects=%d"),
                                *DiagnosticName, Sprites.Num(), SpriteTextured, SpriteRects);
                    }
                }
                if (ClusterParsed)
                {
                    TArray<FKyaG3DDecodedStrip> HierarchyStrips;
                    TArray<int32> ExpandedHierarchies;
                    const bool LinksParsed = KyaG3D::ExpandClusterHierarchies(Data, ClusterHierarchies, ExpandedHierarchies);
                    TConstArrayView<uint8> AnimationData;
                    for (const FKyaBankEntry& AnimationEntry : Sector->Entries)
                        if (AnimationEntry.Type == 23 && AnimationEntry.Subtype == 1 && AnimationEntry.Offset >= 0 &&
                            AnimationEntry.Size >= 0 && AnimationEntry.Size <= MAX_int32 && AnimationEntry.Offset <= Sector->BankBytes.Num() &&
                            AnimationEntry.Size <= Sector->BankBytes.Num() - AnimationEntry.Offset)
                        {
                            AnimationData = MakeArrayView(Sector->BankBytes.GetData() + AnimationEntry.Offset, int32(AnimationEntry.Size));
                            break;
                        }
                    TArray<int32> SceneHierarchies;
                    const bool SceneParsed = KyaG3D::ReadSectorSceneHierarchies(Data, AnimationData, SceneHierarchies);
                    TArray<int32> RenderHierarchies = ExpandedHierarchies;
                    for (int32 H : SceneHierarchies) RenderHierarchies.AddUnique(H);
                    bool Parsed = LinksParsed && SceneParsed;
                    int32 MaxLods = 0;
                    if (LinksParsed)
                        for (int32 Hierarchy : RenderHierarchies)
                            MaxLods = FMath::Max(MaxLods, int32(uint16(Data[Hierarchy + 0x9c]) | (uint16(Data[Hierarchy + 0x9d]) << 8)));
                    for (int32 Lod = 0; Parsed && Lod < MaxLods; ++Lod)
                    {
                        TArray<FKyaG3DDecodedStrip> LodStrips;
                        Parsed = KyaG3D::DecodeHierarchyStrips(Data, LodStrips, Lod);
                        HierarchyStrips.Append(MoveTemp(LodStrips));
                    }
                    int32 Included = 0, HierarchyTextured = 0, TransformFailures = 0, HiddenStrips = 0;
                    int64 HierarchyTriangles = 0;
                    FBox HierarchyBounds(ForceInit);
                    if (Parsed)
                        for (const FKyaG3DDecodedStrip& Strip : HierarchyStrips)
                        {
                            if (!bShowClusterMeshes) continue;
                            if (!RenderHierarchies.Contains(Strip.HierarchyOffset)) continue;
                            const bool IsSceneInstance = SceneHierarchies.Contains(Strip.HierarchyOffset);
                            if (IsSceneInstance && Strip.LodIndex != 0) continue;
                            const uint16 Flags = uint16(Data[Strip.HierarchyOffset + 0x9e]) | (uint16(Data[Strip.HierarchyOffset + 0x9f]) << 8);
                            if (Flags & 0x40) continue;
                            FMatrix HierarchyTransform;
                            if (!KyaG3D::ReadHierarchyWorldMatrix(Data, Strip.HierarchyOffset, HierarchyTransform)) { ++TransformFailures; continue; }
                            // World geometry is selected and toggled per hierarchy: a cluster
                            // node's strips share the serialized hierarchy transform, and a strip
                            // has no independent editable state.
                            const FString HierarchyKey = MakeHierarchyKey(Sector, Strip.HierarchyOffset);
                            FHierarchySelectionInfo& HierarchyInfo = HierarchySelectionInfo.FindOrAdd(HierarchyKey);
                            HierarchyInfo.Bank = const_cast<UKyaLevelBank*>(Sector);
                            HierarchyInfo.SectorId = Sector == Bank ? -1 : ResolveSectorId(*Sector);
                            HierarchyInfo.Offset = Strip.HierarchyOffset;
                            HierarchyInfo.Origin = HierarchyTransform.GetOrigin();
                            ++HierarchyInfo.Strips;
                            HierarchyInfo.Triangles += Strip.Indices.Num() / 3;
                            if (bCaptureHierarchyGeometry && HiddenHierarchies.Contains(HierarchyKey))
                            {
                                ++HiddenStrips;
                                continue;
                            }
                            FSectionData* Textured = ClusterMaterialOffsets.Contains(Strip.MaterialHash)
                                ? GetTexturedSection(Strip.MaterialHash, ClusterTextureData, ClusterMaterialOffsets, ClusterMaterialSections, TEXT("Cluster"), Strip.Primitive)
                                : GetTexturedSection(Strip.MaterialHash, CommonClusterTextures, CommonClusterMaterials, CommonClusterSections, TEXT("CommonCluster"), Strip.Primitive);

                            // Editing mode: the hierarchy handle owns its strips so the
                            // rendered geometry itself is clickable at hierarchy
                            // granularity. Shared-mesh ranges are skipped to avoid
                            // drawing two copies; cluster visibility is the handle's
                            // presentation-only bVisible toggle in this mode.
                            if (bCaptureHierarchyGeometry)
                            {
                                const int32 SectionKey = Textured ? Textured->Section : 0;
                                FKyaActorMeshSection* Capture = nullptr;
                                for (FKyaActorMeshSection& Section : HierarchyInfo.Sections)
                                {
                                    if (Section.StripIndex == SectionKey) { Capture = &Section; break; }
                                }
                                if (!Capture)
                                {
                                    Capture = &HierarchyInfo.Sections.AddDefaulted_GetRef();
                                    Capture->StripIndex = SectionKey;
                                    Capture->MaterialHash = Strip.MaterialHash;
                                    Capture->Material = Textured ? Textured->Material : GeneratedUntexturedMaterial.Get();
                                    HierarchyInfo.MaterialHashes.Add(Strip.MaterialHash);
                                    HierarchyInfo.Materials.Add(Capture->Material
                                        ? Capture->Material->GetName()
                                        : FString::Printf(TEXT("hash %016llx"), Strip.MaterialHash));
                                }
                                const int32 Base = Capture->Positions.Num();
                                for (const FKyaG3DVertex& Vertex : Strip.Vertices)
                                {
                                    const FVector P = HierarchyTransform.TransformPosition(FVector(Vertex.Position));
                                    const FVector Position = FVector(P.X, P.Z, P.Y) * 100.0;
                                    Capture->Positions.Add(Position);
                                    HierarchyInfo.Bounds += Position;
                                }
                                for (const FKyaG3DVertexAttributes& Attribute : Strip.Attributes)
                                {
                                    Capture->UVs.Add(FVector2D(Attribute.UV));
                                    Capture->Colors.Add(Attribute.Color);
                                }
                                AppendUnrealTriangles(Capture->Triangles, Strip.Indices, Base);
                                ++Included;
                                if (Textured) ++HierarchyTextured;
                                HierarchyTriangles += Strip.Indices.Num() / 3;
                                continue;
                            }

                            TArray<FVector>& TargetPositions = Textured ? Textured->Positions : Positions;
                            TArray<int32>& TargetTriangles = Textured ? Textured->Triangles : Triangles;
                            TArray<FVector2D>& TargetUVs = Textured ? Textured->UVs : UVs;
                            TArray<FColor>& TargetColors = Textured ? Textured->Colors : Colors;
                            const int32 Base = TargetPositions.Num();
                            RegisterClusterRange(Strip, HierarchyTransform, Textured ? Textured->Section : 0,
                                TargetTriangles.Num(), Strip.Indices.Num(), !IsSceneInstance);
                            for (const FKyaG3DVertex& Vertex : Strip.Vertices)
                            {
                                const FVector P = HierarchyTransform.TransformPosition(FVector(Vertex.Position));
                                const FVector Position = FVector(P.X, P.Z, P.Y) * 100.0;
                                TargetPositions.Add(Position);
                                HierarchyBounds += Position;
                            }
                            for (const FKyaG3DVertexAttributes& Attribute : Strip.Attributes)
                            {
                                TargetUVs.Add(FVector2D(Attribute.UV));
                                TargetColors.Add(Attribute.Color);
                            }
                            AppendUnrealTriangles(TargetTriangles, Strip.Indices, Base);
                            ++Included;
                            if (Textured) ++HierarchyTextured;
                            HierarchyTriangles += Strip.Indices.Num() / 3;
                        }
                    if (bLogSectorDiagnostics)
                    {
                        UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_SCENE_HIERARCHIES name=%s parsed=%d hierarchies=%d"),
                            *DiagnosticName, SceneParsed, SceneHierarchies.Num());
                        UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_CLUSTER_HIERARCHY_RENDER name=%s parsed=%d roots=%d linked=%d strips=%d textured=%d transform_failures=%d hidden_strips=%d triangles=%lld bounds_cm=%s"),
                            *DiagnosticName, Parsed, ClusterHierarchies.Num(), ExpandedHierarchies.Num(), Included, HierarchyTextured, TransformFailures, HiddenStrips, HierarchyTriangles, *HierarchyBounds.ToString());
                    }
                }
            }
            if (Sector == Bank || SectorEntry.Type != 4 || SectorEntry.Subtype != 2 || SectorEntry.Offset < 0 || SectorEntry.Size < 0 ||
                SectorEntry.Offset > Sector->BankBytes.Num() || SectorEntry.Size > Sector->BankBytes.Num() - SectorEntry.Offset || SectorEntry.Size > MAX_int32) continue;
            const auto Data = MakeArrayView(Sector->BankBytes.GetData() + SectorEntry.Offset, int32(SectorEntry.Size));
            TArray<FKyaG3DDecodedStrip> SectorStrips;
            const bool BackgroundParsed = KyaG3D::DecodeHierarchyStrips(Data, SectorStrips);
            if (!BackgroundParsed)
            {
                if (bLogSectorDiagnostics) UE_LOG(LogTemp, Warning, TEXT("KYA_SECTOR_BACKGROUND name=%s entry=%d parsed=0"), *DiagnosticName, EntryIndex);
                continue;
            }
            FBox BackgroundBounds(ForceInit);
            int32 BackgroundTextured = 0, BackgroundMissingTransform = 0;
            int64 BackgroundVertices = 0, BackgroundTriangles = 0;
            TArray<int32> SectorCameraRelative;
            AddBackgroundFlares(Data, SectorTextureData, SectorMaterialOffsets);
            const FKyaSectorRenderSettings* Settings = SectorSettings.Find(ResolveSectorId(*Sector));
            const uint32 BackgroundFlags = Settings ? Settings->BackgroundFlags : uint32(Bank->SceneEnvironment.BackgroundFlags);
            if (bLogSectorDiagnostics)
                UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_RENDER_SETTINGS name=%s flags=%u clip=%.3f source=%s"),
                    *DiagnosticName, BackgroundFlags, Settings ? Settings->ClipDistance : Bank->SceneEnvironment.ClipDistance,
                    Settings ? TEXT("sector") : TEXT("common fallback"));
            if (BackgroundFlags & 1)
                KyaG3D::ReadCameraRelativeBackgroundHierarchies(Data, SectorCameraRelative);
            for (const FKyaG3DDecodedStrip& Strip : SectorStrips)
            {
                FSectionData* Textured = GetTexturedSection(Strip.MaterialHash, SectorTextureData, SectorMaterialOffsets, SectorMaterialSections, TEXT("SectorBackground"), Strip.Primitive);
                if (Textured) ++BackgroundTextured;
                BackgroundVertices += Strip.Vertices.Num();
                BackgroundTriangles += Strip.Indices.Num() / 3;
                if (!bShowBackgroundMeshes) continue;
                TArray<FVector>& TargetPositions = Textured ? Textured->Positions : Positions;
                TArray<int32>& TargetTriangles = Textured ? Textured->Triangles : Triangles;
                TArray<FVector2D>& TargetUVs = Textured ? Textured->UVs : UVs;
                TArray<FColor>& TargetColors = Textured ? Textured->Colors : Colors;
                const int32 Base = TargetPositions.Num();
                FMatrix HierarchyTransform;
                if (!PrepareBackgroundTransform(Data, Strip.HierarchyOffset, SectorCameraRelative,
                    Textured ? Textured->Section : 0, Base, Strip.Vertices.Num(), HierarchyTransform)) { ++BackgroundMissingTransform; continue; }
                for (const FKyaG3DVertex& Vertex : Strip.Vertices)
                {
                    const FVector P = HierarchyTransform.TransformPosition(FVector(Vertex.Position));
                    const FVector Position = FVector(P.X, P.Z, P.Y) * 100.0;
                    TargetPositions.Add(Position);
                    BackgroundBounds += Position;
                }
                for (const FKyaG3DVertexAttributes& Attribute : Strip.Attributes)
                {
                    TargetUVs.Add(FVector2D(Attribute.UV));
                    TargetColors.Add(Attribute.Color);
                }
                AppendUnrealTriangles(TargetTriangles, Strip.Indices, Base);
            }
            if (bLogSectorDiagnostics)
                UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_BACKGROUND name=%s entry=%d parsed=1 strips=%d textured=%d missing_transform=%d materials=%d vertices=%lld triangles=%lld bounds_cm=%s"),
                    *DiagnosticName, EntryIndex, SectorStrips.Num(), BackgroundTextured, BackgroundMissingTransform,
                    SectorMaterialOffsets.Num(), BackgroundVertices, BackgroundTriangles, *BackgroundBounds.ToString());
        }
        if (bLogSectorDiagnostics) UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_END name=%s"), *DiagnosticName);
    }
    if (CheckRebuildCancel())
    {
        RestorePresentation();
        RebuildStatus = TEXT("Cancelled");
        UE_LOG(LogTemp, Display, TEXT("KYA_REBUILD_CANCELLED bank=%s: keeping the previous geometry"), *GetNameSafe(Bank));
        return;
    }
    PreviewTextures = MoveTemp(BuiltPreviewTextures);
    BackgroundFlares = MoveTemp(BuiltBackgroundFlares);
    FlareMaterials = MoveTemp(BuiltFlareMaterials);
    FlarePostProcess->Settings.WeightedBlendables.Array.Reset();
    for (const TObjectPtr<UMaterialInstanceDynamic>& Material : FlareMaterials)
        if (Material) FlarePostProcess->Settings.AddBlendable(Material, 1.0f);
    Mesh->ClearAllMeshSections();
    Mesh->CreateMeshSection(0, Positions, Triangles, TArray<FVector>(), UVs,
        Colors, TArray<FProcMeshTangent>(), false);
    if (GeneratedUntexturedMaterial) Mesh->SetMaterial(0, GeneratedUntexturedMaterial);
    for (const TPair<int32, FSectionData>& Pair : TexturedSections)
    {
        Mesh->CreateMeshSection(Pair.Key, Pair.Value.Positions, Pair.Value.Triangles, TArray<FVector>(),
            Pair.Value.UVs, Pair.Value.Colors, TArray<FProcMeshTangent>(), false);
        // Materials were assigned while the sections were only being described; after
        // the clear/publish they must be applied explicitly.
        if (Pair.Value.Material) Mesh->SetMaterial(Pair.Key, Pair.Value.Material);
    }
    for (const FBackgroundRange& Range : BackgroundRanges)
        if (!BackgroundBasePositions.Contains(Range.Section))
        {
            const FProcMeshSection* Section = Mesh->GetProcMeshSection(Range.Section);
            if (!Section) continue;
            TArray<FVector>& Base = BackgroundBasePositions.Add(Range.Section);
            Base.Reserve(Section->ProcVertexBuffer.Num());
            for (const FProcMeshVertex& Vertex : Section->ProcVertexBuffer) Base.Add(Vertex.Position);
        }
    for (const FClusterVisibilityRange& Range : ClusterVisibilityRanges)
        if (!ClusterBaseIndices.Contains(Range.Section))
            if (const FProcMeshSection* Section = Mesh->GetProcMeshSection(Range.Section))
                ClusterBaseIndices.Add(Range.Section, Section->ProcIndexBuffer);
    if (bLogSectorDiagnostics)
        UE_LOG(LogTemp, Display, TEXT("KYA_BACKGROUND_FOLLOW flags=%lld ranges=%d sections=%d"),
            Bank->SceneEnvironment.BackgroundFlags, BackgroundRanges.Num(), BackgroundBasePositions.Num());
    ++BuildCount;
    RebuildStatus = TEXT("Completed");
#if WITH_EDITOR
    RefreshActorHandles();
    RefreshSpatialHandles();
    RefreshHierarchyHandles();
    ApplyGeneratedActorPresentation();
    RefreshSceneVisualizers();
#endif
}

void AKyaLevelPreview::Destroyed()
{
    // Stop any in-flight decode and wait for every worker before pooling its event.
    if (DecodeCancelToken.IsValid()) DecodeCancelToken->Increment();
    DrainAsyncDecodeEvents(true);
    UnbindBankNotifications();
    for (const TPair<int32, TObjectPtr<AKyaActorHandle>>& Pair : ActorHandles)
        if (IsValid(Pair.Value)) Pair.Value->Destroy();
    ActorHandles.Reset();
    for (const TObjectPtr<AKyaHierarchyHandle>& Handle : HierarchyHandles)
        if (IsValid(Handle)) Handle->Destroy();
    HierarchyHandles.Reset();
    for (const TPair<int32, TObjectPtr<AKyaActorGroup>>& Pair : ActorSectorGroups)
        if (IsValid(Pair.Value)) Pair.Value->Destroy();
    ActorSectorGroups.Reset();
    for (const TPair<FString, TObjectPtr<AKyaSpatialRecordHandle>>& Pair : SpatialHandles)
        if (IsValid(Pair.Value)) Pair.Value->Destroy();
    SpatialHandles.Reset();
    for (const TPair<int32, TObjectPtr<AKyaActorGroup>>& Pair : PathGroups)
        if (IsValid(Pair.Value)) Pair.Value->Destroy();
    PathGroups.Reset();
    for (const TPair<FString, TObjectPtr<AKyaSceneVisualizer>>& Pair : SceneVisualizers)
        if (IsValid(Pair.Value)) Pair.Value->Destroy();
    SceneVisualizers.Reset();
    for (const TPair<int32, TObjectPtr<USplineComponent>>& Pair : PathGuides)
        if (IsValid(Pair.Value)) Pair.Value->DestroyComponent();
    PathGuides.Reset();
    Super::Destroyed();
}

FString AKyaLevelPreview::MakeHierarchyKey(const UKyaLevelBank* InBank, int32 HierarchyOffset) const
{
    return FString::Printf(TEXT("%s:%d"), InBank ? *InBank->GetName() : TEXT("<none>"), HierarchyOffset);
}

void AKyaLevelPreview::SetHierarchyVisible(UKyaLevelBank* InBank, int32 HierarchyOffset, bool bVisible)
{
    const FString Key = MakeHierarchyKey(InBank, HierarchyOffset);
    if (bVisible) HiddenHierarchies.Remove(Key);
    else HiddenHierarchies.Add(Key);
    RebuildFromBank();
}

UKyaPathPointComponent* AKyaLevelPreview::GetPathMarker(int32 MarkerIndex) const
{
    return PathMarkers.IsValidIndex(MarkerIndex) ? PathMarkers[MarkerIndex].Get() : nullptr;
}

AActor* AKyaLevelPreview::GetActorHandle(int32 ActorIndex) const
{
    const TObjectPtr<AKyaActorHandle>* Found = ActorHandles.Find(ActorIndex);
    return Found ? Found->Get() : nullptr;
}

bool AKyaLevelPreview::IsAuthoringVisibilityActive() const
{
    if (!bAuthoringVisibility || bExplicitPreviewCamera) return false;
    const UWorld* World = GetWorld();
    return !World || !World->IsGameWorld();
}

int32 AKyaLevelPreview::ResolveSectorId(const UKyaLevelBank& Sector) const
{
    for (const TPair<int32, TObjectPtr<UKyaLevelBank>>& Pair : LevelSetSectors)
        if (Pair.Value == &Sector) return Pair.Key;
    return GetSectorId(Sector);
}

void AKyaLevelPreview::ApplyLevelSet()
{
    LevelSetSectors.Reset();
    if (!LevelSet) return;
    if (LevelSet->MainBank) Bank = LevelSet->MainBank;
    SectorBanks.Reset();
    for (const FKyaLevelSetSector& Sector : LevelSet->Sectors)
    {
        if (!Sector.Bank) continue;
        SectorBanks.AddUnique(Sector.Bank);
        // The descriptor owns the sector ID mapping: a manually associated bank may
        // not match the SECT<number> in its filename.
        LevelSetSectors.Add(Sector.SectorId, Sector.Bank);
    }
}

void AKyaLevelPreview::RequestCancelRebuild()
{
    bCancelRebuildRequested = true;
}

#if WITH_EDITOR
void AKyaLevelPreview::WaitForAsyncDecode()
{
    if (DecodeCompletionEvent) DecodeCompletionEvent->Wait();
}
#endif

void AKyaLevelPreview::DrainAsyncDecodeEvents(bool bWait)
{
    for (int32 Index = DecodeEventHistory.Num() - 1; Index >= 0; --Index)
    {
        FEvent* Event = DecodeEventHistory[Index];
        if (!Event)
        {
            DecodeEventHistory.RemoveAt(Index);
            continue;
        }
        // Only recycle an event once its worker finished triggering it; waiting on a
        // live event would block the editor (and the raw event must not be pooled while
        // a worker may still touch it).
        if (!bWait && !Event->Wait(0)) continue;
        if (bWait) Event->Wait();
        FPlatformProcess::ReturnSynchEventToPool(Event);
        DecodeEventHistory.RemoveAt(Index);
    }
    if (DecodeEventHistory.IsEmpty()) DecodeCompletionEvent = nullptr;
}

void AKyaLevelPreview::StartAsyncActorDecode()
{
#if WITH_EDITOR
    if (!bAsyncDecodeMeshes || !Bank) return;
    {
        FScopeLock Lock(&GActorDecodeLock);
        if (const TSharedPtr<FKyaActorDecodeCache>* Found = GActorDecodeCache.Find(Bank))
            if (Found->IsValid() && (*Found)->Generation == Bank->EditGeneration) return;
    }
    struct FRequest
    {
        const UKyaLevelBank* Bank = nullptr;
        int32 Generation = 0;
        int32 Serial = 0;
        TMap<int32, TArray<uint8>> MeshData;
    };
    TSharedPtr<FRequest, ESPMode::ThreadSafe> Request = MakeShared<FRequest, ESPMode::ThreadSafe>();
    Request->Bank = Bank;
    Request->Generation = Bank->EditGeneration;
    // A bank-global request id is comparable across every preview of the same bank;
    // per-preview serials could never supersede each other's cache entries.
    Request->Serial = GActorDecodeRequestId.Increment();
    int32 MeshOrdinal = 0;
    for (const FKyaBankEntry& Entry : Bank->Entries)
    {
        if (Entry.Type != 4 || Entry.Subtype != 1) continue;
        const int32 Ordinal = MeshOrdinal++;
        if (Entry.Offset < 0 || Entry.Size <= 0 || Entry.Offset > Bank->BankBytes.Num() ||
            Entry.Size > Bank->BankBytes.Num() - Entry.Offset) continue;
        Request->MeshData.Add(Ordinal,
            TArray<uint8>(Bank->BankBytes.GetData() + Entry.Offset, int32(Entry.Size)));
    }
    // Cancel any older worker cooperatively instead of blocking the editor on it.
    if (DecodeCancelToken.IsValid()) DecodeCancelToken->Increment();
    TSharedPtr<FThreadSafeCounter> CancelToken = MakeShared<FThreadSafeCounter>();
    DecodeCancelToken = CancelToken;
    // Recycle only completed events; a running worker may still trigger its event.
    DrainAsyncDecodeEvents(false);
    FEvent* Completion = FPlatformProcess::GetSynchEventFromPool(true);
    DecodeEventHistory.Add(Completion);
    DecodeCompletionEvent = Completion;
    Async(EAsyncExecution::ThreadPool, [Request, Completion, CancelToken]()
    {
        TSharedPtr<FKyaActorDecodeCache> Result = MakeShared<FKyaActorDecodeCache>();
        Result->Bank = const_cast<UKyaLevelBank*>(Request->Bank);
        Result->Generation = Request->Generation;
        Result->Serial = Request->Serial;
        for (const TPair<int32, TArray<uint8>>& Pair : Request->MeshData)
        {
            if (CancelToken->GetValue() != 0) break;
            TArray<FKyaG3DDecodedStrip> Strips;
            if (KyaG3D::DecodeHierarchyStrips(MakeArrayView(Pair.Value.GetData(), Pair.Value.Num()), Strips))
                Result->StripsByMesh.Add(Pair.Key, MoveTemp(Strips));
        }
        if (CancelToken->GetValue() == 0)
        {
            FScopeLock Lock(&GActorDecodeLock);
            TSharedPtr<FKyaActorDecodeCache>& Slot = GActorDecodeCache.FindOrAdd(Request->Bank);
            // Publish only when this request is newer than the cached one; an obsolete
            // completion (newer request already stored) is discarded.
            if (!Slot.IsValid() || Slot->Serial < Result->Serial) Slot = Result;
        }
        Completion->Trigger();
    });
    AsyncDecodeCompletions.Increment();
#endif
}

UKyaLevelSettings* AKyaLevelPreview::GetOrCreateLevelSettings(){
    const bool bBankChanged = LevelSettings && LevelSettings->Bank != Bank;
    if (!LevelSettings)
    {
        LevelSettings = NewObject<UKyaLevelSettings>(this, NAME_None, RF_Transient);
        LevelSettings->Bank = Bank;
        LevelSettings->LoadFromBank();
    }
    else if (bBankChanged)
    {
        LevelSettings->Bank = Bank;
        LevelSettings->LoadFromBank();
    }
    return LevelSettings;
}

bool AKyaLevelPreview::ShouldActorHandlesRenderGeometry() const
{
    return bSpawnActorHandles && bShowActorMeshesOnHandles && !bShowAllActorMeshes && !bShowActorMesh;
}

bool AKyaLevelPreview::ShouldShowActorIcon() const
{
    return bSpawnActorHandles && bShowActorIcons && !bShowAllActorMeshes && !bShowActorMesh;
}

const TArray<FKyaActorMeshSection>* AKyaLevelPreview::FindActorMeshSections(int32 ActorIndex) const
{
    return ActorMeshSections.Find(ActorIndex);
}

void AKyaLevelPreview::HandleBankEdited(UKyaLevelBank* InBank)
{
    if (!InBank || (InBank != Bank && !SectorBanks.Contains(InBank))) return;
    RebuildFromBank();
}

void AKyaLevelPreview::HandleActorEdited(UKyaLevelBank* InBank, int32 ActorIndex, bool bDeferred)
{
#if WITH_EDITOR
    // Handles are index-keyed by the main bank; sector-bank actor notifications must
    // never rebind a main-bank handle.
    if (!InBank || InBank != Bank) return;
    // Shared diagnostic meshes bake actor transforms; they need the full capture path.
    if (!ShouldActorHandlesRenderGeometry())
    {
        if (bDeferred) FKyaLevelEditService::Get().NotifyBankChangedDeferred(InBank);
        else FKyaLevelEditService::Get().NotifyBankChanged(InBank, true);
        return;
    }
    const FKyaActorPlacement* Placement = InBank->SceneEnvironment.Actors.IsValidIndex(ActorIndex)
        ? &InBank->SceneEnvironment.Actors[ActorIndex] : nullptr;
    if (!Placement) return;
    // Ground-light sampling is position-dependent; a pure translate changes baked
    // vertex colours, so defer a capture rebuild until all selected commits finish.
    const bool bGroundLighting = bApplyActorLighting && (Placement->LightingFlags & 2) == 0 && bHasGroundLightData;
    const FVector* CapturedPosition = ActorCapturedPositions.Find(ActorIndex);
    if (bGroundLighting && CapturedPosition && !CapturedPosition->Equals(Placement->Position, 1.0e-4))
    {
        ActorSectionRevisions.Remove(ActorIndex);
        ActorCapturedPositions.Remove(ActorIndex);
        if (bDeferred) FKyaLevelEditService::Get().NotifyBankChangedDeferred(InBank);
        else FKyaLevelEditService::Get().NotifyBankChanged(InBank, true);
        return;
    }
    AKyaActorHandle* Handle = Cast<AKyaActorHandle>(GetActorHandle(ActorIndex));
    if (!Handle)
    {
        // The actor is outside the loaded sectors or handles were toggled; the cheap
        // handle refresh recreates/disambiguates labels.
        RefreshActorHandles();
        ApplyGeneratedActorPresentation();
        RefreshSceneVisualizers();
        return;
    }
    const int32 PreviousSector = Handle->ActorSectorId;
    Handle->Bank = InBank;
    Handle->Preview = this;
    Handle->ActorIndex = ActorIndex;
    Handle->SyncFromBank();
    if (Handle->ActorSectorId != PreviousSector)
    {
        // Sector grouping changed; reattach through the full handle pass.
        RefreshActorHandles();
    }
    else
    {
        Handle->SetGeneratedLabel(GetActorDisplayLabel(ActorIndex));
        Handle->SetIconEnabled(ShouldShowActorIcon());
    }
    ApplyGeneratedActorPresentation();
    RefreshSceneVisualizers();
#endif
}

void AKyaLevelPreview::HandleWaypointEdited(UKyaLevelBank* InBank, int32 WaypointIndex)
{
#if WITH_EDITOR
    if (!InBank || InBank != Bank) return;
    if (AKyaSpatialRecordHandle* Handle = Cast<AKyaSpatialRecordHandle>(
        GetWaypointHandle(WaypointIndex)))
    {
        Handle->Bank = InBank;
        Handle->Preview = this;
        Handle->SyncFromBank();
    }
    RefreshSceneVisualizers();
#endif
}

void AKyaLevelPreview::HandlePathPointEdited(UKyaLevelBank* InBank, int32 PathIndex, int32 PointIndex)
{
#if WITH_EDITOR
    if (!InBank || InBank != Bank) return;
    if (AKyaSpatialRecordHandle* Handle = Cast<AKyaSpatialRecordHandle>(
        GetPathPointHandle(PathIndex, PointIndex)))
    {
        Handle->Bank = InBank;
        Handle->Preview = this;
        Handle->SyncFromBank();
    }
    RefreshSceneVisualizers();
#endif
}

void AKyaLevelPreview::HandleLightEdited(UKyaLevelBank* InBank, bool bDeferred)
{
#if WITH_EDITOR
    if (!InBank) return;
    // Light data is baked into captured vertex colours; invalidate the actor capture
    // cache and coalesce the rebuilt capture for deferred commits.
    ActorSectionRevisions.Reset();
    ActorCapturedPositions.Reset();
    if (bDeferred) FKyaLevelEditService::Get().NotifyBankChangedDeferred(InBank);
    else FKyaLevelEditService::Get().NotifyBankChanged(InBank, true);
#endif
}

void AKyaLevelPreview::HandleSceneCameraEdited(UKyaLevelBank* InBank)
{
#if WITH_EDITOR
    if (!InBank) return;
    RefreshSceneVisualizers();
#endif
}

void AKyaLevelPreview::BindBankNotifications()
{
#if WITH_EDITOR
    if (!BankChangedHandle.IsValid())
        BankChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(
            this, &AKyaLevelPreview::OnMonitoredObjectChanged);
    if (!BankTransactedHandle.IsValid())
        BankTransactedHandle = FCoreUObjectDelegates::OnObjectTransacted.AddUObject(
            this, &AKyaLevelPreview::OnMonitoredObjectTransacted);

    // Register every bank this preview displays, including sector banks, so undo/redo
    // and asset-editor changes refresh the geometry they affect.
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    TSet<TWeakObjectPtr<UKyaLevelBank>> Wanted;
    if (Bank) Wanted.Add(Bank);
    for (const TObjectPtr<UKyaLevelBank>& Sector : SectorBanks)
        if (Sector) Wanted.Add(Sector);
    for (auto It = BoundBanks.CreateIterator(); It; ++It)
    {
        if (Wanted.Contains(*It)) continue;
        if (It->IsValid()) Service.UnregisterView(It->Get(), this);
        It.RemoveCurrent();
    }
    for (const TWeakObjectPtr<UKyaLevelBank>& Entry : Wanted)
    {
        if (!Entry.IsValid()) continue;
        Service.RegisterView(Entry.Get(), this);
        BoundBanks.Add(Entry);
    }
#endif
}

void AKyaLevelPreview::UnbindBankNotifications()
{
#if WITH_EDITOR
    if (BankChangedHandle.IsValid())
    {
        FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(BankChangedHandle);
        BankChangedHandle.Reset();
    }
    if (BankTransactedHandle.IsValid())
    {
        FCoreUObjectDelegates::OnObjectTransacted.Remove(BankTransactedHandle);
        BankTransactedHandle.Reset();
    }
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.UnregisterView(this);
    for (const TWeakObjectPtr<UKyaLevelBank>& Entry : BoundBanks)
        if (Entry.IsValid()) Service.UnregisterView(Entry.Get(), this);
    BoundBanks.Reset();
#endif
}

void AKyaLevelPreview::OnMonitoredObjectChanged(UObject* Object, FPropertyChangedEvent& Event)
{
    if (bIsRebuildingPreview) return;
    UKyaLevelBank* ChangedBank = Cast<UKyaLevelBank>(Object);
    if (ChangedBank && (ChangedBank == Bank || SectorBanks.Contains(ChangedBank))) RebuildFromBank();
}

void AKyaLevelPreview::OnMonitoredObjectTransacted(UObject* Object, const FTransactionObjectEvent& Event)
{
#if WITH_EDITOR
    if (bIsRebuildingPreview) return;
    UKyaLevelBank* ChangedBank = Cast<UKyaLevelBank>(Object);
    if (!ChangedBank || (ChangedBank != Bank && !SectorBanks.Contains(ChangedBank))) return;
    // A targeted record edit already synchronized the affected displays; its own
    // transaction event must not trigger a full rebuild. Undo/redo later has no
    // pending suppression and still rebuilds.
    if (FKyaLevelEditService::Get().ConsumeTargetedTransaction(ChangedBank)) return;
    // Coalesce transaction notifications (including undo) into one refresh at the end
    // of the frame, so a multi-actor operation does not rebuild the whole preview once
    // per record.
    FKyaLevelEditService::Get().NotifyBankChangedDeferred(ChangedBank);
#endif
}

void AKyaLevelPreview::RebuildFromBank()
{
#if WITH_EDITOR
    if (bIsRebuildingPreview) return;
    TGuardValue<bool> Guard(bIsRebuildingPreview, true);
    OnConstruction(GetActorTransform());
#endif
}

#if WITH_EDITOR
void AKyaLevelPreview::RefreshActorHandles()
{
    UWorld* World = GetWorld();
    if (!bSpawnActorHandles || !Bank || !World || World->IsGameWorld())
    {
        for (const TPair<int32, TObjectPtr<AKyaActorHandle>>& Pair : ActorHandles)
            if (IsValid(Pair.Value)) Pair.Value->Destroy();
        ActorHandles.Reset();
        for (const TPair<int32, TObjectPtr<AKyaActorGroup>>& Pair : ActorSectorGroups)
            if (IsValid(Pair.Value)) Pair.Value->Destroy();
        ActorSectorGroups.Reset();
        return;
    }

    // Display labels are derived from the whole bank, not from the currently spawned
    // handles, so loading or unloading a sector cannot relabel a record. Serialized
    // names are never modified; duplicate names are disambiguated with the index.
    AuthoringOnlyHandleCount = 0;
    const bool bAuthoringUnloadedSectors = IsAuthoringVisibilityActive() && bAuthoringShowUnloadedSectors;
    TArray<FString> BaseNames;
    TArray<FString> Labels;
    TSet<FString> DuplicateNames;
    BaseNames.Reserve(Bank->SceneEnvironment.Actors.Num());
    Labels.Reserve(Bank->SceneEnvironment.Actors.Num());
    for (int32 Index = 0; Index < Bank->SceneEnvironment.Actors.Num(); ++Index)
    {
        const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[Index];
        const FString Base = Placement.Name.IsEmpty()
            ? FString::Printf(TEXT("Actor %d"), Index) : Placement.Name;
        if (BaseNames.Contains(Base)) DuplicateNames.Add(Base);
        BaseNames.Add(Base);
        Labels.Add(FString::Printf(TEXT("%s [class %lld]"), *Base, Placement.ClassId));
    }
    for (int32 Index = 0; Index < Labels.Num(); ++Index)
        if (DuplicateNames.Contains(BaseNames[Index]))
            Labels[Index] = FString::Printf(TEXT("%s [%d]"), *Labels[Index], Index);

    TSet<int32> LoadedSectors;
    for (const UKyaLevelBank* Sector : SectorBanks)
        if (Sector && Sector != Bank) LoadedSectors.Add(ResolveSectorId(*Sector));

    // Sector grouping actors keep the Outliner organized while the handles stay
    // attached (and therefore follow the preview root transform).
    const auto FindOrCreateSectorGroup = [this, World](int32 SectorId) -> AKyaActorGroup*
    {
        const FString ExpectedLabel = SectorId <= 0
            ? FString::Printf(TEXT("%s global actors"), *GetNameSafe(Bank))
            : FString::Printf(TEXT("%s sector %d"), *GetNameSafe(Bank), SectorId);
        if (TObjectPtr<AKyaActorGroup>* Found = ActorSectorGroups.Find(SectorId))
        {
            if (IsValid(*Found))
            {
                AKyaActorGroup* Existing = *Found;
                // The preview can be reassigned to another bank; keep the label and the
                // level-root transform authoritative on every refresh.
                if (Existing->GetActorLabel() != ExpectedLabel) Existing->SetGeneratedLabel(ExpectedLabel);
                if (!Existing->GetRootComponent()->GetRelativeTransform().Equals(FTransform::Identity))
                    Existing->SetActorRelativeTransform(FTransform::Identity, false, nullptr, ETeleportType::TeleportPhysics);
                return Existing;
            }
        }
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.ObjectFlags |= RF_Transient | RF_Transactional;
        AKyaActorGroup* Group = World->SpawnActor<AKyaActorGroup>(
            AKyaActorGroup::StaticClass(), FTransform::Identity, Params);
        if (!Group) return nullptr;
        Group->SetFlags(RF_Transient | RF_Transactional);
        Group->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
        Group->SetGeneratedLabel(ExpectedLabel);
        ActorSectorGroups.Add(SectorId, Group);
        return Group;
    };

    TSet<int32> Wanted;
    TSet<int32> WantedSectors;
    const bool bShowIcon = ShouldShowActorIcon();
    for (int32 Index = 0; Index < Bank->SceneEnvironment.Actors.Num(); ++Index)
    {
        const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[Index];
        // Global actors (sector -1) always belong to the scene; sector actors only
        // get a handle when their sector is loaded. Without sector banks, show all.
        // Authoring discovery can keep unloaded-sector actors selectable in the editor.
        const bool bSectorLoaded = Placement.SectorId < 0 || LoadedSectors.IsEmpty() ||
            LoadedSectors.Contains(Placement.SectorId);
        if (!bSectorLoaded)
        {
            if (!bAuthoringUnloadedSectors) continue;
            ++AuthoringOnlyHandleCount;
        }
        Wanted.Add(Index);
        const int32 GroupSector = Placement.SectorId < 0 ? 0 : Placement.SectorId;
        WantedSectors.Add(GroupSector);
        AKyaActorGroup* Group = FindOrCreateSectorGroup(GroupSector);
        const TArray<FKyaActorMeshSection>* Sections = ActorMeshSections.Find(Index);
        TObjectPtr<AKyaActorHandle>* Existing = ActorHandles.Find(Index);
        if (Existing && IsValid(*Existing))
        {
            AKyaActorHandle* Handle = *Existing;
            Handle->Bank = Bank;
            Handle->Preview = this;
            Handle->ActorIndex = Index;
            if (Group && Handle->GetAttachParentActor() != Group)
                Handle->AttachToActor(Group, FAttachmentTransformRules::KeepRelativeTransform);
            Handle->SyncFromBank();
            Handle->SetIconEnabled(bShowIcon);
            if (Sections) Handle->SetActorMeshSections(*Sections);
            else Handle->SetActorMeshSections(TArray<FKyaActorMeshSection>());
            if (Handle->GetActorLabel() != Labels[Index]) Handle->SetGeneratedLabel(Labels[Index]);
            continue;
        }
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        // Generated representations must never become part of a saved map. Explicit
        // transient flags keep them out regardless of the handle's class defaults.
        Params.ObjectFlags |= RF_Transient | RF_Transactional;
        AKyaActorHandle* Handle = World->SpawnActor<AKyaActorHandle>(
            AKyaActorHandle::StaticClass(), FTransform::Identity, Params);
        if (!Handle)
        {
            UE_LOG(LogTemp, Warning, TEXT("KYA_ACTOR_HANDLE_SPAWN_FAILED index=%d"), Index);
            continue;
        }
        Handle->SetFlags(RF_Transient | RF_Transactional);
        if (Group) Handle->AttachToActor(Group, FAttachmentTransformRules::KeepRelativeTransform);
        Handle->Initialize(this, Bank, Index, bShowIcon);
        Handle->SetGeneratedLabel(Labels[Index]);
        if (Sections) Handle->SetActorMeshSections(*Sections);
        ActorHandles.Add(Index, Handle);
    }

    for (auto It = ActorHandles.CreateIterator(); It; ++It)
    {
        if (Wanted.Contains(It.Key())) continue;
        if (IsValid(It.Value())) It.Value()->Destroy();
        It.RemoveCurrent();
    }

    for (auto It = ActorSectorGroups.CreateIterator(); It; ++It)
    {
        if (WantedSectors.Contains(It.Key())) continue;
        if (IsValid(It.Value())) It.Value()->Destroy();
        It.RemoveCurrent();
    }

    if (bSpawnActorHandles)
        UE_LOG(LogTemp, Display, TEXT("KYA_ACTOR_HANDLES wanted=%d handles=%d groups=%d loaded_sectors=%d bank=%s world_game=%d"),
            Wanted.Num(), ActorHandles.Num(), ActorSectorGroups.Num(), LoadedSectors.Num(), *GetNameSafe(Bank),
            World->IsGameWorld() ? 1 : 0);
}

AActor* AKyaLevelPreview::GetWaypointHandle(int32 WaypointIndex) const
{
    const TObjectPtr<AKyaSpatialRecordHandle>* Found =
        SpatialHandles.Find(FString::Printf(TEXT("W:%d"), WaypointIndex));
    return Found ? Found->Get() : nullptr;
}

AActor* AKyaLevelPreview::GetPathPointHandle(int32 PathIndex, int32 PointIndex) const
{
    const TObjectPtr<AKyaSpatialRecordHandle>* Found =
        SpatialHandles.Find(FString::Printf(TEXT("P:%d:%d"), PathIndex, PointIndex));
    return Found ? Found->Get() : nullptr;
}

USplineComponent* AKyaLevelPreview::GetPathGuide(int32 PathIndex) const
{
    const TObjectPtr<USplineComponent>* Found = PathGuides.Find(PathIndex);
    return Found ? Found->Get() : nullptr;
}

AActor* AKyaLevelPreview::GetPathGroup(int32 PathIndex) const
{
    const TObjectPtr<AKyaActorGroup>* Found = PathGroups.Find(PathIndex);
    return Found ? Found->Get() : nullptr;
}

bool AKyaLevelPreview::OwnsGeneratedActor(const AActor* Actor) const
{
    if (!Actor) return false;
    if (const AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Actor))
    {
        const TObjectPtr<AKyaActorHandle>* Found = ActorHandles.Find(Handle->ActorIndex);
        return Found && Found->Get() == Actor;
    }
    if (const AKyaSpatialRecordHandle* Spatial = Cast<AKyaSpatialRecordHandle>(Actor))
    {
        const FString Key = Spatial->Kind == EKyaSpatialRecordKind::Waypoint
            ? FString::Printf(TEXT("W:%d"), Spatial->RecordIndex)
            : FString::Printf(TEXT("P:%d:%d"), Spatial->RecordIndex, Spatial->PointIndex);
        const TObjectPtr<AKyaSpatialRecordHandle>* Found = SpatialHandles.Find(Key);
        return Found && Found->Get() == Actor;
    }
    for (const TPair<int32, TObjectPtr<AKyaActorGroup>>& Pair : ActorSectorGroups)
        if (Pair.Value == Actor) return true;
    for (const TPair<int32, TObjectPtr<AKyaActorGroup>>& Pair : PathGroups)
        if (Pair.Value == Actor) return true;
    for (const TPair<FString, TObjectPtr<AKyaSceneVisualizer>>& Pair : SceneVisualizers)
        if (Pair.Value == Actor) return true;
    for (const TObjectPtr<AKyaHierarchyHandle>& Handle : HierarchyHandles)
        if (Handle == Actor) return true;
    return false;
}

int32 AKyaLevelPreview::DestroyStrayGeneratedActors()
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    TArray<AActor*> Strays;
    for (TActorIterator<AKyaActorHandle> It(World); It; ++It)
        if (It->Preview == this && !OwnsGeneratedActor(*It)) Strays.Add(*It);
    for (TActorIterator<AKyaSpatialRecordHandle> It(World); It; ++It)
        if (It->Preview == this && !OwnsGeneratedActor(*It)) Strays.Add(*It);
    for (TActorIterator<AKyaHierarchyHandle> It(World); It; ++It)
        if (It->Preview == this && !OwnsGeneratedActor(*It)) Strays.Add(*It);
    for (TActorIterator<AKyaSceneVisualizer> It(World); It; ++It)
        if (It->Preview == this && !OwnsGeneratedActor(*It)) Strays.Add(*It);
    for (TActorIterator<AKyaActorGroup> It(World); It; ++It)
        if (It->GetAttachParentActor() == this && !OwnsGeneratedActor(*It)) Strays.Add(*It);
    for (AActor* Stray : Strays)
    {
        UE_LOG(LogTemp, Display, TEXT("KYA_DESTROY_STRAY_GENERATED preview=%s actor=%s"),
            *GetNameSafe(this), *GetNameSafe(Stray));
        Stray->Destroy();
    }
    return Strays.Num();
}

FString AKyaLevelPreview::GetActorDisplayLabel(int32 ActorIndex) const
{
    if (!Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex)) return FString();
    const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[ActorIndex];
    const FString Base = Placement.Name.IsEmpty()
        ? FString::Printf(TEXT("Actor %d"), ActorIndex) : Placement.Name;
    // Duplicate names are disambiguated from the whole bank, exactly like the
    // refresh path, so a rename revert always restores the correct label.
    int32 Occurrences = 0;
    for (const FKyaActorPlacement& Other : Bank->SceneEnvironment.Actors)
    {
        const FString OtherBase = Other.Name.IsEmpty() ? FString() : Other.Name;
        if (OtherBase == Base) ++Occurrences;
    }
    FString Label = FString::Printf(TEXT("%s [class %lld]"), *Base, Placement.ClassId);
    if (Occurrences > 1) Label += FString::Printf(TEXT(" [%d]"), ActorIndex);
    return Label;
}

void AKyaLevelPreview::SetGeneratedActorTemporarilyHidden(AActor* Actor, bool bInHidden)
{
    if (!OwnsGeneratedActor(Actor)) return;
    if (bInHidden) TemporarilyHiddenGeneratedActors.Add(Actor);
    else TemporarilyHiddenGeneratedActors.Remove(Actor);
    ApplyGeneratedActorPresentation();
}

bool AKyaLevelPreview::IsGeneratedActorTemporarilyHidden(const AActor* Actor) const
{
    for (const AActor* Current = Actor; Current; Current = Current->GetAttachParentActor())
        if (TemporarilyHiddenGeneratedActors.Contains(Current)) return true;
    return false;
}

void AKyaLevelPreview::IsolateGeneratedActors(const TArray<AActor*>& Actors)
{
    IsolatedGeneratedActors.Reset();
    for (AActor* Actor : Actors)
        if (OwnsGeneratedActor(Actor)) IsolatedGeneratedActors.Add(Actor);
    bIsolationActive = !IsolatedGeneratedActors.IsEmpty();
    ApplyGeneratedActorPresentation();
}

void AKyaLevelPreview::ClearGeneratedActorIsolation()
{
    bIsolationActive = false;
    IsolatedGeneratedActors.Reset();
    ApplyGeneratedActorPresentation();
}

bool AKyaLevelPreview::IsGeneratedActorIsolated(const AActor* Actor) const
{
    if (!bIsolationActive) return false;
    for (const AActor* Current = Actor; Current; Current = Current->GetAttachParentActor())
        if (IsolatedGeneratedActors.Contains(Current)) return true;
    return false;
}

void AKyaLevelPreview::ApplyGeneratedActorPresentation()
{
    // Editor-only: SetIsTemporarilyHiddenInEditor never changes game visibility, and
    // the bank data is untouched. Isolation hides every representation except the
    // isolated set; explicit hiding applies when isolation is not active. Membership
    // is inherited through the sector grouping actors, so hiding a group hides its
    // handles and isolating a group keeps its handles visible.
    const auto ContainsOrAncestor = [](const TSet<TWeakObjectPtr<AActor>>& Set, const AActor* Actor)
    {
        for (const AActor* Current = Actor; Current; Current = Current->GetAttachParentActor())
            if (Set.Contains(Current)) return true;
        return false;
    };
    const auto Apply = [this, &ContainsOrAncestor](AActor* Actor)
    {
        if (!IsValid(Actor)) return;
        const bool bShouldHide = bIsolationActive
            ? !ContainsOrAncestor(IsolatedGeneratedActors, Actor)
            : ContainsOrAncestor(TemporarilyHiddenGeneratedActors, Actor);
        if (Actor->IsTemporarilyHiddenInEditor() != bShouldHide) Actor->SetIsTemporarilyHiddenInEditor(bShouldHide);
    };
    for (const TPair<int32, TObjectPtr<AKyaActorHandle>>& Pair : ActorHandles) Apply(Pair.Value);
    for (const TPair<FString, TObjectPtr<AKyaSpatialRecordHandle>>& Pair : SpatialHandles) Apply(Pair.Value);
    for (const TObjectPtr<AKyaHierarchyHandle>& Handle : HierarchyHandles) Apply(Handle);
    for (const TPair<int32, TObjectPtr<AKyaActorGroup>>& Pair : ActorSectorGroups) Apply(Pair.Value);
    for (const TPair<int32, TObjectPtr<AKyaActorGroup>>& Pair : PathGroups) Apply(Pair.Value);
    for (const TPair<FString, TObjectPtr<AKyaSceneVisualizer>>& Pair : SceneVisualizers) Apply(Pair.Value);
}

void AKyaLevelPreview::RefreshSpatialHandles()
{
    UWorld* World = GetWorld();
    const bool bWantPathHandles = bSpawnPathPointHandles || bShowAllPaths;
    if ((!bSpawnWaypointHandles && !bWantPathHandles) || !Bank || !World || World->IsGameWorld())
    {
        for (const TPair<FString, TObjectPtr<AKyaSpatialRecordHandle>>& Pair : SpatialHandles)
            if (IsValid(Pair.Value)) Pair.Value->Destroy();
        SpatialHandles.Reset();
        for (const TPair<int32, TObjectPtr<AKyaActorGroup>>& Pair : PathGroups)
            if (IsValid(Pair.Value)) Pair.Value->Destroy();
        PathGroups.Reset();
        return;
    }

    // One Outliner group per path makes point ownership explicit without relying on a
    // numeric preview index.
    const auto FindOrCreatePathGroup = [this, World](int32 PathIndex) -> AKyaActorGroup*
    {
        const FString Label = FString::Printf(TEXT("%s path %d"), *GetNameSafe(Bank), PathIndex);
        if (TObjectPtr<AKyaActorGroup>* Found = PathGroups.Find(PathIndex))
        {
            if (IsValid(*Found))
            {
                AKyaActorGroup* Existing = *Found;
                if (Existing->GetActorLabel() != Label) Existing->SetGeneratedLabel(Label);
                return Existing;
            }
        }
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.ObjectFlags |= RF_Transient | RF_Transactional;
        AKyaActorGroup* Group = World->SpawnActor<AKyaActorGroup>(
            AKyaActorGroup::StaticClass(), FTransform::Identity, Params);
        if (!Group) return nullptr;
        Group->SetFlags(RF_Transient | RF_Transactional);
        Group->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
        Group->SetGeneratedLabel(Label);
        PathGroups.Add(PathIndex, Group);
        return Group;
    };

    TSet<FString> Wanted;
    TSet<int32> WantedPathGroups;
    const auto SpawnOrUpdate = [this, World, &Wanted](const FString& Key, EKyaSpatialRecordKind Kind,
        int32 Index, int32 PointIndex, AKyaActorGroup* Group)
    {
        Wanted.Add(Key);
        if (TObjectPtr<AKyaSpatialRecordHandle>* Existing = SpatialHandles.Find(Key))
        {
            if (IsValid(*Existing))
            {
                AKyaSpatialRecordHandle* Handle = *Existing;
                Handle->Bank = Bank;
                Handle->Preview = this;
                Handle->Kind = Kind;
                Handle->RecordIndex = Index;
                Handle->PointIndex = PointIndex;
                if (Group && Handle->GetAttachParentActor() != Group)
                    Handle->AttachToActor(Group, FAttachmentTransformRules::KeepRelativeTransform);
                Handle->SyncFromBank();
                return;
            }
        }
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.ObjectFlags |= RF_Transient | RF_Transactional;
        AKyaSpatialRecordHandle* Handle = World->SpawnActor<AKyaSpatialRecordHandle>(
            AKyaSpatialRecordHandle::StaticClass(), FTransform::Identity, Params);
        if (!Handle) return;
        Handle->SetFlags(RF_Transient | RF_Transactional);
        AActor* AttachParent = Group ? static_cast<AActor*>(Group) : static_cast<AActor*>(this);
        Handle->AttachToActor(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);
        Handle->Initialize(this, Bank, Kind, Index, PointIndex);
        SpatialHandles.Add(Key, Handle);
    };

    if (bSpawnWaypointHandles)
        for (int32 Index = 0; Index < Bank->SceneEnvironment.Waypoints.Num(); ++Index)
            SpawnOrUpdate(FString::Printf(TEXT("W:%d"), Index), EKyaSpatialRecordKind::Waypoint,
                Index, INDEX_NONE, nullptr);
    if (bWantPathHandles)
        for (int32 PathIndex = 0; PathIndex < Bank->SceneEnvironment.Paths.Num(); ++PathIndex)
        {
            const FKyaPath& Path = Bank->SceneEnvironment.Paths[PathIndex];
            if (Path.Points.IsEmpty()) continue;
            AKyaActorGroup* Group = FindOrCreatePathGroup(PathIndex);
            if (Group) WantedPathGroups.Add(PathIndex);
            for (int32 PointIndex = 0; PointIndex < Path.Points.Num(); ++PointIndex)
                SpawnOrUpdate(FString::Printf(TEXT("P:%d:%d"), PathIndex, PointIndex),
                    EKyaSpatialRecordKind::PathPoint, PathIndex, PointIndex, Group);
        }

    for (auto It = SpatialHandles.CreateIterator(); It; ++It)
    {
        if (Wanted.Contains(It.Key())) continue;
        if (IsValid(It.Value())) It.Value()->Destroy();
        It.RemoveCurrent();
    }
    for (auto It = PathGroups.CreateIterator(); It; ++It)
    {
        if (WantedPathGroups.Contains(It.Key())) continue;
        if (IsValid(It.Value())) It.Value()->Destroy();
        It.RemoveCurrent();
    }
    UE_LOG(LogTemp, Display, TEXT("KYA_SPATIAL_HANDLES waypoints=%d paths=%d handles=%d guides=%d bank=%s"),
        bSpawnWaypointHandles ? 1 : 0, bWantPathHandles ? 1 : 0, SpatialHandles.Num(), PathGroups.Num(),
        *GetNameSafe(Bank));
}

void AKyaLevelPreview::RefreshHierarchyHandles(){
    UWorld* World = GetWorld();
    if (!bSpawnHierarchyHandles || !World || World->IsGameWorld())
    {
        for (const TObjectPtr<AKyaHierarchyHandle>& Handle : HierarchyHandles)
            if (IsValid(Handle)) Handle->Destroy();
        HierarchyHandles.Reset();
        return;
    }

    const FTransform ActorTransform = GetActorTransform();
    for (const TPair<FString, FHierarchySelectionInfo>& Pair : HierarchySelectionInfo)
    {
        const FHierarchySelectionInfo& Info = Pair.Value;
        if (!Info.Bank) continue;
        const FVector LocalOrigin = FVector(Info.Origin.X, Info.Origin.Z, Info.Origin.Y) * 100.0;
        const FBox WorldBounds = Info.Bounds.IsValid
            ? Info.Bounds.TransformBy(ActorTransform.ToMatrixWithScale()) : FBox(ForceInit);
        AKyaHierarchyHandle* Existing = nullptr;
        for (const TObjectPtr<AKyaHierarchyHandle>& Candidate : HierarchyHandles)
        {
            if (IsValid(Candidate) && Candidate->Bank == Info.Bank && Candidate->HierarchyOffset == Info.Offset)
            {
                Existing = Candidate;
                break;
            }
        }
        if (Existing)
        {
            Existing->Preview = this;
            Existing->SectorId = Info.SectorId;
            Existing->SourceOrigin = Info.Origin;
            Existing->StripCount = Info.Strips;
            Existing->TriangleCount = Info.Triangles;
            Existing->bVisible = !HiddenHierarchies.Contains(Pair.Key);
            Existing->SetHierarchyGeometry(Info.Sections, WorldBounds, Info.Materials, Info.MaterialHashes);
            continue;
        }
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.ObjectFlags |= RF_Transient | RF_Transactional;
        AKyaHierarchyHandle* Handle = World->SpawnActor<AKyaHierarchyHandle>(
            AKyaHierarchyHandle::StaticClass(), FTransform::Identity, Params);
        if (!Handle) continue;
        Handle->SetFlags(RF_Transient | RF_Transactional);
        Handle->Initialize(this, Info.Bank, Info.SectorId, Info.Offset, LocalOrigin, Info.Origin,
            Info.Strips, Info.Triangles, !HiddenHierarchies.Contains(Pair.Key));
        Handle->SetHierarchyGeometry(Info.Sections, WorldBounds, Info.Materials, Info.MaterialHashes);
        HierarchyHandles.Add(Handle);
    }

    for (auto It = HierarchyHandles.CreateIterator(); It; ++It)
    {
        AKyaHierarchyHandle* Handle = *It;
        if (!IsValid(Handle) || !HierarchySelectionInfo.Contains(MakeHierarchyKey(Handle->Bank, Handle->HierarchyOffset)))
        {
            if (IsValid(Handle)) Handle->Destroy();
            It.RemoveCurrent();
        }
    }
    UE_LOG(LogTemp, Display, TEXT("KYA_HIERARCHY_HANDLES hierarchies=%d handles=%d hidden=%d"),
        HierarchySelectionInfo.Num(), HierarchyHandles.Num(), HiddenHierarchies.Num());
}

void AKyaLevelPreview::RefreshSceneVisualizers()
{
    UWorld* World = GetWorld();
    const bool bAny = (bSpawnLightVisualizers || bSpawnCameraVisualizer || bSpawnActorLinkVisualizers ||
        bSpawnTriggerVisualizers) && World && !World->IsGameWorld();
    if (!bAny || !Bank)
    {
        for (const TPair<FString, TObjectPtr<AKyaSceneVisualizer>>& Pair : SceneVisualizers)
            if (IsValid(Pair.Value)) Pair.Value->Destroy();
        SceneVisualizers.Reset();
        return;
    }

    TSet<FString> Wanted;
    const auto SpawnOrUpdate = [this, World, &Wanted](const FString& Key, const FString& Label,
        TFunction<void(AKyaSceneVisualizer*)> Initialize)
    {
        Wanted.Add(Key);
        if (TObjectPtr<AKyaSceneVisualizer>* Existing = SceneVisualizers.Find(Key))
        {
            if (IsValid(*Existing))
            {
                (*Existing)->Preview = this;
                (*Existing)->Bank = Bank;
                Initialize(*Existing);
                (*Existing)->SyncFromBank();
                (*Existing)->SetGeneratedLabel(Label);
                return;
            }
        }
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.ObjectFlags |= RF_Transient | RF_Transactional;
        AKyaSceneVisualizer* Visualizer = World->SpawnActor<AKyaSceneVisualizer>(
            AKyaSceneVisualizer::StaticClass(), FTransform::Identity, Params);
        if (!Visualizer) return;
        Visualizer->SetFlags(RF_Transient | RF_Transactional);
        Visualizer->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
        Initialize(Visualizer);
        Visualizer->SetGeneratedLabel(Label);
        SceneVisualizers.Add(Key, Visualizer);
    };

    TArray<UKyaLevelBank*> Banks;
    Banks.Add(Bank);
    for (const TObjectPtr<UKyaLevelBank>& Sector : SectorBanks)
        if (Sector) Banks.AddUnique(Sector);

    for (UKyaLevelBank* VisualBank : Banks)
    {
        if (!VisualBank) continue;
        if (bSpawnLightVisualizers)
        {
            for (int32 Index = 0; Index < VisualBank->Lights.Num(); ++Index)
            {
                const FString Key = FString::Printf(TEXT("light:%s:%d"), *VisualBank->GetName(), Index);
                const FString Label = FString::Printf(TEXT("%s light %d"), *VisualBank->GetName(), Index);
                SpawnOrUpdate(Key, Label, [this, VisualBank, Index](AKyaSceneVisualizer* Visualizer)
                {
                    Visualizer->InitializeLight(this, VisualBank, Index);
                });
            }
        }
        if (bSpawnActorLinkVisualizers)
        {
            for (int32 ActorIndex = 0; ActorIndex < VisualBank->SceneEnvironment.Actors.Num(); ++ActorIndex)
            {
                const FKyaActorPlacement& Actor = VisualBank->SceneEnvironment.Actors[ActorIndex];
                for (const FKyaReferenceField& Field : GetKyaReferenceFields())
                {
                    if (Field.Kind != EKyaReferenceKind::Actor && Field.Kind != EKyaReferenceKind::Waypoint) continue;
                    if (Field.ClassId != -1 && Field.ClassId != Actor.ClassId) continue;
                    const int64 Value = GetKyaNumericField(Actor, Field.PropertyName);
                    if (Value < 0) continue;
                    FVector Target;
                    FString TargetLabel;
                    if (Field.Kind == EKyaReferenceKind::Actor)
                    {
                        if (!VisualBank->SceneEnvironment.Actors.IsValidIndex(int32(Value))) continue;
                        Target = VisualBank->SceneEnvironment.Actors[int32(Value)].Position;
                        TargetLabel = FString::Printf(TEXT("actor %lld"), Value);
                    }
                    else
                    {
                        if (!VisualBank->SceneEnvironment.Waypoints.IsValidIndex(int32(Value))) continue;
                        Target = VisualBank->SceneEnvironment.Waypoints[int32(Value)].Position;
                        TargetLabel = FString::Printf(TEXT("waypoint %lld"), Value);
                    }
                    const FString Key = FString::Printf(TEXT("link:%s:%d:%s"),
                        *VisualBank->GetName(), ActorIndex, *Field.PropertyName.ToString());
                    const FString Label = FString::Printf(TEXT("%s -> %s"), *Field.PropertyName.ToString(), *TargetLabel);
                    SpawnOrUpdate(Key, Label, [this, VisualBank, ActorIndex, Field, Target](AKyaSceneVisualizer* Visualizer)
                    {
                        Visualizer->InitializeActorLink(this, VisualBank, ActorIndex, Field.PropertyName,
                            VisualBank->SceneEnvironment.Actors[ActorIndex].Position, Target);
                    });
                }
            }
        }
        if (bSpawnTriggerVisualizers)
        {
            for (int32 ActorIndex = 0; ActorIndex < VisualBank->SceneEnvironment.Actors.Num(); ++ActorIndex)
            {
                const FKyaActorPlacement& Actor = VisualBank->SceneEnvironment.Actors[ActorIndex];
                for (int32 CheckpointIndex = 0; CheckpointIndex < Actor.Checkpoints.Num(); ++CheckpointIndex)
                {
                    const int32 WaypointIndex = Actor.Checkpoints[CheckpointIndex].WaypointA;
                    if (!VisualBank->SceneEnvironment.Waypoints.IsValidIndex(WaypointIndex)) continue;
                    const FVector Center = VisualBank->SceneEnvironment.Waypoints[WaypointIndex].Position;
                    const FString Key = FString::Printf(TEXT("trigger:%s:%d:%d"),
                        *VisualBank->GetName(), ActorIndex, CheckpointIndex);
                    const FString Label = FString::Printf(TEXT("Trigger %d/%d (approx.)"), ActorIndex, CheckpointIndex);
                    SpawnOrUpdate(Key, Label, [this, VisualBank, ActorIndex, CheckpointIndex, Center](
                        AKyaSceneVisualizer* Visualizer)
                    {
                        Visualizer->InitializeTrigger(this, VisualBank, ActorIndex, CheckpointIndex, Center);
                    });
                }
            }
        }
    }

    if (bSpawnCameraVisualizer)
    {
        const FString Key = FString::Printf(TEXT("camera:%s"), *Bank->GetName());
        SpawnOrUpdate(Key, FString::Printf(TEXT("%s main camera"), *Bank->GetName()),
            [this](AKyaSceneVisualizer* Visualizer) { Visualizer->InitializeCamera(this, Bank); });
    }

    for (auto It = SceneVisualizers.CreateIterator(); It; ++It)
    {
        if (Wanted.Contains(It.Key())) continue;
        if (IsValid(It.Value())) It.Value()->Destroy();
        It.RemoveCurrent();
    }
    UE_LOG(LogTemp, Display, TEXT("KYA_SCENE_VISUALIZERS lights=%d camera=%d links=%d triggers=%d total=%d"),
        bSpawnLightVisualizers ? 1 : 0, bSpawnCameraVisualizer ? 1 : 0, bSpawnActorLinkVisualizers ? 1 : 0,
        bSpawnTriggerVisualizers ? 1 : 0, SceneVisualizers.Num());
    VisualizerLimitations = TEXT("Zones are not visualized: this schema stores zone references without serialized "
        "bounds. Trigger bounds are approximate checkpoint/waypoint markers, and actor links are derived "
        "diagnostics; neither has native geometry.");
}

AKyaSceneVisualizer* AKyaLevelPreview::GetSceneVisualizer(const FString& Key) const
{
    const TObjectPtr<AKyaSceneVisualizer>* Found = SceneVisualizers.Find(Key);
    return Found ? Found->Get() : nullptr;
}
#endif

void AKyaLevelPreview::SetBackgroundCameraPosition(const FVector& WorldCameraPosition)
{
    const FVector Local = GetActorTransform().InverseTransformPosition(WorldCameraPosition) / 100.0;
    const FVector Camera(Local.X, Local.Z, Local.Y);
    if (bHasBackgroundCamera && LastBackgroundCamera.Equals(Camera, 0.0001)) return;
    LastBackgroundCamera = Camera;
    bHasBackgroundCamera = true;
    TMap<int32, TArray<FVector>> Updated = BackgroundBasePositions;
    for (const FBackgroundRange& Range : BackgroundRanges)
    {
        TArray<FVector>* Positions = Updated.Find(Range.Section);
        if (!Positions || Range.Start < 0 || Range.Count < 0 || Range.Start + Range.Count > Positions->Num()) continue;
        const FVector Delta = Range.CameraX * Camera.X + Range.CameraY * Camera.Y + Range.CameraZ * Camera.Z;
        for (int32 Index = Range.Start; Index < Range.Start + Range.Count; ++Index) (*Positions)[Index] += Delta;
    }
    for (const TPair<int32, TArray<FVector>>& Pair : Updated)
        Mesh->UpdateMeshSection(Pair.Key, Pair.Value, {}, {}, {}, {});
}

void AKyaLevelPreview::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
#if WITH_EDITOR
    if (!bExplicitPreviewCamera && GCurrentLevelEditingViewportClient && GCurrentLevelEditingViewportClient->GetWorld() == GetWorld())
    {
        const FLevelEditorViewportClient& View = *GCurrentLevelEditingViewportClient;
        const FIntPoint Size = View.Viewport ? View.Viewport->GetSizeXY() : FIntPoint(1024, 768);
        const FRotationMatrix Rotation(View.GetViewRotation());
        UpdatePreviewCamera(View.GetViewLocation(), Rotation.GetUnitAxis(EAxis::X), Rotation.GetUnitAxis(EAxis::Z),
            View.ViewFOV, Size.X, Size.Y);
    }
#endif
}

void AKyaLevelPreview::SetPreviewCamera(const FVector& Position, const FVector& Forward, const FVector& Up,
    float HorizontalFovDegrees, int32 Width, int32 Height)
{
    bExplicitPreviewCamera = true;
    UpdatePreviewCamera(Position, Forward, Up, HorizontalFovDegrees, Width, Height);
#if WITH_EDITOR
    // Capture/comparison mode must drop authoring-only handles immediately, without
    // waiting for an unrelated rebuild.
    RefreshActorHandles();
#endif
}

void AKyaLevelPreview::ClearPreviewCamera()
{
    if (!bExplicitPreviewCamera) return;
    bExplicitPreviewCamera = false;
#if WITH_EDITOR
    // Recompute culling from the current editor viewport camera on the next update.
    if (GCurrentLevelEditingViewportClient && GCurrentLevelEditingViewportClient->GetWorld() == GetWorld())
        SetBackgroundCameraPosition(GCurrentLevelEditingViewportClient->GetViewLocation());
    // Authoring discovery can expose unloaded-sector actors again.
    RefreshActorHandles();
#endif
}

void AKyaLevelPreview::UpdatePreviewCamera(const FVector& Position, const FVector& Forward, const FVector& Up,
    float HorizontalFovDegrees, int32 Width, int32 Height)
{
    SetBackgroundCameraPosition(Position);
    if (Width <= 0 || Height <= 0 || HorizontalFovDegrees <= 0 || HorizontalFovDegrees >= 180) return;
    const FVector F = Forward.GetSafeNormal();
    const FVector R = FVector::CrossProduct(Up, F).GetSafeNormal();
    const FVector U = FVector::CrossProduct(F, R);
    const double TanX = FMath::Tan(FMath::DegreesToRadians(HorizontalFovDegrees) * 0.5);
    const double TanY = TanX * Height / Width;
    UpdateClusterVisibility(Position, F, R, U, TanX, TanY);
    const double SurfaceWidth = FMath::Max(1, NativeSurfaceSize.X);
    const double SurfaceHeight = FMath::Max(1, NativeSurfaceSize.Y);
    // Native main scene defaults: near=.001, far=5000, depth max=0xffffef >> 3.
    // Fixed flares test against GS depth 500 (ed3DFlushSpriteFlareFX), not their source distance.
    constexpr double Near = 0.001, Far = 5000, DepthMax = 0xffffef >> 3;
    constexpr double MarkerDepth = Near * Far * (DepthMax - 1) / ((500 - 1) * (Far - Near) + Near * (DepthMax - 1));
    for (const FBackgroundFlare& Flare : BackgroundFlares)
    {
        if (!FlareMaterials.IsValidIndex(Flare.MaterialIndex)) continue;
        auto* Material = FlareMaterials[Flare.MaterialIndex].Get();
        const FVector V = GetActorTransform().TransformPosition(Flare.Position) - Position;
        const double Z = FVector::DotProduct(V, F);
        if (Z <= 0) { Material->SetVectorParameterValue(TEXT("FlareRect"), FLinearColor(0, 0, 0, 0)); continue; }
        const double X = 0.5 + FVector::DotProduct(V, R) / (2 * Z * TanX);
        const double Y = 0.5 - FVector::DotProduct(V, U) / (2 * Z * TanY);
        // The native flare path accepts sources within GS coordinates [1300,2700].
        const double GsX = (X - 0.5) * SurfaceWidth, GsY = (Y - 0.5) * SurfaceHeight;
        if (GsX < -748 || GsX > 652 || GsY < -748 || GsY > 652)
        { Material->SetVectorParameterValue(TEXT("FlareRect"), FLinearColor(0, 0, 0, 0)); continue; }
        Material->SetVectorParameterValue(TEXT("FlareRect"), FLinearColor(X, Y, 512.0 / SurfaceWidth, 512.0 / SurfaceHeight));
        Material->SetVectorParameterValue(TEXT("FlareMarker"), FLinearColor(
            FMath::Clamp(X, 0.5 - 248.0 / SurfaceWidth, 0.5 + 248.0 / SurfaceWidth),
            FMath::Clamp(Y, 0.5 - 248.0 / SurfaceHeight, 0.5 + 248.0 / SurfaceHeight), float(MarkerDepth * 100.0), 0));
    }
}

void AKyaLevelPreview::UpdateClusterVisibility(const FVector& Position, const FVector& Forward, const FVector& Right,
    const FVector& Up, double TanX, double TanY)
{
    const FTransform Actor = GetActorTransform();
    const double Scale = Actor.GetScale3D().GetAbsMax();
    auto CameraSphere = [&](const FVector4f& Sphere) {
        const FVector Delta = Actor.TransformPosition(FVector(Sphere.X, Sphere.Y, Sphere.Z)) - Position;
        return FVector4f(FVector::DotProduct(Delta, Right) / 100, FVector::DotProduct(Delta, Up) / 100,
            FVector::DotProduct(Delta, Forward) / 100, Sphere.W * Scale / 100);
    };
    TSet<int32> Changed;
    int32 Hidden = 0;
    // Authoring discovery draws culled cluster geometry so hidden scenery stays
    // inspectable. A locked comparison/capture camera restores native culling.
    const bool bShowAllAuthoring = IsAuthoringVisibilityActive() && bAuthoringShowCulledGeometry;
    for (FClusterVisibilityRange& Range : ClusterVisibilityRanges)
    {
        const FVector4f Lod = CameraSphere(Range.LodSphere);
        const double Factor = (FMath::Square(double(Lod.X)) + FMath::Square(double(Lod.Y)) + FMath::Square(double(Lod.Z))) * TanY * TanY;
        const bool Visible = bShowAllAuthoring || (Factor >= Range.MinLodBiasSquared && Factor < Range.MaxLodBiasSquared &&
            KyaScene::IsClusterStripVisible(CameraSphere(Range.ObjectSphere), CameraSphere(Range.StripSphere), TanX, TanY, ActiveClipDistance));
        if (!Visible) ++Hidden;
        if (Range.bVisible != Visible) { Range.bVisible = Visible; Changed.Add(Range.Section); }
    }
    for (int32 SectionIndex : Changed)
    {
        const TArray<uint32>* Base = ClusterBaseIndices.Find(SectionIndex);
        const FProcMeshSection* Original = Mesh->GetProcMeshSection(SectionIndex);
        if (!Base || !Original) continue;
        FProcMeshSection Updated = *Original;
        Updated.ProcIndexBuffer = *Base;
        for (const FClusterVisibilityRange& Range : ClusterVisibilityRanges)
            if (Range.Section == SectionIndex && !Range.bVisible && Range.Count > 0 &&
                Range.Start >= 0 && Range.Start + Range.Count <= Updated.ProcIndexBuffer.Num())
            {
                const uint32 Anchor = (*Base)[Range.Start];
                for (int32 I = Range.Start; I < Range.Start + Range.Count; ++I) Updated.ProcIndexBuffer[I] = Anchor;
            }
        Mesh->SetProcMeshSection(SectionIndex, Updated);
    }
    if (bLogSectorDiagnostics && !Changed.IsEmpty())
        UE_LOG(LogTemp, Display, TEXT("KYA_CLUSTER_VISIBILITY ranges=%d hidden=%d changed_sections=%d clip=%.3f"),
            ClusterVisibilityRanges.Num(), Hidden, Changed.Num(), ActiveClipDistance);
}

#if WITH_EDITOR
void AKyaLevelPreview::UpdatePathGuidePoint(int32 PathIndex, int32 PointIndex, const FVector& Position)
{
    TObjectPtr<USplineComponent>* Found = PathGuides.Find(PathIndex);
    if (!Found || !IsValid(*Found)) return;
    USplineComponent* Guide = *Found;
    if (PointIndex >= 0 && PointIndex < Guide->GetNumberOfSplinePoints())
        Guide->SetLocationAtSplinePoint(PointIndex, Position, ESplineCoordinateSpace::Local, true);
}
#endif
