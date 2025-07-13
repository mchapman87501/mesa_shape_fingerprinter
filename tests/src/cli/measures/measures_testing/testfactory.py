"""
Generates test methods.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import logging
from pathlib import Path
import typing as tp

from . import measure_factory
from . import results_verifier_factory
from . import fp_file_generator
from . import fp_measurer
from . import measures_runner
from . import expected_results_generator

GetArglistFn = tp.Callable[
    [Path, fp_file_generator.FPFileGenerator, expected_results_generator.Base],
    list,
]


# Oddball name?  Yes.  I want to hide it from nosetests.
def get_valid_args_method(
    measure_name,
    is_similar,
    format_name,
    search_number,
    tversky_alpha,
    threshold,
    num_bits=5,
    *,
    default_exe: Path,
    get_arglist: GetArglistFn | None = None,
    compressed_shape_fps: bool = False,
):
    """
    Get a test method which runs a measures program with valid arguments,
    checking its actual output against its expected output.

    Example: get a method to test Tanimoto similarity with full matrix
    format.
    >>> import unittest
    >>> class TestCase(unittest.TestCase):
    ...     test_tani_matrix = get_valid_args_method("Tani", True, "Matrix", -1, -1, -1,  default_exe='../../measures_all')

    >>> loader = unittest.TestLoader()
    >>> suite = loader.loadTestsFromTestCase(TestCase)
    >>> result = unittest.TextTestRunner(verbosity=2).run(suite)
    """

    def valid_args_test(self):
        measure = measure_factory.measure_by_name(measure_name, tversky_alpha)
        if measure.is_shape():
            file_gen = fp_file_generator.ShapeFPFileGenerator(
                num_bits, compressed=compressed_shape_fps
            )
        else:
            file_gen = fp_file_generator.FPFileGenerator(num_bits)

        fp_set_measurer = fp_measurer.get_measurer(
            measure, num_bits, is_similar, file_gen.fingerprints()
        )

        verifier = results_verifier_factory.by_format(
            format_name, fp_set_measurer, search_number, threshold
        )
        runner = measures_runner.MeasuresRunner(
            file_gen,
            verifier.expecter(),
            default_exe=default_exe,
            get_arglist=get_arglist,
        )
        completion = runner.run()
        self.assertEqual(
            0,
            completion.returncode,
            f"Exit status {completion.returncode} for {completion.args}:\n{completion.stderr}",
        )

        # Fail on the first difference, rather than flooding the
        # test output.
        errors = 0
        out = completion.stdout
        err = completion.stderr
        for diff in verifier.diffs(out.splitlines()):
            logging.error(diff)
            # self.fail()
            errors += 1
        if 0 != errors:
            logging.error("Measures stderr:")
            logging.error(err)
        self.assertEqual(0, errors)

    sim_str = "Similarity" if is_similar else "Distance"
    exe_name = default_exe.name
    name = "test_{0}_{1}_{2}_{3}_{4}_{5}_{6}_{7}_{8}".format(
        measure_name,
        sim_str,
        format_name,
        search_number,
        tversky_alpha,
        threshold,
        num_bits,
        compressed_shape_fps,
        exe_name,
    )
    valid_args_test.__name__ = name.replace(".", "_")
    return valid_args_test


def main():
    import doctest

    status = doctest.testmod()
    assert (status.failed == 0) and (status.attempted > 0)


if __name__ == "__main__":
    main()
