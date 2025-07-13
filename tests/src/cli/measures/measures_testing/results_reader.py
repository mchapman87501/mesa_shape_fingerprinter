"""
Provides readers for measures output formats.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""


def round_row(r):
    return [round(f, 6) for f in r]


class Base:
    """Interface for readers."""

    def rows(self, inf):
        """Generate rows of measures output data from inf."""
        raise NotImplementedError("Implement your own rows generator")


class Matrix(Base):
    """
    Reads full matrices.

    Example:
    >>> import tempfile, os
    >>> content = []
    >>> fd, pathname = tempfile.mkstemp()
    >>> outf = os.fdopen(fd, 'w')
    >>> for i in range(10):
    ...     row = [0.01 * i * j for j in range(10)]
    ...     content.append(row)
    ...     outf.write(" ".join([str(f) for f in row]))
    ...     outf.write('\\n')

    >>> outf.close()
    >>> with open(pathname) as inf:
    ...     m = Matrix()
    ...     for i, row in enumerate(m.rows(inf)):
    ...         exp = round_row(content[i])
    ...         act = round_row(row)
    ...         if exp != act:
    ...             print('Row {0}:\\n  {1}\\n  {2}'.format(i + 1, exp, act))

    """

    def rows(self, inf):
        for line in inf:
            yield [float(f) for f in line.strip().split()]


class SimMatrix(Matrix):
    pass


class OrderedPairs(Base):
    """Reads ordered-pair output."""

    def rows(self, inf):
        for line in inf:
            fields = line.strip().split()
            assert len(fields) == 3
            yield int(fields[0]), int(fields[1]), float(fields[2])


class SimOrderedPairs(Base):
    def rows(self, inf):
        for line in inf:
            fields = line.strip().split()
            # When invoked w. Tversky, measure_sim w. ordered pairs outputs
            # both Tversky measures.
            assert 3 <= len(fields) <= 4
            result = (int(fields[0]), int(fields[1]), float(fields[2]))
            if len(fields) > 3:
                result += (float(fields[3]),)
            yield result


class SparseMatrix(Base):
    def rows(self, inf):
        for line in inf:
            fields = line.strip().split()
            # How to do unittest assertions without being a
            # unittest.TestCase?
            assert fields.pop() == "-1"
            indices = [int(f) for f in fields[::2]]
            values = [float(f) for f in fields[1::2]]
            yield dict(zip(indices, values))


class SparseMatrixSearching(Base):
    def rows(self, inf):
        for line in inf:
            fields = line.strip().split()
            # How to do unittest assertions without being a
            # unittest.TestCase?
            assert fields.pop() == "-1"
            row_index = int(fields.pop(0))
            indices = [int(f) for f in fields[::2]]
            values = [float(f) for f in fields[1::2]]
            yield dict(row=row_index, entries=dict(zip(indices, values)))


SimSparseMatrix = SparseMatrixSearching


class PVM(Base):
    def rows(self, inf):
        for line in inf:
            fields = line.strip().split()
            # This is essentially the same as sparse matrix format,
            # but the semantics differ:
            # indices are based on search_number rather than
            # raw fingerprint index.
            assert fields.pop() == "-1"
            indices = [int(f) for f in fields[::2]]
            values = [float(f) for f in fields[1::2]]
            yield dict(zip(indices, values))


def main():
    import doctest

    status = doctest.testmod()
    assert (status.failed == 0) and (status.attempted > 0)


if __name__ == "__main__":
    main()
