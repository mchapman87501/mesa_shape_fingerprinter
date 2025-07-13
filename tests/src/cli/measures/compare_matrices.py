#!/usr/bin/env python
"""Compare two sparse matrix files, at a given threshold."""

import sys

class Error(Exception): pass

def genSparseEntries(line):
    fields = [f for f in line.strip().split()]
    while fields:
        i = int(fields.pop(0))
        if i == -1:
            return
        yield (i, float(fields.pop(0)))
        
def genSparseRows(inf):
    for line in inf:
        result = [e for e in genSparseEntries(line.strip())]
        yield result
    yield None
        
def genRowPairs(inf1, inf2):
    g1 = genSparseRows(inf1)
    g2 = genSparseRows(inf2)
    while True:
        l1 = g1.next()
        l2 = g2.next()
        result = (l1, l2)
        if result == (None, None):
            return
        elif None in result:
            raise Error("Files have different lengths: {0}".format(result))
        yield result
        
def compareMatrixFiles(pathname1, pathname2, thresh):
    result = True
    eps = 1.0e-5
    inf1 = open(pathname1)
    inf2 = open(pathname2)
    lineNum = 0
    for l1, l2 in genRowPairs(inf1, inf2):
        lineNum += 1
        s1 = dict(l1)
        s2 = dict(l2)
        keys = set(k1 for k1 in s1) 
        keys.add(k2 for k2 in s2)
        
        for k in keys:
            d1 = s1.get(k, 10.0)
            d2 = s2.get(k, 10.0)
            if (abs(d1 - d2) > eps):
                print("Line %(lineNum)s -- column %(k)s: %(d1)s vs. %(d2)s" %
                      dict(lineNum=lineNum, k=k, d1=d1, d2=d2))
                result = False
    inf2.close()
    inf1.close()
    if not result:
        raise Error("Matrices are not identical")
                
def main():
    args = sys.argv[1:] or ["run_in_count_order", "data/ref/run_in_count_order", "0.3"]
    p1, p2 = args[:2]
    thresh = float(args[2])
    compareMatrixFiles(p1, p2, thresh)
    
if __name__ == "__main__":
    main()
    