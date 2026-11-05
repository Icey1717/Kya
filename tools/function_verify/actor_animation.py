"""Actor/controller playback domain with exact Windows diagnostic observations."""
from verify import Unsupported
import z3
from memory_relation import store, load

ACTOR_PLAY = '?PlayAnim@CActor@@QEAAXH@Z'
ACTOR_ENTRY = '00103db0'
CONTROLLER = '?PlayAnim@CAnimation@@QEAAXPEAVCActor@@HH@Z'
META = '?SetAnim@edAnmBinMetaAnimator@@QEAAXHH@Z'
ORIGINAL_ENTRIES = (ACTOR_ENTRY, '001801b0', '00242230', '00103e70')


def seed(manifest, relation, memories, bases, sizes, original_id):
    actor, controller = manifest['layouts']['Actor'], manifest['layouts']['Animation']
    if (actor.get('macro_entries'), actor.get('macro_entry_size'), actor.get('macro_entry_animation'),
            actor.get('macro_entry_id')) != (4, 8, 0, 4) or 'controller_track_data' not in controller:
        raise Unsupported('Missing compiler-backed actor playback resource layouts')
    regions = dict(playback_actor=((0x1a0000, actor['actor_size']), (0xe000, 0x1000)),
        playback_controller=((0x1b0000, controller['controller_size']), (0xf000, 0x60)),
        playback_macro=((0x1c0000, 12), (0x10000, 12)),
        playback_bank_prefix=((bases['bank'][0] - 8, 8), (bases['bank'][1] - 8, 8)))
    for name, pair in regions.items():
        for index, side in enumerate(('windows', 'ps2')):
            relation.region(side, name, *pair[index])
    for pair in ((0x1b0000, 0xf000), (0x1c0000, 0x10000),
                 (bases['bank'][0] - 8, bases['bank'][1] - 8)):
        relation.identity(*pair)
    def field(name, wo, so, values, pointer=False):
        relation.field(name, wo, name, so, 8 if pointer else 4, pointer=pointer)
        for index in range(2):
            value = values[index]
            if isinstance(value, int):
                value = z3.BitVecVal(value, 64 if pointer and index == 0 else 32)
            memories[index] = store(memories[index], regions[name][index][0] + (wo if index == 0 else so), value)
    field('playback_actor', actor['actor_animation_controller'], 0x28, (0x1b0000, 0xf000), True)
    field('playback_actor', actor['actor_macro_table'], 0x114, (0x1c0000, 0x10000), True)
    field('playback_actor', actor['actor_animation'], 0x11c, (-1, -1))
    prev = z3.BitVec('actor_playback_previous_animation', 32)
    field('playback_actor', actor['actor_previous_animation'], 0x118, (prev, prev))
    field('playback_macro', 0, 0, (1, 1))
    field('playback_macro', 4, 4, (original_id, original_id))
    field('playback_macro', 8, 8, (0, 0))
    field('playback_controller', controller['animator_layer_count'], 0, (1, 1))
    field('playback_controller', controller['animator_layers'], 4, bases['layer'], True)
    field('playback_controller', controller['animator_bank'], 8,
          (bases['bank'][0] - 8, bases['bank'][1] - 8), True)
    field('playback_controller', controller['controller_current_animation'], 0x30, (-1, -1))
    field('playback_controller', controller['controller_count'], 0x2c, (1, 1))
    for i in range(4):
        value = z3.BitVec('actor_playback_track_data_' + str(i), 32)
        field('playback_controller', controller['controller_track_data'] + i * 4, 0x14 + i * 4, (value, value))
    field('playback_bank_prefix', 0, 0, (4, 4))
    field('playback_bank_prefix', 4, 4, (-1, -1))
    return seed_diagnostics(manifest, relation, memories, bases['bank'][0] - 8, sizes['bank'][0] + 8)


def seed_diagnostics(manifest, relation, memories, bank, bank_size):
    controller = manifest['layouts']['Animation']
    # Windows-only storage is not paired with invented PS2 writes. Each byte is
    # checked against its own contract, including the arbitrary history counter.
    def global_region(key):
        entry = manifest['globals'][key]
        address = manifest['addresses'][entry['symbol']]
        relation.region('windows', 'diagnostic_' + key, address, entry['size'], private=True)
        return address
    debug = {key: global_region(key) for key in ('snapshot', 'bank', 'history', 'lookup_count', 'scene')}
    snapshot = 0x1d0000
    relation.region('windows', 'diagnostic_snapshot_data', snapshot, bank_size, private=True)
    memories[0] = store(memories[0], debug['bank'], z3.BitVecVal(bank, 64))
    for offset, value in ((8, snapshot), (16, snapshot + bank_size), (24, snapshot + bank_size)):
        memories[0] = store(memories[0], debug['snapshot'] + offset, z3.BitVecVal(value, 64))
    memories[0] = store(memories[0], snapshot, load(memories[0], bank, bank_size))
    count = z3.BitVec('actor_playback_history_count', 32)
    memories[0] = store(memories[0], debug['lookup_count'], count)
    memories[0] = store(memories[0], debug['scene'] + controller['scene_track_manager'], z3.BitVecVal(0, 64))
    return memories


