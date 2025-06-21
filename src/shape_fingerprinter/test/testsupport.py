#!/usr/bin/env python
# encoding: utf-8

"""Common imports, functions, etc. for unit tests.
   Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""
import sys
import os
import unittest
import logging
import subprocess
import re
import math

thisdir = os.path.abspath(os.path.dirname(__file__))
def relpath(p):
    return os.path.abspath(os.path.join(thisdir, p))
    
def data_path(p):
    return relpath(os.path.join("data", "in", p))
    
def ref_path(p):
    return relpath(os.path.join("data", "ref", p))

def shared_data_path(p):
    return relpath(os.path.join("../../../test_data", p))

exePath = os.environ.get("EXE", relpath("../shape_fingerprinter"))


def gen_sd_records(inf):
    buff = []
    for line in inf:
        buff.append(line)
        if (line.strip() == "$$$$"):
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
