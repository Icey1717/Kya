"""Capture independent native SSE results from the separately built oracle."""
import argparse
import hashlib
import json
from pathlib import Path
import random
import subprocess


def capture(oracle):
    inputs = [(0x3f800000, 0x33800000), (0x3f800000, 0x40400000),
              (0x00800000, 0x3f000000), (0x00800000, 0x3f7fffff),
              (0x7f7fffff, 0x40000000), (0x80000000, 0), (1, 0x3f800000),
              (0x3f800000, 0), (0, 0), (0x7f800000, 0xff800000),
              (0x3f800001, 0x3f800000), (0x00800001, 0x3f7fffff)]
    randomizer = random.Random(5900)
    # Exclude arithmetic NaN inputs: that model intentionally rejects them.
    inputs += [(randomizer.randrange(0x7f800000), randomizer.randrange(0x7f800000)) for _ in range(12)]
    vectors = []
    for mnemonic in ('ADDSS', 'SUBSS', 'MULSS', 'DIVSS', 'COMISS', 'UCOMISS'):
        selected = inputs + ([(0x7fc00001, 1), (0x7f800001, 1), (1, 0x7fc00001),
                              (0x7fc00001, 0x3f800000)] if mnemonic.endswith('COMISS') else [])
        for control in (0x1f80, 0x3f80, 0x5f80, 0x7f80, 0x9fc0, 0x9f80, 0x1fc0):
            for a, b in selected:
                result = json.loads(subprocess.check_output([str(oracle), mnemonic, f'{a:x}', f'{b:x}', f'{control:x}']))
                vectors.append(dict(mnemonic=mnemonic, a=a, b=b, control=control, **result))
    return dict(scope='Native x64 SSE oracle; exact bits, EFLAGS and MXCSR',
                oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),
                source_sha256=hashlib.sha256(Path(__file__).with_name('fp_oracle.cpp').read_bytes()).hexdigest(),
                vectors=vectors)


def capture_conversions(oracle):
    vectors = []
    inputs = (0, 1, -1, 500, 1000, 16777215, 16777216, 16777217,
              -16777217, 2147483647, -2147483648)
    for control in (0x1f80, 0x3f80, 0x5f80, 0x7f80, 0x9fc0, 0x1fa0):
        for value in inputs:
            result = json.loads(subprocess.check_output(
                [str(oracle), 'CVTSI2SS', f'{value & 0xffffffff:x}', '0', f'{control:x}']))
            vectors.append(dict(a=value & 0xffffffff, control=control, **result))
    return dict(scope='Native signed 32-bit CVTSI2SS, exact bits and MXCSR',
                oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),
                source_sha256=hashlib.sha256(Path(__file__).with_name('fp_oracle.cpp').read_bytes()).hexdigest(),
                vectors=vectors)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('oracle', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--conversions-only', action='store_true')
    args = parser.parse_args()
    args.output.write_text(json.dumps((capture_conversions if args.conversions_only else capture)(args.oracle.resolve()), indent=2) + '\n')
