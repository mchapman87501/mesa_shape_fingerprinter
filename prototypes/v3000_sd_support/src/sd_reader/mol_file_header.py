from dataclasses import dataclass
import io


@dataclass
class MolFileHeader:
    name: str
    metadata: str
    comments: str
    # The counts line is not supposed to be part of a molfile header.
    # It's included here for convenience.
    counts_line: str

    def is_empty(self) -> bool:
        return (
            self.name
            == self.metadata
            == self.comments
            == self.counts_line
            == ""
        )

    @classmethod
    def read(cls, inf: io.TextIOBase) -> "MolFileHeader":
        # Raise OSError on eof
        block_lines = [inf.readline().rstrip() for i in range(4)]
        result = cls(*block_lines)
        if result.is_empty():
            raise OSError("End of input")
        return result
