"""
Provides tools for working with Python-based binary fingerprints.
Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""


def bitcount(n):
    bit_str = bin(n)[2:]
    return bit_str.count("1")
