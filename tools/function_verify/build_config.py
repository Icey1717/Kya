"""Build identity shared by compiled verification entry points."""
import hashlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def arguments(parser):
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'out/build/x64-verify')
    parser.add_argument('--output-root', type=Path, default=ROOT / 'out/verification/x64-verify')


def identity(build):
    cache = (build / 'CMakeCache.txt').read_bytes()
    values = {}
    for line in cache.decode().splitlines():
        if line and not line.startswith(('#', '//')) and ':' in line and '=' in line:
            key, value = line.split('=', 1)
            values[key.split(':', 1)[0]] = value
    logging = values.get('KYA_ENABLE_GAME_LOGGING')
    if logging not in ('ON', 'OFF'):
        raise ValueError('Build must explicitly configure KYA_ENABLE_GAME_LOGGING ON or OFF')
    return dict(build_directory=str(build.resolve()), cache_sha256=hashlib.sha256(cache).hexdigest(),
                game_logging=logging, build_type=values.get('CMAKE_BUILD_TYPE'),
                scope='Logging-disabled Windows build' if logging == 'OFF' else 'Logging-enabled Windows build')
