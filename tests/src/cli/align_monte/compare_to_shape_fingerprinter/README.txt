This directory contains a manual test, which compares shape tanimoto scores produced by align_monte with those produced by shape_fingerprinter.

The general approach:
Run align_monte on an SD file
Extract max tani scores from the aligned, output SD file
Run shape_fingerprinter on the SD file, to generate fingerprints.
Use shape_measures to generate a full Tani sim matrix from the fingerprints.
The matrix 1st row compares the first SD structure w. all others, in order.
Compare the first row of the matrix with the extracted max tani scores.
Report any differences.
