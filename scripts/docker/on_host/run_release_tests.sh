#!/bin/sh
set -e -u

DOCKER_BUILD_DIR=${PWD}/build/docker
mkdir -p ${DOCKER_BUILD_DIR}

docker run --rm \
       -v${PWD}:/source \
       -v${DOCKER_BUILD_DIR}:/source/build \
       build_shape_fingerprints \
       "/source/scripts/docker/in_container/build_release_tests.sh"
