"""Concrete collision/combat targets and compiler-probed memory layouts."""
CASES = [
    ('ActorFighter', '00328300', 'EndFlip', '?_EndFighterFlip@CActorFighter@@QEAAXXZ'),
    ('ActorFighter', '0031cdb0', 'Stagger', '?IsInStaggerSequence@CActorFighter@@QEAA_NXZ'),
    ('CollisionManager', '0019e310', 'CollisionClear', '?ClearInternalData@CCollision@@QEAAXXZ'),
    ('CollisionManager', '0019dba0', 'CollisionReset', '?Reset@CCollision@@QEAAXXZ'),
    ('MathOps', '00267850', 'MatrixIdentity', '?edF32Matrix4SetIdentityHard@@YAXPEATedF32MATRIX4@@@Z'),
]
IDENTITIES = {'EndFlip': 'CActorFighter::_EndFighterFlip', 'Stagger': 'CActorFighter::IsInStaggerSequence',
              'CollisionClear': 'CCollision::ClearInternalData', 'CollisionReset': 'CCollision::Reset',
              'MatrixIdentity': 'edF32Matrix4SetIdentityHard'}
FIELDS = [('flags', 'flags_0x0', 0, 4), ('contactFlags', 'flags_0x4', 4, 4),
          ('counter', 'field_0x18', 0x18, 4), ('actor', 'field_0x1c', 0x1c, 8),
          ('other', 'actorField', 0x28, 8), ('attached', 'actorFieldA', 0x2c, 8),
          ('matrix', 'transformMatrix', 0x30, 64), ('highest', 'highestVertex', 0x70, 16),
          ('lowest', 'lowestVertex', 0x80, 16), ('vector', 'field_0x90', 0x90, 16)]
LAYOUT_SOURCE = '#include "ActorFighter.h"\n#include "CollisionManager.h"\nextern "C" {\n'
LAYOUT_SOURCE += '\n'.join('extern const unsigned int ' + key + ' = __builtin_offsetof(CCollision, ' + field + ');' for key, field, _, _ in FIELDS)
LAYOUT_SOURCE += '''
extern const unsigned int collision = __builtin_offsetof(CActorFighter, pCollisionData);
extern const unsigned int state = __builtin_offsetof(CActorFighter, actorState);
extern const unsigned int pointer_size = sizeof(void*);
}
'''
LAYOUT_KEYS = [f[0] for f in FIELDS] + ['collision', 'state', 'pointer_size']


def compare(manifest, name, originals, *, full_contact_flags=False):
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
    if name == 'Stagger':
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
    graph = name in ('CollisionClear', 'CollisionReset')
    if graph:
        needed = ['CollisionClear', 'MatrixIdentity'] + (['CollisionReset'] if name == 'CollisionReset' else [])
        pc = link([pcs[n] for n in needed], int(pc['entry'], 16))
        ps2 = link([originals[pcs[n]['coff']['ps2_entry']] for n in needed], int(ps2['entry'], 16))
    pc_outcomes = []
    for path in expand(pc, max_instructions=512):
        result = execute_pc(path, z3.BitVecVal(0, 64), pm, path_mode=True, precondition=domain, regions=regions)
        if result is not None:
            pc_outcomes.append(result)
    ps2_outcomes = [execute_ps2(path, z3.BitVecVal(0, 64), sm, regions=regions + [(0x7f00, 0x110)],
                               precondition=domain, path_mode=True, call_mode=graph,
                               graph_mode=graph, vu_mode=name in ('MatrixIdentity', 'CollisionClear', 'CollisionReset'))
                    for path in expand(ps2, max_instructions=512)]
    prove(z3.Or(*[r[3] for r in pc_outcomes]), domain, 'Windows graph path coverage not proven')
    prove(z3.Or(*[r[2] for r in ps2_outcomes]), domain, 'PS2 graph path coverage not proven')
    def get(array, offset, size):
        parts = [array[offset + i] for i in reversed(range(size))]
        return z3.Concat(*parts) if size > 1 else parts[0]
    pa, sa = z3.BitVec('pc_witness', 64), z3.BitVec('ps2_witness', 32)
    pc_observed = z3.Or(z3.ULT(pa, 0x7e00), z3.UGE(pa, 0x8000))
    ps2_observed = z3.Or(z3.ULT(sa, 0x7f00), z3.UGE(sa, 0x8010)) if graph else z3.BoolVal(True)
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
                        'all actor state bit patterns' if name == 'Stagger' else
                        'arbitrary mapped fields; contact flags <= 255' if name == 'CollisionReset' and not full_contact_flags else 'arbitrary mapped field bits')
    if answer == z3.sat:
        result['counterexample'] = {key: hex(solver.model().eval(value, model_completion=True).as_long()) for key, value in symbols}
    elif answer == z3.unknown:
        result['reason'] = solver.reason_unknown()
    return result
