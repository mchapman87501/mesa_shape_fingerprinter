//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>
#include <vector>

namespace mesaac {
namespace mol {
class Atom {
public:
  Atom() : m_atomic_num(0), m_x(0), m_y(0), m_z(0) {}
  Atom(unsigned int atomic_num, float x, float y, float z)
      : m_atomic_num(atomic_num), m_x(x), m_y(y), m_z(z) {}

  // XXX FIX THIS:  Immutable properties should be set in constructor.
  // Setters like x(newvalue) are OK for mutable properties.
  void atomic_num(unsigned int new_value);

  void x(float new_value);
  void y(float new_value);
  void z(float new_value);

  // SD-specific HACK: Save the tail of the line from which this was read.
  void optional_cols(const std::string &tail);

  unsigned int atomic_num() const { return m_atomic_num; };
  float x() const { return m_x; }
  float y() const { return m_y; }
  float z() const { return m_z; }
  std::string optional_cols() const { return m_optional_cols; }

  std::string symbol() const;
  float radius() const;
  bool is_hydrogen() const;

protected:
  unsigned int m_atomic_num;
  float m_x, m_y, m_z;
  std::string m_optional_cols;
};

typedef std::vector<Atom> AtomVector;
} // namespace mol
} // namespace mesaac
