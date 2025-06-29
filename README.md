# Shape Fingerprints

## Overview

This repository intends to hold all open-source code for the Mesa Analytics shape fingerprinter ca. 2010-2011.

## Building

The original Mesa code was built with SCons. This repository uses CMake 4.0 or later.

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

```shell
cmake --preset coverage
cmake --build --preset coverage
ctest --preset coverage
cmake --build --preset coverage --target process_coverage
```

The HTML-formatted coverage reports can be viewed with your default web browser. For example:

```shell
# macOS
open build/coverage/report/index.html
# Ubuntu
firefox build/coverage/report/index.html
```

## Issues

### Lots of 3rd Party Tests

The 3rd-party packages that are made available through `FetchContent` define a lot of time-consuming unit tests. Eigen defines the bulk of these tests.

For this project, 3rd-party tests are distracting. That's why `CMakePresets.json` explores a way of running only the project's own tests, as identified by CMake labels.

### OpenBabel and FetchContent

[OpenBabel](https://github.com/openbabel/openbabel) is not compatible with CMake >= 4.0, because of its requirement for an older version of CMake. It looks as though efforts are underway to address this problem: See [issue #2784](https://github.com/openbabel/openbabel/pull/2784).

### gcovr

`gcovr` may not be installed by default.

```shell
# macOS
brew install gcovr
# Ubuntu
sudo apt install gcovr
```



## TODO

### Re-use `ArgParser`

[shape_filter_by_radius.cpp](./src/shape_filter_by_radius/shape_filter_by_radius.cpp) defines an `ArgParser` for parsing command-line options. It should be factored out to its own library for use in all of the user-facing executables.
