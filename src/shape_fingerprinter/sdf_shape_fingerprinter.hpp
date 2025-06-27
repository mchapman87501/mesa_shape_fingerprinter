//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>
#include <vector>

#include "shared_types.hpp"

namespace mesaac::shape_fingerprinter {
class SDFShapeFingerprinter {
public:
  typedef enum { FMT_ASCII, FMT_COMPRESSED_ASCII, FMT_BINARY } FormatEnum;

  SDFShapeFingerprinter(std::string sdPathname,
                        std::string hammsEllipsoidPathname,
                        std::string hammsSpherePathname, float radiiEpsilon,
                        bool includeIDs, FormatEnum format,
                        unsigned int numFolds);

  void run(int startIndex, int endIndex);

protected:
  std::string m_sdPathname;
  std::string m_hammsEllipsoidPathname;
  std::string m_hammsSpherePathname;

  const float m_epsilonSqr;
  bool m_includeIDs;
  FormatEnum m_format;
  unsigned int m_numFolds;

  void processMolecules(PointList &ellipsoid, PointList &sphere, int startIndex,
                        int endIndex);

private:
  SDFShapeFingerprinter(const SDFShapeFingerprinter &src);
  SDFShapeFingerprinter &operator=(const SDFShapeFingerprinter src);
};
} // namespace mesaac::shape_fingerprinter
