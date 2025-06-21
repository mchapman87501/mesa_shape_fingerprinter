//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/io/sdwriter.hpp"

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

namespace mesaac {
namespace mol {
SDWriter::SDWriter(ostream &outf) : m_outf(outf) { m_fmt.imbue(locale("C")); }

SDWriter::~SDWriter() {}

string SDWriter::f_str(float f, const unsigned int field_width,
                       unsigned int decimals) {
  // Bah!  C++ stream operators make it very difficult to
  // achieve strict formatting such as %10.4f.
  char buffer[field_width + 1];
  m_fmt.str("");
  m_fmt << "%" << field_width << "." << decimals << "f";
  snprintf(buffer, field_width + 1, m_fmt.str().c_str(), f);
  return string(buffer);
}

string SDWriter::uint_str(unsigned int u, const unsigned int field_width) {
  char buffer[field_width + 1];
  m_fmt.str("");
  m_fmt << "%" << field_width << "u";
  snprintf(buffer, field_width + 1, m_fmt.str().c_str(), u);
  return string(buffer);
}

static inline string molfile_timestamp() {
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
  AtomVector::const_iterator i;
  for (i = mol.atoms().begin(); i != mol.atoms().end(); ++i) {
    Atom *a(*i);
    if (!a) {
      cerr << "Null atom!" << endl;
    } else {
      m_outf << f_str(a->x()) << f_str(a->y()) << f_str(a->z()) << " "
             << setw(3) << left << a->symbol() << a->optional_cols() << endl;
    }
  }
  BondVector::const_iterator j;
  for (j = mol.bonds().begin(); j != mol.bonds().end(); ++j) {
    Bond *b(*j);
    if (!b) {
      cerr << "Null bond!" << endl;
    } else {
      m_outf << uint_str(b->a0()) << uint_str(b->a1()) << uint_str(b->type())
             << uint_str(b->stereo()) << b->optional_cols() << endl;
    }
  }

  m_outf << mol.properties_block();

  const SDTagMap &tags(mol.tags());
  SDTagMap::const_iterator k;
  for (k = tags.begin(); k != tags.end(); ++k) {
    string value(k->second);
    // Strip all trailing blank lines in value.
    // Also strip trailing whitespace from the last line of value --
    // hope that's legitimate.
    while ((value.size() > 0) &&
           (value.find_last_of("\n\t ") == value.size() - 1)) {
      value.erase(value.size() - 1);
    }
    m_outf << k->first << endl << value << endl << endl;
  }
  m_outf << "$$$$" << endl;

  return result;
}
} // namespace mol
} // namespace mesaac
