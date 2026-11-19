#include "KyaGeometryWorkingActor.h"
#include "KyaGeometryEncoding.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "KyaActorEditService.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "Editor.h"
#include "EditorModeManager.h"
#include "ModelingToolsEditorMode.h"
#include "Selection/GeometrySelectionManager.h"
#include "InteractiveToolsContext.h"
#include "Tools/EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "ToolContextInterfaces.h"
#include "Changes/MeshReplacementChange.h"
#include "EngineUtils.h"
#include "Misc/MessageDialog.h"
#include "ScopedTransaction.h"
#include "DrawDebugHelpers.h"

using namespace UE::Geometry;

namespace
{
FVector GameToMesh(const FVector& V) { return FVector(V.X, V.Z, V.Y) * 100; }
FVector MeshToGame(const FVector& V) { return FVector(V.X, V.Z, V.Y) / 100; }
bool ActiveGeometryTool()
{
    UModelingToolsEditorMode* Mode = Cast<UModelingToolsEditorMode>(GLevelEditorModeTools().GetActiveScriptableMode(UModelingToolsEditorMode::EM_ModelingToolsEditorModeId));
    return Mode && Mode->GetInteractiveToolsContext() && Mode->GetInteractiveToolsContext()->ToolManager->HasActiveTool(EToolSide::Left);
}
void HighlightGeometry(const AKyaGeometryWorkingActor* Actor, const TArray<int32>& Triangles)
{
    UModelingToolsEditorMode* Mode = Cast<UModelingToolsEditorMode>(GLevelEditorModeTools().GetActiveScriptableMode(UModelingToolsEditorMode::EM_ModelingToolsEditorModeId));
    if (!Mode || !Mode->GetSelectionManager() || Triangles.IsEmpty()) return;
    FGeometrySelection Selection;
    for (int32 T : Triangles) Selection.Selection.Add(FGeoSelectionID::MeshTriangle(T).Encoded());
    Mode->GetSelectionManager()->SetMeshSelectionTypeAndMode(EGeometryElementType::Face, UGeometrySelectionManager::EMeshTopologyMode::Triangle, false);
    Mode->GetSelectionManager()->SetSelectionForComponent(Actor->GetDynamicMeshComponent(), Selection);
}
TArray<AKyaGeometryWorkingActor*> SessionActors(AKyaGeometryWorkingActor* Actor)
{
    TArray<AKyaGeometryWorkingActor*> Result;
    for (TActorIterator<AKyaGeometryWorkingActor> It(Actor->GetWorld()); It; ++It)
        if (It->SessionId == Actor->SessionId && It->Bank == Actor->Bank) Result.Add(*It);
    return Result;
}
bool SameGeometry(const FKyaGeometryDocument& A, const FKyaGeometryDocument& B)
{
    if (A.Faces.Num() != B.Faces.Num()) return false;
    TSet<int32> Seen;
    for (const FKyaGeometryFace& Face : A.Faces)
    {
        if (!B.Faces.IsValidIndex(Face.SourceFaceIndex) || Seen.Contains(Face.SourceFaceIndex)) return false;
        Seen.Add(Face.SourceFaceIndex);
        const FKyaGeometryFace& Original = B.Faces[Face.SourceFaceIndex];
        if (Face.Corners.Num() != Original.Corners.Num() || Face.MaterialIndex != Original.MaterialIndex || Face.SurfaceFlags != Original.SurfaceFlags) return false;
        for (int32 I = 0; I < Face.Corners.Num(); ++I)
        {
            const FKyaGeometryCorner& X = Face.Corners[I]; const FKyaGeometryCorner& Y = Original.Corners[I];
            if (FVector3f(A.Vertices[X.VertexIndex]) != FVector3f(B.Vertices[Y.VertexIndex]) || X.NativeVertexFlags != Y.NativeVertexFlags) return false;
            if (B.bRequiresColors && X.Color != Y.Color) return false;
            if (B.bRequiresUVs && X.UV != Y.UV) return false;
            if (B.bRequiresNormals && X.Normal != Y.Normal) return false;
        }
    }
    return true;
}
}

AKyaGeometryWorkingActor::AKyaGeometryWorkingActor(const FObjectInitializer& Initializer) : Super(Initializer)
{
    PrimaryActorTick.bCanEverTick = true;
    SetFlags(RF_Transient | RF_Transactional);
    bIsEditorOnlyActor = true;
    GetDynamicMeshComponent()->bExplicitShowWireframe = true;
}

