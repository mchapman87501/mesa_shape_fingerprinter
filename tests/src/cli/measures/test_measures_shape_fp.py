#!/usr/bin/env python
"""Unit test for measures_sim.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import io
import logging
import subprocess
import tempfile
import unittest
from dataclasses import dataclass
from pathlib import Path

import config
from measures_testing import (
    fp_file_generator,
    fp_measurer,
    shape_measure,
    results_verifier as rv,
)

EXE = config.MEASURES_SHAPE_FP_EXE


@dataclass(frozen=True)
class CmdLineArgs:
    measure: str | None
    tversky_alpha: float | None
    compute_similarity: bool | None
    search_index: int | None
    output_format: str | None
    sparse_threshold: float | None
    fingerprint_path: Path

    def as_subprocess_args(self):
        """Convert to a subprocess.run argument list."""
        raw_args = [config.MEASURES_SHAPE_FP_EXE]
        if self.measure is not None:
            raw_args += ["-m", self.measure]
        if self.tversky_alpha is not None:
            raw_args += ["--alpha", self.tversky_alpha]
        if self.compute_similarity is False:
            raw_args += ["--dissimilarity"]
        if self.search_index is not None:
            raw_args += ["--search", self.search_index]
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
            [str(EXE)], capture_output=True, encoding="utf8"
        )
        self.assertNotEqual(0, completion.returncode)
        self.assertTrue("usage:" in completion.stderr.lower())
        self.assertTrue("missing argument" in completion.stderr.lower())

    def test_help(self):
        completion = subprocess.run(
            [str(EXE), "--help"], capture_output=True, encoding="utf8"
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

    def test_invalid_format(self):
        with tempfile.TemporaryDirectory() as dirname:
            tmpdir = Path(dirname)
            completion = subprocess.run(
                [
                    str(config.MEASURES_NXN_EXE),
                    "-f",
                    "InVaLiD FORMAT",
                ],
                capture_output=True,
                encoding="utf8",
                cwd=tmpdir,
            )
            self.assertNotEqual(0, completion.returncode)
            self.assertTrue("usage:" in completion.stderr.lower())
            self.assertTrue("invalid choice" in completion.stderr.lower())

    def test_wrong_number_of_fingerprints(self):
        def add_one(pathname, fp_bits):
            with pathname.open("a") as outf:
                print("1" * fp_bits, file=outf)

        completion = self._test_corrupt_fingerprints(add_one)
        self.assertTrue(
            "a shape fingerprint file must contain blocks of"
            in completion.stderr.lower()
        )

    def test_wrong_fingerprint_length(self):
        def alter_one_length(pathname, fp_bits):
            lines = pathname.read_text().splitlines()
            lines[0] += "101010"
            pathname.write_text("\n".join(lines))

        completion = self._test_corrupt_fingerprints(alter_one_length)
        self.assertTrue(
            "expected fingerprint of size" in completion.stderr.lower()
        )

    def test_invalid_fingerprint(self):
        def add_bogus_fp(pathname, _fp_bits):
            with pathname.open("a") as outf:
                print("I am not a fingerprint", file=outf)

        completion = self._test_corrupt_fingerprints(add_bogus_fp)
        self.assertTrue(
            "invalid fingerprint string" in completion.stderr.lower()
        )

    def _test_corrupt_fingerprints(self, corrupter_fn):
        fp_bits = 4
        with fp_file_generator.ShapeFPFileGenerator(fp_bits) as fp_gen:
            fp_filename = Path(fp_gen.pathname())
            corrupter_fn(fp_filename, fp_bits)
            with fp_filename.open("a") as outf:
                print("1" * fp_bits, file=outf)

            args = CmdLineArgs(
                measure=None,
                tversky_alpha=None,
                compute_similarity=True,
                search_index=fp_gen.num_fps() * 2,
                output_format="S",
                sparse_threshold=1.0,
                fingerprint_path=fp_filename,
            )
            completion = self._run_with_args(args)
            self.assertNotEqual(0, completion.returncode)
            return completion

    def test_search_index_out_of_bounds(self):
        with fp_file_generator.ShapeFPFileGenerator(4) as fp_gen:
            fp_filename = Path(fp_gen.pathname())
            args = CmdLineArgs(
                measure=None,
                tversky_alpha=None,
                compute_similarity=True,
                search_index=fp_gen.num_fps() * 2,
                output_format="S",
                sparse_threshold=1.0,
                fingerprint_path=fp_filename,
            )
            completion = self._run_with_args(args)
            self.assertNotEqual(0, completion.returncode)
            self.assertTrue(
                "must be less than number of fingerprints"
                in completion.stderr.lower()
            )

    def test_non_existent_fp_file(self):
        with fp_file_generator.ShapeFPFileGenerator(4) as fp_gen:
            fp_filename = Path(fp_gen.pathname() + ".non_existent")
            args = CmdLineArgs(
                measure=None,
                tversky_alpha=None,
                compute_similarity=True,
                search_index=fp_gen.num_fps() * 2,
                output_format="S",
                sparse_threshold=1.0,
                fingerprint_path=fp_filename,
            )
            completion = self._run_with_args(args)
            self.assertNotEqual(0, completion.returncode)
            self.assertTrue(
                "cannot open fingerprint file" in completion.stderr.lower()
            )

    def test_matrix_output(self):
        with fp_file_generator.ShapeFPFileGenerator(4) as fp_gen:
            _completion = self._test_matrix(None, fp_gen)

    def test_sparse_matrix_no_search(self):
        with fp_file_generator.ShapeFPFileGenerator(4) as fp_gen:
            fp_filename = Path(fp_gen.pathname())
            thresh = 1.0
            args = CmdLineArgs(
                measure="C",
                tversky_alpha=None,
                compute_similarity=True,
                search_index=None,
                output_format="S",
                sparse_threshold=thresh,
                fingerprint_path=fp_filename,
            )
            completion = self._run_with_args(args)
            self.assertEqual(0, completion.returncode)

            meas = shape_measure.Cosine()
            fpm = fp_measurer.FPSimMeasurer(
                meas, fp_gen.numbits(), fp_gen.fingerprints()
            )
            verifier = rv.SparseMatrix(fpm, None, thresh)

            inf = io.StringIO(completion.stdout)
            diffs = list(verifier.diffs(inf))
            if len(diffs) > 0:
                for diff in diffs[:10]:
                    print(diff)

            self.assertEqual(len(diffs), 0)

    def test_pvm_output(self):
        with fp_file_generator.ShapeFPFileGenerator(4) as fp_gen:
            fp_filename = Path(fp_gen.pathname())
            search_index = 4
            thresh = 0.25
            args = CmdLineArgs(
                measure="E",
                tversky_alpha=None,
                compute_similarity=False,
                search_index=search_index,
                output_format="P",
                sparse_threshold=thresh,
                fingerprint_path=fp_filename,
            )
            completion = self._run_with_args(args)
            self.assertEqual(0, completion.returncode)

            meas = shape_measure.Euclidean()
            fpm = fp_measurer.FPDistMeasurer(
                meas, fp_gen.numbits(), fp_gen.fingerprints()
            )
            verifier = rv.PVM(fpm, search_index, thresh)

            inf = io.StringIO(completion.stdout)
            diffs = list(verifier.diffs(inf))
            if len(diffs) > 0:
                for diff in diffs[:10]:
                    print(diff)

            self.assertEqual(len(diffs), 0)

    def test_pvm_bad_search_index(self):
        with fp_file_generator.ShapeFPFileGenerator(4) as fp_gen:
            fp_filename = Path(fp_gen.pathname())
            search_index = fp_gen.num_fps()
            thresh = 0.25
            args = CmdLineArgs(
                measure="E",
                tversky_alpha=None,
                compute_similarity=False,
                search_index=search_index,
                output_format="P",
                sparse_threshold=thresh,
                fingerprint_path=fp_filename,
            )
            completion = self._run_with_args(args)
            self.assertNotEqual(0, completion.returncode)
            self.assertTrue("must be less than" in completion.stderr.lower())

    def test_matrix_ignores_search(self):
        with fp_file_generator.ShapeFPFileGenerator(4) as fp_gen:
            completion = self._test_matrix(2, fp_gen)
            self.assertTrue("--search is ignored" in completion.stderr.lower())

    def test_sparse_matrix_with_search(self):
        with fp_file_generator.ShapeFPFileGenerator(4) as fp_gen:
            fp_filename = Path(fp_gen.pathname())
            thresh = 0.25
            search_index = 4
            args = CmdLineArgs(
                measure="C",
                tversky_alpha=None,
                compute_similarity=True,
                search_index=search_index,
                output_format="S",
                sparse_threshold=thresh,
                fingerprint_path=fp_filename,
            )
            completion = self._run_with_args(args)
            self.assertEqual(0, completion.returncode)

            meas = shape_measure.Cosine()
            fpm = fp_measurer.FPSimMeasurer(
                meas, fp_gen.numbits(), fp_gen.fingerprints()
            )
            # "SimSparseMatrx" - SparseMatrix, with a non-null search index
            verifier = rv.SimSparseMatrix(fpm, search_index, thresh)

            inf = io.StringIO(completion.stdout)
            diffs = list(verifier.diffs(inf))
            if len(diffs) > 0:
                for diff in diffs[:10]:
                    print(diff)

            self.assertEqual(len(diffs), 0)

    def test_pvm_default_search(self):
        """If no search value is given, it should default to zero."""
        with fp_file_generator.ShapeFPFileGenerator(4) as fp_gen:
            fp_filename = Path(fp_gen.pathname())
            thresh = 0.25
            search_index = None
            default_search_index = 0
            args = CmdLineArgs(
                measure="B",
                tversky_alpha=None,
                compute_similarity=True,
                search_index=search_index,
                output_format="P",
                sparse_threshold=thresh,
                fingerprint_path=fp_filename,
            )
            completion = self._run_with_args(args)
            self.assertEqual(0, completion.returncode)

            meas = shape_measure.Bub()
            fpm = fp_measurer.FPSimMeasurer(
                meas, fp_gen.numbits(), fp_gen.fingerprints()
            )
            verifier = rv.PVM(fpm, default_search_index, thresh)

            inf = io.StringIO(completion.stdout)
            diffs = list(verifier.diffs(inf))
            if len(diffs) > 0:
                for diff in diffs[:10]:
                    print(diff)

            self.assertEqual(len(diffs), 0)

    def _run_with_args(self, args: CmdLineArgs) -> subprocess.CompletedProcess:
        return subprocess.run(
            args.as_subprocess_args(), capture_output=True, encoding="utf8"
        )

    def _test_matrix(
        self,
        search_index: int | None,
        fp_gen: fp_file_generator.ShapeFPFileGenerator,
    ) -> subprocess.CompletedProcess:
        args = CmdLineArgs(
            measure=None,
            tversky_alpha=None,
            compute_similarity=True,
            search_index=search_index,
            output_format="M",
            sparse_threshold=None,
            fingerprint_path=Path(fp_gen.pathname()),
        )
        completion = self._run_with_args(args)

        meas = shape_measure.Tani()
        fpm = fp_measurer.FPSimMeasurer(
            meas, fp_gen.numbits(), fp_gen.fingerprints()
        )
        verifier = rv.Matrix(fpm, None, None)

        inf = io.StringIO(completion.stdout)
        diffs = list(verifier.diffs(inf))
        self.assertEqual(len(diffs), 0)
        return completion


def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()


if __name__ == "__main__":
    main()
