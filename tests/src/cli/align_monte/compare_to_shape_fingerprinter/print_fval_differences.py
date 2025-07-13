#!/usr/bin/env python
# encoding: utf-8

import sys
import os

_thisdir = os.path.abspath(os.path.dirname(__file__))


def _relpath(p):
    return os.path.abspath(os.path.join(_thisdir, p))


sys.path.insert(0, _relpath(".."))
from testsupport import *


def file_floats(pathname):
    with open(pathname) as inf:
        for line in inf:
            yield float(line.strip())


def main():
    file1_vals = [f for f in file_floats(sys.argv[1])]
    file2_vals = [f for f in file_floats(sys.argv[2])]
    assert len(file1_vals) == len(file2_vals)
    diffs = []
    for v1, v2 in zip(file1_vals, file2_vals):
        d = v2 - v1
        print(d)
        diffs.append(d)

    mean, sd = get_stats(diffs)
    print("Mean difference: {:.4f} +/- {:.4f}".format(mean, sd))


if __name__ == "__main__":
    main()
