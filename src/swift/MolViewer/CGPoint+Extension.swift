import CoreGraphics

// Provide functions for vector transformations, e.g., rotation of one vector onto another.
// I should probably be doing all of this with quaternions.
// This is based on code from mesaac/lib/python/view_dendrogram.

extension CGPoint {
  static func + (lhs: Self, rhs: Self) -> Self {
    Self(x: lhs.x + rhs.x, y: lhs.y + rhs.y)
  }

  static func - (lhs: Self, rhs: Self) -> Self {
    Self(x: lhs.x - rhs.x, y: lhs.y - rhs.y)
  }

  static func * (lhs: Self, scalar: CGFloat) -> Self {
    Self(x: lhs.x * scalar, y: lhs.y * scalar)
  }

  static func / (lhs: Self, amount: CGFloat) -> Self {
    lhs * (1.0 / amount)
  }

  func magSqr() -> CGFloat {
    let x2 = x * x
    let y2 = y * y
    return x2 + y2
  }

  func mag() -> CGFloat {
    sqrt(magSqr())
  }

  func unit() -> Self {
    self / mag()
  }

  /// Get the dot product of self with other.
  func dot(_ other: CGPoint) -> CGFloat {
    return x * other.x + y * other.y
  }

  /// Get the angle between self and other.
  func angle(_ other: CGPoint) -> CGFloat {
    let u = unit()
    return acos(u.dot(other) / other.mag())
  }
}
