"""Record instruction-level semantic obligations without treating an audit as proof."""
import re


def inventory(exports):
    floating, dispatch, controls, userops = [], [], [], []
    for export in exports:
        for instruction in export['instructions']:
            text = instruction.get('text', '')
            common = dict(function=export['name'], symbol=export.get('coff', {}).get('symbol'),
                          entry=export['entry'], address=instruction['address'], text=text)
            operations = [o['opcode'] for o in instruction['ops']]
            floats = [o for o in operations if o.startswith('FLOAT_')]
            if floats:
                from processor_fp import numeric_source
                from verify import Unsupported
                try:
                    numeric = numeric_source(instruction, 'ps2' if export['language'].startswith('r5900') else 'windows')
                    mnemonic = numeric.get('mnemonic', numeric.get('text', '').split(' ')[0]).upper()
                except Unsupported:
                    mnemonic = ''
                reference = mnemonic in ('ADD.S', 'SUB.S', 'MUL.S', 'DIV.S', 'SQRT.S', 'ABS.S', 'NEG.S')
                scalar = reference or mnemonic in ('ADDSS', 'SUBSS', 'MULSS', 'DIVSS', 'COMISS', 'UCOMISS', 'C.EQ.S', 'C.LT.S', 'C.LE.S')
                floating.append(dict(common, opcodes=floats,
                    result='requires-execution-proof' if scalar and instruction.get('bytes') else 'unsupported',
                    conditional_profile_required=reference,
                    obligation='Validate instruction bytes and p-code, explicit processor control state and operand-domain obligations; EE arithmetic requires explicit pinned reference opt-in, and general VU arithmetic remains unsupported'))
            if any(o in ('CALLIND', 'BRANCHIND') for o in operations):
                dispatch.append(common)
            if re.search(r'\b(?:LDMXCSR|STMXCSR|CFC1|CTC1|CFC2|CTC2)\b', text, re.I):
                controls.append(common)
            if 'CALLOTHER' in operations:
                userops.append(dict(common, inputs=[o['inputs'] for o in instruction['ops'] if o['opcode'] == 'CALLOTHER']))
    return dict(scope='Syntactic overapproximation of exported bodies; no feasibility or floating-point equivalence claimed',
                floating_point=floating, indirect_transfers=dispatch, processor_control=controls, user_operations=userops,
                sources=[dict(function=e['name'], entry=e['entry'], executable_sha256=e['executable_sha256'],
                              complete=e['complete']) for e in exports])
