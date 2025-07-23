import Foundation
import SwiftUI
import simd

enum ArcBallError: Error {
  case invalidRadius
}
/// ArcBall is for interpreting mouse movements as rotations, panz, zooms
/// on a 3D scene.
/// It's inspired by https://raw.org/code/trackball-rotation-using-quaternions/
struct ArcBall {
  /// Multiplicative identity quaternion:
  private static let quatOne = simd_quatf(ix: 0.0, iy: 0.0, iz: 0.0, r: 1.0)
  /// Additive identity quaternion:
  private static let quatZero = simd_quatf(ix: 0.0, iy: 0.0, iz: 0.0, r: 0.0)

  private var size = CGSize(width: 1, height: 1)
  private var minExtent = CGFloat(1)

  private var last = Self.quatOne
  private var curr = Self.quatOne

  mutating func reset(surfacePatch: CGSize) {
    size = surfacePatch
    minExtent = min(size.width, size.height)

    last = Self.quatOne
    curr = Self.quatOne
  }

  /// Get the quaternion representing the rotation between two points on the arcball surface.
  /// This is for providing updates for an in-progress mouse move.
  /// - Parameters:
  ///   - from: starting point
  ///   - to: ending point
  /// - Returns: the axis-angle quaternion for the rotation
  mutating func move(from start: CGPoint, to end: CGPoint) {
    let a = project(start)
    let b = project(end)
    curr = simd_quatf(from: b, to: a)
  }

  /// Get the quaternion representing the rotation between two points on the arcball surface.
  /// This is for updating the ArcBall when the mouse move ends.
  /// - Parameters:
  ///   - from: starting point
  ///   - to: ending point
  /// - Returns: the axis-angle quaternion for the rotation
  mutating func completeMove(from start: CGPoint, to end: CGPoint) {
    last = simd_mul(curr, last)
    curr = Self.quatOne
  }

  /// Project a 2D point onto the surface of the ArcBall.
  /// - Parameters:
  ///   - p: the point to project
  /// - Returns: the point p projected onto the ArcBall surface
  private func project(_ p: CGPoint) -> RKPosition {
    // Get normalized x and y.
    // With global mouse tracking, p can run beyond the bounds of self's size.
    // So just clip if normalized coords run outside -1...1.
    let xUnclipped = ((2 * p.x) - size.width - 1) / minExtent
    // Invert y to compensate for p's coordinate system.
    let yUnclipped = (size.height - (2 * p.y) - 1) / minExtent

    let x = min(1, max(-1, xUnclipped))
    let y = min(1, max(-1, yUnclipped))

    let rSqr = CGFloat(1)
    let d = x * x + y * y
    let isInBounds = (2 * d <= rSqr)
    let z = isInBounds ? sqrt(rSqr - d) : ((rSqr / 2) / sqrt(d))
    let result = RKPosition(Float(x), Float(y), Float(z)).unit()
    return result
  }

  /// Get self's current rotation quaternion.
  /// The result can act() upon a position vector to yield a rotated position vector.
  /// - Returns: the current rotation quaternion
  func currRotation() -> simd_quatf {
    simd_mul(curr, last)
  }
}
