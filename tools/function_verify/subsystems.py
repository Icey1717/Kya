"""Audited identities and ABI/layout mappings; no source behavior models."""
CASES = [
    ('MemoryStream', '00189b50', 'ReadU32', '?GetU32@ByteCode@@QEAAIXZ'),
    ('MemoryStream', '00189b70', 'ReadS32', '?GetS32@ByteCode@@QEAAHXZ'),
    ('MemoryStream', '00189be0', 'SetCursor', '?SetPosition@ByteCode@@QEAAXPEAD@Z'),
    ('SaveManagement', '002f5220', 'ClearSlot', '?clear_slot@CSaveManagement@@QEAAXXZ'),
    ('Frontend', '001dcdc0', 'GetMagic', '?HasMagicInteractionAround@CMagicInterface@@QEAAHXZ'),
    ('Frontend', '001dcdd0', 'SetMagic', '?SetHasMagicInteractionAround@CMagicInterface@@QEAAXH@Z'),
    ('Frontend', '001dc990', 'GetPriority', '?GetPriority@CLifeInterface@@QEAAHXZ'),
    ('Frontend', '001dc9a0', 'SetPriority', '?SetPriority@CLifeInterface@@QEAAXH@Z'),
]
# (original identity, layout constant, original field offset, return bits).
MAPPINGS = {
    'ReadU32': ('ByteCode::GetU32', 'cursor', 12, 32),
    'ReadS32': ('ByteCode::GetS32', 'cursor', 12, 32),
    'SetCursor': ('ByteCode::SetPosition', 'cursor', 12, 0),
    'ClearSlot': ('CSaveManagement::clear_slot', 'slot', 0x28, 0),
    'GetMagic': ('CMagicInterface::HasMagicInteractionAround', 'magic', 8, 32),
    'SetMagic': ('CMagicInterface::SetHasMagicInteractionAround', 'magic', 8, 0),
    'GetPriority': ('CLifeInterface::GetPriority', 'priority', 4, 32),
    'SetPriority': ('CLifeInterface::SetPriority', 'priority', 4, 0),
}
LAYOUT_SOURCE = '''#include "MemoryStream.h"
#include "SaveManagement.h"
#include "Frontend.h"
extern "C" {
extern const unsigned int cursor = __builtin_offsetof(ByteCode, currentSeekPos);
extern const unsigned int slot = __builtin_offsetof(CSaveManagement, slotID_0x28);
extern const unsigned int magic = __builtin_offsetof(CMagicInterface, bHasMagicAround);
extern const unsigned int priority = __builtin_offsetof(CLifeInterface, priority);
extern const unsigned int pointer_size = sizeof(char*);
}
'''


def compare(manifest, name, originals):
    from verify import Unsupported, execute as ps2_execute
    import z3
    from windows_pcode import execute as pc_execute
    from paths import expand
    from compare_extended import store, word, prove

    pc = next(e for e in manifest['exports'] if e['name'] == name)
    ps2 = originals[pc['coff']['ps2_entry']]
    _, layout_key, ps2_offset, bits = MAPPINGS[name]
    if manifest['layout']['pointer_size'] != 8:
        raise Unsupported('Expected 64-bit Windows pointers')
    pointer = layout_key == 'cursor'
    po, so = 0x1000 + manifest['layout'][layout_key], 0x1000 + ps2_offset
    width = 8 if pointer else 4
    pm = z3.Array('pc_memory', z3.BitVecSort(64), z3.BitVecSort(8))
    sm = z3.Array('ps2_memory', z3.BitVecSort(32), z3.BitVecSort(8))
    field, argument, payload = (z3.BitVec(key, 32) for key in ['initial_field', 'argument', 'payload'])
    reading = name in ('ReadU32', 'ReadS32')
    initial = z3.BitVecVal(0x4000, 32) if reading else field
    pm = store(pm, po, z3.ZeroExt(32, initial) if pointer else initial)
    sm = store(sm, so, initial)
    if reading:
        pm, sm = store(pm, 0x4000, payload), store(sm, 0x4000, payload)
    pc_regions = [(0x1000, 0x1000)] + ([(0x4000, 4)] if reading else [])
    ps2_regions = [(0x1000, ps2_offset + 4)] + ([(0x4000, 4)] if reading else [])
    domain = z3.BoolVal(True)
    pc_outcomes = [pc_execute(path, z3.ZeroExt(32, argument), pm, path_mode=True,
                             precondition=domain, regions=pc_regions) for path in expand(pc)]
    pc_outcomes = [r for r in pc_outcomes if r is not None]
    ps2_outcomes = [ps2_execute(path, z3.SignExt(32, argument), sm, precondition=domain,
                               regions=ps2_regions, path_mode=True) for path in expand(ps2)]
    prove(z3.Or(*[r[3] for r in pc_outcomes]), domain, 'Windows path coverage not proven')
    prove(z3.Or(*[r[2] for r in ps2_outcomes]), domain, 'PS2 path coverage not proven')
    pa, sa = z3.BitVec('pc_address', 64), z3.BitVec('ps2_address', 32)
    pc_observed = z3.And(z3.Or(z3.ULT(pa, 0x7e00), z3.UGE(pa, 0x8000)),
                         z3.Or(z3.ULT(pa, po), z3.UGE(pa, po + width)))
    ps2_observed = z3.Or(z3.ULT(sa, so), z3.UGE(sa, so + 4))
    differences = []
    observations = []
    for pv, pf, baseline, pg in pc_outcomes:
        for sv, sf, sg in ps2_outcomes:
            pc_field, ps2_field = word(pf, po, width), word(sf, so, 4)
            mapped_field = z3.ZeroExt(32, ps2_field) if pointer else ps2_field
            diff = [pc_field != mapped_field,
                    z3.And(pc_observed, pf[pa] != baseline[pa]),
                    z3.And(ps2_observed, sf[sa] != sm[sa])]
            if bits:
                if pv is None or sv is None:
                    raise Unsupported('Missing return value')
                diff.append(z3.Extract(bits - 1, 0, pv) != z3.Extract(bits - 1, 0, sv))
            differences.append(z3.And(pg, sg, z3.Or(*diff)))
            observations.append((pg, sg, pv, sv, pc_field, ps2_field))
    solver = z3.Solver()
    solver.set(timeout=10000)
    solver.add(z3.Or(*differences))
    verdict = solver.check()
    result = dict(result={'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(verdict)],
                  windows_paths=len(pc_outcomes), ps2_paths=len(ps2_outcomes),
                  domain=('aligned readable uint32 at modeled cursor 0x4000; all payload bits' if reading else
                          'all 32-bit initial field and argument values'),
                  mapping=dict(windows_offset=po-0x1000, ps2_offset=ps2_offset, pointer=pointer))
    if verdict == z3.sat:
        model = solver.model()
        result['counterexample'] = {key: hex(model.eval(value, model_completion=True).as_long())
                                   for key, value in [('initial_field', field), ('argument', argument), ('payload', payload)]}
        for pg, sg, pv, sv, pc_field, ps2_field in observations:
            if z3.is_true(model.eval(z3.And(pg, sg), model_completion=True)):
                values = [('windows_field', pc_field), ('ps2_field', ps2_field)]
                if bits:
                    values += [('windows_return', z3.Extract(bits-1, 0, pv)), ('ps2_return', z3.Extract(bits-1, 0, sv))]
                result['counterexample'].update({key: hex(model.eval(value, model_completion=True).as_long()) for key, value in values})
    elif verdict == z3.unknown:
        result['reason'] = solver.reason_unknown()
    return result
