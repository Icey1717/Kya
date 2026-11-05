"""Finite virtual target sets are checked before executing complete callees."""
import copy
import json
from pathlib import Path
import unittest
from unittest.mock import patch
from call_graph import link
from paths import expand
from verify import execute, Unsupported
import z3


def node(space, offset, size=8):
    return dict(space=space, offset=hex(offset)[2:], size=size, constant=space == 'const')


def body(entry, instructions):
    return dict(schema='kya.raw-pcode.v1', language='r5900:LE:32:default', compiler='default',
                big_endian=False, complete=True, executable_sha256='test-image', entry=hex(entry)[2:],
                instructions=[dict(address=hex(address)[2:], text='test', ops=ops) for address, ops in instructions],
                operation_count=sum(len(ops) for _, ops in instructions))


class DispatchSetTests(unittest.TestCase):
    def graph(self, targets=(0x200, 0x300)):
        caller = body(0x100, [
            (0x100, [dict(opcode='COPY', output=node('register', 0x1f0), inputs=[node('const', 0x108)])]),
            (0x104, [dict(opcode='CALLIND', inputs=[node('register', 0x50)])]),
            (0x108, [dict(opcode='COPY', output=node('register', 0x1f0), inputs=[node('const', 0xdead0000)])]),
            (0x10c, [dict(opcode='RETURN', inputs=[node('register', 0x1f0)])])])
        callees = [body(entry, [(entry, [dict(opcode='COPY', output=node('register', 0x20), inputs=[node('const', value)])]),
                               (entry + 4, [dict(opcode='RETURN', inputs=[node('register', 0x1f0)])])])
                   for entry, value in [(0x200, 7), (0x300, 9)]]
        return link([caller, *callees], 0x100, indirect_targets={0x104: list(targets)})

    def test_both_targets_execute_and_cover_symbolic_dispatch(self):
        graph = self.graph()
        target = z3.BitVec('virtual_target', 64)
        domain = z3.Or(target == 0x200, target == 0x300)
        memory = z3.Array('dispatch_memory', z3.BitVecSort(32), z3.BitVecSort(8))
        guards = []
        for path in expand(graph):
            value, _, condition = execute(path, target, memory, call_mode=True, graph_mode=True,
                                          path_mode=True, precondition=domain)
            solver = z3.Solver()
            solver.add(domain, condition, value != z3.If(target == 0x200, z3.BitVecVal(7, 64), z3.BitVecVal(9, 64)))
            self.assertEqual(solver.check(), z3.unsat)
            guards.append(condition)
        solver = z3.Solver()
        solver.add(domain, z3.Not(z3.Or(*guards)))
        self.assertEqual(solver.check(), z3.unsat)
        self.assertEqual(len(guards), 2)
        self.assertTrue(all('original_address' in i and 'call_context' in i for i in graph['instructions']))

    def test_outside_target_is_rejected(self):
        memory = z3.Array('invalid_dispatch_memory', z3.BitVecSort(32), z3.BitVecSort(8))
        with self.assertRaisesRegex(Unsupported, 'dispatch target not proven'):
            execute(expand(self.graph())[0], z3.BitVecVal(0x400, 64), memory,
                    call_mode=True, graph_mode=True, path_mode=True, precondition=z3.BoolVal(True))

    def test_empty_duplicate_missing_and_recursive_sets_rejected(self):
        for targets in [(), (0x200, 0x200), (0x200, 0x400), (0x100, 0x200)]:
            with self.subTest(targets=targets), self.assertRaises(Unsupported):
                self.graph(targets)

    def test_raw_synthetic_checks_cannot_be_relinked(self):
        graph = copy.deepcopy(self.graph())
        with self.assertRaisesRegex(Unsupported, 'Synthetic'):
            link([graph], int(graph['entry'], 16))

    def test_real_windows_call_with_a_finite_target_set(self):
        from combat_cases import compare
        fixture = json.loads((Path(__file__).parent / 'fixtures/combat.json').read_text())
        manifest = copy.deepcopy(fixture['positive'])
        root = next(e for e in manifest['exports'] if e['name'] == 'CollisionReset')
        alternative = int(next(e for e in manifest['exports'] if e['name'] == 'MatrixIdentity')['entry'], 16)
        instruction = next(i for i in root['instructions'] if any(op['opcode'] == 'CALL' for op in i['ops']))
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'CALL')
        target = int(operation['inputs'][0]['offset'], 16)
        operation.update(opcode='CALLIND', inputs=[node('const', target)])
        def checked(exports, entry):
            targets = {int(instruction['address'], 16): [target, alternative]} if exports[0]['language'].startswith('x86') else None
            return link(exports, entry, indirect_targets=targets)
        with patch('call_graph.link', side_effect=checked):
            self.assertEqual(compare(manifest, 'CollisionReset', fixture['originals'], full_contact_flags=False)['result'], 'equivalent')

