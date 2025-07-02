# shape_fingerprinter

`shape_fingerprinter` generates shape fingerprints for the 3D conformers in an SD file. It writes the shape fingerprints to stdout.

## Usage

```shell
shape_fingerprinter [options] sd_file hamms_sphere_file atom_scale

sd_file              - a file of conformers in SD format, with 3D coordinates
hamms_sphere_file    - a file containing 3D Hammersley sphere points, one point
                       per line with space-separated coordinates, for principal
                       axes generation via SVD and fingerprint generation
atom_scale           - the amount, in the range [1.0 .. 2.0], by which to
                       increase atom radii for alignment

Options:
-i | --id            - if specified, include the name of each SD conformer
                       after each fingerprint, separated by a space
-f fmt | --format fmt
                     - write fingerprints in the specified format:
                       A - ascii (default)
                       C - compressed ascii
                       B - binary
-n folds | --num_folds folds
                     - fold fingerprints the specified number of times,
                       to save space on output.  The default is zero
                       (unfolded).
-e | --ellipsoid ELLIPSOID_FILE
                     - use points from ELLIPSOID_FILE, a file containing 3D
                       Hammersley ellipsoid points, one point per line with
                       space-separated coords, for fingerprint generation
-r | --records START END
                     - process records START..(END - 1), inclusive, of the
                       sd_file.  By default all records,
                       0..(# fingerprints - 1), are processed.
-h | --help          - print this help message and exit
```
