#include "KyaPrimitiveComponent.h"
#include "KyaActorTransform.h"
#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"
#include "Materials/Material.h"
#include "Materials/MaterialRenderProxy.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#if WITH_EDITOR
#include "Materials/MaterialExpressionVertexColor.h"
#endif

#if WITH_EDITOR
IMPLEMENT_HIT_PROXY(HKyaPrimitiveProxy, HComponentVisProxy);
#endif

namespace
{
struct FPrimitiveWire
{
    TArray<FVector> Edges;
    TArray<FVector> Triangles;
    FLinearColor Color;
    bool bWireframe = true;
#if WITH_EDITOR
    TRefCountPtr<HHitProxy> HitProxy;
#endif
};

class FKyaPrimitiveProxy final : public FPrimitiveSceneProxy
{
    TArray<FPrimitiveWire> Wires;
    const FMaterialRenderProxy* SolidMaterial = nullptr;
    float WireThickness;
    float WireDepthBias;
    float SolidOpacity;
public:
    FKyaPrimitiveProxy(const UKyaPrimitiveComponent* Component) : FPrimitiveSceneProxy(Component),
        WireThickness(Component->WireThickness), WireDepthBias(Component->WireDepthBias), SolidOpacity(Component->SolidOpacity)
    {
        if (Component->SolidMaterial) SolidMaterial = Component->SolidMaterial->GetRenderProxy();
        for (const FKyaPrimitiveRecord& R : Component->GetRecords())
        {
            FPrimitiveWire& Wire = Wires.AddDefaulted_GetRef();
            const EKyaPrimitiveDisplayMode Mode = R.ZoneIndex == INDEX_NONE ? Component->CollisionMode : Component->EventMode;
            Wire.bWireframe = Mode != EKyaPrimitiveDisplayMode::Solid;
            if (Wire.bWireframe) KyaPrimitives::BuildEdges(R, Wire.Edges);
            if (Mode != EKyaPrimitiveDisplayMode::Wireframe) KyaPrimitives::BuildTriangles(R, Wire.Triangles);
            for (FVector& P : Wire.Edges) P = KyaActorTransform::SwapGameAxes(P) * 100.0;
            for (FVector& P : Wire.Triangles) P = KyaActorTransform::SwapGameAxes(P) * 100.0;
            Wire.Color = R.ZoneIndex == INDEX_NONE ? FLinearColor(1, 0.65f, 0.05f)
                : R.GroupIndex == 1 ? FLinearColor(1, 0.15f, 0.2f) : FLinearColor(0, 0.85f, 1);
            if (Wires.Num() - 1 == Component->GetSelectedRecord()) Wire.Color = FLinearColor::Yellow;
        }
    }
#if WITH_EDITOR
    virtual HHitProxy* CreateHitProxies(UPrimitiveComponent* Component, TArray<TRefCountPtr<HHitProxy>>& OutHitProxies) override
    {
        for (int32 I = 0; I < Wires.Num(); ++I)
        {
            Wires[I].HitProxy = new HKyaPrimitiveProxy(Component, I,
                CastChecked<UKyaPrimitiveComponent>(Component)->GetRecordGeneration());
            OutHitProxies.Add(Wires[I].HitProxy);
        }
        return nullptr;
    }
#endif
    virtual SIZE_T GetTypeHash() const override { static size_t Unique; return reinterpret_cast<SIZE_T>(&Unique); }
    virtual uint32 GetMemoryFootprint() const override
    {
        uint32 Bytes = sizeof(*this) + GetAllocatedSize() + Wires.GetAllocatedSize();
        for (const FPrimitiveWire& Wire : Wires) Bytes += Wire.Edges.GetAllocatedSize() + Wire.Triangles.GetAllocatedSize();
        return Bytes;
    }
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
    {
        FPrimitiveViewRelevance R;
        R.bDrawRelevance = IsShown(View);
        R.bDynamicRelevance = true;
        R.bNormalTranslucency = SolidMaterial != nullptr;
        R.bSeparateTranslucency = SolidMaterial != nullptr;
        R.bEditorPrimitiveRelevance = UseEditorCompositing(View);
        return R;
    }
    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
        const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
    {
        for (int32 V = 0; V < Views.Num(); ++V)
        {
            if (!(VisibilityMap & (1u << V))) continue;
            FPrimitiveDrawInterface* PDI = Collector.GetPDI(V);
            FDynamicMeshBuilder Solid(Views[V]->GetFeatureLevel());
            int32 SolidVertices = 0;
            TArray<FDynamicMeshDrawOffset> Ranges;
            for (const FPrimitiveWire& Wire : Wires)
            {
                FDynamicMeshDrawOffset& Range = Ranges.AddDefaulted_GetRef();
#if WITH_EDITOR
                PDI->SetHitProxy(Wire.HitProxy.GetReference());
#endif
                for (int32 I = 0; I + 1 < Wire.Edges.Num(); I += 2)
                    PDI->DrawLine(GetLocalToWorld().TransformPosition(Wire.Edges[I]),
                        GetLocalToWorld().TransformPosition(Wire.Edges[I + 1]), Wire.Color, SDPG_World,
                        WireThickness, WireDepthBias, true);
                if (SolidMaterial && !Wire.Triangles.IsEmpty())
                {
                    // Normal rendering batches every face in this bank; only the picking
                    // pass needs a separate draw range for each source primitive.
                    Range.FirstIndex = SolidVertices;
                    Range.MinVertexIndex = SolidVertices;
                    Range.MaxVertexIndex = SolidVertices + Wire.Triangles.Num() - 1;
                    Range.NumPrimitives = Wire.Triangles.Num() / 3;
                    for (int32 I = 0; I < Wire.Triangles.Num(); I += 3)
                    {
                        const FVector3f A(Wire.Triangles[I]), B(Wire.Triangles[I + 1]), C(Wire.Triangles[I + 2]);
                        const FVector3f Normal = FVector3f::CrossProduct(B - A, C - A).GetSafeNormal();
                        const FVector3f Tangent = (B - A).GetSafeNormal();
                        const float Shade = 0.45f + 0.55f * FMath::Abs(FVector3f::DotProduct(Normal, FVector3f(0.3f, -0.4f, 0.8660254f)));
                        FColor Color = (Wire.Color * Shade).ToFColor(false);
                        Color.A = uint8(FMath::RoundToInt(SolidOpacity * 255));
                        for (const FVector3f& P : {A, B, C})
                            Solid.AddVertex(P, FVector2f::ZeroVector, Tangent, FVector3f::CrossProduct(Normal, Tangent), Normal, Color);
                        Solid.AddTriangle(SolidVertices, SolidVertices + 1, SolidVertices + 2);
                        SolidVertices += 3;
                    }
                }
            }
#if WITH_EDITOR
            if (SolidVertices > 0 && ViewFamily.EngineShowFlags.HitProxies)
            {
                FDynamicMeshBuilderSettings Settings;
                Settings.CastShadow = false; Settings.bDisableBackfaceCulling = true;
                Settings.bReceivesDecals = false; Settings.bUseSelectionOutline = false;
                for (int32 I = 0; I < Wires.Num(); ++I)
                    if (Ranges[I].NumPrimitives && Wires[I].HitProxy)
                        Solid.GetMesh(GetLocalToWorld(), SolidMaterial, SDPG_World, Settings, &Ranges[I], V, Collector, Wires[I].HitProxy->Id);
            }
#endif
            if (SolidVertices > 0 && !ViewFamily.EngineShowFlags.HitProxies)
                Solid.GetMesh(GetLocalToWorld(), SolidMaterial, SDPG_World, true, false, false, V, Collector);
#if WITH_EDITOR
            PDI->SetHitProxy(nullptr);
#endif
        }
    }
};
}

