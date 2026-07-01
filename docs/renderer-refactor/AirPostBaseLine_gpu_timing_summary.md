# AirPostBaseLine GPU Timing Summary

- Capture: `C:\Users\icey1\Documents\renderdoc\AirPostBaseLine.rdc`
- Counter requested: `rd.GPUCounter.EventGPUDuration`
- EventGPUDuration available: `yes`
- Counter samples averaged: `10`
- Counter unit: `Seconds`
- Enumerated counter count: `14`
- Flattened action count: `2988`
- Draw action count: `846`

## Total GPU Duration By Top-Level Pass/Marker

| Pass/marker | Timed events | Timed draws | Total draws | Duration us |
|---|---:|---:|---:|---:|
| Native Render | 386 | 383 | 821 | 1594.099 |
| DebugMenu | 11 | 11 | 11 | 136.192 |
| Display List Render | 13 | 13 | 13 | 14.336 |
| Post Processing - Alpha Fix | 1 | 1 | 1 | 9.523 |

## Slowest 20 Draw Events

| Rank | Event ID | Action | Asset | Material | Texture/resource | Indices | Blend | Depth | Duration us |
|---:|---:|---|---|---|---|---:|---|---|---:|
| 1 | 1138 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 23 l: 0) | 23 | GSTexImage Image (128, 32) pallete: 0 (ResourceId::12160) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | 3168 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 118.784 |
| 2 | 6922 | vkCmdDrawIndexed() | Native NEW_BONUS_FLARE.G2D (m: 0 l: 0) | 0 | GSTexImage Image (32, 32) pallete: 0 (ResourceId::9472) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | 30 | rt0:blend=on,mask=RGBA | test=on write=off func=CompareFunction.Greater | 116.634 |
| 3 | 53 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 57 l: 0) | 57 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::12704) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 369 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 97.792 |
| 4 | 69 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 57 l: 0) | 57 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::12704) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 2883 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 55.910 |
| 5 | 7360 | vkCmdDrawIndexed() | (none) | (none) | (none) | 6 |  |  | 54.067 |
| 6 | 1581 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 8 l: 0) | 8 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11920) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 2904 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 53.760 |
| 7 | 7362 | vkCmdDrawIndexed() | (none) | (none) | (none) | 2439 |  |  | 51.814 |
| 8 | 40 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 59 l: 0) | 59 | GSTexImage Image (256, 128) pallete: 0 (ResourceId::12736) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 435 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 48.230 |
| 9 | 2268 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 1 l: 0) | 1 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11808) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 573 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 47.718 |
| 10 | 2276 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 1 l: 0) | 1 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11808) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 936 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 46.490 |
| 11 | 61 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 57 l: 0) | 57 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::12704) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 1134 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 36.250 |
| 12 | 1225 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 6 l: 0) | 6 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11888) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 1086 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 34.611 |
| 13 | 1193 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 6 l: 0) | 6 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11888) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 564 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 30.515 |
| 14 | 1151 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 13 l: 0) | 13 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::12000) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | 441 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 26.419 |
| 15 | 7007 | vkCmdDrawIndexed() | Native BNS_HALO.G2D (m: 0 l: 0) | 0 | GSTexImage Image (64, 64) pallete: 0 (ResourceId::9456) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | 66 | rt0:blend=on,mask=RGBA | test=on write=off func=CompareFunction.Greater | 25.907 |
| 16 | 925 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 2 l: 0) | 2 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11824) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 438 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 22.835 |
| 17 | 77 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 57 l: 0) | 57 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::12704) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 1680 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 21.709 |
| 18 | 7365 | vkCmdDrawIndexed() | (none) | (none) | (none) | 6 |  |  | 19.354 |
| 19 | 5611 | vkCmdDrawIndexed() | Native Dupe (m: 0 l: 0) | 0 | GSTexImage Image (128, 64) pallete: 0 (ResourceId::13792) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 144 | rt0:blend=on,mask=RGBA | test=on write=off func=CompareFunction.Greater | 16.486 |
| 20 | 2388 | vkCmdDrawIndexed() | Native SECT1.g2d (m: 1 l: 0) | 1 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11808) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 927 | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | 14.950 |

## Native Timed Draws By Marker Asset

| Group | Timed draws | Total draws | Indices | Duration us |
|---|---:|---:|---:|---:|
| Native SECT1.g2d (m: 57 l: 0) | 4 | 4 | 6066 | 211.661 |
| Native SECT1.g2d (m: 1 l: 0) | 25 | 25 | 15414 | 185.037 |
| Native SECT1.g2d (m: 23 l: 0) | 11 | 11 | 10755 | 143.155 |
| Native NEW_BONUS_FLARE.G2D (m: 0 l: 0) | 11 | 11 | 5166 | 123.290 |
| Native SECT1.g2d (m: 6 l: 0) | 5 | 5 | 3963 | 80.896 |
| Native SECT1.g2d (m: 8 l: 0) | 6 | 6 | 7989 | 56.115 |
| Native SECT1.g2d (m: 59 l: 0) | 1 | 1 | 435 | 48.230 |
| Native SECT1.g2d (m: 7 l: 0) | 13 | 13 | 48108 | 46.490 |
| Native SECT1.g2d (m: 18 l: 0) | 9 | 9 | 2886 | 44.237 |
| Native SECT1.g2d (m: 2 l: 0) | 7 | 7 | 2904 | 33.075 |
| Native BNS_HALO.G2D (m: 0 l: 0) | 22 | 22 | 450 | 32.358 |
| Native SECT1.g2d (m: 24 l: 0) | 4 | 4 | 9477 | 30.208 |

