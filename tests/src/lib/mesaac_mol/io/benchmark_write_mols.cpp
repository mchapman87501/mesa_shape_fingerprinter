#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mesaac_mol/io/sdreader.hpp"
#include "mesaac_mol/io/sdwriter.hpp"

namespace mesaac::mol {
namespace {
const std::filesystem::path test_sdf_path(const std::string &rel_path) {
  const std::filesystem::path test_data_dir(TEST_DATA_DIR);
  const std::filesystem::path sd_files_dir = test_data_dir / "sd_files";
  return sd_files_dir / rel_path;
}

void read_mols(const std::string &rel_path, std::vector<Mol> &mols) {
  mols.clear();
  const auto pathname = test_sdf_path(rel_path);
  std::ifstream inf(pathname);
  mesaac::mol::SDReader reader(inf, pathname);

  while (true) {
    auto result = reader.read();
    if (!result.is_ok()) {
      return;
    }
    mols.emplace_back(std::move(result.value()));
  }
}

void write_mols(const std::vector<Mol> &mols) {
  std::ostringstream outs;
  mesaac::mol::SDWriter writer(outs);
  for (const auto &mol : mols) {
    writer.write(mol);
  }
  CHECK(!outs.str().empty());
}

TEST_CASE("mesaac::mol::SDWriter Benchmarks", "[mesaac][mesaac_benchmark]") {

  BENCHMARK_ADVANCED("write from VS 2000 input")(
      Catch::Benchmark::Chronometer meter) {
    std::vector<Mol> mols;
    read_mols("cox2_3d_first_5.sd", mols);
    CHECK(!mols.empty());

    meter.measure([&mols] { write_mols(mols); });
  };

  BENCHMARK_ADVANCED("write from VS 3000 input")(
      Catch::Benchmark::Chronometer meter) {
    std::vector<Mol> mols;
    read_mols("v3000_with_coords_first_5.sdf", mols);
    CHECK(!mols.empty());

    meter.measure([&mols] { write_mols(mols); });
  };
}

} // namespace

} // namespace mesaac::mol