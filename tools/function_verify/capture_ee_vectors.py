"""Capture concrete corrected-add/sub reference vectors; not hardware results."""
import argparse
import hashlib
import json
from pathlib import Path
import random
import subprocess
from ee_reference import MODEL, SOURCE


def capture(oracle):
    operands = [(0, 0x80000000), (0x3f800000, 0x33800000), (0x3f800001, 0x3f800000),
                (0x00800001, 0x00800000), (0x00800002, 0x00800000),
                (0x7fffffff, 0x7fffffff), (0x7f800000, 0x7f800001),
                (0xff800000, 0x7f800000), (1, 0x80000001)]
    generator = random.Random(5900)
    operands += [(generator.getrandbits(32), generator.getrandbits(32)) for _ in range(128)]
    rows = []
    for mnemonic in ('ADD.S', 'SUB.S'):
        for a, b in operands:
            initial = generator.getrandbits(32)
            output = json.loads(subprocess.check_output([str(oracle), mnemonic, f'{a:x}', f'{b:x}', f'{initial:x}']))
            rows.append(dict(mnemonic=mnemonic, a=a, b=b, initial_fcr31=initial, **output))
    return dict(scope='Concrete reimplementation of pinned PCSX2 corrected-add/sub algorithm; not hardware validation',
                model=MODEL, source=SOURCE, oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),
                oracle_source_sha256=hashlib.sha256(Path(__file__).with_name('ee_reference_oracle.cpp').read_bytes()).hexdigest(),
                vectors=rows)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('oracle', type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    args.output.write_text(json.dumps(capture(args.oracle.resolve()), indent=2) + '\n')
