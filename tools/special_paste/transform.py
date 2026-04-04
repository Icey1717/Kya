#!/usr/bin/env python3
"""
Special Paste transformer for Ghidra-decompiled Kya C++ output.

Reads text from stdin, applies transforms, writes to stdout.

Transforms (in order applied):
  1. Vtable dereferences  -> member calls
  2. Free-function calls  -> member calls (first arg is object)
  3. Base chain stripping -> (this->base).base.field -> this->field
  4. C-style casts        -> static_cast<T>(x)
  5. Double literals      -> append f suffix (0.0 -> 0.0f)
"""

import re
import struct
import sys


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def find_matching_paren(text: str, start: int) -> int:
    """Return index of the ')' matching text[start] == '(', or -1."""
    assert text[start] == "(", f"Expected '(' at {start}, got {text[start]!r}"
    depth = 0
    for i in range(start, len(text)):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return i
    return -1


def split_top_level_args(args_text: str) -> list[str]:
    """Split a comma-separated argument string on top-level commas only."""
    args: list[str] = []
    depth = 0
    current: list[str] = []
    for ch in args_text:
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        if ch == "," and depth == 0:
            args.append("".join(current).strip())
            current = []
        else:
            current.append(ch)
    tail = "".join(current).strip()
    if tail:
        args.append(tail)
    return args


def strip_base_chain_in(expr: str) -> str:
    """
    Strip base-class navigation chains of any form:
        (this->base).field                        ->  this->field
        (this->base).base.base.field              ->  this->field
        (ptr->base).base.field                    ->  ptr->field
        (this->pOwner->base).base.x.y             ->  this->pOwner->x.y
        (ptr->character).characterBase.base.field ->  ptr->field

    Two patterns are applied iteratively:
      1. ->base inside the parens (with optional .base* outside)
      2. ->anyField inside the parens with at least one .base. outside
    """
    # ->base inside parens, zero or more .base. outside
    p1 = re.compile(r"\((\w+(?:->\w+)*)->base\)(?:\.base)*\.(\w+(?:\.\w+)*)")
    # ->anyField inside parens, one or more named segments then .base. outside
    p2 = re.compile(r"\((\w+(?:->\w+)*)->\w+\)(?:\.\w+)*\.base\.(\w+(?:\.\w+)*)")
    prev = None
    while prev != expr:
        prev = expr
        expr = p1.sub(r"\1->\2", expr)
        expr = p2.sub(r"\1->\2", expr)
    return expr


# ---------------------------------------------------------------------------
# Individual transforms
# ---------------------------------------------------------------------------

def _is_object_arg(arg: str) -> bool:
    """Return True if arg looks like a this-pointer/object rather than a scalar."""
    arg = arg.strip()
    # Numeric literal (int, float, hex)
    if re.match(r"^-?(?:0[xX][\da-fA-F]+|\d+(?:\.\d*)?(?:[eE][+-]?\d+)?)[fFlLuU]*$", arg):
        return False
    return True


def t_vtable_calls(text: str) -> str:
    """
    (*pVTableExpr->Method)(maybeScalars..., thisArg, rest...)
        ->  (thisArg)->Method(maybeScalars..., rest...)

    Scans arguments left-to-right and picks the first one that looks like an
    object/pointer (not a numeric literal) as the this-pointer. Any arguments
    before it are moved to after the method name.
    """
    vtable_inner_re = re.compile(r"^\*(.*)->(\w+)$")
    result: list[str] = []
    pos = 0
    i = 0
    while i < len(text):
        if text[i : i + 2] == "(*":
            close = find_matching_paren(text, i)
            if close != -1 and close + 1 < len(text) and text[close + 1] == "(":
                inner = text[i + 1 : close]
                m = vtable_inner_re.match(inner.strip())
                if m:
                    args_start = close + 1
                    args_end = find_matching_paren(text, args_start)
                    if args_end != -1:
                        args_text = text[args_start + 1 : args_end]
                        args = split_top_level_args(args_text)
                        method = m.group(2)

                        result.append(text[pos:i])
                        if args:
                            # Find first object/pointer arg — skip leading scalars
                            this_idx = next(
                                (i for i, a in enumerate(args) if _is_object_arg(a)),
                                0,
                            )
                            this_arg = args[this_idx]
                            leading = args[:this_idx]
                            trailing = args[this_idx + 1:]
                            rest_parts = leading + trailing
                            rest_str = ", ".join(rest_parts)

                            # If the object arg is just `this` (possibly cast), use implicit this
                            bare = re.sub(r"^\s*\([\w\s*:]+\)\s*", "", this_arg).strip()
                            if bare == "this":
                                result.append(f"{method}({rest_str})")
                            elif re.match(r"^[\w][\w\->]*$", bare):
                                # Cast stripped to a pointer expression — use it directly
                                if rest_str:
                                    result.append(f"{bare}->{method}({rest_str})")
                                else:
                                    result.append(f"{bare}->{method}()")
                            elif rest_str:
                                result.append(f"({this_arg})->{method}({rest_str})")
                            else:
                                result.append(f"({this_arg})->{method}()")
                        else:
                            result.append(text[i : args_end + 1])

                        pos = args_end + 1
                        i = pos
                        continue
        i += 1

    result.append(text[pos:])
    return "".join(result)


