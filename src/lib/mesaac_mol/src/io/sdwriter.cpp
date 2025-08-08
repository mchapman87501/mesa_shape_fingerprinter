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
#include <vector>

#include "mesaac_mol/element_info.hpp"

using namespace std;

namespace mesaac::mol {
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

bool SDWriter::write(const Mol &mol) {
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
    write_atom(atom);
  }

  for (const auto &bond : mol.bonds()) {
    m_outf << uint_str(bond.a0()) << uint_str(bond.a1())
           << bond_type_str(bond.type()) << stereo_type_str(bond.stereo())
           << bond.optional_cols() << endl;
  }

  write_properties_block(mol);

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

namespace {
template <typename Value>
void write_indices_prop(
    std::ostream &outs, const std::string &prop_name,
    const std::vector<std::pair<unsigned int, Value>> &indexed_values) {
  // TBD
  outs << std::format("M  {:3s}{:3d}", prop_name, indexed_values.size());
  for (const auto &[index, value] : indexed_values) {
    outs << std::format("{:4d}{:4}", index, value);
  }
  outs << std::endl;
}
} // namespace

bool SDWriter::write_atom(const Atom &atom) const {

  const auto &pos(atom.pos());
  m_outf << f_str(pos.x()) << f_str(pos.y()) << f_str(pos.z()) << " " << setw(3)
         << left << atom.symbol();
  const auto &props(atom.props());
  // Best effort...  Perhaps props should store both mass and mass_diff?
  const int mass_diff =
      props.mass == 0
          ? 0
          : static_cast<int>(props.mass - get_atomic_mass(atom.atomic_num()));
  // Always write a charge of 0, then write non-zero charges via a
  // "M  CHG" line.
  m_outf << std::format("{:2d}  0{:3d}{:3d}{:3d}{:3d}  0  0  0{:3d}{:3d}{:3d}",
                        mass_diff, props.cfg, props.hcount, props.stbox,
                        props.val, props.aamap, props.invret, props.exachg)
         << std::endl;
  return true;
}

bool SDWriter::write_properties_block(const Mol &mol) const {
  std::vector<std::pair<unsigned int, int>> radical_indices;
  std::vector<std::pair<unsigned int, int>> charge_indices;
  for (unsigned int i = 0; i != mol.num_atoms(); ++i) {
    const unsigned int out_index = i + 1;
    const auto &atom(mol.atoms().at(i));
    const auto &props(atom.props());
    if (props.chg != 0) {
      charge_indices.push_back(std::make_pair(out_index, props.chg));
    }
    if (props.rad != 0) {
      radical_indices.push_back(std::make_pair(out_index, props.rad));
    }
  }

  // Write charges.
  if (!charge_indices.empty()) {
    write_indices_prop(m_outf, "CHG", charge_indices);
  }

  // Write radicals.
  if (!radical_indices.empty()) {
    write_indices_prop(m_outf, "RAD", radical_indices);
  }

  // TODO write other properties.
  m_outf << "M  END" << endl;
  return true;
}

} // namespace mesaac::mol
