#pragma once

#include <string>

namespace mesaac::mol {

// Capture the properties documented in CTfile spec.  Note that properties
// such as atom type and coordinates are defined directly on mesaac::mol::Atom.
// See examples in comments.  In most cases, the default is a numeric value of
// 0.
struct AtomProps {
  unsigned int index = 0; // Atom index - This *should* be a core atom property.
  unsigned int aamap = 0; // Atom-atom mapping - should be core.

  int chg = 0;              // CHG=-3
  unsigned int rad = 0;     // RAD=1
  unsigned int cfg = 0;     // CFG=2
  float mass = 0.0;         // MASS=14.0032420 -- default is "natural abundance"
  int val = 0;              // VAL=1
  int hcount = 0;           // HCOUNT=2
  unsigned int stbox = 0;   // STBOX=1
  unsigned int invret = 0;  // INVRET=2
  unsigned int exachg = 0;  // EXACHG=1
  int subst = 0;            // SUBST=1
  unsigned int unsat = 0;   // UNSAT=1
  int rbcnt = 0;            // RBCNT=1
  int attchpt = 0;          // ATTCHPT=2 -- not sure how to specify "default"
  std::string rgroups = ""; // RGROUPS=(3 1 2 3)
  std::string attchord = "";  // ATTCHORD=(5 1 3 2 2 3 1) or ATTCHORD=3
  std::string cta_class = ""; // CLASS=PHOSPHATE
  unsigned int seqid = 0;     // SEQID=2

  // I can find no examples of SEQNAME in rdkit, openbabel, jmol, or trey.
  std::string seqname; // SEQNAME='A'
};

} // namespace mesaac::mol
