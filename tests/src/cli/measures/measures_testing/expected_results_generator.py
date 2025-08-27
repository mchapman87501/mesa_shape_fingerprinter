"""
Provides expected results generators for various measures output formats.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import logging
import typing as tp

from . import fp_measurer as fpm

logger = logging.getLogger("ExpectedResults")
logger.setLevel(logging.ERROR)


class Base:
    """Base class for expected results generators"""

    def __init__(
        self,
        fp_measurer: fpm.IFPMeasurer,
        search_number: tp.Optional[int],
        threshold: tp.Optional[float],
    ):
        self._fp_measurer = fp_measurer
        # search_number and threshold are placeholder parameters,
        # provided for API consistency among the expected results
        # generator classes.
        # Interested subclasses can save these parameter values.

    def fp_measurer(self):
        return self._fp_measurer

    def format_opt(self) -> str:
        """Get the option string to produce this format from a Mesa measures binary."""
        return "-INVALID"

    # See SearchingBase below
    def searching_opt(self):
        return "-F" if self.search_number() is None else "-T"

    def search_number(self):
        return None

    def threshold(self):
        return None

    def rows(self) -> tp.Iterable[tp.Iterable]:
        """Generate row by row expected results."""
        # STUB
        yield tuple()


class Matrix(Base):
    """Generates expected output for a full matrix.

    Example of Matrix metadata:
    >>> from fp_measurer import get_measurer
    >>> from measure import Tani
    >>> m = Matrix(get_measurer(Tani(), 4, True), None, None)
    >>> m.format_opt() == "-M"
    True
    >>> m.searching_opt() == "-F"
    True
    >>> m.search_number() is None
    True
    >>> m.threshold() is None
    True
    """

    def format_opt(self):
        return "-M"

    def rows(self) -> tp.Iterable[tp.Iterable]:
        m = self._fp_measurer
        size = len(m.fingerprints())
        for i in range(size):
            yield [m.value(i, j)[0] for j in range(size)]


# Would I could remember what "Sim" means, here.
# It appears to indicate that a search index is being used.
class SimMatrix(Matrix):
    def __init__(self, measurer, search_number, threshold):
        super(SimMatrix, self).__init__(measurer, search_number, threshold)
        self._search_number = search_number

    def search_number(self):
        return self._search_number

    def rows(self):
        m = self._fp_measurer
        size = len(m.fingerprints())
        sn = self._search_number or 0
        for i in range(sn):
            result = []
            for j in range(sn, size):
                result.append(m.value(i, j)[0])
            yield result

        if m.tversky_alpha() is not None:
            for i in range(sn):
                result = []
                for j in range(sn, size):
                    result.append(m.value(j, i)[0])
                yield result


class OrderedPairs(Base):
    def format_opt(self):
        return "-O"

    def rows(self) -> tp.Iterable[tp.Iterable]:
        m = self._fp_measurer
        size = len(m.fingerprints())
        for i in range(size):
            for j in range(size):
                yield (i, j, m.value(i, j)[0])


class SimOrderedPairs(OrderedPairs):
    def __init__(self, measurer, search_number, threshold):
        super().__init__(measurer, search_number, threshold)
        self._search_number = search_number

    def search_number(self):
        return self._search_number

    def rows(self) -> tp.Iterable[tp.Iterable]:
        m = self._fp_measurer
        size = len(m.fingerprints())
        is_tversky = m.tversky_alpha() is not None
        for i in range(self._search_number):
            for j in range(self._search_number, size):
                value, in_range = m.value(i, j)
                result = (i, j, value)
                if is_tversky:
                    v2, in_range = m.value(j, i)
                    result += (v2,)
                yield result


class SparseMatrix(Base):
    def __init__(self, measurer, search_number, threshold):
        super().__init__(measurer, search_number, threshold)
        self._threshold = threshold
        self._fp_measurer.set_threshold(threshold)

    def format_opt(self):
        return "-S"

    def threshold(self):
        return self._threshold

    def rows(self):
        m = self._fp_measurer
        size = len(m.fingerprints())
        for i in range(size):
            row = {}
            for j in range(size):
                # Sparse never includes the diagonals.
                if i != j:
                    v, in_range = m.value(i, j)
                    if in_range:
                        row[j] = v
            yield row


class SearchingBase(Base):
    def __init__(self, measurer, search_number, threshold):
        super().__init__(measurer, search_number, threshold)
        self._search_number = search_number
        self._fp_measurer.set_threshold(threshold)
        self._threshold = threshold

    def search_number(self):
        return self._search_number

    def threshold(self):
        # At time of writing, all 'searching' output formats also have
        # associated thresholds.
        return self._threshold


class SparseMatrixSearching(SearchingBase):
    """
    Generate expected results for Sparse, with Searching, format.
    Example:
    >>> from fp_measurer import get_measurer
    >>> from measure import Tani
    >>> fingerprints = [0b1011, 0b0101, 0b1010, 0b1011, 0b1111]
    >>> m = get_measurer(Tani(), 4, True, fingerprints)
    >>> egen = SparseMatrixSearching(m, 2, 0.25)
    >>> egen.search_number() == 2
    True
    >>> egen.threshold() == 0.25
    True
    >>> rows = [r for r in egen.rows()]
    >>> rows == [{'row':0, 'entries':{2:2.0/3.0, 3:1.0, 4:0.75}}, {'row':1, 'entries':{3:0.25, 4:0.5}}]
    True
    """

    def format_opt(self):
        return "-S"

    def rows(self):
        m = self._fp_measurer
        size = len(m.fingerprints())
        snum = self._search_number
        for i in range(snum):
            entries = {}
            for j in range(snum, size):
                v, in_range = m.value(i, j)
                if in_range:
                    entries[j] = v

            # TODO: Move row type def to a separate module, to share w.
            # results_reader.
            row = dict(row=i, entries=entries)
            yield row


class SimSparseMatrix(SparseMatrixSearching):
    # Like SparseMatrixSearching, but will contain complementary entries
    # for Tversky measure.
    def rows(self):
        m = self._fp_measurer
        size = len(m.fingerprints())
        snum = self._search_number
        for i in range(snum):
            entries = {}
            for j in range(snum, size):
                v, in_range = m.value(i, j)
                if in_range:
                    entries[j] = v

            row = dict(row=i, entries=entries)
            yield row

        # Using Tversky measure?  Expect a second sparse matrix of
        # complementary values:
        if m.tversky_alpha() is not None:
            for i in range(snum):
                entries = {}
                for j in range(snum, size):
                    v, in_range = m.value(j, i)
                    if in_range:
                        entries[j] = v

                row = dict(row=i, entries=entries)
                yield row


class PVM(SearchingBase):
    """
    Generate expected results for PVM format.
    Example:
    >>> from fp_measurer import get_measurer
    >>> from measure import Tani
    >>> fingerprints = [0b1011, 0b0101, 0b1010, 0b1011, 0b1111]
    >>> m = get_measurer(Tani(), 4, True, fingerprints)
    >>> egen = PVM(m, 2, 0.75)
    >>> egen.search_number() == 2
    True
    >>> egen.threshold() == 0.75
    True
    >>> rows = [r for r in egen.rows()]
    >>> rows == [{1: 1.0, 2: 0.75}, {}]
    True
    """

    def format_opt(self):
        return "-P"

    def rows(self):
        m = self._fp_measurer
        size = len(m.fingerprints())
        snum = self._search_number
        for i in range(snum):
            row = {}
            for j in range(snum, size):
                v, in_range = m.value(i, j)
                if in_range:
                    row[j - snum] = v
            yield row


def main():
    import doctest

    status = doctest.testmod()
    assert (status.failed == 0) and (status.attempted > 0)


if __name__ == "__main__":
    main()
