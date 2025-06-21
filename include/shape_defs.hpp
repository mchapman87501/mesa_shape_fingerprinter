#pragma once

#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <string>
#include <vector>

// Various global constants and typedefs
#define FPLEN 768

typedef boost::dynamic_bitset<> BitVector;
typedef std::vector<BitVector> ArrayBitVectors;
typedef std::vector<ArrayBitVectors> ShapeFPBlocks;

typedef std::vector<std::vector<char>> ArrayCountVectors;
