"""Context-specific expansion of approved direct calls and tail transfers."""
import copy
from verify import Unsupported


def link(exports, root, max_depth=4):
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
            i['address'] = hex(mapping[original_address])[2:]
            instructions.append(i)
            for opindex, op in enumerate(list(i['ops'])):
                code = op['opcode']
                if code in ('PATH_GUARD', 'CHECK_TARGET'):
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
                        raise Unsupported('Unapproved graph transfer to ' + hex(address))
                elif code in ('CALLIND', 'BRANCHIND'):
                    raise Unsupported('Unresolved indirect graph transfer')
                elif code == 'RETURN' and resume is not None:
                    i['ops'][opindex:] = [dict(opcode='CHECK_TARGET', inputs=op['inputs'], expected=expected),
                                          dict(opcode='BRANCH', inputs=[ram(resume)])]
        return mapping[entry]

    entry = instantiate(root, [])
    # Fallthrough follows original instruction order within each body, not the
    # order recursive instantiation happened to append bodies.
    instructions.sort(key=lambda i: int(i['address'], 16))
    return dict(reference, entry=hex(entry)[2:], instructions=instructions,
                operation_count=sum(len(i['ops']) for i in instructions))
