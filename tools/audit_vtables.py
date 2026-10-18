#!/usr/bin/env python3
"""Read-only PS2 vtable/source override audit using the Totala Ghidra bridge.

This is a conservative declaration scanner, not a C++ compiler or an ABI checker.
It compares original table targets, not Windows vtable addresses/layouts.
"""

import argparse
from collections import Counter
from dataclasses import asdict, dataclass, field
from datetime import datetime, timezone
import fnmatch
import json
from pathlib import Path
import re
import socket
import struct
import sys


ROOT = Path(__file__).resolve().parents[1]
NOISE = re.compile(
    r'R"(?P<d>[^\s()\\]{0,16})\(.*?\)(?P=d)"|//[^\n]*|/\*.*?\*/'
    r'|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
CLASS = re.compile(r'\b(class|struct)\s+(\w+)\s*(?:final\s*)?(?::([^;{}]+))?\s*\{')
READ_COMMANDS = {'status', 'list_functions', 'references_from', 'get_labels',
                 'inspect_data', 'get_function', 'get_data_type_members'}


def masked(text):
    return NOISE.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


def close_delimiter(text, start, opening='{', closing='}'):
    depth = 0
    for i in range(start, len(text)):
        if text[i] == opening:
            depth += 1
        elif text[i] == closing:
            depth -= 1
            if depth == 0:
                return i
    raise ValueError(f'Unclosed {opening} at offset {start}')


def split_params(text):
    parts, start, depth = [], 0, 0
    for i, c in enumerate(text):
        if c in '(<[{':
            depth += 1
        elif c in ')>]}':
            depth -= 1
        elif c == ',' and depth == 0:
            parts.append(text[start:i].strip())
            start = i + 1
    parts.append(text[start:].strip())
    return [] if parts in ([''], ['void']) else parts


@dataclass
class Method:
    name: str
    arity: int
    virtual: bool
    const: bool
    declaration: str
    param_types: tuple = ()


def parameter_type(param):
    param = param.split('=', 1)[0].strip()
    # Strip a parameter identifier, not an unnamed builtin type.
    match = re.search(r'\b(\w+)\s*(\[[^]]*\])?$', param)
    builtins = {'int', 'float', 'double', 'void', 'char', 'short', 'long', 'bool', 'signed', 'unsigned'}
    if match and match[1] not in builtins and param[:match.start()].strip() not in {'', 'const', 'volatile', 'struct', 'class'}:
        param = param[:match.start()] + (match[2] or '')
    param = re.sub(r'\bunsigned\s+int\b', 'uint', param)
    return re.sub(r'\s+', '', param)


@dataclass
class SourceClass:
    name: str
    bases: list
    file: str
    line: int
    methods: list = field(default_factory=list)
    issues: list = field(default_factory=list)


def parse_methods(body):
    """Read immediate members, skipping nested class bodies and inline function bodies."""
    result, start, i = [], 0, 0
    while i < len(body):
        if body[i] not in ';{':
            i += 1
            continue
        declaration = re.sub(r'\b(public|protected|private)\s*:', '', body[start:i]).strip()
        # Declarations with function-pointer fields are deliberately excluded.
        match = re.search(r'(~?\w+)\s*\(', declaration)
        if match and not re.search(r'\b(class|struct|enum|typedef|using|friend|static)\b|\(\s*[*&]', declaration):
            p = match.end() - 1
            end = close_delimiter(declaration, p, '(', ')')
            params = split_params(declaration[p + 1:end])
            result.append(Method(match[1], len(params),
                                 bool(re.search(r'\b(virtual|override)\b', declaration)),
                                 bool(re.search(r'\bconst\b', declaration[end + 1:])),
                                 ' '.join(declaration.split()), tuple(parameter_type(p) for p in params)))
        if body[i] == '{':
            i = close_delimiter(body, i)
        start = i + 1
        i += 1
    return result


def scan_source(source):
    classes, duplicates = {}, []
    for path in sorted(source.rglob('*')):
        if path.suffix not in {'.h', '.hpp', '.cpp', '.inl'}:
            continue
        code = masked(path.read_text(encoding='utf-8', errors='replace'))
        for match in CLASS.finditer(code):
            end = close_delimiter(code, match.end() - 1)
            bases = []
            issues = []
            for base in (match[3] or '').split(','):
                base = re.sub(r'\b(public|protected|private|virtual)\b', '', base).strip()
                if base:
                    if re.fullmatch(r'\w+(?:::\w+)*', base):
                        bases.append(base.split('::')[-1])
                    else:
                        issues.append('template/complex base: ' + base)
            name = match[2]
            cls = SourceClass(name, bases, str(path.resolve()), code.count('\n', 0, match.start()) + 1,
                              parse_methods(code[match.end():end]), issues)
            if name in classes:
                # Namespace/template/preprocessor ambiguities must not become false passes.
                classes[name].issues.append('duplicate class name/conditional definition')
                duplicates.append(cls)
            else:
                classes[name] = cls
    return classes, duplicates


def virtual_methods(name, classes, visiting=None):
    visiting = set() if visiting is None else visiting
    if name in visiting or name not in classes:
        return set()
    cls = classes[name]
    inherited = set()
    for base in cls.bases:
        inherited |= virtual_methods(base, classes, visiting | {name})
    return inherited | {m.name for m in cls.methods if m.virtual and m.name not in {name, '~' + name}}


class BridgeError(RuntimeError):
    pass


class Bridge:
    def __init__(self, host, port, snapshot, offline=False, refresh=False):
        self.host, self.port, self.path, self.offline = host, port, snapshot, offline
        self.data = {'schema': 1, 'responses': {}}
        if snapshot.exists() and not refresh:
            self.data = json.loads(snapshot.read_text(encoding='utf-8'))
            if self.data.get('schema') != 1:
                raise BridgeError('Unsupported snapshot schema')
        if offline and not snapshot.exists():
            raise BridgeError('Offline snapshot does not exist')
        if offline and self.data.get('program', {}).get('language') != 'r5900:LE:32:default':
            raise BridgeError('Offline snapshot is missing the supported PS2 program identity')
        if not offline:
            status = self.call('status', fresh=True)
            old = self.data.get('program')
            identity = {k: status.get(k) for k in ('program', 'executable_path', 'image_base', 'language')}
            if old and old != identity:
                raise BridgeError('Snapshot belongs to another Ghidra program; use a different snapshot or --refresh')
            if status.get('language') != 'r5900:LE:32:default':
                raise BridgeError('This checker currently supports the little-endian 32-bit PS2 R5900 ABI only')
            self.data['program'] = identity

    def call(self, command, fresh=False, **args):
        if command not in READ_COMMANDS:
            raise BridgeError('Non-read-only bridge command refused: ' + command)
        request = {'command': command, **args}
        key = json.dumps(request, sort_keys=True)
        if not fresh and key in self.data['responses']:
            response = self.data['responses'][key]
        else:
            if self.offline:
                raise BridgeError('Snapshot has no response for ' + key)
            try:
                with socket.create_connection((self.host, self.port), timeout=30) as sock:
                    sock.sendall((json.dumps(request) + '\n').encode())
                    line = sock.makefile('rb').readline(32 * 1024 * 1024)
                response = json.loads(line)
            except (OSError, ValueError) as exc:
                raise BridgeError(f'Ghidra bridge {self.host}:{self.port}: {exc}') from exc
            self.data['responses'][key] = response
        if not response.get('ok'):
            raise BridgeError(response.get('error', 'Invalid bridge response'))
        return response['result']

    def save(self):
        if not self.offline:
            self.data['captured_at'] = datetime.now(timezone.utc).isoformat()
            self.path.parent.mkdir(parents=True, exist_ok=True)
            temporary = self.path.with_suffix(self.path.suffix + '.tmp')
            temporary.write_text(json.dumps(self.data, indent=2), encoding='utf-8')
            temporary.replace(self.path)


class Auditor:
    def __init__(self, bridge, classes, max_slots=256):
        self.bridge, self.classes, self.max_slots = bridge, classes, max_slots
        self.tables, self.targets = {}, {}
        self.slot_hints = {}
        self.discovered = {}

    def discover(self, name):
        functions = self.bridge.call('list_functions', filter=name, limit=1000)['functions']
        if len(functions) == 1000:
            raise BridgeError('Constructor function search was truncated')
        constructors = [f for f in functions if f['name'] == name and f['namespace'].split('::')[-1] == name]
        tables = set()
        for ctor in constructors:
            refs = self.bridge.call('references_from', address=ctor['entry'], scope='function', limit=5000)['references']
            if len(refs) == 5000:
                raise BridgeError('Constructor references truncated')
            for address in sorted({r['to'] for r in refs if re.fullmatch(r'[0-9a-fA-F]{8}', r['to'])
                                   and r['type'] not in {'UNCONDITIONAL_CALL', 'CONDITIONAL_CALL', 'CONDITIONAL_JUMP', 'UNCONDITIONAL_JUMP'}}):
                labels = self.bridge.call('get_labels', address=address)['labels']
                for label in labels:
                    owner = label['namespace'].split('::')[-1]
                    if label['name'] in {'_vt', 'vtable', 'vftable'} and owner in self.classes:
                        self.discovered.setdefault(owner, set()).add(address)
                        if owner == name:
                            tables.add(address)
        tables |= self.discovered.get(name, set())
        if len(tables) != 1:
            raise BridgeError(f'Expected one constructor-referenced vtable, found {len(tables)} (constructors: {len(constructors)})')
        return next(iter(tables))

    def target(self, address, seen=None):
        if address in self.targets:
            return self.targets[address]
        seen = set() if seen is None else seen
        if address in seen:
            return {'address': address, 'canonical': address, 'kind': 'unknown', 'reason': 'thunk cycle'}
        raw = bytes.fromhex(self.bridge.call('inspect_data', address=address, length=32)['hex'])
        result = {'address': address, 'canonical': address, 'kind': 'unknown', 'bytes': raw.hex()}
        words = struct.unpack('<' + 'I' * (len(raw) // 4), raw[:len(raw) // 4 * 4])
        # Follow only an unconditional MIPS j with nop/this-adjustment delay slot.
        if len(words) >= 2 and words[0] >> 26 == 2 and (words[1] == 0 or words[1] & 0xffff0000 in {0x24840000, 0x64840000}):
            dest = ((int(address, 16) + 4) & 0xf0000000) | ((words[0] & 0x03ffffff) << 2)
            inner = self.target(f'{dest:08x}', seen | {address})
            result.update(kind='thunk', canonical=inner['canonical'], function=inner.get('function'))
        else:
            try:
                function = self.bridge.call('get_function', address=address)
                if function['entry'].lower() == address.lower():
                    result.update(kind='function', function=function)
            except BridgeError:
                pass
            # Missing Ghidra function definitions are common for tiny virtual getters.
            if result['kind'] == 'unknown' and len(words) >= 2 and words[0] == 0x03e00008:
                result['kind'] = 'unnamed_leaf'
            if len(words) >= 4 and words[0] == 0x8c990000 and words[1] & 0xffff0000 == 0x8f390000 and words[2:4] == (0x03200008, 0):
                result.update(kind='virtual_dispatch', dispatch_offset=words[1] & 0xffff)
        self.targets[address] = result
        return result

    def capture(self, name, address):
        raw = bytes.fromhex(self.bridge.call('inspect_data', address=address, length=min(4096, (self.max_slots + 2) * 4))['hex'])
        words = struct.unpack('<' + 'I' * (len(raw) // 4), raw[:len(raw) // 4 * 4])
        if len(words) < 3 or words[:2] != (0, 0):
            raise BridgeError('Unsupported vtable header (expected two zero words)')
        entries, boundary = [], 'slot limit reached'
        for index, word in enumerate(words[2:self.max_slots + 2], 2):
            if word == 0:
                boundary = 'zero entry / possible next table header'
                break
            # A table label on this slot is a hard boundary even if headerless.
            try:
                target = self.target(f'{word:08x}')
            except BridgeError:
                boundary = 'unmapped/non-code entry'
                break
            if target['kind'] == 'unknown':
                boundary = f'unresolved target at +0x{index * 4:x}'
                break
            entries.append({'offset': index * 4, 'target': target})
        if not entries:
            raise BridgeError('No function targets recovered')
        return {'address': address, 'entries': entries, 'boundary': boundary,
                'bounded': boundary != 'slot limit reached'}

    def method_name(self, offset, child, parent, inherited, use_actor_hints=False):
        # An inherited source name is stronger evidence than an inferred new name.
        possibilities = []
        for target in (parent, child):
            name = (target.get('function') or {}).get('name', '')
            if name in inherited:
                possibilities.append(name)
        offset_name = f'Func_0x{offset:x}'
        if offset_name in inherited:
            possibilities.append(offset_name)
        # Actor table offsets have no meaning in unrelated virtual hierarchies.
        hint = self.slot_hints.get(offset) if use_actor_hints else None
        if hint in inherited:
            possibilities.append(hint)
        unique = set(possibilities)
        return next(iter(unique)) if len(unique) == 1 else None

    def compare(self, name):
        cls, table = self.classes[name], self.tables.get(name)
        if not table:
            return []
        if len(cls.bases) > 1 or cls.issues:
            return [{'class': name, 'kind': 'review', 'reason': 'complex/ambiguous source class: ' + '; '.join(cls.issues or ['multiple inheritance'])}]
        if not cls.bases:
            return []
        base = cls.bases[0]
        if base not in self.tables:
            return [{'class': name, 'kind': 'review', 'reason': 'No original direct-parent table: ' + base}]
        parent_slots = {e['offset']: e['target'] for e in self.tables[base]['entries']}
        inherited = virtual_methods(base, self.classes)
        own_names = {m.name for m in cls.methods}
        ancestors, pending = set(), [base]
        while pending:
            ancestor = pending.pop()
            if ancestor not in ancestors and ancestor in self.classes:
                ancestors.add(ancestor)
                pending.extend(self.classes[ancestor].bases)
        findings = []
        for entry in table['entries']:
            offset, child = entry['offset'], entry['target']
            if offset == 8:  # ABI destructor/free entry; implicit C++ destructors are not misses.
                continue
            if offset not in parent_slots:
                continue  # Newly introduced slots are not missing overrides of a parent slot.
            parent = parent_slots[offset]
            if child['canonical'] == parent['canonical']:
                continue
            # Equivalent tiny leaf bodies are often emitted once per class.
            if child.get('bytes', '').startswith('0800e003') and child['bytes'][:16] == parent.get('bytes', '')[:16]:
                continue
            method = self.method_name(offset, child, parent, inherited, 'CActor' in ancestors)
            if method in own_names:
                # Name presence alone cannot prove that an overloaded signature overrides.
                own = [m for m in cls.methods if m.name == method]
                inherited_decls = self.inherited_declarations(base, method)
                if len(own) == len(inherited_decls) == 1 and own[0].arity == inherited_decls[0].arity and own[0].const == inherited_decls[0].const and own[0].param_types == inherited_decls[0].param_types:
                    continue
                reason = 'Overload/signature requires compiler review'
                kind = 'review'
            else:
                kind = 'missing_override' if method else 'review'
                reason = 'Changed PS2 target; class has no declaration of inherited virtual' if method else 'Changed target but slot name could not be mapped to an inherited source virtual'
                if method and len(self.inherited_declarations(base, method)) != 1:
                    kind, reason = 'review', 'Overloaded inherited name; slot signature is ambiguous'
                if child['kind'] == 'virtual_dispatch':
                    kind, reason = 'review', f"Virtual-dispatch wrapper to slot +0x{child['dispatch_offset']:x}; inherited source behavior needs review"
            findings.append({'class': name, 'parent': base, 'kind': kind, 'method': method,
                             'offset': offset, 'table': table['address'], 'target': child['address'],
                             'parent_target': parent['address'], 'target_kind': child['kind'], 'reason': reason})
        return findings

    def inherited_declarations(self, name, method):
        if name not in self.classes:
            return []
        cls = self.classes[name]
        own = [m for m in cls.methods if m.name == method]
        if own:
            return own
        return [m for base in cls.bases for m in self.inherited_declarations(base, method)]


def markdown(report):
    captured = sum(bool(c.get('table')) for c in report['classes'])
    lines = ['# PS2 vtable override audit', '',
             f"Program: `{report.get('program', {}).get('program', 'inventory only')}`. Source classes: {len(report['classes'])}; tables captured: {captured}.", '',
             'Declaration and target comparison only; findings require review. Unavailable/partial coverage is not a pass.', '',
             '## Findings', '', '| Class | Kind | Method / slot | PS2 target | Reason |', '|---|---|---|---|---|']
    for f in report['findings']:
        slot = f"{f.get('method') or '?'} / +0x{f['offset']:x}" if 'offset' in f else '?'
        lines.append(f"| {f['class']} | {f['kind']} | {slot} | {f.get('target', '')} | {f['reason']} |")
    lines += ['', '## Coverage', '', '| Class | Source | Table | Slots | Boundary / limitation |', '|---|---|---|---|---|']
    for c in report['classes']:
        table = c.get('table') or {}
        lines.append(f"| {c['name']} | {c['file']}:{c['line']} | {table.get('address', '')} | {len(table.get('entries', []))} | {c.get('error') or table.get('boundary', 'not captured')} |")
    return '\n'.join(lines) + '\n'


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=ROOT / 'src/b-witch')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/vtable-audit')
    parser.add_argument('--snapshot', type=Path, default=ROOT / 'out/vtable-audit/ghidra-snapshot.json')
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--class', dest='patterns', action='append', help='Class glob; repeatable. Parent tables are included automatically.')
    parser.add_argument('--inventory-only', action='store_true')
    parser.add_argument('--offline', action='store_true')
    parser.add_argument('--refresh', action='store_true', help='Discard saved Ghidra responses after annotations change')
    parser.add_argument('--max-slots', type=int, default=256)
    parser.add_argument('--table-map', type=Path, help='Optional read-only Ghidra ExportVtableMap.java output for tables not found in constructors')
    parser.add_argument('--fail-on-findings', action='store_true')
    args = parser.parse_args(argv)
    if args.offline and args.refresh:
        parser.error('--offline and --refresh cannot be combined')
    if not 1 <= args.max_slots <= 1022:
        parser.error('--max-slots must be between 1 and 1022')
    if not args.source.is_dir():
        parser.error('--source must be an existing directory')
    classes, duplicates = scan_source(args.source)
    if not classes:
        parser.error('No class definitions found in the source directory')
    selected = {n for n in classes if virtual_methods(n, classes) and
                (not args.patterns or any(fnmatch.fnmatchcase(n, p) for p in args.patterns))}
    if args.patterns and not selected:
        parser.error('No virtual classes match the requested patterns')
    pending = list(selected)
    while pending:
        for base in classes[pending.pop()].bases:
            if base in classes and base not in selected:
                selected.add(base)
                pending.append(base)
    report = {'schema': 1, 'classes': [], 'findings': [], 'duplicates': [asdict(c) for c in duplicates]}
    bridge = None
    try:
        if not args.inventory_only:
            bridge = Bridge(args.host, args.port, args.snapshot, args.offline, args.refresh)
            report['program'] = bridge.data.get('program', {})
            auditor = Auditor(bridge, classes, args.max_slots)
            if args.table_map:
                exported = json.loads(args.table_map.read_text(encoding='utf-8'))
                identity = exported.get('program', {})
                if any(identity.get(k) != report['program'].get(k) for k in ('program', 'image_base', 'language')):
                    raise BridgeError('Table map belongs to another Ghidra program')
                for owner, addresses in exported['tables'].items():
                    name = owner.split('::')[-1]
                    if name in classes:
                        auditor.discovered.setdefault(name, set()).update(a.lower() for a in addresses)
            # Typed names are hints only, and only used if the source parent declares them.
            try:
                members = bridge.call('get_data_type_members', data_type='CActorFighterVTable')['members']
                auditor.slot_hints = {m['offset']: m['field_name'] for m in members if m.get('field_name')}
            except BridgeError:
                pass
        for index, name in enumerate(sorted(selected), 1):
            cls = classes[name]
            row = asdict(cls)
            if bridge:
                try:
                    auditor.discover(name)
                except BridgeError as exc:
                    row['error'] = str(exc)
                if index % 20 == 0 or index == len(selected):
                    print(f'Discovered {index}/{len(selected)} classes ({len(auditor.discovered)} table owners)', flush=True)
                    bridge.save()
            report['classes'].append(row)
        if bridge:
            # Inline base constructors have no standalone Ghidra function. Derived
            # constructors still reference their base tables, collected above.
            for index, row in enumerate(report['classes'], 1):
                addresses = auditor.discovered.get(row['name'], set())
                if len(addresses) == 1:
                    try:
                        row['table'] = auditor.capture(row['name'], next(iter(addresses)))
                        auditor.tables[row['name']] = row['table']
                        row.pop('error', None)
                    except BridgeError as exc:
                        row['error'] = str(exc)
                if index % 20 == 0 or index == len(selected):
                    print(f'Captured {index}/{len(selected)} classes ({len(auditor.tables)} tables)', flush=True)
                    bridge.save()
            for name in sorted(selected):
                report['findings'].extend(auditor.compare(name))
        args.output.mkdir(parents=True, exist_ok=True)
        (args.output / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
        (args.output / 'report.md').write_text(markdown(report), encoding='utf-8')
        print(f"{len(selected)} classes; tables captured: {sum(bool(c.get('table')) for c in report['classes'])}; findings: {dict(Counter(f['kind'] for f in report['findings']))}; "
              f"unavailable: {sum('error' in c for c in report['classes'])}. Report: {args.output / 'report.md'}")
        incomplete = any('error' in c or (c.get('table') and
                         (not c['table']['bounded'] or c['table']['boundary'].startswith('unresolved'))) for c in report['classes'])
        return 1 if args.fail_on_findings and (report['findings'] or incomplete) else 0
    except (BridgeError, OSError, ValueError) as exc:
        print(f'error: {exc}', file=sys.stderr)
        return 2
    finally:
        if bridge:
            bridge.save()


if __name__ == '__main__':
    sys.exit(main())
