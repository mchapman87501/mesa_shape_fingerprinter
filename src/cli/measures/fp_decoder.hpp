#pragma once

#include "mesaac_common/b64.hpp"
#include "mesaac_common/gzip.hpp"

#include <stdexcept>
#include <string>

// Inline functions for converting std::strings to fingerprints
static inline void str_to_fp(std::string &s,
                             mesaac::shape_defs::BitVector &fp) {
  const unsigned int imax = s.size();
  unsigned int i;
  fp.clear();
  fp.resize(imax);
  for (i = 0; i != imax; ++i) {
    if ('1' == s[i]) {
      fp.set(i);
    }
  }
}

static inline bool
read_compressed_binascii_fp(std::string encoded,
                            mesaac::shape_defs::BitVector &fp) {
  bool result = true;
  mesaac::common::B64 codec;
  std::string decoded = mesaac::common::gzip::decompress(codec.decode(encoded));
  try {
    str_to_fp(decoded, fp);
  } catch (std::exception &e) {
    result = false;
  }
  return result;
}

static inline bool
read_compressed_binary_fp(std::string encoded,
                          mesaac::shape_defs::BitVector &fp) {
  //    typedef BitVector::block_type Block;

  bool result = true;
  mesaac::common::B64 codec;
  std::string decoded = mesaac::common::gzip::decompress(codec.decode(encoded));

  // Ugh.
  const unsigned int i_block_max = decoded.size();
  const unsigned int block_bits = sizeof(unsigned char) * 8;
  fp.clear();
  fp.resize(i_block_max * block_bits);
  unsigned int i_block;
  unsigned int i = 0;
  for (i_block = 0; i_block != i_block_max; ++i_block) {
    const unsigned char &block(decoded[i_block]);
    // I am sure to get this exactly backwards...
    unsigned int mask = 0x80;
    for (unsigned int j = 0; j != 8; ++j, ++i) {
      if (block & mask) {
        fp.set(i);
      }
      mask >>= 1;
    }
  }
  // // This is not robust.  T.ex. what happens if encoded is corrupt?
  // const size_t num_blocks = decoded.size() / sizeof(Block);
  // fp.resize(num_blocks * BitVector::bits_per_block);
  // Block *buffer = (Block *)&decoded[0];
  // boost::from_block_range(buffer, buffer + num_blocks, fp);
  return result;
}

static inline bool read_binascii_fp(std::string fpstr,
                                    mesaac::shape_defs::BitVector &fp) {
  bool result = true;
  try {
    str_to_fp(fpstr, fp);
  } catch (std::exception &e) {
    result = false;
  }
  return result;
}

static inline bool decode_fp(std::string fpstr,
                             mesaac::shape_defs::BitVector &fp) {
  bool result = true;
  const std::string format_marker = fpstr.substr(0, 1);

  if (format_marker == "C") {
    result = read_compressed_binascii_fp(fpstr.substr(1), fp);
  } else if (format_marker == "B") {
    result = read_compressed_binary_fp(fpstr.substr(1), fp);
  } else if (fpstr.find_first_not_of("01") == std::string::npos) {
    result = read_binascii_fp(fpstr, fp);
  } else {
    result = false;
  }
  return result;
}
