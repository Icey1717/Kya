"""Run all compiled comparisons once, or rerun when their dependencies change."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import time
from contextlib import contextmanager

from compiled import SYMBOLS
from compiled_extended import CASES as FLOW_CASES
from subsystems import CASES as SUBSYSTEM_CASES
from combat_cases import CASES as COMBAT_CASES

ROOT = Path(__file__).resolve().parents[2]
NINJA = 'C:/Program Files/Microsoft Visual Studio/18/Insiders/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'
FILES = sorted({'Actor'} | {case[0] for case in FLOW_CASES + SUBSYSTEM_CASES + COMBAT_CASES})
SUITES = [
    ('actor', 'compiled.py', [], 'compiled', [item[0] for item in SYMBOLS]),
    ('control-flow', 'compiled_extended.py', [], 'compiled-extended', [item[1] for item in FLOW_CASES]),
    ('subsystems', 'compiled_extended.py', ['--suite', 'subsystems'], 'compiled-subsystems', [item[1] for item in SUBSYSTEM_CASES]),
    ('combat-collision', 'compiled_extended.py', ['--suite', 'combat'], 'compiled-combat', [item[1] for item in COMBAT_CASES]),
]


class RunBusy(Exception):
    pass


@contextmanager
def run_lock(output, filename='check.lock'):
    """Serialize runs so a watcher and a manual check cannot mix artifacts."""
    with (output / filename).open('a+b') as stream:
        stream.seek(0)
        try:
            if sys.platform == 'win32':
                import msvcrt
                msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)
            else:
                import fcntl
                fcntl.flock(stream.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError as error:
            if getattr(error, 'winerror', None) == 33 or error.errno in (11, 13):
                raise RunBusy from error
            raise
        try:
            yield
        finally:
            stream.seek(0)
            if sys.platform == 'win32':
                msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
            else:
                fcntl.flock(stream.fileno(), fcntl.LOCK_UN)


def snapshot(ninja):
    """Content hashes of Ninja's known local headers, sources, and tool inputs."""
    build = ROOT / 'out/build/x64-debug'
    paths = set()
    targets = ['CMakeFiles/Kya.dir/src/b-witch/' + name + '.cpp.obj' for name in FILES]
    deps = subprocess.check_output([ninja, '-C', str(build), '-t', 'deps', *targets], text=True)
    for line in deps.splitlines():
        if line.startswith('    '):
            path = Path(line.strip())
            if not path.is_absolute():
                path = build / path
            path = path.resolve()
            if path.is_relative_to(ROOT):
                paths.add(path)
    paths.update(ROOT / ('src/b-witch/' + name + '.cpp') for name in FILES)
    for folder, patterns in [('tools/function_verify', ['*.py', 'requirements.txt']),
                             ('tools/function_verify/fixtures', ['*.json']),
                             ('tools/ghidra-module/totala/ghidra_scripts', ['*.py', '*.java'])]:
        for pattern in patterns:
            paths.update((ROOT / folder).glob(pattern))
    paths.update([ROOT / 'CMakeLists.txt', ROOT / 'CMakePresets.json', build / 'build.ninja',
                  build / 'CMakeCache.txt', build / 'CMakeFiles/rules.ninja',
                  ROOT / 'tools/ghidra-module/totala/build/ghidra10/totala.jar'])
    # CMake changes can alter include paths, definitions, or compiler settings.
    paths.add(ROOT / 'CMakeUserPresets.json')
    for folder in ['cmake', 'port', 'src']:
        paths.update((ROOT / folder).rglob('CMakeLists.txt'))
        paths.update((ROOT / folder).rglob('*.cmake'))
    return {str(p): hashlib.sha256(p.read_bytes()).hexdigest() if p.is_file() else None for p in sorted(paths)}


def run(args):
    output = ROOT / 'out/verification'
    output.mkdir(parents=True, exist_ok=True)
    try:
        with run_lock(output, 'offline.lock' if args.offline else 'check.lock'):
            return run_checks(args, output)
    except RunBusy:
        print('Another verification run owns the artifacts; retry after it finishes.', flush=True)
        return 2


def validate_report(document, expected):
    functions = document.get('functions', [])
    addresses = [row.get('ps2_entry') for row in functions]
    if len(addresses) != len(expected) or set(addresses) != set(expected):
        raise ValueError('Report does not cover the exact expected function set')
    if any(row.get('result') != 'equivalent' for row in functions):
        raise ValueError('Report contains a mismatch or incomplete proof')
    return functions


