"""Common imports, functions, etc. for unit tests.
Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import math
import re
import subprocess
import sys
import typing as tp
import unittest
from pathlib import Path

import config


def get_max_tani_scores(sd_pathname: Path) -> list[float]:
    """Get values of MaxAlignTanimoto tags from an SD file."""
    expr = re.compile(r"MaxAlignTanimoto>\n(.*)$", re.M)
    content = sd_pathname.read_text()
    all = re.findall(expr, content)
    return [float(m) for m in all]


def get_stats(scores: list[float]) -> tuple[float, float]:
    if not scores:
        raise ValueError("Can't compute stats for an empty scores list")
    num_scores = len(scores)
    s = sum(scores)
    mean = s / num_scores
    sum_sqr_err = sum((v - mean) ** 2 for v in scores)
    sdev = math.sqrt(sum_sqr_err) / (num_scores - 1)
    return mean, sdev


def get_max_tani_stats(sd_pathname: Path) -> tuple[float, float]:
    scores = get_max_tani_scores(sd_pathname)
    return get_stats(scores)


class TestCaseBase(unittest.TestCase):
    def _run(self, *args: tp.Any) -> subprocess.CompletedProcess:
        args = [str(config.EXE)] + [str(arg) for arg in args]
        try:
            return subprocess.run(args, capture_output=True, encoding="utf8")
        except UnicodeDecodeError as info:
            print(f"_run failed for {' '.join(args)}: {info}", file=sys.stderr)
            raise

    def _align(self, *args: tp.Any) -> None:
        completion = self._run(*args)
        if completion.returncode != 0:
            raise subprocess.CalledProcessError(completion.returncode, args)

    def _fail_align(self, *args):
        def align():
            self._align(*args)

        self.assertRaises(subprocess.CalledProcessError, align)
