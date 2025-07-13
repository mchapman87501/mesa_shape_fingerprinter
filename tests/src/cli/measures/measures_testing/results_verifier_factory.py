"""
Produces result verifiers by name.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

from . import results_verifier

_by_name = {
    "Matrix": results_verifier.Matrix,
    "SimMatrix": results_verifier.SimMatrix,
    "OrderedPairs": results_verifier.OrderedPairs,
    "SimOrderedPairs": results_verifier.SimOrderedPairs,
    "SparseMatrix": results_verifier.SparseMatrix,
    "SparseMatrixSearching": results_verifier.SparseMatrixSearching,
    "SimSparseMatrix": results_verifier.SimSparseMatrix,
    "PVM": results_verifier.PVM,
}


def by_format(name, fp_set_measurer, search_number, threshold):
    global _by_name
    klass = _by_name.get(name, None)
    if klass is None:
        raise KeyError("Unknown results_verifier {0!r}".format(name))
    result = klass(fp_set_measurer, search_number, threshold)
    return result


def format_names():
    global _by_name
    return _by_name.keys()
