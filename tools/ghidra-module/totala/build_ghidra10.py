"""Build against Ghidra 10.0 without its obsolete Gradle API. Requires JDK 21.

python build_ghidra10.py --ghidra <ghidra_10.0_PUBLIC> --jdk <jdk-21>
The host Ghidra must also run on Java 21 (the existing bridge uses modern Java).
"""
import argparse
import datetime
from pathlib import Path
import subprocess
import zipfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ghidra', type=Path, required=True)
    parser.add_argument('--jdk', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parent
    properties = dict(line.split('=', 1) for line in
                      (args.ghidra / 'Ghidra/application.properties').read_text().splitlines()
                      if '=' in line and not line.startswith('#'))
    version = properties['application.version']
    if version != '10.0':
        parser.error('This builder targets Ghidra 10.0 only')
    build = root / 'build/ghidra10'
    classes = build / 'classes'
    classes.mkdir(parents=True, exist_ok=True)
    jars = sorted((args.ghidra / 'Ghidra').rglob('*.jar'))
    # javac argument files avoid Windows command-line length limits.
    options = ['--release', '21', '-classpath', ';'.join(str(p) for p in jars),
               '-d', str(classes)] + [str(p) for p in (root / 'src/main/java').rglob('*.java')]
    argfile = build / 'javac.args'
    argfile.write_text('\n'.join('"' + value.replace('\\', '/') + '"' for value in options))
    subprocess.run([str(args.jdk / 'bin/javac.exe'), '@' + str(argfile)], check=True)
    jar = build / 'totala.jar'
    subprocess.run([str(args.jdk / 'bin/jar.exe'), 'cf', str(jar), '-C', str(classes), '.'], check=True)
    archive = root / 'dist' / f'ghidra_{version}_PUBLIC_{datetime.date.today():%Y%m%d}_totala.zip'
    archive.parent.mkdir(exist_ok=True)
    with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as out:
        # Ghidra 10's installer creates directories only for explicit ZIP
        # directory entries; it does not create parents when extracting files.
        for directory in ('totala/', 'totala/lib/', 'totala/ghidra_scripts/'):
            out.writestr(directory, '')
        out.write(jar, 'totala/lib/totala.jar')
        out.writestr('totala/extension.properties', (root / 'extension.properties').read_text()
                     .replace('@extname@', 'totala').replace('@extversion@', version))
        out.write(root / 'Module.manifest', 'totala/Module.manifest')
        for path in (root / 'ghidra_scripts').glob('*'):
            if path.is_file():
                out.write(path, 'totala/ghidra_scripts/' + path.name)
    # Validate extraction order against the legacy installer's requirements.
    with zipfile.ZipFile(archive) as packaged:
        directories = {''}
        for entry in packaged.infolist():
            parent = entry.filename.rstrip('/').rpartition('/')[0]
            if parent not in directories:
                raise RuntimeError(f'Missing preceding directory entry: {parent}')
            if entry.is_dir():
                directories.add(entry.filename.rstrip('/'))
        if packaged.testzip() is not None:
            raise RuntimeError('Archive CRC validation failed')
    print(archive)


if __name__ == '__main__':
    main()
