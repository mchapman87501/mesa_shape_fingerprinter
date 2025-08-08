import MesaacSwift
import SwiftUI
import UniformTypeIdentifiers

nonisolated struct MolViewerAppDocument: FileDocument {
  var filename = "Unnamed"
  var mols: [Mol] = []
  var errorMsgs: [String] = []

  init() {
    filename = "Unnamed"
    mols = []
    errorMsgs = []
  }

  static let readableContentTypes: [UTType] = [.SDFUTType]
  static let writableContentTypes: [UTType] = []

  private func molsFrom(fileWrapper: FileWrapper) throws -> ReadAllResult {
    if fileWrapper.isRegularFile {
      let filename = fileWrapper.filename ?? "Unnamed"
      return try SDReader.readMols(
        regularFileContents: fileWrapper.regularFileContents, filename: filename)
    }
    return ReadAllResult(mols: [], errorMsgs: ["fileWrapper is not a regular file"])
  }

  init(configuration: ReadConfiguration) throws {
    guard configuration.file.isRegularFile else {
      throw CocoaError(.textReadInapplicableDocumentType)
    }
    let readResult = try molsFrom(fileWrapper: configuration.file)
    filename = configuration.file.filename ?? "Unnamed"
    mols = readResult.mols
    errorMsgs = readResult.errorMsgs
  }

  func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
    // TODO implement V2000SDWriter.writeMols.
    throw CocoaError(.featureUnsupported)
  }
}
