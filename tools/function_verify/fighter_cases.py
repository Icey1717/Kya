"""Stable all-state case identities; generating an inventory is never a proof."""
from dataclasses import asdict, dataclass
import hashlib
import json
import struct

FIRST_STATE, STATE_COUNT = 6, 108
ANIMATION_MODES = ('explicit-unchanged', 'explicit-changed', 'configured')


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()


@dataclass(frozen=True)
class FighterCase:
    function: str
    old_state: int
    new_state: int
    animation: str
    idle_loops: str
    configuration_sha256: str

    @property
    def key(self):
        return digest(asdict(self))

    def document(self):
        return dict(asdict(self), case_id=self.key)


def configuration(raw):
    if len(raw) != STATE_COUNT * 8:
        raise ValueError('Fighter configuration must contain exactly 108 eight-byte entries')
    return [dict(state=FIRST_STATE + i, animation=struct.unpack_from('<i', raw, i * 8)[0],
                 flags=struct.unpack_from('<I', raw, i * 8 + 4)[0]) for i in range(STATE_COUNT)]


def cases(raw, *, pairs=(), family=None, shard=(0, 1)):
    table = configuration(raw)
    states = {row['state'] for row in table}
    pairs = set(pairs)
    if any(len(pair) != 2 or not set(pair) <= states for pair in pairs):
        raise ValueError('State pairs must be inside the fighter table (6..113)')
    index, count = shard
    if type(count) is not int or type(index) is not int or count < 1 or not 0 <= index < count:
        raise ValueError('Shard must satisfy 0 <= INDEX < COUNT')
    if family is not None and family not in {row['flags'] & 0xff800 for row in table}:
        raise ValueError('Unknown fighter execution-flags family')
    identity = hashlib.sha256(raw).hexdigest()
    for old in table:
        for new in table:
            if pairs and (old['state'], new['state']) not in pairs:
                continue
            if family is not None and new['flags'] & 0xff800 != family:
                continue
            for animation in ANIMATION_MODES:
                for loops in ('zero', 'nonzero'):
                    case = FighterCase('SetState', old['state'], new['state'], animation, loops, identity)
                    if int(case.key, 16) % count == index:
                        yield case
        if (not pairs or (old['state'], old['state']) in pairs) and (
                family is None or old['flags'] & 0xff800 == family):
            for animation in ('stand-unchanged', 'stand-changed'):
                for loops in ('zero', 'nonzero'):
                    case = FighterCase('SetStandAnim', old['state'], old['state'], animation, loops, identity)
                    if int(case.key, 16) % count == index:
                        yield case


def parse_pair(value):
    try:
        old, new = value.split(':')
        pair = (int(old, 0), int(new, 0))
    except ValueError:
        raise ValueError('Expected OLD:NEW, with decimal or 0x-prefixed state IDs') from None
    if not all(FIRST_STATE <= state < FIRST_STATE + STATE_COUNT for state in pair):
        raise ValueError('State pairs must be inside 6..113')
    return pair


def parse_shard(value):
    try:
        index, count = map(int, value.split('/'))
    except ValueError:
        raise ValueError('Expected INDEX/COUNT') from None
    if count < 1 or not 0 <= index < count:
        raise ValueError('Shard must satisfy 0 <= INDEX < COUNT')
    return index, count
