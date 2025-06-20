#!/usr/bin/env python
"""Unit test for ShapeVolume.
   Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""
import sys
import os
import unittest
import logging
import subprocess

_thisdir = os.path.abspath(os.path.dirname(__file__))
def _relpath(p):
    return os.path.abspath(os.path.join(_thisdir, p))


exePath = os.environ.get("EXE", _relpath("../shape_volume"))

class TestCase(unittest.TestCase):
    def _run(self, *args, **kw):
        global exePath
        
        kwargs = {}
        kwargs.update(kw)
        kwargs['stdout'] = kwargs['stderr'] = subprocess.PIPE
        args = [exePath] + list(args)
        p = subprocess.Popen(args, **kwargs)
        out, err = p.communicate()
        status = p.returncode
        return (status, out, err)
        
    # OBS: from Python 3.2 onward this will be built in to unittest.TestCase.
    def _almostEqual(self, a, b, epsilon):
        return (abs(a - b) <= epsilon)
        
    def assertAlmostEqual(self, a, b, epsilon=1.0e-6):
        self.assertTrue(self._almostEqual(a, b, epsilon), 
                        "{0} almost equals {1}".format(a, b))
                        
    def _errorFract(self, expected, actual):
        e = float(expected)
        result = abs(e - actual) / e
        return result
        
    def assertMaxError(self, expected, actual, maxFraction):
        error = self._errorFract(expected, actual)
        self.assertTrue(error <= maxFraction,
            "Max error fraction: expected {0}, actual {1}, error {2}".format(
            expected, actual, error))
        
    def test_usage(self):
        status, out, err = self._run()
        self.assertNotEqual(0, status)
        self.assertTrue("usage" in err.lower())
        
    def test_valid_args(self):
        dataDir = _relpath("../../../test_data")
        def dataPath(p):
            return os.path.join(dataDir, p)
            
        status, out, err = self._run(
            _relpath("data/in/concoction.sdf"),
            dataPath("hammersley/hamm_spheroid_20k_11rad.txt"),
            # Not sure about the radius with which the sphere set was
            # created.
            "11.0", "1.0"
        )
        if status:
            print(err)
        self.assertEqual(0, status)
        
        # The concocted molecule is a series of 6 non-overlapping carbons,
        # in various conformations, all arranged to fit within the 
        # hammersley sphere radius of 11.0 angstroms.
        # 4/3*pi*(6*r[carbon]**3)
        # 4.0/3.0*pi*(6.0*(1.7**3))
        expected = 123.477
        
        for line in out.splitlines():
            actual = float(line.strip())
            self.assertMaxError(expected, actual, 6.0e-3)
            print("{0:.3f} error = {1:.3f}".format(actual, self._errorFract(expected, actual)))
        
def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()

if __name__ == "__main__":
    main()
