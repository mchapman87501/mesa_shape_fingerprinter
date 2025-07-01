#pragma once

namespace mesaac::mol {
class Position {
public:
  Position() : m_x(0.0f), m_y(0.0f), m_z(0.0f) {}
  Position(float x, float y, float z) : m_x(x), m_y(y), m_z(z) {}

  float x() const { return m_x; }
  float y() const { return m_y; }
  float z() const { return m_z; }

private:
  float m_x;
  float m_y;
  float m_z;
};
} // namespace mesaac::mol