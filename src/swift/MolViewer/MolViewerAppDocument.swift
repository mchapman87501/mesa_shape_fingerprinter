import MesaacSwift
import SwiftUI
import UniformTypeIdentifiers

nonisolated struct MolViewerAppDocument: FileDocument {
  var mols: [Mol] = []

  init() {
    mols = []
  }

  static let readableContentTypes: [UTType] = [.SDFUTType]
  static let writableContentTypes: [UTType] = []

  private func molsFrom(fileWrapper: FileWrapper) throws -> [Mol] {
    if fileWrapper.isRegularFile {
      let filename = fileWrapper.filename ?? "<unnamed file>"
      return try V2000SDReader.readMols(
        regularFileContents: fileWrapper.regularFileContents, filename: filename)
    }
    print("fileWrapper has no filename.")
    return []
  }

  init(configuration: ReadConfiguration) throws {
    guard configuration.file.isRegularFile else {
      throw CocoaError(.textReadInapplicableDocumentType)
    }
    mols = try molsFrom(fileWrapper: configuration.file)
  }

  func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
    // TODO implement V2000SDWriter.writeMols.
    throw CocoaError(.featureUnsupported)
  }
}
