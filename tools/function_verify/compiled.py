"""Build, extract real Windows COFF functions, and lift them with headless Ghidra."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import shutil

ROOT = Path(__file__).resolve().parents[2]
SYMBOLS = [('00100460', 'IsKindOfObject', '?IsKindOfObject@CActor@@UEAA_N_K@Z'),
           ('001003f0', 'IsLookingAt', '?IsLookingAt@CActor@@UEAAIXZ'),
           ('00100400', 'SetLookingAtOn', '?SetLookingAtOn@CActor@@UEAAXXZ'),
           ('00100410', 'SetLookingAtOff', '?SetLookingAtOff@CActor@@UEAAXXZ')]


def coff_symbols(data):
    machine, sections, _, symoff, count, optional, _ = struct.unpack_from('<HHIIIHH', data)
    if machine != 0x8664 or optional != 0:
        raise ValueError('Expected ordinary x64 COFF object')
    string_base = symoff + count * 18
    symbols = []
    index = 0
    while index < count:
        pos = symoff + index * 18
        raw, value, section, kind, storage, aux = struct.unpack_from('<8sIhHBB', data, pos)
        if raw[:4] == b'\0' * 4:
            start = string_base + struct.unpack_from('<I', raw, 4)[0]
            name = data[start:data.index(b'\0', start)].decode()
        else:
            name = raw.rstrip(b'\0').decode(errors='replace')
        symbols.append((name, value, section, kind))
        index += aux + 1
    return symbols, sections


def constant(data, name):
    symbols, sections = coff_symbols(data)
    matches = [s for s in symbols if s[0] == name]
    if len(matches) != 1:
        raise ValueError('Missing layout constant: ' + name)
    _, offset, section, _ = matches[0]
    if not 1 <= section <= sections:
        raise ValueError('Layout constant is not defined')
    rawoff = struct.unpack_from('<I', data, 20 + (section - 1) * 40 + 20)[0]
    return struct.unpack_from('<I', data, rawoff + offset)[0]


def extract(data):
    symbols, sections = coff_symbols(data)
    blob = bytearray()
    manifest = []
    for ps2, name, decorated in SYMBOLS:
        found = [s for s in symbols if s[0] == decorated]
        if len(found) != 1:
            raise ValueError('Missing or ambiguous symbol: ' + decorated)
        _, value, section, kind = found[0]
        if section < 1 or section > sections or kind != 0x20:
            raise ValueError('Symbol is not a defined function')
        header = 20 + (section - 1) * 40
        size, rawoff, relocoff = struct.unpack_from('<III', data, header + 16)
        reloc_count = struct.unpack_from('<H', data, header + 32)[0]
        if reloc_count == 0xffff:
            raise ValueError('COFF relocation overflow unsupported')
        end = min([s[1] for s in symbols if s[2] == section and s[3] == 0x20 and s[1] > value] + [size])
        if not 0 <= value < end <= size:
            raise ValueError('Invalid COFF function bounds')
        for i in range(reloc_count):
            offset = struct.unpack_from('<I', data, relocoff + i * 10)[0]
            if value <= offset < end:
                raise ValueError('Relocation in selected function; linked image required: ' + name)
        while len(blob) % 16:
            blob.append(0xcc)
        body = data[rawoff + value:rawoff + end]
        if len(body) != end - value:
            raise ValueError('Truncated function')
        manifest.append(dict(name=name, symbol=decorated, ps2_entry=ps2, blob_offset=len(blob),
                             coff_section=section, coff_offset=value, size=len(body),
                             bytes_sha256=hashlib.sha256(body).hexdigest()))
        blob.extend(body)
    return bytes(blob), manifest


def main():
    from build_config import arguments, identity
    parser = argparse.ArgumentParser(description=__doc__)
    arguments(parser)
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    parser.add_argument('--ninja', default='C:/Program Files/Microsoft Visual Studio/18/Insiders/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe')
    parser.add_argument('--mutate-mask', action='store_true', help='Compile a disposable source copy with a wrong IsKindOfObject mask')
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--export-only', action='store_true', help='Build/lift without querying the live PS2 bridge')
    args = parser.parse_args()
    build = args.build_dir.resolve()
    configuration = identity(build)
    target = 'CMakeFiles/Kya.dir/src/b-witch/Actor.cpp.obj'
    subprocess.run([args.ninja, '-C', str(build), target], check=True)
    command = subprocess.check_output([args.ninja, '-C', str(build), '-t', 'commands', target], text=True).splitlines()[-1]
    output = args.output_root.resolve() / ('compiled-mutant' if args.mutate_mask else 'compiled')
    output.mkdir(parents=True, exist_ok=True)
    object_path = build / target
    compiled_source = ROOT / 'src/b-witch/Actor.cpp'
    if args.mutate_mask:
        source = compiled_source.read_text()
        before = 'bool CActor::IsKindOfObject(ulong kind)\n{\n\treturn (kind & 1) != 0;\n}'
        if source.count(before) != 1:
            raise ValueError('Mutation target changed')
        mutated_source = output / 'Actor-mutated.cpp'
        mutated_source.write_text(source.replace(before, before.replace('kind & 1', 'kind & 2')))
        object_path = output / 'Actor-mutated.obj'
        mutant_command = command.replace(compiled_source.as_posix(), mutated_source.as_posix())
        mutant_command = mutant_command.replace(target.replace('/', '\\') + '.d', (output / 'mutant.d').as_posix())
        mutant_command = mutant_command.replace(target, object_path.as_posix())
        subprocess.run(mutant_command, cwd=build, check=True)
        compiled_source = mutated_source
        compiled_command = mutant_command
    else:
        compiled_command = command
    data = object_path.read_bytes()
    blob, functions = extract(data)
    (output / 'functions.bin').write_bytes(blob)
    manifest = dict(object=str(object_path), object_sha256=hashlib.sha256(data).hexdigest(),
                    source_sha256=hashlib.sha256((ROOT / 'src/b-witch/Actor.cpp').read_bytes()).hexdigest(),
                    compiled_source=str(compiled_source), compiled_source_sha256=hashlib.sha256(compiled_source.read_bytes()).hexdigest(),
                    compile_command=compiled_command, mutation=args.mutate_mask,
                    build=configuration, functions=functions)
    manifest['compiler_version'] = subprocess.check_output([command.split()[0], '--version'], text=True)
    layout_source = output / 'layout.cpp'
    layout_object = output / 'layout.obj'
    layout_source.write_text('#include "Actor.h"\nextern "C" {\n'
                             'extern const unsigned int pilot_flags_offset = __builtin_offsetof(CActor, flags);\n'
                             'extern const unsigned int pilot_ulong_size = sizeof(ulong);\n}\n')
    layout_command = command.replace('G:/repos/Kya/src/b-witch/Actor.cpp', layout_source.as_posix())
    layout_command = layout_command.replace(target.replace('/', '\\') + '.d', (output / 'layout.d').as_posix())
    layout_command = layout_command.replace(target, layout_object.as_posix())
    subprocess.run(layout_command, cwd=build, check=True)
    layout_data = layout_object.read_bytes()
    manifest['compiled_layout'] = dict(flags_offset=constant(layout_data, 'pilot_flags_offset'),
                                       ulong_size=constant(layout_data, 'pilot_ulong_size'),
                                       object_sha256=hashlib.sha256(layout_data).hexdigest(),
                                       compile_command=layout_command)
    (output / 'manifest.json').write_text(json.dumps(manifest, indent=2))
    lift(output, args.ghidra, args.java)
    print(output / 'exports.json')
    if not args.export_only:
        from compare_binaries import run
        return run(output / 'exports.json', args.port)
    return 0


def lift(output, ghidra, java):
    """Lift a prepared byte bundle in an isolated Ghidra 10 project."""
    user_home = output / 'home'
    extension = user_home / '.ghidra/.ghidra_10.0_PUBLIC/Extensions/totala'
    (extension / 'lib').mkdir(parents=True, exist_ok=True)
    plugin = ROOT / 'tools/ghidra-module/totala'
    shutil.copyfile(plugin / 'build/ghidra10/totala.jar', extension / 'lib/totala.jar')
    shutil.copyfile(plugin / 'Module.manifest', extension / 'Module.manifest')
    (extension / 'extension.properties').write_text((plugin / 'extension.properties').read_text()
                        .replace('@extname@', 'totala').replace('@extversion@', '10.0'))
    (output / 'exports.json').unlink(missing_ok=True)
    # Direct launcher avoids Ghidra 10's interactive JDK selection. Only this
    # disposable project is written; the user's live PS2 project is untouched.
    subprocess.run([str(java), '--add-opens=java.base/java.lang=ALL-UNNAMED',
                    '--add-opens=java.base/java.net=ALL-UNNAMED',
                    '-Djava.system.class.loader=ghidra.GhidraClassLoader',
                    '-Duser.home=' + str(user_home), '-cp',
                    str(ghidra / 'Ghidra/Framework/Utility/lib/Utility.jar'),
                    'ghidra.GhidraLauncher', 'ghidra.app.util.headless.AnalyzeHeadless',
                    str(output), 'compiled-pilot', '-import', str(output / 'functions.bin'),
                    '-log', str(output / 'headless.log'), '-scriptlog', str(output / 'script.log'),
                    '-overwrite', '-loader', 'BinaryLoader', '-processor', 'x86:LE:64:default',
                    '-cspec', 'windows', '-noanalysis', '-scriptPath',
                    str(ROOT / 'tools/ghidra-module/totala/ghidra_scripts'),
                    '-postScript', 'ExportCompiledPilot.py', str(output / 'manifest.json'),
                    str(output / 'exports.json'), '-deleteProject'], check=True)
    if not (output / 'exports.json').exists():
        raise RuntimeError('Headless Ghidra did not produce exports')


if __name__ == '__main__':
    sys.exit(main())
