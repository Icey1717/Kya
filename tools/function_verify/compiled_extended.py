"""Build and lift production branch, direct-call, and bounded-loop functions."""
import argparse
import hashlib
import json
import re
from pathlib import Path
import struct
import subprocess

from compiled import ROOT, coff_symbols, constant, lift

CASES = [
    ('ActorHero_Inventory', '001dc7b0', 'HasChanged', '?HasChanged@CInventoryInterface@@QEAA_NXZ'),
    ('ActorMoney', '00362f30', 'GetType', '?GetType@CActorMoney@@QEAAHXZ'),
    ('Actor', '001022b0', 'BaseFlags', '?GetBehaviourFlags@CActor@@UEAAIH@Z'),
    ('ActorAutonomous', '00123c50', 'AutFlags', '?GetBehaviourFlags@CActorAutonomous@@UEAAIH@Z'),
    ('ActorProjectile', '00131f10', 'IsInList', '?IsInList@?$CFixedTable@PEAVCActor@@$03@@QEAA_NPEAVCActor@@@Z'),
]
TABLES = {'?_gBehaviourFlags_ACT@CActor@@2PAIA': ('Actor', 8),
          '?_gBehaviourFlags_AUT@CActorAutonomous@@2PAIA': ('ActorAutonomous', 4)}
# Trap storage only, never executable contracts. Reaching one remains unsupported.
FAILURE_IMPORTS = {'__imp__wassert', '__imp__CrtDbgReport', '__imp_abort'}


def sha(data):
    return hashlib.sha256(data).hexdigest()


class UnapprovedRelocations(ValueError):
    def __init__(self, targets):
        self.targets = sorted(set(targets))
        super().__init__('Unapproved relocation target: ' + '; '.join(self.targets))


def bundle_closure(objects, cases, tables=None, max_functions=256, globals=None):
    """Include complete compiled direct callees; never synthesize missing bodies.

    Returns the remaining dependency frontier as data, without claiming a
    successful bundle. Globals/imports require explicit mappings or analysis.
    """
    if not isinstance(max_functions, int) or not 1 <= len(cases) <= max_functions:
        raise ValueError('Invalid direct dependency function bound or initial roots')
    if len({c[3] for c in cases}) != len(cases):
        raise ValueError('Duplicate direct dependency root symbol')
    definitions = {}
    for source, data in objects.items():
        for symbol, _, section, kind in coff_symbols(data)[0]:
            if section > 0 and kind == 0x20:
                definitions.setdefault(symbol, []).append(source)
    selected = list(cases)
    while True:
        try:
            prepared = (bundle(objects, selected, tables, globals=globals) if globals is not None
                        else bundle(objects, selected, tables))
            return selected, prepared, []
        except UnapprovedRelocations as error:
            pending = error.targets
            additions = []
            for target in pending:
                sources = definitions.get(target, [])
                if len(sources) > 1:
                    from coff_data import identical_comdat
                    source = identical_comdat(objects, sources, target)
                    sources = [source] if source is not None else sources
                if len(sources) == 1:
                    additions.append((sources[0], None, 'Dependency' + str(len(selected) + len(additions)), target))
            if not additions:
                return selected, None, pending
            if len(selected) + len(additions) > max_functions:
                raise ValueError('Direct dependency closure exceeds explicit function bound')
            selected.extend(additions)


