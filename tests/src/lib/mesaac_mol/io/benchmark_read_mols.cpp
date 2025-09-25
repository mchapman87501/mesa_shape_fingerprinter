#include <filesystem>
#include <fstream>
#include <string>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mesaac_mol/io/sdreader.hpp"

namespace mesaac::mol {
namespace {
const std::filesystem::path test_sdf_path(const std::string &rel_path) {
  const std::filesystem::path test_data_dir(TEST_DATA_DIR);
  const std::filesystem::path sd_files_dir = test_data_dir / "sd_files";
  return sd_files_dir / rel_path;
}

void read_and_discard(const std::string &rel_path) {
  const auto pathname = test_sdf_path(rel_path);
  std::ifstream inf(pathname);
  mesaac::mol::SDReader reader(inf, pathname);
  while (true) {
    auto result = reader.read();
    if (!result.is_ok()) {
      return;
    }
  }
}

TEST_CASE("mesaac::mol::SDReader Benchmarks", "[mesaac][mesaac_benchmark]") {

  BENCHMARK_ADVANCED("read VS 2000")(Catch::Benchmark::Chronometer meter) {
    meter.measure([] { read_and_discard("cox2_3d_first_5.sd"); });
  };

  BENCHMARK_ADVANCED("read VS 3000")(Catch::Benchmark::Chronometer meter) {
    meter.measure([] { read_and_discard("v3000_with_coords_first_5.sdf"); });
  };
}

} // namespace

} // namespace mesaac::mol