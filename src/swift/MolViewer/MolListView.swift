import MesaacSwift
import RealityKit
import SwiftUI

struct MolListView: View {
  @Binding var mols: [Mol]
  @Binding var selectedItems: MolSelectionMgr

  private let gridColumns = [GridItem(.flexible())]

  private func isSelected(_ mol: Mol) -> Bool {
    selectedItems.contains(mol)
  }

  var body: some View {
    ScrollView {
      LazyVGrid(columns: gridColumns, alignment: .leading, spacing: 4) {
        ForEach(mols) { mol in
          SingleMolView(mol: mol)
            .frame(minWidth: 128, minHeight: 96)
            .overlay(
              RoundedRectangle(cornerRadius: 5)
                .stroke(isSelected(mol) ? Color.accentColor : Color.clear, lineWidth: 2)
            )
            .gesture(
              TapGesture()
                .modifiers(.command)
                .onEnded {
                  selectedItems.cmdSelect(mol)
                }
            )
            .gesture(
              TapGesture()
                .modifiers(.shift)
                .onEnded {
                  selectedItems.shiftSelect(mol)
                }
            )
            .gesture(
              // Handler with no modifiers must come last.
              TapGesture()
                .onEnded {
                  selectedItems.select(mol)
                }
            )
        }
      }
    }
    .frame(width: 144)
  }
}
