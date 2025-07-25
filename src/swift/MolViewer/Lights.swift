import RealityKit

struct Lights {
  private static func lightSource(_ pos: RKPosition) -> Entity {
    let result = PointLight()
    result.light.color = .white
    result.light.intensity = 0.4
    result.position = pos
    return result
  }

  private static func highlightSource() -> Entity {
    lightSource(RKPosition(0, 10, 10))
  }

  private static func backlightSource() -> Entity {
    lightSource(RKPosition(0, 10, -40))
  }

  private static func ambientLight() -> Entity {
    // I don't know how to do the equivalent of SCNLight with type .ambient,
    // in RealityKit.
    let result = Entity()
    // let light = SCNLight()
    // light.type = .ambient
    // light.color = CGColor(gray: 0.6, alpha: 1.0)
    // let result = Entity()
    // let lightComponent =
    //   result.light = light
    return result
  }

  let entity: Entity

  init() {
    let result = Entity()
    result.addChild(Self.highlightSource())
    result.addChild(Self.backlightSource())
    // result.addChild(Self.ambientLight())
    result.name = "My Lights"
    entity = result
  }
}