void AKyaGeometryWorkingActor::Report(const FString& Message)
{
    Status = Message;
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
}

bool AKyaGeometryWorkingActor::CanApplyNow() const
{
    return Source.NativeFormatDiagnostic.IsEmpty() && Source.Key.IsCurrent() && !ActiveGeometryTool();
}

bool AKyaGeometryWorkingActor::InitializeDocument(const FKyaGeometryDocument& Document, FString& Error)
{
    Source = Document; Bank = Document.Key.Bank.Get();
    ResourceIdentity = FString::Printf(TEXT("Entry %d, resource %d, LOD %d, generation %d"), Document.Key.EntryIndex, Document.Key.ResourceIndex, Document.Key.LodIndex, Document.Key.Generation);
    NativeFormatStatus = Document.NativeFormatDiagnostic.IsEmpty() ? TEXT("Native writer available; Apply validates edited geometry and staged bank bytes.") : Document.NativeFormatDiagnostic;
    GetDynamicMeshComponent()->WireframeColor = Document.Key.Kind == EKyaGeometryKind::Collision ? FLinearColor(1, .5f, 0) : FLinearColor(0, .7f, 1);
    OriginalTriangles.Reset(); NativeCorners.Reset(); OriginalFaces.Reset(); OriginalVertices.Reset();
    FDynamicMesh3 Mesh; Mesh.EnableTriangleGroups(); Mesh.EnableAttributes(); Mesh.Attributes()->EnablePrimaryColors();
    auto* UV = Mesh.Attributes()->PrimaryUV(); auto* Normals = Mesh.Attributes()->PrimaryNormals(); auto* Colors = Mesh.Attributes()->PrimaryColors();
    for (int32 I = 0; I < Document.Vertices.Num(); ++I) OriginalVertices.Add(Mesh.AppendVertex(GameToMesh(Document.Vertices[I])), I);
    for (int32 FaceIndex = 0; FaceIndex < Document.Faces.Num(); ++FaceIndex)
    {
        const FKyaGeometryFace& Face = Document.Faces[FaceIndex];
        for (int32 J = 1; J + 1 < Face.Corners.Num(); ++J)
        {
            const FIntVector Corners(0, J, J + 1);
            FIndex3i Triangle(Face.Corners[0].VertexIndex, Face.Corners[J + 1].VertexIndex, Face.Corners[J].VertexIndex);
            int32 Id = Mesh.AppendTriangle(Triangle, FaceIndex + 1);
            if (Id < 0)
            {
                // Dynamic Mesh forbids nonmanifold edges and duplicate triangles.
                // Split only that target face, retaining native source identities.
                for (int32 K = 0; K < 3; ++K)
                {
                    const int32 Native = OriginalVertices[Triangle[K]];
                    Triangle[K] = Mesh.AppendVertex(GameToMesh(Document.Vertices[Native])); OriginalVertices.Add(Triangle[K], Native);
                }
                Id = Mesh.AppendTriangle(Triangle, FaceIndex + 1);
            }
            if (Id < 0) { Error = TEXT("Dynamic Mesh could not represent this native face."); return false; }
            OriginalTriangles.Add(Id, FIntVector(Triangle.A, Triangle.B, Triangle.C)); NativeCorners.Add(Id, Corners); OriginalFaces.Add(Id, FaceIndex);
            FIndex3i UVs, Ns, Cs;
            const FVector FaceNormal = FVector::CrossProduct(Document.Vertices[Face.Corners[J].VertexIndex] - Document.Vertices[Face.Corners[0].VertexIndex],
                Document.Vertices[Face.Corners[J + 1].VertexIndex] - Document.Vertices[Face.Corners[0].VertexIndex]).GetSafeNormal();
            for (int32 K = 0; K < 3; ++K)
            {
                const FKyaGeometryCorner& Corner = Face.Corners[Corners[K == 0 ? 0 : 3 - K]];
                const FVector N = Corner.Normal.IsSet() ? FVector(Corner.Normal.GetValue()) : FaceNormal;
                UVs[K] = UV->AppendElement(Corner.UV.Get(FVector2f::ZeroVector));
                Ns[K] = Normals->AppendElement(FVector3f(N.X, N.Z, N.Y));
                const FColor Color = Corner.Color.Get(FColor::White);
                Cs[K] = Colors->AppendElement(FVector4f(Color.R / 255.f, Color.G / 255.f, Color.B / 255.f, Color.A / 255.f));
            }
            UV->SetTriangle(Id, UVs); Normals->SetTriangle(Id, Ns); Colors->SetTriangle(Id, Cs);
        }
    }
    GetDynamicMeshComponent()->GetDynamicMesh()->SetMesh(MoveTemp(Mesh));
    Status = TEXT("Working copy. Accept Modeling Mode tools, then Apply to Kya Bank. Other LODs are unchanged.");
    return true;
}