# Classes where Class::Method(obj, rest...) always means obj->Method(rest...)
# even when the first arg has no cast or base-chain.
_KNOWN_FIRST_ARG_IS_OBJECT: set[str] = {
    "ByteCode",
}


def t_free_to_member_calls(text: str) -> str:
    """
    Class::Method(baseChainExpr, rest...)
        ->  strippedExpr->Method(rest...)

    Only fires when the first argument is (or contains) a base-chain expression,
    i.e. stripping it changes the expression.
    """
    call_re = re.compile(r"\b(\w+)::(\w+)\(")
    result: list[str] = []
    pos = 0

    for m in call_re.finditer(text):
        if m.start() < pos:
            continue

        result.append(text[pos : m.start()])
        call_start = m.end() - 1
        call_end = find_matching_paren(text, call_start)

        if call_end == -1:
            result.append(text[m.start() :])
            pos = len(text)
            break

        args_text = text[call_start + 1 : call_end]
        args = split_top_level_args(args_text)
        fn_name = m.group(2)

        if args:
            first_arg = args[0].strip()
            stripped = strip_base_chain_in(first_arg)
            bare = re.sub(r"^\s*\([\w\s*:]+\)\s*", "", stripped).strip()
            rest_str = ", ".join(args[1:])

            if bare == "this":
                # Object is this (possibly cast) — use implicit this
                result.append(f"{fn_name}({rest_str})")
                pos = call_end + 1
                continue
            elif re.match(r"^\w+$", bare) and (stripped != first_arg or bare != first_arg):
                # Object is a cast of a simple variable — use explicit arrow
                if rest_str:
                    result.append(f"{bare}->{fn_name}({rest_str})")
                else:
                    result.append(f"{bare}->{fn_name}()")
                pos = call_end + 1
                continue
            elif re.match(r"^\w+$", first_arg) and m.group(1) in _KNOWN_FIRST_ARG_IS_OBJECT:
                # Known class where first arg is always the object, no cast needed
                if rest_str:
                    result.append(f"{first_arg}->{fn_name}({rest_str})")
                else:
                    result.append(f"{first_arg}->{fn_name}()")
                pos = call_end + 1
                continue
            elif stripped != first_arg:
                # Object was a base-chain — use explicit arrow
                if rest_str:
                    result.append(f"{stripped}->{fn_name}({rest_str})")
                else:
                    result.append(f"{stripped}->{fn_name}()")
                pos = call_end + 1
                continue

        result.append(text[m.start() : call_end + 1])
        pos = call_end + 1

    result.append(text[pos:])
    return "".join(result)


def t_remove_base_chains(text: str) -> str:
    """Strip all remaining (X->base).base*.field patterns."""
    return strip_base_chain_in(text)


