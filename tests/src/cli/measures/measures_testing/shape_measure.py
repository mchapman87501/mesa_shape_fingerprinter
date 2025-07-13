"""
Provides measures for shape fingerprints.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved

Example:
>>> import math
>>> ref1 = [0b1000, 0b1000, 0b1000, 0b1000]
>>> ref2 = [0b1000, 0b1001, 0b0001, 0b0011]
>>> comp = [0b0001, 0b0010, 0b0011, 0b1100]
>>> def r(x): return round(x, 9)
>>> result1 = []
>>> result2 = []
>>> for m in [Bub(), Cosine(), Euclidean(), Hamann(), Tani(), Tversky(0.0)]:
...     f = m(ref1, comp, 4)
...     result1.append(r(m(ref1, comp, 4)))
...     result2.append(r(m(ref2, comp, 4)))
>>> result1 == [0.0, r(0.5 * math.sqrt(2)), 0.5, 0.5, 0.5, r(1.0/3.0)]
True
>>> result2 == result1
True
"""

from .i_shape_measure import IShapeMeasure
from . import measure


class ShapeFPBase(IShapeMeasure):
    def __init__(self, measure):
        self._measure = measure

    def __call__(self, i, j, num_bits=None):
        m = self._measure
        ifp = i[0]
        result = m(ifp, j[0], num_bits)
        for jfp in j[1:]:
            curr = m(ifp, jfp, num_bits)
            if curr > result:
                result = curr
        return result

    def measure_opt(self):
        return self._measure.measure_opt()

    def tversky_alpha(self):
        return self._measure.tversky_alpha()

    def is_shape(self):
        return True


# Metaclasses could save some typing, here...
class Bub(ShapeFPBase):
    def __init__(self):
        super().__init__(measure.Bub())


class Cosine(ShapeFPBase):
    def __init__(self):
        super().__init__(measure.Cosine())


class Euclidean(ShapeFPBase):
    def __init__(self):
        super().__init__(measure.Euclidean())


class Hamann(ShapeFPBase):
    def __init__(self):
        super().__init__(measure.Hamann())


class Tani(ShapeFPBase):
    def __init__(self):
        super().__init__(measure.Tani())


class Tversky(ShapeFPBase):
    def __init__(self, tversky_alpha):
        super().__init__(measure.Tversky(tversky_alpha))


def main():
    import doctest

    status = doctest.testmod()
    assert (status.failed == 0) and (status.attempted > 0)


if __name__ == "__main__":
    main()
