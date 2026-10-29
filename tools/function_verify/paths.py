"""Enumerate acyclic raw P-code instruction paths; reject loops and truncation."""
import copy
from verify import Unsupported


def expand(export, max_paths=32, max_instructions=128, loop_visits=None):
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
        controls = [i for i, op in enumerate(ops) if op['opcode'] in ('BRANCH', 'CBRANCH', 'RETURN')]
        if controls and controls != [len(ops) - 1]:
            raise Unsupported('Control transfer must end instruction; internal P-code flow unsupported')
        if controls and ops[-1]['opcode'] == 'RETURN':
            results.append(prefix + [instruction])
            if len(results) + len(cutoffs) > max_paths:
                raise Unsupported('Path count limit exceeded')
            return
        visited = dict(visited)
        visited[index] = visited.get(index, 0) + 1
        if controls:
            branch = ops.pop()
            target = branch['inputs'][0]
            if target['space'] != 'ram' or target['constant'] or int(target['offset'], 16) not in indexes:
                raise Unsupported('Branch target outside function or internal P-code branch')
            destination = indexes[int(target['offset'], 16)]
            if branch['opcode'] == 'BRANCH':
                walk(destination, prefix + [instruction], visited)
            else:
                if len(branch['inputs']) != 2:
                    raise Unsupported('Invalid conditional branch')
                for taken, next_index in [(True, destination), (False, index + 1)]:
                    selected = copy.deepcopy(instruction)
                    selected['ops'].append(dict(opcode='PATH_GUARD', inputs=[branch['inputs'][1]], taken=taken))
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
