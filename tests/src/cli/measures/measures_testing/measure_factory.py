"""
Get measures instances by name.
Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
"""

from . import measure
from . import shape_measure

_by_name = dict(
    B=measure.Bub,
    C=measure.Cosine,
    E=measure.Euclidean,
    H=measure.Hamann,
    T=measure.Tani,
    V=measure.Tversky,
)

_shape_by_name = dict(
    B=shape_measure.Bub,
    C=shape_measure.Cosine,
    T=shape_measure.Tani,
    E=shape_measure.Euclidean,
    H=shape_measure.Hamann,
    V=shape_measure.Tversky,
)


def shape_measure_by_name(name: str | None, tversky_alpha: float | None):
    """Get a shape measure, by name or by C++ command-line code."""
    # Default to Tani.
    klass = _shape_by_name[name or "T"]
    return (
        klass(tversky_alpha)
        if issubclass(klass, shape_measure.Tversky)
        else klass()
    )


# Get either a measure or a shape measure, by name.
def measure_by_name(name, tversky_alpha):
    """Get either a measure or a shape measure, by name."""
    global _by_name, _shape_by_name

    klass = _by_name.get(name, None)
    if klass is None:
        klass = _shape_by_name[name]

    is_tversky = issubclass(klass, (measure.Tversky, shape_measure.Tversky))
    return klass(tversky_alpha) if is_tversky else klass()


def measure_names():
    global _by_name
    return _by_name.keys()


def shape_measure_names():
    global _shape_by_name
    return _shape_by_name.keys()