def bundle(objects, cases=None, tables=None, *, globals=None, deferred_functions=(), jump_tables=False):
    """Resolve only AMD64 REL32 in allow-listed production slices.

    Assertion strings/imports receive trap placeholders. They are never modeled
    as no-op calls: execution must prove every assertion path unreachable.
    """
    cases = CASES if cases is None else cases
    tables = TABLES if tables is None else tables
    deferred_functions = set(deferred_functions)
    declared_functions = {name for data in objects.values() for name, _, _, kind in coff_symbols(data)[0]
                          if kind == 0x20}
    if not deferred_functions <= declared_functions or deferred_functions & {case[3] for case in cases}:
        raise ValueError('Deferred targets must be declared functions without selected bodies')
    blob, functions, addresses, pending = bytearray(), [], {}, []
    for file, ps2, name, symbol in cases:
        data = objects[file]
        symbols, sections = coff_symbols(data)
        matches = [s for s in symbols if s[0] == symbol and s[2] > 0]
        if len(matches) != 1:
            raise ValueError('Missing/ambiguous production function: ' + symbol)
        _, start, section, kind = matches[0]
        if kind != 0x20 or not 1 <= section <= sections:
            raise ValueError('Invalid production function')
        header = 20 + (section - 1) * 40
        size, raw, reloc = struct.unpack_from('<III', data, header + 16)
        count = struct.unpack_from('<H', data, header + 32)[0]
        if count == 0xffff:
            raise ValueError('Relocation overflow unsupported')
        end = min([s[1] for s in symbols if s[2] == section and s[3] == 0x20 and s[1] > start] + [size])
        if not 0 <= start < end <= size or raw + end > len(data):
            raise ValueError('Invalid slice bounds')
        while len(blob) % 16:
            blob.append(0xcc)
        base = len(blob)
        addresses[symbol] = base
        body = data[raw + start:raw + end]
        blob.extend(body)
        item = dict(name=name, symbol=symbol, ps2_entry=ps2, source=file,
                    blob_offset=base, size=len(body), coff_section=section, coff_offset=start,
                    original_sha256=sha(body), relocations=[])
        functions.append(item)
        symoff, nsyms = struct.unpack_from('<II', data, 8)
        for i in range(count):
            offset, index, typ = struct.unpack_from('<IIH', data, reloc + i * 10)
            if not start <= offset < end:
                continue
            if typ != 4 or offset + 4 > end or index >= nsyms:
                raise ValueError('Only bounded AMD64 REL32 relocations are supported')
            name_raw = data[symoff + index * 18:symoff + index * 18 + 8]
            if name_raw[:4] == b'\0' * 4:
                pos = symoff + nsyms * 18 + struct.unpack_from('<I', name_raw, 4)[0]
                target = data[pos:data.index(b'\0', pos)].decode()
            else:
                target = name_raw.rstrip(b'\0').decode()
            position = base + offset - start
            addend = struct.unpack_from('<i', blob, position)[0]
            if name_raw.rstrip(b'\0') == b'.rdata' and jump_tables:
                from coff_jump_tables import recover
                table = recover(data, file, item, offset, index)
                target, addend = table['symbol'], 0
                item.setdefault('jump_tables', []).append(table)
            pending.append((position, target, addend))
            item['relocations'].append(dict(offset=offset-start, symbol=target, addend=addend))
    for symbol, (_, size) in tables.items():
        if symbol in addresses:
            raise ValueError('Duplicate bundled symbol: ' + symbol)
        while len(blob) % 16:
            blob.append(0xcc)
        addresses[symbol] = len(blob)
        # Globals are deliberately symbolic at execution time on both sides.
        blob.extend(bytes(size))
    data_fixups = []
    for symbol, (source, size) in (globals or {}).items():
        from coff_data import extract as extract_data
        if symbol in addresses:
            raise ValueError('Duplicate bundled global: ' + symbol)
        item = extract_data(objects[source], symbol, size)
        while len(blob) % 16:
            blob.append(0xcc)
        addresses[symbol] = len(blob)
        blob.extend(item['bytes'])
        data_fixups.extend((addresses[symbol] + r['offset'], r['symbol'], r['addend'])
                           for r in item['relocations'])
    for item in functions:
        if item.get('jump_tables'):
            from coff_jump_tables import append
            item['jump_tables'] = [append(blob, table, item) for table in item['jump_tables']]
            for table in item['jump_tables']:
                if table['symbol'] in addresses:
                    raise ValueError('Duplicate jump table identity')
                addresses[table['symbol']] = table['address']
    unresolved = sorted({target for _, target, _ in pending if target not in addresses
                         and not re.fullmatch(r'__(?:real@[0-9a-fA-F]{8}|xmm@[0-9a-fA-F]{32})', target)
                         and not target.startswith('??_C@') and target not in FAILURE_IMPORTS
                         and target not in deferred_functions})
    unresolved = sorted(set(unresolved) | {target for _, target, _ in data_fixups if target not in addresses})
    if unresolved:
        raise UnapprovedRelocations(unresolved)
    for position, target, addend in pending:
        if target not in addresses:
            literal = re.fullmatch(r'__(?:real@([0-9a-fA-F]{8})|xmm@([0-9a-fA-F]{32}))', target)
            if literal:
                expected = int(literal[1] or literal[2], 16).to_bytes(4 if literal[1] else 16, 'little')
                definitions = []
                for data in objects.values():
                    for symbol, offset, section, _ in coff_symbols(data)[0]:
                        if symbol == target and section > 0:
                            raw = struct.unpack_from('<I', data, 20 + (section-1)*40 + 20)[0]
                            definitions.append(data[raw+offset:raw+offset+len(expected)])
                if not definitions or any(value != expected for value in definitions):
                    raise ValueError('Missing/conflicting float literal definition')
                addresses[target] = len(blob)
                blob.extend(expected)
            elif not (target.startswith('??_C@') or target in FAILURE_IMPORTS or target in deferred_functions):
                raise ValueError('Unapproved relocation target: ' + target)
            else:
                addresses[target] = len(blob)
                blob.extend(b'\xcc' * 16)
        struct.pack_into('<i', blob, position, addresses[target] + addend - (position + 4))
    for position, target, addend in data_fixups:
        value = addresses[target] + addend
        if not 0 <= value < 1 << 64:
            raise ValueError('ADDR64 global relocation overflow')
        struct.pack_into('<Q', blob, position, value)
    for item in functions:
        start = item['blob_offset']
        item['bytes_sha256'] = sha(blob[start:start + item['size']])
    return bytes(blob), functions, addresses


