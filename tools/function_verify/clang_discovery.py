"""Investigate Tree-sitter failures with production-configured libclang.

Read-only source discovery, not an equivalence check. Translation units with
compiler errors supply no accepted definitions. No source rewriting or grammar
error suppression is used.
"""
import argparse
from collections import Counter
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

from build_config import ROOT, identity


class Cursor(C.Structure):
    _fields_ = [('kind', C.c_uint), ('xdata', C.c_int), ('data', C.c_void_p * 3)]


class String(C.Structure):
    _fields_ = [('data', C.c_void_p), ('flags', C.c_uint)]


class Location(C.Structure):
    _fields_ = [('data', C.c_void_p * 2), ('value', C.c_uint)]


class Range(C.Structure):
    _fields_ = [('data', C.c_void_p * 2), ('begin', C.c_uint), ('end', C.c_uint)]


class Type(C.Structure):
    _fields_ = [('kind', C.c_int), ('data', C.c_void_p * 2)]


Visitor = C.CFUNCTYPE(C.c_uint, Cursor, Cursor, C.c_void_p)


def windows_arguments(command):
    """Decode a Ninja Windows command without shell interpolation."""
    shell = C.WinDLL('shell32')
    shell.CommandLineToArgvW.argtypes = [C.c_wchar_p, C.POINTER(C.c_int)]
    shell.CommandLineToArgvW.restype = C.POINTER(C.c_wchar_p)
    count = C.c_int()
    result = shell.CommandLineToArgvW(command, C.byref(count))
    if not result:
        raise OSError('CommandLineToArgvW failed')
    kernel = C.WinDLL('kernel32')
    kernel.LocalFree.argtypes = [C.c_void_p]
    kernel.LocalFree.restype = C.c_void_p
    try:
        return [result[i] for i in range(count.value)]
    finally:
        kernel.LocalFree(C.cast(result, C.c_void_p))


def parse_arguments(command, source):
    args, selected = windows_arguments(command), []
    index = 1
    while index < len(args):
        value = args[index]
        if value in ('-o', '-MF', '-MT', '-MQ'):
            index += 2
            continue
        is_source = not value.startswith('-') and Path(value).resolve() == source
        if value not in ('-c', '-MD', '-MMD') and not is_source:
            selected.append(value)
        index += 1
    return args[0], selected


