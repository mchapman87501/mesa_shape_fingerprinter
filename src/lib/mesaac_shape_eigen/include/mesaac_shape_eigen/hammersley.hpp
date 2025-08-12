// Provides a Hammersley point set generator.
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_shape_eigen/shared_types.hpp"

namespace mesaac::shape_eigen {

/**
 * @brief Generates Hammersley points.
 */
struct Hammersley {
  /**
   * @brief Parameters to use when generating a spheroid point set
   */
  struct EllipsoidParams {
    /**
     * @brief max mumber of points to generate
     */
    const size_t num_points;

    /**
     * @brief extent of the the largest ellipsoid axis
     */
    const float scale;

    /**
     * @brief ellipsoid x axis scale
     */
    const float a;

    /**
     * @brief ellipsoid y axis scale
     */
    const float b;

    /**
     * @brief ellipsoid z axis scale
     */
    const float c;
  };

  /**
   * @brief Parameters to use when generating a cuboid point set
   */
  struct CuboidParams {
    /**
     * @brief max mumber of points to generate
     */
    const size_t num_points;

    /**
     * @brief minimum x coordinate of the point set
     */
    const float xmin;

    /**
     * @brief maximum x coordinate of the point set
     */
    const float xmax;

    /**
     * @brief minimum y coordinate of the point set
     */
    const float ymin;

    /**
     * @brief maximum y coordinate of the point set
     */
    const float ymax;

    /**
     * @brief minimum z coordinate of the point set
     */
    const float zmin;

    /**
     * @brief maximum z coordinate of the point set
     */
    const float zmax;
  };

  /**
   * @brief Create a (3D) Hammersley spheroid point generator.
   * @param num_points the number of points to generate
   */
  Hammersley(const size_t num_points)
      : m_num_points(num_points), m_point_index(0) {}

  /**
   * @brief Get a Hammersley spheroid point set.
   * @param params specifies the point set to be generated
   * @param result on return, the generated points
   */
  static void get_ellipsoid(const EllipsoidParams &params, PointList &result);

  /**
   * @brief Get a Hammersley cuboid point set.
   * @param params specifies the point set to be generated
   * @param result on return, the generated points
   */
  static void get_cuboid(const CuboidParams &params, PointList &result);

private:
  const size_t m_num_points;
  unsigned int m_point_index;

  Point next_point();
};

} // namespace mesaac::shape_eigen
