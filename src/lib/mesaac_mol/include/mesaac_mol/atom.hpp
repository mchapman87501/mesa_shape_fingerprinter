//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>
#include <vector>

#include "mesaac_mol/atom_props.hpp"
#include "mesaac_mol/position.hpp"

namespace mesaac::mol {

/// @brief Represents an atom as read from an SD file.
class Atom {
public:
  /// @brief AtomParams defines parameters used to construct an Atom.
  struct AtomParams {
    /// @brief The atomic number of the Atom
    const unsigned int atomic_num;
    /// @brief Position in 3-space
    const Position pos;
    /// @brief Optional properties, as read from (for example) an SD file
    const AtomProps props;
    /// @brief Any optional columns, as read from (for example) an SD file
    const std::string optional_cols;
  };

  /// @brief Construct a "null" Atom with atomic number 0.
  Atom() {}

  /// @brief Construct an Atom.
  /// @param params properties of the atom - atomic number, 3-space coordinates,
  /// etc.
  Atom(const AtomParams &&params)
      : m_atomic_num(params.atomic_num), m_pos(params.pos),
        m_props(params.props), m_optional_cols(params.optional_cols) {}

  /// @brief Change the position of an Atom.
  /// @param new_value the new position
  void set_pos(const Position &new_value) { m_pos = new_value; }

  /// @brief Get the atomic number of an Atom.
  /// @return the Atom's atomic number
  unsigned int atomic_num() const { return m_atomic_num; };

  /// @brief Get the position of an Atom.
  /// @return the Atom's position
  Position pos() const { return m_pos; }

  /**
   * @brief Copy the position of an Atom.
   * @details This is a sop to C++-Swift language interoperability.
   * @param result On return, holds the position of this Atom.
   */
  void get_pos(Position &result) const { result = m_pos; }

  const AtomProps &props() const { return m_props; }

  AtomProps &mutable_props() { return m_props; }

  /// @brief Get any optional columns of an Atom, as read from an SD file.
  /// @return The option columns
  std::string optional_cols() const { return m_optional_cols; }

  /// @brief Get the atomic symbol of an Atom.
  /// @return the atomic symbol
  std::string symbol() const;

  /// @brief Get the radius of an Atom, in Ångstroms.
  /// @return the radius of the Atom
  float radius() const;

  /// @brief Find out whether an Atom is a hydrogen.
  /// @return whether the Atom is a hydrogen atom
  bool is_hydrogen() const;

private:
  unsigned int m_atomic_num;
  Position m_pos;
  AtomProps m_props;
  std::string m_optional_cols;
};

typedef std::vector<Atom> AtomVector;
} // namespace mesaac::mol
