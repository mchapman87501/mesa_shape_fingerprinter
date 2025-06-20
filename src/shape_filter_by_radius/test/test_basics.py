#!/usr/bin/env python
# encoding: utf-8

"""Basic tests for shape_filter_by_radius.
   Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""
import sys
import os
from os.path import abspath, dirname, join
import unittest
import logging
import subprocess
import re
import math

_thisdir = abspath(dirname(__file__))
def _relpath(p):
    return abspath(os.path.join(_thisdir, p))
    
# If the test framework provides the exe path, use it.
exe_path = os.environ.get("EXE", _relpath("../shape_filter_by_radius"))

def test_data(p):
    return os.path.join(_relpath("../../../test_data"), p)
    
class TestCase(unittest.TestCase):
    def setUp(self):
        pass
        
    def _runcmd(self, args, **kwargs):
        args = [exe_path] + args
        kw = dict(
            stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        kw.update(kwargs)
        p = subprocess.Popen(args, **kw)
        out, err = p.communicate()
        return p.returncode, out, err
        
    def _almostEqual(self, a, b, epsilon):
        return (abs(a - b) <= epsilon)
        
    # OBS: Use of assertAlmostEqual requires Python 2.7
    def assertListAlmostEqual(self, a, b, epsilon=1.0e-6):
        self.assertEqual(len(a), len(b), "Lists have different lengths: {0}, {1}".format(a, b))
        summary = []
        almost = self._almostEqual
        for a_value, b_value in zip(a, b):
            if almost(a_value, b_value, epsilon):
                summary.append(".")
            else:
                summary.append("X")
        self.assertTrue(
            "X" not in summary,
            "assertListAlmostEqual:\n{0}\n{1}\n{2}".format(
                a, b, "".join(summary)))

        
    def test_no_args(self):
        status, out, err = self._runcmd([])
        self.assertNotEqual(0, status)
        
    def test_no_such_file(self):
        status, out, err = self._runcmd(["/tmp/no_such_file.sdf"])
        self.assertNotEqual(0, status)
        # Requires Python 2.7+
        self.assertIn("Cannot open", err)
        
    def test_atom_scale_validation(self):
        sd_pathname = _relpath("data/in/concoction.sdf")
        max_rad = "12.0"
        for atom_scale in [-1.0, 0.0, 1.0, 2.0]:
            status, out, err = self._runcmd(
                [sd_pathname, str(atom_scale), max_rad])
            if atom_scale <= 0:
                self.assertNotEqual(0, status)
                self.assertIn("atom_scale", err)
            else:
                self.assertEqual(0, status)
        
    def test_max_radius_validation(self):
        sd_pathname = _relpath("data/in/concoction.sdf")
        atom_scale = "1.0"
        for max_rad in [-4, 0, 0.1, 4.0, 12.0, 22.0]:
            status, out, err = self._runcmd([sd_pathname, atom_scale, 
                                             str(max_rad)])
            if max_rad <= 0:
                self.assertNotEqual(0, status)
                self.assertIn("max_radius", err)
            else:
                self.assertEqual(0, status)
        
    def test_max_radii(self):
        sd_pathname = _relpath("data/in/concoction.sdf")
        atom_scale = "1.0"
        RADIUS_N = 1.55
        RADIUS_C = 1.7 # Carbon radius
        CONCOCTED_DIAMETER = 12.0 + RADIUS_N + RADIUS_C
        CONCOCTED_RADIUS = CONCOCTED_DIAMETER / 2.0
        for max_rad in [0.5, 4.0, 12.0, 15.0, 22.0]:
            status, out, err = self._runcmd([sd_pathname, atom_scale,
                                             str(max_rad)])
            if status:
                logging.error("For max_rad {}: {}".format(max_rad, err))
            self.assertEqual(0, status)
            num_confs = out.count("$$$$")
            expected_num_confs = 2 if max_rad > CONCOCTED_RADIUS else 0
            self.assertEqual(expected_num_confs, num_confs,
                "Wrong # confs for radius {}: {} != {}".format(
                    max_rad, expected_num_confs, num_confs))
        
        
def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()

if __name__ == "__main__":
    main()
