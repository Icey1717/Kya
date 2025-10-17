# Demangler / Itanium remangler for the CodeWarrior ABI
# Updated for Python 3

import argparse
import re
import sys
from typing import Tuple, Optional, List, Literal

mode: Optional[str] = None
verbose: bool = False

def is_demangle() -> bool:
    return mode == 'demangle'

names_mapping = {
    'v': ('void', 'v'),
    'b': ('bool', 'b'),
    'c': ('char', 'c'),
    's': ('short', 's'),
    'i': ('int', 'i'),
    'l': ('long', 'l'),
    'x': ('long long', 'x'),
    'Sc': ('signed char', 'a'),
    'Uc': ('unsigned char', 'h'),
    'Us': ('unsigned short', 't'),
    'Ui': ('unsigned int', 'j'),
    'Ul': ('unsigned long', 'm'),
    'Ux': ('unsigned long long', 'y'),
    'f': ('float', 'f'),
    'd': ('double', 'd'),
    'r': ('long double', 'e'),
    'w': ('wchar_t', 'w'),
    'e': ('...', 'z')
}

method_mapping = {
    '__dt':  ('~$CLS$', 'D0'),
    '__ct':  ('$CLS$', 'C1'),
    '__nw':  ('operator new', 'nw'),
    '__nwa': ('operator new[]', 'na'),
    '__dl':  ('operator delete', 'dl'),
    '__dla': ('operator delete[]', 'da'),
    '__pl':  ('operator+', 'pl'),
    '__mi':  ('operator-', 'mi'),
    '__ml':  ('operator*', 'ml'),
    '__dv':  ('operator/', 'dv'),
    '__md':  ('operator%', 'rm'),
    '__er':  ('operator^', 'eo'),
    '__ad':  ('operator&', 'an'),
    '__or':  ('operator|', 'or'),
    '__co':  ('operator~', 'co'),
    '__nt':  ('operator!', 'nt'),
    '__as':  ('operator=', 'aS'),
    '__lt':  ('operator<', 'lt'),
    '__gt':  ('operator>', 'gt'),
    '__apl': ('operator+=', 'pL'),
    '__ami': ('operator-=', 'mI'),
    '__amu': ('operator*=', 'mL'),
    '__adv': ('operator/=', 'dV'),
    '__amd': ('operator%=', 'rM'),
    '__aer': ('operator^=', 'eO'),
    '__aad': ('operator&=', 'aN'),
    '__aor': ('operator|=', 'oR'),
    '__ls':  ('operator<<', 'ls'),
    '__rs':  ('operator>>', 'rs'),
    '__ars': ('operator>>=', 'rS'),
    '__als': ('operator<<=', 'lS'),
    '__eq':  ('operator==', 'eq'),
    '__ne':  ('operator!=', 'ne'),
    '__le':  ('operator<=', 'le'),
    '__ge':  ('operator>=', 'ge'),
    '__aa':  ('operator&&', 'aa'),
    '__oo':  ('operator||', 'oo'),
    '__pp':  ('operator++', 'pp'),
    '__mm':  ('operator--', 'mm'),
    '__cm':  ('operator,', 'cm'),
    '__rm':  ('operator->*', 'pm'),
    '__rf':  ('operator->', 'pt'),
    '__cl':  ('operator()', 'cl'),
    '__vc':  ('operator[]', 'ix'),
}

def parse_number(s: str, i: int) -> Tuple[int, int]:
    num = 0
    while i < len(s) and s[i].isdigit():
        num = num * 10 + int(s[i])
        i += 1
    return num, i

def parse_typename(s: str, i: int) -> Tuple[str, int]:
    if s[i] == 'Q':
        count = int(s[i + 1])
        i += 2
        bits = []
        for _ in range(count):
            size, i = parse_number(s, i)
            bits.append(resolve_templates(s[i:(i + size)], True))
            i += size
        return '::'.join(bits) if is_demangle() else ''.join(bits), i
    else:
        size, i = parse_number(s, i)
        return resolve_templates(s[i:(i + size)], True), i + size

def join_modifiers(modifiers: List[str]) -> str:
    return ''.join(modifiers[::-1] if is_demangle() else modifiers)

