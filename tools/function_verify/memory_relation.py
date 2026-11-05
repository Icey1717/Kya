"""Explicit byte and pointer relations for differently laid-out machine heaps."""
from dataclasses import dataclass
from verify import Unsupported
import z3


def load(memory, address, size):
    address = z3.BitVecVal(address, memory.domain().size()) if isinstance(address, int) else address
    parts = [memory[address + i] for i in reversed(range(size))]
    return z3.Concat(*parts) if size > 1 else parts[0]


def store(memory, address, value):
    address = z3.BitVecVal(address, memory.domain().size()) if isinstance(address, int) else address
    for i in range(value.size() // 8):
        memory = z3.Store(memory, address + i, z3.Extract(i * 8 + 7, i * 8, value))
    return memory


@dataclass(frozen=True)
class Region:
    name: str
    base: int
    size: int


class MemoryRelation:
    """Mappings constrain outputs; private regions still require separate checks.

    No address equality between platforms is assumed. Aliases are expressed by
    reusing the same region, rather than declaring overlapping allocations.
    """
    def __init__(self):
        self.regions = {'windows': {}, 'ps2': {}}
        self.fields = []
        self.identities = []
        self.private = {'windows': [], 'ps2': []}
        self.numeric_policies = {}
        self.integer_extensions = {}

    def region(self, side, name, base, size, *, private=False):
        if (side not in self.regions or not isinstance(base, int) or not isinstance(size, int)
                or size <= 0 or base < 0 or base + size > 1 << (64 if side == 'windows' else 32)):
            raise ValueError('Invalid memory relation region')
        if name in self.regions[side] or any(base < r.base + r.size and r.base < base + size
                                             for r in self.regions[side].values()):
            raise ValueError('Duplicate or overlapping memory relation region')
        self.regions[side][name] = Region(name, base, size)
        if private:
            self.private[side].append((base, size))

    def address(self, side, name, offset, size):
        region = self.regions[side][name]
        if not 0 <= offset <= region.size - size or size <= 0:
            raise ValueError('Field outside memory relation region')
        return region.base + offset

    def identity(self, windows, ps2):
        if (not 0 < windows < 1 << 64 or not 0 < ps2 < 1 << 32
                or any(windows == w or ps2 == s for w, s in self.identities)):
            raise ValueError('Pointer identities must be nonzero and one-to-one')
        self.identities.append((windows, ps2))

    def field(self, windows_region, windows_offset, ps2_region, ps2_offset, size, *, pointer=False,
              numeric_policy=None, name=None, ps2_size=None, signed=False):
        if ps2_size is not None and (pointer or type(ps2_size) is not int or not 0 < ps2_size <= size):
            raise ValueError('Integer field widths require 0 < PS2 width <= Windows width')
        if signed and (pointer or ps2_size is None):
            raise ValueError('Signed extension requires an explicit integer source width')
        if numeric_policy is not None and (pointer or size != 4 or not name or ps2_size is not None):
            raise ValueError('Numeric policies require named binary32 fields')
        if numeric_policy is not None and any(value[0] == name for value in self.numeric_policies.values()):
            raise ValueError('Duplicate numeric field name')
        wa = self.address('windows', windows_region, windows_offset, 8 if pointer else size)
        sa = self.address('ps2', ps2_region, ps2_offset, 4 if pointer else ps2_size or size)
        widths = (8, 4) if pointer else (size, ps2_size or size)
        for old_wa, old_sa, old_widths, _ in self.fields:
            if any(a < b + n and b < a + m for a, m, b, n in
                   [(wa, widths[0], old_wa, old_widths[0]), (sa, widths[1], old_sa, old_widths[1])]):
                raise ValueError('Overlapping mapped fields')
        if any(base <= wa < base + length for base, length in self.private['windows']) or any(
                base <= sa < base + length for base, length in self.private['ps2']):
            raise ValueError('Mapped field in private storage')
        self.fields.append((wa, sa, widths, pointer))
        if ps2_size is not None:
            self.integer_extensions[len(self.fields) - 1] = signed
        if numeric_policy is not None:
            self.numeric_policies[len(self.fields) - 1] = (name, numeric_policy)

    def condition(self, windows_memory, ps2_memory, *, outputs=False):
        claims = []
        for index, (wa, sa, widths, pointer) in enumerate(self.fields):
            w, s = load(windows_memory, wa, widths[0]), load(ps2_memory, sa, widths[1])
            if index in self.integer_extensions and widths[0] != widths[1]:
                s = (z3.SignExt if self.integer_extensions[index] else z3.ZeroExt)((widths[0] - widths[1]) * 8, s)
            if outputs and index in self.numeric_policies:
                claims.append(self.numeric_policies[index][1].condition(w, s))
                continue
            claims.append(z3.Or(z3.And(w == 0, s == 0), *[z3.And(w == wp, s == sp)
                          for wp, sp in self.identities]) if pointer else w == s)
        return z3.And(*claims)

    def unchanged(self, side, initial, final, *, writes=None):
        """Require every byte outside mapped writes/private storage unchanged."""
        allowed = list(self.private[side])
        if writes is not None and all(z3.is_bv_value(z3.simplify(address)) for address, _ in writes):
            # Restore only actually touched private bytes. Retain full-array
            # equality, so the optimization cannot hide an untracked write.
            touched = set()
            for address, value in writes:
                start = z3.simplify(address).as_long()
                touched.update(offset for offset in range(start, start + value.size() // 8)
                               if any(base <= offset < base + size for base, size in allowed))
            allowed = [(offset, 1) for offset in sorted(touched)]
        index = 0 if side == 'windows' else 1
        allowed += [(f[index], f[2][index]) for f in self.fields]
        # Array equality after restoring permitted bytes avoids quantifiers.
        restored = final
        for base, size in allowed:
            for offset in range(size):
                address = z3.BitVecVal(base + offset, final.domain().size())
                restored = z3.Store(restored, address, initial[address])
        return restored == initial

    def require_satisfiable(self, condition):
        solver = z3.Solver()
        solver.set(timeout=5000)
        solver.add(condition)
        if solver.check() != z3.sat:
            raise Unsupported('Memory relation domain is empty or inconclusive')

    def writes_within_frame(self, side, writes):
        """Frame proof for executors that record every store, including RAM outputs."""
        index = 0 if side == 'windows' else 1
        allowed = self.private[side] + [(row[index], row[2][index]) for row in self.fields]
        return z3.And(*[z3.Or(*[z3.And(z3.UGE(address, base),
                                      z3.ULE(address, base + size - value.size() // 8))
                               for base, size in allowed if size >= value.size() // 8])
                        for address, value in writes])
