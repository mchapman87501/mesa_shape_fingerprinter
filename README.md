# Shape Fingerprints

## Overview

This repository will hold all open-source code for the Mesa Analytics shape fingerprinter ca. 2010-2011.

## Building

This repository uses CMake 3.31 or later.

Several build configurations are supported. Here's how to build the default (Debug) configuration.

```shell
# To configure a build directory in ./build:
cmake --preset default

# To compile in the configured build directory:
cmake --build --preset default

# To run unit tests:
ctest --preset default
```

## Running Tests With Code Coverage

To get a code coverage report for all tests, a similar set of cmake commands can be used.

First, configure and build.

```shell
cmake --preset coverage
cmake --build --preset coverage
```

If `gcovr` is installed on your system, you can build the `coverage_report_gcovr` target.

```shell
cmake --build --preset coverage --target coverage_report_gcovr
```

Similarly, if `lcov` and `genhtml` are installed, you can build the `coverage_report_lcov` target.

```shell
cmake --build --preset coverage --target coverage_report_lcov
```

Each of these targets starts by resetting code coverage counters. Then it re-runs `ctest`. Finally, it produces an HTML coverage report. If all steps succeed, the coverage reports can be viewed with your default web browser.

For `coverage_report_gcovr`:

```shell
# macOS
open build/coverage/coverage_reports/gcovr/index.html
# Ubuntu
firefox build/coverage/coverage_reports/gcovr/index.html &
```

For `coverage_report_lcov`:

```shell
# macOS
open build/coverage/coverage_reports/lcov/index.html
# Ubuntu
firefox build/coverage/coverage_reports/lcov/index.html &
```

## Executables

Here's some basic info about the executables provided by this repository.

- [shape_fingerprinter](src/cli/shape_fingerprinter/doc/shape_fingerprinter.md)

## Issues

### Lots of 3rd Party Tests

The 3rd-party packages that are made available through CMake's `FetchContent_MakeAvailable` define time-consuming unit tests. Eigen defines the bulk of these tests.

For this project, 3rd-party tests are distracting. That's why `CMakePresets.json` specifies to run only those tests that have the `mesaac` CMake test label.

### OpenBabel and FetchContent

[OpenBabel](https://github.com/openbabel/openbabel) is not compatible with the latest (at time of writing) CMake revisions, because of its requirement for an older version of CMake. It looks as though efforts are underway to address this problem: See [issue #2784](https://github.com/openbabel/openbabel/pull/2784).

In the meantime, OpenBabel must be installed separately before building this project.

```shell
# macOS
brew install open-babel

# linux
sudo apt install libopenbabel-dev
```

### gcovr

As mentioned above, `gcovr` is needed for `coverage` builds. It can be installed on macOS and linux as follows:

```shell
# macOS
brew install gcovr

# Ubuntu
sudo apt install gcovr
```

## TODO

### Re-use `ArgParser`

The `shape_filter_by_radius` source code defines an `ArgParser` for parsing command-line options. It should be factored out to its own library for use in all of the user-facing executables.

### Simplify Code Coverage

A "second-party" :smile: project, [mchapman87501/arg_parse](https://github.com/mchapman87501/arg_parse), supports creating test code coverage reports using lcov and a "Profile" build type. That system for generating coverage reports appears to be more straightforward than what is used in this repository. (I have no memory of the inspiration for that "Profile" code -- Mitch) Considering incorporating the "Profile" approach into this repository.
