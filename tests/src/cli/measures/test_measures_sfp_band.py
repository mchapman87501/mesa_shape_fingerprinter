#!/usr/bin/env python
"""Unit test for measures_sfp_band.
Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import sys
import os
import math
import unittest
import logging
import subprocess
import time
import multiprocessing
import itertools

from measures_testing import (
    measure_factory,
    results_verifier_factory,
    testfactory,
)

_thisdir = os.path.abspath(os.path.dirname(__file__))


def _relpath(p):
    return os.path.abspath(os.path.join(_thisdir, p))


_default_exe = _relpath("../measures_sfp_band")


def get_exe():
    global _default_exe
    return os.environ.get("EXE", _default_exe)


def _run(args, **kw):
    all_args = [get_exe()] + args
    completion = subprocess.run(all_args, capture_output=True, encoding="utf8")
    return completion.returncode, completion.stdout, completion.stderr


def _mkdir(p):
    if not os.path.exists(p):
        os.makedirs(p)


# For multiprocessing tests:
def process_band(params):
    (
        sfp_path,
        sim_flag,
        format,
        threshold,
        records_flag,
        start,
        end,
        band_path,
    ) = params
    with open(band_path, "w") as outf:
        args = [sim_flag, "-f", format, records_flag, str(start), str(end)]
        if format == "S":
            args += ["-t", str(threshold)]
        args.append(sfp_path)
        status, out, err = _run(args, stdout=outf)
    return (status, band_path, args, err)


# This is nowhere near so robust as the other measures tests...
class TestCase(unittest.TestCase):
    def _get_linecount(self, pathname):
        with open(pathname) as inf:
            return sum(1 for line in inf)

    def _time(self, fn):
        t0 = time.time()
        fn()
        tf = time.time()
        logging.debug("{:.0f} seconds for {}".format(tf - t0, fn.__name__))

    def test_help(self):
        for opt in ["-h", "--help"]:
            status, out, err = _run([opt])
            self.assertEqual(0, status)
            self.assertTrue("Usage" in err)

    def test_too_few_args(self):
        status, out, err = _run([])
        self.assertTrue(0 != status)
        self.assertTrue("for more information" in err)

    def test_minimal_valid(self):
        self._time(self._test_minimal_valid)

    def _test_minimal_valid(self):
        status, out, err = _run([_relpath("data/in/sample_shape_fps.txt")])
        self.assertEqual(0, status)
        # TODO:  Verify output is consistent w. that of measures_shape_fp,
        # same data file, same threshold, sparse distance tani.

    # TODO:  Test alpha params.

    def _gen_measures_args(self, measure_names):
        dist_flags = itertools.cycle("-d --dist -s --sim".split())
        alpha_flags = itertools.cycle("-a --alpha".split())
        alphas = itertools.cycle([0.4, 1.6])
        for measure_name in measure_names:
            alpha = next(alphas)
            yield [
                next(dist_flags),
                next(alpha_flags),
                str(alpha),
                "-m",
                measure_name,
                _relpath("data/in/sample_shape_fps.txt"),
            ]

    def test_valid_measures(self):
        # Ensure valid measures arguments are not rejected.
        for args in self._gen_measures_args("B C E H V T".split()):
            status, out, err = _run(args)
            if status:
                logging.error("Non-zero exit status for %s" % repr(args))
                logging.error(err)
            self.assertEqual(0, status)
            # TODO: validate out

    def test_invalid_measures(self):
        for args in self._gen_measures_args("X I M".split()):
            status, out, err = _run(args)
            if status == 0:
                logging.error("Unexpected success for %s" % repr(args))
                logging.error(err)
            self.assertNotEqual(0, status)
            # TODO: validate out

    def test_bands(self):
        for format in "S M".split():
            for sim_flag in "-d -s --dist --sim".split():

                def run_test():
                    self._test_bands(format, sim_flag)

                run_test.__name__ = "test_bands, {}, {}".format(
                    format, sim_flag
                )
                self._time(run_test)

    def test_sparse_bands_performance(self):
        def sparse_dist_bands():
            self._test_bands(
                "S",
                "--dist",
                sfp_pathname=_relpath("data/in/in_count_order.txt"),
            )

        self._time(sparse_dist_bands)

    def _test_bands(self, format, sim_flag, sfp_pathname=None):
        # Use a small sfp file -- don't have all day...
        sfp_pathname = sfp_pathname or _relpath("data/in/sample_shape_fps.txt")
        threshold = 0.3

        # Each shape fingerprint uses 4 lines.
        expected_rows = self._get_linecount(sfp_pathname) // 4
        num_bands = max(multiprocessing.cpu_count(), 4)
        rows_per_job = int(math.ceil(expected_rows / float(num_bands)))

        records_options = itertools.cycle(["-r", "--records"])
        jobs = []
        cwd = _relpath(".")
        for band_num, i in enumerate(range(0, expected_rows, rows_per_job)):
            i_end = min(expected_rows, i + rows_per_job)
            band_path = _relpath("data/out/band_{}.txt".format(band_num))
            jobs.append(
                [
                    sfp_pathname,
                    sim_flag,
                    format,
                    threshold,
                    next(records_options),
                    i,
                    i_end,
                    band_path,
                ]
            )

        p = multiprocessing.Pool()

        _mkdir(_relpath("data/out"))
        results = p.map(process_band, jobs)
        failures = 0
        total_lines = 0
        for status, band_path, args, err in results:
            if status:
                failures += 1
                logging.error("Status {} for args: {}".format(status, args))
                logging.error("Errors:")
                logging.error(err)
            with open(band_path) as inf:
                plausible_content = True
                for i, line in enumerate(inf):
                    if format == "S":
                        if not line.strip().endswith("-1"):
                            if plausible_content:
                                logging.error(
                                    "Format {}, sim {}: Line {} does not end with expected sentinel".format(
                                        format, sim_flag, i + 1
                                    )
                                )
                                plausible_content = False
                    else:
                        # Square matrix, expected_rows == expected_cols
                        if len(line.strip().split()) != expected_rows:
                            if plausible_content:
                                logging.error(
                                    "Format {}, sim {}: Line {} does not have expected number of entries".format(
                                        format, sim_flag, i + 1
                                    )
                                )
                                plausible_content = False
                    total_lines += 1
                if not plausible_content:
                    failures += 1
            # Forced cleanup:
            os.remove(band_path)
        p.close()
        # How to validate consolidated matrix file?
        self.assertEqual(0, failures)
        self.assertEqual(expected_rows, total_lines)


def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()


if __name__ == "__main__":
    main()
