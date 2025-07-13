#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "fingerprint_reader.hpp"
#include "measure_type_converter.hpp"

#include "mesaac_measures/measures_factory.hpp"
#include "mesaac_measures/shape_measures_factory.hpp"

#include "mesaac_shape/shared_types.hpp"

#include "mesaac_arg_parser/arg_parser.hpp"
#include "mesaac_arg_parser/value_converter.hpp"

using namespace std;

namespace {
using namespace mesaac::measures;

constexpr string Version = "1.2";
constexpr string CreationDate = "May, 2004";

void show_blurb() {
  cerr << "Running Measures.  Source code Copyright (c) 2002-2010 Mesa "
          "Analytics & Computing, Inc."
       << endl
       << "Version number " << Version << " Creation Date: " << CreationDate
       << endl;
}
} // namespace

namespace {
using namespace mesaac::arg_parser;

enum class OutputFormat {
  ordered_pair,
  matrix,
  sparse_matrix,
};

struct CmdParams {
  int parse_status;
  bool usage_requested;

  mesaac::measures::MeasureType measure_type;
  float tversky_alpha;
  bool compute_similarity;
  OutputFormat out_format;
  float sparse_threshold;
  std::filesystem::path fingerprint_file;
};

struct CmdLineParser {
  Choice::Ptr measure_choice = Choice::create(
      "-m", "--measure", "the measure to use",
      {
          {"B", "BUB measure"},
          {"C", "Cosine measure"},
          {"E", "Euclidean measure"},
          {"H", "Hamann measure"},
          {"T", "Tanimoto measure - default"},
          {"V",
           "Tversky measure - can be used in conjunction with -a | --alpha"},
      });

  Option<float>::Ptr alpha_opt = Option<float>::create(
      "-a", "--alpha",
      "alpha value to use for measure V (Tversky) - default is 0.0");

  Flag::Ptr dissim = Flag::create(
      "-d", "--dissimilarity",
      "compute dissimilarity values - default is to compute similarity");

  Choice::Ptr format_choice = Choice::create(
      "-f", "--format", "how to format the output - default is 'S'",
      {
          {"M", "full matrix"},
          {"S", "sparse matrix"},
          {"O", "ordered pairs"},
      });

  Option<float>::Ptr sparse_opt =
      Option<float>::create("-t", "--threshold",
                            "(dis)similarity sparse threshold to use for "
                            "output format S - default is 1.0");

  Argument<std::filesystem::path>::Ptr fp_path_arg =
      Argument<std::filesystem::path>::create(
          "fingerprint_file",
          "plaintext file of binary fingerprints, one per line");

  ArgParser parser = ArgParser(
      {measure_choice, alpha_opt, dissim, format_choice, sparse_opt},
      {fp_path_arg}, "Print pairwise measures for a set of fingerprints.");

  CmdParams parse_args(int argc, const char *argv[]) {
    CmdParams result{
        .parse_status = 0,
        .usage_requested = false,
        .measure_type = MeasureType::tanimoto,
        .tversky_alpha = 0.0,
        .compute_similarity = true,
        .out_format = OutputFormat::sparse_matrix,
        .sparse_threshold = 1.0,
        .fingerprint_file = std::filesystem::path(""),
    };
    result.parse_status = parser.parse_args(argc, argv);
    result.usage_requested = parser.usage_requested();
    if (result.parse_status != 0 || result.usage_requested) {
      return result;
    }
    const auto measure_str = measure_choice->value_or("T");
    result.measure_type =
        mesaac::cli::measures::get_measure_type(measure_str.at(0));

    result.tversky_alpha = alpha_opt->value_or(0.0);

    result.compute_similarity = !dissim->value();

    const auto format_str = format_choice->value_or("S");
    if (format_str == "M") {
      result.out_format = OutputFormat::matrix;
    } else if (format_str == "O") {
      result.out_format = OutputFormat::ordered_pair;
    } else if (format_str == "S") {
      result.out_format = OutputFormat::sparse_matrix;
    } else {
      // Should not get here.
      parser.show_usage("Unknown format value '" + format_str + "'");
      result.parse_status = 1;
      return result;
    }
    result.sparse_threshold = sparse_opt->value_or(1.0);
    result.fingerprint_file = fp_path_arg->value();
    return result;
  }

