// Reads fingerprints from a named file.
// All fingerprints must have the same length.
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#ifndef _FINGERPRINT_READER4C1BC755_H_
#define _FINGERPRINT_READER4C1BC755_H_

#include <string>
#include <iostream>
#include <fstream>
#include "Globals.h"

namespace mesaac {
    class FingerprintReader {
    public:
        // Reads fingerprints from pathname
        // If pathname is "-", reads from cin.
        FingerprintReader(std::string& pathname);
        virtual ~FingerprintReader();
        
        // Read the next fingerprint.  Returns true if successful,
        // false if at end of file.
        bool next(BitVector& next_fp);
        
        // Read all remaining fingerprints.
        void read_all(ArrayBitVectors& all_fingerprints);
    
    protected:
        std::string m_pathname;
        std::ifstream m_inf;
        unsigned int m_fpnum;
        unsigned int m_fplen;
        
    private:
        FingerprintReader(const FingerprintReader& src);
        FingerprintReader& operator=(const FingerprintReader& src);
    };
} // namespace mesaac
#endif // _FINGERPRINT_READER4C1BC755_H_
