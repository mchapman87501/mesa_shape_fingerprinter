import CxxStdlib
import Foundation
// The wrapper module is "MesaacSwift".  The wrapped C++ module is "mesaac".
import mesaac

extension String {
  func strip() -> String {
    trimmingCharacters(in: .whitespacesAndNewlines)
  }
}

struct MolGeometry {
  let mol: mesaac.mol.Mol

  func printOverview() {
    let metadata = String(mol.metadata()).strip()
    let comments = String(mol.comments()).strip()
    print(
      "\(mol.name()) - \(metadata) - \(comments) - \(mol.dimensionality())-D")

  }

  private func atomPos(_ index: UInt32) -> mesaac.mol.Position {
    var atom = mesaac.mol.Atom()
    var result = mesaac.mol.Position()
    mol.get_atom(index, &atom)
    atom.get_pos(&result)
    return result
  }

  private func posStr(_ pos: mesaac.mol.Position) -> String {
    "(\(pos.x()), \(pos.y()), \(pos.z()))"
  }

  typealias BondType = mesaac.mol.BondType

  private func bondTypeStr(_ bondType: BondType) -> String {
    switch bondType {
    case BondType.bt_single:
      "Single"
    case BondType.bt_double:
      "Double"
    case BondType.bt_triple:
      "Triple"
    case BondType.bt_aromatic:
      "Aromatic"
    case BondType.bt_single_or_double:
      "Single or Double"
    case BondType.bt_single_or_aromatic:
      "Single or Aromatic"
    case BondType.bt_double_or_aromatic:
      "Double or Aromatic"
    case BondType.bt_any:
      "Any"
    @unknown default:
      "**Unknown Bond Type**"
    }
  }

  typealias BondStereo = mesaac.mol.BondStereo

  private func bondStereoStr(_ bondStereo: BondStereo) -> String {
    switch bondStereo {
    case BondStereo.bs_not_stereo:
      "Not Stereo"
    case BondStereo.bs_up:
      "Stereo Up"
    case BondStereo.bs_cis_trans_double:
      "Either Cis or Trans (double)"
    case BondStereo.bs_either:
      "Either"
    case BondStereo.bs_down_double:
      "Stereo Down"
    @unknown default:
      "**Unknown Stereochemistry**"
    }
  }

  func printAtomGeom() {
    print("  Atoms:")
    for i in 0..<mol.num_atoms() {
      var atom = mesaac.mol.Atom()
      mol.get_atom(i, &atom)
      if !atom.is_hydrogen() {
        var position = mesaac.mol.Position()
        atom.get_pos(&position)

        print(
          "    \(atom.symbol()), radius=\(atom.radius()), pos=(\(posStr(position)))"
        )
      }
    }
    print()
  }

  func printBondGeom() {
    print("  Bonds:")
    for i in 0..<mol.num_bonds() {
      var bond = mesaac.mol.Bond()
      mol.get_bond(i, &bond)

      let numAtoms = mol.num_atoms()

      // From the CTfile spec for V3000 CT file format, from 2010:
      // atom indexes must be > 0.  So I guess they are one-based.
      let indexA0 = bond.a0() - 1
      let indexA1 = bond.a1() - 1
      let outOfBounds = [indexA0, indexA1].filter {
        $0 < 0 || $0 >= numAtoms
      }
      for index in outOfBounds {
        print("***Bond atom index \(index) is out of range 0..<\(numAtoms)")
      }
      if outOfBounds.isEmpty {
        let end1 = posStr(atomPos(indexA0))
        let end2 = posStr(atomPos(indexA1))

        let bondTypeStr = bondTypeStr(bond.type())
        let stereoStr = bondStereoStr(bond.stereo())  // Not needed for 3D rendering
        print("    \(end1) .. \(end2), \(bondTypeStr) bond, \(stereoStr)")
      }
    }
    print("")
  }
}

func demoBinASCII() {
  print("Demo BinASCII encoding")
  let b32 = mesaac.common.B32()

  let src = "Hello mol_demo"
  let encoded = b32.encode(std.string(src))
  print("Src: '\(src)' Encoded: '\(encoded)'")

  print("B64-encoded: '\(mesaac.common.B64().encode(std.string(src)))'")
  print()
}

func demoMeasures() {
  print("Demo measures")
  let tani = mesaac.measures.Tanimoto()

  // Trying to construct a boost dynamic bitset in Swift, from a size and a value,
  // results in compilation errors.  Does construction from a string work any better?
  let bitset1 = mesaac.shape_defs.bit_vector_from_str("01010101")
  let bitset2 = mesaac.shape_defs.bit_vector_from_str("10101011")
  print("Tanimoto similarity: \(tani.similarity(bitset1, bitset2))")
  print()
}

func demoAtomConstruction() {
  print("Demo atom construction")
  let pos = mesaac.mol.Position()
  let atomParams = mesaac.mol.Atom.AtomParams(atomic_num: 6, pos: pos, optional_cols: "")
  let atom = mesaac.mol.Atom(consuming: atomParams)
  print("Atom: \(atom.symbol())")
  print()
}

func demoSDFileReading() {
  print("Demo Reading SD files")
  let pathname = std.string("tests/data/lib/mesaac_mol/in/cox2_3d.sd")
  var reader = mesaac.mol.PathSDReader(pathname)
  var mol = mesaac.mol.Mol()
  while reader.read(&mol) {
    let geom = MolGeometry(mol: mol)
    geom.printOverview()
    geom.printAtomGeom()
    geom.printBondGeom()
  }
}

// demoBinASCII()
// demoMeasures()
// demoAtomConstruction()
demoSDFileReading()
