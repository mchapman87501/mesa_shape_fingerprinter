from dataclasses import dataclass
import io
from sd_reader.mol_file_header import MolFileHeader
from sd_reader.mol import Mol
from sd_reader.atom import Atom
from sd_reader.bond import Bond


@dataclass
class _Counts:
    num_atoms: int
    num_bonds: int


class V2000CTabReader:
    def __init__(self, inf: io.TextIOBase, header: MolFileHeader) -> None:
        self._inf = inf
        self._header = header

        self._num_atoms = int(self._header.counts_line[0:3])
        self._num_bonds = int(self._header.counts_line[3:6])

    def read(self) -> Mol | None:
        try:
            atoms = self._read_atoms()
            bonds = self._read_bonds()
            return Mol(
                name=self._header.name.strip(),
                metadata=self._header.metadata.strip(),
                atoms=atoms,
                bonds=bonds,
                raw_properties="",
            )
        except OSError:
            # Presume end of file.
            return None

    def _read_atoms(self) -> list[Atom]:
        return [self._read_atom(i) for i in range(self._num_atoms)]

    def _read_atom(self, index: int) -> Atom:
        line = self._inf.readline()
        x = float(line[0:10])
        y = float(line[11:20])
        z = float(line[21:30])
        symbol = line[31:34].strip()
        # Ignore charge, stereo parity, etc.
        return Atom(index, symbol, x, y, z)

    def _read_bonds(self) -> list[Bond]:
        return [self._read_bond() for i in range(self._num_bonds)]

    def _read_bond(self) -> Bond:
        line = self._inf.readline()
        a0_index = int(line[0:3])
        a1_index = int(line[3:6])
        bond_type = int(line[6:9])
        stereo = int(line[9:12])
        # Ignore bond topology, reacting center status, etc.
        return Bond(a0_index, a1_index, bond_type, stereo)
