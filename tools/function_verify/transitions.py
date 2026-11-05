"""Compose complete p-code invocations without reinitializing runtime storage."""
from dataclasses import dataclass, field
from execution_state import ExecutionState
from paths import expand
from verify import Unsupported, execute as ps2_execute
from windows_pcode import execute as windows_execute
import z3


def prove(claim, domain, reason, timeout=10000):
    solver = z3.Solver()
    solver.set(timeout=timeout)
    solver.add(domain, z3.Not(claim))
    if solver.check() != z3.unsat:
        raise Unsupported(reason)


@dataclass
class BranchDecision:
    windows: str
    ps2: str
    inverted: bool = False


@dataclass
class TransitionStep:
    name: str
    windows: dict
    ps2: dict
    argument: object
    windows_options: dict = field(default_factory=dict)
    ps2_options: dict = field(default_factory=dict)
    precondition: object = None  # Callable of the two actual entry states.
    decisions: tuple = ()  # Explicit pairs of gameplay branch source addresses.
    return_bits: int = 0  # Void functions have no return observation.
    windows_auxiliary_decisions: tuple = ()
    ps2_auxiliary_decisions: tuple = ()
    loop_visits: int = 4
    max_paths: int = 4096
    max_instructions: int = 4096
    max_pairs: int = 4096
    progress: object = None

    def __post_init__(self):
        self.decisions = tuple(d if isinstance(d, BranchDecision) else BranchDecision(*d) for d in self.decisions)
        if min(self.max_paths, self.max_instructions, self.max_pairs) < 1:
            raise ValueError('Transition exploration limits must be positive')


def invocation(previous):
    """External call ABI setup; heap, globals, and processor state survive."""
    registers = dict(previous.registers)
    # Reset only ABI call-entry storage; do not introduce overlapping aliases.
    entries = [(0x20, 8, 0x8000)] if previous.architecture == 'windows' else [
        (0x1d0, 8, 0x8010), (0x1f0, 8, 0xdead0000)]
    for offset, size, value in entries:
        for old in list(registers):
            if old[1] < offset + size and offset < old[1] + old[2]:
                del registers[old]
        registers[('register', offset, size)] = z3.BitVecVal(value, size * 8)
    return ExecutionState(previous.architecture, previous.memory, registers,
                          previous.condition, dict(previous.processor))


def outcomes(graph, entry, argument, domain, options, step):
    execute = windows_execute if entry.architecture == 'windows' else ps2_execute
    results = []
    def feasible_prefix(instructions):
        if step.progress is not None:
            step.progress(dict(phase='prefix', architecture=entry.architecture,
                               instructions=len(instructions), address=instructions[-1].get('original_address', instructions[-1]['address'])))
        # Prune only after the architecture executor has established UNSAT.
        # It may encounter a prefix without a return; cutoff requests an exact
        # state snapshot without treating that prefix as a completed invocation.
        prefix = dict(graph, instructions=instructions + [dict(address='cutoff', ops=[])],
                      operation_count=sum(len(i['ops']) for i in instructions))
        outcome = execute(prefix, argument, entry.memory, state_in=entry, return_state=True,
                          path_mode=True, precondition=domain, **options)
        solver = z3.Solver()
        solver.set(timeout=10000)
        solver.add(domain, outcome.state.condition)
        answer = solver.check()
        if answer == z3.unknown:
            raise Unsupported('Transition prefix feasibility inconclusive')
        return answer != z3.unsat
    for path in expand(graph, max_paths=step.max_paths, max_instructions=step.max_instructions,
                       loop_visits=step.loop_visits, prefix_feasible=feasible_prefix):
        trace, stores = [], []
        result = execute(path, argument, entry.memory, state_in=entry, return_state=True,
                         path_mode=True, precondition=domain, guard_trace=trace, store_trace=stores, **options)
        solver = z3.Solver()
        solver.set(timeout=10000)
        solver.add(domain, result.state.condition)
        feasible = solver.check()
        if feasible == z3.unsat:
            continue
        if feasible != z3.sat:
            raise Unsupported('Transition path feasibility inconclusive')
        if path.get('cutoff') or not result.returned or result.obligations:
            raise Unsupported('Reachable transition frontier or cutoff')
        results.append(result)
        if step.progress is not None:
            step.progress(dict(phase='return', architecture=entry.architecture,
                               instructions=len(path['instructions']), returning_paths=len(results)))
    prove(z3.Or(*[r.state.condition for r in results]), z3.And(domain, entry.condition),
          'Transition returning-path coverage not proved')
    return results