class Clang:
    def __init__(self, path):
        self.dll = C.CDLL(str(path))
        def bind(name, result, *arguments):
            fn = getattr(self.dll, 'clang_' + name)
            fn.restype, fn.argtypes = result, list(arguments)
            setattr(self, name, fn)
        bind('createIndex', C.c_void_p, C.c_int, C.c_int)
        bind('disposeIndex', None, C.c_void_p)
        bind('parseTranslationUnit2', C.c_int, C.c_void_p, C.c_char_p,
             C.POINTER(C.c_char_p), C.c_int, C.c_void_p, C.c_uint, C.c_uint, C.POINTER(C.c_void_p))
        bind('disposeTranslationUnit', None, C.c_void_p)
        bind('getTranslationUnitCursor', Cursor, C.c_void_p)
        bind('visitChildren', C.c_uint, Cursor, Visitor, C.c_void_p)
        bind('isCursorDefinition', C.c_uint, Cursor)
        bind('getCursorSpelling', String, Cursor)
        bind('getCursorDisplayName', String, Cursor)
        bind('Cursor_getMangling', String, Cursor)
        bind('getCursorSemanticParent', Cursor, Cursor)
        bind('getCursorLocation', Location, Cursor)
        bind('getCursorExtent', Range, Cursor)
        bind('getRangeStart', Location, Range)
        bind('getRangeEnd', Location, Range)
        bind('getFileLocation', None, Location, C.POINTER(C.c_void_p), C.POINTER(C.c_uint),
             C.POINTER(C.c_uint), C.POINTER(C.c_uint))
        bind('getFileName', String, C.c_void_p)
        bind('getCString', C.c_char_p, String)
        bind('disposeString', None, String)
        bind('getClangVersion', String)
        bind('getNumDiagnostics', C.c_uint, C.c_void_p)
        bind('getDiagnostic', C.c_void_p, C.c_void_p, C.c_uint)
        bind('getDiagnosticSeverity', C.c_uint, C.c_void_p)
        bind('getDiagnosticSpelling', String, C.c_void_p)
        bind('disposeDiagnostic', None, C.c_void_p)
        bind('getCursorResultType', Type, Cursor)
        bind('getTypeSpelling', String, Type)
        bind('Cursor_getNumArguments', C.c_int, Cursor)
        bind('Cursor_getArgument', Cursor, Cursor, C.c_uint)
        bind('getCursorType', Type, Cursor)

    def string(self, value):
        try:
            return (self.getCString(value) or b'').decode('utf-8')
        finally:
            self.disposeString(value)

    def location(self, value):
        file = C.c_void_p()
        line, column, offset = C.c_uint(), C.c_uint(), C.c_uint()
        self.getFileLocation(value, C.byref(file), C.byref(line), C.byref(column), C.byref(offset))
        return (Path(self.string(self.getFileName(file))).resolve() if file else None,
                line.value, offset.value)

    def functions(self, source, arguments, wanted, declarations=None):
        tu, index = C.c_void_p(), self.createIndex(0, 0)
        encoded = [arg.encode() for arg in arguments]
        args = (C.c_char_p * len(encoded))(*encoded)
        errors, definitions, callback_errors = [], [], []
        files = {}
        examined = {source}
        try:
            status = self.parseTranslationUnit2(index, str(source).encode(), args, len(args), None, 0, 0, C.byref(tu))
            if status != 0 or not tu:
                raise ValueError('libclang translation-unit error ' + str(status))
            for i in range(self.getNumDiagnostics(tu)):
                diagnostic = self.getDiagnostic(tu, i)
                if self.getDiagnosticSeverity(diagnostic) >= 3:
                    errors.append(self.string(self.getDiagnosticSpelling(diagnostic)))
                self.disposeDiagnostic(diagnostic)
            if errors:
                return [], errors, []

            @Visitor
            def visit(cursor, parent, data):
                try:
                    file, line, _ = self.location(self.getCursorLocation(cursor))
                    if file in wanted:
                        examined.add(file)
                    # Namespace cursors may contain declarations from several
                    # included files. Other unrelated subtrees can be skipped.
                    if file not in wanted:
                        return 2 if cursor.kind in (22, 23) else 1
                    if declarations is not None and cursor.kind in (2, 3, 4) and self.isCursorDefinition(cursor):
                        extent = self.getCursorExtent(cursor)
                        sf, sl, start = self.location(self.getRangeStart(extent))
                        ef, el, end = self.location(self.getRangeEnd(extent))
                        if sf == file == ef:
                            declarations.append(dict(path=file.relative_to(ROOT).as_posix(),
                                line=sl, end_line=el, start_byte=start, end_byte=end,
                                kind='type', name=self.string(self.getCursorSpelling(cursor))))
                    if cursor.kind not in (8, 21, 24, 25, 26, 30):
                        return 2 if cursor.kind in (1, 2, 3, 4, 22, 23, 31, 32) else 1
                    if not self.isCursorDefinition(cursor):
                        return 1
                    extent = self.getCursorExtent(cursor)
                    start_file, start_line, start = self.location(self.getRangeStart(extent))
                    end_file, end_line, end = self.location(self.getRangeEnd(extent))
                    if start_file != file or end_file != file or end <= start:
                        return 1
                    parts = [self.string(self.getCursorSpelling(cursor))]
                    owner = self.getCursorSemanticParent(cursor)
                    while owner.kind not in (0, 70, 300, 350):
                        name = self.string(self.getCursorDisplayName(owner))
                        name = re.sub(r'\s+', '', name)
                        if name:
                            parts.insert(0, name)
                        owner = self.getCursorSemanticParent(owner)
                    if file not in files:
                        files[file] = file.read_bytes()
                    raw = files[file][start:end]
                    # A function produced by a declaration macro has an expansion
                    # range, not a recoverable literal source definition.
                    if not raw.rstrip().endswith(b'}'):
                        return 1
                    try:
                        text, utf8_valid = raw.decode('utf-8'), True
                    except UnicodeDecodeError:
                        # Match Node's replacement decoding for display only.
                        # Keep byte provenance; never rewrite the source file.
                        text, utf8_valid = raw.decode('utf-8', errors='replace'), False
                    count = self.Cursor_getNumArguments(cursor)
                    definitions.append(dict(name='::'.join(parts), path=file.relative_to(ROOT).as_posix(),
                        line=start_line, end_line=end_line, start_byte=start, end_byte=end,
                        lines=raw.count(b'\n') + 1, source=text, source_utf8_valid=utf8_valid,
                        **({} if utf8_valid else dict(source_bytes_hex=raw.hex())),
                        source_sha256=hashlib.sha256(raw).hexdigest(),
                        mangled=self.string(self.Cursor_getMangling(cursor)),
                        return_type=self.string(self.getTypeSpelling(self.getCursorResultType(cursor))),
                        argument_types=[self.string(self.getTypeSpelling(self.getCursorType(
                            self.Cursor_getArgument(cursor, i)))) for i in range(max(0, count))]))
                    return 1
                except Exception as error:
                    callback_errors.append(str(error))
                    return 0
            self.visitChildren(self.getTranslationUnitCursor(tu), visit, None)
            if callback_errors:
                raise ValueError('; '.join(callback_errors))
            return definitions, [], sorted(str(file) for file in examined)
        finally:
            if tu:
                self.disposeTranslationUnit(tu)
            self.disposeIndex(index)


