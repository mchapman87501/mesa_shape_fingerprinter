#!/usr/bin/env python
"""Unit test for ShapeVolume.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import unittest
import logging
import subprocess
from pathlib import Path
import tempfile

import config


class TestCase(unittest.TestCase):
    def test_usage(self):
        """Verify that help is shown when no args are given."""
        completion = self._run()
        self.assertNotEqual(0, completion.returncode)
        self.assertTrue("usage" in completion.stderr.lower())

    def test_nonexistent_sdf_pathname(self):
        """Verify expected behavior when the given SD file does not exist."""

        # TODO: test behavior when sdf_path does exist, but doesn't have read permissions.
        with tempfile.TemporaryDirectory() as tmp_dir_name:
            sdf_pathname = Path(tmp_dir_name) / "no_such_sdf.nope"
            spheroid_pathname = (
                config.HAMMS_DATA_DIR / "hamm_spheroid_20k_11rad.txt"
            )

            completion = self._run(sdf_pathname, spheroid_pathname, 11.0, 1.0)
            self.assertNotEqual(0, completion.returncode)
            self.assertTrue("Could not open SD file" in completion.stderr)

    def test_nonexistent_spheroid_pathname(self):
        """Verify expected behavior when the given spheroid file doesn't exist."""
        with tempfile.TemporaryDirectory() as tmp_dir_name:
            sdf_pathname = config.MY_TEST_DATA_DIR / "concoction.sdf"
            spheroid_pathname = Path(tmp_dir_name) / "hamm_spheroid.txt"

            completion = self._run(sdf_pathname, spheroid_pathname, 11.0, 1.0)
            self.assertNotEqual(0, completion.returncode)
            self.assertTrue(
                "Could not open sphere points file" in completion.stderr
            )

    def test_valid_args(self):
        """Verify expected results for known inputs."""
        sdf_pathname = config.MY_TEST_DATA_DIR / "concoction.sdf"
        spheroid_pathname = (
            config.HAMMS_DATA_DIR / "hamm_spheroid_20k_11rad.txt"
        )

        completion = self._run(
            sdf_pathname,
            spheroid_pathname,
            # Not sure about the radius with which the sphere set was
            # created.
            11.0,
            1.0,
        )
        if completion.returncode != 0:
            print(completion.stderr)
        self.assertEqual(0, completion.returncode)

        # The concocted molecule is a series of 6 non-overlapping carbons,
        # in various conformations, all arranged to fit within the
        # hammersley sphere radius of 11.0 angstroms.
        # 4/3*pi*(6*r[carbon]**3)
        # 4.0/3.0*pi*(6.0*(1.7**3))
        expected = 123.477

        for line in completion.stdout.splitlines():
            actual = float(line.strip())
            self.assert_max_error(expected, actual, 6.0e-3)
            error_fract = self._error_fract(expected, actual)
            print(f"{actual:.3f} error = {error_fract:.3f}")

    def _run(self, *args, **kw):
        args = [config.EXE_PATH] + [str(arg) for arg in args]
        return subprocess.run(args, **kw, capture_output=True, encoding="utf8")

    def assert_max_error(
        self, expected: float, actual: float, max_fraction: float
    ) -> None:
        error = self._error_fract(expected, actual)
        self.assertLessEqual(
            error,
            max_fraction,
            f"Max error fraction: expected {expected}, actual {actual}, error {error}",
        )

    def _error_fract(self, expected: float, actual: float) -> float:
        return abs(expected - actual) / expected


def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()


if __name__ == "__main__":
    main()
