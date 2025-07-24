import RealityKit

struct Camera {
  let camEntity: Entity
  let component: PerspectiveCameraComponent

  init() {
    let camEntity = Entity()
    camEntity.name = "My Camera"
    camEntity.look(at: .zero, from: [0, 0, 50], relativeTo: nil)

    var component = PerspectiveCameraComponent()
    component.fieldOfViewOrientation = .vertical
    camEntity.components.set(component)

    self.camEntity = camEntity
    self.component = component
  }

  mutating func reposition(toSee entity: Entity) {
    let bounds = entity.visualBounds(relativeTo: nil)
    guard !bounds.isEmpty else { return }

    let targetPos = bounds.center
    let boundingRadius = bounds.boundingRadius

    let fovDegrees = component.fieldOfViewInDegrees
    let fovRads = (fovDegrees / 180.0) * .pi

    let distance = boundingRadius / tan(fovRads / 2)

    let cameraPosition = targetPos + SIMD3<Float>(0, 0, distance)

    camEntity.look(at: targetPos, from: cameraPosition, relativeTo: nil)
  }
}