def t_c_casts_to_static_cast(text: str) -> str:
    """
    Convert C-style casts to static_cast:
        (Type *)(expr)  ->  static_cast<Type *>(expr)
        (Type *)word    ->  static_cast<Type *>(word)

    Matches pointer casts (type ending with *) and plain type casts where
    the type looks like a class name (starts with uppercase) or known primitive.
    """
    PRIMITIVES = {"int", "unsigned", "char", "short", "long", "float",
                  "double", "bool", "void", "uint", "ulong", "uchar", "ushort"}

    NULL_EXPRS = re.compile(r"^(0x0+|0|NULL|nullptr)$")

    type_pat = r"(?:const\s+)?[\w:]+(?:\s*\*+)?"
    cast_re = re.compile(r"\((" + type_pat + r")\)")

    result: list[str] = []
    pos = 0
    i = 0

    while i < len(text):
        m = cast_re.match(text, i)
        if m:
            type_part = m.group(1).strip()
            base_type = type_part.lstrip("const ").split("::")[-1].rstrip("* ").strip()
            is_pointer = "*" in type_part
            is_class = base_type[:1].isupper() if base_type else False
            is_primitive = base_type in PRIMITIVES

            if (is_pointer or is_class or is_primitive) and base_type:
                after = m.end()
                if after < len(text) and text[after] == "(":
                    close = find_matching_paren(text, after)
                    if close != -1:
                        expr = text[after + 1 : close]
                        if NULL_EXPRS.match(expr.strip()):
                            i = close + 1
                            continue
                        result.append(text[pos:i])
                        result.append(f"static_cast<{type_part}>({expr})")
                        pos = close + 1
                        i = pos
                        continue
                else:
                    word_m = re.match(r"[\w:]+", text[after:])
                    if word_m:
                        expr = word_m.group(0)
                        if NULL_EXPRS.match(expr.strip()) or expr.strip() == "this":
                            i = after + word_m.end()
                            continue
                        result.append(text[pos:i])
                        result.append(f"static_cast<{type_part}>({expr})")
                        pos = after + word_m.end()
                        i = pos
                        continue

        i += 1

    result.append(text[pos:])
    return "".join(result)


def t_float_literals(text: str) -> str:
    """
    Append 'f' suffix to unqualified floating-point literals:
        0.0  ->  0.0f
        1.5e3  ->  1.5e3f
    Skips literals already suffixed, hex literals, and numbers that are
    part of identifiers (e.g. auStack64.ba must not match as 64.).
    """
    return re.sub(
        r"(?<![0-9a-zA-Z_])(\d+\.\d*|\d*\.\d+)(?:[eE][+-]?\d+)?(?!\w)",
        lambda m: m.group(0) + "f",
        text,
    )


def t_inline_bytecode_temps(text: str) -> str:
    """
    Inline single-use temporaries produced by ->Get(F32|S32|U32)() calls:

        var = expr->GetF32();
        lhs = var;
      ->
        lhs = expr->GetF32();

    Only fires when 'var' is the entire RHS of the immediately following line,
    ensuring no side-effects from collapsing.
    """
    get_re = re.compile(
        r"^(\s*)(\w+)\s*=\s*(.+?->Get(?:F32|S32|U32)\([^)]*\))\s*;\s*$"
    )

    lines = text.splitlines(keepends=True)
    out: list[str] = []
    i = 0
    while i < len(lines):
        m = get_re.match(lines[i])
        if m and i + 1 < len(lines):
            indent, var, rhs = m.group(1), m.group(2), m.group(3)
            use_re = re.compile(
                r"^(\s*)(.+?)\s*=\s*" + re.escape(var) + r"\s*;\s*$"
            )
            use_m = use_re.match(lines[i + 1])
            use_m = use_re.match(lines[i + 1])
            # Scan forward from i+2: if var is read before its next reassignment,
            # inlining is NOT safe (the value would be lost).
            next_next_uses_var = False
            reassign_re = re.compile(r"^\s*" + re.escape(var) + r"\s*=")
            for future in lines[i + 2:]:
                m2 = reassign_re.match(future)
                if m2:
                    future_rhs = future[m2.end():]
                    if re.search(r"\b" + re.escape(var) + r"\b", future_rhs):
                        next_next_uses_var = True
                    break
                elif re.search(r"\b" + re.escape(var) + r"\b", future):
                    next_next_uses_var = True
                    break
            if use_m and not next_next_uses_var:
                lhs = use_m.group(2)
                # Preserve the line ending of the surviving (use) line
                raw = lines[i + 1]
                if raw.endswith("\r\n"):
                    eol = "\r\n"
                elif raw.endswith("\n"):
                    eol = "\n"
                else:
                    eol = ""
                out.append(f"{indent}{lhs} = {rhs};{eol}")
                i += 2
                continue
        out.append(lines[i])
        i += 1
    return "".join(out)




