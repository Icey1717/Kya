**Absolutely — here are the detailed versions of each finding.** I’m keeping the same severity split, but expanding the exact failure mode, why I think it happens, and what I’d change.

## Critical

### 1. Fence can become permanently unsignaled after `vkAcquireNextImageKHR()` fails
**Files:**  
- `port\Windows\Renderer\Vulkan\src\VulkanRenderer.cpp:819-829`
- `port\Windows\Renderer\Vulkan\src\VulkanRenderer.cpp:858-869`

#### What the code does
In `waitUntilReady()`:

```cpp
vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
vkResetFences(device, 1, &inFlightFences[currentFrame]);

VkResult result = vkAcquireNextImageKHR(..., &presentImageIndex);

if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
}
```

Then later in `present()`:

```cpp
vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
```

So the intended lifecycle is:

1. wait for frame fence
2. reset frame fence
3. acquire image
4. submit GPU work using that fence
5. fence gets signaled when submit completes

#### The bug
If step 3 returns `VK_ERROR_OUT_OF_DATE_KHR`, the function returns **after resetting the fence but before queue submission**.

That means the fence for `currentFrame` is now unsignaled, and nothing will ever signal it.

On the next reuse of that frame slot, this line:

```cpp
vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
```

can block forever.

#### Why this is a real problem
This is not theoretical. Swapchain acquire failures happen during:
- window resize
- minimize/restore
- display mode changes
- some alt-tab / surface invalidation cases

So the exact path where you most need recovery is also the path that can poison the fence state.

#### Why the current structure makes it easy to miss
The code splits frame flow across:
- `waitUntilReady()`
- `present()`

That separation makes it less obvious that fence reset and fence signal are not guaranteed to happen in the same control path.

#### How I’d fix it
One of these approaches:

1. **Only reset the fence after a successful acquire and just before submit**
   - safest conceptual fix
   - keeps “reset” tightly coupled to “I am definitely about to submit”

2. **If acquire fails with out-of-date, re-signal or avoid reusing the reset fence**
   - possible, but more awkward

3. **Use a per-image / per-frame ownership model like the common Vulkan tutorial pattern**
   - track acquired image readiness and in-flight image ownership explicitly

#### Best practical fix here
Move `vkResetFences()` so it happens only after `vkAcquireNextImageKHR()` succeeds and immediately before `vkQueueSubmit()`.

That preserves the invariant:

> “Every reset fence must have a corresponding submit that will signal it.”

---

### 2. `BeginSingleTimeCommands()` ignores allocation failure and records into an invalid command buffer
**File:**  
- `port\Windows\Renderer\Vulkan\src\Objects\VulkanCommands.cpp:11-18`

#### What the code does
```cpp
VkCommandBuffer commandBuffer;
vkAllocateCommandBuffers(GetDevice(), &allocInfo, &commandBuffer);

VkCommandBufferBeginInfo beginInfo{};
...
vkBeginCommandBuffer(commandBuffer, &beginInfo);
```

#### The bug
`vkAllocateCommandBuffers()` returns a `VkResult`, but it is ignored.

If allocation fails:
- `commandBuffer` may be uninitialized garbage
- or an invalid/null handle depending on driver/runtime behavior
- then `vkBeginCommandBuffer()` is called anyway

That turns an ordinary recoverable Vulkan failure into undefined behavior / invalid API usage.

#### Why this matters
Command buffer allocation can fail due to:
- device loss
- exhausted pool memory
- invalid command pool state
- allocation fragmentation
- earlier lifetime bugs

Once that happens, the code continues as though allocation succeeded, which makes later crashes much harder to diagnose. The original fault gets hidden behind secondary invalid-handle behavior.

#### Why I marked this critical
This is core command-recording infrastructure. If this helper is used broadly, one unchecked failure can contaminate many higher-level systems.

#### What I’d change
At minimum:

```cpp
VkResult result = vkAllocateCommandBuffers(...);
if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate single-time command buffer");
}

result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
if (result != VK_SUCCESS) {
    vkFreeCommandBuffers(..., &commandBuffer);
    throw std::runtime_error("failed to begin single-time command buffer");
}
```

