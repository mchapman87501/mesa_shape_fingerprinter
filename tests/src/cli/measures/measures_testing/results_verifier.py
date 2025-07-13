"""
Provides reporting of expected vs. actual results differences.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import typing as tp

from . import results_reader as rr
from . import expected_results_generator as erg


def approx(expected, actual, max_error_fract=0.00001):
    diff = float(abs(expected - actual))
    if expected:
        error_fract = diff / expected
    else:
        error_fract = diff
    return error_fract <= max_error_fract


class Base:
    """Verifies expected vs. actual results."""

    # Override in subclasses to provide compatible
    # results_reader and expected_results_generator
    _reader_klass: type | None = None
    _expecter_klass: type | None = None

    @classmethod
    def _get_reader_klass(cls):
        assert cls._reader_klass is not None
        return cls._reader_klass

    @classmethod
    def _get_expecter_klass(cls) -> type:
        assert cls._expecter_klass is not None
        return cls._expecter_klass

    def __init__(self, measurer, search_number, threshold):
        self._expecter = self._get_expecter_klass()(
            measurer, search_number, threshold
        )
        self._reader = self._get_reader_klass()()

    def expecter(self):
        return self._expecter

    def reader(self):
        return self._reader

    def _next_pair(self):
        e_done = a_done = False
        try:
            e = next(self._e_gen)
        except StopIteration:
            e = None
            e_done = True

        try:
            a = next(self._a_gen)
        except StopIteration:
            a = None
            a_done = True

        result = (e, a, e_done, a_done)
        return result

    def _diff_line(self, i, e, a: dict) -> tp.Iterable: ...

    def diffs(self, inf):
        """Generate diffs between actual results from inf and expected results."""
        self._e_gen = self._expecter.rows()
        self._a_gen = self._reader.rows(inf)

        max_total_errs = 10
        total_errs = 0
        i = 0
        while True:
            i += 1
            e, a, e_done, a_done = self._next_pair()
            if e_done and a_done:
                return
            if e_done:
                yield f"Line {i}: ran out of expected output with actual output remaining"
                return
            if a_done:
                yield f"Line {i}: ran out of actual output with expected output remaining"
                return
            for d in self._diff_line(i, e, a):  # type: ignore
                yield f"Line {i:3d}: {d}"
                yield f"    Expected: {e}"
                yield f"      Actual: {a}"
                total_errs += 1
                if total_errs >= max_total_errs:
                    yield "Too many errors.  Giving up."
                    return


class Matrix(Base):
    _reader_klass = rr.Matrix
    _expecter_klass = erg.Matrix

    def _diff_line(self, i, e, a):
        if len(e) != len(a):
            yield "Expected Matrix row of length {0}, got {1}".format(
                len(e), len(a)
            )
        max_reports = 3
        count = 0
        for col, (eitem, aitem) in enumerate(zip(e, a)):
            if not approx(eitem, aitem):
                yield "Column {0}: expected Matrix value {1}, got {2}".format(
                    col + 1, eitem, aitem
                )
                count += 1
                if count >= max_reports:
                    yield "..."
                    return


class SimMatrix(Matrix):
    _reader_klass = rr.SimMatrix
    _expecter_klass = erg.SimMatrix


class OrderedPairs(Base):
    _reader_klass = rr.OrderedPairs
    _expecter_klass = erg.OrderedPairs

    def _diff_line(self, i, e, a):
        if e[:2] != a[:2]:
            yield "Expected ordered pair indices {0}, got {1}".format(
                e[:2], a[:2]
            )
        if not approx(e[2], a[2]):
            yield "Expected ordered pair value {0!r}, got {1!r}".format(
                e[2], a[2]
            )


class SimOrderedPairs(Base):
    _reader_klass = rr.SimOrderedPairs
    _expecter_klass = erg.SimOrderedPairs

    def _diff_line(self, i, e, a):
        if e[:2] != a[:2]:
            yield "Expected SOP indices {0}, got {1}".format(e[:2], a[:2])
        # Try to handle Tversky and other measures in one go:
        for ev, av in zip(e[2:], a[2:]):
            if not approx(ev, av):
                yield "Expected SOP value {0!r}, got {1!r}".format(ev, av)


class SparseMatrix(Base):
    _reader_klass = rr.SparseMatrix
    _expecter_klass = erg.SparseMatrix

    def _diff_line(self, i, e, a):
        # Make a copy of a, which we'll mutate.
        a_copy = {} | a
        for index, evalue in e.items():
            try:
                avalue = a_copy[index]
                if not approx(evalue, avalue):
                    yield "Column {0}: expected sparse matrix value {1}, got {2}".format(
                        index, evalue, avalue
                    )
                del a_copy[index]
            except KeyError:
                yield "Did not find expected sparse matrix column {0} = {1} in {2}".format(
                    index, evalue, a_copy
                )
        for index, avalue in a_copy.items():
            yield "Unexpected sparse matrix entry {0} = {1}".format(
                index, avalue
            )


class SparseMatrixSearching(SparseMatrix):
    _reader_klass = rr.SparseMatrixSearching
    _expecter_klass = erg.SparseMatrixSearching

    def _diff_line(self, i, e, a):
        # If expected row is wrong, fail.  (i is 1-based.)
        assert e["row"] == (i - 1)
        if a["row"] != e["row"]:
            yield "Expected sparse matrix searching row index {0}, got {1}".format(
                i - 1, a["row"]
            )

        ecols = e["entries"]
        acols = a["entries"]
        for d in super(SparseMatrixSearching, self)._diff_line(i, ecols, acols):
            yield d


class SimSparseMatrix(SparseMatrixSearching):
    _reader_klass = rr.SimSparseMatrix
    _expecter_klass = erg.SimSparseMatrix

    def _diff_line(self, i, e, a):
        # Sparse matrix output for measures_sim may contain two copies of
        # the matrix, when using Tversky.  So the row numbers may not be
        # strictly increasing, like i.
        if a["row"] != e["row"]:
            yield "Expected sparse matrix searching row index {0}, got {1}".format(
                i - 1, a["row"]
            )

        ecols = e["entries"]
        acols = a["entries"]
        for d in super(SparseMatrixSearching, self)._diff_line(i, ecols, acols):
            yield d


class PVM(SparseMatrix):
    _reader_klass = rr.PVM
    _expecter_klass = erg.PVM

    # The semantics of PVM indices are slightly different than for
    # SparseMatrix format, but the comparison logic is identical.


def main():
    import doctest

    status = doctest.testmod()
    assert (status.failed == 0) and (status.attempted > 0)


if __name__ == "__main__":
    main()