void UKyaPrimitiveComponent::ConfigureDisplay(EKyaPrimitiveDisplayMode InCollisionMode, EKyaPrimitiveDisplayMode InEventMode,
    float InOpacity, float InWireThickness, float InDepthBias)
{
    CollisionMode = InCollisionMode; EventMode = InEventMode;
    SolidOpacity = FMath::Clamp(InOpacity, 0.05f, 1.0f);
    WireThickness = FMath::Clamp(InWireThickness, 0.5f, 6.0f);
    WireDepthBias = FMath::Clamp(InDepthBias, 0.0f, 10.0f);
#if WITH_EDITOR
    if (CollisionMode != EKyaPrimitiveDisplayMode::Wireframe || EventMode != EKyaPrimitiveDisplayMode::Wireframe)
    {
        static TWeakObjectPtr<UMaterial> SharedMaterial;
        SolidMaterial = SharedMaterial.Get();
        if (!SolidMaterial)
        {
            SolidMaterial = NewObject<UMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
            SolidMaterial->SetShadingModel(MSM_Unlit);
            SolidMaterial->BlendMode = BLEND_Translucent;
            SolidMaterial->TwoSided = true;
            SolidMaterial->bEnableResponsiveAA = true;
            auto* Color = NewObject<UMaterialExpressionVertexColor>(SolidMaterial);
            SolidMaterial->GetExpressionCollection().AddExpression(Color);
            SolidMaterial->GetEditorOnlyData()->EmissiveColor.Connect(0, Color);
            SolidMaterial->GetEditorOnlyData()->Opacity.Connect(4, Color);
            SolidMaterial->PostEditChange();
            SharedMaterial = SolidMaterial;
        }
    }
#endif
    MarkRenderStateDirty();
}

