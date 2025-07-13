#!/usr/bin/env python
"""Basic tests for shape_fingerprinter.
Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import base64
import gzip
import io
import itertools
import logging
import struct
import subprocess
import unittest

import config

# First conformers from the full cox2_3d:
COX2_CONFS = config.SHARED_DATA_DIR / "sd_files" / "cox2_3d_first_few.sd"
COX2_FPS = config.SHAPE_FP_DATA_DIR / "ref_cox2_3d_first_few.fp.txt.gz"
SPHERE = config.SHARED_DATA_DIR / "hammersley" / "hamm_spheroid_10k_11rad.txt"
ELLIPSE = config.SHARED_DATA_DIR / "hammersley" / "hamm_ellipsoid_10k_11rad.txt"


class TestCase(unittest.TestCase):
    def test_help(self):
        """Test that usage output is generated."""
        for option in ["-h", "--help"]:
            completion = self._run([option])
            self.assertEqual(0, completion.returncode)
            err = completion.stderr.lower()
            self.assertTrue("usage" in err)
            # Ensure all valid options appear in the help msg.
            for opt in "-h --help -i --id".split():
                self.assertTrue(opt in err, opt)

    def test_correct_usage_with_ellipsoid(self):
        """Test generating ellipsoid fingerprints, against reference output."""
        for eopt in ["-e", "--ellipsoid"]:
            options = [eopt, ELLIPSE]
            completion, sd_pathname, sphere = self._run_cox2(options)
            self.assertEqual(0, completion.returncode)

            lines = [line.strip() for line in completion.stdout.splitlines()]
            self._verify_cox2_fps(lines, sd_pathname)

    def test_correct_usage(self):
        """Test generating fingerprints, with valid invocation."""
        completion, sd_pathname, sphere = self._run_cox2()
        self.assertEqual(0, completion.returncode)

        lines = [line.strip() for line in completion.stdout.splitlines()]
        # Can't verify the actual fingerprints.  Just verify their
        # sizes, I guess.
        self._verify_fp_basics(lines, sd_pathname)

    def test_invalid_option(self):
        """Test expected response to use of (example) unsupported options."""
        for option in ["-j", "--junk"]:
            completion, _u1, _u2 = self._run_cox2([option])
            self.assertNotEqual(0, completion.returncode)
            self.assertTrue("unknown flag/option" in completion.stderr.lower())

    def test_invalid_atom_scale(self):
        """Test expected response to use of invalid atom_scale."""
        for atom_scale in ["not_a_float", "-1.0"]:
            args = [COX2_CONFS, SPHERE, atom_scale]
            with self.subTest(args=args):
                completion = self._run(args)
                self.assertNotEqual(0, completion.returncode)
                self.assertTrue("atom_scale" in completion.stderr.lower())

    def test_with_ids(self):
        """Test generation of fingerprints with associated structure IDs."""
        for option in ["-i", "--id"]:
            completion, sd_pathname, _sph = self._run_cox2_ell([option])
            self.assertEqual(0, completion.returncode)
            ids, fps = self._get_ids_and_fps(completion.stdout)
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

    def test_compressed_ascii_output(self):
        """Test compressed ASCII output, with IDs."""
        for format_flag in ["-f", "--format"]:
            options = [format_flag, "C", "--id"]
            completion, sd_pathname, _sph = self._run_cox2_ell(options)
            self.assertEqual(0, completion.returncode)
            ids, fps = self._get_ids_and_cbinascii_fps(completion.stdout)
            self._verify_cox2_fps(fps, sd_pathname)
            self.assertTrue(self._verify_cox2_ids(ids, sd_pathname))

    def test_binary_output(self):
        """Test binary output, with IDs."""
        for format_flag in ["-f", "--format"]:
            options = [format_flag, "B", "--id"]
            completion, sd_pathname, _sph = self._run_cox2_ell(options)
            status = completion.returncode
            if status != 0:
                print("DEBUG: stderr:")
                print(completion.stderr)
            self.assertEqual(0, status)
            ids, fps = self._get_ids_and_binary_fps(completion.stdout)
            self._verify_cox2_fps(fps, sd_pathname)
            self.assertTrue(self._verify_cox2_ids(ids, sd_pathname))

    def test_invalid_format(self):
        """Verify expected behavior for unsupported formats."""
        for format_flag in ["-f", "--format"]:
            options = [format_flag, "INVALID"]
            with self.subTest(options=options):
                completion, _path, _sph = self._run_cox2(options)
                self.assertNotEqual(0, completion.returncode)
                # Error msg always uses the long name of the option.
                has_expected_text = (
                    "Invalid choice for '--format'" in completion.stderr
                )
                if not has_expected_text:
                    print("DEBUG: test_invalid_format.  Actual stderr:")
                    print(completion.stderr)
                self.assertTrue(has_expected_text)

    def test_records_option(self):
        """Test processing of specific SD file records/structures."""
        sd_pathname = COX2_CONFS
        num_confs = self._num_sd_structures(sd_pathname)
        record_flags = itertools.cycle(["-r", "--records"])
        for num_records in [0, 10, num_confs]:
            for start_index in [0, 10, 15]:
                if start_index < num_confs:
                    end_index = min(start_index + num_records, num_confs)
                    record_flag = next(record_flags)
                    args = (start_index, end_index, record_flag)
                    with self.subTest(args=args):
                        self._records_subtest(
                            start_index, end_index, record_flag
                        )

    def test_invalid_records_option(self):
        """Test processing of specific, invalid SD file records/structures."""
        sd_pathname = COX2_CONFS
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
            with self.subTest(args=args):
                completion = self._run(args)
                self.assertNotEqual(0, completion.returncode)

    def test_folding(self):
        """Test generating folded fingerprints."""
        # It should be enough to test ASCII output.
        completion, _sd_pathname, _sph = self._run_cox2()
        self.assertEqual(0, completion.returncode)
        unfolded = [line.strip() for line in completion.stdout.splitlines()]
        full_len = len(unfolded[0])

        fold_flags = itertools.cycle(["-n", "--num_folds"])
        for num_folds in [0, 1, 4]:
            options = [next(fold_flags), str(num_folds)]
            with self.subTest(options=options):
                completion, _sdp, _sph = self._run_cox2(options)
                folded = [
                    line.strip() for line in completion.stdout.splitlines()
                ]
                self.assertEqual(len(unfolded), len(folded))

                do_fold = self._get_folder(full_len, num_folds)
                for u, f in zip(unfolded, folded):
                    self.assertEqual(do_fold(u), f)

    def test_missing_num_folds(self):
        """Verify expected behavior when number of folds is not given."""
        options = ["-n"]
        completion = self._run(options)
        self.assertNotEqual(0, completion.returncode)
        self.assertTrue("requires 1" in completion.stderr.lower())

    def _get_ids_and_fps(self, fp_output: str) -> tuple[list, list]:
        ids = []
        fps = []
        for curr_line in fp_output.splitlines():
            fields = curr_line.strip().split()
            self.assertEqual(2, len(fields))
            fps.append(fields[0])
            ids.append(fields[1])
        return ids, fps

    def _get_ids_and_cbinascii_fps(self, fp_output: str) -> tuple[list, list]:
        ids = []
        fps = []
        for curr_line in fp_output.splitlines():
            fields = curr_line.strip().split()
            self.assertEqual(2, len(fields))
            cbinascii_content, fp_id = fields
            fps.append(self._get_cbinascii_fp(cbinascii_content))
            ids.append(fp_id)
        return (ids, fps)

    def _get_cbinascii_fp(self, cbinascii_content: str) -> str:
        # FP must start w. the format flag: "C" => "compressed"
        self.assertTrue(cbinascii_content.startswith("C"))
        return self._decode_b64_gzipped(cbinascii_content[1:]).decode("utf8")

    def _get_ids_and_binary_fps(self, fp_output: str) -> tuple[list, list]:
        ids = []
        fps = []
        for curr_line in fp_output.splitlines():
            fields = curr_line.strip().split()
            self.assertEqual(2, len(fields))
            compressed_fp, fp_id = fields
            fps.append(self._get_compressed_fp(compressed_fp))
            ids.append(fp_id)
        return (ids, fps)

    def _get_compressed_fp(self, compressed_content: str) -> str:
        # FP must start w. the format flag: "B" => "binary"
        self.assertTrue(compressed_content.startswith("B"))

        # The remainder of the FP is a B64 encoded sequence of bytes, B1.
        # B64-decoding B1 produces a gzip-compressed sequence of bytes, B2.
        # Decompressing B2 produces a sequence of boost::dynamic_bitset blocks.
        # By default, each block is an unsigned long. From the boost docs:
        # The first block written represents the bits in the position range
        # [0,bits_per_block) in the bitset, the second block written the bits
        # in the range [bits_pre_block,2*bits_per_block), and so on.
        # For each block bval written, the bit (bval >> i) & 1 corresponds to
        # the bit at position (b * bits_per_block + i) in the bitset.
        bytes = self._decode_b64_gzipped(compressed_content[1:])
        fp_bits = []
        ULONG_FMT = "<L"  # Little-endian, unsigned long
        block_size = struct.calcsize(ULONG_FMT)
        for block_tuple in struct.iter_unpack(ULONG_FMT, bytes):
            block = block_tuple[0]
            for bit_pos in range(8 * block_size):
                curr_bit = (block >> bit_pos) & 1
                fp_bits.append(str(curr_bit))
        return "".join(reversed(fp_bits))

    def _decode_b64_gzipped(self, content: str):
        gzipped = base64.b64decode(content)
        buf = io.BytesIO(gzipped)
        inf = gzip.GzipFile(mode="r", compresslevel=0, fileobj=buf)
        result = inf.read()
        inf.close()
        return result

    def _records_subtest(self, start_index, end_index, record_flag):
        # My ref results were generated using an ellipsoid.
        args = [
            record_flag,
            str(start_index),
            str(end_index),
            "-e",
            ELLIPSE,
            COX2_CONFS,
            SPHERE,
            "1.0",
        ]
        completion = self._run(args)
        self.assertEqual(0, completion.returncode)
        actual = completion.stdout.splitlines()
        actual_count = len(actual) // 4  # 4 lines per record
        expected_count = end_index - start_index
        if expected_count != actual_count:
            logging.error(f"Wrong # records for {start_index}..{end_index}")
        self.assertEqual(expected_count, actual_count)

        expected = self._get_cox2_fps()[start_index * 4 : end_index * 4]
        self._compare_fp_lines(expected, actual)

    def _num_sd_structures(self, pathname):
        with open(pathname) as inf:
            return sum(1 for line in inf if line.strip() == "$$$$")

    def _run(self, args: list[str]):
        args = [str(config.SHAPE_FP_EXE)] + [str(arg) for arg in args]
        return subprocess.run(args, capture_output=True, encoding="utf8")

    def _run_cox2(self, options=None):
        args = (options or []) + [COX2_CONFS, SPHERE, "1.0"]
        return (self._run(args), COX2_CONFS, SPHERE)

    def _run_cox2_ell(self, options=None):
        return self._run_cox2(["-e", ELLIPSE] + (options or []))

    def _line_diffs_acceptable(self, line_index, expected, actual, max_diffs):
        diffs = 0
        prefix = f"Line {line_index + 1}"
        chars = []
        for e, a in zip(expected, actual):
            chars.append("." if e == a else (">" if e < a else "<"))
            diffs += 0 if e == a else 1
        result = diffs <= max_diffs

        if diffs:
            print(f"{prefix}: {''.join(chars)}")

        if len(expected) != len(actual):
            print(
                f"{prefix}: Expected length {len(expected)}, actual length {len(actual)}"
            )
            result = False
        return result

    def _differences_acceptable(self, expected, actual, max_diffs_per_fp):
        result = True
        if len(expected) != len(actual):
            print(f"Expected {len(expected)} lines, got {len(actual)}")
            result = False
        # Compare as many lines as possible:
        for i, (eline, aline) in enumerate(zip(expected, actual)):
            if eline != aline:
                result = result and self._line_diffs_acceptable(
                    i, eline, aline, max_diffs_per_fp
                )

        return result

    def _get_cox2_fps(self):
        with gzip.open(COX2_FPS) as inf:
            raw = inf.read()
            return [line for line in raw.decode("utf8").splitlines()]

    def _compare_fp_lines(self, expected, actual):
        if actual != expected:
            # Allow up to <small number> fingerprint bit discrepancies
            # per line before failing.
            if not self._differences_acceptable(expected, actual, 1):
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
            for eid in config.gen_sd_names(inf):
                expected_ids.extend([eid] * expected_copies)

        if expected_ids != actual_ids:
            logger.error("Did not get expected IDs")
            if len(expected_ids) != len(actual_ids):
                logger.error(
                    f"Expected {len(expected_ids)} IDs, got {len(actual_ids)}"
                )
            for i, (eid, aid) in enumerate(zip(expected_ids, actual_ids)):
                if eid != aid:
                    logger.error(
                        "Line {i + 1}:  Expected '{eid}', actual '{aid}'"
                    )
            result = False
        return result

    def _verify_fp_basics(self, lines, sd_pathname):
        # Expect all fingerprints to have the same length:
        expected_fp_len = 10240
        failures = []
        for i, line in enumerate(lines):
            if len(line) != expected_fp_len:
                failures.append(i + 1)
        self.assertFalse(
            failures,
            f"These fingerprints did not have the expected length: {failures}",
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
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
