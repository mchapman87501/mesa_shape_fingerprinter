#!/usr/bin/env python
# encoding: utf-8
"""Demos generation of a sparse shape distance matrix using multiple cores,
   via measures_sfp_band and Python's multiprocessing module.
   
   Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import sys
if sys.version_info[:3] < (2, 7, 0):
    raise SystemExit("Sorry, this example requires Python 2.7 or later")

import os
import math
import subprocess
import multiprocessing
import argparse

def parse_cmd_line():
    default_nprocs = multiprocessing.cpu_count()
    default_alpha = 0.0
    default_thresh = 0.3

    parser = argparse.ArgumentParser(
        description="Generate a matrix of shape fingerprint measures.")
    add_arg = parser.add_argument
    
    add_arg('sfp_path', help='pathname of shape fingerprints file')
    add_arg('-n', '--num_processors', type=int, default=default_nprocs,
        help="Num. processors to use (default {})".format(default_nprocs))
    add_arg('-m', '--measure', choices=['T', 'V', 'C'], default='T',
        help="Measure to use: 'T' (Tanimoto, the default), 'V' (Tversky), 'C' (Cosine)")
    add_arg('-a', '--alpha', type=float, default=default_alpha, 
        help="Tversky alpha value (default {})".format(default_alpha))
    add_arg('-t', '--threshold', type=float, default=default_thresh,
        help="Dissimilarity threshold (default {})".format(default_thresh))
        
    result = parser.parse_args()
    
    if result.num_processors < 1:
        parser.error("num_processors must be at least 1")
    if not 0.0 <= result.alpha <= 2.0:
        parser.error("alpha must be in the range 0..2, inclusive")
    if not 0.0 <= result.threshold <= 1.0:
        parser.error("threshold must be in the range 0..1, inclusive")
        
    return result
    
def get_num_shape_fingerprints(pathname):
    with open(pathname) as inf:
        return sum(1 for line in inf) // 4  # shape fps span 4 lines each

def process_band(args):
    band_id, sfp_path, measure, tv_alpha, start, end, thresh = args
    cmd_args = ['measures_sfp_band', '-r', str(start), str(end), '-d',
                '-m', measure, '-t', str(thresh)]
    if measure == "V":
        cmd_args += ['-a', str(tv_alpha)]
    cmd_args.append(sfp_path)
    
    dirname = os.path.dirname(os.path.abspath(sfp_path))
    corename, ext = os.path.splitext(os.path.basename(sfp_path))
    band_path = os.path.join(dirname, "{}.band.{}".format(corename, band_id))
    with open(band_path, "w") as outf:
        with open("/dev/null", "w") as errf:
            status = subprocess.call(cmd_args, stdout=outf, stderr=errf)
    if status:
        print("Status {} for args {}".format(status, cmd_args))
    return status, band_path
    
def reduce_results(results):
    failures = [i for (i, (status, path)) in enumerate(results) if 0 != status]
    if failures:
        raise SystemExit("Could not compute bands {}".format(failures))
    
    for status, band_path in results:
        with open(band_path) as inf:
            for line in inf:
                sys.stdout.write(line)
        os.remove(band_path)

def run(num_jobs, sfp_path, measure, tv_alpha, thresh):
    num_fps = get_num_shape_fingerprints(sfp_path)
    fps_per_job = int(math.ceil(num_fps / float(num_jobs)))
    jobs = []
    for i, start in enumerate(range(0, num_fps, fps_per_job)):
        end = min(start + fps_per_job, num_fps)
        jobs.append([i, sfp_path, measure, tv_alpha, start, end, thresh])
        
    p = multiprocessing.Pool()
    reduce_results(p.map(process_band, jobs))
    p.close()
    
if __name__ == "__main__":
    args = parse_cmd_line()
    run(args.num_processors, args.sfp_path, args.measure, args.alpha,
        args.threshold)
