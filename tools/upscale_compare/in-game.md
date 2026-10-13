# In-game texture upscaling

The Texture menu and selected texture Preview expose an **Upscale model** selector for UltraSharp and SPAN. Choose a model, then click **Upscale** for the selected texture or **Upscale All** for the currently loaded texture library. **Revert** restores the original texture. The selector is disabled while jobs are pending; both models always start from original pixels. The UI displays loading errors, the inference backend, job progress, and cache hit/miss/error counts. The selected model is saved as the `Texture Upscale Model` DebugSetting.

## Disk cache

Fresh inference results are always saved under `bin/WIN/texture-upscale-cache`, next to the running executable. Enable **Load Upscaled Textures From Cache** in the Texture menu or Preview to automatically apply cached results for the selected model to existing and newly loaded textures. This is a persisted `Debug::Setting<bool>`, off by default. Automatic loading checks up to sixteen new textures per frame and does not run inference for cache misses. The Upscale buttons reuse cached results while this setting is enabled, running inference on a miss. With it disabled, Upscale performs fresh inference and updates the cache. Disabling the option does not revert textures that are already replaced.

Entries use SHA-256 keys covering the original tightly packed RGBA pixels (including alpha), dimensions, scale, model contents, and conversion recipe version. Texture names are not keys, so identical textures share entries and changed models/source pixels cannot reuse stale entries. Each binary `.rgba` entry contains a checked header and payload checksum. Corrupt or truncated entries count as misses; a manual Upscale regenerates them. Writes use unique temporary files followed by atomic replacement. Cache read/write failures do not prevent fresh inference from replacing a texture.

Revert invalidates pending results and remains in effect for that texture until you toggle cache loading or change models. Changing models reloads applicable cached textures for the newly selected model. Cached results remain on disk across launches; model files still need to be present to establish the model's content hash. There is no automatic eviction; deleting individual `.rgba` cache files safely removes those entries.

The Windows build enables ONNX and DirectML by default for new CMake caches. Existing caches need the flags below. CMake downloads ONNX Runtime 1.20.1 and DirectML 1.15.2, verifies their hashes, and copies both native DLLs into `bin/WIN`. DirectML uses one background worker with CPU fallback if GPU session creation fails. Its required sequential execution and disabled memory patterns follow the [ONNX Runtime provider documentation](https://onnxruntime.ai/docs/execution-providers/DirectML-ExecutionProvider.html).

```powershell
& tools/upscale_compare/prepare_in_game.ps1 -DownloadSpan
cmake --preset x64-debug -DENABLE_UPSCALING_ONNX=ON -DENABLE_UPSCALING_DIRECTML=ON
cmake --build out/build/x64-debug --target Kya_Win KyaPortTest
```

Run the CMake commands in a Visual Studio x64 developer shell. To build a CPU-only ONNX version, set `ENABLE_UPSCALING_DIRECTML=OFF`. Model paths resolve relative to the executable, so the working directory does not determine which weights load. In this checkout they are under `upscaling/models`; packaged builds can supply them under `bin/WIN/upscaling/models`.

The SPAN model is Philip Hofmann's CC-BY-4.0 `4xNomosUni_span_multijpg_fp32_opset17.onnx`, from the [author's release](https://github.com/Phhofm/models/releases/tag/4xNomosUni_span_multijpg). The preparation script verifies its hash. On the exported UI texture, its RGB output differed from the harness's original SPAN checkpoint by only 0.015 byte values on average (maximum 1).

The base image is enlarged 4x in each dimension. Alpha uses nearest-neighbour scaling. Existing authored lower mip levels are retained, so distant/mipmapped appearance may still use original detail. GPU replacements are uploaded at frame boundaries; that upload can cause a brief stall. The job queue has a 128 MiB source-data budget and processes one inference at a time. Revert and unloading invalidate stale queued results.

## Verification

The GPU integration test uses an original game texture exported by KyaBank; it reads Vulkan images back to check 4x dimensions, exact alpha preservation, descriptor reuse, and byte-exact Revert. It also verifies cache hits upload exactly the saved pixels, disabled cache loading is honored, cache-only misses do not run inference, a manual Upscale supersedes background cache probes, and Revert wins over unfinished work. Normal cache tests cover model/source/scale invalidation, checksums/truncation, replacement, write failure, and persistence of the DebugSetting.

```powershell
& tools/upscale_compare/prepare_review.ps1
& out/upscale-compare-venv/Scripts/python.exe -c "from PIL import Image; from pathlib import Path; import struct; p=Path('out/upscale-compare/selected/CMP_ICONS_M0_L0.png'); im=Image.open(p).convert('RGBA'); p.with_suffix('.rgba').write_bytes(struct.pack('<II',*im.size)+im.tobytes())"
Push-Location bin/WIN
& ./KyaPortTest.exe --gtest_also_run_disabled_tests --gtest_filter='TextureUpscaleRendering.*'
Pop-Location
```

After the first GPU test has populated the cache, run it in a second process with `KYA_TEST_UPSCALE_CACHE_ONLY=1` to require both models' initial results to come from the saved entries. The test then verifies those pixels by GPU readback; missing entries fail instead of being regenerated for that initial check.

For an opt-in smoke check in the running game, set `KYA_UPSCALE_VERIFY` to an absolute log path before launching `Kya_Debug.exe`. Once gameplay loads Kya's atlas, the check applies UltraSharp, reverts, applies SPAN, reverts, and applies SPAN again, leaving that texture selected and upscaled for inspection. Normal launches do not run this check. It reports failure if model loading fails, the texture unloads, or the three-minute deadline expires. The GPU integration test is stronger evidence of pixel correctness than this runtime dimension check.

Local evidence is under ignored `out/upscale-compare`: `cache-gpu-test.log`, `cache-gpu-restart-test.log`, `cache-all-tests.log`, and `running-game-desktop.log`. Both models ran on DirectML in this checkout. Cache save/reload and a second-process cache-only GPU check passed. The normal test suite passed 259 tests with one unrelated opt-in save-backup test skipped.
