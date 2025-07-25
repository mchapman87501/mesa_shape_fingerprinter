import Foundation
import SwiftUI
import simd

enum ArcBallError: Error {
  case invalidRadius
}
/// ArcBall is for interpreting mouse movements as rotations
/// Its use of quaternions is inspired by https://raw.org/code/trackball-rotation-using-quaternions/
struct ArcBall {
  /// Multiplicative identity quaternion:
  private static let quatOne = simd_quatf(ix: 0.0, iy: 0.0, iz: 0.0, r: 1.0)
  /// Additive identity quaternion:
  private static let quatZero = simd_quatf(ix: 0.0, iy: 0.0, iz: 0.0, r: 0.0)

  private var minExtent = CGFloat(1)

  private var last = ArcBall.quatOne
  private var curr = ArcBall.quatOne

  mutating func reset(surfacePatch size: CGSize) {
    minExtent = min(size.width, size.height)
    last = Self.quatOne
    curr = Self.quatOne
  }

  /// Update the quaternion representing the rotation between two points on the arcball surface.
  /// This is for tracking rotation for an in-progress mouse move.
  /// - Parameters:
  ///   - from: starting point
  ///   - to: ending point
  /// - Returns: the axis-angle quaternion for the rotation
  mutating func move(from start: CGPoint, to end: CGPoint) {
    // Mental image: the user, in dragging from start to end, is unwinding
    // thread from a spool.  The direction of the "pull" is ortho to the spool's
    // axis, so the axis always lies in the x-y plane.  The length of the unwound
    // thread is proportional to the "spin" of the spool:
    // angle in rads = length / radius
    let ds = end - start

    let radius = minExtent / 4  // Chosen arbitrarily.
    let quatAngle = Float(ds.mag() / radius)

    let dsUnit = ds.unit()
    curr = simd_quatf(angle: quatAngle, axis: [Float(dsUnit.y), Float(dsUnit.x), 0])
  }

  /// Finalize the quaternion representing the rotation between two points on the arcball surface.
  /// This is for updating the ArcBall when the mouse move ends.
  /// - Parameters:
  ///   - from: starting point
  ///   - to: ending point
  /// - Returns: the axis-angle quaternion for the rotation
  mutating func completeMove(from start: CGPoint, to end: CGPoint) {
    last = simd_mul(curr, last)
    curr = Self.quatOne
  }

  /// Get self's current in-progress rotation quaternion.
  /// The result can act() upon a position vector to yield a rotated position vector.
  /// - Returns: the current rotation quaternion, *normalized*
  func currRotation() -> simd_quatf {
    simd_normalize(curr)  // simd_mul(curr, last))
  }
}
