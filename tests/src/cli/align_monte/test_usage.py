"""Unit test for usage of align_features.
Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import logging
from pathlib import Path
import tempfile
import unittest

import base_test_case as btc
import config


# These tests are weak.  They simply confirm that you get a non-zero exit
# status for a variey of incorrect usages.
class TestCase(btc.TestCaseBase):
    def test_no_args(self):
        self._fail_align()

    def test_too_few_args(self):
        self._fail_align("foo.sdf")
        self._fail_align("foo.sdf", "hammersly.txt")

    def test_unsupported_option(self):
        completion = self._run("-o", "foo.sdf", "hamms.txt", "1.0")
        self.assertNotEqual(0, completion.returncode)
        self.assertTrue("usage" in completion.stderr.lower())

    def test_bad_scale(self):
        self._fail_align("foo.sdf", "hammersly.txt", "not-a-number")
        self._fail_align("f", "h", "0.9")
        self._fail_align("f", "h", "2.1")

    def test_missing_files(self):
        with tempfile.TemporaryDirectory() as dirname:
            temp_dir = Path(dirname)
            self._fail_align(
                temp_dir / "missing.sd",
                config.SHARED_TEST_DATA_DIR
                / "hammersley"
                / "hamm_spheroid_10k_11rad.txt",
                "1.0",
            )
            self._fail_align(
                config.SHARED_TEST_DATA_DIR / "cox2_3d.sd",
                temp_dir / "missing.txt",
                "1.0",
            )


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
