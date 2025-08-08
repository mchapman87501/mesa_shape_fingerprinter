// Copyright © 2025 Mitch Chapman.  All rights reserved.

import SwiftUI

@main
struct MolViewerApp: App {
  var body: some Scene {
    DocumentGroup(newDocument: MolViewerAppDocument()) { file in
      ContentView(document: file.$document)
    }

    WindowGroup("Structure File Errors", for: ErrorsViewData.self) { $data in
      if let filename = data?.filename {
        ErrorsView(data: $data)
          .frame(minWidth: 512, maxWidth: 768, minHeight: 288, maxHeight: 432)
          .navigationTitle("\(filename) - Errors")

      } else {
        ErrorsView(data: $data)
      }
    }
    .windowResizability(.contentSize)
    .windowIdealSize(.fitToContent)
  }
}
