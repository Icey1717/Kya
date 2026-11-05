"""Equivalent region views of one byte array, with proved address ownership."""
from verify import Unsupported
import z3


class RegionMemory:
    def __init__(self, initial, regions, domain, *, prune_initial_stores=False):
        self.memory, self.domain = initial, domain
        self.regions = []
        for base, size in sorted(regions):
            if size <= 0 or base < 0 or base + size > 1 << initial.domain().size():
                raise Unsupported('Invalid region memory extent')
            if self.regions and base <= self.regions[-1][0] + self.regions[-1][1]:
                previous, length = self.regions[-1]
                self.regions[-1] = (previous, max(previous + length, base + size) - previous)
            else:
                self.regions.append((base, size))
        self.initial = initial
        self.initial_layers = None
        self.views = [None for _ in self.regions]
        self.certificates = {}
        self.prune_initial_stores = prune_initial_stores
        self.initial_disjoint = {}

    def view(self, index):
        if self.views[index] is None:
            if self.initial_layers is None:
                layers, root = [], self.initial
                while z3.is_store(root):
                    address = z3.simplify(root.arg(1))
                    layers.append((address, root.arg(2)))
                    root = root.arg(0)
                self.initial_layers = (root, list(reversed(layers)))
            memory, layers = self.initial_layers
            base, size = self.regions[index]
            for address, value in layers:
                # A view is read only after proving ownership of the complete
                # access. Concrete initial stores outside that region cannot
                # affect it; symbolic stores must remain because they may alias.
                if z3.is_bv_value(address):
                    keep = base <= address.as_long() < base + size
                else:
                    keep = True
                    if self.prune_initial_stores:
                        key = (address, base, size)
                        if key not in self.initial_disjoint:
                            outside = z3.simplify(z3.Or(z3.ULT(address, base), z3.UGE(address, base + size)))
                            disjoint = z3.is_true(outside)
                            if not disjoint and not z3.is_false(outside):
                                solver = z3.Solver()
                                solver.set(timeout=1000)
                                # Only entry-domain facts license caching. A
                                # later branch condition must never leak here.
                                solver.add(self.domain, z3.Not(outside))
                                disjoint = solver.check() == z3.unsat
                            self.initial_disjoint[key] = disjoint
                        keep = not self.initial_disjoint[key]
                if keep:
                    memory = z3.Store(memory, address, value)
            self.views[index] = memory
        return self.views[index]

    def owner(self, address, size, condition):
        # Z3 AST keys retain structural identity without serializing the whole
        # memory/path expression at every load and store. Keeping the ASTs in
        # the key also keeps their identities alive for the certificate lifetime.
        key = (address, size, condition)
        if key in self.certificates:
            return self.certificates[key]
        for index, (base, length) in enumerate(self.regions):
            if size > length:
                continue
            claim = z3.simplify(z3.And(z3.UGE(address, base), z3.ULE(address, base + length - size)))
            if z3.is_false(claim):
                continue
            if not z3.is_true(claim):
                solver = z3.Solver()
                solver.set(timeout=1000)
                solver.add(self.domain, condition, z3.Not(claim))
                if solver.check() != z3.unsat:
                    continue
            # Certificates are scoped to the exact path condition.
            self.certificates[key] = index
            return index
        return None  # Use the full array; uncertainty never licenses pruning.

    def load(self, address, size, condition):
        owner = self.owner(address, size, condition)
        memory = self.memory if owner is None else self.view(owner)
        parts = [memory[address + i] for i in reversed(range(size))]
        return z3.Concat(*parts) if len(parts) > 1 else parts[0]

    def store(self, address, value, condition):
        size = value.size() // 8
        owner = self.owner(address, size, condition)
        for i in range(size):
            byte = z3.Extract(i * 8 + 7, i * 8, value)
            self.memory = z3.Store(self.memory, address + i, byte)
            for index in range(len(self.views)):
                if owner is None or owner == index:
                    self.views[index] = z3.Store(self.view(index), address + i, byte)