bool AKyaGeometryWorkingActor::ReadDocument(FKyaGeometryDocument& Document, FString& Error) const
{
    if (!KyaGeometry::MatchesSource(Source) || !Preview) { Error = TEXT("Session is stale after reimport or bank replacement. Discard and reopen it."); return false; }
    FKyaGeometryDocument Result = Source; Result.Vertices.Reset(); Result.Faces.Reset(); bool bValid = true;
    TMap<int32, int32> FaceTriangles; TArray<int32> VertexIds;
    const FTransform ToPreview = GetActorTransform().GetRelativeTransform(Preview->GetActorTransform());
    const FMatrix NormalTransform = ToPreview.ToMatrixWithScale().Inverse().GetTransposed();
    const bool Reverse = ToPreview.ToMatrixWithScale().Determinant() >= 0;
    GetDynamicMeshComponent()->GetDynamicMesh()->ProcessMesh([&](const FDynamicMesh3& Mesh)
    {
        TMap<int32, int32> Vertices;
        for (int32 V : Mesh.VertexIndicesItr()) { Vertices.Add(V, Result.Vertices.Add(MeshToGame(ToPreview.TransformPosition(FVector(Mesh.GetVertex(V)))))); VertexIds.Add(V); }
        TSet<int32> PreservedQuads;
        if (Source.Key.Kind == EKyaGeometryKind::Collision)
            for (int32 I = 0; I < Source.Faces.Num(); ++I)
            {
                if (Source.Faces[I].Corners.Num() != 4) continue;
                int32 Count = 0, FirstTriangle = INDEX_NONE; bool Unchanged = Reverse; TOptional<int64> Override;
                for (const auto& Pair : OriginalFaces)
                    if (Pair.Value == I)
                    {
                        ++Count;
                        FirstTriangle = Pair.Key;
                        if (!Mesh.IsTriangle(Pair.Key)) { Unchanged = false; break; }
                        const FIndex3i Triangle = Mesh.GetTriangle(Pair.Key); const FIntVector Expected = OriginalTriangles[Pair.Key];
                        if (Triangle.A != Expected.X || Triangle.B != Expected.Y || Triangle.C != Expected.Z) { Unchanged = false; break; }
                        const int64 Flags = SurfaceOverrides.Contains(Pair.Key) ? SurfaceOverrides[Pair.Key] : int64(Source.Faces[I].SurfaceFlags.GetValue());
                        if (Override.IsSet() && Override.GetValue() != Flags) { Unchanged = false; break; }
                        Override = Flags;
                        for (int32 J = 0; J < 3; ++J)
                        {
                            const int32 Native = OriginalVertices[Triangle[J]];
                            if (!Mesh.IsVertex(Native) || Mesh.GetVertex(Native) != Mesh.GetVertex(Triangle[J])) Unchanged = false;
                        }
                    }
                if (!Unchanged || Count != 2) continue;
                FKyaGeometryFace Quad = Source.Faces[I]; Quad.SourceFaceIndex = I; Quad.SurfaceFlags = uint32(Override.GetValue());
                for (FKyaGeometryCorner& Corner : Quad.Corners)
                { Corner.VertexIndex = Vertices[Corner.VertexIndex]; Corner.SourceVertexIndex = Corner.VertexIndex; }
                FaceTriangles.Add(Result.Faces.Num(), FirstTriangle); Result.Faces.Add(MoveTemp(Quad)); PreservedQuads.Add(I);
            }
        for (int32 T : Mesh.TriangleIndicesItr())
        {
            const FIndex3i Triangle = Mesh.GetTriangle(T); const FIntVector* Original = OriginalTriangles.Find(T);
            const bool Provenance = Original && Original->X == Triangle.A && Original->Y == Triangle.B && Original->Z == Triangle.C && OriginalFaces.Contains(T) &&
                Mesh.GetTriangleGroup(T) == OriginalFaces[T] + 1;
            if (!Provenance && !bAssignPropertiesToNewFaces)
            { HighlightGeometry(this, {T}); Error = TEXT("New or remeshed faces require an explicit property choice. Enable Assign Properties To New Faces (render uses the original material; collision uses Surface Flags)."); bValid = false; return; }
            const int32 FaceIndex = Provenance ? OriginalFaces[T] : INDEX_NONE;
            if (PreservedQuads.Contains(FaceIndex)) continue;
            FKyaGeometryFace Face;
            Face.SourceFaceIndex = FaceIndex; Face.SurfaceFlags = Provenance ? Source.Faces[FaceIndex].SurfaceFlags : TOptional<uint32>(uint32(SurfaceFlags));
            Face.MaterialIndex = Provenance ? Source.Faces[FaceIndex].MaterialIndex : (Source.Faces.IsEmpty() ? TOptional<int32>() : Source.Faces[0].MaterialIndex);
            if (const int64* Flags = SurfaceOverrides.Find(T)) Face.SurfaceFlags = uint32(*Flags);
            const auto* Attributes = Mesh.Attributes();
            if (Source.Key.Kind == EKyaGeometryKind::Render && (!Attributes || !Attributes->PrimaryUV() || !Attributes->PrimaryUV()->IsSetTriangle(T) ||
                !Attributes->PrimaryNormals() || !Attributes->PrimaryNormals()->IsSetTriangle(T) || !Attributes->PrimaryColors() || !Attributes->PrimaryColors()->IsSetTriangle(T)))
            { Error = TEXT("Modeling result is missing native corner UV, normal or colour attributes."); bValid = false; return; }
            for (int32 J = 0; J < 3; ++J)
            {
                const int32 K = Reverse && J > 0 ? 3 - J : J;
                FKyaGeometryCorner Corner;
                if (Provenance) Corner = Source.Faces[FaceIndex].Corners[NativeCorners[T][(!Reverse && J > 0) ? 3 - J : J]];
                else if (!Source.Faces.IsEmpty())
                {
                    const int32 Group = Mesh.GetTriangleGroup(T) - 1;
                    const FKyaGeometryFace& Inherit = Source.Faces[Source.Faces.IsValidIndex(Group) ? Group : 0];
                    Corner.NativeVertexFlags = Inherit.Corners[0].NativeVertexFlags;
                }
                Corner.VertexIndex = Vertices[Triangle[K]];
                if (Source.Key.Kind == EKyaGeometryKind::Render)
                {
                    Corner.UV = Attributes->PrimaryUV()->GetElement(Attributes->PrimaryUV()->GetTriangle(T)[K]);
                    FVector N = FVector(NormalTransform.TransformVector(FVector(Attributes->PrimaryNormals()->GetElement(Attributes->PrimaryNormals()->GetTriangle(T)[K]))));
                    if (!ToPreview.GetScale3D().Equals(FVector::OneVector)) N.Normalize();
                    Corner.Normal = FVector3f(N.X, N.Z, N.Y);
                    const FVector4f C = Attributes->PrimaryColors()->GetElement(Attributes->PrimaryColors()->GetTriangle(T)[K]);
                    Corner.Color = FLinearColor(C.X, C.Y, C.Z, C.W).ToFColor(false);
                }
                Face.Corners.Add(Corner);
            }
            FaceTriangles.Add(Result.Faces.Num(), T); Result.Faces.Add(MoveTemp(Face));
        }
    });
    if (!bValid) return false;
    TArray<FKyaGeometryIssue> Issues;
    if (!KyaGeometry::Validate(Result, Issues))
    {
        const FKyaGeometryIssue* Issue = Issues.FindByPredicate([](const FKyaGeometryIssue& I) { return I.bError; });
        if (const int32* T = FaceTriangles.Find(Issue->FaceIndex)) HighlightGeometry(this, {*T});
        else if (VertexIds.IsValidIndex(Issue->VertexIndex))
        {
            TArray<int32> Triangles;
            GetDynamicMeshComponent()->GetDynamicMesh()->ProcessMesh([&](const FDynamicMesh3& Mesh) {
                for (int32 T : Mesh.TriangleIndicesItr()) if (Mesh.GetTriangle(T).Contains(VertexIds[Issue->VertexIndex])) Triangles.Add(T);
            });
            HighlightGeometry(this, Triangles);
        }
        Error = FString::Printf(TEXT("Face %d, vertex %d: %s"), Issue->FaceIndex, Issue->VertexIndex, *Issue->Message); return false;
    }
    Document = MoveTemp(Result); return true;
}

