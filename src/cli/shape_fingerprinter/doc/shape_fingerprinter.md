# shape_fingerprinter

`shape_fingerprinter` generates shape fingerprints for the 3D conformers in an SD file. It writes the shape fingerprints to stdout.

## Usage

```shell

shape_fingerprinter [-h | --help] [-i | --id] [-f FORMAT | --format FORMAT] [-n NUM_FOLDS | --num_folds NUM_FOLDS] [-e ELLIPSOID | --ellipsoid ELLIPSOID] [-r RECORDS | --records RECORDS] sd_file hamms_sphere_file atom_scale

Generate shape fingerprints for 3D conformers.

-h | --help
        Show this help message and exit
-i | --id
        include the name of each SD conformer after each fingerprint
-f FORMAT | --format FORMAT
        write fingerprints in the specified format
        valid values:
            A - ASCII (default)
            B - binary
            C - compressed ASCII

-n NUM_FOLDS | --num_folds NUM_FOLDS
        fold fingerprints NUM_FOLDS times, to save space on output (default: 0 - not folded)
-e ELLIPSOID | --ellipsoid ELLIPSOID
        use points from the named file, containing 3D Hammersley ellipsoid points, one point per line with space-separated coords, for fingerprint generation
-r RECORDS | --records RECORDS
        indices of first and last SD file records to process (default: process all records)
sd_file
        file of conformers in SD format, with 3D coordinates
hamms_sphere_file
        file of 3D Hammersley sphere points, one point per line with space-separated coordinates, for principal axes generation via SVD and fingerprint generation
atom_scale
        amount (1.0...2.0) by which to increase atom radii for alignment
```
