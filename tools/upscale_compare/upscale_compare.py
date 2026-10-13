#!/usr/bin/env python3
"""Offline texture upscaler comparison and contact-sheet generator."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import sys
import time
from pathlib import Path
from typing import Any

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError as exc:  # pillow is the only dependency for help/sheet-only mode
    raise SystemExit("Pillow is required. Install it with: python -m pip install Pillow") from exc


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CONFIG = Path(__file__).with_name("models.example.json")
METHODS = ("ultrasharp", "span", "realplksr", "hat")
LABELS = {"ultrasharp": "UltraSharp", "span": "SPAN", "realplksr": "RealPLKSR", "hat": "HAT"}


def load_config(path: Path) -> dict[str, Any]:
    data = json.loads(path.read_text(encoding="utf-8"))
    models = data.get("models")
    if not isinstance(models, dict):
        raise ValueError("Config must have a 'models' object")
    base = path.parent
    for key, entry in models.items():
        if key not in METHODS:
            raise ValueError(f"Unknown model key '{key}'; expected one of {', '.join(METHODS)}")
        if not isinstance(entry, dict):
            raise ValueError(f"Model '{key}' must be an object")
        model_path = entry.get("path")
        if model_path:
            candidate = Path(model_path)
            entry["resolved_path"] = (base / candidate).resolve() if not candidate.is_absolute() else candidate
    return data


def gather_inputs(paths: list[Path]) -> list[Path]:
    found: list[Path] = []
    for path in paths:
        if path.is_dir():
            found.extend(p for p in sorted(path.rglob("*")) if p.is_file() and p.suffix.lower() == ".png")
        elif path.is_file() and path.suffix.lower() == ".png":
            found.append(path)
        else:
            raise ValueError(f"Input is not a PNG file or directory: {path}")
    unique = list(dict.fromkeys(p.resolve() for p in found))
    if not unique:
        raise ValueError("No PNG inputs found")
    return unique


def load_rgba(path: Path) -> Image.Image:
    with Image.open(path) as image:
        return image.convert("RGBA").copy()


def nearest_resize(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    return image.resize(size, Image.Resampling.NEAREST)


def crop_image(image: Image.Image, crop: tuple[int, int, int, int] | None) -> Image.Image:
    if crop is None:
        return image
    x, y, width, height = crop
    if x < 0 or y < 0 or width < 1 or height < 1 or x + width > image.width or y + height > image.height:
        raise ValueError(f"Crop {crop} is outside source image {image.width}x{image.height}")
    return image.crop((x, y, x + width, y + height))


def parse_crop(raw: str | None) -> tuple[int, int, int, int] | None:
    if raw is None:
        return None
    try:
        values = tuple(int(value.strip()) for value in raw.split(","))
    except ValueError as exc:
        raise argparse.ArgumentTypeError("Crop must be x,y,width,height in source pixels") from exc
    if len(values) != 4:
        raise argparse.ArgumentTypeError("Crop must be x,y,width,height in source pixels")
    return values  # type: ignore[return-value]


def _providers(ort: Any, preference: str) -> list[str]:
    available = set(ort.get_available_providers())
    if preference == "cpu":
        return ["CPUExecutionProvider"]
    if preference == "directml":
        if "DmlExecutionProvider" not in available:
            raise RuntimeError("DirectML requested but DmlExecutionProvider is unavailable; install onnxruntime-directml")
        return ["DmlExecutionProvider", "CPUExecutionProvider"]
    if preference == "cuda":
        if "CUDAExecutionProvider" not in available:
            raise RuntimeError("CUDA requested but CUDAExecutionProvider is unavailable; install a CUDA-enabled onnxruntime")
        return ["CUDAExecutionProvider", "CPUExecutionProvider"]
    if "CUDAExecutionProvider" in available:
        return ["CUDAExecutionProvider", "CPUExecutionProvider"]
    return ["CPUExecutionProvider"]


def validate_architecture(expected: str | None, detected: str, model_name: str) -> None:
    if expected and detected.casefold() != expected.casefold():
        raise ValueError(f"Architecture mismatch for {model_name}: expected {expected}, detected {detected}")


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def validate_scale(expected: int | None, actual: int | None, model_name: str) -> None:
    if expected is not None and actual is not None and int(expected) != int(actual):
        raise ValueError(f"Scale mismatch for {model_name}: expected {expected}x, detected {actual}x")


class OnnxModel:
    def __init__(self, entry: dict[str, Any], preference: str, threads: int):
        try:
            import numpy as np
            import onnxruntime as ort
        except ImportError as exc:
            raise RuntimeError("ONNX inference requires numpy and onnxruntime. Install requirements-onnx.txt") from exc
        path: Path = entry["resolved_path"]
        if not path.is_file():
            raise FileNotFoundError(f"Model file not found: {path}")
        requested = preference if preference != "auto" else entry.get("execution_provider", "auto")
        providers = _providers(ort, requested)
        session_options = ort.SessionOptions()
        session_options.intra_op_num_threads = threads
        session_options.inter_op_num_threads = min(4, threads)
        if providers[0] == "DmlExecutionProvider":
            session_options.enable_mem_pattern = False
            session_options.execution_mode = ort.ExecutionMode.ORT_SEQUENTIAL
        self.session = ort.InferenceSession(str(path), sess_options=session_options, providers=providers)
        self.np = np
        self.input = self.session.get_inputs()[0]
        self.output = self.session.get_outputs()[0]
        shape = self.input.shape
        if len(shape) != 4:
            raise ValueError(f"Expected a 4D ONNX input, received shape {shape}")
        if shape[1] in (1, 3, 4):
            self.nchw = True
            input_channels = shape[1]
        elif shape[-1] in (1, 3, 4):
            self.nchw = False
            input_channels = shape[-1]
        else:
            raise ValueError(f"Cannot determine RGB channel axis for ONNX input shape {shape}")
        if input_channels != 3:
            raise ValueError(f"Expected a 3-channel ONNX input, model expects {input_channels} channels")
        self.architecture = "ONNX"
        self.scale = entry.get("scale")
        self.expected_scale = entry.get("scale")
        self.path = str(path)
        self.sha256 = file_sha256(path)
        self.device = self.session.get_providers()[0]
        self.input_range = entry.get("input_range", [0.0, 1.0])
        self.output_range = entry.get("output_range", [0.0, 1.0])
        if len(self.input_range) != 2 or self.input_range[1] <= self.input_range[0]:
            raise ValueError(f"Invalid input_range for {path.name}: expected [min, max]")
        if len(self.output_range) != 2 or self.output_range[1] <= self.output_range[0]:
            raise ValueError(f"Invalid output_range for {path.name}: expected [min, max]")

    def infer(self, image: Image.Image, wrap: int) -> tuple[Image.Image, float, dict[str, Any]]:
        np = self.np
        rgb = image.convert("RGB")
        if wrap:
            rgb = Image.fromarray(np.pad(np.asarray(rgb), ((wrap, wrap), (wrap, wrap), (0, 0)), mode="wrap"))
        arr = np.asarray(rgb, dtype=np.float32) / 255.0
        low, high = self.input_range
        arr = arr * (high - low) + low
        tensor = np.transpose(arr, (2, 0, 1))[None, ...] if self.nchw else arr[None, ...]
        start = time.perf_counter()
        result = self.session.run([self.output.name], {self.input.name: tensor})[0]
        elapsed = time.perf_counter() - start
        result = np.asarray(result)
        if result.ndim != 4:
            raise RuntimeError(f"Expected 4D ONNX image output, received shape {result.shape}")
        if result.shape[1] in (1, 3, 4):
            result = result[0].transpose(1, 2, 0)
        elif result.shape[-1] in (1, 3, 4):
            result = result[0]
        else:
            raise RuntimeError(f"Cannot determine RGB channel axis for ONNX output shape {result.shape}")
        if result.ndim == 2:
            result = result[:, :, None]
        if result.shape[2] not in (1, 3, 4):
            raise RuntimeError(f"Expected 1, 3, or 4 ONNX output channels, received shape {result.shape}")
        input_height, input_width = arr.shape[:2]
        output_height, output_width = result.shape[:2]
        if output_height % input_height or output_width % input_width:
            raise RuntimeError(f"ONNX output dimensions {output_width}x{output_height} are not an integer scale of padded input {input_width}x{input_height}")
        actual_scale = output_width // input_width
        if output_height // input_height != actual_scale:
            raise RuntimeError(f"ONNX output has non-uniform scale: input {input_width}x{input_height}, output {output_width}x{output_height}")
        validate_scale(self.expected_scale, actual_scale, Path(self.path).name)
        result = np.nan_to_num(result, nan=0.0, posinf=1.0, neginf=0.0)
        result = (result - self.output_range[0]) / (self.output_range[1] - self.output_range[0])
        result = np.clip(result, 0.0, 1.0)
        if result.shape[2] == 1:
            result = np.repeat(result, 3, axis=2)
        output = Image.fromarray(np.uint8(np.round(result[:, :, :3] * 255)))
        if wrap:
            output = output.crop((wrap * output.width // rgb.width, wrap * output.height // rgb.height,
                                  output.width - wrap * output.width // rgb.width,
                                  output.height - wrap * output.height // rgb.height))
        info = {"input_shape": list(tensor.shape), "output_shape": list(result.shape), "actual_scale": actual_scale}
        return output, elapsed, info


class TorchModel:
    def __init__(self, entry: dict[str, Any], preference: str, threads: int):
        try:
            import numpy as np
            import torch
            from spandrel import ImageModelDescriptor, ModelLoader
        except ImportError as exc:
            raise RuntimeError("PyTorch model inference requires numpy, torch, and spandrel. Install requirements-torch.txt") from exc
        torch.set_num_threads(threads)
        try:
            torch.set_num_interop_threads(min(4, threads))
        except RuntimeError:
            # Torch only permits this setting before the inter-op pool starts.
            pass
        path: Path = entry["resolved_path"]
        if not path.is_file():
            raise FileNotFoundError(f"Model file not found: {path}")
        descriptor = ModelLoader().load_from_file(str(path))
        if not isinstance(descriptor, ImageModelDescriptor):
            raise RuntimeError(f"{path.name} is not a Spandrel image model")
        spandrel_architecture = str(descriptor.architecture.id)
        model_architecture = type(descriptor.model).__name__
        # RealPLKSR shares Spandrel's PLKSR descriptor and is distinguished by
        # the concrete model implementation class.
        architecture = model_architecture if model_architecture == "RealPLKSR" else spandrel_architecture
        expected = entry.get("expected_architecture")
        validate_architecture(expected, architecture, f"{path.name} (Spandrel {spandrel_architecture})")
        if descriptor.purpose not in ("SR", "FaceSR"):
            raise ValueError(f"Expected super-resolution model, detected purpose {descriptor.purpose} ({architecture})")
        if descriptor.input_channels != 3 or descriptor.output_channels not in (3, 4):
            raise ValueError(f"Expected RGB input and RGB/RGBA output, got {descriptor.input_channels} -> {descriptor.output_channels} channels")
        if preference == "cpu":
            device = torch.device("cpu")
        elif preference in ("cuda", "auto") and torch.cuda.is_available():
            device = torch.device("cuda")
        elif preference == "cuda":
            raise RuntimeError("CUDA requested but torch.cuda.is_available() is false")
        elif preference == "directml":
            raise RuntimeError("DirectML is supported only by ONNX Runtime models")
        else:
            device = torch.device("cpu")
        descriptor.to(device).eval()
        self.descriptor, self.torch, self.np = descriptor, torch, np
        self.architecture, self.scale = architecture, descriptor.scale
        self.expected_architecture, self.path, self.device = expected, str(path), str(device)
        self.sha256 = file_sha256(path)
        self.expected_scale = entry.get("scale")
        validate_scale(self.expected_scale, descriptor.scale, path.name)
        self.requirements = descriptor.size_requirements

    def infer(self, image: Image.Image, wrap: int) -> tuple[Image.Image, float, dict[str, Any]]:
        torch, np, model = self.torch, self.np, self.descriptor
        rgb = image.convert("RGB")
        if wrap:
            rgb = Image.fromarray(np.pad(np.asarray(rgb), ((wrap, wrap), (wrap, wrap), (0, 0)), mode="wrap"))
        array = np.asarray(rgb, dtype=np.float32) / 255.0
        tensor = torch.from_numpy(array.transpose(2, 0, 1)).unsqueeze(0).to(device=model.device, dtype=model.dtype)
        if model.device.type == "cuda":
            torch.cuda.synchronize(model.device)
        start = time.perf_counter()
        with torch.inference_mode():
            output = model(tensor)
        if model.device.type == "cuda":
            torch.cuda.synchronize(model.device)
        elapsed = time.perf_counter() - start
        arr = output[0].detach().float().cpu().numpy().transpose(1, 2, 0)
        arr = np.clip(arr, 0.0, 1.0)
        rendered = Image.fromarray(np.uint8(np.round(arr[:, :, :3] * 255)))
        if wrap:
            factor = model.scale
            rendered = rendered.crop((wrap * factor, wrap * factor, rendered.width - wrap * factor, rendered.height - wrap * factor))
        actual_scale_x = rendered.width / image.width
        actual_scale_y = rendered.height / image.height
        if not actual_scale_x.is_integer() or actual_scale_x != actual_scale_y:
            raise RuntimeError(f"PyTorch output dimensions {rendered.width}x{rendered.height} are not an integer uniform scale of input {image.width}x{image.height}")
        actual_scale = int(actual_scale_x)
        validate_scale(self.expected_scale, actual_scale, Path(self.path).name)
        return rendered, elapsed, {"input_shape": list(tensor.shape), "output_shape": list(arr.shape),
                                  "actual_scale": actual_scale, "size_requirements": str(self.requirements)}


def compose_rgba(rgb: Image.Image, source: Image.Image) -> Image.Image:
    alpha = nearest_resize(source.getchannel("A"), rgb.size)
    result = rgb.convert("RGB")
    result.putalpha(alpha)
    return result


def fit_panel(image: Image.Image, width: int, height: int) -> Image.Image:
    # Uniform fit into a common panel keeps the source and model results comparable.
    scale = min(width / image.width, height / image.height)
    size = (max(1, round(image.width * scale)), max(1, round(image.height * scale)))
    return image.resize(size, Image.Resampling.NEAREST)


def checkerboard(size: tuple[int, int], cell_size: int = 16) -> Image.Image:
    image = Image.new("RGB", size, (195, 195, 195))
    draw = ImageDraw.Draw(image)
    for y in range(0, size[1], cell_size):
        for x in range(0, size[0], cell_size):
            if (x // cell_size + y // cell_size) % 2:
                draw.rectangle((x, y, min(x + cell_size - 1, size[0] - 1),
                                min(y + cell_size - 1, size[1] - 1)), fill=(238, 238, 238))
    return image


def paste_preview(sheet: Image.Image, image: Image.Image, box: tuple[int, int, int, int]) -> None:
    x, y, width, height = box
    background = checkerboard((width, height))
    preview = fit_panel(image.convert("RGBA"), width, height)
    px, py = (width - preview.width) // 2, (height - preview.height) // 2
    background.paste(preview, (px, py), preview.getchannel("A"))
    sheet.paste(background, (x, y))


def make_sheet(source: Image.Image, results: dict[str, Image.Image | None], statuses: dict[str, str],
               out_path: Path, crop: tuple[int, int, int, int]) -> None:
    font = ImageFont.load_default()
    methods = ["source", *METHODS]
    preview_images: dict[str, Image.Image | None] = {"source": source, **results}
    successful = [image for image in results.values() if image is not None]
    display_scale = round(successful[0].width / source.width) if successful else 4
    display_scale = max(1, display_scale)
    sx, sy, sw, sh = crop
    source_crop = nearest_resize(source.crop((sx, sy, sx + sw, sy + sh)), (sw * display_scale, sh * display_scale))
    crop_images: dict[str, Image.Image | None] = {"source": source_crop}
    for method, image in results.items():
        if image is None:
            crop_images[method] = None
            continue
        x0 = round(sx * image.width / source.width)
        y0 = round(sy * image.height / source.height)
        x1 = round((sx + sw) * image.width / source.width)
        y1 = round((sy + sh) * image.height / source.height)
        crop_images[method] = image.crop((x0, y0, x1, y1))

    panel_w = 384
    panel_h = max(240, min(384, round(panel_w * source.height / source.width)))
    pad, header_h, row_label_h, gutter = 14, 38, 24, 8
    sheet_w = pad * 2 + len(methods) * panel_w + (len(methods) - 1) * gutter
    sheet_h = pad * 2 + header_h + 2 * (row_label_h + panel_h + gutter)
    sheet = Image.new("RGB", (sheet_w, sheet_h), (25, 27, 31))
    draw = ImageDraw.Draw(sheet)
    for col, method in enumerate(methods):
        x = pad + col * (panel_w + gutter)
        image = preview_images.get(method)
        title = "Source" if method == "source" else LABELS[method]
        if image is not None:
            title += f"  {image.width}x{image.height}"
            fill = (238, 240, 242)
        else:
            title += f"  ({statuses.get(method, 'missing')})"
            fill = (235, 196, 130)
        draw.text((x, pad + 5), title, fill=fill, font=font)

    row1_y = pad + header_h
    row2_y = row1_y + row_label_h + panel_h + gutter
    draw.text((pad, row1_y + 4), "Full texture", fill=(238, 240, 242), font=font)
    draw.text((pad, row2_y + 4), f"Close-up crop {crop} (source pixels)", fill=(238, 240, 242), font=font)
    full_y = row1_y + row_label_h
    crop_y = row2_y + row_label_h
    for col, method in enumerate(methods):
        x = pad + col * (panel_w + gutter)
        full = preview_images.get(method)
        close = crop_images.get(method)
        if full is not None:
            paste_preview(sheet, full, (x, full_y, panel_w, panel_h))
        else:
            draw.rectangle((x, full_y, x + panel_w, full_y + panel_h), fill=(45, 46, 50))
            draw.text((x + 8, full_y + 8), statuses.get(method, "missing"), fill=(235, 196, 130), font=font)
        if close is not None:
            paste_preview(sheet, close, (x, crop_y, panel_w, panel_h))
        else:
            draw.rectangle((x, crop_y, x + panel_w, crop_y + panel_h), fill=(45, 46, 50))
            draw.text((x + 8, crop_y + 8), statuses.get(method, "missing"), fill=(235, 196, 130), font=font)
    sheet.save(out_path)


def run(args: argparse.Namespace) -> int:
    config = load_config(args.config)
    input_paths = gather_inputs(args.inputs)
    args.output.mkdir(parents=True, exist_ok=True)
    requested = METHODS if args.models == "all" else tuple(name.strip() for name in args.models.split(",") if name.strip())
    unknown = set(requested) - set(METHODS)
    if unknown:
        raise ValueError(f"Unknown model(s): {', '.join(sorted(unknown))}")
    loaded: dict[str, Any] = {}
    statuses: dict[str, str] = {}
    for method in requested:
        entry = config["models"].get(method)
        if not entry or not entry.get("path"):
            statuses[method] = "missing (no model path configured)"
            if not args.allow_missing:
                raise FileNotFoundError(f"No path configured for {method}; see {args.config}")
            continue
        kind = entry.get("kind", "onnx" if str(entry.get("path", "")).lower().endswith(".onnx") else "torch")
        try:
            loaded[method] = OnnxModel(entry, args.device, args.threads) if kind == "onnx" else TorchModel(entry, args.device, args.threads)
            statuses[method] = "ready"
        except Exception as exc:
            statuses[method] = f"unavailable: {exc}"
            if not args.allow_missing:
                raise
    configured_crop = args.crop
    report: dict[str, Any] = {
        "config": str(args.config.resolve()),
        "run": {"models": list(requested), "device_preference": args.device, "threads": args.threads,
                "wrap_padding": args.wrap_padding,
                "crop_source_pixels": list(configured_crop) if configured_crop else "center up to 64px (clamped per source)"},
        "inputs": [],
    }
    for path in input_paths:
        source = load_rgba(path)
        crop = configured_crop
        path_key = hashlib.sha256(str(path).casefold().encode("utf-8")).hexdigest()[:8]
        output_root = args.output / f"{path.stem}-{path_key}"
        output_root.mkdir(parents=True, exist_ok=True)
        output_images: dict[str, Image.Image | None] = {}
        input_record: dict[str, Any] = {"source": str(path), "source_sha256": file_sha256(path),
                                        "source_dimensions": list(source.size), "models": {}}
        for method in METHODS:
            if method not in requested:
                statuses.setdefault(method, "not selected")
                output_images[method] = None
                continue
            model = loaded.get(method)
            if model is None:
                output_images[method] = None
                input_record["models"][method] = {"status": statuses[method]}
                continue
            try:
                rgb, elapsed, tensor_info = model.infer(source, args.wrap_padding)
                rgba = compose_rgba(rgb, source)
                output_file = output_root / f"{method}.png"
                rgba.save(output_file)
                output_images[method] = rgba
                input_record["models"][method] = {"status": "ok", "file": str(output_file), "dimensions": list(rgba.size),
                    "elapsed_seconds": round(elapsed, 6), "device": model.device, "model": model.path, "model_sha256": model.sha256,
                    "architecture": model.architecture, "scale": getattr(model, "scale", None), **tensor_info}
            except Exception as exc:
                statuses[method] = f"inference failed: {exc}"
                output_images[method] = None
                input_record["models"][method] = {"status": statuses[method]}
                if not args.allow_missing:
                    raise
        if crop is None:
            crop_w, crop_h = min(64, source.width), min(64, source.height)
            crop = ((source.width - crop_w) // 2, (source.height - crop_h) // 2, crop_w, crop_h)
        crop_image(source, crop)
        input_record["crop_source_pixels"] = list(crop)
        sheet_path = output_root / "comparison.png"
        make_sheet(source, output_images, statuses, sheet_path, crop)
        input_record["comparison_sheet"] = str(sheet_path)
        report["inputs"].append(input_record)
        print(f"{path.name} -> {sheet_path}")
    report_path = args.output / "report.json"
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Report: {report_path}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path, help="PNG files or directories containing PNGs")
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG, help=f"Model manifest (default: {DEFAULT_CONFIG})")
    parser.add_argument("--output", type=Path, default=ROOT / "out" / "upscale-compare")
    parser.add_argument("--models", default="all", help="Comma-separated model keys or 'all'")
    parser.add_argument("--allow-missing", action="store_true", help="Generate partial sheets and mark unavailable models")
    parser.add_argument("--device", choices=("auto", "cpu", "cuda", "directml"), default="auto",
                        help="Device preference; auto chooses CUDA, then DirectML for ONNX, then CPU")
    parser.add_argument("--wrap-padding", type=int, default=0, metavar="PIXELS",
                        help="Repeat-border pad before inference and crop it afterward")
    parser.add_argument("--threads", type=int, default=4, metavar="N",
                        help="CPU inference thread limit (1-64; default 4)")
    parser.add_argument("--crop", type=parse_crop, metavar="X,Y,W,H",
                        help="Optional source-pixel crop shown beside each full output")
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    if args.wrap_padding < 0:
        parser.error("--wrap-padding must be zero or greater")
    if not 1 <= args.threads <= 64:
        parser.error("--threads must be between 1 and 64")
    try:
        return run(args)
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