bool AKyaGeometryWorkingActor::SelectedTriangles(TArray<int32>& Triangles, FString& Error) const
{
    UModelingToolsEditorMode* Mode = Cast<UModelingToolsEditorMode>(GLevelEditorModeTools().GetActiveScriptableMode(UModelingToolsEditorMode::EM_ModelingToolsEditorModeId));
    FGeometrySelection Selection;
    if (!Mode || !Mode->GetSelectionManager() || !Mode->GetSelectionManager()->GetSelectionForComponent(GetDynamicMeshComponent(), Selection) || Selection.ElementType != EGeometryElementType::Face)
    { Error = TEXT("Select faces on this working mesh with Modeling Mode's mesh element selection."); return false; }
    GetDynamicMeshComponent()->GetDynamicMesh()->ProcessMesh([&](const FDynamicMesh3& Mesh)
    {
        for (uint64 Encoded : Selection.Selection)
        {
            const FGeoSelectionID Id(Encoded);
            if (Selection.TopologyType == EGeometryTopologyType::Triangle)
            { if (Mesh.IsTriangle(Id.GeometryID)) Triangles.AddUnique(Id.GeometryID); }
            else for (int32 T : Mesh.TriangleIndicesItr()) if (Mesh.GetTriangleGroup(T) == int32(Id.TopologyID)) Triangles.AddUnique(T);
        }
    });
    Triangles.Sort();
    if (Triangles.IsEmpty()) { Error = TEXT("No working mesh faces are selected."); return false; }
    return true;
}