def parse_function(s: str, i: int, modifiers: list[str], name='', rettype_mode: Literal['show', 'hide_in_demangle', 'remove'] = 'show') -> Tuple[str, int]:
    """
    Parses a function from a demangled string.
    Examples:
        (demangle) parse_function('v_v', 0, ['*']) -> ('void (*) ()', 4)
        (demangle) parse_function('s_b', 0, ['&']) -> ('bool (&) (short)', 4)
        (remangle) parse_function('i_v', 0, ['*']) -> ('FviE', 4)
    Args:
        s (str): The demangled string.
        i (int): The current index in the string.
        modifiers (list[str]): The list of modifiers.
        name (str): An identifier, if available. This is the "main" symbol name.
        rettype_mode (Literal['show', 'hide_in_demangle', 'remove']): How to handle the return type.
    Returns:
        Tuple[str, int]: The transformed function signature and the new position in the string.
    """
    # Parse the function args, return type handled later
    args = []
    while i < len(s) and s[i] != '_' and s[i] != '@':
        argtype, i = parse_type(s, i)
        args.append(argtype)

    # Special case: const
    # Note that if the function is const, it will be the last modifier
    # because e.g. CPFv is a (const pointer) to a function
    const_str = ''
    if len(modifiers) > 0 and (modifiers[-1] == ' const' or modifiers[-1] == 'K'):
        const_str = ' const' if is_demangle() else 'K'
        modifiers.pop()

    mod_str = join_modifiers(modifiers)

    if is_demangle():
        if mod_str != '':
            mod_str = f'({mod_str.strip()})'
        arg_str = ', '.join(args) if args[0] != 'void' else ''
        func_str = f'{name}{mod_str}({arg_str}){const_str}'
        if i >= len(s) or s[i] == '@':
            return func_str, i
        if rettype_mode == 'hide_in_demangle' or rettype_mode == 'remove':
            _, i = parse_type(s, i + 1)
            return func_str, i
        else:
            return parse_type(s, i + 1, [' ' + func_str])
    else:
        if i < len(s) and s[i] != '@':
            rettype, i = parse_type(s, i + 1)
        else:
            rettype, i = ('', i)
        if rettype_mode == 'remove':
            rettype = ''
        func_encoding = f'{rettype}{"".join(args)}' if name != '' else f'F{rettype}{"".join(args)}E'
        if name != '':
            func_encoding = f'N{const_str}{name}E{func_encoding}'
        else:
            func_encoding = f'{const_str}{func_encoding}'
        return mod_str + func_encoding, i


def parse_type(
    s: str, i: int,
    modifiers: Optional[List[str]] = None,
    name: str = '',
    rettype_mode: Literal['show', 'hide_in_demangle', 'remove'] = 'show'
) -> Tuple[str, int]:
    if modifiers is None:
        modifiers = []

    type_modifier = ''
    while i < len(s) and s[i].isupper() and s[i] != 'Q':
        c = s[i]
        if c == 'C':
            modifiers.append(' const' if is_demangle() else 'K')
        elif c == 'P':
            modifiers.append('*' if is_demangle() else 'P')
        elif c == 'R':
            modifiers.append('&' if is_demangle() else 'R')
        elif c == 'O':
            modifiers.append('&&' if is_demangle() else 'O')
        elif c == 'V':
            modifiers.append(' volatile' if is_demangle() else 'V')
        elif c in ('U', 'S'):
            type_modifier = c
        elif c == 'F':
            return parse_function(s, i + 1, modifiers, name, rettype_mode)
        elif c == 'M':
            class_name, i = parse_type(s, i + 1)
            modifiers.append(f' {class_name}::*' if is_demangle() else f'M{class_name}')
            if i < len(s) and s[i] == 'F':
                if s[i:].startswith('FPCvPCv'):
                    modifiers.append(' const' if is_demangle() else 'K')
                    i += 7
                elif s[i:].startswith('FPCvPv'):
                    i += 6
                if s[i] == '_':
                    i -= 1
                return parse_function(s, i, modifiers)
        elif c == 'A':
            count, i = parse_number(s, i + 1)
            if is_demangle():
                modstr = join_modifiers(modifiers)
                if re.search(r'\[.*\]$', modstr):
                    modifiers.insert(0, f'[{count}]')
                elif modstr == '':
                    modifiers.insert(0, f' [{count}]')
                else:
                    modifiers = [f' ({modstr}) [{count}]']
            else:
                modifiers.append(f'A{count}_')
        else:
            raise Exception(f'Invalid type modifier "{c}"')
        i += 1

    if i >= len(s):
        raise Exception('Unexpected end of string in parse_type')

    if s[i] == 'Q' or s[i].isdigit():
        type_name, i = parse_typename(s, i)
        if not is_demangle():
            type_name = f'N{type_name}E'
    else:
        actual_type = type_modifier + s[i]
        if actual_type not in names_mapping:
            raise Exception(f'Invalid type "{actual_type}"')
        type_name = names_mapping[actual_type][0 if is_demangle() else 1]
        i += 1

    mod_str = join_modifiers(modifiers)
    return (f'{type_name}{mod_str}' if is_demangle() else f'{mod_str}{type_name}'), i

