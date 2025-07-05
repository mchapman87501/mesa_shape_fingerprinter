//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/io/sdwriter.hpp"

#include <cstdio>
#include <ctime>
#include <format>
#include <iomanip>
#include <sstream>
#include <type_traits>

using namespace std;

namespace mesaac {
namespace mol {
SDWriter::SDWriter(ostream &outf) : m_outf(outf) {}

namespace {
string f_str(float f, const unsigned int field_width = 10,
             unsigned int decimals = 4) {
  std::string fmt(std::format("{{:{:d}.{:d}f}}", field_width, decimals));
  return std::vformat(fmt, std::make_format_args(f));
}

string uint_str(unsigned int u, const unsigned int field_width = 3) {
  std::string fmt(std::format("{{:{}d}}", field_width));
  return std::vformat(fmt, std::make_format_args(u));
}

string bond_type_str(BondType value) {
  // This is another argument for C++23, which provides std::to_underlying.
  auto ivalue = static_cast<std::underlying_type_t<BondType>>(value);
  return uint_str(ivalue);
}

string stereo_type_str(BondStereo value) {
  auto ivalue = static_cast<std::underlying_type_t<BondStereo>>(value);
  return uint_str(ivalue);
}

inline string molfile_timestamp() {
  string result = "0000000000";
  // MMDDYYHHmm -- from ctfile specification.
  const size_t length(11);
  char buffer[length];
  time_t rawnow;
  struct tm *now;

  time(&rawnow);
  now = localtime(&rawnow);
  if (0 != now) {
    strftime(buffer, length, "%m%d%y%H%M", now);
    result = buffer;
  }
  return result;
}

} // namespace

bool SDWriter::write(Mol &mol) {
  bool result = true;
  // According to the spec, the 'metadata' line needs to list the
  // program which wrote the file, and the date/time at which it was
  // written.  It should also list dimensionality info, but OpenBabel
  // appears not to do that.
  const string program_name("_Mesaac_"); // Must be 8 chars
  int d = mol.dimensionality();

  m_outf << mol.name()
         << endl
         // I have no idea about the dimensional codes.
         // I assume scaling factors should be whatever they were for
         // the input; OpenBabel omits them altogether.
         << "  " << program_name << molfile_timestamp() << setw(1) << d << "D"
         << endl
         << mol.comments() << endl
         << mol.counts_line() << endl;
  for (const auto &atom : mol.atoms()) {
    const auto &pos(atom.pos());
    m_outf << f_str(pos.x()) << f_str(pos.y()) << f_str(pos.z()) << " "
           << setw(3) << left << atom.symbol() << atom.optional_cols() << endl;
  }

  for (const auto &bond : mol.bonds()) {
    m_outf << uint_str(bond.a0()) << uint_str(bond.a1())
           << bond_type_str(bond.type()) << stereo_type_str(bond.stereo())
           << bond.optional_cols() << endl;
  }

  m_outf << mol.properties_block();

  const SDTagMap &tags(mol.tags());
  for (const auto &tag : tags) {
    string value(tag.second);
    // Strip all trailing blank lines in value.
    // Also strip trailing whitespace from the last line of value --
    // hope that's legitimate.
    while ((value.size() > 0) &&
           (value.find_last_of("\n\t ") == value.size() - 1)) {
      value.erase(value.size() - 1);
    }
    m_outf << tag.first << endl << value << endl << endl;
  }
  m_outf << "$$$$" << endl;

  return result;
}
} // namespace mol
} // namespace mesaac
