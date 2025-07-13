#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <libgen.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "mesaac_measures/measures_factory.hpp"
#include "mesaac_measures/shape_measures_factory.hpp"

#include "mesaac_common/b64.hpp"
#include "mesaac_common/gzip.hpp"

#include "mesaac_arg_parser/arg_parser.hpp"

#include "fp_decoder.hpp"
#include "measure_type_converter.hpp"

using namespace std;

namespace {
using namespace mesaac::arg_parser;

enum class OutputFormat {
  matrix,
  sparse_matrix,
  pvm,
};

struct CmdParams {
  int parse_status;
  bool usage_requested;

  mesaac::measures::MeasureType measure_type;
  float tversky_alpha;
  bool compute_similarity;
  unsigned int search_index;
  OutputFormat out_format;
  float sparse_threshold;
  filesystem::path fingerprints_path;
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

  Option<unsigned int>::Ptr search_opt = Option<unsigned int>::create(
      "-s", "--search",
      ("compare each of the first SEARCH fingerprints in shape_fingerprints "
       "with each of the remaining\n"
       "        (num-fingerprints - SEARCH) fingerprints - default is 0, to "
       "compare all pairs of fingerprints"));

  Choice::Ptr format_choice = Choice::create(
      "-f", "--format", "how to format the output - default is 'S'",
      {
          {"M", "full matrix"},
          {"S", "sparse matrix"},
          {"P", "PVM"},
      });

  Option<float>::Ptr sparse_opt =
      Option<float>::create("-t", "--threshold",
                            "(dis)similarity sparse threshold to use for "
                            "output formats S and P - default is 1.0");

  Argument<filesystem::path>::Ptr fingerprints_arg =
      Argument<filesystem::path>::create(
          "shape_fingerprints", "plaintext file of shape fingerprints");

  ArgParser parser = ArgParser(
      {measure_choice, alpha_opt, dissim, search_opt, format_choice,
       sparse_opt},
      {fingerprints_arg}, "Print pairwise measures of a set of fingerprints.");

  CmdParams parse_args(int argc, const char *argv[]) {
    CmdParams result{.parse_status = 0,
                     .usage_requested = false,
                     .measure_type = mesaac::measures::MeasureType::tanimoto,
                     .tversky_alpha = 0.0,
                     .compute_similarity = true,
                     .search_index = 0, // In effect, no search
                     .out_format = OutputFormat::sparse_matrix,
                     .sparse_threshold = 1.0,
                     .fingerprints_path = filesystem::path("")};

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
    result.search_index = search_opt->value_or(0);

    const auto format_str = format_choice->value_or("S");
    if (format_str == "M") {
      result.out_format = OutputFormat::matrix;
    } else if (format_str == "P") {
      result.out_format = OutputFormat::pvm;
    } else if (format_str == "S") {
      result.out_format = OutputFormat::sparse_matrix;
    } else {
      // Should not get here.
      parser.show_usage("Unknown format value '" + format_str + "'");
      result.parse_status = 1;
      return result;
    }

    result.sparse_threshold = sparse_opt->value_or(1.0);
    result.fingerprints_path = fingerprints_arg->value();
    return result;
  }

  void show_usage(const string &err_msg) { parser.show_usage(err_msg); }
};
} // namespace

