import MesaacSwift
import SwiftUI

// Via ChatGPT:
@Observable
class MolSelectionMgr {
  typealias IDType = Mol.ID

  public private(set) var mols = [Mol]()
  public private(set) var selected: Set<IDType> = []
  private var lastSelected: IDType?
  private var prevSelectionCount = 0

  func setMols(_ newValue: [Mol]) {
    mols = newValue
    selected = []
  }

  func contains(_ mol: Mol) -> Bool {
    selected.contains(mol.id)
  }

  func selection() -> [Mol] {
    mols.filter { selected.contains($0.id) }
  }

  /// Find out whether the last change in selection was from
  /// nothing selected to something selected.
  func justChangedFromNoneToSome() -> Bool {
    return selected.count > 0 && prevSelectionCount == 0
  }

  func cmdSelect(_ mol: Mol) {
    prevSelectionCount = selected.count
    let molID = mol.id
    if selected.contains(molID) {
      selected.remove(molID)
    } else {
      selected.insert(molID)
      lastSelected = molID
    }
  }

  func shiftSelect(_ mol: Mol) {
    prevSelectionCount = selected.count
    let molID = mol.id
    if let last = lastSelected, let lastIndex = mols.firstIndex(where: { $0.id == last }),
      let thisIndex = mols.firstIndex(where: { $0.id == molID })
    {
      let range = min(lastIndex, thisIndex)...max(lastIndex, thisIndex)
      let newSelection = Set(mols[range].map { $0.id })
      selected.formUnion(newSelection)
      lastSelected = molID
    }
    lastSelected = molID

  }

  func select(_ mol: Mol) {
    prevSelectionCount = selected.count
    let molID = mol.id
    selected = [molID]
    lastSelected = molID

  }
}
