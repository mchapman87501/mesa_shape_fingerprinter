#!/bin/sh
set -e -u

cd /source

cmake --workflow coverage-report
