//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_common/b64.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

// See http://tools.ietf.org/html/rfc3548.html#page-3,
// section 3. Base 64 Encoding
const unsigned int BitsPerEncodedWord = 6;
const unsigned int BitsPerDecodedWord = 8;

namespace mesaac {
namespace common {
B64::B64() {}

B64::~B64() {}

const char PadChar = '=';
static string alphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline char masked(unsigned int inbuff) {
  return alphabet[inbuff & 0x3F];
}

string B64::encode(string src) {
  string result;

  unsigned int iMax = src.size();
  unsigned int i = 0;
  while (i < iMax) {
    // Correctness before speed :\  legibility doesn't enter into it.
    unsigned int inbuff = 0;
    unsigned int iPadIndex = 2;
    inbuff |= ((0xFF & src[i++]) << 16);
    if (i < iMax) {
      iPadIndex++;
      inbuff |= ((0xFF & src[i++]) << 8);
      if (i < iMax) {
        iPadIndex++;
        inbuff |= ((0xFF & src[i++]));
      }
    }
    char outchars[5] = {0, 0, 0, 0, 0};
    outchars[3] = masked(inbuff);
    outchars[2] = masked(inbuff >> 6);
    outchars[1] = masked(inbuff >> 12);
    outchars[0] = masked(inbuff >> 18);
    while (iPadIndex <= 3) {
      outchars[iPadIndex++] = PadChar;
    }
    result.append(string(outchars));
  }
  return result;
}

static inline unsigned int from_char(char c) {
  // This should be quicker than a string.find, given the latter
  // involves average 32 comparisons.
  if ((c >= 'A') && (c <= 'Z')) {
    return (unsigned int)(c - 'A');
  }
  if ((c >= 'a') && (c <= 'z')) {
    return (unsigned int)(c - 'a') + 26;
  }
  if ((c >= '0') && (c <= '9')) {
    return (unsigned int)(c - '0') + 52;
  }
  switch (c) {
  case '+':
    return 62;
    break;

  case '/':
    return 63;
    break;

  default: {
    ostringstream msg;
    msg << "Invalid base64 character '" << c << "'";
    throw invalid_argument(msg.str());
  } break;
  }
}

string B64::decode(string src) {
  string result("");

  size_t i_max = src.find_first_of(PadChar);
  if (i_max == string::npos) {
    i_max = src.size();
  }

  size_t i = 0;
  while (i < i_max) {
    unsigned int inbuff = 0;
    int shift = 18;
    unsigned int total_bits = 0;
    while ((i < i_max) && (shift >= 0)) {
      inbuff |= (from_char(src[i++]) << shift);
      shift -= 6;
      total_bits += 6;
    }

    char outchars[4] = {0, 0, 0, 0};
    unsigned int oshift = 16;
    unsigned int valid_bytes = (total_bits / 8);
    for (unsigned int i_byte = 0; i_byte != valid_bytes; i_byte++) {
      outchars[i_byte] = (inbuff >> oshift) & 0xFF;
      oshift -= 8;
    }
    result.append(string(outchars, valid_bytes));
  }

  size_t exp_result_len = (i_max * 6) / 8;
  if (exp_result_len != result.size()) {
    throw runtime_error("Unexpected decoded string length.");
  }
  return result;
}

} // namespace common
} // namespace mesaac