namespace {
void read_fpblocks_from_stream(
    const string &pathname, istream &ins,
    mesaac::shape_defs::ShapeFPBlocks &fingerprints) {
  mesaac::shape_defs::ArrayBitVectors block;
  const unsigned int FPsPerBlock = 4;
  bool first = true;
  unsigned int vector_size = 0;
  string fpstr;
  unsigned int line_num = 0;

  fingerprints.clear();
  while (ins >> fpstr) {
    line_num++;
    mesaac::shape_defs::BitVector fp;
    if (!decode_fp(fpstr, fp)) {
      cerr << "Error at line " << line_num << " of " << pathname << ":" << endl
           << "  Invalid fingerprint string '" << fpstr << "'." << endl;
      exit(1);
    } else {
      if (first) {
        vector_size = fp.size();
        first = false;
      } else if (fp.size() != vector_size) {
        // TODO:  Use exceptions, or an error return
        cerr << "Error at line " << line_num << " of " << pathname << ":"
             << endl
             << "  Expected fingerprint of size " << vector_size
             << ", got fingerprint of size " << fp.size() << endl;
        exit(1);
      }
      block.push_back(mesaac::shape_defs::BitVector(fp));
      if (block.size() == FPsPerBlock) {
        fingerprints.push_back(block);
        block.clear();
      }
    }
  }
  // Discard any leftover sub-block.
  cerr << "Number of fingerprints is " << fingerprints.size() << endl
       << fingerprints.size() << " " << FPsPerBlock << " " << vector_size
       << endl;

  if (block.size() != 0) {
    cerr << "A Shape Fingerprint file must contain blocks of " << FPsPerBlock
         << " fingerprints.  " << endl
         << pathname << " has only " << block.size()
         << ((block.size() == 1) ? "fingerprint " : "fingerprints ")
         << "in its last block." << endl
         << "This may not be a shape fingerprint file." << endl;
    exit(1);
  }
}

void read_fingerprint_blocks(const string pathname,
                             mesaac::shape_defs::ShapeFPBlocks &fingerprints) {
  // Input from either stdin or file
  if (pathname == "-") {
    read_fpblocks_from_stream("standard input", cin, fingerprints);
  } else {
    ifstream inf(pathname.c_str());
    if (!inf) {
      // TODO:  Use exceptions, or an error return
      cerr << "Cannot open fingerprint file " << pathname << "." << endl;
      exit(1);
    }
    read_fpblocks_from_stream(pathname, inf, fingerprints);
    inf.close();
  }
}

// Compare-and-print functions:
void compute_and_output_matrix(
    const CmdParams &params,
    mesaac::measures::shape::IIndexedShapeFPMeasure::Ptr const measurer,
    const mesaac::shape_defs::ShapeFPBlocks &fps) {

  if (params.search_index > 0) {
    cerr << "Warning: --search is ignored for --format M." << endl;
  }
  for (unsigned int i = 0; i < fps.size(); ++i) {
    string sep("");
    for (unsigned int j = 0; j < fps.size(); ++j) {
      cout << sep << measurer->value(i, j);
      sep = " ";
    }
    cout << endl;
  }
}

function<bool(float)> get_thresh_filter(bool compute_similarity,
                                        const float threshold) {
  const function<bool(float)> above_thresh = [threshold](const float value) {
    return value >= threshold;
  };
  const function<bool(float)> below_thresh = [threshold](const float value) {
    return value <= threshold;
  };
  return compute_similarity ? above_thresh : below_thresh;
}

void fail_bad_search_index(unsigned int search_index, unsigned int num_fps) {
  ostringstream outs;
  outs << "Error: search index (" << search_index
       << ") must be less than number of fingerprints - 1 (" << num_fps - 1
       << ")" << endl;
  throw invalid_argument(outs.str());
}

void compute_and_output_sparse_matrix(
    const CmdParams &params,
    mesaac::measures::shape::IIndexedShapeFPMeasure::Ptr const measurer,
    const mesaac::shape_defs::ShapeFPBlocks &fps) {

  const auto should_output =
      get_thresh_filter(params.compute_similarity, params.sparse_threshold);

  const bool is_searching = params.search_index > 0;
  const size_t num_fps = fps.size();
  if (is_searching && params.search_index >= (num_fps - 1)) {
    fail_bad_search_index(params.search_index, num_fps);
  }

  const size_t i_end = is_searching ? params.search_index : num_fps;
  const size_t j_start = is_searching ? params.search_index : 0;

  for (size_t i = 0; i < i_end; ++i) {
    if (is_searching) {
      cout << i << " ";
    }
    for (size_t j = j_start; j < num_fps; ++j) {
      if (i != j) {
        const float value = measurer->value(i, j);
        if (should_output(value)) {
          cout << j << " " << value << " ";
        }
      }
    }
    cout << -1 << endl;
  }
}

void compute_and_output_pvm(
    const CmdParams &params,
    mesaac::measures::shape::IIndexedShapeFPMeasure::Ptr const measurer,
    const mesaac::shape_defs::ShapeFPBlocks &fps) {

  const auto should_output =
      get_thresh_filter(params.compute_similarity, params.sparse_threshold);
  const size_t num_fps = fps.size();
  const size_t search_index = params.search_index;
  if (search_index >= (num_fps - 1)) {
    fail_bad_search_index(search_index, num_fps);
  }

  for (size_t i = 0; i < search_index; ++i) {
    for (size_t j = search_index; j < num_fps; ++j) {
      const float value = measurer->value(i, j);
      if (should_output(value)) {
        cout << (j - search_index) << " " << value << " ";
      }
    }
    cout << -1 << endl;
  }
}

int compute_and_output_results(
    const CmdLineParser &parser, const CmdParams &params,
    mesaac::measures::shape::IIndexedShapeFPMeasure::Ptr const measurer,
    const mesaac::shape_defs::ShapeFPBlocks &fps) {
  switch (params.out_format) {
  case OutputFormat::matrix:
    compute_and_output_matrix(params, measurer, fps);
    return 0;

  case OutputFormat::sparse_matrix:
    compute_and_output_sparse_matrix(params, measurer, fps);
    return 0;

  case OutputFormat::pvm:
    compute_and_output_pvm(params, measurer, fps);
    return 0;
  }

  cerr << "Internal error: unknown format '" << parser.format_choice->value()
       << "'" << endl;
  return 2;
}
} // namespace

int main(int argc, const char **argv) {
  CmdLineParser parser;
  const CmdParams params = parser.parse_args(argc, argv);

  if (params.usage_requested) {
    return 0;
  }
  if (params.parse_status != 0) {
    return params.parse_status;
  }

  mesaac::shape_defs::ShapeFPBlocks fingerprints;
  read_fingerprint_blocks(params.fingerprints_path, fingerprints);

  auto measure =
      mesaac::measures::get_measures(params.measure_type, params.tversky_alpha);
  auto measurer = mesaac::measures::shape::get_shape_measurer(
      measure, params.compute_similarity, fingerprints);
  if (0 == measurer) {
    cerr << "Internal error - could not create shape measurer." << endl;
    return 2;
  }

  return compute_and_output_results(parser, params, measurer, fingerprints);
}
