"""
Provides similarity and dissimilarity measurers.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

from .i_measure import IMeasure
from .i_shape_measure import IShapeMeasure


def get_measurer(
    measure: IMeasure | IShapeMeasure,
    num_bits: int,
    is_similarity: bool,
    fingerprints: list,
):
    """Returns a new fingerprint measurer."""
    if is_similarity:
        return FPSimMeasurer(measure, num_bits, fingerprints)
    return FPDistMeasurer(measure, num_bits, fingerprints)


class IFPMeasurer:
    """Measure similarity or distance values for a set of fingerprints."""

    def fingerprints(self) -> list: ...
    def num_bit(self) -> int: ...
    def is_similarity(self) -> bool: ...
    def measure_opt(self) -> str: ...
    def tversky_alpha(self) -> float | None: ...
    def similarity_opt(self) -> str: ...
    def set_threshold(self, new_value: float) -> None: ...
    def threshold(self) -> float | None: ...
    def value(self, i, j) -> tuple[float, bool]:
        """
        Get the measure for fingerprints with indices i and j.
        Returns a tuple:
          v (a float): the measured value for i and j
          in_range (boolean): whether v lies within self's threshold
        """
        ...


class FPSimMeasurer(IFPMeasurer):
    """Provides similarity values for a set of fingerprints."""

    def __init__(
        self,
        measure: IMeasure | IShapeMeasure,
        num_bits: int,
        fingerprints: list,
    ):
        self._measure = measure
        self._num_bits = num_bits
        # Default threshold should accept all values.
        self._threshold = 0.0
        self._fingerprints = fingerprints

    def fingerprints(self):
        return self._fingerprints[:]

    def num_bits(self):
        return self._num_bits

    def is_similarity(self) -> bool:
        return True

    def measure_opt(self):
        return self._measure.measure_opt()

    def tversky_alpha(self):
        return self._measure.tversky_alpha()

    def similarity_opt(self):
        return "-S" if self.is_similarity() else "-D"

    def set_threshold(self, new_value):
        assert (0 <= new_value <= 1.0) or (new_value < 0)
        if new_value < 0:
            # All similarities are within threshold
            new_value = 0.0 if self.is_similarity() else 1.0
        self._threshold = new_value

    def threshold(self):
        return self._threshold

    def value(self, i, j):
        """
        Get the measure for fingerprints i and j.
        Returns a tuple:
          v (a float): the measured value for i and j
          in_range (boolean): whether v lies within self's threshold
        """
        fps = self._fingerprints
        if i == j:
            value = 1.0
        else:
            value = self._measure(fps[i], fps[j], self._num_bits)
        in_range = value >= self._threshold
        return (value, in_range)


class FPDistMeasurer(FPSimMeasurer):
    """Provides distance values for a set of fingerprints."""

    def __init__(
        self,
        measure: IMeasure | IShapeMeasure,
        num_bits: int,
        fingerprints: list,
    ):
        super().__init__(measure, num_bits, fingerprints)
        # By default, accept all values no matter how dissimilar.
        self._threshold = 1.0

    def is_similarity(self):
        return False

    def value(self, i, j):
        """
        Get the measure for fingerprints i and j.
        Returns a tuple:
          v (a float): the measured value for i and j
          in_range (boolean): whether v exceeds self's threshold
        """
        fps = self._fingerprints
        if i == j:
            value = 0.0
        else:
            value = 1.0 - self._measure(fps[i], fps[j], self._num_bits)
        in_range = value <= self._threshold
        return (value, in_range)


def main():
    import doctest

    status = doctest.testmod()
    assert (status.failed == 0) and (status.attempted > 0)


if __name__ == "__main__":
    main()
