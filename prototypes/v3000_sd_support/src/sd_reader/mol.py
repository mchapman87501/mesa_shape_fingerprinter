from dataclasses import dataclass
from sd_reader.atom import Atom
from sd_reader.bond import Bond


@dataclass
class Mol:
    name: str
    metadata: str

    atoms: list[Atom]
    bonds: list[Bond]

    raw_properties: str
