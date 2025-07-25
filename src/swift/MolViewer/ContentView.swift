// Copyright © 2025 Mitch Chapman.  All rights reserved.

import MesaacSwift
import RealityKit
import SwiftUI
import simd

struct ContentView: View {
  @Binding var document: MolViewerAppDocument
  @State private var selectedItems = MolSelectionMgr()

  private let gridColumns = [GridItem(.flexible())]

  var body: some View {
    HStack(alignment: .top, spacing: 10) {
      MolListView(mols: $document.mols, selectedItems: $selectedItems)
      MolOverlayView(mols: $document.mols, selectedItems: $selectedItems)
        .frame(minWidth: 320, minHeight: 240)
    }
    .onAppear {
      selectedItems.setMols(document.mols)
    }
    .padding()
  }
}