def gameplay_memory(relation, side, outcome, domain):
    """Project only certified private stores out of gameplay observations.

    Actual carried memory is never changed. Every skipped store is proved
    disjoint from mapped/public regions; diagnostics still inspect actual memory.
    Unknown ownership keeps the store in the view, never suppresses it.
    """
    certificates = {}
    def private_store(address, value):
        size = value.size() // 8
        key = (address, size)
        if key in certificates:
            return certificates[key]
        private = z3.simplify(z3.Or(*[z3.And(z3.UGE(address, base), z3.ULE(address, base + length - size))
                                    for base, length in relation.private[side] if length >= size]))
        omit = z3.is_true(private)
        if not omit and not z3.is_false(private):
            solver = z3.Solver()
            solver.set(timeout=5000)
            solver.add(domain, outcome.state.condition, z3.Not(private))
            omit = solver.check() == z3.unsat
        certificates[key] = omit
        return omit
    # Entry storage may contain private writes from a preceding invocation.
    layers, memory = [], outcome.baseline
    while z3.is_store(memory):
        layers.append((z3.simplify(memory.arg(1)), memory.arg(2)))
        memory = memory.arg(0)
    for address, value in list(reversed(layers)) + list(outcome.writes):
        address = z3.BitVecVal(address, memory.domain().size()) if isinstance(address, int) else address
        if not private_store(address, value):
            memory = store(memory, address, value)
    return memory


def observations(manifest, relation, bases, initial, final, side, original_id, writes, observed, domain):
    index = int(side == 'ps2')
    actor, controller = manifest['layouts']['Actor'], manifest['layouts']['Animation']
    claims = []
    for name in ('playback_actor', 'playback_controller', 'playback_macro', 'playback_bank_prefix'):
        region = relation.regions[side][name]
        expected = initial
        if name == 'playback_actor':
            expected = store(expected, region.base + (actor['actor_animation'] if index == 0 else 0x11c), original_id)
            expected = store(expected, region.base + (actor['actor_previous_animation'] if index == 0 else 0x118), z3.BitVecVal(-1, 32))
        elif name == 'playback_controller':
            expected = store(expected, region.base + (controller['controller_current_animation'] if index == 0 else 0x30), z3.BitVecVal(0, 32))
            expected = store(expected, region.base + (controller['controller_track_data'] if index == 0 else 0x14), z3.BitVecVal(0, 128))
        claims.append(load(observed, region.base, region.size) == load(expected, region.base, region.size))
    if index == 0:
        claims += diagnostic_observations(manifest, relation, initial, final, 0x1b0000, 0x1a0000, writes, domain)
    return claims


def diagnostic_observations(manifest, relation, initial, final, animator, actor, writes, domain):
    from region_memory import RegionMemory
    controller = manifest['layouts']['Animation']
    debug = {key: manifest['addresses'][manifest['globals'][key]['symbol']]
             for key in ('history', 'lookup_count')}
    count = load(initial, debug['lookup_count'], 4)
    # Separate heaps deliberately keep adjacent history/counter allocations
    # from merging into one view and reintroducing symbolic ring aliases.
    history_view = RegionMemory(final, [(debug['history'], manifest['globals']['history']['size'])],
                               domain, prune_initial_stores=True)
    counter_view = RegionMemory(final, [(debug['lookup_count'], 4)], domain, prune_initial_stores=True)
    claims = [counter_view.load(z3.BitVecVal(debug['lookup_count'], 64), 4, z3.BoolVal(True)) == count + 1]
    size = controller['history_entry_size']
    if size != 32 or manifest['globals']['history']['size'] != 32 * size:
        raise Unsupported('Unexpected animation history layout')
    entry = z3.BitVecVal(debug['history'], 64) + z3.ZeroExt(32, count & 31) * size
    for key, value, width in (('animator', animator, 64), ('actor', actor, 64),
                              ('layer', 0, 32), ('animation', 0, 32), ('offset', 4, 32), ('track', -1, 32)):
        claims.append(history_view.load(entry + controller['history_' + key], width // 8, z3.BoolVal(True)) == value % (1 << width))
    # The complete executor write trace proves all other history bytes and all
    # readonly diagnostic globals unchanged, without expanding 32 whole slots
    # into thousands of symbolic array reads.
    for address, value in writes:
        address = z3.BitVecVal(address, 64) if isinstance(address, int) else address
        end = address + value.size() // 8
        for name, region in relation.regions['windows'].items():
            if not name.startswith('diagnostic_'):
                continue
            disjoint = z3.Or(z3.ULE(end, region.base), z3.UGE(address, region.base + region.size))
            if name == 'diagnostic_history':
                claims.append(z3.Or(disjoint, z3.And(z3.UGE(address, entry), z3.ULE(end, entry + size))))
            elif name != 'diagnostic_lookup_count':
                claims.append(disjoint)
    return claims