## Native Timed Draws By Material

| Group | Timed draws | Total draws | Indices | Duration us |
|---|---:|---:|---:|---:|
| 0 | 154 | 154 | 14181 | 257.331 |
| 57 | 4 | 4 | 6066 | 211.661 |
| 1 | 32 | 32 | 16701 | 194.048 |
| 23 | 11 | 11 | 10755 | 143.155 |
| 6 | 6 | 6 | 4050 | 80.998 |
| 8 | 7 | 7 | 8373 | 56.320 |
| 18 | 17 | 17 | 12387 | 52.634 |
| 59 | 1 | 1 | 435 | 48.230 |
| 7 | 13 | 13 | 48108 | 46.490 |
| 2 | 21 | 21 | 3153 | 36.352 |
| 24 | 4 | 4 | 9477 | 30.208 |
| 3 | 16 | 16 | 7440 | 28.160 |

## Native Timed Draws By Texture/Resource

| Group | Timed draws | Total draws | Indices | Duration us |
|---|---:|---:|---:|---:|
| GSTexImage Image (128, 128) pallete: 0 (ResourceId::12704) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 4 | 4 | 6066 | 211.661 |
| GSTexImage Image (128, 128) pallete: 0 (ResourceId::11808) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 25 | 25 | 15414 | 185.037 |
| GSTexImage Image (128, 32) pallete: 0 (ResourceId::12160) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | 11 | 11 | 10755 | 143.155 |
| GSTexImage Image (32, 32) pallete: 0 (ResourceId::9472) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | 11 | 11 | 5166 | 123.290 |
| GSTexImage Image (128, 128) pallete: 0 (ResourceId::11888) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 5 | 5 | 3963 | 80.896 |
| GSTexImage Image (128, 128) pallete: 0 (ResourceId::11920) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 6 | 6 | 7989 | 56.115 |
| GSTexImage Image (256, 128) pallete: 0 (ResourceId::12736) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 1 | 1 | 435 | 48.230 |
| GSTexImage Image (128, 128) pallete: 0 (ResourceId::11904) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 13 | 13 | 48108 | 46.490 |
| GSTexImage Image (128, 64) pallete: 0 (ResourceId::12080) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 9 | 9 | 2886 | 44.237 |
| GSTexImage Image (128, 128) pallete: 0 (ResourceId::11824) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | 7 | 7 | 2904 | 33.075 |
| GSTexImage Image (64, 64) pallete: 0 (ResourceId::9456) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | 22 | 22 | 450 | 32.358 |
| GSTexImage Image (128, 128) pallete: 0 (ResourceId::12176) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | 4 | 4 | 9477 | 30.208 |

## Native Timed Draws By Index Count Bucket

| Group | Timed draws | Total draws | Indices | Duration us |
|---|---:|---:|---:|---:|
| 257-1024 | 111 | 111 | 60120 | 561.357 |
| 1025-4096 | 46 | 46 | 76200 | 454.758 |
| 49-256 | 83 | 83 | 12381 | 155.955 |
| 13-48 | 40 | 40 | 978 | 142.950 |
| 4097+ | 8 | 8 | 48969 | 43.725 |
| 1-3 | 86 | 86 | 258 | 42.496 |
| 4-12 | 9 | 9 | 66 | 2.458 |

## Native Timed Draws By Blend State

| Group | Timed draws | Total draws | Indices | Duration us |
|---|---:|---:|---:|---:|
| rt0:blend=on,mask=RGBA | 383 | 383 | 198972 | 1403.699 |

## Native Timed Draws By Depth State

| Group | Timed draws | Total draws | Indices | Duration us |
|---|---:|---:|---:|---:|
| test=on write=on func=CompareFunction.Greater | 236 | 236 | 187425 | 1154.253 |
| test=on write=off func=CompareFunction.Greater | 147 | 147 | 11547 | 249.446 |

## Target Event Attribution