def resolve_templates(s: str, remangle_add_length: bool) -> str:
    """
    Resolves template types in a type string.
    Examples:
        (demangle) resolve_templates('std<c>', False) -> 'std<char>'
        (remangle) resolve_templates('std<c>', False) -> 'stdIcE'
        (remangle) resolve_templates('std<c>', True) -> '3stdIcE'
    Args:
        s (str): The string to resolve.
        remangle_add_length (bool): Whether to add the length prefix in remangling.
    Returns:
        str: The resolved string.
    """
    begin_pos = s.find('<')
    if begin_pos == -1:
        if re.match(r'^@unnamed@.+@$', s):
            return '(anonymous namespace)' if is_demangle() else '12_GLOBAL__N_1'
        unnamed_type_m = re.match(r'^@class\$(\d*).+$', s)
        if unnamed_type_m:
            typenum = int(unnamed_type_m.group(1)) if unnamed_type_m.group(1) else -1
            return f'{{unnamed type#{typenum + 2}}}' if is_demangle() else f'Ut{typenum if typenum > -1 else ""}_'
        if not is_demangle() and remangle_add_length:
            return f'{len(s)}{s}'
        return s

    template_str = ''
    i = begin_pos + 1
    while i < len(s):
        if s[i] == ',':
            if is_demangle():
                template_str += ', '
            i += 1
            continue
        if s[i] == '>':
            break
        elif re.match(r'^-?\d+', s[i:]):
            # Integer literal
            literal_match = re.match(r'^-?\d+', s[i:])
            literal = literal_match.group(0)
            template_str += literal if is_demangle() else f'XLi{literal.replace("-", "n")}EE'
            i += len(literal)
        else:
            type_name, i = parse_type(s, i)
            template_str += type_name

    if is_demangle():
        template_str = f'<{template_str}>'
        # replicate c++filt behavior
        if template_str[-2:] == '>>':
            template_str = template_str[:-1] + ' >'
        return s[:begin_pos] + template_str
    else:
        return (f'{len(s[:begin_pos])}{s[:begin_pos]}I{template_str}E') if remangle_add_length else s[:begin_pos] + f'I{template_str}E'


