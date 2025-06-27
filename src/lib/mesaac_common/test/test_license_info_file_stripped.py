#!/usr/bin/env python2.6
# encoding: utf-8

"""Tests the license info file operations after stripping the license_info_file_cmd executable.

   We've just discovered a bug which causes a bus error, on Mac OS X 10.5, when
   running any program that links against libmesaac_common.a in the presence of
   a license file, provided the program is stripped.
   
   Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import sys
import os
import logging
import subprocess

import test_license_info_file_2 as base

base.LICENSE_INFO_FILE_CMD = os.environ.get("EXE", base._relpath("license_info_file_cmd_stripped"))
class TestCase(base.TestCase):
    pass

main = base.main

if __name__ == "__main__":
    main()
