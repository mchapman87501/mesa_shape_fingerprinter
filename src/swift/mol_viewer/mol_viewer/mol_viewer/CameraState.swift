import CoreGraphics
import RealityKit
import simd

struct CameraState {
  private var targetPos = RKPosition()
  private var camPos = RKPosition()
  private var patchSize = CGSize(width: 1.0, height: 1.0)
  private var minExtent = CGFloat(1.0)
  private var distance = Float(1.0)
  private var zoomFactor = Float(1.0)
  private var rotator = ArcBall()

  mutating func reset(
    targetPos: RKPosition, distance: CGFloat, patchSize: CGSize
  ) {
    self.targetPos = targetPos
    self.distance = Float(distance)
    self.patchSize = patchSize
    zoomFactor = Float(1.0)

    minExtent = min(patchSize.width, patchSize.height)

    rotator.reset(surfacePatch: patchSize)
    camPos = homePosition()
  }

  private func homePosition() -> RKPosition {
    RKPosition(0, 0, distance / zoomFactor)
  }

  private mutating func updateCamPos() {
    camPos = targetPos + rotator.currRotation().act(homePosition())
  }
  mutating func drag(from start: CGPoint, to end: CGPoint) {
    rotator.move(from: start, to: end)
    updateCamPos()
  }

  mutating func completeDrag(from start: CGPoint, to end: CGPoint) {
    rotator.completeMove(from: start, to: end)
    updateCamPos()
  }

  private func calcZoomFactor(from start: CGPoint, to end: CGPoint) -> Float {
    let delta = (end.y - start.y)
    let zoomChange = 1.0 + delta / minExtent
    return Float(max(0.1, zoomChange))
  }

  mutating func zoomDrag(from start: CGPoint, to end: CGPoint) {
    zoomFactor = calcZoomFactor(from: start, to: end)
    updateCamPos()
  }

  mutating func completeZoomDrag(from start: CGPoint, to end: CGPoint) {
    distance = max(0.01, distance / calcZoomFactor(from: start, to: end))
    zoomFactor = 1.0
    updateCamPos()
  }

  func reposition(camEntity: Entity) {
    camEntity.look(at: targetPos, from: camPos, relativeTo: nil)
  }
}
