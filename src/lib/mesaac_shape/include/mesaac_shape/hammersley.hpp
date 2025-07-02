// Provides a Hammersley point set generator.
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_shape/shared_types.hpp"

namespace mesaac::shape {

class Hammersley {
public:
  static void get_cubic(float xmin, float xmax, float ymin, float ymax,
                        float zmin, float zmax, unsigned int num_points,
                        PointList &result);

  Hammersley();

  void start(unsigned int num_points);
  bool next_point(Point &p);

protected:
  unsigned int m_num_points;
  unsigned int m_num_generated;

private:
  Hammersley(const Hammersley &src);
  Hammersley &operator=(const Hammersley &src);
};

} // namespace mesaac::shape
