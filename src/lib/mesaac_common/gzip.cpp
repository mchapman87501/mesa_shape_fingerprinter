//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_common/gzip.hpp"

#include <cstring> // memcpy
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <zlib.h>

using namespace std;

namespace mesaac {
namespace common {
namespace gzip {
string compress(const string src, int level) {
  string result("");

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  // Must use deflateInit2 to request gzip compression.
  // window_bits:  > 15 for optional gzip encoding.
  //               + 16 to force a simple gzip header and trailer.
  int window_bits = 15 + 16;
  int mem_level = 8;
  int strategy = Z_DEFAULT_STRATEGY;
  int status =
      deflateInit2(&strm, level, Z_DEFLATED, window_bits, mem_level, strategy);
  if (Z_OK == status) {
    // Just compress something, anything, in multiple steps.
    unsigned int uc_buff_size = src.size();
    unsigned char uncompressed_buff[uc_buff_size];
    memcpy((void *)uncompressed_buff, (void *)src.c_str(), uc_buff_size);
    strm.next_in = uncompressed_buff;
    strm.avail_in = uc_buff_size;
    // Loop until the compressor fails to completely exhaust the
    // compression buffer.
    do {
      const int BuffSize = 1024;
      unsigned char compressed_buff[BuffSize + 1];
      strm.avail_out = BuffSize;
      strm.next_out = compressed_buff;

      int flush = Z_FINISH; // Other legal value is Z_NO_FLUSH
      status = deflate(&strm, flush);
      if (Z_STREAM_ERROR != status) {
        unsigned int bytes_written = BuffSize - strm.avail_out;
        string chunk((char *)compressed_buff, bytes_written);
        result += chunk;
      } else {
        deflateEnd(&strm);
        ostringstream msg;
        msg << "gzip deflate returned error code " << Z_ERRNO;
        throw runtime_error(msg.str());
      }
    } while (0 == strm.avail_out);

    if (0 != strm.avail_in) {
      throw runtime_error("gz stream did not consume all input.");
    } else if (Z_STREAM_END != status) {
      throw runtime_error("gz stream is not complete.");
    } else {
      deflateEnd(&strm);
    }
  } else {
    throw runtime_error("Could not initialize gzip compressor");
  }
  return result;
}

string decompress(const string src) {
  string result = "";

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  int windowBits = 15 + 32;

  int status = inflateInit2(&strm, windowBits);
  if (Z_OK != status) {
    throw runtime_error("Could not initialize gzip decompressor");
  } else {
    // See zlib's examples/zpipe.c
    const unsigned int Chunk = 16 * 1024;
    unsigned char in[Chunk];
    unsigned int i_src = 0;
    do {
      string srcChunk(src.substr(i_src, Chunk - 1));
      unsigned int num_chars = srcChunk.size();
      if (0 == num_chars) {
        break;
      }

      memcpy((void *)in, (void *)srcChunk.c_str(), num_chars);
      i_src += num_chars;
      strm.avail_in = num_chars;
      strm.next_in = in;

      do {
        unsigned char out[Chunk];
        strm.avail_out = Chunk;
        strm.next_out = out;
        status = inflate(&strm, Z_NO_FLUSH);
        string msg;
        switch (status) {
        case Z_NEED_DICT:
          msg = "Inflate failed: Z_NEED_DICT";
          break;
        case Z_DATA_ERROR:
          msg = "Inflate failed: Z_DATA_ERROR";
          break;
        case Z_MEM_ERROR:
          msg = "Inflate failed: Z_MEM_ERROR";
          break;
        case Z_STREAM_ERROR:
          msg = "Inflate failed: Z_STREAM_ERROR";
          break;
        }
        if (msg.size() > 0) {
          if (Z_STREAM_ERROR != status) {
            inflateEnd(&strm);
          }
          throw runtime_error(msg);
        }
        unsigned int bytes_written = Chunk - strm.avail_out;
        result += string((char *)out, bytes_written);
      } while (0 == strm.avail_out);
    } while (Z_STREAM_END != status);

    inflateEnd(&strm);
    if (Z_STREAM_END != status) {
      throw runtime_error("decompress - not all data processed");
    }
  }
  return result;
}
} // namespace gzip
} // namespace common
} // namespace mesaac
