"""Optional PyTorch/Spandrel adapter plumbing smoke test; no quality claims."""

from __future__ import annotations

from pathlib import Path

import numpy as np
import torch
from PIL import Image
from spandrel import Architecture, ImageModelDescriptor

import upscale_compare as harness


class TinyScaleModel(torch.nn.Module):
    """Random initialized nearest-scale model used only to exercise tensor plumbing."""

    def __init__(self) -> None:
        super().__init__()
        self.anchor = torch.nn.Parameter(torch.ones(()))

    def forward(self, image: torch.Tensor) -> torch.Tensor:
        return torch.nn.functional.interpolate(image * self.anchor, scale_factor=2, mode="nearest")


class TinyArchitecture(Architecture[TinyScaleModel]):
    def __init__(self) -> None:
        super().__init__(id="TinySmoke", detect=lambda _state: False)

    def load(self, state_dict):  # not used by this in-memory fixture
        raise NotImplementedError


def main() -> None:
    architecture = TinyArchitecture()
    descriptor = ImageModelDescriptor(
        TinyScaleModel(), {"anchor": torch.ones(())}, architecture, purpose="SR", tags=["smoke"],
        supports_half=False, supports_bfloat16=False, scale=2, input_channels=3, output_channels=3,
    ).to("cpu").eval()
    model = harness.TorchModel.__new__(harness.TorchModel)
    model.descriptor = descriptor
    model.torch = torch
    model.np = np
    model.architecture = "TinySmoke"
    model.scale = descriptor.scale
    model.device = "cpu"
    model.path = str(Path(__file__))
    model.sha256 = harness.file_sha256(Path(__file__))
    model.expected_scale = 2
    model.requirements = descriptor.size_requirements

    source = Image.new("RGBA", (8, 6), (70, 120, 190, 255))
    source.putpixel((1, 2), (0, 0, 0, 0))
    output, elapsed, info = model.infer(source, wrap=2)
    assert output.size == (16, 12)
    rgba = harness.compose_rgba(output, source)
    assert rgba.getpixel((2, 4))[3] == 0
    assert info["output_shape"] == [20, 24, 3]  # pre-crop dimensions include repeat-border padding
    assert elapsed >= 0
    assert len(model.sha256) == 64
    print("PyTorch/Spandrel tensor plumbing smoke checks passed (not a quality comparison)")


if __name__ == "__main__":
    main()
