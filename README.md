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

## Installing

A release build can be created as follows:

```shell
cmake --preset release --fresh
cmake --build --preset release --fresh
```

If the build succeeds, the build artifacts can be installed to the default location `/usr/local`:

```shell
sudo cmake --install build/release
```

Or, to install to an arbitrary `/directory/path`:

```shell
cmake --install build/release --prefix=/directory/path
```

<h3>Using `--workflow`</h3>

If you are using a version of CMake that supports the `--workflow` option, a slightly simpler sequence of commands can be used:

```shell
cmake --workflow release
sudo cmake --install build/release

# Or, to install to an arbitrary ${INSTALL_DIR}:
cmake --install build/release --prefix=${INSTALL_DIR}
```

## Running Tests With Code Coverage

To get a code coverage report for all tests, a similar set of cmake commands can be used.

First, configure and build.

```shell
cmake --preset coverage
cmake --build --preset coverage
```

If `lcov` and `genhtml` are installed, you can build the `coverage_report_lcov` target.

```shell
cmake --build --preset coverage --target coverage_report_lcov
```

This target starts by resetting code coverage counters. Then it re-runs `ctest`. Finally, it produces an HTML coverage report. If all steps succeed, the coverage reports can be viewed with your default web browser.

For `coverage_report_lcov`:

```shell
# macOS
open build/coverage/coverage_reports/lcov/index.html
# Ubuntu
firefox build/coverage/coverage_reports/lcov/index.html &
```

<h3>Using `--workflow`</h3>

If your version of CMake supports `--workflow`, you can create an lcov coverage report as follows:

```shell
cmake --workflow coverage-report

# macOS
open build/coverage/coverage_reports/index.html
# Ubuntu
firefox build/coverage/coverage_reports/index.html &
```

## Executables

Here's some basic info about the executables provided by this repository.

- [shape_fingerprinter](src/cli/shape_fingerprinter/doc/shape_fingerprinter.md)

## Issues

### Hamann Similarity Measure Needs Review

The Hamann measure is unique in mesaac_measures in the sense that, unlike the others, it returns similarity values in range -1...1. According to Wikipedia the [Simple Matching Coefficient](https://en.wikipedia.org/wiki/Simple_matching_coefficient) is equivalent to normalizing Hamann to produce values in 0...1. After discussion, we've decided to incorporate Simple Matching into mesaac_measures.

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

### Performance of align_monte

On macOS, `align_monte` runs very quickly, even when compiled using the `coverage` preset. In docker (linux) and on native linux, the same preset runs very slowly, while consuming all CPU cores.

I tried to use `perf` to understand why this is, but I found the results (which lacked symbolic names) not terribly useful. See [Notes on performance profiling](notes_on_performance_profiling.md). At the same time I discovered that `align_monte` compiled with the `release` preset runs about as quickly as on macOS.

## TODO

### Simple Matching Measure

As discussed above, introduce a "simple matching" similarity measure that has many of the same properties as Hamann but that produces similarity values in the range 0...1.

### Consistent Argument Parsing

The `shape_filter_by_radius` source code defines an `ArgParser` for parsing command-line options. It should be factored out to its own library for use in all of the user-facing executables.

Alternatively, a new `ArgumentParser` could be introduced that simplifies definition of command-line syntax.

### View Aligned Structures

It would be good to have a way to view conformers after alignment. Visualization of conformers and their shape fingerprints would also be useful.
