//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>
#include <vector>

// Singular value decomposition, for PCA -- this defines ap::real_2d_array
#include "svd.h"

#include "shared_types.hpp"

namespace mesaac::align_monte {
class SDFMolAligner {
public:
  SDFMolAligner(const std::string &sd_pathname,
                const std::string &hamms_sphere_pathname, float radii_epsilon,
                bool atom_centers_only, MeasuresList &measures,
                std::string sorted_pathname)
      : m_sd_pathname(sd_pathname),
        m_hamms_sphere_pathname(hamms_sphere_pathname),
        m_epsilon_sqr(radii_epsilon * radii_epsilon),
        m_atom_centers_only(atom_centers_only), m_measures(measures),
        m_sorted_pathname(sorted_pathname) {}

  void run();

protected:
  std::string m_sd_pathname;
  std::string m_hamms_sphere_pathname;

  const float m_epsilon_sqr;
  bool m_atom_centers_only;
  MeasuresList &m_measures;
  std::string m_sorted_pathname;

  PointList m_hamms_sphere_coords;
  shape_defs::BitVector m_ref_fingerprint;

  void read_sphere_points();
  void process_molecules();

private:
  SDFMolAligner(const SDFMolAligner &src);
  SDFMolAligner &operator=(const SDFMolAligner src);
};
} // namespace mesaac::align_monte
