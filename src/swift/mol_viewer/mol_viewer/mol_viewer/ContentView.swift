// Copyright © 2025 Mitch Chapman.  All rights reserved.

import MesaacSwift
// NOTE: SceneKit is deprecated.  Use RealityKit instead.
import RealityKit
import SwiftUI
import simd

struct ContentView: View {
  @Binding var document: mol_viewerAppDocument
  @State private var selectedItems = MolSelectionMgr()

  private let gridColumns = [GridItem(.flexible())]

  var body: some View {
    HStack(alignment: .top, spacing: 10) {
      MolListView(mols: $document.mols, selectedItems: $selectedItems)
      MolOverlayView(mols: $document.mols, selectedItems: $selectedItems)
        .frame(minWidth: 320, minHeight: 240)
    }
    .onAppear {
      print("Number of Mols: \(document.mols.count)")
      selectedItems.setMols(document.mols)
    }
    .padding()
  }
}
