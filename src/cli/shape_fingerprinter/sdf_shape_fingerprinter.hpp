//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>
#include <vector>

#include "mesaac_shape/shared_types.hpp"

namespace mesaac::shape_fingerprinter {
class SDFShapeFingerprinter {
public:
  typedef enum {
    FMT_ASCII,
    FMT_COMPRESSED_ASCII,
    FMT_BINARY,
    FMT_INVALID
  } FormatEnum;

  SDFShapeFingerprinter(std::string sd_pathname,
                        std::string hamms_ellipsoid_pathname,
                        std::string hams_sphere_pathname, float radii_epsilon,
                        bool include_ids, FormatEnum format,
                        unsigned int num_folds);

  void run(int start_index, int end_index);

protected:
  std::string m_sd_pathname;
  std::string m_hamms_ellipsoid_pathname;
  std::string m_hamms_sphere_pathname;

  const float m_epsilon_sqr;
  bool m_include_ids;
  FormatEnum m_format;
  unsigned int m_num_folds;

  void process_molecules(shape::Point3DList &ellipsoid,
                         shape::Point3DList &sphere, int start_index,
                         int end_index);

private:
  SDFShapeFingerprinter(const SDFShapeFingerprinter &src);
  SDFShapeFingerprinter(SDFShapeFingerprinter &&src);
  SDFShapeFingerprinter &operator=(const SDFShapeFingerprinter &src);
  SDFShapeFingerprinter &operator=(SDFShapeFingerprinter &&src);
};
} // namespace mesaac::shape_fingerprinter
