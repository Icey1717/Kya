"""Concrete collision/combat targets and compiler-probed memory layouts."""
CASES = [
    ('ActorFighter', '00328300', 'EndFlip', '?_EndFighterFlip@CActorFighter@@QEAAXXZ'),
    ('ActorFighter', '0031cdb0', 'Stagger', '?IsInStaggerSequence@CActorFighter@@QEAA_NXZ'),
    ('CollisionManager', '0019e310', 'CollisionClear', '?ClearInternalData@CCollision@@QEAAXXZ'),
    ('CollisionManager', '0019dba0', 'CollisionReset', '?Reset@CCollision@@QEAAXXZ'),
    ('MathOps', '00267850', 'MatrixIdentity', '?edF32Matrix4SetIdentityHard@@YAXPEATedF32MATRIX4@@@Z'),
    ('Audio', '00184470', 'AudioLeave', '?FUN_00184470@CAudioManager@@QEAAXXZ'),
    ('Audio', '001844a0', 'AudioEnter', '?FUN_001844a0@CAudioManager@@QEAAXXZ'),
    ('Animation', '002425a0', 'LayerTimeWarper', '?SetLayerTimeWarper@edAnmBinMetaAnimator@@QEAAXMH@Z'),
    ('ActorFighter', '0031ba50', 'StandAnimNoDispatch', '?SetStandAnim@CActorFighter@@QEAAXH@Z'),
]
IDENTITIES = {'EndFlip': 'CActorFighter::_EndFighterFlip', 'Stagger': 'CActorFighter::IsInStaggerSequence',
              'CollisionClear': 'CCollision::ClearInternalData', 'CollisionReset': 'CCollision::Reset',
              'MatrixIdentity': 'edF32Matrix4SetIdentityHard',
              'AudioLeave': 'CAudioManager::FUN_00184470', 'AudioEnter': 'CAudioManager::FUN_001844a0',
              'LayerTimeWarper': 'edAnmMetaAnimator::SetLayerTimeWarper',
              'StandAnimNoDispatch': 'CActorFighter::SetStandAnim'}
FIELDS = [('flags', 'flags_0x0', 0, 4), ('contactFlags', 'flags_0x4', 4, 4),
          ('counter', 'field_0x18', 0x18, 4), ('actor', 'field_0x1c', 0x1c, 8),
          ('other', 'actorField', 0x28, 8), ('attached', 'actorFieldA', 0x2c, 8),
          ('matrix', 'transformMatrix', 0x30, 64), ('highest', 'highestVertex', 0x70, 16),
          ('lowest', 'lowestVertex', 0x80, 16), ('vector', 'field_0x90', 0x90, 16)]
LAYOUT_SOURCE = '#include "ActorFighter.h"\n#include "CollisionManager.h"\n#include "Audio.h"\nextern "C" {\n'
LAYOUT_SOURCE += '\n'.join('extern const unsigned int ' + key + ' = __builtin_offsetof(CCollision, ' + field + ');' for key, field, _, _ in FIELDS)
LAYOUT_SOURCE += '''
extern const unsigned int collision = __builtin_offsetof(CActorFighter, pCollisionData);
extern const unsigned int state = __builtin_offsetof(CActorFighter, actorState);
extern const unsigned int pointer_size = sizeof(void*);
extern const unsigned int music = __builtin_offsetof(CAudioManager, field_0x290);
extern const unsigned int musicCount = __builtin_offsetof(CAudioManager, field_0x294);
extern const unsigned int layers = __builtin_offsetof(edAnmBinMetaAnimator, aAnimData);
extern const unsigned int playRate = __builtin_offsetof(edAnmLayer, playRate);
extern const unsigned int standAnim = __builtin_offsetof(CActorFighter, standAnim);
}
'''
LAYOUT_KEYS = [f[0] for f in FIELDS] + ['collision', 'state', 'pointer_size', 'music', 'musicCount', 'layers', 'playRate', 'standAnim']