def demangle(s: str) -> str:
    """
    Demangles a mangled symbol name.
    """
    at_sym = ''
    thunk_offsets = []

    m = re.match(r'^@([^@]+)@(.+)$', s)
    if m:
        m_thunk = re.match(r'^@(\d+)(?:@(\d+))?@(.+)$', s)
        if m_thunk:
            thunk_offsets = [int(m_thunk.group(1))]
            if m_thunk.group(2):
                thunk_offsets.append(int(m_thunk.group(2)))
            s = m_thunk.group(3)
        else:
            at_sym = m.group(1)
            if at_sym not in ['LOCAL', 'GUARD', 'STRING']:
                raise Exception(f'Invalid symbol name "{s}"')
            s = m.group(2)

    template_depth = 0
    last_possible_end = -1
    for i in range(1, len(s)):
        if s[i] == '<':
            template_depth += 1
        elif s[i] == '>':
            template_depth -= 1
        if template_depth == 0 and i + 2 < len(s) and s[i:i + 2] == '__' and s[i + 2] in 'CFQ0123456789':
            last_possible_end = i
            break

    if last_possible_end == -1:
        return s

    i = last_possible_end
    method, remainder = s[:i], s[i + 2:]

    if remainder[0] == 'F':
        class_name = ''
        i = 0
    else:
        class_name, i = parse_typename(remainder, 0)

    if '<' in method:
        template_start = method.find('<')
        pre_template, template = method[:template_start], method[template_start:]
        resolved_templates = resolve_templates(template, False)
    else:
        pre_template, resolved_templates = method, ''

    if pre_template in ['__ct', '__dt']:
        rettype_mode = 'remove'
    elif at_sym:
        rettype_mode = 'hide_in_demangle'
    else:
        rettype_mode = 'show'

    if method == '__vt':
        return f'vtable for {class_name}' if is_demangle() else f'_ZTVN{class_name}E'
    elif method.startswith('__op'):
        cv_type_name, _ = parse_type(method[4:], 0)
        pre_template = f'operator {cv_type_name}' if is_demangle() else f'cv{cv_type_name}'
        resolved_templates = ''
    elif pre_template in method_mapping:
        pre_template = method_mapping[pre_template][0 if is_demangle() else 1]
        if pre_template == 'operator<' and resolved_templates:
            pre_template = 'operator< '
        if is_demangle() and '$CLS$' in pre_template:
            class_name_no_templates = re.sub(r'<[^<>]+>', '', class_name)
            last_class_name = class_name_no_templates.split('::')[-1]
            pre_template = pre_template.replace('$CLS$', last_class_name)
    else:
        if not is_demangle():
            pre_template = f'{len(pre_template)}{pre_template}'

    method = f'{pre_template}{resolved_templates}'
    demangled = '::'.join(filter(None, [class_name, method])) if is_demangle() else class_name + method

    if i < len(remainder):
        demangled, i = parse_type(remainder, i, name=demangled, rettype_mode=rettype_mode)
    elif not is_demangle():
        demangled = f'N{demangled}E'

    cov_ret_thunk = None
    if remainder[i:].startswith('@@'):
        cov_ret_thunk = remainder[i + 2:].split('@')[0]
        i += 2 + len(cov_ret_thunk)

    # Local / guard / string handling
    local_sym_name = ''
    if i < len(remainder) and remainder[i] == '@' and at_sym in ['LOCAL', 'GUARD']:
        subs = remainder[i + 1:].split('@')
        local_sym_name = subs[0]
        local_sym_extra = ('_' + subs[1]) if len(subs) > 1 else ''
        if not is_demangle():
            local_sym_name = f'{len(local_sym_name)}{local_sym_name}{local_sym_extra}'
    elif at_sym == 'GUARD' and i >= len(remainder):
        local_sym_name = method
    elif at_sym == 'STRING' and i < len(remainder) and remainder[i] == '@' and not is_demangle():
        local_sym_name = '_' + remainder[i + 1]

    if is_demangle():
        if local_sym_name:
            demangled += f'::{local_sym_name}'
        # Remove extra spaces like c++filt does
        while True:
            m = re.search(r'\((?:[*&]|const| )+ (\w+.+)$', demangled)
            if not m or m.group(1).startswith('const'):
                break
            demangled = demangled[:m.start(1) - 1] + m.group(1)

        if thunk_offsets or cov_ret_thunk:
            thunk_type = 'virtual' if len(thunk_offsets) == 2 else 'non-virtual'
            if cov_ret_thunk:
                thunk_type = 'covariant return'
            demangled = f'{thunk_type} thunk to {demangled}'
        if at_sym == 'STRING':
            demangled += '::string literal'
        if at_sym == 'GUARD':
            demangled = f'guard variable for {demangled}'
        return demangled
    else:
        if thunk_offsets or cov_ret_thunk:
            thunk_char = 'v' if len(thunk_offsets) == 2 else 'h'
            if cov_ret_thunk:
                if not thunk_offsets:
                    thunk_offsets.append(0)
                thunk_offsets.append(f'h{hash(cov_ret_thunk) & 0xFFFF}')
                thunk_char = 'c' + thunk_char
            demangled = f"T{thunk_char}{'_'.join([str(o) for o in thunk_offsets])}_{demangled}"
        if at_sym == 'LOCAL':
            demangled = f'Z{demangled}E{local_sym_name}'
        if at_sym == 'GUARD':
            demangled = f'GVZ{demangled}E{local_sym_name}'
        if at_sym == 'STRING':
            demangled = f'Z{demangled}Es{local_sym_name}'
        return f'_Z{demangled}'


def demangle_try(s: str) -> str:
    try:
        return demangle(s)
    except Exception as e:
        sys.stderr.write('Demangler error: ' + str(e) + '\n')
        raise e

def main():
    global mode
    global verbose
    parser = argparse.ArgumentParser()
    parser.add_argument('symbol', type=str, nargs='?')
    parser.add_argument('-m', '--mode', choices=['demangle', 'remangle_itanium'], required=True)
    parser.add_argument('-v', '--verbose', action='store_true', default=False)
    args = parser.parse_args()
    mode = args.mode
    verbose = args.verbose
    if args.symbol is None:
        while True:
            sym = input()
            print(demangle_try(sym))
    else:
        print(demangle_try(args.symbol))
        return

if __name__ == '__main__':
    main()