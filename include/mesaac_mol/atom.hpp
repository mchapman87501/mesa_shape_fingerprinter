//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>
#include <vector>

#include "position.hpp"

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
    /// @brief Any optional columns, as read from (for example) an SD file
    const std::string optional_cols;
  };

  /// @brief Construct an 'null' atom with atomic number 0.
  Atom() {}

  /// @brief Construct an atom.
  /// @param params properties of the atom - atomic weight, 3-space coordinates,
  /// etc.
  Atom(const AtomParams &&params)
      : m_atomic_num(params.atomic_num), m_pos(params.pos),
        m_optional_cols(params.optional_cols) {}

  //   /// @brief Construct an atom positioned at the 3-space origin.
  //   /// @param atomic_num The atomic number of the atom
  //   Atom(unsigned int atomic_num) : m_atomic_num(atomic_num) {}

  //   /// @brief Construct an atom, with coordinates.
  //   /// @param atomic_num The atomic number of the atom
  //   /// @param pos The location of the atom in 3-space
  //   Atom(unsigned int atomic_num, const Position &pos)
  //       : Atom(atomic_num, pos, "") {}

  //   /// @brief Construct an atom, with coordinates and with optional columns
  //   as
  //   /// read from an SD file.
  //   /// @param atomic_num The atomic number of the atom
  //   /// @param pos The location of the atom in 3-space
  //   /// @param optional_cols Any optional (SD file) columns associated w. the
  //   atom Atom(unsigned int atomic_num, const Position &pos,
  //        const std::string &optional_cols)
  //       : m_atomic_num(atomic_num), m_pos(pos),
  //       m_optional_cols(optional_cols) {}

  void set_pos(const Position &new_value) { m_pos = new_value; }

  unsigned int atomic_num() const { return m_atomic_num; };

  float x() const { return m_pos.x(); }
  float y() const { return m_pos.y(); }
  float z() const { return m_pos.z(); }
  std::string optional_cols() const { return m_optional_cols; }

  std::string symbol() const;
  float radius() const;
  bool is_hydrogen() const;

private:
  unsigned int m_atomic_num;
  Position m_pos;
  std::string m_optional_cols;
};

typedef std::vector<Atom> AtomVector;
} // namespace mesaac::mol
