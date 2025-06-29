#pragma once

namespace mesaac::mol {
class Position {
public:
  Position() : Position(0.0, 0.0, 0.0) {}
  Position(double x, double y, double z) : m_x(x), m_y(y), m_z(z) {}
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