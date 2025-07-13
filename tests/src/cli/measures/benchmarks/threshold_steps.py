#!/usr/bin/env python
# encoding: utf-8

"""Measure matrix generator relative performance over a range of thresholds.
Assumes Unix-style executable names.
Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import sys
import os
import time
import subprocess
import math
import hashlib

thisdir = os.path.abspath(os.path.dirname(__file__))


def relpath(p):
    return os.path.abspath(os.path.join(thisdir, p))


indatapath = relpath("../data/in/in_count_order.txt")


def print_input_md5():
    md5 = hashlib.md5()
    with open(indatapath) as inf:
        md5.update(inf.read())
    print("{0}.md5 = {1}".format(indatapath, md5.hexdigest()))


# Why is this not part of the standard library?
def mean_stddev(values):
    s = float(sum(values))
    num_values = len(values)
    mean = s / num_values

    ssqr = sum((v - mean) ** 2 for v in values)
    stddev = math.sqrt(ssqr / (num_values - 1))
    return mean, stddev


def measure(threshold):
    args = [relpath("../../MeasuresShapeFPTani"), indatapath, str(threshold)]
    outf = open("/dev/null", "w")
    t0 = time.time()
    subprocess.check_call(args, stdout=outf)
    tf = time.time()
    outf.close()
    return tf - t0


def print_all_measures():
    print_input_md5()
    print("Thresh\tTime\tStdDev")
    for scaledThresh in range(100, 320, 20):
        thresh = scaledThresh / 1000.0
        dt_mean, dt_stddev = mean_stddev([measure(thresh) for i in range(5)])
        print("%.2f\t%.2f\t%.2f" % (thresh, dt_mean, dt_stddev))
        sys.stdout.flush()


def main():
    print_all_measures()


if __name__ == "__main__":
    main()
