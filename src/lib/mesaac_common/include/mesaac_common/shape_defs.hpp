#pragma once

#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <string>
#include <vector>

namespace mesaac::shape_defs {
/**
 * @brief This is a bit vector representing, for example, a shape fingerprint
 * for a single orientation of a conformer.
 */
using BitVector = boost::dynamic_bitset<>;

/**
 * @brief This is a sequence of bit vectors.  It can hold multiple shape
 * fingerprints for a conformer, each representing a single canonical
 * orientation.
 */
using ArrayBitVectors = std::vector<BitVector>;

/**
 * @brief This is a sequence of ArrayBitVectors.  It can hold the canonical
 * shape fingerprints for multiple conformers.
 */
using ShapeFPBlocks = std::vector<ArrayBitVectors>;

/**
 * @brief Construct a BitVector from a string.
 * @details This is mainly for interoperability with other programming
 * languages.
 * @param strval the string representation of the bitvector, e.g., "01011101"
 * @return a BitVector with the given value
 */
BitVector bit_vector_from_str(const std::string &strval);
} // namespace mesaac::shape_defs