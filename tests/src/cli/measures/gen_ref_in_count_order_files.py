#!/usr/bin/env python
"""Generate the expected output matrix for ../in/in_count_order.txt."""

import sys
import os
import random
import time
import collections
import multiprocessing
import zlib
import base64


class Error(Exception):
    pass


class Timer:
    def __init__(self):
        self.t0 = time.time()

    def start(self):
        self.t0 = time.time()

    def finish(self, msg):
        tf = time.time()
        print("%s: %.3f" % (msg, (tf - self.t0)))


thisdir = os.path.abspath(os.path.dirname(__file__))


def _relpath(p):
    return os.path.abspath(os.path.join(thisdir, p))


def makedir(p):
    if not os.path.isdir(p):
        os.makedirs(p)


def bitstr(numbits):
    # Try to bias the ratio of 1s and 0s?
    numToSet = int(random.gauss(0.7 * numbits, 0.1 * numbits))
    numToSet = max(0, min(numToSet, numbits))
    bits = (["1"] * numToSet) + (["0"] * (numbits - numToSet))
    random.shuffle(bits)
    return int("".join(bits), 2)


if sys.version_info[:2] >= (2, 6):

    def bitcount(n):
        return bin(n).count("1")
else:
    # Cache bit counts for non-zero hex nybbles.
    _bc_counts = {
        "1": 1,
        "2": 1,
        "3": 2,
        "4": 1,
        "5": 2,
        "6": 2,
        "7": 3,
        "8": 1,
        "9": 2,
        "a": 2,
        "b": 3,
        "c": 2,
        "d": 3,
        "e": 3,
        "f": 4,
    }
    _bc_counts = [i for i in _bc_counts.items()]

    def bitcount(n):
        global _bc_counts
        s = "%x" % n
        count = s.count
        return sum(bits * count(hexchar) for hexchar, bits in _bc_counts)


def tani_dist(a, b):
    return round(1.0 - (float(bitcount(a & b)) / bitcount(a | b)), 6)


# Would we could count on Python 2.6 availability.  That would make it much
# easier to format bit strings.
if sys.version_info[:2] >= (2, 6):

    def format_bin(v, numbits):
        format = "{{0:0>{0}b}}".format(numbits)
        return format.format(v)
else:
    _fb_precomp = [
        "0000",
        "0001",
        "0010",
        "0011",
        "0100",
        "0101",
        "0110",
        "0111",
        "1000",
        "1001",
        "1010",
        "1011",
        "1100",
        "1101",
        "1110",
        "1111",
    ]
    _fb_mask = 0xF
    _fb_shift = 4

    def format_bin(v, numbits):
        global _fb_precomp, _fb_mask, _fb_shift
        result = []
        for i in range(0, numbits, _fb_shift):
            result.append(_fb_precomp[v & _fb_mask])
            v >>= _fb_shift
        result.reverse()
        result = "".join(result)
        return result


def create_fps(numfps, numbits):
    jobs = [numbits] * numfps
    p = multiprocessing.Pool()
    result = p.map(bitstr, jobs)
    return result


def format_fp(args):
    fp, numbits = args
    return "C" + base64.b64encode(zlib.compress(format_bin(fp, numbits))) + "\n"


def write_fps(fps, numbits):
    """Generate the fingerprint file."""
    pathname = _relpath("data/in/in_count_order.txt")
    outf = open(pathname, "w")
    p = multiprocessing.Pool()
    jobs = [(fp, numbits) for fp in fps]
    formatted = p.map(format_fp, jobs)
    # Pretend each fingerprint is identical for all flips.
    for fpstr in formatted:
        for j in range(4):
            outf.write(fpstr)
    # for fp in fps:
    #     fpout = format_bin(fp, numbits) + "\n"
    #     for j in range(4):
    #         outf.write(fpout)
    outf.close()


def compute_matrix_entry(args):
    j, fp1, fp2, thresh = args
    distance = tani_dist(fp1, fp2)
    return j, distance


def write_matrix(fps, thresh):
    pathname = _relpath("data/ref/run_in_count_order")
    triang = collections.defaultdict(list)
    triang_get = triang.get
    outf = open(pathname, "w")
    wr = outf.write
    numFPs = len(fps)
    empty = True
    pool = multiprocessing.Pool()
    for i, ref in enumerate(fps):
        # Output the cached lower diagonal fragment.
        lower_diag = triang_get(i, None)
        if lower_diag is not None:
            for cell_str in triang_get(i, []):
                wr(cell_str)
            # Save space.  Don't need this entry any more.
            del triang[i]

        # Compute the upper diag.
        jobs = [(j, ref, fps[j], thresh) for j in range(i + 1, numFPs)]
        for j, d in pool.map(compute_matrix_entry, jobs):
            if d <= thresh:
                # Store the transpose -- lower diag.
                triang[j].append("%d %g " % (i, d))
                # Write the entry:
                wr("%d %g " % (j, d))
                empty = False
        wr("-1\n")
    outf.close()
    if empty:
        raise Error("Expected output matrix is empty!")


def main():
    makedir(_relpath("data/ref"))
    makedir(_relpath("data/in"))

    numfps = 2400
    numbits = 1024 * 16
    thresh = 0.3

    # Generate a bunch of random bitstrings.
    t = Timer()
    fps = create_fps(numfps, numbits)
    t.finish("Generated fingerprints")

    t.start()
    write_fps(fps, numbits)
    t.finish("Wrote fingerprints")

    t.start()
    write_matrix(fps, thresh)
    t.finish("Wrote matrix")


if __name__ == "__main__":
    main()
