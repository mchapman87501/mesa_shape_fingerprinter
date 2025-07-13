#!/bin/sh
set -e -u


BA_DIR=/source/build/release

cd /source

cmake --preset release --fresh
cmake --build --preset release
ctest --preset release
