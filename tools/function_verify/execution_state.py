"""Explicit machine states shared by transition execution and legacy adapters."""
from dataclasses import dataclass, field
from typing import Any

from verify import Unsupported
import z3


@dataclass
class ExecutionState:
    architecture: str
    memory: Any
    registers: dict = field(default_factory=dict)
    condition: Any = field(default_factory=lambda: z3.BoolVal(True))
    processor: dict = field(default_factory=dict)

    def validate(self, architecture):
        if self.architecture != architecture:
            raise Unsupported('Execution state architecture mismatch')
        if any(key[0] != 'register' or value.size() != key[2] * 8
               for key, value in self.registers.items()):
            raise Unsupported('Invalid execution state register storage')
        occupied = set()
        for _, offset, size in self.registers:
            if size <= 0 or offset < 0 or occupied.intersection(range(offset, offset + size)):
                raise Unsupported('Overlapping execution state registers')
            occupied.update(range(offset, offset + size))
        if self.memory is None or self.memory.domain().size() != (64 if architecture == 'windows' else 32):
            raise Unsupported('Execution state memory width mismatch')


@dataclass
class ExecutionOutcome:
    state: ExecutionState
    return_value: Any = None
    baseline: Any = None
    returned: bool = True
    guards: tuple = ()
    obligations: tuple = ()
    writes: tuple = ()