def reconcile(fn, definitions, declarations, units):
    """Reconcile by literal source and range, never by an unqualified name."""
    def same_source(row):
        source, original = row['source'].replace('\r\n', '\n'), fn['source'].replace('\r\n', '\n')
        if row['line'] == fn['line'] and row['lines'] == fn['lines'] and source == original:
            return True
        # A FunctionTemplate extent includes its template declaration, whereas
        # Tree-sitter's nested function_definition starts after that declaration.
        if not source.endswith(original):
            return False
        prefix = source[:-len(original)]
        return (re.fullmatch(r'\s*template\s*<[^{};]+>\s*', prefix) is not None
                and row['line'] + prefix.count('\n') == fn['line']
                and row['line'] + row['lines'] == fn['line'] + fn['lines'])
    candidates = [row for row in definitions if Path(row['path']) == Path(fn['path'])
                  and same_source(row)]
    matches = [row for row in candidates if row['name'] == fn['name']]
    if not matches:
        # Tree-sitter can omit a generic owner's template argument list. Keep
        # Clang's identity; the runner still rejects uninstantiated templates.
        matches = [row for row in candidates if re.sub(r'<[^<>]*>', '', row['name']) == fn['name']]
    examined = {Path(file).resolve() for unit in units if not unit['errors'] for file in unit['examined_files']}
    packed_type = re.match(r'PACK\s*\(\s*struct\s+(\w+)\b', fn['source']) if fn['name'] == 'PACK' else None
    types = [row for row in declarations if packed_type and row['name'] == packed_type[1]
             and Path(row['path']) == Path(fn['path'])
             and fn['line'] <= row['line'] <= row['end_line'] < fn['line'] + fn['lines']]
    result = ('recovered' if len(matches) == 1 else 'ambiguous' if matches else
              'not-function' if types else
              'identity-review' if candidates else
              'no-active-definition' if (ROOT / fn['path']).resolve() in examined else 'not-examined')
    return dict(id=fn['id'], name=fn['name'], path=fn['path'], line=fn['line'], original_lines=fn['lines'],
                result=result, matches=matches, range_matches=candidates if not matches else [])


def validate_definition(row):
    """Require the exact compiler source slice to remain present at its offsets."""
    raw = (ROOT / row['path']).read_bytes()[row['start_byte']:row['end_byte']]
    if hashlib.sha256(raw).hexdigest() != row['source_sha256']:
        raise ValueError('Compiler source range changed; regenerate discovery: ' + row['path'])
    return raw


