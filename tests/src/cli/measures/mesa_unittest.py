#!/usr/bin/env python
# encoding: utf-8

"""Extends Python's stdlib unittest facilities.
Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

from __future__ import with_statement

import sys
import os
import subprocess
import unittest
import logging
import difflib

_thisdir = os.path.abspath(os.path.dirname(__file__))


def _relpath(p):
    return os.path.abspath(os.path.join(_thisdir, p))


exe = os.environ.get("EXE", "EXE_NOT_PROVIDED")


class TestCase(unittest.TestCase):
    def _run(self, *args, **kw):
        global exe
        args = [exe] + list(args)
        # print("Trying to run %s" % str(args))
        verbose = kw.get("verbose", False)
        if verbose:
            logging.debug("run%s" % repr(args))
        completion = subprocess.run(args, capture_output=True, encoding="utf8")
        return completion.returncode, completion.stdout, completion.stderr

    def _get_line_count(self, pathname):
        with open(pathname) as inf:
            return sum(1 for line in inf)

    def _get_content(self, pathname):
        with open(pathname) as inf:
            return inf.read()

    def _log_error(self, msg):
        sys.stderr.write(msg + "\n")

    def _log_error_lines(self, lines):
        log = self._log_error
        for line in lines:
            log(line.rstrip())

    def _log_diffs(
        self,
        expected,
        actual,
        expected_pathname,
        actual_pathname="<actual output>",
    ):
        diff = difflib.unified_diff(
            expected.splitlines(),
            actual.splitlines(),
            expected_pathname,
            actual_pathname,
        )
        self._log_error_lines(diff)


main = unittest.main
