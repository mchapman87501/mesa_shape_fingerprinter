#!/usr/bin/env python
# encoding: utf-8

"""Compare shape_feature_fingerprinter vs. shape_fingerprinter.
   Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import subprocess
import os
import time
import math

_thisdir = os.path.abspath(os.path.dirname(__file__))
def _relpath(p):
    return os.path.abspath(os.path.join(_thisdir, p))


class Error(Exception): pass


# A single test case with little feature diversity -- what could be better?
class Benchmark(object):
    """"""
    
    def _gen_sfps(self, exe_path, *options, **kw_args):
        dataset = _relpath("data/in/cox2_3d_first_few.sd")
        sphere = _relpath("../../../test_data/hammersley/hamm_spheroid_16k_14rad.txt")
        atom_scale = "1.0"
        args = [exe_path] + list(options) + [dataset, sphere, atom_scale]

        # TODO:  Preserve output for comparison
        lib_path = os.path.dirname(exe_path)
        env = {}
        env.update(os.environ)
        for varname in ["DYLD_LIBRARY_PATH", "LD_LIBRARY_PATH"]:
            env[varname] = os.pathsep.join([
                lib_path, env.get(varname, "")
            ])
        
        outpath = "out.{}_{}.txt".format(os.path.basename(exe_path),
                                         "_".join(options))
        with open(outpath, "w") as outf:
            kw = {}
            kw.update(kw_args)
            kw.update(dict(env=env, stdout=outf, stderr=subprocess.PIPE))
            p = subprocess.Popen(args, **kw)
        
        out, err = p.communicate()
        status = p.returncode
        if status:
            raise Error("Status {} for {}".format(status, args))
        
    def _time_sfps_once(self, exe_path, *options, **kw):
        t0 = time.time()
        self._gen_sfps(exe_path, *options, **kw)
        tf = time.time()
        return tf - t0
        
    def _time_sfps(self, exe_path, *options, **kw):
        num_samples = 5
        samples = []
        for i in range(num_samples):
            curr = self._time_sfps_once(exe_path, *options, **kw)
            print("{} sample {}: {:.2f}".format(os.path.basename(exe_path), i + 1, curr))
            samples.append(curr)
        
        s = sum(samples)
        m = s / num_samples
        ss = sum((sample - m)**2 for sample in samples)
        sd = math.sqrt(ss / (num_samples - 1))
        return m, sd
        
    def run(self):
        shape_stats = self._time_sfps(_relpath("../../ShapeFingerprinter/shape_fingerprinter"))
        sf_stats = self._time_sfps(_relpath("../shape_feature_fingerprinter"), "-f", "B")
        ratio = sf_stats[0] / shape_stats[0]
        print("Shape + features is {:.1f}x slower than shape alone".format(ratio))

def main():
    b = Benchmark()
    b.run()

if __name__ == "__main__":
    main()
