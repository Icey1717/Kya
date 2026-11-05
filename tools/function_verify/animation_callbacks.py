"""Checked layer-zero callback dispatch into the real fighter weight update."""
from verify import Unsupported
import z3
from memory_relation import store, load

CALLBACK = '?gAnimation_Callback_Layer0@@YAXPEAUedAnmMacroAnimator@@PEAVCActor@@I@Z'
FIGHTER = '?AnimEvaluate@CActorFighter@@UEAAXIPEAUedAnmMacroAnimator@@I@Z'
BASE = '?AnimEvaluate@CActor@@UEAAXIPEAUedAnmMacroAnimator@@I@Z'
ENTRIES = ('0017f470', '0031baa0', '00100bf0')


def dispatch(manifest, selected, originals):
    rows = [r for r in manifest['dispatch']['mappings'] if r['windows_symbol'] == FIGHTER]
    if len(rows) != 1 or rows[0]['ps2_offset'] != 0xa0 or rows[0]['ps2_target'] != ENTRIES[1]:
        raise Unsupported('Missing concrete fighter callback dispatch evidence')
    row = rows[0]
    windows, ps2 = {}, {}
    for body, targets, target in ((selected['?UpdateAnimParams@edAnmMacroAnimator@@QEAAXXZ'], windows,
                                  int(selected[CALLBACK]['entry'], 16)),
                                 (selected[CALLBACK], windows, int(selected[FIGHTER]['entry'], 16)),
                                 (originals['00241b40'], ps2, int(ENTRIES[0], 16)),
                                 (originals[ENTRIES[0]], ps2, int(ENTRIES[1], 16))):
        instructions = [i for i in body['instructions'] if any(op['opcode'] in ('CALLIND', 'BRANCHIND') for op in i['ops'])]
        if len(instructions) != 1:
            raise Unsupported('Ambiguous callback indirect transfer')
        targets[int(instructions[0]['address'], 16)] = target
    return windows, ps2, row


def seed(manifest, relation, memories, row):
    a, f, c = manifest['layouts']['Actor'], manifest['layouts']['ActorFighter'], manifest['layouts']['Animation']
    if 'fighter_animation_blend' not in f:
        raise Unsupported('Missing compiler-backed fighter callback blend layout')
    regions = dict(callback_actor=((0x1a0000, f['fighter_size']), (0xe000, 0x1000)),
                   callback_controller=((0x1b0000, c['controller_size']), (0xf000, 0x60)),
                   callback_vtable=((0x1c0000, row['windows_offset'] + 8), (0x441a90, 0xa4)))
    for name, pair in regions.items():
        for index, side in enumerate(('windows', 'ps2')):
            relation.region(side, name, *pair[index])
    for pair in ((0x1b0000, 0xf000), (0x1c0000, 0x441a90)):
        relation.identity(*pair)
    symbols = {e['coff']['symbol']: int(e['entry'], 16) for e in manifest['exports']}
    relation.identity(symbols[CALLBACK], int(ENTRIES[0], 16))
    relation.identity(symbols[FIGHTER], int(ENTRIES[1], 16))
    def field(name, offsets, values, pointer=False):
        relation.field(name, offsets[0], name, offsets[1], 8 if pointer else 4, pointer=pointer)
        for index in range(2):
            memories[index] = store(memories[index], regions[name][index][0] + offsets[index],
                                    z3.BitVecVal(values[index], 64 if pointer and index == 0 else 32))
    field('callback_actor', (0, 0), (0x1c0000, 0x441a90), True)
    field('callback_actor', (a['actor_animation_controller'], 0x28), (0x1b0000, 0xf000), True)
    field('callback_actor', (f['fighter_animation_blend'], 0x4fc), (0x3e800000, 0x3e800000))
    field('callback_controller', (c['controller_count'], 0x2c), (1, 1))
    field('callback_vtable', (row['windows_offset'], 0xa0), (symbols[FIGHTER], int(ENTRIES[1], 16)), True)
    literal = manifest['addresses'].get('__real@3f800000')
    if literal is None:
        raise Unsupported('Missing compiler-backed fighter callback unit constant')
    relation.region('windows', 'callback_unit_constant', literal, 4, private=True)
    memories[0] = store(memories[0], literal, z3.BitVecVal(0x3f800000, 32))
    return memories, (symbols[CALLBACK], int(ENTRIES[0], 16))


def preservation(relation, side, initial, final):
    return [load(initial, r.base, r.size) == load(final, r.base, r.size)
            for name, r in relation.regions[side].items() if name.startswith('callback_')]
