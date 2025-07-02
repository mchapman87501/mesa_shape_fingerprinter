#pragma once

#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <string>
#include <vector>

// Various global constants and typedefs
#define FPLEN 768

namespace mesaac {
namespace shape_defs {
using BitVector = boost::dynamic_bitset<>;
using ArrayBitVectors = std::vector<BitVector>;
using ShapeFPBlocks = std::vector<ArrayBitVectors>;

using ArrayCountVectors = std::vector<std::vector<char>>;
} // namespace shape_defs
} // namespace mesaac