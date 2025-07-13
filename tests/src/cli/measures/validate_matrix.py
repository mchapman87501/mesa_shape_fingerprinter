#!/usr/bin/env python
# encoding: utf-8

"""Validate the contents of a sparse matrix, read from stdin.
   Copyright (c) 2009 Desert Moon Consulting, LLC
"""
import sys
import os
import logging

def gen_cols(line):
    field_strs = line.strip().split()
    i = 0
    while True:
        col_index = int(field_strs[i])
        i += 1
        if col_index == -1:
            break
        val = float(field_strs[i])
        i += 1
        yield (col_index, val)
        
def validate_file(inf, threshold):
    cached = {}
    pathname = os.path.abspath(inf.name)
    for i, line in enumerate(inf):
        prev_col = None
        for colnum, distance in gen_cols(line):
            if (i == colnum):
                raise SystemExit("File %s, line %d, contains a distance for the diagonal." % (pathname, i + 1))
            if prev_col is not None:
                if colnum <= prev_col:
                    raise SystemExit(
                        "File %s, line %d, columns out of order: %s, %s" % 
                        (pathname, i + 1, prev_col, colnum))
                if distance > threshold:
                    raise SystemExit(
                        "File %s, line %d, distance exceeds threshold: %s" %
                        (pathname, i + 1, distance))
            if i > colnum:
                cdist = cached[(colnum, i)]
                if cdist != distance:
                    raise SystemExit("File %s, entry (%d, %d) value %s differs from transpose %s" % (pathname, i, colnum, distance, cdist))
            elif i < colnum:
                cached[i, colnum] = distance
            prev_col = colnum


def main():
    args = sys.argv[1:] or ["run_test_measures_best_of_4", "0.3"]
    filename = args[0]
    threshold = 0.3
    if len(args) > 1:
        threshold = float(args[1])
        
    inf = open(filename)
    validate_file(inf, threshold)
    inf.close()

if __name__ == "__main__":
    main()
