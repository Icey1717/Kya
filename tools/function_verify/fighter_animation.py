"""Bounded animation memory domains; no handwritten callee summaries."""
import z3
from memory_relation import store
from verify import Unsupported


def seed_reset(manifest, data, wm, sm):
    """One stopped layer and empty lookup table, exercising real reset bodies.

    Original offsets are audited at 00180450, 00241460, 00242680 and 002426f0.
    Callback identities come from actual COFF fixups and PS2 table bytes.
    No event track, animation descriptor or callback body is bypassed as a stub.
    """
    layout, actor = manifest['layouts']['Animation'], manifest['layouts']['Actor']
    required = ('controller_size', 'controller_count', 'controller_current_animation',
                'layer_blend_op', 'layer_play_state', 'layer_callback', 'layer_next_animation',
                'layer_current_animation', 'layer_play_rate', 'layer_field_d4')
    if any(key not in layout for key in required) or 'macro_table_size' not in actor:
        raise Unsupported('Missing compiler-backed populated-reset layouts')
    relation = data['relation']
    for side, name, base, size in [('windows', 'controller', 0x150000, layout['controller_size']),
                                  ('ps2', 'controller', 0x9000, 0x60),
                                  ('windows', 'layer', 0x160000, layout['layer_size']),
                                  ('ps2', 'layer', 0xa000, 0xd8),
                                  ('windows', 'macro', 0x170000, actor['macro_table_size']),
                                  ('ps2', 'macro', 0xb000, 4)]:
        relation.region(side, name, base, size)
        data['regions'][side].append((base, size))
    for wp, sp in [(0x150000, 0x9000), (0x160000, 0xa000), (0x170000, 0xb000), (0x100000, 0x1000)]:
        if (wp, sp) not in relation.identities:
            relation.identity(wp, sp)

    def field(region, wo, so, value, *, pointer=False, ps2_value=None):
        nonlocal wm, sm
        relation.field(region, wo, region, so, 8 if pointer else 4, pointer=pointer)
        wa = relation.regions['windows'][region].base + wo
        sa = relation.regions['ps2'][region].base + so
        wv = z3.BitVecVal(value, 64 if pointer else 32) if isinstance(value, int) else value
        sv = z3.BitVecVal(ps2_value if ps2_value is not None else value, 32) if isinstance(value, int) else value
        wm, sm = store(wm, wa, wv), store(sm, sa, sv)

    # Actor pointer fields have already been added to the relation by the caller.
    for key, wo, so, wp, sp in [('animation_controller', actor['actor_animation_controller'], 0x28, 0x150000, 0x9000),
                               ('macro_table', actor['actor_macro_table'], 0x114, 0x170000, 0xb000)]:
        wm = store(wm, 0x100000 + wo, z3.BitVecVal(wp, 64))
        sm = store(sm, 0x1000 + so, z3.BitVecVal(sp, 32))
    field('macro', actor['macro_count'], 0, 0)
    field('controller', layout['animator_layer_count'], 0, 1)
    field('controller', layout['animator_layers'], 4, 0x160000, pointer=True, ps2_value=0xa000)
    field('controller', layout['controller_count'], 0x2c, 0)
    field('controller', layout['controller_current_animation'], 0x30, z3.BitVec('reset_controller_animation', 32))
    for key, offset in [('blend_op', 0), ('next_animation', 0x64), ('current_animation', 0x10),
                        ('play_rate', 0xd0), ('field_d4', 0xd4)]:
        field('layer', layout['layer_' + key], offset, z3.BitVec('reset_layer_' + key, 32))
    field('layer', layout['layer_play_state'], 8, 0)
    field('layer', layout['layer_callback'], 0xc0, 0, pointer=True)
    field('layer', layout['layer_actor'], 0xc4, 0, pointer=True)

    callbacks = manifest['globals']['callbacks']
    evidence = manifest.get('ps2_data', {}).get('animation_callbacks', {})
    raw = bytes.fromhex(evidence.get('hex', ''))
    fixups = callbacks['relocations']
    if (len(raw) != 16 or evidence.get('address') != '0040e760' or len(fixups) != 4
            or [r['offset'] for r in fixups] != [0, 8, 16, 24] or any(r['addend'] for r in fixups)):
        raise Unsupported('Missing/invalid original animation callback table evidence')
    wb, sb = manifest['addresses'][callbacks['symbol']], 0x40e760
    data['regions']['windows'].append((wb, callbacks['size']))
    data['regions']['ps2'].append((sb, 16))
    for index, fixup in enumerate(fixups):
        wp = manifest['addresses'].get(fixup['symbol'])
        sp = int.from_bytes(raw[index * 4:index * 4 + 4], 'little')
        if not wp or not sp:
            raise Unsupported('Missing relocated callback identity')
        relation.identity(wp, sp)
        wm = store(wm, wb + index * 8, z3.BitVecVal(wp, 64))
        sm = store(sm, sb + index * 4, z3.BitVecVal(sp, 32))
    return wm, sm
