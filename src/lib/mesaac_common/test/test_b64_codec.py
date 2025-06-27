#!/usr/bin/env python
"""Unit test for the B64 codec.
   Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""
import sys
import os
import unittest
import logging
import subprocess
import base64
import zlib
import tempfile

_thisdir = os.path.abspath(os.path.dirname(__file__))
def _relpath(p):
    return os.path.abspath(os.path.join(_thisdir, p))

EXE = os.environ.get("EXE", _relpath("b64_stdin"))

class TestCase(unittest.TestCase):
    def _run(self, test_string):
        fd, pathname = tempfile.mkstemp()
        outf = os.fdopen(fd, "wb")
        outf.write(test_string)
        outf.close()
        with open(pathname, "rb") as inf:
            p = subprocess.Popen([EXE], stdin=inf, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
        out, err = p.communicate()
        os.remove(pathname)
        return p.returncode, out, err
        
    def test_known_input(self):
        inp = "01" * 5120
        status, out, err = self._run(inp)
        self.assertEqual(0, status)
        out = out.rstrip()
        exp = base64.b64encode(inp)
        self.assertEqual(exp, out)
        
    def test_compressed_input(self):
        inp = "01" * 5120
        inp = zlib.compress(inp)
        status, out, err = self._run(inp)
        self.assertEqual(0, status)
        out = out.rstrip()
        exp = base64.b64encode(inp)
        self.assertEqual(exp, out)
    

def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()

if __name__ == "__main__":
    main()