| Event ID | Duration us | Asset | Material | Leaf | Indices | Bucket | Texture/resource | Pipeline | Blend | Depth | Targets | Group note |
|---:|---:|---|---|---|---:|---|---|---|---|---|---|---|
| 40 | 48.230 | Native SECT1.g2d (m: 59 l: 0) | 59 | SECT1.g3d_5_0_11 | 435 | 257-1024 | GSTexImage Image (256, 128) pallete: 0 (ResourceId::12736) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | Native Previewer GLSL Blend 256 (ResourceId::15346) | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | rt0=2D Color Attachment 3466 (ResourceId::3466) / depth 2D Depth/Stencil Attachment 3470 (ResourceId::3470) | asset `Native SECT1.g2d (m: 59 l: 0)` 48.230 us/1 timed/1 draws; material `59` 48.230 us/1 timed/1 draws |
| 53 | 97.792 | Native SECT1.g2d (m: 57 l: 0) | 57 | SECT1.g3d_5_0_3 | 369 | 257-1024 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::12704) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | Native Previewer GLSL Blend 256 (ResourceId::15346) | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | rt0=2D Color Attachment 3466 (ResourceId::3466) / depth 2D Depth/Stencil Attachment 3470 (ResourceId::3470) | asset `Native SECT1.g2d (m: 57 l: 0)` 211.661 us/4 timed/4 draws; material `57` 211.661 us/4 timed/4 draws |
| 69 | 55.910 | Native SECT1.g2d (m: 57 l: 0) | 57 | SECT1.g3d_5_0_1 | 2883 | 1025-4096 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::12704) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | Native Previewer GLSL Blend 256 (ResourceId::15346) | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | rt0=2D Color Attachment 3466 (ResourceId::3466) / depth 2D Depth/Stencil Attachment 3470 (ResourceId::3470) | asset `Native SECT1.g2d (m: 57 l: 0)` 211.661 us/4 timed/4 draws; material `57` 211.661 us/4 timed/4 draws |
| 1138 | 118.784 | Native SECT1.g2d (m: 23 l: 0) | 23 | SECT1.g3d_111 | 3168 | 1025-4096 | GSTexImage Image (128, 32) pallete: 0 (ResourceId::12160) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | Native Previewer GLSL Blend 256 (ResourceId::15346) | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | rt0=2D Color Attachment 3466 (ResourceId::3466) / depth 2D Depth/Stencil Attachment 3470 (ResourceId::3470) | asset `Native SECT1.g2d (m: 23 l: 0)` 143.155 us/11 timed/11 draws; material `23` 143.155 us/11 timed/11 draws |
| 1581 | 53.760 | Native SECT1.g2d (m: 8 l: 0) | 8 | SECT1.g3d_98 | 2904 | 1025-4096 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11920) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | Native Previewer GLSL Blend 256 (ResourceId::15346) | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | rt0=2D Color Attachment 3466 (ResourceId::3466) / depth 2D Depth/Stencil Attachment 3470 (ResourceId::3470) | asset `Native SECT1.g2d (m: 8 l: 0)` 56.115 us/6 timed/6 draws; material `8` 56.320 us/7 timed/7 draws |
| 2260 | 8.192 | Native SECT1.g2d (m: 1 l: 0) | 1 | SECT1.g3d_128 | 2895 | 1025-4096 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11808) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | Native Previewer GLSL Blend 256 (ResourceId::15346) | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | rt0=2D Color Attachment 3466 (ResourceId::3466) / depth 2D Depth/Stencil Attachment 3470 (ResourceId::3470) | asset `Native SECT1.g2d (m: 1 l: 0)` 185.037 us/25 timed/25 draws; material `1` 194.048 us/32 timed/32 draws |
| 2308 | 10.342 | Native SECT1.g2d (m: 1 l: 0) | 1 | SECT1.g3d_42 | 1629 | 1025-4096 | GSTexImage Image (128, 128) pallete: 0 (ResourceId::11808) (ImageSampler, addr=Wrap,mip=0,cmp=Never) | Native Previewer GLSL Blend 256 (ResourceId::15346) | rt0:blend=on,mask=RGBA | test=on write=on func=CompareFunction.Greater | rt0=2D Color Attachment 3466 (ResourceId::3466) / depth 2D Depth/Stencil Attachment 3470 (ResourceId::3470) | asset `Native SECT1.g2d (m: 1 l: 0)` 185.037 us/25 timed/25 draws; material `1` 194.048 us/32 timed/32 draws |
| 5777 | 1.536 | Native LIGHT_IMPACT.G2D (m: 0 l: 0) | 0 | None_0_0_0 | 3 | 1-3 | GSTexImage Image (32, 32) pallete: 0 (ResourceId::7072) (ImageSampler, addr=ClampEdge,mip=0,cmp=Never) | Native Previewer GLSL Blend 275 (ResourceId::16816) | rt0:blend=on,mask=RGBA | test=on write=off func=CompareFunction.Greater | rt0=2D Color Attachment 3466 (ResourceId::3466) / depth 2D Depth/Stencil Attachment 3470 (ResourceId::3470) | asset `Native LIGHT_IMPACT.G2D (m: 0 l: 0)` 22.528 us/34 timed/34 draws; material `0` 257.331 us/154 timed/154 draws |

## Targeted GPU Fix Candidate

Target the repeated blend/depth path first. The largest blend group is `rt0:blend=on,mask=RGBA` at 1403.699 us, and the largest depth group is `test=on write=on func=CompareFunction.Greater` at 1154.253 us out of 1403.699 us native timed draw cost. Validate alpha/depth correctness before changing ordering-sensitive behavior.

## Small Draws

- Draws with `0 < numIndices <= 48`: `265`
- Total duration: `287.130 us`
