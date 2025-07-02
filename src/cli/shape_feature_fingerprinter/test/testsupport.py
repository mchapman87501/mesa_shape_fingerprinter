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
import tempfile
import functools
import time

thisdir = os.path.abspath(os.path.dirname(__file__))
def relpath(p):
    return os.path.abspath(os.path.join(thisdir, p))
    
def data_path(p):
    return relpath(os.path.join("data", "in", p))
    
def ref_path(p):
    return relpath(os.path.join("data", "ref", p))

def shared_data_path(p):
    return relpath(os.path.join("../../../test_data", p))

exePath = os.environ.get("EXE", relpath("../shape_feature_fingerprinter"))

def tempf(mode="w"):
    fd, pathname = tempfile.mkstemp()
    fileobj = os.fdopen(fd, mode)
    # Can you assign to the name of a file object?  Probably not...
    return fileobj, pathname
    
def run(args, **kw):
    global exePath
    args = [exePath] + list(args)
    libPath = os.path.dirname(os.path.abspath(exePath))
    env = {}
    env.update(os.environ)
    for varname in ["DYLD_LIBRARY_PATH", "LD_LIBRARY_PATH"]:
        env[varname] = os.pathsep.join([
            libPath, env.get(varname, "")
        ])
    kwArgs = {}
    kwArgs.update(kw)
    kwArgs.update(dict(
        env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    ))
    p = subprocess.Popen(args, **kwArgs)
    out, err = p.communicate()
    return p.returncode, out, err

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
        
_warnings = {}
def warn_once(msg):
    global _warnings
    if _warnings.setdefault(msg, 0) < 1:
        logging.warn(msg)
    _warnings[msg] += 1
    
def timed(fn):
    @functools.wraps(fn)
    def wrapper(*args, **kw):
        t0 = time.time()
        result = fn(*args, **kw)
        tf = time.time()
        logging.debug("%s: %.2f seconds" % (fn.__name__, tf - t0))
        return result
    return wrapper


def testmain():
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