void AKyaGeometryWorkingActor::SetSelectedCollisionSurfaceFlags()
{
    if (Source.Key.Kind != EKyaGeometryKind::Collision) { Report(TEXT("Select a collision working object.")); return; }
    TArray<int32> Triangles; FString Error; if (!SelectedTriangles(Triangles, Error)) { Report(Error); return; }
    const FScopedTransaction Transaction(FText::FromString(TEXT("Set Kya Collision Surface Flags"))); Modify();
    for (int32 T : Triangles)
    {
        const uint32 Before = SurfaceOverrides.Contains(T) ? uint32(SurfaceOverrides[T]) :
            (OriginalFaces.Contains(T) ? Source.Faces[OriginalFaces[T]].SurfaceFlags.GetValue() : uint32(SurfaceFlags));
        SurfaceOverrides.Add(T, (Before & ~uint32(SurfaceFlagMask)) | (uint32(SurfaceFlags) & uint32(SurfaceFlagMask)));
    }
    Status = FString::Printf(TEXT("Surface flags 0x%08x assigned to %d working triangles."), uint32(SurfaceFlags), Triangles.Num());
}

void AKyaGeometryWorkingActor::ApplyToKyaBank()
{
    if (ActiveGeometryTool()) { Report(TEXT("Accept or cancel the active Modeling Mode tool before applying.")); return; }
    TMap<int32, TArray<FKyaGeometryDocument>> Entries;
    FString Error;
    for (AKyaGeometryWorkingActor* Actor : SessionActors(this))
    {
        FKyaGeometryDocument Document;
        if (!Actor->ReadDocument(Document, Error)) { Report(Error); return; }
        if (SameGeometry(Document, Actor->Source)) continue;
        if (!Document.NativeFormatDiagnostic.IsEmpty()) { Report(Document.NativeFormatDiagnostic); return; }
        Entries.FindOrAdd(Document.Key.EntryIndex).Add(MoveTemp(Document));
    }
    TMap<int32, TArray<uint8>> Replacements;
    if (Entries.IsEmpty()) { DiscardSession(); return; }
    for (const auto& Pair : Entries)
    {
        TArray<uint8> Bytes;
        const bool Collision = Pair.Value[0].Key.Kind == EKyaGeometryKind::Collision;
        if (!(Collision ? Pair.Value.Num() == 1 && KyaGeometry::EncodeCollision(Pair.Value[0], Bytes, Error) : KyaGeometry::EncodeRootRender(Pair.Value, Bytes, Error)))
        { Report(Error); return; }
        Replacements.Add(Pair.Key, MoveTemp(Bytes));
    }
    TArray<uint8> Bytes; TArray<FKyaBankEntry> NewEntries;
    if (!Bank || !KyaGeometry::ReplaceBankEntries(*Bank, Replacements, Bytes, NewEntries, Error)) { Report(Error); return; }
    TArray<FKyaAppliedGeometryResource> Applied = Bank->AppliedGeometryResources;
    for (const auto& Pair : Entries)
        for (const FKyaGeometryDocument& Document : Pair.Value)
            if (!Applied.ContainsByPredicate([&](const FKyaAppliedGeometryResource& Resource) {
                return Resource.EntryIndex == Document.Key.EntryIndex && Resource.ResourceIndex == Document.Key.ResourceIndex &&
                    Resource.bCollision == (Document.Key.Kind == EKyaGeometryKind::Collision);
            }))
            {
                FKyaAppliedGeometryResource Resource; Resource.EntryIndex = Document.Key.EntryIndex;
                Resource.ResourceIndex = Document.Key.ResourceIndex; Resource.bCollision = Document.Key.Kind == EKyaGeometryKind::Collision;
                Applied.Add(Resource);
            }
    UKyaLevelBank* Staged = NewObject<UKyaLevelBank>(); Staged->BankBytes = Bytes; Staged->Entries = NewEntries; Staged->AppliedGeometryResources = Applied;
    if (!KyaGeometry::ValidateAppliedGeometry(*Staged, Bytes, Error)) { Report(Error); return; }
    {
        const FScopedTransaction Transaction(FText::FromString(TEXT("Apply Kya Sector Geometry")));
        Bank->SetFlags(RF_Transactional); Bank->Modify(); Bank->BankBytes = MoveTemp(Bytes); Bank->Entries = MoveTemp(NewEntries);
        Bank->AppliedGeometryResources = MoveTemp(Applied);
        Bank->BumpEditGeneration(); Bank->MarkPackageDirty();
    }
    FKyaLevelEditService::Get().OnBankEdited.Broadcast(Bank);
    FKyaLevelEditService::Get().NotifyBankChanged(Bank, true);
    DiscardSession();
}

