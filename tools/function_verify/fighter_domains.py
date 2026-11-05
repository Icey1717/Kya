"""Explicit runtime domains and content-addressed transition proof jobs."""
from dataclasses import asdict, dataclass
from fighter_cases import digest
from proof_policy import validate_result


RESOURCE_PROFILES = ('null', 'controller-only', 'macro-table-only', 'populated-reset', 'populated')
BEHAVIOUR_PROFILES = ('empty', 'fighter-slot-0', 'fighter-transient-slot-0', 'fighter-general-one', 'fighter-general-two')
CONTRACT_OBLIGATIONS = (
    'runtime-inputs', 'animation-resources', 'configuration-invariants',
    'dispatch-closure', 'effect-closure', 'processor-semantics',
    'returning-paths', 'observations', 'memory-frame', 'callee-preservation',
)


@dataclass(frozen=True)
class ProofDomain:
    resources: str = 'null'
    behaviour: str = 'empty'

    def __post_init__(self):
        if self.resources not in RESOURCE_PROFILES or self.behaviour not in BEHAVIOUR_PROFILES:
            raise ValueError('Unknown fighter proof domain')

    def document(self):
        return asdict(self)

    @property
    def restrictions(self):
        restrictions = [
            'Animation resource profile: ' + self.resources,
            'Behaviour profile: ' + self.behaviour + ' (bounded list/registry domain)',
            'Current animation equals old-state configuration',
        ]
        if self.resources == 'populated-reset':
            restrictions += ['Empty macro lookup table; one initially stopped animation layer; no event tracks',
                             'Animation playback, morphing and callback invocation are outside this domain']
        return restrictions


def job_document(case, domain, inputs_sha256, *, policy='strict', reference_models=(), timeout=600, sequence_states=None):
    identity = dict(case_id=case.key, domain_profile=domain.document(), inputs_sha256=inputs_sha256,
                    proof_policy=policy, approved_models=list(reference_models), timeout=timeout)
    if sequence_states is not None:
        identity['sequence_states'] = list(sequence_states)
    return dict(identity, job_id=digest(identity))


def full_contract(row, *, policy='strict', approved_models=(), allowed_numeric_fields=()):
    """Legacy full_fighter_contract flags alone cannot discharge any obligation."""
    if row.get('restrictions') != [] or row.get('domain_profile') != dict(resources='full', behaviour='full'):
        return False
    obligations = row.get('obligations', {})
    if not isinstance(obligations, dict) or set(obligations) != set(CONTRACT_OBLIGATIONS):
        return False
    try:
        validate_result(row, policy=policy, approved_models=approved_models,
                        allowed_numeric_fields=allowed_numeric_fields)
        for proof in obligations.values():
            if not isinstance(proof, dict) or not proof.get('evidence_sha256'):
                return False
            if not row.get('inputs_sha256') or proof.get('inputs_sha256') != row['inputs_sha256']:
                return False
            evidence = proof['evidence_sha256']
            if not isinstance(evidence, str) or len(evidence) != 64 or any(c not in '0123456789abcdef' for c in evidence):
                return False
            validate_result(proof, policy=policy, approved_models=approved_models,
                            allowed_numeric_fields=allowed_numeric_fields)
    except (ValueError, TypeError):
        return False
    return True


def read_cached(path, job):
    """Reuse only validated completed restricted proofs with identical inputs."""
    import json
    try:
        row = json.loads(path.read_text())
        if not isinstance(row, dict) or any(row.get(key) != value for key, value in job.items()):
            return None
        validate_result(row, policy=job['proof_policy'], approved_models=job['approved_models'])
        return row
    except (OSError, ValueError, TypeError):
        return None
