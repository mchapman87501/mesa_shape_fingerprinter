#!/usr/bin/env python
"""Unit test for measures_nxn.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

from dataclasses import dataclass
import io
import logging
import subprocess
import unittest
from pathlib import Path
import tempfile

import config

from measures_testing import (
    fp_file_generator,
    fp_measurer,
    measure,
    results_verifier as rv,
)


@dataclass(frozen=True)
class CmdLineArgs:
    measure: str | None
    tversky_alpha: float | None
    compute_similarity: bool | None
    output_format: str | None
    sparse_threshold: float | None
    fingerprint_path: Path

    def as_args(self):
        """Convert to a subprocess.run argument list."""
        raw_args = [config.MEASURES_NXN_EXE]
        if self.measure is not None:
            raw_args += ["-m", self.measure]
        if self.tversky_alpha is not None:
            raw_args += ["--alpha", self.tversky_alpha]
        if self.compute_similarity is False:
            raw_args += ["--dissimilarity"]
        if self.output_format is not None:
            raw_args += ["-f", self.output_format]
        if self.sparse_threshold is not None:
            raw_args += ["-t", self.sparse_threshold]
        raw_args.append(self.fingerprint_path)
        return [str(arg) for arg in raw_args]


class TestCase(unittest.TestCase):
    def test_no_args(self):
        """Verify usage is shown when no args are provided."""
        # All measures_nxn arguments are optional except the fingerprint
        # filename.
        completion = subprocess.run(
            [str(config.MEASURES_NXN_EXE)], capture_output=True, encoding="utf8"
        )
        self.assertNotEqual(0, completion.returncode)
        self.assertTrue("usage:" in completion.stderr.lower())
        self.assertTrue("missing argument" in completion.stderr.lower())

    def test_help(self):
        """Test the help option(s)."""
        completion = subprocess.run(
            [str(config.MEASURES_NXN_EXE), "--help"],
            capture_output=True,
            encoding="utf8",
        )
        self.assertEqual(0, completion.returncode)
        self.assertTrue("usage:" in completion.stderr.lower())

    def test_invalid_measure_flag(self):
        with tempfile.TemporaryDirectory() as dirname:
            tmpdir = Path(dirname)
            completion = subprocess.run(
                [
                    str(config.MEASURES_NXN_EXE),
                    "--measure",
                    "InVaLiD MeAsUrE",
                ],
                capture_output=True,
                encoding="utf8",
                cwd=tmpdir,
            )
            self.assertNotEqual(0, completion.returncode)
            self.assertTrue("usage:" in completion.stderr.lower())
            self.assertTrue("invalid choice" in completion.stderr.lower())

    def test_default_measure(self):
        with fp_file_generator.FPFileGenerator(4) as fp_gen:
            cli_args = CmdLineArgs(
                measure=None,
                fingerprint_path=Path(fp_gen.pathname()),
                tversky_alpha=None,
                compute_similarity=None,
                output_format="O",  # Ordered pairs
                sparse_threshold=None,
            )

            # Run measures_nxn.  Verify it doesn't crash.
            # Verify it produces expected output.
            meas = measure.Tani()
            fpm = fp_measurer.FPSimMeasurer(
                meas, fp_gen.numbits(), fp_gen.fingerprints()
            )

            args = cli_args.as_args()
            completion = subprocess.run(
                args, capture_output=True, encoding="utf8"
            )

            self.assertEqual(0, completion.returncode)
            verifier = rv.OrderedPairs(fpm, None, None)

            inf = io.StringIO(completion.stdout)
            diffs = list(verifier.diffs(inf))
            self.assertEqual(len(diffs), 0)

    def test_matrix_output(self):
        with fp_file_generator.FPFileGenerator(4) as fp_gen:
            cli_args = CmdLineArgs(
                measure="C",
                fingerprint_path=Path(fp_gen.pathname()),
                tversky_alpha=None,
                compute_similarity=None,
                output_format="M",  # full Matrix
                sparse_threshold=None,
            )

            completion = subprocess.run(
                cli_args.as_args(), capture_output=True, encoding="utf8"
            )

            self.assertEqual(0, completion.returncode)

            meas = measure.Cosine()
            fpm = fp_measurer.FPSimMeasurer(
                meas, fp_gen.numbits(), fp_gen.fingerprints()
            )
            verifier = rv.Matrix(fpm, None, None)

            inf = io.StringIO(completion.stdout)
            diffs = list(verifier.diffs(inf))
            self.assertEqual(len(diffs), 0)

    def test_sparse_dist_matrix_output(self):
        self._test_sparse_matrix_output(False)

    def test_sparse_sim_matrix_output(self):
        self._test_sparse_matrix_output(True)

    def _test_sparse_matrix_output(self, compute_similarity: bool):
        with fp_file_generator.FPFileGenerator(4) as fp_gen:
            threshold = 0.5
            cli_args = CmdLineArgs(
                measure="E",
                fingerprint_path=Path(fp_gen.pathname()),
                tversky_alpha=None,
                compute_similarity=compute_similarity,
                output_format="S",  # Sparse matrix
                sparse_threshold=threshold,
            )

            meas = measure.Euclidean()
            klass = (
                fp_measurer.FPSimMeasurer
                if compute_similarity
                else fp_measurer.FPDistMeasurer
            )
            fpm = klass(meas, fp_gen.numbits(), fp_gen.fingerprints())

            completion = subprocess.run(
                cli_args.as_args(), capture_output=True, encoding="utf8"
            )

            self.assertEqual(0, completion.returncode)
            verifier = rv.SparseMatrix(fpm, None, threshold)

            inf = io.StringIO(completion.stdout)
            diffs = list(verifier.diffs(inf))
            self.assertEqual(len(diffs), 0)


def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()


if __name__ == "__main__":
    main()
