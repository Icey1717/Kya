"""Explicit acceptance policy. Reference assumptions can never become exact proofs."""
from ee_reference import REVISION

POLICIES = ('strict', 'pcsx2-reference')
BUDGET = dict(absolute='0.000001', relative='0.00001')


def reference_dependencies(value):
    dependencies = set()
    if isinstance(value, dict):
        own = value.get('reference_models', [])
        if not isinstance(own, (list, tuple)) or any(not isinstance(model, str) for model in own):
            raise ValueError('Invalid reference-model provenance')
        dependencies.update(own)
        for child in value.values():
            dependencies.update(reference_dependencies(child))
    elif isinstance(value, (list, tuple)):
        for child in value:
            dependencies.update(reference_dependencies(child))
    return dependencies


def validate_result(row, *, policy='strict', allowed_numeric_fields=(), approved_models=()):
    if policy not in POLICIES:
        raise ValueError('Unknown proof policy')
    references = row.get('reference_models', [])
    if (not isinstance(references, (list, tuple)) or any(not isinstance(m, str) for m in references)
            or len(references) != len(set(references))):
        raise ValueError('Invalid reference-model provenance')
    if reference_dependencies(row) != set(references):
        raise ValueError('Report dropped nested reference-model provenance')
    result = row.get('result')
    conditional = result in ('conditional-equivalent', 'conditional-equivalent-with-tolerance')
    tolerant = result in ('equivalent-with-tolerance', 'conditional-equivalent-with-tolerance')
    if references and not conditional:
        raise ValueError('Report depends on reference semantics but lost its conditional verdict')
    if conditional and not references:
        raise ValueError('Conditional verdict lacks reference-model provenance')
    if references and (policy != 'pcsx2-reference' or not set(references) <= set(approved_models)):
        raise ValueError('Report depends on unapproved reference semantics')
    if result not in ('equivalent', 'equivalent-with-tolerance',
                      'conditional-equivalent', 'conditional-equivalent-with-tolerance'):
        raise ValueError('Report contains a mismatch or incomplete proof')
    policies = row.get('numeric_policies', {})
    if tolerant:
        if (not policies or not set(policies) <= set(allowed_numeric_fields)
                or any(value != BUDGET for value in policies.values())):
            raise ValueError('Report contains unapproved numeric tolerance')
    elif policies:
        raise ValueError('Exact verdict contains hidden numeric tolerance')
    return row


def approve_reference(manifest):
    """Validate a captured oracle identity, not a source-only audit or relabelled vectors."""
    import re
    if (manifest.get('revision') != REVISION or manifest.get('kind') != 'pcsx2-instruction-oracle'
            or manifest.get('hardware_validated') is not False):
        raise ValueError('Reference approval requires the pinned executable emulator oracle')
    hashes = [manifest.get(key, '') for key in ('executable_sha256', 'vectors_sha256', 'source_manifest_sha256')]
    if any(not isinstance(value, str) or not re.fullmatch('[0-9a-f]{64}', value) for value in hashes):
        raise ValueError('Oracle identity is missing a content hash')
    if not manifest.get('backend') or not isinstance(manifest.get('settings'), dict) or not manifest['settings']:
        raise ValueError('Oracle backend and complete effective settings are required')
    models = manifest.get('models', [])
    if not models or len(models) != len(set(models)) or any(
            not isinstance(model, str) or not model.endswith(':' + REVISION) for model in models):
        raise ValueError('Oracle must identify pinned reference models')
    from ee_interpreter import MODEL, MXCSR
    settings = dict(mxcsr=hex(MXCSR), host_rounding='toward-zero', daz=False, ftz=False,
                    branch_and_memory_services='trap', guest_patches=False)
    if (models != [MODEL] or manifest['settings'] != settings
            or manifest['backend'] != 'COP1 interpreter (unmodified upstream FPU.cpp)'):
        raise ValueError('Instruction oracle cannot approve other emulator backends or settings')
    sources = {'pcsx2/FPU.cpp': '204dcb290f9f56e9112622ab313ef12d2192eda5b43d24efcf2dbc8ca3fa7344',
               'pcsx2/R5900.h': 'a6ef18e1b646b5ba6c956c10836aa4f75fd8309798f0ac0cf8644faa41cbf323',
               'common/Pcsx2Defs.h': '4fe46113263156a24ab56897c8c8606814400c0460c1de829aae81bed0b1156b',
               'common/Pcsx2Types.h': 'f1e3e26d639ae224f9a834bbb7cefb315ee3176316a486101a061de7275734c3'}
    if manifest.get('source_manifest') != sources:
        raise ValueError('Instruction oracle source differs from pinned upstream')
    return tuple(models)


def load_reference(path):
    import hashlib
    import json
    from pathlib import Path
    path = Path(path)
    manifest = json.loads(path.read_text())
    models = approve_reference(manifest)
    artifacts = {}
    for key in ('executable', 'vectors'):
        artifact = Path(manifest[key])
        if not artifact.is_absolute():
            artifact = path.parent / artifact
        if hashlib.sha256(artifact.read_bytes()).hexdigest() != manifest[key + '_sha256']:
            raise ValueError('Reference oracle artifact changed: ' + key)
        artifacts[key] = artifact.resolve()
    source_hash = hashlib.sha256(json.dumps(manifest['source_manifest'], sort_keys=True,
                                          separators=(',', ':')).encode()).hexdigest()
    if source_hash != manifest['source_manifest_sha256']:
        raise ValueError('Reference oracle source identity changed')
    import subprocess
    from ee_interpreter import MODEL, MXCSR, OPERATIONS
    vectors = json.loads(artifacts['vectors'].read_text())
    if (vectors.get('model') != MODEL or vectors.get('input_mxcsr') != MXCSR
            or {row['mnemonic'] for row in vectors.get('vectors', [])} != set(OPERATIONS)):
        raise ValueError('Reference vectors do not cover the approved interpreter profile')
    for key in ('revision', 'backend', 'settings', 'source_manifest', 'executable_sha256'):
        if vectors['oracle'].get(key) != manifest[key]:
            raise ValueError('Reference capture/build identities disagree: ' + key)
    command_input = ''.join(f"{r['mnemonic']} {r['a']:x} {r['b']:x} {r['acc_before']:x} {r['initial_fcr31']:x} {MXCSR:x}\n"
                            for r in vectors['vectors'])
    run = subprocess.run([str(artifacts['executable'])], input=command_input, text=True,
                         capture_output=True, timeout=30, check=True)
    expected = [f"{r['bits']:08x} {r['acc']:08x} {r['fcr31']:08x} {r['mxcsr']:08x}" for r in vectors['vectors']]
    if run.stdout.splitlines() != expected:
        raise ValueError('Reference executable no longer reproduces its captured observations')
    return models
