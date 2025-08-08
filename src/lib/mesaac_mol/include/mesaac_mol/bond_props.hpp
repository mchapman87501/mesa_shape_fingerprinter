#pragma once

#include <vector>

namespace mesaac::mol {

// Capture bond properties documented in CTfile spec.
// See examples in comments
struct BondProps {
  unsigned int cfg;                 // CFG=1
  unsigned int topo;                // TOPO=1
  int rxctr;                        // RXCTR=-1
  unsigned int stbox;               // STBOX=1
  std::vector<unsigned int> endpts; // ENDPTS=(5 1 2 3 4 5)
  std::string attach;               // ATTACH=ALL
};
} // namespace mesaac::mol
