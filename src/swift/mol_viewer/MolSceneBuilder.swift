import MesaacSwift
import RealityKit
import SwiftUI  // for Color
import simd

extension RKPosition {
  init(_ pos: MesaacSwift.Position) {
    self.init(x: pos.x, y: pos.y, z: pos.z)
  }
}

struct AtomColors {
  private static let colors: [String: Color] = [
    "C": Color(red: 0.5, green: 0.5, blue: 0.5),
    "O": Color(red: 1, green: 0, blue: 0),
    "H": Color(red: 1, green: 1, blue: 1),
    "N": Color(red: 0.56, green: 0.56, blue: 1.0),
    "S": Color(red: 1.0, green: 0.78, blue: 0.2),
    "P": Color(red: 1, green: 0.65, blue: 0),
    "Cl": Color(red: 0, green: 1.0, blue: 0),
    "Br": Color(red: 0.65, green: 0.16, blue: 0.16),
    "Zn": Color(red: 0.65, green: 0.16, blue: 0.16),
    "Na": Color(red: 0, green: 0, blue: 1),
    "Fe": Color(red: 1, green: 0.65, blue: 0),
    "Mg": Color(red: 0.13, green: 0.55, blue: 0.13),
    "Ca": Color(red: 0.5, green: 0.5, blue: 0.56),
  ]
  private static let unknownColor = Color(red: 1.0, green: 0.08, blue: 0.58)

  static func color(atom: Atom) -> Color {
    let result = colors[atom.symbol] ?? unknownColor
    return result
  }
}

struct MolSceneBuilder {
  let mol: Mol

  private static let bondRadius = Float(0.05)
  private static let bondMaterial = SimpleMaterial(color: NSColor.gray, isMetallic: false)

  private func bondQuat(from: RKPosition, to: RKPosition) -> simd_quatf {
    // This is based on Accelerate->simd documentation.

    let ds: RKPosition = to - from

    // Get the unit SIMD vector in the direction from ... to.
    let udsSIMD = SIMD3<Float>(ds.unit())

    // An SCNCylinder is oriented along the y axis.  Rotate it
    // into uds.
    let uCylinderSIMD = SIMD3<Float>(0, 1, 0)

    return simd_quatf(from: uCylinderSIMD, to: udsSIMD)
  }

  private func bondEntity(from: RKPosition, to: RKPosition) -> Entity {
    // This is based on Accelerate->simd documentation.
    let ds = to - from
    let rot = bondQuat(from: from, to: to)

    let result = ModelEntity(
      mesh: .generateCylinder(height: Float(ds.mag()), radius: Self.bondRadius),
      materials: [Self.bondMaterial])
    result.orientation = rot
    // The cylinder is centered on the origin.  Hence the ds/2.0.
    result.position = from + (ds / 2.0)
    return result
  }

  private func doubleBondEntity(from: RKPosition, to: RKPosition) -> Entity {
    // Create two y-axis bonds, each offset from the line from..to , along x.
    // Add them to a parent node.
    // Orient the parent node according to bondQuat.
    // "Scale" the bonds individually, vs. scaling the
    // parent node, so the bond radii don't scale unnaturally.

    let ds = to - from
    let dsMag = ds.mag()

    let cyl1 = ModelEntity(
      mesh: .generateCylinder(height: dsMag, radius: Self.bondRadius),
      materials: [Self.bondMaterial])
    cyl1.position = RKPosition(x: -1.5 * Self.bondRadius, y: dsMag / 2.0, z: 0)

    let cyl2 = ModelEntity(
      mesh: .generateCylinder(height: dsMag, radius: Self.bondRadius),
      materials: [Self.bondMaterial])
    cyl2.position = RKPosition(x: 1.5 * Self.bondRadius, y: dsMag / 2.0, z: 0)

    let rot = bondQuat(from: from, to: to)

    let result = Entity()
    result.addChild(cyl1)
    result.addChild(cyl2)
    result.orientation = rot
    result.position = from
    return result
  }

  private func atomEntity(radius: Float, position: RKPosition, color: Color) -> Entity {
    let material = SimpleMaterial(color: NSColor(color), isMetallic: false)
    let result = ModelEntity(mesh: .generateSphere(radius: radius), materials: [material])
    result.position = position
    return result
  }

  private func addAtomEntity(root: Entity, atom: Atom) {
    let position = atom.pos
    let posVec = RKPosition(position.x, position.y, position.z)
    let color = AtomColors.color(atom: atom)
    let atomEntity = atomEntity(
      radius: atom.radius / 5.0, position: posVec, color: color)
    root.addChild(atomEntity)
  }

  private func addAllAtoms(root: Entity) {
    for atom in mol.atoms {
      if !atom.isHydrogen {
        addAtomEntity(root: root, atom: atom)
      }
    }
  }

  private func addBondEntity(root: Entity, bond: Bond) {
    let atom0 = mol.atoms[bond.a0Index]
    guard !atom0.isHydrogen else { return }

    let atom1 = mol.atoms[bond.a1Index]
    guard !atom1.isHydrogen else { return }

    let end0V = RKPosition(atom0.pos)
    let end1V = RKPosition(atom1.pos)

    let bondType = bond.bondType
    // TODO: support triple bonds; decide how to handle ambiguous bond strength.
    let bondEntity =
      (bondType == BondType.bt_double)
      ? bondEntity(from: end0V, to: end1V) : doubleBondEntity(from: end0V, to: end1V)
    root.addChild(bondEntity)
  }

  private func addAllBonds(root: Entity) {
    for bond in mol.bonds {
      addBondEntity(root: root, bond: bond)
    }
  }

  /// Build a SceneKit node depicting self's mol.
  func molEntity() -> Entity {
    let result = Entity()
    addAllAtoms(root: result)
    addAllBonds(root: result)
    return result
  }
}
