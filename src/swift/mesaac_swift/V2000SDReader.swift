import CxxStdlib
import Foundation
import mesaac

/// Reads Mols from V2000-format SD files
public struct V2000SDReader {
  /// Read molecules from an SD file.
  /// - Parameter pathURL: file:// URL for the SD file to read
  /// - Returns: molecules read from pathURL
  public static func readMols(pathURL: URL) -> [Mol] {
    var result = [Mol]()
    let pathname = std.string(pathURL.path(percentEncoded: false))
    let t0 = Date()
    var reader = mesaac.mol.PathSDReader(pathname)
    var mesaMol = mesaac.mol.Mol()
    var molIndex = 0
    while reader.read(&mesaMol) {
      molIndex += 1
      let swiftMol = Mol(mol: mesaMol, id: "\(pathURL)-\(molIndex)")
      result.append(swiftMol)
    }
    let dt = t0.distance(to: Date())
    print("Time to read \(pathname): \(dt)")
    return result
  }

  /// Read molecules from Data.
  /// - Parameters:
  ///   - regularFileContents: a Data object holding the contents of an SD file
  ///   - filename: name of the file containing regularFileContents
  /// - Throws: if regularFileContents can't be decoded as UTF8 text
  /// - Returns: molecules read from regularFileContents
  public static func readMols(regularFileContents: Data?, filename: String) throws -> [Mol] {
    var result = [Mol]()
    guard let regularFileContents else {
      // TODO find a more appropriate exception
      throw CocoaError(.fileReadCorruptFile)
    }
    guard let contentStr = String(data: regularFileContents, encoding: .utf8) else {
      throw CocoaError(.fileReadCorruptFile)
    }
    var reader = mesaac.mol.PathSDReader(std.string(contentStr), std.string(filename))

    // TODO DRY
    var mesaMol = mesaac.mol.Mol()
    var molIndex = 0
    while reader.read(&mesaMol) {
      molIndex += 1
      let swiftMol = Mol(mol: mesaMol, id: "\(filename)-\(molIndex)")
      result.append(swiftMol)
    }
    return result
  }
}
