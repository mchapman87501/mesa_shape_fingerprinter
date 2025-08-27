"""
Generate full matrices for validating measures output.
Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import math
import typing as tp

from .fp_utils import bitcount
from .i_measure import IMeasure


class Base(IMeasure):
    def tversky_alpha(self) -> tp.Optional[float]:
        return None

    def is_shape(self):
        return False


class Tani(Base):
    def __call__(self, i: int, j: int, num_bits: int = 4):
        """Get the tani measure of two bit vectors, i and j.

        Examples:
        >>> m = Tani()
        >>> m(0b0111, 0b0101) == 2.0 / 3.0
        True
        >>> m(0b000, 0b111) == 0.0
        True
        >>> m(0, 0) == 0.0
        True
        >>> m(0b0101, 0b1011) == 1.0 / 4.0
        True
        >>> m.measure_opt() == "-T"
        True
        >>> m.tversky_alpha() is None
        True
        """
        result = 0.0
        denom = bitcount(i | j)
        if denom:
            result = bitcount(i & j) / float(denom)
        return result

    def measure_opt(self):
        return "-T"


class Euclidean(Base):
    def __call__(self, i: int, j: int, num_bits: int = 4):
        """Get the normalized Euclidean similarity between i and j.
        Examples:
        >>> m = Euclidean()
        >>> m(0b010, 0b010, 3)
        1.0
        >>> m(0b000, 0b111, 3)
        0.0
        >>> m(0b00, 0b11, 2)
        0.0
        >>> m(0b010, 0b011, 3) == 1.0 - (1.0 / math.sqrt(3.0))
        True
        >>> m.measure_opt() == "-E"
        True
        >>> m.tversky_alpha() is None
        True
        """
        distance = 1.0
        if num_bits:
            max_distance = math.sqrt(num_bits)
            distance = math.sqrt(bitcount(i ^ j)) / max_distance
        result = 1.0 - distance
        return result

    def measure_opt(self):
        return "-E"


class Cosine(Base):
    def __call__(self, i: int, j: int, num_bits: int = 4):
        """Get the Cosine measure of two bit vectors, i and j.

        Examples:
        >>> m = Cosine()
        >>> m(0b011, 0b011)
        1.0
        >>> m(0b001, 0b110)
        0.0
        >>> m(0b01, 0b11) == 1.0 / math.sqrt(2.0)
        True
        >>> m.measure_opt() == "-C"
        True
        >>> m.tversky_alpha() is None
        True
        """
        result = 0.0
        a = bitcount(i)
        b = bitcount(j)
        c = bitcount(i & j)
        denom = math.sqrt(a * b)
        if denom > 0:
            result = c / denom
        return result

    def measure_opt(self):
        return "-C"


class Tversky(Base):
    def __init__(self, alpha: float):
        # According to http://www.ncbi.nlm.nih.gov/pmc/articles/PMC2527184/?tool=pubmed
        # alpha and beta must be >= 0.
        # Mesa's convention is to set beta = 2.0 - alpha, hence the asserts:
        assert 0 <= alpha <= 2.0
        self._alpha = alpha
        self._beta = 2.0 - alpha

    def __call__(self, i: int, j: int, num_bits: int = 4):
        """For a given alpha, get a tversky measure calculator.
        Examples:
        >>> Tversky(0.0)(0b011, 0b011)
        1.0
        >>> Tversky(2.0)(0b011, 0b011)
        1.0
        >>> Tversky(1.0)(0b011, 0b011)
        1.0

        Tversky with alpha and beta both set to 1 should be equivalent to
        Tanimoto:
        >>> Tversky(1.0)(0b011, 0b110) == 1.0/3.0
        True

        Asymmetry:
        >>> Tversky(0.0)(0b0011, 0b1010) == Tversky(2.0)(0b1010, 0b0011)
        True
        >>> Tversky(0.0)(0b0011, 0b1010) == 1.0/3.0
        True

        >>> Tversky(0.0).measure_opt() == "-V"
        True
        >>> Tversky(0.85).tversky_alpha() == 0.85
        True
        """

        result = 1.0
        a = bitcount(i)
        b = bitcount(j)
        c = bitcount(i & j)
        denom = self._alpha * (a - c) + self._beta * (b - c) + c
        if denom:
            result = c / denom
        # if 0 == denom then a and b must both be all zeroes,
        # therefore identical, therefore result = 1.0
        return result

    def measure_opt(self):
        return "-V"

    def tversky_alpha(self):
        return self._alpha


class Bub(Base):
    def __call__(self, i: int, j: int, num_bits: int = 4):
        both = bitcount(i & j)
        either = bitcount(i | j)
        # Heh.  Python integers aren't *exactly* like bitvectors:
        mask = (1 << num_bits) - 1
        neither = bitcount(mask & ~(i | j))
        s = math.sqrt(float(both) * neither)

        result = 0.0
        denom = s + either
        if denom:
            result = (s + both) / denom
        return result

    def measure_opt(self):
        return "-B"


class Hamann(Base):
    def __call__(self, i: int, j: int, num_bits: int = 4):
        """Get the Hamann measure of two bit vectors, i and j.
        That's the fraction of bits which are the same in both i and j.

        I can find only one solid reference to this measure:
        Spath, H. (1980).  Cluster Analysis Algorithms.  Trans. Ursula Bull.
         Ellis Horwood (Halstead/Wiley), Chichester, England.
        (via the documentation for RAPDistance Version 1.04, from
         http://www.anu.edu.au/BoZo/software/)

        Examples:
        >>> m = Hamann()
        >>> m(0b0001, 0b0010, 4) == 2.0 / 4.0
        True
        >>> m(0b0001, 0b0011, 4) == 0.0 / 4.0
        True
        >>> m(0b0001, 0b0001, 4) == 2.0 / 4.0
        True
        >>> m.measure_opt() == "-H"
        True
        >>> m.tversky_alpha() is None
        True
        """
        # Update to match current C++ implementation.
        mask = (1 << num_bits) - 1
        a = bitcount((i & j) & mask)
        b = bitcount((i & ~j) & mask)
        c = bitcount((~i & j) & mask)
        d = bitcount((~i & ~j) & mask)
        result = ((a + d) - (b + c)) / (a + b + c + d)
        return result

    def measure_opt(self):
        return "-H"


def main():
    import doctest

    status = doctest.testmod()
    assert (status.failed == 0) and (status.attempted > 0)


if __name__ == "__main__":
    main()
