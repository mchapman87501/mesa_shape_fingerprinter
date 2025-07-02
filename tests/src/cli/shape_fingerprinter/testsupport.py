#!/usr/bin/env python
# encoding: utf-8

"""Common imports, functions, etc. for unit tests.
Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import unittest
import logging

import cmake_paths


THIS_DIR = cmake_paths.CURRENT_SRC_DIR
RELPATH = THIS_DIR

DATA_PATH = THIS_DIR / "data" / "in"
REF_PATH = THIS_DIR / "data" / "ref"
SHARED_DATA_DIR = cmake_paths.WORKSPACE_ROOT / "tests" / "data"
SHAPE_FP_EXE = cmake_paths.SHAPE_FP_EXE


def gen_sd_records(inf):
    buff = []
    for line in inf:
        buff.append(line)
        if line.strip() == "$$$$":
            yield buff
            buff = []
    if buff:
        yield buff


def gen_sd_names(inf):
    for buff in gen_sd_records(inf):
        yield buff[0].strip()


def testmain():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