def run_checks(args, output):
    started = datetime.now(timezone.utc).isoformat()
    status_path = output / ('offline-status.json' if args.offline else 'check-status.json')
    status_path.write_text(json.dumps(dict(started_utc=started, result='running')) + '\n')
    commands = [[sys.executable, '-m', 'unittest', 'discover', '-s', str(ROOT / 'tools/function_verify'), '-p', 'test_*.py']]
    for _, script, options, _, _ in ([] if args.offline else SUITES):
        command = [sys.executable, str(ROOT / 'tools/function_verify' / script), *options, '--ninja', args.ninja, '--port', str(args.port)]
        for key in ['ghidra', 'java']:
            if getattr(args, key):
                command += ['--' + key, getattr(args, key)]
        commands.append(command)
    results, coverage = [], []
    for i, command in enumerate(commands):
        logfile = output / ('offline.log' if args.offline else 'check-' + str(i) + '.log')
        if i:
            (output / SUITES[i - 1][3] / 'report.json').unlink(missing_ok=True)
        print('Running ' + ' '.join(command) + ' (log: ' + str(logfile) + ')', flush=True)
        with logfile.open('w') as stream:
            try:
                code = subprocess.run(command, cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT).returncode
            except OSError as error:
                stream.write(str(error) + '\n')
                code = 127
        record = dict(command=command, exit_code=code, log=str(logfile))
        if i:
            name, _, _, folder, expected = SUITES[i - 1]
            suite = dict(suite=name, report=str(output / folder / 'report.json'), functions=[])
            if code == 0:
                try:
                    suite['functions'] = validate_report(json.loads(Path(suite['report']).read_text()), expected)
                except (OSError, ValueError, KeyError) as error:
                    code = record['exit_code'] = 1
                    suite['reason'] = record['reason'] = str(error)
            suite['result'] = 'passed' if code == 0 else 'failed'
            coverage.append(suite)
        results.append(record)
        if code:
            print(logfile.read_text(errors='replace')[-6000:], flush=True)
    status = dict(started_utc=started, finished_utc=datetime.now(timezone.utc).isoformat(),
                  result='passed' if all(r['exit_code'] == 0 for r in results) else 'failed', checks=results)
    status['scope'] = 'offline fixture regressions only' if args.offline else 'fresh compiled comparisons within per-function domains'
    if not args.offline:
        status['equivalent_functions'] = sum(len(s['functions']) for s in coverage if s['result'] == 'passed')
        document = dict(status, suites=coverage)
        (output / 'coverage.json').write_text(json.dumps(document, indent=2) + '\n')
        lines = ['# Compiled function coverage', '', 'Result: **' + status['result'] + '**', '',
                 'Generated: ' + status['finished_utc'], '',
                 '| Suite | Function | PS2 entry | Result | Domain |', '| --- | --- | --- | --- | --- |']
        for suite in coverage:
            if suite['result'] != 'passed':
                lines.append('| ' + suite['suite'] + ' | Check failed; see log | | failed | |')
            else:
                for row in suite['functions']:
                    lines.append('| ' + ' | '.join([suite['suite'], row['function'], row['ps2_entry'], row['result'], row.get('domain', 'See suite assumptions')]).replace('\n', ' ') + ' |')
        lines += ['', 'These proofs cover pre-link production objects under each suite\'s documented ABI, layout, memory, and input assumptions.',
                  '', 'Known blockers: SetStandAnim virtual dispatch and the full SetState behavior/animation/audio/logging graph remain unsupported. CollisionReset requires contact flags <= 255.',
                  'See tools/function_verify/COMBAT.md and out/verification/combat-blockers/report.json for evidence.', '']
        (output / 'coverage.md').write_text('\n'.join(lines))
    status_path.write_text(json.dumps(status, indent=2) + '\n')
    print(('Offline fixture regressions: ' if args.offline else 'Compiled verification: ') + status['result'], flush=True)
    return 0 if status['result'] == 'passed' else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--watch', action='store_true', help='Run now and again after local dependency changes; Ctrl+C stops')
    parser.add_argument('--stop', action='store_true', help='Ask a running watcher to stop after its current check')
    parser.add_argument('--offline', action='store_true', help='Run retained compiler-output regressions without building or Ghidra')
    parser.add_argument('--interval', type=float, default=5)
    parser.add_argument('--ninja', default=NINJA)
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--ghidra')
    parser.add_argument('--java')
    args = parser.parse_args()
    if args.offline and args.watch:
        parser.error('--offline and --watch cannot be combined')
    stop = ROOT / 'out/verification/watch.stop'
    if args.stop:
        stop.parent.mkdir(parents=True, exist_ok=True)
        stop.touch()
        print('Watcher stop requested; any current check will finish first.')
        return 0
    if args.interval < 1:
        parser.error('--interval must be at least one second')
    try:
        if args.watch:
            stop.unlink(missing_ok=True)
        before = snapshot(args.ninja) if args.watch else None
        while True:
            if args.watch and stop.exists():
                print('Verification watcher stopped.', flush=True)
                return 0
            result = run(args)
            if not args.watch:
                return result
            if result == 2:
                time.sleep(args.interval)
                continue
            after = snapshot(args.ninja)
            if after != before:
                # Re-run to account for edits made while a check was in flight.
                before = after
                continue
            print('Watching local source/header/build/tool dependencies for changes...', flush=True)
            while True:
                time.sleep(args.interval)
                if stop.exists():
                    print('Verification watcher stopped.', flush=True)
                    return 0
                current = snapshot(args.ninja)
                if current != before:
                    # Wait one polling interval to let an editor finish a save.
                    time.sleep(args.interval)
                    before = snapshot(args.ninja)
                    break
    except KeyboardInterrupt:
        print('Verification watcher stopped.', flush=True)
        return 0


if __name__ == '__main__':
    raise SystemExit(main())
