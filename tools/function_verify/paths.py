"""Enumerate acyclic raw P-code instruction paths; reject loops and truncation."""
import copy
from verify import Unsupported


def expand(export, max_paths=32, max_instructions=128, loop_visits=None, prefix_feasible=None):
    if loop_visits is not None and not 1 <= loop_visits <= 16:
        raise Unsupported('Loop visit bound must be between 1 and 16')
    instructions = export['instructions']
    if sum(len(i['ops']) for i in instructions) != export['operation_count']:
        raise Unsupported('Inconsistent original operation count')
    addresses = [int(i['address'], 16) for i in instructions]
    if len(set(addresses)) != len(addresses) or not addresses or addresses[0] != int(export['entry'], 16):
        raise Unsupported('Invalid instruction addresses')
    indexes = {address: index for index, address in enumerate(addresses)}
    results = []
    cutoffs = []

    def walk(index, prefix, visited):
        if (prefix_feasible is not None and prefix
                and any(op['opcode'] == 'PATH_GUARD' for op in prefix[-1]['ops'])
                and prefix_feasible(prefix) is False):
            return
        if index in visited and loop_visits is None:
            raise Unsupported('Cycle detected; loops are not supported')
        if loop_visits is not None and visited.get(index, 0) >= loop_visits:
            # Keep the prefix guard, so callers must prove this frontier
            # unreachable. Never silently discard paths beyond the loop bound.
            terminal = dict(address='cutoff', ops=[dict(opcode='RETURN', inputs=[
                dict(space='register', offset='1f0', size=8, constant=False)])])
            cutoffs.append(prefix + [terminal])
            if len(results) + len(cutoffs) > max_paths:
                raise Unsupported('Path count limit exceeded')
            return
        if len(prefix) >= max_instructions or index >= len(instructions):
            raise Unsupported('Path limit or fallthrough outside function')
        instruction = copy.deepcopy(instructions[index])
        ops = instruction['ops']
        for op in ops:
            if op['opcode'] == 'PATH_GUARD':
                raise Unsupported('Synthetic opcode in raw export')
        controls = [i for i, op in enumerate(ops) if op['opcode'] in ('BRANCH', 'CBRANCH', 'RETURN', 'UNRESOLVED_TRANSFER')]
        two_targets = (len(controls) == 2 and controls == [len(ops) - 2, len(ops) - 1]
                       and [ops[c]['opcode'] for c in controls] == ['CBRANCH', 'BRANCH'])
        if controls and not two_targets and (len(controls) != 1 or
                         (controls[0] != len(ops) - 1 and ops[controls[0]]['opcode'] != 'CBRANCH')):
            raise Unsupported('Control transfer must end instruction; internal P-code flow unsupported')
        if controls and ops[-1]['opcode'] in ('RETURN', 'UNRESOLVED_TRANSFER'):
            results.append(prefix + [instruction])
            if len(results) + len(cutoffs) > max_paths:
                raise Unsupported('Path count limit exceeded')
            return
        visited = dict(visited)
        visited[index] = visited.get(index, 0) + 1
        if controls:
            branch_index = controls[0]
            branch = ops[branch_index]
            suffix = ops[branch_index + 1:]
            del ops[branch_index:]
            target = branch['inputs'][0]
            if target['space'] != 'ram' or target['constant'] or int(target['offset'], 16) not in indexes:
                raise Unsupported('Branch target outside function or internal P-code branch')
            destination = indexes[int(target['offset'], 16)]
            if branch['opcode'] == 'BRANCH':
                walk(destination, prefix + [instruction], visited)
            else:
                if len(branch['inputs']) != 2:
                    raise Unsupported('Invalid conditional branch')
                fallthrough = index + 1
                if two_targets:
                    alternative = suffix[0]['inputs'][0]
                    if (alternative['space'] != 'ram' or alternative['constant']
                            or int(alternative['offset'], 16) not in indexes):
                        raise Unsupported('Invalid alternate conditional target')
                    fallthrough = indexes[int(alternative['offset'], 16)]
                    suffix = []
                for taken, next_index in [(True, destination), (False, fallthrough)]:
                    selected = copy.deepcopy(instruction)
                    selected['ops'].append(dict(opcode='PATH_GUARD', inputs=[branch['inputs'][1]], taken=taken))
                    if not taken:
                        # Conditional moves can branch to the next machine
                        # instruction before their COPY. Keep the untaken suffix
                        # in this instruction so its unique temporaries survive.
                        selected['ops'].extend(copy.deepcopy(suffix))
                    walk(next_index, prefix + [selected], visited)
        else:
            walk(index + 1, prefix + [instruction], visited)

    walk(0, [], {})
    exports = []
    for path in results:
        item = dict(export, instructions=path, operation_count=sum(len(i['ops']) for i in path))
        exports.append(item)
    for path in cutoffs:
        exports.append(dict(export, instructions=path, cutoff=True,
                            operation_count=sum(len(i['ops']) for i in path)))
    return exports