def compare(manifest, name, originals, *, full_contact_flags=True):
    from verify import Unsupported, execute as execute_ps2
    import z3
    from windows_pcode import execute as execute_pc
    from compare_extended import store, word, prove
    from call_graph import link
    from paths import expand

    pcs = {e['name']: e for e in manifest['exports']}
    pc = pcs[name]
    ps2 = originals[pc['coff']['ps2_entry']]
    layout = manifest['layout']
    if layout['pointer_size'] != 8:
        raise Unsupported('Expected 64-bit Windows pointers')
    pm = z3.Array('pc_memory', z3.BitVecSort(64), z3.BitVecSort(8))
    sm = z3.Array('ps2_memory', z3.BitVecSort(32), z3.BitVecSort(8))
    mapped, symbols = [], []
    regions = [(0x1000, 0x2000)]
    for symbol, offset in manifest.get('addresses', {}).items():
        if symbol.startswith('__real@'):
            value = z3.BitVecVal(int(symbol.split('@')[1], 16), 32)
            pm = store(pm, offset, value)
            regions.append((offset, 4))
    domain = z3.BoolVal(True)
    # EndFlip follows a symbolic, aligned, non-null pointer to a separate
    # collision object. The remaining functions operate directly on this.
    pointer = z3.BitVec('collision_pointer', 32)
    pc_base = z3.ZeroExt(32, pointer) if name == 'EndFlip' else z3.BitVecVal(0x1000, 64)
    ps2_base = pointer if name == 'EndFlip' else z3.BitVecVal(0x1000, 32)
    if name == 'EndFlip':
        domain = z3.And(z3.UGE(pointer, 0x4000), z3.ULE(pointer, 0x5000), pointer & 15 == 0)
        pm, sm = store(pm, 0x1000 + layout['collision'], z3.ZeroExt(32, pointer)), store(sm, 0x102c, pointer)
        regions += [(0x4000, 0x1200)]
        symbols.append(('collision_pointer', pointer))
    def put(array, address, value):
        for i in range(value.size() // 8):
            array = z3.Store(array, address + i, z3.Extract(i*8+7, i*8, value))
        return array
    argument = z3.BitVec('stand_animation_argument', 32) if name == 'StandAnimNoDispatch' else z3.BitVecVal(0, 32)
    float_argument = z3.BitVec('play_rate_bits', 32) if name == 'LayerTimeWarper' else None
    if name == 'StandAnimNoDispatch':
        values = {}
        for key, offset in [('standAnim', 0x480), ('state', 0x104)]:
            value = z3.BitVec('initial_' + key, 32)
            values[key] = value
            symbols.append((key, value))
            pm = store(pm, 0x1000 + layout[key], value)
            sm = store(sm, 0x1000 + offset, value)
            mapped.append((pc_base + layout[key], ps2_base + offset, 4, 4))
        symbols.append(('argument', argument))
        domain = z3.Or(values['standAnim'] == argument, values['state'] != 6)
    elif name == 'LayerTimeWarper':
        pm = store(pm, 0x1000 + layout['layers'], z3.BitVecVal(0x4000, 64))
        sm = store(sm, 0x1004, z3.BitVecVal(0x4000, 32))
        value = z3.BitVec('initial_play_rate', 32)
        pm = store(pm, 0x4000 + layout['playRate'], value)
        sm = store(sm, 0x40d0, value)
        regions.append((0x4000, 0x1000))
        mapped.append((z3.BitVecVal(0x4000 + layout['playRate'], 64), z3.BitVecVal(0x40d0, 32), 4, 4))
        symbols.extend([('play_rate_bits', float_argument), ('initial_play_rate', value)])
    elif name in ('AudioEnter', 'AudioLeave'):
        for key, offset, width in [('music', 0x290, 8), ('musicCount', 0x294, 4)]:
            value = z3.BitVec('initial_' + key, 32)
            symbols.append((key, value))
            po, so = pc_base + layout[key], ps2_base + offset
            pm = put(pm, po, z3.ZeroExt(32, value) if width == 8 else value)
            sm = put(sm, so, value)
            mapped.append((po, so, width, 4))
    elif name == 'Stagger':
        value = z3.BitVec('actor_state', 32)
        pm, sm = store(pm, 0x1000 + layout['state'], value), store(sm, 0x1104, value)
        symbols.append(('actor_state', value))
    elif name == 'MatrixIdentity':
        value = z3.BitVec('initial_matrix', 512)
        pm, sm = store(pm, 0x1000, value), store(sm, 0x1000, value)
        mapped.append((z3.BitVecVal(0x1000, 64), z3.BitVecVal(0x1000, 32), 64, 64))
    else:
        for key, _, offset, width in FIELDS:
            if name == 'EndFlip' and key not in ('flags', 'attached'):
                continue
            pointer_field = width == 8
            value = z3.BitVec('initial_' + key, 32 if pointer_field else width * 8)
            symbols.append((key, value))
            if key == 'contactFlags' and name == 'CollisionReset' and not full_contact_flags:
                domain = z3.And(domain, z3.ULE(value, 255))
            po, so = pc_base + layout[key], ps2_base + offset
            pm = put(pm, po, z3.ZeroExt(32, value) if pointer_field else value)
            sm = put(sm, so, value)
            mapped.append((po, so, width, 4 if pointer_field else width))
    feasibility = z3.Solver()
    feasibility.set(timeout=5000)
    feasibility.add(domain)
    if feasibility.check() != z3.sat:
        raise Unsupported('Input domain is unsatisfiable or feasibility is inconclusive')
    graph = name in ('CollisionClear', 'CollisionReset')
    if name == 'StandAnimNoDispatch':
        # Keep the unresolved idle dispatch as an executable frontier. Both
        # executors must prove its guard impossible in this restricted domain.
        pc = link([pc], int(pc['entry'], 16), defer_unresolved=True)
        ps2 = link([ps2], int(ps2['entry'], 16), defer_unresolved=True)
    if graph:
        needed = ['CollisionClear', 'MatrixIdentity'] + (['CollisionReset'] if name == 'CollisionReset' else [])
        pc = link([pcs[n] for n in needed], int(pc['entry'], 16))
        ps2 = link([originals[pcs[n]['coff']['ps2_entry']] for n in needed], int(ps2['entry'], 16))
    pc_outcomes = []
    loop_bound = 1 if name == 'LayerTimeWarper' else None
    for path in expand(pc, max_instructions=512, loop_visits=loop_bound):
        result = execute_pc(path, z3.ZeroExt(32, argument), pm, path_mode=True, precondition=domain, regions=regions,
                            second_argument=z3.BitVecVal(0, 64) if float_argument is not None else None,
                            float_argument=float_argument)
        if result is not None:
            pc_outcomes.append(result)
    ps2_outcomes = [execute_ps2(path, z3.SignExt(32, argument), sm, regions=regions + [(0x7f00, 0x110)],
                               precondition=domain, path_mode=True, call_mode=graph or name == 'StandAnimNoDispatch',
                               graph_mode=graph, vu_mode=name in ('MatrixIdentity', 'CollisionClear', 'CollisionReset'),
                               float_argument=float_argument, loop_mode=loop_bound is not None)
                    for path in expand(ps2, max_instructions=512, loop_visits=loop_bound)]
    prove(z3.Or(*[r[3] for r in pc_outcomes]), domain, 'Windows graph path coverage not proven')
    prove(z3.Or(*[r[2] for r in ps2_outcomes]), domain, 'PS2 graph path coverage not proven')
    def get(array, offset, size):
        parts = [array[offset + i] for i in reversed(range(size))]
        return z3.Concat(*parts) if size > 1 else parts[0]
    pa, sa = z3.BitVec('pc_witness', 64), z3.BitVec('ps2_witness', 32)
    pc_observed = z3.Or(z3.ULT(pa, 0x7e00), z3.UGE(pa, 0x8000))
    ps2_observed = z3.Or(z3.ULT(sa, 0x7f00), z3.UGE(sa, 0x8010)) if graph else (
        z3.Or(z3.ULT(sa, 0x8000), z3.UGE(sa, 0x8010)) if name == 'StandAnimNoDispatch' else z3.BoolVal(True))
    for po, so, pw, sw in mapped:
        pc_observed = z3.And(pc_observed, z3.Or(z3.ULT(pa, po), z3.UGE(pa, po + pw)))
        ps2_observed = z3.And(ps2_observed, z3.Or(z3.ULT(sa, so), z3.UGE(sa, so + sw)))
    differences = []
    for pv, pf, baseline, pg in pc_outcomes:
        for sv, sf, sg in ps2_outcomes:
            changes = [z3.And(pc_observed, pf[pa] != baseline[pa]), z3.And(ps2_observed, sf[sa] != sm[sa])]
            for po, so, pw, sw in mapped:
                target = get(sf, so, sw)
                if pw != sw:
                    target = z3.ZeroExt((pw-sw)*8, target)
                changes.append(get(pf, po, pw) != target)
            if name == 'Stagger':
                changes.append(z3.Extract(7, 0, pv) != z3.Extract(7, 0, sv))
            differences.append(z3.And(pg, sg, z3.Or(*changes)))
    solver = z3.Solver()
    solver.set(timeout=30000)
    solver.add(domain, z3.Or(*differences))
    answer = solver.check()
    result = dict(result={'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(answer)],
                  windows_paths=len(pc_outcomes), ps2_paths=len(ps2_outcomes))
    result['domain'] = ('aligned collision pointer in 0x4000..0x5000; arbitrary uint32 flags and attached pointer identity' if name == 'EndFlip' else
                        'unchanged standing animation OR actor state != 6; dispatching idle branch excluded and remains unsupported' if name == 'StandAnimNoDispatch' else
                        'layer index 0; disjoint valid layer storage at 0x4000; every raw binary32 play-rate bit pattern' if name == 'LayerTimeWarper' else
                        'all actor state bit patterns' if name == 'Stagger' else
                        'arbitrary mapped fields; contact flags <= 255' if name == 'CollisionReset' and not full_contact_flags else 'arbitrary mapped field bits')
    if answer == z3.sat:
        result['counterexample'] = {key: hex(solver.model().eval(value, model_completion=True).as_long()) for key, value in symbols}
    elif answer == z3.unknown:
        result['reason'] = solver.reason_unknown()
    return result
