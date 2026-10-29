"""Reproduce concrete unresolved collision/combat call-graph blockers."""
import json
from pathlib import Path
import sys
import argparse
import subprocess
from compiled import ROOT, lift
from compiled_extended import bundle, sha
from call_graph import link
from verify import request, Unsupported


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--ninja', default='C:/Program Files/Microsoft Visual Studio/18/Insiders/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe')
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    args = parser.parse_args()
    build = ROOT / 'out/build/x64-debug'
    output = ROOT / 'out/verification/combat-blockers'
    output.mkdir(parents=True, exist_ok=True)
    cases = [('ActorFighter', '0031ba50', 'SetStandAnim', '?SetStandAnim@CActorFighter@@QEAAXH@Z'),
             ('Actor', '00102a00', 'SetState', '?SetState@CActor@@UEAAXHH@Z')]
    targets = ['CMakeFiles/Kya.dir/src/b-witch/' + c[0] + '.cpp.obj' for c in cases]
    subprocess.run([args.ninja, '-C', str(build), *targets], check=True)
    objects = {c[0]: (build / target).read_bytes() for c, target in zip(cases, targets)}
    rows = []
    for case in cases:
        original = request('export_pcode', args.port, address=case[1])
        (output / (case[1] + '.json')).write_text(json.dumps(original, indent=2))
        row = dict(function=original['name'], ps2_entry=case[1], windows_object_sha256=sha(objects[case[0]]),
                   ps2_executable_sha256=original['executable_sha256'], status='blocked',
                   indirect_calls=[i['address'] for i in original['instructions'] if any(o['opcode'] == 'CALLIND' for o in i['ops'])])
        try:
            blob, functions, addresses = bundle(objects, [case], {})
            folder = output / case[2]
            folder.mkdir(exist_ok=True)
            (folder / 'functions.bin').write_bytes(blob)
            (folder / 'manifest.json').write_text(json.dumps(dict(functions=functions, addresses=addresses), indent=2))
            lift(folder, args.ghidra, args.java)
            exports = json.loads((folder / 'exports.json').read_text())['exports']
            link(exports, int(exports[0]['entry'], 16))
            row.update(status='needs-memory-and-callee-setup', reason='Call expansion passed; whole-function proof not attempted')
        except (ValueError, Unsupported) as error:
            row['reason'] = str(error)
        rows.append(row)
    document = dict(scope='Capability audit only; no equivalence claimed', functions=rows,
                    required_support=['Finite target sets and proof of vtable target identity on both architectures',
                                      'Expanded behavior/animation/audio call graph with object/global/stack memory mappings',
                                      'Floating-point/VU semantics wherever those reachable callees require them'])
    (output / 'report.json').write_text(json.dumps(document, indent=2) + '\n')
    print(json.dumps(document, indent=2))
    return 0  # An audit successfully documenting a blocker is not a proof.


if __name__ == '__main__':
    raise SystemExit(main())
