# CMake And Module Boundaries

## Finding

The renderer build definition reinforces the loose structure of the source tree.

Examples:

- One large flat `SOURCES` list in `port/Windows/Renderer/CMakeLists.txt`.
- Broad public include directories, including `Vulkan/src`.
- A global `include_directories("ext/glm")`.
- Vendored dependencies, shader tooling, PCSX2-derived texture upload code, and
  renderer-owned Vulkan code are wired together in one target scope.

## Why This Is A Problem

The build file does not express architectural boundaries. Because many include
directories are public, internals are easy to include from anywhere. That makes
it harder to tell which APIs are stable renderer-facing interfaces and which are
implementation details.

The flat source list also makes the renderer look like one large module even
though it has distinct areas:

- Public renderer interface.
- Vulkan platform/device/swapchain layer.
- Object/resource wrappers.
- Native renderer.
- PS2/display-list renderer.
- Texture cache/upload.
- Shader compilation and reflection.
- Vendored dependencies.

## Suggested Fix

Refactor CMake to expose boundaries gradually. This does not require changing
runtime behavior first.

Suggested target structure:

- `Renderer`: public game-facing renderer library.
- `RendererVulkanCore`: Vulkan context, device, swapchain, command helpers.
- `RendererVulkanObjects`: buffer/image/pipeline/render pass wrappers.
- `RendererNative`: native mesh renderer and preview renderer.
- `RendererTexture`: texture cache/update/upscale code.
- `RendererShaders`: shader compilation/config integration.
- `TextureUpload`: keep as separate public dependency if other code needs it.

Use `target_sources()` to group files near their targets:

```cmake
add_library(RendererVulkanCore)
target_sources(RendererVulkanCore PRIVATE
    Vulkan/src/VulkanRenderer.cpp
    Vulkan/src/VulkanPS2.cpp
)
target_include_directories(RendererVulkanCore
    PUBLIC include
    PRIVATE Vulkan/src
)
```

Migration path:

1. Replace global `include_directories("ext/glm")` with target-scoped includes.
2. Make `Vulkan/src` private where possible.
3. Group existing `SOURCES` with comments or `target_sources()` before creating
   new libraries.
4. Split internal libraries only after headers stop depending on global
   `VulkanRenderer.h`.
5. Keep the public `Renderer` target stable for the rest of the port.