void AKyaGeometryWorkingActor::DiscardSession()
{
    if (ActiveGeometryTool()) { Report(TEXT("Accept or cancel the active Modeling Mode tool before discarding.")); return; }
    for (AKyaGeometryWorkingActor* Actor : SessionActors(this)) Actor->Destroy();
}

void AKyaGeometryWorkingActor::CopySelectedFacesToCollision()
{
    if (ActiveGeometryTool()) { Report(TEXT("Accept or cancel the active Modeling Mode tool before copying faces.")); return; }
    if (Source.Key.Kind != EKyaGeometryKind::Render || !bAssignPropertiesToNewFaces)
    { Report(TEXT("Select a render working mesh and explicitly choose Surface Flags by enabling Assign Properties To New Faces.")); return; }
    AKyaGeometryWorkingActor* Target = nullptr;
    for (AKyaGeometryWorkingActor* Actor : SessionActors(this))
        if (Actor->Source.Key.Kind == EKyaGeometryKind::Collision)
        { if (Target) { Report(TEXT("Copy requires a single collision working resource in this session.")); return; } Target = Actor; }
    if (!Target) { Report(TEXT("Open this sector's collision working resource before copying faces.")); return; }
    TArray<int32> Selected; FString Error; if (!SelectedTriangles(Selected, Error)) { Report(Error); return; }
    FKyaGeometryDocument Render, Collision;
    if (!ReadDocument(Render, Error) || !Target->ReadDocument(Collision, Error)) { Report(Error); return; }
    TMap<int32, int32> Indices; int32 Index = 0;
    GetDynamicMeshComponent()->GetDynamicMesh()->ProcessMesh([&](const FDynamicMesh3& Mesh) { for (int32 T : Mesh.TriangleIndicesItr()) Indices.Add(T, Index++); });
    for (int32& T : Selected) T = Indices[T];
    const int32 FirstNewFace = Collision.Faces.Num();
    if (!KyaGeometry::AppendFacesToCollision(Render, Selected, FMatrix::Identity, uint32(SurfaceFlags), Collision, Error)) { Report(Error); return; }
    TSharedPtr<FDynamicMesh3, ESPMode::ThreadSafe> Before = MakeShared<FDynamicMesh3, ESPMode::ThreadSafe>();
    Target->GetDynamicMeshComponent()->GetDynamicMesh()->ProcessMesh([&](const FDynamicMesh3& Mesh) { *Before = Mesh; });
    TSharedPtr<FDynamicMesh3, ESPMode::ThreadSafe> After = MakeShared<FDynamicMesh3, ESPMode::ThreadSafe>(*Before);
    const FTransform ToPreview = Target->GetActorTransform().GetRelativeTransform(Preview->GetActorTransform());
    const bool Reverse = ToPreview.ToMatrixWithScale().Determinant() >= 0;
    TMap<int32, int32> VertexMap; TArray<int32> AddedTriangles;
    for (int32 I = FirstNewFace; I < Collision.Faces.Num(); ++I)
    {
        FIndex3i Triangle;
        for (int32 J = 0; J < 3; ++J)
        {
            const int32 V = Collision.Faces[I].Corners[Reverse && J > 0 ? 3 - J : J].VertexIndex;
            int32* Existing = VertexMap.Find(V);
            const int32 Id = Existing ? *Existing : After->AppendVertex(ToPreview.InverseTransformPosition(GameToMesh(Collision.Vertices[V])));
            if (!Existing) VertexMap.Add(V, Id); Triangle[J] = Id;
        }
        const int32 T = After->AppendTriangle(Triangle, 0);
        if (T < 0) { Report(TEXT("Copied triangles cannot be represented by the collision working mesh.")); return; }
        AddedTriangles.Add(T);
        const FVector3f N = FVector3f(FVector::CrossProduct(FVector(After->GetVertex(Triangle.B) - After->GetVertex(Triangle.A)),
            FVector(After->GetVertex(Triangle.C) - After->GetVertex(Triangle.A))).GetSafeNormal());
        FIndex3i Ns, UVs, Cs;
        for (int32 J = 0; J < 3; ++J)
        {
            Ns[J] = After->Attributes()->PrimaryNormals()->AppendElement(N);
            UVs[J] = After->Attributes()->PrimaryUV()->AppendElement(FVector2f::ZeroVector);
            Cs[J] = After->Attributes()->PrimaryColors()->AppendElement(FVector4f(1, .5f, 0, 1));
        }
        After->Attributes()->PrimaryNormals()->SetTriangle(T, Ns); After->Attributes()->PrimaryUV()->SetTriangle(T, UVs); After->Attributes()->PrimaryColors()->SetTriangle(T, Cs);
    }
    {
        const FScopedTransaction Transaction(FText::FromString(TEXT("Copy Kya Faces to Collision")));
        Target->Modify(); Target->bAssignPropertiesToNewFaces = true;
        for (int32 T : AddedTriangles) Target->SurfaceOverrides.Add(T, SurfaceFlags);
        TUniquePtr<FMeshReplacementChange> Change = MakeUnique<FMeshReplacementChange>(Before, After);
        Change->Apply(Target->GetDynamicMeshComponent());
        UModelingToolsEditorMode* Mode = CastChecked<UModelingToolsEditorMode>(GLevelEditorModeTools().GetActiveScriptableMode(UModelingToolsEditorMode::EM_ModelingToolsEditorModeId));
        Mode->GetSelectionManager()->GetTransactionsAPI()->AppendChange(Target->GetDynamicMeshComponent(), MoveTemp(Change), FText::FromString(TEXT("Append Kya Collision Triangles")));
    }
    Target->Status = FString::Printf(TEXT("Appended %d triangles; review the collision preview before Apply."), Selected.Num());
}

