import mesaac

/// An Atom
public struct Atom {
  public let atomicNum: UInt16
  // TODO obviate this by providing color from atomicNum:
  public let symbol: String
  public let isHydrogen: Bool

  // Mutable position allows, e.g., Swift-based alignment:
  public var pos: Position
  public let radius: Float

  public let optionalCols: String

  /// Convenience initializer
  public init(atom: mesaac.mol.Atom) {
    atomicNum = UInt16(atom.atomic_num())
    let mesaPos = atom.pos()
    pos = Position(x: Float(mesaPos.x()), y: Float(mesaPos.y()), z: Float(mesaPos.z()))
    radius = Float(atom.radius())
    symbol = String(atom.symbol())
    isHydrogen = atom.is_hydrogen()

    // Probably don't need this:
    optionalCols = String(atom.optional_cols())
  }
}