  void show_usage(const std::string &err_msg) { parser.show_usage(err_msg); }
};

void output_ordered_pairs(size_t num_fingerprints,
                          shape::IIndexedShapeFPMeasure::Ptr measurer) {
  for (size_t i = 0; i < num_fingerprints; i++) {
    for (size_t j = 0; j < num_fingerprints; j++) {
      cout << i << " " << j << " " << measurer->value(i, j) << endl;
    }
  }
}

void output_full_matrix(size_t num_fingerprints,
                        shape::IIndexedShapeFPMeasure::Ptr measurer) {
  for (size_t i = 0; i < num_fingerprints; i++) {
    string sep("");
    for (size_t j = 0; j < num_fingerprints; j++) {
      cout << sep << measurer->value(i, j);
      sep = " ";
    }
    cout << endl;
  }
}

void output_sparse_sim_matrix(size_t num_fingerprints,
                              shape::IIndexedShapeFPMeasure::Ptr measurer,
                              const float sparse_threshold) {
  for (size_t i = 0; i < num_fingerprints; i++) {
    for (size_t j = 0; j < num_fingerprints; j++) {
      if (i != j) {
        float v = measurer->value(i, j);
        if (sparse_threshold <= v) {
          cout << j << " " << v << " ";
        }
      }
    }
    cout << -1 << endl;
  }
}

void output_sparse_dist_matrix(size_t num_fingerprints,
                               shape::IIndexedShapeFPMeasure::Ptr measurer,
                               const float sparse_threshold) {
  for (size_t i = 0; i < num_fingerprints; i++) {
    for (size_t j = 0; j < num_fingerprints; j++) {
      if (i != j) {
        float v = measurer->value(i, j);
        if (sparse_threshold >= v) {
          cout << j << " " << v << " ";
        }
      }
    }
    cout << -1 << endl;
  }
}

void output_results(const size_t num_fingerprints,
                    const OutputFormat &out_format,
                    shape::IIndexedShapeFPMeasure::Ptr measurer,
                    const bool compute_similarity,
                    const float sparse_threshold) {

  switch (out_format) {
  case OutputFormat::matrix:
    output_full_matrix(num_fingerprints, measurer);
    break;

  case OutputFormat::ordered_pair:
    output_ordered_pairs(num_fingerprints, measurer);
    break;

  case OutputFormat::sparse_matrix:
    if (compute_similarity) {
      output_sparse_sim_matrix(num_fingerprints, measurer, sparse_threshold);
    } else {
      output_sparse_dist_matrix(num_fingerprints, measurer, sparse_threshold);
    }
    break;
  }
  cerr << "Internal error: Unknown output format value" << endl;
}
} // namespace

int main(int argc, const char **argv) {
  using namespace mesaac;
  using namespace mesaac::measures;
  using namespace mesaac::cli::measures;

  show_blurb();

  CmdLineParser parser;

  const auto params = parser.parse_args(argc, argv);
  if (params.parse_status != 0) {
    return params.parse_status;
  }
  if (params.usage_requested) {
    return 0;
  }

  shape_defs::ArrayBitVectors fingerprints;
  cli::measures::read_fingerprints(params.fingerprint_file, fingerprints);

  auto measure =
      mesaac::measures::get_measures(params.measure_type, params.tversky_alpha);
  const auto measurer = mesaac::measures::shape::get_fp_measurer(
      measure, params.compute_similarity, fingerprints);
  if (0 == measurer) {
    ostringstream msg;
    msg << "Unknown measure -" << parser.measure_choice->value();
    parser.show_usage(msg.str());
    return 1;
  }

  const unsigned int num_fingerprints = fingerprints.size();
  output_results(num_fingerprints, params.out_format, measurer,
                 params.compute_similarity, params.sparse_threshold);
  return 0;
}
