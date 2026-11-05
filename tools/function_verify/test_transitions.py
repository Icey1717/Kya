"""Composition must retain actual production side effects between invocations."""
import copy
import json
from pathlib import Path
import unittest
from verify import Unsupported
import z3
from execution_state import ExecutionState
from memory_relation import MemoryRelation, store, load
from fp_relation import FloatPolicy
from transitions import TransitionStep, compare


class CompositionTests(unittest.TestCase):
    def setup_audio(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/transition-leaves.json').read_text())
        manifest, originals = fixture['positive'], fixture['originals']
        layout = manifest['layout']
        relation = MemoryRelation()
        relation.region('windows', 'audio', 0x1000, 0x1000)
        relation.region('ps2', 'audio', 0x1000, 0x1000)
        relation.region('windows', 'stack', 0x7fc0, 0x48, private=True)
        relation.region('ps2', 'stack', 0x7ff0, 0x20, private=True)
        relation.field('audio', layout['musicCount'], 'audio', 0x294, 4)
        relation.field('audio', layout['music'], 'audio', 0x290, 8, pointer=True)
        relation.identity(0x5000, 0x6000)
        w = z3.Array('audio_w', z3.BitVecSort(64), z3.BitVecSort(8))
        s = z3.Array('audio_s', z3.BitVecSort(32), z3.BitVecSort(8))
        counter = z3.BitVec('audio_count', 32)
        w = store(store(w, 0x1000 + layout['musicCount'], counter), 0x1000 + layout['music'], z3.BitVecVal(0x5000, 64))
        s = store(store(s, 0x1294, counter), 0x1290, z3.BitVecVal(0x6000, 32))
        steps = []
        for name, address in [('AudioEnter', '001844a0'), ('AudioLeave', '00184470')]:
            graph = next(e for e in manifest['exports'] if e['name'] == name)
            steps.append(TransitionStep(name, graph, originals[address], z3.BitVecVal(0, 64),
                windows_options=dict(regions=[(0x1000, 0x1000)], this_argument=z3.BitVecVal(0x1000, 64)),
                ps2_options=dict(regions=[(0x1000, 0x1000), (0x7f00, 0x110)], this_argument=z3.BitVecVal(0x1000, 32))))
        return steps, ExecutionState('windows', w), ExecutionState('ps2', s), relation, counter, layout

    def test_real_audio_enter_leave_composition(self):
        steps, w, s, relation, counter, layout = self.setup_audio()
        steps[1].precondition = lambda ws, ss: z3.And(
            load(ws.memory, 0x1000 + layout['musicCount'], 4) == counter + 1,
            load(ss.memory, 0x1294, 4) == counter + 1)
        result = compare(steps, w, s, relation, z3.BoolVal(True))
        self.assertEqual(result['result'], 'equivalent')
        self.assertEqual(len(result['steps']), 2)

    def test_write_footprint_frame_preserves_composition_and_detects_missing_store(self):
        steps, w, s, relation, _, _ = self.setup_audio()
        self.assertEqual(compare(steps, w, s, relation, z3.BoolVal(True), frame_by_writes=True)['result'], 'equivalent')
        steps[0].ps2 = copy.deepcopy(steps[0].ps2)
        for instruction in steps[0].ps2['instructions']:
            stores = [op for op in instruction['ops'] if op['opcode'] == 'STORE']
            for op in stores:
                instruction['ops'].remove(op)
                steps[0].ps2['operation_count'] -= 1
        self.assertEqual(compare(steps, w, s, relation, z3.BoolVal(True), frame_by_writes=True)['result'], 'mismatch')

    def test_write_footprint_rejects_unmapped_and_cross_boundary_stores(self):
        _, _, _, relation, _, layout = self.setup_audio()
        permitted = 0x1000 + layout['musicCount']
        for address, size in [(permitted, 4), (permitted + 3, 4), (0x3000, 4)]:
            claim = relation.writes_within_frame('windows', [(z3.BitVecVal(address, 64), z3.BitVecVal(0, size * 8))])
            self.assertEqual(z3.simplify(claim), z3.BoolVal(address == permitted))

    def test_successor_precondition_is_proved(self):
        steps, w, s, relation, counter, _ = self.setup_audio()
        steps[1].precondition = lambda ws, ss: load(ss.memory, 0x1294, 4) == counter
        with self.assertRaisesRegex(Unsupported, 'Successor precondition'):
            compare(steps, w, s, relation, z3.BoolVal(True))

    def test_missing_store_detected(self):
        steps, w, s, relation, _, _ = self.setup_audio()
        graph = copy.deepcopy(steps[0].ps2)
        for instruction in graph['instructions']:
            stores = [o for o in instruction['ops'] if o['opcode'] == 'STORE']
            for op in stores:
                instruction['ops'].remove(op)
                graph['operation_count'] -= 1
        steps[0].ps2 = graph
        self.assertEqual(compare(steps, w, s, relation, z3.BoolVal(True))['result'], 'mismatch')

    def test_empty_domain_rejected(self):
        steps, w, s, relation, _, _ = self.setup_audio()
        with self.assertRaisesRegex(Unsupported, 'empty'):
            compare(steps, w, s, relation, z3.BoolVal(False))

    def test_void_calls_can_use_an_explicit_state_without_integer_arguments(self):
        steps, w, s, relation, _, _ = self.setup_audio()
        for step in steps:
            step.argument = None
        self.assertEqual(compare(steps, w, s, relation, z3.BoolVal(True))['result'], 'equivalent')


class NumericRelationTests(unittest.TestCase):
    def check(self, a, b):
        return z3.is_true(z3.simplify(FloatPolicy().condition(z3.BitVecVal(a, 32), z3.BitVecVal(b, 32))))

    def test_tolerance_boundaries(self):
        self.assertTrue(self.check(0x3f800001, 0x3f800000))
        self.assertFalse(self.check(0x3f800100, 0x3f800000))

    def test_special_values(self):
        self.assertTrue(self.check(0, 0x80000000))
        self.assertTrue(self.check(0x7fc00001, 0x7fc00001))
        self.assertFalse(self.check(0x7fc00001, 0x7fc00002))
        self.assertFalse(self.check(0x7f800000, 0xff800000))

    def test_tolerance_requires_named_numeric_field(self):
        relation = MemoryRelation()
        with self.assertRaisesRegex(ValueError, 'named binary32'):
            relation.field('a', 0, 'a', 0, 8, pointer=True, numeric_policy=FloatPolicy())

    def test_tolerance_cannot_hide_a_later_gameplay_branch_difference(self):
        def node(space, offset, size):
            return dict(space=space, offset=f'{offset:x}', size=size, constant=space == 'const')
        def graph(side, branch=False):
            width, selector = (8, 0x1b1) if side == 'windows' else (4, 0x1a1)
            def store(value):
                return dict(opcode='STORE', inputs=[node('const', selector, width),
                    node('const', 0x1000, width), node('const', value, 4)])
            returning = ([dict(opcode='COPY', output=node('register', 0x288, 8), inputs=[node('const', 0xdead0000, 8)]),
                          dict(opcode='COPY', output=node('register', 0x20, 8), inputs=[node('const', 0x8008, 8)]),
                          dict(opcode='RETURN', inputs=[node('register', 0x288, 8)])] if side == 'windows' else
                         [dict(opcode='RETURN', inputs=[node('register', 0x1f0, 8)])])
            if branch:
                temporary, condition = node('unique', 0x100, 4), node('unique', 0x200, 1)
                instructions = [dict(address='200', text='decision', ops=[
                    dict(opcode='LOAD', output=temporary, inputs=[node('const', selector, width), node('const', 0x1000, width)]),
                    dict(opcode='INT_LESS', output=condition, inputs=[node('const', 0x3f800000, 4), temporary]),
                    dict(opcode='CBRANCH', inputs=[node('ram', 0x208, width), condition])]),
                    dict(address='204', text='false', ops=[dict(opcode='BRANCH', inputs=[node('ram', 0x20c, width)])]),
                    dict(address='208', text='true', ops=[dict(opcode='BRANCH', inputs=[node('ram', 0x20c, width)])]),
                    dict(address='20c', text='return', ops=returning)]
            else:
                instructions = [dict(address='100', text='store', ops=[store(0x3f800001 if side == 'windows' else 0x3f800000)]),
                                dict(address='104', text='return', ops=returning)]
            return dict(schema='kya.raw-pcode.v1', language='x86:LE:64:default' if side == 'windows' else 'r5900:LE:32:default',
                        compiler='windows' if side == 'windows' else 'default', big_endian=False, complete=True,
                        entry=instructions[0]['address'], instructions=instructions,
                        operation_count=sum(len(i['ops']) for i in instructions))
        relation = MemoryRelation()
        for side in ('windows', 'ps2'):
            relation.region(side, 'object', 0x1000, 4)
        relation.field('object', 0, 'object', 0, 4, numeric_policy=FloatPolicy(), name='animationTime')
        states = [ExecutionState(side, store(z3.Array(side + '_numeric', z3.BitVecSort(width), z3.BitVecSort(8)),
                                            0x1000, z3.BitVecVal(0x3f800000, 32))) for side, width in [('windows', 64), ('ps2', 32)]]
        steps = [TransitionStep('rounded output', graph('windows'), graph('ps2'), z3.BitVecVal(0, 64)),
                 TransitionStep('later gameplay decision', graph('windows', True), graph('ps2', True),
                                z3.BitVecVal(0, 64), decisions=(('200', '200'),))]
        for step in steps:
            step.windows_options['regions'] = [(0x1000, 4)]
            step.ps2_options['regions'] = [(0x1000, 4)]
        self.assertEqual(compare(steps[:1], *states, relation, z3.BoolVal(True))['result'], 'equivalent-with-tolerance')
        result = compare(steps, *states, relation, z3.BoolVal(True))
        self.assertEqual(result['result'], 'mismatch')
        self.assertEqual(result['step'], 'later gameplay decision')

    def test_unapproved_tolerance_report_rejected(self):
        from check import validate_report
        row = dict(result='equivalent-with-tolerance', ps2_entry='100',
                   numeric_policies={'animationTime': dict(absolute='0.000001', relative='0.00001')})
        with self.assertRaisesRegex(ValueError, 'unapproved numeric'):
            validate_report(dict(functions=[row]), ['100'])
        self.assertEqual(validate_report(dict(functions=[row]), ['100'], allowed_numeric_fields=['animationTime']), [row])

