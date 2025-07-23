//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>
#include <vector>

/// @brief Namespace for molecular structures
namespace mesaac::mol {

/// @enum BondType
/// @brief Valid Bond types
enum class BondType : unsigned int {
  bt_single = 1,             ///< single bond
  bt_double = 2,             ///< double bond
  bt_triple = 3,             ///< triple bond
  bt_aromatic = 4,           ///< aromatic bond
  bt_single_or_double = 5,   ///< single or double bond
  bt_single_or_aromatic = 6, ///< single or aromatic bond
  bt_double_or_aromatic = 7, ///< double or aromatic bond
  bt_any = 8                 ///< any bond type
};

/// @enum BondStereo
/// @brief Valid bond stereochemistry values
enum class BondStereo : unsigned int {
  /// NOTE: This does not match the 2010 CTfile format specification.
  /// It refers to Bond configuration, not Bond stereo values, and it
  /// lists the following values:
  /// 0 = none (default)
  /// 1 = up
  /// 2 = either
  /// 3 = down

  /// These values are for V2000 and are from
  /// https://en.wikipedia.org/wiki/Chemical_table_file
  /// I took bad notes - not sure where "wedge" and "hashed wedge" info came
  /// from.
  /// See also
  /// https://discover.3ds.com/sites/default/files/2020-08/biovia_ctfileformats_2020.pdf
  /// and in particular Chapter 6: V2000 Connection Table [CTAB], "Meaning of
  /// values in the bond block"

  /// Not stereo.  For double bonds, use atom coords to determine cis or trans
  bs_not_stereo = 0,

  /// stereo up (wedge)
  bs_up = 1,

  /// For double bonds: either cis or trans
  bs_cis_trans_double = 3,

  /// For single bonds: either cis or trans
  bs_either = 4,

  /// Down (hashed wedge)
  bs_down_double = 6
};

class Bond {
public:
  struct BondParams {
    const unsigned int a0, a1;
    const BondType bond_type = BondType::bt_single;
    const BondStereo stereo = BondStereo::bs_not_stereo;
    const std::string optional_cols;
  };

  Bond()
      : m_a0(0), m_a1(0), m_type(BondType::bt_single),
        m_stereo(BondStereo::bs_not_stereo), m_optional_cols("") {}

  Bond(const BondParams &&params)
      : m_a0(params.a0), m_a1(params.a1), m_type(params.bond_type),
        m_stereo(params.stereo), m_optional_cols(params.optional_cols) {}

  unsigned int a0() const { return m_a0; }
  unsigned int a1() const { return m_a1; }
  BondType type() const { return m_type; }
  BondStereo stereo() const { return m_stereo; }
  std::string optional_cols() const { return m_optional_cols; }

protected:
  unsigned int m_a0, m_a1;
  BondType m_type;
  BondStereo m_stereo;
  std::string m_optional_cols;
};

typedef std::vector<Bond> BondVector;
} // namespace mesaac::mol
