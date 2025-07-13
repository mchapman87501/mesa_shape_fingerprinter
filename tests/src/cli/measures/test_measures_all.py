#!/usr/bin/env python
"""Unit test for measures_all.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import sys
import os
import unittest
import logging

from measures_testing import (
    measure_factory,
    results_verifier_factory,
    testfactory,
)

_thisdir = os.path.abspath(os.path.dirname(__file__))


def _relpath(p):
    return os.path.abspath(os.path.join(_thisdir, p))


_default_exe = _relpath("../measures_all")
_formats = "Matrix OrderedPairs SparseMatrix SparseMatrixSearching PVM".split()


def _get_arglist(exe, file_gen, expecter):
    measurer = expecter.fp_measurer()
    result = [
        exe,
        file_gen.pathname(),
        measurer.measure_opt(),
        measurer.similarity_opt(),
        expecter.format_opt(),
        expecter.searching_opt(),
        str(expecter.search_number() or "0"),
    ]
    alpha = measurer.tversky_alpha()
    if alpha is not None:
        result.append(str(alpha))
    thresh = expecter.threshold()
    if thresh is not None:
        result.append(str(thresh))
    return result


def _make_tc(
    measure_name, is_sim, format_name, search_number, tversky_alpha, threshold
):
    return testfactory.get_valid_args_method(
        measure_name,
        is_sim,
        format_name,
        search_number=search_number,
        tversky_alpha=tversky_alpha,
        threshold=threshold,
        default_exe=_default_exe,
        get_arglist=_get_arglist,
    )


# Create the placeholder test case:
class TestCase(unittest.TestCase):
    pass


# Populate it with combinations of good inputs:
for format in _formats:
    for measure_name in measure_factory.measure_names():
        for is_sim in [False, True]:
            tc = _make_tc(measure_name, is_sim, format, 4, 0.25, 0.7)
            setattr(TestCase, tc.__name__, tc)


def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()


if __name__ == "__main__":
    main()
