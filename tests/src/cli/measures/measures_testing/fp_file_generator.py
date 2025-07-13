"""
Tools for generating fingerprint files.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import base64
import os
import random
import tempfile
import zlib


class FPFileGenerator:
    """
    For use in with statements: creates a temporary fp file.
    Deletes the file when client is done with it.

    Example:

    >>> import os
    >>> input_numbits = 5
    >>> with FPFileGenerator(input_numbits) as g:
    ...     pname = g.pathname()
    ...     num_fps = g.num_fps()
    ...     fingerprints = g.fingerprints()
    ...     numbits = g.numbits()
    ...     with open(pname) as inf:
    ...         linecount = sum(1 for line in inf)

    >>> num_fps
    32
    >>> len(fingerprints)
    32
    >>> linecount
    32
    >>> os.path.exists(pname)
    False
    """

    def __init__(self, numbits, compressed=False):
        self._outf = tempfile.NamedTemporaryFile(mode="w")
        self._numbits = numbits
        # OBS:  Compressed has meaning only for shape fingerprint files.
        # See ShapeFPFileGenerator, below.
        self._compressed = compressed
        self._fingerprints = self._gen_fingerprints()

    def __enter__(self):
        # Write the fingerprints file.
        self._write_fps(self._outf)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        del self._outf
        return False

    def _gen_fingerprints(self):
        i_max = 1 << self._numbits
        return [i for i in range(i_max)]

    def _write_fps(self, outf):
        fmt = "{{0:0{numbits}b}}".format(numbits=self._numbits)
        wfp = self._fp_writer(outf, fmt)
        for fp in self._fingerprints:
            wfp(fp)
        outf.flush()

    def _fp_writer(self, outf, fmt):
        def wr(fp):
            outf.write(fmt.format(fp) + "\n")

        wr.__name__ = "_fp_writer.wr"
        return wr

    def pathname(self):
        return self._outf.name

    def numbits(self):
        return self._numbits

    def fingerprints(self):
        """Get the fingerprints, as integers, as they were written to the file."""
        return self._fingerprints[:]

    def num_fps(self):
        return len(self._fingerprints)


class ShapeFPFileGenerator(FPFileGenerator):
    """
    For use in with statements:  creates a temporary shape fp file.
    Deletes the file when client is done with it.

    Example:

    >>> import os
    >>> input_numbits = 5
    >>> with ShapeFPFileGenerator(input_numbits) as g:
    ...     pname = g.pathname()
    ...     num_fps = g.num_fps()
    ...     fingerprints = g.fingerprints()
    ...     numbits = g.numbits()
    ...     with open(pname) as inf:
    ...         linecount = sum(1 for line in inf)

    >>> num_fps  # Expect twice as many as can be represented in num_bits.
    64
    >>> len(fingerprints)
    64
    >>> linecount  # Expect 4 * num_fps
    256
    >>> os.path.exists(pname)
    False
    """

    def _gen_fingerprints(self):  # type: ignore
        # Generate fingerprint groups.
        i_max = 1 << self._numbits
        # Generate both exhaustive, uniform groups and random groups.
        result = [[i, i, i, i] for i in range(i_max)]
        for i in range(i_max):
            group = [random.randrange(0, i_max) for j in range(4)]
            result.append(group)
        random.shuffle(result)
        return result

    def _fp_writer(self, outf, fmt):
        if not self._compressed:

            def wr(fp):
                for entry in fp:
                    outf.write(fmt.format(entry) + "\n")
        else:

            def wr(fp):
                for entry in fp:
                    uncompressed = fmt.format(entry)
                    compressed = zlib.compress(uncompressed)
                    encoded = base64.b64encode(compressed)
                    with_prefix = b"C" + encoded
                    outf.write(with_prefix + b"\n")

        wr.__name__ = "_fp_writer.wr"
        return wr


def main():
    import doctest

    status = doctest.testmod()
    assert (status.failed == 0) and (status.attempted > 0)


if __name__ == "__main__":
    main()
