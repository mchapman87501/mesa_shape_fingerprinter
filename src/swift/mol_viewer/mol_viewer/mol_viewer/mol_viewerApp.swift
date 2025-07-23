// Copyright © 2025 Mitch Chapman.  All rights reserved.

import SwiftUI

@main
struct mol_viewerApp: App {
  var body: some Scene {
    DocumentGroup(newDocument: mol_viewerAppDocument()) { file in
      ContentView(document: file.$document)
    }
  }
}