#### Secondary concern
`EndSingleTimeCommands()` also ignores:
- `vkEndCommandBuffer()`
- `vkQueueSubmit()`

Those are not as immediately dangerous as the invalid-handle path, but they should also be checked for consistency.

---

### 3. `PipelineKey` equality is incorrect because it compares hash values instead of actual fields
**Files:**  
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.h:22-46`
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.h:117-122`
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.cpp:267-285`

#### What the code does
`PipelineKeyHash` hashes:
- `shaderDefinitions.gsDef`
- `shaderDefinitions.psDef`
- `shaderDefinitions.vsDef`
- `topology`
- `stride`
- `pipelineSelector`

But equality is defined as:

```cpp
inline bool operator==(const PipelineKey& lhs, const PipelineKey& rhs) {
    PipelineKeyHash hasher;
    return hasher(lhs) == hasher(rhs);
}
```

#### The bug
For `unordered_map`, the contract is:

- hash narrows candidate bucket
- equality determines actual key identity

Here, equality is not true equality. It is “same hash output”.

That means two different keys that collide hash-wise are considered identical.

#### What can go wrong
Inside:

```cpp
using PipelineMap = std::unordered_map<PipelineKey, Renderer::Pipeline, PipelineKeyHash>;
```

a collision can cause:
- wrong pipeline reuse
- cache poisoning
- failed assertions that seem unrelated
- rendering state mismatches that appear nondeterministic

This path is especially dangerous in graphics because the failure symptom won’t necessarily be a crash; it may be:
- wrong blending
- wrong shader variant
- wrong stride/topology pairing
- corrupted output only on certain materials/draws

#### Why the existing assert doesn’t save it
In `GetPipeline()`:

```cpp
assert(pipeline.key.debugData.hash == hash);
assert(key.pipelineSelector.bs.key == pipeline.key.pipelineSelector.bs.key);
```

This does **not** prove the whole key matches.

Problems:
1. first assert compares hash to hash, which is tautological in a collision case
2. second assert checks only one subfield of `pipelineSelector`
3. in release builds, asserts disappear anyway

So release behavior could silently reuse the wrong pipeline.

#### How I’d fix it
Implement structural equality:

```cpp
inline bool operator==(const PipelineKey& lhs, const PipelineKey& rhs) {
    return lhs.shaderDefinitions.vsDef == rhs.shaderDefinitions.vsDef &&
           lhs.shaderDefinitions.gsDef == rhs.shaderDefinitions.gsDef &&
           lhs.shaderDefinitions.psDef == rhs.shaderDefinitions.psDef &&
           lhs.topology == rhs.topology &&
           lhs.stride == rhs.stride &&
           lhs.pipelineSelector == rhs.pipelineSelector;
}
```

If `PipelineSelector` lacks equality, add it there.

#### One more subtle point
`debugData` should probably **not** participate in identity unless it changes runtime behavior. It looks diagnostic, so I would leave it out of equality/hash.

---

## Important

### 4. Swapchain recreation is incomplete for resources derived from swapchain image count and format
**Files:**  
- `port\Windows\Renderer\Vulkan\src\VulkanRenderer.cpp:351-376`
- `port\Windows\Renderer\Vulkan\src\VulkanRenderer.cpp:451-466`
- `port\Windows\Renderer\Vulkan\src\VulkanRenderer.cpp:725-790`
- `port\Windows\Renderer\Vulkan\src\Native\NativeRendererSetup.inl:369-380`

#### What the code does initially
During startup:
- `createSwapChain()`
- `createImageViews()`
- `createGlobalRenderPass()`
- `PS2::Setup()`
- `createFramebuffers()`
- `createSyncObjects()`

`createSyncObjects()` allocates:
- `imageAvailableSemaphores` sized to `MAX_FRAMES_IN_FLIGHT`
- `inFlightFences` sized to `MAX_FRAMES_IN_FLIGHT`
- `renderFinishedSemaphores` sized to `swapChainImages.size()`

Also, `createGlobalRenderPass()` bakes `swapChainImageFormat` into the render pass.

#### What recreation does
`recreateSwapChain()` currently does only:

```cpp
vkDeviceWaitIdle(device);
cleanupSwapChain();
createSwapChain();
createImageViews();
createFramebuffers();
```

#### What’s missing
It does **not** recreate:
1. `renderFinishedSemaphores`
2. `renderPass`
3. any subsystems whose render-pass/framebuffer assumptions depend on swapchain format/extent/image count
4. possibly PS2/native render resources if they are extent-coupled

#### Why `renderFinishedSemaphores` matter
Submission uses:
```cpp
submitInfo.pSignalSemaphores = &renderFinishedSemaphores[presentImageIndex];
```

Presentation uses:
```cpp
presentInfo.pWaitSemaphores = &renderFinishedSemaphores[presentImageIndex];
```

But `presentImageIndex` comes from the **current** swapchain image set.

If the new swapchain has a different image count than the old one:
- vector size may no longer match valid image indices
- this becomes out-of-bounds access or stale semaphore reuse

Even if counts happen to match most of the time, the assumption is not safe.

#### Why render pass recreation matters
`createGlobalRenderPass()` builds the color attachment with:

```cpp
colorAttachment.format = swapChainImageFormat;
```

If the surface format changes across recreation and the old render pass survives, then new framebuffers/images can become incompatible with the old render pass.

That kind of mismatch tends to show up as:
- validation errors
- framebuffer creation failures
- pipeline/render pass incompatibility
- unexplained presentation/render failures on some setups

#### Why the native renderer setup also worries me
`Native::Setup()` creates render stages, framebuffers, samplers, command pool/buffers, postprocessing, preview setup, etc. Several of those appear extent- or format-sensitive, but `recreateSwapChain()` never revisits them.

I’m not claiming every one of those objects is wrong on resize, but the current recreation path is narrowly scoped enough that I would expect stale assumptions to accumulate.

#### What I’d change
Treat swapchain recreation as rebuilding **all swapchain-dependent resources**, not just:
- swapchain
- image views
- final framebuffers

At minimum I’d re-evaluate/recreate:
1. global render pass
2. swapchain framebuffers
3. per-swapchain-image semaphores or change the design to per-frame signaling
4. any pipelines/framebuffers whose attachment compatibility depends on the swapchain format
5. native/PS2 pass resources tied to dimensions or target format

#### Better design direction
A clear split helps:
- **device-lifetime resources**
- **swapchain-lifetime resources**
- **frame-lifetime resources**

Right now some swapchain-lifetime resources are created in init paths that are not mirrored in recreate paths.

---

### 5. `cleanup()` destroys objects without an explicit `vkDeviceWaitIdle()`
**File:**  
- `port\Windows\Renderer\Vulkan\src\VulkanRenderer.cpp:405-437`

#### What the code does
`cleanup()` destroys:
- swapchain resources
- uniform buffers
- PS2 subsystem resources
- graphics pipeline / pipeline layout / render pass
- vertex/index buffers
- semaphores/fences
- command pool
- device

But it never first does:
```cpp
vkDeviceWaitIdle(device);
```

#### Why this matters
Destroying Vulkan resources that are still in use by GPU work is unsafe and violates the normal Vulkan lifetime model.

Typical shutdown hazards:
- app closes while work is still queued
- resize/recovery happened recently
- worker/render thread timing leaves commands in flight
- present queue / graphics queue still owns submissions referencing these objects

#### Why this is more than a theoretical cleanup nit
You do have `vkDeviceWaitIdle(device)` in `recreateSwapChain()`, which shows the codebase already recognizes that GPU/CPU teardown ordering matters.

The absence of the same protection in final cleanup makes the shutdown path weaker than the resize path.

#### Possible failure symptoms
- validation errors on shutdown
- sporadic driver/device errors
- crashes in debug layers
- “works on my machine” behavior that varies by GPU/driver timing

#### What I’d change
At the start of `cleanup()`:
```cpp
if (device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device);
}
```

That should happen before any child-resource destruction.

#### Additional nuance
If some resources are owned by threads or subsystems, the thread shutdown ordering should also guarantee:
- no more recording/submission
- no more deferred callbacks touching those resources

`vkDeviceWaitIdle()` is necessary, but it doesn’t replace good host-side ownership shutdown.

---

### 6. Several Vulkan creation calls ignore failure results
**Files:**  
- `port\Windows\Renderer\Vulkan\src\Native\NativeRendererSubmission.inl:384-399`
- `port\Windows\Renderer\Vulkan\src\Objects\FrameBuffer.cpp:248-272`

#### Concrete examples

##### Example A: preview sampler creation
```cpp
vkCreateSampler(GetDevice(), &samplerInfo, GetAllocator(), &gPreviewFrameBufferSampler);
```
No result check.

##### Example B: final pass pipeline creation
```cpp
vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, GetAllocator(), &finalPipeline.pipeline);
```
No result check.

Then immediately:
```cpp
finalPipeline.CreateDescriptorPool();
finalPipeline.CreateDescriptorSets();
```
and later the pipeline is bound in `ExecuteFinalPass()`.

#### Why this matters
These aren’t harmless omissions. The code later assumes those handles are valid.

If creation fails:
- the failure is not surfaced at the call site
- later code may bind or name invalid objects
- root-cause diagnosis becomes much harder

This is especially problematic because elsewhere in the renderer you **do** throw on Vulkan creation failure. So the error-handling model is inconsistent.

#### Why `vkCreateGraphicsPipelines()` is particularly important
Pipeline creation is one of the most failure-prone Vulkan calls because it depends on:
- shader validity
- descriptor layouts
- render pass compatibility
- dynamic state
- attachment formats
- driver support

If it fails and you proceed, later draw-time failures can look unrelated.

#### What I’d change
Check every creation call that returns `VkResult`, and fail immediately with context-rich errors.

For example:
```cpp
VkResult result = vkCreateGraphicsPipelines(...);
if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to create final pass graphics pipeline");
}
```

Same for sampler creation and similar resource constructors.

#### Severity note
I marked this Important rather than Critical because the impact depends on whether those failure paths happen in practice. But in a renderer, silent creation failures are exactly the kind of thing that turn portability/debuggability into a nightmare.

---

### 7. Descriptor set support looks internally inconsistent for multi-set shaders
**Files:**  
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.cpp:298-318`
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.cpp:341-376`
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.cpp:399-417`
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.cpp:445-461`

#### What the code claims to support
Reflection builds descriptor bindings by set number:

```cpp
descriptorSetLayoutBindings[layout.setNumber][bindingStage] = layout.bindings;
```

Then `CreateDescriptorSetLayouts()` iterates all sets and creates a layout for each set index.

So at the data-model level, the code appears to support **multiple descriptor sets**.

#### Where that breaks down

##### A. Descriptor set allocation uses only set 0
```cpp
std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayouts[0]);
...
vkAllocateDescriptorSets(..., descriptorSets.data())
```

Only `descriptorSetLayouts[0]` is used.

If a shader uses sets 0 and 1, set 1 is not allocated here.

##### B. Descriptor pool sizing hardcodes set 0
```cpp
auto& descriptorSetLayoutBindings = descriptorSetLayoutBindingsMap.at(0);
```

Pool sizing ignores any descriptor bindings in other sets.

##### C. Descriptor writes flatten binding maps into a single destination set
`CreateWriteDescriptorSetList()` iterates the whole `layoutBindingMap`, but all writes target the single `dstSet` passed in.

That means even if reflection discovers multiple sets, write generation does not maintain set identity.

#### Why this matters
Today this may work if all current shaders effectively use only set 0.

But the architecture is misleading:
- part of the code says “multi-set is supported”
- allocation and pool logic say “only set 0 really works”

That creates a trap for future shader work. Someone can add a reflected set 1 and think the renderer supports it because layout creation succeeds, but runtime allocation/update behavior will be wrong.

#### Likely failure modes
- descriptor allocation succeeds but some bindings are never allocated
- update code silently writes only a subset of required descriptors
- pipeline layout includes multiple set layouts, but bind/update paths don’t match
- validation errors when shaders expect sets that were never created/updated/bound

#### What I’d change
Pick one of two directions:

1. **Explicitly support only set 0**
   - assert during reflection if any set number other than 0 appears
   - simplify the code so the limitation is obvious

2. **Actually support multiple sets**
   - allocate descriptor sets per reflected set
   - size pools across all sets
   - preserve set number through update/write/bind paths
   - bind multiple sets when needed

Right now it’s in an ambiguous middle state, which is risky.

---

## Minor

### 8. `selfDependency` is constructed but never used
**File:**  
- `port\Windows\Renderer\Vulkan\src\Objects\FrameBuffer.cpp:82-102`

#### What the code does
It defines:

```cpp
VkSubpassDependency selfDependency{};
selfDependency.srcSubpass = 0;
selfDependency.dstSubpass = 0;
...
```

But then creates:

```cpp
VkSubpassDependency dependencies[] = { colorDependency, depthDependency };
```

So `selfDependency` is never included in `renderPassInfo`.

#### Why I flagged it
This is either:
1. dead code left behind from an earlier synchronization attempt, or
2. an intended dependency that was accidentally omitted

Either case is worth cleaning up because render-pass synchronization code is not the place to leave ambiguity.

#### Why it’s only Minor
I don’t have enough evidence from this snippet alone to say the missing self-dependency definitely causes a bug.

But it does indicate one of:
- sync reasoning was incomplete
- refactor left stale code
- intended hazard handling is missing

#### What I’d do
Either:
- remove it if unnecessary, or
- add it intentionally and document why

For Vulkan synchronization logic, “unused but maybe important” is technical debt.

---

### 9. Pipeline-owned descriptor resources appear not to be released during PS2 cleanup
**Files:**  
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.h:70-74`
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.cpp:298-318`
- `port\Windows\Renderer\Vulkan\src\Objects\Pipeline.cpp:358-376`
- `port\Windows\Renderer\Vulkan\src\VulkanPS2.cpp:2104-2109`

#### What the pipeline owns
`Renderer::Pipeline` contains:
- `VkPipeline pipeline`
- `VkPipelineLayout layout`
- `VkDescriptorPool descriptorPool`
- `std::vector<VkDescriptorSetLayout> descriptorSetLayouts`

Creation code explicitly creates:
- descriptor set layouts
- descriptor pool
- descriptor sets

#### What cleanup destroys
`PS2::Cleanup()` only does:

```cpp
vkDestroyPipeline(..., pipeline.second.pipeline, ...);
vkDestroyPipelineLayout(..., pipeline.second.layout, ...);
```

It does **not** destroy:
- `descriptorPool`
- each `descriptorSetLayout`

#### Why this matters
On final process exit, OS cleanup may hide the leak. But it is still a real ownership bug.

It matters more if:
- renderer is ever torn down and reinitialized in-process
- device reset / reload workflows are added
- tests or tools create/destroy renderer repeatedly

#### Why Minor and not Important
Right now this looks like a teardown leak rather than a direct runtime correctness failure.

If the renderer becomes more dynamic, this issue rises in severity.

#### What I’d change
Give `Renderer::Pipeline` a clear destruction path that releases:
1. descriptor sets implicitly via pool destruction
2. descriptor pool
3. descriptor set layouts
4. pipeline
5. pipeline layout

The important architectural point is: ownership should live with the object that created those resources.

---

## Testing / confidence gap behind several findings

This wasn’t one of the numbered issues, but it explains why I’m less comfortable with the subtree as-is.

**File inspected:**  
- `port\Test\src\tests.cpp:1-220`

What I found there is mostly:
- a trivial `Add()` test
- pointer conversion tests
- some commented-out IO-related tests

I did **not** see focused automated coverage for:
- swapchain recreation
- out-of-date acquire recovery
- pipeline cache key correctness
- descriptor reflection / descriptor set allocation logic

For a Vulkan renderer, those are exactly the areas where latent bugs survive until:
- a different GPU
- a driver update
- a resize edge case
- a new shader layout

So even some of the “Important” findings deserve extra caution because there isn’t much evidence they’ve been exercised systematically.

## If I were fixing in priority order
1. fence reset/acquire/submit ordering
2. real `PipelineKey` equality
3. swapchain recreation completeness
4. device idle before cleanup
5. check all Vulkan creation/submit/end results
6. decide whether descriptor sets are “set 0 only” or truly multi-set
7. cleanup ownership leaks / dead sync code

That ordering gets the correctness and deadlock risks down first.