import io
from sd_reader.mol import Mol

from sd_reader.v2000_ctab_reader import V2000CTabReader
from sd_reader.v3000_ctab_reader import V3000CTabReader
from sd_reader.mol_file_header import MolFileHeader
from sd_reader.parse_error import ParseError


class SDReader:
    def __init__(self, inf: io.TextIOBase, inf_descr: str) -> None:
        self._inf = inf
        self._inf_descr = inf_descr
        self._done = False

    def next_mol(self) -> Mol | None:
        if (result := self._read_molfile()) is not None:
            result.raw_properties = "\n".join(self._read_raw_mol_props())
        return result

    def _read_molfile(self) -> Mol | None:
        if header_block := self._read_header_block():
            if header_block.counts_line.upper().endswith("V3000"):
                return self._read_v3000_molfile(header_block)
            return self._read_v2000_molfile(header_block)
        return None

    def _read_raw_mol_props(self) -> list[str]:
        # STUB!  Just blindly read until mol end marker.
        lines = []
        line = self._inf.readline().rstrip()
        if line != "M  END":
            raise ParseError(f"Expected 'M  END', got '{line}'")

        while True:
            try:
                raw_line = self._inf.readline()
                if raw_line == "":
                    raise OSError("End of input")
                line = raw_line.rstrip()
                if line.rstrip() == "$$$$":
                    return lines
            except OSError:
                # End of file.
                return lines

    def _read_header_block(self) -> MolFileHeader | None:
        try:
            return MolFileHeader.read(self._inf)
        except OSError:
            # Presumably end of file.
            return None

    def _read_v3000_molfile(self, header: MolFileHeader) -> Mol | None:
        return V3000CTabReader(self._inf, header).read()

    def _read_v2000_molfile(self, header: MolFileHeader) -> Mol | None:
        return V2000CTabReader(self._inf, header).read()
