#!/usr/bin/env python
# encoding: utf-8

"""Measure the time needed to generate a set of fingerprints, 
   largely ignoring correctness of results.
   Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import time
from testsupport import *
import unittest

class TestCase(unittest.TestCase):
    def _run(self):
        sdPathname = data_path("cox2_3d_first_few.sd")
        sphere = shared_data_path("hammersley/hamm_spheroid_10k_11rad.txt")
        ellipse = shared_data_path("hammersley/hamm_ellipsoid_10k_11rad.txt")
        args = [exePath, sdPathname, ellipse, sphere, "1.0"]
        p = subprocess.Popen(args,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate()
        if p.returncode:
            print 72 * "="
            print("gdb {0}".format(args[0]))
            print(" ".join(["run"] + args[1:]))
        return p.returncode, out, err

    def test_timed(self):
        t0 = time.time()
        status, out, err = self._run()
        tf = time.time()
        if status:
            logging.error(err)
            self.assertEqual(0, status)
        logging.debug("Runtime: {:.2f}".format(tf - t0))

def main():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
    
if __name__ == "__main__":
    main()
