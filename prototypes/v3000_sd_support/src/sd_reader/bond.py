from dataclasses import dataclass


@dataclass(frozen=True)
class Bond:
    a0_index: int
    a1_index: int
    bond_type: int  # Should be an enum.
    stereo: int  # Ditto