UKyaPrimitiveComponent::UKyaPrimitiveComponent()
{
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetGenerateOverlapEvents(false);
    CastShadow = false;
    bIsEditorOnly = true;
    SetHiddenInGame(true);
}

void UKyaPrimitiveComponent::Rebuild(UKyaLevelBank* InBank, bool bCollision, bool bEvents)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Kya_PrimitiveComponent_Rebuild);
    SourceBank = InBank;
    ++RecordGeneration;
    Records.Reset(); Diagnostics.Reset();
    SelectedRecord = INDEX_NONE; SelectedSource.Reset();
    if (SourceBank) KyaPrimitives::Decode(*SourceBank, bCollision, bEvents, Records, Diagnostics);
    PrimitiveCount = Records.Num();
    UpdateBounds(); MarkRenderTransformDirty(); MarkRenderStateDirty();
}

void UKyaPrimitiveComponent::SelectRecord(int32 Index)
{
    SelectedRecord = Records.IsValidIndex(Index) ? Index : INDEX_NONE;
    SelectedSource.Reset();
    if (SelectedRecord != INDEX_NONE)
    {
        const FKyaPrimitiveRecord& R = Records[SelectedRecord];
        SelectedSource = FString::Printf(TEXT("Entry %d, record +0x%X, transform +0x%X, zone %d, group %d, primitive %d, flags 0x%08X"),
            R.EntryIndex, R.SourceOffset, R.TransformOffset, R.ZoneIndex, R.GroupIndex, R.PrimitiveIndex, R.Flags);
    }
    MarkRenderStateDirty();
}

FPrimitiveSceneProxy* UKyaPrimitiveComponent::CreateSceneProxy()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Kya_PrimitiveComponent_CreateSceneProxy);
    return Records.IsEmpty() ? nullptr : new FKyaPrimitiveProxy(this);
}

void UKyaPrimitiveComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
    if (SolidMaterial) OutMaterials.Add(SolidMaterial);
}

FBoxSphereBounds UKyaPrimitiveComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Kya_PrimitiveComponent_CalcBounds);
    FBox LocalBounds(ForceInit);
    for (const FKyaPrimitiveRecord& R : Records)
    {
        TArray<FVector> Edges;
        KyaPrimitives::BuildEdges(R, Edges);
        for (const FVector& P : Edges) LocalBounds += KyaActorTransform::SwapGameAxes(P) * 100.0;
    }
    if (!LocalBounds.IsValid) LocalBounds = FBox(FVector(-1), FVector(1));
    return FBoxSphereBounds(LocalBounds).TransformBy(LocalToWorld);
}
