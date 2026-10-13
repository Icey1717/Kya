# Texture upscale comparison

For selecting and verifying SPAN/UltraSharp in the game, see [in-game.md](in-game.md).

This offline tool runs the same PNG textures through the repository's UltraSharp ONNX model and local SPAN, RealPLKSR, and HAT PyTorch weights. It saves each full-size result, a labeled contact sheet per source image, and a JSON report with dimensions, detected architecture, device, and inference time. It does not download weights.

## Setup

Create a Python environment that supports your chosen PyTorch build, then install the relevant runtime requirements. Use `requirements-onnx.txt` for ONNX Runtime and `requirements-torch.txt` for Spandrel models. The ONNX requirement installs the standard CPU package; for GPU acceleration, install the matching CUDA ONNX Runtime package instead, or use `onnxruntime-directml` on Windows. Avoid installing both CPU and DirectML ONNX Runtime distributions in one environment.

Place trained `.pth`, `.pt`, or `.safetensors` weights under `out/upscale-compare/models/`, or edit `models.example.json` to point to local files. Relative paths in the manifest are resolved against the manifest's directory. Spandrel detects the actual architecture and scale; each PyTorch entry's `expected_architecture` and `scale` are checked before inference so a mislabeled or non-4x model fails clearly. HAT and RealPLKSR are supported when the installed Spandrel release recognizes the weight format and architecture. RealPLKSR uses Spandrel's shared `PLKSR` descriptor and is identified by its concrete `RealPLKSR` model class.

The manifest points UltraSharp at the existing, ignored `upscaling/models/4x-UltraSharpV2_fp32_op17.onnx` and the three PyTorch methods at `out/upscale-compare/models/`. Put the trained weights at those paths or edit the manifest to point to another local copy. The model files are intentionally kept out of version control.

For the six-texture review used here, run these commands from the repository root:

```powershell
python -m venv out/upscale-compare-venv
& out/upscale-compare-venv/Scripts/python.exe -m pip install -r tools/upscale_compare/requirements-onnx.txt -r tools/upscale_compare/requirements-torch.txt
& tools/upscale_compare/prepare_review.ps1 -DownloadModels
& out/upscale-compare-venv/Scripts/python.exe tools/upscale_compare/upscale_compare.py out/upscale-compare/selected --output out/upscale-compare/review --device cpu --threads 4
```

`prepare_review.ps1` uses KyaBank at `G:/repos/KyaBank/out/build/x64-debug/KyaBank.exe` by default; override `-KyaBankExe` if needed. It exports the original Level 1 textures, selects six character/UI/painted-surface/foliage/prop samples, records bank/exporter hashes in `texture_sources.json`, and optionally downloads the three reference checkpoints with SHA-256 verification. All generated assets and dependencies stay under ignored `out/`. The existing UltraSharp file must already be present.

## Checkpoint provenance

The configured weights are specific checkpoints, with their own training data and objectives:

| Method | Checkpoint | Attribution | License |
| --- | --- | --- | --- |
| SPAN | `4xNomosUni_span_multijpg.safetensors` | Phhofm, 4x NomosUni span multijpg | CC BY 4.0 |
| RealPLKSR | `4x-RealPLKSR_Pretrain_V4.pth` | Kim2091, RealPLKSR V4 pretrain | CC0 |
| HAT | `HAT_SRx4_ImageNet-pretrain.pth` | Acly / hat mirror, HAT SRx4 ImageNet pretrain | Apache 2.0 |

The source URLs and SHA-256 values for these files are recorded in `model_sources.json`. These checkpoints use different training objectives, so a sheet compares these exact files on the chosen textures; it does not rank SPAN, RealPLKSR, or HAT as architectures in general.

## Run

```powershell
KyaBank.exe extract assets/CDEURO/LEVEL/LEVEL_1/LEVEL.BNK --output out/upscale-compare/extracted --flatten
KyaBank.exe texconvert out/upscale-compare/extracted --output out/upscale-compare/textures
python tools/upscale_compare/upscale_compare.py out/upscale-compare/textures --config tools/upscale_compare/models.example.json --output out/upscale-compare/results --models all
```

The commands above extract `LEVEL.BNK` from the European level archive and convert extracted textures to PNG. Inputs can be PNG files or folders; folders are searched recursively for PNGs. Use `--models ultrasharp,span` to run a subset. `--allow-missing` lets the harness create a partial sheet and labels missing/unavailable methods; without it, a missing model or runtime is an error. `--device auto` prefers CUDA, then CPU. Select `cpu`, `cuda`, or `directml` explicitly when needed. DirectML applies only to ONNX Runtime. `--threads` limits CPU inference threads and defaults to 4.

`--wrap-padding N` adds N repeated texture pixels on each side before inference, then removes the generated border. This can reduce edge artifacts for repeating textures. It defaults to off. Input alpha is carried through to each output using nearest-neighbor scaling, matching the game's alpha scaling behavior. Model inference receives RGB only. ONNX models default to RGB values in `[0, 1]` for both input and output; adjust `input_range` or `output_range` in the manifest only when a model export uses another range. Output dimensions and configured scale are checked so the comparison doesn't silently mix different scales.

Each source gets a uniquely named directory containing `ultrasharp.png`, `span.png`, `realplksr.png`, `hat.png` (for methods that ran) and `comparison.png`. Each sheet has five columns (Source, UltraSharp, SPAN, RealPLKSR, HAT), a full-texture row, and a close-up crop row below. Use `--crop X,Y,W,H` to choose the crop in source pixels; otherwise the sheet uses a centered crop up to 64×64 source pixels. Transparent pixels display over a checkerboard. The full result files retain their native model dimensions. `report.json` records run options, source and model SHA-256 hashes, native dimensions, scale, device, model path, and inference time; model loading and image IO are excluded from inference timing. The hashes make it possible to confirm that two reports used the same input and weight files.

For a Pillow-only source/sheet smoke run without inference, use `--models "" --allow-missing`; it writes the source row and clearly marks the unselected methods. Pillow is the only import needed for this mode and `--help`. Run `python tools/upscale_compare/smoke_check.py` for the dependency-light CLI/sheet checks. With the inference environment installed, `python tools/upscale_compare/runtime_smoke_check.py` exercises the PyTorch/Spandrel tensor adapter using a tiny random initialized fixture; it is only a plumbing check and is not a quality comparison.

Contact sheets are for visual review, not a quality score. Use representative real game textures when comparing; synthetic smoke images only verify the harness mechanics.
