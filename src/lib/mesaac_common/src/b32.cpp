//
// Copyright (c) 2008 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_common/b32.hpp"

#include <sstream>

using namespace std;

namespace mesaac::common {

namespace {
const string alphabet = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
string transcode(string src, int src_bits_per_word, int dest_bits_per_word,
                 bool pad) {
  string result;
  unsigned char word = 0;
  int top_bit = 0x01 << (src_bits_per_word - 1);
  int bits_to_go = dest_bits_per_word;
  const int iMax = src_bits_per_word * src.size();
  for (int i = 0; i < iMax; ++i) {
    unsigned char chunk = (unsigned char)src[i / src_bits_per_word];
    unsigned char bit_mask = top_bit >> (i % src_bits_per_word);
    word <<= 1;
    if (chunk & bit_mask) {
      word |= 1;
    }
    bits_to_go--;
    if (bits_to_go <= 0) {
      result.push_back(word);
      word = 0;
      bits_to_go = dest_bits_per_word;
    }
  }

  if (pad && (bits_to_go < dest_bits_per_word)) {
    word <<= bits_to_go;
    result.push_back(word);
  }
  return result;
}

string to_chars(string bytes) {
  string result(bytes.size(), '\0');
  for (size_t i = 0; i < bytes.size(); ++i) {

    result[i] = alphabet.at(0xFF & bytes[i]);
  }
  return result;
}

string from_chars(string chars) {
  string result(chars.size(), '\0');
  for (size_t i = 0; i < chars.size(); ++i) {
    auto alpha = alphabet.find(chars[i]);
    if (alpha == string::npos) {
      ostringstream msg;
      msg << "Invalid B32 character " << (unsigned int)(chars[i] & 0xFF);
      throw runtime_error(msg.str());
    }
    result[i] = alpha;
  }
  return result;
}

} // namespace

string B32::encode(string src) { return to_chars(transcode(src, 8, 5, true)); }

string B32::decode(string src) {
  return transcode(from_chars(src), 5, 8, false);
}

} // namespace mesaac::common
