import CxxStdlib
import Foundation
import mesaac

public struct ReadAllResult {
  public let mols: [Mol]
  public let errorMsgs: [String]

  public init(mols: [Mol], errorMsgs: [String]) {
    self.mols = mols
    self.errorMsgs = errorMsgs
  }
}

/// Reads Mols from V2000-format SD files
public struct SDReader {

  /// Read molecules from an SD file.
  /// - Parameter pathURL: file:// URL for the SD file to read
  /// - Returns: molecules read from pathURL, and any error messages
  public static func readMols(pathURL: URL) -> ReadAllResult {
    let pathname = std.string(pathURL.path(percentEncoded: false))
    let t0 = Date()
    defer {
      let dt = t0.distance(to: Date())
      print("Time to read \(pathname): \(dt)")
    }

    var reader = mesaac.mol.PathSDReader(pathname)
    return getMols(reader: &reader)
  }

  /// Read molecules from Data.
  /// - Parameters:
  ///   - regularFileContents: a Data object holding the contents of an SD file
  ///   - filename: name of the file containing regularFileContents
  /// - Throws: if regularFileContents can't be decoded as UTF8 text
  /// - Returns: molecules read from regularFileContents, and any error messages.
  public static func readMols(regularFileContents: Data?, filename: String) throws -> ReadAllResult
  {
    guard let regularFileContents else {
      // TODO find a more appropriate exception
      throw CocoaError(.fileReadCorruptFile)
    }
    guard let contentStr = String(data: regularFileContents, encoding: .utf8) else {
      throw CocoaError(.fileReadCorruptFile)
    }
    var reader = mesaac.mol.PathSDReader(std.string(contentStr), std.string(filename))
    return getMols(reader: &reader)
  }

  private static func getMols(reader: inout mesaac.mol.PathSDReader) -> ReadAllResult {
    let pathname = String(reader.pathname())

    let t0 = Date()
    defer {
      let dt = t0.distance(to: Date())
      print("Time to read \(pathname): \(dt)")

    }

    var mols = [Mol]()
    var errorMsgs = [String]()
    var molIndex = 0
    while !reader.eof() {
      // Always incr molIndex, so it reflects even unreadable mols
      molIndex += 1
      let readResult = reader.read()
      if !readResult.is_ok() {
        let msg = String(readResult.error())
        // Filter out one innocuous message:
        if msg != "End of file" {
          errorMsgs.append(msg)
        }
        reader.skip()
      } else {
        let smolID = "\(pathname)-\(molIndex)"
        let swiftMol = Mol(mol: readResult.value(), id: smolID)
        mols.append(swiftMol)
      }
    }

    return ReadAllResult(mols: mols, errorMsgs: errorMsgs)
  }
}
