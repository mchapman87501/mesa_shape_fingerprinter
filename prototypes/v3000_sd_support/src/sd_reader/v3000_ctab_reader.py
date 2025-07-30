import io
import re
import string

from sd_reader.mol_file_header import MolFileHeader
from sd_reader.mol import Mol
from sd_reader.atom import Atom
from sd_reader.bond import Bond
from sd_reader.parse_error import ParseError


class IStringStream:
    """Imitate a std::istringstream."""

    def __init__(self, content: str):
        self._input = io.StringIO(content)

    def read_int(self) -> int:
        """Imitate 'ins >> int_val'."""
        first_non_ws = self.skip_ws()
        return int(first_non_ws + self.read_string())

    def read_float(self) -> float:
        """Imitate 'ins >> float_val'."""
        first_non_ws = self.skip_ws()
        return float(first_non_ws + self.read_string())

    def getline(self, delim_char: str) -> str:
        """Imitage 'std::getline(ins, result, delim_char);''"""
        result = ""
        while ch := self._input.read(1):
            if ch == delim_char:
                return result
            result += ch
        # Should not reach here.
        return result

    def read_char(self) -> str:
        """Imitate 'char c; ins.get(c);'"""
        result = self._input.read(1)
        if not result:
            raise OSError("End of input")
        return result

    def read_string(self) -> str:
        """Imitate 'std::string s; ins >> s;'"""
        result = ""
        while ch := self._input.read(1):
            if ch in string.whitespace:
                return result
            result += ch
        raise OSError("End of input")

    def skip_ws(self):
        while True:
            ch = self._input.read(1)
            if not ch:
                raise OSError("End of input")
            if ch not in string.whitespace:
                return ch


class V3000CTabReader:
    _prefix = "M  V30 "

    def __init__(self, inf: io.TextIOBase, header: MolFileHeader) -> None:
        self._inf = inf
        self._header = header

    def read(self) -> Mol | None:
        line = self._next_line()
        if line != "BEGIN CTAB":
            raise ParseError(f"Expected 'BEGIN CTAB', got '{line}'")
        line = self._next_line()
        if not line.startswith("COUNTS "):
            raise ParseError(f"Expected 'COUNTS ...', got '{line}'")
        fields = line.split()[1:]  # Ditch 'COUNTS'
        num_atoms = int(fields[0])
        num_bonds = int(fields[1])
        # Ignore other fields
        atoms = self._read_atoms(num_atoms)
        bonds = self._read_bonds(num_bonds)
        raw_properties = self._read_other_blocks()
        return Mol(
            name=self._header.name.strip(),
            metadata=self._header.metadata.strip(),
            atoms=atoms,
            bonds=bonds,
            raw_properties=raw_properties,
        )

    def _read_atoms(self, num_atoms: int) -> list[Atom]:
        line = self._next_line()
        if line != "BEGIN ATOM":
            raise ParseError(f"Expected 'BEGIN ATOM', got '{line}'")
        result = [self._read_atom() for i in range(num_atoms)]
        line = self._next_line()
        if line != "END ATOM":
            raise ParseError(f"Expected 'END ATOM', got '{line}'")
        return result

    def _read_atom(self) -> Atom:
        atom_line = self._next_line()
        # How might one parse an atom line, using C++?
        ins = IStringStream(atom_line)
        index = ins.read_int()
        atom_type = self._read_atom_type(ins)
        x = ins.read_float()
        y = ins.read_float()
        z = ins.read_float()
        _remainder = ins.getline("\0")

        if (
            atom_type in ["*", "R#"]
            or atom_type.startswith("[")
            or atom_type.find(" ") >= 0
        ):
            raise ParseError(f"Query atoms are not supported: '{atom_type}'")

        return Atom(index, atom_type, x, y, z)

    def _read_atom_type(self, ins: IStringStream) -> str:
        # Strategy: read up until the next whitespace.  If the value
        # read starts with
        ch = ins.read_char()
        if ch == '"':
            return ins.getline('"')
        return ch + ins.read_string()

    def _read_bonds(self, num_bonds: int) -> list[Bond]:
        if num_bonds <= 0:
            return []
        line = self._next_line()
        if line != "BEGIN BOND":
            raise ParseError(f"Expected 'BEGIN BOND', got '{line}'")
        result = [self._read_bond() for i in range(num_bonds)]
        line = self._next_line()
        if line != "END BOND":
            raise ParseError(f"Expected 'END BOND', got '{line}'")
        return result

    def _read_bond(self) -> Bond:
        _bond_line = self._next_line()
        fields = _bond_line.split()
        # bond_index = int(fields[0])
        bond_type = int(fields[1])
        a0_index = int(fields[2])
        a1_index = int(fields[3])
        stereo = 0
        for field in fields[4:]:
            if field.upper().startswith("CFG="):
                stereo = int(field[4:])
        # TODO: Ignore remaining fields, but save them for output as-is.
        # remainder = " ".join(fields[4:])

        return Bond(a0_index, a1_index, bond_type, stereo)

    def _read_other_blocks(self) -> str:
        lines = []
        while True:
            # TODO actually look for balanced BEGIN...END
            # TODO understand the optional trailing collection-block, rgroup-block, template-block
            # in the spec.  They can occur after the "END CTAB" -- what does that mean?
            line = self._next_line()
            if line == "END CTAB":
                return "\n".join(lines)
            if not line:
                # Assume end of file
                return "\n".join(lines)

    def _next_line(self) -> str:
        result = self._v30_line()
        while result.endswith("-"):
            # It's a continuation line.
            result = result[:-1] + self._v30_line()
        return result

    def _v30_line(self) -> str:
        line = self._inf.readline().rstrip()  # Ditch the trailing newline
        if not line.startswith(self._prefix):
            raise ValueError(f"Missing V30 prefix: '{line}'")
        return line[len(self._prefix) :]