def main():
    from build_config import arguments, identity
    parser = argparse.ArgumentParser(description=__doc__)
    arguments(parser)
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    parser.add_argument('--ninja', default='C:/Program Files/Microsoft Visual Studio/18/Insiders/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe')
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--export-only', action='store_true')
    parser.add_argument('--suite', choices=['control-flow', 'subsystems', 'combat'], default='control-flow')
    parser.add_argument('--mutate', choices=['branch', 'call', 'loop', 'cursor', 'flip'], help='Compile an isolated incorrect implementation (expected comparison failure)')
    args = parser.parse_args()
    if args.mutate and ((args.suite == 'subsystems') != (args.mutate == 'cursor') or (args.suite == 'combat') != (args.mutate == 'flip')):
        parser.error('Mutation must match its suite: cursor/subsystems, flip/combat, branch|call|loop/control-flow')
    if args.suite == 'subsystems':
        from subsystems import CASES as selected_cases, LAYOUT_SOURCE
    elif args.suite == 'combat':
        from combat_cases import CASES as selected_cases, LAYOUT_SOURCE, LAYOUT_KEYS
    else:
        selected_cases = CASES
    build = args.build_dir.resolve()
    configuration = identity(build)
    output = args.output_root.resolve() / ({'subsystems': 'compiled-subsystems', 'combat': 'compiled-combat', 'control-flow': 'compiled-extended'}[args.suite] + ('-mutant-' + args.mutate if args.mutate else ''))
    output.mkdir(parents=True, exist_ok=True)
    targets = {f: 'CMakeFiles/Kya.dir/src/b-witch/' + f + '.cpp.obj' for f, _, _, _ in selected_cases}
    subprocess.run([args.ninja, '-C', str(build), *targets.values()], check=True)
    commands, provenance, objects = {}, {}, {}
    for file, target in targets.items():
        commands[file] = subprocess.check_output([args.ninja, '-C', str(build), '-t', 'commands', target], text=True).splitlines()[-1]
        path = build / target
        objects[file] = path.read_bytes()
        source = ROOT / ('src/b-witch/' + file + '.cpp')
        provenance[file] = dict(object=str(path), object_sha256=sha(objects[file]),
                                source=str(source), source_sha256=sha(source.read_bytes()), command=commands[file])
    if args.mutate:
        file = {'branch': 'ActorMoney', 'call': 'ActorAutonomous', 'loop': 'ActorProjectile', 'cursor': 'MemoryStream', 'flip': 'ActorFighter'}[args.mutate]
        source = ROOT / ('src/b-witch/' + file + '.cpp')
        mutant_source = output / (file + '-mutated.cpp')
        mutant_object = output / 'mutated.obj'
        target = targets[file]
        command = commands[file].replace(target.replace('/', '\\') + '.d', (output / 'mutated.d').as_posix())
        command = command.replace(target, mutant_object.as_posix())
        if args.mutate == 'loop':
            # Expand includes first so a disposable template edit cannot affect
            # the production header or any other build target.
            preprocessed = output / 'original.ii'
            preprocess = command.replace(mutant_object.as_posix(), preprocessed.as_posix()).replace(' -c ', ' -E ')
            subprocess.run(preprocess, cwd=build, check=True)
            text = preprocessed.read_text()
            pattern = r'(inline bool IsInList\(T value\)\s*\{\s*for \(int i = 0; i < )this->nbEntries'
            text, count = re.subn(pattern, r'\1(this->nbEntries - 1)', text)
        else:
            text = source.read_text()
            before, after = {'branch': ('if (this->moneyValue < 2)', 'if (this->moneyValue < 3)'),
                             'call': ('CActor::GetBehaviourFlags(state)', 'CActor::GetBehaviourFlags(0)'),
                             'cursor': ('this->currentSeekPos = this->currentSeekPos + sizeof(uint);',
                                        'this->currentSeekPos = this->currentSeekPos + 8;'),
                             'flip': ('void CActorFighter::_EndFighterFlip()\n{\n\tCCollision* pCVar1;\n\n\tpCVar1 = this->pCollisionData;\n\tpCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;',
                                      'void CActorFighter::_EndFighterFlip()\n{\n\tCCollision* pCVar1;\n\n\tpCVar1 = this->pCollisionData;\n\tpCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x2000;')}[args.mutate]
            count = text.count(before)
            text = text.replace(before, after)
        if count != 1:
            raise ValueError('Mutation target missing/ambiguous')
        mutant_source.write_text(text)
        command = command.replace(source.as_posix(), mutant_source.as_posix())
        subprocess.run(command, cwd=build, check=True)
        objects[file] = mutant_object.read_bytes()
        provenance[file].update(object=str(mutant_object), object_sha256=sha(objects[file]), command=command,
                                source=str(mutant_source), source_sha256=sha(mutant_source.read_bytes()))
    blob, functions, addresses = bundle(objects, selected_cases, TABLES if args.suite == 'control-flow' else {})
    (output / 'functions.bin').write_bytes(blob)
    layout_source, layout_object = output / 'layout.cpp', output / 'layout.obj'
    layout_text = ('#include "ActorHero_Inventory.h"\n#include "ActorMoney.h"\n'
        'using PilotTable = CFixedTable<CActor*, 4>;\nextern "C" {\n'
        'extern const unsigned int dirty = __builtin_offsetof(CInventoryInterface, bDirty);\n'
        'extern const unsigned int money = __builtin_offsetof(CActorMoney, moneyValue);\n'
        'extern const unsigned int count = __builtin_offsetof(PilotTable, nbEntries);\n'
        'extern const unsigned int entries = __builtin_offsetof(PilotTable, aEntries);\n'
        'extern const unsigned int pointer_size = sizeof(CActor*);\n}\n')
    layout_keys = ['dirty', 'money', 'count', 'entries', 'pointer_size']
    if args.suite == 'subsystems':
        layout_text = LAYOUT_SOURCE
        layout_keys = ['cursor', 'slot', 'magic', 'priority', 'pointer_size']
    elif args.suite == 'combat':
        layout_text, layout_keys = LAYOUT_SOURCE, LAYOUT_KEYS
    layout_source.write_text(layout_text)
    layout_file = {'subsystems': 'MemoryStream', 'combat': 'ActorFighter', 'control-flow': 'Actor'}[args.suite]
    target = targets[layout_file]
    command = commands[layout_file].replace((ROOT / ('src/b-witch/' + layout_file + '.cpp')).as_posix(), layout_source.as_posix())
    command = command.replace(target.replace('/', '\\') + '.d', (output / 'layout.d').as_posix())
    command = command.replace(target, layout_object.as_posix())
    subprocess.run(command, cwd=build, check=True)
    layout = {key: constant(layout_object.read_bytes(), key) for key in layout_keys}
    manifest = dict(build=configuration, functions=functions, addresses=addresses, provenance=provenance, layout=layout, mutation=args.mutate, suite=args.suite,
                    layout_command=command, compiler_version=subprocess.check_output([commands[layout_file].split()[0], '--version'], text=True))
    (output / 'manifest.json').write_text(json.dumps(manifest, indent=2))
    lift(output, args.ghidra, args.java)
    if not args.export_only:
        from compare_extended import run
        return run(output / 'exports.json', args.port)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
