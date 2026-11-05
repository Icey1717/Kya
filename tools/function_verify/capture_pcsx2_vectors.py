"""Capture raw outputs from the pinned upstream interpreter instruction oracle."""
import argparse
import hashlib
import json
from pathlib import Path
import random
import subprocess
from ee_interpreter import OPERATIONS, MXCSR, MODEL


def capture(executable, output, *, random_count=32):
    metadata = json.loads((executable.parent / 'oracle.json').read_text())
    if metadata['models'] != [MODEL] or hashlib.sha256(executable.read_bytes()).hexdigest() != metadata['executable_sha256']:
        raise ValueError('Oracle executable identity differs from its build manifest')
    special = (0, 0x80000000, 1, 0x807fffff, 0x00800000, 0x80800000,
               0x3f800000, 0xbf800000, 0x3f800001, 0x3f7fffff,
               0x40000000, 0x40400000, 0x7f7fffff, 0xff7fffff,
               0x7f800000, 0x7fffffff, 0xff800001)
    rng = random.Random(0x51473)
    inputs = []
    for opcode in OPERATIONS:
        for i, a in enumerate(special):
            for j, b in enumerate(special):
                inputs.append(dict(mnemonic=opcode, a=a, b=b, acc=special[(i + j) % len(special)],
                                   initial_fcr31=(0x01000001, 0x01ffffff)[(i + j) % 2]))
        for _ in range(random_count):
            inputs.append(dict(mnemonic=opcode, a=rng.getrandbits(32), b=rng.getrandbits(32),
                               acc=rng.getrandbits(32), initial_fcr31=rng.getrandbits(32)))
    command_input = ''.join(f"{r['mnemonic']} {r['a']:x} {r['b']:x} {r['acc']:x} {r['initial_fcr31']:x} {MXCSR:x}\n" for r in inputs)
    result = subprocess.run([str(executable.resolve())], input=command_input, text=True, capture_output=True, check=True)
    lines = result.stdout.splitlines()
    if len(lines) != len(inputs):
        raise ValueError('Oracle capture is incomplete')
    rows = []
    for row, line in zip(inputs, lines):
        values = [int(part, 16) for part in line.split()]
        if len(values) != 4:
            raise ValueError('Malformed oracle response')
        rows.append(dict(row, bits=values[0], acc_before=row['acc'], acc=values[1],
                         fcr31=values[2], mxcsr=values[3]))
    document = dict(model=MODEL, scope='Actual upstream instruction interpreter; not hardware or recompiler evidence',
                    oracle=metadata, input_mxcsr=MXCSR, vectors=rows)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(document, separators=(',', ':')) + '\n')
    approval = dict(metadata, executable=str(executable.resolve()), vectors=str(output.resolve()),
                    vectors_sha256=hashlib.sha256(output.read_bytes()).hexdigest(),
                    source_manifest_sha256=hashlib.sha256(json.dumps(metadata['source_manifest'], sort_keys=True,
                        separators=(',', ':')).encode()).hexdigest())
    output.with_suffix('.oracle.json').write_text(json.dumps(approval, indent=2) + '\n')
    return len(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('executable', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    print(capture(args.executable, args.output))


if __name__ == '__main__':
    main()
