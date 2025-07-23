import RealityKit

struct Camera {
  let camEntity: Entity
  let component: PerspectiveCameraComponent

  init() {
    let camEntity = Entity()
    camEntity.look(at: .zero, from: [0, 0, 50], relativeTo: nil)
    var component = PerspectiveCameraComponent()
    component.fieldOfViewOrientation = .vertical
    self.component = component
    camEntity.components.set(component)
    self.camEntity = camEntity
  }
}
