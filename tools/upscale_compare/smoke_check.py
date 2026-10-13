"""Dependency-light checks for contact sheet, scaling/cropping, and arch guard."""

from __future__ import annotations

import importlib.util
import json
import subprocess
import sys
import tempfile
from pathlib import Path

from PIL import Image


SCRIPT = Path(__file__).with_name("upscale_compare.py")
SPEC = importlib.util.spec_from_file_location("upscale_compare", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def main() -> None:
    source = Image.new("RGBA", (8, 6), (190, 80, 40, 255))
    source.putpixel((1, 1), (0, 255, 0, 0))
    scaled = MODULE.nearest_resize(source, (32, 24))
    assert scaled.size == (32, 24)
    assert scaled.getpixel((4, 4)) == source.getpixel((1, 1))
    assert MODULE.crop_image(scaled, (4, 4, 8, 6)).size == (8, 6)
    rgba = MODULE.compose_rgba(scaled.convert("RGB"), source)
    assert rgba.getpixel((4, 4))[3] == 0
    try:
        MODULE.crop_image(source, (7, 5, 2, 2))
    except ValueError:
        pass
    else:
        raise AssertionError("out-of-bounds crop should fail")
    try:
        MODULE.validate_architecture("SPAN", "HAT", "fixture.pth")
    except ValueError as exc:
        assert "expected SPAN, detected HAT" in str(exc)
    else:
        raise AssertionError("architecture mismatch should fail")

    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        input_files = []
        for folder, image in ((root / "a", Image.new("RGBA", (100, 80), (190, 80, 40, 255))),
                              (root / "b", source)):
            folder.mkdir()
            input_file = folder / "fixture.png"
            image.save(input_file)
            input_files.append(input_file)
        cli_output = root / "cli-output"
        completed = subprocess.run(
            [sys.executable, str(SCRIPT), *(str(path) for path in input_files), "--output", str(cli_output),
             "--models", "", "--allow-missing"],
            check=True, capture_output=True, text=True,
        )
        assert "comparison.png" in completed.stdout
        assert len(list(cli_output.glob("*/comparison.png"))) == 2
        report = json.loads((cli_output / "report.json").read_text(encoding="utf-8"))
        assert len(report["inputs"]) == 2
        assert report["inputs"][0]["source_dimensions"] == [100, 80]
        assert report["inputs"][1]["source_dimensions"] == [8, 6]
        assert report["inputs"][0]["crop_source_pixels"] == [18, 8, 64, 64]
        assert report["inputs"][1]["crop_source_pixels"] == [0, 0, 8, 6]
        assert report["inputs"][0]["source_sha256"] != report["inputs"][1]["source_sha256"]
        assert report["inputs"][0]["comparison_sheet"] != report["inputs"][1]["comparison_sheet"]

        output = root / "comparison.png"
        images = {name: None for name in MODULE.METHODS}
        images["span"] = scaled
        status = {name: "missing (smoke fixture)" for name in MODULE.METHODS}
        status["span"] = "ready"
        MODULE.make_sheet(source, images, status, output, (1, 1, 4, 3))
        with Image.open(output) as sheet:
            assert sheet.width > 0 and sheet.height > 0
            assert sheet.width > sheet.height  # method columns, full and crop rows
            assert sheet.getpixel((0, 0)) == (25, 27, 31)
        pattern = MODULE.checkerboard((32, 32), cell_size=8)
        assert pattern.getpixel((1, 1)) != pattern.getpixel((9, 1))
        preview_canvas = Image.new("RGB", (20, 20), (0, 0, 0))
        transparent = Image.new("RGBA", (10, 10), (255, 0, 0, 0))
        MODULE.paste_preview(preview_canvas, transparent, (0, 0, 20, 20))
        assert preview_canvas.getpixel((1, 1)) != preview_canvas.getpixel((17, 1))
    print("upscale comparison smoke checks passed")


if __name__ == "__main__":
    main()
