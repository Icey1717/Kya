"""Build an instruction oracle from unmodified, pinned PCSX2 source bodies.

The small VM boundary traps branch/memory services. This is an isolated upstream
COP1 interpreter, not a booted emulator or the extended-FPU recompiler backend.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

from ee_reference import REVISION

ROOT = Path(__file__).resolve().parents[2]
FILES = ('pcsx2/FPU.cpp', 'pcsx2/R5900.h', 'common/Pcsx2Defs.h', 'common/Pcsx2Types.h')
MODEL = 'pcsx2-cop1-interpreter:' + REVISION


def sha(data):
    return hashlib.sha256(data).hexdigest()


def build(source, output, compiler='clang'):
    source, output = source.resolve(), output.resolve()
    revision = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
    if revision != REVISION:
        raise ValueError('Oracle requires the pinned PCSX2 revision')
    subprocess.run(['git', '-C', str(source), 'diff', '--exit-code', 'HEAD', '--', *FILES], check=True)
    audit = json.loads((ROOT / 'tools/function_verify/fixtures/pcsx2-reference.json').read_text())
    expected = next(f['sha256'] for f in audit['files'] if f['path'] == 'pcsx2/FPU.cpp')
    canonical = {path: subprocess.check_output(['git', '-C', str(source), 'show', REVISION + ':' + path])
                 for path in FILES}
    if sha(canonical[FILES[0]]) != expected:
        raise ValueError('Upstream interpreter does not match the pinned source audit')
    output.mkdir(parents=True, exist_ok=True)
    local = Path(__file__).parent
    # A byte-identical copy puts the test Common.h boundary ahead of the VM's
    # unrelated full Common.h; all arithmetic bodies and register types remain upstream.
    (output / 'FPU.cpp').write_bytes(canonical[FILES[0]])
    shutil.copyfile(local / 'pcsx2_oracle_common.h', output / 'Common.h')
    executable = output / 'pcsx2-ee-interpreter-oracle.exe'
    command = [compiler, '-std=c++20', '-O0', '-ffp-model=strict', '-ffp-contract=off',
               '-fno-strict-aliasing', '-msse4.1', '-I' + str(source), '-I' + str(output),
               str(output / 'FPU.cpp'), str(local / 'pcsx2_ee_oracle.cpp'), '-o', str(executable)]
    subprocess.run(command, check=True)
    manifest = dict(kind='pcsx2-instruction-oracle', revision=REVISION, hardware_validated=False,
                    backend='COP1 interpreter (unmodified upstream FPU.cpp)', models=[MODEL],
                    source_manifest={path: sha(canonical[path]) for path in FILES},
                    compiled_header_manifest={path: sha((source / path).read_bytes()) for path in FILES[1:]},
                    boundary_sha256=sha((output / 'Common.h').read_bytes()),
                    driver_sha256=sha((local / 'pcsx2_ee_oracle.cpp').read_bytes()),
                    executable_sha256=sha(executable.read_bytes()), command=command,
                    executable=str(executable),
                    compiler_version=subprocess.check_output([compiler, '--version'], text=True),
                    settings=dict(mxcsr='0x7f80', host_rounding='toward-zero', daz=False, ftz=False,
                                  branch_and_memory_services='trap', guest_patches=False),
                    limitations=['Instruction interpreter only; no VM boot, EE recompiler or VU backend validation'])
    (output / 'oracle.json').write_text(json.dumps(manifest, indent=2) + '\n')
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=ROOT / 'out/verification/pcsx2-upstream')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/verification/pcsx2-oracle')
    parser.add_argument('--compiler', default='clang')
    args = parser.parse_args()
    manifest = build(args.source, args.output, args.compiler)
    print(json.dumps(manifest, indent=2))


if __name__ == '__main__':
    main()
