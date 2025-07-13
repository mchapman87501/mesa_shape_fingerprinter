#!/usr/bin/env python
"""Unit test for find_diverse.
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

find_diverse = os.environ.get("EXE", _relpath("../find_diverse"))

def run(args, **kw):
    args = [find_diverse] + args
    kwargs = dict(stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    kwargs.update(kw)
    p = subprocess.Popen(args, **kwargs)
    out, err = p.communicate()
    return p.returncode, out, err
    
class TestCase(unittest.TestCase):
    def test_usage(self):
        status, out, err = run([])
        self.assertNotEqual(0, status)
        self.assertTrue("usage" in err.lower())
        
    def test_bad_target_name(self):
        targetpath = "no_such_file.txt"
        dbpath = _relpath("data/in/database.fp.txt")
        thresh = "0.7"
        status, out, err = run([targetpath, dbpath, thresh])
        self.assertNotEqual(0, status)
        self.assertTrue(targetpath in err)
        
    def test_bad_db_name(self):
        targetpath = _relpath("data/in/targets.fp.txt")
        dbpath = "no_such_file.txt"
        thresh = "0.7"
        status, out, err = run([targetpath, dbpath, thresh])
        self.assertNotEqual(0, status)
        self.assertTrue(dbpath in err)
        
    def test_non_numeric_thresh(self):
        targetpath = _relpath("data/in/targets.fp.txt")
        dbpath = _relpath("data/in/database.fp.txt")
        thresh = "foo"
        status, out, err = run([targetpath, dbpath, thresh])
        self.assertNotEqual(0, status)
        self.assertTrue(thresh in err)
    
    def test_thresh_small(self):
        targetpath = _relpath("data/in/targets.fp.txt")
        dbpath = _relpath("data/in/database.fp.txt")
        thresh = "-0.1"
        status, out, err = run([targetpath, dbpath, thresh])
        self.assertNotEqual(0, status)
        self.assertTrue(thresh in err)
    
    def test_thresh_large(self):
        targetpath = _relpath("data/in/targets.fp.txt")
        dbpath = _relpath("data/in/database.fp.txt")
        thresh = "2.1"
        status, out, err = run([targetpath, dbpath, thresh])
        self.assertNotEqual(0, status)
        self.assertTrue(thresh in err)
        
    def test_good_1(self):
        targetpath = _relpath("data/in/targets.fp.txt")
        dbpath = _relpath("data/in/database.fp.txt")
        results = []
        for thresh in [0.2, 0.3, 0.4]:
            status, out, err = run([targetpath, dbpath, str(thresh)])
            self.assertEqual(0, status)
            indices = [int(i) for i in out.splitlines()]
            results.append(indices)
        self.assertEqual(results, [[1], [1, 2], [1, 2, 3]])
    

def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()

if __name__ == "__main__":
    main()
