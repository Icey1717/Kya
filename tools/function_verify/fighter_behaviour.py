"""Prepare complete fighter behaviour bodies with checked switch-table frontiers."""
import argparse
import json
from pathlib import Path
import subprocess

from compiled import ROOT, coff_symbols, lift
from compiled_extended import bundle, sha
from client import export_pcode, request
from fighter_configuration import prepare_and_run as configure
from fighter_dispatch import recover as recover_dispatch
from transition_data import prepare


ROOTS = [
    ('Animation', '001801b0', 'ControllerPlayAnim', '?PlayAnim@CAnimation@@QEAAXPEAVCActor@@HH@Z'),
    ('Animation', '00242230', 'MetaSetAnim', '?SetAnim@edAnmBinMetaAnimator@@QEAAXHH@Z'),
    ('AnmManager', '0017f470', 'CallbackLayer0', '?gAnimation_Callback_Layer0@@YAXPEAUedAnmMacroAnimator@@PEAVCActor@@I@Z'),
    ('ActorFighter', '0031baa0', 'FighterAnimEvaluate', '?AnimEvaluate@CActorFighter@@UEAAXIPEAUedAnmMacroAnimator@@I@Z'),
    ('Actor', '00100bf0', 'ActorAnimEvaluate', '?AnimEvaluate@CActor@@UEAAXIPEAUedAnmMacroAnimator@@I@Z'),
    ('Animation', '002410e0', 'MorphingStart', '?MorphingStartDT@edAnmLayer@@QEAA_NXZ'),
    ('Animation', '002411d0', 'MorphingInit', '?MorphingInitDT@edAnmLayer@@QEAA_NPEAUedAnmStateDesc@@@Z'),
    ('AnmManager', '00241b40', 'MacroUpdateAnimParams', '?UpdateAnimParams@edAnmMacroAnimator@@QEAAXXZ'),
    ('Animation', '00240530', 'LayerSetAnim', '?SetAnim@edAnmLayer@@QEAAXPEAUedAnmStateDesc@@@Z'),
    ('AnmManager', '00241ce0', 'MacroInitialize', '?Initialize@edAnmMacroAnimator@@QEAAXMPEAUedANM_HDR@@_NI@Z'),
    ('Animation', '00241490', 'BuildAnimDesc', '?BuildDesc@edAnmStateParser@@QEAAPEAUedAnmStateDesc@@PEAU2@HH@Z'),
    ('ActorFighter', '00313d80', 'FighterInitState', '?InitState@CBehaviourFighter@@UEAAXH@Z'),
    ('ActorFighter', '00313290', 'FighterTermState', '?TermState@CBehaviourFighter@@UEAAXHH@Z'),
    ('TimeController', '001ba810', 'GetTimer', '?GetTimer@@YAPEAVTimer@@XZ'),
    ('Actor', '00100400', 'LookingAtOn', '?SetLookingAtOn@CActor@@UEAAXXZ'),
    ('ActorFighter', '0031ba50', 'SetStandAnim', '?SetStandAnim@CActorFighter@@QEAAXH@Z'),
    ('ActorMovable', '0011e7c0', 'MovableSetState', '?SetState@CActorMovable@@UEAAXHH@Z'),
    ('Actor', '00103e70', 'GetIdMacroAnim', '?GetIdMacroAnim@CActor@@QEAAHH@Z'),
    ('Animation', '00180450', 'AnimationReset', '?Reset@CAnimation@@QEAAXPEAVCActor@@@Z'),
    ('Animation', '00241460', 'LayerReset', '?Reset@edAnmLayer@@QEAAXXZ'),
    ('Animation', '002426f0', 'LayerCallback', '?SetLayerMacroAnimCallback@edAnmBinMetaAnimator@@QEAAXHP6AXPEAUedAnmMacroAnimator@@PEAVCActor@@I@Z@Z'),
    ('Animation', '00242680', 'LayerUserParams', '?SetLayerMacroAnimUserParams@edAnmBinMetaAnimator@@QEAAXHPEAVCActor@@@Z'),
]


