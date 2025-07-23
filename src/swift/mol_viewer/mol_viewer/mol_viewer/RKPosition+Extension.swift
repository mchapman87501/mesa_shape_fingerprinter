import Foundation

// Provide functions for vector transformations, e.g., rotation of one vector onto another.
// I should probably be doing all of this with quaternions.
// This is based on code from mesaac/lib/python/view_dendrogram.

extension RKPosition {
  static func + (lhs: Self, rhs: Self) -> Self {
    Self(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z)
  }

  static func - (lhs: Self, rhs: Self) -> Self {
    Self(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z)
  }

  static func * (lhs: Self, scalar: Float) -> Self {
    Self(lhs.x * scalar, lhs.y * scalar, lhs.z * scalar)
  }

  static func / (lhs: Self, amount: Float) -> Self {
    lhs * (1.0 / amount)
  }

  func mag() -> Float {
    let x2 = x * x
    let y2 = y * y
    let z2 = z * z
    return sqrt(x2 + y2 + z2)
  }

  func unit() -> Self {
    self / mag()
  }

  /// Get the dot product of self with other.
  func dot(_ other: RKPosition) -> Float {
    return x * other.x + y * other.y + z * other.z
  }

  /// Get the angle between self and other.
  func angle(_ other: RKPosition) -> Float {
    let u = unit()
    return acos(u.dot(other) / other.mag())
  }

  /// Get the angle between self and the z axis.
  func zAngle() -> Float {
    return angle(Self.unitZ())
  }

  /// Get the cross product of self and other.
  func cross(_ other: Self) -> Self {
    Self(
      y * other.z - z * other.y,
      z * other.x - x * other.z,
      x * other.y - y * other.x
    )
  }

  /// Get the unit z vector
  static func unitZ() -> Self {
    Self(0, 0, 1)
  }

  /// Get the normal to the plane defined by self and the unit z vector.
  func vzNormal() -> Self {
    cross(Self.unitZ())
  }
}
