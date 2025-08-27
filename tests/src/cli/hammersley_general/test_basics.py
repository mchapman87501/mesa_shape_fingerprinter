#!/usr/bin/env python
"""Basic tests for ShapeFingerprinter.
Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import logging
import subprocess
import unittest
import typing as tp

import config


class TestCase(unittest.TestCase):
    def test_no_args(self):
        """Test the exit status when no args are given."""
        completion = self._run()
        self.assertNotEqual(0, completion.returncode)

    def test_help(self):
        """Test that usage output is generated."""
        for option in ["-h", "--help"]:
            with self.subTest(option=option):
                completion = self._run(option)
                self.assertEqual(0, completion.returncode)
                err = completion.stderr.lower()
                self.assertTrue("usage" in err)
                # Ensure all valid options appear in the help msg.
                for opt in "-h --help".split():
                    self.assertTrue(opt in err, opt)

    def test_correct_usage(self):
        """Run and compare output against reference output."""

        completion = self._run(3, 50)
        if completion.returncode != 0:
            print("Failed correct usage.")
            print("=" * 72)
            print("stdout:")
            print(completion.stdout)
            print("=" * 72)
            print("stderr:")
            print(completion.stderr)

        self.assertEqual(0, completion.returncode)

        actual = completion.stdout
        expected = config.HAMM_REF_PATH.read_text()

        self._compare_lines(actual, expected, "hammersley_general 3 50")

    def test_invalid_option(self):
        """Test expected response to use of (example) unsupported options."""
        for option in ["-j", "--junk"]:
            with self.subTest(option=option):
                completion = self._run(option)
                self.assertNotEqual(0, completion.returncode)
                self.assertTrue("usage:" in completion.stderr.lower())

    def test_out_of_range_dimension(self):
        for dimension in [-3, 0, 50]:
            with self.subTest(dimension=dimension):
                completion = self._run(dimension, 10)
                self.assertNotEqual(0, completion.returncode)
                self.assertTrue("dimension" in completion.stderr.lower())

    def test_missing_args(self):
        """Test invocation with missing required arguments."""
        completion = self._run(15)
        self.assertNotEqual(completion.returncode, 0)
        self.assertTrue("usage" in completion.stderr.lower())

    def _run(self, *args: tp.Any):
        arg_list = [str(config.EXE_PATH)] + [str(v) for v in args]
        return subprocess.run(arg_list, capture_output=True, encoding="utf8")

    def _compare_lines(
        self, actual: str, expected: str, description: str
    ) -> None:
        act_lines = actual.splitlines()
        exp_lines = expected.splitlines()

        diff_count = 0
        for i, (act_line, exp_line) in enumerate(
            zip(act_lines, exp_lines), start=1
        ):
            if act_line != exp_line:
                if diff_count == 0:
                    print(72 * "=")
                    print(description)
                diff_count += 1
                if diff_count > 10:
                    print("Differences truncated.")
                    break
                print(f"Line {i} differs:")
                e_stripped = exp_line.rstrip("\r\n")
                a_stripped = act_line.rstrip("\r\n")
                print(f"E: {e_stripped}")
                print(f"A: {a_stripped}")

        if len(act_lines) != len(exp_lines):
            print(
                f"Expected number of lines: {len(exp_lines)}; actual {len(act_lines)}"
            )


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
