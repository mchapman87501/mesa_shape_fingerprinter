//
// Copyright (c) 2008 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_common/b32.hpp"

using namespace std;

namespace mesaac {
namespace common {
B32::B32() {}

B32::~B32() {}

const string B32::getAlphabet() { return "0123456789ABCDEFGHJKMNPQRSTVWXYZ"; }

string B32::encode(string src) { return toChars(transcode(src, 8, 5, true)); }

string B32::decode(string src) {
  return transcode(fromChars(src), 5, 8, false);
}

string B32::transcode(string src, int srcBitsPerWord, int destBitsPerWord,
                      bool pad) {
  string result;
  unsigned char word = 0;
  int topBit = 0x01 << (srcBitsPerWord - 1);
  int bitsToGo = destBitsPerWord;
  const int iMax = srcBitsPerWord * src.size();
  for (int i = 0; i < iMax; ++i) {
    unsigned char chunk = (unsigned char)src[i / srcBitsPerWord];
    unsigned char bitMask = topBit >> (i % srcBitsPerWord);
    word <<= 1;
    if (chunk & bitMask) {
      word |= 1;
    }
    bitsToGo--;
    if (bitsToGo <= 0) {
      result.push_back(word);
      word = 0;
      bitsToGo = destBitsPerWord;
    }
  }

  if (pad && (bitsToGo < destBitsPerWord)) {
    word <<= bitsToGo;
    result.push_back(word);
  }
  return result;
}

string B32::toChars(string bytes) {
  string result;
  const string alphabet(getAlphabet());
  for (string::iterator i = bytes.begin(); i != bytes.end(); ++i) {
    result += alphabet[0xFF & *i];
  }
  return result;
}

string B32::fromChars(string chars) {
  string result;
  const string alphabet(getAlphabet());
  for (string::iterator i = chars.begin(); i != chars.end(); ++i) {
    result.push_back((char)(alphabet.find(*i)));
  }
  return result;
}
} // namespace common
} // namespace mesaac
