// Unit test for shape_measures_factory
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include "mesaac_measures/bub.hpp"
#include "mesaac_measures/cosine.hpp"
#include "mesaac_measures/euclidean.hpp"
#include "mesaac_measures/hamann.hpp"
#include "mesaac_measures/measures_base.hpp"
#include "mesaac_measures/shape_measures_factory.hpp"
#include "mesaac_measures/tanimoto.hpp"
#include "mesaac_measures/tversky.hpp"

using namespace std;

namespace mesaac::measures {

namespace {

// Get a random number i, i_min <= i < i_max.
// This is probably wrong :)
unsigned int rand_range(unsigned int i_min, unsigned int i_max) {
  const unsigned int full_range = 0xFFFFFFFF;
  unsigned int scaled_range = i_max - i_min;
  unsigned int result = i_min + (random() * scaled_range) / full_range;
  return result;
}

class FPGroupGen {
public:
  FPGroupGen(unsigned int numbits, const MeasuresBase::Ptr m,
             float tversky_alpha = 0.0)
      : m_numbits(numbits), m_measure(m), m_tversky_alpha(tversky_alpha),
        m_max(1 << m_numbits), m_mask(m_max - 1) {}

  string name() const {
    ostringstream result;
    result << basename() << "(" << m_numbits;
    result << ", " << m_measure->name();
    result << ", talpha=" << m_tversky_alpha;
    result << ", " << (is_similarity() ? "similarity" : "distance");
    result << ")";
    return result.str();
  }

  unsigned int numbits() const { return m_numbits; }

  const MeasuresBase::Ptr measure() const { return m_measure; }

  // Does this generator produce similarity values?
  virtual bool is_similarity() const { return true; }

  float tversky_alpha() const { return m_tversky_alpha; }

  void append_random_bitvectors(const unsigned int num_bvs,
                                shape_defs::ArrayBitVectors &result) {
    for (unsigned int i = 0; i != num_bvs; ++i) {
      result.push_back(make_bv(random() & m_mask));
    }
  }

  void gen_ref(unsigned int value, shape_defs::ArrayBitVectors &result) {
    result.clear();
    result.push_back(make_bv(value));
    append_random_bitvectors(3, result);
    m_ref = result;
  }

  void gen_cmp(unsigned int value, shape_defs::ArrayBitVectors &result,
               float &best_sim) const {
    result.clear();
    shape_defs::BitVector c1(m_numbits, value);
    result.push_back(c1);

    const shape_defs::BitVector &ref_fp(m_ref.at(0));
    best_sim = get_similarity_measure(ref_fp, c1);
    // Generate other members at random.  Okay, not really at random.
    for (unsigned int i = 0; (result.size() < 4) && (i != m_max); ++i) {
      shape_defs::BitVector fp(make_bv(i));
      if (!exceeds_best_measure(get_similarity_measure(ref_fp, fp), best_sim)) {
        result.push_back(fp);
      }
    }
    // Maybe there aren't enough distinct values within m_numbits,
    // to fill out the group.  In that case, duplicate some fps.
    while (result.size() != 4) {
      result.push_back(result.at(result.size() - 1));
    }
    REQUIRE(result.size() == 4);

    shuffle_group(result);
  }

protected:
  const unsigned int m_numbits, m_max, m_mask;
  const MeasuresBase::Ptr m_measure;
  const float m_tversky_alpha;

  shape_defs::ArrayBitVectors m_ref;

  virtual string basename() const { return "Similarity"; }

  shape_defs::BitVector make_bv(unsigned int value) const {
    return shape_defs::BitVector(m_numbits, value);
  }

  virtual float get_similarity_measure(const shape_defs::BitVector &v1,
                                       const shape_defs::BitVector &v2) const {
    return m_measure->similarity(v1, v2);
  }

  virtual bool exceeds_best_measure(float v, float best) const {
    return v > best;
  }

