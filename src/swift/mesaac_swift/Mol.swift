import mesaac

/// A molecule
public struct Mol: Identifiable {
  /// SwiftUI likes Identifiable objects
  public let id: String

  public let name: String
  public let metadata: String
  public let comments: String
  // Won't need this unless implementing SD file output
  public let countsLine: String
  public let propertiesBlock: String

  public let tags: [String: String]

  public let dimensionality: UInt8

  public private(set) var atoms: [Atom]
  public let bonds: [Bond]

  public var heavyAtoms: [Atom] {
    atoms.filter { !$0.isHydrogen }
  }

  /// Convenience initializer
  public init(mol: mesaac.mol.Mol, id: String) {
    self.id = id
    name = String(mol.name())
    // These are not needed for depiction
    metadata = String(mol.metadata())
    comments = String(mol.comments())
    countsLine = String(mol.counts_line())
    propertiesBlock = String(mol.properties_block())
    dimensionality = UInt8(mol.dimensionality())

    atoms = (0..<mol.num_atoms()).map { atomIndex in
      var mmAtom = mesaac.mol.Atom()
      mol.get_atom(atomIndex, &mmAtom)
      return Atom(atom: mmAtom)
    }

    bonds = (0..<mol.num_bonds()).map { bondIndex in
      var mmBond = mesaac.mol.Bond()
      mol.get_bond(bondIndex, &mmBond)
      return Bond(bond: mmBond)
    }

    // Defer copying tags -- not really needed for a mol viewer...
    tags = [:]
  }

  public mutating func meanCenter() {
    let numAtoms = Float(atoms.count)
    if numAtoms <= 0 { return }

    let positions = atoms.map { $0.pos }
    let xVals = positions.map { $0.x }
    let xMid = xVals.reduce(0.0, +) / numAtoms

    let yVals = positions.map { $0.y }
    let yMid = yVals.reduce(0.0, +) / numAtoms

    let zVals = positions.map { $0.z }
    let zMid = zVals.reduce(0.0, +) / numAtoms

    let offset = Position(x: xMid, y: yMid, z: zMid)
    for i in 0..<atoms.count {
      let pos = atoms[i].pos
      atoms[i].pos = Position(
        x: pos.x - offset.x, y: pos.y - offset.y, z: pos.z - offset.z)
    }
  }
}
