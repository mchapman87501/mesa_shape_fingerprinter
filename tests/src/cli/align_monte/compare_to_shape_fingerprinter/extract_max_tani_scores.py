#!/usr/bin/env python
# encoding: utf-8

import sys
import optparse
import os

_thisdir = os.path.abspath(os.path.dirname(__file__))


def _relpath(p):
    return os.path.abspath(os.path.join(_thisdir, p))


# To get at getMaxTaniScores:
sys.path.insert(0, _relpath(".."))
from testsupport import *


def main():
    filename = sys.argv[1]
    for v in get_max_tani_scores(filename):
        print("{:.4f}".format(v))


if __name__ == "__main__":
    main()
