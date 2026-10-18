"""Recover Eden vector assignments using clipboard and repository declarations."""

import re


def NormalizePath(expression):
    expression = re.sub(r"\s*(->|\.)\s*", r"\1", expression.strip())
    path = r"\w+(?:(?:->|\.)\w+)*"
    previous = None
    while previous != expression:
        previous = expression
        expression = re.sub(r"\((" + path + r")\)", r"\1", expression)
    return expression if re.fullmatch(path, expression) else None


class VectorTypes:
    def __init__(self, text, index, thisType=None, parameters=""):
        self.index = index
        self.locals = {}
        declaration = (
            r"(?:const\s+)?(?P<type>\w+(?:::\w+)*)"
            r"(?:\s*(?P<pointer>\*)\s*|\s+)(?P<name>\w+)"
        )
        for match in re.finditer(r"^[ \t]*" + declaration + r"\s*(?:;|=)", text, re.MULTILINE):
            self.Add(match)
        for parameter in parameters.split(','):
            match = re.fullmatch(r"\s*" + declaration + r"\s*", parameter)
            if match:
                self.Add(match)
        if thisType:
            self.locals['this'] = [[thisType, True]]

    def Add(self, match):
        if match['type'] in {'return', 'throw', 'goto', 'delete', 'case', 'co_return'}:
            return
        self.locals.setdefault(match['name'], []).append([match['type'], bool(match['pointer'])])

    def Member(self, owner, name, seen=None):
        seen = set() if seen is None else seen
        if owner in seen:
            return []
        seen = seen | {owner}
        direct = self.index.get('members', {}).get(owner + '::' + name)
        if direct is not None:
            return direct
        inherited = []
        for base in self.index.get('bases', {}).get(owner, []):
            namespace = owner.rpartition('::')[0]
            qualified = namespace + '::' + base if namespace else base
            if namespace and any(key.startswith(qualified + '::')
                                 for key in self.index.get('members', {})):
                base = qualified
            inherited.extend(self.Member(base, name, seen))
        return inherited

    def Resolve(self, expression):
        path = NormalizePath(expression)
        if path is None:
            return None
        parts = re.split(r"(->|\.)", path)
        values = self.locals.get(parts[0], self.index.get('symbols', {}).get(parts[0], []))
        for offset in range(1, len(parts), 2):
            if not values or any(value != values[0] for value in values):
                return None
            typeName, pointer = values[0]
            if pointer != (parts[offset] == '->'):
                return None
            member = parts[offset + 1]
            if typeName == 'edF32VECTOR4' and member == 'xyz':
                values = [['edF32VECTOR3', False]]
            else:
                values = self.Member(typeName, member)
        if values and all(value == values[0] for value in values):
            return values[0]
        return None

    def View(self, expression, components):
        value = self.Resolve(expression)
        if value == ['edF32VECTOR4', False]:
            return expression + '.xyz' if components == 'xyz' else expression
        if value == ['edF32VECTOR3', False] and components == 'xyz':
            return expression
        return None


def CollapseBlock(text, index, thisType=None, parameters=""):
    types = VectorTypes(text, index, thisType, parameters)
    lines = text.splitlines(keepends=True)
    assignment = re.compile(
        r"^(?P<indent>[ \t]*)(?P<lhs>.+?)\.(?P<component>[xyzw])\s*=\s*"
        r"(?P<rhs>[^;\r\n]+);[ \t]*(?P<eol>\r?\n)?$"
    )

    def Parse(line):
        match = assignment.fullmatch(line)
        if match is None:
            return None
        rhs = re.fullmatch(r"(.+)\." + match['component'] + r"(?:\s*\*\s*(\w+|\d+\.\d*f?))?", match['rhs'])
        if rhs is None:
            return None
        lhsPath, rhsPath = NormalizePath(match['lhs']), NormalizePath(rhs[1])
        if lhsPath is None or rhsPath is None:
            return None
        scalar = rhs[2]
        if scalar and types.Resolve(scalar) != ['float', False] and not re.fullmatch(r"\d+\.\d*f", scalar):
            return None
        return match, lhsPath, rhsPath, scalar

    out = []
    pos = 0
    while pos < len(lines):
        first = Parse(lines[pos])
        collapsed = False
        if first and first[0]['component'] == 'x':
            for components in ('xyzw', 'xyz'):
                if pos + len(components) > len(lines):
                    continue
                group = [Parse(line) for line in lines[pos:pos + len(components)]]
                if any(item is None or item[0]['component'] != component
                       or item[0]['indent'] != first[0]['indent'] or item[1:] != first[1:]
                       for item, component in zip(group, components)):
                    continue
                lhs, rhs = types.View(first[1], components), types.View(first[2], components)
                if lhs is None or rhs is None:
                    continue
                scalar = ' * ' + first[3] if first[3] else ''
                out.append(first[0]['indent'] + lhs + ' = ' + rhs + scalar + ';' + (group[-1][0]['eol'] or ''))
                pos += len(components)
                collapsed = True
                break
        if not collapsed:
            out.append(lines[pos])
            pos += 1
    return ''.join(out)


def CollapseVectorAssignments(text, index):
    # Resolve functions independently: Ghidra local names and `this` types must
    # not leak between functions pasted together.
    header = re.compile(
        r"^[ \t]*(?:[\w:*&<>]+[ \t]+)+(?P<name>[\w:~]+)\s*"
        r"\((?P<parameters>[^{};]*)\)\s*\{", re.MULTILINE,
    )
    out = []
    pos = 0
    for match in header.finditer(text):
        if match.start() < pos:
            continue
        end, depth = match.end(), 1
        while end < len(text) and depth:
            depth += (text[end] == '{') - (text[end] == '}')
            end += 1
        if depth:
            continue
        out.append(CollapseBlock(text[pos:match.start()], index))
        thisType = match['name'].rpartition('::')[0] or None
        out.append(CollapseBlock(text[match.start():end], index, thisType, match['parameters']))
        pos = end
    out.append(CollapseBlock(text[pos:], index))
    return ''.join(out)
