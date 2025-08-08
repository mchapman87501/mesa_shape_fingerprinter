#include <catch2/catch_test_macros.hpp>

#include "mesaac_mol/io/sdreader.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace mesaac::mol {
namespace {
const std::filesystem::path test_sdf_path(const std::string &rel_path) {
  const std::filesystem::path test_data_dir(TEST_DATA_DIR);
  const std::filesystem::path sd_files_dir = test_data_dir / "sd_files";
  return sd_files_dir / rel_path;
}

TEST_CASE("mesaac::mol::SDReader - simple V3000 structure", "[mesaac]") {
  std::istringstream ins(R"LINES(aminomethylbenzoic acid
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 11 11 0 0 0
M  V30 BEGIN ATOM
M  V30 1 N 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 C 0 0 0 0
M  V30 6 C 0 0 0 0
M  V30 7 C 0 0 0 0
M  V30 8 C 0 0 0 0
M  V30 9 C 0 0 0 0
M  V30 10 O 0 0 0 0
M  V30 11 O 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 4 3 4
M  V30 4 4 4 5
M  V30 5 4 5 6
M  V30 6 4 6 7
M  V30 7 4 7 8
M  V30 8 4 3 8
M  V30 9 1 6 9
M  V30 10 1 9 10
M  V30 11 2 9 11
M  V30 END BOND
M  V30 END CTAB
M  END
>  <ID> 
22

>  <PREFERRED_NAME>
aminomethylbenzoic acid

>  <CAS_RN>
56-91-7

>  <SYNONYMS>
aminomethylbenzoic acid
p-aminomethylbenzoic acid
para-aminomethylbenzoic acid
gumbix
4-aminomethyl-benzoic acid
4-aminomethylbenzoic acid
benzylamine-4-carboxylic acid
p-carboxybenzylamine

>  <URL>
http://drugcentral.org/drugcard/22/view

$$$$
)LINES");

  SDReader reader(ins, "<from a string>");
  const auto read_result = reader.read();

  if (!read_result.is_ok()) {
    std::cerr << read_result.error() << std::endl;
    FAIL();
  }

  const auto mol = read_result.value();
  REQUIRE(mol.num_atoms() == 11);
  REQUIRE(mol.num_bonds() == 11);
  // Spot-check the tags.
  const auto &tags = mol.tags();
  REQUIRE(tags.at(">  <PREFERRED_NAME>") == "aminomethylbenzoic acid\n");
  REQUIRE(tags.at(">  <URL>") == "http://drugcentral.org/drugcard/22/view\n");
}

TEST_CASE("mesaac::mol::SDReader - read all DrugCentral V3000 structures",
          "[mesaac]") {
  std::filesystem::path pathname(test_sdf_path("structures.molV3.sdf"));
  REQUIRE(std::filesystem::is_regular_file(pathname));

  std::ifstream inf(pathname);
  SDReader reader(inf, pathname.stem().string());
  Mol mol;

  const size_t expected_count = 4278;
  size_t actual_count = 0;
  for (;;) {
    const auto read_result = reader.read();
    if (!read_result.is_ok()) {
      // Is it just end of file, or is something wrong?
      break;
    }
    actual_count += 1;
  }
  REQUIRE(actual_count == expected_count);
}

TEST_CASE("mesaac::mol::SDReader - V3000 no-structure", "[mesaac]") {
  // Ensure ability to read "no-structure" molfiles.
  const auto no_structure_sd = R"LINES(No structure

This is a no-structure as documented in CTfile Formats, ch. 3.
  0  0  0  0  0  0  0  0  0  0999 V2000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 0 0 0 0
M  V30 END CTAB
M  END
$$$$)LINES";
  std::istringstream ins(no_structure_sd);
  SDReader reader(ins, "<no-structure - see CTfile Formats chapter 3.>");

  const auto read_result = reader.read();
  REQUIRE(read_result.is_ok());

  const auto mol = read_result.value();
  REQUIRE(mol.name() == "No structure");
  REQUIRE(mol.num_atoms() == 0);
  REQUIRE(mol.num_bonds() == 0);
}

} // namespace
} // namespace mesaac::mol
