// Copyright © 2025 Mitch Chapman.  All rights reserved.

import RealityKit
import SwiftUI
import simd

struct ViewTransform {
  private var managedEntity: Entity? = nil
  // Track whether the managedEntity is being moved.
  private var isMoving = false

  // Transient transform parameters are used during user interaction.
  // They get saved to the "persistent" transform parameters when interaction is completed.
  // Analogy: a chess move doesn't count until the player releases the
  // game piece.

  // Managed entity's "persistent" transform:
  private var transform = Transform.identity

  // The translation that centers the managed entity on the world origin:
  private var centeredTranslation = RKPosition(0, 0, 0)

  // Store the constituents of the current transformation separately,
  // to ease separate rotation / translation interactions.
  private var currTranslation = RKPosition(0, 0, 0)
  private var rotator = ArcBall()

  mutating func configure(managing: Entity, controlWindowSize: CGSize) {
    managedEntity = managing
    transform = managing.transform

    centeredTranslation = {
      if let managedEntity {
        let bounds = managedEntity.visualBounds(relativeTo: nil)
        if !bounds.isEmpty {
          return bounds.center
        }
      }
      return RKPosition(0, 0, 0)
    }()

    currTranslation = RKPosition(0, 0, 0)
    rotator.reset(surfacePatch: controlWindowSize)
    isMoving = false
  }

  mutating func moveIsInProgress() {
    if !isMoving {
      if let managedEntity {
        transform = managedEntity.transform
        currTranslation = RKPosition(0, 0, 0)
      } else {
        transform = Transform.identity
        currTranslation = RKPosition(0, 0, 0)
      }
      isMoving = true
    }
  }

  private func deltaMatrix() -> float4x4 {
    let mRot = Transform(rotation: simd_normalize(rotator.currRotation())).matrix
    let mMoveBack = Transform(translation: centeredTranslation).matrix
    let mMoveToCenter = Transform(translation: -centeredTranslation).matrix

    let mCenterRotate = mMoveBack * mRot * mMoveToCenter
    let mCurrTranslate = Transform(translation: currTranslation).matrix

    return mCenterRotate * mCurrTranslate
  }

  mutating func updateEntityTransform() {
    let deltaMatrix = deltaMatrix()
    managedEntity?.transform = Transform(matrix: deltaMatrix * transform.matrix)
  }

  mutating func completeMovement() {
    if isMoving {
      transform = managedEntity?.transform ?? Transform(matrix: deltaMatrix() * transform.matrix)
      centeredTranslation += currTranslation
      currTranslation = RKPosition(0, 0, 0)
      isMoving = false
    }
  }

  mutating func rotateDrag(from start: CGPoint, to end: CGPoint) {
    moveIsInProgress()
    rotator.move(from: start, to: end)
    updateEntityTransform()
  }

  mutating func completeRotateDrag(from start: CGPoint, to end: CGPoint) {
    rotateDrag(from: start, to: end)
    rotator.completeMove(from: start, to: end)
    completeMovement()
  }

  static let panZoomFactor = Float(50.0)

  private func getZOffset(from start: CGPoint, to end: CGPoint) -> Float {
    Float(end.y - start.y) / Self.panZoomFactor
  }

  mutating func zoomDrag(from start: CGPoint, to end: CGPoint) {
    moveIsInProgress()
    currTranslation = RKPosition(0, 0, getZOffset(from: start, to: end))
    updateEntityTransform()
  }

  mutating func completeZoomDrag(from start: CGPoint, to end: CGPoint) {
    zoomDrag(from: start, to: end)
    completeMovement()
  }

  private func getPanOffset(from start: CGPoint, to end: CGPoint) -> RKPosition {
    let ds = end - start
    return RKPosition(Float(ds.x), Float(-ds.y), 0.0) / Self.panZoomFactor
  }

  mutating func panDrag(from start: CGPoint, to end: CGPoint) {
    moveIsInProgress()
    currTranslation = getPanOffset(from: start, to: end)
    updateEntityTransform()
  }

  mutating func completePanDrag(from start: CGPoint, to end: CGPoint) {
    panDrag(from: start, to: end)
    completeMovement()
  }
}
