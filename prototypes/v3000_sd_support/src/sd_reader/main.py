import argparse
from pathlib import Path

from sd_reader.sd_reader import SDReader
from sd_reader.parse_error import ParseError


def parse_cmdline() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Read an SD file.")
    parser.add_argument("pathname", nargs="+", type=Path)
    return parser.parse_args()


def main() -> None:
    """Mainline for standalone execution."""
    opts = parse_cmdline()
    for pathname in opts.pathname:
        print("Reading", str(pathname))
        with pathname.open() as inf:
            reader = SDReader(inf, str(opts.pathname))
            try:
                while mol := reader.next_mol():
                    print(f"Read {mol}")
            except ParseError as info:
                print(info)
                print("Moving on...")
                print()


if __name__ == "__main__":
    main()