def compare(steps, windows, ps2, relation, domain, *, symbols=(), frame_by_writes=False):
    """Check each stage against its own exact predecessor state.

    Tolerance is an observation relation, never a constraint on later inputs.
    All feasible paired outcomes are retained, so numeric differences can still
    trigger a later branch/state mismatch.
    """
    relation.require_satisfiable(z3.And(domain, windows.condition, ps2.condition,
                                      relation.condition(windows.memory, ps2.memory)))
    prove(relation.condition(windows.memory, ps2.memory),
          z3.And(domain, windows.condition, ps2.condition), 'Initial memory relation not established')
    pairs = [(windows, ps2)]
    reports = []
    if not steps:
        raise ValueError('Transition sequence cannot be empty')
    for step in steps:
        if relation.numeric_policies:
            for side, graph, index, auxiliary in [
                    ('Windows', step.windows, 0, step.windows_auxiliary_decisions),
                    ('PS2', step.ps2, 1, step.ps2_auxiliary_decisions)]:
                branches = {i.get('original_address', i['address']) for i in graph['instructions']
                            if i.get('text') not in ('checked virtual dispatch', 'checked switch dispatch')
                            and any(op['opcode'] == 'CBRANCH' for op in i['ops'])}
                classified = {d.windows if index == 0 else d.ps2 for d in step.decisions} | set(auxiliary)
                if branches != classified:
                    raise Unsupported(side + ' tolerant transition requires complete branch classification')
        following = []
        for previous_w, previous_s in pairs:
            entry_w, entry_s = invocation(previous_w), invocation(previous_s)
            paired = z3.And(domain, entry_w.condition, entry_s.condition)
            if step.precondition is not None:
                prove(step.precondition(entry_w, entry_s), paired,
                      'Successor precondition not established: ' + step.name)
            ws = outcomes(step.windows, entry_w, step.argument, paired, step.windows_options, step)
            ss = outcomes(step.ps2, entry_s, step.argument, paired, step.ps2_options, step)
            for w in ws:
                for s in ss:
                    feasible = z3.And(paired, w.state.condition, s.state.condition)
                    solver = z3.Solver()
                    solver.set(timeout=30000)
                    solver.add(feasible)
                    answer = solver.check()
                    if answer == z3.unsat:
                        continue
                    if answer != z3.sat:
                        raise Unsupported('Transition pair feasibility inconclusive')
                    branch_equal = all(
                        tuple(taken for address, taken in w.guards if address == decision.windows) ==
                        tuple(taken != decision.inverted for address, taken in s.guards if address == decision.ps2)
                        for decision in step.decisions)
                    return_equal = z3.BoolVal(True)
                    if step.return_bits:
                        if (w.return_value is None or s.return_value is None or step.return_bits < 1
                                or step.return_bits > min(w.return_value.size(), s.return_value.size())):
                            raise Unsupported('Invalid transition return observation')
                        return_equal = z3.Extract(step.return_bits - 1, 0, w.return_value) == z3.Extract(
                            step.return_bits - 1, 0, s.return_value)
                    frame_w = (relation.writes_within_frame('windows', w.writes) if frame_by_writes else
                               relation.unchanged('windows', w.baseline, w.state.memory, writes=w.writes))
                    frame_s = (relation.writes_within_frame('ps2', s.writes) if frame_by_writes else
                               relation.unchanged('ps2', s.baseline, s.state.memory, writes=s.writes))
                    changes = z3.simplify(z3.Or(z3.Not(relation.condition(w.state.memory, s.state.memory, outputs=True)),
                        z3.Not(frame_w), z3.Not(frame_s),
                        z3.BoolVal(not branch_equal), z3.Not(return_equal)))
                    solver.add(changes)
                    answer = solver.check()
                    if answer == z3.unknown:
                        return dict(result='inconclusive', step=step.name, reason=solver.reason_unknown())
                    if answer == z3.sat:
                        model = solver.model()
                        return dict(result='mismatch', step=step.name,
                            counterexample={name: str(model.eval(value, model_completion=True)) for name, value in symbols},
                            windows_guards=w.guards, ps2_guards=s.guards)
                    following.append((w.state, s.state))
                    if len(following) > step.max_pairs:
                        raise Unsupported('Transition paired-outcome limit exceeded')
        if not following:
            raise Unsupported('Empty transition stage: ' + step.name)
        reports.append(dict(name=step.name, paired_outcomes=len(following)))
        pairs = following
    references = sorted({model for w, s in pairs for state in (w, s) for model in state.processor.get('reference_models', ())})
    return dict(result=('conditional-equivalent-with-tolerance' if relation.numeric_policies else 'conditional-equivalent')
                if references else 'equivalent-with-tolerance' if relation.numeric_policies else 'equivalent',
                reference_models=references,
                steps=reports, numeric_policies={name: dict(absolute=p.absolute, relative=p.relative)
                    for name, p in relation.numeric_policies.values()})
