"""
Defines the common interface for all shape measures classes.
"""


class IShapeMeasure:
    def __call__(self, i: list[int], j: list[int], num_bits: int = 4):
        # This interface is wrong -- i and j can be ints for regular measures.
        # For shape measures they can be lists of ints.
        # In both cases the ints are treated as bit vectors.
        raise NotImplementedError("Should return a float in 0..1 inclusive.")

    def measure_opt(self):
        raise NotImplementedError(
            "Should return a measure option string like '-T'"
        )

    def tversky_alpha(self):
        raise NotImplementedError(
            "Should return a Tversky alpha value if applicable, else None"
        )

    def is_shape(self):
        raise NotImplementedError(
            "Should return True if self measures shape fingerprints"
        )