void AKyaGeometryWorkingActor::Open(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, bool bCollision)
{
    if (!InPreview || !InBank || ActiveGeometryTool()) return;
    FGuid Session = FGuid::NewGuid();
    for (TActorIterator<AKyaGeometryWorkingActor> It(InPreview->GetWorld()); It; ++It)
        if (It->Bank == InBank && It->Preview == InPreview)
        {
            if (!KyaGeometry::MatchesSource(It->Source)) { It->Report(TEXT("Discard the stale geometry session before reopening this bank.")); return; }
            Session = It->SessionId; break;
        }
    TArray<FKyaGeometryDocument> Documents; FString Error;
    for (int32 I = 0; I < InBank->Entries.Num(); ++I)
    {
        const FKyaBankEntry& Entry = InBank->Entries[I];
        if (Entry.Subtype != 1 || Entry.Type != (bCollision ? 7 : 4)) continue;
        if (bCollision)
        {
            FKyaGeometryDocument Document;
            if (!KyaGeometry::OpenCollision(*InBank, I, Document, Error)) { FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Error)); return; }
            Documents.Add(MoveTemp(Document));
        }
        else
        {
            TArray<FKyaGeometryDocument> Render;
            if (!KyaGeometry::OpenRootRender(*InBank, I, Render, Error)) { FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Error)); return; }
            Documents.Append(MoveTemp(Render));
        }
    }
    if (Documents.IsEmpty()) { FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("No supported sector geometry resources found."))); return; }
    TArray<AKyaGeometryWorkingActor*> Spawned;
    for (const FKyaGeometryDocument& Document : Documents)
    {
        bool Exists = false;
        for (TActorIterator<AKyaGeometryWorkingActor> It(InPreview->GetWorld()); It; ++It)
            if (It->SessionId == Session && It->Source.Key == Document.Key) { Exists = true; break; }
        if (Exists) continue;
        FActorSpawnParameters Params; Params.ObjectFlags = RF_Transient | RF_Transactional;
        AKyaGeometryWorkingActor* Actor = InPreview->GetWorld()->SpawnActor<AKyaGeometryWorkingActor>(Params);
        Actor->SessionId = Session; Actor->Preview = InPreview; Actor->SetActorTransform(InPreview->GetActorTransform());
        if (!Actor->InitializeDocument(Document, Error))
        { Actor->Destroy(); for (AKyaGeometryWorkingActor* Previous : Spawned) Previous->Destroy(); FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Error)); return; }
        Actor->SetActorLabel(FString::Printf(TEXT("Kya %s working: %s entry %d resource %d"), bCollision ? TEXT("collision") : TEXT("render"), *InBank->GetName(), Document.Key.EntryIndex, Document.Key.ResourceIndex));
        TArray<FString> Placements;
        for (TActorIterator<AKyaLevelPreview> It(InPreview->GetWorld()); It; ++It)
            if (It->Bank == InBank || It->SectorBanks.Contains(InBank)) Placements.Add(It->GetActorLabel() + TEXT(" at ") + It->GetActorLocation().ToString());
        Actor->AffectedPlacements = FString::Join(Placements, TEXT("\n"));
        Spawned.Add(Actor);
    }
    if (!Spawned.IsEmpty())
    {
        GEditor->SelectNone(false, true); GEditor->SelectActor(Spawned[0], true, true);
        GLevelEditorModeTools().ActivateMode(UModelingToolsEditorMode::EM_ModelingToolsEditorModeId);
    }
}

void AKyaGeometryWorkingActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Source.Key.IsCurrent()) Status = TEXT("Stale session after reimport or bank replacement. Discard and reopen it.");
    if (!bShowNormals || IsHiddenEd()) return;
    GetDynamicMeshComponent()->GetDynamicMesh()->ProcessMesh([&](const FDynamicMesh3& Mesh)
    {
        int32 Count = 0;
        for (int32 T : Mesh.TriangleIndicesItr())
        {
            if (++Count > 10000) break;
            const FIndex3i V = Mesh.GetTriangle(T);
            const FVector Center = FVector((Mesh.GetVertex(V.A) + Mesh.GetVertex(V.B) + Mesh.GetVertex(V.C)) / 3);
            const FVector N = FVector::CrossProduct(FVector(Mesh.GetVertex(V.B) - Mesh.GetVertex(V.A)), FVector(Mesh.GetVertex(V.C) - Mesh.GetVertex(V.A))).GetSafeNormal();
            DrawDebugLine(GetWorld(), GetActorTransform().TransformPosition(Center), GetActorTransform().TransformPosition(Center + N * 20), FColor::Green, false, -1, 0, 1);
        }
    });
}
