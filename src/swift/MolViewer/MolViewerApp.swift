// Copyright © 2025 Mitch Chapman.  All rights reserved.

import SwiftUI

@main
struct MolViewerApp: App {
  var body: some Scene {
    DocumentGroup(newDocument: MolViewerAppDocument()) { file in
      ContentView(document: file.$document)
    }
  }
}
