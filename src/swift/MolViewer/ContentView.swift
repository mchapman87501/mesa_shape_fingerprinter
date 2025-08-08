// Copyright © 2025 Mitch Chapman.  All rights reserved.

import MesaacSwift
import RealityKit
import SwiftUI
import simd

struct ContentView: View {
  @Binding var document: MolViewerAppDocument

  @State var errorsData = ErrorsViewData(filename: "Unnamed", errors: [])
  @State private var selectedItems = MolSelectionMgr()

  @Environment(\.openWindow) private var openWindow

  private let gridColumns = [GridItem(.flexible())]

  var body: some View {
    HStack(alignment: .top, spacing: 10) {
      MolListView(mols: $document.mols, selectedItems: $selectedItems)
      MolOverlayView(mols: $document.mols, selectedItems: $selectedItems)
        .frame(minWidth: 320, minHeight: 240)
    }
    .onAppear {
      selectedItems.setMols(document.mols)
      errorsData = ErrorsViewData(filename: document.filename, errors: document.errorMsgs)
    }
    .toolbar {
      ToolbarItem {
        Button(
          action: {
            openWindow(value: errorsData)
          },
          label: {
            Image(systemName: "exclamationmark.triangle")
              .symbolRenderingMode(.palette)
              .imageScale(.medium)
          }
        )
        .help("^[\(document.errorMsgs.count) read error](inflect: true)")
        .disabled(document.errorMsgs.isEmpty)
        .opacity(document.errorMsgs.isEmpty ? 0.0 : 1.0)
      }
    }
    .padding()
  }
}
