# Performance Profiling on Ubuntu

## Use the `profile` preset

To do performance profiling on Ubuntu linux, compile for profiling:

```shell
cmake --preset profile
cmake --build --preset profile
```

## Ensure `perf` is installed

Ensure that the `perf` command is installed, using `sudo apt install` as necessary.

## Enable performance monitoring

You may need to enable performance monitoring, at least until system reboot:

```shell
sudo sysctl kernel.perf_event_paranoid=-1
```

## Run with perf

To collect performance data, run the executable of interest via `perf record`.  For example, from the directory containing this markdown document:

```shell
perf record build/profile/src/cli/align_monte/align_monte tests/data/sd_files/cox2_3d.sd tests/data/hammersley/hamm_spheroid_20k_11rad.txt 1.0
```

To see the profiling results, run `perf report`.

Here's example output for the `align_monte` invocation, above.


```text
Samples: 65K of event 'cycles:P', Event count (approx.): 66806926982
Overhead  Command      Shared Object         Symbol
  72.49%  align_monte  align_monte           [.] void ap::_vadd<double, double>(double*, double const*, int, double)
  11.80%  align_monte  align_monte           [.] rmatrixqrunpackq(ap::template_2d_array<double, true> const&, int, int, ap::template_1d_arr
   2.43%  align_monte  align_monte           [.] mesaac::shape::VolBox::set_bits_for_one_sphere_unchecked(std::array<float, 4ul> const&, bo
   1.19%  align_monte  [unknown]             [k] 0xffffffffa5125027
   0.91%  align_monte  align_monte           [.] applyreflectionfromtheleft(ap::template_2d_array<double, true>&, double, ap::template_1d_a
   0.32%  align_monte  align_monte           [.] ap::vmove(double*, double const*, int)
   ...
```