  void shuffle_group(shape_defs::ArrayBitVectors &v) const {
    // Shuffle the entries of v.
    swap(v.at(0), v.at(rand_range(0, 4)));
    swap(v.at(0), v.at(rand_range(0, 4)));
    swap(v.at(0), v.at(rand_range(0, 4)));
  }
};

class FPDistGroupGen : public FPGroupGen {
public:
  FPDistGroupGen(unsigned int numbits, const MeasuresBase::Ptr m,
                 float tversky_alpha = 0.0)
      : FPGroupGen(numbits, m, tversky_alpha) {}
  bool is_similarity() const override { return false; }

protected:
  string basename() const override { return "Distance"; }

  float get_similarity_measure(const shape_defs::BitVector &v1,
                               const shape_defs::BitVector &v2) const override {
    return m_measure->distance(v1, v2);
  }

  bool exceeds_best_measure(float v, float best) const override {
    // Any distance < best exceeds best.
    return v < best;
  }
};

void test_measurer(FPGroupGen &&fp_gen) {
  // Exercise a measurer.
  // Measurers compute pairwise similarities or distances over arrays of bit
  // vectors.

  // Some measure types produce asymmetric measures.
  // Others produce values that are not in the range 0...1;
  // for example, Hamann produces a value ranging from -1 for
  // perfect disagreement to +1 for perfect agreement.
  // In the mesaac_measures implementation of distance measures, this means
  // Hamann distances should range from 2 for perfect disagreement
  // to 0 for perfect agreement.

  // So what can be tested here?

  // Test the extreme cases:
  // * Perfect similarity
  // * Perfect dissimilarity
  const int num_bits = 8;
  const shape_defs::ArrayBitVectors bit_vectors{{num_bits, 0b00000000},
                                                {num_bits, 0b11111111}};

  // Similarity Category
  enum class SimCat { same, disjoint, similar };

  const auto sim_cat_str = [](const SimCat cat) {
    if (cat == SimCat::same) {
      return "completely similar";
    } else if (cat == SimCat::disjoint) {
      return "completely different";
    }
    return "partially similar";
  };

  // Expected outcome matrix assumes column-major evaluation order.
  const std::vector<std::vector<SimCat>> expected_outcomes{
      // compare 0 to other indices
      {SimCat::same, SimCat::disjoint},
      // compare 1 to other indices
      {SimCat::disjoint, SimCat::same},
  };

  const auto get_similarity = [](FPGroupGen &fp_gen, const float score) {
    // XXX FIX THIS MeasuresBase needs to be able to report its
    // range of similarity values and its range of distance values.

    // What should be the score for identical bit vectors?
    const auto is_hamann = fp_gen.measure()->name() == "Hamann";
    const float same_score = is_hamann ? (fp_gen.is_similarity() ? 1.0 : 0.0)
                                       : (fp_gen.is_similarity() ? 1.0 : 0.0);

    // What should be the score for completely different bit vectors?
    const float disjoint_score = is_hamann
                                     ? (fp_gen.is_similarity() ? -1.0 : 2.0)
                                     : (fp_gen.is_similarity() ? 0.0 : 1.0);
    if (score == same_score) {
      return SimCat::same;
    } else if (score == disjoint_score) {
      return SimCat::disjoint;
    } else {
      return SimCat::similar;
    }
  };

  const auto measurer = shape::get_fp_measurer(
      fp_gen.measure(), fp_gen.is_similarity(), bit_vectors);
  REQUIRE(measurer != nullptr);

  for (size_t i = 0; i != bit_vectors.size(); ++i) {
    for (size_t j = 0; j != bit_vectors.size(); ++j) {
      // Two bit_vectors entries could randomly have the same bit sequence,
      // hence comparing values instead of simply comparing the indices.
      const float actual_value(measurer->value(i, j));
      // Categorize the similarity.  This is difficult w.o. knowing the
      // range of similarity values for a particular measure.
      const SimCat actual_category(get_similarity(fp_gen, actual_value));
      const SimCat expected_category = expected_outcomes[i][j];
      if (actual_category != expected_category) {
        ostringstream msg;
        msg << "For " << fp_gen.name() << ", bit vectors " << hex
            << bit_vectors[i] << ", " << hex << bit_vectors[j] << ": expected "
            << sim_cat_str(expected_category) << "; got "
            << sim_cat_str(actual_category) << ", value " << actual_value;
        INFO(msg.str());
        CHECK(false);
      }
    }
  }
}

void test_shape_measurer(FPGroupGen &&fp_gen) {
  // Try to verify that the shape similarity measurer is
  // really finding the best similarity score between two
  // fingerprint groups.

  // Approach:
  // for every fingerprint FP1 of length n bits:
  //     generate a 1st fingerprint consisting of
  //         FP1
  //         3 randomly chosen other FPs
  //           (the measurer should do all of its
  //           comparisons using FP1, so these don't matter)
  //     for every fingerprint FP2 (including FP):
  //         generate a 2nd fingerprint consisting of
  //             FP2
  //             3 randomly chosen other FPs, each of which
  //               is less similar to FP1 than FP2 is.
  //         verify that the measure's similarity value for
  //           these two groups is similarity(FP1, FP2)
  const unsigned int num_fps(1 << fp_gen.numbits());
  for (unsigned int i = 0; i != num_fps; ++i) {
    shape_defs::ArrayBitVectors first;
    fp_gen.gen_ref(i, first);

    shape_defs::ShapeFPBlocks all_fps;
    all_fps.push_back(first);

    for (unsigned int j = 0; j != num_fps; ++j) {
      float best_sim = 0;
      shape_defs::ArrayBitVectors second;
      fp_gen.gen_cmp(j, second, best_sim);

      all_fps.push_back(second);

      const auto measurer = shape::get_shape_measurer(
          fp_gen.measure(), fp_gen.is_similarity(), all_fps);
      REQUIRE(measurer != nullptr);

      // Ensure that self-comparisons always return absolute similarity (or
      // dissimilarity).
      const float expected_self_measure = fp_gen.is_similarity() ? 1.0f : 0.0f;
      REQUIRE(measurer->value(0, 0) == expected_self_measure);

      float actual(measurer->value(0, 1));

      const auto expected = Catch::Matchers::WithinAbs(best_sim, 1.0e-3);
      if (!expected.match(actual)) {
        ostringstream msg;
        msg << "For " << fp_gen.name() << ", values " << hex << i << ", " << hex
            << j << "; expected best_sim " << best_sim << ", got " << actual;
        INFO(msg.str());
        CHECK(false);
      }

      all_fps.pop_back();
    }
  }
}

void test_shape_pair_measurer(FPGroupGen &&fp_gen) {
  // Although it isn't well defined in the type declarations,
  // a shape fingerprint is a sequence of 4 equal-length bit vectors.
  // Each bit vector represents the overlap between a conformer volume and
  // points of a spheroid, for a single orientation of the conformer.

  // An IShapeFPMeasure reports the highest similarity between
  // the first fingerprint of fp1 and any fingerprint of fp2, when
  // measuring similarity.
  // When measuring distance, it reports the minimum distance.

  mesaac::shape::ShapeFingerprint fp1;
  mesaac::shape::ShapeFingerprint fp2;

  // TBD
}

TEST_CASE("mesaac::measures::shape_measures_factory",
          "[mesaac][mesaac_measures]") {
  constexpr int num_bits = 4;
  float tversky_alpha(0.85);

  auto tani_measure = std::make_shared<Tanimoto>();
  auto cosine_measure = std::make_shared<Cosine>();
  auto euclidean_measure = std::make_shared<Euclidean>();
  auto hamann_measure = std::make_shared<Hamann>();
  auto tversky_measure = std::make_shared<Tversky>(tversky_alpha);
  auto bub_measure = std::make_shared<BUB>();

  const std::vector<MeasuresBase::Ptr> measures{
      tani_measure,   cosine_measure,  euclidean_measure,
      hamann_measure, tversky_measure, bub_measure};

  SECTION("measurers") {

    SECTION("Measurers") {
      SECTION("Test similarity") {
        test_measurer(FPGroupGen(num_bits, tani_measure));
        test_measurer(FPGroupGen(num_bits, cosine_measure));
        test_measurer(FPGroupGen(num_bits, euclidean_measure));
        test_measurer(FPGroupGen(num_bits, hamann_measure));
        test_measurer(FPGroupGen(num_bits, tversky_measure));
        test_measurer(FPGroupGen(num_bits, bub_measure));
      }

      SECTION("Test distance") {
        test_measurer(FPDistGroupGen(num_bits, tani_measure));
        test_measurer(FPDistGroupGen(num_bits, cosine_measure));
        test_measurer(FPDistGroupGen(num_bits, euclidean_measure));
        test_measurer(FPDistGroupGen(num_bits, hamann_measure));
        test_measurer(FPDistGroupGen(num_bits, tversky_measure));
        test_measurer(FPDistGroupGen(num_bits, bub_measure));
      }
    }

    SECTION("Shape measurers") {
      SECTION("Test shape similarity") {
        test_shape_measurer(FPGroupGen(num_bits, tani_measure));
        test_shape_measurer(FPGroupGen(num_bits, cosine_measure));
        test_shape_measurer(FPGroupGen(num_bits, euclidean_measure));
        test_shape_measurer(FPGroupGen(num_bits, hamann_measure));
        test_shape_measurer(FPGroupGen(num_bits, tversky_measure));
        test_shape_measurer(FPGroupGen(num_bits, bub_measure));
      } // namespace

      SECTION("Test shape distance") {
        test_shape_measurer(FPDistGroupGen(num_bits, tani_measure));
        test_shape_measurer(FPDistGroupGen(num_bits, cosine_measure));
        test_shape_measurer(FPDistGroupGen(num_bits, euclidean_measure));
        test_shape_measurer(FPDistGroupGen(num_bits, hamann_measure));
        test_shape_measurer(FPDistGroupGen(num_bits, tversky_measure));
        test_shape_measurer(FPDistGroupGen(num_bits, bub_measure));
      }
    }

    SECTION("Shape Pair Measurers") {
      SECTION("Test shape pair similarity") {
        // TBD
      }

      SECTION("Test shape pair distance") {
        // TBD
      }
    }
  }

  SECTION("Getting fingerprint measures") {
    mesaac::shape::ShapeFingerprintVector empty_shape_fingerprints;
    mesaac::shape::ShapeFingerprint empty_sfp;

    SECTION("Instantiating measurers for valid measure types") {
      const std::vector<bool> compute_sim_values{false, true};

      SECTION("shape::get_fp_measurer") {
        for (const auto compute_sim : compute_sim_values) {
          for (const auto measure : measures) {
            const auto measurer =
                shape::get_fp_measurer(measure, compute_sim, empty_sfp);
            REQUIRE(measurer != nullptr);
          }
        }
      }

      SECTION("shape::get_shape_measurer") {
        for (const auto compute_sim : compute_sim_values) {
          for (const auto measure : measures) {
            const auto measurer = shape::get_shape_measurer(
                measure, compute_sim, empty_shape_fingerprints);
            REQUIRE(measurer != nullptr);
          }
        }
      }

      SECTION("shape::get_shape_pair_measurer") {
        for (const auto compute_sim : compute_sim_values) {
          for (const auto measure : measures) {
            const auto measurer =
                shape::get_shape_pair_measurer(measure, compute_sim);
            REQUIRE(measurer != nullptr);
          }
        }
      }
    }
  }
}
} // namespace
} // namespace mesaac::measures
