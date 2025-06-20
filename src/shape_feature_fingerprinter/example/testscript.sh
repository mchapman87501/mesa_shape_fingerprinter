#!/bin/bash

EXPECTED=expected_output
ACTUAL=actual_output

if [ -d ${ACTUAL} ]; then
    rm -rf ${ACTUAL}
fi
mkdir ${ACTUAL}

declare -i test_cnt=0
declare -i fail_cnt=0
failed_names=""
function verify() {
    diff -q ${EXPECTED}/${1} ${ACTUAL}/${1}
    status=$?
    if [ ${status} != 0 ]; then
        echo "**** Failed ${1}. ****"
        failed_names="${failed_names} ${1}"
        let fail_cnt++
    fi
    let test_cnt++
}

function summarize_tests() {
    status=0
    echo "Completed ${test_cnt} tests; ${fail_cnt} failures."
    if [ ${fail_cnt} != 0 ]; then
        echo "These tests failed:"
        for name in ${failed_names}; do
            echo "  ${name}"
        done
        status=1
    fi
    
    if [ "$#" != "0" ]; then
        exit ${status}
    fi
}

#Generate volumes.  There are 467 distinct compounds and conformers in the cox2_3d.sd file.
BASE=shape_volume_usage.txt
./ShapeVolume 2> ${ACTUAL}/${BASE}
verify ${BASE}


#Example
BASE=cox2.volume.10rad.1.4eps.txt
./ShapeVolume cox2_3d.sd hammersly16384Sphere.txt 10 1.4 > ${ACTUAL}/${BASE}
verify ${BASE}

#With 16384 points in a with a sphere radius of 10 angstroms and a space filing parameter
#of 1.3 this takes 2 seconds on a single core of a 2.2 GHz Intel Core 2 Duo         

#Align one conformer to the remaining in an sd file.
BASE=align_monte_usage.txt
./AlignMonte 2> ${ACTUAL}/${BASE}
verify ${BASE}


#Example
BASE=cox2.aligned.sd
./AlignMonte cox2_3d.sd hammersly16384Sphere.txt 1.4 | sed -e 's,^ OpenBabel............, OpenBabelMoleculeTitle,'> ${ACTUAL}/${BASE}
verify ${BASE}
#With 16384 points and a space filling of 1.4 per atom radii, this takes roughly 12 seconds 
#on a single core of a 2.2 GHz Intel Core 2 Duo

#Shape Fingerprint generation
BASE=shape_fingerprinter_usage.txt
./ShapeFingerprinter 2>${ACTUAL}/${BASE}
verify ${BASE}

#Example
BASE=cox2.ellipsoid.fps
./ShapeFingerprinter cox2_3d.sd hammersly8192Ellipsoid.txt hammersly16384Sphere.txt 1.5 > ${ACTUAL}/${BASE}
verify ${BASE}
#With this parameter set, this takes 11 seconds on a single core as in the above examples.


#Shape measures comparisons
BASE=measures_shape_fp_usage.txt
./MeasuresShapeFP 2>&1 | sed -e 's,^Expiration Date.*,,' > ${ACTUAL}/${BASE}
verify ${BASE}

#Matrix generation
BASE=cox2.smat.txt
./MeasuresShapeFP cox2.fps -T -D -S -F 0 0.3 > ${ACTUAL}/${BASE}
verify ${BASE}

#Again, on the same processor configuration as above, this takes 10 seconds to generate the sparse matrix
#suitable for clustering

#Similarity searching MXN
BASE=cox2.simsearch.txt
./MeasuresShapeFP cox2.fps -T -S -S -T 5 0.7 > ${ACTUAL}/${BASE}
verify ${BASE}

#This takes less than a second.  SimilarityOutput and SimilaritySets can be used with this output, though only
#with 2D information such as SMILES and/or conformer IDs and ancillary conformation data, not sdf file format.

#Generation of Hammersley points
BASE=hammersley_spheroid_usage.txt
./HammersleySpheroid 

#Examples
#Sphere
BASE=hammersley_lines_90000.txt
./HammersleySpheroid 90000 1 1 1 10 |wc -l >${ACTUAL}/${BASE}
verify ${BASE}

BASE=hammersley_lines_sphere_36616.txt
./HammersleySpheroid 70000 1 1 1 10 | wc -l >${ACTUAL}/${BASE}
verify ${BASE}

BASE=hamSphere36616.txt
./HammersleySpheroid 70000 1 1 1 10 >${ACTUAL}/${BASE}
verify ${BASE}

#Scalene Ellipsoid
BASE=hammersley_lines_ellipsoid_21675.txt
./HammersleySpheroid 70000 1 0.7 0.5 10 |wc -l >${ACTUAL}/${BASE}
verify ${BASE}

BASE=hamEllipsoid21675.txt
./HammersleySpheroid 70000 1 0.7 0.5 10 >${ACTUAL}/${BASE}
verify ${BASE}

#Each of these take roughly one second to compute on one core as above.

summarize_tests 1