def t_remove_this_param(text: str) -> str:
    """
    Remove the explicit `ClassName *this` first parameter from member function signatures:
        void CShadow::SetDisplayable(CShadow *this, int displayable)
          -> void CShadow::SetDisplayable(int displayable)

    Only fires when the first parameter is exactly `Word *this` and the
    function is a `ClassName::Method(...)` form.
    """
    # Match: anything Class::Method(Type *this) or Class::Method(Type *this, rest)
    pattern = re.compile(
        r"(\b\w+::\w+\()\s*\w+\s*\*\s*this\s*(?:,\s*)?(.*?)(\))",
        re.DOTALL,
    )
    return pattern.sub(r"\1\2\3", text)



def t_dat_float_literals(text: str) -> str:
    """
    Convert Ghidra DAT float references to real float literals:
        (float)&DAT_bf000000  ->  -0.5f
        (float)&DAT_3f800000  ->  1.0f

    The 8 hex digits are the IEEE 754 big-endian representation of the float.
    NaN and infinity are left unchanged.
    """
    def _replace(m: re.Match) -> str:
        try:
            value = struct.unpack(">f", bytes.fromhex(m.group(1)))[0]
        except (ValueError, struct.error):
            return m.group(0)
        if value != value or value in (float("inf"), float("-inf")):
            return m.group(0)
        s = f"{value:.8g}"
        if "." not in s and "e" not in s:
            s += ".0"
        return s + "f"

    return re.sub(r"\(float\)&DAT_([0-9a-fA-F]{8})", _replace, text)


def t_math_macros(text: str) -> str:
    """Replace Ghidra all-caps math macros with their C runtime equivalents."""
    text = re.sub(r"\bSQRT\b", "sqrtf", text)
    text = re.sub(r"\bABS\b", "fabsf", text)
    return text


TRANSFORMS = [
    t_vtable_calls,
    t_free_to_member_calls,
    t_remove_base_chains,
    t_c_casts_to_static_cast,
    t_dat_float_literals,
    t_float_literals,
    t_math_macros,
    t_remove_this_param,
    t_inline_bytecode_temps,
]


def transform(text: str) -> str:
    for fn in TRANSFORMS:
        text = fn(text)
    return text


# ---------------------------------------------------------------------------
# CLI entry point + self-tests
# ---------------------------------------------------------------------------

