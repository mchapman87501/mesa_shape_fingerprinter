//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>
#include <vector>

#include "position.hpp"

namespace mesaac::mol {
class Atom {
public:
  Atom() {}
  Atom(unsigned int atomic_num) : m_atomic_num(atomic_num) {}
  Atom(unsigned int atomic_num, const Position &pos)
      : Atom(atomic_num, pos, "") {}
  Atom(unsigned int atomic_num, const Position &pos,
       const std::string &optional_cols)
      : m_atomic_num(atomic_num), m_pos(pos), m_optional_cols(optional_cols) {}

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
