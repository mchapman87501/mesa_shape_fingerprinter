#!/usr/bin/env python
# encoding: utf-8

import sys

def main():
    with open(sys.argv[1]) as inf:
        line = inf.readline()
        fields = line.strip().split()
        for f in fields:
            print("{:.4f}".format(float(f)))

if __name__ == "__main__":
    main()
