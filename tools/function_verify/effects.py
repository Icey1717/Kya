"""Build and prove recovered LOD effect leaves against live PS2 p-code.

This is an independent callee suite, not a whole fighter-transition proof.
The Stop domain explicitly excludes EE extended encodings and subnormals,
whose comparison behavior differs from SSE. All finite normal values and
both zeros are included; no rounded arithmetic is used by these methods.
"""
import argparse
import json
from pathlib import Path
import subprocess
from build_config import arguments, identity
from check import NINJA
from client import export_pcode
from compiled import ROOT, constant, lift
from compiled_extended import bundle_closure, sha
from call_graph import link
from execution_state import ExecutionState
from memory_relation import MemoryRelation, store
from transitions import TransitionStep, compare
from verify import Unsupported
import z3

CASES = [('FxLod', '0036a5d0', 'LodIsLooped', '?IsLooped@CLodFx@@UEAA_NXZ'),
         ('FxLod', '0036a1f0', 'LodStop', '?Stop@CLodFx@@UEAAXM@Z'),
         ('Fx', '0013fb70', 'BaseStop', '?Stop@CNewFx@@UEAAXM@Z')]
LAYOUT = {'size': 'sizeof(CLodFx)', 'flags': '__builtin_offsetof(CLodFx, flags)',
          'time': '__builtin_offsetof(CLodFx, field_0x10)',
          'handle': '__builtin_offsetof(CLodFx, field_0x80)',
          'looped': '__builtin_offsetof(CLodFx, field_0x88)',
          'id': '__builtin_offsetof(CNewFx, id)', 'fx_size': 'sizeof(CNewFx)',
          'handle_id': '__builtin_offsetof(CFxHandle, id)',
          'handle_pointer': '__builtin_offsetof(CFxHandle, pFx)'}


