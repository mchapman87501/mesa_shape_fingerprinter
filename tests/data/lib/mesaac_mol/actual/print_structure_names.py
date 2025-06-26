#!/usr/bin/env python
# encoding: utf-8

"""Print the IDs (er, names) of all structures in specified SD files."""
import sys

def gen_names(inf):
    next_is_id = True
    for line in inf:
        if next_is_id:
            yield line.rstrip()
            next_is_id = False
        else:
            next_is_id = (line.strip() == "$$$$")

def print_names_in_file(pathname):
    print(72 * "=")
    print(pathname)
    with open(pathname) as inf:
        for name in gen_names(inf):
            print(name)
    print("")
    
def main():
    for pathname in sys.argv[1:]:
        print_names_in_file(pathname)

if __name__ == "__main__":
    main()
