#!/usr/bin/env python
"""Unit test for measures_shape_fp.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import unittest
import logging

import config

from measures_testing import (
    measure_factory,
    testfactory,
)


_default_exe = config.MEASURES_SIM_EXE
_formats = "SimMatrix SimOrderedPairs SimSparseMatrix".split()


def _get_arglist(exe, file_gen, expecter):
    measurer = expecter.fp_measurer()
    search_number = expecter.search_number()
    if search_number is None:
        logging.warning(
            f"No search_number for expecter {expecter} -- default to 2"
        )
        search_number = 2
    result = [
        str(exe),
        file_gen.pathname(),
        measurer.measure_opt(),
        measurer.similarity_opt(),
        expecter.format_opt(),
        str(search_number),
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


class TestCase(unittest.TestCase):
    # Test some bad inputs.
    _tani_sim_matrix_bad_search_num = _make_tc(
        "T", True, "SimMatrix", -1, None, -1
    )

    def test_tani_sim_matrix_bad_search_number(self):
        self.assertRaises(AssertionError, self._tani_sim_matrix_bad_search_num)


# Add tests for combinations of good inputs:
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
