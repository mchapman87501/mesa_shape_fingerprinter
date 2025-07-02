#pragma once

#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <string>
#include <vector>

namespace mesaac::shape_defs {
using BitVector = boost::dynamic_bitset<>;
using ArrayBitVectors = std::vector<BitVector>;
} // namespace mesaac::shape_defs