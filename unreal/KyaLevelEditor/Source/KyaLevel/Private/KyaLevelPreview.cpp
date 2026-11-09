#include "KyaLevelPreview.h"
#include "KyaG3DChunks.h"
#include "KyaG2D.h"
#include "KyaLevelBank.h"
#include "KyaWaypointComponent.h"
#include "KyaPathPointComponent.h"
#include "KyaActorMarkerComponent.h"
#include "KyaActorTransform.h"
#include "ProceduralMeshComponent.h"
#include "Components/SplineComponent.h"
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
#endif

AKyaLevelPreview::AKyaLevelPreview()
{
    PrimaryActorTick.bCanEverTick = true;
    Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AKyaLevelPreview::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    Mesh->ClearAllMeshSections();
    BackgroundRanges.Reset();
    BackgroundBasePositions.Reset();
    bHasBackgroundCamera = false;
    PreviewTextures.Reset();
#if WITH_EDITOR
    const int32 DesiredMarkers = Bank && bShowWaypoints ? Bank->SceneEnvironment.Waypoints.Num() : 0;
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
    if (!PathPreview)
    {
        PathPreview = NewObject<USplineComponent>(this, NAME_None, RF_Transient);
        PathPreview->SetupAttachment(RootComponent);
        PathPreview->SetDrawDebug(true);
        PathPreview->bSelectable = false;
        PathPreview->RegisterComponent();
    }
    PathPreview->ClearSplinePoints(false);
    if (Bank && Bank->SceneEnvironment.Paths.IsValidIndex(PreviewPathIndex))
    {
        const FKyaPath& Path = Bank->SceneEnvironment.Paths[PreviewPathIndex];
        for (const FVector4& P : Path.Points)
            PathPreview->AddSplinePoint(FVector(P.X, P.Z, P.Y) * 100.0, ESplineCoordinateSpace::Local, false);
        for (int32 Point = 0; Point < Path.Points.Num(); ++Point)
            PathPreview->SetSplinePointType(Point, ESplinePointType::Linear, false);
    }
    PathPreview->UpdateSpline();
    const int32 DesiredPathMarkers = Bank && Bank->SceneEnvironment.Paths.IsValidIndex(PreviewPathIndex)
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
    if (!Bank || !Bank->Entries.IsValidIndex(Bank->BackgroundMeshEntry)) return;
#if WITH_EDITOR
    if (bGeneratedRawColorModulation != bRawColorModulation)
    {
        GeneratedPreviewMaterial = nullptr;
        GeneratedMaskedPreviewMaterial = nullptr;
        bGeneratedRawColorModulation = bRawColorModulation;
    }
    if (!GeneratedUntexturedMaterial)
    {
        GeneratedUntexturedMaterial = NewObject<UMaterial>(this, NAME_None, RF_Transient);
        GeneratedUntexturedMaterial->SetShadingModel(MSM_Unlit);
        GeneratedUntexturedMaterial->TwoSided = true;
        UMaterialExpressionVertexColor* VertexColor = NewObject<UMaterialExpressionVertexColor>(GeneratedUntexturedMaterial);
        GeneratedUntexturedMaterial->GetExpressionCollection().AddExpression(VertexColor);
        GeneratedUntexturedMaterial->GetEditorOnlyData()->EmissiveColor.Connect(0, VertexColor);
        GeneratedUntexturedMaterial->PostEditChange();
    }
    auto CreateTexturedMaterial = [this](bool Masked) -> UMaterial*
    {
        UMaterial* Material = NewObject<UMaterial>(this, NAME_None, RF_Transient);
        Material->SetShadingModel(MSM_Unlit);
        Material->TwoSided = true;
        if (Masked) { Material->BlendMode = BLEND_Masked; Material->OpacityMaskClipValue = 1.5f / 255.0f; }
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
        if (bRawColorModulation)
        {
            UMaterialExpressionCustom* Convert = NewObject<UMaterialExpressionCustom>(Material);
            Convert->OutputType = CMOT_Float3;
            Convert->Code = TEXT("float3 c = saturate(encoded.rgb); return lerp(c / 12.92, pow((c + 0.055) / 1.055, 2.4), step(0.04045, c));");
            FCustomInput Input;
            Input.InputName = TEXT("encoded");
            Input.Input.Connect(0, Normalize);
            Convert->Inputs.Add(Input);
            Material->GetExpressionCollection().AddExpression(Convert);
            Material->GetEditorOnlyData()->EmissiveColor.Connect(0, Convert);
        }
        else Material->GetEditorOnlyData()->EmissiveColor.Connect(0, Normalize);
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
#endif
    if (GeneratedUntexturedMaterial) Mesh->SetMaterial(0, GeneratedUntexturedMaterial);
    UMaterialInterface* EffectiveMaterial = TextureMaterial ? TextureMaterial.Get() : GeneratedPreviewMaterial.Get();
    const FKyaBankEntry& Entry = Bank->Entries[Bank->BackgroundMeshEntry];
    if (Entry.Offset < 0 || Entry.Size < 0 || Entry.Offset > Bank->BankBytes.Num() ||
        Entry.Size > Bank->BankBytes.Num() - Entry.Offset || Entry.Size > MAX_int32) return;
    TArray<FKyaG3DDecodedStrip> Strips;
    if (!KyaG3D::DecodeHierarchyStrips(MakeArrayView(Bank->BankBytes.GetData() + Entry.Offset, int32(Entry.Size)), Strips)) return;
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
    TMap<uint64, int32> MaterialSections;
    struct FSectionData { int32 Section = 0; TArray<FVector> Positions; TArray<int32> Triangles; TArray<FVector2D> UVs; TArray<FColor> Colors; };
    TMap<int32, FSectionData> TexturedSections;
    auto GetTexturedSection = [&](uint64 Hash, TConstArrayView<uint8> Source,
        const TMap<uint64, int32>& Offsets, TMap<uint64, int32>& Sections) -> FSectionData*
    {
        const int32* Offset = Offsets.Find(Hash);
        if (!EffectiveMaterial || !Offset) return nullptr;
        int32* Found = Sections.Find(Hash);
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
                    void* Dest = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
                    FMemory::Memcpy(Dest, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
                    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
                    Texture->UpdateResource();
                    uint64 TestRegister = 0;
                    const bool Masked = !TextureMaterial && KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x47, TestRegister) &&
                        (TestRegister & 1) && ((TestRegister >> 1) & 7) == 5 && ((TestRegister >> 4) & 255) == 1 && ((TestRegister >> 12) & 3) == 0;
                    UMaterialInterface* Parent = Masked ? GeneratedMaskedPreviewMaterial.Get() : EffectiveMaterial;
                    UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Parent, this);
                    if (Material)
                    {
                        Material->SetTextureParameterValue(TEXT("KyaTexture"), Texture);
                        Section = NextSection++;
                        Mesh->SetMaterial(Section, Material);
                        PreviewTextures.Add(Texture);
                    }
                }
            }
            Found = &Sections.Add(Hash, Section);
        }
        if (*Found == INDEX_NONE) return nullptr;
        FSectionData& Result = TexturedSections.FindOrAdd(*Found);
        Result.Section = *Found;
        return &Result;
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
    const auto CommonBackgroundData = MakeArrayView(Bank->BankBytes.GetData() + Entry.Offset, int32(Entry.Size));
    TArray<int32> CommonCameraRelative;
    if ((Bank->SceneEnvironment.BackgroundFlags & 1) && !bHasSectorBackground)
        KyaG3D::ReadCameraRelativeBackgroundHierarchies(CommonBackgroundData, CommonCameraRelative);
    if (!bHasSectorBackground) for (const FKyaG3DDecodedStrip& Strip : Strips)
    {
        FSectionData* Textured = GetTexturedSection(Strip.MaterialHash, TextureData, MaterialOffsets, MaterialSections);
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
        for (uint32 Index : Strip.Indices)
            TargetTriangles.Add(Base + int32(Index));
    }
    if (bShowAllActorMeshes || (bShowActorMesh && Bank->SceneEnvironment.Actors.IsValidIndex(PreviewActorIndex)))
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
        const auto DefaultTextures = EntryData(FindEntry(5, Bank->SceneEnvironment.AlternateTextureIndex));
        TMap<uint64, int32> DefaultMaterials, DefaultSections;
        KyaG2D::ReadMaterials(DefaultTextures, DefaultMaterials);
        TMap<int32, TMap<uint64, int32>> MaterialsByTexture, SectionsByTexture;
        TMap<int32, TArray<FKyaG3DDecodedStrip>> StripsByMesh;
        for (int32 ActorIndex = 0; ActorIndex < Bank->SceneEnvironment.Actors.Num(); ++ActorIndex)
        {
        if (!bShowAllActorMeshes && ActorIndex != PreviewActorIndex) continue;
        const FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[ActorIndex];
        const auto ModelData = EntryData(FindEntry(4, Actor.MeshIndex));
        const int32 TextureIndex = Actor.TextureIndex == -1 ? Bank->SceneEnvironment.AlternateTextureIndex : Actor.TextureIndex;
        const auto ModelTextures = EntryData(FindEntry(5, TextureIndex));
        if (!MaterialsByTexture.Contains(TextureIndex))
        {
            TMap<uint64, int32>& Materials = MaterialsByTexture.Add(TextureIndex);
            KyaG2D::ReadMaterials(ModelTextures, Materials);
        }
        const TMap<uint64, int32>& ModelMaterials = MaterialsByTexture[TextureIndex];
        TMap<uint64, int32>& ModelSections = SectionsByTexture.FindOrAdd(TextureIndex);
        TMap<uint64, uint64> MaterialPatches;
        for (const FKyaActorModelReference& Patch : Actor.Models)
        {
            const uint64 Replacement = KyaG2D::HashName(Patch.TextureName);
            if (DefaultMaterials.Contains(Replacement)) MaterialPatches.Add(KyaG2D::HashName(Patch.MeshName), Replacement);
        }
        if (!StripsByMesh.Contains(Actor.MeshIndex))
        {
            TArray<FKyaG3DDecodedStrip>& ModelStrips = StripsByMesh.Add(Actor.MeshIndex);
            KyaG3D::DecodeHierarchyStrips(ModelData, ModelStrips);
        }
        for (const FKyaG3DDecodedStrip& Strip : StripsByMesh[Actor.MeshIndex])
            {
                FMatrix HierarchyTransform;
                if (!KyaG3D::ReadHierarchyWorldMatrix(ModelData, Strip.HierarchyOffset, HierarchyTransform)) continue;
                FSectionData* Textured = nullptr;
                if (const uint64* Replacement = MaterialPatches.Find(Strip.MaterialHash))
                    Textured = GetTexturedSection(*Replacement, DefaultTextures, DefaultMaterials, DefaultSections);
                else if (ModelMaterials.Contains(Strip.MaterialHash))
                    Textured = GetTexturedSection(Strip.MaterialHash, ModelTextures, ModelMaterials, ModelSections);
                else
                    Textured = GetTexturedSection(Strip.MaterialHash, DefaultTextures, DefaultMaterials, DefaultSections);
                TArray<FVector>& TargetPositions = Textured ? Textured->Positions : Positions;
                TArray<int32>& TargetTriangles = Textured ? Textured->Triangles : Triangles;
                TArray<FVector2D>& TargetUVs = Textured ? Textured->UVs : UVs;
                TArray<FColor>& TargetColors = Textured ? Textured->Colors : Colors;
                const int32 Base = TargetPositions.Num();
                for (const FKyaG3DVertex& Vertex : Strip.Vertices)
                {
                    FVector P = HierarchyTransform.TransformPosition(FVector(Vertex.Position)) * Actor.Scale;
                    P = KyaActorTransform::ApplyClassRotation(P, Actor.Rotation, Actor.ClassId);
                    P += Actor.Position;
                    TargetPositions.Add(FVector(P.X, P.Z, P.Y) * 100.0);
                }
                for (const FKyaG3DVertexAttributes& Attribute : Strip.Attributes)
                {
                    TargetUVs.Add(FVector2D(Attribute.UV));
                    TargetColors.Add(Attribute.Color);
                }
                for (uint32 Index : Strip.Indices) TargetTriangles.Add(Base + int32(Index));
            }
        }
    }
    // The common scene mesh is a static cluster, just like sector clusters.
    TArray<const UKyaLevelBank*> ClusterBanks;
    ClusterBanks.Add(Bank);
    for (const UKyaLevelBank* Sector : SectorBanks) ClusterBanks.Add(Sector);
    TConstArrayView<uint8> CommonClusterTextures;
    TMap<uint64, int32> CommonClusterMaterials, CommonClusterSections;
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
    for (const UKyaLevelBank* Sector : ClusterBanks)
    {
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
        TMap<uint64, int32> SectorMaterialOffsets, SectorMaterialSections;
        TConstArrayView<uint8> ClusterTextureData;
        TMap<uint64, int32> ClusterMaterialOffsets, ClusterMaterialSections;
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
                                Textured = GetTexturedSection(MaterialHash, ClusterTextureData, ClusterMaterialOffsets, ClusterMaterialSections);
                            else
                                Textured = GetTexturedSection(MaterialHash, CommonClusterTextures, CommonClusterMaterials, CommonClusterSections);
                        }
                        else ++MissingHash;
                        ++Decoded;
                        if (Textured) ++TexturedCount;
                        VertexCount += Expanded.Num();
                        TriangleCount += Indices.Num() / 3;
                        TArray<FVector>& TargetPositions = Textured ? Textured->Positions : Positions;
                        TArray<int32>& TargetTriangles = Textured ? Textured->Triangles : Triangles;
                        TArray<FVector2D>& TargetUVs = Textured ? Textured->UVs : UVs;
                        TArray<FColor>& TargetColors = Textured ? Textured->Colors : Colors;
                        const int32 Base = TargetPositions.Num();
                        for (int32 VertexIndex = 0; VertexIndex < Expanded.Num(); ++VertexIndex)
                        {
                            const FKyaG3DVertex& Vertex = Expanded[VertexIndex];
                            const FVector Position = FVector(Vertex.Position.X, Vertex.Position.Z, Vertex.Position.Y) * 100.0;
                            TargetPositions.Add(Position);
                            Bounds += Position;
                            TargetUVs.Add(FVector2D(Attributes[VertexIndex].UV));
                            TargetColors.Add(Attributes[VertexIndex].Color);
                        }
                        for (uint32 Index : Indices) TargetTriangles.Add(Base + int32(Index));
                    }
                }
                if (bLogSectorDiagnostics)
                    UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_CLUSTER name=%s entry=%d parsed=%d strips=%d decoded=%d failed=%d textured=%d no_hash=%d missing_material=%d local_materials=%d common_materials=%d vertices=%lld triangles=%lld bounds_cm=%s"),
                        *DiagnosticName, EntryIndex, ClusterParsed, ClusterStrips.Num(), Decoded, Failed, TexturedCount, MissingHash,
                        MissingMaterial, ClusterMaterialOffsets.Num(), CommonClusterMaterials.Num(), VertexCount, TriangleCount, *Bounds.ToString());
                if (ClusterParsed && !ClusterHierarchies.IsEmpty())
                {
                    TArray<FKyaG3DDecodedStrip> HierarchyStrips;
                    TArray<int32> ExpandedHierarchies;
                    const bool LinksParsed = KyaG3D::ExpandClusterHierarchies(Data, ClusterHierarchies, ExpandedHierarchies);
                    const bool Parsed = LinksParsed && KyaG3D::DecodeHierarchyStrips(Data, HierarchyStrips);
                    int32 Included = 0, HierarchyTextured = 0, TransformFailures = 0;
                    int64 HierarchyTriangles = 0;
                    FBox HierarchyBounds(ForceInit);
                    if (Parsed)
                        for (const FKyaG3DDecodedStrip& Strip : HierarchyStrips)
                        {
                            if (!ExpandedHierarchies.Contains(Strip.HierarchyOffset)) continue;
                            const uint16 Flags = uint16(Data[Strip.HierarchyOffset + 0x9e]) | (uint16(Data[Strip.HierarchyOffset + 0x9f]) << 8);
                            if (Flags & 0x40) continue;
                            FMatrix HierarchyTransform;
                            if (!KyaG3D::ReadHierarchyWorldMatrix(Data, Strip.HierarchyOffset, HierarchyTransform)) { ++TransformFailures; continue; }
                            FSectionData* Textured = ClusterMaterialOffsets.Contains(Strip.MaterialHash)
                                ? GetTexturedSection(Strip.MaterialHash, ClusterTextureData, ClusterMaterialOffsets, ClusterMaterialSections)
                                : GetTexturedSection(Strip.MaterialHash, CommonClusterTextures, CommonClusterMaterials, CommonClusterSections);
                            TArray<FVector>& TargetPositions = Textured ? Textured->Positions : Positions;
                            TArray<int32>& TargetTriangles = Textured ? Textured->Triangles : Triangles;
                            TArray<FVector2D>& TargetUVs = Textured ? Textured->UVs : UVs;
                            TArray<FColor>& TargetColors = Textured ? Textured->Colors : Colors;
                            const int32 Base = TargetPositions.Num();
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
                            for (uint32 Index : Strip.Indices) TargetTriangles.Add(Base + int32(Index));
                            ++Included;
                            if (Textured) ++HierarchyTextured;
                            HierarchyTriangles += Strip.Indices.Num() / 3;
                        }
                    if (bLogSectorDiagnostics)
                        UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_CLUSTER_HIERARCHY_RENDER name=%s parsed=%d roots=%d linked=%d strips=%d textured=%d transform_failures=%d triangles=%lld bounds_cm=%s"),
                            *DiagnosticName, Parsed, ClusterHierarchies.Num(), ExpandedHierarchies.Num(), Included, HierarchyTextured, TransformFailures, HierarchyTriangles, *HierarchyBounds.ToString());
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
            if (Bank->SceneEnvironment.BackgroundFlags & 1)
                KyaG3D::ReadCameraRelativeBackgroundHierarchies(Data, SectorCameraRelative);
            for (const FKyaG3DDecodedStrip& Strip : SectorStrips)
            {
                FSectionData* Textured = GetTexturedSection(Strip.MaterialHash, SectorTextureData, SectorMaterialOffsets, SectorMaterialSections);
                if (Textured) ++BackgroundTextured;
                BackgroundVertices += Strip.Vertices.Num();
                BackgroundTriangles += Strip.Indices.Num() / 3;
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
                for (uint32 Index : Strip.Indices) TargetTriangles.Add(Base + int32(Index));
            }
            if (bLogSectorDiagnostics)
                UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_BACKGROUND name=%s entry=%d parsed=1 strips=%d textured=%d missing_transform=%d materials=%d vertices=%lld triangles=%lld bounds_cm=%s"),
                    *DiagnosticName, EntryIndex, SectorStrips.Num(), BackgroundTextured, BackgroundMissingTransform,
                    SectorMaterialOffsets.Num(), BackgroundVertices, BackgroundTriangles, *BackgroundBounds.ToString());
        }
        if (bLogSectorDiagnostics) UE_LOG(LogTemp, Display, TEXT("KYA_SECTOR_END name=%s"), *DiagnosticName);
    }
    Mesh->CreateMeshSection(0, Positions, Triangles, TArray<FVector>(), UVs,
        Colors, TArray<FProcMeshTangent>(), false);
    for (const TPair<int32, FSectionData>& Pair : TexturedSections)
        Mesh->CreateMeshSection(Pair.Key, Pair.Value.Positions, Pair.Value.Triangles, TArray<FVector>(),
            Pair.Value.UVs, Pair.Value.Colors, TArray<FProcMeshTangent>(), false);
    for (const FBackgroundRange& Range : BackgroundRanges)
        if (!BackgroundBasePositions.Contains(Range.Section))
        {
            const FProcMeshSection* Section = Mesh->GetProcMeshSection(Range.Section);
            if (!Section) continue;
            TArray<FVector>& Base = BackgroundBasePositions.Add(Range.Section);
            Base.Reserve(Section->ProcVertexBuffer.Num());
            for (const FProcMeshVertex& Vertex : Section->ProcVertexBuffer) Base.Add(Vertex.Position);
        }
    if (bLogSectorDiagnostics)
        UE_LOG(LogTemp, Display, TEXT("KYA_BACKGROUND_FOLLOW flags=%lld ranges=%d sections=%d"),
            Bank->SceneEnvironment.BackgroundFlags, BackgroundRanges.Num(), BackgroundBasePositions.Num());
}

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
    if (GCurrentLevelEditingViewportClient && GCurrentLevelEditingViewportClient->GetWorld() == GetWorld())
        SetBackgroundCameraPosition(GCurrentLevelEditingViewportClient->GetViewLocation());
#endif
}

#if WITH_EDITOR
void AKyaLevelPreview::UpdatePathGuidePoint(int32 PathIndex, int32 PointIndex, const FVector& Position)
{
    if (PathPreview && PathIndex == PreviewPathIndex && PointIndex >= 0 && PointIndex < PathPreview->GetNumberOfSplinePoints())
        PathPreview->SetLocationAtSplinePoint(PointIndex, Position, ESplineCoordinateSpace::Local, true);
}
#endif