def _run_tests() -> None:
    cases = [
        # float literals
        ("x = 0.0;", "x = 0.0f;"),
        ("x = 1.5e3;", "x = 1.5e3f;"),
        ("x = 0.0f;", "x = 0.0f;"),  # already suffixed, no change
        # number embedded in identifier must not be treated as float literal
        ("auStack64.ba = (this->field_0x20).x;", "auStack64.ba = (this->field_0x20).x;"),
        # base chain stripping
        ("(this->base).base.base.pAnim", "this->pAnim"),
        # ->base inside parens, field directly after (no .base. outside)
        ("(this->base).field_0x38", "this->field_0x38"),
        ("if (param_2 != (this->base).field_0x38) {", "if (param_2 != this->field_0x38) {"),
        # base chain with ptr chain before ->base and dotted sub-field after
        ("(this->pOwner->base).base.dynamic.speed = 0.0;", "this->pOwner->dynamic.speed = 0.0f;"),
        # base chain with deeper ptr chain
        ("(this->pOwner->pChild->base).base.x", "this->pOwner->pChild->x"),
        # named base member (not literally ->base) with intermediate chain
        (
            "local_30.x = (pCVar1->character).characterBase.base.dynamic.horizontalVelocityDirectionEuler.x;",
            "local_30.x = pCVar1->dynamic.horizontalVelocityDirectionEuler.x;",
        ),
        # named intermediate + multiple .base levels
        ("(this->actor).actorBase.base.base.field", "this->field"),
        # bytecode temp inlining — consecutive pairs
        (
            "fVar12 = pByteCode->GetF32();\nthis->field_0x214 = fVar12;\nfVar12 = pByteCode->GetF32();\nthis->field_0x210 = fVar12;",
            "this->field_0x214 = pByteCode->GetF32();\nthis->field_0x210 = pByteCode->GetF32();",
        ),
        # bytecode temp — var read several lines later before reassignment, NOT safe
        (
            "fVar12 = pByteCode->GetF32();\nthis->field_0x218 = fVar12;\nfoo();\nbar(fVar12);",
            "fVar12 = pByteCode->GetF32();\nthis->field_0x218 = fVar12;\nfoo();\nbar(fVar12);",
        ),
        # bytecode temp — var used on line after use, should NOT inline
        (
            "fVar12 = pByteCode->GetF32();\nthis->field_0x218 = fVar12;\nthis->field_0x21c = fVar12;",
            "fVar12 = pByteCode->GetF32();\nthis->field_0x218 = fVar12;\nthis->field_0x21c = fVar12;",
        ),
        # bytecode temp — var used elsewhere, should NOT inline
        (
            "fVar12 = pByteCode->GetF32();\nfoo(fVar12, fVar12);",
            "fVar12 = pByteCode->GetF32();\nfoo(fVar12, fVar12);",
        ),
        # bytecode temp — next line is not `lhs = var`, don't inline
        (
            "fVar12 = pByteCode->GetF32();\nfoo(fVar12);",
            "fVar12 = pByteCode->GetF32();\nfoo(fVar12);",
        ),
        # bytecode temp — var appears in arithmetic on next line, don't inline
        (
            "fVar12 = pByteCode->GetF32();\nthis->x = fVar12 + 1.0f;",
            "fVar12 = pByteCode->GetF32();\nthis->x = fVar12 + 1.0f;",
        ),
        # bytecode temp — no following line at all, don't inline
        (
            "fVar12 = pByteCode->GetF32();",
            "fVar12 = pByteCode->GetF32();",
        ),
        # bytecode temp — var appears in RHS of its own reassignment (self-ref), NOT safe
        (
            "fVar12 = pByteCode->GetF32();\nthis->x = fVar12;\nfVar12 = fVar12 + 1.0f;",
            "fVar12 = pByteCode->GetF32();\nthis->x = fVar12;\nfVar12 = fVar12 + 1.0f;",
        ),
        # bytecode temp — var reassigned cleanly (no self-ref), safe even with more code after
        (
            "fVar12 = pByteCode->GetF32();\nthis->x = fVar12;\nfVar12 = 0.0f;\nfoo(fVar12);",
            "this->x = pByteCode->GetF32();\nfVar12 = 0.0f;\nfoo(fVar12);",
        ),
        # bytecode temp — indentation preserved
        (
            "    fVar12 = pByteCode->GetF32();\n    this->x = fVar12;",
            "    this->x = pByteCode->GetF32();",
        ),
        # bytecode temp — var name is substring of another var (word boundary check)
        (
            "fVar1 = pByteCode->GetF32();\nthis->x = fVar1;\nfoo(fVar12);",
            "this->x = pByteCode->GetF32();\nfoo(fVar12);",
        ),
        # bytecode temp — S32 variant inlined
        (
            "iVar3 = pByteCode->GetS32();\nthis->count = iVar3;",
            "this->count = pByteCode->GetS32();",
        ),
        # bytecode temp — U32 variant inlined
        (
            "uVar7 = pByteCode->GetU32();\nthis->flags = uVar7;",
            "this->flags = pByteCode->GetU32();",
        ),
        # bytecode temp — non-GetX call on ptr, don't inline (different method)
        (
            "fVar12 = pByteCode->Read();\nthis->x = fVar12;",
            "fVar12 = pByteCode->Read();\nthis->x = fVar12;",
        ),
        # bytecode temp — three pairs in a row all inlined
        (
            "fVar12 = pByteCode->GetF32();\nthis->x = fVar12;\nfVar12 = pByteCode->GetF32();\nthis->y = fVar12;\nfVar12 = pByteCode->GetF32();\nthis->z = fVar12;",
            "this->x = pByteCode->GetF32();\nthis->y = pByteCode->GetF32();\nthis->z = pByteCode->GetF32();",
        ),
        # remove explicit this param from member function signatures
        (
            "void CShadow::SetDisplayable(CShadow *this, int displayable)",
            "void CShadow::SetDisplayable(int displayable)",
        ),
        # only param — leaves empty parens
        (
            "void CActor::Reset(CActor *this)",
            "void CActor::Reset()",
        ),
        # non-member free function — should NOT change
        (
            "void SetDisplayable(CShadow *this, int displayable)",
            "void SetDisplayable(CShadow *this, int displayable)",
        ),
        # DAT float literal decoding
        ("(float)&DAT_bf000000", "-0.5f"),        # -0.5
        ("(float)&DAT_3f800000", "1.0f"),          # 1.0
        ("(float)&DAT_3f000000", "0.5f"),          # 0.5
        ("(float)&DAT_00000000", "0.0f"),          # 0.0
        ("(float)&DAT_40490fdb", "3.1415927f"),    # pi
        ("x = (float)&DAT_bf000000;", "x = -0.5f;"),
        # math macros
        ("x = SQRT(val);", "x = sqrtf(val);"),
        ("x = ABS(val);", "x = fabsf(val);"),
        ("x = SQRT(ABS(val));", "x = sqrtf(fabsf(val));"),
        # should not match lowercase or partial
        ("x = sqrt(val);", "x = sqrt(val);"),
        ("ABSOLUTELY = 1;", "ABSOLUTELY = 1;"),
        # ByteCode::Get* variants
        ("ByteCode::GetF32(pByteCode);", "pByteCode->GetF32();"),
        ("ByteCode::GetS32(pByteCode);", "pByteCode->GetS32();"),
        ("ByteCode::GetU32(pByteCode);", "pByteCode->GetU32();"),
        # free-function with cast of non-this pointer
        (
            "CActor::DoMessage((CActor *)pHunter,pHunter->field_0x364,0x27,0);",
            "pHunter->DoMessage(pHunter->field_0x364, 0x27, 0);",
        ),
        # free-function with cast-of-this — use implicit this
        (
            "CActor::RestartCurAnim((CActor *)this);",
            "RestartCurAnim();",
        ),
        # free-function to member
        (
            "CAnimation::RegisterBone((this->base).base.base.pAnimationController, this->boneId);",
            "this->pAnimationController->RegisterBone(this->boneId);",
        ),
        # vtable call - this is only arg, use implicit this
        (
            "(*((this->base).base.base.pVTable)->GetLifeInterface)((CActor *)this);",
            "GetLifeInterface();",
        ),
        # vtable call - this is first arg, use implicit this
        (
            "(*((this->base).base.base.pVTable)->GetLifeInterfaceOther)((CActor *)this);",
            "GetLifeInterfaceOther();",
        ),
        # vtable call - scalar args before this, use implicit this
        (
            "(*((this->base).base.base.pVTable)->LifeDecrease)(2.0,(CActorAutonomous *)this);",
            "LifeDecrease(2.0f);",
        ),
        # vtable call - cast of this->member as object arg
        (
            "(*((this->pOwner->base).base.pVTable)->SetState)((CActor *)this->pOwner,9,-1);",
            "this->pOwner->SetState(9, -1);",
        ),
        # C-style cast of `this` alone — must NOT be transformed (never cast this)
        ("(CActor *)this", "(CActor *)this"),
        # C-style cast of non-this pointer — still transforms
        ("(CActor *)pActor", "static_cast<CActor *>(pActor)"),
        # C-style cast - parenthesised expr
        ("(int *)(ptr)", "static_cast<int *>(ptr)"),
        # C-style null casts — must NOT be transformed
        ("(CActorsTable *)0x0", "(CActorsTable *)0x0"),
        ("(CActor *)0", "(CActor *)0"),
        ("(void *)NULL", "(void *)NULL"),
        ("(CActor *)nullptr", "(CActor *)nullptr"),
    ]

    failures = 0
    for src, expected in cases:
        got = transform(src)
        status = "OK" if got == expected else "FAIL"
        if status == "FAIL":
            failures += 1
            print(f"  FAIL\n    input:    {src!r}\n    expected: {expected!r}\n    got:      {got!r}")
        else:
            print(f"  OK    {src!r}")

    print(f"\n{len(cases) - failures}/{len(cases)} tests passed.")
    if failures:
        sys.exit(1)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--test":
        _run_tests()
    else:
        input_text = sys.stdin.read()
        sys.stdout.write(transform(input_text))
