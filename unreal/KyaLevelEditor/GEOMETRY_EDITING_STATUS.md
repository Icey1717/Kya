# Sector geometry editing status

The approved plan is partially implemented. The workflow supports native static
collision and a subset of prepared render root clusters. General hierarchy/LOD
resources, nested render spatial trees and additional UV layer-stride layouts are
not implemented. Unsupported resources have diagnostics and disabled Apply.
Full running-game visual/traversal acceptance remains outstanding.

## Workflow

Select a level preview and use **Edit Sector Geometry** or **Edit Sector Collision**
under its sector controls. Both actions join one bank session and create transient
Dynamic Mesh actors. Modeling Mode tools edit selection, movement, sculpting,
topology, normals and UVs. Tool Accept updates working data. **Apply to Kya Bank**
validates every changed resource and commits one bank transaction; **Discard
Session** removes temporary objects. Undo/redo restores bank data and previews.

Working actor Details show resource identity, affected preview placements, native
support and status. Component wireframe controls and Outliner visibility are
independent. **Show Normals** draws guides for up to 10,000 triangles per object.

New/remeshed faces require **Assign Properties To New Faces**. Render faces use
their strip's existing material slot; collision faces use **Surface Flags**.
**Set Selected Collision Surface Flags** changes only **Surface Flag Mask** bits,
preserving all others. The default mask is the verified 0x80000000 detailed
contact-record bit, explained beside the raw value.

With render faces selected in Modeling Mode, explicitly choose Surface Flags and
enable Assign Properties To New Faces, then **Copy Selected Faces to Collision
(Append)**. Exactly one collision working resource must be open. Every invocation
appends another copy for preview, with undo/redo; bank bytes stay unchanged until
Apply. Reimport/replacement invalidates stale sessions.

## Data and encoding

- Keys use bank identity, entry/resource ordinals, LOD and generation. Offsets are
  provenance only. Render resources share an immutable entry snapshot.
- Corner UVs, colours, normals, native vertex flags and source provenance preserve
  seams/hard boundaries without automatic position welding.
- Collision retains shared vertices, raw flags, untouched quads and referenced
  boxes/spheres. Changed quad topology emits triangles. Box/sphere editing is rejected.
- Deterministic collision trees use at most 255 contiguous primitives per leaf,
  with a conservative 100-leaf limit for native ray queues.
- Prepared root-strip encoding rebuilds positions, attributes, GIF/VIF sections,
  relocations and bounds using the verified 96-vertex static DMA budget.
  Animation/skinning and unsupported packet/UV layouts are rejected.
- Variable-size bank replacement appends aligned payloads and updates entry tables,
  terminal records and size accounting. Unrelated entry locations/bytes remain
  intact. No-op replacements are byte-identical. Old payload storage is retained,
  so repeated applies can grow the bank.
- Encoding/decode-back and complete staged-bank validation precede mutation.
  Persisted resource provenance requests repeat validation before export publication.
  Applied bytes/provenance save with the bank; reimport resets provenance.
  Temporary meshes never become authoritative assets.

## Native reference

The user authorized verified repository src code instead of Ghidra. No Ghidra
comparison or address verification is claimed. Compared source symbols:

- edCollisions.cpp: edColLoadStatic, edColIntersectRayUnitBoxUnit,
  edColIntersectRayUnitSphereUnit; edCollisions.h and OBBTree.h layouts.
  Boxes use unit half extents 0.5; spheres use radius 1.
- OBBTree.cpp: CheckRayObbTreeIntersection, edObbIntersectObbTreeRayPrim:
  node axes/centres, half extents, seven child slots, contiguous leaf ranges,
  and fixed ray queues of 100 nodes.
- edBankFile.h/.cpp: native offsets, get_entry_data, get_entry, unpack.
- src/b-witch/ed3D.h/.cpp: prepared strip/chunk layouts, ed3DPreparePointer,
  ed3DFlushStripGetIncPacket, ed3DSceneRenderCluster, ed3DRenderCluster and
  ed3DPrepareClusterALL; port/KyaMesh/src/Mesh.cpp for runtime consumption.
  No game runtime behavior was changed.

## Verification and remaining work

Automation group: **Kya.Geometry** in the existing UnrealBankTest host. Covers
validation, seams, flags, transforms, copy append, collision decode-back, leaf
splitting, deterministic trees, tree rays versus brute force, render packet
splitting, empty meshes, shared sessions, undo/redo, export and asset save/reopen.
The Modeling Mode selection test exercises the actual copy command and undo/redo.

Set KYA_TEST_GEOMETRY_BANK_DIR for disposable in-memory real-bank roundtrips.
The initial run rebuilt/reopened 125 collision entries and edited/reopened 17
render entries; unsupported/invalid resources were rejected with diagnostics.

Set KYA_GEOMETRY_EXPORT_BANK to write the shared-session fixture from Unreal,
then run KyaPortTest.exe --gtest_filter=RunningPortExport.ValidatesEditedSectorGeometry
with the same variable. This test passed using actual PC port bank structures,
G3D relocation, mesh preprocessing, collision installation and native OBB ray queries.

Remaining before full acceptance: hierarchy/LOD authoring, nested render spatial
trees, broader packet/UV formats, material-slot changes across a resource,
representative shape-query coverage, and running-game visual/distance/traversal
verification. No full-plan completion is claimed.