def prepare(build, output, ninja, ghidra, java):
    output.mkdir(parents=True, exist_ok=True)
    objects, commands, provenance = {}, {}, {}
    for source in ('FxLod', 'Fx'):
        target = 'CMakeFiles/Kya.dir/src/b-witch/' + source + '.cpp.obj'
        subprocess.run([ninja, '-C', str(build), target], check=True)
        path = build / target
        objects[source] = path.read_bytes()
        commands[source] = subprocess.check_output([ninja, '-C', str(build), '-t', 'commands', target], text=True).splitlines()[-1]
        provenance[source] = dict(object=str(path), object_sha256=sha(objects[source]),
            source_sha256=sha((ROOT / ('src/b-witch/' + source + '.cpp')).read_bytes()), command=commands[source])
    provenance['headers'] = {name: sha((ROOT / 'src/b-witch' / name).read_bytes()) for name in ('FxLod.h', 'Fx.h')}
    probe, obj = output / 'layout.cpp', output / 'layout.obj'
    probe.write_text('#include "FxLod.h"\nextern "C" {\n' + ''.join(
        'extern const unsigned int fv_' + key + ' = ' + expression + ';\n' for key, expression in LAYOUT.items()) + '}\n')
    target = 'CMakeFiles/Kya.dir/src/b-witch/FxLod.cpp.obj'
    command = commands['FxLod'].replace((ROOT / 'src/b-witch/FxLod.cpp').as_posix(), probe.as_posix())
    command = command.replace(target.replace('/', '\\') + '.d', (output / 'layout.d').as_posix()).replace(target, obj.as_posix())
    subprocess.run(command, cwd=build, check=True)
    layout = {key: constant(obj.read_bytes(), 'fv_' + key) for key in LAYOUT}
    selected, prepared, frontier = bundle_closure(objects, CASES, {})
    if prepared is None:
        raise Unsupported('Effect dependency frontier: ' + '; '.join(frontier))
    blob, functions, addresses = prepared
    manifest = dict(build=identity(build), functions=functions, addresses=addresses, layout=layout,
                    provenance=provenance, layout_command=command, layout_sha256=sha(obj.read_bytes()),
                    bundle_sha256=sha(blob))
    (output / 'functions.bin').write_bytes(blob)
    (output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    lift(output, ghidra, java)
    return json.loads((output / 'exports.json').read_text())


def compare_case(manifest, originals, name, handle='null'):
    if handle not in ('null', 'zero-id', 'stale', 'valid'):
        raise ValueError('Unknown effect handle partition')
    pc = next(e for e in manifest['exports'] if e['name'] == name)
    if any(e['executable_sha256'] != manifest['bundle_sha256'] for e in manifest['exports']):
        raise Unsupported('Effect exports do not match the compiled image')
    if len({e['executable_sha256'] for e in originals.values()}) != 1:
        raise Unsupported('Mixed PS2 effect image identities')
    ps2 = originals[pc['coff']['ps2_entry']]
    layout = manifest['layout']
    relation = MemoryRelation()
    for side in ('windows', 'ps2'):
        relation.region(side, 'lod', 0x1000, layout['size'] if side == 'windows' else 0x90)
        relation.region(side, 'child', 0x4000, layout['fx_size'] if side == 'windows' else 0x80)
        relation.region(side, 'table', 0x5000, 0x100)
        relation.region(side, 'stack', 0x7e00 if side == 'windows' else 0x7f00,
                        0x208 if side == 'windows' else 0x110, private=True)
    relation.identity(0x4000, 0x4000)
    w = z3.Array('effect_w', z3.BitVecSort(64), z3.BitVecSort(8))
    s = z3.Array('effect_s', z3.BitVecSort(32), z3.BitVecSort(8))
    argument = z3.BitVec('effect_stop_bits', 32)
    domain = z3.And(z3.Extract(30, 23, argument) != 255,
                    z3.Or(z3.Extract(30, 23, argument) != 0, argument & 0x7fffffff == 0))
    symbols = [('stop_bits', argument)]
    def field(region, wo, so, size, value, pointer=False):
        nonlocal w, s
        relation.field(region, wo, region, so, size, pointer=pointer)
        base = 0x1000 if region == 'lod' else 0x4000
        w = store(w, base + wo, z3.ZeroExt(32, value) if pointer else value)
        s = store(s, base + so, value)
    looped = z3.BitVec('effect_looped', 8)
    field('lod', layout['looped'], 0x88, 1, looped)
    if name == 'LodIsLooped':
        # C++ bool object representations are 0 and 1.
        domain = z3.Or(looped == 0, looped == 1)
    else:
        for region in ('lod', 'child'):
            for key, so in [('flags', 0x14), ('time', 0x10)]:
                value = z3.BitVec(region + '_' + key, 32)
                symbols.append((region + '_' + key, value))
                field(region, layout[key], so, 4, value)
        field('lod', layout['handle'] + layout['handle_id'], 0x80, 4,
              z3.BitVecVal(0 if handle == 'zero-id' else 5, 32))
        field('lod', layout['handle'] + layout['handle_pointer'], 0x84, 8,
              z3.BitVecVal(0 if handle == 'null' else 0x4000, 32), pointer=True)
        field('child', layout['id'], 0x18, 4, z3.BitVecVal(6 if handle == 'stale' else 5, 32))
    regions_w = [(r.base, r.size) for r in relation.regions['windows'].values() if r.name != 'stack']
    regions_s = [(r.base, r.size) for r in relation.regions['ps2'].values()]
    for symbol, address in manifest['addresses'].items():
        if symbol.startswith('__real@'):
            w = store(w, address, z3.BitVecVal(int(symbol.split('@')[1], 16), 32))
            regions_w.append((address, 4))
    if name == 'LodStop':
        base_stop = next(e for e in manifest['exports'] if e['name'] == 'BaseStop')
        # Explicit vtable storage, proved at the call site on both machines.
        w = store(store(w, 0x4000, z3.BitVecVal(0x5000, 64)), 0x5000 + 7 * 8,
                  z3.BitVecVal(int(base_stop['entry'], 16), 64))
        s = store(store(s, 0x4000, z3.BitVecVal(0x5000, 32)), 0x5024, z3.BitVecVal(0x13fb70, 32))
        def targets(body, target):
            return {int(i['address'], 16): target for i in body['instructions'] if any(o['opcode'] == 'CALLIND' for o in i['ops'])}
        pc = link(manifest['exports'], int(pc['entry'], 16), max_depth=16,
                  indirect_targets=targets(pc, int(base_stop['entry'], 16)))
        ps2 = link([ps2, originals['0013fb70']], int(ps2['entry'], 16), max_depth=16,
                   indirect_targets=targets(ps2, 0x13fb70))
    step = TransitionStep(name, pc, ps2, z3.BitVecVal(0, 64),
        windows_options=dict(regions=regions_w, this_argument=z3.BitVecVal(0x1000, 64),
                             float_argument=argument, fp_mode=True),
        ps2_options=dict(regions=regions_s, this_argument=z3.BitVecVal(0x1000, 64),
                         float_argument=argument, fp_mode=True, graph_mode=name == 'LodStop', call_mode=True),
        return_bits=8 if name == 'LodIsLooped' else 0)
    result = compare([step], ExecutionState('windows', w, processor=dict(mxcsr=0x1f80)),
                     ExecutionState('ps2', s), relation, domain, symbols=symbols)
    result.update(function=pc['name'], ps2_entry=ps2.get('coff', {}).get('ps2_entry',
        '0036a5d0' if name == 'LodIsLooped' else '0036a1f0'), handle=handle)
    result['domain'] = ('Canonical bool field; all other mapped initial bytes arbitrary' if name == 'LodIsLooped' else
        'Finite normal binary32 and signed zero; ' + handle + ' child handle; checked CNewFx::Stop dispatch')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    arguments(parser)
    parser.add_argument('--ninja', default=NINJA)
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    args = parser.parse_args()
    folder = args.output_root.resolve() / 'compiled-effects'
    folder.mkdir(parents=True, exist_ok=True)
    (folder / 'report.json').unlink(missing_ok=True)
    try:
        manifest = prepare(args.build_dir.resolve(), folder, args.ninja, args.ghidra, args.java)
        originals = {case[1]: export_pcode(case[1], args.port) for case in CASES}
        (folder / 'originals.json').write_text(json.dumps(originals, indent=2) + '\n')
        rows = [compare_case(manifest, originals, 'LodIsLooped')]
        partitions = [compare_case(manifest, originals, 'LodStop', handle) for handle in ('null', 'zero-id', 'stale', 'valid')]
        rows.append(dict(function='CLodFx::Stop', ps2_entry='0036a1f0',
                         result='equivalent' if all(r['result'] == 'equivalent' for r in partitions) else 'inconclusive',
                         partitions=partitions, domain='Finite normal binary32 and signed zero; null, zero-id, stale and valid child handles; checked CNewFx::Stop dispatch'))
        document = dict(scope='Independent LOD callee proofs; not whole fighter transitions',
                        build=manifest['build'], functions=rows, domain='Stop: finite normal binary32 and signed zero; child dispatch to checked CNewFx::Stop')
    except (Unsupported, ValueError, RuntimeError, z3.Z3Exception) as error:
        document = dict(result='inconclusive', reason=str(error), functions=[])
    (folder / 'report.json').write_text(json.dumps(document, indent=2) + '\n')
    print(json.dumps(document, indent=2))
    return 0 if document['functions'] and all(r['result'] == 'equivalent' for r in document['functions']) else 1


if __name__ == '__main__':
    raise SystemExit(main())
