#!/bin/sh
set -e -u

cmake --preset release --fresh
cmake --build --preset release
cmake --install build/release --prefix=build/release/local