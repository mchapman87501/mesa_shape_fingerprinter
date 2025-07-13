#!/bin/sh
set -e -u

cmake --workflow coverage-report
open build/coverage/coverage_report/index.html