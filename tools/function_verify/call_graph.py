"""Context-specific expansion of approved direct calls and tail transfers."""
import copy
from verify import Unsupported


def link(exports, root, max_depth=4, indirect_targets=None, defer_unresolved=False):
    # A finite target set is a checked obligation, never a dispatch assumption.
    # This is an obligation, not an assumption: execution proves the loaded
    # address equals this target before entering its body.
    indirect_targets = dict(indirect_targets or {})
    used_targets = set()
    bodies = {int(e['entry'], 16): e for e in exports}
    if len(bodies) != len(exports) or root not in bodies:
        raise Unsupported('Ambiguous or missing graph entry')
    reference = bodies[root]
    for body in exports:
        if (body['language'] != reference['language'] or not body.get('complete') or
                body['operation_count'] != sum(len(i['ops']) for i in body['instructions'])):
            raise Unsupported('Incomplete/incompatible graph body')
    if len({e['executable_sha256'] for e in exports}) != 1:
        raise Unsupported('Mixed executable identities in graph')
    instructions = []
    serial = 0x10000000

    def ram(address):
        return dict(space='ram', offset=hex(address)[2:], size=8 if reference['language'].startswith('x86') else 4, constant=False)

    def instantiate(entry, stack, resume=None, expected=None):
        nonlocal serial
        if entry in stack or len(stack) >= max_depth:
            raise Unsupported('Recursive call graph or depth limit exceeded')
        body = copy.deepcopy(bodies[entry])
        mapping = {}
        for i in body['instructions']:
            address = int(i['address'], 16)
            if address in mapping:
                raise Unsupported('Duplicate graph instruction address')
            mapping[address] = serial
            serial += 16
        if entry not in mapping:
            raise Unsupported('Graph entry lacks instruction')
        for index, i in enumerate(body['instructions']):
            original_address = int(i['address'], 16)
            i['original_address'] = i.get('original_address', i['address'])
            i['call_context'] = [hex(a)[2:] for a in stack + [entry]]
            i['address'] = hex(mapping[original_address])[2:]
            instructions.append(i)
            for opindex, op in enumerate(list(i['ops'])):
                code = op['opcode']
                if code in ('PATH_GUARD', 'CHECK_TARGET', 'CHECK_DISPATCH', 'UNRESOLVED_TRANSFER'):
                    raise Unsupported('Synthetic opcode in raw call graph')
                if code in ('CALL', 'BRANCH', 'CBRANCH'):
                    target = op['inputs'][0]
                    if target['space'] != 'ram' or target['constant']:
                        raise Unsupported('Internal P-code graph transfer unsupported')
                    address = int(target['offset'], 16)
                    if address in mapping and code != 'CALL':
                        op['inputs'][0] = ram(mapping[address])
                    elif address in bodies and code in ('CALL', 'BRANCH'):
                        if code == 'CALL':
                            if index + 1 >= len(body['instructions']):
                                raise Unsupported('Call has no continuation')
                            next_address = int(body['instructions'][index + 1]['address'], 16)
                            child = instantiate(address, stack + [entry], mapping[next_address], next_address)
                        else:
                            child = instantiate(address, stack + [entry], resume, expected)
                        op['opcode'], op['inputs'] = 'BRANCH', [ram(child)]
                    else:
                        if defer_unresolved and code in ('CALL', 'BRANCH'):
                            op.update(opcode='UNRESOLVED_TRANSFER', reason='Unapproved graph transfer to ' + hex(address))
                            i['ops'][opindex:] = [op]
                            break
                        else:
                            raise Unsupported('Unapproved graph transfer to ' + hex(address))
                elif code in ('CALLIND', 'BRANCHIND'):
                    target = indirect_targets.get(original_address)
                    if target is None:
                        if defer_unresolved:
                            op.update(opcode='UNRESOLVED_TRANSFER', reason='Unresolved indirect graph transfer at ' + hex(original_address))
                            i['ops'][opindex:] = [op]
                            break
                        raise Unsupported('Unresolved indirect graph transfer at ' + hex(original_address))
                    targets = [target] if isinstance(target, int) else list(target) if isinstance(target, (list, tuple)) else []
                    internal = bool(targets) and code == 'BRANCHIND' and all(type(t) is int and t in mapping for t in targets)
                    if (not targets or any(type(t) is not int or (t not in bodies and not internal) for t in targets)
                            or len(set(targets)) != len(targets) or len(op['inputs']) != 1):
                        raise Unsupported('Invalid or missing indirect target body')
                    if opindex != len(i['ops']) - 1:
                        raise Unsupported('Indirect transfer must end instruction')
                    used_targets.add(original_address)
                    if code == 'CALLIND':
                        if index + 1 >= len(body['instructions']):
                            raise Unsupported('Indirect call has no continuation')
                        next_address = int(body['instructions'][index + 1]['address'], 16)
                        continuation, return_target = mapping[next_address], next_address
                    else:
                        continuation, return_target = resume, expected
                    def destination(candidate):
                        return mapping[candidate] if internal else instantiate(candidate, stack + [entry], continuation, return_target)
                    if internal and len(targets) > 1:
                        # Membership remains checked before the tree. Unsigned
                        # comparisons select exactly one of the audited targets
                        # in logarithmic depth, rather than replaying a linear
                        # chain for large compiler-generated switch tables.
                        def switch_tree(candidates):
                            nonlocal serial
                            node = serial
                            serial += 16
                            if len(candidates) == 1:
                                operations = [dict(opcode='BRANCH', inputs=[ram(destination(candidates[0]))])]
                            else:
                                middle = len(candidates) // 2
                                left = switch_tree(candidates[:middle])
                                right = switch_tree(candidates[middle:])
                                temporary = dict(space='unique', offset='7fffff00', size=1, constant=False)
                                literal = dict(space='const', offset=hex(candidates[middle])[2:],
                                               size=op['inputs'][0]['size'], constant=True)
                                operations = [dict(opcode='INT_LESS', inputs=[op['inputs'][0], literal], output=temporary),
                                              dict(opcode='CBRANCH', inputs=[ram(left), temporary]),
                                              dict(opcode='BRANCH', inputs=[ram(right)])]
                            instructions.append(dict(address=hex(node)[2:], original_address=i['original_address'],
                                call_context=i['call_context'], text='checked switch dispatch', ops=operations))
                            return node
                        decision = switch_tree(sorted(targets))
                        i['ops'][opindex:] = [dict(opcode='CHECK_DISPATCH', inputs=op['inputs'], expected=targets),
                                              dict(opcode='BRANCH', inputs=[ram(decision)])]
                    elif len(targets) == 1:
                        child = destination(targets[0])
                        i['ops'][opindex:] = [dict(opcode='CHECK_DISPATCH', inputs=op['inputs'], expected=targets[0]),
                                              dict(opcode='BRANCH', inputs=[ram(child)])]
                    else:
                        # Reserve contiguous decision nodes before instantiating
                        # children, so untaken CBRANCH fallthrough stays local.
                        decisions = list(range(serial, serial + 16 * len(targets), 16))
                        serial += 16 * len(targets)
                        i['ops'][opindex:] = [dict(opcode='CHECK_DISPATCH', inputs=op['inputs'], expected=targets),
                                              dict(opcode='BRANCH', inputs=[ram(decisions[0])])]
                        for number, candidate in enumerate(targets):
                            child = destination(candidate)
                            operations = [dict(opcode='BRANCH', inputs=[ram(child)])]
                            if number < len(targets) - 1:
                                temporary = dict(space='unique', offset='7fffff00', size=1, constant=False)
                                literal = dict(space='const', offset=hex(candidate)[2:],
                                               size=op['inputs'][0]['size'], constant=True)
                                operations = [dict(opcode='INT_EQUAL', inputs=[op['inputs'][0], literal], output=temporary),
                                              dict(opcode='CBRANCH', inputs=[ram(child), temporary])]
                            instructions.append(dict(address=hex(decisions[number])[2:],
                                original_address=i['original_address'], call_context=i['call_context'],
                                text='checked switch dispatch' if internal else 'checked virtual dispatch', ops=operations))
                elif code == 'RETURN' and resume is not None:
                    i['ops'][opindex:] = [dict(opcode='CHECK_TARGET', inputs=op['inputs'], expected=expected),
                                          dict(opcode='BRANCH', inputs=[ram(resume)])]
        return mapping[entry]

    entry = instantiate(root, [])
    if used_targets != set(indirect_targets):
        raise Unsupported('Unused indirect target mapping')
    # Fallthrough follows original instruction order within each body, not the
    # order recursive instantiation happened to append bodies.
    instructions.sort(key=lambda i: int(i['address'], 16))
    return dict(reference, entry=hex(entry)[2:], instructions=instructions,
                operation_count=sum(len(i['ops']) for i in instructions))
