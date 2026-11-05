"""Field-specific output tolerances; instruction arithmetic remains exact."""
from dataclasses import dataclass
from fractions import Fraction
import z3


@dataclass(frozen=True)
class FloatPolicy:
    absolute: str = '0.000001'
    relative: str = '0.00001'

    def __post_init__(self):
        if Fraction(self.absolute) < 0 or Fraction(self.relative) < 0:
            raise ValueError('Floating-point tolerances must be nonnegative')

    def condition(self, windows, ps2):
        if windows.size() != 32 or ps2.size() != 32:
            raise ValueError('Tolerance requires binary32 fields')
        w, s = [z3.fpBVToFP(bits, z3.Float32()) for bits in (windows, ps2)]
        finite = z3.And(z3.Not(z3.fpIsNaN(w)), z3.Not(z3.fpIsInf(w)),
                        z3.Not(z3.fpIsNaN(s)), z3.Not(z3.fpIsInf(s)))
        wr, sr = z3.fpToReal(w), z3.fpToReal(s)
        difference = wr - sr
        magnitude = z3.If(sr < 0, -sr, sr)
        budget = z3.RealVal(self.absolute) + z3.RealVal(self.relative) * magnitude
        close = z3.If(difference < 0, -difference, difference) <= budget
        return z3.If(finite, close, windows == ps2)

