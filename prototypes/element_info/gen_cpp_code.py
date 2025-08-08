# Generate C++ code fragments holding element_info.
"""
Generate C++ maps of element info.
"""

from pathlib import Path

import duckdb


class Generator:
    def __init__(self):
        here = Path(__file__).resolve().parent
        csv_path = here / "PubChemElements_all.csv"
        self._rel = duckdb.sql(f"SELECT * FROM '{csv_path}'")

    def gen_atomic_weight_by_num(self):
        # Generate vector data structures where possible, for faster retrieval.
        yield "const std::vector<float> atomic_mass{"
        prev_num: int | None = None
        for atomic_num, atomic_mass in (
            self._rel.select("AtomicNumber, AtomicMass")
            .order("AtomicNumber ASC")
            .fetchall()
        ):
            yield f"  {atomic_mass},"
            if prev_num is not None:
                # Make sure there are no gaps.
                assert prev_num + 1 == atomic_num
            prev_num = atomic_num
        yield "};"

    def gen_num_by_symbol(self):
        yield "const std::map<std::string, int> atomic_num_by_atomic_symbol{"
        for atomic_symbol, atomic_num in (
            self._rel.select("Symbol, AtomicNumber")
            .order("Symbol ASC")
            .fetchall()
        ):
            yield f'  {{"{atomic_symbol}", {atomic_num}}},'
        yield "};"

    def gen_symbol_by_num(self):
        yield "const std::vector<std::string> atomic_symbol {"
        prev_num: int | None = None
        for atomic_num, atomic_symbol in (
            self._rel.select("AtomicNumber, Symbol")
            .order("AtomicNumber ASC")
            .fetchall()
        ):
            yield f'  "{atomic_symbol}",'
            if prev_num is not None:
                # Make sure there are no gaps.
                assert prev_num + 1 == atomic_num
            prev_num = atomic_num
        yield "};"

    def gen_atomic_radius(self):
        yield "const std::vector<float> atomic_radius {"
        prev_num: int | None = None
        for atomic_num, atomic_radius in (
            self._rel.select("AtomicNumber, AtomicRadius")
            .filter("AtomicRadius IS NOT NULL")
            .order("AtomicNumber ASC")
            .fetchall()
        ):
            yield f"  {atomic_radius},"
            if prev_num is not None:
                # Make sure there are no gaps.
                assert prev_num + 1 == atomic_num
            prev_num = atomic_num
        yield "};"


def main() -> None:
    gen = Generator()
    for items in [
        gen.gen_atomic_radius(),
        gen.gen_atomic_weight_by_num(),
        gen.gen_symbol_by_num(),
        gen.gen_num_by_symbol(),
    ]:
        for item in items:
            print(item)
        print()


if __name__ == "__main__":
    main()
