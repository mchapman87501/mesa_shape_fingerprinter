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
unsigned int bond_type_raw(BondType value) {
  return static_cast<underlying_type_t<BondType>>(value);
}

unsigned int stereo_type_raw(BondStereo value) {
  return static_cast<underlying_type_t<BondStereo>>(value);
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

  // Program name (_Mesaac_) must be 8 chars.
  // I have no idea about the dimensional codes.
  // I assume scaling factors should be whatever they were for
  // the input; OpenBabel omits them altogether.
  const string metadata =
      format("  _Mesaac_{}{:1d}D", molfile_timestamp(), mol.dimensionality());

  // endl is analogous to writing "\n" and flushing.
  // cppreference recommends (weakly) using "\n" where possible.
  m_outf << mol.name() << "\n"
         << metadata << "\n"
         << mol.comments() << "\n"
         << mol.counts_line() << "\n";
  for (const auto &atom : mol.atoms()) {
    write_atom(atom);
  }

  for (const auto &bond : mol.bonds()) {
    const auto bond_line =
        format("{:3d}{:3d}{:3d}{:3d}{}", bond.a0(), bond.a1(),
               bond_type_raw(bond.type()), stereo_type_raw(bond.stereo()),
               bond.optional_cols());
    m_outf << bond_line << "\n";
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
    m_outf << tag.first << "\n" << value << "\n" << "\n";
  }
  m_outf << "$$$$" << "\n";
  m_outf.flush();
  return result;
}

namespace {
template <typename Value>
void write_indices_prop(
    ostream &outs, const string &prop_name,
    const vector<pair<unsigned int, Value>> &indexed_values) {
  // TBD
  outs << format("M  {:3s}{:3d}", prop_name, indexed_values.size());
  for (const auto &[index, value] : indexed_values) {
    outs << format("{:4d}{:4}", index, value);
  }
  outs << "\n";
}
} // namespace

bool SDWriter::write_atom(const Atom &atom) const {

  const auto &pos(atom.pos());
  const auto pos_and_symbol = format("{:10.4f}{:10.4f}{:10.4f} {:<3s}", pos.x(),
                                     pos.y(), pos.z(), atom.symbol());

  const auto &props(atom.props());
  // Best effort...  Perhaps props should store both mass and mass_diff?
  const int mass_diff =
      props.mass == 0
          ? 0
          : static_cast<int>(props.mass - get_atomic_mass(atom.atomic_num()));
  // Always write a charge of 0, then write non-zero charges via a
  // "M  CHG" line.
  const auto props_str =
      format("{:2d}  0{:3d}{:3d}{:3d}{:3d}  0  0  0{:3d}{:3d}{:3d}", mass_diff,
             props.cfg, props.hcount, props.stbox, props.val, props.aamap,
             props.invret, props.exachg);

  m_outf << pos_and_symbol << props_str << "\n";
  return true;
}

bool SDWriter::write_properties_block(const Mol &mol) const {
  vector<pair<unsigned int, int>> radical_indices;
  vector<pair<unsigned int, int>> charge_indices;
  for (unsigned int i = 0; i != mol.num_atoms(); ++i) {
    const unsigned int out_index = i + 1;
    const auto &atom(mol.atoms().at(i));
    const auto &props(atom.props());
    if (props.chg != 0) {
      charge_indices.push_back(make_pair(out_index, props.chg));
    }
    if (props.rad != 0) {
      radical_indices.push_back(make_pair(out_index, props.rad));
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
  m_outf << "M  END" << "\n";
  return true;
}

} // namespace mesaac::mol