def recover_inventory(inventory, report):
    """Attach compiler evidence while preserving the original parser verdict."""
    recovered = dict(inventory, functions=[])
    reviews = {row['id']: row for row in report['functions']}
    excluded = []
    for fn in inventory['functions']:
        fn = dict(fn)
        review = reviews.get(fn['id'])
        if review:
            fn['discovery_result'] = review['result']
            if review['result'] == 'not-function':
                excluded.append(review)
                continue
            if review['result'] == 'recovered':
                row, = review['matches']
                unit = report['translation_units'][row['translation_unit']]
                if unit['errors']:
                    raise ValueError('Compiler errors in recovered translation unit')
                validate_definition(row)
                fn.update(name=row['name'], source=row['source'], line=row['line'], lines=row['lines'],
                          compiler_definition=row, compiler_translation_unit=unit,
                          compiler_version=report['clang_version'], compiler_build=report['build'])
        recovered['functions'].append(fn)
    recovered['excluded_declarations'] = excluded
    return recovered


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/verification/parser-investigation/clang.json')
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'out/build/x64-verify')
    parser.add_argument('--line-limit', type=int, default=20)
    parser.add_argument('--all-failures', action='store_true')
    parser.add_argument('--identities', action='store_true',
                        help='Also recover exact emitted symbols for short overloaded definitions')
    parser.add_argument('--max-tus', type=int, default=0)
    parser.add_argument('--source', action='append', type=Path, help='Restrict investigation to these production translation units')
    parser.add_argument('--ninja', default='C:/Program Files/Microsoft Visual Studio/18/Insiders/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe')
    args = parser.parse_args()
    if os.name != 'nt':
        parser.error('This discovery tool requires the Windows production compiler environment')
    if args.max_tus < 0 or args.line_limit < 1:
        parser.error('Require a positive line limit and a nonnegative translation-unit limit')
    inventory = json.loads(args.inventory.read_text())
    name_counts = Counter(fn['name'] for fn in inventory['functions'])
    failed = [fn for fn in inventory['functions'] if
              (fn['parseError'] and (args.all_failures or fn['lines'] < args.line_limit)) or
              (args.identities and fn['lines'] < args.line_limit and name_counts[fn['name']] > 1)]
    wanted = {(ROOT / fn['path']).resolve() for fn in failed}
    build = args.build_dir.resolve()
    targets = re.findall(r'^build (CMakeFiles/Kya\.dir/src/[^:]+\.obj):', (build / 'build.ninja').read_text(), re.M)
    sources = {target: (ROOT / target.removeprefix('CMakeFiles/Kya.dir/').removesuffix('.obj')).resolve() for target in targets}
    selected = {target for target, source in sources.items() if source in wanted}
    # Header failures need a consuming translation unit. Ninja records actual
    # include dependencies; choose one available consumer per requested header.
    headers = {file for file in wanted if file.suffix in ('.h', '.hpp', '.inl')}
    if headers:
        deps = subprocess.check_output([args.ninja, '-C', str(build), '-t', 'deps'], text=True)
        target = None
        for line in deps.splitlines():
            if line and not line.startswith(' '):
                target = line.split(': #deps', 1)[0]
            elif line.startswith('    ') and target in sources:
                file = Path(line.strip()).resolve()
                if file in headers:
                    selected.add(target)
                    headers.remove(file)
    selected = sorted(selected)
    if args.source:
        requested = {file.resolve() for file in args.source}
        selected = [target for target in sources if sources[target] in requested]
        if {sources[target] for target in selected} != requested:
            parser.error('--source must name production Kya translation units')
    if args.max_tus:
        selected = selected[:args.max_tus]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    units, definitions, declarations, clang = [], {}, [], None
    original_cwd = Path.cwd()
    try:
        os.chdir(build)
        for number, target in enumerate(selected):
            source = sources[target]
            command = subprocess.check_output([args.ninja, '-C', str(build), '-t', 'commands', target], text=True).splitlines()[-1]
            executable, arguments = parse_arguments(command, source)
            if clang is None:
                clang = Clang(Path(executable).with_name('libclang.dll'))
            try:
                types = []
                rows, errors, examined = clang.functions(source, arguments, wanted | {source}, types)
            except ValueError as error:
                rows, errors, examined = [], [str(error)], []
            units.append(dict(source=str(source), command=command, compiler=executable,
                              arguments=arguments, errors=errors, definitions=len(rows), examined_files=examined))
            if not errors:
                declarations.extend(types)
            for row in rows:
                row['translation_unit'] = len(units) - 1
                key = (row['path'], row['start_byte'], row['end_byte'], row['mangled'])
                definitions[key] = row
            print(f'{number+1}/{len(selected)} {source.name}: {len(rows)} definitions, {len(errors)} errors', flush=True)
    finally:
        os.chdir(original_cwd)
    reviewed = [reconcile(fn, list(definitions.values()), declarations, units) for fn in failed]
    document = dict(scope='Compiler AST discovery only; no equivalence claimed', build=identity(build),
                    clang_version=clang.string(clang.getClangVersion()) if clang else None,
                    counts=dict(Counter(row['result'] for row in reviewed)),
                    translation_units=units, definitions=list(definitions.values()), declarations=declarations, functions=reviewed)
    args.output.write_text(json.dumps(document, indent=2) + '\n')
    print(document['counts'], flush=True)
    return 1 if any(unit['errors'] for unit in units) else 0


if __name__ == '__main__':
    raise SystemExit(main())