def graphs(manifest, originals, function='SetState'):
    from call_graph import link
    from coff_jump_tables import targets
    windows, ps2 = {}, {}
    symbols = {e['coff']['symbol']: int(e['entry'], 16) for e in manifest['exports']}
    getter = symbols['?GetStateCfg@CActorFighter@@UEAAPEAUStateConfig@@H@Z']
    term = symbols['?TermState@CBehaviourFighter@@UEAAXHH@Z']
    init = symbols['?InitState@CBehaviourFighter@@UEAAXH@Z']
    movable = symbols.get('?SetState@CActorMovable@@UEAAXHH@Z')
    for body in manifest['exports']:
        windows.update(targets(body))
        for instruction in body['instructions']:
            if not any(op['opcode'] == 'CALLIND' for op in instruction['ops']):
                continue
            text = instruction.get('text', '')
            if text.endswith(' + 0x98]'):
                windows[int(instruction['address'], 16)] = getter
            elif body['coff']['symbol'] == '?SetStandAnim@CActorFighter@@QEAAXH@Z':
                if function != 'SetStandAnim':
                    continue
                if movable is None:
                    raise ValueError('Missing concrete movable SetState dispatch body')
                windows[int(instruction['address'], 16)] = movable
            elif body['coff']['symbol'] == '?SetState@CActor@@UEAAXHH@Z':
                if text.endswith(' + 0x60]'):
                    windows[int(instruction['address'], 16)] = term
                elif text.endswith(' + 0x58]'):
                    windows[int(instruction['address'], 16)] = init
    for body in originals.values():
        for index, instruction in enumerate(body['instructions']):
            if not any(op['opcode'] == 'CALLIND' for op in instruction['ops']):
                continue
            previous = [row['text'] for row in body['instructions'][max(0, index - 3):index]
                        if row.get('text', '').startswith('lw t9,')]
            if previous and previous[-1] == 'lw t9,0x54(t9)':
                ps2[int(instruction['address'], 16)] = 0x31dd50
    ps2.update({0x102b44: 0x313290, 0x102bac: 0x313d80})
    if function not in ('SetState', 'SetStandAnim'):
        raise ValueError('Unknown fighter transition entry')
    root = symbols['?SetState@CActor@@UEAAXHH@Z' if function == 'SetState' else
                   '?SetStandAnim@CActorFighter@@QEAAXH@Z']
    if function == 'SetStandAnim':
        ps2[0x31ba80] = 0x11e7c0
    return (link(manifest['exports'], root, max_depth=20, indirect_targets=windows, defer_unresolved=True),
            link(list(originals.values()), 0x102a00 if function == 'SetState' else 0x31ba50,
                 max_depth=20, indirect_targets=ps2, defer_unresolved=True))


def main():
    from build_config import arguments, identity
    from check import NINJA
    parser = argparse.ArgumentParser(description=__doc__)
    arguments(parser)
    parser.add_argument('--base-artifacts', type=Path, default=ROOT / 'out/verification/all-state-config-v2/fighter-runtime')
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--ninja', default=NINJA)
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    args = parser.parse_args()
    build, output = args.build_dir.resolve(), args.output_root.resolve()
    objects = {}
    globals, memory = prepare(build, output / 'memory', args.ninja, objects)
    sources = ('ActorMovable', 'Audio', 'EventTrack', 'Fx', 'ActorFactory', 'Dynamic')
    locations = ['CMakeFiles/Kya.dir/src/b-witch/' + source + '.cpp.obj' for source in sources]
    subprocess.run([args.ninja, '-C', str(build), *locations], check=True)
    objects.update({source: (build / location).read_bytes() for source, location in zip(sources, locations)})
    previous = json.loads((args.base_artifacts / 'manifest.json').read_text())
    selected = [(row['source'], row.get('ps2_entry'), row['name'], row['symbol']) for row in previous['functions']]
    recovered = {case[3]: case for case in ROOTS}
    # A dependency can already be compiled without an original entry assigned.
    # Promote its metadata as well as adding previously absent roots.
    selected = [recovered.get(case[3], case) for case in selected]
    for case in ROOTS:
        if case[3] not in {row[3] for row in selected}:
            selected.append(case)
    names = {row[3] for row in selected}
    deferred = {symbol for data in objects.values() for symbol, _, _, kind in coff_symbols(data)[0]
                if kind == 0x20 and symbol not in names}
    # Import pointer storage is retained as an explicit frontier. No runtime
    # memory region or callable body is supplied for acosf; reaching it fails.
    import_slots = {'__imp_acosf': ('ActorFighter', 8)}
    blob, functions, addresses = bundle(objects, selected, import_slots, globals=globals,
                                         deferred_functions=deferred, jump_tables=True)
    folder = output / 'fighter-behaviour'
    folder.mkdir(parents=True, exist_ok=True)
    configuration = configure(objects, globals, memory, output / 'fighter-configuration',
                              args.ghidra, args.java, identity(build), args.port)
    dispatch = recover_dispatch(build, args.port)
    entries = {row[1] for row in selected if row[1]}
    entries.add('00103db0')  # CActor::PlayAnim, reached after state entry.
    originals = {address: export_pcode(address, args.port) for address in sorted(entries)}
    if len({body['executable_sha256'] for body in originals.values()}) != 1:
        raise ValueError('Mixed original executables in behaviour closure')
    # Audited InitState loads at 00314560..00314574.
    zero = request('inspect_data', args.port, address='00431700', length=16)
    callbacks = request('inspect_data', args.port, address='0040e760', length=16)
    manifest = dict(build=identity(build), functions=functions, addresses=addresses, layouts=memory['layouts'],
                    frontier_import_slots=list(import_slots),
                    globals=memory['globals'], configuration=configuration, dispatch=dispatch,
                    bundle_sha256=sha(blob), object_sha256={name: sha(data) for name, data in objects.items()},
                    deferred_functions={name: addresses[name] for name in deferred if name in addresses},
                    jump_table_bytes={table['symbol']: blob[table['address']:table['address'] + table['entries'] * 4].hex()
                                      for function in functions for table in function.get('jump_tables', [])},
                    ps2_data={'zero_vector': zero, 'animation_callbacks': callbacks})
    (folder / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    (folder / 'functions.bin').write_bytes(blob)
    (folder / 'originals.json').write_text(json.dumps(originals, indent=2) + '\n')
    lift(folder, args.ghidra, args.java)
    from semantic_frontier import inventory
    exports = json.loads((folder / 'exports.json').read_text())['exports']
    (folder / 'semantic-obligations.json').write_text(json.dumps(dict(
        windows=inventory(exports), ps2=inventory(originals.values())), indent=2) + '\n')
    print(folder)


if __name__ == '__main__':
    main()
