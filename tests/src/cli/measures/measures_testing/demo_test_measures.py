"""
Unit test for all measures.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import unittest
import logging

from . import config
from . import measure_factory
from . import testfactory


def get_all_arglist(exe, file_gen, expecter):
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


def get_nxn_arglist(exe, file_gen, expecter):
    measurer = expecter.fp_measurer()
    result = [
        exe,
        file_gen.pathname(),
        measurer.measure_opt(),
        measurer.similarity_opt(),
        expecter.format_opt(),
    ]
    alpha = measurer.tversky_alpha()
    if alpha is not None:
        result.append(str(alpha))
    thresh = expecter.threshold()
    if thresh is not None:
        result.append(str(thresh))
    return result


_supported_formats = dict(
    all="Matrix OrderedPairs SparseMatrix SparseMatrixSearching PVM".split(),
    nxn="Matrix OrderedPairs SparseMatrix".split(),
)

_supported_shape_formats = dict(shape_fp="Matrix SparseMatrix PVM".split())


class MeasuresSimTCTools:
    _default_exe = config.MEASURES_SIM_EXE

    @classmethod
    def formats(cls):
        return "SimMatrix SimOrderedPairs SimSparseMatrix".split()

    @classmethod
    def _get_arglist(cls, exe, file_gen, expecter):
        measurer = expecter.fp_measurer()
        search_number = expecter.search_number()
        if search_number is None:
            logging.warn(
                "Unexpected search_number None for expecter {0} -- default to 2".format(
                    expecter
                )
            )
            search_number = 2
        result = [
            exe,
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

    @classmethod
    def make_tc(
        cls,
        measure_name,
        is_sim,
        format_name,
        search_number,
        tversky_alpha,
        threshold,
    ):
        return testfactory.get_valid_args_method(
            measure_name,
            is_sim,
            format_name,
            search_number=search_number,
            tversky_alpha=tversky_alpha,
            threshold=threshold,
            default_exe=cls._default_exe,
            get_arglist=cls._get_arglist,
        )


class MeasuresSimTestCase(unittest.TestCase):
    # Test some bad inputs.
    _tani_sim_matrix_bad_search_num = MeasuresSimTCTools.make_tc(
        "Tani", True, "SimMatrix", -1, None, -1
    )

    def test_tani_sim_matrix_bad_search_number(self):
        self.assertRaises(AssertionError, self._tani_sim_matrix_bad_search_num)


# Add tests for combinations of good inputs:
for format in MeasuresSimTCTools.formats():
    for measure_name in measure_factory.measure_names():
        for is_sim in [False, True]:
            tc = MeasuresSimTCTools.make_tc(
                measure_name, is_sim, format, 4, 0.25, 0.7
            )
            setattr(MeasuresSimTestCase, tc.__name__, tc)


def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()


if __name__ == "__main__":
    main()
