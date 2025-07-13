# Motivation

When I try to test with code coverage enabled, whether on linux or in a linux docker container, the align_monte executable takes forever to complete. It uses all available cpu while running.
By contrast, a release build completes quickly.

I'm trying to use `perf` inside a docker container to understand why the `coverage` build behaves so badly. Since `perf` uses system performance counters, its use requires
root privileges.

Alas, this doesn't work under docker, even when I run an image interactively, connecting as root.

It has been about five years since I last needed to do this...

```shell
# Run as root:

cd /source
perf stat build/release/src/cli/align_monte/align_monte tests/data/align_monte/cox2_3d.sd tests/data/hammersley/hamm_spheroid_20k_11rad.txt 1.0
```

It suffices to run the docker image with `--cap-add SYS_ADMIN`:

```shell
docker run -it --cap-add SYS_ADMIN -u root --rm -v${PWD}:/source -v${DOCKER_BUILD_DIR}:/source/build build_shape_fingerprints bash

# Inside the container:
cd /source
perf stat build/coverage/src/cli/align_monte/align_monte tests/data/align_monte/cox2_3d.sd tests/data/hammersley/hamm_spheroid_20k_11rad.txt 1.0
perf record --call-graph dwarf build/coverage/src/cli/align_monte/align_monte tests/data/align_monte/cox2_3d.sd tests/data/hammersley/hamm_spheroid_20k_11rad.txt 1.0
perf report
```

Unfortunately, this turns out to be useless because `perf` does not show any symbolic names -- it shows only addresses. Perhaps CMake is stripping executables?

I guess it's good enough to know that the `release` build produces fast executables.
