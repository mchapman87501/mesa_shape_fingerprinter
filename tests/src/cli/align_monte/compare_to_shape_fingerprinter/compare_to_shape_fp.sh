#!/bin/bash
# See the README.

set -e

# Work in the directory containing this script.
cd $(dirname "$0")

startdir=${PWD}

cd ../../../../
topdir=${PWD}

# Locate the binaries
ALIGN=${PWD}/src/AlignMonte/align_monte
SHAPE_FP=${PWD}/src/ShapeFingerprinter/shape_fingerprinter
SHAPE_MEASURES=${PWD}/src/measures/measures_shape_fp

# Test input data
SPHERE=${PWD}/test_data/hammersley/hamm_spheroid_10k_11rad.txt
#ELLIPSOID=${PWD}/test_data/hammersley/hamm_ellipsoid_10k_11rad.txt
ELLIPSOID=${SPHERE}
ATOM_SCALE=1.0
COX2=${PWD}/src/AlignMonte/test/data/in/cox2_3d.sd

# Intermediate data:
ALIGNED_SD=aligned.sdf
SHAPE_FP_VALS=shape.fps.txt
FULL_SIM_MATRIX=shape.fps.matrix.txt

# Final data files:
ALIGNED_TANIS=align.max_tani.txt
SFP_TANIS=sfp.max_tani.txt
DIFFS=diffs.txt

# Make sure everything is up to date.
scons -u expires=never ${ALIGN} ${SHAPE_FP} ${SHAPE_MEASURES} ${SPHERE} ${ELLIPSOID}

# Align the conformers, then generate shape fps and a full matrix.
cd ${startdir}

${ALIGN} ${COX2} ${SPHERE} ${ATOM_SCALE} >${ALIGNED_SD}
python extract_max_tani_scores.py ${ALIGNED_SD} >${ALIGNED_TANIS}

${SHAPE_FP} ${COX2} ${ELLIPSOID} ${SPHERE} ${ATOM_SCALE} >${SHAPE_FP_VALS}
${SHAPE_MEASURES} ${SHAPE_FP_VALS} -T -S -M -F 0 >${FULL_SIM_MATRIX}
python extract_matrix_similarities.py ${FULL_SIM_MATRIX} >${SFP_TANIS}

python print_fval_differences.py ${ALIGNED_TANIS} ${SFP_TANIS} >${DIFFS}
