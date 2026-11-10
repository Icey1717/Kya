"""Compare editor captures with a matching game capture.

Usage:
    python compare_capture_colors.py GAME.png CANDIDATE.png [CANDIDATE.png ...]

The game image is resized to each candidate's resolution before comparison. This
measures whole-frame colour agreement only; framing, dynamic actors and sprites
still affect the result.
"""
import sys

from PIL import Image, ImageChops, ImageStat


def main(paths):
    if len(paths) < 2:
        raise SystemExit(__doc__)
    game = Image.open(paths[0]).convert('RGB')
    for path in paths[1:]:
        candidate = Image.open(path).convert('RGB')
        reference = game.resize(candidate.size, Image.LANCZOS)
        difference = ImageChops.difference(candidate, reference)
        mean = ImageStat.Stat(candidate).mean
        reference_mean = ImageStat.Stat(reference).mean
        mae = ImageStat.Stat(difference).mean
        rms = ImageStat.Stat(difference).rms
        print('%s size=%dx%d' % (path, candidate.width, candidate.height))
        print('  mean RGB %s (game %s)' % (mean, reference_mean))
        print('  MAE      (%.2f, %.2f, %.2f)' % tuple(mae))
        print('  RMS      (%.2f, %.2f, %.2f)' % tuple(rms))


if __name__ == '__main__':
    main(sys.argv[1:])
