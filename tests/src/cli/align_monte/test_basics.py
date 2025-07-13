"""Unit test for basic usage of align_features.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import logging
import subprocess
import unittest
from pathlib import Path

import base_test_case as btc
import config


class TestCase(btc.TestCaseBase):
    SMALL_SD_PATH = (
        config.SHARED_TEST_DATA_DIR / "sd_files" / "cox2_3d_first_5.sd"
    )

    def test_help(self):
        for option in ["-h", "--help"]:
            completion = self._run(option)
            self.assertEqual(0, completion.returncode)
            err = completion.stderr.lower()
            self.assertTrue("usage" in err)
            # Ensure all valid options appear in the help msg.
            for (
                opt
            ) in "-h --help -a --atom-centers -s --sort -m --measure".split():
                self.assertTrue(opt in err, opt)

    def test_same_shape_max_tani_align(self):
        self._test_align(config.TEST_DATA_DIR / "same_shape.sdf", 0.928)

    def test_same_shape_atom_centers_only(self):
        # Same shape should align exactly, when using only atom centers:
        for options in ["-a", "--atom-centers"]:
            self._test_align(
                config.TEST_DATA_DIR / "same_shape.sdf", 1.0, [options]
            )

    def test_cox2_3d_align(self):
        self._test_align(config.TEST_DATA_DIR / "cox2_3d.sd", 0.383)

    def test_cox2_3d_align_atom_centers_only(self):
        self._test_align(config.TEST_DATA_DIR / "cox2_3d.sd", 0.383, ["-a"])

    def test_herg_actives(self):
        self._test_align(
            config.TEST_DATA_DIR / "herg_actives_cluster_132.sd", 0.24
        )

    def test_herg_actives_align_atom_centers_only(self):
        self._test_align(
            config.TEST_DATA_DIR / "herg_actives_cluster_132.sd", 0.24, ["-a"]
        )

    def test_with_sort_file(self):
        sort_file_name = "best_alignments.csv"

        best_sort_path = config.OUT_DATA_DIR / sort_file_name
        self._test_align(
            config.TEST_DATA_DIR / "cox2_3d.sd",
            0.383,
            ["-s", str(best_sort_path)],
        )
        self.assertTrue(best_sort_path.exists())
        # TODO Verify that
        # - best_sort_path contains valid CSV data,
        # - the number of rows corresponds to the number of
        # structures in cox2_3d.sd
        # - the measure column's values are in descending order.

    def test_non_tversky_measures(self):
        for measure in ["B", "T", "C"]:
            for measure_case in [measure, measure.lower()]:
                with self.subTest(measure=measure_case):
                    completion = self._run_align(
                        self.SMALL_SD_PATH,
                        ["-m", measure_case],
                    )
                    self.assertEqual(0, completion.returncode)

    def test_invalid_measure(self):
        completion = self._run_align(
            self.SMALL_SD_PATH, ["--measure", "invalid"]
        )
        self.assertNotEqual(0, completion.returncode)
        if "usage" not in completion.stderr.lower():
            self._show_unexpected_completion("test_invalid_measure", completion)
        self.assertTrue("usage" in completion.stderr.lower())

    def test_duplicate_measure(self):
        for measure in ["B", "T", "C"]:
            with self.subTest(measure=measure):
                completion = self._run_align(
                    self.SMALL_SD_PATH,
                    ["-m", measure, "--measure", measure],
                )
                self.assertEqual(0, completion.returncode)
                self.assertTrue("multiple times" in completion.stderr.lower())

    def test_duplicate_tversky(self):
        completion = self._run_align(
            self.SMALL_SD_PATH,
            ["-m", "V", "0.1", "-m", "V", "0.5"],
        )
        if completion.returncode != 0:
            self._show_unexpected_completion(
                "test_duplicate_tversky", completion
            )
        self.assertEqual(0, completion.returncode)
        self.assertTrue(
            "last-specified tversky alpha" in completion.stderr.lower()
        )

    def test_non_numeric_tversky_alpha(self):
        completion = self._run_align(
            self.SMALL_SD_PATH,
            ["-m", "V", "not a number"],
        )
        self.assertNotEqual(0, completion.returncode)
        self.assertTrue("could not convert alpha" in completion.stderr.lower())

    def test_out_of_range_tversky_alpha(self):
        for alpha in [-0.01, 2.01]:
            with self.subTest(alpha=alpha):
                completion = self._run_align(
                    self.SMALL_SD_PATH,
                    ["-m", "V", alpha],
                )
                self.assertNotEqual(0, completion.returncode)
                self.assertTrue(
                    "must be in the range" in completion.stderr.lower()
                )

    def test_missing_measure(self):
        completion = self._run_align(self.SMALL_SD_PATH, ["-m"])
        self.assertNotEqual(0, completion.returncode)
        # This *should* produce a "no value specified" error, but due to
        # weak command-line processing it is more likely to show "Unknown measure".
        err = completion.stderr.lower()
        if not ("no value specified" in err or "unknown measure" in err):
            self._show_unexpected_completion("test_missing_measure", completion)
            self.fail("Did not see expected error message")

    def _test_align(self, sd_pathname: Path, min_tani: float, options=None):
        options = options or []
        completion = self._run_align(sd_pathname, options)
        if completion.returncode != 0:
            self._show_unexpected_completion("_test_align", completion)
        self.assertEqual(completion.returncode, 0)

        # Save a copy of the output for future reference.
        out_dir = config.OUT_DATA_DIR
        opts_str = "_".join(options + [""])
        filename = "aligned_" + opts_str + sd_pathname.name
        cleaned_filename = (
            filename.replace("/", "_").replace("-", "_").replace("\\", "_")
        )
        actual_path = out_dir / cleaned_filename
        actual_path.write_text(completion.stdout)

        mean, sdev = btc.get_max_tani_stats(actual_path)
        logging.debug(
            f"Stats for {actual_path.name}: {mean:.3f} +/- {sdev:.3f}"
        )
        self.assertTrue(
            mean >= min_tani,
            msg=f"Mean tanimoto = {mean}, expected {min_tani}",
        )

    def _run_align(
        self, sd_pathname: Path, options=None
    ) -> subprocess.CompletedProcess:
        hamm_sphere = (
            config.SHARED_TEST_DATA_DIR
            / "hammersley"
            / "hamm_spheroid_20k_11rad.txt"
        )
        options = options or []
        args = options + [str(sd_pathname), str(hamm_sphere), "1.0"]
        return self._run(*args)

    def _show_unexpected_completion(
        self, test_name: str, completion: subprocess.CompletedProcess
    ) -> None:
        print(f"""DEBUG: Unexpected completion for {test_name}:
Args: {completion.args}
Stderr:
{completion.stderr}""")


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
