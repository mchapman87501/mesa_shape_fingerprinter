import CxxStdlib
import Foundation
import mesaac

/// This module provides some shims that reduce the repeated copying of C++ objects.

public typealias BondType = mesaac.mol.BondType
public typealias StereoChemistry = mesaac.mol.BondStereo

/// A 3-space position
public struct Position {
  public let x: Float
  public let y: Float
  public let z: Float
}

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

/// A molecular Bond
public struct Bond {
  /// Zero-based index of the first atom, in this Bond's mol, "bonded"
  /// by self.
  public let a0Index: Int
  /// Zero-based index of the second atom
  public let a1Index: Int
  public let bondType: BondType
  public let stereoChem: StereoChemistry
  public let optionalCols: String

  /// Convenience initializer
  public init(bond: mesaac.mol.Bond) {
    a0Index = Int(bond.a0()) - 1  // mesaac.mol.Bond indices are 1-based
    a1Index = Int(bond.a1()) - 1
    bondType = bond.type()
    stereoChem = bond.stereo()
    optionalCols = String(bond.optional_cols())
  }
}

public struct BoundingVolume {
  public let xMin: Float
  public let yMin: Float
  public let zMin: Float
  public let xMax: Float
  public let yMax: Float
  public let zMax: Float
}

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

  public func boundingVolume() -> BoundingVolume {
    if atoms.isEmpty {
      return BoundingVolume(xMin: 0.0, yMin: 0.0, zMin: 0.0, xMax: 0.0, yMax: 0.0, zMax: 0.0)
    }

    // This ignores atom radii
    let positions = atoms.map {
      $0.pos
    }
    let xVals = positions.map { $0.x }
    let yVals = positions.map { $0.y }
    let zVals = positions.map { $0.z }
    return BoundingVolume(
      xMin: xVals.min() ?? 0, yMin: yVals.min() ?? 0, zMin: zVals.min() ?? 0,
      xMax: xVals.max() ?? 0, yMax: yVals.max() ?? 0,
      zMax: zVals.max() ?? 0)
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

/// Reads Mols from V2000-format SD files
public struct V2000SDReader {

  /// Read the molecules from an SD file.
  /// - Parameter pathURL: file:// URL for the SD file to read
  /// - Returns: molecules read from pathURL
  public static func readMols(pathURL: URL) -> [Mol] {
    var result = [Mol]()
    let pathname = std.string(pathURL.path(percentEncoded: false))
    let t0 = Date()
    var reader = mesaac.mol.PathSDReader(pathname)
    var mesaMol = mesaac.mol.Mol()
    var molIndex = 0
    while reader.read(&mesaMol) {
      molIndex += 1
      let swiftMol = Mol(mol: mesaMol, id: "\(pathURL)-\(molIndex)")
      result.append(swiftMol)
    }
    let dt = t0.distance(to: Date())
    print("Time to read \(pathname): \(dt)")
    return result
  }

  /// Read molecules from Data.
  /// - Parameters:
  ///   - regularFileContents: a Data object holding the contents of an SD file
  ///   - filename: name of the file containing regularFileContents
  /// - Throws: if regularFileContents can't be decoded as UTF8 text
  /// - Returns: molecules read from regularFileContents
  public static func readMols(regularFileContents: Data?, filename: String) throws -> [Mol] {
    var result = [Mol]()
    guard let regularFileContents else {
      // TODO find a more appropriate exception
      throw CocoaError(.fileReadCorruptFile)
    }
    guard let contentStr = String(data: regularFileContents, encoding: .utf8) else {
      throw CocoaError(.fileReadCorruptFile)
    }
    var reader = mesaac.mol.PathSDReader(std.string(contentStr), std.string(filename))

    // TODO DRY
    var mesaMol = mesaac.mol.Mol()
    var molIndex = 0
    while reader.read(&mesaMol) {
      molIndex += 1
      let swiftMol = Mol(mol: mesaMol, id: "\(filename)-\(molIndex)")
      result.append(swiftMol)
    }
    return result
  }

}
