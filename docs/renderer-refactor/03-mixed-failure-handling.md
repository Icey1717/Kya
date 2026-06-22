# Mixed Failure Handling

## Finding

The renderer uses a mixture of exceptions, assertions, boolean returns, and
silent assumptions for hard setup failures.

Examples:

- Many Vulkan object creation failures throw `std::runtime_error`.
- `createLogicalDevice()` validates required runtime features with `assert()`.
- Extension and feature availability is split between suitability checks,
  creation checks, and later assertions.
- Several places use `assert()` for data or runtime assumptions that may be hit
  outside debug builds.

## Why This Is A Problem

For renderer initialization and Vulkan capability checks, `assert()` is the
wrong failure mechanism. In release builds, the check disappears and the user may
get a later, less useful Vulkan failure.

This also makes error policy harder to understand:

- Some missing capabilities are reported cleanly.
- Some are debug-only checks.
- Some fail later through unrelated calls.

Since the port is Windows-focused and Vulkan setup failures are hard failures,
exception-based reporting is a good fit for setup and resource creation.

## Suggested Fix

Use exceptions for hard runtime failures. Keep assertions only for programmer
invariants that cannot be caused by hardware, driver, config, game data, or user
environment.

Add a small helper for Vulkan result checking:

```cpp
void CheckVk(VkResult result, const char* operation)
{
    if (result != VK_SUCCESS) {
        throw std::runtime_error(std::string(operation) + " failed");
    }
}
```

Then replace repeated patterns:

```cpp
if (vkCreateImage(device, &info, allocator, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
}
```

with:

```cpp
CheckVk(vkCreateImage(device, &info, allocator, &image), "vkCreateImage");
```

For feature validation, create an explicit capability check:

```cpp
struct RequiredDeviceFeatures {
    bool samplerAnisotropy = true;
    bool geometryShader = true;
    bool fillModeNonSolid = true;
    bool synchronization2 = true;
    bool colorWriteEnable = true;
    bool extendedDynamicState3ColorBlendEnable = true;
    bool extendedDynamicState3ColorBlendEquation = true;
    bool extendedDynamicState3ColorWriteMask = true;
};
```

Suggested migration path:

1. Replace device feature `assert()` calls with exception-throwing validation.
2. Report missing required extensions/features by name.
3. Add `CheckVk()` for Vulkan calls where `VK_SUCCESS` is the only valid result.
4. Keep special handling for expected swapchain outcomes such as
   `VK_ERROR_OUT_OF_DATE_KHR` and `VK_SUBOPTIMAL_KHR`.
5. Audit `assert(false)` and decide whether each case is a programmer invariant
   or a runtime failure that should throw.

