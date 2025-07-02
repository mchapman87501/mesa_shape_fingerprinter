#!/usr/bin/env python
"""Basic tests for ShapeFingerprinter.
   Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""
from testsupport import *
import gzip
import zlib
import base64
import logging
import itertools
from cStringIO import StringIO

import pybel

class Error(Exception): pass

NUM_CLOUD_POINTS = 10240
NUM_FEATURES = 6
NUM_EXPECTED_BITS = NUM_CLOUD_POINTS * (NUM_FEATURES + 1)

# First conformers from the full cox2_3d:
COX2_CONFS = data_path("cox2_3d_first_few.sd")
SPHERE = shared_data_path("hammersley/hamm_spheroid_10k_11rad.txt")
ELLIPSE = shared_data_path("hammersley/hamm_ellipsoid_10k_11rad.txt")

class TestCase(unittest.TestCase):
    def _num_sd_structures(self, pathname):
        inf = open(pathname, "r")
        count = sum(1 for line in inf if line.strip() == "$$$$")
        inf.close()
        return count
        
    def _run(self, args):
        return run(args)  # From testsupport
        
    @timed
    def test_no_args(self):
        # Redirect stdout/stderr to prevent them appearing during builds.
        status, out, err = self._run([])
        self.assertNotEqual(0, status)
        
    @timed
    def test_help(self):
        for option in ["-h", "--help"]:
            status, out, err = self._run([option])
            self.assertEqual(0, status)
            err = err.lower()
            self.assertTrue("usage" in err)
            # Ensure all valid options appear in the help msg.
            for opt in "-h --help -i --id".split():
                self.assertTrue(opt in err, opt)

    def _line_diffs_acceptable(self, line_index, expected, actual, max_diffs):
        warn_once("Expected fingerprint data is incomplete.  Only the shape portion of each fingerprint is examined.")
        
        result = True
        diffs = 0
        prefix = "Line %s" % (line_index + 1)
        chars = []
        for e, a in zip(expected, actual):
            if e == a:
                chars.append(".")
            elif e < a:
                chars.append(">")  # Greater than expected
                diffs += 1
            else:
                chars.append("<")
                diffs += 1
        print("%s: %s" % (prefix, "".join(chars)))
        result = (diffs <= max_diffs)

        if len(expected) != len(actual):
            print("%s: Expected length %s, actual length %s" % 
                  (prefix, len(expected), len(actual)))
            result = False
        return result
            
    def _differences_acceptable(self, expected, actual, max_diffs_per_fp):
        result = True
        if len(expected) != len(actual):
            print("Expected %s lines, got %s" % (len(expected), len(actual)))
            result = False
        # Compare as many lines as possible:
        for i, (eline, aline) in enumerate(zip(expected, actual)):
            # OBS!  Expected fingerprint data is incomplete just now --
            # I have expected bits only for the shape portion of the FP.
            aline = aline[-NUM_CLOUD_POINTS:]
        
            if eline != aline:
                result = result and self._line_diffs_acceptable(
                    i, eline, aline, max_diffs_per_fp)

        return result
                
    def _run_cox2(self, options=None, sphere=None):
        sd_pathname = COX2_CONFS
        if sphere is None:
            sphere = SPHERE

        args = (options or []) + [sd_pathname, sphere, "1.0"]
        return self._run(args) + (sd_pathname, sphere)
        
    def _run_cox2_ell(self, options=None):
        return self._run_cox2(options=(["-e", ELLIPSE] + (options or [])))
        
    def _verify_fp_basics(self, lines, sd_pathname):
        # Expect all fingerprints to have the same length:
        expectedFPLen = NUM_EXPECTED_BITS
        failures = []
        for i, line in enumerate(lines):
            if len(line) != expectedFPLen:
                failures.append(i + 1)
        self.assertFalse(
            failures, 
            ("These fingerprints did not have the expected length: %s" %
             failures))
        
        fps_per_struct = 4
        self.assertEqual(len(lines), 
                         fps_per_struct * self._num_sd_structures(sd_pathname))
    
    def _get_cox2_fps(self):
        inf = gzip.open(ref_path("cox2_3d_first_few.fp.txt.gz"))
        result = [l.strip() for l in inf]
        inf.close()
        return result
        
    def _compare_fp_lines(self, expected, actual):
        if actual != expected:
            # Allow up to <small number> fingerprint bit discrepancies 
            # per line before failing.
            max_discrepancies_per_conf = 1
            if not self._differences_acceptable(
                expected, actual, max_discrepancies_per_conf):
                self.fail("Actual fingerprints had too many discrepancies")
        
    def _verify_cox2_fps(self, lines, sd_pathname):
        self._verify_fp_basics(lines, sd_pathname)

        # To generate new reference output:
        # outf = gzip.open(ref_path("cox2_3d_first_few.fp.txt.gz"), "w")
        # outf.write_lines(lines)
        # outf.close()
        
        expected = self._get_cox2_fps()
        self._compare_fp_lines(expected, lines)
            
    def _verify_cox2_ids(self, actual_ids, sd_pathname, 
                         expected_copies=4, logger=None):
        result = True
        logger = logger or logging.getLogger()
                         
        expected_ids = []
        with open(sd_pathname) as inf:
            for eid in gen_sd_names(inf):
                expected_ids.extend([eid] * expected_copies)
                
        if expected_ids != actual_ids:
            logger.error("Did not get expected IDs")
            if len(expected_ids) != len(actual_ids):
                logger.error("Expected %s IDs, got %s" % 
                             (len(expected_ids), len(actual_ids)))
            for i, (eid, aid) in enumerate(zip(expected_ids, actual_ids)):
                if eid != aid:
                    logger.error("Line %d:  Expected '%s', actual '%s'" %
                                 (i + 1, eid, aid))
            result = False
        return result
        
    def _verify_fp_basics(self, lines, sd_pathname):
        # Expect all fingerprints to have the same length:
        expectedFPLen = NUM_EXPECTED_BITS
        failures = []
        for i, line in enumerate(lines):
            if len(line) != expectedFPLen:
                failures.append(i + 1)
        self.assertFalse(
            failures, 
            ("These fingerprints did not have the expected length: %s" %
             failures))
        
        fps_per_struct = 4
        self.assertEqual(len(lines), 
                         fps_per_struct * self._num_sd_structures(sd_pathname))

    @timed
    def test_correct_usage_with_ellipsoid(self):
        ellipse = shared_data_path("hammersley/hamm_ellipsoid_10k_11rad.txt")
        for eopt in ["-e", "--ellipsoid"]:
            options = [eopt, ellipse]
            status, out, err, sd_pathname, sphere = self._run_cox2(
                options=options)
            self.assertEqual(0, status)
        
            lines = [l.strip() for l in out.splitlines()]
            self._verify_cox2_fps(lines, sd_pathname)
        
    @timed
    def test_correct_usage(self):
        status, out, err, sd_pathname, sphere = self._run_cox2()
        self.assertEqual(0, status)
        
        lines = [l.strip() for l in out.splitlines()]
        # Can't verify the actual fingerprints.  Just verify their
        # sizes, I guess.
        self._verify_fp_basics(lines, sd_pathname)
            
    @timed
    def test_invalid_option(self):
        for option in ["-j", "--junk"]:
            status, out, err, u1, u2 = self._run_cox2(options=[option])
            self.assertNotEqual(0, status)
            self.assertTrue("unsupported" in err.lower())
            
    @timed
    def test_with_ids(self):
        for option in ["-i", "--id"]:
            status, out, err, sd_pathname, sph = self._run_cox2_ell([option])
            if status:
                logging.error("test_with_ids -- error output:")
                logging.error(err)
            self.assertEqual(0, status)
            lines = [l.strip() for l in out.splitlines()]
            ids = []
            fps = []
            for i, curr_line in enumerate(lines):
                fields = curr_line.split()
                self.assertEqual(2, len(fields))
                fps.append(fields[0])
                ids.append(fields[1])
            self._verify_cox2_fps(fps, sd_pathname)
            self.assertTrue(self._verify_cox2_ids(ids, sd_pathname))
            # Just to help ensure the positive test is doing something...
            logger = logging.getLogger("silent")
            logger.setLevel(logging.CRITICAL)
            self.assertFalse(self._verify_cox2_ids(ids[-20:], sd_pathname, logger=logger))
            ids[len(ids) / 3] += " -- 1/3rd error"
            self.assertFalse(self._verify_cox2_ids(ids, sd_pathname, logger=logger))
        
    @timed
    def test_compressed_output(self):
        decode = base64.b64decode
        gzfile = gzip.GzipFile
        
        for format_flag in ["-f", "--format"]:
            options = [format_flag, "C", "--id"]
            status, out, err, sd_pathname, sph = self._run_cox2_ell(options)
            self.assertEqual(0, status)
            lines = [l.strip() for l in out.splitlines()]
            ids = []
            fps = []
            for i, curr_line in enumerate(lines):
                fields = curr_line.split()
                self.assertEqual(2, len(fields))
                compressed_fp = fields[0]
                # FP must start w. the format flag: "C" => "compressed"
                self.assertTrue(compressed_fp.startswith("C"))
                gzipped = decode(compressed_fp[1:])
                fp = gzfile('', 'r', 1, StringIO(gzipped)).read()
                fps.append(fp)
                ids.append(fields[1])
            self._verify_cox2_fps(fps, sd_pathname)
            self.assertTrue(self._verify_cox2_ids(ids, sd_pathname))
            # Just to help ensure the positive test is doing something...
            logger = logging.getLogger("silent")
            logger.setLevel(logging.CRITICAL)
            self.assertFalse(self._verify_cox2_ids(ids[-20:], sd_pathname, logger=logger))
            ids[len(ids) / 3] += " -- 1/3rd error"
            self.assertFalse(self._verify_cox2_ids(ids, sd_pathname, logger=logger))
            
    # Positive tests:  try to confirm that feature fingerprints are
    # correct at least some of the time.
    def _test_feature_bits(self, smiles, feature_index):
        sphere = shared_data_path("hammersley/hamm_spheroid_1k_11rad.txt")
        NUM_CLOUD_POINTS = 1024
        NUM_FEATURES = 6
        NUM_EXPECTED_BITS = NUM_CLOUD_POINTS * (NUM_FEATURES + 1)

        mol = pybel.readstring("smi", smiles)
        # Assign arbitrary coords.
        for i, atom in enumerate(mol.atoms):
            atom.OBAtom.SetVector(i*2.0, 0.0, 0.0)
        outf, pathname = tempf()
        
        try:
            outf.write(mol.write("sdf"))
            outf.flush()

            args = [pathname, sphere, "1.0"] 
            status, out, err = run(args)
        finally:
            outf.close()
        
        if 0 != status:
            logging.error("Exit status {} for {}".format(status, args))
            logging.error("Stderr:")
            logging.error(err)
        self.assertEqual(0, status)
        fullfp = out.splitlines()[0].rstrip()
        
        chunks = []
        fullfp_chunks = fullfp
        while fullfp_chunks:
            chunks.append(fullfp_chunks[:NUM_CLOUD_POINTS])
            fullfp_chunks = fullfp_chunks[NUM_CLOUD_POINTS:]
        
        self.assertEqual(NUM_EXPECTED_BITS, len(fullfp))
        shape = chunks[-1]
        feature_chunks = chunks[:-1]
        # Features are stacked up backwards, in front of the shape chunk.
        feature = feature_chunks[-(1 + feature_index)]

        msg = "Feature {}:\n{}\n!=\n{}\n:\n{}".format(feature_index, shape, feature, "\n".join(chunks))
        self.assertEqual(shape, feature, msg)
        
    @timed
    def test_hbond_acceptor(self):
        self._test_feature_bits("C=O", 0)
        
    @timed
    def test_hbond_donor(self):
        # This entire structure matches as a hydrogen-bond donor.
        self._test_feature_bits("O", 1)
        
    @timed
    def test_charged(self):
        self._test_feature_bits("[O+]", 2)
        
    @timed
    def test_4ring(self):
        logging.warn("TBD: Test for 4-membered aromatic rings")
        
    @timed
    def test_5ring(self):
        self._test_feature_bits("n1cccc1", 4)
        
    @timed
    def test_6ring(self):
        self._test_feature_bits("c1ccccc1", 5)
        
    @timed
    def test_records_option(self):
        sd_pathname = COX2_CONFS
        num_confs = self._num_sd_structures(sd_pathname)
        record_flags = itertools.cycle(["-r", "--records"])
        for num_records in [0, 10, num_confs]:
            for start_index in [0, 15, 50]:
                if start_index < num_confs:
                    end_index = min(start_index + num_records, num_confs)
                    record_flag = next(record_flags)
                    # My ref results were generated using an ellipsoid.
                    args = [record_flag, str(start_index), str(end_index), 
                            "-e", ELLIPSE,
                            sd_pathname, SPHERE, "1.0"]
                    status, out, err = self._run(args)
                    self.assertEqual(0, status)
                    actual = out.splitlines()
                    actual_count = len(actual) // 4
                    expected_count = end_index - start_index
                    if expected_count != actual_count:
                        logging.error("Wrong # records for %s..%s" %
                                      (start_index, end_index))
                    self.assertEqual(expected_count, actual_count)
                    
                    expected = self._get_cox2_fps()[start_index*4:end_index*4]
                    self._compare_fp_lines(expected, actual)
                    
    @timed
    def test_invalid_records_option(self):
        sd_pathname = COX2_CONFS
        num_confs = self._num_sd_structures(sd_pathname)
        record_flags = itertools.cycle(["-r", "--records"])
        for start_index in [-10, -1]:
            # Try to get all records, starting w. start_index
            args = [next(record_flags), str(start_index), "-1", 
                    sd_pathname, SPHERE, "1.0"]
            status, out, err = self._run(args)
            self.assertNotEqual(0, status)
            
    def _get_folder(self, full_len, num_folds):
        # Brute-force double-checking of folded fingerprints.
        NUM_FEATURES = 6
        full_stride = full_len // (NUM_FEATURES + 1)
        
        plies = 1 << num_folds
        folded_size = full_len // plies
        folded_stride = full_stride // plies
        
        def folder(fpstr):
            result = ["0"] * folded_size
            # fpstr is divided into NUM_FEATURES segments
            for i, bit in enumerate(fpstr):
                if bit == "1":
                    segment, index = divmod(i, full_stride)
                    folded_index = index % folded_stride
                    result[(segment * folded_stride) + folded_index] = "1"
            return "".join(result)
        return folder
        
    @timed
    def test_folder(self):
        # Make sure the folder is working correctly.
        full_len = 28
        num_folds = 1
        folder = self._get_folder(full_len, num_folds)

        src = "0101" "0101" "0100" "0001" "0101" "0000" "0100"
        folded = folder(src)
        self.assertEqual(14, len(folded))
        self.assertEqual("01" "01" "01" "01" "01" "00" "01", folded)
        
    @timed
    def test_folding(self):
        sphere = shared_data_path("hammersley/hamm_spheroid_1k_11rad.txt")
        status, out, err, sd_pathname, sph = self._run_cox2(sphere=sphere)
        self.assertEqual(0, status)
        unfolded = [l.strip() for l in out.splitlines()]
        full_len = len(unfolded[0])
        
        fold_flags = itertools.cycle(["-n", "--num_folds"])
        for num_folds in range(4):
            options = [next(fold_flags), str(num_folds)]
            # logging.debug("Fold options: %s" % options)
            status, out, err, sdp, sph = self._run_cox2(
                options=options, sphere=sphere)
            folded = [l.strip() for l in out.splitlines()]
            self.assertEqual(len(unfolded), len(folded))

            do_fold = self._get_folder(full_len, num_folds)
            for i, (u, f) in enumerate(itertools.izip(unfolded, folded)):
                uf = do_fold(u)
                self.assertEqual(len(uf), len(f))
                if uf != f:
                    logging.error("Did not get expected fp[%s] for %s folds" %
                                  (i, num_folds))
                    ufi = int(uf, 2)
                    fi = int(f, 2)
                    diffi = ufi ^ fi
                    logging.error("Bit mismatches: %s" % bin(diffi)[2:])
                    self.fail()


if __name__ == "__main__":
    testmain()
