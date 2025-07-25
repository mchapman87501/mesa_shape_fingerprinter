import mesaac

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
