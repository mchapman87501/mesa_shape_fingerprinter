#!/usr/bin/env python
"""Basic tests for ShapeFingerprinter.
Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import unittest
import testsupport as tsupp
import gzip
import base64
import logging
import io
import itertools
import subprocess

# First conformers from the full cox2_3d:
COX2_CONFS = tsupp.DATA_PATH / "cox2_3d_first_few.sd"
COX2_FPS = tsupp.REF_PATH / "cox2_3d_first_few.fp.txt.gz"
SPHERE = tsupp.SHARED_DATA_DIR / "hammersley" / "hamm_spheroid_10k_11rad.txt"
ELLIPSE = tsupp.SHARED_DATA_DIR / "hammersley" / "hamm_ellipsoid_10k_11rad.txt"


class Error(Exception):
    pass


class TestCase(unittest.TestCase):
    def test_no_args(self):
        # Redirect stdout/stderr to prevent them appearing during builds.
        status, out, err = self._run([])
        self.assertNotEqual(0, status)

    def test_help(self):
        for option in ["-h", "--help"]:
            status, out, err = self._run([option])
            self.assertEqual(0, status)
            err = err.lower()
            self.assertTrue("usage" in err)
            # Ensure all valid options appear in the help msg.
            for opt in "-h --help -i --id".split():
                self.assertTrue(opt in err, opt)

    def test_correct_usage_with_ellipsoid(self):
        for eopt in ["-e", "--ellipsoid"]:
            options = [eopt, ELLIPSE]
            status, out, err, sd_pathname, sphere = self._run_cox2(options)
            self.assertEqual(0, status)

            lines = [line.strip() for line in out.splitlines()]
            self._verify_cox2_fps(lines, sd_pathname)

    def test_correct_usage(self):
        status, out, err, sd_pathname, sphere = self._run_cox2()
        self.assertEqual(0, status)

        lines = [line.strip() for line in out.splitlines()]
        # Can't verify the actual fingerprints.  Just verify their
        # sizes, I guess.
        self._verify_fp_basics(lines, sd_pathname)

    def test_invalid_option(self):
        for option in ["-j", "--junk"]:
            status, out, err, u1, u2 = self._run_cox2([option])
            self.assertNotEqual(0, status)
            self.assertTrue("unsupported" in err.lower())

    def test_with_ids(self):
        for option in ["-i", "--id"]:
            status, out, err, sd_pathname, sph = self._run_cox2_ell([option])
            self.assertEqual(0, status)
            lines = [line.strip() for line in out.splitlines()]
            ids = []
            fps = []
            for curr_line in lines:
                fields = curr_line.split()
                self.assertEqual(2, len(fields))
                fps.append(fields[0])
                ids.append(fields[1])
            self._verify_cox2_fps(fps, sd_pathname)
            self.assertTrue(self._verify_cox2_ids(ids, sd_pathname))
            # Just to help ensure the positive test is doing something...
            logger = logging.getLogger("silent")
            logger.setLevel(logging.CRITICAL)
            self.assertFalse(
                self._verify_cox2_ids(ids[-20:], sd_pathname, logger=logger)
            )
            ids[len(ids) // 3] += " -- 1/3rd error"
            self.assertFalse(
                self._verify_cox2_ids(ids, sd_pathname, logger=logger)
            )

    def test_compressed_output(self):
        for format_flag in ["-f", "--format"]:
            options = [format_flag, "C", "--id"]
            status, out, err, sd_pathname, sph = self._run_cox2_ell(options)
            self.assertEqual(0, status)
            lines = [line.strip() for line in out.splitlines()]
            ids = []
            fps = []
            for i, curr_line in enumerate(lines):
                fields = curr_line.split()
                self.assertEqual(2, len(fields))
                compressed_fp = fields[0]
                # FP must start w. the format flag: "C" => "compressed"
                self.assertTrue(compressed_fp.startswith("C"))
                gzipped = base64.b64decode(compressed_fp[1:])
                # Get the fingerprints:
                fp = (
                    gzip.GzipFile(
                        mode="r",
                        compresslevel=0,
                        fileobj=io.BytesIO(gzipped),
                    )
                    .read()
                    .decode("utf8")
                )
                fps.append(fp)
                ids.append(fields[1])
            self._verify_cox2_fps(fps, sd_pathname)
            self.assertTrue(self._verify_cox2_ids(ids, sd_pathname))
            # Just to help ensure the positive test is doing something...
            logger = logging.getLogger("silent")
            logger.setLevel(logging.CRITICAL)
            self.assertFalse(
                self._verify_cox2_ids(ids[-20:], sd_pathname, logger=logger)
            )
            ids[len(ids) // 3] += " -- 1/3rd error"
            self.assertFalse(
                self._verify_cox2_ids(ids, sd_pathname, logger=logger)
            )

    def test_records_option(self):
        sd_pathname = COX2_CONFS
        num_confs = self._num_sd_structures(sd_pathname)
        record_flags = itertools.cycle(["-r", "--records"])
        for num_records in [0, 10, num_confs]:
            for start_index in [0, 10, 15]:
                if start_index < num_confs:
                    end_index = min(start_index + num_records, num_confs)
                    record_flag = next(record_flags)
                    # My ref results were generated using an ellipsoid.
                    args = [
                        record_flag,
                        str(start_index),
                        str(end_index),
                        "-e",
                        ELLIPSE,
                        sd_pathname,
                        SPHERE,
                        "1.0",
                    ]
                    status, out, err = self._run(args)
                    self.assertEqual(0, status)
                    actual = out.splitlines()
                    actual_count = len(actual) // 4
                    expected_count = end_index - start_index
                    if expected_count != actual_count:
                        logging.error(
                            "Wrong # records for %s..%s"
                            % (start_index, end_index)
                        )
                    self.assertEqual(expected_count, actual_count)

                    expected = self._get_cox2_fps()[
                        start_index * 4 : end_index * 4
                    ]
                    self._compare_fp_lines(expected, actual)

    def test_invalid_records_option(self):
        sd_pathname = COX2_CONFS
        _num_confs = self._num_sd_structures(sd_pathname)
        record_flags = itertools.cycle(["-r", "--records"])
        for start_index in [-10, -1]:
            # Try to get all records, starting w. start_index
            args = [
                next(record_flags),
                str(start_index),
                "-1",
                sd_pathname,
                SPHERE,
                "1.0",
            ]
            status, out, err = self._run(args)
            self.assertNotEqual(0, status)

    def test_folding(self):
        # It should be enough to test ASCII output.
        status, out, err, sd_pathname, sph = self._run_cox2()
        self.assertEqual(0, status)
        unfolded = [l.strip() for l in out.splitlines()]
        full_len = len(unfolded[0])

        fold_flags = itertools.cycle(["-n", "--num_folds"])
        for num_folds in range(4):
            options = [next(fold_flags), str(num_folds)]
            # logging.debug("Fold options: %s" % options)
            status, out, err, sdp, sph = self._run_cox2(options)
            folded = [l.strip() for l in out.splitlines()]
            self.assertEqual(len(unfolded), len(folded))

            do_fold = self._get_folder(full_len, num_folds)
            for u, f in zip(unfolded, folded):
                self.assertEqual(do_fold(u), f)

    def _num_sd_structures(self, pathname):
        inf = open(pathname, "r")
        count = sum(1 for line in inf if line.strip() == "$$$$")
        inf.close()
        return count

    def _run(self, args):
        args = [str(tsupp.SHAPE_FP_EXE)] + list(args)
        result = subprocess.run(args, capture_output=True, encoding="utf8")
        return result.returncode, result.stdout, result.stderr

    def _line_diffs_acceptable(self, line_index, expected, actual, max_diffs):
        result = True
        diffs = 0
        prefix = "Line %s" % (line_index + 1)
        chars = []
        for e, a in zip(expected, actual):
            print(f"DEBUG: Compare {repr(e)} vs. {repr(a)}")
            if e == a:
                chars.append(".")
            elif e < a:
                chars.append(">")  # Greater than expected
                diffs += 1
            else:
                chars.append("<")
                diffs += 1
        print("%s: %s" % (prefix, "".join(chars)))
        result = diffs <= max_diffs

        if len(expected) != len(actual):
            print(
                "%s: Expected length %s, actual length %s"
                % (prefix, len(expected), len(actual))
            )
            result = False
        return result

    def _differences_acceptable(self, expected, actual, max_diffs_per_fp):
        result = True
        if len(expected) != len(actual):
            print("Expected %s lines, got %s" % (len(expected), len(actual)))
            result = False
        # Compare as many lines as possible:
        for i, (eline, aline) in enumerate(zip(expected, actual)):
            if eline != aline:
                result = result and self._line_diffs_acceptable(
                    i, eline, aline, max_diffs_per_fp
                )

        return result

    def _run_cox2(self, options=None):
        args = (options or []) + [COX2_CONFS, SPHERE, "1.0"]
        return self._run(args) + (COX2_CONFS, SPHERE)

    def _run_cox2_ell(self, options=None):
        return self._run_cox2(["-e", ELLIPSE] + (options or []))

    def _get_cox2_fps(self):
        inf = gzip.open(tsupp.REF_PATH / "cox2_3d_first_few.fp.txt.gz")
        raw = inf.read()
        result = [line for line in raw.decode("utf8").splitlines()]
        inf.close()
        return result

    def _compare_fp_lines(self, expected, actual):
        if actual != expected:
            # Allow up to <small number> fingerprint bit discrepancies
            # per line before failing.
            max_discrepancies_per_conf = 1
            if not self._differences_acceptable(
                expected, actual, max_discrepancies_per_conf
            ):
                self.fail("Actual fingerprints had too many discrepancies")

    def _verify_cox2_fps(self, lines, sd_pathname):
        self._verify_fp_basics(lines, sd_pathname)

        # To generate new reference output:
        # outf = gzip.open(COX2_FPS, "w")
        # outf.write_lines(lines)
        # outf.close()

        expected = self._get_cox2_fps()
        self._compare_fp_lines(expected, lines)

    def _verify_cox2_ids(
        self, actual_ids, sd_pathname, expected_copies=4, logger=None
    ):
        result = True
        logger = logger or logging.getLogger()

        expected_ids = []
        with open(sd_pathname) as inf:
            for eid in tsupp.gen_sd_names(inf):
                expected_ids.extend([eid] * expected_copies)

        if expected_ids != actual_ids:
            logger.error("Did not get expected IDs")
            if len(expected_ids) != len(actual_ids):
                logger.error(
                    "Expected %s IDs, got %s"
                    % (len(expected_ids), len(actual_ids))
                )
            for i, (eid, aid) in enumerate(zip(expected_ids, actual_ids)):
                if eid != aid:
                    logger.error(
                        "Line %d:  Expected '%s', actual '%s'"
                        % (i + 1, eid, aid)
                    )
            result = False
        return result

    def _verify_fp_basics(self, lines, sd_pathname):
        # Expect all fingerprints to have the same length:
        expectedFPLen = 10240
        failures = []
        for i, line in enumerate(lines):
            if len(line) != expectedFPLen:
                failures.append(i + 1)
        self.assertFalse(
            failures,
            (
                "These fingerprints did not have the expected length: %s"
                % failures
            ),
        )

        fps_per_struct = 4
        self.assertEqual(
            len(lines), fps_per_struct * self._num_sd_structures(sd_pathname)
        )

    def _get_folder(self, full_len, num_folds):
        # Brute-force double-checking of folded fingerprints.
        folded_size = full_len // (1 << num_folds)

        def folder(fpstr):
            result = ["0"] * folded_size
            for i, bit in enumerate(fpstr):
                if bit == "1":
                    result[i % folded_size] = "1"
            return "".join(result)

        return folder


if __name__ == "__main__":
    tsupp.testmain()
