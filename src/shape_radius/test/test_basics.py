#!/usr/bin/env python
# encoding: utf-8

"""Basic tests for ShapeFingerprinter.
   Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
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
exePath = os.environ.get("EXE", _relpath("../ShapeRadius"))

def testData(p):
    return os.path.join(_relpath("../../../test_data"), p)
    
class TestCase(unittest.TestCase):
    def setUp(self):
        pass
        
    # OBS: from Python 3.2 onward this will be built in to unittest.TestCase.
    def _almostEqual(self, a, b, epsilon):
        return (abs(a - b) <= epsilon)
        
    def assertAlmostEqual(self, a, b, epsilon=1.0e-6):
        self.assertTrue(self._almostEqual(a, b, epsilon), 
                        "{0} almost equals {1}".format(a, b))
        
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

        
    def _numSDStructures(self, pathname):
        inf = open(pathname, "r")
        count = sum(1 for line in inf if line.strip() == "$$$$")
        inf.close()
        return count
        
    def test_no_args(self):
        # Redirect stdout/stderr to prevent them appearing during builds.
        status = subprocess.call([exePath], stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
        self.assertNotEqual(0, status)
        
    def test_correct_usage(self):
        sdPathname = _relpath("data/in/concoction.sdf")
        p = subprocess.Popen(
            [exePath, sdPathname, 
             testData("hammersley/hamm_spheroid_20k_11rad.txt"), "1.0"],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()
        status = p.returncode
        if (0 != status):
            logging.error(stderr)
        self.assertEqual(0, status)

        # Output should be a header line and a single line of shape extents.
        header, values = stdout.splitlines()
        # Don't worry about the header content.
        actual = [float(f) for f in values.strip().split()]
        xmax, ymax, zmax, max_atom_radius, max_r_plus_atom_radius = actual
        r = math.sqrt(xmax**2 + ymax**2 + zmax**2)
        max_radial = r + max_atom_radius
        self.assertAlmostEqual(max_r_plus_atom_radius, max_radial)

        # TODO:  Check that xmax, ymax, zmax and max_atom_radius are
        # correct, based on SD coords.
        expected = [6, 0, 0, 1.7, 7.7]
        
        # Big sampling errors in max extent computations -- don't know why.
        self.assertListAlmostEqual(expected, actual, 0.02)
        
def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()

if __name__ == "__main__":
    main()
