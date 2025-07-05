// Shape Fingerprinter using quasi-Monte Carlo
// Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
// Author John MacCuish.
//
// This program takes as input an sd file of conformers, a file of Hammersly
// points arrayed in a scalene ellipsoid of a size into which small molecules
// will fit, in an orientation whereby the small molecule is aligned to the
// principle component.

// Step
// 1.  Find Hammersly points in atom defined volume of conformer and add to
//     atom center points.  Call this point data matrix X.
//
// 2.  Calculate eigenvectors and eigenvalues of X, and return X' rotated
//     to the eigenvector axes.
//
// 3.  Rotate X' minus atom points about eigenvector axes to obtain 8
//     fingerprints of length |rows of X'| minus atom points -- a bit
//     per point, such that a bit is 1 if the point is in the volume of the
//     conformer, 0 if the point lies outside any of the atoms (volume) of
//     the conformer.
//
// 4.  Output 4 fingerprints per conformer (optionally, output eigenvectors
//     eigenvalues, and rotation matrix per conformer).
//

#include <iostream>
#include <libgen.h>
#include <sstream>
#include <string>

#include "sdf_shape_fingerprinter.hpp"
#include "shared_types.hpp"

#include "mesaac_arg_parser/arg_parser.hpp"

using namespace std;

namespace {
using namespace mesaac::shape_fingerprinter;
using namespace mesaac::arg_parser;
struct CmdLine {
  Flag::Ptr id_flag =
      Flag::create("-i", "--id",
                   "include the name of each SD conformer after "
                   "each fingerprint");

  static constexpr std::string default_format = "A";
  static constexpr unsigned int default_folds = 0;

  Choice::Ptr format_opt = Choice::create(
      "-f", "--format", "write fingerprints in the specified format",
      {{.value = "A", .help = "ASCII (default)"},
       {.value = "B", .help = "binary"},
       {.value = "C", .help = "compressed ASCII"}});
  Option<unsigned int>::Ptr num_folds_opt =
      Option<unsigned int>::create("-n", "--num_folds",
                                   "fold fingerprints NUM_FOLDS times, to save "
                                   "space on output (default: 0 - not folded)");
  Option<std::string>::Ptr ellipsoid_opt = Option<std::string>::create(
      "-e", "--ellipsoid",
      "use points from the named file, containing 3D Hammersley "
      "ellipsoid points, "
      "one point per line with space-separated coords, for "
      "fingerprint generation");
  MultiValuedOption<unsigned int, 2>::Ptr records_opt =
      MultiValuedOption<unsigned int, 2>::create(
          "-r", "--records",
          "indices of first and last SD file records to process (default: "
          "process "
          "all records)");

  Argument<string>::Ptr sd_file = Argument<std::string>::create(
      "sd_file", "file of conformers in SD format, with 3D coordinates");
  Argument<string>::Ptr hamms_sphere_file = Argument<string>::create(
      "hamms_sphere_file",
      "file of 3D Hammersley sphere points, one point "
      "per line with space-separated coordinates, for principal "
      "axes generation via SVD and fingerprint generation");
  Argument<float>::Ptr atom_scale = Argument<float>::create(
      "atom_scale", "amount (1.0...2.0) by which to "
                    "increase atom radii for alignment");

  ArgParser parser = ArgParser(
      {id_flag, format_opt, num_folds_opt, ellipsoid_opt, records_opt},
      {sd_file, hamms_sphere_file, atom_scale},
      "Generate shape fingerprints for 3D conformers.");
};

auto get_format(ArgParser &parser, const Choice::Ptr format) {
  const auto format_str = format->value_or("A");
  if ("A" == format_str) {
    return SDFShapeFingerprinter::FMT_ASCII;
  } else if ("C" == format_str) {
    return SDFShapeFingerprinter::FMT_COMPRESSED_ASCII;
  } else if ("B" == format_str) {
    return SDFShapeFingerprinter::FMT_BINARY;
  }
  // Shouldn't get here -- parse_args should have caught invalid value.
  return SDFShapeFingerprinter::FMT_INVALID;
}

} // namespace

int main(int argc, const char **const argv) {

  CmdLine opts;
  const int status = opts.parser.parse_args(argc, argv);
  if (status != 0 || opts.parser.usage_requested()) {
    return status;
  }

  // Convert and validate options.
  const auto format = get_format(opts.parser, opts.format_opt);
  if (format == SDFShapeFingerprinter::FMT_INVALID) {
    return 1;
  }

  int start_index = 0, end_index = -1;
  if (opts.records_opt->has_values()) {
    start_index = opts.records_opt->values()[0];
    end_index = opts.records_opt->values()[1];
  }

  const auto spheroid = opts.hamms_sphere_file->value();
  const auto ellipsoid =
      opts.ellipsoid_opt->has_value() ? opts.ellipsoid_opt->value() : spheroid;

  const float atom_scale = opts.atom_scale->value();
  if (atom_scale < 1.0 || atom_scale > 2.0) {
    opts.parser.show_usage("atom_scale must be in the range 1.0...2.0");
    return 1;
  }

  SDFShapeFingerprinter sfper(opts.sd_file->value(), ellipsoid, spheroid,
                              atom_scale, opts.id_flag->value(), format,
                              opts.num_folds_opt->value_or(0));
  sfper.run(start_index, end_index);
  return 0;
}
